/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: r_draw.c,v 1.15 1999/11/01 17:09:15 cphipps Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      The actual span/column drawing functions.
 *      Here find the main potential for optimization,
 *       e.g. inline assembly, different algorithms.
 *
 *-----------------------------------------------------------------------------*/

static const char
rcsid[] = "$Id: r_draw.c,v 1.15 1999/11/01 17:09:15 cphipps Exp $";

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "v_video.h"
#include "st_stuff.h"
#include "g_game.h"
#include "am_map.h"
#include "lprintf.h"

#define MAXWIDTH  MAX_SCREENWIDTH          /* kilough 2/8/98 */
#define MAXHEIGHT MAX_SCREENHEIGHT

// CPhipps - height of status bar on the screen
#define SBARHEIGHT st_height
#define SBARWIDTH ST_TWIDTH

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

// leban 1/17/99:
//
// these next two are pre-calculated to help the speed of the inner
// loops.  however, they're not a win on a powerpc, and probably not
// on any other modern cpu that isn't afraid of multiplies.
//
// consider ylookup.  below, it's initialized in a loop as
//    columnofs[i] = viewwindowx + i;
// that's one addition.  indexing into an array is one addition.
// but since columnofs is an array with global scope, loading usually
// is another instruction.  on powerpc, the value is stored in the
// TOC instead of the address.  i think x86 does something similar,
// as someone added a bunch of local copies of similar variables below.
// that tactic can move an extra load out of an inner loop.
//
// but wait, there's more, as an array offset must be converted into
// array units, which in this case is most likely a shift left.  that's
// one more instruction wasted per array index.
//
// there's also an extra benefit on powerpc:  the number of registers
// used in R_DrawColumn is reduced, and a stack frame is no longer
// needed.  there's another two instructions saved.
//
// i'll leave these two in for now, but they could eventually get
// removed.  columnofs[] is actually referenced elsewhere.  topleft
// isn't static to work around a metrowerks compiler bug.
//
// XXX
//
// CPhipps - also to use it in the i386 asm I need it global

byte *ylookup[MAXHEIGHT]; 
//int  columnofs[MAXWIDTH]; 
byte *topleft;

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//

// CPhipps - made const*'s
const byte *tranmap;          // translucency filter maps 256x256   // phares 
const byte *main_tranmap;     // killough 4/11/98

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

#ifndef I386_ASM     // killough 2/15/98

