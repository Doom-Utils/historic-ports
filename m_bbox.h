// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_bbox.h,v 1.3 1998/05/05 19:55:58 phares Exp $
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

#include "z_zone.h"         // killough 1/18/98

#include <values.h>
#include "m_fixed.h"

// Bounding box coordinate storage.
enum
{
  BOXTOP,
  BOXBOTTOM,
  BOXLEFT,
  BOXRIGHT
};  // bbox coordinates

// Bounding box functions.

void M_ClearBox(fixed_t* box);

void M_AddToBox(fixed_t* box,fixed_t x,fixed_t y);

#endif

//----------------------------------------------------------------------------
//
// $Log: m_bbox.h,v $
// Revision 1.3  1998/05/05  19:55:58  phares
// Formatting and Doc changes
//
// Revision 1.2  1998/01/26  19:27:06  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
