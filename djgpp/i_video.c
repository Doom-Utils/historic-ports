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

#include <allegro.h>

#include "doomstat.h"
#include "i_system.h"
#include "i_allegv.h"
#include "v_res.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomdef.h"

//dosstuff -newly added
extern int timingdemo;

int DX,DXI,DY,DYI,DY2,DYI2;
int retrace;                // Wait for vsync, stops shearing w/out dblbuf.
int SCREENWIDTH;
int SCREENHEIGHT;
int SCREENPITCH;
int BPP;
fixed_t weirdaspect;
boolean graphicsmode;

byte *hicolortable;
short hicolortransmask1,hicolortransmask2;


void calctranslucencytable();
char *translucencytable25;
char *translucencytable50;
char *translucencytable75;
//end of newly added stuff

void I_AutodetectBPP()
  {
  BPP = get_config_int("system", "bpp", 1);
  if (M_CheckParm("-hicolor"))
    BPP=2;
  }

void I_GetResolution(void)
{

 int p;

  SCREENWIDTH =  get_config_int("system", "screenwidth", 320);
  SCREENHEIGHT = get_config_int("system", "screenheight", 200);

  p=M_CheckParm("-width");
  if (p && p < myargc-1)
    SCREENWIDTH=atoi(myargv[p+1]);
  p=M_CheckParm("-height");
  if (p && p < myargc-1)
    SCREENHEIGHT=atoi(myargv[p+1]);

  DX  =(SCREENWIDTH<<16)  / 320;
  DXI =(320<<16)          / SCREENWIDTH;
  DY  =(SCREENHEIGHT<<16) / 200;
  DYI =(200<<16)          / SCREENHEIGHT;
  DY2 =DY                 / 2;
  DYI2=DYI                * 2;
}

//
// I_StartFrame
//
void I_StartFrame (void)
  {
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
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH*BPP + i*BPP)] = 0xff;
       if (BPP==2)
	      screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH*BPP + i*BPP)+1] = 0xff;
       }
	for ( ; i<35*2 ; i+=2)
       {
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH*BPP + i*BPP)] = 0x0;
       if (BPP==2)
	      screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH*BPP + i*BPP)+1] = 0x0;
       }
    }

  //Give the option of using vsync.
  if (retrace && !timingdemo) vsync();

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


void I_InitGraphics(void)
  {
  static int firsttime=1;
  int i,j;

  if (!firsttime)
    return;
  firsttime=0;


  //calc translucencytable if needed
  if ((BPP==1)&&(!M_CheckParm("-notrans")))
    calctranslucencytable();
  //enter graphics mode
  enter_graphics_mode();
  graphicsmode = true;

  //do the hicolorpal table if necessary
  if (BPP==2)
    {
    byte *tempptr,*tempptr2;

    tempptr=hicolortable=(byte *)Z_Malloc(256*32*9, PU_STATIC, NULL);
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

  }

void I_ShutdownGraphics(void)
{
//  if (KB_CAPSLOCK_FLAG)
//    set_leds(KB_CAPSLOCK_FLAG);
  set_gfx_mode(GFX_TEXT,80,25,0,0);
  graphicsmode = false;
}

/* progress indicator for the translucency table calculations */
void callback_func()
  {
  static int i = 0;

  if (!(15&i++))
    I_Printf(".");
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
  translucencytable25=(char *)Z_Malloc(65536*3, PU_STATIC, NULL);
  translucencytable50=translucencytable25+65536;
  translucencytable75=translucencytable25+65536*2;

  for (i = 0; i < 256; i++) // The palette must first be converted to Allegro's format (RGB values 0-63)
    {
    pal[i].r = thepalette[i*3+0] >> 2;
    pal[i].g = thepalette[i*3+1] >> 2;
    pal[i].b = thepalette[i*3+2] >> 2;
    }
  I_Printf("Calculating translucency table\n");

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
   I_Printf("\n");
}



