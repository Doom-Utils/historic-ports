// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw.c,v 1.16 1998/05/03 22:41:46 killough Exp $
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
//
// DESCRIPTION:
//      The actual span/column drawing functions.
//      Here find the main potential for optimization,
//       e.g. inline assembly, different algorithms.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: r_draw.c,v 1.16 1998/05/03 22:41:46 killough Exp $";

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "v_video.h"

#define MAXWIDTH  MAX_SCREENWIDTH          /* kilough 2/8/98 */
#define MAXHEIGHT MAX_SCREENHEIGHT

#define SBARHEIGHT 32             /* status bar height at bottom of screen */

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//

byte *viewimage; 
int  viewwidth;
int  scaledviewwidth;
int  viewheight;
int  viewwindowx;
int  viewwindowy; 
byte *ylookup[MAXHEIGHT]; 
int  columnofs[MAXWIDTH]; 

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//

byte translations[3][256];
 
byte *tranmap;          // translucency filter maps 256x256   // phares 
byte *main_tranmap;     // killough 4/11/98

//
// R_DrawColumn
// Source is the top of the column to scale.
//

lighttable_t *dc_colormap; 
int     dc_x; 
int     dc_yl; 
int     dc_yh; 
fixed_t dc_iscale; 
fixed_t dc_texturemid;
int     dc_texheight;    // killough
byte    *dc_source;      // first pixel in a column (possibly virtual) 

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
// 

#ifndef DJGPP     // killough 2/15/98
#ifndef LINUX // Added CPhipps

void R_DrawColumn (void) 
{ 
  int              count; 
  register byte    *dest;            // killough
  register fixed_t frac;            // killough
  fixed_t          fracstep;     

  count = dc_yh - dc_yl + 1; 

  if (count <= 0)    // Zero length, column does not exceed a pixel.
    return; 
                                 
#ifdef RANGECHECK 
  if ((unsigned)dc_x >= SCREENWIDTH
      || dc_yl < 0
      || dc_yh >= SCREENHEIGHT) 
    I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  // Use columnofs LUT for subwindows? 

  dest = ylookup[dc_yl] + columnofs[dc_x];  

  // Determine scaling, which is the only mapping to be done.

  fracstep = dc_iscale; 
  frac = dc_texturemid + (dc_yl-centery)*fracstep; 

  // Inner loop that does the actual texture mapping,
  //  e.g. a DDA-lile scaling.
  // This is as fast as it gets.       (Yeah, right!!! -- killough)
  //
  // killough 2/1/98: more performance tuning

  {
    register const byte *source = dc_source;            
    register const lighttable_t *colormap = dc_colormap; 
    register heightmask = dc_texheight-1;
    if (dc_texheight & heightmask)   // not a power of 2 -- killough
      {
        heightmask++;
        heightmask <<= FRACBITS;
          
        if (frac < 0)
          while ((frac += heightmask) <  0);
        else
          while (frac >= heightmask)
            frac -= heightmask;
          
        do
          {
            // Re-map color indices from wall texture column
            //  using a lighting/special effects LUT.
            
            // heightmask is the Tutti-Frutti fix -- killough
            
            *dest = colormap[source[frac>>FRACBITS]];
            dest += SCREENWIDTH; 
            if ((frac += fracstep) >= heightmask)
              frac -= heightmask;
          } 
        while (--count);
      }
    else
      {
        while ((count-=2)>=0)   // texture height is a power of 2 -- killough
          {
            *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
            dest += SCREENWIDTH; 
            frac += fracstep;
            *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
            dest += SCREENWIDTH; 
            frac += fracstep;
          }
        if (count & 1)
          *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
      }
  }
} 
#endif
#endif

// Here is the version of R_DrawColumn that deals with translucent  // phares
// textures and sprites. It's identical to R_DrawColumn except      //    |
// for the spot where the color index is stuffed into *dest. At     //    V
// that point, the existing color index and the new color index
// are mapped through the TRANMAP lump filters to get a new color
// index whose RGB values are the average of the existing and new
// colors.
//
// Since we're concerned about performance, the 'translucent or
// opaque' decision is made outside this routine, not down where the
// actual code differences are.