void R_DrawColumn_Normal (void) 
{
  int              count; 
  register byte    *dest;            // killough
  register fixed_t frac;            // killough
  fixed_t          fracstep;     

  // leban 1/17/99:
  // on powerpc, this routine currently does not have a stack frame,
  // but it's right at the point.  check out the assembler if it
  // gets changed.


  // leban 1/17/99:
  // removed the + 1 here, adjusted the if test, and added an increment
  // later.  this helps a compiler pipeline a bit better.  the x86
  // assembler also does this.

  count = dc_yh - dc_yl; 

  // leban 1/17/99:
  // this case isn't executed too often.  depending on how many instructions
  // there are between here and the second if test below, this case could
  // be moved down and might save instructions overall.  since there are
  // probably different wads that favor one way or the other, i'll leave
  // this alone for now.
  if (count < 0)    // Zero length, column does not exceed a pixel.
    return; 

  count++;
 {
    // leban 1/5/99:
    // making local copies of these variables allows
    // compilers to optimize out a few instructions.  in the
    // powerpc case, the values sit in registers instead of
    // being loaded thru the TOC.
    register int                   scrwid = SCREENWIDTH;
    register const byte *source = dc_source;            
    register const lighttable_t *colormap = dc_colormap; 


#ifdef RANGECHECK 
  if ((unsigned)dc_x >= SCREENWIDTH
      || dc_yl < 0
      || dc_yh >= SCREENHEIGHT) 
    I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  // Use columnofs LUT for subwindows? 

//  dest = ylookup[dc_yl] + columnofs[dc_x];  
  dest = topleft + dc_yl*scrwid + dc_x;  

  // Determine scaling, which is the only mapping to be done.

  fracstep = dc_iscale; 
  frac = dc_texturemid + (dc_yl-centery)*fracstep; 

  // Inner loop that does the actual texture mapping,
  //  e.g. a DDA-lile scaling.
  // This is as fast as it gets.       (Yeah, right!!! -- killough)
  //
  // killough 2/1/98: more performance tuning

    // leban 1/17/99:
    // changing this to heightmask == 127 causes a stack frame to
    // be created...using too many registers at once, i think.
    // strange.

    if(dc_texheight == 128)
    {
        // leban 1/5/99:
        // no tutti-frutti possible.  most textures are of this variety.
        // the x86 assembler already performs this optimization.
        // yes, the silliness with "count" in the loop is for a reason:
        // the metrowerks powerpc compiler generates the bdnz (branch
        // and decrement on non-zero) instruction.  that's one add
        // and a conditional branch in one.  zero-cycle branch in
        // this case.
        //
        // about unrolling (and inlining)...modern cpu makers put a
        // great deal of silicon into prefetching instructions,
        // detecting branches, predicting branches, and executing
        // spectulatively.  in the world of "free" branches,
        // mindlessly unrolling or inlining can increase the code
        // size with no benefit.  on machines with small caches,
        // this is particularly important.
        //
        // note the word "mindlessly."  unrolling a loop to combine
        // reads or writes is quite valid (see R_DrawSpan below).
        // here, since consecutive *dests are not sucessive in
        // memory and the reads aren't consistent, unrolling the
        // loop doesn't help much.  cpu makers have also been
        // known to combine writes anyway.
        //
        // the metrowerks compiler utterly refuses to generate
        // dbnz for any do/while loop but the one you see here.
        // or i can't get it to.  this loop should be written
        // as a do{}while(count>0), since we've tested for
        // the count<=0 case above.  that's two extra instructions
        // for each while() in the wrong place.  the ppc assembler
        // for this routine takes these two instructions out and
        // leaves everything else the same.
        //
        // oh yeah, this loop is about 20% of doom on my 2x603 66mhz
        // bebox.  six instructions in the loop.

        while(count>0)
        {
                *dest = colormap[source[(frac>>FRACBITS)&127]];
                dest += scrwid; 
                frac += fracstep;
                count--;
        }
    }
    else
    {
     register unsigned heightmask = dc_texheight-1; // CPhipps - specify type
     if (! (dc_texheight & heightmask) )   // power of 2 -- killough
     {
         while (count>0)   // texture height is a power of 2 -- killough
           {
             *dest = colormap[source[(frac>>FRACBITS) & heightmask]];
             dest += scrwid; 
             frac += fracstep;
            count--;
           }
     }
     else
     {
         heightmask++;
         heightmask <<= FRACBITS;
           
         if (frac < 0)
           while ((frac += heightmask) <  0);
         else
           while (frac >= heightmask)
             frac -= heightmask;
           
         while(count>0)
           {
             // Re-map color indices from wall texture column
             //  using a lighting/special effects LUT.
             
             // heightmask is the Tutti-Frutti fix -- killough
             
             *dest = colormap[source[frac>>FRACBITS]];
             dest += scrwid; 
             if ((frac += fracstep) >= heightmask)
               frac -= heightmask;
            count--;
           } 
     }
    }
 }
}

void R_DrawColumn_HighRes (void)
{
        R_DrawColumn_Normal(); 
}

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

#ifndef I386_ASM                       // killough 2/21/98: converted to x86 asm

void R_DrawTLColumn_Normal (void)                                           
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

  dest = topleft + dc_yl*SCREENWIDTH + dc_x;  
  
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
    register unsigned heightmask = dc_texheight-1; // CPhipps - specify type
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

void R_DrawTLColumn_HighRes (void)
{
	R_DrawTLColumn_Normal();
}
#endif  // killough 2/21/98: converted to x86 asm

//
// Spectre/Invisibility.
//

#define FUZZTABLE 50 
// proff 08/17/98: Changed for high-res
//#define FUZZOFF (SCREENWIDTH)
#define FUZZOFF 1

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
  dest = topleft + dc_yl*SCREENWIDTH + dc_x;
  
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
  dest = topleft + dc_yl*SCREENWIDTH + dc_x; 

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

