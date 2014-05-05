/*
 (C) 2003-2014  Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  

//required libraries: htmlhelp.lib version.lib winmm.lib comctl32.lib ws2_32.lib

#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"
#include "resource.h"   // for resource generated identifiers

const DWORD priorCnst[]={IDLE_PRIORITY_CLASS,NORMAL_PRIORITY_CLASS,
  HIGH_PRIORITY_CLASS,REALTIME_PRIORITY_CLASS,16384,32768};
char *priorTab[]={"idle","normal","high","realtime","below normal","above normal"};
const WORD showCnst[]={SW_SHOWNORMAL,SW_SHOWMAXIMIZED,SW_SHOWMINIMIZED};
char *showTab[]={"normal","maximized","minimized"};
char *categoryTab[]={"default","all","keyboard","mouse","joystick","remote","commands","programs","documents","web links","autorun","tray menu"};

char *cmdNames[]={
"Eject CD","Close CD","Shutdown","Reboot","Suspend",
"Logoff","Screen saver","Maximize window","Minimize window","Close window",
/*10*/"Always on top","Terminate process","Control panel","Volume +","Volume -",
"Wave volume +","Wave volume -","Mute","Desktop resolution","Monitor power off",
/*20*/"Process priority","Send window command","Empty recycle bin","Random wallpaper","Disk free space",
"Hide window","Send keys to window","Macro","Center of the desktop","Bottom left corner",
/*30*/"Bottom","Bottom right corner","Left","Right","Top left corner",
"Top","Top right corner","Move mouse cursor","Show shutdown dialog","Play CD",
/*40*/"CD next track","CD stop","CD previous track","Left click","Middle click",
"Right click","Idle priority","Normal priority","High priority","Realtime priority",
/*50*/"Add/Remove programs","Display properties","Internet options","Mouse","Multimedia",
"Power options","System","Date and time","Mute Wave","Below normal priority",
/*60*/"Above normal priority","Multi command","Eject/Close CD","Lock computer","Hibernate",
"Minimize others","Information","Paste text","Next wallpaper","Previous wallpaper",
/*70*/"Commands list","Move window","Resize window","Command to active window","Keys to active window",
"Wheel","Double click","Opacity","Volume","Fourth button click",
/*80*/"Fifth button click","Show desktop","Change wallpaper","Previous task","Next task",
"Show text","Disable key","Disable all hotkeys","Disable mouse shortcuts","Desktop snapshot",
/*90*/"Window snapshot","Hide tray icon","Stop service","Start service","Macro to active window",
"Disable joystick shortcuts","Disable remote control","Disable keyboard shortcuts","Hide icon","Restore tray icon",
/*100*/"CD speed","Hide application","Minimize to tray","Magnifier","Clear recent documents",
"Delete temporary files","Save desktop icons","Restore desktop icons","Horizontal wheel","Remove drive"
};

BYTE cmdIcons[]={ 
      4,4,18,18,18,18,14,9,9,21,9,21,10,25,26,
/*15*/25,26,11,14,14,9,17,6,5,8,9,17,17,20,20,
/*30*/20,20,20,20,20,20,20,13,18,4,4,4,4,13,13,
/*45*/13,9,9,9,9,10,10,10,13,10,10,10,10,11,9,
/*60*/9,22,4,16,18,9,12,3,5,5,22,20,20,17,17,
/*75*/13,13,9,11,13,13,5,5,23,24,7,2,2,2,14,
/*90*/9,9,10,10,17,2,2,2,9,9,4,9,9,20,6,
/*105*/6,5,5,13
};

const BYTE specialWinKeysList[]={'E','R','D','F','M',VK_F1,VK_PAUSE,VK_TAB, //Win 98
 'U', //Win 2000
 'B','L', //Win XP
 'T','H','C','V','G',' ','X', '0','1','2','3','4','5','6','7','8','9', //Win Vista
 'P',VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_HOME,VK_ADD, VK_OEM_PLUS, //Win 7
 0};

TmainButton mainButton[]={
 {108,503,"&Add"},
 {104,502,"&Insert"},
 {102,500,"&Edit"},
 {216,504,"D&uplicate"},
 {103,501,"&Delete"},
 {107,505,"&Run"},
 {212,510,"&Options"},
};

const int version=5;  // config file version number
const char *magic="hotKeys"; //config file header
COLORREF colors[]= {
  0x90f0a0,0xffc0c0,0xf5f5f5,0xe8d2c7,0xf5fafa,0,0,0,0,
  0x9e9cfc,0x57ebf7,0x4b9ced,0xf5f5f5,0,0,0xf0f050};
const int senz=15;

TfileName exeBuf,lockBMP,regFile,iniFile,snapFile,wavFile;
char keyMap[1024],*keyMapIndex[256];
char *title="HotkeyP"; //window title
HotKey *hotKeyA;       //hotkeys table
int menuSubId[]={404,405,403,402,401,400};
int popupSubId[]={0,609,608,607,606,605,604,603,602,601,600};

bool
 modif,     //the file has been modified
 delreg,    //user deleted the registry settings
 renaming,
 altDown,   //used for task switch
 pcLocked,
 editing,
 isWin9X,
 isVista,
 disableAll,
 disableMouse,
 disableJoystick,
 disableLirc,
 disableKeys,
 isHilited,
 isZoom,
 treeResizing;

int
 dlgW=620,dlgH=375, //main window size
 dlgX=100,dlgY=100, //main window position
 fontH,             //system font height
 numKeys,           //number of hotkeys
 numList,
 autoRun=1,
 buttons=-1,        //mouse buttons that are down (when hook is on)
 ignoreButtons=0,
 ignoreButtons2=0,
 mouseDelay=600,
 notDelayButtons[15]={1},
 colWidth[]={50,150,200},//columns width
 sortedCol=0,       //sort by
 descending=-1,     //sort desc.
 trayicon=1,        //show icon in system tray
 minToTray=0,       //minimize button hides the window
 closeToTray=1,     //close button hides the window
 iconDelay=600,     //tray icon highlight duration
 insertAfterCurrent=1,
 mainBtnEnabled[sizeA(mainButton)]={1,1,1,0,1,0,1},
 pcLockX,pcLockY,
 curVolume[Mvolume][2], //volume, [line][mute]
 optionsPage,
 isLockColor,
 lockSpeed=10,
 lockMute=1,
 highPriority=1,
 disableTaskMgr=-1,
 oldMute,
 useHook=3,
 numImages,
 notDelayFullscreen,
 keepHook=0,
 keepHookInterval=500,
 cmdLineCmd,
 treeW=120,
 selectedCategory=-1,
 numCategories=1,
 passwdAlg,
 hidePasswd=0, //don't show Password:*** when computer is locked
 cmdFromKeyPress=0,
 delayExecute=0;

const int splitterW=4;

double pcLockDx,pcLockDy;
LPARAM keyLastScan;
char *pcLockParam;
char *showTextStr;
static HotKey dlgKey;
POINT mousePos;
static int show,oldW,oldH;
char notDelayApp[512],delayApp[512];
HHOOK hookK,hookM;
static char searchBuf[32];
static int searchLen;
static DWORD searchLastKey;
static COLORREF custom[16];
DiskInfo disks[26];
Tpopup popup[Npopup];
Zoom zoom;
static int treeLock;
char **categoryName;

HWND hWin,hHotKeyDlg,listBox,hWndLock,hWndLircState,tree,hWndBeforeLock;
HINSTANCE inst;
WNDPROC editWndProc;
HACCEL haccel;
LOGFONT font;
HFONT hFont;
HBRUSH brVolBk,brDskBk;
CRITICAL_SECTION listCritSect,cdCritSect;
HGDIOBJ lockImg;
HIMAGELIST himl;
DWORD idHookThreadK,idHookThreadM;
HANDLE joyThread;

const char *subkey="Software\\Petr Lastovicka\\hotkey";
const char *runkey="Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const char *runName="HotkeyP";
struct Treg { char *s; int *i; } regVal[]={
 {"height",&dlgH},
 {"width",&dlgW},
 {"X",&dlgX},
 {"Y",&dlgY},
 {"volumeX",&popupVolume.x},
 {"volumeY",&popupVolume.y},
 {"volumeW",&popupVolume.width},
 {"volumeDelay",&popupVolume.delay},
 {"volumeOpacity",&popupVolume.opacity},
 {"diskFreePrecision",&diskfreePrec},
 {"diskFreeX",&popupDiskFree.x},
 {"diskFreeY",&popupDiskFree.y},
 {"diskFreeWidth",&popupDiskFree.width},
 {"diskFreeOpacity",&popupDiskFree.opacity},
 {"showTextX",&popupShowText.x},
 {"showTextY",&popupShowText.y},
 {"showTextOpacity",&popupShowText.opacity},
 {"colId",&colWidth[0]},
 {"colKey",&colWidth[1]},
 {"colNote",&colWidth[2]},
 {"sort",&sortedCol},
 {"descending",&descending},
 {"trayicon",&trayicon},
 {"hiliteicon",&iconDelay},
 {"autoRun",&autoRun},
 {"minimizeToTray",&minToTray},
 {"closeToTray",&closeToTray},
 {"insertAfterCurrent",&insertAfterCurrent},
 {"notDelayLeftButton",&notDelayButtons[M_Left]},
 {"notDelayM",&notDelayButtons[M_Middle]},
 {"notDelayRightButton",&notDelayButtons[M_Right]},
 {"notDelayX1",&notDelayButtons[M_X1]},
 {"notDelayX2",&notDelayButtons[M_X1+1]},
 {"mouseDelay",&mouseDelay},
 {"optionsPage",&optionsPage},
 {"lockColorMode",&isLockColor},
 {"lockSpeed",&lockSpeed},
 {"lockMute",&lockMute},
 {"highPriority",&highPriority},
 {"oldTaskMgr",&disableTaskMgr},
 {"oldMute",&oldMute},
 {"lircPort",&lircPort},
 {"lircRepeat",&lircRepeat},
 {"lircOn",&lircEnabled},
 {"hook",&useHook},
 {"notDelayFullscreen",&notDelayFullscreen},
 {"hookInterval",&keepHookInterval},
 {"hookRefresh",&keepHook},
 {"joyThreshold",&joyThreshold},
 {"joyMultiplier",&joyMultiplier},
 {"joyNotFullscreen",&joyNotFullscreen},
 {"joyMouseEnabled",&joyMouseEnabled},
 {"joyMouseJoy",&joyMouseJoy},
 {"joyMouseX",&joyMouseX},
 {"joyMouseY",&joyMouseY},
 {"treeW",&treeW},
 {"vers",&passwdAlg},
 {"hidePasswd",&hidePasswd},
 {"delayExecute",&delayExecute},
};
struct Tregs { char *s; char *i; DWORD n; } regValS[]={
 {"file",iniFile,sizeof(iniFile)},
 {"language",lang,sizeof(lang)},
 {"volumeStr",volumeStr,sizeof(volumeStr)},
 {"regFile",regFile,sizeof(regFile)},
 {"keyMap",keyMap,sizeof(keyMap)},
 {"lockBMP",lockBMP,sizeof(lockBMP)},
 {"lircAddress",lircAddress,sizeof(lircAddress)},
 {"lircExe",lircExe,sizeof(lircExe)},
 {"delayApp",delayApp,sizeof(delayApp)},
 {"notDelayApp",notDelayApp,sizeof(notDelayApp)},
 {"snapshotFile",snapFile,sizeof(snapFile)},
 {"joyApp",joyApp,sizeof(joyApp)},
 {"joyFullscreenApp",joyFullscreenApp,sizeof(joyFullscreenApp)},
};
struct Tregs2 { char *s; char **i; } regValS2[]={
 {"param",&pcLockParam},
};
struct Tregb { TCHAR *s; void *i; DWORD n; } regValB[]={
 {"font",&font,sizeof(LOGFONT)},
 {"colors",colors,sizeof(colors)},
 {"data",password,sizeof(password)},
 {"buttons",mainBtnEnabled,sizeof(mainBtnEnabled)},
};

OPENFILENAME ofn={
 OPENFILENAME_SIZE_VERSION_400,0, 0,0,0,0,1,
 iniFile, sizeof(iniFile),
 0,0,0, 0, 0,0,0, "HTK", 0,0,0
};
OPENFILENAME exeOfn={
 OPENFILENAME_SIZE_VERSION_400,0, 0,0,0,0,1,
 exeBuf, sizeof(exeBuf),
 0,0,0, 0, 0,0,0, "EXE", 0,0,0
};
OPENFILENAME argOfn={
 OPENFILENAME_SIZE_VERSION_400,0, 0,0,0,0,1,
 exeBuf, sizeof(exeBuf),
 0,0,0, 0, 0,0,0, "", 0,0,0
};
OPENFILENAME regOfn={
 OPENFILENAME_SIZE_VERSION_400,0, 0,0,0,0,1,
 regFile, sizeof(regFile),
 0,0,0, 0, 0,0,0, "REG", 0,0,0
};
OPENFILENAME snapOfn={
 OPENFILENAME_SIZE_VERSION_400,0, 0,0,0,0,1,
 snapFile, sizeof(snapFile),
 0,0,0, 0, 0,0,0, "BMP", 0,0,0
};
OPENFILENAME wavOfn = {
  OPENFILENAME_SIZE_VERSION_400, 0, 0, 0, 0, 0, 1,
  wavFile, sizeof(wavFile),
  0, 0, 0, 0, 0, 0, 0, "WAV", 0, 0, 0
};
//-------------------------------------------------------------------------
int vmsg(char *caption, char *text, int btn, va_list v)
{
 char buf[1024];
 if(!text) return IDCANCEL;
 _vsnprintf(buf,sizeof(buf),text,v);
 buf[sizeof(buf)-1]=0;
 return MessageBox(0,buf,caption,btn|MB_SETFOREGROUND);
}

int msg1(int btn, char *text, ...)
{
 va_list ap;
 va_start(ap,text);
 int result = vmsg(title,text,btn,ap);
 va_end(ap);
 return result;
}

void msg(char *text, ...)
{
 va_list ap;
 va_start(ap,text);
 vmsg(title,text,MB_OK|MB_ICONERROR,ap);
 va_end(ap);
}

void msglng(int id, char *text, ...)
{
 va_list ap;
 va_start(ap,text);
 vmsg(title,lng(id,text),MB_OK|MB_ICONERROR,ap);
 va_end(ap);
}

//change button position in dialog
void moveX(HDWP p, HWND hDlg, int id, int dx)
{
 RECT rc;
 POINT pt;
 HWND hWnd;

 hWnd=GetDlgItem(hDlg,id);
 GetWindowRect(hWnd,&rc);
 pt.x=rc.left;
 pt.y=rc.top;
 ScreenToClient(hDlg,&pt);
 DeferWindowPos(p,hWnd,0, pt.x+dx, pt.y,
   0,0,SWP_NOSIZE|SWP_NOZORDER);
}

void moveW(HDWP p, HWND hDlg, int id, int dx, int dy)
{
 RECT rc;
 HWND hWnd;

 hWnd=GetDlgItem(hDlg,id);
 GetWindowRect(hWnd,&rc);
 DeferWindowPos(p,hWnd,0,0,0,
   rc.right-rc.left+dx, rc.bottom-rc.top+dy,
   SWP_NOMOVE|SWP_NOZORDER);
}

//test if folder exists
bool testDir(char *dir)
{
 if(!dir[0]) return false;
 DWORD attr= GetFileAttributes(dir);
 return attr==0xFFFFFFFF || !(attr&FILE_ATTRIBUTE_DIRECTORY);
}

//read edit box
int getText(HWND dlg, int id, char *&s)
{
 delete[] s;
 int l= (int) SendMessage(GetDlgItem(dlg,id),WM_GETTEXTLENGTH,0,0)+1;
 s = new char[l];
 return GetDlgItemText(dlg,id,s,l);
}

void cpStr(char *&dest, char const *src)
{
 char *old=dest; 
 dest = new char[strlen(src)+1];
 strcpy(dest, src);
 delete[] old; //src can be a pointer into old 
}

void cpStrQuot(char *&dest, char *src)
{
 delete[] dest;
 size_t len=strlen(src);
 char *d= dest= new char[len+3];
 *d++='\"';
 strcpy(d, src);
 d+=len;
 *d++='\"';
 *d=0;
}

void cutQuot(char *&s)
{
 if(s[0]=='\"'){
   char *e= strchr(s,0)-1;
   if(*e=='\"'){
     *e=0;
     cpStr(s,s+1);
   }
 }
}

bool isWWW(char const *s) // zef: made const correct
{
 return !_strnicmp(s,"www.",4) || !_strnicmp(s,"http://",7) 
   || !_strnicmp(s,"https://",8) || !_strnicmp(s,"mailto:",7);
}

bool isExe(char const *f) // zef: made const correct
{
 if(isWWW(f)) return false;
 char const *s=strchr(f,0)-3;
 if(s<=f || s[-1]!='.') return false;
 return !_stricmp(s,"exe") || !_stricmp(s,"com") ||
  !_stricmp(s,"bat") || !_stricmp(s,"scr") || !_stricmp(s,"cmd");
}

char *getCmdName(int id)
{
 return lng(1000+id, cmdNames[id]);
}


