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

//this is for 8bpp modes

static const char
rcsid[] = "$Id: v_video1.c,v 1.5 1997/02/03 22:45:13 b1 Exp $";


#include	"i_system.h"
#include	"r_local.h"

#include	"doomdef.h"
#include	"doomdata.h"
#include	"doomstat.h"

#include	"m_bbox.h"
#include	"m_swap.h"

#include	"v_video1.h"
#include	"w_wad.h"
#include	"z_zone.h"
#include "d_main.h"

#include "multires.h"

int				dirtybox[4];

//
// V_MarkRect8
// 
void
V_MarkRect8
( int		x,
  int		y,
  int		width,
  int		height )	
{ 
	 M_AddToBox	(dirtybox, x, y);	
	 M_AddToBox	(dirtybox, x+width-1, y+height-1); 
} 
 

//
// V_CopyRect8
// 
void
V_CopyRect8
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
	I_Error ("Bad V_CopyRect8");
	 }
#endif 
	 V_MarkRect8	(destx, desty,	width, height);
	 
	 src = screens[srcscrn]+SCREENWIDTH*srcy+srcx; 
	 dest	= screens[destscrn]+SCREENWIDTH*desty+destx;	

	 for ( ;	height>0	; height--)	
	 {	
	memcpy (dest, src, width);	
	src += SCREENWIDTH; 
	dest += SCREENWIDTH;	
	 }	
} 
 

//
// V_DrawPatch8
// Masks a column based masked pic to the screen. 
//
void
V_DrawPatch8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	) 
{ 

	 int		count;
	 int		col; 
	 column_t*	column; 
	 byte*	desttop;
	 byte*	dest;
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
		fprintf(	stderr, "V_DrawPatch8: bad patch (ignored)\n");
		return;
	 }
#endif 
 
	 if (!scrn)
	V_MarkRect8 (x,	y,	SHORT(patch->width),	SHORT(patch->height));

	 col = 0; 
	 desttop	= screens[scrn]+y*SCREENWIDTH+x;	
	 
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
		*dest	= *source++; 
		dest += SCREENWIDTH;	
		 }	
		 column = (column_t *)(	 (byte *)column +	column->length	
					 +	4 ); 
	} 
	 }			 
} 
 
//
// V_DrawPatchFlipped8
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
void
V_DrawPatchFlipped8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	) 
{ 

	 int		count;
	 int		col; 
	 column_t*	column; 
	 byte*	desttop;
	 byte*	dest;
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
		I_Error ("Bad V_DrawPatch8 in V_DrawPatchFlipped8");
	 }
#endif 
 
	 if (!scrn)
	V_MarkRect8 (x,	y,	SHORT(patch->width),	SHORT(patch->height));

	 col = 0; 
	 desttop	= screens[scrn]+y*SCREENWIDTH+x;	
	 
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
		*dest	= *source++; 
		dest += SCREENWIDTH;	
		 }	
		 column = (column_t *)(	 (byte *)column +	column->length	
					 +	4 ); 
	} 
	 }			 
} 
 


//
// V_DrawPatchDirect8
// Draws directly to the screen on the pc. 
//
void
V_DrawPatchDirect8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	) 
{
	 V_DrawPatch8 (x,y,scrn,	patch);
}
 
void	//stretches bitmap to fill screen
V_DrawPatchInDirect8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	)
  {
  int		count;
  int		col;
  column_t*	column;
  byte*	desttop;
  byte*	dest;
  byte*	source;
  int		w;

  int deltax,deltay,deltaxi,deltayi,stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);
#ifdef RANGECHECK	
	 if (x<0
	||x+SHORT(patch->width)	>320
	||	y<0
	||	y+SHORT(patch->height)>200
	||	(unsigned)scrn>4)
	 {
		fprintf(	stderr, "Patch at %d,%d exceeds LFB\n", x,y );
		// No I_Error abort - what is up with TNT.WAD?
		fprintf(	stderr, "V_DrawPatch8: bad patch (ignored)\n");
		return;
	 }
#endif 

  deltax=(SCREENWIDTH<<16)/320;
  deltaxi=(320<<16)/SCREENWIDTH;
  deltay=(SCREENHEIGHT<<16)/200;
  deltayi=(200<<16)/SCREENHEIGHT;
  stretchx=(x*deltax)>>16;
  stretchy=(y*deltay)>>16;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*deltax)>>16,((patch->height)*deltax)>>16);

  col = 0;
  desttop	= screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=deltaxi,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*deltay)>>16)*SCREENWIDTH;
	   collen=count=(column->length*deltay)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=source[srccol>>16];
		  dest += SCREENWIDTH;
        srccol+=deltayi;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
  }

void	//stretches bitmap to fill screen
V_DrawPatchInDirectFlipped8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	)
  {
  int		count;
  int		col;
  column_t*	column;
  byte*	desttop;
  byte*	dest;
  byte*	source;
  int		w;

  int deltax,deltay,deltaxi,deltayi,stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);
