// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-2000 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
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
// DESCRIPTION:
//	Put all global state variables here.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#ifdef __GNUG__
#pragma implementation "doomstat.h"
#endif
#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t	gamemode = indetermined;
GameMission_t	gamemission = doom;

// Language.
Language_t	language = english;

// Set if homebrew PWAD stuff has been added.
boolean		modifiedgame;

// Set if MT_PUSH thing is allowed
int		allow_pushers = 1;

// Set if friction is allowed
int		variable_friction = 1;

// whether you're on the ground, for ice purpose
boolean		onground;
