// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_system.c,v 1.3 1998/01/08 17:20:05 pekangas Exp $
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
// $Log: i_system.c,v $
// Revision 1.3  1998/01/08 17:20:05  pekangas
// MIDAS is now uninitialized on an I_Error exit
//
// Revision 1.2  1997/12/29 19:50:55  pekangas
// Ported to WinNT/95 environment using Watcom C 10.6.
// Everything expect joystick support is implemented, but networking is
// untested. Crashes way too often with problems in FixedDiv().
// Compiles with no warnings at warning level 3, uses MIDAS 1.1.1.
//
// Revision 1.1.1.1  1997/12/28 12:59:07  pekangas
// Initial DOOM source release from id Software
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const char
i_system_rcsid[] = "$Id: i_system.c,v 1.3 1998/01/08 17:20:05 pekangas Exp $";

/* [Petteri] Check if compiling for Win32: */
#if defined(__WINDOWS__) || defined(__NT__) || defined(_MSC_VER)
#   define __WIN32__
#endif
/* Follow #ifdef __WIN32__ marks */


/* [Petteri] Windows includes for Win32: */
#ifdef __WIN32__
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <mmsystem.h>
#   include <winsock.h>
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#ifndef __WIN32__
#include <sys/time.h>
#include <unistd.h>
#endif

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

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
    *size = mb_used*1024*1024;
    return (byte *) malloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
#ifdef __WIN32__
    static DWORD startTime = 0;
    DWORD ms;
    int t;

    ms = timeGetTime();
    if ( startTime == 0 )
        startTime = ms;
    t = (int) (((ms - startTime) * TICRATE) / 1000);
    return t;
#else
    struct timeval	tp;
    struct timezone	tzp;
    int			newtics;
    static int		basetime=0;
  
    gettimeofday(&tp, &tzp);
    if (!basetime)
	basetime = tp.tv_sec;
    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;
#endif    
}



//
// I_Init
//
void I_Init (void)
{
#ifdef __WIN32__
    timeBeginPeriod(1);
#endif
    I_InitSound();
    //  I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    
#ifdef __WIN32__    
    WSACleanup();
    timeEndPeriod(1);
#endif
    
    exit(0);
}

void I_WaitVBL(int count)
{
#ifdef SGI
    sginap(1);                                           
#else
#ifdef SUN
    sleep(0);
#else
#ifdef __WIN32__
    Sleep(1000/70); /* [Petteri] Is this a good idea? */
#else    
    usleep (count * (1000000/70) );
#endif    
#endif
#endif
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

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();

    /* [Petteri] We'd better shut down the sound system as well: */
    I_ShutdownSound();

#ifdef __WIN32__    
    WSACleanup();
    timeEndPeriod(1);
#endif        
    
    exit(-1);
}
