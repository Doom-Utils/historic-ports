// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_setup.h,v 1.3 1998/05/03 23:03:31 killough Exp $
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
//   Setup a game, startup stuff.
//
//-----------------------------------------------------------------------------

#ifndef __P_SETUP__
#define __P_SETUP__

#include "p_mobj.h"

#ifdef __GNUG__
#pragma interface
#endif

void P_SetupLevel(int episode, int map, int playermask, skill_t skill);
void P_Init(void);               // Called by startup code.

extern byte     *rejectmatrix;   // for fast sight rejection

// killough 3/1/98: change blockmap from "short" to "long" offsets:
extern long     *blockmaplump;   // offsets in blockmap are from here
extern long     *blockmap;
extern int      bmapwidth;
extern int      bmapheight;      // in mapblocks
extern fixed_t  bmaporgx;
extern fixed_t  bmaporgy;        // origin of block map
extern mobj_t   **blocklinks;    // for thing chains

#endif

//----------------------------------------------------------------------------
//
// $Log: p_setup.h,v $
// Revision 1.3  1998/05/03  23:03:31  killough
// beautification, add external declarations for blockmap
//
// Revision 1.2  1998/01/26  19:27:28  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:08  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
