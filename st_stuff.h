//
// DOSDoom Status Bar Code
//
// Based on the Doom Source Code,
//
// Released by Id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "dm_type.h"
#include "d_event.h"

// Size of statusbar.
// Now sensitive for scaling.
#define ST_HEIGHT	32*SCREEN_MUL
#define ST_WIDTH 320
#define ST_Y		(SCREENHEIGHT - ST_HEIGHT)


//
// STATUS BAR
//

// Called by main loop.
boolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);

// Called by main loop.
void ST_Drawer (boolean fullscreen, boolean refresh);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called when changing resolution
void ST_ReInit (void);

// Called by startup code.
void ST_Init (void);



// States for status bar code.
typedef enum
{
    AutomapState,
    FirstPersonState
    
} st_stateenum_t;


// States for the chat code.
typedef enum
{
    StartChatState,
    WaitDestState,
    GetChatState
    
} st_chatstateenum_t;


boolean ST_Responder(event_t* ev);



#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
