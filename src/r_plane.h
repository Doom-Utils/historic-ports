// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_plane.h,v 1.6 1998/04/27 01:48:34 killough Exp $
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
//      Refresh, visplane stuff (floor, ceilings).
//
//-----------------------------------------------------------------------------

#ifndef __R_PLANE__
#define __R_PLANE__

#include "r_data.h"

#ifdef __GNUG__
#pragma interface
#endif

// Visplane related.
extern  short *lastopening;

extern short floorclip[], ceilingclip[];
extern fixed_t yslope[], distscale[];

void R_InitPlanes(void);
void R_ClearPlanes(void);
void R_DrawPlanes (void);

visplane_t *R_FindPlane(
                        fixed_t height, 
                        int picnum,
                        int lightlevel,
                        fixed_t xoffs,  // killough 2/28/98: add x-y offsets
                        fixed_t yoffs
                       );

visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop);

#endif

//----------------------------------------------------------------------------
//
// $Log: r_plane.h,v $
// Revision 1.6  1998/04/27  01:48:34  killough
// Program beautification
//
// Revision 1.5  1998/03/02  11:47:16  killough
// Add support for general flats xy offsets
//
// Revision 1.4  1998/02/09  03:16:06  killough
// Change arrays to use MAX height/width
//
// Revision 1.3  1998/02/02  14:20:45  killough
// Made some functions static
//
// Revision 1.2  1998/01/26  19:27:42  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:03  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
