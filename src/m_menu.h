// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_menu.h,v 1.4 1998/05/16 09:17:18 killough Exp $
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
//   Menu widget stuff, episode selection and such.
//    
//-----------------------------------------------------------------------------


#ifndef __M_MENU__
#define __M_MENU__



#include "d_event.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
boolean M_Responder (event_t *ev);


// Called by main loop,
// only used for menu (skull cursor) animation.
void M_Ticker (void);

// Called by main loop,
// draws the menus directly into the screen buffer.
void M_Drawer (void);

// Called by D_DoomMain,
// loads the config file.
void M_Init (void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel (void);

void M_ForcedLoadGame(const char *msg); // killough 5/15/98: forced loadgames

extern int traditional_menu;  // display the menu traditional way

#endif    

//----------------------------------------------------------------------------
//
// $Log: m_menu.h,v $
// Revision 1.4  1998/05/16  09:17:18  killough
// Make loadgame checksum friendlier
//
// Revision 1.3  1998/05/03  21:56:53  killough
// Add traditional_menu declaration
//
// Revision 1.2  1998/01/26  19:27:11  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
