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


#ifndef __D_EVENT__
#define __D_EVENT__


#include "dm_type.h"


//
// Event handling.
//

// Input event types.
// -KM- 1998/09/01 Amalgamate joystick/mouse into analogue
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_analogue
} evtype_t;

// Event structure.
// -KM- 1998/09/01 Added another field
typedef struct
{
    evtype_t	type;
    int		data1;		// buttons (keys) / data2 axis
    int		data2;		// analogue axis 1
    int		data3;		// data4 axis
    int         data4;          // analogue axis 2
} event_t;

// -KM- 1998/11/25 Added support for finales before levels
typedef enum
{
    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_loadgame,
    ga_savegame,
    ga_playdemo,
    ga_completed,
    ga_victory,
    ga_worlddone,
    ga_screenshot,
    ga_briefing
} gameaction_t;



//
// Button/action code definitions.
//
typedef enum
{
    // Press "Fire".
    BT_ATTACK		= 1,
    // Use button, to open doors, activate switches.
    BT_USE		= 2,

    // Flag: game events, not really buttons.
    BT_SPECIAL		= 128,
    BT_SPECIALMASK	= 3,
    
    // Flag, weapon change pending.
    // If true, the next 3 bits hold weapon num.
    BT_CHANGE		= 4,

    // The 3bit weapon mask and shift, convenience.
    // -KM- 1998/11/25 Allow 10 weapon keys
    BT_WEAPONMASK	= (8+16+32+64),
    BT_WEAPONSHIFT	= 3,

    // Pause the game.
    BTS_PAUSE		= 1,
    // Save the game at each console.
    BTS_SAVEGAME	= 2,

    // Savegame slot numbers
    //  occupy the second byte of buttons.    
    BTS_SAVEMASK	= (4+8+16),
    BTS_SAVESHIFT 	= 2,
  
} buttoncode_t;

//
// Extended Buttons: DOSDoom Specfics
// -ACB- 1998/07/03
//
typedef enum
{
    EBT_JUMP             = 1,
    EBT_VERTLOOK         = 2,
    EBT_CENTER           = 4
} extbuttoncode_t;

//
// GLOBAL VARIABLES
//
#define MAXEVENTS		64

extern  event_t		events[MAXEVENTS];
extern  int             eventhead;
extern	int		eventtail;

extern  gameaction_t    gameaction;


#endif

