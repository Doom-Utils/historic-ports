// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_saveg.h,v 1.5 1998/05/03 23:10:40 killough Exp $
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
//      Savegame I/O, archiving, persistence.
//
//-----------------------------------------------------------------------------

#ifndef __P_SAVEG__
#define __P_SAVEG__

#ifdef __GNUG__
#pragma interface
#endif

// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayers(void);
void P_UnArchivePlayers(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ArchiveThinkers(void);
void P_UnArchiveThinkers(void);
void P_ArchiveSpecials(void);
void P_UnArchiveSpecials(void);

// 1/18/98 killough: add RNG info to savegame
void P_ArchiveRNG(void);
void P_UnArchiveRNG(void);

// 2/21/98 killough: add automap info to savegame
void P_ArchiveMap(void);
void P_UnArchiveMap(void);

extern byte *save_p;
void CheckSaveGame(size_t);              // killough

#endif

//----------------------------------------------------------------------------
//
// $Log: p_saveg.h,v $
// Revision 1.5  1998/05/03  23:10:40  killough
// beautification
//
// Revision 1.4  1998/02/23  04:50:09  killough
// Add automap marks and properties to saved state
//
// Revision 1.3  1998/02/17  06:26:04  killough
// Remove unnecessary plat archive/unarchive funcs
//
// Revision 1.2  1998/01/26  19:27:26  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:06  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
