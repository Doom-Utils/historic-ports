// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_ticcmd.h,v 1.2 1997/12/29 19:50:46 pekangas Exp $
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


#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.
typedef struct
{
    /* [Petteri] *LART*LART*LART* - assumes chars are signed
     *char	forwardmove;	// *2048 for move
     *char	sidemove;	// *2048 for move*/
    signed char	forwardmove;	// *2048 for move
    signed char	sidemove;	// *2048 for move*/
    short	angleturn;	// <<16 for angle delta
    short	consistancy;	// checks for net game
    byte	chatchar;
    byte	buttons;
} ticcmd_t;



#endif
//-----------------------------------------------------------------------------
//
// $Log: d_ticcmd.h,v $
// Revision 1.2  1997/12/29 19:50:46  pekangas
// Ported to WinNT/95 environment using Watcom C 10.6.
// Everything expect joystick support is implemented, but networking is
// untested. Crashes way too often with problems in FixedDiv().
// Compiles with no warnings at warning level 3, uses MIDAS 1.1.1.
//
// Revision 1.1.1.1  1997/12/28 12:59:07  pekangas
// Initial DOOM source release from id Software
//
//
//-----------------------------------------------------------------------------