void HotKey::destroy()
{
 delete[] exe;
 delete[] args;
 delete[] note;
 delete[] dir;
 delete[] lirc;
 if(process) CloseHandle(process);
}

void destroyAll()
{
  int i;
  for(i=0; i<numKeys; i++){
    hotKeyA[i].destroy();
  }
  numKeys=0;
  for(i=1; i<numCategories; i++){
    delete[] categoryName[i];
  }
  numCategories=1;
  selectedCategory=-1;
}

BOOL createProcess(char *exe)
{
  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  BOOL result;

  memset(&si,0,sizeof(STARTUPINFO));
  si.cb= sizeof(STARTUPINFO);
  if((result= CreateProcess(0,exe,0,0,FALSE,0,0,0,&si,&pi))!=0){
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }else{
    msglng(743,"Cannot run %s",exe);
  }
  return result;
}

void HotKey::resolveLNK() 
{ 
  int i;
  IShellLink *psl; 
  IPersistFile *ppf; 
  WIN32_FIND_DATA wfd; 
  char buf[MAX_PATH]; 
  WCHAR wsz[MAX_PATH]; 
  
  char *s=strchr(exe,0)-4;
  if(s<=exe || (_stricmp(s,".lnk") && _stricmp(s,".pif"))) return;

  CoInitialize(0);
  if(SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, 
    CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl))){
    if(SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (void**)&ppf))){
      MultiByteToWideChar(CP_ACP, 0, exe, -1, wsz, MAX_PATH); 
      if(SUCCEEDED(ppf->Load(wsz, STGM_READ))){ 
        if(SUCCEEDED(psl->Resolve(0, SLR_NO_UI))){
          if(SUCCEEDED(psl->GetPath(buf,sizeA(buf), &wfd,0))){
            cpStr(exe,buf);
          }
          if(SUCCEEDED(psl->GetArguments(buf,sizeA(buf)))){
            cpStr(args,buf);
          }
          if(SUCCEEDED(psl->GetWorkingDirectory(buf,sizeA(buf)))){
            if(buf[0]!='%') cpStr(dir,buf);
          }
          if(SUCCEEDED(psl->GetShowCmd(&i))){
            if(i==SW_MINIMIZE || i==SW_SHOWMINNOACTIVE) cmdShow=2;
            if(i==SW_MAXIMIZE) cmdShow=1;
          }
        } 
      } 
      ppf->Release(); 
    }
    psl->Release(); 
  }
  CoUninitialize();
} 

int emptyImlSlot()
{
  int i,n,j,result;
  bool *b;

  n=ImageList_GetImageCount(himl);
  b= new bool[n];
  memset(b,0,n);
  for(i=0; i<numKeys; i++){
    j=hotKeyA[i].icon;
    if(j>0) b[j]=true;
  }
  result=-1;
  for(i=numImages; i<n; i++){
    if(!b[i]) result=i;
  }
  delete[] b;
  return result;
}


int HotKey::getIcon()
{
  // zef: added support for environment variables in path names
  const std::string fullExe = getFullExe();
  const char *exe = fullExe.c_str();

 if(!icon){
   if(cmd>=0){
     icon= cmdIcons[cmd];
   }else if(isWWW(exe)){
     icon= !_strnicmp(exe,"mailto:",7) ? 15 : 19;
   }else{
     icon= -1;
     char *s;
     char buf[MAX_PATH],buf2[MAX_PATH];
     int iconIndex=0;
     bool docIcon = false;
     if(isExe(exe)){
       //get full path of the exe file
       SearchPath(0,exe,0,sizeof(buf),buf,&s);
     }else{
       {
         //get file extension of the document
         const char *t=strrchr(exe,'.');
         if(t){
           //find DefaultIcon in the registry
           HKEY key;
           DWORD d;
           if(RegOpenKeyEx(HKEY_CLASSES_ROOT,t,0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){
             d=sizeof(buf)-13;
             if(RegQueryValueEx(key,0,0,0,(BYTE*)buf,&d)==ERROR_SUCCESS){
               strcat(buf,"\\DefaultIcon");
               HKEY key2;
               if(RegOpenKeyEx(HKEY_CLASSES_ROOT,buf,0,KEY_QUERY_VALUE,&key2)==ERROR_SUCCESS){
                 d=sizeof(buf2);
                 if(RegQueryValueEx(key2,0,0,0,(BYTE*)buf2,&d)==ERROR_SUCCESS){
                   if(ExpandEnvironmentStrings(buf2,buf,sizeof(buf)-1)){
                     //parse icon index
                     char * t=strrchr(buf,',');
                     if(t){
                       char *e;
                       iconIndex= strtol(t+1,&e,10);
                       if(!*e) *t=0;
                     }
                     //remove quotes
                     if(*buf=='"'){
                       strchr(buf,0)[-1]=0;
                       strcpy(buf,buf+1);
                     }
                     //ignore empty string or "%1"
                     if(*buf && *buf!='%') docIcon=true;
                   }
                 }
                 RegCloseKey(key2);
               }
             }
             RegCloseKey(key);
           }
         }
       }
l1:
       //which exe is used to open the document
       if(!docIcon) FindExecutable(exe,dir,buf);
     }
     HICON hi;
     if((int)ExtractIconEx(buf,iconIndex,0,&hi,1) > 0){
       //add icon to the image list
       icon= ImageList_ReplaceIcon(himl,emptyImlSlot(),hi);
       DestroyIcon(hi);
     }
     else if(docIcon){ docIcon=false; iconIndex=0; goto l1; }
   }
 }
 return (icon>=0) ? icon : 9;
}

// Gets the fully expanded executable to invoke
std::string HotKey::getFullExe() const
{
    return ExpandVars( exe );
}

// Gets the fully expanded executable or command name that will be invoked
std::string HotKey::getFullCmd() const
{
    if ( cmd >= 0 )
        return getCmdName( cmd );

    return getFullExe();
}

//-------------------------------------------------------------------------
void drawLockText()
{
  HDC dc;
  char *s;
  int i;
  double K;
  HGDIOBJ oldF;
  LOGFONT font;
  char buf[24+Dpasswd];
  RECT rc;
  static RECT rcOld;
  static int R,G,B;
  static int cnt,n;

  //get text
  if(passwdLen && !hidePasswd){
    s=buf+sprintf(buf,"%.20s:  ",lng(373,"Password"));
    for(i=0; i<passwdLen; i++) s[i]='*';
    s[i]=0;
    s=buf;
  }else{
    s=formatText(pcLockParam);
    if(s[0]==' ' && s[1]==0) s[0]=0;
  }
  //create font
  memset(&font,0,sizeof(font));
  font.lfHeight=28;
  font.lfWeight=FW_NORMAL;
  font.lfCharSet=DEFAULT_CHARSET;
  strcpy(font.lfFaceName,"Arial");
  dc=GetDC(hWndLock);
  oldF=SelectObject(dc,CreateFontIndirect(&font));
  //move
  pcLockX=int(pcLockX+pcLockDx);
  pcLockY=int(pcLockY+pcLockDy);
  //calculate size of new text
  rc.left=rc.top=0;
  DrawText(dc,s,-1,&rc,DT_CALCRECT|DT_NOPREFIX|DT_NOCLIP|DT_CENTER);
  OffsetRect(&rc,pcLockX-(rc.right>>1),pcLockY-(rc.bottom>>1));
  //bounce
  i=-rc.left;
  if(i>0){ 
    OffsetRect(&rc,i,0); 
    pcLockX+=i; 
    pcLockDx=-pcLockDx; 
  }
  i=-rc.top;
  if(i>0){ 
    OffsetRect(&rc,0,i);
    pcLockY+=i; 
    pcLockDy=-pcLockDy; 
  }
  i=GetSystemMetrics(SM_CXSCREEN)-rc.right;
  if(i<0){ 
    OffsetRect(&rc,i,0); 
    pcLockX+=i; 
    pcLockDx=-pcLockDx; 
  }
  i=GetSystemMetrics(SM_CYSCREEN)-rc.bottom;
  if(i<0){ 
    OffsetRect(&rc,0,i); 
    pcLockY+=i; 
    pcLockDy=-pcLockDy; 
  }
  //color
  if(--cnt<0){
    cnt=5;
    i=random(128);
    switch(random(3)){
      case 0: R^=i; break;
      case 1: G^=i; break;
      case 2: B^=i; break;
    }
    K=600.0/(R+G+B+1);
    R=(int)(R*K); amax(R,255);
    G=(int)(G*K); amax(G,255);
    B=(int)(B*K); amax(B,255); 
  }
  SetTextColor(dc, isLockColor ? colors[clLockText] : RGB(R,G,B) );
  SetBkMode(dc,TRANSPARENT);
  //erase previous text
  if(lockImg){
    SetBkColor(dc,0);
    RedrawWindow(hWndLock,&rcOld,0,RDW_INVALIDATE|RDW_UPDATENOW);
  }else{
    FillRect(dc,&rcOld,(HBRUSH)GetStockObject(BLACK_BRUSH));
  }
  //draw
  DrawText(dc,s,-1,&rc,DT_NOPREFIX|DT_NOCLIP|DT_CENTER);
  DeleteObject(SelectObject(dc,oldF));
  ReleaseDC(hWndLock,dc);
  CopyRect(&rcOld,&rc);
  if(s!=buf) delete[] s;
}

bool isEmptyPassword()
{
 for(int i=0; i<Dpasswd; i++){
   if(password[i]) return false;
 }
 return true;
}

void restoreTaskMgr()
{
  HKEY key;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",0,KEY_SET_VALUE,&key)==ERROR_SUCCESS){
    RegSetValueEx(key,"DisableTaskMgr",0,REG_DWORD,(LPBYTE)&disableTaskMgr,4);
    RegCloseKey(key);
  }
}

void restoreAfterLock()
{
  restoreTaskMgr();
  if(lockMute) volume(0,oldMute,3);
}

//-------------------------------------------------------------------------
void sortChanged()
{
 HD_ITEM hd;
 HWND header= ListView_GetHeader(listBox);

 for(int i=0; i<sizeA(colWidth); i++){
   if(i==sortedCol){
     hd.mask = HDI_IMAGE | HDI_FORMAT;
     hd.fmt = HDF_STRING | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
     hd.iImage = descending==1;
   }else{
     hd.mask = HDI_FORMAT;
     hd.fmt = HDF_STRING;
   }
   Header_SetItem(header,i,&hd);
 }
}

HMENU insertSubmenu(HMENU menu, char const *name)
{
  HMENU p;
  for(int i=GetMenuItemCount(menu)-1; i>=0; i--){
    p=GetSubMenu(menu,i);
    if(p){
      char buf[96];
      GetMenuString(menu,i,buf,sizeA(buf),MF_BYPOSITION);
      if(!strcmp(name,buf)) return p;
    }
  }
  p=CreatePopupMenu();
  AppendMenu(menu,MF_POPUP,(UINT)p,name);
  return p;
}

void addItemToSubmenu(HMENU menu, bool checked, int id, char const *name)
{
  for(char const *s=name; *s; s++){
    if(*s=='-' && s[1]=='>'){
      if(s==name){
        name+=2;
      }else{
        std::string menuName(name,s);
        addItemToSubmenu(insertSubmenu(menu,menuName.c_str()), checked, id, s+2);
        return;
      }
    }
  }
  AppendMenu(menu, checked ? MF_CHECKED|MF_STRING:MF_STRING, id, name);
}


void showPopup(int x, int y, char *name, int *subId, HWND wnd)
{
 HMENU hMenu= loadMenu(name,subId);
 HMENU subMenu= GetSubMenu(hMenu,0);
 if(!strcmp(name,"ICONPOPUP")){
   bool sep=false;
   for(int i=0; i<numKeys; i++){
     HotKey *hk=&hotKeyA[i];
     if(hk->trayMenu){
       if(!sep){
         sep=true;
         AppendMenu(subMenu,MF_SEPARATOR,0,0);
       }
       addItemToSubmenu(subMenu, 
         disableAll && hk->cmd==87 || disableMouse && hk->cmd==88 || disableJoystick && hk->cmd==95 || disableLirc && hk->cmd==96 || disableKeys && hk->cmd==97,
         1000+i, hk->getNote());
     }
   }
 }
 TrackPopupMenuEx(subMenu,
   TPM_CENTERALIGN|TPM_RIGHTBUTTON, x, y, wnd, NULL);
 DestroyMenu(hMenu);
}

void langChanged()
{
 int i;

 //menu
 loadMenu(hWin,"MENU1",menuSubId);
 //buttons in main window
 for(i=0; i<sizeA(mainButton); i++){
   TmainButton *b = &mainButton[i];
   SetWindowText(GetDlgItem(hWin,b->cmd),lng(b->langId,b->caption));
 }
 //file filters
 ofn.lpstrFilter=lng(506,"Hotkeys (*.htk)\0*.htk\0All files\0*.*\0");
 exeOfn.lpstrFilter=lng(507,"All files\0*.*\0Executable files\0*.exe;*.com;*.bat;*.scr\0");
 argOfn.lpstrFilter=lng(508,"All files\0*.*\0");
 regOfn.lpstrFilter=lng(509,"Config files (*.reg)\0*.reg\0All files\0*.*\0");
 snapOfn.lpstrFilter=lng(511,"Bitmap images (*.bmp)\0*.bmp\0");
 wavOfn.lpstrFilter = lng(512, "Waveform audio (*.wav)\0*.wav\0");
 //ListView
 LV_COLUMN lvc;
 static char *colNames[]={"","Key","Description"};
 for(i=0; i<sizeA(colNames); i++){
   lvc.mask = LVCF_TEXT;
   lvc.pszText= lng(720+i,colNames[i]);
   ListView_SetColumn(listBox,i,&lvc);
 }
 initList();
}
//---------------------------------------------------------------------------

int getItemFromIndex(int i)
{
 if(i<0) return i;
 LV_ITEM lvi;
 lvi.iItem= i;
 lvi.iSubItem=0;
 lvi.mask=LVIF_PARAM;
 ListView_GetItem(listBox, &lvi);
 return (int)lvi.lParam;
}

int getIndexFromItem(int i)
{
 LV_FINDINFO lfi;
 lfi.flags=LVFI_PARAM;
 lfi.lParam=(LPARAM)i;
 return ListView_FindItem(listBox, -1, &lfi);
}

bool getSel(int i)
{
  return ListView_GetItemState(listBox, i, LVIS_SELECTED)!=0; 
}

void setSel(int i, bool sel)
{
  ListView_SetItemState(listBox, i, sel ? LVIS_SELECTED:0, LVIS_SELECTED);
}

void setCurIndex(int index)
{
 if(index>=0){
   amax(index,numList-1);
   setSel(-1,false);
   ListView_SetItemState(listBox, index,
     LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
   ListView_EnsureVisible(listBox, index, FALSE);
   SetFocus(listBox);
 }
}

void setCur(int item)
{
 setCurIndex(getIndexFromItem(item));
}

//---------------------------------------------------------------------------

int findCategory(char *name)
{
  if(!*name) return 0;
  for(int i=1; i<numCategories; i++){
    if(!strcmp(categoryName[i],name)) return i;
  }
  //add new category
  char **A = new char*[numCategories+1];
  if(categoryName) memcpy(A,categoryName,numCategories*sizeof(A[0]));
  A[numCategories] = 0;
  cpStr(A[numCategories],name);
  delete[] categoryName;
  categoryName=A;
  return numCategories++;
}

static HTREEITEM hTreeItem;

void addTreeItem(char *text, LPARAM param)
{
 TV_INSERTSTRUCT tvins;
 TV_ITEM &tvi = tvins.item;

 tvins.hParent = TVI_ROOT;
 tvi.mask = TVIF_TEXT | TVIF_PARAM;
 tvi.pszText = text;
 tvi.lParam = param;
 tvins.hInsertAfter = TVI_LAST;
 HTREEITEM h = TreeView_InsertItem(tree, &tvins);
 if(param==selectedCategory) hTreeItem=h;
}

void buildTree()
{
  int i,j;
  bool b;
  
  //clear
  treeLock++;
  SendMessage(tree,WM_SETREDRAW,FALSE,0);
  TreeView_DeleteAllItems(tree);
  hTreeItem=0;
  //insert items
  addTreeItem(lng(979,categoryTab[1]), -1); //all
  if(numKeys){
    //user defined categories
    if(numCategories>1){
      for(i=0; i<numKeys; i++){
        if(hotKeyA[i].inCategory(0)){
          addTreeItem(lng(980,categoryTab[0]),0); //default
          break;
        }
      }
      for(j=1; j<numCategories; j++){
        addTreeItem(categoryName[j], j);
      }
    }
    //built-in categories
    for(j=2; j<sizeA(categoryTab); j++){
      b= hotKeyA[0].inCategory(-j);
      for(i=1; i<numKeys; i++){
        if(hotKeyA[i].inCategory(-j)!=b){
          addTreeItem(lng(980-j,categoryTab[j]), -j);
          break;
        }
      }
    }
  }
  //selected item
  /*if(!hTreeItem){
    selectedCategory=-1;
    hTreeItem= TreeView_GetRoot(tree);
  }*/
  TreeView_Select(tree,hTreeItem,TVGN_CARET);
  //redraw the treeview
  SendMessage(tree,WM_SETREDRAW,TRUE,0);
  UpdateWindow(tree);
  treeLock--;
}

void fillCategories(HWND hWnd, int sel)
{
  HWND combo= GetDlgItem(hWnd,118);
  for(int i=1; i<numCategories; i++){
    SendMessage(combo,CB_ADDSTRING,0,(LPARAM)categoryName[i]);
  }
  SetDlgItemText(hWnd,118, (sel>0) ? categoryName[sel] : "");
}

void swapCategory(int i1, int i2)
{
  int i;
  char *w;

  if(i1<=0 || i2>=numCategories) return;
  w=categoryName[i1];
  categoryName[i1]=categoryName[i2];
  categoryName[i2]=w;

  for(i=0; i<numKeys; i++){
    HotKey *hk= &hotKeyA[i];
    if(hk->category==i1) hk->category=i2;
    else if(hk->category==i2) hk->category=i1;
  }
  if(selectedCategory==i1) selectedCategory=i2;
  else if(selectedCategory==i2) selectedCategory=i1;
  buildTree();
  modif=true;
}

void deleteCategory()
{
  int i,*p;
  char **item;

  //delete category name
  item= &categoryName[selectedCategory];
  delete[] *item;
  numCategories--;
  memmove(item,item+1,(numCategories-selectedCategory)*sizeof(categoryName[0]));
  //move hotkeys
  for(i=0; i<numKeys; i++){
    p= &hotKeyA[i].category;
    if(*p>=selectedCategory){
      if(*p==selectedCategory) *p=0; //default category
      else (*p)--;
    }
  }
  selectedCategory=-1;
  //redraw tree and list
  initList();
  modif=true;
}

BOOL CALLBACK assignCategoryProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
 int i,c;

 switch(msg){
  case WM_INITDIALOG:
   setDlgTexts(hWnd,21);
   fillCategories(hWnd,0);
   return TRUE;

  case WM_COMMAND:
   wP=LOWORD(wP);
   switch(wP){
    case IDOK:
     GetDlgItemText(hWnd,118,exeBuf,sizeof(exeBuf));
     c=findCategory(exeBuf);
     for(i=0; i<numList; i++){
       if(getSel(i)){
         hotKeyA[getItemFromIndex(i)].category = c;
       }
     }
     setSel(-1,false);
     initList();
     modif=true;

    case IDCANCEL:
     EndDialog(hWnd, wP);
    return TRUE;
   }
  break;
 }
 return 0;
}

