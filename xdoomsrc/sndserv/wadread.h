// Emacs style mode select   -*- C++ -*- 
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
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
// DESCRIPTION:
//	WAD and Lump I/O, the second.
//	This time for soundserver only.
//	Welcome to Department of Redundancy Department.
//	 (Yeah, I said that elsewhere already).
//	Note: makes up for a nice w_wad.h.
//
//-----------------------------------------------------------------------------

#ifndef __WADREAD_H__
#define __WADREAD_H__

//
//  Opens the wadfile specified.
// Must be called before any calls to  loadlump() or getsfx().
//

void openwad(char *wadname);

//
//  Gets a sound effect from the wad file.  The pointer points to the
//  start of the data.  Returns a 0 if the sfx was not
//  found.  Sfx names should be no longer than 6 characters.  All data is
//  rounded up in size to the nearest MIXBUFFERSIZE and is padded out with
//  0x80's.  Returns the data length in len.
//

void *getsfx(char *sfxname, int *len);

// gets sfx data from PWADs, similar to musserver

void read_extra_wads(void);

#endif