#ifndef DJGPP                       // killough 2/21/98: converted to x86 asm
#ifndef LINUX // CPhipps added
void R_DrawTLColumn (void)                                           
{ 
  int              count; 
  register byte    *dest;           // killough
  register fixed_t frac;            // killough
  fixed_t          fracstep;

  count = dc_yh - dc_yl + 1; 

  // Zero length, column does not exceed a pixel.
  if (count <= 0)
    return; 
                                 
#ifdef RANGECHECK 
  if ((unsigned)dc_x >= SCREENWIDTH
      || dc_yl < 0
      || dc_yh >= SCREENHEIGHT) 
    I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  // Use columnofs LUT for subwindows? 

  dest = ylookup[dc_yl] + columnofs[dc_x];  
  
  // Determine scaling,
  //  which is the only mapping to be done.

  fracstep = dc_iscale; 
  frac = dc_texturemid + (dc_yl-centery)*fracstep; 

  // Inner loop that does the actual texture mapping,
  //  e.g. a DDA-lile scaling.
  // This is as fast as it gets.       (Yeah, right!!! -- killough)
  //
  // killough 2/1/98, 2/21/98: more performance tuning
  
  {
    register const byte *source = dc_source;            
    register const lighttable_t *colormap = dc_colormap; 
    register heightmask = dc_texheight-1;
    if (dc_texheight & heightmask)   // not a power of 2 -- killough
      {
        heightmask++;
        heightmask <<= FRACBITS;
          
        if (frac < 0)
          while ((frac += heightmask) <  0);
        else
          while (frac >= heightmask)
            frac -= heightmask;
        
        do
          {
            // Re-map color indices from wall texture column
            //  using a lighting/special effects LUT.
            
            // heightmask is the Tutti-Frutti fix -- killough
              
            *dest = tranmap[(*dest<<8)+colormap[source[frac>>FRACBITS]]]; // phares
            dest += SCREENWIDTH; 
            if ((frac += fracstep) >= heightmask)
              frac -= heightmask;
          } 
        while (--count);
      }
    else
      {
        while ((count-=2)>=0)   // texture height is a power of 2 -- killough
          {
            *dest = tranmap[(*dest<<8)+colormap[source[(frac>>FRACBITS) & heightmask]]]; // phares
            dest += SCREENWIDTH; 
            frac += fracstep;
            *dest = tranmap[(*dest<<8)+colormap[source[(frac>>FRACBITS) & heightmask]]]; // phares
            dest += SCREENWIDTH; 
            frac += fracstep;
          }
        if (count & 1)
          *dest = tranmap[(*dest<<8)+colormap[source[(frac>>FRACBITS) & heightmask]]]; // phares
      }
  }
} 
#endif
#endif  // killough 2/21/98: converted to x86 asm

//
// Spectre/Invisibility.
//

#define FUZZTABLE 50 
#define FUZZOFF (SCREENWIDTH)

static const int fuzzoffset[FUZZTABLE] = {
  FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
  FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
  FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
  FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF 
}; 

static int fuzzpos = 0; 

//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//

