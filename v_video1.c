//  
// DOSDoom Video Code for 8-Bit Colour. 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
#include <stdlib.h>

#include "dm_data.h"
#include "dm_defs.h"
#include "dm_state.h"

#include "d_main.h"
#include "i_system.h"
#include "m_bbox.h"
#include "m_swap.h"
#include "r_local.h"
#include "v_res.h"
#include "v_video1.h"
#include "w_wad.h"
#include "z_zone.h"

int dirtybox[4];

//
// V_MarkRect8
// 
void V_MarkRect8 (int x, int y, int width, int height)
{ 
  M_AddToBox (dirtybox, x, y);
  M_AddToBox (dirtybox, x+width-1, y+height-1);
} 

//
// V_CopyRect8
// 
void V_CopyRect8 (int srcx, int srcy, int srcscrn,
                   int width, int height, int destx, int desty, int destscrn)
{ 
  byte* src;
  byte* dest;
         
  V_MarkRect8 (destx, desty, width, height);
         
  src = screens[srcscrn]+SCREENWIDTH*srcy+srcx;
  dest = screens[destscrn]+SCREENWIDTH*desty+destx;

  while (height--)
  {
    memcpy (dest, src, width);
    src += SCREENWIDTH;
    dest += SCREENWIDTH;
  }

} 
 

//
// V_DrawPatch8
//
// Masks a column based masked pic to the screen. 
void V_DrawPatch8 (int x, int y, int scrn, patch_t* patch)
{ 

  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* source;
  int w;
         
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  if (!scrn)
    V_MarkRect8 (x,y,SHORT(patch->width),SHORT(patch->height));

  col = 0;
  desttop = screens[scrn]+y*SCREENWIDTH+x;
         
  for (w = SHORT(patch->width); col<w ; x++, col++, desttop++)
  {
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop + column->topdelta*SCREENWIDTH;
      count  = column->length;

      while (count--)
      {
        *dest = *source++;
        dest += SCREENWIDTH;
      }

      column = (column_t *)((byte *)column +    column->length + 4);
    }

  }


} 
 
//
// V_DrawPatchFlipped8
//
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//
void V_DrawPatchFlipped8(int x, int y, int scrn, patch_t* patch)
{ 
  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* source;
  int w;
         
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);
 
  if (!scrn)
    V_MarkRect8 (x, y, SHORT(patch->width), SHORT(patch->height));

  col = 0;
  desttop = screens[scrn]+y*SCREENWIDTH+x;

  for (w = SHORT(patch->width); col<w ; x++, col++, desttop++)
  {
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-col]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop + column->topdelta*SCREENWIDTH;
      count = column->length;
                         
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
//
void V_DrawPatchDirect8 (int x, int y, int scrn, patch_t* patch)
{
 V_DrawPatch8 (x, y, scrn, patch);
}

