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
// DESCRIPTION:
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "d_ticcmd.h"
#include "d_event.h"

#ifdef __GNUG__
#pragma interface
#endif


// Called by DoomMain.
void I_Init (void);

// Called by startup code
// to get the ammount of memory to malloc
// for the zone management.
byte*	I_ZoneBase (int *size);


// Called by D_DoomLoop,
// returns current time in tics.
int I_GetTime (void);


//
// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
void I_StartFrame (void);


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic (void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void I_Quit (void) __attribute__((noreturn));


// Allocates from low memory under dos,
// just mallocs under unix
byte* I_AllocLow (int length);

void I_Tactile (int on, int off, int total);


void I_Error (char *error, ...) __attribute__((noreturn, format(printf, 1, 2)));
//#define I_Printf(message, args...) printf(message, ## args)
void I_Printf (char *message, ...) __attribute__((format(printf, 1, 2)));

#ifdef DEVELOPERS
#define Debug_Printf(format, args...) \
  if (debugfile) fprintf(debugfile, "%s: ", __FUNCTION__);\
  if (debugfile) fprintf(debugfile, format, ## args)
#else
#define Debug_Printf(format, args...)
#endif
// Done this way so that other systems can use non dos ways.
void I_Window(int left, int top, int right, int bottom);
      
void I_TextAttr(int attr);
      
void I_ClrScr(void);

void I_PutTitle(char *title);
#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
