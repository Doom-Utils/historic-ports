// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_local.h,v 1.2 1998/01/04 19:29:02 pekangas Exp $
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
//	Refresh (R_*) module, global header.
//	All the rendering/drawing stuff is here.
//
//-----------------------------------------------------------------------------

#ifndef __R_LOCAL__
#define __R_LOCAL__

// Binary Angles, sine/cosine/atan lookups.
#include "tables.h"

// Screen size related parameters.
#include "doomdef.h"

// Include the refresh/render data structs.
#include "r_data.h"



//
// Separate header file for each module.
//
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_data.h"
#include "r_things.h"
#include "multires.h"

#endif		// __R_LOCAL__
//-----------------------------------------------------------------------------
//
// $Log: r_local.h,v $
// Revision 1.2  1998/01/04 19:29:02  pekangas
// Added hicolor and multiresulution support from Chi Hoang's DOS port
//
// Revision 1.1.1.1  1997/12/28 12:59:05  pekangas
// Initial DOOM source release from id Software
//
//
//-----------------------------------------------------------------------------
