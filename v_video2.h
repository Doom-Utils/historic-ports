//  
// DOSDoom Video Code for 16-Bit Colour. 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#ifndef __V_VIDEO2__
#define __V_VIDEO2__

#include "dm_type.h"
#include "dm_defs.h"

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

void
V_DrawPatchTrans16
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch	);

void	//stretches bitmap to fill screen
V_DrawPatchInDirectTrans16
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch	);

void
V_DrawPatchShrink16
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

void V_DarkenScreen16(int scrn);

// 98-7-10 KM Reduce code redundancy
void V_TextureBackScreen16(char *flatname);

#endif

