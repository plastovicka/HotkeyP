/*
 (C) 2003-2020  Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"

int
Nvolume=2,
 diskfreePrec=1,      //disk free space precision (digits after decimal point)
 noMsg,
 sentToActiveWnd,
 lockPaste;

UINT cancelAutoPlay;
TCHAR volumeStr[256];
bool blockedKeys[256];
TCHAR *showTextLast;
PasteTextData pasteTextData;
bool keyReal[256];
static bool shifts[Nshift], shifts0[Nshift2];
const int shiftTab[Nshift]={VK_SHIFT, VK_CONTROL, VK_MENU, VK_LWIN, VK_RWIN};
const int shift2Tab[Nshift2]={VK_RMENU, VK_LSHIFT, VK_RSHIFT, VK_RCONTROL, VK_LMENU, VK_LWIN, VK_RWIN, VK_LCONTROL};

static unsigned seed; //random number seed
HWND hiddenWin, keyWin;
HideInfo hiddenApp;
HideInfo trayIconA[30];

struct Tvk {
	char *s;
	int vk;
} vks[]={
	{"ESCAPE", VK_ESCAPE}, {"ESC", VK_ESCAPE}, {"F10", VK_F10},
	{"F11", VK_F11}, {"F12", VK_F12}, {"F1", VK_F1}, {"F2", VK_F2},
	{"F3", VK_F3}, {"F4", VK_F4}, {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7},
	{"F8", VK_F8}, {"F9", VK_F9}, {"PRINTSCREEN", VK_SNAPSHOT},
	{"PRINTSCRN", VK_SNAPSHOT}, {"SCROLLLOCK", VK_SCROLL},
	{"PAUSE", VK_PAUSE}, {"BREAK", VK_CANCEL}, {"CLEAR", VK_CLEAR},
	{"INSERT", VK_INSERT}, {"INS", VK_INSERT}, {"DELETE", VK_DELETE},
	{"DEL", VK_DELETE}, {"HOME", VK_HOME}, {"END", VK_END},
	{"PAGEUP", VK_PRIOR}, {"PAGEDOWN", VK_NEXT}, {"LEFT", VK_LEFT},
	{"RIGHT", VK_RIGHT}, {"UP", VK_UP}, {"DOWN", VK_DOWN}, {"TAB", VK_TAB},
	{"CAPSLOCK", VK_CAPITAL}, {"CAPS", VK_CAPITAL}, {"BACKSPACE", VK_BACK},
	{"BS", VK_BACK}, {"ENTER", VK_RETURN}, {"SPACE", VK_SPACE},
	{"SHIFT", VK_SHIFT}, {"CONTROL", VK_CONTROL}, {"CTRL", VK_CONTROL},
	{"RSHIFT", VK_RSHIFT}, {"RCONTROL", VK_RCONTROL}, {"RCTRL", VK_RCONTROL},
	{"^", VK_CONTROL}, {"ALT", VK_MENU}, {"RALT", VK_RMENU}, {"MENU", VK_MENU}, {"RMENU", VK_RMENU},
	{"WIN", VK_LWIN}, {"LWIN", VK_LWIN}, {"RWIN", VK_RWIN}, {"APPS", VK_APPS},
	{"NUMLOCK", VK_NUMLOCK}, {"DIVIDE", VK_DIVIDE}, {"MULTIPLY", VK_MULTIPLY},
	{"SUBTRACT", VK_SUBTRACT}, {"ADD", VK_ADD}, {"DECIMAL", VK_DECIMAL},
	{"NUM0", VK_NUMPAD0}, {"NUM1", VK_NUMPAD1}, {"NUM2", VK_NUMPAD2},
	{"NUM3", VK_NUMPAD3}, {"NUM4", VK_NUMPAD4}, {"NUM5", VK_NUMPAD5},
	{"NUM6", VK_NUMPAD6}, {"NUM7", VK_NUMPAD7}, {"NUM8", VK_NUMPAD8},
	{"NUM9", VK_NUMPAD9}, {"BACK", VK_BROWSER_BACK},
	{"FORWARD", VK_BROWSER_FORWARD}, {"REFRESH", VK_BROWSER_REFRESH},
	{"SEARCH", VK_BROWSER_SEARCH}, {"FAVORITES", VK_BROWSER_FAVORITES},
	{"BROWSER", VK_BROWSER_HOME}, {"MUTE", VK_VOLUME_MUTE},
	{"VOLUME_DOWN", VK_VOLUME_DOWN}, {"VOLUME_UP", VK_VOLUME_UP},
	{"NEXT_TRACK", VK_MEDIA_NEXT_TRACK}, {"PREV_TRACK", VK_MEDIA_PREV_TRACK},
	{"STOP", VK_MEDIA_STOP}, {"PLAY_PAUSE", VK_MEDIA_PLAY_PAUSE}, {"MEDIA_SELECT", VK_LAUNCH_MEDIA_SELECT},
	{"MAIL", VK_LAUNCH_MAIL}, {"POWER", VK_SLEEP}, {"LAUNCH_APP1", VK_LAUNCH_APP1}, {"LAUNCH_APP2", VK_LAUNCH_APP2},
	{"LBUTTON", 100100+M_Left}, {"RBUTTON", 100100+M_Right}, {"MBUTTON", 100100+M_Middle},
	{"XBUTTON1", 100100+M_X1}, {"XBUTTON2", 100100+M_X1+1},
	{"WHEELUP", 100100+M_WheelUp}, {"WHEELDOWN", 100100+M_WheelDown},
	{"WHEELRIGHT", 100100+M_WheelRight}, {"WHEELLEFT", 100100+M_WheelLeft},
	{"WAIT", 100000}, {"SHOW", 100001}, {"SLEEP", 100002}, {"REP", 100003},
	{"DOUBLECLICK", 100004},
};

char *controlPanels[]={
	"appwiz", "desk", "inetcpl", "main", "mmsys",
	"powercfg", "sysdm", "timedate"
};

//---------------------------------------------------------------------------
void shiftsUp();

bool isShiftKey(int c)
{
	for(int i=0; i<Nshift; i++){
		if(c==shiftTab[i]) return true;
	}
	return false;
}

int isExtendedKey(int c)
{
	unsigned char A[]={VK_INSERT, VK_HOME, VK_PRIOR, VK_DELETE, VK_END, VK_NEXT, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_NUMLOCK, VK_DIVIDE, VK_LWIN, VK_RWIN, VK_APPS, VK_RCONTROL, VK_RMENU, VK_RSHIFT, 0};
	return strchr((char*)A, c)!=0;
}

int vkToScan(int c)
{
	return MapVirtualKey(c, 0)|(isExtendedKey(c)<<8);
}

void keyEventDown(int c)
{
	keybd_event((BYTE)c, (BYTE)MapVirtualKey(c, 0), isExtendedKey(c) ? KEYEVENTF_EXTENDEDKEY : 0, extraInfoIGNORE);
}

void keyEventUp(int c)
{
	keybd_event((BYTE)c, (BYTE)MapVirtualKey(c, 0), isExtendedKey(c) ? KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP : KEYEVENTF_KEYUP, extraInfoIGNORE);
}

void keyDown1(int c)
{
	if(!keyWin || isShiftKey(c)){
		keyEventDown(c);
	}
	if(keyWin){
		PostMessage(keyWin, shifts[shAlt] ? WM_SYSKEYDOWN : WM_KEYDOWN, c, 1|(vkToScan(c)<<16));
		Sleep(isShiftKey(c) ? 10 : 1);
	}
}

void keyDown(int c)
{
	for(int i=0; i<Nshift; i++){
		if(c==shiftTab[i]) shifts[i]=true;
	}
	keyDown1(c);
}

void keyDownI(int c)
{
	if(GetAsyncKeyState(c)>=0) keyDown(c);
}

void keyUp1(int c)
{
	if(!keyWin || isShiftKey(c)){
		keyEventUp(c);
	}
	if(keyWin){
		PostMessage(keyWin, shifts[shAlt] ? WM_SYSKEYUP : WM_KEYUP, c, 0xC0000001|(vkToScan(c)<<16));
		Sleep(1);
	}
}

void keyUp(int c)
{
	keyUp1(c);
	shiftsUp();
}

void keyPress(int c, int updown)
{
	switch(updown){
		default:
			keyDown(c);
			if(!isShiftKey(c)) keyUp(c);
			break;
		case 1:
			keyUp(c);
			break;
		case 2:
			keyDown1(c);
			break;
	}
}

void shiftsUp()
{
	for(int i=0; i<Nshift; i++){
		if(shifts[i]){
			shifts[i]=false;
			keyUp1(shiftTab[i]);
		}
	}
}

void mouse_event1(DWORD f)
{
	mouse_event(f, 0, 0, 0, 0);
}

void mouse_event2(DWORD f, int d)
{
	mouse_event(f, 0, 0, d, 0);
}

void mouse_eventDown(int i)
{
	static const DWORD D[]={MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_RIGHTDOWN, 0, 0, MOUSEEVENTF_MIDDLEDOWN};

	if(i<sizeA(D)){
		mouse_event1(D[i]);
	}
	else if(i<15){
		if(i==M_WheelDown){
			mouse_event2(MOUSEEVENTF_WHEEL, -WHEEL_DELTA);
		}
		else if(i==M_WheelUp){
			mouse_event2(MOUSEEVENTF_WHEEL, WHEEL_DELTA);
		}
		else if(i==M_WheelLeft){
			mouse_event2(MOUSEEVENTF_HWHEEL, -WHEEL_DELTA);
		}
		else if(i==M_WheelRight){
			mouse_event2(MOUSEEVENTF_HWHEEL, WHEEL_DELTA);
		}
		else{
			mouse_event2(MOUSEEVENTF_XDOWN, 1<<(i-5));
		}
	}
}

void mouse_eventUp(int i)
{
	static const DWORD U[]={MOUSEEVENTF_LEFTUP, MOUSEEVENTF_RIGHTUP, 0, 0, MOUSEEVENTF_MIDDLEUP};

	if(i<sizeA(U)){
		mouse_event1(U[i]);
	}
	else if(i<15){
		mouse_event2(MOUSEEVENTF_XUP, 1<<(i-5));
	}
}

void doubleClick()
{
	mouse_event1(MOUSEEVENTF_LEFTDOWN);
	mouse_event1(MOUSEEVENTF_LEFTUP);
	Sleep(1);
	mouse_event1(MOUSEEVENTF_LEFTDOWN);
	mouse_event1(MOUSEEVENTF_LEFTUP);
}

void waitWhileKey()
{
	bool pressed;
	int i, k;

	for(k=0; k<100; k++){
		pressed=false;
		for(i=0; i<256; i++){
			if(GetAsyncKeyState(i)<0) pressed=true;
		}
		if(!pressed) break;
		Sleep(50);
	}
}

static int waitCmd, waitCount;
static TCHAR *waitParam;

bool waitWhileKey(int cmd, TCHAR *param)
{
	if(!hookK){
		waitWhileKey();
	}
	else{
		for(int i=0; i<256; i++){
			if(keyReal[i]){
				if(waitCount>40){
					keyReal[i]=false;
					break;
				}
				waitCmd=cmd;
				cpStr(waitParam, param);
				SetTimer(hWin, 9, 60, 0);
				waitCount++;
				return false;
			}
		}
	}
	waitCount=0;
	return true;
}

void checkWait()
{
	command(waitCmd, waitParam);
}

void forceNoKey()
{
	for(int i=0; i<256; i++){
		if((GetAsyncKeyState(i)<0 || i==VK_CONTROL || i==VK_MENU)){
			keyEventUp(i);
		}
	}
}

void forceNoShift()
{
	int i;

	if(isWin9X){
		for(i=0; i<Nshift; i++){
			shifts0[i]= GetAsyncKeyState(shiftTab[i])<0;
			keyEventUp(shiftTab[i]);
		}
	}
	else{
		for(i=0; i<Nshift2; i++){
			shifts0[i]= GetAsyncKeyState(shift2Tab[i])<0;
		}
		if(preventWinMenu){
			keyEventDown(VK_CONTROL);
			keyEventUp(VK_CONTROL);
		}
		for(i=0; i<Nshift2; i++){
			if(shifts0[i]){
				//NOTE: if right Alt is released in PuTTY, then CTRL+key shortcuts in PuTTY terminal stop working
				keyEventUp(shift2Tab[i]);
			}
			else{
				//keyReal is unreliable if UAC is enabled in Windows Vista and later
				keyReal[shift2Tab[i]] = false;
			}
		}
	}
}

void restoreShift()
{
	int i;

	if(isWin9X){
		for(i=0; i<Nshift; i++){
			if(shifts0[i]){
				if(i!=shLWin && i!=shRWin) keyEventDown(shiftTab[i]);
			}
		}
	}
	else{
		if(hookK){
			for(i=0; i<Nshift2; i++){
				shifts0[i]=keyReal[shift2Tab[i]];
			}
		}
		else if(shifts0[shRAlt]) shifts0[shLCtrl]=false;

		for(i=0; i<Nshift2; i++){
			if(shifts0[i]) keyEventDown(shift2Tab[i]);
		}
		if((GetAsyncKeyState(VK_LWIN)<0 || GetAsyncKeyState(VK_RWIN)<0 || GetAsyncKeyState(VK_LMENU)<0) && !shifts0[shLCtrl]){
			//prevent the Start menu or a normal menu to show after Win+key or Alt+key
			keyEventDown(VK_CONTROL);
			keyEventUp(VK_CONTROL);
		}

		if(hookK){
			//user could release shift just after reading keyReal value, but before calling keyEventDown
			for(i=0; i<Nshift2; i++){
				if(shifts0[i] && !keyReal[shift2Tab[i]]){
					keyEventUp(shift2Tab[i]);
				}
			}
		}
	}
}

int hex(TCHAR c)
{
	if(c>='0' && c<='9') return c-'0';
	if(c>='A' && c<='F') return c-'A'+10;
	if(c>='a' && c<='f') return c-'A'+10;
	return -1;
}

int parseKey(TCHAR *&s, int &vk, int &updown)
{
	int i;
	TCHAR z;

	updown=0;
	if(*s=='\\' && *(s+1)!='\\'){
		s++;
		if(*s=='x' || *s=='X'){
			if((i=hex(s[1]))>=0 && (vk=hex(s[2]))>=0){
				vk+=i<<4;
				s+=3;
			l1:
				if(*s=='u' && s[1]=='p'){
					s+=2;
					updown=1;
				}
				else if(*s=='d' && s[1]=='o' && s[2]=='w' && s[3]=='n'){
					s+=4;
					updown=2;
				}
				if(*s==' ' || *s=='.') s++;
				return 1;
			}
		}
		for(Tvk *v=vks;; v++){
			if(v==endA(vks)){
				z=*s;
				if(z>='A' && z<='Z' || z>='0' && z<='9'){
					vk=z;
					s++;
					goto l1;
				}
				shiftsUp();
				TCHAR *k=s;
				while(*s && *s!=' ' && *s!='.' && *s!='\\') s++;
				z=*s;
				*s=0;
				msglng(744, "Unknown key name: %s", k);
				*s=z;
				return 0;
			}
			if(!strnicmpA(s, v->s)){
				s+=strlen(v->s);
				vk=v->vk;
				goto l1;
			}
		}
	}
	else{
		if(*s=='\\') s++;
		SHORT c= VkKeyScanEx(*s, GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), 0)));
		if(c==-1){
			return 3;
		}
		else{
			vk=c;
			s++;
			return 2;
		}
	}
}

void parseMacro1(TCHAR *s, int cmd)
{
	int vk, d, updown, code;
	TCHAR *s1;
	bool rep=false;

	int capslock=GetKeyState(VK_CAPITAL)&1;
	forceNoShift();
	if (keyWin) Sleep(1); //wait for shifts keyup
	while(*s){
		code=parseKey(s, vk, updown);
		if(!updown){
			if(cmd==904) updown=2; //down
			if(cmd==906) updown=1; //up
		}
		switch(code){
			case 0: //error
				return;
			case 1: //special key or command
				if(vk<100000){
					if(vk==VK_CAPITAL) capslock=0;
					keyPress(vk, updown);
				}
				else if(vk>=100100){ //LBUTTON, RBUTTON, ...
					if(updown!=1) mouse_eventDown(vk%100);
					if(updown!=2) mouse_eventUp(vk%100);
				}
				else{
					switch(vk){
						case 100000: //WAIT
							waitWhileKey();
							break;
						case 100001: //SHOW
							SetForegroundWindow(keyWin);
							Sleep(10);
							break;
						case 100002: //SLEEP
							s1=s;
							d =_tcstol(s, &s, 10);
							if(*s==' ' || *s=='.') s++;
							if(d==0){
								Sleep(s1==s ? 1000 : 10);
							}
							else{
								amax(d, 100);
								Sleep(d*100);
							}
							break;
						case 100003: //REP
							rep=true;
							break;
						case 100004: //DOUBLECLICK
							doubleClick();
							break;
					}
				}
				break;
			case 2: //character
				if(updown!=1){
					if(capslock==1){
						if(LOBYTE(vk)>='A' && LOBYTE(vk)<='Z' && (vk&0x600)==0){
							//invert shift
							vk^=0x100;
						}
						else if(vk!=VK_SPACE){
							//toggle capslock off
							keyPress(VK_CAPITAL, 0);
							capslock=2;
						}
					}
					if(vk&0x100) keyDown(VK_SHIFT);
					if(vk&0x200) keyDown(VK_CONTROL);
					if(vk&0x400) keyDown(VK_MENU);
				}
				keyPress(LOBYTE(vk), updown);
				break;
			default: //localized character
				if(cmd==906) break;
				char buf[8];
				CharToOemBuff(s, buf, 1);
				s++;
				_itoa(*(unsigned char*)buf, buf, 10);
				keyDown(VK_MENU);
				for(char *t=buf; *t; t++){
					vk= VK_NUMPAD0 + *t - '0';
					keyDown(vk);
					keyUp1(vk);
				}
				keyUp(VK_MENU);
				break;
		}
	}
	shiftsUp();
	if(capslock==2) keyPress(VK_CAPITAL, 0);
	if(rep || hookK && !isWin9X) restoreShift();
}

void parseMacro(TCHAR *s)
{
	keyWin=0;
	parseMacro1(s, 0);
}

//---------------------------------------------------------------------------
HWND getActiveWindow()
{
	HWND w=GetForegroundWindow();
	if(!w) msglng(746, "There is no foreground window");
	return w;
}

struct TfindWinInfo
{
	TCHAR *s;
	HWND found;
	int eval;
	DWORD pid;
};

BOOL CALLBACK enumWinStr(HWND hWnd, LPARAM param)
{
	int e;
	DWORD pid;
	TfindWinInfo *data= (TfindWinInfo*)param;
	TCHAR buf[512];

	if(!data->pid || !getWindowThreadProcessId(hWnd, &pid) || data->pid==pid){
		e=0;
		//compare window title
		GetWindowText(hWnd, buf, sizeA(buf));
		if(_tcsstr(buf, data->s)){
			e=20; //substring
			if(!_tcscmp(buf, data->s)) e+=10; //exact match
		}
		//compare window class
		GetClassName(hWnd, buf, sizeA(buf));
		if(!_tcscmp(buf, data->s)) e += 10;
		//better score for visible window
		if(e && IsWindowVisible(hWnd)){
			e+=100;
			LONG exstyle=GetWindowLong(hWnd, GWL_EXSTYLE);
			if(!(exstyle & WS_EX_TOOLWINDOW)) e+=50;
		}
		if(e>data->eval){
			data->eval=e;
			data->found=hWnd;
		}
	}
	return TRUE;
}

HWND getWindow2(TCHAR *s, DWORD pid)
{
	if(!s || !*s) return getActiveWindow();
	TfindWinInfo i;
	i.found=0;
	i.eval=0;
	i.s=s;
	i.pid=pid;
	EnumWindows((WNDENUMPROC)enumWinStr, (LPARAM)&i);
	HWND w=i.found;
	if(!w) w=findWindow(s, 0);
	if(!w && !noMsg){
		//msglng(745,"Window \"%s\" not found", s);
		if(trayicon && iconDelay) SetTimer(hWin, 6, iconDelay/4, 0);
	}
	return w;
}

HWND getWindow(TCHAR *s)
{
	return getWindow2(s, 0);
}

//return true if process pid is running from file exe
bool checkProcess(DWORD pid, TCHAR *exe)
{
	static TCHAR lastPath[MAX_PATH];
	static TCHAR *lastName = _T("");
	static HANDLE lastProcess;
	static DWORD lastPid;

	if(pid!=lastPid){
		if(!queryFullProcessImageName(pid, lastPath) && !queryProcessImageName(pid, lastPath)){
			//error
			lastPid=0;
			return false;
		}
		lastName=cutPath(lastPath);
		CloseHandle(lastProcess);
		//get process handle to lock pid
		lastProcess=OpenProcess(isVista ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_QUERY_INFORMATION, 0, lastPid=pid);
	}
	return !_tcsicmp(exe, lastName) || !_tcsicmp(exe, lastPath);
}


struct TwndProp {
	HWND w;
	DWORD pid;
	TCHAR title[512];
	TCHAR wndClass[512];
	TCHAR *s;
};

bool checkWindowOr(TwndProp *info);


bool checkWindowUnary(TwndProp *info)
{
	bool b=true;
	TCHAR *s, c;

	if(*info->s == '('){
		info->s++;
		b=checkWindowOr(info);
		if(*info->s == ')') info->s++;
	}
	else if(*info->s == '!'){
		info->s++;
		b= !checkWindowUnary(info);
	}
	else{
		s=info->s;
		if(*s=='\''){
			s= ++info->s;
			while(*s && *s!='\'') s++;
		}
		else{
			int par=0;
			while(*s && *s!='&' && *s!='|'){
				if(*s=='(') par++;
				if(*s==')') if(!par--) break;
				s++;
			}
		}
		c=*s;
		*s=0;
		//compare window class
		if(_tcscmp(info->wndClass, info->s))
			//compare window title
			if(!_tcsstr(info->title, info->s))
				//compare exe file name
				if(!info->pid || !checkProcess(info->pid, info->s))
					b=false;
		*s=c;
		if(c=='\'') s++;
		info->s=s;
	}
	return b;
}

bool checkWindowAnd(TwndProp *info)
{
	bool b= checkWindowUnary(info);
	while(*info->s == '&'){
		info->s++;
		b&= checkWindowUnary(info);
	}
	return b;
}

bool checkWindowOr(TwndProp *info)
{
	bool b= checkWindowAnd(info);
	while(*info->s == '|'){
		info->s++;
		b|= checkWindowAnd(info);
	}
	return b;
}

HWND checkWindow(TCHAR *s)
{
	TwndProp info;
	info.w = GetForegroundWindow();
	if(s && *s){
		//get window properties
		GetClassName(info.w, info.wndClass, sizeA(info.wndClass));
		GetWindowText(info.w, info.title, sizeA(info.title));
		info.pid=0;
		getWindowThreadProcessId(info.w, &info.pid);
		//parse expression
		info.s=s;
		bool b =checkWindowOr(&info);
		///if(*info.s) msg("Parse error");
		if(!b) return 0;
	}
	return info.w;
}

void ignoreHotkey(HotKey *hk)
{
	int i, j, id, c, vk;
	int b;

	id= int(hk-hotKeyA);
	registerHK(id, true);
	vk=hk->vkey;
	if(isWin9X && vk==vkMouse) hk->ignore=true;
	keyWin=0;
	if(hk->modifiers & MOD_SHIFT) keyDownI(VK_SHIFT);
	if(hk->modifiers & MOD_CONTROL) keyDownI(VK_CONTROL);
	if(hk->modifiers & MOD_ALT) keyDownI(VK_MENU);
	if(hk->modifiers & MOD_WIN) keyDownI(VK_LWIN);
	if(vk==vkMouse){
		c=(int)hk->scanCode;
		buttonToArray(c);
		for(j=24; j>=0; j-=4){
			i=(c>>j)&15;
			if(i>=15) continue;
			if(i==M_WheelUp || i==M_WheelDown || i==M_WheelRight || i==M_WheelLeft){
				mouse_eventDown(i);
			}
			else{
				b=1<<i;
				if(ignoreButtons & b){
					ignoreButtons&=~b;
					mouse_eventDown(i);
				}
			}
		}
		shiftsUp();
	}
	else if(vk!=vkLirc && vk!=vkJoy){
		keyDown(vk);
		if(!hookK || !keyReal[vk]) keyUp(vk);
	}
	registerHK(id, false);
}

//---------------------------------------------------------------------------
//26: //keys to another window
//74: //keys to active window
//21: //command to another window
//73: //command to active window
//94: //macro to active window
//30: //test if the window is active 
bool cmdToWindow(int cmd, TCHAR *param)
{
	TCHAR *u1, *u2, *u3, *s2, *s3, c2, c3, *s;
	HWND w;
	bool result=false;

	c2=c3=0;
	u2=u3=s3=0;
	//1.parameter
	for(s=param; *s!=' '; s++){
		if(!*s) return false;
	}
	u1=s;
	do{ s++; } while(*s==' ');
	//2.parameter
	s2=s;
	if(*s=='\"'){
		s2=(++s);
		while(*s && *s!='\"') s++;
	}
	else{
		while(*s && *s!=' ') s++;
	}
	if(*s){
		u2=s;
		do{ s++; } while(*s==' ');
		if(*s){
			//3.parameter
			s3=s;
			if(*s=='\"'){
				s3=(++s);
				while(*s && *s!='\"') s++;
				u3=s;
			}
		}
	}
	//put terminating zero characters
	*u1=0;
	if(u2){ c2=*u2; *u2=0; }
	if(u3){ c3=*u3; *u3=0; }
	//find window
	HWND(*f)(TCHAR*) = (cmd<30 ? getWindow : checkWindow);
	if(s3 && c2==' ' && (!u3 || *u3!='\"')){
		*u2=' ';
		noMsg++;
		w=f(s2);
		noMsg--;
		if(w){
			s3=0;
		}
		else{
			*u2=0;
			w=f(s2);
		}
	}
	else{
		w=f(s2);
	}
	//send keys or command
	if(w){
		result=true;
		if(cmd!=30){
			DWORD pid;
			if(s3 && getWindowThreadProcessId(w, &pid)){
				w=getWindow2(s3, pid);
			}
			if(w){
				if(cmd&1){
					PostMessage(w, WM_COMMAND, _tcstol(param, &s, 10), 0);
				}
				else{
					keyWin= (cmd>90) ? 0 : w;
					parseMacro1(param, cmd);
				}
			}
			if(cmd>30) sentToActiveWnd=2;
		}
	}
	else if(cmd>30){
		if(!sentToActiveWnd) sentToActiveWnd=1;
	}
	//restore parameter
	*u1=' ';
	if(u2) *u2=c2;
	if(u3) *u3=c3;
	return result;
}
//---------------------------------------------------------------------------
int strToIntStr(TCHAR *param, TCHAR **param2)
{
	int result = _tcstol(param, param2, 10);
	if(**param2==':'){
		*param2=param;
		return 0;
	}
	while(*param2 && **param2==' ') (*param2)++;
	return result;
}

DWORD iconProc(TCHAR *param, int hide)
{
	TCHAR *name;
	int id= strToIntStr(param, &name);
	for(int cnt=0; cnt<10; cnt++){
		Sleep(500*cnt);
		HWND w=getWindow(name);
		if(w && hideIcon(w, id, hide)) break; //success
	}
	delete[] param;
	return 0;
}

DWORD WINAPI hideIconProc(LPVOID param)
{
	return iconProc((TCHAR*)param, 1);
}

DWORD WINAPI restoreIconProc(LPVOID param)
{
	return iconProc((TCHAR*)param, 0);
}
//---------------------------------------------------------------------------

void hideWindow(HWND w, HideInfo *info)
{
	if(!info->icon || w==info->activeWnd){
		HICON icon = (HICON)GetClassLongPtr(w, GCLP_HICONSM);
		if(icon) info->icon= icon;
	}
	ShowWindowAsync(w, SW_HIDE);
	WndItem *i = new WndItem();
	i->w= w;
	i->nxt= info->list;
	info->list= i;
}

BOOL CALLBACK hideProc(HWND w, LPARAM param)
{
	HideInfo *info = (HideInfo*)param;
	DWORD pid;
	TCHAR buf[16];

	if(IsWindowVisible(w) &&
		getWindowThreadProcessId(w, &pid) && pid==info->pid){
		if(!checkProcess(pid, _T("explorer.exe")) || 
			//find all normal explorer windows (not desktop), ExploreWClass is on Windows XP or older after Browse context menu command
			GetClassName(w, buf, sizeA(buf)) && (!_tcscmp(buf, _T("CabinetWClass")) || !_tcscmp(buf, _T("ExploreWClass")) || !_tcscmp(buf, _T("#32770"))))
		{
			hideWindow(w, info);
		}
	}
	return TRUE;
}

void hideApp(HWND w, HideInfo *info, bool all)
{
	if(w && getWindowThreadProcessId(w, &info->pid)){
		info->icon=0;
		info->mustDestroyIcon=false;
		//remember foreground window
		info->activeWnd=w;
		if(all){
			DWORD pid;
			if(getWindowThreadProcessId(info->activeWnd, &pid) && pid!=info->pid){
				info->activeWnd=0;
			}
			//hide all windows which belong to the same process as window w
			EnumWindows(hideProc, (LPARAM)info);
		}
		else{
			hideWindow(w, info);
		}
	}
}

void getExeIcon(HideInfo *info)
{
	//get icon from the exe file
	TCHAR buf[MAX_PATH];
	if(queryFullProcessImageName(info->pid, buf))
		if(ExtractIconEx(buf, 0, 0, &info->icon, 1)>0) info->mustDestroyIcon=true;

	if(!info->icon){
		//get icon from window
		ULONG_PTR icon = 0;
		HWND w = info->activeWnd;
		if(w){
			icon= SendMessage(w, WM_GETICON, ICON_SMALL2, 0);
			if(!icon) icon= GetClassLongPtr(w, GCLP_HICON);
		}
		if(!icon) icon= GetClassLongPtr(hWin, GCLP_HICONSM); //HotkeyP icon
		info->icon= (HICON)icon;
	}
}

void unhideApp(HideInfo *info)
{
	if(!info->pid) return;
	for(WndItem *i= info->list; i;)
	{
		ShowWindowAsync(i->w, SW_SHOW);
		WndItem *i0 = i;
		i=i->nxt;
		delete i0;
	}
	info->list=0;
	if(info->mustDestroyIcon) DestroyIcon(info->icon);
	info->icon=0;
	info->pid=0;
	SetForegroundWindow(info->activeWnd);
}

void unhideAll()
{
	for(int i=0; i<sizeA(trayIconA); i++){
		if(trayIconA[i].pid){
			deleteTrayIcon(i+100);
			unhideApp(&trayIconA[i]);
		}
	}
	unhideApp(&hiddenApp);
	if(hiddenWin) ShowWindowAsync(hiddenWin, SW_SHOW);
}
void Tpopup::show(bool toggle)
{
	createPopups();
	if(!delay || !hWnd) return;
	int ind= static_cast<int>(this-popup);
	if(!IsWindowVisible(hWnd)){
		aminmax(x, 1, 10000);
		aminmax(y, 1, 10000);
		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);
		aminmax(width, 50, cx);
		aminmax(height, 10, cy);
		SetWindowPos(hWnd, HWND_TOPMOST,
			int(cx*x/10000)-width/2,
			int(cy*y/10000)-height/2,
			width, height, SWP_NOACTIVATE);
		ShowWindow(hWnd, SW_SHOWNOACTIVATE);
		SetTimer(hWin, 60+ind, refresh, 0);
	}
	else if(toggle){
		//hide
		PostMessage(hWin, WM_TIMER, 50+ind, 0);
		return;
	}
	if(cmdLineCmd>=0){
		UpdateWindow(hWnd);
		Sleep(delay);
		ShowWindow(hWnd, SW_HIDE);
	}
	else{
		SetTimer(hWin, 50+ind, delay, 0);
	}
}

void volumeCmd(TCHAR *which, int value, int action)
{
	volume(which, value, action);
	//show window
	popupVolume.height= volumeH();
	popupVolume.show(false);
}

//------------------------------------------------------------------

static int cdDoorResult;
static HANDLE hDevice;
static BYTE inqData[36];

struct SCSI_PASS_THROUGH_DIRECT {
	USHORT Length;
	UCHAR ScsiStatus;
	UCHAR PathId;
	UCHAR TargetId;
	UCHAR Lun;
	UCHAR CdbLength;
	UCHAR SenseInfoLength;
	UCHAR DataIn;
	ULONG DataTransferLength;
	ULONG TimeOutValue;
	PVOID DataBuffer;
	ULONG SenseInfoOffset;
	UCHAR Cdb[16];
};

struct SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT spt;
	UCHAR ucSenseBuf[32];
};

#ifndef CTL_CODE
#define CTL_CODE(DevType, Function, Method, Access) \
	(((DevType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif
#define IOCTL_SCSI_PASS_THROUGH_DIRECT CTL_CODE(4, 0x405, 0, 3)
#define SCSI_IOCTL_DATA_IN 1
#define DTC_WORM 4
#define DTC_CDROM 5


int getCDinfo(TCHAR letter)
{
	ULONG returned;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb;
	char buf[12];

	sprintf(buf, "\\\\.\\%c:", letter);
	hDevice = CreateFileA(buf, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hDevice == INVALID_HANDLE_VALUE){
		hDevice = CreateFileA(buf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
	}
	if(hDevice != INVALID_HANDLE_VALUE){
		memset(&swb, 0, sizeof(swb));
		swb.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		swb.spt.CdbLength = 6;
		swb.spt.SenseInfoLength = 24;
		swb.spt.DataIn = SCSI_IOCTL_DATA_IN;
		swb.spt.DataTransferLength = sizeof(inqData);
		swb.spt.TimeOutValue = 5;
		swb.spt.DataBuffer = &inqData;
		swb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
		swb.spt.Cdb[0] = 0x12;
		swb.spt.Cdb[4] = 0x24;
		if(DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
				&swb, sizeof(swb), &swb, sizeof(swb), &returned, NULL)){
			return 1;
		}
	}
	return 0;
}

//action: 0=info,2=eject,3=close,4=speed
void cdCmd(TCHAR letter, int action, int param=0)
{
	ULONG	returned;
	bool bBeenHereBefore=false;
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb;

	if(getCDinfo(letter)){
		int btDeviceType= inqData[0] & 15;
		if(btDeviceType==DTC_CDROM || btDeviceType==DTC_WORM){
		start:
			memset(&swb, 0, sizeof(swb));
			swb.spt.CdbLength = 6;
			if(action==2 || action==3){
				swb.spt.Cdb[0]=0x1B;
				swb.spt.Cdb[4]=(BYTE)action;
			}
			if(action==4){
				swb.spt.Cdb[0] = 0xBB;
				//swb.spt.Cdb[1] = (BYTE)(LunID<<5);	
				param = unsigned(176.4 * param + 1);
				swb.spt.Cdb[2] = HIBYTE(param);
				swb.spt.Cdb[3] = LOBYTE(param);
				swb.spt.Cdb[4] = swb.spt.Cdb[5] = 0xFF;
				swb.spt.CdbLength = 12;
			}
			swb.spt.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
			swb.spt.DataIn = SCSI_IOCTL_DATA_IN;
			swb.spt.TimeOutValue = 15;
			swb.spt.SenseInfoLength = 14;
			swb.spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);

			if(DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
					&swb, sizeof(swb), &swb, sizeof(swb), &returned, NULL)){
				int s= swb.ucSenseBuf[2] & 0xf;
				if(s==0) cdDoorResult=1;
				if(s==2 && swb.ucSenseBuf[12]==0x3A){
					cdDoorResult= swb.ucSenseBuf[13];
				}
			}
			else{
				DWORD dwErrCode = GetLastError();
				if(!bBeenHereBefore &&
					(dwErrCode == ERROR_MEDIA_CHANGED || dwErrCode == ERROR_INVALID_HANDLE)){
					CloseHandle(hDevice);
					getCDinfo(letter);
					bBeenHereBefore=true;
					goto start;
				}
			}
		}
	}
	CloseHandle(hDevice);
}

DWORD WINAPI cdProc(LPVOID param)
{
	MCI_OPEN_PARMS m;
	MCI_PLAY_PARMS pp;
	MCI_STATUS_PARMS sp;
	MCI_GENERIC_PARMS gp;

	int op=(((int)param)>>8)&255;
	int speed=((int)param)>>16;
	TCHAR d[3];
	d[0]=(TCHAR)(int)param;
	d[1]=':';
	d[2]=0;

	EnterCriticalSection(&cdCritSect);
	m.dwCallback=0;
	m.lpstrDeviceType=_T("CDAudio");
	m.lpstrElementName=d;
	m.lpstrAlias=0;
	if(mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_ELEMENT, (WPARAM)&m)){
		msglng(748, "Drive is already used by another process");
	}
	else{
		if(op==62){ //eject/close
			cdDoorResult=0;
			cdCmd(d[0], 0);
			if(cdDoorResult==1){
				cdCmd(d[0], 2);
				op=-1;
			}
			else if(cdDoorResult==2){
				cdCmd(d[0], 3);
				op=-1;
			}
			else{ //Windows 95/98/ME
				DWORD t=GetTickCount();
				mciSendCommand(m.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, 0);
				if(GetTickCount()-t < 200) op=1;
			}
		}
		switch(op){
			case 0: //eject
				mciSendCommand(m.wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, 0);
				break;
			case 1: //close
				mciSendCommand(m.wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, 0);
				break;
			case 39: //play
				mciSendCommand(m.wDeviceID, MCI_PLAY, 0, 0);
				break;
			case 41: //stop
				mciSendCommand(m.wDeviceID, MCI_STOP, 0, (WPARAM)&gp);
				break;
			case 40: //next
			case 42: //previous
				sp.dwItem=MCI_STATUS_CURRENT_TRACK;
				if(!mciSendCommand(m.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (WPARAM)&sp)){
					sp.dwTrack=(DWORD)sp.dwReturn+41-op;
					sp.dwItem=MCI_STATUS_POSITION;
					if(!mciSendCommand(m.wDeviceID, MCI_STATUS, MCI_TRACK|MCI_STATUS_ITEM, (WPARAM)&sp)){
						pp.dwFrom=(DWORD)sp.dwReturn;
						mciSendCommand(m.wDeviceID, MCI_PLAY, MCI_FROM, (WPARAM)&pp);
					}
				}
				break;
			case 100: //speed
				cdCmd(d[0], 4, speed);
				break;
		}
		mciSendCommand(m.wDeviceID, MCI_CLOSE, MCI_OPEN_TYPE|MCI_OPEN_ELEMENT, (WPARAM)&m);
	}
	LeaveCriticalSection(&cdCritSect);
	return 0;
}

void createThread(LPTHREAD_START_ROUTINE proc, LPVOID param)
{
	DWORD id;
	CloseHandle(CreateThread(0, 0, proc, param, 0, &id));
}

//CD command, drive is e.g. "D:"
void audioCD(TCHAR *drive, int op, int speed=0)
{
	TCHAR d[3];

	d[1]=':';
	d[2]=0;
	if(!drive || !*drive){
		for(d[0]='A';; d[0]++){
			if(GetDriveType(d)==DRIVE_CDROM) break;
			if(d[0]=='Z') return;
		}
	}
	else{
		d[0]=*drive;
	}
	if(GetDriveType(d)!=DRIVE_CDROM){
		msglng(747, "%s is not a CD drive", d);
		return;
	}
	LPVOID param = (LPVOID)((op<<8)|d[0]|(speed<<16));
	if(cmdLineCmd>=0){
		//do not create a thread if running from the command line,
		// because it would be immediately terminated at the end of WinMain
		cdProc(param);
	}
	else{
		createThread(cdProc, param);
	}
}

DWORD strToDword(TCHAR *&s)
{
	return _tcstoul(s, &s, 10);
}

LONG changeDisplay(char *device, DEVMODEA &dm, DWORD flag)
{
	if(device){
		TChangeDisplaySettingsEx p= (TChangeDisplaySettingsEx)GetProcAddress(GetModuleHandleA("user32.dll"), "ChangeDisplaySettingsExA");
		return p(device, &dm, 0, flag, 0);
	}
	else{
		return ChangeDisplaySettingsA(&dm, flag);
	}
}

//change monitor resolution, color bit depth, frequency, display number
void resolution(TCHAR *param)
{
	DWORD w, h, c, f, d, w2, h2, c2, f2;
	TCHAR *s;
	DEVMODEA dm;
	char device[22];
	strcpy(device, "\\\\.\\DISPLAY0");

	s=param;
	w=strToDword(s);
	h=strToDword(s);
	c=strToDword(s);
	f=strToDword(s);
	d=strToDword(s);
	if(d>0) _itoa(d, device+11, 10);
	char *dev = (d>0) ? device : 0;

	dm.dmSize = sizeof(dm);
	if(!EnumDisplaySettingsA(dev, ENUM_CURRENT_SETTINGS, &dm)){
		msg(_T("EnumDisplaySettings failed for display %d"), d); // (error code %d)", d, GetLastError());
		return;
	}
	if(*param){
		while(*s==' ') s++;
		if(*s==','){
			s++;
			w2=strToDword(s);
			h2=strToDword(s);
			c2=strToDword(s);
			f2=strToDword(s);
			if((!w || dm.dmPelsWidth==w) &&
				(w2!=w || (!c || dm.dmBitsPerPel==c) &&
				(!f || dm.dmDisplayFrequency==f))){
				w=w2; h=h2; c=c2; f=f2;
			}
		}
		if(w) dm.dmPelsWidth=w;
		if(h) dm.dmPelsHeight=h;
		if(c) dm.dmBitsPerPel=c;
		if(f) dm.dmDisplayFrequency=f;
	}
	else{
		if(dm.dmPelsWidth==800){
			dm.dmPelsWidth=1024;
			dm.dmPelsHeight=768;
		}
		else{
			dm.dmPelsWidth=800;
			dm.dmPelsHeight=600;
		}
	}

	dm.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL|DM_DISPLAYFREQUENCY;
	LONG err = changeDisplay(dev, dm, CDS_UPDATEREGISTRY);
	if(err==DISP_CHANGE_NOTUPDATED) err = changeDisplay(dev, dm, 0);
	switch(err){
		case DISP_CHANGE_SUCCESSFUL:
			break;
		case DISP_CHANGE_BADMODE:
			msg(_T("The graphics mode is not supported"));
			break;
		case DISP_CHANGE_RESTART:
			msg(_T("The computer must be restarted in order for the graphics mode to work"));
			break;
		default:
			msg(_T("ChangeDisplaySettings failed for display %d (error code %d)"), d, err);
			break;
	}
}

struct TsearchInfo
{
	TCHAR *result;
	int recurse, action, pass;
	bool done;
	unsigned maxWallpaper;
	TCHAR const *last;
	TCHAR prev[MAX_PATH];
	TCHAR lastPath[MAX_PATH];
};

void getFullPathName(TCHAR *fn, TCHAR *path)
{
	TCHAR *dummy;
	GetFullPathName(fn, 256, path, &dummy);
}

void search(TCHAR *dir, TsearchInfo *info)
{
	HANDLE h;
	WIN32_FIND_DATA fd;
	TCHAR *s, oldDir[MAX_PATH], ext[4];

	GetCurrentDirectory(sizeA(oldDir), oldDir);
	SetCurrentDirectory(dir);

	h = FindFirstFile(_T("*.*"), &fd);
	if(h!=INVALID_HANDLE_VALUE){
		do{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(fd.cFileName[0]!='.' && info->recurse){
					search(fd.cFileName, info);
				}
			}
			else{
				s=_tcschr(fd.cFileName, 0);
				while(*--s!='\\'){
					if(*s=='.'){
						ext[0]=(TCHAR)toupper(s[1]);
						ext[1]=(TCHAR)toupper(s[2]);
						ext[2]=(TCHAR)toupper(s[3]);
						if(ext[0]=='B' && ext[1]=='M' && ext[2]=='P' ||
							 ext[0]=='J' && ext[1]=='P' && (ext[2]=='G' || ext[2]=='E') ||
							 ext[0]=='G' && ext[1]=='I' && ext[2]=='F'){
							switch(info->action){
								default: //random
									seed=seed*367413989+174680251;
									if(seed>=info->maxWallpaper){
										info->maxWallpaper=seed;
										getFullPathName(fd.cFileName, info->result);
									}
									break;
								case 1: //next
									getFullPathName(fd.cFileName, info->result);
									if(info->pass){
										info->done=true;
									}
									else if(!_tcsicmp(info->last, fd.cFileName) &&
									 !_tcsicmp(info->lastPath, info->result)){
										info->pass++;
									}
									break;
								case 2: //previous
									getFullPathName(fd.cFileName, info->result);
									if(!_tcsicmp(info->last, fd.cFileName) &&
										!_tcsicmp(info->lastPath, info->result)){
										if(info->prev[0]){
											_tcscpy(info->result, info->prev);
											info->done=true;
										}
									}
									else _tcscpy(info->prev, info->result);
									break;
							}
						}
						break;
					}
				}
			}
		} while(!info->done && FindNextFile(h, &fd));
		FindClose(h);
	}
	SetCurrentDirectory(oldDir);
}

void searchC(TCHAR *dir, TsearchInfo *info)
{
	info->done=false;
	info->pass=0;
	do{
		search(dir, info);
		info->pass++;
	} while(info->action && !info->done && info->pass<5);
}

//change wallpaper
void changeWallpaper(TCHAR *dir, int action)
{
	HKEY key;
	DWORD d=0, d2;
	int foundInd=0, b;
	TCHAR result0[MAX_PATH], *u, *s, *wallpaperStr=0, *wallpaperStr2;
	TsearchInfo info;

	//remove quotes
	u=0;
	if(dir[0]=='\"'){
		u=_tcschr(dir, 0)-1;
		if(*u=='\"'){
			dir++;
			*u=0;
		}
		else{
			u=0;
		}
	}
	info.lastPath[0]=0;
	//registry value contains pairs (directory, last wallpaper file), both are null-terminated
	if(RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_QUERY_VALUE, &key)==ERROR_SUCCESS){
		if(RegQueryValueEx(key, _T("wallpaper"), 0, 0, 0, &d2)==ERROR_SUCCESS){
			d=d2/sizeof(TCHAR);
			if(d>0){
				wallpaperStr= new TCHAR[d+4];
				if(RegQueryValueEx(key, _T("wallpaper"), 0, 0, (BYTE*)wallpaperStr, &d2)==ERROR_SUCCESS){
					if(wallpaperStr[d-1]){
						//error, value is not null-terminated
						d=0;
					}
					else{
						*(DWORD*)(wallpaperStr+d)=0;
						//find dir
						for(s=wallpaperStr; (DWORD)(s-wallpaperStr)<d; s=_tcschr(s, 0)+1){
							b=_tcscmp(s, dir);
							s=_tcschr(s, 0)+1;
							d2=static_cast<int>(s-wallpaperStr);
							if(d<=d2) d=d2+1;
							if(!b){
								//dir was found, copy file name to lastPath
								lstrcpyn(info.lastPath, s, sizeA(info.lastPath));
								foundInd=int(s-wallpaperStr);
								break;
							}
						}
					}
				}
			}
		}
		RegCloseKey(key);
	}

	seed=(unsigned)GetTickCount();
	info.action=action;
	info.maxWallpaper=0;
	info.result=result0;
	result0[0]=0;
	info.prev[0]=0;
	info.last=cutPath(info.lastPath);
	if(*dir){
		DWORD a = GetFileAttributes(dir);
		if(a==0xFFFFFFFF){
			msglng(749, "Cannot find folder or file %s", dir);
		}
		else if(!(a&FILE_ATTRIBUTE_DIRECTORY)){
			//parameter is file name
			info.result=dir;
		}
		else{
			//parameter is folder
			info.recurse=1;
			searchC(dir, &info);
		}
	}
	else{
		//hotkey has no parameter
		TCHAR buf[256];
		if(GetWindowsDirectory(buf, sizeA(buf))){
			info.recurse=0;
			searchC(buf, &info);
		}
	}
	if(info.result[0]){
		//write file name to registry
		if(RegCreateKey(HKEY_CURRENT_USER, subkey, &key)==ERROR_SUCCESS){
			if(!foundInd){
				//append dir to registry value
				d2=(int)_tcslen(dir)+2;
				wallpaperStr2=wallpaperStr;
				wallpaperStr=new TCHAR[d+d2];
				memcpy(wallpaperStr, wallpaperStr2, d * sizeof(TCHAR));
				delete[] wallpaperStr2;
				_tcscpy(wallpaperStr+d, dir);
				d+=d2;
				//append null character
				wallpaperStr[foundInd=d-1]=0;
			}
			//replace previous file name with result
			d2=(int)_tcslen(info.result)-(int)_tcslen(wallpaperStr+foundInd);
			d+=d2;
			wallpaperStr2=wallpaperStr;
			wallpaperStr=new TCHAR[d];
			memcpy(wallpaperStr, wallpaperStr2, foundInd * sizeof(TCHAR));
			_tcscpy(wallpaperStr+foundInd, info.result);
			memcpy(_tcschr(wallpaperStr+foundInd, 0), _tcschr(wallpaperStr2+foundInd, 0), (d-foundInd-(int)_tcslen(info.result)) * sizeof(TCHAR));
			delete[] wallpaperStr2;
			RegSetValueEx(key, _T("wallpaper"), 0, REG_BINARY, (BYTE*)wallpaperStr, d * sizeof(TCHAR));
			RegCloseKey(key);
		}
		//change wallpaper
		if(!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, info.result, SPIF_UPDATEINIFILE|SPIF_SENDCHANGE)){
#ifndef NOWALLPAPER
			CoInitialize(0);
			HRESULT hr;
			IActiveDesktop *pActiveDesktop;
			hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
				IID_IActiveDesktop, (void**)&pActiveDesktop);
			if(hr==S_OK){
				convertT2W(info.result, resultW);
				pActiveDesktop->SetWallpaper(resultW, 0);
				pActiveDesktop->ApplyChanges(AD_APPLY_ALL);
				pActiveDesktop->Release();
			}
			CoUninitialize();
			DWORD_PTR dummy;
			SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETDESKWALLPAPER, 0, SMTO_ABORTIFHUNG, 100, &dummy);
#endif
		}
		writeini();
	}
	if(u) *u='\"';
	delete[] wallpaperStr;
}

void deleteTemp(TCHAR *dir, FILETIME *time)
{
	HANDLE h;
	bool b;
	WIN32_FIND_DATA fd;
	TCHAR oldDir[MAX_PATH];

	if(GetCurrentDirectory(sizeA(oldDir), oldDir)){
		if(SetCurrentDirectory(dir)){
			h = FindFirstFile(_T("*"), &fd);
			if(h!=INVALID_HANDLE_VALUE){
				do{
					b= *(LONGLONG*)&fd.ftLastWriteTime < *(LONGLONG*)time &&
						*(LONGLONG*)&fd.ftCreationTime < *(LONGLONG*)time;
					if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
						if(fd.cFileName[0]!='.'){
							deleteTemp(fd.cFileName, time);
							if(b) RemoveDirectory(fd.cFileName);
						}
					}
					else if(b)
					{
						if(!DeleteFile(fd.cFileName))
						{
							//delete read-only files
							DWORD attr=GetFileAttributes(fd.cFileName);
							if(attr & FILE_ATTRIBUTE_READONLY){
								attr-=FILE_ATTRIBUTE_READONLY;
								SetFileAttributes(fd.cFileName, attr);
								DeleteFile(fd.cFileName);
							}
						}
					}
				} while(FindNextFile(h, &fd));
				FindClose(h);
			}
		}
		SetCurrentDirectory(oldDir);
	}
}

void privilege(TCHAR *name)
{
	HANDLE token;
	TOKEN_PRIVILEGES tkp;

	if(OpenProcessToken(GetCurrentProcess(),
		 TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &token))
	 if(LookupPrivilegeValue(0, name, &tkp.Privileges[0].Luid)){
		 tkp.PrivilegeCount=1;
		 tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		 AdjustTokenPrivileges(token, FALSE, &tkp, 0, 0, 0);
	 }
}

//set privileges to shutdown or restart computer
void privilege()
{
	saveAtExit();
	privilege(SE_SHUTDOWN_NAME);
}


BOOL CALLBACK closeProc(HWND w, LPARAM explorerPid)
{
	DWORD pid;
	DWORD_PTR r;

	if(IsWindowVisible(w) && GetWindowThreadProcessId(w, &pid) &&
		pid!=(DWORD)explorerPid && pid!=GetCurrentProcessId()){
		SendMessageTimeout(w, WM_SYSCOMMAND, SC_CLOSE, 0,
			SMTO_ABORTIFHUNG|SMTO_BLOCK, 10000, &r);
	}
	return TRUE;
}

static bool isCloseAll;

void closeAll(int param)
{
	if(param==2) isCloseAll=true;
}

void closeAll2()
{
	if(isCloseAll) EnumWindows(closeProc, findProcess(_T("explorer.exe")));
}

void saveDesktopIcons(int action)
{
	HWND w = FindWindowEx(FindWindowExA(FindWindowA("Progman", "Program Manager"), 0, "SHELLDLL_DefView", 0), 0, WC_LISTVIEW, 0/*"FolderView"*/);
	if(w){
		DWORD tid = GetWindowThreadProcessId(w, 0);
		if(!klib) klib=LoadLibraryA("hook.dll");
		HOOKPROC hproc = (HOOKPROC)GetProcAddress(klib, "_CallWndProcD@12");
		if(hproc){
			HHOOK hook= SetWindowsHookEx(WH_CALLWNDPROC, hproc, (HINSTANCE)klib, tid);
			if(hook){
				SendMessage(w, WM_USER+47348+action, 0, 0);
				UnhookWindowsHookEx(hook);
				if(action==0 && cmdFromKeyPress && !noMsg)
				{
					msg1(MB_OK|MB_ICONINFORMATION, lng(766, "Desktop icon positions are saved"));
				}
			}
		}
	}
}



