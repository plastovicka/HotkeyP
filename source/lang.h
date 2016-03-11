/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef langH
#define langH

extern TCHAR lang[64];

TCHAR *lng(int i, char *s);
void initLang();
int setLang(int cmd);
void loadLang();
HMENU loadMenu(char *name, int *subId);
void loadMenu(HWND hwnd, char *name, int *subId);
void changeDialog(HWND &wnd, int x, int y, LPCTSTR dlgTempl, DLGPROC dlgProc);
void setDlgTexts(HWND hDlg);
void setDlgTexts(HWND hDlg, int id);
void getExeDir(TCHAR *fn, TCHAR *e);
TCHAR const *cutPath(TCHAR const *s); // zef: made const correct
inline TCHAR *cutPath(TCHAR * s) { return const_cast<TCHAR *>(cutPath(const_cast<TCHAR const *>(s))); }

extern void langChanged();
extern void msg(TCHAR *text, ...);
void msglng(int id, char *text, ...);
extern HINSTANCE inst;

#endif
