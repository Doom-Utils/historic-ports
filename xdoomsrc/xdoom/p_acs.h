// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
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
// DESCRIPTION: ACS Script support for XDoomPlus
//
//-----------------------------------------------------------------------------

//
// Hexen sources from Raven Software have been examined to implement
// this.
//

#ifndef __P_ACS__
#define __P_ACS__

#define MAX_ACS_SCRIPT_VARS	10	// max no. of local variables/script
#define MAX_ACS_MAP_VARS	32	// max no. of map variables
#define MAX_ACS_WORLD_VARS	64	// max no. of world variables
#define STACK_SIZE		32	// stack size for P code engine
#define RUNAWAY			10000	// no. of instruction cycles allowed

// The following game type defines must match the defines in the
// include files for the script compiler.
#define GAME_SINGLE_PLAYER	0
#define GAME_NET_COOPERATIVE	1
#define GAME_NET_DEATHMATCH	2
#define GAME_NET_ALTCOOP	3

// The following defines for texture position must match the defines
// in the include file for the compiler
#define TEXTURE_TOP		0
#define TEXTURE_MIDDLE		1
#define TEXTURE_BOTTOM		2

// Valid script numbers are 0 - 999. The following offset is added
// by the compiler to a script number, when the script is declared
// OPEN. This are the scripts automatically started at level start.
#define OPEN_SCRIPTS_BASE	1000

//
// Events a script can wait on
//
typedef enum
{
	wait_sector,
	wait_poly,
	wait_script
} ev_wait;

//
// structure of the header in BEHAVIOR lump
//
typedef struct
{
	int		marker;		// ACS marker, 0 terminated
	int		info;		// offset to info section
	int		code;		// start of P code
} acs_header_t;

//
// structure of script control blocks
//
typedef struct acs_s
{
	struct acs_s	*next;		// linked list pointer
	struct acs_s	*prev;		// dito
	struct acs_s	**queue;	// the queue it is linked into
	int		snum;		// script number
	int		*ip;		// P code instruction pointer
	int		*code;		// P code start address
	int		stack[STACK_SIZE]; // stack for P code engine
	int		sp;		// stack pointer
	int		argc;		// number of arguments
	int		vars[MAX_ACS_SCRIPT_VARS]; // local variables
	int		timer;		// delay timer
	ev_wait		wait_event;	// wait for event
	int		wait_value;	// wait value
	int		inst_count;	// P codes executed
	mobj_t		*activator;	// thing which activated the script
	line_t		*line;		// line which activated the script
	int		side;		// side of the line which caused activ.
} acs_t;

extern void P_ACSInitNewGame(void);
extern void P_LoadACScripts(int);
extern void P_RunScripts(void);
extern void P_StartACS(int, int, byte *, mobj_t *, line_t *, int);
extern void P_SuspendACS(int, int);
extern void P_TerminateACS(int, int);

#endif
