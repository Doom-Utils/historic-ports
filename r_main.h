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
//	System specific interface stuff.
// -KM- 1998/09/27 Dynamic Colourmaps.
//-----------------------------------------------------------------------------


#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"


#ifdef __GNUG__
#pragma interface
#endif


//
// POV related.
//
extern fixed_t		viewcos;
extern fixed_t		viewsin;

extern int		viewwidth;
extern int		viewheight;
extern int		viewwindowx;
extern int		viewwindowy;



// the x and y coords of the focus
// -ES- 1999/03/19 Renamed center to focus
extern int		focusx;
extern int		focusy;

extern fixed_t		focusxfrac;
extern fixed_t		focusyfrac;

extern int		validcount;

extern int		linecount;

// -ES- 1999/03/29 Added these
extern fixed_t normalfov,zoomedfov;
extern boolean viewiszoomed;

extern boolean setsizeneeded;
extern boolean changeresneeded; // -ES- 1998/08/20

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now why not 32 levels here?
#define LIGHTLEVELS	        16
#define LIGHTSEGSHIFT	         4

#define MAXLIGHTSCALE		48
#define LIGHTSCALESHIFT		12
#define MAXLIGHTZ	       512
#define LIGHTZSHIFT		18

extern int              scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern int              scalelightfixed[MAXLIGHTSCALE];
extern int              zlight[LIGHTLEVELS][MAXLIGHTZ];

extern int		extralight;
extern lighttable_t*	fixedcolormap;


// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS		32


// Blocky/low detail mode.
//B remove this?
//  0 = high, 1 = low
//extern	int		detailshift;
#define detailshift 0


//
// Function pointers to switch refresh/drawing functions.
// Used to select shadow mode etc.
//
extern void		(*colfunc) (void);
extern void		(*basecolfunc) (void);
extern void		(*fuzzcolfunc) (void);
// No shadow effects on floors.
extern void		(*spanfunc) (void);


//
// Utility functions.
int
R_PointOnSide
( fixed_t	x,
  fixed_t	y,
  node_t*	node );

int
R_PointOnSegSide
( fixed_t	x,
  fixed_t	y,
  seg_t*	line );

angle_t
R_PointToAngle
( fixed_t	x,
  fixed_t	y );

angle_t
R_PointToAngle2
( fixed_t	x1,
  fixed_t	y1,
  fixed_t	x2,
  fixed_t	y2 );

fixed_t
R_PointToDist
( fixed_t	x,
  fixed_t	y );


fixed_t R_ScaleFromGlobalAngle (angle_t visangle);

subsector_t*
R_PointInSubsector
( fixed_t	x,
  fixed_t	y );

void
R_AddPointToBox
( int		x,
  int		y,
  fixed_t*	box );



//
// REFRESH - the actual rendering functions.
//

// Called by R_Render.
void R_RenderPlayerView (player_t *player);

// Renders the view for the next frame.
extern void R_Render (void);

// Called by startup code.
void R_Init (void);

// -ES- 1998/09/11 Added these prototypes.
void R_SetViewSize (int	 blocks, int detail);

// Changes Field of View to the specified angle.
void R_SetFOV(angle_t fov);

// Changes the FOV variables that the zoom key toggles between.
void R_SetNormalFOV(angle_t newfov);
void R_SetZoomedFOV(angle_t newfov);

// call this to change the resolution before the next frame.
void R_ChangeResolution (int width, int height, int bpp);


// Called by M_Responder.
void R_SetViewSize (int blocks, int detail);

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
