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
//
//    
//-----------------------------------------------------------------------------


#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"
//
// MISC
//
typedef struct
{
    char*	name;
    int*	location;
    int		defaultvalue;
    int		scantranslate;		// PC scan code hack
    int		untranslated;		// lousy hack
} default_t;



boolean
M_WriteFile
( char const*	name,
  void*		source,
  int		length );

int
M_ReadFile
( char const*	name,
  byte**	buffer );

void M_ScreenShot (void);

void M_LoadDefaults (void);

void M_SaveDefaults (void);


int
M_DrawText
( int		x,
  int		y,
  boolean	direct,
  char*		string );


extern int	key_right;
extern int	key_left;
extern int      key_lookup;
extern int      key_lookdown;
extern int      key_lookcenter;
extern int	key_up;
extern int	key_down;

extern int	key_strafeleft;
extern int	key_straferight;

extern int	key_fire;
extern int	key_use;
extern int	key_strafe;
extern int	key_speed;
extern int      key_nextweapon;
extern int      key_jump;
extern int      key_map;
extern int      key_180;
extern int      key_talk;

extern int	mousebfire;
extern int	mousebstrafe;
extern int	mousebforward;

extern int	joybfire;
extern int	joybstrafe;
extern int	joybuse;
extern int	joybspeed;


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
