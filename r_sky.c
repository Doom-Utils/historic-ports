// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_sky.c,v 1.6 1998/05/03 23:01:06 killough Exp $
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
//  Sky rendering. The DOOM sky is a texture map like any
//  wall, wrapping around. A 1024 columns equal 360 degrees.
//  The default sky map is 256 columns and repeats 4 times
//  on a 320 screen?
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: r_sky.c,v 1.6 1998/05/03 23:01:06 killough Exp $";

#ifdef __GNUG__
#pragma implementation "r_sky.h"
#endif
#include "r_sky.h"

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap (void)
{
  skytexturemid = 100*FRACUNIT;
}

//----------------------------------------------------------------------------
//
// $Log: r_sky.c,v $
// Revision 1.6  1998/05/03  23:01:06  killough
// beautification
//
// Revision 1.5  1998/05/01  14:14:24  killough
// beautification
//
// Revision 1.4  1998/02/05  12:14:31  phares
// removed dummy comment
//
// Revision 1.3  1998/01/26  19:24:49  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/19  16:17:59  rand
// Added dummy line to be removed later.
//
// Revision 1.1.1.1  1998/01/19  14:03:07  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
