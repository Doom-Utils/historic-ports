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
#include <stdio.h>
#include <conio.h>
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
#include <allegro.h>

#include "doomstat.h"
#include "i_system.h"
#include "multires.h"
#include "m_argv.h"
#include "d_main.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomdef.h"
#include "allegvid.h"

//dosstuff -newly added
int SCREENWIDTH;
int SCREENHEIGHT;
int SCREENPITCH;
int BPP;
int weirdaspect;

char oldkeystate[128];
volatile int mselapsed=0;
int mousepresent;

byte *hicolortable;
short hicolortransmask1,hicolortransmask2;

extern int usejoystick;
extern int usemouse;

void calctranslucencytable();
char *translucencytable25;
char *translucencytable50;
char *translucencytable75;
//end of newly added stuff

void I_AutodetectBPP()
  {
  if (M_CheckParm("-hicolor"))
    BPP=2;
  else
    BPP=1;
  }


//
// I_StartFrame
//
void I_StartFrame (void)
  {
  if (usejoystick)
    poll_joystick();
  }

int I_ScanCode2DoomCode (int a)
  {
  switch (a)
    {
    case 0x48: return KEYD_UPARROW;
    case 0x4d: return KEYD_RIGHTARROW;
    case 0x50: return KEYD_DOWNARROW;
    case 0x4b: return KEYD_LEFTARROW;
    case 0x0e: return KEYD_BACKSPACE;
    case 0x2a: return KEYD_RSHIFT;
    case KEY_ALTGR: return KEYD_RALT;
    case KEY_RCONTROL: return KEYD_RCTRL;
    case KEY_PGUP: return KEYD_PGUP;
    case KEY_PGDN: return KEYD_PGDN;
    case KEY_HOME: return KEYD_HOME;
    }
  if (a>=0x100)
    return a;
  else if (key_ascii_table[a]>8)
    return key_ascii_table[a];
  else
    return a+0x80;
  }

int I_DoomCode2ScanCode (int a)
  {
  int i;

  switch (a)
    {
    case KEYD_UPARROW: return 0x48;
    case KEYD_RIGHTARROW: return 0x4d;
    case KEYD_DOWNARROW: return 0x50;
    case KEYD_LEFTARROW: return 0x4b;
    case KEYD_BACKSPACE: return 0x0e;
    case KEYD_PGUP: return KEY_PGUP;
    case KEYD_PGDN: return KEY_PGDN;
    case KEYD_HOME: return KEY_HOME;
    }
  if (a>=0x100)
    return a;
  else if (a>=0x80)
    return (a-0x80);
  else
    {
    for (i=0;i<128;i++)
      if (key_ascii_table[i]==a)
        return i;
    return 0;
    }
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
          {
          if (BPP==1)
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)] = 0xff;
          else
            {
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0xff;
            screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0xff;
            }
          }
	for ( ; i<20*2 ; i+=2)
          {
          if (BPP==1)
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)] = 0x0;
          else
            {
            screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0x0;
            screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0x0;
            }
          }    
    }

  //blast it to the screen
  flipscreens();
  }

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT*BPP);
}


void I_SetPalette (byte* palette, int redness)
  {
  int c,i,j;
  PALETTE pal;
  short *tempcolormap;
  byte* tempptr;

  if (BPP==1)
    {
    for (i=0;i<256;i++)
      {
      c=gammatable[usegamma][*palette++];
      pal[i].r=c>>2;
      c=gammatable[usegamma][*palette++];
      pal[i].g=c>>2;
      c=gammatable[usegamma][*palette++];
      pal[i].b=c>>2;
      }
    set_palette(pal);
    }
  else
    {  //replace this with some sorta caching system?
    int tempr[256],tempg[256],tempb[256];

    tempcolormap=(short *)colormaps;
    for (i=0;i<256;i++)
      {
      tempr[i]=gammatable[usegamma][*palette++];
      tempg[i]=gammatable[usegamma][*palette++];
      tempb[i]=gammatable[usegamma][*palette++];
      }
    if ((redness<1)||(redness>8))
      {
      for (i=32;i>0;i--)
        {
        tempptr=hicolortable+256*(i-1);
        for (j=0;j<256;j++)
          {
          *tempcolormap=makecol(tempptr[tempr[j]],tempptr[tempg[j]],tempptr[tempb[j]]);
          tempcolormap++;
          }
        }
      }
    else
      {
      int redconst=redness*31;
      for (i=32;i>0;i--)
        {
        tempptr=hicolortable+256*(i-1)+(256*32*redness);
        for (j=0;j<256;j++)
          {
          *tempcolormap=makecol(redconst+tempptr[tempr[j]],tempptr[tempg[j]],tempptr[tempb[j]]);
          tempcolormap++;
          }
        }
      }
    tempptr=hicolortable+256*32;
    for (i=0;i<256;i++)
      {
      j=tempr[i]*54+tempg[i]*183+tempb[i]*19;  //from the colorspace faq
      *tempcolormap=makecol(255-(j/256),255-(j/256),255-(j/256));  //lookup table?
      tempcolormap[256]=0;
      tempcolormap++;
      pal[i].r=tempr[i]>>2; pal[i].g=tempg[i]>>2; pal[i].b=tempb[i]>>2;
      }
    set_palette(pal);
    }
  }

