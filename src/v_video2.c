// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: v_video2.c,v 1.2 1998/01/05 13:41:08 pekangas Exp $
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
// $Log: v_video2.c,v $
// Revision 1.2  1998/01/05 13:41:08  pekangas
// No more duplicate symbols
//
// Revision 1.1  1998/01/04 19:28:23  pekangas
// Initial revision
//
//
// DESCRIPTION:
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

//this is for 16bpp modes

const char
v_video2_rcsid[] = "$Id: v_video2.c,v 1.2 1998/01/05 13:41:08 pekangas Exp $";


#include <stdio.h>

#include	"i_system.h"
#include	"r_local.h"

#include	"doomdef.h"
#include	"doomdata.h"
#include	"doomstat.h"

#include	"m_bbox.h"
#include	"m_swap.h"

#include	"v_video2.h"
#include	"w_wad.h"
#include	"z_zone.h"

#include "multires.h"

/* [Petteri] no-allegro-hack: */
#ifndef palette_color
extern int palette_color[256];
#endif

/* [Petteri] From v_video1: */
extern int				dirtybox[4];

//
// V_MarkRect16
// 
void
V_MarkRect16
( int		x,
  int		y,
  int		width,
  int		height )	
{ 
	 M_AddToBox	(dirtybox, x, y);	
	 M_AddToBox	(dirtybox, x+width-1, y+height-1); 
} 
 

//
// V_CopyRect16
// 
void
V_CopyRect16
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn	) 
{ 
	 byte*	src;
	 byte*	dest;	
	 
#ifdef RANGECHECK	
	 if (srcx<0
	||srcx+width >SCREENWIDTH
	||	srcy<0
	||	srcy+height>SCREENHEIGHT 
	||destx<0||destx+width >SCREENWIDTH
	||	desty<0
	||	desty+height>SCREENHEIGHT 
	||	(unsigned)srcscrn>4
	||	(unsigned)destscrn>4)
	 {
	I_Error ("Bad V_CopyRect16");
	 }
#endif 
	 V_MarkRect16	(destx, desty,	width, height);
	 
	 src = screens[srcscrn]+(SCREENWIDTH*srcy+srcx)*2;
	 dest	= screens[destscrn]+(SCREENWIDTH*desty+destx)*2;

	 for ( ;	height>0	; height--)	
	 {	
	memcpy (dest, src, width*2);
	src += SCREENWIDTH*2;
	dest += SCREENWIDTH*2;
	 }	
} 
 

//
// V_DrawPatch16
// Masks a column based masked pic to the screen. 
//
void
V_DrawPatch16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	) 
{ 

	 int		count;
	 int		col; 
	 column_t*	column; 
	 short*	desttop;
	 short*	dest;
	 byte*	source; 
	 int		w;	
	 
	 y	-=	SHORT(patch->topoffset); 
	 x	-=	SHORT(patch->leftoffset); 
#ifdef RANGECHECK	
	 if (x<0
	||x+SHORT(patch->width)	>SCREENWIDTH
	||	y<0
	||	y+SHORT(patch->height)>SCREENHEIGHT	
	||	(unsigned)scrn>4)
	 {
		fprintf(	stderr, "Patch at %d,%d exceeds LFB\n", x,y );
		// No I_Error abort - what is up with TNT.WAD?
		fprintf(	stderr, "V_DrawPatch16: bad patch (ignored)\n");
		return;
	 }
#endif 
 
	 if (!scrn)
	V_MarkRect16 (x,	y,	SHORT(patch->width),	SHORT(patch->height));

	 col = 0; 
	 desttop	= (short *)(screens[scrn]+(y*SCREENWIDTH+x)*2);
	 
	 w	= SHORT(patch->width); 

	 for ( ;	col<w	; x++, col++, desttop++)
	 {	
	column =	(column_t *)((byte *)patch	+ LONG(patch->columnofs[col])); 
 
	// step through the posts in a column 
	while	(column->topdelta	!=	0xff ) 
	{ 
		 source = (byte *)column +	3;	
		 dest	= desttop +	column->topdelta*SCREENWIDTH;
		 count =	column->length; 
			 
		 while (count--) 
		 {	
		*dest	= palette_color[*source++];
		dest += SCREENWIDTH;
		 }	
		 column = (column_t *)(	 (byte *)column +	column->length	
					 +	4 ); 
	} 
	 }			 
} 
 
