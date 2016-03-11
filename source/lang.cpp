/*
	(C) 2002-2014 Petr Lastovicka

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	Usage:
	read lang from registry, call initLang() and langChanged()
	paste setLang(cmd) to WM_COMMAND
	paste setDlgTexts(hDlg,titleId) to WM_INITDIALOG
	implement langChanged() - reload menu, file filters, invalidate, reload not modal dialogs
	*/
#include "hdr.h"
#pragma hdrstop
#include "lang.h"
#include "hotkeyp.h"

#ifndef MAXLNGSTR
#define MAXLNGSTR 1500
#endif

#ifndef MAXLNG_PARALLEL
#define MAXLNG_PARALLEL 16
#endif

extern char *cmdNames[];

//---------------------------------------------------------------------------
const int MAXLANG=60;
TCHAR lang[64];         //current language name
TCHAR *langFile;        //file content (\n replaced with \0)
TCHAR *lngstr[MAXLNGSTR];   //pointers to lines in langFile
TCHAR *lngNames[MAXLANG+1]; //all found languages names
static TCHAR *recycl[MAXLNG_PARALLEL]; //temporary buffers for Unicode strings
extern bool isWin9X;
//-------------------------------------------------------------------------
#define sizeA(A) (sizeof(A)/sizeof(*A))

TCHAR *lng(int i, char *s)
{
	if(i>=0 && i<sizeA(lngstr) && lngstr[i]) return lngstr[i];
#ifdef UNICODE
	if(!s) return 0;
	static int ind;
	int len=strlen(s)+1;
	TCHAR *w= new TCHAR[len];
	MultiByteToWideChar(CP_ACP, 0, s, -1, w, len);
	delete[] recycl[ind];
	recycl[ind]=w;
	ind++;
	if(ind==sizeA(recycl)) ind=0;
	return w;
#else
	return s;
#endif
}

#ifdef UNICODE
WCHAR *lng(int i, WCHAR *s)
{
	if(i>=0 && i<sizeA(lngstr) && lngstr[i]) return lngstr[i];
	return s;
}
#endif

//return pointer to name after a path
TCHAR const *cutPath(TCHAR const *s) // zef: made const correct
{
	TCHAR const *t;
	t=_tcschr(s, 0);
	while(t>=s && *t!='\\') t--;
	t++;
	return t;
}

//concatenate current directory and e, write result to fn
void getExeDir(TCHAR *fn, TCHAR *e)
{
	GetModuleFileName(0, fn, 192);
	_tcscpy(cutPath(fn), e);
}
//-------------------------------------------------------------------------
static BOOL CALLBACK enumControls(HWND hwnd, LPARAM)
{
	int i=GetDlgCtrlID(hwnd);
	if((i>=300 && i<sizeA(lngstr) || i<11 && i>0) && lngstr[i]){
		SetWindowText(hwnd, lngstr[i]);
	}
	return TRUE;
}

void setDlgTexts(HWND hDlg)
{
	EnumChildWindows(hDlg, (WNDENUMPROC)enumControls, 0);
}

void setDlgTexts(HWND hDlg, int id)
{
	TCHAR *s=lng(id, (char*)0);
	if(s) SetWindowText(hDlg, s);
	setDlgTexts(hDlg);
}

