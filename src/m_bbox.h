// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_bbox.h,v 1.2 1997/12/29 19:51:00 pekangas Exp $
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
//    Nil.
//    
//-----------------------------------------------------------------------------


#ifndef __M_BBOX__
#define __M_BBOX__

#include "doomtype.h"

#include "m_fixed.h"


// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};	// bbox coordinates

// Bounding box functions.
void M_ClearBox (fixed_t*	box);

void
M_AddToBox
( fixed_t*	box,
  fixed_t	x,
  fixed_t	y );


#endif
//-----------------------------------------------------------------------------
//
// $Log: m_bbox.h,v $
// Revision 1.2  1997/12/29 19:51:00  pekangas
// Ported to WinNT/95 environment using Watcom C 10.6.
// Everything expect joystick support is implemented, but networking is
// untested. Crashes way too often with problems in FixedDiv().
// Compiles with no warnings at warning level 3, uses MIDAS 1.1.1.
//
// Revision 1.1.1.1  1997/12/28 12:59:03  pekangas
// Initial DOOM source release from id Software
//
//
//-----------------------------------------------------------------------------
