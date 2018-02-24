/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef trayiconH
#define trayiconH

extern int trayicon;

BOOL addTrayIcon(const TCHAR *tooltip, HICON icon, UINT id);
void deleteTrayIcon(UINT id);
void modifyTrayIcon(HWND hWin, int rsrcId);
BOOL hideIcon(HWND w, UINT id, int hide);

void addTrayIcon();
void deleteTrayIcon();

#endif
