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
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";


#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>
#include <allegro.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "d_main.h"
#include "g_game.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

//#define	BGCOLOR		7
//#define	FGCOLOR		8
// Alternate Colour scheme...
#define BGCOLOR		4
#define FGCOLOR		0xf

int     mb_used = 10;

char oldkeystate[128];
volatile int mselapsed=0;
int mousepresent;

extern int usejoystick;
extern int usemouse;

void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}

void I_GetEvent()
  {
  event_t event;
  int i,j;

  char keystate[128];
  int xmickeys,ymickeys,buttons;
  static int lastbuttons=0;

  //key presses
  for (i=0;i<128;i++)
    keystate[i]=key[i];
  for (i=0;i<128;i++)
    {
    char normkey,extkey,oldnormkey,oldextkey;

    normkey=keystate[i]&KB_NORMAL; extkey=keystate[i]&KB_EXTENDED;
    oldnormkey=oldkeystate[i]&KB_NORMAL; oldextkey=oldkeystate[i]&KB_EXTENDED;

    if ((normkey!=0)&&(oldnormkey==0))
      {
      event.type=ev_keydown;
      event.data1=I_ScanCode2DoomCode(i);
      D_PostEvent(&event);
      }
    if ((normkey==0)&&(oldnormkey!=0))
      {
      event.type=ev_keyup;
      event.data1=I_ScanCode2DoomCode(i);
      D_PostEvent(&event);
      }
    if ((extkey!=0)&&(oldextkey==0))
      {
      if ((i==0x48)||(i==0x4d)||(i==0x50)||(i==0x4b)||(i==KEY_ALTGR)||(i==KEY_RCONTROL)||(i==KEY_PGDN)||(i==KEY_PGUP)||(i==KEY_HOME)||(i==KEY_PRTSCR))
        {
        event.type=ev_keydown;
        event.data1=I_ScanCode2DoomCode(i);
        D_PostEvent(&event);
        }
      else if (i==0x7b)
        {
        event.type=ev_keydown;
        event.data1=KEYD_PAUSE;
        D_PostEvent(&event);
        key[0x7b]=0;break;
        event.type=ev_keyup;
        event.data1=KEYD_PAUSE;
        D_PostEvent(&event);
        }
      }
    if ((extkey==0)&&(oldextkey!=0))
      {
      if ((i==0x48)||(i==0x4d)||(i==0x50)||(i==0x4b)||(i==KEY_ALTGR)||(i==KEY_RCONTROL)||(i==KEY_PGDN)||(i==KEY_PGUP)||(i==KEY_HOME)||(i==KEY_PRTSCR))
        {
        event.type=ev_keyup;
        event.data1=I_ScanCode2DoomCode(i);
        D_PostEvent(&event);
        }
      }
    }
  memcpy(oldkeystate,keystate,128);

  //mouse movement
  if ((mousepresent!=-1)&&(usemouse))
    {
    get_mouse_mickeys(&xmickeys,&ymickeys);

    event.type=ev_mouse;
    event.data1=0;
    event.data2=xmickeys;
    if (novert==0)
      event.data3=-ymickeys;
    else
      event.data3=0;
    if ((xmickeys!=0)||(ymickeys!=0))
      D_PostEvent(&event);

    //now, do buttons
    buttons=mouse_b;
    if (buttons!=lastbuttons)
      {
      j=1;
      for (i=0;i<3;i++)
        {
        if (((buttons&j)==j)&&((lastbuttons&j)==0))
          {
          event.type=ev_keydown;
          event.data1=KEYD_MOUSE1+i;
          D_PostEvent(&event);
          }
        if (((buttons&j)==0)&&((lastbuttons&j)==j))
          {
          event.type=ev_keyup;
          event.data1=KEYD_MOUSE1+i;
          D_PostEvent(&event);
          }
        j*=2;
        }
      }
    lastbuttons=buttons;
    }

  //joystick
  if (usejoystick)
    {
    static int oldb=0;
    int currb;

    poll_joystick();
    event.type=ev_joystick;
    event.data1=0;
    event.data2=joy_x;
    event.data3=joy_y;
    event.data2=(abs(event.data2)<4)?0:event.data2;
    event.data3=(abs(event.data3)<4)?0:event.data3;
    D_PostEvent(&event);

    //now, do buttons
    currb=0;
    if (joy_b1) currb|=1;
    if (joy_b2) currb|=2;
    if (joy_b3) currb|=4;
    if (joy_b4) currb|=8;
    if (currb!=oldb)
      {
      j=1;
      for (i=0;i<4;i++)
        {
        if (((currb&j)==j)&&((oldb&j)==0))
          {
          event.type=ev_keydown;
          event.data1=KEYD_JOY1+i;
          D_PostEvent(&event);
          }
        if (((currb&j)==0)&&((oldb&j)==j))
          {
          event.type=ev_keyup;
          event.data1=KEYD_JOY1+i;
          D_PostEvent(&event);
          }
        j*=2;
        }
      }
    oldb=currb;
    }
  }