//reload not modal dialog or create new dialog at position x,y
void changeDialog(HWND &wnd, int x, int y, LPCTSTR dlgTempl, DLGPROC dlgProc)
{
	HWND a, w;

	a=GetActiveWindow();
	w=CreateDialog(inst, dlgTempl, 0, dlgProc);
	if(wnd){
		RECT rc;
		GetWindowRect(wnd, &rc);
		MoveWindow(w, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		if(IsWindowVisible(wnd)) ShowWindow(w, SW_SHOW);
		DestroyWindow(wnd);
	}
	else{
		SetWindowPos(w, 0, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	}
	wnd=w;
	if(a) SetActiveWindow(a);
}
//-------------------------------------------------------------------------
static int *subPtr;

//recurse through menu and change all item names
static void fillPopup(HMENU h)
{
	int i, id, j;
	TCHAR *s, *a;
	UINT f;
	HMENU sub;
	MENUITEMINFO mii;
	WCHAR w[21];

	for(i=GetMenuItemCount(h)-1; i>=0; i--){
		id=GetMenuItemID(h, i);
		if(id==29999){
			for(j=0; (a=lngNames[j])!=0; j++){
				f=MF_BYPOSITION|(_tcsicmp(a, lang) ? 0 : MF_CHECKED);
				w[0]=0;
				if(!isWin9X){
					size_t len = _tcslen(a);
					// L"\x10c" does not compile correctly in Microsoft Visual C++ 6.0
					if(len==5 && !_tcsnicmp(a+1, _T("esky"), 4)){ wcscpy(w, L"0esky"); w[0]=0x10c; }
					if(len==7 && !_tcsnicmp(a, _T("Espa"), 4)){ wcscpy(w, L"Espa0ol"); w[4]=0xf1; }
					if(len==20 && !_tcsnicmp(a, _T("Portugu"), 7)){ wcscpy(w, L"Portugu0s brasileiro"); w[7]=0xea; }
				}
				if(w[0]) InsertMenuW(h, 0xFFFFFFFF, f, 30000+j, w);
				else InsertMenu(h, 0xFFFFFFFF, f, 30000+j, a);
			}
			DeleteMenu(h, 0, MF_BYPOSITION);
		}
		else{
			if(id<0 || id>=0xffffffff){
				sub=GetSubMenu(h, i);
				if(sub){
					id=*subPtr++;
					fillPopup(sub);
				}
			}
			s=lng(id, (char*)0);
			if(!s && id>=1000 && id<1500 && cmdNames){
				convertA2T(cmdNames[id-1000], name);
				s=name;
			}
			if(s){
				mii.cbSize=sizeof(MENUITEMINFO);
				mii.fMask=MIIM_TYPE|MIIM_STATE;
				mii.fType=MFT_STRING;
				mii.fState=MFS_ENABLED;
				mii.dwTypeData=s;
				mii.cch= (UINT)_tcslen(s);
				SetMenuItemInfo(h, i, TRUE, &mii);
			}
		}
	}
}

//load menu from resources
//subId are string numbers for submenus
HMENU loadMenu(char *name, int *subId)
{
	HMENU hMenu= LoadMenuA(inst, name);
	subPtr=subId;
	fillPopup(hMenu);
	return hMenu;
}

void loadMenu(HWND hwnd, char *name, int *subId)
{
	if(!hwnd) return;
	HMENU m= GetMenu(hwnd);
	SetMenu(hwnd, loadMenu(name, subId));
	DestroyMenu(m);
}
//-------------------------------------------------------------------------
static void parseLng()
{
	TCHAR *s, *d, *e;
	int id, err=0, line=1;

	for(s=langFile; *s; s++){
		if(*s==';' || *s=='#' || *s=='\n' || *s=='\r'){
			//comment
		}
		else{
			id=(int)_tcstol(s, &e, 10);
			if(s==e){
				if(!err) msglng(755, "Error in %s\nLine %d", lang, line);
				err++;
			}
			else if(id<0 || id>=sizeA(lngstr)){
				if(!err) msglng(756, "Error in %s\nMessage number %d is too big", lang, id);
				err++;
			}
			else if(lngstr[id]){
				if(!err) msglng(757, "Error in %s\nDuplicated number %d", lang, id);
				err++;
			}
			else{
				s=e;
				while(*s==' ' || *s=='\t') s++;
				if(*s=='=') s++;
				lngstr[id]=s;
			}
		}
		for(d=s; *s!='\n' && *s!='\r'; s++){
			if(*s=='\\'){
				s++;
				if(*s=='\r'){
					line++;
					if(s[1]=='\n') s++;
					continue;
				}
				else if(*s=='\n'){
					line++;
					continue;
				}
				else if(*s=='0'){
					*s='\0';
				}
				else if(*s=='n'){
					*s='\n';
				}
				else if(*s=='r'){
					*s='\r';
				}
				else if(*s=='t'){
					*s='\t';
				}
			}
			*d++=*s;
		}
		if(*s!='\r' || s[1]!='\n') line++;
		*d='\0';
	}
}
//-------------------------------------------------------------------------
void scanLangDir()
{
	int n;
	HANDLE h;
	WIN32_FIND_DATA fd;
	TCHAR buf[256];

	lngNames[0]=_T("English");
	getExeDir(buf, _T("language\\*.lng"));
	h = FindFirstFile(buf, &fd);
	if(h!=INVALID_HANDLE_VALUE){
		n=1;
		do{
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
				int len= (int)_tcslen(fd.cFileName)-4;
				if(len>0){
					cpStr(lngNames[n], fd.cFileName);
					lngNames[n][len] = 0;
					n++;
				}
			}
		} while(FindNextFile(h, &fd) && n<MAXLANG);
		FindClose(h);
	}
}
//-------------------------------------------------------------------------
void loadLang()
{
	memset(lngstr, 0, sizeof(lngstr));
	TCHAR buf[256];
	GetModuleFileName(0, buf, sizeA(buf)-static_cast<int>(_tcslen(lang))-14);
	_tcscpy(cutPath(buf), _T("language\\"));
	TCHAR *fn=_tcschr(buf, 0);
	_tcscpy(fn, lang);
	_tcscat(buf, _T(".lng"));
	HANDLE f=CreateFile(buf, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(f!=INVALID_HANDLE_VALUE){
		DWORD len=GetFileSize(f, 0);
		if(len>10000000){
			msglng(753, "File %s is too long", fn);
		}
		else{
			delete[] langFile;
			char *langFileA= new char[len+3];
			DWORD r;
			ReadFile(f, langFileA, len, &r, 0);
			if(r<len){
				msglng(754, "Error reading file %s", fn);
			}
			else{
				UINT cp=CP_ACP;
				if(langFileA[0]=='#' && langFileA[1]=='C' && langFileA[2]=='P'){
					cp=atoi(langFileA+3);
				}
#ifdef UNICODE
				langFile= new TCHAR[len+3];
				MultiByteToWideChar(cp, 0, langFileA, len, langFile, len);
				delete[] langFileA;
#else
				langFile= langFileA;
#endif
				langFile[len]='\n';
				langFile[len+1]='\n';
				langFile[len+2]='\0';
				parseLng();
			}
		}
		CloseHandle(f);
	}
}
//---------------------------------------------------------------------------
int setLang(int cmd)
{
	if(cmd>=30000 && cmd<30000+MAXLANG && lngNames[cmd-30000]){
		_tcscpy(lang, lngNames[cmd-30000]);
		loadLang();
		langChanged();
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------
void initLang()
{
	scanLangDir();
	if(!lang[0]){
		//language detection
		const TCHAR* s;
		switch(PRIMARYLANGID(GetUserDefaultLangID()))
		{
			case LANG_CATALAN: s=_T("Catalan"); break;
			case LANG_CZECH: s=_T("Èesky"); break;
			case LANG_FRENCH: s=_T("French"); break;
			case LANG_GREEK: s=_T("Greek"); break;
			case LANG_CHINESE: s=_T("Chinese (Simplified)"); break;
			case LANG_ITALIAN: s=_T("Italiano"); break;
			case LANG_DUTCH: s=_T("Nederlands"); break;
			case LANG_POLISH: s=_T("Polish"); break;
			case LANG_PORTUGUESE: s=_T("Português brasileiro"); break;
			case LANG_RUSSIAN: s=_T("Russian"); break;
			case LANG_SLOVAK: s=_T("Slovak"); break;
			default: s=_T("English"); break;
		}
		_tcscpy(lang, s);
	}
	loadLang();
}
//---------------------------------------------------------------------------
