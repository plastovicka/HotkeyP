/*
(C) 2003-2016  Petr Lastovicka

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License.
*/

#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"

int
delayButtons=-1,
	delayButtons2=-1,
	lastButtons=-1;
bool preventWinMenu;

HMODULE klib;
char passwd[Dpasswd];
int passwdLen;
BYTE password[Dpasswd];

char *keyNames[]={"Browser_Back", "Browser_Forward", "Browser_Refresh",
"Browser_Stop", "Browser_Search", "Browser_Favorites", "Browser",
	"Mute", "Volume_Down", "Volume_Up", "Media_Next", "Media_Prev",
	"Media_Stop", "Media_Play_Pause", "Email", "Media_Select", "Launch_App1", "Launch_App2"
};

const BYTE specialKeys[256]={
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //Tab
	0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //Caps Lock, Esc
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //Sleep
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //F12
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

BYTE specialWinKeys[256];

//-------------------------------------------------------------------------
void keyMapChanged()
{
	TCHAR *s, **p;
	p=keyMapIndex;
	for(s=keyMap;; s++){
		while(*s==' ') s++;
		if(!*s) goto lend;
		*p++=s;
		while(*s!=';'){
			if(!*s) goto lend;
			s++;
		}
	}
lend:
	*p=0;
}

void printKey(TCHAR *s, HotKey* hk)
{
	*s=0;
	if(hk->modifiers & MOD_CONTROL) _tcscat(s, _T("Ctrl+"));
	if(hk->modifiers & MOD_SHIFT) _tcscat(s, _T("Shift+"));
	if(hk->modifiers & MOD_ALT) _tcscat(s, _T("Alt+"));
	if(hk->modifiers & MOD_WIN) _tcscat(s, _T("Win+"));
	printKeyNoShift(_tcschr(s, 0), hk);
}

void printKeyNoShift(TCHAR *s, HotKey* hk)
{
	TCHAR *e, *t, **p, *s0;
	int i, scan, j;

	s0=s;
	*s=0;
	scan=hk->scanCode;
	if(hk->vkey>='0' && hk->vkey<='9'){
		*s++=(TCHAR)hk->vkey;
		*s=0;
	}
	else if(hk->vkey==vkMouse){
		buttonToArray(scan);
		//mouse
		if(scan!=-1){
			s+=_stprintf(s, _T("%s "), lng(719, "Mouse"));
			for(i=24; i>=0; i-=4){
				j=(scan>>i)&15;
				if(j==15) continue;
				*s="LRSCM456789DUDU."[j];
				if(j==M_WheelUp){ tcscpyA(s, "Up"); s++; }
				if(j==M_WheelDown){ tcscpyA(s, "Down"); s+=3; }
				if(j==M_WheelRight){ tcscpyA(s, "Right"); s+=4; }
				if(j==M_WheelLeft){ tcscpyA(s, "Left"); s+=3; }
				s++;
				*s++= (hk->scanCode>=0) ? '&' : '+';
			}
			*(s-1)=0;
		}
	}
	else if(hk->vkey==vkLirc){
		//remote control
		_stprintf(s, _T("{%.30hs}"), hk->lirc);
	}
	else if(hk->vkey==vkJoy){
		//joystick
		s+=_stprintf(s, _T("Joy"));
		i= unsigned(scan)>>28; //joystick ID
		if(i) s+=_stprintf(s, _T("%d"), i+1);
		s+=_stprintf(s, _T(": "));
		i= scan & 0x3ffffff;
		switch((scan>>26)&3){
			case J_BUTTON:
				_stprintf(s, _T("%d"), i+1);
				break;
			case J_AXIS:
				*s++= axisInd2Name(i);
				*s++= (i&JOY_SIGN) ? '-' : '+';
				*s=0;
				break;
			case J_POV:
				_stprintf(s, _T("Pov%d"), i/100);
				break;
		}
	}
	else{
		GetKeyNameText(scan, s, 32);
		if(*s==0 && hk->vkey || s[1]==0 && *s>='A' && *s<='Z' && (scan&0x1000000)){
			//multimedia keys
			if(hk->vkey>=166 && hk->vkey<166+sizeA(keyNames)){
				tcscpyA(s, keyNames[hk->vkey-166]);
			}
			else if(hk->vkey==95){
				_tcscpy(s, _T("Sleep"));
			}
			else if(hk->vkey!=255){
				_stprintf(s, _T("(%d)"), hk->vkey);
			}
			else{
				_stprintf(s, _T("[%d]"), (scan>>16)&255);
			}
		}
	}
	//keyMap
	i= (int)_tcslen(s0);
	for(p= keyMapIndex;; p++){
		t=*p;
		if(!t) break;
		if(!_tcsncmp(s0, t, i) && t[i]=='='){
			t+=i+1;
			e=_tcschr(t, ';');
			if(e){
				memcpy(s0, t, (i=min(int(e-t), 31)) * sizeof(TCHAR));
				s0[i]=0;
			}
			else{
				lstrcpyn(s0, t, 32);
			}
			break;
		}
	}
}

//-------------------------------------------------------------------------
bool CmpProcessPath(PROCESSENTRY32 *pe, TCHAR const *exe, TCHAR const *n1)
{
	//compare file name
	if(isWinXP){
		if(_tcsicmp(pe->szExeFile, n1)) return false; //not equal
		if(n1==exe) return true; //exe parameter is without path
	}
	else{
		//szExeFile is truncated to 15 characters on Windows 2000
		//szExeFile is with full path on Windows 98
		if(_tcsnicmp(cutPath(pe->szExeFile), n1, 15)) return false;
	}
	//compare full path
	TCHAR buf[MAX_PATH];
	if(!queryFullProcessImageName(pe->th32ProcessID, buf)) return true;
	TCHAR const *n2 = buf;
	if(n1==exe) n2 = cutPath(buf);
	return !_tcsicmp(exe, n2);
}

//find PID of process which belongs to exe file
DWORD findProcess(TCHAR const *exe) // zef: made const correct
{
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(h!=(HANDLE)-1){
		TCHAR const *n = cutPath(exe);
		Process32First(h, &pe);
		do{
			if(CmpProcessPath(&pe, exe, n)){
				CloseHandle(h);
				return pe.th32ProcessID;
			}
		} while(Process32Next(h, &pe));
		CloseHandle(h);
	}
	return 0;
}

static HWND found;

//enumerate all windows, activate window of process pid
BOOL CALLBACK enumWin(HWND hWnd, LPARAM pid)
{
	unsigned long id;
	RECT rc;

	if(getWindowThreadProcessId(hWnd, &id) && id==(DWORD)pid){
		if(IsWindowVisible(hWnd)){
			GetWindowRect(hWnd, &rc);
			if(!IsRectEmpty(&rc)){
				LONG exstyle=GetWindowLong(hWnd, GWL_EXSTYLE);
				if(!(exstyle & WS_EX_TOOLWINDOW) || !found){
					found=hWnd;
				}
			}
		}
	}
	return TRUE;
}

HWND findWindow(TCHAR const *exe, DWORD pid) // zef: made const correct
{
	found=0;
	DWORD pid1= findProcess(exe);
	if(pid) EnumWindows((WNDENUMPROC)enumWin, (LPARAM)pid);
	if(pid1 && pid1!=pid) EnumWindows((WNDENUMPROC)enumWin, (LPARAM)pid1);
	return found;
}
//---------------------------------------------------------------------------
struct TopacityInfo
{
	DWORD pid;
	HANDLE process;
	int opacity;
	int done;
	HWND oldW;
};

BOOL CALLBACK opacityEnumWin(HWND hWnd, LPARAM param)
{
	DWORD id;
	RECT rc;
	TopacityInfo *oi= (TopacityInfo*)param;

	if(GetWindowThreadProcessId(hWnd, &id) && id==oi->pid){
		if(IsWindowVisible(hWnd)){
			GetWindowRect(hWnd, &rc);
			if(!IsRectEmpty(&rc)){
				setOpacity(hWnd, oi->opacity);
				oi->done++;
			}
		}
	}
	return TRUE;
}

DWORD WINAPI opacityProc(LPVOID param)
{
	HWND w;
	int i;
	DWORD e;
	TopacityInfo *oi= (TopacityInfo*)param;

	WaitForInputIdle(oi->process, 20000);
	oi->done=0;
	for(i=0; i<20; i++){
		EnumWindows((WNDENUMPROC)opacityEnumWin, (LPARAM)oi);
		if(oi->done) break;
		if(GetExitCodeProcess(oi->process, &e) && e!=STILL_ACTIVE){
			w=GetForegroundWindow();
			if(w!=oi->oldW) setOpacity(w, oi->opacity);
			break;
		}
		Sleep(500);
	}
	CloseHandle(oi->process);
	delete oi;
	return 0;
}

//---------------------------------------------------------------------------

void executeHotKey(int i)
{
	TCHAR *workDir, *filePart, *s;
	const TCHAR *exe;
	HWND w;
	DWORD d;
	DWORD_PTR dp;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	HotKey *hk= &hotKeyA[i];

	if(hk->cmd>=0){
		//internal command
		command(hk->cmd, hk->args, hk);
	}
	else{
		workDir=0;
		if(*hk->dir) workDir=hk->dir;
		tstring fullExe = hk->getFullExe(); // zef: added support for environment vars in paths
		exe=fullExe.c_str();
		if(!isExe(exe)){
			//document
			if(!hk->admin && isElevated()){
				//use explorer.exe to open document at medium integrity level
				if(*exe!='"') {
					exeBuf[0]='"';
					if(!_tcsnicmp(exe, _T("www."), 4)){
						_tcscpy(exeBuf+1, _T("http://"));
						_tcscpy(exeBuf+8, exe);
					}
					else {
						_tcscpy(exeBuf+1, exe);
					}
					_tcscat(exeBuf, _T("\""));
					exe=exeBuf;
				}
				ShellExecute(0, 0, _T("explorer.exe"), exe, 0, SW_SHOWNORMAL);
			}
			else{
				//use ShellExecute to open document in associated application
				if(!workDir){
					_tcscpy(exeBuf, exe);
					filePart=cutPath(exeBuf);
					if(filePart>exeBuf){
						*(filePart-1)=0;
						workDir=exeBuf;
					}
				}
				ShellExecute(0, 0, exe, *hk->args ? hk->args : 0, workDir, showCnst[hk->cmdShow]);
			}
		}
		else{
			//close finished processes handles
			for(i=0; i<numKeys; i++){
				HANDLE h= hotKeyA[i].process;
				if(h){
					DWORD c;
					if(!GetExitCodeProcess(h, &c) || c!=STILL_ACTIVE){
						CloseHandle(h);
						hotKeyA[i].processId=0;
						hotKeyA[i].process=0;
					}
				}
			}
			//find out whether program is already running
			if(!hk->multInst && (w=findWindow(exe, hk->processId))!=0){
				if(w==GetForegroundWindow() && !IsIconic(w)){
					//minimize
					PostMessage(w, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				}
				else{
					//bring to front
					if(IsIconic(w)){
						HWND w0=GetForegroundWindow();
						if(SendMessageTimeout(w, WM_SYSCOMMAND, SC_RESTORE, 0, SMTO_ABORTIFHUNG|SMTO_BLOCK, 5000, &dp)){
							Sleep(10);
							HWND w1=GetForegroundWindow();
							if(w1 && w1!=w0) w=w1;
						}
					}
					if(hk->opacity){
						setOpacity(w, hk->opacity);
						InvalidateRect(w, 0, TRUE);
					}
					if(!SetForegroundWindow(w)){
						PostMessage(w, WM_SYSCOMMAND, SC_MINIMIZE, 0);
						PostMessage(w, WM_SYSCOMMAND, SC_RESTORE, 0);
					}
				}
			}
			else{
				//append parameters to exe file name
				TCHAR *args=hk->args;
				size_t len=fullExe.length();
				if(!*args && len>4 && !_tcsicmp(exe+len-4, _T(".scr"))) args=_T("/S"); //screen saver needs parameter /S
				s= new TCHAR[len+_tcslen(args)+4];
				s[0]='\"';
				_tcscpy(s+1, exe);
				_tcscat(s, _T("\""));
				if(*args){
					_tcscat(s, _T(" "));
					_tcscat(s, args);
				}
				//set working directory
				if(!workDir && SearchPath(0, exe, 0, sizeA(exeBuf), exeBuf, &filePart)){
					workDir=exeBuf;
					*filePart=0;
				}
				memset(&si, 0, sizeof(STARTUPINFO));
				si.cb= sizeof(STARTUPINFO);
				si.dwFlags= STARTF_USESHOWWINDOW;
				si.wShowWindow= showCnst[hk->cmdShow];
				pi.dwProcessId=0;
				w=GetForegroundWindow();
				bool success=false;
				DWORD prior = priorCnst[hk->priority];
				//run process
				if(!hk->admin && isElevated() && CreateMediumIntegrityProcess(s, prior, workDir, &si, &pi) ||
					(!hk->admin || isElevated() || isWin9X) && CreateProcess(0, s, 0, 0, 0, prior, 0, workDir, &si, &pi)){
					if(hk->process) CloseHandle(hk->process);
					hk->process=pi.hProcess;
					hk->processId=pi.dwProcessId;
					CloseHandle(pi.hThread);
					success=true;
				}
				else if((hk->admin && !isElevated()) || GetLastError()==ERROR_ELEVATION_REQUIRED){
					SHELLEXECUTEINFO sei;
					ZeroMemory(&sei, sizeof(sei));
					sei.cbSize=sizeof(SHELLEXECUTEINFO);
					sei.fMask=SEE_MASK_NOCLOSEPROCESS;
					sei.hwnd=hWin;
					sei.lpDirectory=workDir;
					sei.lpFile=exe;
					cpStr(s, args);
					sei.lpParameters=s;
					sei.nShow=showCnst[hk->cmdShow];
					if(hk->admin) sei.lpVerb=_T("runas");
					BOOL vis = IsWindowVisible(hWin);
					BOOL iconic = IsIconic(hWin);
					if(!vis){
						setOpacity(hWin, 1);
						ShowWindow(hWin, SW_SHOW);
					}
					if(iconic) ShowWindow(hWin, SW_RESTORE);
					SetForegroundWindow(hWin);
					if(ShellExecuteEx(&sei)){
						SetPriorityClass(sei.hProcess, priorCnst[hk->priority]);
						if(hk->process) CloseHandle(hk->process);
						hk->process= sei.hProcess;
						hk->processId= ((TGetProcessId)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcessId"))(sei.hProcess);
						success=true;
					}
					if(iconic) ShowWindow(hWin, SW_MINIMIZE);
					if(!vis){
						ShowWindow(hWin, SW_HIDE);
						setOpacity(hWin, 255);
					}
				}
				else{
					if(testDir(hk->dir)){
						msglng(740, "Invalid working directory");
					}
					else{
						msglng(743, "Cannot start process %s", hk->exe);
					}
				}
				if(success){
					if(hk->opacity && pi.dwProcessId){
						TopacityInfo *o = new TopacityInfo;
						o->opacity= hk->opacity;
						o->pid= pi.dwProcessId;
						o->process= OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE, FALSE, pi.dwProcessId);
						o->oldW= w;
						CloseHandle(CreateThread(0, 0, opacityProc, o, 0, &d));
						Sleep(1);
					}
				}
				delete[] s;
			}
		}
	}

	// zef: play sound
	if(*hk->sound)
		PlaySound(ExpandVars(hk->sound).c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

//-------------------------------------------------------------------------


//revise parameters of multiple commands
void correctMultiCmd(int item, int action, int item2)
{
	TCHAR *s, *d;
	TCHAR buf[1024];

	for(int k=0; k<numKeys; k++){
		HotKey *hk= &hotKeyA[k];
		if(hk->cmd!=61 && hk->cmd!=70) continue; //multi command
		d=buf;
		for(s=hk->args;; s++){
			int i=_tcstol(s, &s, 10);
			if(!i) break;
			if(action==1){ //delete
				if(i-1==item) i=0;
				else if(i-1>item) i--;
			}
			else if(action==2){ //insert
				if(i>item) i++;
			}
			else{ //swap
				if(i-1==item) i=item2+1;
				else if(i-1==item2) i=item+1;
			}
			if(i) d+=_stprintf(d, _T("%d "), i);
			if(!*s || d>buf+sizeA(buf)-20) break;
		}
		if(d>buf){
			*(d-1)=0;
			cpStr(hk->args, buf);
		}
	}
}

//return true if the function f returns true for at least one subcommand
bool HotKey::parseMultiCmd(bool (HotKey::*f)() const) const
{
	TCHAR *s;
	int i;

	if(!lock){
		lock++;
		for(s=args;; s++){
			i=_tcstol(s, &s, 10);
			if(!i) break;
			i--;
			if(i>=0 && i<numKeys && (hotKeyA[i].*f)()){
				lock--;
				return true;
			}
			if(!*s || cmd==70) break;
		}
		lock--;
	}
	return false;
}

bool HotKey::isLocal1() const
{
	return cmd==73 || cmd==74 || cmd==94;
}

bool HotKey::isLocal() const
{
	return isLocal1() || //command/keys/macro to active window
		(cmd==61 || cmd==70) && parseMultiCmd(&HotKey::isLocal); //multi command or command list
}

bool HotKey::isActive() const
{
	return !((cmd==61 || cmd==70) && !parseMultiCmd(&HotKey::isActive) ||
		isLocal1() && !cmdToWindow(30, args));
}

//-------------------------------------------------------------------------
//'scan' can contain up to 7 mouse butttons which are pressed simultaneously
//'which' is enum M_Left,...
int buttonFind(int which, int scan)
{
	int i;

	for(i=24; i>=0; i-=4){
		if(((scan>>i)&15)==which) break;
	}
	return i;
}

//remove button which from scan
bool buttonUp(int which, int &scan)
{
	int i= buttonFind(which, scan);
	if(i<0) return false; //the button is not pressed
	i= (1<<i)-1;
	scan= scan&i | (scan>>4)&~i;
	return true;
}

//append button which to scan
void buttonDown(int which, int &scan)
{
	if((scan>>28)!=-1) return; //too many buttons
	buttonUp(which, scan);
	scan= (scan<<4)|which;
}

//convert array to bitmask
void buttonToBitMask(int &param)
{
	int i, m;

	if(param>=0) return;
	m=0;
	for(i=24; i>=0; i-=4){
		m|= 1<<((param>>i)&15);
	}
	if(m&(1<<M_WheelUp)) m^= scanWheelUp|(1<<M_WheelUp);
	if(m&(1<<M_WheelDown)) m^= scanWheelDown|(1<<M_WheelDown);
	if(m&(1<<M_WheelRight)) m^= scanWheelRight|(1<<M_WheelRight);
	if(m&(1<<M_WheelLeft)) m^= scanWheelLeft|(1<<M_WheelLeft);
	param= m&~(1<<15);
}

//old versions had bitmap in HotKey->scancode
//new version has array in HotKey->scancode
void buttonToArray(int &param)
{
	int i, a;

	if(param<0) return; //new version, nothing to do
	a=-1;
	for(i=0; i<16; i++){
		if(param&(1<<i)) buttonDown(i, a);
	}
	if(param & scanWheelUp) buttonDown(M_WheelUp, a);
	if(param & scanWheelDown) buttonDown(M_WheelDown, a);
	if(param & scanWheelRight) buttonDown(M_WheelRight, a);
	if(param & scanWheelLeft) buttonDown(M_WheelLeft, a);
	param=a;
}

bool checkProcessList(TCHAR *list, DWORD pid)
{
	TCHAR *s, *e;
	for(s=list; *s;){
		e=_tcschr(s, ';');
		if(e) *e=0;
		bool b= checkProcess(pid, s);
		if(e) *e=';';
		if(b) return true;
		if(!e) break;
		s=e+1;
		while(*s==' ') s++;
	}
	return false;
}

bool checkProcessList(TCHAR *list)
{
	HWND w= GetForegroundWindow();
	DWORD pid;
	if(w && getWindowThreadProcessId(w, &pid)){
		return checkProcessList(list, pid);
	}
	return false;
}

bool checkFullscreen(TCHAR *list)
{
	HWND w= GetForegroundWindow();
	DWORD pid;
	if(w && getWindowThreadProcessId(w, &pid)){
		RECT rc;
		GetWindowRect(w, &rc);
		if(rc.left<=0 && rc.top<=0 &&
			rc.right>=GetSystemMetrics(SM_CXSCREEN) &&
			rc.bottom>=GetSystemMetrics(SM_CYSCREEN)){
			return !checkProcessList(list, pid) && !checkProcess(pid, _T("explorer.exe"));
		}
	}
	return false;
}

bool eqBtn(int h, int p)
{
	if(h>=0) buttonToBitMask(p);
	return h==p;
}

void postHotkey(int i, LPARAM updown)
{
	PostMessage(hWin, WM_USER+2654, i, updown);
}

//-------------------------------------------------------------------------

LRESULT msgFromHook(WPARAM vk, LPARAM scan, int updown)
{
	int i;
	bool shift[Nshift];

	if(pcLocked) return 1;

	if(updown!=K_UP){
		for(i=0; i<Nshift; i++){
			shift[i]= (GetAsyncKeyState(shiftTab[i])<0);
		}
	}
	EnterCriticalSection(&listCritSect);
	for(i=0; i<numKeys; i++){
		HotKey *hk= &hotKeyA[i];
		if(vk==vkMouse ? hk->vkey==vkMouse && eqBtn(hk->scanCode, scan) :
			!((hk->scanCode^scan)&0x1ff0000) && hk->vkey<512 && hk->vkey &&
			(vk==hk->vkey || hk->vkey==255)){
			if(updown==K_UP){
				if(!hk->isDown) continue;
				hk->isDown=false;
			}
			UINT mod= hk->modifiers;
			if(updown==K_UP ||
				shift[shShift]==((mod&MOD_SHIFT)!=0) &&
				shift[shCtrl]==((mod&MOD_CONTROL)!=0) &&
				(shift[shAlt]==((mod&MOD_ALT)!=0) || altDown) &&
				(shift[shLWin]|shift[shRWin])==((mod&MOD_WIN)!=0)){
				if(hk->disable) break;
				if(hk->ignore){ hk->ignore=false; break; }
				if(hk->isActive()){
					//prevent the Start menu or a normal menu to show after Win+key or Alt+key
					if((hk->modifiers&(MOD_WIN|MOD_ALT))!=0 &&
						(hk->modifiers&~(MOD_WIN|MOD_ALT))==0 && updown!=K_UP){
						if(vk==vkMouse && useHook)
							preventWinMenu=true;
						else{
							keyEventDown(VK_CONTROL);
							keyEventUp(VK_CONTROL);
						}
					}
					/*if(!(hk->cmd==94 && hk->isDown && updown==K_DOWN))*/ //disable repeat for "Macro to active window"
					{
						if(updown==K_DOWN) hk->isDown=true;
						//hotkeys are executed in the main thread, not in the hook thread
						postHotkey(i, updown);
					}
					LeaveCriticalSection(&listCritSect);
					return 1;
				}
			}
		}
	}
	LeaveCriticalSection(&listCritSect);
	return 0;
}

bool HotKey::checkModifiers()
{
	return checkShifts(modifiers);
}

bool partofHotMouse(int u)
{
	int i, c, m;
	HotKey *hk;
	bool result=false;

	EnterCriticalSection(&listCritSect);
	for(i=0; i<numKeys; i++){
		hk=&hotKeyA[i];
		if(hk->vkey!=vkMouse || hk->disable) continue;
		c=hk->scanCode;
		if(c>=0){
			//old version - bitmask
			m=u;
			buttonToBitMask(m);
			if((c|m)==c && c!=m && hk->checkModifiers() && hk->isActive()){
				result=true;
				break;
			}
		}
		else{
			//new version - array
			for(c>>=4; c!=-1; c>>=4){
				if(c==u && hk->checkModifiers() && hk->isActive()){
					result=true;
					break;
				}
			}
		}
	}
	LeaveCriticalSection(&listCritSect);
	return result;
}

bool unDelayButtons()
{
	if (delayButtons == -1 || delayButtons2 != -1) {
		return false;
	}
	delayButtons2 = delayButtons;
	KillTimer(hWin, 3);
	//simulate previous mouse press
	for(int i=24; i>=0; i-=4){
		int down = (delayButtons >> i) & 15;
		if(down!=15) PostThreadMessage(idHookThreadM2, WM_USER + 203, down, 0); //mouse_eventDown
	}
	delayButtons=-1;
	return true;
}

int bitsearch(int m)
{
	int i;
	for(i=-1; m; i++) m>>=1;
	return i;
}

LPARAM clickFromHook(WPARAM mesg, LPARAM lP)
{
	int down, up, i;
	bool b;

	down=up=15;
	switch(mesg){
		case WM_LBUTTONDOWN: down=M_Left; break;
		case WM_LBUTTONUP:   up=M_Left; break;
		case WM_RBUTTONDOWN: down=M_Right; break;
		case WM_RBUTTONUP:   up=M_Right; break;
		case WM_MBUTTONDOWN: down=M_Middle; break;
		case WM_MBUTTONUP:   up=M_Middle; break;
		case WM_XBUTTONDOWN: down=bitsearch(lP); break;
		case WM_XBUTTONUP: up=bitsearch(lP); break;
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			down=buttons;
			i= (mesg==WM_MOUSEWHEEL) ? ((lP&0x100000) ? M_WheelDown : M_WheelUp) : ((lP&0x100000) ? M_WheelLeft : M_WheelRight);
			buttonDown(i, down);
			lastButtons=down;
			if(msgFromHook(vkMouse, down, K_ONLYDOWN)){
				buttonToBitMask(delayButtons);
				ignoreButtons|=delayButtons;
				delayButtons=-1;
				return 1;
			}
			if(unDelayButtons()){
				PostThreadMessage(idHookThreadM2, WM_USER + 203, i, 0); //mouse_eventDown
				return 1;
			}
			return 0;
	}
	if(down<15){
		//dbg("click down %d, %x,%x", down, delayButtons, delayButtons2);
		buttonDown(down, buttons);
		lastButtons=buttons;
		b=buttonUp(down, delayButtons2);
		if(!b && (delayButtons!=-1 || !notDelayButtons[down]) &&
			partofHotMouse(buttons) &&
			(!notDelayFullscreen || !checkFullscreen(delayApp)) &&
			!checkProcessList(notDelayApp) &&
			(!editing || GetForegroundWindow()!=hHotKeyDlg)){
			//delay this button press
			buttonDown(down, delayButtons);
			SetTimer(hWin, 3, mouseDelay, 0);
			return 1;
		}
		if(msgFromHook(vkMouse, buttons, K_ONLYDOWN)){ ///
			//ignore all pressed buttons
			ignoreButtons|=(1<<down);
			buttonToBitMask(delayButtons);
			ignoreButtons|=delayButtons;
			delayButtons=-1;
			return 1;
		}
		if(!b && unDelayButtons()){
			PostThreadMessage(idHookThreadM2, WM_USER + 203, down, 0); //mouse_eventDown
			return 1;
		}
	}
	if(up<15){
		//dbg("click up %d, %x,%x", up, delayButtons, delayButtons2);
		if(unDelayButtons()){
			PostThreadMessage(idHookThreadM2, WM_USER + 204, up, 0); //mouse_eventUp
			return 1;
		}
		buttonUp(up, buttons);
		if(altDown && buttons==-1){
			PostMessage(hWin, WM_USER+82, 0, 0);
		}
		if(ignoreButtons&(1<<up)){
			ignoreButtons&=~(1<<up);
			return 1;
		}
	}
	return 0;
}

char IgnoreLeftRight(LPARAM vk)
{
	if(vk==VK_LSHIFT || vk==VK_RSHIFT) return VK_SHIFT;
	if(vk==VK_LCONTROL || vk==VK_RCONTROL) return VK_CONTROL;
	if(vk==VK_LMENU || vk==VK_RMENU) return VK_MENU;
	return (char)vk;
}


bool KeyNeedsHook(UINT vk, UINT modifiers)
{
#ifndef NOHOOK
	if(useHook>0 && !disableAll)
	{
		if(useHook==3 || useHook==2 && vk>=0xA6 && vk<=0xB9 || specialKeys[vk]) return true;

		if((modifiers&MOD_WIN)!=0 && specialWinKeys[vk])
		{
			return modifiers==MOD_WIN
				|| vk>='0' && vk<='9' && (modifiers==(MOD_WIN|MOD_CONTROL) || modifiers==(MOD_WIN|MOD_SHIFT) || modifiers==(MOD_WIN|MOD_ALT))
				|| modifiers==(MOD_WIN|MOD_CONTROL) && (vk=='P' || vk=='F')
				|| modifiers==(MOD_WIN|MOD_SHIFT) && (vk=='M' || vk=='T' || vk==VK_UP || vk==VK_LEFT || vk==VK_RIGHT || vk==VK_DOWN);
		}
	}
#else
	(void)vk; (void)modifiers;
#endif
	return false;
}

LRESULT keyFromHook(WPARAM mesg, LPARAM vk, LPARAM scan)
{
	if(unsigned(vk)<sizeA(keyReal)){
		keyReal[vk]= (mesg==WM_KEYDOWN || mesg==WM_SYSKEYDOWN);
	}
	keyLastScan = scan;
	if(pcLocked){
		if(vk==VK_RETURN){
			if(mesg==WM_KEYUP){
				//check password
				BYTE p[Dpasswd];
				encrypt(p, Dpasswd, passwd, passwdLen, passwdAlg);
				if(!memcmp(p, password, Dpasswd)){
					//unlock
					PostMessage(hWin, WM_USER+6341, 0, 0);
				}
				passwdLen=0;
			}
		}
		else if(mesg==WM_KEYDOWN){
			if(vk==VK_BACK){
				if(passwdLen>0) passwdLen--;
			}
			else if(vk==VK_ESCAPE){
				passwdLen=0;
			}
			else if(passwdLen<sizeA(passwd)){
				passwd[passwdLen++]=IgnoreLeftRight(vk);
			}
		}
		return 1;
	}
	if(unsigned(vk)<sizeA(blockedKeys) && blockedKeys[vk]) return 1;

	if(preventWinMenu && mesg==WM_KEYUP && (vk==VK_LWIN || vk==VK_RWIN || vk==VK_LMENU)){
		preventWinMenu=false;
		keyEventDown(VK_CONTROL);
		keyEventUp(VK_CONTROL);
	}

	UINT modifiers=0;
	if(vk<255 && specialWinKeys[vk] && (GetAsyncKeyState(VK_LWIN)<0 || GetAsyncKeyState(VK_RWIN)<0)){
		modifiers=MOD_WIN;
		if(GetAsyncKeyState(VK_SHIFT)<0) modifiers|=MOD_SHIFT;
		if(GetAsyncKeyState(VK_CONTROL)<0) modifiers|=MOD_CONTROL;
		if(GetAsyncKeyState(VK_MENU)<0) modifiers|=MOD_ALT;
	}

	if(scan==6619136 && vk==76) vk=255; ///Copy key pressed after Win key

	if(KeyNeedsHook(vk, modifiers))
		return msgFromHook(vk, scan, (mesg==WM_KEYUP || mesg==WM_SYSKEYUP) ? K_UP : K_DOWN);
	return 0;
}

//-------------------------------------------------------------------------

LRESULT CALLBACK LowLevelMouseProc(int code, WPARAM wP, LPARAM lP)
{
	if(wP!=WM_MOUSEMOVE && code==HC_ACTION){
		if(clickFromHook(wP, ((MSLLHOOKSTRUCT*)lP)->mouseData>>11)) return 1;
	}
	return CallNextHookEx(hookM, code, wP, lP);
}

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wP, LPARAM lP)
{
	if(code==HC_ACTION && ((KBDLLHOOKSTRUCT*)lP)->dwExtraInfo != extraInfoIGNORE){
		if(keyFromHook(wP, ((KBDLLHOOKSTRUCT*)lP)->vkCode,
			(((KBDLLHOOKSTRUCT*)lP)->scanCode<<16)|(((KBDLLHOOKSTRUCT*)lP)->flags<<24))) return 1;
	}
	return CallNextHookEx(hookK, code, wP, lP);
}

void installHook(HHOOK &hook, int type, char *proc, HOOKPROC hproc)
{
	HINSTANCE mod;

	if(!hook){
		mod=inst;
		if(isWin9X){
			if(!klib) klib=LoadLibrary(_T("hook.dll"));
			hproc = (HOOKPROC)GetProcAddress(mod=klib, proc);
		}
		if(hproc){
#ifndef NOHOOK
			hook= SetWindowsHookEx(type, hproc, mod, 0);
			//if(!hook) msg("SetWindowsHookEx failed");
#else
			type;
#endif
		}
	}
}

void installHookT(WPARAM mouse)
{
	if(mouse){
		installHook(hookM, isWin9X ? WH_MOUSE : WH_MOUSE_LL,
			"_MouseProc95@12", LowLevelMouseProc);
	}
	else{
		memset(keyReal, 0, sizeof(keyReal));
		installHook(hookK, isWin9X ? WH_KEYBOARD : WH_KEYBOARD_LL,
			"_KeyboardProc95@12", LowLevelKeyboardProc);
	}
}

void uninstallHookT(WPARAM mouse)
{
	HHOOK &hook= mouse ? hookM : hookK;
	if(hook){
		if(!mouse) memset(keyReal, 0, sizeof(keyReal));
		UnhookWindowsHookEx(hook);
		hook=0;
	}
}

void installHook(bool mouse)
{
	messageToHook(WM_USER+201, mouse, mouse);
}

void uninstallHook(bool mouse)
{
	messageToHook(WM_USER+202, mouse, mouse);
}

void reloadHook()
{
	if(hookK){ uninstallHook(false); installHook(false); }
	if(hookM){ uninstallHook(true); installHook(true); }
}

void setHook(bool mouse)
{
	int i;
	bool b;
	HotKey *hk;

	b=pcLocked;
	if(mouse){
		if(editing) b=true;
		if(!disableMouse){
			for(i=0; i<numKeys; i++){
				hk=&hotKeyA[i];
				if(hk->vkey==vkMouse && (!disableAll ||
					hk->cmd==87 && hk->args[0]!='1')){
					b=true;
					break;
				}
			}
		}
	}
	else{
		for(i=0; i<256; i++){
			if(blockedKeys[i]){ b=true; break; }
		}
		if(useHook && !disableAll) b=true;
	}
	if(b){
		installHook(mouse);
	}
	else{
		uninstallHook(mouse);
	}
}

void setHook()
{
	setHook(false); //keyboard
	setHook(true);  //mouse
	if(keepHook && (hookK || hookM)){
		SetTimer(hWin, 10, keepHookInterval, 0);
	}
	//joystick
	bool b=editing;
	if(!disableJoystick && !disableAll){
		if(joyMouseEnabled) b=true;
		else
			for(int i=0; i<numKeys; i++){
				if(hotKeyA[i].vkey==vkJoy) b=true;
			}
	}
	if(b){
		while((signed)ResumeThread(joyThread) > 1);
	}
	else{
		SuspendThread(joyThread);
	}
}

void messageToHook(UINT mesg, WPARAM wP, bool mouse)
{
	int cnt=0;
	while(!PostThreadMessage(mouse ? idHookThreadM : idHookThreadK, mesg, wP, 0)){
		Sleep(10);
		if(++cnt > 1000) break;
	}
}

DWORD WINAPI hookProc(LPVOID)
{
	MSG mesg;

	while(GetMessage(&mesg, NULL, 0, 0)>0){
		switch (mesg.message)
		{
		case WM_USER + 201:
			installHookT(mesg.wParam);
			break;
		case WM_USER + 202:
			uninstallHookT(mesg.wParam);
			break;
		case WM_USER + 203:
			mouse_eventDown(mesg.wParam);
			Sleep(10);
			break;
		case WM_USER + 204:
			mouse_eventUp(mesg.wParam);
			break;
		default:
			DispatchMessage(&mesg);
		}
	}
	return 0;
}

//-------------------------------------------------------------------------

void registerHK(int i, bool disable)
{
	HotKey *hk= &hotKeyA[i];
	if(!disable && (disableAll && (hk->cmd!=87 || hk->args[0]=='1') ||
		disableMouse && hk->vkey==vkMouse || disableJoystick && hk->vkey==vkJoy ||
		disableLirc && hk->vkey==vkLirc ||
		disableKeys && hk->vkey<512 && (hk->cmd!=97 || hk->args[0]=='1'))) return;
	UINT vkey= hk->vkey;
	if(vkey){
		if(vkey>=255 || KeyNeedsHook(vkey, hk->modifiers)){
			hk->disable= disable;
		}
		else{
			if(disable) UnregisterHotKey(hWin, i);
			else RegisterHotKey(hWin, i, hk->modifiers, vkey);
		}
	}
}

void registerKeys()
{
	for(int i=0; i<numKeys; i++){
		registerHK(i, false);
	}
}

void unregisterKeys()
{
	for(int i=0; i<numKeys; i++){
		registerHK(i, true);
	}
}
