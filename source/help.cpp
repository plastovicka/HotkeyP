/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"

bool chmUnlocked=false;
const TCHAR *helpClass = _T("HotKeyP_Help");
//-------------------------------------------------------------------------
//web browsers attach Zone.Identifier to block every downloaded file
//blocked CHM file does not show any content, just an empty window
//we must delete that alternate data stream
//annoying UAC dialog is required if the app is installed in the Program Files folder
bool unblockHelp()
{
	if (!chmUnlocked) {
		TCHAR buf[MAX_PATH + 24];
		getExeDir(buf, lng(13, "help.chm"));
		_tcscat(buf, _T(":Zone.Identifier:$DATA"));
		if (!DeleteFile(buf) && GetLastError() == ERROR_ACCESS_DENIED) return true;
		chmUnlocked = true;
	}
	return false;
}
//-------------------------------------------------------------------------
bool showHelp(int anchorId)
{
	HWND hHidden = FindWindow(helpClass, 0);
	if(!hHidden){
		//save current language setting
		writeini();
		//run another hotkeyp.exe process
		TCHAR buf[MAX_PATH];
		GetModuleFileName(0, buf, 192);
		if((INT_PTR)ShellExecute(0, unblockHelp() ? _T("runas") : 0, buf, _T("--htmlhelp"), 0, SW_SHOWNORMAL)<=32){
			return false;
		}
		//find hidden window which is used to receive commands
		for(int i=0; !hHidden; i++){
			hHidden = FindWindow(helpClass, 0);
			if(i>50) return false;
			Sleep(100);
		}
	}
	//send message to show a requested topic
	HWND hHH = (HWND)SendMessage(hHidden, WM_USER+4370, anchorId, 0);
	if(!hHH) return false;
	//activate help window
	SetForegroundWindow(hHH);
	return true;
}
//-------------------------------------------------------------------------
LRESULT helpProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	static HWND hHH;
	static int cnt;

	TCHAR buf[MAX_PATH], buf2[MAX_PATH+24];
	if(mesg==WM_USER+4370){
		getExeDir(buf, lng(13, "help.chm"));
		_stprintf(buf2, _T("%.99s#%d"), lng(30, "main.htm"), (int)wP);
		hHH = HtmlHelpW(0, buf, 0, (!wP) ? 0 : (DWORD_PTR)buf2);
		return (LPARAM)hHH;
	}
	if(mesg==WM_TIMER && wP==11){
		if(!hHH ? ++cnt>5 : !IsWindow(hHH)) DestroyWindow(hWnd);
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
	unblockHelp();

	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc=(WNDPROC)helpProc;
	wc.hInstance=inst;
	wc.lpszClassName=helpClass;
	if(!RegisterClass(&wc)) return 2;

	HWND hHidden = CreateWindowEx(0, helpClass, _T("Help"), WS_POPUP, 0, 0, 100, 100, 0, 0, inst, 0);
	if(!hHidden) return 3;
	SetTimer(hHidden, 11, 1000, 0);

	while(GetMessage(&mesg, NULL, 0, 0)>0){
		TranslateMessage(&mesg);
		DispatchMessage(&mesg);
	}
	return 0;
}
