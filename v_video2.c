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
// $Log:$
//
// DESCRIPTION:
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

//this is for 16bpp modes

static const char
rcsid[] = "$Id: v_video2.c,v 1.5 1997/02/03 22:45:13 b1 Exp $";


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
#include        "d_main.h"

#include        "v_res.h"

extern struct { int ploc; int numc;} pdecode[MAXTRANSLATIONS]; //-JC-

int				dirtybox[4];

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
 
void	//stretches bitrmap to fill screen
V_DrawPatchInDirect16
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

  int stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=DXI,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
	   collen=count=(column->length*DY)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=palette_color[source[srccol>>16]];
		  dest += SCREENWIDTH;
        srccol+=DYI;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
  }

void	//stretches bitrmap to fill screen
V_DrawPatchInDirectFlipped16
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

  int stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=DXI,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[patch->width-1-(col>>16)]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
	   collen=count=(column->length*DY)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=palette_color[source[srccol>>16]];
		  dest += SCREENWIDTH;
        srccol+=DYI;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
  }

void	//stretches bitrmap to fill screen
V_DrawPatchShrink16
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

  int stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY2)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=DXI,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*DY2)>>16)*SCREENWIDTH;
	   collen=count=(column->length*DY2)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=palette_color[source[srccol>>16]];
		  dest += SCREENWIDTH;
        srccol+=DYI2;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
  }

//
// V_DrawPatchTrans16
// Masks a column based masked pic to the screen. 
//
void
V_DrawPatchTrans16
( int		x,
  int		y,
  int           index,
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
		*dest	= palette_color[pdecode[index].ploc+(*source++ & pdecode[index].numc)];
		dest += SCREENWIDTH;
		 }	
		 column = (column_t *)(	 (byte *)column +	column->length	
					 +	4 ); 
	} 
	 }			 
} 


void	//stretches bitrmap to fill screen
V_DrawPatchInDirectTrans16
( int		x,
  int		y,
  int           index,
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

  int stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=DXI,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
	   collen=count=(column->length*DY)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=palette_color[pdecode[index].ploc+(source[srccol>>16] & pdecode[index].numc)];
		  dest += SCREENWIDTH;
        	  srccol+=DYI;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
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

void V_DarkenScreen16(int scrn)
  {
  short *lineptr;
  int i,j;

  redrawsbar=true;
  lineptr=(short *)screens[scrn];
  for (i=0;i<SCREENHEIGHT;i+=2)
    {
    for (j=0;j<SCREENWIDTH;j+=4)
      {
      lineptr[j+1]=lineptr[j+2]=lineptr[j+3]=0;
      }
    lineptr+=SCREENWIDTH;
    for (j=0;j<SCREENWIDTH;j+=4)
      {
      lineptr[j+0]=lineptr[j+1]=lineptr[j+3]=0;
      }
    lineptr+=SCREENWIDTH;
    }
  }