byte playernumtotrans[MAXPLAYERS];
extern lighttable_t *(*c_zlight)[LIGHTLEVELS][MAXLIGHTZ];

void R_InitTranslationTables (void)
{
  int i, j;
#define MAXTRANS 3
  byte transtocolour[MAXTRANS];
        
  // killough 5/2/98:
  // Remove dependency of colormaps aligned on 256-byte boundary

  if (translationtables == NULL) // CPhipps - allow multiple calls
    translationtables = Z_Malloc(256*MAXTRANS, PU_STATIC, 0);

  for (i=0; i<MAXTRANS; i++) transtocolour[i] = 255;

  for (i=0; i<MAXPLAYERS; i++) {
    byte wantcolour = mapcolor_plyr[i];
    playernumtotrans[i] = 0;
    if (wantcolour != 0x70) // Not green, would like translation
      for (j=0; j<MAXTRANS; j++)
	if (transtocolour[j] == 255) {
	  transtocolour[j] = wantcolour; playernumtotrans[i] = j+1; break;
	}
  }
    
  // translate just the 16 green colors
  for (i=0; i<256; i++)
    if (i >= 0x70 && i<= 0x7f)
      {
	// CPhipps - configurable player colours
        translationtables[i] = colormaps[0][((i&0xf)<<9) + transtocolour[0]];
        translationtables[i+256] = colormaps[0][((i&0xf)<<9) + transtocolour[1]];
        translationtables[i+512] = colormaps[0][((i&0xf)<<9) + transtocolour[2]];
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

#ifndef I386_ASM      // killough 2/15/98

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
  dest = topleft + ds_y*SCREENWIDTH + ds_x1;       
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

//
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//

void R_InitBuffer(int width, int height)
{ 
  int i=0;
  // Handle resize,
  //  e.g. smaller view windows
  //  with border and/or status bar.

  viewwindowx = (SCREENWIDTH-width) >> 1; 

  // Same with base row offset.

  viewwindowy = width==SCREENWIDTH ? 0 : (SCREENHEIGHT-SBARHEIGHT-height)>>1; 

  topleft = screens[0] + viewwindowy*SCREENWIDTH + viewwindowx;

  // Preclaculate all row offsets.
  // CPhipps - merge viewwindowx into here

  for (i=0 ; i<height ; i++) 
    ylookup[i] = screens[0] + (i+viewwindowy)*SCREENWIDTH + viewwindowx; 
} 

//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
// CPhipps - patch drawing updated

void R_FillBackScreen (void) 
{ 
  byte    *dest;
  const byte *src;
  int     x,y; 
  int     patch; // CPhipps - the lump number of the patch
  int     flump; // cph - lump number of the flat

  flump = firstflat +
          R_FlatNumForName(gamemode == commercial ? "GRNROCK" : "FLOOR7_2");
  src = W_CacheLumpNum(flump);

  dest = screens[1]; 
         
  for ( y = 0 ; y < SCREENHEIGHT; y++ ) // proff/nicolas
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
  W_UnlockLumpNum(flump);

  // CPhipps - bevelled edge on background either side of status bar
  if (viewwindowy + viewheight < SCREENHEIGHT) {
    patch = W_GetNumForName("brdr_b");
    
    for (x=0; x<scaledviewwidth; x+=8)
      V_DrawNumPatch(viewwindowx+x, viewwindowy+viewheight, 1, patch, NULL, VPT_NONE);
  }
// proff 08/17/98: Changed for high-res
// proff/nicolas 09/20/98: Moved down for high-res
  if (scaledviewwidth == SCREENWIDTH)
    return;

  patch = W_GetNumForName("brdr_t");
  for (x=0; x<scaledviewwidth; x+=8)
    V_DrawNumPatch(viewwindowx+x, viewwindowy-8, 1, patch, NULL, VPT_NONE);

  patch = W_GetNumForName("brdr_l");
  for (y=0; y<viewheight; y+=8)
    V_DrawNumPatch(viewwindowx-8, viewwindowy+y, 1, patch, NULL, VPT_NONE);

  patch = W_GetNumForName("brdr_r");
  for (y=0; y<viewheight; y+=8)
    V_DrawNumPatch(viewwindowx+scaledviewwidth, viewwindowy+y, 
		   1, patch, NULL, VPT_NONE);

  // Draw beveled edge. 
  V_DrawNamePatch(viewwindowx-8, viewwindowy-8, 1,
		  "brdr_tl", NULL, VPT_NONE);
    
  V_DrawNamePatch(viewwindowx+scaledviewwidth, viewwindowy-8,
              1, "brdr_tr", NULL, VPT_NONE);
    
  V_DrawNamePatch(viewwindowx-8, viewwindowy+viewheight,
		  1, "brdr_bl", NULL, VPT_NONE);
    
  V_DrawNamePatch(viewwindowx+scaledviewwidth, viewwindowy+viewheight,
		  1, "brdr_br", NULL, VPT_NONE);
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
// proff/nicolas 09/20/98: Added for high-res (inspired by DosDOOM)
  int side2;

// proff/nicolas 09/20/98: Removed for high-res
  //  if (scaledviewwidth == SCREENWIDTH) 
  //  return; 
 
// proff/nicolas 09/20/98: Added for high-res (inspired by DosDOOM)
  if (( SCREENWIDTH > SBARWIDTH ) && ((SCREENHEIGHT != viewheight) || 
				      ((automapmode & am_active) && ! (automapmode & am_overlay))))
  {
    ofs = ( SCREENHEIGHT - SBARHEIGHT ) * SCREENWIDTH;
    side= ( SCREENWIDTH - SBARWIDTH ) / 2; 
	  side2 = side * 2;

    R_VideoErase ( ofs, side );
    
    ofs += ( SCREENWIDTH - side );
    for ( i = 1; i < SBARHEIGHT; i++ )
	  {
      R_VideoErase ( ofs, side2 );
      ofs += SCREENWIDTH;
    }

    R_VideoErase ( ofs, side );
  }

  if ( viewheight >= ( SCREENHEIGHT - SBARHEIGHT ))
    return; // if high-res, don�t go any further!

// proff/nicolas 09/20/98: End of addition
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
// Revision 1.15  1999/11/01 17:09:15  cphipps
// Added lprintf.h (needed for RANGECHECK debugging I_Error calls)
//
// Revision 1.14  1999/10/12 13:01:14  cphipps
// Changed header to GPL
//
// Revision 1.13  1999/08/31 19:33:00  cphipps
// Modified R_DrawViewBorder so it draws the border either side of the status bar
// always when on the automap
//
// Revision 1.12  1999/03/24 13:47:34  cphipps
// Player colours done properly, colourmap is used to get darker colours for that
// colour without assuming palette order
//
// Revision 1.11  1999/03/22 12:09:54  cphipps
// Extend configurable player colour support
//
// Revision 1.10  1999/02/08 08:46:30  cphipps
// Fix status bar width to use macro (to pick up status bar scaling)
//
// Revision 1.9  1999/02/04 21:36:45  cphipps
// Improved status bar height handling
//
// Revision 1.8  1999/01/29 22:21:10  cphipps
// Remove columnofs[]
// Use multiplies instead of ylookup[] for non-i386 targets
// Verified to compile without -DI386
// Included leban's optimised R_DrawColumn and comments
//
// Revision 1.7  1998/12/31 23:05:26  cphipps
// New wad lump handling
// Made tranmap's const
//
// Revision 1.6  1998/12/31 11:23:46  cphipps
// Patch drawing updated
// R_FillBackScreen recoded slightly
//
// Revision 1.5  1998/12/28 13:11:40  cphipps
// Made multiplayer colours controlled by variable
//
// Revision 1.4  1998/12/24 18:08:45  cphipps
// Add bevelled edge to background either side of status bar
// Remove unused array
//
// Revision 1.3  1998/11/17 07:58:11  cphipps
// Hi-res changes
//
// Revision 1.2  1998/10/17 14:59:30  cphipps
// Specify type on heightmask variables (was defaulting to int)
//
// Revision 1.1  1998/09/13 16:49:50  cphipps
// Initial revision
//
// Revision 1.2  1998/09/13 11:15:42  cphipps
// Comment out C versions of asm'ed funcs for I386 targets, instead of DJGPP
//
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
