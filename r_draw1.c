//
// DOSDoom Column/Span Drawing for 8-bit Colour Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -ACB- 1998/09/10 Cleaned up.
//

#include "dm_defs.h"
#include "dm_state.h"
#include "i_system.h"
#include "r_draw1.h"  // -ES- 1998/08/05 Include Header
#include "r_local.h"
#include "v_res.h"
#include "w_wad.h"
#include "z_zone.h"

#define SBARHEIGHT 32

long long mmxcomm __attribute__ ((aligned(8))); // communication betw. int and mmx regs

//
// All drawing to the view buffer is accomplished in this file.
//
// The other refresh files only know about ccordinates, not the
// architecture of the frame buffer.
//
// Conveniently, the frame buffer is a linear one, and we need
// only the base address, and the total size == width*height*depth/8.
//
byte* viewimage;
int viewwidth;
int viewheight;
int viewwindowx;
int viewwindowy;
int scaledviewwidth;

// just for profiling
int dccount;

void resinit_r_draw_c8(void)
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
  set_screenwidth(SCREENWIDTH,  "rdc8iwidth1");
  set_screenwidth(2*SCREENWIDTH,  "rdc8iwidth2");
  set_screenwidth(SCREENWIDTH,  "rdc8ewidth1");
  set_screenwidth(2*SCREENWIDTH,  "rdc8ewidth2");
  set_screenwidth(SCREENWIDTH,  "rdc8nwidth1");
  set_screenwidth(SCREENWIDTH,  "rdc8nwidth2");
  set_screenwidth(SCREENWIDTH,  "rdc8mwidth1");
  set_screenwidth(2*SCREENWIDTH,"rdc8mwidth2");
  set_screenwidth(-2*SCREENWIDTH,"rdc8mwidth3");
  set_screenwidth(2*SCREENWIDTH,"rdc8mwidth4");
  set_screenwidth(-SCREENWIDTH, "rdc8mwidth5");
  set_screenwidth(2*SCREENWIDTH, "rdc8pwidth1");
  set_screenwidth(3*SCREENWIDTH, "rdc8pwidth2");
  set_screenwidth(2*SCREENWIDTH, "rdc8pwidth3");
  set_screenwidth(2*SCREENWIDTH, "rdc8pwidth4");
  set_screenwidth(-2*SCREENWIDTH, "rdc8pwidth5");
  set_screenwidth(-SCREENWIDTH, "rdc8pwidth6");
#undef set_screenwidth

}

//
// R_DrawColumn8_CVersion
//
// A column is a vertical slice/span from a wall texture that,
// given the DOOM style restrictions on the view orientation,
// will always have constant z depth.
//
// Thus a special case loop for very fast rendering can
// be used. It has also been used with Wolfenstein 3D.
//
void R_DrawColumn8_CVersion (void)
{ 
  int count;
  byte* dest;
  fixed_t frac;
  fixed_t fracstep;
 
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
  dest = ylookup[dc_yl] + columnofs[dc_x];

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  do
  {
    // Re-map color indices from wall texture column
    // using a lighting/special effects LUT.
    *dest = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
        
    dest += SCREENWIDTH;
    frac += fracstep;
        
  }
  while (--count);
} 

// -ES- 1998/12/18 Converted this to 8-bit
void R_DrawColumn8_KM ()
{
#ifndef SMOOTHING
  R_DrawColumn8_CVersion();
#else
  int count;
  char* dest;
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
  dest = ylookup[dc_yl] + columnofs[dc_x];

  // Determine scaling, which is the only mapping to be done.
  fracstep = (dc_iscale);
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Inner loop that does the actual texture mapping, e.g. a DDA-lile scaling.
  if (fracstep > 0x12000)
  {
    do
    {
      // Re-map color indices from wall texture column
      // using a lighting/special effects LUT.
      *dest = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
        
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
      level[3] = (frac&0xffff)            * dc_xfrac;
      level[2] = (FRACUNIT - (frac&0xffff))* dc_xfrac;
      level[1] = (frac&0xffff)             * (FRACUNIT - dc_xfrac-1);
      level[0] = (FRACUNIT - (frac&0xffff))* (FRACUNIT - dc_xfrac-1);
      spot[0] = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
      spot[1] = dc_colormap[dc_source[((frac>>FRACBITS)+1)&127]];
      spot[2] = dc_colormap[dc_source2[(frac>>FRACBITS)&127]];
      spot[3] = dc_colormap[dc_source2[((frac>>FRACBITS)+1)&127]];

      for (i = 0, c = 0; i < 4; i++)
      {
        c += col2rgb8[level[i]>>26][spot[i]];
      }
      c |= 0xF07C3E1F;
      *dest = rgb_8k[0][0][(c>>5) & (c>>19)];

      dest += SCREENWIDTH;
      frac += fracstep;
    } while (--count);
  }
#endif
};