//
// V_DrawPatchFlipped16
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
void
V_DrawPatchFlipped16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	) 
{ 

	 int		count;
	 int		col; 
	 column_t*	column; 
	 short*	desttop;
	 short*	dest;
	 byte*	source; 
	 int		w;	
	 
	 y	-=	SHORT(patch->topoffset); 
	 x	-=	SHORT(patch->leftoffset); 
#ifdef RANGECHECK	
	 if (x<0
	||x+SHORT(patch->width)	>SCREENWIDTH
	||	y<0
	||	y+SHORT(patch->height)>SCREENHEIGHT	
	||	(unsigned)scrn>4)
	 {
		fprintf(	stderr, "Patch origin %d,%d exceeds LFB\n", x,y	);
		I_Error ("Bad V_DrawPatch16 in V_DrawPatchFlipped16");
	 }
#endif 
 
	 if (!scrn)
	V_MarkRect16 (x,	y,	SHORT(patch->width),	SHORT(patch->height));

	 col = 0; 
	 desttop	= (short *)(screens[scrn]+(y*SCREENWIDTH+x)*2);
	 
	 w	= SHORT(patch->width); 

	 for ( ;	col<w	; x++, col++, desttop++) 
	 {	
	column =	(column_t *)((byte *)patch	+ LONG(patch->columnofs[w-1-col]));	
 
	// step through the posts in a column 
	while	(column->topdelta	!=	0xff ) 
	{ 
		 source = (byte *)column +	3;	
		 dest	= desttop +	column->topdelta*SCREENWIDTH;
		 count =	column->length; 
			 
		 while (count--) 
		 {	
		*dest	= palette_color[*source++];
		dest += SCREENWIDTH;	
		 }	
		 column = (column_t *)(	 (byte *)column +	column->length	
					 +	4 ); 
	} 
	 }			 
} 
 


//
// V_DrawPatchDirect16
// Draws directly to the screen on the pc. 
//
void
V_DrawPatchDirect16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	) 
{
	 V_DrawPatch16 (x,y,scrn,	patch);
}
 
void	//moves x and y to center of screen  - in future, stretch bitmap?
V_DrawPatchInDirect16
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	)
  {
  V_DrawPatchDirect16(x+((SCREENWIDTH-320)/2),y+((SCREENHEIGHT-200)/2),scrn,patch);
  }


//
// V_DrawBlock16
// Draw a linear block of pixels into the view buffer.
//
void
V_DrawBlock16
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		src )	
{ 
	byte*	dest;
	 
#ifdef RANGECHECK	
	 if (x<0
	||x+width >SCREENWIDTH
	||	y<0
	||	y+height>SCREENHEIGHT 
	||	(unsigned)scrn>4 )
	 {
	I_Error ("Bad V_DrawBlock16");
	 }
#endif 
 
	 V_MarkRect16	(x, y, width, height);
 
	 dest	= screens[scrn] +	(y*SCREENWIDTH+x)*2;

	 while (height--)	
      {
      memcpy (dest, src, width*2);
      src += width*2;
      dest += SCREENWIDTH*2;
      }
} 
 


//
// V_GetBlock16
// Gets a linear block of pixels from the view buffer.
//
void
V_GetBlock16
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest ) 
{ 
	 byte*	src; 
	 
#ifdef RANGECHECK	
	 if (x<0
	||x+width >SCREENWIDTH
	||	y<0
	||	y+height>SCREENHEIGHT 
	||	(unsigned)scrn>4 )
	 {
	I_Error ("Bad V_DrawBlock16");
	 }
#endif 
 
	 src = screens[scrn]	+ (y*SCREENWIDTH+x)*2;

	 while (height--)	
	 {	
	memcpy (dest, src, width*2);
	src += SCREENWIDTH*2;
	dest += width*2;
	 }	
} 




//
// V_Init16
// 
void V_Init16	(void)
{ 
	 int		i;
	 byte*	base;
		
	 // stick these in low dos memory on PCs

	 base	= I_AllocLow (SCREENWIDTH*SCREENHEIGHT*4*2+255);
    base= (byte *)((((int)base)+255)&~0xff);  //alignment


	 for (i=0 ;	i<4 ;	i++)
	screens[i] = base	+ i*SCREENWIDTH*SCREENHEIGHT*2;
}

void V_ClearScreen16(int scrn)
  {
  char name[20];
  byte*	src;
  short*	dest;
  int	x,y;
  byte* tempsrc;

  if (gamemode==commercial)
	 strcpy(name,"GRNROCK");
  else
	 strcpy(name,"FLOOR7_2");

  src = W_CacheLumpName (name , PU_CACHE);
  dest	= (short *)(screens[scrn]);

  for (y=0 ;	y<SCREENHEIGHT	; y++)
    {
    tempsrc=src+((y&63)<<6);
    for (x=0;x<SCREENWIDTH;x++)
      {
      *dest=palette_color[tempsrc[x&63]];
      dest++;
      }
    }
  }


