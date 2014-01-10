/*
  (C) 2002-2012 Petr Lastovicka

  contents of this file are subject to the Reciprocal Public License ("RPL")

Usage:
read lang from registry, call initLang() and langChanged()
paste setLang(cmd) to WM_COMMAND 
paste setDlgTexts(hDlg,titleId) to WM_INITDIALOG 
implement langChanged() - reload menu, file filters, invalidate, reload not modal dialogs
*/
#include "hdr.h"
#pragma hdrstop
#include "lang.h"

#ifndef MAXLNGSTR
 #define MAXLNGSTR 1500
#endif

extern char *cmdNames[];

//---------------------------------------------------------------------------
const int MAXLANG=60;
char lang[64];         //current language name
char *langFile;        //file content (\n replaced with \0)
char *lngstr[MAXLNGSTR];   //pointers to lines in langFile
char *lngNames[MAXLANG+1]; //all found languages names
extern bool isWin9X;
//-------------------------------------------------------------------------
#define sizeA(A) (sizeof(A)/sizeof(*A))

char *lng(int i, char *s)
{
 return (i>=0 && i<sizeA(lngstr) && lngstr[i]) ? lngstr[i] : s;
}

//return pointer to name after a path
char *cutPath(char *s)
{
 char *t;
 t=strchr(s,0);
 while(t>=s && *t!='\\') t--;
 t++;
 return t;
}

//concatenate current directory and e, write result to fn
void getExeDir(char *fn, char *e)
{
 GetModuleFileName(0,fn,192);
 strcpy(cutPath(fn), e);
}
//-------------------------------------------------------------------------
static BOOL CALLBACK enumControls(HWND hwnd, LPARAM)
{
 int i=GetDlgCtrlID(hwnd);
 if((i>=300 && i<sizeA(lngstr) || i<11 && i>0) && lngstr[i]){
   SetWindowText(hwnd,lngstr[i]);
 }
 return TRUE;
}

void setDlgTexts(HWND hDlg)
{
 EnumChildWindows(hDlg,(WNDENUMPROC)enumControls,0);
}

void setDlgTexts(HWND hDlg, int id)
{
 char *s=lng(id,0);
 if(s) SetWindowText(hDlg,s);
 setDlgTexts(hDlg);
}