void resetDiskfree()
{
	for(int i=0; i<26; i++){
		disks[i].text[0]='\0';
	}
}

void addDrive(char *root)
{
	DWORDLONG free, total;
	DiskInfo *d;
	TdiskFreeFunc p;

	p= (TdiskFreeFunc)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetDiskFreeSpaceExA");
	if(p){
		if(!p(root, (ULARGE_INTEGER*)&free, (ULARGE_INTEGER*)&total, 0)) return;
	}
	else{
		DWORD sectPerClust, bytesPerSect, freeClust, totalClust;
		if(!GetDiskFreeSpaceA(root, &sectPerClust, &bytesPerSect, &freeClust, &totalClust)) return;
		free= freeClust*(DWORDLONG)sectPerClust*bytesPerSect;
		total= totalClust*(DWORDLONG)sectPerClust*bytesPerSect;
	}
	d= &disks[root[0]-'A'];
	if(d->free!=free || !d->text[0]){
		d->free=free;
		d->bar=(int)(free*1000/total);
		if(free>2000000000){
			sprintf(d->text, "%.*f GB", diskfreePrec, (double)(LONGLONG)free/1073741824);
		}
		else if(free>2000000){
			sprintf(d->text, "%.*f MB", diskfreePrec, (double)(LONGLONG)free/1048576);
		}
		else{
			sprintf(d->text, "%d kB", (int)(free/1024));
		}
		RECT rc;
		rc.left=0;
		rc.right= popupDiskFree.width;
		rc.top= popupDiskFree.height;
		rc.bottom= rc.top + fontH;
		InvalidateRect(popupDiskFree.hWnd, &rc, FALSE);
	}
}

