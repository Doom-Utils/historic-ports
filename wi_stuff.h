// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: wi_stuff.h,v 1.3 1998/05/04 21:36:12 thldrmn Exp $
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
//  Intermission.
//
//-----------------------------------------------------------------------------

#ifndef __WI_STUFF__
#define __WI_STUFF__

//#include "v_video.h"

#include "doomdef.h"

// States for the intermission

typedef enum
{
  NoState = -1,
  StatCount,
  ShowNextLoc

} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker (void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer (void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t*   wbstartstruct);

#endif

//----------------------------------------------------------------------------
//
// $Log: wi_stuff.h,v $
// Revision 1.3  1998/05/04  21:36:12  thldrmn
// commenting and reformatting
//
// Revision 1.2  1998/01/26  19:28:03  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:05  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
