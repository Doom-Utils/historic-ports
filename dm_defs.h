// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
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
//  Internally used data structures for virtually everything,
//   key definitions, lots of other stuff.
//
//-----------------------------------------------------------------------------
//
// -ACB- 1998/07/20 Removed unused INVULTICS etc...(Replaced by DDF)
//                  Move Invisibility enum here, more applicable.
//

#ifndef __DOOMDEF__
#define __DOOMDEF__

#include <stdio.h>
#include <string.h>

#include "dm_type.h"
//
// Global parameters/defines.
//
// DOOM version
extern int VERSION, VERSIONFIX, DOSDOOMVER, DOSDOOMVERFIX;
#define DEMOVERSION 065

/*
// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum
{
  shareware,	// DOOM 1 shareware, E1, M9
  registered,	// DOOM 1 registered, E3, M27
  commercial,	// DOOM 2 retail, E1 M34
  retail,	// DOOM 1 retail, E4, M36
  dosdoom,      // Combination of DOOM 1 + 2 E5 M68 :-)
  indetermined	// Well, no IWAD found.  
}
GameMode_t;


// Mission packs - might be useful for TC stuff?
typedef enum
{
  doom,		// DOOM 1
  doom2,	// DOOM 2
  pack_tnt,	// TNT mission pack
  pack_plut,	// Plutonia pack
  none
}
GameMission_t;


// Identify language to use, software localization.
typedef enum
{
  english,
  french,
  german1,
  german2,
  turkish,
  swedish,
  spanish,
  dutch,
  unknown
}
Language_t;
*/
//
// For resize of screen, at start of game.
// It will not work dynamically, see visplanes.
//
#define BASE_WIDTH SCREENWIDTH

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.
#define	SCREEN_MUL		1
#define	INV_ASPECT_RATIO	0.625 // 0.75, ideally

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS	8

// State updates, number of tics / second.
#define TICRATE		35

// The current state of the game: whether we are
// playing, gazing at the intermission screen,
// the game final animation, or a demo. 
typedef enum
{
  GS_LEVEL,
  GS_INTERMISSION,
  GS_FINALE,
  GS_DEMOSCREEN
}
gamestate_t;

//
// Difficulty/skill settings/filters.
//

// Skill flags.
#define	MTF_EASY		1
#define	MTF_NORMAL		2
#define	MTF_HARD		4

// Deaf monsters/do not react to sound.
#define	MTF_AMBUSH		8

typedef enum
{
  sk_baby,
  sk_easy,
  sk_medium,
  sk_hard,
  sk_nightmare
}
skill_t;

// -KM- 1998/12/16 Added gameflags typedef here.
typedef enum
{
  RS_TELEPORT,
  RS_RESURRECT
} respawn_t;

typedef struct gameflags_s
{
  boolean nomonsters; // checkparm of -nomonsters
  boolean fastparm;   // checkparm of -fast

  respawn_t respawnsetting;
  boolean respawn;
  boolean itemrespawn;

  boolean true3dgameplay;
  int grav;
  // -KM- 1998/07/21 Changed from an int to boolean
  boolean blood;
  
  boolean jump;
  boolean freelook;

  boolean trans;
  boolean cheats;

  boolean stretchsky;

  boolean drone;
  angle_t viewangleoffset;
} gameflags_t;

//
// Key cards.
//
typedef enum
{
  it_bluecard,
  it_yellowcard,
  it_redcard,
  it_blueskull,
  it_yellowskull,
  it_redskull,
  NUMCARDS
}
card_t;

// Power up artifacts.
//
// -MH- 1998/06/17  Jet Pack Added
// -ACB- 1998/07/15 NightVision Added
//
typedef enum
{
  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,
  pw_jetpack,
  pw_nightvision,
  NUMPOWERS
}
powertype_t;

// -ACB- 1998/07/10 New enum for visibility
// -KM- 1998/11/25 Visibililty changed to fraction.
#define  VISIBLE FRACUNIT
#define  VISSTEP FRACUNIT/256
#define  INVISIBLE 0

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define KEYD_RIGHTARROW	0xae
#define KEYD_LEFTARROW	0xac
#define KEYD_UPARROW	0xad
#define KEYD_DOWNARROW	0xaf
#define KEYD_ESCAPE	27
#define KEYD_ENTER	13
#define KEYD_TAB	9
#define KEYD_F1		(0x80+0x3b)
#define KEYD_F2		(0x80+0x3c)
#define KEYD_F3		(0x80+0x3d)
#define KEYD_F4		(0x80+0x3e)
#define KEYD_F5		(0x80+0x3f)
#define KEYD_F6		(0x80+0x40)
#define KEYD_F7		(0x80+0x41)
#define KEYD_F8		(0x80+0x42)
#define KEYD_F9		(0x80+0x43)
#define KEYD_F10	(0x80+0x44)
#define KEYD_F11	(0x80+0x57)
#define KEYD_F12	(0x80+0x58)
#define KEYD_BACKSPACE	127
#define KEYD_PAUSE	0xff
#define KEYD_EQUALS	0x3d
#define KEYD_MINUS	0x2d
#define KEYD_RSHIFT	(0x80+0x36)
#define KEYD_RCTRL	(0x80+0x1d)
#define KEYD_RALT	(0x80+0x38)
#define KEYD_LALT	KEYD_RALT
#define KEYD_PRTSCR     (0x80+84)
#define KEYD_PGUP       (0x80+73)
#define KEYD_PGDN       (0x80+81)
#define KEYD_HOME       (0x80+71)
#define KEYD_END        (0x80+79)
#define KEYD_INSERT     (0x80+82)
#define KEYD_DELETE     (0x80+83)
#define KEYD_MOUSE1     0x100
#define KEYD_MOUSE2     0x101
#define KEYD_MOUSE3     0x102
#define KEYD_JOY1       0x110
#define KEYD_JOY2       0x111
#define KEYD_JOY3       0x112
#define KEYD_JOY4       0x113
#define KEYD_JOY5       0x114
#define KEYD_JOY6       0x115
#define KEYD_JOY7       0x116
#define KEYD_JOY8       0x117

// -KM- 1998/09/01 Extended Joystick support
#define KEYD_HATW       0x118
#define KEYD_HATS       0x119
#define KEYD_HATE       0x11a
#define KEYD_HATN       0x11b

// -KM- 1998/09/27 Analogue binding, added a fly axis
#define AXIS_DISABLE     5
#define AXIS_TURN        0
#define AXIS_FORWARD     1
#define AXIS_STRAFE      2
#define AXIS_MLOOK       3
#define AXIS_FLY         4

#endif          // __DOOMDEF__

