/*
  (C) 2006  Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  
#include <windows.h>
#include <string.h>
#include <tlhelp32.h>
#include <stdio.h>
#pragma hdrstop

#define Mitem 512
#define Ncolumns 4

struct TspyItem
{
  WPARAM wP;
  HWND wnd;
  DWORD pid;
  char *wclass;
  char *title;
  char *exe;
  int action;
};

enum { A_cmd, A_icon };

HMODULE klib;
HWND hWin,listBox;
HHOOK hook,hookG;
int first,count;
int colWidth[Ncolumns+1];
char *title="Spy";
bool isWin9X;

TspyItem A[Mitem];

//-------------------------------------------------------------------------
void msg(char *text)
{
  MessageBox(0,text,"Spy",MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
}

void cpStr(char *&dest, char *src)
{
  char *old=dest; 
  dest = new char[strlen(src)+1];
  strcpy(dest, src);
  delete[] old; //src can be a pointer into old 
}

//return pointer to name after a path
char *cutPath(char *s)
{
  char *t;
  t=strchr(s,0);
  while(t>=s && *t!='\\') t--;
  t++;
  return t;
}

//get exe file name of process pid
void getProcessName(DWORD pid, char *&exe)
{
  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(PROCESSENTRY32);
  HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  if(h!=(HANDLE)-1){
    Process32First(h,&pe);
    do{
      if(pe.th32ProcessID==pid){
        cpStr(exe,cutPath(pe.szExeFile));
        CloseHandle(h);
        return; 
      }
    }while(Process32Next(h,&pe));
    CloseHandle(h);
  }
  cpStr(exe,"");
}

void copyToClipboard(char *s)
{
  HGLOBAL hmem;
  char *ptr;
  DWORD len=strlen(s)+1;
  
  if(OpenClipboard(0)){
    if(EmptyClipboard()){
      if((hmem=GlobalAlloc(GMEM_DDESHARE, isWin9X ? len : 2*len))!=0){
        if((ptr=(char*)GlobalLock(hmem))!=0){
          if(isWin9X){
            strcpy(ptr,s);
          }else{
            MultiByteToWideChar(CP_ACP,0,s,-1,(WCHAR*)ptr,len);
          }
          GlobalUnlock(hmem);
          SetClipboardData(isWin9X ? CF_TEXT : CF_UNICODETEXT, hmem);
        }else{
          GlobalFree(hmem);
        }
      }
    }
    CloseClipboard();
  }
}

//-------------------------------------------------------------------------
void initList()
{
  SendMessage(listBox, LB_SETCOUNT, count, 0);
  SendMessage(listBox, LB_SETTOPINDEX, count-1, 0);
  InvalidateRect(listBox,0,TRUE);
}

int ind(int r)
{
  int i;
  i=first+r;
  if(i>=Mitem) i-=Mitem;
  return i;
}

//add new item to the end of list
void add(WPARAM wP, HWND wnd, int action)
{
  TspyItem *t;
  DWORD pid;
  char buf[256];
  
  if(!GetWindowThreadProcessId(wnd,&pid)) return;
  if(pid==GetCurrentProcessId()) return;
  if(count){
    t=&A[ind(count-1)];
    if(t->wP==wP && t->pid==pid) return;
  } 
  t=&A[ind(count)];
  if(count==Mitem){
    first++;
    if(first==Mitem) first=0;
  }else{
    count++;
  }
  t->wnd=wnd;
  t->wP=wP;
  t->pid=pid;
  t->action=action;
  GetClassName(wnd,buf,sizeof(buf));
  cpStr(t->wclass,buf);
  GetWindowText(wnd,buf,sizeof(buf));
  cpStr(t->title,buf);
  getProcessName(pid,t->exe);
  initList();
}

//set or cancel hooks
void startstop()
{
  if(!hook){
#ifdef _WIN64
    HOOKPROC hproc = (HOOKPROC)GetProcAddress(klib,"CallWndProc");
#else
    HOOKPROC hproc = (HOOKPROC)GetProcAddress(klib,"_CallWndProc@12");
#endif
    hook= SetWindowsHookEx(WH_CALLWNDPROC,hproc,klib,0);
#ifdef _WIN64
    hproc = (HOOKPROC)GetProcAddress(klib,"GetMsgProc");
#else
    hproc = (HOOKPROC)GetProcAddress(klib,"_GetMsgProc@12");
#endif
    hookG= SetWindowsHookEx(WH_GETMESSAGE,hproc,klib,0);
    if(!hook || !hookG){
      msg("SetWindowsHookEx failed"); 
    }
    if(hook) SetDlgItemText(hWin,301,"Stop");
  }else{
    UnhookWindowsHookEx(hook);
    hook=0;
    UnhookWindowsHookEx(hookG);
    hookG=0;
    SetDlgItemText(hWin,301,"Start");
  }
}

void getControlPos(int id, HWND hDlg, POINT *p)
{
  RECT rc;
  GetWindowRect(GetDlgItem(hDlg,id),&rc);
  ScreenToClient(hDlg,(POINT*)&rc);
  p->x= rc.left;
  p->y= rc.top;
}

void calcColWidth(HWND hWnd)
{
  int i,x;
  RECT rc;
  POINT p;
  static const int colWidthRel[] = {15,22,25,38};
  
  GetClientRect(listBox,&rc);
  //calculate columns width
  for(i=0; i<Ncolumns; i++){
    colWidth[i]= colWidthRel[i]*rc.right/100;
  }
  //set header position
  getControlPos(200,hWnd,&p);
  HDWP h = BeginDeferWindowPos(Ncolumns);
  for(i=0,x=p.x; i<Ncolumns; x+=colWidth[i++]){
    DeferWindowPos(h,GetDlgItem(hWnd,200+i),0,x,p.y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
  }
  EndDeferWindowPos(h);
}

void command(int cmd)
{
  if(!count) return;
  int i = SendMessage(listBox,LB_GETCURSEL,0,0);
  if(i==LB_ERR) i=count-1;
  TspyItem *item = &A[ind(i)];
  char *s,buf[16];
  switch(cmd){
  case 200:
    sprintf(s=buf, "%d ", item->wP);
    break;
  case 201:
    s = item->exe;
    break;
  case 202:
    s = item->wclass;
    break;
  case 203:
    s = item->title;
    break;
  default:
    return;
  }
  copyToClipboard(s);
  HWND w;
  if((w=FindWindow("PlasHotKey",0))!=0){
    SendMessage(w,WM_USER+2377,0,0);
  }
}
//-------------------------------------------------------------------------
//procedure for main window
BOOL CALLBACK MainWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
  int i,cmd;
  RECT rc;
  TspyItem *t;
  POINT p;
  
  switch (mesg) {
  case WM_INITDIALOG:
    listBox=GetDlgItem(hWnd,101);
    calcColWidth(hWnd);
    break;
  case WM_USER+60:
    SetWindowLongPtr(hWnd,DWLP_MSGRESULT,(LRESULT)hook);
    break;
  case WM_USER+62:
    SetWindowLongPtr(hWnd,DWLP_MSGRESULT,(LRESULT)hookG);
    break;
  case WM_USER+61:
    add(LOWORD(wP),(HWND)lP, A_cmd);
    break;
  case WM_USER+63:
    add(LOWORD(wP),(HWND)lP, A_icon);
    break;
    
  case WM_DRAWITEM:
    {
      DRAWITEMSTRUCT *lpdis = (LPDRAWITEMSTRUCT) lP;
      i=lpdis->itemID;
      if(i<0 || i>=count) break;
      t= &A[ind(i)];
      bool sel= lpdis->itemState & ODS_SELECTED;
      //set color and fill background
      COLORREF bk= sel ? 0xff0000:0xffffff;
      HBRUSH br=CreateSolidBrush(bk);
      SetTextColor(lpdis->hDC, sel ? 0xffffff:0);
      SetBkMode(lpdis->hDC, TRANSPARENT);
      FillRect(lpdis->hDC, &lpdis->rcItem, br);
      DeleteObject(br);
      //prepare rectangle
      rc.top= lpdis->rcItem.top;
      rc.bottom= lpdis->rcItem.bottom;
      rc.right=3;
      
      for(i=0; i<Ncolumns; i++){
        rc.left=rc.right;
        rc.right+=colWidth[i];
        char *s;
        char buf[32];
        switch(i){
        default:
          sprintf(s=buf, "%s%d", 
            (t->action==A_icon) ? "ID: " : "", t->wP);
          break;
        case 1:
          s=t->exe;
          break;
        case 2:
          s=t->wclass;
          break;
        case 3:
          s=t->title;
          break;
        }
        if(s) DrawText(lpdis->hDC, s, (int)strlen(s), &rc, 
          DT_LEFT|DT_END_ELLIPSIS|DT_NOPREFIX);
      }
      break;
    }

  case WM_SIZE:
    if(!lP) break;
    getControlPos(101,hWnd,&p);
    GetClientRect(hWnd,&rc);
    SetWindowPos(listBox,0, p.x,p.y, rc.right-2*p.x, rc.bottom-p.y-p.x, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    calcColWidth(hWnd);
    InvalidateRect(listBox,0,TRUE);
    break;
  case WM_GETMINMAXINFO:
    ((MINMAXINFO FAR*) lP)->ptMinTrackSize.x = 545;
    ((MINMAXINFO FAR*) lP)->ptMinTrackSize.y = 280;
    break;

  case WM_COMMAND:
    cmd=LOWORD(wP);
    if(cmd>=200 && cmd<200+Ncolumns){
      command(cmd);
      break;
    }
    switch(cmd){
    case 301: 
      startstop();
      break;
    case 303:
      first=0; 
      count=0;
      initList();
      break;
    }
    break;
    case WM_CLOSE:
      DestroyWindow(hWnd);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return FALSE;
  }
  return TRUE;
}
//-------------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR , int cmdShow)
{
  MSG mesg;
  WNDCLASS wc;
  
  OSVERSIONINFO v;
  v.dwOSVersionInfoSize= sizeof(OSVERSIONINFO);
  GetVersionEx(&v);
  isWin9X = v.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS;
  
#ifdef _WIN64
  klib=LoadLibrary("hook64.dll");
  if(!klib){ msg("Cannot find hook64.dll"); return 1; }
#else
  klib=LoadLibrary("hook.dll");
  if(!klib){ msg("Cannot find hook.dll"); return 1; }
#endif
  //register class
  wc.style=0;
  wc.lpfnWndProc=(WNDPROC)DefDlgProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=DLGWINDOWEXTRA;
  wc.hInstance=hInstance;
  wc.hIcon=LoadIcon(0,IDI_APPLICATION);
  wc.hCursor=LoadCursor(0, IDC_ARROW);
  wc.hbrBackground=(HBRUSH)COLOR_BTNFACE;
  wc.lpszMenuName=NULL;
  wc.lpszClassName="PlasSpy";
  if(!hPrev && !RegisterClass(&wc)){ msg("RegisterClass error"); return 2; }
  //create main window
  hWin = CreateDialog(hInstance,MAKEINTRESOURCE(100),0,(DLGPROC)MainWndProc);
  if(!hWin){ msg("CreateDialog error"); return 3; }
  ShowWindow(hWin,cmdShow);
  PostMessage(hWin,WM_COMMAND,301,0);
  
  while(GetMessage(&mesg, NULL, 0, 0)>0){
    if(!IsDialogMessage(hWin,&mesg)){
      TranslateMessage(&mesg);
      DispatchMessage(&mesg);
    }
  }
  if(hook) startstop();
  FreeLibrary(klib);
  return 0;
}
