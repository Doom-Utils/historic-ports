// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_cheat.h,v 1.5 1998/05/03 22:10:56 killough Exp $
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
//      Cheat code checking.
//
//-----------------------------------------------------------------------------

#ifndef __M_CHEAT__
#define __M_CHEAT__

// killough 4/16/98: Cheat table structure

extern struct cheat_s {
  const unsigned char *cheat;
  const char *const deh_cheat;
  enum { 
    always   = 0,
    not_dm   = 1,
    not_coop = 2,
    not_demo = 4, 
    not_menu = 8,
    not_deh = 16,
    not_net = not_dm | not_coop
  } const when;
  void (*const func)();
  const int arg;
  unsigned long long code, mask;
} cheat[];

boolean M_FindCheats(int key);

extern int idmusnum;

#endif

//----------------------------------------------------------------------------
//
// $Log: m_cheat.h,v $
// Revision 1.5  1998/05/03  22:10:56  killough
// Cheat engine, moved from st_stuff
//
// Revision 1.4  1998/05/01  14:38:08  killough
// beautification
//
// Revision 1.3  1998/02/09  03:03:07  killough
// Rendered obsolete by st_stuff.c
//
// Revision 1.2  1998/01/26  19:27:08  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
