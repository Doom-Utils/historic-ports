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

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"
#include "exit.h"
#include "d_net.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

int	mb_used = 6;

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

// CPhipps:
// I_Exit
//
// An attempt to combat terrible Linux exit problems
void I_Exit(code) {
  I_ShutdownGraphics();
  exit(code);
  // I must stop ENDOOM printing somehow...?
}

//
// I_Quit
//
static quitting=0;

void I_Quit (void)
{
  if (quitting==1) return; // Avoid recursive quit
  quitting=1;
  if (demorecording)
    G_CheckDemoStatus();
  
  D_QuitNetGame ();
  if (!M_CheckParm("-nosound")) {
    I_ShutdownSound();
    I_ShutdownMusic();
  }
  M_SaveDefaults ();
  D_QuitNetGame();
  I_ShutdownGraphics();
  Z_Close();
  //#ifdef DOSDOOM
  // CPhipps:
  // Boom calls this as an exit handler, so cannot re-exit!
  // but Boom's method completely crashes Linux, so modify it back :-(
  // Darn, that don't work either.
  //  exit(0);
  //#endif
}

void I_WaitVBL(int count)
{
usleep (count * (1000000/70) );
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//
int  I_GetTime (void)
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
int  I_GetMilliTime (void)
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
