//  
// DOSDoom Video Code for 16-Bit Colour. 
//
// Based on the Doom Source Code released by id Software, (c) 1993-1996
// (see DOOMLIC.TXT).
//

#include <stdio.h>

#include "dm_data.h"
#include "dm_defs.h"
#include "dm_state.h"

#include "d_main.h"
#include "i_alleg.h"
#include "i_system.h"
#include "r_local.h"
#include "m_bbox.h"
#include "m_swap.h"
#include "v_res.h"
#include "v_video2.h"
#include "w_wad.h"
#include "z_zone.h"

int dirtybox[4];

//
// V_MarkRect16
// 
void V_MarkRect16 (int x, int y, int width, int height)
{ 
  M_AddToBox(dirtybox, x, y);
  M_AddToBox(dirtybox, x+width-1, y+height-1);
} 

//
// V_CopyRect16
// 
void V_CopyRect16 (int srcx, int srcy, int srcscrn,
                    int width, int height, int destx, int desty, int destscrn)
{ 
  byte* src;
  byte* dest;
	 
  V_MarkRect16(destx, desty, width, height);
	 
  src = screens[srcscrn]+(SCREENWIDTH*srcy+srcx)*2;
  dest = screens[destscrn]+(SCREENWIDTH*desty+destx)*2;

  while (height--)
  {
    memcpy (dest, src, width*2);
    src += SCREENWIDTH*2;
    dest += SCREENWIDTH*2;
  }
} 

//
// V_DrawPatch16
//
void V_DrawPatch16 (int x, int y, int scrn, patch_t* patch)
{ 
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte*	source;
  int w;
	 
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  if (!scrn)
    V_MarkRect16 (x, y, SHORT(patch->width), SHORT(patch->height));

  col = 0;
  desttop = (short *)(screens[scrn]+(y*SCREENWIDTH+x)*2);

  for (w = SHORT(patch->width); col<w; x++, col++, desttop++)
  {
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop + column->topdelta*SCREENWIDTH;
      count = column->length;
			 
      while (count--)
      {
        *dest = palette_color[*source++];
        dest += SCREENWIDTH;
      }

      column = (column_t *)((byte *)column+column->length+4);
    }
  }
} 
 
//
// V_DrawPatchFlipped16
//
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
void V_DrawPatchFlipped16 (int x, int y, int scrn, patch_t* patch)
{ 
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte* source;
  int w;
	 
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);
 
  if (!scrn)
    V_MarkRect16 (x, y, SHORT(patch->width), SHORT(patch->height));

  col = 0;
  desttop = (short *)(screens[scrn]+(y*SCREENWIDTH+x)*2);

  for (w=SHORT(patch->width); col<w; x++, col++, desttop++)
  {
    column = (column_t *)((byte *)patch+LONG(patch->columnofs[w-1-col]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop+column->topdelta*SCREENWIDTH;
      count = column->length;
			 
      while (count--)
      {
        *dest = palette_color[*source++];
        dest += SCREENWIDTH;
      }

      column = (column_t *)((byte *)column + column->length + 4);
    }
  }
} 

//
// V_DrawPatchDirect16
//
void V_DrawPatchDirect16 (int x, int y, int scrn, patch_t* patch)
{
 V_DrawPatch16 (x,y,scrn,	patch);
}

