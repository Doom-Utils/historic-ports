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
#include        "d_main.h"

#include        "v_res.h"

extern struct { int ploc; int numc;} pdecode[MAXTRANSLATIONS]; //-JC-

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

	 if (!scrn)
	 V_MarkRect8 (x,y,SHORT(patch->width),SHORT(patch->height));

	 col = 0; 
	 desttop = screens[scrn]+y*SCREENWIDTH+x;
	 
	 w	= SHORT(patch->width); 

	 for ( ; col<w ; x++, col++, desttop++)
	 {	
	column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
 
	// step through the posts in a column 
	while	(column->topdelta	!=	0xff ) 
	{ 
		 source = (byte *)column + 3;
		 dest	= desttop + column->topdelta*SCREENWIDTH;
		 count  = column->length;
			 
		 while (count--) 
		 {	
		  *dest	= *source++;
		  dest += SCREENWIDTH;
		 }	
		 column = (column_t *)((byte *)column +	column->length + 4);
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
 
	 if (!scrn)
	   V_MarkRect8 (x, y, SHORT(patch->width), SHORT(patch->height));

	 col = 0; 
	 desttop = screens[scrn]+y*SCREENWIDTH+x;
	 
	 w = SHORT(patch->width);

	 for ( ; col<w ; x++, col++, desttop++)
	 {	
	column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-col]));
 
	// step through the posts in a column 
	while	(column->topdelta	!=	0xff ) 
	{ 
		 source = (byte *)column +	3;	
		 dest	= desttop +	column->topdelta*SCREENWIDTH;	
		 count =	column->length; 
			 
		 while (count--) 
		 {	
		   *dest = *source++;
		   dest += SCREENWIDTH;
		 }	
		 column = (column_t *)( (byte *)column + column->length + 4 );
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

  int stretchx,stretchy;
  int srccol,collen;

  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop	= screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
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
	    *dest=source[srccol>>16];
	    dest += SCREENWIDTH;
            srccol+=DYI;
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

  int stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=DXI,desttop++)
    {
    column=(column_t *)((byte *)patch + LONG(patch->columnofs[patch->width-1-(col>>16)]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff )
      {
       source = (byte *)column +	3;
       dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
       collen=count=(column->length*DY)>>16;
       srccol=0;
      while (count--)
        {
	 *dest=source[srccol>>16];
	 dest += SCREENWIDTH;
         srccol+=DYI;
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

  int stretchx,stretchy;
  int srccol,collen;

  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;
  
  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY2)>>16);

  col = 0;
  desttop	= screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
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
		*dest=source[srccol>>16];
		dest += SCREENWIDTH;
		srccol+=DYI2;
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
// V_DrawPatch8
// Masks a column based masked pic to the screen. 
//

void
V_DrawPatchTrans8
( int		x,
  int		y,
  int           index,
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

	 if (!scrn)
	   V_MarkRect8 (x,	y,	SHORT(patch->width),	SHORT(patch->height));

	 col = 0; 
	 desttop	= screens[scrn]+y*SCREENWIDTH+x;	
	 
	 w	= SHORT(patch->width); 

         for ( ;col<w; x++, col++, desttop++)
         {	
	    column =(column_t *)((byte *)patch+ LONG(patch->columnofs[col])); 
 
	   // step through the posts in a column 
	   while (column->topdelta!=0xff ) 
	   { 
	      source = (byte *)column +	3;	
	      dest= desttop+column->topdelta*SCREENWIDTH;	
	      count = column->length; 
			 
              while (count--) 
              {	
	         *dest = pdecode[index].ploc+(*source++ & pdecode[index].numc);
		 dest += SCREENWIDTH;	
              }	
	      column = (column_t *)( (byte *)column +column->length+4 ); 
	   } 
	 }
} 


void	//stretches bitmap to fill screen
V_DrawPatchInDirectTrans8
( int		x,
  int		y,
  int           index,
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

  int stretchx,stretchy;
  int srccol,collen;
	 
  y	-=	SHORT(patch->topoffset);
  x	-=	SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop	= screens[scrn]+stretchy*SCREENWIDTH+stretchx;
	 
  w = (patch->width)<<16;

  for (;col<w;x++,col+=DXI,desttop++)
    {
    column=(column_t *)((byte *)patch	+ LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
      {
	source = (byte *)column +	3;
	dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
	collen=count=(column->length*DY)>>16;
	srccol=0;
	while (count--)
          {
	   *dest=pdecode[index].ploc+(source[srccol>>16] & pdecode[index].numc);
	   dest += SCREENWIDTH;
	   srccol+=DYI;
	  }
        column = (column_t *)((byte *)column+(column->length)+4);
      }
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

	 base = I_AllocLow (SCREENWIDTH*SCREENHEIGHT*4+255);
         base = (byte *)((((int)base)+255)&~0xff);  //alignment

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