//
// R_DrawFuzzColumn8
//
// Spectre/Invisibility.
//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels from adjacent ones to left and right.
//
// Used with an all black colormap, this creates the SHADOW effect,
// i.e. spectres and invisible players.
//

void R_DrawFuzzColumn8 (void)
{ 
  int count;
  byte* dest;
  fixed_t frac;
  fixed_t fracstep;

  // Adjust borders. Low...
  if (!dc_yl)
    dc_yl = 1;

  // .. and high.
  if (dc_yh == viewheight-1)
    dc_yh = viewheight-2;
                 
  count = dc_yh - dc_yl;

  // Zero length.
  if (count < 0)
    return;
    
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ("R_DrawFuzzColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

  // Does not work with blocky mode.
  dest = ylookup[dc_yl] + columnofs[dc_x];

  // Looks familiar.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  //
  // Looks like an attempt at dithering, using the colormap #6 (of 0-31, a bit
  // brighter than average).
  //
  do
  {
    //
    // Lookup framebuffer, and retrieve a pixel that is either one column
    // left or right of the current one. Add index from colormap to index.
    //
    *dest = colormaps[6*256+dest[fuzzoffset[fuzzpos]]];

    // Clamp table lookup index.
    if (++fuzzpos == FUZZTABLE)
      fuzzpos = 0;
        
    dest += SCREENWIDTH;

    frac += fracstep;
  }
  while (count--);
} 


// -ES- 1998/10/29 New translucency
//
//
// -KM- 1998/11/25 Modified rest of DOSDoom to work with this.
void R_DrawTranslucentColumn8 ()
{ 
  int count;
  byte *dest;
  fixed_t frac;
  fixed_t fracstep;

  fixed_t fglevel, bglevel;
  unsigned long *fg2rgb, *bg2rgb;
  unsigned long fg,bg;  // current colours

  fglevel = dc_translucency;
  fglevel = fglevel&~0x3ff;
  bglevel = FRACUNIT-fglevel;

  fg2rgb = col2rgb8[fglevel>>10];
  bg2rgb = col2rgb8[bglevel>>10];

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
  dest = ylookup[dc_yl] + columnofs[dc_x];

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  //
  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  //
  do
  {
    /*

    New translucency algorithm, by Erik Sandberg:

    Basically, we compute the red, green and blue values for each pixel, and
    then use a RGB table to check which one of the palette colours that best
    represents those RGB values. The RGB table is 8k big, with 4 R-bits,
    5 G-bits and 4 B-bits. A 4k table gives a bit too bad precision, and a 32k
    table takes up more memory and results in more cache misses, so an 8k
    table seemed to be quite ultimate.

    The computation of the RGB for each pixel is accelerated by using two
    1k tables for each translucency level.
    The xth element of one of these tables contains the r, g and b values for
    the colour x, weighted for the current translucency level (for example,
    the weighted rgb values for background colour at 75% translucency are 1/4
    of the original rgb values). The rgb values are stored as three
    low-precision fixed point values, packed into one long per colour:
    Bit 0-4:   Frac part of blue  (5 bits)
    Bit 5-8:   Int  part of blue  (4 bits)
    Bit 9-13:  Frac part of red   (5 bits)
    Bit 14-17: Int  part of red   (4 bits)
    Bit 18-22: Frac part of green (5 bits)
    Bit 23-27: Int  part of green (5 bits)
    Bit 28-31: All zeros          (4 bits)

    The point of this format is that the two colours now can be added, and
    then be converted to a RGB table index very easily: First, we just set
    all the frac bits and the four upper zero bits to 1. It's now possible
    to get the RGB table index by anding the current value >> 5 with the
    current value >> 19. When asm-optimised, this should be the fastest
    algorithm that uses RGB tables.

    */

    fg = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
    bg = *dest;

    fg=fg2rgb[fg];
    bg=bg2rgb[bg];
    fg = (fg+bg) | 0xF07C3E1F;
    *dest = rgb_8k[0][0][(fg>>5) & (fg>>19)];

    dest+=SCREENWIDTH;
    frac+=fracstep;
  }
  while (--count);
} 


//
// R_DrawTranslatedColumn8
//
// Uses the translationtables to remap one set of palette colours to
// another. One prime example is the player greens to the other player
// colours.
//
// Could be used with different translation tables, e.g. the lighter colored
// version of the BaronOfHell, the HellKnight, uses identical sprites,
// kinda brightened up.
//
// 8-Bit Colour Version.
//
void R_DrawTranslatedColumn8 (void)
{ 
  int count;
  byte* dest;
  fixed_t frac;
  fixed_t fracstep;
 
  count = dc_yh - dc_yl;

  if (count < 0)
    return;
                                 
#ifdef DEVELOPERS 
  if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    I_Error ( "R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif 

  dest = ylookup[dc_yl] + columnofs[dc_x];

  // Looks familiar.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  // Here we do an additional index re-mapping.
  do
  {
    //
    // Translation tables are used to map certain colorramps to other ones,
    // used with PLAY sprites. Thus the "green" ramp of the player 0 sprite
    // is mapped to gray, red, black/indigo.
    //
    *dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
    dest += SCREENWIDTH;
        
    frac += fracstep;
  }
  while (count--);
} 

void R_DrawTranslucentTranslatedColumn8 ()
{ 
  int count;
  byte* dest;
  fixed_t frac;
  fixed_t fracstep;
 
  fixed_t fglevel, bglevel;
  unsigned long *fg2rgb, *bg2rgb;
  unsigned long fg,bg;  // current colours

  fglevel = dc_translucency;
  fglevel = fglevel&~0x3ff;
  bglevel = FRACUNIT-fglevel;

  fg2rgb = col2rgb8[fglevel>>10];
  bg2rgb = col2rgb8[bglevel>>10];

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
  dest = ylookup[dc_yl] + columnofs[dc_x];

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl-centery)*fracstep;

  //
  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  //
  do
  {
    //
    // This is R_DrawTranslucentColumn with an extra translation table lookup
    //
    fg = dc_colormap[dc_translation[dc_source[(frac>>FRACBITS)&127]]];
    bg = *dest;

    fg=fg2rgb[fg];
    bg=bg2rgb[bg];
    fg = (fg+bg) | 0xF07C3E1F;
    *dest = rgb_8k[0][0][(fg>>5) & (fg>>19)];
        
    dest+=SCREENWIDTH;
    frac+=fracstep;
        
  }
  while (--count);
} 

//
// R_InitTranslationTables8
//
// Reads the PALREMAP lump for information on translating one
// colour on the palette to another. The PALREMAP is broken down
// into 64-byte sections(lumps). Each section has 32 remaps; a remap
// consists of one byte that is the number of the colour to be
// replaced, the second byte contains the number of the replacement
// colour.
//
// The prime example of PALREMAP in action is the player sprites,
// in order to tell your fellow players apart, the remapping function
// changes the greens (numbers: 112->128) into another colour.
//
// 8-Bit Colour Version.
//
// -ACB- 1998/09/10 Replaced the old procedure with this.
//
#define REMAPLENGTH 64
#define REMAPSPERLUMP 32

// -ES- 1998/11/08 Fixed memory leak
static byte *translationtables8 = NULL;
void R_InitTranslationTables8 (void)
{
  int i;
  int j;
  int lump;
  int length;
  int lump_p;
  byte* lumpdata;

  if (!translationtables8)
  {
    // Load in the light tables, 256 byte align tables.
    lump = W_GetNumForName("PALREMAP");
    length = W_LumpLength (lump);
    lumpdata = Z_Malloc(length*sizeof(byte), PU_STATIC, 0);
    W_ReadLump(lump, lumpdata);
    translationtables8 = Z_Malloc (256*(length/REMAPLENGTH)*2+255, PU_STATIC, 0);
    translationtables8 = (byte *)(( (int)translationtables8 + 255 )& ~255);

    lump_p = 0;

    for (i=0; i<((length/REMAPLENGTH)*256); i+=256)
    {
      for (j=0; j<256; j++)
        translationtables8[i+j] = j;

      for (j=0; j<REMAPSPERLUMP; j++, lump_p+=2)
        translationtables8[i+lumpdata[lump_p]]=translationtables8[i+lumpdata[lump_p+1]];
    }
    Z_Free(lumpdata);
  }
  translationtables = translationtables8;
}

//
// R_DrawSpan8_CVersion
//
// With DOOM style restrictions on view orientation, the floors and ceilings
// consist of horizontal slices or spans with constant z depth.
//
// However, rotation around the world z axis is possible, thus this mapping,
// while simpler and faster than perspective correct texture mapping, has to
// traverse the texture at an angle in all but a few cases.
//
// In consequence, flats are not stored by column (like walls), and the inner
// loop has to step in texture space u and v.
//
// Draws the actual span.
//
void R_DrawSpan8_CVersion (void)
{ 
  fixed_t xfrac;
  fixed_t yfrac;
  byte* dest;
  int count;
  int spot;
         
#ifdef DEVELOPERS 
  if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    I_Error( "R_DrawSpan: %i to %i at %i",ds_x1,ds_x2,ds_y);
#endif 

  xfrac = ds_xfrac;
  yfrac = ds_yfrac;
         
  dest = ylookup[ds_y] + columnofs[ds_x1];

  // We do not check for zero spans here?
  count = ds_x2 - ds_x1;

  do
  {
    // Current texture index in u,v.
    spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

    // Lookup pixel from flat texture tile,
    // re-index using light/colormap.
    *dest++ = ds_colormap[ds_source[spot]];

    // Next step in u,v.
    xfrac += ds_xstep;
    yfrac += ds_ystep;  
  }
  while (count--);
}

// -ES- 1998/12/18 Added this one
void R_DrawSpan8_KM (void)
{
  unsigned long xfrac;
  unsigned long yfrac;
  unsigned char *dest;
  int count;
  int spot;
         
#ifdef DEVELOPERS 
  if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    I_Error( "R_DrawSpan16_CVersion: %i to %i at %i", ds_x1,ds_x2,ds_y);
#endif 

  xfrac = ds_xfrac; 
  yfrac = ds_yfrac;
         
  dest = ylookup[ds_y] + columnofs[ds_x1];

  // We do not check for zero spans here?
  count = ds_x2 - ds_x1;

  if (((ds_xstep&0xffff) > FRACUNIT) && ((ds_ystep&0xffff) > FRACUNIT))
  {
    do
    {
      // Current texture index in u,v.
      spot = ((yfrac>>(FRACBITS-6))&(63*64)) + ((xfrac>>FRACBITS)&63);
  
      // Lookup pixel from flat texture tile, re-index using light/colormap.
      *dest++ = dc_colormap[ds_source[spot]];
  
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
  
      level[3] = ((yfrac&0xffff) * (xfrac&0xffff));
      level[1] = (FRACUNIT - (yfrac&0xffff)-1) * (xfrac&0xffff);
      level[2] = (yfrac&0xffff) * (FRACUNIT - (xfrac&0xffff)-1);
      level[0] = (FRACUNIT - (yfrac&0xffff)-1) * (FRACUNIT - (xfrac&0xffff)-1);

      for (i = 0, c = 0; i < 4; i++)
      {
        spot[i]=(unsigned long) ((ds_colormap)[ds_source[spot[i]]]);
        c += col2rgb8[level[i]>>26][spot[i]];
      }
      c |= 0xF07C3E1F;
      *dest++ = rgb_8k[0][0][(c>>5) & (c>>19)];

      // Next step in u,v.
      xfrac += ds_xstep;
      yfrac += ds_ystep;
    }
    while (count--);
  }

}

//
// R_InitBuffer8
//
// Creats lookup tables that avoid multiplies and other hazzles
// for getting the framebuffer address of a pixel to draw.
//
void R_InitBuffer8 (int width, int height)
{ 
  int i;

  // Handle resize, e.g. smaller view windows with border and/or status bar.
  viewwindowx = (SCREENWIDTH-width) >> 1;

  // Column offset. For windows.
  for (i=0 ; i<width ; i++)
    columnofs[i] = viewwindowx + i;

  // Samw with base row offset.
  if (width == SCREENWIDTH)
    viewwindowy = 0;
  else
    viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1;

  // Precalculate all row offsets.
  for (i=0; i<height; i++)
    ylookup[i] = screens[0] + (i+viewwindowy)*SCREENWIDTH;
} 

//
// R_FillBackScreen8
//
// Fills the back screen with a pattern for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen8 (void)
{ 
  byte* src;
  byte* dest;
  int x;
  int y;
  patch_t* patch;

  if ((scaledviewwidth == SCREENWIDTH)&&(viewheight==SCREENHEIGHT))
    return;

  src = W_CacheLumpName (currentmap->surround, PU_CACHE);
  dest = screens[1];
         
  for (y=0 ; y<SCREENHEIGHT; y++)
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

    for (y=0 ; y<viewheight ; y+=8)
      V_DrawPatch (viewwindowx+scaledviewwidth,viewwindowy+y,1,patch);
  }

  // Draw beveled edge.
  if (((viewwindowx-8)>=0)&&((viewwindowy-8)>=0))
  {
    V_DrawPatch (viewwindowx-8, viewwindowy-8, 1,
                   W_CacheLumpName ("brdr_tl",PU_CACHE));
  }

  if (((viewwindowx+scaledviewwidth+8)<SCREENWIDTH)&&((viewwindowy-8)>=0))
  {
    V_DrawPatch (viewwindowx+scaledviewwidth, viewwindowy-8, 1,
                   W_CacheLumpName ("brdr_tr",PU_CACHE));
  }

  if (((viewwindowx-8)>=0)&&((viewwindowy+viewheight+8)<(SCREENHEIGHT-SBARHEIGHT)))
  {
    V_DrawPatch (viewwindowx-8, viewwindowy+viewheight, 1,
                   W_CacheLumpName ("brdr_bl",PU_CACHE));
  }

  if (((viewwindowx+scaledviewwidth+8)<SCREENWIDTH)&&
          ((viewwindowy+viewheight+8)<(SCREENHEIGHT-SBARHEIGHT)))
  {
    V_DrawPatch (viewwindowx+scaledviewwidth, viewwindowy+viewheight, 1,
                   W_CacheLumpName ("brdr_br",PU_CACHE));
  }
}
 