void R_DrawFuzzColumn(void) 
{ 
  int      count; 
  byte     *dest; 
  fixed_t  frac;
  fixed_t  fracstep;     

  // Adjust borders. Low... 
  if (!dc_yl) 
    dc_yl = 1;

  // .. and high.
  if (dc_yh == viewheight-1) 
    dc_yh = viewheight - 2; 
                 
  count = dc_yh - dc_yl; 

  // Zero length.
  if (count < 0) 
    return; 
    
#ifdef RANGECHECK 
  if ((unsigned) dc_x >= SCREENWIDTH
      || dc_yl < 0 
      || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawFuzzColumn: %i to %i at %i",
             dc_yl, dc_yh, dc_x);
#endif

  // Keep till detailshift bug in blocky mode fixed,
  //  or blocky mode removed.

  // Does not work with blocky mode.
  dest = ylookup[dc_yl] + columnofs[dc_x];
  
  // Looks familiar.
  fracstep = dc_iscale; 
  frac = dc_texturemid + (dc_yl-centery)*fracstep; 

  // Looks like an attempt at dithering,
  // using the colormap #6 (of 0-31, a bit brighter than average).

  do 
    {
      // Lookup framebuffer, and retrieve
      //  a pixel that is either one column
      //  left or right of the current one.
      // Add index from colormap to index.
      // killough 3/20/98: use fullcolormap instead of colormaps

      *dest = fullcolormap[6*256+dest[fuzzoffset[fuzzpos]]]; 

// Some varying invisibility effects can be gotten by playing // phares
// with this logic. For example, try                          // phares
//                                                            // phares
//    *dest = fullcolormap[0*256+dest[FUZZOFF]];              // phares

      // Clamp table lookup index.
      if (++fuzzpos == FUZZTABLE) 
        fuzzpos = 0;
        
      dest += SCREENWIDTH;

      frac += fracstep; 
    } while (count--); 
}

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//

byte *dc_translation, *translationtables;

void R_DrawTranslatedColumn (void) 
{ 
  int      count; 
  byte     *dest; 
  fixed_t  frac;
  fixed_t  fracstep;     
 
  count = dc_yh - dc_yl; 
  if (count < 0) 
    return; 
                                 
#ifdef RANGECHECK 
  if ((unsigned)dc_x >= SCREENWIDTH
      || dc_yl < 0
      || dc_yh >= SCREENHEIGHT)
    I_Error ( "R_DrawColumn: %i to %i at %i",
              dc_yl, dc_yh, dc_x);
#endif 

  // FIXME. As above.
  dest = ylookup[dc_yl] + columnofs[dc_x]; 

  // Looks familiar.
  fracstep = dc_iscale; 
  frac = dc_texturemid + (dc_yl-centery)*fracstep; 
  
  // Here we do an additional index re-mapping.
  do 
    {
      // Translation tables are used
      //  to map certain colorramps to other ones,
      //  used with PLAY sprites.
      // Thus the "green" ramp of the player 0 sprite
      //  is mapped to gray, red, black/indigo. 
      
      *dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
      dest += SCREENWIDTH;
        
      frac += fracstep; 
    }
  while (count--); 
} 

//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//

void R_InitTranslationTables (void)
{
  int i;
        
  // killough 5/2/98:
  // Remove dependency of colormaps aligned on 256-byte boundary

  translationtables = Z_Malloc(256*3, PU_STATIC, 0);
    
  // translate just the 16 green colors
  for (i=0; i<256; i++)
    if (i >= 0x70 && i<= 0x7f)
      {   // map green ramp to gray, brown, red
        translationtables[i] = 0x60 + (i&0xf);
        translationtables [i+256] = 0x40 + (i&0xf);
        translationtables [i+512] = 0x20 + (i&0xf);
      }
    else  // Keep all other colors as is.
      translationtables[i]=translationtables[i+256]=translationtables[i+512]=i;
}

//
// R_DrawSpan 
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//

int  ds_y; 
int  ds_x1; 
int  ds_x2;

lighttable_t *ds_colormap; 

fixed_t ds_xfrac; 
fixed_t ds_yfrac; 
fixed_t ds_xstep; 
fixed_t ds_ystep;

// start of a 64*64 tile image 
byte *ds_source;        