void diskFree()
{
	UINT t;
	char c, root[4];

	root[1]=':';
	root[2]='\\';
	root[3]=0;
	popupDiskFree.height=0;
	for(c='A'; c<='Z'; c++){
		root[0]=c;
		t=GetDriveTypeA(root);
		if(t==DRIVE_FIXED || t==DRIVE_REMOTE){
			popupDiskFree.height+= diskSepH+1;
			addDrive(root);
			popupDiskFree.height+= fontH+diskSepH;
		}
		else{
			disks[c-'A'].text[0]=0;
		}
	}
	popupDiskFree.height++;
}

void screenshot(HWND wnd)
{
	BITMAPFILEHEADER hdr;
	HBITMAP hBmp;
	HDC winDC, dcb;
	BYTE *bits;
	FILE *f;
	RECT rc;
	int w, h, palSize;
	HGDIOBJ oldB;
	BITMAP b;
	BITMAPINFOHEADER &bmi = *(BITMAPINFOHEADER*)_alloca(sizeof(BITMAPINFOHEADER)+1024);

	//create bitmap
	winDC= GetWindowDC(wnd);
	dcb= CreateCompatibleDC(winDC);
	GetWindowRect(wnd, &rc);
	w= rc.right-rc.left;
	h= rc.bottom-rc.top;
	hBmp= CreateCompatibleBitmap(winDC, w, h);
	oldB= SelectObject(dcb, hBmp);
	//snapshot
	BitBlt(dcb, 0, 0, w, h, winDC, 0, 0, SRCCOPY);
	GetObject(hBmp, sizeof(BITMAP), &b);
	SelectObject(dcb, oldB);
	//initialize bitmap structure
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = b.bmWidth;
	bmi.biHeight = b.bmHeight;
	bmi.biPlanes = b.bmPlanes;
	bmi.biBitCount = b.bmBitsPixel;
	bmi.biCompression = BI_RGB;
	bmi.biXPelsPerMeter=bmi.biYPelsPerMeter=bmi.biClrImportant=0;

	//get pixels
	bits = new BYTE[h*(w*4+4)];
	if(!GetDIBits(dcb, hBmp, 0, h, bits, (BITMAPINFO*)&bmi, DIB_RGB_COLORS)){
		msg(_T("GetDIBits failed"));
	}
	else{
		//save file
		if(save1(snapOfn, OFN_PATHMUSTEXIST)){
			f=_tfopen(snapOfn.lpstrFile, _T("wb"));
			if(!f){
				msglng(733, "Cannot create file %s", snapOfn.lpstrFile);
			}
			else{
				hdr.bfType = 0x4d42;  //'BM'
				bmi.biClrUsed = (b.bmBitsPixel<16) ? 1<<b.bmBitsPixel : 0;
				palSize = bmi.biClrUsed * sizeof(RGBQUAD);
				hdr.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+palSize+bmi.biSizeImage;
				hdr.bfReserved1 = hdr.bfReserved2 = 0;
				hdr.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+palSize;
				fwrite(&hdr, sizeof(BITMAPFILEHEADER), 1, f);
				fwrite(&bmi, sizeof(BITMAPINFOHEADER)+palSize, 1, f);
				fwrite(bits, bmi.biSizeImage, 1, f);
				fclose(f);
			}
		}
	}
	delete[] bits;
	ReleaseDC(wnd, winDC);
	DeleteObject(hBmp);
	DeleteDC(dcb);
}

