// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <signal.h>


#include <go32.h>
#include <pc.h>
#include <dpmi.h>
#include <dos.h>
#include <sys/nearptr.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

//dosstuff -newly added
byte* dascreen;
_go32_dpmi_seginfo oldkeyinfo,newkeyinfo;
volatile char keydown[128];
volatile char extendedkeydown[128];
volatile char nextkeyextended;
void initkeyhandler();
void killkeyhandler();
void keyhandler();
char oldkeystate[128];
char oldextendedkeystate[128];

void initkeyhandler()
{
int i;

for (i=0;i<128;i++) keydown[i]=0;
for (i=0;i<128;i++) extendedkeydown[i]=0;
for (i=0;i<128;i++) oldkeystate[i]=0;
for (i=0;i<128;i++) oldextendedkeystate[i]=0;
nextkeyextended=0;

asm("cli");
_go32_dpmi_get_protected_mode_interrupt_vector(9, &oldkeyinfo);
newkeyinfo.pm_offset=(int)keyhandler;
newkeyinfo.pm_selector=_go32_my_cs();
_go32_dpmi_allocate_iret_wrapper(&newkeyinfo);
_go32_dpmi_set_protected_mode_interrupt_vector(9, &newkeyinfo);
asm("sti");
}

void killkeyhandler()
{
asm("cli");
_go32_dpmi_set_protected_mode_interrupt_vector(9, &oldkeyinfo);
_go32_dpmi_free_iret_wrapper(&newkeyinfo);
asm("sti");
}

void keyhandler()
{
unsigned char keyhandlercurrkey;

asm("cli");
keyhandlercurrkey=inportb(0x60);

if (nextkeyextended)
  {
  if ((keyhandlercurrkey&0x80)==0)
    extendedkeydown[keyhandlercurrkey&0x7f]=1;
  else
    extendedkeydown[keyhandlercurrkey&0x7f]=0;
  nextkeyextended=0;
  }
else
  {
  if ((keyhandlercurrkey&0x80)==0)
    keydown[keyhandlercurrkey&0x7f]=1;
  else
    keydown[keyhandlercurrkey&0x7f]=0;
  }

if (keyhandlercurrkey==0xe0)
  nextkeyextended=1;

if ((keydown[0x1d])&&(keyhandlercurrkey==0x2e))
  {
  asm ("movb $0x79, %%al
        call ___djgpp_hw_exception"
       : : :"%eax","%ebx","%ecx","%edx","%esi","%edi","memory");
  }

//reset keyoard
{char b; b=inportb(0x61); outportb(0x61,b|80); outportb(0x61,b); outportb(0x20,0x20);}
asm("sti");
}
#ifdef PHILL
/* This version of ASCIINames, does *NOT* return numeric codes for */
/* The keypad arrow keys !, which allows them to be used as direction keys ! */
byte ASCIINames[] =     // Unshifted ASCII for scan codes
                  //left-shift must be turned to right shift, cuz doom doesnt recognize left-shit
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',KEYD_RSHIFT,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,'-',0  ,0  ,0  ,'+',0  ,    // 4
    0  ,0  ,0  ,127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					};
#else
byte ASCIINames[] =     // Unshifted ASCII for scan codes
                  //left-shift must be turned to right shift, cuz doom doesnt recognize left-shit
					{
//	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	0  ,27 ,'1','2','3','4','5','6','7','8','9','0','-','=',8  ,9  ,	// 0
	'q','w','e','r','t','y','u','i','o','p','[',']',13 ,0  ,'a','s',	// 1
	'd','f','g','h','j','k','l',';',39 ,'`',KEYD_RSHIFT,92 ,'z','x','c','v',	// 2
	'b','n','m',',','.','/',0  ,'*',0  ,' ',0  ,0  ,0  ,0  ,0  ,0  ,	// 3
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,'7','8','9','-','4','5','6','+','1',	// 4
	'2','3','0',127,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
					};
#endif
//end of newly added stuff


void I_ShutdownGraphics(void)
{
  __dpmi_regs r;


  killkeyhandler();
  r.x.ax=0x3;
  __dpmi_int(0x10,&r);

}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}


