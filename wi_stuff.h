//  
// DOSDoom Intermission Screen Code
//
// Based on the Doom Source Code,
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
#ifndef __WI_STUFF__
#define __WI_STUFF__

//#include "v_video.h"

#include "dm_defs.h"

// States for the intermission

typedef enum
{
    NoState = -1,
    StatCount,
    ShowNextLoc

} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker (void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer (void);

// Setup for an intermission screen.
void WI_Start(wbstartstruct_t*	 wbstartstruct);


extern wi_map_t        worldmap;
void WI_MapInit(wi_map_t* map);

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