#ifndef DJGPP      // killough 2/15/98
#ifndef LINUX // CPhipps added
void R_DrawSpan (void) 
{ 
  register unsigned position;
  unsigned step;

  byte *source;
  byte *colormap;
  byte *dest;
    
  unsigned count;
  unsigned spot; 
  unsigned xtemp;
  unsigned ytemp;
                
  position = ((ds_xfrac<<10)&0xffff0000) | ((ds_yfrac>>6)&0xffff);
  step = ((ds_xstep<<10)&0xffff0000) | ((ds_ystep>>6)&0xffff);
                
  source = ds_source;
  colormap = ds_colormap;
  dest = ylookup[ds_y] + columnofs[ds_x1];       
  count = ds_x2 - ds_x1 + 1; 
        
  while (count >= 4)
    { 
      ytemp = position>>4;
      ytemp = ytemp & 4032;
      xtemp = position>>26;
      spot = xtemp | ytemp;
      position += step;
      dest[0] = colormap[source[spot]]; 

      ytemp = position>>4;
      ytemp = ytemp & 4032;
      xtemp = position>>26;
      spot = xtemp | ytemp;
      position += step;
      dest[1] = colormap[source[spot]];
        
      ytemp = position>>4;
      ytemp = ytemp & 4032;
      xtemp = position>>26;
      spot = xtemp | ytemp;
      position += step;
      dest[2] = colormap[source[spot]];
        
      ytemp = position>>4;
      ytemp = ytemp & 4032;
      xtemp = position>>26;
      spot = xtemp | ytemp;
      position += step;
      dest[3] = colormap[source[spot]]; 
                
      dest += 4;
      count -= 4;
    } 

  while (count)
    { 
      ytemp = position>>4;
      ytemp = ytemp & 4032;
      xtemp = position>>26;
      spot = xtemp | ytemp;
      position += step;
      *dest++ = colormap[source[spot]]; 
      count--;
    } 
} 
#endif
#endif

//
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//

void R_InitBuffer(int width, int height)
{ 
  int i; 
    
  // Handle resize,
  //  e.g. smaller view windows
  //  with border and/or status bar.

  viewwindowx = (SCREENWIDTH-width) >> 1; 

  // Column offset. For windows.

  for (i=0 ; i<width ; i++) 
    columnofs[i] = viewwindowx + i;
    
  // Same with base row offset.

  viewwindowy = width==SCREENWIDTH ? 0 : (SCREENHEIGHT-SBARHEIGHT-height)>>1; 

  // Preclaculate all row offsets.

  for (i=0 ; i<height ; i++) 
    ylookup[i] = screens[0] + (i+viewwindowy)*SCREENWIDTH; 
} 

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//

void R_FillBackScreen (void) 
{ 
  byte    *dest, *src;
  int     x,y; 
  patch_t *patch;

  if (scaledviewwidth == 320)
    return;

  // killough 4/17/98: 
  src = W_CacheLumpNum(firstflat +
          R_FlatNumForName(gamemode == commercial ? "GRNROCK" : "FLOOR7_2"),
            PU_CACHE);

  dest = screens[1]; 
         
  for (y=0 ; y<SCREENHEIGHT-SBARHEIGHT ; y++) 
    { 
      int x;
      for (x=0 ; x<SCREENWIDTH/64 ; x++) 
        { 
          memcpy(dest, src+((y&63)<<6), 64); 
          dest += 64; 
        } 
      if (SCREENWIDTH&63) 
        { 
          memcpy(dest, src+((y&63)<<6), SCREENWIDTH&63); 
          dest += (SCREENWIDTH&63); 
        } 
    } 
        
  patch = W_CacheLumpName("brdr_t", PU_CACHE);

  for (x=0; x<scaledviewwidth; x+=8)
    V_DrawPatch(viewwindowx+x,viewwindowy-8,1,patch);

  patch = W_CacheLumpName("brdr_b",PU_CACHE);

  for (x=0; x<scaledviewwidth; x+=8)
    V_DrawPatch (viewwindowx+x,viewwindowy+viewheight,1,patch);

  patch = W_CacheLumpName("brdr_l",PU_CACHE);

  for (y=0; y<viewheight; y+=8)
    V_DrawPatch (viewwindowx-8,viewwindowy+y,1,patch);
  patch = W_CacheLumpName("brdr_r",PU_CACHE);

  for (y=0; y<viewheight; y+=8)
    V_DrawPatch(viewwindowx+scaledviewwidth,viewwindowy+y,1,patch);

  // Draw beveled edge. 
  V_DrawPatch(viewwindowx-8,
              viewwindowy-8,
              1,
              W_CacheLumpName("brdr_tl",PU_CACHE));
    
  V_DrawPatch(viewwindowx+scaledviewwidth,
              viewwindowy-8,
              1,
              W_CacheLumpName("brdr_tr",PU_CACHE));
    
  V_DrawPatch(viewwindowx-8,
              viewwindowy+viewheight,
              1,
              W_CacheLumpName("brdr_bl",PU_CACHE));
    
  V_DrawPatch(viewwindowx+scaledviewwidth,
              viewwindowy+viewheight,
              1,
              W_CacheLumpName("brdr_br",PU_CACHE));
} 

