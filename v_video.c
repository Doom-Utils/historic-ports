
// V_video.c

#include "DoomDef.h"

#define SC_INDEX 0x3c4

int dirtybox[4];

void
V_MarkRect
( int		x,
  int		y,
  int		width,
  int		height )	
{ 
	 M_AddToBox	(dirtybox, x, y);	
	 M_AddToBox	(dirtybox, x+width-1, y+height-1); 
} 


//---------------------------------------------------------------------------
//
// PROC V_DrawPatch
//
// Draws a column based masked pic to the screen.
//
//---------------------------------------------------------------------------

void V_DrawPatch(int x, int y, patch_t *patch)
{
	 int		count;
	 int		col; 
         int            scrn =0; // Should really be included within the main
                                 // function. Why?
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
	V_MarkRect (x,	y,	SHORT(patch->width),	SHORT(patch->height));

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



void V_DrawRealPatch(int x, int y, int scrn, patch_t *patch)
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
	V_MarkRect (x,	y,	SHORT(patch->width),	SHORT(patch->height));

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




////////////////////////////
// V_DrawPatchInDirect - Right onto the view buffer, streched out to fit
//                       screewidth and screenheight
////////////////////////////

void V_DrawPatchInDirect (int x, int y, int scrn, patch_t* patch)
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
    V_MarkRect (stretchx,stretchy,((patch->width)*deltax)>>16,((patch->height)*deltax)>>16);

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





/*
==================
=
= V_DrawFuzzPatch
=
= Masks a column based translucent masked pic to the screen.
=
==================
*/
extern byte *tinttable;

void V_DrawFuzzPatch (int x, int y, patch_t *patch)
{
	 int		count;
	 int		col; 
         int            scrn =0; // Should really be included within the main
                                 // function. Why?

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
	V_MarkRect (x,	y,	SHORT(patch->width),	SHORT(patch->height));

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

/*
==================
=
= V_DrawShadowedPatch
=
= Masks a column based masked pic to the screen.
=
==================
*/

void V_DrawShadowedPatch(int x, int y, patch_t *patch)
{
	 int		count;
	 int		col; 
         int            scrn =0; // Should really be included within the main
                                 // function. Why?
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
	V_MarkRect (x,	y,	SHORT(patch->width),	SHORT(patch->height));

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

//---------------------------------------------------------------------------
//
// PROC V_DrawRawScreen
//
//---------------------------------------------------------------------------

void V_DrawRawScreen(patch_t *patch)
{
V_DrawPatch(0, 0, patch);
}


void V_ClearScreen(int scrn)
{  
  char name[20];
  byte*	src;
  byte*	dest;
  int	x,y;

         strcpy(name,"FLOOR18");

	 src = W_CacheLumpName (name , PU_CACHE);
	 dest	= screens[scrn];

	 for (y=0 ;	y<SCREENHEIGHT	; y++)
	 {
	for (x=0	; x<SCREENWIDTH/64 ;	x++)
	{
		 memcpy (dest,	src+((y&63)<<6), 64);
		 dest	+=	64;
	}
	if	(SCREENWIDTH&63)
	{
		 memcpy (dest,	src+((y&63)<<6), SCREENWIDTH&63);
		 dest	+=	(SCREENWIDTH&63);
	}
         }
}





//---------------------------------------------------------------------------
//
// PROC V_Init
//
//---------------------------------------------------------------------------

void V_Init(void)
{
	 int		i;
	 byte*	base;
		
	 // stick these in low dos memory on PCs

	 base	= I_AllocLow (SCREENWIDTH*SCREENHEIGHT*4+255);
    base= (byte *)((((int)base)+255)&~0xff);  //alignment

	 for (i=0 ;	i<4 ;	i++)
      screens[i] = base	+ i*SCREENWIDTH*SCREENHEIGHT;
}
