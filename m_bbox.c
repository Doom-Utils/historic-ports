// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_bbox.c,v 1.4 1998/05/05 19:55:56 phares Exp $
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
//
// DESCRIPTION:
//      Main loop menu stuff.
//      Random number LUT.
//      Default Config File.
//      PCX Screenshots.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_bbox.c,v 1.4 1998/05/05 19:55:56 phares Exp $";


#ifdef __GNUG__
#pragma implementation "m_bbox.h"
#endif
#include "m_bbox.h"

void M_ClearBox (fixed_t *box)
  {
  box[BOXTOP] = box[BOXRIGHT] = MININT;
  box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
  }

void M_AddToBox(fixed_t* box,fixed_t x,fixed_t y)
  {
  if (x<box[BOXLEFT])
    box[BOXLEFT] = x;
  else if (x>box[BOXRIGHT])
    box[BOXRIGHT] = x;
  if (y<box[BOXBOTTOM])
    box[BOXBOTTOM] = y;
  else if (y>box[BOXTOP])
    box[BOXTOP] = y;
  }

//----------------------------------------------------------------------------
//
// $Log: m_bbox.c,v $
// Revision 1.4  1998/05/05  19:55:56  phares
// Formatting and Doc changes
//
// Revision 1.3  1998/05/03  22:52:12  killough
// beautification
//
// Revision 1.2  1998/01/26  19:23:42  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
