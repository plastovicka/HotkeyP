/*
 (C) Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#include "hdr.h"
#pragma hdrstop
#include "hotkeyp.h"

#define MAX_JOYSTICKS 4
#define MAX_JOY_BUTTONS 32

int
joyThreshold=60,
 joyMultiplier=180,
 joyNotFullscreen=0,
 joyMouseEnabled=0,
 joyMouseJoy=1,
 joyMouseX=0, //axis X
 joyMouseY=1; //axis Y
TCHAR joyApp[512]; //disable global joystick shortcuts in these applications
TCHAR joyFullscreenApp[512]; //disable in fullscreen except these


void joyAction1(int scan, int updown)
{
	SendMessage(hWin, WM_USER+83, updown, scan);
}

void joyAction(int joyId, int type, int scan, int updown)
{
	joyAction1((joyId<<28)|(type<<26)|scan, updown);
}

DWORD WINAPI joyProc(void *)
{
	int i, j, d, swap, dx, dy, s, sprev;
	DWORD x, xprev, c, t, r, x1;
	static JOYINFOEX jiA[MAX_JOYSTICKS][2];
	JOYCAPS jc;

	static TjoyGetPosEx pjoyGetPosEx;
	winmm((FARPROC&)pjoyGetPosEx, "joyGetPosEx");
	static TjoyGetDevCaps pjoyGetDevCaps;
	winmm((FARPROC&)pjoyGetDevCaps, "joyGetDevCapsW");

	for(swap=0;; swap=1-swap){
		Sleep(10);
		dx=dy=0;

		for(j=0; j<MAX_JOYSTICKS; j++)
		{
			JOYINFOEX *jij = jiA[j], &ji = jij[swap], &jiPrev = jij[1-swap];

			ji.dwSize = sizeof(JOYINFOEX);
			ji.dwFlags = JOY_RETURNALL;

#ifdef JOYTEST
			const UINT Xmax=4000000000;
			const UINT Ymax=100;
			if(j==0){
				ji.dwButtons=0;
				for(i=0; i<9; i++){
					if(GetKeyState(VK_NUMPAD1+i)<0) ji.dwButtons|= 1<<i;
				}
				ji.dwPOV=JOY_POVCENTERED;
				if(GetKeyState(VK_LEFT)<0) ji.dwPOV=9000;
				if(GetKeyState(VK_DOWN)<0) ji.dwPOV=18000;
				if(GetKeyState(VK_RIGHT)<0) ji.dwPOV=27000;
				if(GetKeyState(VK_UP)<0) ji.dwPOV=0;
				ji.dwXpos=Xmax>>1;
				ji.dwYpos=Ymax>>1;
				if(GetKeyState('A')<0) ji.dwXpos=0;
				if(GetKeyState('D')<0) ji.dwXpos=Xmax;
				if(GetKeyState('W')<0) ji.dwYpos=0;
				if(GetKeyState('S')<0) ji.dwYpos=Ymax;
				if(GetAsyncKeyState('1')<0) ji.dwXpos=0;
				if(GetAsyncKeyState('2')<0) ji.dwXpos=Xmax/10;
				if(GetAsyncKeyState('3')<0) ji.dwXpos=Xmax/10*2;
				if(GetAsyncKeyState('4')<0) ji.dwXpos=Xmax/10*3;
				if(GetAsyncKeyState('5')<0) ji.dwXpos=Xmax/10*4;
				if(GetAsyncKeyState('6')<0) ji.dwXpos=Xmax/10*5;
				if(GetAsyncKeyState('7')<0) ji.dwXpos=Xmax/10*6;
				if(GetAsyncKeyState('8')<0) ji.dwXpos=Xmax/10*7;
				if(GetAsyncKeyState('9')<0) ji.dwXpos=Xmax/10*8;
				if(GetAsyncKeyState('0')<0) ji.dwXpos=Xmax/10*9;
			}else 
#endif

			if(pjoyGetPosEx(j, &ji) != JOYERR_NOERROR) continue; //error

			//buttons
			DWORD diff = ji.dwButtons ^ jiPrev.dwButtons;
			for(i=0; diff; i++){
				if(diff & 1){
					joyAction(j, J_BUTTON, i, (ji.dwButtons&(1<<i)) ? K_DOWN : K_UP);
				}
				diff>>=1;
			}
			//XYZRUV
			bool caps=false;
			for(i=0; i<6; i++){
				x = (&ji.dwXpos)[i];
				xprev = (&jiPrev.dwXpos)[i];
				if(x!=xprev || joyMouseEnabled && (i==joyMouseX || i==joyMouseY)){
					//get caps
					if(!caps){
						caps=true;
#ifdef JOYTEST
						if(j==0){
							jc.wXmin=0; jc.wXmax=Xmax;
							jc.wYmin=0; jc.wYmax=Ymax;
						}else 
#endif
						pjoyGetDevCaps(j, &jc, sizeof(JOYCAPS));
					}
					//get center
					UINT *u = (i<3) ? (&jc.wXmin)+2*i : (&jc.wRmin)+2*(i-3);
					c= (u[0] + u[1])>>1;
					r=  u[1] - u[0];
					t= signed(r)>0 ? MulDiv(r, joyThreshold, 2000) : r/2000*joyThreshold;
					//get sign
					if(x < c-t) s=-1;
					else if(x > c+t) s=+1;
					else s=0;
					if(xprev < c-t) sprev=-1;
					else if(xprev > c+t) sprev=+1;
					else sprev=0;
					//mouse move
					if(s && joyMouseEnabled && j==joyMouseJoy-1 &&
						(i==joyMouseX || i==joyMouseY) && !disableAll && !disableJoystick){
						x1 = x-c-t*s;
						d= ((signed)r>0) ? MulDiv(x1, joyMultiplier, r) :
							(int)(Int32x32To64(x1, joyMultiplier)/r);
						if(i==joyMouseX) dx=d; else dy=d;
					}
					//send message
					if(s!=sprev){
						if(sprev){
							joyAction(j, J_AXIS, i|(sprev<0 ? JOY_SIGN : 0), K_UP);
						}
						if(s){
							joyAction(j, J_AXIS, i|(s<0 ? JOY_SIGN : 0), K_DOWN);
						}
					}
				}
			}
			//POV
			if(ji.dwPOV != jiPrev.dwPOV){
				if(jiPrev.dwPOV!=JOY_POVCENTERED){
					joyAction(j, J_POV, jiPrev.dwPOV, K_UP);
				}
				if(ji.dwPOV!=JOY_POVCENTERED){
					joyAction(j, J_POV, ji.dwPOV, K_DOWN);
				}
			}
		}

		if((dx || dy) && joyGlobalEnabled()){
			static int ddx, ddy;
			div_t tx = div(dx+ddx, 10);
			ddx = tx.rem;
			div_t ty = div(dy+ddy, 10);
			ddy = ty.rem;
			mouse_event(MOUSEEVENTF_MOVE, tx.quot, ty.quot, 0, 0);
		}
	}
}

TCHAR axisInd2Name(int i)
{
	return "XYZRUV  "[i&7];
}

int axisName2Ind(TCHAR c)
{
	if(c=='X') return 0;
	if(c=='Y') return 1;
	if(c=='Z') return 2;
	if(c=='R') return 3;
	if(c=='U') return 4;
	if(c=='V') return 5;
	return -1;
}

bool joyGlobalEnabled()
{
	return !checkProcessList(joyApp) &&
		(!joyNotFullscreen || !checkFullscreen(joyFullscreenApp));
}
