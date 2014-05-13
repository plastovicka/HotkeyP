/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef hotkeyPH
#define hotkeyPH
#include "lang.h"
#include "trayicon.h"


struct HotKey {
	char *note;      //comment
	char *exe;       //exe file or document with full path
	char *args;      //parameters
	char *dir;       //working directory
	char *sound;     //sound to play (zef)
	UINT modifiers;  //ctrl,shift,alt,win
	UINT vkey;       //virtual key code
	LPARAM scanCode; //scan key code (lParam from WM_KEYDOWN)
	int cmdShow;     //0=normal,1=maximized,2=minimized
	int opacity;     //0 to 255 (transparent to opaque)
	int priority;    //0=idle,1=normal,2=high,3=realtime
	int cmd;         //internal command (only if exe is NULL)
	DWORD processId; //running process pid, otherwise 0
	HANDLE process;  //running process handle, otherwise  0
	mutable int lock;//for multi command (zef: made mutable)
	bool ignore;
	bool disable;    //unregistered hotkey (for mouse buttons or when useHook==3)
	bool multInst;   //enable multiple instances
	bool trayMenu;   //show item in the system tray popup menu
	bool autoStart;  //execute this hotkey when HotkeyP starts
	bool ask;        //ask before executing this hotkey
	bool delay;      //wait before executing this hotkey
	bool isDown;
	char *lirc;      //WinLIRC remote control button name
	int icon;        //index into the small image list
	int category;
	int item;

	char const *getNote() const; //listview item description
	void resolveLNK();
	bool isActive() const;  //global hotkey or the window is active
	bool isLocal() const;   //cmd is only to active window
	bool isLocal1() const;  //cmd is only to active window, not multi-command
	bool parseMultiCmd(bool (HotKey::*f)() const) const;
	bool checkModifiers();
	void destroy();  //destructor
	bool inCategory(int category);
	int getIcon();
	std::string getFullExe() const;    // gets fully expanded exe name (zef)
	std::string getFullCmd() const;    // gets command name or fully expanded exe name (zef)
};

struct Tpopup {
	HWND hWnd;
	int x, y, width, height, opacity;
	int delay, refresh;
	void show(bool toggle=true);
};

struct DiskInfo {
	DWORDLONG free;
	int bar;
	char text[14];
};
/*
struct PROCESS_MEMORY_COUNTERS {
DWORD cb,PageFaultCount,PeakWorkingSetSize,WorkingSetSize,
QuotaPeakPagedPoolUsage,QuotaPagedPoolUsage,
QuotaPeakNonPagedPoolUsage,QuotaNonPagedPoolUsage,
PagefileUsage,PeakPagefileUsage;
};
*/
struct TmainButton {
	int cmd;
	WORD langId;
	char *caption;
};

struct WndItem
{
	WndItem *nxt;
	HWND w;
	WndItem(){ nxt=0; }
};

struct HideInfo
{
	WndItem *list;
	HWND activeWnd;
	DWORD pid;
	HICON icon;
};

struct TendpointName
{
	TendpointName *nxt;
	WCHAR *id;
	char *name, *nameLong;
	~TendpointName(){ delete[] name; delete[] nameLong; delete[] id; }
};

struct Zoom
{
	int magnification;
	int width, height;
	HWND wnd;
	HBITMAP bmp;
	HGDIOBJ oldBmp;
	HDC dc;
	void start();
	void move();
	void end();
};

struct ClipboardData
{
	UINT format; //data format
	void *data;  //clipboard content
	size_t size; //data size
	ClipboardData *nxt; //next format
};

struct CharList
{
	CharList* next;
	char* text;
};

struct PasteTextData
{
	ClipboardData *prev; //previous clipboard content
	char *text;  //text which will be pasted to the active window
	enum{ M=50 };
	char *L[M];  //list box
	int n;       //length of L
	CharList *queueFirst,*queueLast;
	bool busy;
	void save();
	void restore();
	void addToQueue(char *s);
	bool processQueue();
};

enum{
	clVol0, clVol1, clVolBkgnd, clDiskFree, clDskBkgnd, clVolText,
	clDskText, clVolBorder, clDskBorder, clVol2, clVol3, clVol4,
clTxtBkgnd, clTxtText, clTxtBorder, clLockText, Ncl
};
enum{ shShift, shCtrl, shAlt, shLWin, shRWin, Nshift };
enum{ shRAlt, shLShift, shRShift, shRCtrl, shLAlt, sh2LWin, sh2RWin, shLCtrl, Nshift2 };
enum{ M_Left, M_Right, M_Middle=4, M_X1, M_WheelLeft=11, M_WheelRight, M_WheelDown, M_WheelUp, M_None=15 };
enum{ P_Volume, P_DiskFree, P_ShowText, Npopup };
enum{ J_BUTTON, J_AXIS, J_POV };
#define JOY_SIGN 16