void moveCmd(TCHAR *param, int actionx, int actiony)
{
	RECT rcd, rcw;
	int x, y;
	HWND w;

	w=getWindow(param);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcd, 0);
	GetWindowRect(w, &rcw);
	switch(actionx){
		default: //don't change
			x=rcw.left;
			break;
		case 1: //left
			x=rcd.left;
			break;
		case 2: //right
			x=rcd.right-rcw.right+rcw.left;
			break;
		case 3: //center
			x=(rcd.right + rcd.left - rcw.right + rcw.left)>>1;
			break;
	}
	switch(actiony){
		default: //don't change
			y=rcw.top;
			break;
		case 1: //up
			y=rcd.top;
			break;
		case 2: //down
			y=rcd.bottom-rcw.bottom+rcw.top;
			break;
		case 3: //center
			y=(rcd.bottom + rcd.top - rcw.bottom + rcw.top)>>1;
			break;
	}
	SetWindowPos(w, 0, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_ASYNCWINDOWPOS);
}

void priority(int p)
{
	DWORD pid;
	HANDLE h;
	HWND w;

	w=getWindow(0);
	if(w){
		BOOL ok=FALSE;
		if(getWindowThreadProcessId(w, &pid)){
			if((h=OpenProcess(PROCESS_SET_INFORMATION, 0, pid))!=0){
				aminmax(p, 0, Npriority-1);
				ok=SetPriorityClass(h, priorCnst[p]);
				CloseHandle(h);
			}
		}
		if(!ok) msglng(750, "Can't change priority");
	}
}

