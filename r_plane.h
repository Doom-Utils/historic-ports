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
//	Refresh, visplane stuff (floor, ceilings).
// -KM- 1998/09/27 Dynamic colourmaps
//-----------------------------------------------------------------------------


#ifndef __R_PLANE__
#define __R_PLANE__


#include "r_data.h"

#ifdef __GNUG__
#pragma interface
#endif


// Visplane related.
extern  short*		lastopening;
extern	visplane_t*	visplanes;

typedef void (*planefunction_t) (int top, int bottom);

extern planefunction_t	floorfunc;
extern planefunction_t	ceilingfunc_t;

extern short		*floorclip;
extern short		*ceilingclip;

extern fixed_t		*yslope;
extern fixed_t          *origyslope;
extern fixed_t		*distscale;

void resinit_r_plane_c(void);  //called before anything else

void R_InitPlanes (void);
void R_ClearPlanes (void);

void
R_MapPlane
( int		y,
  int		x1,
  int		x2 );

void
R_MakeSpans
( int		x,
  int		t1,
  int		b1,
  int		t2,
  int		b2 );

void R_DrawPlanes (void);

int
R_FindPlane
( fixed_t	height,
  int		picnum,
  int		lightlevel,
  int           colourmaplump,
  int           colourmap);

int
R_CheckPlane
( int		pl,
  int		start,
  int		stop );



#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
