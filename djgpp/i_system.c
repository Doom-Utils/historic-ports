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
#include <string.h>

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <allegro.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"




int     mb_used = 7;


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


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
  {
  int p;

  p = M_CheckParm ("-heapsize");
  if (p && p < myargc-1)
    {
    mb_used=atoi(myargv[p+1]);
    }

  printf ("Heapsize: %d Megabytes\n",mb_used);
  *size = mb_used*1024*1024;
  return (byte *) malloc (*size);
}


//
// I_Init
//
void I_Init (void)
{
  if (!M_CheckParm("-nosound"))
    I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
int i;
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

    for (i=0;i<=25;i++) printf("\n");
    memcpy((void *)(__djgpp_conventional_base)+0xb8000,W_CacheLumpNum(W_GetNumForName("ENDOOM"),PU_CACHE),4000);
    exit(0);
}

extern volatile int mselapsed;

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
  return (mselapsed*TICRATE/1000);
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
    I_ShutdownGraphics();
    
    // Message last, so i actually prints on the screen
    va_start (argptr,error);
//    fprintf (stderr, "Error: ");
    vfprintf (stdout,error,argptr);
    fprintf (stdout, "\n");
    va_end (argptr);
    fflush( stdout );


    exit(-1);
}
