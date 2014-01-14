/*
 (C) Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  
#ifndef langH
#define langH

extern char lang[64];

char *lng(int i, char *s);
void initLang();
int setLang(int cmd);
void loadLang();
HMENU loadMenu(char *name, int *subId);
void loadMenu(HWND hwnd, char *name, int *subId);
void changeDialog(HWND &wnd, int x,int y,LPCTSTR dlgTempl, DLGPROC dlgProc);
void setDlgTexts(HWND hDlg);
void setDlgTexts(HWND hDlg, int id);
void getExeDir(char *fn, char *e);
char const *cutPath(char const *s); // zef: made const correct
inline char *cutPath( char * s ) { return const_cast<char * >(cutPath( const_cast<char const * >(s) )); }

extern void langChanged();
extern void msg(char *text, ...);
void msglng(int id, char *text, ...);
extern HINSTANCE inst;

#endif
