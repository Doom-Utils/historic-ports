//
// DOSDoom Column/Span Drawing for 16-bit Colour Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// Note: The best place for optimisation!
//
// -ACB- 1998/09/10 Cleaned up.
//

#include <stdio.h>

#include "dm_defs.h"
#include "dm_state.h"
#include "i_alleg.h"
#include "i_system.h"
#include "r_draw2.h" // -ES- 1998/08/05 Include Header
#include "r_local.h"
#include "v_res.h"
#include "w_wad.h"
#include "z_zone.h"

#include "d_debug.h"

#define SBARHEIGHT 32

//
// All drawing to the view buffer is accomplished in this file.
//
// The other refresh files only know about ccordinates, not the
// architecture of the frame buffer.
//
// Conveniently, the frame buffer is a linear one, and we need
// only the base address, and the total size.
//
byte* viewimage;
int viewwidth;
int viewheight;
int viewwindowx;
int viewwindowy;
int scaledviewwidth;
 
// just for profiling
int dccount;

void resinit_r_draw_c16(void)
{
  int i;

  // -ES- 1998/08/20 Made fuzzoffset calculation work with dynamic resolution changing
  for (FUZZTABLE=0,i=0;fuzzoffset[i];i++, FUZZTABLE++)
  {
    if (fuzzoffset[i] > 0)
      fuzzoffset[i]=SCREENDEPTH*FUZZOFF;
    if (fuzzoffset[i] < 0)
      fuzzoffset[i]=-SCREENDEPTH*FUZZOFF;
  }
  fuzzpos=0;

#define set_screenwidth(val, lbl) asm("movl %0," lbl "-4":: "r" (val) : "memory")
  set_screenwidth(2*SCREENWIDTH,  "rdc16owidth1");
  set_screenwidth(2*SCREENWIDTH,  "rdc16owidth2");
#undef set_screenwidth
}

//
// R_DrawColumn16_CVersion
//
// A column is a vertical slice/span from a wall texture that, given the
// DOOM style restrictions on the view orientation, will always have
// constant z depth.
//
// Thus a special case loop for very fast rendering can
// be used. It has also been used with Wolfenstein 3D.
// 
void R_DrawColumn16_CVersion (void)
{ 
  int count;
  short* dest;
  short* tempcolormap;
  fixed_t frac;
  fixed_t fracstep;
 
  count = dc_yh - dc_yl+1;

  // Zero length, column does not exceed a pixel.
  if (count <= 0)
    return;
                                 
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawColumn16_CVersion: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif 

  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  dest = (short *)(ylookup[dc_yl] + columnofs[dc_x]);

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Inner loop that does the actual texture mapping, e.g. a DDA-lile scaling.
  tempcolormap=(short *)(dc_colormap);

  do
  {
    // Re-map color indices from wall texture column
    // using a lighting/special effects LUT.
    *dest = tempcolormap[dc_source[(frac>>FRACBITS)&127]];
        
    dest += SCREENWIDTH;
    frac += fracstep;
        
  }
  while (--count);
}

