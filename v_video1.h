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
//	Gamma correction LUT.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------


//this is for 8bpp modes

#ifndef __V_VIDEO1__
#define __V_VIDEO1__

#include "doomtype.h"
#include "doomdef.h"
// Needed because we are refering to patches.
#include "r_data.h"


// Allocates buffer screens, call before R_Init.
void V_Init8 (void);

void
V_CopyRect8
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn );

void
V_DrawPatch8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

void
V_DrawPatchFlipped8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

void
V_DrawPatchDirect8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void  //stretches bitmap to full screen
V_DrawPatchInDirect8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void  //stretches bitmap to full screen
V_DrawPatchInDirectFlipped8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void
V_DrawPatchShrink8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

// Draw a linear block of pixels into the view buffer.
void
V_DrawBlock8
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		src );

// Reads a linear block of pixels into the view buffer.
void
V_GetBlock8
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest );


void
V_MarkRect8
( int		x,
  int		y,
  int		width,
  int		height );

void V_DarkenScreen8(int scrn);

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
