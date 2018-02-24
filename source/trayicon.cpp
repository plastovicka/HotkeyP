/*
	(C) 2005-2017 Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.
	*/

#include "hdr.h"
#pragma hdrstop

extern HINSTANCE inst;
extern HWND hWin;
extern const TCHAR *title;

//---------------------------------------------------------------------------
BOOL addTrayIcon(const TCHAR *tooltip, HICON icon, UINT id)
{
	NOTIFYICONDATA myNID;
	myNID.cbSize = sizeof(NOTIFYICONDATA);
	myNID.hWnd = hWin;
	myNID.uID = id;
	myNID.hIcon = icon;
	myNID.uCallbackMessage = WM_USER+55;
	myNID.uFlags = NIF_ICON|NIF_MESSAGE;
	if(tooltip){
		myNID.uFlags |= NIF_TIP;
		lstrcpyn(myNID.szTip, tooltip, 64);
	}
	return Shell_NotifyIcon(NIM_ADD, &myNID);
}

void addTrayIcon()
{
	for(int i=0; i<9; i++)
	{
		if(addTrayIcon(title, (HICON)GetClassLongPtr(hWin, GCLP_HICONSM), 1)) break;
		Sleep(1000);
	};
}

//---------------------------------------------------------------------------
void modifyTrayIcon(HWND hWnd, int rsrcId)
{
	NOTIFYICONDATA myNID;
	myNID.cbSize = sizeof(NOTIFYICONDATA);
	myNID.hWnd = hWnd;
	myNID.uID = 1;
	myNID.hIcon = (HICON)LoadImage(inst, MAKEINTRESOURCE(rsrcId), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	myNID.uFlags = NIF_ICON;
	Shell_NotifyIcon(NIM_MODIFY, &myNID);
	DestroyIcon(myNID.hIcon);
}
//---------------------------------------------------------------------------
void deleteTrayIcon(UINT id)
{
	NOTIFYICONDATA myNID;
	myNID.cbSize = sizeof(NOTIFYICONDATA);
	myNID.hWnd = hWin;
	myNID.uID = id;
	myNID.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &myNID);
}

void deleteTrayIcon()
{
	deleteTrayIcon(1);
}
//---------------------------------------------------------------------------
BOOL hideIcon(HWND w, UINT id, int hide)
{
	NOTIFYICONDATA myNID;
	myNID.cbSize = sizeof(NOTIFYICONDATA);
	myNID.hWnd = w;
	myNID.uID = id;
	myNID.uFlags = NIF_STATE;
	myNID.dwState = hide ? NIS_HIDDEN : 0;
	myNID.dwStateMask = NIS_HIDDEN;
	return Shell_NotifyIcon(NIM_MODIFY, &myNID);
}
