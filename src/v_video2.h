// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: v_video2.h,v 1.2 1998/01/27 15:42:22 pekangas Exp $
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


//this is for 16 bpp modes

#ifndef __V_VIDEO2__
#define __V_VIDEO2__

#include "doomtype.h"
#include "doomdef.h"
// Needed because we are refering to patches.
#include "r_data.h"


// Allocates buffer screens, call before R_Init.
void V_Init16 (void);

void
V_CopyRect16
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn );

void
V_DrawPatch16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

void
V_DrawPatchFlipped16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

void
V_DrawPatchDirect16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void  //Stretches bitmap to full screen
V_DrawPatchInDirect16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void  //Stretches bitmap to full screen
V_DrawPatchInDirectFlipped16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

// Draw a linear block of pixels into the view buffer.
void
V_DrawBlock16
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		src );

// Reads a linear block of pixels into the view buffer.
void
V_GetBlock16
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest );


void
V_MarkRect16
( int		x,
  int		y,
  int		width,
  int		height );

void V_ClearScreen16(int scrn);

#endif
//-----------------------------------------------------------------------------
//
// $Log: v_video2.h,v $
// Revision 1.2  1998/01/27 15:42:22  pekangas
// Merged a bunch of highcolor and other fixes from Chi Hoang's
// DOSDOOm 0.45
//
//
//-----------------------------------------------------------------------------
