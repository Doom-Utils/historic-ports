// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hu_stuff.h,v 1.6 1998/05/10 19:03:50 jim Exp $
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
// DESCRIPTION:  Head up display
//
//-----------------------------------------------------------------------------

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"

//
// Globally visible constants.
//
#define HU_FONTSTART    '!'     /* the first font characters */
#define HU_FONTEND      (0x7f) /*jff 2/16/98 '_' the last font characters */

// Calculate # of glyphs in font.
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1) 

#define HU_BROADCAST    5

//#define HU_MSGREFRESH   KEYD_ENTER                                // phares
#define HU_MSGX         0
#define HU_MSGY         0
#define HU_MSGWIDTH     64      /* in characters */
#define HU_MSGHEIGHT    1       /* in lines */

#define HU_MSGTIMEOUT   (4*TICRATE)

//
// Heads up text
//
void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);

// killough 5/2/98: moved from m_misc.c:

//jff 2/16/98 hud supported automap colors added
extern int hudcolor_titl;   // color range of automap level title
extern int hudcolor_xyco;   // color range of new coords on automap
//jff 2/16/98 hud text colors, controls added
extern int hudcolor_mesg;   // color range of scrolling messages
extern int hudcolor_chat;   // color range of chat lines
//jff 2/26/98 hud message list color and background enable
extern int hudcolor_list;   // color of list of past messages
extern int hud_list_bgon;   // solid window background for list of messages
extern int hud_msg_lines;   // number of message lines in window up to 16
extern int hud_distributed; // whether hud is all in lower left or distributed
//jff 2/23/98 hud is currently displayed
extern int hud_displayed;   // hud is displayed
//jff 2/18/98 hud/status control
extern int hud_active;      // hud mode 0=off, 1=small, 2=full
extern int hud_nosecrets;   // status does not list secrets/items/kills

#endif

//----------------------------------------------------------------------------
//
// $Log: hu_stuff.h,v $
// Revision 1.6  1998/05/10  19:03:50  jim
// formatted/documented hu_stuff
//
// Revision 1.5  1998/05/03  22:25:03  killough
// add external declarations for hud options
//
// Revision 1.4  1998/02/18  00:59:04  jim
// Addition of HUD
//
// Revision 1.3  1998/02/15  02:48:12  phares
// User-defined keys
//
// Revision 1.2  1998/01/26  19:26:54  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:56  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
