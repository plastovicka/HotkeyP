/*
 (C) 2008  Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  
#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"

bool chmUnlocked=false;
const char *helpClass = "HotKeyP_Help";
//-------------------------------------------------------------------------
bool showHelp(int anchorId)
{
  HWND hHidden = FindWindow(helpClass,0);
  if(!hHidden){
    //save current language setting
    writeini(); 
    //run another hotkeyp.exe process
    char buf[MAX_PATH];
    GetModuleFileName(0,buf,192);
    if((int)ShellExecute(0,0,buf,"--htmlhelp",0,SW_SHOWNORMAL)<=32){
      return false;
    }
    //find hidden window which is used to receive commands
    for(int i=0; !hHidden; i++){
      hHidden = FindWindow(helpClass,0);
      if(i>50) return false;
      Sleep(100);
    }
  }
  //send message to show a requested topic
  HWND hHH = (HWND) SendMessage(hHidden,WM_USER+4370,anchorId,0);
  if(!hHH) return false;
  //activate help window
  SetForegroundWindow(hHH);
  return true;
}
//-------------------------------------------------------------------------
LRESULT helpProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	static HWND hHH;

  char buf[MAX_PATH],buf2[MAX_PATH+24];
  if(mesg==WM_USER+4370){
    getExeDir(buf,lng(13,"help.chm"));
    if(!chmUnlocked){
      sprintf(buf2,"%s:Zone.Identifier:$DATA",buf);
      DeleteFile(buf2);
      chmUnlocked=true;
    }
    sprintf(buf2,"%.99s#%d",lng(30,"main.htm"), wP);
    hHH = HtmlHelp(0,buf,0, (!wP) ? 0 : (DWORD_PTR)buf2);
    SetTimer(hWnd,11,1000,0);
    return (LPARAM) hHH;
  }
  if(mesg==WM_TIMER && wP==11){
    if(!IsWindow(hHH)) DestroyWindow(hWnd);
  }
  if(mesg==WM_DESTROY) PostQuitMessage(0);

  return DefWindowProc(hWnd, mesg, wP, lP);
}
//-------------------------------------------------------------------------
int helpProcess()
{
  MSG mesg;
  WNDCLASS wc;

  readini();
  initLang();

  ZeroMemory(&wc,sizeof(wc));
  wc.lpfnWndProc=(WNDPROC)helpProc;
  wc.hInstance=inst;
  wc.lpszClassName=helpClass;
  if(!RegisterClass(&wc)) return 2;

	HWND hHidden = CreateWindowEx(0,helpClass,"Help",WS_POPUP,0,0,100,100,0,0,inst,0);
  if(!hHidden) return 3;

  while(GetMessage(&mesg, NULL, 0, 0)>0){
    TranslateMessage(&mesg);
    DispatchMessage(&mesg);
  }
  return 0;
}