//
// V_DrawPatchInDirect16
//
// The co-ordinates for this procedure are always based upon a
// 320x200 screen and multiplies the size of the patch by the
// scaledwidth & scaledheight. The purpose of this is to produce
// a clean and undistorted patch opon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200.
//
void V_DrawPatchInDirect16 (int x, int y, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte*	source;
  int w;

  int stretchx,stretchy;
  int srccol;
	 
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  // -ACB- 1998/06/14 Adjusts coordinates to scale.
  x += X_OFFSET;
  y += Y_OFFSET;

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch + LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
      count=(column->length*DY)>>16;
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

//
// V_DrawPatchInDirectFlipped16
//
// The co-ordinates for this procedure are always based upon a
// 320x200 screen and multiplies the size of the patch by the
// scaledwidth & scaledheight. The purpose of this is to produce
// a clean and undistorted patch opon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200.
//
// This Procedure flips the patch horizontally.
//
void V_DrawPatchInDirectFlipped16 (int x, int y, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte*	source;
  int w;

  int stretchx,stretchy;
  int srccol;
	 
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  // -ACB- 1998/06/14 Adjusts coordinates to scale.
  x += X_OFFSET;
  y += Y_OFFSET;

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t*)((byte*)patch+LONG(patch->columnofs[patch->width-1-(col>>16)]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column +	3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
      count=(column->length*DY)>>16;
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

//
// V_DrawPatchShrink16
//
// Shrinks a patch to half its size; again this uses the
// scaled size. It amounts to a cheap hack for the thermometers
// on the text menus.
//
void V_DrawPatchShrink16 (int x, int y, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte*	source;
  int w;

  int stretchx,stretchy;
  int srccol;
	 
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  // -ACB- 1998/06/14 Adjusts coordinates to scale.
  x += X_OFFSET;
  y += Y_OFFSET;

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY2)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch + LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column+3;
      dest=desttop+((column->topdelta*DY2)>>16)*SCREENWIDTH;
      count=(column->length*DY2)>>16;
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
//
// This the same as V_DrawPatch16, but it uses the PALREMAP translation
// tables to alter the necessary colours. Reference by Index.
//
// -ACB- 1998/09/11 Use PALREMAP translation tables
//
void V_DrawPatchTrans16 (int x, int y, int index, int scrn, patch_t* patch)
{ 
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte* remaptable;
  byte* source;
  int w;

  remaptable = &translationtables[256*index];

  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);
 
  if (!scrn)
    V_MarkRect16 (x, y, SHORT(patch->width), SHORT(patch->height));

  col = 0;
  desttop = (short *)(screens[scrn]+(y*SCREENWIDTH+x)*2);

  for (w = SHORT(patch->width); col<w; x++, col++, desttop++)
  {
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop + column->topdelta*SCREENWIDTH;
      count = column->length;
			 
      while (count--)
      {
        *dest = remaptable[*source++];
	dest += SCREENWIDTH;
      }

      column = (column_t *)((byte *)column+column->length+4);
    }
  }
} 

//
// V_DrawPatchInDirectTrans16
//
// The co-ordinates for this procedure are always based upon a
// 320x200 screen and multiplies the size of the patch by the
// scaledwidth & scaledheight. The purpose of this is to produce
// a clean and undistorted patch opon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200. The procedure uses the PALREMAP
// translation tables to alter the necessary colours, the table
// is selected by index.
//
// -ACB- 1998/09/11 Use PALREMAP translation tables
//
void V_DrawPatchInDirectTrans16 (int x, int y, int index, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  short* desttop;
  short* dest;
  byte*	source;
  byte* remaptable;
  int w;

  int stretchx,stretchy;
  int srccol;

  remaptable = &translationtables[256*index];
	 
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  // -ACB- 1998/06/14 Adjusts coordinates to scale.
  x += X_OFFSET;
  y += Y_OFFSET;

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect16 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = (short *)(screens[scrn]+(stretchy*SCREENWIDTH+stretchx)*2);

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch + LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
      count=(column->length*DY)>>16;
      srccol=0;

      while (count--)
      {
        *dest=palette_color[remaptable[source[srccol>>16]]];
	dest += SCREENWIDTH;
        srccol+=DYI;
      }

      column = (column_t *)((byte *)column+(column->length)+4);
    }
  }
}

//
// V_DrawBlock16
//
// Draw a linear block of pixels into the view buffer.
//
void V_DrawBlock16 (int x, int y, int scrn, int width, int height, byte* src)
{ 
  byte* dest;
	 
  V_MarkRect16 (x, y, width, height);
 
  dest = screens[scrn] + (y*SCREENWIDTH+x)*2;

  while (height--)
  {
    memcpy (dest, src, width*2);
    src += width*2;
    dest += SCREENWIDTH*2;
  }
} 

//
// V_GetBlock16
//
// Gets a linear block of pixels from the view buffer.
//
void V_GetBlock16 (int x, int y, int scrn, int width, int height, byte* dest)
{ 
  byte* src;
	  
  src = screens[scrn] + (y*SCREENWIDTH+x)*2;

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
// -KM- 1998/07/10 Prepare for dynamic screen sizing
// -ACB- 1998/07/19 Implemented KM's Bugfix
//
void V_Init16 (void)
{ 
  int  i;
  byte* base = screens[1];

  // stick these in low dos memory on PCs
  // screens[0] is set by allegro in i_allegv.c
  base = realloc (base, SCREENWIDTH*SCREENHEIGHT*3*2+255);

  for (i=0; i<3; i++)
    screens[i + 1] = base + i*SCREENWIDTH*SCREENHEIGHT*2;
}

//
// V_DarkenScreen16
//
// Darkens the background screen in menus etc...
//
// -ES- 1998/08/17 Darkens screen to 22/32 (about 70%) of RGB values
//
void V_DarkenScreen16(int scrn)
{
  short *lineptr;
  int i,j,c;

  redrawsbar=true;
  lineptr=(short *)screens[scrn];
  for (i=0;i<SCREENHEIGHT;i++)
  {
    for (j=0;j<SCREENWIDTH;j++)
    {
      c = lineptr[j];
      lineptr[j] =
       ((((((c>>_rgb_r_shift_16)&0x1F)*11)>>4) << _rgb_r_shift_16) |
        (((((c>>_rgb_g_shift_16)&0x3F)*11)>>4) << _rgb_g_shift_16) |
        (((((c>>_rgb_b_shift_16)&0x1F)*11)>>4) << _rgb_b_shift_16));
    }
    lineptr+=SCREENWIDTH;
  }
}

//
// -KM- 1998-07-10 Reduce code redundancy
//
void V_TextureBackScreen16(char *flatname)
{
  int x, y;
  byte *src;
  short *dest;
  src = W_CacheLumpName(flatname, PU_CACHE);
  dest = (short *)(screens[0]);

  for (y=0; y<SCREENHEIGHT; y++)
  {
    for (x=0;x<SCREENWIDTH;x++)
    {
      *dest=palette_color[src[((y&63)<<6) + (x&63)]];
      dest++;
    }
  }
}