BOOL CALLBACK renameCategoryProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
 switch(msg){
  case WM_INITDIALOG:
   setDlgTexts(hWnd,20);
   SetDlgItemText(hWnd,118,categoryName[selectedCategory]);
   return TRUE;

  case WM_COMMAND:
   wP=LOWORD(wP);
   switch(wP){
    case IDOK:
     getText(hWnd,118,categoryName[selectedCategory]);
     buildTree();
     modif=true;
    case IDCANCEL:
     EndDialog(hWnd, wP);
    return TRUE;
   }
  break;
 }
 return 0;
}
//-------------------------------------------------------------------------
int CALLBACK sortId(LPARAM a, LPARAM b, LPARAM p)
{
 return int(p)*int(b-a);
}

int modifOrder(int m)
{
 int result=0;
 if(m & MOD_CONTROL) result|=8;
 if(m & MOD_SHIFT) result|=4;
 if(m & MOD_ALT) result|=2;
 if(m & MOD_WIN) result|=1;
 return result;
}

int CALLBACK sortKey(LPARAM a, LPARAM b, LPARAM p)
{
 HotKey *ha,*hb;
 ha=&hotKeyA[a];
 hb=&hotKeyA[b];
 int result;
 result= int(modifOrder(hb->modifiers) - modifOrder(ha->modifiers));
 if(!result) result= int(hb->vkey - ha->vkey);
 if(!result) result= int(hb->scanCode - ha->scanCode);
 return int(p)*result;
}

int CALLBACK sortNote(LPARAM a, LPARAM b, LPARAM p)
{
 return int(p) * _stricmp(hotKeyA[b].getNote(), hotKeyA[a].getNote());
}

bool HotKey::inCategory(int category)
{
  if(category>=0){
    return this->category==category;
  }
  switch(category){
  case -1:
    return true;
  case -2:
    return vkey<512;
  case -3:
    return vkey==vkMouse;
  case -4:
    return vkey==vkJoy;
  case -5:
    return vkey==vkLirc;
  case -6:
    return cmd>=0;
  case -7:
    return cmd<0 && isExe(exe);
  case -8:
    return cmd<0 && !isExe(exe) && !isWWW(exe);
  case -9:
    return cmd<0 && isWWW(exe);
  case -10:
    return autoStart;
  case -11:
    return trayMenu;
  }
  return false;;
}

//insert items to the listBox
void initList1()
{
 int i,bottom,n;
 LV_ITEM lvi;

 //filter
 numList=0;
 for(i=0; i<numKeys; i++){
   if(hotKeyA[i].inCategory(selectedCategory)){
     hotKeyA[numList++].item = i;
   }
 }

 SendMessage(listBox, WM_SETREDRAW, FALSE, 0);
 bottom=ListView_GetTopIndex(listBox)+ListView_GetCountPerPage(listBox)-1;
 amax(bottom,numList-1);
 //delete old items and insert new items
 n=ListView_GetItemCount(listBox);
 while(n>numList){
   n--;
   ListView_DeleteItem(listBox,n);
 }
 ListView_SetItemCount(listBox, numList);
 lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
 lvi.state = 0;
 lvi.stateMask = 0;
 lvi.pszText = LPSTR_TEXTCALLBACK;
 lvi.iImage = -1;
 lvi.iSubItem = 0;
 while(n<numList){
   lvi.iItem = n;
   ListView_InsertItem(listBox, &lvi);
   n++;
 }
 //sort
 for(i=0; i<numList; i++){
   lvi.iItem = i;
   lvi.lParam = (LPARAM) hotKeyA[i].item;
   lvi.mask = LVIF_PARAM | LVIF_IMAGE;
   lvi.iImage = I_IMAGECALLBACK;
   ListView_SetItem(listBox, &lvi);
 }
 static int (CALLBACK *sortFunc[])(LPARAM,LPARAM,LPARAM)=
   {sortId,sortKey,sortNote};
 ListView_SortItems(listBox, sortFunc[sortedCol], (LPARAM)descending);
 SendMessage(listBox, WM_SETREDRAW, TRUE, 0);
 ListView_EnsureVisible(listBox, bottom, TRUE);
}

void initList()
{
 initList1();
 buildTree();
}
//-------------------------------------------------------------------------

char *fReadStr(FILE *f)
{
 char buf[1024];
 if(fgets(buf,sizeof(buf),f)==0){
   buf[0]='\n';
   buf[1]=0;
 }
 size_t l= strlen(buf);
 char *s= new char[l];
 l--;
 memcpy(s,buf,l);
 s[l]=0;
 return s;
}

void modified()
{
 registerKeys();
 initList();
 setHook();
 modif=true;
}

//read config file
void rd(char *fn)
{
 FILE *f;
 HotKey *hk;
 int i,v,flags;
 char buf[16];

 if(!fn || !*fn) return;
 if((f=fopen(fn,"rt"))==0){
   msglng(730,"Cannot open file %s",fn);
 }else{
   fread(buf,strlen(magic),1,f);
   if(strncmp(buf,magic,strlen(magic))){
     msglng(731,"Invalid format of %s",fn);
     *iniFile=0;
   }else{
     fscanf(f,"%d ",&v);
     if(v<1 || v>version){
       msglng(732,"Invalid version of %s",fn);
       *iniFile=0;
     }else{
       EnterCriticalSection(&listCritSect);
       unregisterKeys();
       destroyAll();
       delete[] hotKeyA;
       fscanf(f,"%d", &numKeys);
       numCategories=1;
       if(v>3) fscanf(f," %d", &numCategories);
       fgetc(f);
       aminmax(numKeys,0,999);
       aminmax(numCategories,1,999);
       hotKeyA= new HotKey[numKeys];
       memset(hotKeyA,0,numKeys*sizeof(hotKeyA[0]));
       for(i=0; i<numKeys; i++){
         hk= &hotKeyA[i];
         hk->note= fReadStr(f);
         hk->exe= fReadStr(f);
         hk->args= fReadStr(f);
         hk->dir= fReadStr(f);
         // zef: added support to play sound file per command
         if (v>=5) hk->sound = fReadStr(f); else cpStr( hk->sound, "" );
         fscanf(f,"%d %d %d %d %d %d %d",
           &hk->scanCode,&hk->vkey,&hk->modifiers,
           &hk->cmdShow,&hk->priority,&flags,&hk->cmd);
         hk->multInst= flags&1;
         hk->autoStart= (flags>>1)&1;
         hk->trayMenu= (flags>>2)&1;
         hk->ask= (flags>>3)&1;
         if(v>1){
           fscanf(f," %d",&hk->opacity);
           if(v>3) fscanf(f," %d",&hk->category);
           if(v>2 && hk->vkey==vkLirc){
             fgetc(f);
             hk->lirc=fReadStr(f);
             ungetc('\n',f);
           }
         }
         fgetc(f);
         aminmax(hk->cmdShow,0,(int)sizeA(showCnst)-1);
         aminmax(hk->priority,0,Npriority-1);
         aminmax(hk->cmd,-1,sizeA(cmdNames)-1);
         aminmax(hk->category,0,numCategories-1);
       }
       delete[] categoryName;
       categoryName= new char*[numCategories];
       for(i=1; i<numCategories; i++){
         categoryName[i]= fReadStr(f);
       }
       modified();
       modif=false;
       LeaveCriticalSection(&listCritSect);
     }
   }
   fclose(f);
 }
}

//save config file
bool wr(char *fn)
{
 FILE *f;
 HotKey *hk;
 int i;

start:
 if((f=fopen(fn,"wt"))==0){
   msglng(733,"Cannot create file %s",fn);
 }else{
   fprintf(f,"%s%d %d %d\n", magic, version, numKeys, numCategories);
   for(i=0; i<numKeys; i++){
     hk= &hotKeyA[i];
     // zef: added support to play sound file per command
     fprintf(f,"%s\n%s\n%s\n%s\n%s\n%d %d %d %d %d %d %d %d %d",
       hk->note,hk->exe,hk->args,hk->dir,hk->sound ? hk->sound : "",
       hk->scanCode,hk->vkey,hk->modifiers,
       hk->cmdShow,hk->priority,
       (int)hk->multInst|(hk->autoStart<<1)|(hk->trayMenu<<2)|(hk->ask<<3),
       hk->cmd,hk->opacity,hk->category);
     if(hk->vkey==vkLirc){
       fprintf(f," %s",hk->lirc);
     }
     fputc('\n',f);
   }
   for(i=1; i<numCategories; i++){
     fprintf(f,"%s\n",categoryName[i]);
   }
   if(fclose(f)){
     if( msg1(MB_ICONEXCLAMATION|MB_RETRYCANCEL,
       lng(734,"Write error, file %s"), fn)
       ==IDRETRY) goto start;
   }
   modif=false;
   return true; //success
 }
 return false; //error
}

bool open(OPENFILENAME &ofn)
{
 for(;;){
   ofn.hwndOwner= hWin;
   ofn.Flags= OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
   if(GetOpenFileName(&ofn)) return true;
   if(CommDlgExtendedError()!=FNERR_INVALIDFILENAME
     || !ofn.lpstrFile[0]) return false; //cancel
   ofn.lpstrFile[0]=0;
 }
}

bool save1(OPENFILENAME &ofn, DWORD flags)
{
 for(;;){
   ofn.hwndOwner= IsWindowVisible(hWin) ? hWin : 0;
   ofn.Flags= flags|OFN_HIDEREADONLY;
   if(GetSaveFileName(&ofn)) return true;
   if(CommDlgExtendedError()!=FNERR_INVALIDFILENAME
     || !ofn.lpstrFile[0]) return false; //cancel
   ofn.lpstrFile[0]=0;
 }
}

bool save(OPENFILENAME &ofn)
{
  return save1(ofn,OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT);
}

//-------------------------------------------------------------------------
void delRun(HKEY root)
{
 HKEY key;
 if(RegOpenKeyEx(root,runkey,0,KEY_QUERY_VALUE|KEY_SET_VALUE,&key)==ERROR_SUCCESS){
   if(RegQueryValueEx(key,runName,0,0,0,0)==ERROR_SUCCESS){
     RegDeleteValue(key,runName);
   }
   RegCloseKey(key);
 }
}

void setRun()
{
 HKEY key;
 DWORD i,d;
 char buf1[MAX_PATH],buf2[MAX_PATH];

 if(!autoRun){
   delRun(HKEY_CURRENT_USER);
 }else{
   if(RegOpenKeyEx(HKEY_CURRENT_USER, runkey,0,KEY_QUERY_VALUE|KEY_SET_VALUE, &key)==ERROR_SUCCESS){
     i=GetModuleFileName(0,buf1,sizeof(buf1)-2);
     if(i){
       strcat(buf1," 0");
       d=sizeof(buf2);
       if(RegQueryValueEx(key,runName,0,0,(BYTE*)buf2,&d)!=ERROR_SUCCESS ||
         strcmp(buf1,buf2)){
         RegSetValueEx(key,runName, 0,REG_SZ, (BYTE*)buf1, i+3);
       }
     }
     RegCloseKey(key);
   }
 }
}

//delete settings from the registry
void deleteini()
{
 HKEY key;
 DWORD i;

 delreg=true;
 if(RegDeleteKey(HKEY_CURRENT_USER, subkey)==ERROR_SUCCESS){
  if(RegOpenKeyEx(HKEY_CURRENT_USER,
    "Software\\Petr Lastovicka",0,KEY_QUERY_VALUE|KEY_SET_VALUE,&key)==ERROR_SUCCESS){
   i=1;
   RegQueryInfoKey(key,0,0,0,&i,0,0,0,0,0,0,0);
   RegCloseKey(key);
   if(!i)
    RegDeleteKey(HKEY_CURRENT_USER, "Software\\Petr Lastovicka");
  }
 }
 delRun(HKEY_CURRENT_USER);
}

//save settings to the registry
void writeini()
{
 HKEY key;
 if(RegCreateKey(HKEY_CURRENT_USER, subkey, &key)!=ERROR_SUCCESS)
   msglng(735,"Cannot write to Windows registry");
 else{
  for(Treg *u=regVal; u<endA(regVal); u++){
    RegSetValueEx(key, u->s, 0,REG_DWORD,
      (BYTE *)u->i, sizeof(int));
  }
  for(Tregs *v=regValS; v<endA(regValS); v++){
    RegSetValueEx(key, v->s, 0,REG_SZ,
      (BYTE *)v->i, (int)strlen(v->i)+1);
  }
  for(Tregs2 *z=regValS2; z<endA(regValS2); z++){
    char *t= *z->i;
    if(t) RegSetValueEx(key, z->s, 0,REG_SZ,
      (BYTE *)t, (int)strlen(t)+1);
  }
  for(Tregb *w=regValB; w<endA(regValB); w++){
    RegSetValueEx(key, w->s, 0,REG_BINARY, (BYTE *)w->i, w->n);
  }
  RegCloseKey(key);
 }
}

//read settings from the registry
void readini()
{
 HKEY key;
 DWORD d;
 if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){
  for(Treg *u=regVal; u<endA(regVal); u++){
    d=sizeof(int);
    RegQueryValueEx(key,u->s,0,0, (BYTE *)u->i, &d);
  }
  for(Tregs *v=regValS; v<endA(regValS); v++){
    d=v->n;
    RegQueryValueEx(key,v->s,0,0, (BYTE *)v->i, &d);
  }
  for(Tregs2 *z=regValS2; z<endA(regValS2); z++){
    if(RegQueryValueEx(key,z->s,0,0,0,&d)==ERROR_SUCCESS && d<10000000){
      char *&s= *z->i;
      delete[] s;
      s= new char[d];
      s[0]=0;
      RegQueryValueEx(key,z->s,0,0,(BYTE*)s, &d);
    }
  }
  for(Tregb *w=regValB; w<endA(regValB); w++){
    d=w->n;
    RegQueryValueEx(key,w->s,0,0, (BYTE *)w->i, &d);
  }
  RegCloseKey(key);
 }
}

//save settings and config file
bool saveAtExit()
{
 if(modif && numKeys){
   if(!*iniFile) save(ofn);
   if(*iniFile && !wr(iniFile)) return false;
 }
 if(!delreg) writeini();
 return true;
}
//-------------------------------------------------------------------------
DWORD getVer()
{
 HRSRC r;
 HGLOBAL h;
 void *s;
 VS_FIXEDFILEINFO *v;
 UINT i;

 r=FindResource(0,(char*)VS_VERSION_INFO,RT_VERSION);
 h=LoadResource(0,r);
 s=LockResource(h);
 if(!s || !VerQueryValue(s,"\\",(void**)&v,&i)) return 0;
 return v->dwFileVersionMS;
}
  