//
// I_StartTic
//
void I_StartTic()
  {
  I_GetEvent();
  //i dont think i have to do anything else here

  }


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
  {
   static void *zonebase = NULL;
  int p, s = 1;

  if (zonebase) return zonebase;
  p = M_CheckParm ("-heapsize");
  *size = mb_used * 1024 * 1024;
  if (p && p < myargc-1)
  {
    mb_used=atoi(myargv[p+1]);
    *size = mb_used * 1024 * 1024;
  } else {
    s = get_config_int("system", "heapsize", 0);
    if (s) {
      mb_used = s;
      *size = mb_used * 1024; // Kilobytes :-) Kester
      s = 0;
    } else {
      *size = mb_used * 1024 * 1024;
      s = 1;
    }
  }

  I_Printf ("Heapsize: %d %s\n\r",mb_used,s?"Megabytes":"Kilobytes");
  
  return (byte *)(zonebase = malloc (*size));
}

void I_timer(void)
{
  mselapsed++;
  cdcounter--;
}
END_OF_FUNCTION(I_timer);


//
// I_Init
//
void I_Init (void)
{
  allegro_init();
  //init the joystick
  if (usejoystick)
    {
    joy_type = JOY_TYPE_4BUTTON;
    I_Printf("CENTER the joystick and press a key:"); getch(); I_Printf("\n");
    initialise_joystick();
    I_Printf("Push the joystick to the UPPER LEFT corner and press a key:"); getch(); I_Printf("\n");
    calibrate_joystick_tl();
    I_Printf("Push the joystick to the LOWER RIGHT corner and press a key:"); getch(); I_Printf("\n");
    calibrate_joystick_br();
    }
  //init timer
  LOCK_VARIABLE(mselapsed);
  LOCK_VARIABLE(cdcounter);
  LOCK_FUNCTION(I_timer);
  i_love_bill = get_config_int("system", "i_love_bill", FALSE);
  install_timer();
  install_int_ex(I_timer,BPS_TO_TIMER(TICRATE));

  //init the mouse
  if (usemouse)
    {
    mousepresent=install_mouse();
    if (mousepresent!=-1)
      show_mouse(NULL);
    if (M_CheckParm("-novert"))
      novert=1;
    }

  //init keyboard
  memset(oldkeystate,0,128);
  install_keyboard();
  if (!M_CheckParm("-nosound"))
    I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    if (demorecording)
      G_CheckDemoStatus();

    D_QuitNetGame ();
    if (!M_CheckParm("-nosound"))
      {
      I_ShutdownSound();
      I_ShutdownMusic();
      }
    M_SaveDefaults ();
    I_ShutdownGraphics();
  remove_keyboard();
  if (mousepresent!=-1)
    remove_mouse();
  remove_timer();

    // Throw the end text at the screen & exit - Kester
    puttext(1, 1, 80, 25, W_CacheLumpName("ENDOOM", PU_CACHE));
    gotoxy(1, 24);
    exit(0);
}


