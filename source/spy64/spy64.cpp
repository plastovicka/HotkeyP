/*
	(C) 2016  Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/
#include <windows.h>
#pragma hdrstop

HMODULE klib;
HHOOK hook, hookG;
//-------------------------------------------------------------------------
void msg(char *text)
{
	MessageBox(0, text, "Spy", MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
}

//release hooks
void unhook()
{
	if(hook){
		UnhookWindowsHookEx(hook); 
		hook=0;
	}
	if(hookG){
		UnhookWindowsHookEx(hookG);
		hookG=0;
	}
}

//procedure for hidden window
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT mesg, WPARAM wP, LPARAM lP)
{
	switch(mesg) {
		case WM_USER+60:
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LRESULT)hook);
			break;
		case WM_USER+62:
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LRESULT)hookG);
			break;
		case WM_USER+64:
		{
			//create hooks
			HOOKPROC hproc = (HOOKPROC)GetProcAddress(klib, "CallWndProc");
			hook= SetWindowsHookEx(WH_CALLWNDPROC, hproc, klib, 0);
			hproc = (HOOKPROC)GetProcAddress(klib, "GetMsgProc");
			hookG= SetWindowsHookEx(WH_GETMESSAGE, hproc, klib, 0);
			if(!hook || !hookG){ msg("SetWindowsHookEx error"); }
			break;
		}
		case WM_USER+65:
			unhook();
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, mesg, wP, lP);
	}
	return 0;
}
//-------------------------------------------------------------------------
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdLine, int)
{
	MSG mesg;
	WNDCLASS wc;
	HWND hWin;

	//hook64.exe should be started only from spy.exe
	if(strcmp(cmdLine, "-s")) return 1;

	klib=LoadLibrary("hook64.dll");
	if(!klib){ msg("Cannot find hook64.dll"); return 1; }

	//register class
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc=(WNDPROC)MainWndProc;
	wc.hInstance=hInstance;
	wc.lpszClassName="PlasSpy64";
	if(!hPrev && !RegisterClass(&wc)){ msg("RegisterClass error"); return 2; }

	//create main window
	hWin = CreateWindow("PlasSpy64", "Spy64", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	if(!hWin){ msg("CreateWindow error"); return 3; }


	while(GetMessage(&mesg, NULL, 0, 0)>0){
		DispatchMessage(&mesg);
	}

	unhook();
	FreeLibrary(klib);
	return 0;
}