//procedure for AboutBox
BOOL CALLBACK AboutProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM)
{
 char buf[48];
 DWORD d;
 
 switch(msg){
  case WM_INITDIALOG:
   setDlgTexts(hWnd,11);
   d=getVer();
   sprintf(buf,"%d.%d",HIWORD(d),LOWORD(d));
   SetDlgItemText(hWnd,101,buf);
   return TRUE;

  case WM_COMMAND:
   wP=LOWORD(wP);
   switch(wP){
    case IDOK:
    case IDCANCEL:
     EndDialog(hWnd, wP);
    return TRUE;
    case 123:
     GetDlgItemText(hWnd,wP,buf,static_cast<int>(sizeA(buf)-13));
     if (!strcmp(lang,"English")) strcat(buf,"/indexEN.html");
     ShellExecute(0,0,buf,0,0,SW_SHOWNORMAL);
     break;
   }
  break;
 }
 return 0;
}

//-------------------------------------------------------------------------
bool checkShifts(int m)
{
  return ((m&MOD_SHIFT)!=0)==(GetAsyncKeyState(VK_SHIFT)<0) &&
    ((m&MOD_CONTROL)!=0)==(GetAsyncKeyState(VK_CONTROL)<0) &&
    ((m&MOD_ALT)!=0)==(GetAsyncKeyState(VK_MENU)<0) &&
    ((m&MOD_WIN)!=0)==(GetAsyncKeyState(VK_LWIN)<0 || GetAsyncKeyState(VK_RWIN)<0);
}

bool shiftPressed()
{
 return GetKeyState(VK_SHIFT)<0 || GetKeyState(VK_MENU)<0 ||
   GetKeyState(VK_CONTROL)<0 || GetKeyState(VK_LWIN)<0 ||
   GetKeyState(VK_RWIN)<0;
}
//-------------------------------------------------------------------------
void acceptKey(UINT vkey, DWORD scan)
{
  char buf[64];

  dlgKey.vkey=vkey;
  dlgKey.scanCode=scan;
  printKey(buf, &dlgKey);
  SetDlgItemText(hHotKeyDlg,103,buf);
  //checkboxes ctrl,shift,alt,win
  bool b = (vkey!=vkLirc && vkey!=vkJoy);
  CheckDlgButton(hHotKeyDlg,106, b && GetKeyState(VK_CONTROL)<0 ? BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hHotKeyDlg,107, b && GetKeyState(VK_SHIFT)<0 ? BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hHotKeyDlg,108, b && GetKeyState(VK_MENU)<0 ? BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hHotKeyDlg,109, b && (GetKeyState(VK_LWIN)<0 || GetKeyState(VK_RWIN)<0)
    ? BST_CHECKED:BST_UNCHECKED);
}

void acceptKey(UINT vkey)
{
  acceptKey(vkey,1+(MapVirtualKey(vkey,0)<<16));
}
//-------------------------------------------------------------------------

//window procedure for hotkey edit control
LRESULT CALLBACK hotKeyClassProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 switch(mesg){
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
   if(!isShiftKey(wP)){
     if((wP==VK_BACK || wP==VK_DELETE) && dlgKey.vkey) wP=0,lP=0;
     //write key name to edit box
     acceptKey((UINT)wP,(DWORD)lP);
     return 0;
   }
  break;
  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
  case WM_MOUSEWHEEL:
  case WM_MOUSEHWHEEL:
   if(wP==MK_LBUTTON && !shiftPressed()) break;
   //write mouse buttons to edit box
   acceptKey(vkMouse,lastButtons);
   return 0;
  break;
  case WM_APPCOMMAND:
  case WM_CHAR:
   return 0; //don't add a char to edit box
  case WM_KILLFOCUS:
   if(GetKeyState(VK_TAB)<0 && (!dlgKey.vkey || shiftPressed())){
     acceptKey(VK_TAB);
     break;
   }
  break;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if(wP==VK_SNAPSHOT){
      acceptKey(wP,0x1370001);
      break;
    }
    if(wP==VK_ESCAPE || wP==VK_CANCEL){
      acceptKey((UINT)wP,(DWORD)lP);
      return 0;
    }
  break;
 }
 return CallWindowProc((WNDPROC)editWndProc, hWnd, mesg, wP, lP);
}
//-------------------------------------------------------------------------
int findCmd(const char *s)
{
 for(int i=0; i<sizeA(cmdNames); i++){
   if(!_stricmp(s,getCmdName(i))){
     return i;
   }
 }
 return -1;
}
//-------------------------------------------------------------------------
static char s[3][Dpasswd];
static int len[3];

BOOL CALLBACK passwdClassProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 char buf[Dpasswd+1];
 int i,j;
 
 i=GetDlgCtrlID(hWnd)-170;
 int &len=::len[i];
 switch(mesg){
 case WM_KEYDOWN:
   if(wP==VK_BACK){
     if(len>0) len--;
   }else if(wP!=VK_MENU && len<Dpasswd){
     s[i][len++]=(char)wP;
   }
   for(j=0; j<Dpasswd; j++) buf[j]='*';
   buf[len]=0;
   SetWindowText(hWnd,buf);
   SendMessage(hWnd,EM_SETSEL,len,len);
   break;
 case WM_CHAR:
   break;
 case WM_SETFOCUS:
   len=0;
   SetWindowText(hWnd,"");
 default:
   return CallWindowProc((WNDPROC)editWndProc, hWnd, mesg, wP, lP);
 }
 return TRUE;
}

BOOL CALLBACK passwdProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM)
{
 int i;
 HWND w[3];
 BYTE p[Dpasswd];

 for(i=0; i<3; i++){
   w[i]=GetDlgItem(hWnd,170+i);
 }
 switch(mesg){
  case WM_INITDIALOG:
   setDlgTexts(hWnd,1063);
   for(i=0; i<3; i++){
     len[i]=0;
     editWndProc=(WNDPROC)SetWindowLongPtr(w[i],
       GWLP_WNDPROC, (LONG_PTR) passwdClassProc);
   }
   SetFocus(isEmptyPassword() ? w[1] : w[0]);
   return FALSE;                      
  case WM_COMMAND:
   wP=LOWORD(wP);
   switch(wP){
   case IDOK:
     if(len[0] || len[1]){
       encrypt(p,Dpasswd,s[0],len[0],passwdAlg);
       if(memcmp(p,password,Dpasswd) && !isEmptyPassword()){
         SetFocus(w[0]);
         break;
       }
       if(len[1]!=len[2] || memcmp(s[1],s[2],len[2])){
         SetFocus(w[1]);
         break;
       }
       passwdAlg= encrypt(password,Dpasswd,s[1],len[1],-1);
     }
   case IDCANCEL:
     EndDialog(hWnd,wP);
   }
  break;
  default:
   return FALSE;
 }
 return TRUE;
}
//-------------------------------------------------------------------------
void checkDlgButton(HWND w, int id, bool c)
{
 CheckDlgButton(w, id, c ? BST_CHECKED:BST_UNCHECKED);
}

//hotkey properties dialog procedure
BOOL CALLBACK hotkeyProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 HotKey *hk;
 char buf[256];
 static char *cmdBuf;
 int i;
 DWORD a;
 HWND combo,edit;
 char *filePart;
 char const * docPart;

 switch(mesg){
  case WM_INITDIALOG:
   setDlgTexts(hWnd,12);
   hHotKeyDlg=hWnd;
   SetWindowLongPtr(hWnd, GWLP_USERDATA, lP);
   hk= (HotKey*) lP;
   SetDlgItemText(hWnd,101, (hk->cmd>=0)? getCmdName(hk->cmd):hk->exe);
   SetDlgItemText(hWnd,102, hk->args);
   SetDlgItemText(hWnd,110, hk->dir);
   SetDlgItemText(hWnd,IDC_EDIT_SOUND, hk->sound); // zef: added sound
   SetDlgItemText(hWnd,112, hk->note);
   checkDlgButton(hWnd,309, hk->multInst);
   checkDlgButton(hWnd,311, hk->trayMenu);
   checkDlgButton(hWnd,312, hk->autoStart);
   checkDlgButton(hWnd,313, hk->ask);
   if(hk->opacity) SetDlgItemInt(hWnd,114, hk->opacity, FALSE);
   dlgKey.scanCode=hk->scanCode;
   dlgKey.vkey=hk->vkey;
   if(hk->vkey==vkLirc) cpStr(dlgKey.lirc,hk->lirc);
   printKey(buf,&dlgKey);
   SetDlgItemText(hWnd,103, buf);
   CheckDlgButton(hWnd,106, hk->modifiers&MOD_CONTROL ? BST_CHECKED:BST_UNCHECKED);
   CheckDlgButton(hWnd,107, hk->modifiers&MOD_SHIFT ? BST_CHECKED:BST_UNCHECKED);
   CheckDlgButton(hWnd,108, hk->modifiers&MOD_ALT ? BST_CHECKED:BST_UNCHECKED);
   CheckDlgButton(hWnd,109, hk->modifiers&MOD_WIN ? BST_CHECKED:BST_UNCHECKED);

   combo= GetDlgItem(hWnd,105);
   for(i=0; i<Npriority; i++){
     SendMessage(combo,CB_ADDSTRING,0,(LPARAM)lng(700+i,priorTab[i]));
   }
   SendMessage(combo,CB_SETCURSEL,hk->priority,0);
   combo= GetDlgItem(hWnd,104);
   for(i=0; i<sizeA(showTab); i++){
     SendMessage(combo,CB_ADDSTRING,0,(LPARAM)lng(710+i,showTab[i]));
   }
   SendMessage(combo,CB_SETCURSEL,hk->cmdShow,0);
   fillCategories(hWnd,hk->category);

   editWndProc = (WNDPROC) SetWindowLongPtr(GetDlgItem(hWnd,103),
                GWLP_WNDPROC, (LONG_PTR) hotKeyClassProc);
   DragAcceptFiles(hWnd,TRUE);
   if(renaming){
     edit=GetDlgItem(hWnd,112);
     SetFocus(edit);
     SendMessage(edit,EM_SETSEL,0,-1);
     return FALSE;
   }
  return TRUE;

  case WM_DROPFILES:
   DragQueryFile((HDROP)wP, 0, exeBuf, sizeof(exeBuf));
   DragFinish((HDROP)wP);
   SetDlgItemText(hWnd,101,exeBuf);
   SetForegroundWindow(hWnd);
  break;

  case WM_COMMAND:
   edit= GetDlgItem(hWnd,103);
   hk= (HotKey*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
   wP=LOWORD(wP);
   switch(wP){
    default:
     if(wP>=1000 && wP<1000+sizeA(cmdNames)){
       SetDlgItemText(hWnd,101,getCmdName(static_cast<int>(wP-1000)));
       SetFocus(GetDlgItem(hWnd,102));
     }
    break;
    case 111:  //button at "Command:" edit box
     exeOfn.hwndOwner= hWnd;
     exeOfn.Flags= OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
     *exeBuf=0;
     if(GetOpenFileName(&exeOfn)){
       cpStr(hk->exe,exeBuf);
       SetDlgItemText(hWnd,101,hk->exe);
       SetFocus(GetDlgItem(hWnd,102));
     }
    break;
    // zef: added sound support
    case IDC_BUTTON_SOUND:
     wavOfn.hwndOwner= hWnd;
     wavOfn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
     *wavFile = 0;
     if (GetOpenFileName(&wavOfn)){
       cpStr(hk->sound, wavFile);
       SetDlgItemText(hWnd,IDC_EDIT_SOUND,hk->sound);
       SetFocus(GetDlgItem(hWnd,IDC_EDIT_SOUND));
     }
    break;
    case 115: //button at "Parameters:" edit box
     argOfn.hwndOwner= hWnd;
     argOfn.Flags= OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
     *exeBuf=0;
     if(GetOpenFileName(&argOfn)){
       cpStrQuot(hk->args,exeBuf);
       SetDlgItemText(hWnd,102,hk->args);
       SetFocus(GetDlgItem(hWnd,102));
     }
    break;
    case 116:{ //button at "Command:" edit box
     BROWSEINFO bi;
     bi.hwndOwner= hWnd;
     bi.pidlRoot=0;
     bi.pszDisplayName= exeBuf;
     bi.lpszTitle= lng(736,"Select working directory");
     bi.ulFlags=BIF_RETURNONLYFSDIRS;
     bi.lpfn=0;
     bi.iImage=0;
     ITEMIDLIST *iil;
     iil=SHBrowseForFolder(&bi);
     if(iil){
       if(SHGetPathFromIDList(iil,exeBuf)){
         cpStr(hk->dir,exeBuf);
         SetDlgItemText(hWnd,110,hk->dir);
         SetFocus(GetDlgItem(hWnd,112));
       }
       IMalloc *ma;
       SHGetMalloc(&ma);
       ma->Free(iil);
       ma->Release();
     }
    }
    break;
    case 117: //internal command
    {
     RECT rc;
     GetWindowRect(GetDlgItem(hWnd,117),&rc);
     showPopup(rc.left, rc.bottom, "CMDPOPUP", popupSubId, hHotKeyDlg);
    }
    break;
    case 7: //help
      getText(hWnd,101,cmdBuf);
      showHelp(findCmd(cmdBuf));
    break;

    case IDOK:{
     if(GetFocus()==edit && shiftPressed()){
       if(hookK && (keyLastScan&0xff0000)==0x1c0000)
         acceptKey(VK_RETURN, keyLastScan);
       else
         acceptKey(VK_RETURN);
       break;
     }
     EnterCriticalSection(&listCritSect);
     getText(hWnd,112,hk->note);
     getText(hWnd,110,hk->dir);
     getText(hWnd,101,hk->exe);
     getText(hWnd,102,hk->args);
     getText(hWnd,IDC_EDIT_SOUND,hk->sound);  // zef: added sound support
     hk->icon=0; //icon will be found in LVN_GETDISPINFO
     hk->priority= (int) SendMessage(GetDlgItem(hWnd,105),CB_GETCURSEL,0,0);
     hk->cmdShow= (int) SendMessage(GetDlgItem(hWnd,104),CB_GETCURSEL,0,0);
     hk->multInst= IsDlgButtonChecked(hWnd,309)==BST_CHECKED;
     hk->trayMenu= IsDlgButtonChecked(hWnd,311)==BST_CHECKED;
     hk->autoStart= IsDlgButtonChecked(hWnd,312)==BST_CHECKED;
     hk->ask= IsDlgButtonChecked(hWnd,313)==BST_CHECKED;
     hk->opacity= GetDlgItemInt(hWnd,114,0,FALSE);
     hk->cmd=findCmd(hk->exe);
     GetDlgItemText(hWnd,118,exeBuf,sizeof(exeBuf));
     hk->category= findCategory(exeBuf);
     LeaveCriticalSection(&listCritSect);
     if(hk->cmd>=0){
       //internal command
       cpStr(hk->exe,"");
     }else{
       if(!*hk->exe){
         msglng(737,"Command item is empty");
         SetFocus(GetDlgItem(hWnd,101));
         break;
       }
       if(isWWW(hk->exe)){
         if(!*hk->note){
           cpStr(hk->note, hk->exe);
         }
       }else{
         hk->resolveLNK();
         //remove quotation marks
         cutQuot(hk->exe);
         cutQuot(hk->dir);
         /*
         //find full path
         if(SearchPath(0,hk->exe,".exe",sizeof(exeBuf),exeBuf,&filePart)
           || SearchPath(0,hk->exe,".com",sizeof(exeBuf),exeBuf,&filePart)
           || SearchPath(0,hk->exe,".bat",sizeof(exeBuf),exeBuf,&filePart)){
           cpStr(hk->exe,exeBuf);
         } */
         //append extension
         if(!SearchPath(0,hk->exe,0,sizeof(exeBuf),exeBuf,&filePart)){
           static char *E[]={".exe",".com",".bat"};
           for(int j=0; j<3; j++){
             if(SearchPath(0,hk->exe,E[j],sizeof(exeBuf)-4,exeBuf,&filePart)){
               strcpy(exeBuf,hk->exe);
               strcat(exeBuf,E[j]);
               cpStr(hk->exe,exeBuf);
               break;
             }
           }
         }
         //find associated application
         // zef: added support for environment vars in commands
         std::string fullExe = hk->getFullExe();
         a=GetFileAttributes(fullExe.c_str());
         HINSTANCE result=FindExecutable(fullExe.c_str(),hk->dir,exeBuf);
         if(result>(HINSTANCE)32 || 
           (a&FILE_ATTRIBUTE_DIRECTORY) && a!=0xFFFFFFFF){
           docPart=cutPath(hk->exe);
           if(!*hk->note){
             if(isExe(docPart)){
               //cut off extension
               // zef: re-wrote to be const correct
               std::string docPartCpy( docPart,  strlen(docPart)-4 );
               cpStr(hk->note, docPartCpy.c_str() );
             }else{
               cpStr(hk->note, docPart);
             }
           }
           /*
           filePart=cutPath(exeBuf);
           if(!*hk->dir && docPart>hk->exe){
             *(docPart-1)=0;
             cpStr(hk->dir,hk->exe);
             *(docPart-1)='\\';
             SetDlgItemText(hWnd,110, hk->dir);
           }
           if(!*hk->dir && filePart>exeBuf){
             *(filePart-1)=0;
             cpStr(hk->dir,exeBuf);
             *(filePart-1)='\\';
             SetDlgItemText(hWnd,110, hk->dir);
           }
           */
         }else{
           if(result==(HINSTANCE)31){
             msglng(738,"There is no association for the specified file type");
           }else{
             msglng(739,"%s not found",hk->exe);
           }
           SetFocus(GetDlgItem(hWnd,101));
           break;
         }
         if(testDir(hk->dir)){
           msglng(740,"Invalid working directory");
           SetFocus(GetDlgItem(hWnd,110));
           break;
         }
       }
     }
     EnterCriticalSection(&listCritSect);
     hk->scanCode= dlgKey.scanCode;
     hk->vkey= dlgKey.vkey;
     if(dlgKey.vkey==vkLirc) cpStr(hk->lirc,dlgKey.lirc);
     hk->modifiers=0;
     if(hk->vkey!=vkLirc && hk->vkey!=vkJoy){
       if(IsDlgButtonChecked(hWnd,106)==BST_CHECKED) hk->modifiers|=MOD_CONTROL;
       if(IsDlgButtonChecked(hWnd,107)==BST_CHECKED) hk->modifiers|=MOD_SHIFT;
       if(IsDlgButtonChecked(hWnd,108)==BST_CHECKED) hk->modifiers|=MOD_ALT;
       if(IsDlgButtonChecked(hWnd,109)==BST_CHECKED) hk->modifiers|=MOD_WIN;
     }
     if(hk->modifiers==MOD_WIN && hk->vkey<255 && specialWinKeys[hk->vkey]){
       hk->vkey=255;
     }
     LeaveCriticalSection(&listCritSect);
     if(hk->cmd>=0 || !isExe(hk->exe)){
       if(hk->priority!=1){
         hk->priority=1;
         msglng(741,"Process priority can be specified only for executables");
       }
       if(hk->opacity){
         hk->opacity=0;
         msglng(761,"Opacity can be specified only for executables");
       }
     }
     if(hk->cmd==63 && *hk->args && isEmptyPassword()){
       DialogBoxParam(inst,"PASSWD",hWin,(DLGPROC)passwdProc,0);
     }
     EndDialog(hWnd, wP);
     break;
    }
    case IDCANCEL:
     if(GetFocus()==edit && shiftPressed()) break;
     EndDialog(hWnd, wP);
    break;
   }
  break;
  default:
   return FALSE;
 }
 return TRUE;
}
//-------------------------------------------------------------------------
void swapKey(int index1, int index2)
{
 int item = getItemFromIndex(index1);
 int item2 = getItemFromIndex(index2);
 correctMultiCmd(item,0,item2);
 HotKey k=hotKeyA[item];
 hotKeyA[item]=hotKeyA[item2];
 hotKeyA[item2]=k;
}