void I_WaitVBL(int count)
{
if (mselapsed>0)
  rest(count*1000/70);
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//
int  I_GetTime (void)
{
if (mselapsed>0)
  return (mselapsed);
else
  {
  struct timeval	tp;
  struct timezone	tzp;
  int			newtics;
  static int		basetime=0;
  
  gettimeofday(&tp, &tzp);
  if (!basetime)
    basetime = tp.tv_sec;
  newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
  return newtics;
  }
}
/*
int  I_GetMilliTime (void)
{
if (mselapsed>0)
  return mselapsed;
else
  {
  struct timeval	tp;
  struct timezone	tzp;
  int			newtics;
  static int		basetime=0;
  
  gettimeofday(&tp, &tzp);
  if (!basetime)
    basetime = tp.tv_sec;
  newtics = (tp.tv_sec-basetime)*1000 + tp.tv_usec*1000/1000000;
  return newtics;
  }
}
*/
void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte*	I_AllocLow(int length)
{
    byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;

}


//
// I_Error
//
extern boolean demorecording;
extern boolean graphicsmode;

void I_Error (char *error, ...)
{
    va_list	argptr;

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    if (!M_CheckParm("-nosound"))
      {
      I_ShutdownSound();
      I_ShutdownMusic();
      }
    if (graphicsmode)
      I_ShutdownGraphics();
    
    // Message last, so it actually prints on the screen
    va_start (argptr,error);
//    fprintf (stderr, "Error: ");
    vfprintf (stdout,error,argptr);
    fprintf (stdout, "\n");
    va_end (argptr);
    fflush( stdout );


    exit(-1);
}

static char         msgbuf[512];
void I_Printf (char *message, ...)
{
    va_list      argptr;
    char	*string = msgbuf;
    va_start (argptr, message);
    // Print the message into a text string
    vsprintf(msgbuf, message, argptr);
    
    // If debuging enabled, print to the debugfile
    Debug_Printf(msgbuf);
    
    // Clean up \n\r combinations
    while (*string) {
      if (*string == '\n') {
        memmove(string + 2, string + 1, strlen(string));
        string[1] = '\r';
        string++;
      }
    string++;
    }

    // If we are in graphics mode, we make the message
    // appear on the status line
    if (graphicsmode) players[consoleplayer].message = msgbuf;
    
    // Otherwise, print it to the screen
    else cprintf(msgbuf);
    
    va_end (argptr);
}
void I_Window(int left, int top, int right, int bottom)
{
  window(left, top, right, bottom);
}
      
void
I_TextAttr(int attr)
{
  textattr(attr);
}
      
void
I_ClrScr(void)
{
  clrscr();
}

void I_PutTitle(char *title)
{
      char string[81] = { [0 ... 79] = ' ', [80] = 0 };
      char dosdoom[81];
      int centre;

      // Build the title
      sprintf(dosdoom, "DOSDoom v%d.%d", DOSDOOMVER / DOSDOOMVERFIX, DOSDOOMVER % DOSDOOMVERFIX);
      centre = (80 - strlen(title)) / 2;
      memcpy(&string[centre], title, strlen(title));
      memcpy(&string[1], dosdoom, strlen(dosdoom));
      sprintf(dosdoom, "DOOM v%d.%d", VERSION / VERSIONFIX, VERSION % VERSIONFIX);
      centre = 79 - strlen(dosdoom);
      memcpy(&string[centre], dosdoom, strlen(dosdoom));

      // Print the title
      I_TextAttr(0x07);
      I_ClrScr();
      I_TextAttr((BGCOLOR << 4) + FGCOLOR);
      I_Printf ("%s\n",string);
      I_TextAttr(0x07);
      I_Window(1, 2, 80, 25);
}

