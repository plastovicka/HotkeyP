#ifndef trayiconH
#define trayiconH

extern int trayicon;

void addTrayIcon(char *tooltip, HICON icon, UINT id);
void deleteTrayIcon(UINT id);
void modifyTrayIcon(HWND hWin, int rsrcId);
BOOL hideIcon(HWND w, UINT id, int hide);

void addTrayIcon();
void deleteTrayIcon();

#endif