void moveItem(int i, int &dist)
{
  if(getSel(i)){
    int j=i+dist;
    if(j<0){ j=0; dist=-i; }
    if(j>=numList){ j=numList-1; dist=j-i; }
    swapKey(i,j);
    setSel(i,false);
    setSel(j,true);
  }
}

void moveSelected(int dist)
{
  int i;
  
  if(sortedCol!=0){
    msglng(762,"Moving is possible only if the list is sorted by the first column.");
    return;
  }
  EnterCriticalSection(&listCritSect);
  unregisterKeys();
  if(dist<0){
    for(i=0; i<numList; i++){
      moveItem(i,dist);
    }
  }else{
    for(i=numList-1; i>=0; i--){
      moveItem(i,dist);
    }
  }
  registerKeys();
  modif=true;
  LeaveCriticalSection(&listCritSect);
}

static int dragStart = -1;

void itemDrag(LPARAM coord)
{
  LVHITTESTINFO hi;
  hi.pt.x=LOWORD(coord);
  hi.pt.y=HIWORD(coord);
  int i= ListView_HitTest(listBox,&hi); 
  if(i<0) return;
  if(i!=dragStart){
    moveSelected(i-dragStart);
    dragStart=i;
  }
}


int editKey1(HotKey *hk)
{
 editing=true;
 setHook();
 int result= (int) DialogBoxParam(inst, "HOTKEYWIN",
   hWin, (DLGPROC) hotkeyProc, (LPARAM)hk);
 editing=false;
 hHotKeyDlg=0;
 setHook();
 return result;
}

//edit hotkey
void editKey(int item)
{
 registerHK(item,true);
 if(editKey1(&hotKeyA[item])==IDOK){
   modified();
   setCur(item);
 }else{
   registerKeys();
 }
}

//remove hotkey
void delKey(int item)
{
 correctMultiCmd(item,1);
 HotKey *hk= &hotKeyA[item];
 hk->destroy();
 --numKeys;
 memcpy(hotKeyA+item, hotKeyA+item+1, (numKeys-item)*sizeof(HotKey));
}

//create new hotkey
void addKey(char *exe, bool copy, int item)
{
 if(item<0) item=0;
 //default attributes
 HotKey tmpHk, *hk=&tmpHk;
 memset(hk,0,sizeof(HotKey));
 hk->priority= 1;
 hk->cmd=-1;
 //copy hotkey
 if(copy){
   HotKey *copy= &hotKeyA[item-1];
   cpStr(hk->note,copy->note);
   cpStr(hk->exe,copy->exe);
   cpStr(hk->args,copy->args);
   cpStr(hk->dir,copy->dir);
   cpStr(hk->sound,copy->sound); // zef: added sound support
   hk->cmdShow=copy->cmdShow;
   hk->priority=copy->priority;
   hk->multInst=copy->multInst;
   hk->cmd=copy->cmd;
   hk->opacity=copy->opacity;
   hk->category=copy->category;
 }
 //exe file name from drag&drop
 if(exe){
   cpStr(hk->exe,exe);
   hk->resolveLNK();
 }
 //show dialog box
 if(editKey1(hk)==IDOK){
   EnterCriticalSection(&listCritSect);
   //insert new hotkey to the array
   unregisterKeys();
   correctMultiCmd(item,2);
   HotKey *newA= new HotKey[++numKeys];
   memcpy(newA, hotKeyA, sizeof(HotKey)*item);
   memcpy(newA+item, hk, sizeof(HotKey));
   memcpy(newA+item+1, hotKeyA+item, sizeof(HotKey)*(numKeys-1-item));
   delete[] hotKeyA;
   hotKeyA= newA;
   //redraw listview
   modified();
   LeaveCriticalSection(&listCritSect);
   setCur(item);
 }else{
   hk->destroy();
 }
}

bool hideMainWindow()
{
 if(!IsZoomed(hWin) && !IsIconic(hWin)){
   //remember window position and size
   RECT rc;
   GetWindowRect(hWin,&rc);
   dlgX=rc.left;
   dlgY=rc.top;
   dlgW=rc.right-rc.left;
   dlgH=rc.bottom-rc.top;
 }
 //remember columns width
 for(int i=0; i<sizeA(colWidth); i++){
   colWidth[i]= ListView_GetColumnWidth(listBox,i);
 }
 if(!saveAtExit()) return false;
 ShowWindow(hWin,SW_HIDE);
 return true;
}

void colorChanged()
{
  for(int i=0; i<Npopup; i++){
    InvalidateRect(popup[i].hWnd,0,0);
  }
}

//---------------------------------------------------------------------------

