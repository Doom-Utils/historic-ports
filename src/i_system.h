// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_system.h,v 1.7 1998/05/03 22:33:43 killough Exp $
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
//      System specific interface stuff.
//
//-----------------------------------------------------------------------------

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "d_ticcmd.h"

#ifdef __GNUG__
#pragma interface
#endif

// Called by DoomMain.
void I_Init(void);

// Called by D_DoomLoop,
// returns current time in tics.
// int I_GetTime (void);

int (*I_GetTime)();           // killough
#ifdef _WIN32
int I_GetTrueTime();     // proff 10/31/98
#endif
int I_GetTime_RealTime();     // killough
int I_GetTime_Adaptive(void); // killough 4/10/98
extern int GetTime_Scale;

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

// atexit handler -- killough

void I_Quit (void);

// Allocates from low memory under dos, just mallocs under unix

#define I_AllocLow(length) calloc((length),1)            /* killough */
#define I_Tactile(on, off, total)

// killough 3/20/98: add const
// killough 4/25/98: add gcc attributes
#ifdef _WIN32 // proff: __attribute__ is not defined in Visual C
void I_Error (const char *error, ...);
#else //_WIN32
void I_Error(const char *error, ...) __attribute__((format(printf,1,2)));
#endif //_WIN32

extern int mousepresent;                // killough

void I_EndDoom(void);         // killough 2/22/98: endgame screen

// killough 3/21/98: keyboard queue

#define KQSIZE 256

extern struct keyboard_queue_s {
  volatile int head,tail,queue[KQSIZE];
} keyboard_queue;

#endif

//----------------------------------------------------------------------------
//
// $Log: i_system.h,v $
// Revision 1.7  1998/05/03  22:33:43  killough
// beautification, remove unnecessary #includes
//
// Revision 1.6  1998/04/27  01:52:47  killough
// Add __attribute__ to I_Error for gcc checking
//
// Revision 1.5  1998/04/10  06:34:07  killough
// Add adaptive gametic timer
//
// Revision 1.4  1998/03/23  03:17:19  killough
// Add keyboard FIFO queue and make I_Error arg const
//
// Revision 1.3  1998/02/23  04:28:30  killough
// Add ENDOOM support
//
// Revision 1.2  1998/01/26  19:26:59  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:10  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
