//  
// DOSDoom Video Code for 8-Bit Colour. 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
#ifndef __V_VIDEO1__
#define __V_VIDEO1__

#include "dm_type.h"
#include "dm_defs.h"

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
V_DrawPatchTrans8
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch	);

void
V_DrawPatchInDirectTrans8
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch	);


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

// 98-7-10 KM Reduce code redundancy
void V_TextureBackScreen8(char *flatname);


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