void mainButtonChanged()
{
 int i,y;
 HWND w;
 RECT rc,rc2;
 rc.left=rc.right=0;

 y=0;
 for(i=0; i<sizeA(mainButton); i++){
   TmainButton *b = &mainButton[i];
   w= GetDlgItem(hWin,b->cmd);
   GetWindowRect(w,&rc);
   MapWindowPoints(0,hWin,(POINT*)&rc,2);
   if(mainBtnEnabled[i]){
     ShowWindow(w,SW_SHOW);
     SetWindowPos(w,0,rc.left,y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
     y+=rc.bottom-rc.top+10;
   }else{
     ShowWindow(w,SW_HIDE);
   }
 }
 y+=5;
 SetWindowPos(GetDlgItem(hWin,105),0, rc.left+10,y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
 GetClientRect(w=GetDlgItem(hWin,106),&rc2);
 SetWindowPos(w,0, rc.right-rc2.right+rc2.left-10,y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

static int oldTrayIcon=-1,oldDiskPrec=-1,oldPriority=-1,oldPort=-1,oldLirc=-1,oldHook,oldKeepHook=-1,oldHookInterval=-1,oldJoyMouse=-1;
static Tpopup oldPopup[Npopup];
static COLORREF clold[Ncl];
static char oldAddress[64];
static TfileName oldLircExe;

void oldOptions()
{
 oldTrayIcon=trayicon;
 oldDiskPrec=diskfreePrec;
 oldPriority=highPriority;
 oldLirc=lircEnabled;
 oldPort=lircPort;
 oldHook=useHook;
 oldKeepHook=keepHook;
 oldHookInterval=keepHookInterval;
 oldJoyMouse=joyMouseEnabled;
 strcpy(oldAddress,lircAddress);
 strcpy(oldLircExe,lircExe);
 memcpy(oldPopup,popup,sizeof(oldPopup));
 memcpy(clold, colors, sizeof(clold));
}

void optionChanged()
{
 if(trayicon!=oldTrayIcon){
   if(trayicon) addTrayIcon();
   else deleteTrayIcon();
 }
 if(diskfreePrec!=oldDiskPrec){
   resetDiskfree();
 }
 if(highPriority!=oldPriority){
   SetPriorityClass(GetCurrentProcess(),highPriority ? HIGH_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS);
 }
 for(int i=0; i<Npopup; i++){
   Tpopup *p= &popup[i];
   if(oldPopup[i].opacity!=p->opacity){
     setOpacity(p->hWnd,p->opacity);
   }
   if(oldPopup[i].width!=p->width){
     SetWindowPos(p->hWnd,0,0,0,p->width,p->height,SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
   }
 }
 if(useHook!=oldHook){
   int o=useHook;
   useHook=oldHook;
   unregisterKeys();
   useHook=o;
   setHook();
   registerKeys();
 }
 if(keepHook!=oldKeepHook || keepHookInterval!=oldHookInterval || joyMouseEnabled!=oldJoyMouse){
   setHook();
 }
 setRun();
 keyMapChanged();
 mainButtonChanged();
 InvalidateRect(listBox,0,TRUE);
 if(oldPort!=lircPort || strcmp(oldAddress,lircAddress) || 
   oldLirc!=lircEnabled || strcmp(oldLircExe,lircExe)) lircEnd(true);
 lircStart();
 aminmax(joyThreshold,0,1000);
 InvalidateRect(popupVolume.hWnd,0,TRUE);

 for(TendpointName *en=endpointNameList; en; ){
   TendpointName *en1=en->nxt;
   delete en;
   en=en1;
 }
 endpointNameList=0;
}

int getRadioButton(HWND hWnd, int item1, int item2)
{
  for(int i=item1; i<=item2; i++){
    if(IsDlgButtonChecked(hWnd,i)){
      return i-item1;
    }
  }
  return 0;
}

struct TintValue{ int *value; WORD id; short dlgId; };
struct TboolValue{ int *value; WORD id; short dlgId; };
struct TstrValue{ char *value; int len; WORD id; short dlgId; };
struct TgroupValue{ int *value; WORD first; WORD last; short dlgId; };
struct TcomboValue{ int *value; WORD id; WORD lngId; char **strA; WORD len; short dlgId; };

static TintValue intOpts[]={
  {&iconDelay,101,0},
  {&mouseDelay,120,2},
  {&popupVolume.width,102,1},
  {&popupVolume.delay,103,1},
  {&diskfreePrec,104,1},
  {&popupDiskFree.width,105,1},
  {&popupVolume.opacity,180,1},
  {&popupDiskFree.opacity,181,1},
  {&popupShowText.opacity,182,1},
  {&lockSpeed,146,1},
  {&lircPort,133,2},
  {&lircRepeat,134,2},
  {&keepHookInterval,145,0},
  {&joyThreshold,167,3},
  {&joyMultiplier,168,3},
  {&joyMouseJoy,187,3},
};
static TboolValue boolOpts[]={
  {&insertAfterCurrent,342,0},
  {&trayicon,320,0},
  {&autoRun,331,0},
  {&minToTray,334,0},
  {&closeToTray,335,0},
  {&lockMute,355,1},
  {&highPriority,357,0},
  {&mainBtnEnabled[0],mainButton[0].langId,0},
  {&mainBtnEnabled[1],mainButton[1].langId,0},
  {&mainBtnEnabled[2],mainButton[2].langId,0},
  {&mainBtnEnabled[3],mainButton[3].langId,0},
  {&mainBtnEnabled[4],mainButton[4].langId,0},
  {&mainBtnEnabled[5],mainButton[5].langId,0},
  {&mainBtnEnabled[6],mainButton[6].langId,0},
  {&lircEnabled,360,2},
  {&notDelayButtons[M_Left],378,2},
  {&notDelayButtons[M_Middle],379,2},
  {&notDelayButtons[M_Right],380,2},
  {&notDelayButtons[M_X1],381,2},
  {&notDelayButtons[M_X1+1],382,2},
  {&notDelayFullscreen,349,2},
  {&keepHook,386,0},
  {&joyNotFullscreen,390,3},
  {&joyMouseEnabled,391,3},
  {&hidePasswd,395,1},
};
static TstrValue strOpts[]={
  {volumeStr,sizeA(volumeStr),151,1},
  {keyMap,sizeA(keyMap),152,0},
  {lockBMP,sizeA(lockBMP),156,1},
  {lircExe,sizeA(lircExe),131,2},
  {lircAddress,sizeA(lircAddress),132,2},
  {notDelayApp,sizeA(notDelayApp),174,2},
  {delayApp,sizeA(delayApp),175,2},
  {joyApp,sizeA(joyApp),194,3},
  {joyFullscreenApp,sizeA(joyFullscreenApp),195,3},
};

static TgroupValue groupOpts[]={
  {&isLockColor,350,351,1},
  {&useHook,396,399,0},
};
static TcomboValue comboOpts[]={
  {0,0,0,0,0,-1},
};

//procedure for options dialog
BOOL propPageInit(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP, int curPage)
{
  int j,cmd;
  HWND sheet,combo;
  TintValue *ti;
  TboolValue *tb;
  TstrValue *ts;
  TgroupValue *tg;
  TcomboValue *tc;
  DRAWITEMSTRUCT *di;
  HBRUSH br;

  switch(mesg){
  case WM_INITDIALOG:
    oldOptions();
    setDlgTexts(hWnd);
    sheet=GetParent(hWnd);
    SetWindowLong(sheet,GWL_EXSTYLE,GetWindowLong(sheet,GWL_EXSTYLE)&~WS_EX_CONTEXTHELP);
    for(ti=intOpts; ti<endA(intOpts); ti++){
      if(ti->dlgId==curPage) SetDlgItemInt(hWnd, ti->id, *ti->value, FALSE);
    }
    for(tb=boolOpts; tb<endA(boolOpts); tb++){
      if(tb->dlgId==curPage) CheckDlgButton(hWnd, tb->id, *tb->value ? BST_CHECKED:BST_UNCHECKED);
    }
    for(ts=strOpts; ts<endA(strOpts); ts++){
      if(ts->dlgId==curPage) SetDlgItemText(hWnd, ts->id, ts->value);
    }
    for(tg=groupOpts; tg<endA(groupOpts); tg++){
      if(tg->dlgId==curPage) CheckRadioButton(hWnd,tg->first, tg->last, *tg->value+tg->first);
    }
    for(tc=comboOpts; tc<endA(comboOpts); tc++){
      if(tc->dlgId==curPage){
        combo= GetDlgItem(hWnd,tc->id);
        for(j=0; j<tc->len; j++){
          SendMessage(combo,CB_ADDSTRING,0,
            (LPARAM)lng(tc->lngId+j,tc->strA[j]));
        }
        SendMessage(combo,CB_SETCURSEL,*tc->value,0);
      }
    }
    return TRUE;
  case WM_COMMAND:
    if(HIWORD(wP)==EN_CHANGE || HIWORD(wP)==BN_CLICKED || HIWORD(wP)==CBN_SELCHANGE){
      PropSheet_Changed(GetParent(hWnd),hWnd);
    }
    cmd=LOWORD(wP);
    if(cmd>=200 && cmd<200+Ncl){ //color square
      static CHOOSECOLOR chc;
      chc.lStructSize= sizeof(CHOOSECOLOR);
      chc.hwndOwner= hWnd;
      chc.hInstance= 0;
      chc.rgbResult= colors[cmd-200];
      chc.lpCustColors= custom;
      chc.Flags= CC_RGBINIT|CC_FULLOPEN;
      if(ChooseColor(&chc)){
        colors[cmd-200]=chc.rgbResult;
        InvalidateRect(GetDlgItem(hWnd,cmd),0, TRUE);
        colorChanged();
      }
    }
    if(cmd==373){
      DialogBoxParam(inst,"PASSWD",hWin,(DLGPROC)passwdProc,0);
    }
    break;
  case WM_NOTIFY:
   switch(((NMHDR*)lP)->code){
    case PSN_APPLY:
      for(ti=intOpts; ti<endA(intOpts); ti++){
        if(ti->dlgId==curPage) *ti->value= GetDlgItemInt(hWnd,ti->id, NULL, FALSE);
      }
      for(tb=boolOpts; tb<endA(boolOpts); tb++){
        if(tb->dlgId==curPage) *tb->value= IsDlgButtonChecked(hWnd,tb->id);
      }
      for(ts=strOpts; ts<endA(strOpts); ts++){
        if(ts->dlgId==curPage) GetDlgItemText(hWnd, ts->id, ts->value, ts->len);
      }
      for(tg=groupOpts; tg<endA(groupOpts); tg++){
        if(tg->dlgId==curPage) *tg->value= getRadioButton(hWnd,tg->first, tg->last);
      }
      for(tc=comboOpts; tc<endA(comboOpts); tc++){
        if(tc->dlgId==curPage) *tc->value= (int) SendMessage(GetDlgItem(hWnd,tc->id),CB_GETCURSEL,0,0);
      }
      optionChanged();
      oldOptions();
      SetWindowLongPtr(hWnd,DWLP_MSGRESULT,FALSE);
      return TRUE;
    case PSN_RESET:
      memcpy(colors, clold, sizeof(clold));
      colorChanged();
      break;
    case PSN_SETACTIVE:
      optionsPage=curPage;
      break;
   }
  break;
  case WM_DRAWITEM:
    di = (DRAWITEMSTRUCT*) lP;
    br= CreateSolidBrush(colors[di->CtlID-200]);
    FillRect(di->hDC, &di->rcItem, br);
    DeleteObject(br);
  break;
  } 
  return FALSE;
}

BOOL CALLBACK optionsGeneralProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 return propPageInit(hWnd,mesg,wP,lP,0);
}

BOOL CALLBACK optionsCmdProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 return propPageInit(hWnd,mesg,wP,lP,1);
}

BOOL CALLBACK optionsMouseLirc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 if(mesg==WM_INITDIALOG){
   hWndLircState= GetDlgItem(hWnd,135);
   lircNotify();
 }
 if(mesg==WM_COMMAND && wP==136){ //WinLIRC exe
   exeOfn.hwndOwner= hWnd;
   exeOfn.Flags= OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
   *exeBuf=0;
   if(GetOpenFileName(&exeOfn)){
     SetDlgItemText(hWnd,131,exeBuf);
   }
 }
 return propPageInit(hWnd,mesg,wP,lP,2);
}

BOOL CALLBACK optionsJoystick(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
  char c[2];
  if(mesg==WM_INITDIALOG){
    c[1]=0;
    c[0]=axisInd2Name(joyMouseX);
    SetDlgItemText(hWnd,185,c);
    c[0]=axisInd2Name(joyMouseY);
    SetDlgItemText(hWnd,186,c);
  }
  if(mesg==WM_NOTIFY){
   switch(((NMHDR*)lP)->code){
    case PSN_APPLY:
      GetDlgItemText(hWnd,185,c,2);
      joyMouseX = axisName2Ind(c[0]);
      GetDlgItemText(hWnd,186,c,2);
      joyMouseY = axisName2Ind(c[0]);
      break;
   }
  }
 return propPageInit(hWnd,mesg,wP,lP,3);
}

void options()
{
  int i;
  static DLGPROC P[]={
    (DLGPROC)optionsGeneralProc,(DLGPROC)optionsCmdProc,
    (DLGPROC)optionsMouseLirc ,(DLGPROC)optionsJoystick};
  static char *T[]={"General","Commands","Mouse/Remote","Joystick"};
  PROPSHEETPAGE psp[sizeA(P)],*p;
  PROPSHEETHEADER psh;

  for(i=0; i<sizeA(psp); i++){
    p=&psp[i];
    p->dwSize = sizeof(PROPSHEETPAGE);
    p->dwFlags = PSP_USETITLE;
    p->hInstance = inst;
    p->pszTemplate = MAKEINTRESOURCE(110+i);
    p->pfnDlgProc = P[i];
    p->pszTitle = lng(15+i,T[i]);
  }
  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_PROPSHEETPAGE;
  psh.hwndParent = hWin;
  psh.hInstance = inst;
  psh.pszCaption = lng(327,"Options");
  psh.nPages = sizeA(psp);
  psh.nStartPage = optionsPage;
  psh.ppsp = psp;
  PropertySheet(&psh);
  
  hWndLircState=0;
}

//-------------------------------------------------------------------------
char const *HotKey::getNote() const
{
 if(!note) return "";
 if(!*note && cmd>=0) return getCmdName(cmd);
 return note;
}

void search(UINT vkey)
{
 DWORD time= GetTickCount();
 if(time-searchLastKey>1500) searchLen=0;
 searchLastKey=time;
 //convert virtual key code to character
 BYTE keystate[256];
 GetKeyboardState(keystate);
 char ch[2];
 if(ToAscii(vkey,0,keystate,(LPWORD)&ch,0)!=1) return;
 //append character to the search string
 if(searchLen==sizeof(searchBuf)) return;
 searchBuf[searchLen++]=ch[0];
 //find item which begins with the string
 for(int i=0; i<numKeys; i++){
   if(!_strnicmp(hotKeyA[i].getNote(), searchBuf, searchLen)){
     setCur(i);
     break;
   }
 }
}

void modifyTrayIcon()
{
  static const int I[2][2]={{1,6},{7,8}};
  if(trayicon){
    modifyTrayIcon(hWin, 
      disableAll ? 5 : (isHilited ? 2 : I[disableMouse][disableKeys]));
  }
}

void hiliteIcon()
{
  if(trayicon && iconDelay && !isHilited){
    isHilited=true;
    modifyTrayIcon();
    SetTimer(hWin, 6, iconDelay, 0);
  }
}

//------------------------------------------------------------------
void setFont()
{
  DeleteObject(hFont);
  hFont=CreateFontIndirect(&font);
  SendMessage(listBox,WM_SETFONT,(WPARAM)hFont,(LPARAM)MAKELPARAM(TRUE,0));
}

UINT_PTR APIENTRY CFHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM )
{
  if(message==WM_COMMAND && LOWORD(wParam)==1026){
    SendMessage(hDlg, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM)&font);
    setFont();
    return 1;
  }         
  return 0;
}
//-------------------------------------------------------------------------
//procedure for main window
BOOL CALLBACK MainWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 int i,j,x,cmd,h,item,index,top;
 char *s;
 RECT rc;
 POINT pt;
 HotKey *hk;
 LV_COLUMN lvc;
 HBITMAP hbmp;
 ULONG_PTR u;
 static UINT taskbarRestart;
 PROCESS_INFORMATION pi;
 STARTUPINFO si;

 switch (mesg) {
  case WM_INITDIALOG:
   GetWindowRect(GetDlgItem(hWnd,102),&rc);
   pt.x=rc.left; pt.y=rc.top;
   ScreenToClient(hWnd,&pt);
   GetClientRect(hWnd,&rc);
   h= rc.bottom-GetSystemMetrics(SM_CYMENU); 
   //create treeview
   tree = CreateWindowEx(0, WC_TREEVIEW, "Categories",
     WS_CHILD | TVS_SHOWSELALWAYS | WS_BORDER | WS_VISIBLE | TVS_DISABLEDRAGDROP,
     0, 0, treeW,h, hWnd, (HMENU) 121, inst, NULL);
   //create listview
   listBox = CreateWindow(WC_LISTVIEW, "",
     WS_CHILD | LVS_REPORT | 
     LVS_SHOWSELALWAYS | WS_VISIBLE | WS_TABSTOP,
     treeW+splitterW, 0, pt.x-11-treeW-splitterW, h,
     hWnd, (HMENU) 101, inst, NULL);
   ListView_SetExtendedListViewStyle(listBox,LVS_EX_FULLROWSELECT);
   //add icons to the listview
   himl = ImageList_Create(16,16, ILC_COLOR16, 35, 10);
   hbmp = (HBITMAP) LoadImage(inst, "ASCDESC", IMAGE_BITMAP, 0,0, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
   ImageList_Add(himl, hbmp, (HBITMAP) NULL);
   DeleteObject(hbmp);
   hbmp = (HBITMAP) LoadImage(inst, "ICONS", IMAGE_BITMAP, 0,0, LR_LOADTRANSPARENT);
   ImageList_Add(himl, hbmp, (HBITMAP) NULL);
   DeleteObject(hbmp);
   ListView_SetImageList(listBox,himl,LVSIL_SMALL);
   numImages= ImageList_GetImageCount(himl);
   //create columns
   lvc.mask = LVCF_WIDTH | LVCF_SUBITEM;
   for(i=0; i<sizeA(colWidth); i++){
     lvc.iSubItem = i;
     lvc.cx = colWidth[i];
     ListView_InsertColumn(listBox,i,&lvc);
   }
   SetFocus(listBox);
   setFont();
   //set up,down buttons images
   SendDlgItemMessage(hWnd,105,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(inst,MAKEINTRESOURCE(3)));
   SendDlgItemMessage(hWnd,106,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(inst,MAKEINTRESOURCE(4)));
   DragAcceptFiles(hWnd,TRUE);
   //message that is used when the taskbar (explorer.exe) is restarted
   taskbarRestart = RegisterWindowMessage(TEXT("Taskbarcreated"));
   return FALSE;

  case WM_DROPFILES:
   DragQueryFile((HDROP)wP, 0, exeBuf, sizeof(exeBuf));
   DragFinish((HDROP)wP);
   SetForegroundWindow(hWnd);
   addKey(exeBuf,0,numKeys);
  break;

  case WM_GETMINMAXINFO:
   ((MINMAXINFO FAR*) lP)->ptMinTrackSize.x = treeW+260;
   ((MINMAXINFO FAR*) lP)->ptMinTrackSize.y = 280;
  break;

  case WM_NOTIFY:
   switch(((LPNMHDR) lP)->code){
    case LVN_GETDISPINFO:
    {
     LV_DISPINFO *pnmv= (LV_DISPINFO*) lP;
     hk = &hotKeyA[pnmv->item.lParam];
     if(pnmv->item.mask & LVIF_TEXT){
       char *buf = pnmv->item.pszText;
       *buf=0;
       if(pnmv->item.lParam>=numKeys) break;
       int n = pnmv->item.cchTextMax;
       switch(pnmv->item.iSubItem){
        case 0:
         sprintf(buf, "%d", (hk-hotKeyA)+1);
        break;
        case 1:
         printKey(buf, hk);
        break;
        case 2: {
         char const * s= hk->getNote();
         lstrcpyn(buf,s,n-2);
         if(hk->cmd>=0 && s!=hk->note && *hk->args){
           i=lstrlen(s);
           strcpy(buf+i,": ");
           i+=2;
           lstrcpyn(buf+i, hk->args, n-i);
         }
        } break;
       }
     }
     if(pnmv->item.mask & LVIF_IMAGE){
       pnmv->item.iImage= hk->getIcon();
     }
    }
    break;
    case LVN_COLUMNCLICK:
    {
     NM_LISTVIEW *pnm= (NM_LISTVIEW *) lP;
     if(pnm->iSubItem==sortedCol){
       descending=-descending;
     }else{
       sortedCol=pnm->iSubItem;
       descending=-1;
     }
     //header bitmap
     sortChanged();
     //remember cursor position
     item= getItemFromIndex(index= ListView_GetNextItem(listBox, -1, LVNI_FOCUSED));
     top= ListView_GetTopIndex(listBox);
     j= ListView_GetCountPerPage(listBox);
     //sort
     initList1();
     //restore position
     i= top + getIndexFromItem(item) - index;
     ListView_EnsureVisible(listBox, min(i+j,numKeys)-1, TRUE);
     ListView_EnsureVisible(listBox, max(i,0), TRUE);
     setCur(item);
    }
    break;
    case LVN_KEYDOWN:
     search(((LV_KEYDOWN*) lP)->wVKey);
    break;
    case NM_DBLCLK:
     PostMessage(hWnd,WM_COMMAND,102,0);
    break;
    case LVN_BEGINDRAG:
     dragStart=((NMLISTVIEW*)lP)->iItem;
     SetCapture(hWin);
    break;
    case TVN_SELCHANGED:
     if(!treeLock){
       i=((NMTREEVIEW*)lP)->itemNew.lParam;
       if(i!=selectedCategory){
         selectedCategory=i;
         setSel(-1,false);
         initList1();
         static const int C[]={217,218,219,221};
         for(i=0; i<sizeA(C); i++){
           EnableMenuItem(GetMenu(hWnd),C[i],
           (selectedCategory>0 ? MF_ENABLED : MF_GRAYED)|MF_BYCOMMAND);
         }
       }
     }
    break;
   }
  break;

  case WM_LBUTTONDOWN:
    x=(short)LOWORD(lP);
    if(x>=treeW && x<=treeW+splitterW){
      treeResizing=true;
      SetCapture(hWnd);
      SetCursor(LoadCursor(0,IDC_SIZEWE));
    }
  break;

  case WM_LBUTTONUP:
    if(dragStart>=0){
      itemDrag(lP);
      dragStart= -1;
      ReleaseCapture();
    }
    if(treeResizing){
      treeResizing=false;
      ReleaseCapture();
    }
  break;
  case WM_MOUSEMOVE:
    if(dragStart>=0){
      int d=0;
      RECT rc;
      itemDrag(lP);
      int t=ListView_GetTopIndex(listBox);
      if(t==dragStart && dragStart>0) d=-1;
      if(t+ListView_GetCountPerPage(listBox)-1==dragStart && dragStart<numKeys-1) d=1;
      if(d){
        POINT pt;
        GetCursorPos(&pt);
        ListView_EnsureVisible(listBox,dragStart+d,TRUE);
        ListView_GetItemRect(listBox,dragStart,&rc,LVIR_BOUNDS);
        pt.y-=d*(rc.bottom-rc.top);
        SetCursorPos(pt.x, pt.y);
      }
      return 0;
    }
    x=(short)LOWORD(lP);
    if(treeResizing){
      GetWindowRect(listBox,&rc);
      aminmax(x,1,rc.right-rc.left+treeW-100);
      j=x-treeW;
      treeW=x;
      HDWP p = BeginDeferWindowPos(2);
      DeferWindowPos(p,listBox,0,treeW+splitterW,0,rc.right-rc.left-j,rc.bottom-rc.top,SWP_NOZORDER);
      GetWindowRect(tree,&rc);
      DeferWindowPos(p,tree,0,0,0,treeW,rc.bottom-rc.top,SWP_NOMOVE|SWP_NOZORDER);
      EndDeferWindowPos(p);
    }
    if(x>=treeW && x<=treeW+splitterW || treeResizing){
      SetCursor(LoadCursor(0,IDC_SIZEWE));
    }
  break;

  case WM_COMMAND:
   cmd=LOWORD(wP);
   item= getItemFromIndex(index= ListView_GetNextItem(listBox, -1, LVNI_FOCUSED));
   switch(cmd){
    default:
     if(cmd>=1000 && cmd<1000+numKeys){
       //systray menu command
       delayAndExecuteHotKey(inst,hWnd,cmd-1000);
     }else{
       setLang(cmd);
     }
    break;
    case 100: //program just started
     if(disableTaskMgr<0 && !isEmptyPassword()){
       //fix bug from version 3.5
       disableTaskMgr=0;
       restoreTaskMgr();
     }
     if(*pcLockParam){
       restoreAfterLock();
       command(63,pcLockParam);
     }
     if(show==0){
       //run autoStart hotkeys 
       for(i=0; i<numKeys; i++){
         if(hotKeyA[i].autoStart) executeHotKey(i);
       }
     }
     aminmax(dlgX,0,GetSystemMetrics(SM_CXSCREEN)-200);
     aminmax(dlgY,0,GetSystemMetrics(SM_CYSCREEN)-100);
     MoveWindow(hWnd,dlgX,dlgY,dlgW,dlgH,TRUE);
     if(show==1 || !*iniFile && show!=0){
       ShowWindow(hWin,SW_SHOWDEFAULT);
     }
    break;
    case 102: //Edit
     if(item>=0 && item<numKeys){
       editKey(item);
     }
    break;
    case 103: //Remove
     EnterCriticalSection(&listCritSect);
     unregisterKeys();
     for(i=-1, j=0; ;){
       i= ListView_GetNextItem(listBox, i, LVNI_SELECTED);
       if(i<0) break;
       if(i<index) j++;
       hotKeyA[getItemFromIndex(i)].vkey= vkDelete;
     }
     for(i=0; i<numKeys; ){
       if(hotKeyA[i].vkey==vkDelete){ delKey(i); item=i; }
       else i++;
     }
     modified();
     LeaveCriticalSection(&listCritSect);
     setCurIndex(index-j);
    break;
    case 104: //Insert
     addKey(0,0,item+insertAfterCurrent);
    break;
    case 108: //Add
     addKey(0,0,numKeys);
    break;
    case 216: //Copy
     if(item>=0 && item<numKeys){
       addKey(0,true,item+1);
     }
    break;
    case 105: //Up
     moveSelected(-1);
     ListView_EnsureVisible(listBox,ListView_GetNextItem(listBox,-1,LVNI_SELECTED),FALSE);
    break;
    case 106: //Down
     moveSelected(+1);
     ListView_EnsureVisible(listBox,ListView_GetNextItem(listBox,ListView_GetItemCount(listBox)-1,LVNI_SELECTED|LVNI_ABOVE),FALSE);
    break;
    case 107: //Run
     executeHotKey(item);
    break;
    case 113: //Rename
     if(item>=0 && item<numKeys){
       renaming=true;
       editKey(item);
       renaming=false;
     }
    break;
    case 201: //Open
     if(open(ofn)) rd(iniFile);
    break;
    case 202: //Save
     if(save(ofn)) wr(iniFile);
    break;
    case 203: //Exit
    case 211:
     if(!hideMainWindow()) break;
     DestroyWindow(hWnd);
    break;
    case 204: //Hide
     hideMainWindow();
    break;
    case 205: //Delete settings
     if(msg1(MB_ICONQUESTION|MB_YESNO,lng(760,"Do you really want to remove configuration from the registry ?"))==IDYES){
       deleteini();
     }
    break;
    case 206: //Font
    {
     static CHOOSEFONT f;
     static LOGFONT lf;
     f.lStructSize=sizeof(CHOOSEFONT);
     f.hwndOwner=hWin;
     f.Flags=CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_SCRIPTSONLY|CF_ENABLEHOOK|CF_APPLY;
     memcpy(&lf,&font,sizeof(lf));
     f.lpLogFont=&lf;
     f.lpfnHook=CFHookProc;
     if(ChooseFont(&f)){
       memcpy(&font,&lf,sizeof(lf));
       setFont();
     }
    }
    break;
    case 207: //New file
     if(!saveAtExit()) break;
     unregisterKeys();
     destroyAll();
     initList();
     *iniFile=0;
    break;
    case 208: //Help
    case 200: //What's new
    {
     char buf[256];
     getExeDir(buf,(cmd==200)? lng(14,"WhatsNew.txt"):lng(13,"help.chm"));
     if(cmd==200 || !showHelp(0)){
       if(ShellExecute(0,"open",buf,0,0,SW_SHOWNORMAL)==(HINSTANCE)ERROR_FILE_NOT_FOUND){
         msglng(739,"File %s not found",buf);
       }
     }
    }
    break;
    case 209: //About
     DialogBox(inst, "ABOUT", hWnd, (DLGPROC) AboutProc);
    break;
    case 210: //Show window
     SetForegroundWindow(hWin);
     ShowWindow(hWin,SW_RESTORE);
    break;
    case 212: //Options
     options();
    break;
    case 213: //open settings
    case 214: //save settings
      if(!((cmd==213 ? open:save)(regOfn))) break;
      memset(&si,0,sizeof(STARTUPINFO));
      si.cb= sizeof(STARTUPINFO);
      si.dwFlags= STARTF_USESHOWWINDOW;
      si.wShowWindow= SW_HIDE;
      s= new char[MAX_PATH+75];
      strcpy(s,"regedit.exe /S \"");
      strcat(s,regFile);
      strcat(s,"\"");
      if(cmd==214){
        s[13]='E';
        strcat(s," \"HKEY_CURRENT_USER\\");
        strcat(s,subkey);
        strcat(s,"\"");
        writeini();
      }
      if(CreateProcess(0,s,0,0,0,0,0,0,&si,&pi)){
        CloseHandle(pi.hThread);
        WaitForSingleObject(pi.hProcess,10000);
        CloseHandle(pi.hProcess);
      }else{
        msglng(743,"Cannot run %s","regedit.exe");
      }
      delete[] s;
      if(cmd==213){
        oldOptions();
        readini();
        optionChanged();
        colorChanged();
        sortChanged();
        setFont();
        loadLang();
        langChanged();
      }
    break;
    case 215: //run spy
     createProcess("spy.exe");
    break;
    case 217: //category up
     swapCategory(selectedCategory-1,selectedCategory);
    break;
    case 218: //category down
     swapCategory(selectedCategory,selectedCategory+1);
    break;
    case 219: //rename category
     if(selectedCategory>0){
       DialogBox(inst,"RENAMECAT",hWin,(DLGPROC)renameCategoryProc);
     }
    break;
    case 220: //assign category to selected hotkeys
     DialogBox(inst,"ASSIGNCAT",hWin,(DLGPROC)assignCategoryProc);
    break;
    case 221: //delete category
      if(selectedCategory>0){
        if(msg1(MB_ICONQUESTION|MB_YESNO,lng(765,"Do you want to delete category '%s' ?"),categoryName[selectedCategory])==IDYES){
          deleteCategory();
        }
      }
    break;
    case 222: //select all
      setSel(-1,true);
    break;
   }
  break;

  case WM_SIZE:
   if(!lP) break;
   if(oldW){
     //adjust all controls positions
     int dw=LOWORD(lP)-oldW;
     int dh=HIWORD(lP)-oldH;
     HDWP p = BeginDeferWindowPos(sizeA(mainButton)+4);
     for(i=0; i<sizeA(mainButton); i++){
       moveX(p,hWnd,mainButton[i].cmd,dw);
     }
     moveX(p,hWnd,105,dw);
     moveX(p,hWnd,106,dw);
     moveW(p,hWnd,101,dw,dh); //listBox
     moveW(p,hWnd,121,0,dh); //treeView
     EndDeferWindowPos(p);
   }
   oldW=LOWORD(lP);
   oldH=HIWORD(lP);                
  break;

  case WM_HOTKEY:
    lP=K_ONLYDOWN;
  //!
  case WM_USER+2654:
   //hotkey was pressed
   if(wP<unsigned(numKeys)){
    sentToActiveWnd=0;
    hk= &hotKeyA[(int)wP];
    if(pcLocked){ ignoreHotkey(hk); break; }
    if(lP!=K_UP && !hk->isLocal()) hiliteIcon();
    for(i=0; i<numKeys; i++){
      HotKey *hk1= &hotKeyA[i];
      if(hk->scanCode==hk1->scanCode && hk->vkey==hk1->vkey && 
        hk->modifiers==hk1->modifiers &&
        (hk->vkey!=vkLirc || !strcmp(hk->lirc,hk1->lirc))){
        int cmd= hk->cmd;
        if(cmd==94){
          if(lP==K_DOWN) cmd=904;
          if(lP==K_UP) cmd=906;
          command(cmd,hk1->args);
        }else if(lP!=K_UP){
          cmdFromKeyPress++;
          delayAndExecuteHotKey(inst,hWnd,i);
          cmdFromKeyPress--;
        }
      }
    }
    if(sentToActiveWnd){
      if(sentToActiveWnd==1) ignoreHotkey(hk);
      else hiliteIcon();
    }else if(hk->cmd!=76 && GetForegroundWindow()==hWin && lP!=K_UP){
      setCur((int)wP);
    }
   }
  break;

  case WM_TIMER:
   if(wP>=50 && wP<50+Npopup){ //hide window
     ShowWindow(popup[wP-50].hWnd,SW_HIDE);
     KillTimer(hWin,wP);
     KillTimer(hWin,wP+10);
   }else
   switch(wP){
    case 2: //wake up when monitor is off and mouse moved
     GetCursorPos(&pt);
     if(pcLocked ? passwdLen>0 : abs(mousePos.x-pt.x)>senz || abs(mousePos.y-pt.y)>senz){
       SendMessage(hWin,WM_SYSCOMMAND,SC_MONITORPOWER,-1);
       KillTimer(hWin,wP);
     }
    break;
    case 3:
     unDelayButtons();
    break;
    case 6: //cancel tray icon highlighting
     KillTimer(hWin,wP);
     isHilited=false;
     modifyTrayIcon();
    break;
    case 8: //lock computer               
     SetWindowPos(hWndLock,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),SWP_SHOWWINDOW);
     SetForegroundWindow(hWndLock);
     SetCursorPos(0,0);
     SetCursor(0);
     drawLockText();
    break;
    case 9: //waitWhileKey
     KillTimer(hWin,wP);
     checkWait();
    break;
    case 10: //refresh hooks
     if(keepHook && (hookK || hookM)){
       if(hookK){ uninstallHook(false); installHook(false); }
       if(hookM){ uninstallHook(true); installHook(true); }
     }else{
       KillTimer(hWin,10);
     }
    break;
    case 12: //paste text - try again
     KillTimer(hWin,12);
     if(pasteTextData.busy) parseMacro("\\^\\V");
    break;
    case 13: //paste text - restore clipboard
     KillTimer(hWin,13);
     pasteTextData.restore();
    break;
    case 14: //paste text - Ctrl+V
     KillTimer(hWin,14);
     parseMacro("\\^\\V");
     SetTimer(hWin,12,1000,0);
     lockPaste--;
    break;

    case 60+P_Volume: //update volume
     volume(0,0,4);
    break;
    case 60+P_DiskFree: //update disk free space
     diskFree();
    break;
   }
  break;            

  case WM_QUERYENDSESSION: //Windows are shutting down
   if(IsWindowVisible(hWnd)){
     if(!hideMainWindow()) return TRUE;
   }else{
     if(!saveAtExit()) return TRUE;
   }
  return FALSE; //it will return TRUE, because we are in a dialog box

  case WM_ENDSESSION:
   if(wP) closeAll2();
  break;

  case WM_CLOSE: //close window
   if(IsWindowVisible(hWnd)){
     if(!hideMainWindow()) break;
     if(!closeToTray) DestroyWindow(hWnd);
   }else{
     DestroyWindow(hWnd);
   }
  break;

  case WM_SYSCOMMAND:
   if(wP!=SC_MINIMIZE || !minToTray) return FALSE;
   hideMainWindow();
  break;

  case WM_DESTROY: //exit
   deleteTrayIcon();
   unhideAll();
   hWin= listBox= 0;
   DeleteObject(hFont);
   unregisterKeys();
   PostQuitMessage(0);
  break;

  case WM_RENDERFORMAT:
   if(wP!=(WPARAM)(isWin9X ? CF_TEXT : CF_UNICODETEXT)) break;
  //!
  case WM_RENDERALLFORMATS:
   copyToClipboard1(pasteTextData.text);
   if(pasteTextData.busy){
     pasteTextData.busy=false;
     //restore previous clipboard content
     SetTimer(hWnd,13,300,0);
   }
  break;

  case WM_USER+54:
   SetWindowLongPtr(hWnd,DWLP_MSGRESULT,(LRESULT)hookK);
   return TRUE;
  case WM_USER+51:
   SetWindowLongPtr(hWnd,DWLP_MSGRESULT,(LRESULT)hookM);
   return TRUE;
  
  case WM_USER+57:
  {
   static const WPARAM M[]={WM_KEYDOWN,WM_KEYUP,WM_SYSKEYDOWN,WM_SYSKEYUP};
   SetWindowLongPtr(hWnd,DWLP_MSGRESULT,keyFromHook(M[((lP>>31)&1)|((lP>>28)&2)],wP,lP));
   return TRUE;
  }
  case WM_USER+53:
   SetWindowLongPtr(hWnd,DWLP_MSGRESULT,clickFromHook(wP,lP));
   return TRUE;

  case WM_USER+55:
   if(wP==1){
     switch(LOWORD(lP)){
      case WM_LBUTTONDBLCLK:
       PostMessage(hWin, WM_COMMAND, 210,0);
      break;
      case WM_LBUTTONDOWN:
       PostMessage(hWin,WM_COMMAND,IsWindowVisible(hWin)?204:210,0);
      break;
      case WM_RBUTTONDOWN:
      {
       SetForegroundWindow(popupVolume.hWnd);
       POINT p;
       GetCursorPos(&p);
       static int iconSubId[]={0};
       showPopup(p.x,p.y, "ICONPOPUP", iconSubId, hWin);
       PostMessage(hWin,WM_NULL,0,0);
      }
      break;
     }
   }else if(wP>=100 && wP<100+sizeA(trayIconA)){
     if(LOWORD(lP)==WM_LBUTTONDOWN){
       deleteTrayIcon(static_cast< UINT >(wP));
       unhideApp(&trayIconA[wP-100]);
     }
   }
  break;
  
  case WM_USER+80: //remote control button
   if(editing && GetFocus()==GetDlgItem(hHotKeyDlg,103)){
     cpStr(dlgKey.lirc, lircButton);
     acceptKey(vkLirc,0);
   }else{
     for(i=0; i<numKeys; i++){
       HotKey *hk= &hotKeyA[i];
       if(hk->vkey==vkLirc && !hk->disable && 
         hk->lirc && !strcmp(hk->lirc,lircButton)){
         postHotkey(i,K_ONLYDOWN);
         break;
       }
     }
   }
   break;

  case WM_USER+81:
   if(hWndLircState){
     SetWindowText(hWndLircState, lP ? lng(366,"active") : lng(367,"not running"));
   }
   break;

  case WM_USER+6341: //unlock computer
  {
    pcLocked=false;
    KillTimer(hWin,8);
    SetCursorPos(mousePos.x,mousePos.y);
    DestroyWindow(hWndLock);
    DeleteObject(lockImg);
    setHook();
    TregisterServiceProcess p= (TregisterServiceProcess)GetProcAddress(GetModuleHandle("kernel32.dll"),"RegisterServiceProcess");
    if(p) p(0,0);
    restoreAfterLock();
    cpStr(pcLockParam,"");
    writeini();
    forceNoKey();
    if(hWndBeforeLock) SetForegroundWindow(hWndBeforeLock);
    break;
  }
  case WM_USER+82:
    altDown=false;
    keyEventUp(VK_MENU);
    break;

  case WM_USER+83: //joystick
    if(editing && GetFocus()==GetDlgItem(hHotKeyDlg,103)){
      if(wP!=K_UP) acceptKey(vkJoy,static_cast< DWORD >(lP));
    }else{
      for(i=0; i<numKeys; i++){
        HotKey *hk= &hotKeyA[i];
        if(hk->vkey==vkJoy && !hk->disable && hk->scanCode==lP){
          if(hk->isLocal() || joyGlobalEnabled()){
            postHotkey(i,wP);
          }
          break;
        }
      }
    }
    break;

  case WM_USER+2377:
    if(hHotKeyDlg) SendDlgItemMessage(hHotKeyDlg,102,WM_PASTE,0,0);
    break;

  case WM_COPYDATA:
    u= ((COPYDATASTRUCT*)lP)->dwData - 13000;
    if(u<sizeA(cmdNames)){
      command((int)u,(char*)((COPYDATASTRUCT*)lP)->lpData);
    }
    break;

  default:
   if(mesg==taskbarRestart && trayicon) addTrayIcon();
   return FALSE;
 }
 return TRUE;
}
//-------------------------------------------------------------------------
//volume, disk free space, show text window procedure
LRESULT popupWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 int ind,k,y,x,x2,len,mxId,rec;
 Tpopup *p;
 HDC dc;
 RECT rc;
 HBRUSH br;
 HGDIOBJ oldP,oldB;
 char buf[32],*name,*t,*u;
 PAINTSTRUCT ps;
 static const int clTab[]={clVol0,clVol1,clVol2,clVol3,clVol4};
 static const int CBkgnd[]={clVolBkgnd,clDskBkgnd,clTxtBkgnd};
 static const int CText[]={clVolText,clDskText,clTxtText};
 static const int CBorder[]={clVolBorder,clDskBorder,clTxtBorder};

 for(ind=0; ;ind++){
   p= &popup[ind];
   if(p->hWnd==hWnd || ind==Npopup-1) break;
 }
 switch(mesg){
  case WM_PAINT:
  {
   dc = BeginPaint(hWnd, &ps);
   //window border and background
   oldP=SelectObject(dc,CreatePen(PS_SOLID,0,colors[CBorder[ind]]));
   oldB=SelectObject(dc,CreateSolidBrush(colors[CBkgnd[ind]]));
   Rectangle(dc,0,0,p->width,p->height);
   DeleteObject(SelectObject(dc,oldB));
   //text
   SetTextColor(dc,colors[CText[ind]]);
   SetBkMode(dc,TRANSPARENT);
   switch(ind){
   case P_Volume:
     x=15,x2=(p->width+x+5*LOWORD(GetDialogBaseUnits()))/2;
     for(k=0; getVolumeName(k,name,len,mxId,rec,true); k++){
       //bar
       rc.top=textY(k);
       rc.bottom=rc.top+fontH;
       rc.left=1;
       rc.right=((p->width-2)*max(0,curVolume[k][0]))/100+1;
       br=CreateSolidBrush(colors[clTab[min(k,sizeA(clTab)-1)]]);
       FillRect(dc,&rc,br);
       DeleteObject(br);
       //audio line name
       SetTextAlign(dc,TA_LEFT);
       t=0;
       if(mxId>0 || rec){
         t=name;
         name= u= new char[len+8];
         if(mxId>0){ *u++= (char)(mxId+'1'); *u++=':'; *u++=' '; }
         if(rec){ *u++='R'; *u++=':'; }
         lstrcpynA(u,t,len+1);
         len= static_cast<int>(strlen(name));
       }
       TextOut(dc,x,rc.top,name,len);
       if(t) delete[] name;
       //volume value
       SetTextAlign(dc,TA_CENTER);
       t="???";
       if(curVolume[k][1]>0){
         t=lng(752,"Muted");
       }else if(curVolume[k][0]>=0){
         sprintf(t=buf,"%d %%",curVolume[k][0]);
       }else if(curVolume[k][1]==0){
         t="";
       }
       TextOut(dc,x2,rc.top, t, (int)strlen(t));
     }
     break;
   case P_DiskFree:
     y=0;
     for(k=0; k<26; k++){
       DiskInfo *d= &disks[k];
       if(!d->text[0]) continue;
       y+=diskSepH+1;
       rc.top=y;
       rc.bottom=y+fontH;
       rc.left=19;
       rc.right=rc.left + ((p->width-2-rc.left) * d->bar)/1000;
       br=CreateSolidBrush(colors[clDiskFree]);
       FillRect(dc,&rc,br);
       DeleteObject(br);
       //drive letter
       SetTextAlign(dc,TA_LEFT);
       buf[0]=(char)(k+'A');
       buf[1]=':';
       TextOut(dc,2,y,buf,2);
       //free space
       SetTextAlign(dc,TA_CENTER);
       TextOut(dc,p->width>>1,y, d->text, (int)strlen(d->text));
       y=rc.bottom+diskSepH;
       if(k){
         //horizontal line
         MoveToEx(dc,1,y,0);
         LineTo(dc,p->width,y);
       }
     }
     break;
   case P_ShowText:
     if(showTextStr){
       rc.left=rc.top=showTextBorder;
       DrawText(dc,showTextStr,-1,&rc,DT_NOPREFIX|DT_NOCLIP);
     }
     break;
   }
   DeleteObject(SelectObject(dc,oldP));
   EndPaint(hWnd, &ps);
  }
  break;
  case WM_NCHITTEST:
   return HTCAPTION;
  case WM_MOVE:
   SetTimer(hWin,50+ind, 2000, 0);
  break;
  case WM_EXITSIZEMOVE:
   GetWindowRect(hWnd,&rc);
   p->x= (((rc.left+rc.right)>>1)*10000-1)/GetSystemMetrics(SM_CXSCREEN)+1;
   p->y= (((rc.top+rc.bottom)>>1)*10000-1)/GetSystemMetrics(SM_CYSCREEN)+1;
  break;
  case WM_KEYDOWN:
   if(wP==VK_ESCAPE) PostMessage(hWin,WM_TIMER,50+ind,0);
  //!
  default:
   return DefWindowProc(hWnd, mesg, wP, lP);
 }
 return 0;
}