//
// R_VideoErase8
//
// Linear Frame Buffer copy.
//
// This might not be a good idea if memcpy is not optiomal, e.g. byte by byte on
// a 32bit CPU, as GNU GCC/Linux libc did at one point.
//
void R_VideoErase8 (unsigned ofs, int count)
{
  memcpy (screens[0]+ofs, screens[1]+ofs, count);
} 

//
// R_DrawViewBorder8
//
// Draws the border around the view for different size windows?
// 
void R_DrawViewBorder8 (void)
{ 
  int top;
  int side, side2;
  int ofs;
  int i;

  //
  // if screenwidth>320, draw stuff around status bar, even if
  // scaledviewwidth==SCREENWIDTH
  //
  if ((SCREENWIDTH>320)&&(SCREENHEIGHT!=viewheight))
  {
    ofs=(SCREENHEIGHT-SBARHEIGHT)*SCREENWIDTH;
    side=(SCREENWIDTH-320)/2; side2=side*2;
    R_VideoErase8(ofs,side);

    ofs+=(SCREENWIDTH-side);

    for (i=1;i<SBARHEIGHT;i++)
    {
      R_VideoErase8(ofs,side2);
      ofs+=SCREENWIDTH;
    }

    R_VideoErase8(ofs,side);
  }

  if (viewheight>=(SCREENHEIGHT-SBARHEIGHT))
    return;
  
  top = ((SCREENHEIGHT-SBARHEIGHT)-viewheight)/2;
  side = (SCREENWIDTH-scaledviewwidth)/2;
 
  // copy top and one line of left side
  R_VideoErase8 (0, top*SCREENWIDTH+side);
 
  // copy one line of right side and bottom
  ofs = (viewheight+top)*SCREENWIDTH-side;
  R_VideoErase8 (ofs, top*SCREENWIDTH+side);
 
  // copy sides using wraparound
  ofs = top*SCREENWIDTH + SCREENWIDTH-side;
  side <<= 1;
    
  for (i=1; i<viewheight; i++)
  {
    R_VideoErase8 (ofs, side);
    ofs += SCREENWIDTH;
  }

  V_MarkRect (0,0,SCREENWIDTH, SCREENHEIGHT-SBARHEIGHT);
} 
 
 
