/*
 (C) 2004-2006  Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  

#define _CRT_SECURE_NO_DEPRECATE 1
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#pragma hdrstop

HWND hWin1,hWin2;
HHOOK hookK,hookM,hookW,hookG;
int lock;

//-------------------------------------------------------------------------
void getW1()
{
 if(!IsWindow(hWin1)) hWin1=FindWindow("PlasHotkey",0);
}

HHOOK getH1(int cmd)
{
 getW1();
 return (HHOOK)SendMessage(hWin1,cmd,0,0);
}

extern "C" __declspec(dllexport)
LRESULT CALLBACK MouseProc95(int code, WPARAM wP, LPARAM lP)
{
 if(!hookM) hookM=getH1(WM_USER+51);
 if(wP>=160 && wP<=173) wP+=512-160;
 if(wP!=WM_MOUSEMOVE && code==HC_ACTION){
   getW1();
   if(SendMessage(hWin1,WM_USER+53,wP,((MOUSEHOOKSTRUCT*)lP)->dwExtraInfo<<5)) return 1;
 }
 return CallNextHookEx(hookM, code, wP, lP);
}

extern "C" __declspec(dllexport)
LRESULT CALLBACK KeyboardProc95(int code, WPARAM wP, LPARAM lP)
{
 if(!hookK) hookK=getH1(WM_USER+54);
 if(code==HC_ACTION){
   getW1();
   if(SendMessage(hWin1,WM_USER+57,wP,lP)) return 1;
 }
 return CallNextHookEx(hookK, code, wP, lP);
}

//-------------------------------------------------------------------------

void getW2()
{
 if(!IsWindow(hWin2)) hWin2=FindWindow("PlasSpy",0);
}

HHOOK getH2(int cmd)
{
 getW2();
 return (HHOOK)SendMessage(hWin2,cmd,0,0);
}

extern "C" __declspec(dllexport)
LRESULT CALLBACK CallWndProc(int code, WPARAM wP, LPARAM lP)
{
 if(!hookW){
   if(lock) return 0;
   lock++;
   hookW=getH2(WM_USER+60);
   lock--;
 }
 CWPSTRUCT *c= (CWPSTRUCT*)lP;
 if(c->message==WM_COMMAND && code==HC_ACTION &&
   HIWORD(c->wParam)<=1 && LOWORD(c->wParam)>9){
   getW2();
   PostMessage(hWin2,WM_USER+61,c->wParam,(LPARAM)c->hwnd);
 }
 if(c->lParam==WM_LBUTTONDOWN && code==HC_ACTION &&
   c->message>=WM_USER){
   getW2();
   PostMessage(hWin2,WM_USER+63,c->wParam,(LPARAM)c->hwnd);
 }
 return CallNextHookEx(hookW, code, wP, lP);
}

extern "C" __declspec(dllexport)
LRESULT CALLBACK GetMsgProc(int code, WPARAM wP, LPARAM lP)
{
 if(!hookG) hookG=getH2(WM_USER+62);
 MSG *m= (MSG*)lP;
 if(m->message==WM_COMMAND && code==HC_ACTION && wP==PM_REMOVE && 
   HIWORD(m->wParam)<=1 && LOWORD(m->wParam)>9){
   getW2();
   PostMessage(hWin2,WM_USER+61,m->wParam,(LPARAM)m->hwnd);
 }
 return CallNextHookEx(hookG, code, wP, lP);
}
//-------------------------------------------------------------------------

const char *subkey="Software\\Petr Lastovicka\\hotkey";

void saveIcons(HWND w)
{
  int n = ListView_GetItemCount(w);
  char *buf,*s;
  s = buf = new char[n*(256+24)];
  for(int i=0; i<n; i++){
    POINT pt;
    ListView_GetItemPosition(w,i,&pt);
    s += sprintf(s, "%d %d ", pt.x, pt.y);
    LV_ITEM lvi;
    lvi.iSubItem=0;
    lvi.cchTextMax=256;
    lvi.pszText=s;
    s += SendMessage(w,LVM_GETITEMTEXT,i,(LPARAM)&lvi);
    *s++ = '|';
  }
  HKEY key;
  if(RegCreateKey(HKEY_CURRENT_USER, subkey, &key)==ERROR_SUCCESS)
  {
    RegSetValueEx(key, "desktopIcons", 0,REG_SZ,
      (BYTE *)buf, (DWORD)(s-buf));
    RegCloseKey(key);
  }
  delete[] buf;
}

void loadIcons(HWND w)
{
  HKEY key;
  DWORD d;
  if(RegOpenKeyEx(HKEY_CURRENT_USER,subkey,0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){
    if(RegQueryValueEx(key,"desktopIcons",0,0,0,&d)==ERROR_SUCCESS && d<10000000){
      char *buf,*s,*e;
      s = buf = new char[d+1];
      s[d]=0;
      RegQueryValueEx(key,"desktopIcons",0,0,(BYTE*)s, &d);
      while(*s){
        int x,y;
        x= strtol(s,&s,10);
        if(*s==' ') s++;
        y= strtol(s,&s,10);
        if(*s==' ') s++;
        e=strchr(s,'|');
        if(!e) break;
        *e=0;
        LVFINDINFO lfi;
        lfi.flags= LVFI_STRING;
        lfi.psz= s;
        int i = ListView_FindItem(w, -1, &lfi);
        if(i>=0) ListView_SetItemPosition(w,i,x,y);
        s=e+1;
      }
      delete[] buf;
    }
    RegCloseKey(key);
  }  
}

extern "C" __declspec(dllexport)
LRESULT CALLBACK CallWndProcD(int code, WPARAM wP, LPARAM lP)
{
  if(code==HC_ACTION && wP==PM_REMOVE){
    CWPSTRUCT *m= (CWPSTRUCT*)lP;
    if(m->message==WM_USER+47348) saveIcons(m->hwnd);
    if(m->message==WM_USER+47349) loadIcons(m->hwnd);
  }
  return CallNextHookEx(0, code, wP, lP);
}

//-------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE, DWORD , LPVOID)
{
 return TRUE;
}

int main() {}