void createPopups()
{
  static bool done=false;

  if(!done){
    done=true;
    //register window class
    WNDCLASS wc;
    wc.cbClsExtra=0;
    wc.hInstance=inst;
    wc.hIcon=0;
    wc.hCursor=LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName=NULL;
    wc.lpfnWndProc= (WNDPROC)popupWndProc;
    wc.cbWndExtra=0;
    wc.style= CS_SAVEBITS;
    wc.hbrBackground=0;
    wc.lpszClassName="HotkeyPopup";
    RegisterClass(&wc);
    //create popup windows
    for(int i=0; i<Npopup; i++){
      Tpopup *p= &popup[i];
      p->hWnd= CreateWindowEx(WS_EX_TOOLWINDOW,"HotkeyPopup","",
        WS_POPUP,0,0, 100,10,hWin,0,inst,0);
      setOpacity(p->hWnd,p->opacity);
    }
  }
}

//-------------------------------------------------------------------------

LRESULT zoomProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
  static int cnt;
  switch(mesg){
  case WM_MOUSEMOVE:
    if(--cnt<0){
      cnt=1;
      zoom.move();
    }
    break;
  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
    zoom.end();
    break;
  case WM_PAINT:
  {
   PAINTSTRUCT ps;
   HDC dc = BeginPaint(hWnd, &ps);
   RECT rc;
   GetClientRect(hWnd,&rc);
   int w = rc.right/zoom.magnification;
   int h = rc.bottom/zoom.magnification;
   StretchBlt(dc, 0,0, rc.right,rc.bottom,
     zoom.dc, (zoom.width-w)>>1, (zoom.height-h)>>1,
     w, h, SRCCOPY);
   EndPaint(hWnd, &ps);
   break;
  }
  default:
    return DefWindowProc(hWnd, mesg, wP, lP);
  }
  return 0;
}