//
// Copy a screen buffer.
//

void R_VideoErase(unsigned ofs, int count)
{ 
  memcpy(screens[0]+ofs, screens[1]+ofs, count);   // LFB copy.
} 

//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//

void V_MarkRect(int x, int y, int width, int height); 
 
void R_DrawViewBorder(void) 
{ 
  int top, side, ofs, i;
 
  if (scaledviewwidth == SCREENWIDTH) 
    return; 
  
  top = ((SCREENHEIGHT-SBARHEIGHT)-viewheight)/2; 
  side = (SCREENWIDTH-scaledviewwidth)/2; 
 
  // copy top and one line of left side 
  R_VideoErase (0, top*SCREENWIDTH+side); 
 
  // copy one line of right side and bottom 
  ofs = (viewheight+top)*SCREENWIDTH-side; 
  R_VideoErase (ofs, top*SCREENWIDTH+side); 
  
  // copy sides using wraparound 
  ofs = top*SCREENWIDTH + SCREENWIDTH-side; 
  side <<= 1;
    
  for (i=1 ; i<viewheight ; i++) 
    { 
      R_VideoErase (ofs, side); 
      ofs += SCREENWIDTH; 
    } 

  V_MarkRect (0,0,SCREENWIDTH, SCREENHEIGHT-SBARHEIGHT); 
} 

//----------------------------------------------------------------------------
//
// $Log: r_draw.c,v $
// Revision 1.16  1998/05/03  22:41:46  killough
// beautification
//
// Revision 1.15  1998/04/19  01:16:48  killough
// Tidy up last fix's code
//
// Revision 1.14  1998/04/17  15:26:55  killough
// fix showstopper
//
// Revision 1.13  1998/04/12  01:57:51  killough
// Add main_tranmap
//
// Revision 1.12  1998/03/23  03:36:28  killough
// Use new 'fullcolormap' for fuzzy columns
//
// Revision 1.11  1998/02/23  04:54:59  killough
// #ifdef out translucency code since its in asm
//
// Revision 1.10  1998/02/20  21:57:04  phares
// Preliminarey sprite translucency
//
// Revision 1.9  1998/02/17  06:23:40  killough
// #ifdef out code duplicated in asm for djgpp targets
//
// Revision 1.8  1998/02/09  03:18:02  killough
// Change MAXWIDTH, MAXHEIGHT defintions
//
// Revision 1.7  1998/02/02  13:17:55  killough
// performance tuning
//
// Revision 1.6  1998/01/27  16:33:59  phares
// more testing
//
// Revision 1.5  1998/01/27  16:32:24  phares
// testing
//
// Revision 1.4  1998/01/27  15:56:58  phares
// Comment about invisibility
//
// Revision 1.3  1998/01/26  19:24:40  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/26  05:05:55  killough
// Use unrolled version of R_DrawSpan
//
// Revision 1.1.1.1  1998/01/19  14:03:02  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
