// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_video.h,v 1.4 1998/05/03 22:40:58 killough Exp $
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

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode

void I_InitGraphics (void);
void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (byte* palette);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen (byte* scr);

int I_DoomCode2ScanCode(int);   // killough
int I_ScanCode2DoomCode(int);   // killough

extern int use_vsync;  // killough 2/8/98: controls whether vsync is called

#endif

//----------------------------------------------------------------------------
//
// $Log: i_video.h,v $
// Revision 1.4  1998/05/03  22:40:58  killough
// beautification
//
// Revision 1.3  1998/02/09  03:01:51  killough
// Add vsync for flicker-free blits
//
// Revision 1.2  1998/01/26  19:27:01  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:08  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