enum{ K_NONE, K_ONLYDOWN, K_DOWN, K_UP };

#define Npriority 6
#define Mvolume 12
#define Dpasswd 64
#define vkMouse 512
#define vkDelete 513
#define vkLirc 514
#define vkJoy 515
#define scanWheelUp 0x40000000
#define scanWheelDown 0x20000000
#define scanWheelRight 0x10000000
#define scanWheelLeft 0x8000000
#define extraInfoIGNORE 0x96143581
const int diskSepH=2, showTextBorder=7;

typedef char TfileName[MAX_PATH];

extern int numKeys, fontH, sentToActiveWnd, buttons, ignoreButtons, ignoreButtons2, passwdLen, pcLockX, pcLockY, diskfreePrec, curVolume[Mvolume][2], iconDelay, oldMute, lockSpeed, lockMute, disableTaskMgr, lircEnabled, useHook, cmdLineCmd, lastButtons, notDelayButtons[15], notDelayFullscreen, mouseDelay, keepHook, keepHookInterval, oldCDautorun, passwdAlg, hidePasswd, cmdFromKeyPress, lockPaste;
extern double pcLockDx, pcLockDy;
extern HotKey *hotKeyA;
extern HWND hWin, hWndLock, hWndLircState, hHotKeyDlg, hWndBeforeLock;
extern LPARAM keyLastScan;
extern DWORD idHookThreadK, idHookThreadM;
extern POINT mousePos;
extern bool modif, altDown, blockedKeys[256], pcLocked, isWin9X, isWin64, isVista, disableAll, disableMouse, disableJoystick, disableLirc, disableKeys, isHilited, editing, isZoom;
extern char volumeStr[256], *pcLockParam, notDelayApp[512], delayApp[512];
extern TfileName iniFile, lockBMP, exeBuf;
extern const char *subkey;
extern BYTE specialWinKeys[256];
extern CRITICAL_SECTION cdCritSect, listCritSect;
extern HHOOK hookK, hookM;
extern HMODULE klib;
extern HGDIOBJ lockImg;
extern const DWORD priorCnst[Npriority];
extern char *priorTab[Npriority];
extern const WORD showCnst[];
extern COLORREF colors[];
extern const int shiftTab[Nshift];
extern const int shift2Tab[Nshift2];
extern bool keyReal[256];
extern Tpopup popup[Npopup];
extern DiskInfo disks[26];
extern char *showTextStr;
extern OPENFILENAME snapOfn;
extern BYTE password[Dpasswd];
extern char keyMap[1024], *keyMapIndex[256];
extern HideInfo trayIconA[30];
extern Zoom zoom;
extern PasteTextData pasteTextData;
extern TendpointName *endpointNameList;

extern int lircPort;
extern char lircAddress[64];
extern TfileName lircExe;
extern int lircRepeat;
extern char *lircButton;
extern HANDLE lircThread;

extern HANDLE joyThread;
extern int joyMouseEnabled, joyMouseJoy, joyMouseX, joyMouseY, joyNotFullscreen, joyThreshold, joyMultiplier;
extern char joyApp[512];
extern char joyFullscreenApp[512];

BOOL createProcess(char *exe);
void registerHK(int i, bool disable);
void registerKeys();
void unregisterKeys();
bool saveAtExit();
void command(int cmd, char *param, HotKey *hk=0);
void volume(char *which, int value, int action);
void diskFree();
void resetDiskfree();
void writeini();
bool wr(char *fn);
bool save1(OPENFILENAME &ofn, DWORD flags);
int volumeH();
bool getVolumeName(int i, char *&name, int &len, int &mxId, int &rec, bool display);
int textY(int i);
void cpStr(char *&dest, char const *src);
bool isShiftKey(int c);
void checkWait();
void buttonToArray(int &param);
DWORD findProcess(char const *exe);
bool checkProcess(DWORD pid, char *exe);
void privilege(char *name);
void initList();
void setHook();
void ignoreHotkey(HotKey *hk);
LRESULT CALLBACK VolumeWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP);
LRESULT CALLBACK DiskFreeWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP);
LRESULT CALLBACK showTextWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP);
HWND findWindow(char const *exe, DWORD pid); // zef: made const correct
int msg1(int btn, char *text, ...);
void keyEventUp(int c);
void keyEventDown(int c);
void forceNoKey();
void acceptKey(UINT vkey, DWORD scan);
bool checkShifts(int m);
void mouse_eventDown(int i);
void mouse_eventUp(int i);
void mouse_event2(DWORD f, int d);
void setOpacity(HWND w, int o);
char *formatText(char *param);
bool cmdToWindow(int cmd, char *param);
void closeAll2();
void modifyTrayIcon();
void createPopups();
bool checkProcessList(char *list);
bool checkFullscreen(char *list);
bool isExe(char const *f);
bool testDir(char *dir);
char *getCmdName(int id);
void unhideApp(HideInfo *info);
bool hideMainWindow();
void unhideAll();
void copyToClipboard1(char *s);
void parseMacro(char *s);
void readini();
void removeDrive(char DriveLetter);
void removeUSBdrives();