void R_DrawColumn16_KM (void)
{
#ifndef SMOOTHING
  R_DrawColumn16_CVersion();
#else
  int count;
  short* dest;
  unsigned short* tempcolormap;
  unsigned long frac;
  unsigned long fracstep;


  count = dc_yh - dc_yl+1;

  // Zero length, column does not exceed a pixel.
  if (count <= 0)
    return;
                                 
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawColumn16_CVersion: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif 

  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  dest = (short *)(ylookup[dc_yl] + columnofs[dc_x]);

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Inner loop that does the actual texture mapping, e.g. a DDA-lile scaling.
  tempcolormap=(unsigned short *)(dc_colormap);
  if (fracstep > 0x12000)
  {
    do
    {
      // Re-map color indices from wall texture column
      // using a lighting/special effects LUT.
      *dest = tempcolormap[dc_source[(frac>>FRACBITS)&127]];
        
      dest += SCREENWIDTH;
      frac += fracstep;
        
    }
    while (--count);
  }
  else
  {
    unsigned long spot[4];
    unsigned long level[4];
    unsigned long c;
    int i;
    extern fixed_t dc_xfrac;
    extern byte* dc_source2;

    dc_xfrac &= 0xffff;
    do
    {
      level[3] = (frac&0xffff) * dc_xfrac;
      level[2] = (FRACUNIT - (frac&0xffff)-1) * dc_xfrac;
      level[1] = (frac&0xffff) * (FRACUNIT - dc_xfrac-1);
      level[0] = (FRACUNIT - (frac&0xffff)-1) * (FRACUNIT - dc_xfrac-1);

      spot[0] = tempcolormap[dc_source[(frac>>FRACBITS)&127]];
      spot[1] = tempcolormap[dc_source[((frac>>FRACBITS)+1)&127]];
      spot[2] = tempcolormap[dc_source2[(frac>>FRACBITS)&127]];
      spot[3] = tempcolormap[dc_source2[((frac>>FRACBITS)+1)&127]];

      for (i = 0, c = 0; i < 4; i++)
      {
        level[i] >>= 26;
        c += col2rgb16[level[i]][spot[i]&0xff][0] + col2rgb16[level[i]][(spot[i]>>8)&0xff][1];
      }
      c &= hicolortransmask3;
      c |= c>>16;
      c >>= hicolortransshift;
      *dest = (short) c;

      dest += SCREENWIDTH;
      frac += fracstep;
    } while (--count);
  }
#endif
}

//
// R_DrawFuzzColumn16
//
// Framebuffer postprocessing.
//
// Creates a fuzzy image by copying pixels from adjacent ones to left and right.
//
// Used with an all black colormap, this could create the SHADOW effect,
// i.e. spectres and invisible players.
//