int getOpacity(HWND w)
{
	if(w){
		static TGetLayeredWindowAttributes getLayeredWindowAttributes;
		if(!getLayeredWindowAttributes)
			getLayeredWindowAttributes= (TGetLayeredWindowAttributes)GetProcAddress(GetModuleHandleA("user32.dll"), "GetLayeredWindowAttributes");
		if(getLayeredWindowAttributes){
			COLORREF color;
			BYTE alpha;
			DWORD flags;
			if(getLayeredWindowAttributes(w, &color, &alpha, &flags) && (flags & LWA_ALPHA))
				return alpha;
			return 255;
		}
	}
	return -1;
}

void setOpacity(HWND w, int o)
{
	if(w){
		static TsetOpacityFunc p;
		if(!p){
			p= (TsetOpacityFunc)GetProcAddress(GetModuleHandleA("user32.dll"), "SetLayeredWindowAttributes");
			if(!p) return;
		}
		LONG s=GetWindowLong(w, GWL_EXSTYLE);
		if(((s&0x80000)==0) != (o==255)){
			SetWindowLong(w, GWL_EXSTYLE, s^0x80000);
		}
		p(w, 0, (BYTE)min(o, 255), 2);
	}
}

static HWND noMinimWnd;

BOOL CALLBACK enumMinimize(HWND hWnd, LPARAM)
{
	LONG s=GetWindowLong(hWnd, GWL_STYLE);
	if((s&WS_MINIMIZEBOX) && IsWindowVisible(hWnd) &&
		!GetWindow(hWnd, GW_OWNER) && hWnd!=noMinimWnd){
		RECT rc;
		GetWindowRect(hWnd, &rc);
		if(IsRectEmpty(&rc)){
			//Borland Delphi application
			PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else{
			ShowWindow(hWnd, SW_MINIMIZE);
		}
	}
	return TRUE;
}

BOOL CALLBACK enumMaximize(HWND hWnd, LPARAM ignore)
{
	LONG s=GetWindowLong(hWnd, GWL_STYLE);
	if((s&WS_MAXIMIZEBOX) && IsWindowVisible(hWnd) && (LPARAM)hWnd!=ignore)
	{
		HWND owner=GetWindow(hWnd, GW_OWNER);
		if(owner)
		{
			LONG s1=GetWindowLong(owner, GWL_STYLE);
			if(s1&WS_SYSMENU){
				RECT rc;
				GetWindowRect(owner, &rc);
				if(IsRectEmpty(&rc)){
					//Borland Delphi application
					PostMessage(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				}
			}
		}
		else{
			ShowWindowAsync(hWnd, SW_MAXIMIZE);
		}
	}
	return TRUE;
}

//-------------------------------------------------------------------------
void PasteTextData::save()
{
	HGLOBAL hmem;
	void *ptr;
	UINT f;
	ClipboardData *cd, *cdn, **pnxt;

	if(GetClipboardOwner()==hWin) return;
	if(OpenClipboard(0)){
		for(cd=prev; cd; cd=cdn){
			cdn=cd->nxt;
			operator delete(cd->data);
			delete cd;
		}
		pnxt= &prev;
		f=0;
		while((f=EnumClipboardFormats(f))!=0){
			if((hmem= GetClipboardData(f))!=0){
				if((ptr=GlobalLock(hmem))!=0){
					cd= new ClipboardData;
					cd->format=f;
					cd->size= GlobalSize(hmem);
					cd->data= operator new(cd->size);
					memcpy(cd->data, ptr, cd->size);
					GlobalUnlock(hmem);
					*pnxt= cd;
					pnxt= &cd->nxt;
				}
			}
		}
		*pnxt=0;
		CloseClipboard();
	}
}

void PasteTextData::restore()
{
	HGLOBAL hmem;
	void *ptr;
	int i;
	ClipboardData *cd;

	if(prev){
		for(i=0; i<20; i++){
			if(OpenClipboard(hWin)){
				if(EmptyClipboard()){
					for(cd=prev; cd; cd=cd->nxt){
						if((hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, cd->size))!=0){
							if((ptr=GlobalLock(hmem))!=0){
								memcpy(ptr, cd->data, cd->size);
								GlobalUnlock(hmem);
								SetClipboardData(cd->format, hmem);
							}
							else{
								GlobalFree(hmem);
							}
						}
					}
				}
				CloseClipboard();
				break;
			}
			Sleep(100);
		}
	}
}

static void resizeBuffer(TCHAR *&buf, size_t &bufLen, TCHAR *&ptr, size_t M)
{
	size_t len= ptr-buf;
	if(len+M >= bufLen){
		//increase buffer size
		TCHAR *buf2=new TCHAR[bufLen+=M];
		memcpy(buf2, buf, len * sizeof(TCHAR));
		ptr= buf2+len;
		delete[] buf;
		buf=buf2;
	}
}

//copy text from clipboard to dest
bool pasteFromClipboard(TCHAR *&dest, TCHAR *&buf, size_t &bufLen)
{
	HGLOBAL hmem;
	TCHAR *ptr;
	size_t len;
	bool result=false;

	if(OpenClipboard(0)){
		if((hmem= GetClipboardData(
#ifdef UNICODE
			CF_UNICODETEXT
#else
			CF_TEXT
#endif
			))!=0){
			if((ptr=(TCHAR*)GlobalLock(hmem))!=0){
				len= _tcslen(ptr);
				resizeBuffer(buf, bufLen, dest, len);
				memcpy(dest, ptr, len * sizeof(TCHAR));
				dest+=len;
				result=true;
				GlobalUnlock(hmem);
			}
		}
		CloseClipboard();
	}
	return result;
}

void copyToClipboard1(TCHAR *s)
{
	HGLOBAL hmem;
	TCHAR *ptr;
	size_t len=_tcslen(s)+1;

	if(s && (hmem=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, isWin9X ? len : 2*len))!=0){
		if((ptr=(TCHAR*)GlobalLock(hmem))!=0){
#ifdef UNICODE
			memcpy(ptr, s, 2*len);
#else
			if(isWin9X)
				strcpy(ptr, s);
			else
				MultiByteToWideChar(CP_ACP, 0, s, -1, (WCHAR*)ptr, len);
#endif
			GlobalUnlock(hmem);
			SetClipboardData(isWin9X ? CF_TEXT : CF_UNICODETEXT, hmem);
		}
		else{
			GlobalFree(hmem);
		}
	}
}