void I_GetEvent()
  {
  __dpmi_regs r;
  event_t event;
  char tempkey[128];
  char tempextendedkey[128];
  int i;
  int xmickeys,ymickeys,buttons;
  static int lastbuttons=0;

  //key presses
  asm("cli");
  for (i=0;i<128;i++)
    {
    tempkey[i]=keydown[i];
    tempextendedkey[i]=extendedkeydown[i];
    }
  asm("sti");
  for (i=0;i<128;i++)
    {
    if ((tempkey[i]==1)&&(oldkeystate[i]==0))
      {
      event.type=ev_keydown;
      if (ASCIINames[i]!=0)
        event.data1=ASCIINames[i];
      else
#ifdef PHILL   /* This bit allows numeric pad arrow keys to be used (keypress down )*/
        switch(i)
        {
            case 0x48: event.data1=KEYD_UPARROW; break;
            case 0x4d: event.data1=KEYD_RIGHTARROW; break;
            case 0x50: event.data1=KEYD_DOWNARROW; break;
            case 0x4b: event.data1=KEYD_LEFTARROW; break;
            default : event.data1=i+0x80;
        }
#else
        event.data1=i+0x80;
#endif
     D_PostEvent(&event);
      }
    if ((tempkey[i]==0)&&(oldkeystate[i]==1))
      {
      event.type=ev_keyup;
      if (ASCIINames[i]!=0)
        event.data1=ASCIINames[i];
      else
#ifdef PHILL   /* This bit allows numeric pad arrow keys to be used (keypress down )*/
        switch(i)
        {
            case 0x48: event.data1=KEYD_UPARROW; break;
            case 0x4d: event.data1=KEYD_RIGHTARROW; break;
            case 0x50: event.data1=KEYD_DOWNARROW; break;
            case 0x4b: event.data1=KEYD_LEFTARROW; break;
            default : event.data1=i+0x80;
        }
#else
        event.data1=i+0x80;
#endif
     D_PostEvent(&event);
      }
    if ((tempextendedkey[i]==1)&&(oldextendedkeystate[i]==0))
      {
      event.type=ev_keydown;
      switch (i)
        {
        case 0x48: event.data1=KEYD_UPARROW; D_PostEvent(&event); break;
        case 0x4d: event.data1=KEYD_RIGHTARROW; D_PostEvent(&event); break;
        case 0x50: event.data1=KEYD_DOWNARROW; D_PostEvent(&event); break;
        case 0x4b: event.data1=KEYD_LEFTARROW; D_PostEvent(&event); break;
#ifdef FLIGHT
        case 0x52: event.data1=KEYD_INS; D_PostEvent(&event); break;
        case 0x47: event.data1=KEYD_HOME; D_PostEvent(&event); break;
        case 0x49: event.data1=KEYD_PGUP; D_PostEvent(&event); break;
#endif
        }
      }
    if ((tempextendedkey[i]==0)&&(oldextendedkeystate[i]==1))
      {
      event.type=ev_keyup;
      switch (i)
        {
        case 0x48: event.data1=KEYD_UPARROW; D_PostEvent(&event); break;
        case 0x4d: event.data1=KEYD_RIGHTARROW; D_PostEvent(&event); break;
        case 0x50: event.data1=KEYD_DOWNARROW; D_PostEvent(&event); break;
        case 0x4b: event.data1=KEYD_LEFTARROW; D_PostEvent(&event); break;
#ifdef FLIGHT
        case 0x52: event.data1=KEYD_INS; D_PostEvent(&event); break;
        case 0x47: event.data1=KEYD_HOME; D_PostEvent(&event); break;
        case 0x49: event.data1=KEYD_PGUP; D_PostEvent(&event); break;
#endif
        }
      }
    }
  memcpy(oldkeystate,tempkey,128);
  memcpy(oldextendedkeystate,tempextendedkey,128);

  //mouse movement

  r.x.ax=0x0b;
  __dpmi_int(0x33,&r);
  xmickeys=(signed short)r.x.cx;
  ymickeys=(signed short)r.x.dx;
  r.x.ax=0x03;
  __dpmi_int(0x33,&r);
  buttons=r.x.bx;

  event.type=ev_mouse;
  event.data1=buttons; //???
  event.data2=xmickeys;
  event.data3=-ymickeys;
  if ((xmickeys!=0)||(ymickeys!=0)||(buttons!=lastbuttons))
    D_PostEvent(&event);
  lastbuttons=buttons;

  }


//
// I_StartTic
//
void I_StartTic()
  {
  I_GetEvent();
  //i dont think i have to do anything else here

  }



//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

void I_FinishUpdate(void)
  {
    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    
    }

  //blast it to the screen
  memcpy(dascreen,screens[0],SCREENWIDTH*SCREENHEIGHT);
  }

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


void I_SetPalette (byte* palette)
  {
  int c,i;

  outportb(0x3c8,0);
  for (i=0;i<256;i++)
    {
    c=gammatable[usegamma][*palette++];
    outportb(0x3c9,c>>2);
    c=gammatable[usegamma][*palette++];
    outportb(0x3c9,c>>2);
    c=gammatable[usegamma][*palette++];
    outportb(0x3c9,c>>2);
    }
  }


void I_InitGraphics(void)
  {
  static int firsttime=1;
  __dpmi_regs r;

  __djgpp_nearptr_enable();

  if (!firsttime)
    return;
  firsttime=0;

  //enter graphics mode
  r.x.ax=0x13;
  __dpmi_int(0x10,&r);
  dascreen=(byte *)(__djgpp_conventional_base+0xa0000);
  screens[0]=(byte *)malloc(SCREENWIDTH*SCREENHEIGHT);

  //init the mouse
  r.x.ax=0;
  __dpmi_int(0x33,&r);
  r.x.ax=2;
  __dpmi_int(0x33,&r); //hide cursor
  r.x.ax=0x0b;
  __dpmi_int(0x33,&r); //reset micket count

  //init keyboard
  initkeyhandler();
  }



int	inited;


