// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_sky.h,v 1.4 1998/05/03 22:56:25 killough Exp $
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
//      Sky rendering.
//
//-----------------------------------------------------------------------------

#ifndef __R_SKY__
#define __R_SKY__

#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

// SKY, store the number for name.
#define SKYFLATNAME  "F_SKY1"

// The sky map is 256*128*4 maps.
#define ANGLETOSKYSHIFT         22

extern int skytexture;
extern int skytexturemid;

// Called whenever the view size changes.
void R_InitSkyMap(void);

#endif

//----------------------------------------------------------------------------
//
// $Log: r_sky.h,v $
// Revision 1.4  1998/05/03  22:56:25  killough
// Add m_fixed.h #include
//
// Revision 1.3  1998/05/01  14:15:29  killough
// beautification
//
// Revision 1.2  1998/01/26  19:27:46  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:09  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