void I_timer(void)
{
  mselapsed+=5;
  cdcounter+=5;
}
END_OF_FUNCTION(I_timer);

void I_InitGraphics(void)
  {
  static int firsttime=1;
  int i,j;

  if (!firsttime)
    return;
  firsttime=0;

  //init the joystick
  if (usejoystick)
    {
    joy_type = JOY_TYPE_4BUTTON;
    printf("CENTER the joystick and press a key:"); getch(); printf("\n");
    initialise_joystick();
    printf("Push the joystick to the UPPER LEFT corner and press a key:"); getch(); printf("\n");
    calibrate_joystick_tl();
    printf("Push the joystick to the LOWER RIGHT corner and press a key:"); getch(); printf("\n");
    calibrate_joystick_br();
    }

  //calc translucencytable if needed
  if ((BPP==1)&&(!M_CheckParm("-notrans")))
    calctranslucencytable();
  //enter graphics mode
  enter_graphics_mode();

  //do the hicolorpal table if necessary
  if (BPP==2)
    {
    byte *tempptr,*tempptr2;

    tempptr=hicolortable=(byte *)malloc(256*32*9);
    for (i=0;i<32;i++)
      {
      for (j=0;j<256;j++)
        {
        *tempptr=j*gammatable[3][i*(256/32)]/256;
        tempptr++;
        }
      }
    for (i=1;i<=8;i++)
      {
      tempptr2=hicolortable;
      for (j=0;j<(256*32);j++)
        {
        *tempptr=(byte)(((int)(*tempptr2))*(8-i)/8);
        tempptr++; tempptr2++;
        }
      }
    hicolortransmask1=makecol(127,127,127);
    hicolortransmask2=makecol(63,63,63);
    }

  //init timer
  LOCK_VARIABLE(mselapsed);
  LOCK_FUNCTION(I_timer);
  install_timer();
  install_int(I_timer,5);

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
  }

void I_ShutdownGraphics(void)
{
//  if (KB_CAPSLOCK_FLAG)
//    set_leds(KB_CAPSLOCK_FLAG);
  remove_keyboard();
  if (mousepresent!=-1)
    remove_mouse();
  remove_timer();
  set_gfx_mode(GFX_TEXT,80,25,0,0);
}

/* progress indicator for the translucency table calculations */
void callback_func()
  {
  static int i = 0;

  if (!(15&i++))
    fprintf(stderr, ".");
  }

void calctranslucencytable()
  { /* This function uses Allegro routines to create a translucency table very quickly */
  int x, y;
  RGB_MAP rgb_table;
  unsigned char *thepalette;
  PALETTE pal;
  RGB c;
  int i;

  thepalette = W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
  translucencytable25=(char *)malloc(65536*3);
  translucencytable50=translucencytable25+65536;
  translucencytable75=translucencytable25+65536*2;

  for (i = 0; i < 256; i++) // The palette must first be converted to Allegro's format (RGB values 0-63)
    {
    pal[i].r = thepalette[i*3+0] >> 2;
    pal[i].g = thepalette[i*3+1] >> 2;
    pal[i].b = thepalette[i*3+2] >> 2;
    }
  printf("Calculating translucency table\n");

  /* this isn't needed, but it speeds up the color table calculations */
   create_rgb_table(&rgb_table, pal, callback_func);
   rgb_map = &rgb_table;

   /* Create the table */
   for (x=0; x<PAL_SIZE; x++)
     {
     for (y=0; y<=x/* < PAL_SIZE*/; y++)
       {
       c.r = ((int)pal[x].r + (int)pal[y].r) >> 2;
       c.g = ((int)pal[x].g + (int)pal[y].g) >> 2;
       c.b = ((int)pal[x].b + (int)pal[y].b) >> 2;
       translucencytable50[(x<<8)+y] = rgb_map->data[c.r][c.g][c.b];
       translucencytable50[(y<<8)+x] = rgb_map->data[c.r][c.g][c.b];
       }
     callback_func(x);
     }
   for (x=0; x<PAL_SIZE; x++)
     {
     for (y=0; y<PAL_SIZE; y++)
       {
       c.r = ((int)pal[x].r + ((int)pal[y].r)*3) >> 3;
       c.g = ((int)pal[x].g + ((int)pal[y].g)*3) >> 3;
       c.b = ((int)pal[x].b + ((int)pal[y].b)*3) >> 3;
       translucencytable75[(x<<8)+y]=rgb_map->data[c.r][c.g][c.b];
       translucencytable25[(y<<8)+x]=rgb_map->data[c.r][c.g][c.b];
       }
     callback_func(x);
     }

   rgb_map = NULL;
   printf("\n");
}