//
// V_DrawPatchInDirect8
//
// The co-ordinates for this procedure are always based upon a
// 320x200 screen and multiplies the size of the patch by the
// scaledwidth & scaledheight. The purpose of this is to produce
// a clean and undistorted patch opon the screen, The scaled screen
// size is based upon the nearest whole number ratio from the
// current screen size to 320x200.
//
void V_DrawPatchInDirect8 (int x, int y, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* source;
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
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = screens[scrn]+stretchy*SCREENWIDTH+stretchx;

  for (w=(patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch+LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
      count=(column->length*DY)>>16;
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

//
// V_DrawPatchInDirectFlipped8
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
void V_DrawPatchInDirectFlipped8 (int x, int y, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* source;
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
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = screens[scrn]+stretchy*SCREENWIDTH+stretchx;

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch + LONG(patch->columnofs[patch->width-1-(col>>16)]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
      source = (byte *)column + 3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
      count=(column->length*DY)>>16;
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


//
// V_DrawPatchShrink8
//
// Shrinks a patch to half its size; again this uses the
// scaled size. It amounts to a cheap hack for the thermometers
// on the text menus.
//
void V_DrawPatchShrink8 (int x, int y, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* source;
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
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY2)>>16);

  col = 0;
  desttop = screens[scrn]+stretchy*SCREENWIDTH+stretchx;

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch   + LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop+((column->topdelta*DY2)>>16)*SCREENWIDTH;
      count=(column->length*DY2)>>16;
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
//
// Draw a linear block of pixels into the view buffer.
//
void V_DrawBlock8 (int x, int y, int scrn, int width, int height, byte* src)
{ 
  byte* dest;

  V_MarkRect8 (x, y, width, height);
 
  dest = screens[scrn] + y*SCREENWIDTH+x;

  while (height--)
  {
    memcpy (dest, src, width);
    src += width;
    dest += SCREENWIDTH;
  }
} 
 
//
// V_DrawPatchTrans8
//
// This the same as V_DrawPatch8, but it uses the PALREMAP translation
// tables to alter the necessary colours. Reference by Index.
//
// -ACB- 1998/09/11 Use PALREMAP translation tables
//
void V_DrawPatchTrans8 (int x, int y, int index, int scrn, patch_t* patch)
{ 
  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* remaptable;
  byte* source;
  int w;

  remaptable = &translationtables[256*index];

  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  if (!scrn)
    V_MarkRect8 (x, y, SHORT(patch->width), SHORT(patch->height));

  col = 0;
  desttop = screens[scrn]+y*SCREENWIDTH+x;

  for (w = SHORT(patch->width); col<w; x++, col++, desttop++)
  {
    column=(column_t *)((byte *)patch+LONG(patch->columnofs[col]));

    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column + 3;
      dest = desttop+column->topdelta*SCREENWIDTH;
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
// V_DrawPatchInDirectTrans8
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
void V_DrawPatchInDirectTrans8 (int x, int y, int index, int scrn, patch_t* patch)
{
  int count;
  int col;
  column_t* column;
  byte* desttop;
  byte* dest;
  byte* remaptable;
  byte* source;
  int w;

  int stretchx,stretchy;
  int srccol;

  remaptable = &translationtables[256*index];

  // -ACB- 1998/06/14 Adjusts coordinates to scale.
  x += X_OFFSET;
  y += Y_OFFSET;

  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

  stretchx = (x*DX)>>16;
  stretchy = (y*DY)>>16;

  if (!scrn)
    V_MarkRect8 (stretchx,stretchy,((patch->width)*DX)>>16,((patch->height)*DY)>>16);

  col = 0;
  desttop = screens[scrn]+stretchy*SCREENWIDTH+stretchx;

  for (w = (patch->width)<<16; col<w; x++, col+=DXI, desttop++)
  {
    column=(column_t *)((byte *)patch + LONG(patch->columnofs[col>>16]));
 
    // step through the posts in a column
    while (column->topdelta != 0xff)
    {
      source = (byte *)column+3;
      dest=desttop+((column->topdelta*DY)>>16)*SCREENWIDTH;
      count=(column->length*DY)>>16;
      srccol=0;

      while (count--)
      {
        *dest = remaptable[source[srccol>>16]];
        dest += SCREENWIDTH;
        srccol += DYI;
      }

      column = (column_t *)((byte *)column+(column->length)+4);
    }
  }
}

//
// V_GetBlock8
//
// Gets a linear block of pixels from the view buffer.
//
void V_GetBlock8 (int x, int y, int scrn, int width, int height, byte* dest)
{ 
  byte* src;
          
  src = screens[scrn] + y*SCREENWIDTH+x;

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
// -KM- 1998/07/10 Prepare for dynamic screen sizing
// -ACB- 1998/07/19 Implemented KM's Bugfix
//
void V_Init8(void)
{
  int  i;
  byte* base = screens[1];

  // stick these in low dos memory on PCs
  base = realloc (base, SCREENWIDTH*SCREENHEIGHT*3);

  for (i=0 ; i<3 ; i++)
   screens[i + 1] = base + i*SCREENWIDTH*SCREENHEIGHT;
}

//
// V_DarkenScreen8
//
// Darkens the background screen in menus etc...
//
// -ACB- 1998/06/20 Implemented JC's alternate 'GLQuake' Darken Screen
// -KM- 1998/01/29 Use longs to darken screen 4 pixels at a time: speed increase.
//  This will always work as long as screen sizes are even, ie 320x200 works, but
//  317x201 possibly won't.
//
void V_DarkenScreen8(int scrn)
{
  unsigned long *pixel;
  int i;
  lighttable_t *m_colormap;

  newhupd=true; // Force the entire status bar to update
  pixel=(long *) screens[scrn];
  m_colormap=(colormaps + (10*256));

  for (i=0; i<SCREENHEIGHT*SCREENWIDTH/4; i++)
  {
    *pixel = m_colormap[*pixel & 0xFF]
           | (m_colormap[(*pixel >>  8) & 0xFF] << 8)
           | (m_colormap[(*pixel >> 16) & 0xFF] << 16)
           | (m_colormap[(*pixel >> 24) & 0xFF] << 24);
    pixel++;
  }

}

//
// V_TextureBackScreen8
//
// -KM- 1998/07/10 Reduce code redundancy
//
void V_TextureBackScreen8(char* flatname)
{
  int x, y;
  byte *src, *dest;

  // erase the entire screen to a tiled background
  src = W_CacheLumpName ( flatname , PU_CACHE);
  dest = screens[0];

  for (y=0 ; y<SCREENHEIGHT ; y++)
  {
    for (x=0 ; x<SCREENWIDTH/64 ; x++)
    {
      memcpy (dest, src+((y&63)<<6), 64);
      dest += 64;
    }

    if (SCREENWIDTH&63)
    {
       memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63);
       dest += (SCREENWIDTH&63);
    }
  }
}