#ifdef RANGECHECK	
	 if (x<0
	||x+SHORT(patch->width)	>320
	||	y<0
	||	y+SHORT(patch->height)>200
	||	(unsigned)scrn>4)
	 {
		fprintf(	stderr, "Patch at %d,%d exceeds LFB\n", x,y );
		// No I_Error abort - what is up with TNT.WAD?
		fprintf(	stderr, "V_DrawPatch8: bad patch (ignored)\n");
		return;
	 }
#endif 

  deltax=(SCREENWIDTH<<16)/320;
  deltaxi=(320<<16)/SCREENWIDTH;
  deltay=(SCREENHEIGHT<<16)/200;
  deltayi=(200<<16)/SCREENHEIGHT;
  stretchx=(x*deltax)>>16;
  stretchy=(y*deltay)>>16;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*deltax)>>16,((patch->height)*deltax)>>16);

  col = 0;
  desttop	= screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=deltaxi,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[patch->width-1-(col>>16)]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*deltay)>>16)*SCREENWIDTH;
	   collen=count=(column->length*deltay)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=source[srccol>>16];
		  dest += SCREENWIDTH;
        srccol+=deltayi;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
  }


void	//stretches bitmap to fill screen
V_DrawPatchShrink8
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	)
  {
  int		count;
  int		col;
  column_t*	column;
  byte*	desttop;
  byte*	dest;
  byte*	source;
  int		w;

  int deltax,deltay,deltaxi,deltayi,stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);
#ifdef RANGECHECK	
	 if (x<0
	||x+SHORT(patch->width)	>320
	||	y<0
	||	y+SHORT(patch->height)>200
	||	(unsigned)scrn>4)
	 {
		fprintf(	stderr, "Patch at %d,%d exceeds LFB\n", x,y );
		// No I_Error abort - what is up with TNT.WAD?
		fprintf(	stderr, "V_DrawPatch8: bad patch (ignored)\n");
		return;
	 }
#endif 

  deltax=(SCREENWIDTH<<16)/320;
  deltaxi=(320<<16)/SCREENWIDTH;
  deltay=(SCREENHEIGHT<<16)/200;
  deltayi=(200<<16)/SCREENHEIGHT;
  stretchx=(x*deltax)>>16;
  stretchy=(y*deltay)>>16;
  deltay/=2; deltayi*=2;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*deltax)>>16,((patch->height)*deltax)>>16);

  col = 0;
  desttop	= screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=deltaxi,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while	(column->topdelta	!=	0xff )
      {
  	   source = (byte *)column +	3;
      dest=desttop+((column->topdelta*deltay)>>16)*SCREENWIDTH;
	   collen=count=(column->length*deltay)>>16;
      srccol=0;
	   while (count--)
        {
		  *dest=source[srccol>>16];
		  dest += SCREENWIDTH;
        srccol+=deltayi;
		  }
	   column = (column_t *)((byte *)column+(column->length)+4);
	   }
    }
  }


//
// V_DrawBlock8
// Draw a linear block of pixels into the view buffer.
//
void
V_DrawBlock8
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
	I_Error ("Bad V_DrawBlock8");
	 }
#endif 
 
	 V_MarkRect8	(x, y, width, height);
 
	 dest	= screens[scrn] +	y*SCREENWIDTH+x; 

	 while (height--)	
	 {	
	memcpy (dest, src, width);	
	src += width; 
	dest += SCREENWIDTH;	
	 }	
} 
 


//
// V_GetBlock8
// Gets a linear block of pixels from the view buffer.
//
void
V_GetBlock8
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
	I_Error ("Bad V_DrawBlock8");
	 }
#endif 
 
	 src = screens[scrn]	+ y*SCREENWIDTH+x; 

	 while (height--)	
	 {	
	memcpy (dest, src, width);	
	src += SCREENWIDTH; 
	dest += width;	
	 }	
} 




//
// V_Init8
// 
void V_Init8	(void)
{ 
	 int		i;
	 byte*	base;
		
	 // stick these in low dos memory on PCs

	 base	= I_AllocLow (SCREENWIDTH*SCREENHEIGHT*4+255);
    base= (byte *)((((int)base)+255)&~0xff);  //alignment

	 for (i=0 ;	i<4 ;	i++)
      screens[i] = base	+ i*SCREENWIDTH*SCREENHEIGHT;
}



void V_DarkenScreen8(int scrn)
  {
  char *lineptr;
  int i,j;

  redrawsbar=true;
  lineptr=screens[scrn];
  for (i=0;i<SCREENHEIGHT;i+=2)
    {
    for (j=0;j<SCREENWIDTH;j+=4)
      {
      lineptr[j+1]=0;
      lineptr[j+2]=0;
      lineptr[j+3]=0;
      }
    lineptr+=SCREENWIDTH;
    for (j=0;j<SCREENWIDTH;j+=4)
      {
      lineptr[j+0]=0;
      lineptr[j+1]=0;
      lineptr[j+3]=0;
      }
    lineptr+=SCREENWIDTH;
    }
  }