struct TregKey {
	char* s;
	HKEY key;
}
regRoot[] = {
	{"LOCAL_MACHINE", HKEY_LOCAL_MACHINE},
	{"CURRENT_USER", HKEY_CURRENT_USER},
	{"CLASSES_ROOT", HKEY_CLASSES_ROOT},
	{"USERS", HKEY_USERS},
	{"CURRENT_CONFIG", HKEY_CURRENT_CONFIG},
};

void insertReg(TCHAR *path, TCHAR*& dest, TCHAR*& buf, size_t& bufLen)
{
	if (!strnicmpA(path, "HKEY_")) {
		path += 5;
		for (TregKey* k = regRoot; k < endA(regRoot); k++) {
			if (!strnicmpA(path, k->s)) {
				path += strlen(k->s);
				if (*path++ == '\\') {
					TCHAR *val = cutPath(path);
					val[-1] = 0;
					HKEY key;
					DWORD len, type;
					if(RegOpenKeyEx(k->key, path, 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &key) == ERROR_SUCCESS) {
						if (RegQueryValueEx(key, val, 0, &type, 0, &len) == ERROR_SUCCESS && len < 1000000 && len > 0) {
							switch (type){
							case REG_DWORD:
								DWORD d;
								if (RegQueryValueEx(key, val, 0, 0, (BYTE*)&d, &len) == ERROR_SUCCESS) {
									resizeBuffer(buf, bufLen, dest, 11);
									_ultot(d, dest, 10);
									dest = _tcschr(dest, 0);
								}
								break;
							case REG_SZ:
								resizeBuffer(buf, bufLen, dest, len / sizeof(TCHAR));
								if (RegQueryValueEx(key, val, 0, 0, (BYTE*)dest, &len) == ERROR_SUCCESS) {
									dest += len / sizeof(TCHAR);
									if (!dest[-1]) dest--;
								}
								break;
							}
						}
						RegCloseKey(key);
					}
					val[-1] = '\\';
				}
				break;
			}
		}
	}
}

TCHAR *formatText(TCHAR *param)
{
	size_t i,j;
	time_t t;
	tm* tloc=0;
	DWORD n;
	TCHAR *buf, *s, *d, *s2;
	TCHAR fmt[4];
	const int M=512;
	bool isTime=false;

	setlocale(LC_ALL, "");
	buf=new TCHAR[i=_tcslen(param)+1024];
	for(d=buf, s=param;; s++){
		resizeBuffer(buf, i, d, M);
		*d=*s;
		if(*s==0) break;
		if(*s=='%'){
			s++;
			switch(*s){
				default:
					//format date and time
					if (!isTime)
					{
						isTime = true;
						time(&t);
						tloc = localtime(&t);
						fmt[0] = '%';
						fmt[3] = 0;
					}
					fmt[1] = *s;
					fmt[2] = 0;
					if (*s == '#') fmt[2] = *++s;
					j = _tcsftime(d, M, fmt, tloc);
					d += j;
					if(j==0){
						*d++ = '%';
						*d++ = *s;
					}
					break;
				case 0:
					s--;
					break;
				case 'r':
					*d++='\r';
					*d++='\n';
					break;
				case 'u':
					n=M;
					if(GetUserName(d, &n)) d+=n-1;
					break;
				case 'o':
					n=M;
					if(GetComputerName(d, &n)) d+=n;
					break;
				case 'l':
					pasteFromClipboard(d, buf, i);
					break;
				case '%':
					d++;
					break;
				case '"':
					s2 = _tcschr(++s, '"');
					if (s2) {
						*s2 = 0;
						insertReg(s, d, buf, i);
						*s2 = '"';
						s = s2;
					}
					else s-=2;
					break;
			}
		}
		else{
			d++;
		}
	}
	return buf;
}

INT_PTR CALLBACK pasteTextProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
	int i, w;
	TCHAR *s;
	HWND list;

	switch(msg){
		case WM_INITDIALOG:
			SetWindowText(hWnd, lng(1067, "Paste text"));
			//add items to the listbox
			list = GetDlgItem(hWnd, 157);
			w=10;
			for(i=0; i<pasteTextData.n; i++){
				s= pasteTextData.L[i];
				aminmax(w, (int)_tcslen(s), 150);
				SendMessage(list, LB_ADDSTRING, 0, (LPARAM)s);
			}
			//select the first item
			SendMessage(list, LB_SETCURSEL, 0, 0);
			//resize listbox and dialog box
			w *= 9;
			i *= static_cast<int>(SendMessage(list, LB_GETITEMHEIGHT, 0, 0));
			SetWindowPos(hWnd, HWND_TOPMOST,
				max(0, (GetSystemMetrics(SM_CXSCREEN)-w)>>1),
				(GetSystemMetrics(SM_CYSCREEN)-i)>>1,
				w+2, i+2, 0);
			SetWindowPos(list, 0, 0, 0, w, i, SWP_NOZORDER|SWP_NOMOVE);
			SetForegroundWindow(hWnd);
			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wP)){
				case 157:
					if(HIWORD(wP)!=LBN_DBLCLK) break;
				case IDOK:
					EndDialog(hWnd, SendDlgItemMessage(hWnd, 157, LB_GETCURSEL, 0, 0));
					return TRUE;
				case IDCANCEL:
					EndDialog(hWnd, -1);
					return TRUE;
			}
			break;
	}
	return 0;
}

HWND GetFocusGlobal()
{
	GUITHREADINFO gui;
	gui.cbSize = sizeof(GUITHREADINFO);
	TGetGUIThreadInfo p= (TGetGUIThreadInfo)GetProcAddress(GetModuleHandleA("user32.dll"), "GetGUIThreadInfo");
	if(p && p(0, &gui)) return gui.hwndFocus;
	return 0;
}