//reload not modal dialog or create new dialog at position x,y
void changeDialog(HWND &wnd, int x,int y,LPCTSTR dlgTempl, DLGPROC dlgProc)
{
 HWND a,w;

 a=GetActiveWindow();
 w=CreateDialog(inst,dlgTempl,0,dlgProc);
 if(wnd){
   RECT rc;
   GetWindowRect(wnd,&rc);
   MoveWindow(w,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
   if(IsWindowVisible(wnd)) ShowWindow(w,SW_SHOW);
   DestroyWindow(wnd);
 }else{
   SetWindowPos(w,0,x,y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 }
 wnd=w;
 if(a) SetActiveWindow(a);
}
//-------------------------------------------------------------------------
static int *subPtr;

//recurse through menu and change all item names
static void fillPopup(HMENU h)
{
 int i,id,j;
 char *s,*a;
 UINT f;
 HMENU sub;
 MENUITEMINFO mii;
 WCHAR w[21];

 for(i=GetMenuItemCount(h)-1; i>=0; i--){
   id=GetMenuItemID(h,i);
   if(id==29999){
     for(j=0; (a=lngNames[j])!=0; j++){
       f=MF_BYPOSITION|(_stricmp(a,lang)?0:MF_CHECKED);
       w[0]=0;
       if(!isWin9X){
         size_t len = strlen(a);
         // L"\x10c" does not compile correctly in Microsoft Visual C++ 6.0
         if(len==5 && !_strnicmp(a+1,"esky",4)){ wcscpy(w,L"0esky"); w[0]=0x10c; }
         if(len==7 && !_strnicmp(a,"Espa",4)){ wcscpy(w,L"Espa0ol"); w[4]=0xf1; }
         if(len==20 && !_strnicmp(a,"Portugu",7)){ wcscpy(w,L"Portugu0s brasileiro"); w[7]=0xea; }
       }
       if(w[0]) InsertMenuW(h,0xFFFFFFFF,f,30000+j,w);
       else InsertMenuA(h,0xFFFFFFFF,f,30000+j,a);
     }
     DeleteMenu(h,0,MF_BYPOSITION);
   }else{
     if(id<0 || id>=0xffffffff){
       sub=GetSubMenu(h,i);
       if(sub){
         id=*subPtr++;
         fillPopup(sub);
       }
     }
     s=lng(id,0);
     if(!s && id>=1000 && id<1500 && cmdNames){
       s=cmdNames[id-1000];
     }
     if(s){
       mii.cbSize=sizeof(MENUITEMINFO);
       mii.fMask=MIIM_TYPE|MIIM_STATE;
       mii.fType=MFT_STRING;
       mii.fState=MFS_ENABLED;
       mii.dwTypeData=s;
       mii.cch= (UINT) strlen(s);
       SetMenuItemInfo(h,i,TRUE,&mii);
     }
   }
 }
}

//load menu from resources
//subId are string numbers for submenus
HMENU loadMenu(char *name, int *subId)
{
 HMENU hMenu= LoadMenu(inst,name);
 subPtr=subId;
 fillPopup(hMenu);
 return hMenu;
}

void loadMenu(HWND hwnd, char *name, int *subId)
{
 if(!hwnd) return;
 HMENU m= GetMenu(hwnd);
 SetMenu(hwnd,loadMenu(name,subId));
 DestroyMenu(m);
}
//-------------------------------------------------------------------------
static void parseLng()
{
 char *s,*d,*e;
 int id,err=0,line=1;

 for(s=langFile; *s; s++){
   if(*s==';' || *s=='#' || *s=='\n' || *s=='\r'){
     //comment
   }else{
     id=(int)strtol(s,&e,10);
     if(s==e){
       if(!err) msglng(755,"Error in %s\nLine %d",lang,line);
       err++;
     }else if(id<0 || id>=sizeA(lngstr)){
       if(!err) msglng(756,"Error in %s\nMessage number %d is too big",lang,id);
       err++;
     }else if(lngstr[id]){
       if(!err) msglng(757,"Error in %s\nDuplicated number %d",lang,id);
       err++;
     }else{
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
       }else if(*s=='\n'){
         line++;
         continue;
       }else if(*s=='0'){
         *s='\0';
       }else if(*s=='n'){
         *s='\n';
       }else if(*s=='r'){
         *s='\r';
       }else if(*s=='t'){
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
 char buf[256];

 lngNames[0]="English";
 getExeDir(buf,"language\\*.lng");
 h = FindFirstFile(buf,&fd);
 if(h!=INVALID_HANDLE_VALUE){
  n=1;
  do{
   if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
     int len= (int) strlen(fd.cFileName)-4;
     if(len>0){
       char *s= new char[len+1];
       memcpy(s,fd.cFileName,len);
       s[len]='\0';
       lngNames[n++]=s;
     }
   }
  }while(FindNextFile(h,&fd) && n<MAXLANG);
  FindClose(h);
 }
}
//-------------------------------------------------------------------------
void loadLang()
{
 memset(lngstr,0,sizeof(lngstr));
 char buf[256];
 GetModuleFileName(0,buf,sizeof(buf)-(int)strlen(lang)-14);
 strcpy(cutPath(buf), "language\\");
 char *fn=strchr(buf,0);
 strcpy(fn,lang);
 strcat(buf,".lng");
 HANDLE f=CreateFile(buf,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
 if(f!=INVALID_HANDLE_VALUE){
   DWORD len=GetFileSize(f,0);
   if(len>10000000){
     msglng(753,"File %s is too long",fn);
   }else{
     delete[] langFile;
     langFile= new char[len+3];
     DWORD r;
     ReadFile(f,langFile,len,&r,0);
     if(r<len){
       msglng(754,"Error reading file %s",fn);
     }else{
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
   strcpy(lang,lngNames[cmd-30000]);
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
   //language autodetection
   strcpy(lang,"English");
   BYTE id= (BYTE) GetUserDefaultLangID();
   if(id==0x05) strcpy(lang,"Èesky");
   if(id==0x16) strcpy(lang,"Português brasileiro");
   if(id==0x0c) strcpy(lang,"French");
   if(id==0x1b) strcpy(lang,"Slovak");
   if(id==0x19) strcpy(lang,"Russian");
   if(id==0x04) strcpy(lang,"Chinese (Simplified)");
   if(id==0x15) strcpy(lang,"Polish");
   if(id==0x08) strcpy(lang,"Greek");
 }
 loadLang();
}
//---------------------------------------------------------------------------