void Zoom::move()
{
  int dx,dy;
  POINT p,p0;
  RECT rc;
  HDC dc0;

  GetCursorPos(&p);
  GetWindowRect(wnd,&rc);
  p0.x = rc.left+(width>>1);
  p0.y = rc.top+(height>>1);
  dx = p.x - p0.x;
  dy = p.y - p0.y;
  dc0 = GetDC(0);
  BitBlt(dc,-dx,-dy,width,height,dc,0,0,SRCCOPY);
  if(dx>0){
    if(dy>0){
      //BitBlt(dc, 0,0, width-dx,height-dy, dc, dx,dy, SRCCOPY);
      BitBlt(dc, 0,height-dy, width,dy, dc0, rc.left+dx,rc.bottom, SRCCOPY);
      BitBlt(dc, width-dx,0, dx,height-dy, dc0, rc.right,rc.top+dy, SRCCOPY);
    }else{
      //BitBlt(dc, 0,-dy, width-dx,height+dy, dc, dx,0, SRCCOPY);
      if(dy) BitBlt(dc, 0,0, width,-dy, dc0, rc.left+dx,rc.top+dy, SRCCOPY);
      BitBlt(dc, width-dx,-dy, dx,height+dy, dc0, rc.right,rc.top, SRCCOPY);
    }
  }else{
    if(dy>0){
      //BitBlt(dc, -dx,0, width+dx,height-dy, dc, 0,dy, SRCCOPY);
      BitBlt(dc, 0,height-dy, width,dy, dc0, rc.left+dx,rc.bottom, SRCCOPY);
      if(dx) BitBlt(dc, 0,0, -dx,height-dy, dc0, rc.left+dx,rc.top+dy, SRCCOPY);
    }else{
      //BitBlt(dc, -dx,-dy, width+dx,height+dy, dc, 0,0, SRCCOPY);
      if(dy) BitBlt(dc, 0,0, width,-dy, dc0, rc.left+dx,rc.top+dy, SRCCOPY);
      if(dx) BitBlt(dc, 0,-dy, -dx,height+dy, dc0, rc.left+dx,rc.top, SRCCOPY);
    }
  }
  ReleaseDC(0,dc0);
  SetWindowPos(zoom.wnd,HWND_TOPMOST, p.x-(zoom.width>>1), p.y-(zoom.height>>1),0,0,SWP_NOSIZE|SWP_NOCOPYBITS|SWP_NOACTIVATE);
}

void Zoom::start()
{
  isZoom=true;
  //create bitmap
  HDC dc0 = GetDC(0);
  bmp = CreateCompatibleBitmap(dc0,width,height);
  dc = CreateCompatibleDC(dc0);
  oldBmp = SelectObject(dc,bmp);
  //hide mouse cursor
  SetCapture(wnd);
  POINT p;
  GetCursorPos(&p);
  ShowCursor(FALSE);
  //save screen under the window
  int x,y;
  x= p.x-(width>>1);
  y= p.y-(height>>1);
  BitBlt(dc,0,0,width,height,dc0,x,y,SRCCOPY);
  ReleaseDC(0,dc0);
  //show window
  SetWindowPos(wnd, HWND_TOPMOST, x,y, 
    width,height, SWP_SHOWWINDOW|SWP_NOACTIVATE);
}

void Zoom::end()
{
  ReleaseCapture();
  ShowWindow(wnd,SW_HIDE);
  ShowCursor(TRUE);
  SelectObject(dc,oldBmp);
  DeleteObject(bmp);
  DeleteDC(dc);
  isZoom=false;
}
//-------------------------------------------------------------------------

LRESULT lockWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
 if(mesg==WM_PAINT){
   PAINTSTRUCT ps;
   HDC dc = BeginPaint(hWnd, &ps);
   if(lockImg){
     HDC dcb=CreateCompatibleDC(dc);
     HGDIOBJ oldB=SelectObject(dcb,lockImg);
     BITMAP bmp;
     GetObject(lockImg,sizeof(bmp),&bmp);
     StretchBlt(dc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),
       dcb,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
     SelectObject(dcb,oldB);
     DeleteDC(dcb);
   }else{
     FillRect(dc,&ps.rcPaint,(HBRUSH)GetStockObject(BLACK_BRUSH));
   }
   EndPaint(hWnd, &ps);
   return 0;
 }

 static UINT cancelAutoPlay;
 if (!cancelAutoPlay) cancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
 if (mesg == cancelAutoPlay) return TRUE; //cancel CD auto-play

 return DefWindowProc(hWnd, mesg, wP, lP);
}

//-------------------------------------------------------------------------
int commandS(char *cmdLine)
{
 int i,cmd,cmdLen,l;
 char *s,*param;
 bool quot=false;

 if(*cmdLine==' ') cmdLine++;
 cmdLine++;
 if(cmdLine[0]>='0' && cmdLine[0]<='9'){
   //command number
   cmd=strtol(cmdLine,&param,10);
   if(cmd>=1000) cmd-=1000;
 }else{
   //command name
   cmdLen=0;
   param=0;
   cmd=0;
   for(i=0; i<sizeA(cmdNames); i++){
     l=static_cast<int>(strlen(cmdNames[i]));
     if(!_strnicmp(cmdLine,cmdNames[i],l) && 
         (cmdLine[l]==0 || cmdLine[l]==' ') && l>cmdLen){
       cmdLen=l;
       param=cmdLine+l;
       cmd=i;
     }
   }
 }
 if(!param || cmd>=sizeA(cmdNames)){
   msg("Unknown command: %s",cmdLine);
   return 4;
 }
 if(*param==' ') param++;
 cmdLineCmd=cmd;
 //find next command
 for(s=param; *s; s++){
   if(*s=='\"') quot=!quot;
   if(*s==';' && (s[1]==' ' && s[2]=='-' || s[1]=='-') && !quot){
     *s=0;
     command(cmd,param);
     *s=';';
     return commandS(s+1);
   }
 }
 command(cmd,param);
 return 0;
}
//-------------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE , LPSTR cmdLine, int cmdShow)
{
 MSG mesg;
 WNDCLASS wc;
 size_t l;
 OSVERSIONINFO v;
 BYTE c;

 inst=hInstance;
 if(!strcmp(cmdLine,"--htmlhelp")) return helpProcess();
 cmdLineCmd=-1;
 InitializeCriticalSection(&cdCritSect);
 //Windows version
 v.dwOSVersionInfoSize= sizeof(OSVERSIONINFO);
 GetVersionEx(&v);
 isWin9X = v.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS;
 isVista = v.dwMajorVersion > 5;
 //init specialWinKeys
 if(isVista){
   useHook=1;
   c=0; //Win 7
   if(v.dwMinorVersion == 0) c='P'; //Win Vista
 }else if(isWin9X){
   c='U';
 }else{
   c='T'; //Win XP
   if(v.dwMajorVersion<5 || v.dwMinorVersion == 0) c='B'; //Win 2000
 }
 for(const BYTE *p=specialWinKeysList; *p!=c; p++){
   specialWinKeys[*p]=1;
 }

 //default options
 strcpy(lircExe,"C:\\Program Files\\WinLIRC\\winlirc.exe");
 strcpy(lircAddress,"127.0.0.1");
 if(!isVista) strcpy(volumeStr, "Mixer,Wave");
 popupVolume.x=5000; popupVolume.y=9000; popupVolume.width=130;
 popupVolume.refresh=1000; popupVolume.delay=2400;
 popupDiskFree.x=5000; popupDiskFree.y=6666; popupDiskFree.width=180;
 popupDiskFree.refresh=1000;
 popupShowText.x=7000; popupShowText.y=6000;
 font.lfHeight=-15;
 font.lfWeight=FW_NORMAL;
 font.lfCharSet=DEFAULT_CHARSET;
 strcpy(font.lfFaceName,"Arial");
 cpStr(pcLockParam,"");
 //read registry settings
 readini();
 //system font height
 fontH=HIWORD(GetDialogBaseUnits());
 //command line arguments
 if(cmdLine[0]=='-'){
   return commandS(cmdLine);
 }
 l=strlen(cmdLine);
 show=-1;
 if(cmdLine[l-1]=='0') show=0;
 if(cmdLine[l-1]=='1') show=1;

 //only activate window if this program is already running
 HWND w=FindWindow("PlasHotKey",0);
 if(w){
   if(show!=0){
     ShowWindow(w,cmdShow);
     SetForegroundWindow(w);
   }
   return 1;
 }
 if(GetProcAddress(GetModuleHandle("comctl32.dll"),"DllGetVersion")){
   INITCOMMONCONTROLSEX iccs;
   iccs.dwSize= sizeof(INITCOMMONCONTROLSEX);
   iccs.dwICC= ICC_LISTVIEW_CLASSES|ICC_TREEVIEW_CLASSES;
   InitCommonControlsEx(&iccs);
 }else{
   InitCommonControls();
 }           
 initLang();
 //delete old help files
 getExeDir(exeBuf,"hotkeyp_CZ.txt");
 DeleteFile(exeBuf);
 getExeDir(exeBuf,"hotkeyp_EN.txt");
 DeleteFile(exeBuf);
 //register classes
 wc.style=0;
 wc.lpfnWndProc=(WNDPROC)DefDlgProc;
 wc.cbClsExtra=0;
 wc.cbWndExtra=DLGWINDOWEXTRA;
 wc.hInstance=hInstance;
 wc.hIcon=LoadIcon(inst,MAKEINTRESOURCE(1));
 wc.hCursor=LoadCursor(NULL, IDC_ARROW);
 wc.hbrBackground=(HBRUSH)COLOR_BTNFACE;
 wc.lpszMenuName=NULL;
 wc.lpszClassName="PlasHotKey";
 if(!RegisterClass(&wc)){ msg("RegisterClass error"); return 2; }

 wc.lpfnWndProc= (WNDPROC)lockWndProc;
 wc.lpszClassName="HotkeyLock";
 wc.hCursor=0;
 RegisterClass(&wc);

 wc.lpfnWndProc= (WNDPROC)zoomProc;
 wc.lpszClassName="HotkeyZoom";
 wc.hbrBackground=0;
 RegisterClass(&wc);

 //create main window
 hWin = CreateDialog(hInstance,"WIN",0,(DLGPROC)MainWndProc);
 if(!hWin){ msg("CreateDialog error"); return 3; }
 haccel=LoadAccelerators(hInstance, MAKEINTRESOURCE(3));
 createPopups();
 zoom.wnd = CreateWindowEx(WS_EX_TOOLWINDOW,"HotkeyZoom","Magnifier",WS_POPUP|WS_BORDER,0,0,100,100,0,0,inst,0);

 InitializeCriticalSection(&listCritSect);
 HANDLE hThreadK= CreateThread(0,0,hookProc,0,0,&idHookThreadK);
 SetThreadPriority(hThreadK,THREAD_PRIORITY_HIGHEST);
 HANDLE hThreadM= CreateThread(0,0,hookProc,0,0,&idHookThreadM);
 SetThreadPriority(hThreadM,THREAD_PRIORITY_HIGHEST);
 DWORD idJoyThread;
 joyThread= CreateThread(0,0,joyProc,0,CREATE_SUSPENDED,&idJoyThread);

 rd(iniFile); //read HTK file
 delRun(HKEY_LOCAL_MACHINE);
 langChanged(); //set button captions according to selected language
 sortChanged();
 oldHook=useHook;
 optionChanged();
 srand((unsigned)time(0));
 //enable drag&drop from processes which have normal integrity level
 TChangeWindowMessageFilter pChangeWindowMessageFilter = (TChangeWindowMessageFilter) GetProcAddress(GetModuleHandle("user32.dll"),"ChangeWindowMessageFilter");
 if(pChangeWindowMessageFilter){
   pChangeWindowMessageFilter(WM_DROPFILES, 1);
   pChangeWindowMessageFilter(0x0049, 1);
 }
 //show main window if the program is run for the first time
 PostMessage(hWin,WM_COMMAND,100,0);

 while(GetMessage(&mesg, NULL, 0, 0)>0)
  if(mesg.hwnd==popupVolume.hWnd || mesg.hwnd==popupDiskFree.hWnd || mesg.hwnd==popupShowText.hWnd ||
     !TranslateAccelerator(hWin,haccel,&mesg)){
   if(!IsDialogMessage(hWin,&mesg)){
     TranslateMessage(&mesg);
     DispatchMessage(&mesg);
   }
  }

 lircEnd();
 UnhookWindowsHookEx(hookM);
 UnhookWindowsHookEx(hookK);
 messageToHook(WM_QUIT,0,false);
 messageToHook(WM_QUIT,0,true);
 WaitForSingleObject(hThreadK,5000);
 CloseHandle(hThreadK);
 WaitForSingleObject(hThreadM,5000);
 CloseHandle(hThreadM);
 DeleteCriticalSection(&cdCritSect);
 DeleteCriticalSection(&listCritSect);
 return 0;
}
//-------------------------------------------------------------------------