void pasteText(TCHAR *param)
{
	int i, n, sel;
	TCHAR *s, *e, *t;
	DWORD editSelPos= (DWORD)-1;
	DWORD_PTR p;
	TSendMessageTimeout f=0;

	if(lockPaste){
		pasteTextData.addToQueue(param);
		return;
	}
	lockPaste++;
	//split param to items
	n=0;
	for(s=param;; s=e+2){
		e=_tcsstr(s, _T("%|"));
		while(e){
			for(i=0, t=e; t>=param && *t=='%'; i++, t--);
			if(i&1) break;
			e=_tcsstr(e+2, _T("%|"));
		}
		if(e) *e=0;
		pasteTextData.L[n++] = formatText(s);
		if(!e) break;
		*e='%';
	}
	//show dialog box for multiple texts
	sel=0;
	if(n>1){
		//remember keyboard focus
		HWND focus= GetFocusGlobal();
		if(focus){
			f = IsWindowUnicode(focus) ? SendMessageTimeoutW : SendMessageTimeoutA;
			if(f(focus, EM_GETLINECOUNT, 0, 0, SMTO_BLOCK, 5000, &p) && p>0){
				editSelPos = (DWORD)SendMessage(focus, EM_GETSEL, 0, 0); //SendMessageTimeout returns edit box length of Unicode edit boxes
			}
		}
		//show dialog box
		pasteTextData.n=n;
		sel = (int)DialogBox(inst, _T("LIST"), 0, pasteTextProc);
		if(editSelPos!=-1){
			//restore keyboard focus
			DWORD tid = GetWindowThreadProcessId(focus, 0);
			AttachThreadInput(GetCurrentThreadId(), tid, TRUE);
			SetFocus(focus);
			AttachThreadInput(GetCurrentThreadId(), tid, FALSE);
			//restore EditBox selected position
			f(focus, EM_SETSEL,
				(short)LOWORD(editSelPos), (short)HIWORD(editSelPos), SMTO_BLOCK, 15000, &p);
		}
	}
	if(sel>=0){
		//get selected item
		delete[] pasteTextData.text;
		pasteTextData.text = pasteTextData.L[sel];
		pasteTextData.L[sel]=0;
		for(i=0; i<n; i++)  delete[] pasteTextData.L[i];
		//remember clipboard content
		pasteTextData.save();
		//paste text
		if(OpenClipboard(hWin)){
			if(EmptyClipboard()){
				pasteTextData.busy=true;
				//WM_RENDERFORMAT will set clipboard data
				SetClipboardData(isWin9X ? CF_TEXT : CF_UNICODETEXT, NULL);
				//Ctrl+V
				lockPaste++;
				SetTimer(hWin, 14, checkShifts(MOD_ALT) ? 150 : 10, 0);
				KillTimer(hWin, 13);
			}
			CloseClipboard();
		}
	}
	lockPaste--;
}

void PasteTextData::addToQueue(TCHAR *s)
{
	CharList *item = new CharList;
	item->next=0;
	item->text=0;
	cpStr(item->text, s);
	if(!queueLast){
		queueFirst=queueLast= item;
	}
	else{
		queueLast->next=item;
		queueLast=item;
	}
}

bool PasteTextData::processQueue()
{
	if(!queueFirst || lockPaste) return false;
	//remove first item from queue
	CharList* item=queueFirst;
	queueFirst=item->next;
	if(!queueFirst) queueLast=0;
	//paste
	pasteText(item->text);
	delete item->text;
	delete item;
	return true;
}
//-------------------------------------------------------------------------

void wndInfo(HWND w)
{
	DWORD pid, d;
	int i;
	HANDLE h;
	TmemInfo getMemInfo;
	PROCESS_MEMORY_COUNTERS m;
	TCHAR *p, t[128], c[128], name[MAX_PATH], path[MAX_PATH];

	GetWindowText(w, t, sizeA(t));
	GetClassName(w, c, sizeA(c));
	getWindowThreadProcessId(w, &pid);
	queryProcessImageName(pid, name);
	queryFullProcessImageName(pid, path);

	p=_T("");
	m.WorkingSetSize=0;
	if((h=OpenProcess(PROCESS_QUERY_INFORMATION, 0, pid))!=0){
		d=GetPriorityClass(h);
		if(!isWin9X){
			HMODULE psapi=LoadLibraryA("psapi.dll");
			getMemInfo= (TmemInfo)GetProcAddress(psapi, "GetProcessMemoryInfo");
			if(getMemInfo){
				getMemInfo(h, &m, sizeof(PROCESS_MEMORY_COUNTERS));
			}
			FreeLibrary(psapi);
		}
		CloseHandle(h);
		for(i=0; i<sizeA(priorCnst); i++){
			if(priorCnst[i]==d) p=lng(700+i, priorTab[i]);
		}
	}
	msg1(MB_OK, lng(759, "Title: %s\r\nWindow Class: %s\r\nProcess: %s\r\nPath: %s\r\nPriority: %s\r\nMemory: %d KB"),
		t, c, name, path, p, m.WorkingSetSize/1024);
}

//Windows 7 and later have "show desktop" button (next to the clock) which activates hidden WorkerW window
HWND fixWorkerW(HWND w)
{
	TCHAR c[32];
	HWND w2;

	if(GetClassName(w, c, sizeA(c))>0 && !_tcscmp(c, _T("WorkerW")))
	{
		w2=FindWindow(_T("Progman"), _T("Program Manager"));
		if(w2)
		{
			if(GetWindowThreadProcessId(w, 0) == GetWindowThreadProcessId(w2, 0))
				return w2;
		}
	}
	return w;
}

void click(DWORD down, DWORD up)
{
	forceNoShift();
	mouse_event1(down);
	mouse_event1(up);
	restoreShift();
}