void executeHotKey(int i);
void delayAndExecuteHotKey(HINSTANCE inst, HWND parent, int hotKeyIndex);
void printKey(char *s, HotKey* hk);
void correctMultiCmd(int item, int action, int item2=0);
void keyMapChanged();
void postHotkey(int i, LPARAM updown);
LRESULT keyFromHook(WPARAM mesg, LPARAM vk, LPARAM scan);
LPARAM clickFromHook(WPARAM mesg, LPARAM lP);
void installHook(bool mouse);
void uninstallHook(bool mouse);
DWORD WINAPI hookProc(LPVOID);
void messageToHook(UINT mesg, WPARAM wP, bool mouse);
bool unDelayButtons();

extern "C" int encrypt(BYTE *out, int outLen, char *in, int inLen, int alg);

int helpProcess();
bool showHelp(int anchorId);

void lircStart();
void lircEnd(bool wait=false);
void lircNotify();

DWORD WINAPI joyProc(void *);
char axisInd2Name(int i);
int axisName2Ind(char c);
bool joyGlobalEnabled();

std::string ExpandVars(std::string s);

typedef BOOL(__stdcall *TSetSuspendState)(BOOL, BOOL, BOOL);
typedef BOOL(__stdcall *TLockWorkStation)();
typedef BOOL(__stdcall *TdiskFreeFunc)(LPCTSTR, ULARGE_INTEGER*, ULARGE_INTEGER*, ULARGE_INTEGER*);
typedef BOOL(__stdcall *TmemInfo)(HANDLE, PROCESS_MEMORY_COUNTERS*, DWORD);
typedef BOOL(__stdcall *TsetOpacityFunc)(HWND, COLORREF, BYTE, DWORD);
typedef BOOL(__stdcall *TGetLayeredWindowAttributes)(HWND, COLORREF*, BYTE*, DWORD*);
typedef DWORD(__stdcall *TregisterServiceProcess)(DWORD, DWORD);
typedef BOOL(__stdcall *TGetGUIThreadInfo)(DWORD, LPGUITHREADINFO);
typedef LRESULT(__stdcall *TSendMessageTimeout)(HWND, UINT, WPARAM, LPARAM, UINT, UINT, PDWORD_PTR);
typedef DWORD(__stdcall *TGetProcessId)(HANDLE);
typedef LONG(__stdcall *TChangeDisplaySettingsEx)(LPCTSTR, LPDEVMODE, HWND, DWORD, LPVOID);
typedef BOOL(__stdcall *TChangeWindowMessageFilter)(UINT message, DWORD dwFlag);
typedef BOOL(__stdcall *TIsWow64Process)(HANDLE, PBOOL);


#define popupVolume popup[P_Volume]
#define popupDiskFree popup[P_DiskFree]
#define popupShowText popup[P_ShowText]

#define sizeA(a) (sizeof(a)/sizeof(*a))
#define endA(a) (a+(sizeof(a)/sizeof(*a)))

template <class T> inline void aminmax(T &x, int l, int h){
	if(x<l) x=l;
	if(x>h) x=h;
}

template <class T> inline void amax(T &x, int h){
	if(x>h) x=h;
}
template <class T> inline void amin(T &x, int l){
	if(x<l) x=l;
}

inline int random(int num){ return(int)(((long)rand()*num)/(RAND_MAX+1)); }


extern "C"{
	HWND WINAPI HtmlHelpA(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);
	HWND WINAPI HtmlHelpW(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData);
#ifdef UNICODE
#define HtmlHelp  HtmlHelpW
#else
#define HtmlHelp  HtmlHelpA
#endif
}

#endif
