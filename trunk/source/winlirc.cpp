/*
 (C) 2006  Petr Lastovicka
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
*/  
#include "hdr.h"
#pragma hdrstop
#include <winsock.h>
#include "hotkeyp.h"

int lircEnabled=0;
int lircPort=8765;
char lircAddress[64];
TfileName lircExe;
int lircRepeat=200;
char *lircButton;

static SOCKET sock;
HANDLE lircThread;


int initWSA()
{
  WSAData wd;
  int err= WSAStartup(0x0202,&wd);
  if(err) msg("Cannot initialize WinSock");
  return err;
}

int startConnection(char *address, int port)
{
  int err=1;
  unsigned long a;
  sockaddr_in sa;
  hostent *he;
  
  if(!initWSA()){
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    a= inet_addr(address);
    if(a!=INADDR_NONE){
      sa.sin_addr.S_un.S_addr=a;
    }else if(( he= gethostbyname(address) )!=0){
      memcpy(&sa.sin_addr, he->h_addr, he->h_length);
    }else{
      err=2;
      goto le;
    }
    if(( sock= socket(PF_INET,SOCK_STREAM,0) )==INVALID_SOCKET){
      msg("Error: socket");
    }else{
      sa.sin_port = htons((u_short)port);
      if(connect(sock, (sockaddr*)&sa, sizeof(sa))!=0){
        err=3;
      }else{
        return 0;
      }
      closesocket(sock);
    }
le:
    WSACleanup();
  }
  return err;
}


bool processExist(char *fn)
{
  PROCESSENTRY32 pe;
  bool result=false;
  
  fn=cutPath(fn);
  pe.dwSize = sizeof(PROCESSENTRY32);
  HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  if(h!=(HANDLE)-1){
    Process32First(h,&pe);
    do{
      if(!_strnicmp(cutPath(pe.szExeFile), fn, 15)){
        result=true;
        break;
      }
    }while(Process32Next(h,&pe));
    CloseHandle(h);
  }
  return result;
}

int lircLoop()
{
  int n,i,err;
  char buf[64],*e,*s[4];
  DWORD t;
  static char *prevButton=0;
  static DWORD lastTick;

  //start WinLIRC.exe
  if(!processExist(lircExe)){
    if(!createProcess(lircExe)) return 10;
    Sleep(300);
  }
  //connect to WinLIRC server
  for(n=0; ;n++){
    err= startConnection(lircAddress,lircPort);
    if(err<=1 || n>7) break;
    Sleep(2000);
  }
  if(err){
    if(err>1) msglng(763,"Cannot connect to WinLIRC");
    return err;
  }
  lircNotify();
  
  for(n=0;;){
    //read from the socket
    i=recv(sock,buf+n,sizeof(buf)-n-1,0);
    if(i<=0){
      //error or shutdown
      if(i<0){
        i=WSAGetLastError();
        if(i!=WSAECONNRESET && i!=WSAECONNABORTED){
          msg("Connection to WinLIRC failed (error %u)",i);
        }
      }
      break; 
    }
    n+=i;
    buf[n]=0;
    e=strchr(buf,'\n');
    if(e){
      //find the third field in a line
      *e=0;
      s[0]=strchr(buf,' ');
      if(s[0]++){
        s[1]=strchr(s[0],' ');
        if(s[1]++){
          s[2]=strchr(s[1],' ');
          if(s[2]){
            *s[2]=0;
            t=GetTickCount();
            if(!prevButton || t-lastTick>(DWORD)lircRepeat || 
              strcmp(prevButton,s[1])){
              lastTick=t;
              cpStr(prevButton, lircButton=s[1]);
              SendMessage(hWin,WM_USER+80,0,0);
            }
          }
        }
      }
      i=int(e-buf)+1;
      n-=i;
      memmove(buf,buf+i,n);
    }
  }
  closesocket(sock);
  sock=INVALID_SOCKET;
  WSACleanup();            
  return 0;
}

DWORD WINAPI lircProc(void *)
{
  int result= lircLoop();
  CloseHandle(lircThread);
  lircThread=0;
  lircNotify();
  return result;
}

void lircStart()
{
  if(!lircThread && lircEnabled){
    DWORD id;
    lircThread=CreateThread(0,0,lircProc,0,0,&id);
  }
}

void lircEnd(bool wait)
{
  if(lircThread){
    closesocket(sock);
    if(wait) WaitForSingleObject(lircThread,10000);
  }
}

void lircNotify()
{
  PostMessage(hWin,WM_USER+81,0,(LPARAM)lircThread);
}
