// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_items.h,v 1.3 1998/05/04 21:34:12 thldrmn Exp $
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
//  Items: key cards, artifacts, weapon, ammunition.
//
//-----------------------------------------------------------------------------


#ifndef __D_ITEMS__
#define __D_ITEMS__

#include "doomdef.h"

#ifdef __GNUG__
#pragma interface
#endif


// Weapon info: sprite frames, ammunition use.
typedef struct
{
  ammotype_t  ammo;
  int         upstate;
  int         downstate;
  int         readystate;
  int         atkstate;
  int         flashstate;

} weaponinfo_t;

extern  weaponinfo_t    weaponinfo[NUMWEAPONS];

#endif

//----------------------------------------------------------------------------
//
// $Log: d_items.h,v $
// Revision 1.3  1998/05/04  21:34:12  thldrmn
// commenting and reformatting
//
// Revision 1.2  1998/01/26  19:26:26  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:07  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