void R_DrawFuzzColumn16 (void)
{ 
  int count;
  short* dest;
  fixed_t frac;
  fixed_t fracstep;
  unsigned long c;
  unsigned short temppixel;

  // Adjust borders. Low...
  if (dc_yl < 2)
    dc_yl = 2;

  // .. and high.
  if (dc_yh > viewheight-2)
    dc_yh = viewheight - 3;
                 
  count = dc_yh - dc_yl;

  // Zero length.
  if (count < 0)
    return;
    
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
     I_Error ("R_DrawFuzzColumn16: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

  // Does not work with blocky mode.
  dest = (short *)(ylookup[dc_yl] + columnofs[dc_x]);

  // Looks familiar.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Looks like an attempt at dithering, using the colormap #6 (of 0-31, a bit
  // brighter than average).
  do
  {
    // Lookup framebuffer, and retrieve a pixel that is either one column
    // left or right of the current one. Add index from colormap to index.
    temppixel=dest[fuzzoffset[fuzzpos]];

    // Colours don't quite add to one: invisible creature is darker.
    c = col2rgb16[42][temppixel&0xff][0] +
        col2rgb16[42][temppixel>>8][1]   +
        col2rgb16[16][*dest & 0xff][0] +
        col2rgb16[16][*dest >> 8][1];
    c &= hicolortransmask3;
    c |= c>>16;
    c >>= hicolortransshift;
    *dest = (short) c;

    // Clamp table lookup index.
    if (++fuzzpos == FUZZTABLE)
      fuzzpos = 0;
        
    dest += SCREENWIDTH;

    frac += fracstep;
  }
  while (count--);
} 

// -ES- 1998/11/08 New tranlsucency. It's slow, but it works.
// -KM- 1998/11/25 Modified for use with new trans system. (dc_translucency)
// -ES- 1998/11/29 Improved the translucency algorithm.
void R_DrawTranslucentColumn16 ()
{ 
  int count;
  unsigned short *dest;
  fixed_t frac;
  fixed_t fracstep;

  fixed_t fglevel, bglevel;
  unsigned long c,fg,bg;        // current colour

  fglevel = dc_translucency;
#if FADER
  if (dc_translucency == 0x8000)
    fglevel = abs(256-(leveltime & 0x1ff)) << 8;
#endif
  fglevel = fglevel&~0x3ff;
  bglevel = FRACUNIT-fglevel;
  fglevel >>= 10;
  bglevel >>= 10;

  count = dc_yh - dc_yl+1;


  // Zero length, column does not exceed a pixel.
  if (count <= 0)
    return;
                                 
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif 

  //
  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  //
  dest = (unsigned short *)(ylookup[dc_yl] + columnofs[dc_x]);

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  //
  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  //
  do
  {
    fg=(unsigned long) (((unsigned short*)dc_colormap)[dc_source[(frac>>FRACBITS)&127]]);
    bg = *dest;
    c = col2rgb16[fglevel][fg&0xff][0] +
        col2rgb16[fglevel][fg>>8][1]   +
        col2rgb16[bglevel][bg&0xff][0] +
        col2rgb16[bglevel][bg>>8][1];
    c &= hicolortransmask3;
    c |= c>>16;
    c >>= hicolortransshift;
    *dest = (short)c;
    dest+=SCREENWIDTH;
    frac+=fracstep;
  }
  while (--count);
} 

//
// R_DrawTranslatedColumn16
//
// Uses the translationtables to remap one set of palette colours to
// another. One prime example is the player greens to the other player
// colours.
//
// Could be used with different translation tables, e.g. the lighter colored
// version of the BaronOfHell, the HellKnight, uses identical sprites,
// kinda brightened up.
//
// 16-Bit Version.
//
// -ES- 1998/11/29 Improved the translucency algorithm.
void R_DrawTranslatedColumn16 (void)
{ 
  int count;
  short* dest;
  fixed_t frac;
  fixed_t fracstep;
  short* tempcolormap;

  count = dc_yh - dc_yl;

  if (count < 0) 
    return;
                                 
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawTranslatedColumn16: %i to %i at %i",dc_yl, dc_yh, dc_x);
#endif 

  dest = (short *)(ylookup[dc_yl] + columnofs[dc_x]);

  // Looks familiar.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Here we do an additional index re-mapping.
  tempcolormap=(short *)dc_colormap;

  do
  {
    *dest = tempcolormap[dc_translation[dc_source[frac>>FRACBITS]]];
    dest += SCREENWIDTH;
        
    frac += fracstep;
  }
  while (count--);
} 
void R_DrawTranslucentTranslatedColumn16 ()
{ 
  int count;
  unsigned short *dest;
  fixed_t frac;
  fixed_t fracstep;

  fixed_t fglevel, bglevel;
  unsigned long c,fg,bg;        // current colour

  fglevel = dc_translucency;
#if FADER
  if (dc_translucency == 0x8000)
    fglevel = abs(256-(leveltime & 0x1ff)) << 8;
#endif
  fglevel = fglevel&~0x3ff;
  bglevel = FRACUNIT-fglevel;
  fglevel >>= 10;
  bglevel >>= 10;

  count = dc_yh - dc_yl+1;


  // Zero length, column does not exceed a pixel.
  if (count <= 0)
    return;
                                 
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif 

  //
  // Framebuffer destination address.
  // Use ylookup LUT to avoid multiply with ScreenWidth.
  //
  dest = (unsigned short *)(ylookup[dc_yl] + columnofs[dc_x]);

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  //
  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  //
  do
  {
    fg=(unsigned long) (((unsigned short*)dc_colormap)[dc_translation[dc_source[(frac>>FRACBITS)&127]]]);
    bg = *dest;
    c = col2rgb16[fglevel][fg&0xff][0] +
        col2rgb16[fglevel][fg>>8][1]   +
        col2rgb16[bglevel][bg&0xff][0] +
        col2rgb16[bglevel][bg>>8][1];
    c &= hicolortransmask3;
    c |= c>>16;
    c >>= hicolortransshift;
    *dest = (short)c;
    dest+=SCREENWIDTH;
    frac+=fracstep;
  }
  while (--count);
} 

//
// R_InitTranslationTables16
//
// Reads the PALREMAP lump for information on translating one
// colour on the palette to another. The PALREMAP is broken down
// into 64-byte sections(lumps). Each section has 32 remaps; a remap
// consists on one byte that is the number of the colour to be
// replaced, the second byte contain the number of the replacement
// colour.
//
// The prime example of PALREMAP in action is the player sprites,
// in order to tell your fellow players apart, the remapping function
// changes the greens (numbers: 112->128) into another colour.
//
// 16-Bit Colour Version.
//
// -ACB- 1998/09/10 Replaced the old procedure with this.
//
#define REMAPLENGTH 64
#define REMAPSPERLUMP 32

// -ES- 1998/11/08 Fixed memory leak
static byte *translationtables16 = NULL;
void R_InitTranslationTables16 (void)
{
  int i;
  int j;
  int lump;
  int length;
  int lump_p;
  byte* lumpdata;

  if (!translationtables16)
  {
    // Load in the light tables, 256 byte align tables.
    lump = W_GetNumForName("PALREMAP");
    length = W_LumpLength (lump);
    lumpdata = Z_Malloc(length*sizeof(byte), PU_STATIC, 0);
    W_ReadLump(lump, lumpdata);
    translationtables16 = Z_Malloc (256*(length/REMAPLENGTH)*2+255, PU_STATIC, 0);
    translationtables16 = (byte *)(( (int)translationtables16 + 255 )& ~255);

    lump_p = 0;

    for (i=0; i<((length/REMAPLENGTH)*256); i+=256)
    {
      for (j=0; j<256; j++)
        translationtables16[i+j] = j;

      for (j=0; j<REMAPSPERLUMP; j++, lump_p+=2)
        translationtables16[i+lumpdata[lump_p]]=translationtables16[i+lumpdata[lump_p+1]];
    }
    Z_Free(lumpdata);
  }
  translationtables = translationtables16;

}


//
// R_DrawSpan16_CVersion
//
// With DOOM style restrictions on view orientation,
// the floors and ceilings consist of horizontal slices
// or spans with constant z depth.
//
// However, rotation around the world z axis is possible,
// thus this mapping, while simpler and faster than
// perspective correct texture mapping, has to traverse
// the texture at an angle in all but a few cases.
//
// In consequence, flats are not stored by column (like walls),
// and the inner loop has to step in texture space u and v.
//
void R_DrawSpan16_CVersion (void)
{ 
  fixed_t xfrac;
  fixed_t yfrac;
  short* dest;
  int count;
  int spot;
  short* tempcolormap;
         
#ifdef DEVELOPERS 
  if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    I_Error( "R_DrawSpan16_CVersion: %i to %i at %i", ds_x1,ds_x2,ds_y);
#endif 

  xfrac = ds_xfrac; 
  yfrac = ds_yfrac;
         
  dest = (short *)(ylookup[ds_y] + columnofs[ds_x1]);

  // We do not check for zero spans here?
  count = ds_x2 - ds_x1;

  tempcolormap=(short *)ds_colormap;

  do
  {
    // Current texture index in u,v.
    spot = ((yfrac>>(FRACBITS-6))&(63*64)) + ((xfrac>>FRACBITS)&63);

    // Lookup pixel from flat texture tile, re-index using light/colormap.
    *dest++ = tempcolormap[ds_source[spot]];

    // Next step in u,v.
    xfrac += ds_xstep;
    yfrac += ds_ystep;
        
  }
  while (count--);
} 

// -KM- 1998/11/25 Finished this.  It is *REAL* slow. Make sure you have the hardware.
void R_DrawSpan16_KM (void)
{ 
  unsigned long xfrac;
  unsigned long yfrac;
  short* dest;
  int count;
  int spot;
  short* tempcolormap;
         
#ifdef DEVELOPERS 
  if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    I_Error( "R_DrawSpan16_CVersion: %i to %i at %i", ds_x1,ds_x2,ds_y);
#endif 

  xfrac = ds_xfrac; 
  yfrac = ds_yfrac;
         
  dest = (short *)(ylookup[ds_y] + columnofs[ds_x1]);

  // We do not check for zero spans here?
  count = ds_x2 - ds_x1;

  if (((ds_xstep&0xffff) > FRACUNIT) && ((ds_ystep&0xffff) > FRACUNIT))
  {
    tempcolormap=(short *)ds_colormap;
    do
    {
      // Current texture index in u,v.
      spot = ((yfrac>>(FRACBITS-6))&(63*64)) + ((xfrac>>FRACBITS)&63);
  
      // Lookup pixel from flat texture tile, re-index using light/colormap.
      *dest++ = tempcolormap[ds_source[spot]];
  
      // Next step in u,v.
      xfrac += ds_xstep;
      yfrac += ds_ystep;
        
    }
    while (count--);
  } else {
    unsigned long spot[4];
    unsigned long level[4];
    unsigned long c;    // current colour
    int i;

    do
    {
      spot[0] =((yfrac>>16)&63)*64 + ((xfrac>>16)&63);
      spot[1] =((yfrac>>16)&63)*64 + (((xfrac>>16)+1)&63);
      spot[2] =(((yfrac>>16)+1)&63)*64 + ((xfrac>>16)&63);
      spot[3] =(((yfrac>>16)+1)&63)*64 + (((xfrac>>16)+1)&63);
  
      level[3] = (yfrac&0xffff)* (xfrac&0xffff);
      level[1] = (FRACUNIT - (yfrac&0xffff)-1) * (xfrac&0xffff);
      level[2] = (yfrac&0xffff) * (FRACUNIT - (xfrac&0xffff)-1);
      level[0] = (FRACUNIT - (yfrac&0xffff)) * (FRACUNIT - (xfrac&0xffff)-1);

      for (i = 0, c = 0; i < 4; i++)
      {
        level[i] >>= 26;
        spot[i]=(unsigned long) (((unsigned short*)ds_colormap)[ds_source[spot[i]]]);
        c += col2rgb16[level[i]][spot[i]&0xff][0] + col2rgb16[level[i]][(spot[i]>>8)&0xff][1];
      }
      c &= hicolortransmask3;
      c |= c >> 16;
      c >>= hicolortransshift;

      *dest++ = (short)c;

      // Next step in u,v.
      xfrac += ds_xstep;
      yfrac += ds_ystep;
    }
    while (count--);
  }
} 

//
// R_InitBuffer16
//
// Creates lookup tables that avoid multiplies and other hazzles
// for getting the framebuffer address of a pixel to draw.
//
void R_InitBuffer16 (int width, int height)
{ 
  int i;

  // Handle resize, e.g. smaller view windows with border and/or status bar.
  viewwindowx = (SCREENWIDTH-width) >> 1;

  // Column offset. For windows.
  for (i=0 ; i<width ; i++)
    columnofs[i] = (viewwindowx*2) + i*2;

  if (width == SCREENWIDTH)
    viewwindowy = 0;
  else
    viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1;

  // Precalculate all row offsets.
  for (i=0 ; i<height ; i++)
    ylookup[i] = screens[0] + (i+viewwindowy)*SCREENWIDTH*2;
} 

//
// R_FillBackScreen16
//
// Fills the back screen with a pattern for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen16 (void)
{ 
  byte* src;
  byte* tempsrc;
  short* dest;
  int x;
  int y;
  patch_t* patch;

  if ((scaledviewwidth == SCREENWIDTH)&&(viewheight==SCREENHEIGHT))
    return;
        
  src = W_CacheLumpName (currentmap->surround, PU_CACHE);
  dest = (short *)(screens[1]);
         
  for (y=0 ; y<SCREENHEIGHT; y++)
  {
    tempsrc=src+((y&63)<<6);
    for (x=0;x<SCREENWIDTH;x++)
    {
      *dest=palette_color[tempsrc[x&63]];
      dest++;
    }
  }

  if (SCREENWIDTH==scaledviewwidth)
    return;

  if ((viewwindowy-8)>=0)
  {
    patch = W_CacheLumpName ("brdr_t",PU_CACHE);

    for (x=0 ; x<scaledviewwidth ; x+=8)
      V_DrawPatch (viewwindowx+x,viewwindowy-8,1,patch);
  }

  if ((viewwindowy+viewheight+8)<(SCREENHEIGHT-SBARHEIGHT))
  {
    patch = W_CacheLumpName ("brdr_b",PU_CACHE);

    for (x=0 ; x<scaledviewwidth ; x+=8)
      V_DrawPatch (viewwindowx+x,viewwindowy+viewheight,1,patch);
  }

  if ((viewwindowx-8)>=0)
  {
    patch = W_CacheLumpName ("brdr_l",PU_CACHE);

    for (y=0 ; y<viewheight ; y+=8)
      V_DrawPatch (viewwindowx-8,viewwindowy+y,1,patch);
  }

  if ((viewwindowx+scaledviewwidth+8)<SCREENWIDTH)
  {
    patch = W_CacheLumpName ("brdr_r",PU_CACHE);

    for (y=0; y<viewheight; y+=8)
      V_DrawPatch (viewwindowx+scaledviewwidth,viewwindowy+y,1,patch);
  }

  // Draw beveled edge.
  if (((viewwindowx-8)>=0)&&((viewwindowy-8)>=0))
  {
    V_DrawPatch (viewwindowx-8,
                 viewwindowy-8,
                 1,
                 W_CacheLumpName ("brdr_tl",PU_CACHE));
  }

  if (((viewwindowx+scaledviewwidth+8)<SCREENWIDTH)&&((viewwindowy-8)>=0))
  {
    V_DrawPatch (viewwindowx+scaledviewwidth,
                 viewwindowy-8,
                 1,
                 W_CacheLumpName ("brdr_tr",PU_CACHE));
  }

  if (((viewwindowx-8)>=0)&&((viewwindowy+viewheight+8)<(SCREENHEIGHT-SBARHEIGHT)))
  {
    V_DrawPatch (viewwindowx-8,
                 viewwindowy+viewheight,
                 1,
                 W_CacheLumpName ("brdr_bl",PU_CACHE));
  }

  if (((viewwindowx+scaledviewwidth+8)<SCREENWIDTH)&&
         ((viewwindowy+viewheight+8)<(SCREENHEIGHT-SBARHEIGHT)))
  {
    V_DrawPatch (viewwindowx+scaledviewwidth,
                 viewwindowy+viewheight,
                 1,
                 W_CacheLumpName ("brdr_br",PU_CACHE));
  }
}

//
// Copy a screen buffer.
//
void R_VideoErase16 (unsigned ofs, int count)
{ 
  // LFB copy.
  // This might not be a good idea if memcpy
  //  is not optiomal, e.g. byte by byte on
  //  a 32bit CPU, as GNU GCC/Linux libc did
  //  at one point.
    memcpy (screens[0]+ofs*2, screens[1]+ofs*2, count*2);
} 


//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
 
void R_DrawViewBorder16 (void)
{ 
    int         top;
    int         side,side2;
    int         ofs;
    int         i; 

    //if screenwidth>320, draw stuff around status bar, even if scaledviewwidth==SCREENWIDTH
    if ((SCREENWIDTH>320)&&(SCREENHEIGHT!=viewheight))
      {
      ofs=(SCREENHEIGHT-SBARHEIGHT)*SCREENWIDTH;
      side=(SCREENWIDTH-320)/2; side2=side*2;
      R_VideoErase16(ofs,side);

      ofs+=(SCREENWIDTH-side);
      for (i=1;i<SBARHEIGHT;i++)
        {
        R_VideoErase16(ofs,side2);
        ofs+=SCREENWIDTH;
        }
      R_VideoErase16(ofs,side);
      }

//    if (scaledviewwidth == SCREENWIDTH)
    if (viewheight>=(SCREENHEIGHT-SBARHEIGHT))
      return;
  
    top = ((SCREENHEIGHT-SBARHEIGHT)-viewheight)/2; 
    side = (SCREENWIDTH-scaledviewwidth)/2; 
 
    // copy top and one line of left side 
    R_VideoErase16 (0, top*SCREENWIDTH+side);
 
    // copy one line of right side and bottom 
    ofs = (viewheight+top)*SCREENWIDTH-side; 
    R_VideoErase16 (ofs, top*SCREENWIDTH+side);
 
    // copy sides using wraparound 
    ofs = top*SCREENWIDTH + SCREENWIDTH-side; 
    side <<= 1;
    
    for (i=1 ; i<viewheight ; i++) 
    { 
        R_VideoErase16 (ofs, side);
        ofs += SCREENWIDTH; 
    }

    // ? 
    V_MarkRect (0,0,SCREENWIDTH, SCREENHEIGHT-SBARHEIGHT); 
} 
 
 
