// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: doomstat.c,v 1.5 1998/05/12 12:46:12 phares Exp $
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
//      Put all global state variables here.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: doomstat.c,v 1.5 1998/05/12 12:46:12 phares Exp $";

#ifdef __GNUG__
#pragma implementation "doomstat.h"
#endif
#include "doomstat.h"

// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t gamemode = indetermined;
GameMission_t   gamemission = doom;

// Language.
Language_t   language = english;

// Set if homebrew PWAD stuff has been added.
boolean modifiedgame;

//-----------------------------------------------------------------------------

// compatibility with old engines (monster behavior, metrics, etc.)
int compatibility, default_compatibility;          // killough 1/31/98

// Only true when playing back an old demo -- used only in "corner cases"
// which break playback but are otherwise unnoticable or are just desirable:

int demo_compatibility, default_compatibility;     // killough 1/16/98

// v1.1-like pitched sounds
int pitched_sounds, default_pitched_sounds;        // killough

int     default_translucency; // config file says           // phares
boolean general_translucency; // true if translucency is ok // phares

int demo_insurance, default_demo_insurance;        // killough 1/16/98

int  allow_pushers = 1;      // MT_PUSH Things              // phares 3/10/98
int  default_allow_pushers;  // killough 3/1/98: make local to each game

int  variable_friction = 1;      // ice & mud               // phares 3/10/98
int  default_variable_friction;  // killough 3/1/98: make local to each game

int  weapon_recoil;              // weapon recoil                   // phares
int  default_weapon_recoil;      // killough 3/1/98: make local to each game

int player_bobbing;  // whether player bobs or not          // phares 2/25/98
int default_player_bobbing;  // killough 3/1/98: make local to each game

int monsters_remember;          // killough 3/1/98
int default_monsters_remember;

//----------------------------------------------------------------------------
//
// $Log: doomstat.c,v $
// Revision 1.5  1998/05/12  12:46:12  phares
// Removed OVER_UNDER code
//
// Revision 1.4  1998/05/05  16:29:01  phares
// Removed RECOIL and OPT_BOBBING defines
//
// Revision 1.3  1998/05/03  23:12:13  killough
// beautify, move most global switch variables here
//
// Revision 1.2  1998/01/26  19:23:10  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:06  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