bool noCmdLine(TCHAR *param)
{
	if(cmdLineCmd<0) return false;
	HWND w= FindWindow(_T("PlasHotKey"), 0);
	if(w){
		COPYDATASTRUCT d;
		d.lpData= param;
		d.cbData= (DWORD)_tcslen(param)+1;
		d.dwData= cmdLineCmd+13000;
		SendMessage(w, WM_COPYDATA, (WPARAM)w, (LPARAM)&d);
	}
	else{
		msg(_T("This command cannot be executed, because HotkeyP.exe is not running"));
	}
	return true;
}
//-------------------------------------------------------------------------
void command(int cmd, TCHAR *param, HotKey *hk)
{
	HWND w;
	TCHAR *param2, *s;
	char *buf;
	int iparam= strToIntStr(param, &param2);
	int vol= iparam ? iparam : 1;
	int i, j;
	HANDLE h;
	HKEY key;
	bool b, *pb;
	DWORD d, pid;
	BOOL ok=FALSE;

	switch(cmd){
		case 0: case 1: case 39: case 40: case 41: case 42: case 62:
			audioCD(param, cmd);
			break;
		case 2: //shut down
			privilege();
			closeAll(iparam);
			if(isWin9X || !ExitWindowsEx(EWX_POWEROFF, iparam==1 ? TRUE : FALSE)) //Win2000
				ExitWindowsEx(EWX_SHUTDOWN, iparam==1 ? TRUE : FALSE);   //Win98
			break;
		case 3: //restart
			privilege();
			closeAll(iparam);
			ExitWindowsEx(EWX_REBOOT, iparam==1 ? TRUE : FALSE);
			break;
		case 4: //suspend
			privilege();
			SetSystemPowerState(TRUE, iparam==1 ? TRUE : FALSE);
			break;
		case 5: //log off
			closeAll(iparam);
			ExitWindowsEx(EWX_LOGOFF, iparam==1 ? TRUE : FALSE);
			break;
		case 6: //screen saver
			PostMessage(hWin, WM_SYSCOMMAND, SC_SCREENSAVE, 0);
			break;
		case 7: //maximize
			w=getWindow(param);
			if(w) PostMessage(w, WM_SYSCOMMAND, IsZoomed(w) ? SC_RESTORE : SC_MAXIMIZE, 0);
			break;
		case 8: //minimize
			w=getWindow(param);
			if(w) PostMessage(w, WM_SYSCOMMAND, IsIconic(w) ? SC_RESTORE : SC_MINIMIZE, 0);
			break;
		case 9: //close
			PostMessage(fixWorkerW(getWindow(param)), WM_CLOSE, 0, 0);
			break;
		case 10: //always on top
			w= getWindow(param);
			SetWindowPos(w,
				GetWindowLong(w, GWL_EXSTYLE)&WS_EX_TOPMOST ? HWND_NOTOPMOST : HWND_TOPMOST,
				0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_ASYNCWINDOWPOS);
			break;
		case 11: //kill process
			w=getWindow(param);
			if(w){
				if(getWindowThreadProcessId(w, &pid)){
					if((h=OpenProcess(PROCESS_TERMINATE, 0, pid))!=0){
						ok= TerminateProcess(h, (UINT)-1);
						CloseHandle(h);
					}
				}
				if(!ok) msglng(751, "Can't terminate process");
			}
			break;
		case 12: //control panels
		case 50: case 51: case 52: case 53: case 54:
		case 55: case 56: case 57:
			buf= new char[_tcslen(param)+64];
			strcpy(buf, "rundll32.exe shell32.dll,Control_RunDLL ");
			if(!*param && cmd!=12){
				strcat(buf, controlPanels[cmd-50]);
				strcat(buf, ".cpl");
			}
			else if(*param){
				convertT2A(param, a);
				strcat(buf, a);
				strcat(buf, ".cpl");
			}
			WinExec(buf, SW_NORMAL);
			delete[] buf;
			break;
		case 13:
			volumeCmd(param2, vol, 0);
			break;
		case 14:
			volumeCmd(param2, -vol, 0);
			break;
		case 15:
			volumeCmd(_T("Wave"), vol, 0);
			break;
		case 16:
			volumeCmd(_T("Wave"), -vol, 0);
			break;
		case 17: //mute
			volumeCmd(param2, 1, 1);
			break;
		case 18: //desktop size
			resolution(param);
			break;
		case 19: //monitor power off
			if(!waitWhileKey(cmd, param)) break;
			if(!pcLocked) GetCursorPos(&mousePos);
			w=hWin;
			if(!w) w=FindWindowA("Shell_TrayWnd", 0);
			DefWindowProc(w, WM_SYSCOMMAND, SC_MONITORPOWER, *param ? iparam : 2);
			SetTimer(hWin, 2, 100, 0);
			break;
		case 20: //process priority
			if(!*param) iparam=1;
			else{
				convertT2A(param, paramA);
				for(i=0; i<Npriority; i++){
					if(!_stricmp(priorTab[i], paramA)) iparam=i;
				}
			}
			priority(iparam);
			break;
		case 22: //empty recycle bin
		{
			typedef int(__stdcall *TemptyBin)(HWND, LPCSTR, WORD);
			TemptyBin emptyBin = (TemptyBin)GetProcAddress(GetModuleHandleA("shell32.dll"), "SHEmptyRecycleBinA");
			if(emptyBin) emptyBin(hWin, "", (WORD)iparam);
		}
		break;
		case 23: //random wallpaper
		case 82:
			changeWallpaper(param, 0);
			break;
		case 24: //disk free space
			aminmax(diskfreePrec, 0, 6);
			diskFree();
			popupDiskFree.delay= iparam ? iparam : 6000;
			popupDiskFree.show();
			break;
		case 25: //hide window
			if(hiddenWin){
				ShowWindowAsync(hiddenWin, SW_SHOW);
				SetForegroundWindow(hiddenWin);
				hiddenWin=0;
			}
			else{
				w=getWindow(param);
				if(!*param) hiddenWin=w;
				ShowWindowAsync(w, IsWindowVisible(w) ? SW_HIDE : SW_SHOW);
			}
			modifyTrayIcon();
			break;
		case 94: //macro to active window
		case 904://down
		case 906://up
		case 26: //keys to another window
		case 74: //keys to active window
		case 21: //command to another window
		case 73: //command to active window
			cmdToWindow(cmd, param);
			break;
		case 27: //macro
			parseMacro(param);
			break;
		case 28: //center window
			moveCmd(param, 3, 3);
			break;
		case 29:
			moveCmd(param, 1, 2);
			break;
		case 30:
			moveCmd(param, 0, 2);
			break;
		case 31:
			moveCmd(param, 2, 2);
			break;
		case 32:
			moveCmd(param, 1, 0);
			break;
		case 33:
			moveCmd(param, 2, 0);
			break;
		case 34:
			moveCmd(param, 1, 1);
			break;
		case 35:
			moveCmd(param, 0, 1);
			break;
		case 36:
			moveCmd(param, 2, 1);
			break;
		case 37: //move mouse cursor
		{
			int x, y;
			if(_stscanf(param, _T("%d %d"), &x, &y)!=2){
				mouse_event(MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE,
					32767, 32767, 0, 0);
			}
			else{
				mouse_event(MOUSEEVENTF_MOVE, x, y, 0, 0);
			}
		}
		break;
		case 38: //shutdown dialog
			PostMessage(FindWindowA("Shell_TrayWnd", 0), WM_CLOSE, 0, 0);
			break;
		case 43:
			click(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP);
			break;
		case 44:
			click(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP);
			break;
		case 45:
			click(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
			break;
		case 46: case 47: case 48: case 49:
			priority(cmd-46);
			break;
		case 59: case 60:
			priority(cmd-55);
			break;
		case 58: //mute wave
			volumeCmd(_T("Wave"), 1, 1);
			break;
		case 61: //multi command
		case 70: //commands list
			if(hk){
				if(hk->lock) return;
				hk->lock++;
			}
			noMsg++;
			for(s=param;; s++){
				i=_tcstol(s, &s, 10);
				if(!i) break;
				i--;
				if(i<0 || i>=numKeys){
					msglng(758, "Invalid multi-command number");
					break;
				}
				executeHotKey(i);
				if(cmd==70){
					//move the first number to the end of parameter
					TCHAR sep=*s;
					if(!sep || !hk) break;
					_tcscpy(param, s+1);
					s=_tcschr(param, 0);
					*s=sep;
					_itot(i+1, s+1, 10);
					if(*iniFile) wr(iniFile);
					break;
				}
				if(!*s) break;
			}
			if(hk) hk->lock--;
			noMsg--;
			break;
		case 63: //lock computer
			if(!*param){
				TLockWorkStation p= (TLockWorkStation)GetProcAddress(GetModuleHandleA("user32.dll"), "LockWorkStation");
				if(p) p();
			}
			else if(!pcLocked){
				if(noCmdLine(param)) break;
				GetCursorPos(&mousePos);
				pcLocked=true;
				setHook();
				if(!hookK){ pcLocked=false; break; }
				cpStr(pcLockParam, param);
				lockImg=(HGDIOBJ)LoadImage(0, lockBMP, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				hWndBeforeLock= GetForegroundWindow();
				hWndLock= CreateWindow(_T("HotkeyLock"), _T(""), WS_POPUP, 0, 0, 10, 10, hWin, 0, inst, 0);
				pcLockX=GetSystemMetrics(SM_CXSCREEN)>>1;
				pcLockY=GetSystemMetrics(SM_CYSCREEN)>>1;
				double alpha=random(10000)*(3.141592654/5000);
				amax(lockSpeed, 100);
				pcLockDx=lockSpeed*cos(alpha)/2;
				pcLockDy=lockSpeed*sin(alpha)/2;
				SetTimer(hWin, 8, 60, 0);
				TregisterServiceProcess p= (TregisterServiceProcess)GetProcAddress(GetModuleHandleA("kernel32.dll"), "RegisterServiceProcess");
				if(p) p(0, 1);
				if(RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_QUERY_VALUE|KEY_SET_VALUE, &key)==ERROR_SUCCESS){
					d=4;
					RegQueryValueExA(key, "DisableTaskMgr", 0, 0, (LPBYTE)&disableTaskMgr, &d);
					d=1;
					RegSetValueExA(key, "DisableTaskMgr", 0, REG_DWORD, (LPBYTE)&d, 4);
					RegCloseKey(key);
				}
				if(lockMute) volume(0, 1, 3);
				writeini();
			}
			break;
		case 64: //hibernate
		{
			if(!waitWhileKey(cmd, param)) break;
			isHilited=false;
			modifyTrayIcon();
			HMODULE l= LoadLibraryA("powrprof.dll");
			if(l){
				TSetSuspendState p;
				p= (TSetSuspendState)GetProcAddress(l, "SetSuspendState");
				if(p) p(TRUE, iparam==1 ? TRUE : FALSE, FALSE);
				FreeLibrary(l);
			}
		}
		break;
		case 65: //minimize others
			noMinimWnd=GetForegroundWindow();
			for(;;){
				w=GetWindow(noMinimWnd, GW_OWNER);
				if(!w) break;
				noMinimWnd=w;
			}
			EnumWindows(enumMinimize, 0);
			break;
		case 66: //information about an active window
			w=getActiveWindow();
			if(w) wndInfo(w);
			break;
		case 67: //paste text, date, time, user name, computer name
			pasteText(param);
			break;
		case 68: //next wallpaper
			changeWallpaper(param, 1);
			break;
		case 69: //previous wallpaper
			changeWallpaper(param, 2);
			break;
		case 71: //start moving a window
		case 72: //start sizing a window
			w=getWindow(param);
			if(w) PostMessage(w, WM_SYSCOMMAND, cmd==71 ? SC_MOVE : SC_SIZE, 0);
			break;
		case 75: //mouse wheel
		case 108://mouse horizontal wheel
			forceNoShift();
			mouse_event2(cmd==75 ? MOUSEEVENTF_WHEEL : MOUSEEVENTF_HWHEEL, iparam ? iparam : -WHEEL_DELTA);
			restoreShift();
			break;
		case 76: //double click
			doubleClick();
			break;
		case 77: //window opacity
			setOpacity(getWindow(param2), iparam);
			break;
		case 78: //volume
			volumeCmd(param2, param!=param2 ? iparam : 50, 2);
			break;
		case 79: //mouse button4
		case 80: //mouse button5
			forceNoShift();
			mouse_event2(MOUSEEVENTF_XDOWN, cmd-78);
			mouse_event2(MOUSEEVENTF_XUP, cmd-78);
			restoreShift();
			break;
		case 81: //show desktop
			parseMacro(_T("\\rep\\win\\D"));
			break;
		case 83: //previous task
		case 84: //next task
			if(!altDown){
				altDown=true;
				keyEventDown(VK_MENU);
			}
			if(cmd==83) keyEventDown(VK_SHIFT);
			keyEventDown(VK_TAB);
			keyEventUp(VK_TAB);
			if(cmd==83) keyEventUp(VK_SHIFT);
			if(!hookM || buttons==-1){
				altDown=false;
				keyEventUp(VK_MENU);
			}
			break;
		case 85: //show text
			if(IsWindowVisible(popupShowText.hWnd) &&
				showTextLast && !_tcscmp(showTextLast, param)){
				//hide popup window
				PostMessage(hWin, WM_TIMER, 50+P_ShowText, 0);
			}
			else{
				cpStr(showTextLast, param);
				if(iparam<100){
					iparam=6000;
				}
				else{
					param=param2;
				}
				//get text
				delete[] showTextStr;
				showTextStr= formatText(param);
				//calculate window size
				RECT rc;
				rc.left=rc.top=0;
				HDC dc=GetDC(popupShowText.hWnd);
				DrawText(dc, showTextStr, -1, &rc, DT_CALCRECT|DT_NOPREFIX|DT_NOCLIP);
				ReleaseDC(popupShowText.hWnd, dc);
				popupShowText.width=rc.right+showTextBorder*2;
				popupShowText.height=rc.bottom+showTextBorder*2;
				//show popup window
				popupShowText.delay=iparam;
				popupShowText.show();
			}
			break;
		case 86: //disable keys
			b=false;
			for(j=0; j<2; j++){
				for(s=param; *s;){
					int dummy;
					switch(parseKey(s, i, dummy)){
						case 0:
							return;
						case 1: case 2:
						{
							bool &k= blockedKeys[LOBYTE(i)];
							if(!j) b|=k;
							else k=!b;
						}
						break;
						default:
							s++;
					}
				}
			}
			setHook();
			break;
		case 87: //disable/enable all hotkeys
			pb= &disableAll;
		ldisable:
			if(noCmdLine(param)) break;
			unregisterKeys();
			if(param==param2) *pb=!*pb;
			else *pb=(iparam!=0);
			registerKeys();
			setHook();
			modifyTrayIcon();
			break;
		case 88: //disable/enable mouse hotkeys
			pb= &disableMouse;
			goto ldisable;
		case 89: //desktop screenshot
			screenshot(GetDesktopWindow());
			break;
		case 90: //window screenshot
			screenshot(GetForegroundWindow());
			break;
		case 91: //hide systray icon
		case 98:
			param2=0;
			cpStr(param2, param);
			createThread(hideIconProc, param2);
			break;
		case 92: //stop service
		case 93: //start service
		{
			if(isWin9X) break;
			SC_HANDLE schSCManager = OpenSCManager(0, 0, SC_MANAGER_CONNECT);
			if(!schSCManager) msg(_T("Cannot open SC manager"));
			else{
				SC_HANDLE schService = OpenService(schSCManager, param, cmd==93 ? SERVICE_START : SERVICE_STOP);
				if(!schService) msg(_T("Cannot open service %s"), param);
				else{
					if(cmd==93){
						if(!StartService(schService, 0, 0)){
							d=GetLastError();
							if(d!=1056) msg(_T("Cannot start service (error %u)"), d);
						}
					}
					else{
						SERVICE_STATUS status;
						if(!ControlService(schService, SERVICE_CONTROL_STOP, &status)){
							d=GetLastError();
							if(d!=1062) msg(_T("Cannot stop service (error %u)"), d);
						}
					}
					CloseServiceHandle(schService);
				}
				CloseServiceHandle(schSCManager);
			}
			break;
		}
		case 95: //disable/enable joystick hotkeys
			pb= &disableJoystick;
			goto ldisable;
		case 96: //disable/enable remote control hotkeys
			pb= &disableLirc;
			goto ldisable;
		case 97: //disable/enable keyboard hotkeys
			pb= &disableKeys;
			goto ldisable;
		case 99: //restore tray icon
			param2=0;
			cpStr(param2, param);
			createThread(restoreIconProc, param2);
			break;
		case 100: //change CD speed 
			audioCD(param2, cmd, iparam);
			break;
		case 101: //hide application
			if(hiddenApp.list){
				unhideApp(&hiddenApp);
			}
			else{
				hideApp(getWindow(param), &hiddenApp, true);
			}
			modifyTrayIcon();
			break;
		case 102: //minimize application to system tray
		case 115: //minimize window to system tray
			if(noCmdLine(param)) break;
			w=getWindow(param);
			if(w==hWin && trayicon){ //HotkeyP window
				hideMainWindow();
			}
			else if(w && IsWindowVisible(w)){
				TCHAR title[64];
				if (GetClassName(w, title, sizeA(title)) && !_tcscmp(title, _T("Shell_TrayWnd"))) break; //don't hide the taskbar
				GetWindowText(w, title, 64);
				//find free slot in array
				for(i=0; i<sizeA(trayIconA); i++){
					HideInfo *info = &trayIconA[i];
					if(!info->pid){
						//hide application and add icon
						hideApp(w, info, cmd==102);
						getExeIcon(info);
						addTrayIcon(title, info->icon, 100+i);
						break;
					}
				}
			}
			break;
		case 103: //magnifier
			if(noCmdLine(param)) break;
			if(isZoom){
				zoom.end();
			}
			else{
				i= strToIntStr(param2, &param);
				j= strToIntStr(param, &param2);
				zoom.magnification= iparam ? iparam : 2;
				zoom.width= i ? i+2 : 142;
				zoom.height= j ? j+2 : zoom.width;
				zoom.start();
			}
			break;
		case 104: //clear recent documents
			SHAddToRecentDocs(0, 0);
			break;
		case 105: //delete temporary files
			if(GetTempPath(sizeA(exeBuf), exeBuf)){
				if(_tcslen(exeBuf)>4){
					FILETIME f;
					SYSTEMTIME time;
					GetSystemTime(&time);
					SystemTimeToFileTime(&time, &f);
					*(LONGLONG*)&f -= (iparam ? iparam : 7) *(24*3600*(LONGLONG)10000000);
					deleteTemp(exeBuf, &f);
				}
			}
			break;
		case 106: //save desktop icons
			saveDesktopIcons(0);
			break;
		case 107: //restore desktop icons
			saveDesktopIcons(1);
			break;
		case 109: //remove drive
			if(*param) removeDrive(param[0]);
			else removeUSBdrives();
			break;
		case 110: //window opacity +
		case 111: //window opacity -
			w=getWindow(param2);
			i=getOpacity(w);
			if(i>=0){
				if(iparam==0) iparam=10;
				if(cmd==110) i+=iparam; else i-=iparam;
				aminmax(i, 0, 255);
				setOpacity(w, i);
			}
			break;
		case 112: //maximize all
			w=GetForegroundWindow();
			EnumWindows(enumMaximize, (LPARAM)w);
			Sleep(100);
			enumMaximize(w, 0);
			SetForegroundWindow(w);
			break;
		case 113: //Show HotkeyP window
			SendMessage(hWin, WM_COMMAND, 210, 0);
			break;
		case 114: //Reload hook
			reloadHook();
			break;
	}
}
//---------------------------------------------------------------------------
