// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomdef.c,v 1.2 1997/12/29 19:50:46 pekangas Exp $
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
// $Log: doomdef.c,v $
// Revision 1.2  1997/12/29 19:50:46  pekangas
// Ported to WinNT/95 environment using Watcom C 10.6.
// Everything expect joystick support is implemented, but networking is
// untested. Crashes way too often with problems in FixedDiv().
// Compiles with no warnings at warning level 3, uses MIDAS 1.1.1.
//
// Revision 1.1.1.1  1997/12/28 12:59:08  pekangas
// Initial DOOM source release from id Software
//
//
// DESCRIPTION:
//  DoomDef - basic defines for DOOM, e.g. Version, game mode
//   and skill level, and display parameters.
//
//-----------------------------------------------------------------------------

const char
doomdef_rcsid[] = "$Id: doomdef.c,v 1.2 1997/12/29 19:50:46 pekangas Exp $";


#ifdef __GNUG__
#pragma implementation "doomdef.h"
#endif
#include "doomdef.h"

// Location for any defines turned variables.

// None.


