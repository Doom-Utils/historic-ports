// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
// $Id:$
//
// Copyright (C) 1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION: This handles the line specials for XDoomPlus.
//
//-----------------------------------------------------------------------------

#ifndef __P_SPECPLUS__
#define __P_SPECPLUS__

// convert the angles in linedef args
#define BYTEANGLE(x)	((angle_t)((x) << 24))

// convert the speed in linedef args
#define SPEED(x)	((x) * (FRACUNIT / 8))

//
// Types of key used by locked doors and scripts.
// Must match the defines in the compilers include file.
//
typedef enum
{
	Key_None,
	Key_Card_Red,
	Key_Card_Blue,
	Key_Card_Yellow,
	Key_Skull_Red,
	Key_Skull_Blue,
	Key_Skull_Yellow,
	Key_Any
} keytype_t;

typedef void (*SpecFunc)(byte *args, line_t *, int, mobj_t *);
extern SpecFunc LineSpecials[];

void P_SpawnSpecialsPlus(void);

#endif
