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

// -ES- 1999/02/12 Converted BLF stuff to 8-bit
#define BLFshift 2 // Detail level. High detail levels look better, but
                   // consume more memory. For each detail increase by one,
                   // the BLF table needs approx. four times as much memory.
#define BLFsz (1<<BLFshift)
#define BLFmax (BLFsz-1)

typedef unsigned long BLF8LUT; // Table with 256 longs containing translucency table-style fix-point RGBs.

static BLF8LUT *BLFTab[BLFsz][BLFsz]; // Totally 8*8*256 BLF16LUTs

static void BLF_Init8 (void)
{
  static boolean firsttime = true;
  unsigned int i;

  unsigned long r,g,b,x,y,xy;

  unsigned char *thepalette;
  BLF8LUT *BLFBuf; // Array of all the used BLF16LUTs
  int BLFCheck[4*BLFsz*BLFsz]; // Stores which index to BLFBuf a certain x*y should use (x and y are 31.1 fix point)
  int count=0;

  if (!firsttime)
    return;
  firsttime = false;

  I_Printf("BLF_Init: Init Bilinear Filtering");

  // Init BLFCheck
  for (x=0; x<4*BLFsz*BLFsz; x++)
    BLFCheck[x]=-1;
  for (x=1; x<2*BLFsz; x+=2)
    for (y=1; y<2*BLFsz; y+=2)
      if (BLFCheck[x*y] == -1)
      {
        BLFCheck[x*y] = count;
        count++;
      }
  // Allocate the memory if it isn't already allocated. Use 32-byte alignment.
  BLFBuf = BLFTab[0][0];
  if (!BLFBuf)
  {
    BLFBuf = Z_Malloc(count*256*sizeof(BLF8LUT)+31, PU_STATIC, NULL); // allocate memory
    if (BLFBuf==NULL)
      I_Error("Out of memory!");
    BLFBuf = (BLF8LUT*) (((long)BLFBuf+31) & ~31); // align
  }

  for (x=0; x<BLFsz; x++)
    for (y=0; y<BLFsz; y++)
      BLFTab[x][y] = &BLFBuf[256*BLFCheck[(2*x+1) * (2*y+1)]];

  thepalette = W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
  for (xy=0; xy<4*BLFsz*BLFsz; xy++)
    if (BLFCheck[xy] != -1)
    {
      for (i=0; i<256; i++)
      {
        r = thepalette[i*3+0];
        g = thepalette[i*3+1];
        b = thepalette[i*3+2];
        r = (( r * xy ) << 1) >> (2 + 2*BLFshift);
        g = (( g * xy ) << 4) >> (2 + 2*BLFshift);
        b = (( b * xy ) << 3) >> (2 + 2*BLFshift);
        (&BLFBuf[256*BLFCheck[xy]])[i] = (r<<11) + (g << 20) + b;
      }
    }
  I_Printf("\n");
}

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

#ifdef DJGPP
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
#endif

  if (R_DrawSpan == R_DrawSpan8_BLF || R_DrawColumn == R_DrawColumn8_BLF)
    BLF_Init8();
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
  frac = dc_texturemid + dc_yl*fracstep;

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

// -ES- 1999-04-07 Mipmapping test
void R_DrawColumn8_MIP (void)
{ 
  int count;
  byte* dest;
  fixed_t frac;
  fixed_t fracstep;
  unsigned long mask;
  fixed_t i;
 
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
  frac = dc_texturemid + dc_yl*fracstep;

  mask = 0xffff0000;
  for (i=abs(fracstep); i>FRACUNIT; i>>=1)
    mask <<= 1;
  mask &= 127*FRACUNIT;
  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  do
  {
    // Re-map color indices from wall texture column
    // using a lighting/special effects LUT.
    *dest = dc_colormap[dc_source[((unsigned long)(frac&mask))>>FRACBITS]];
        
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
  frac = dc_texturemid + dc_yl*fracstep;

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

    dc_xfrac &= 0xffff;
    do
    {
      level[3] = (frac&0xffff)            * dc_xfrac;
      level[2] = (FRACUNIT - (frac&0xffff))* dc_xfrac;
      level[1] = (frac&0xffff)             * (FRACUNIT - dc_xfrac-1);
      level[0] = (FRACUNIT - (frac&0xffff))* (FRACUNIT - dc_xfrac-1);
      spot[0] = dc_source[(frac>>FRACBITS)&127];
      spot[1] = dc_source[((frac>>FRACBITS)+1)&127];
      spot[2] = dc_source2[(frac>>FRACBITS)&127];
      spot[3] = dc_source2[((frac>>FRACBITS)+1)&127];

      for (i = 0, c = 0; i < 4; i++)
      {
        c += col2rgb8[level[i]>>26][spot[i]];
      }

      // -ES- 1999/03/13 Fixed the RGB conversion
      c &= 0xF80F0780;
      *dest = dc_colormap[rgb_8k[0][0][(0x1FFF & (c>>7)) | (c>>23)]];

      dest += SCREENWIDTH;
      frac += fracstep;
    } while (--count);
  }
#endif
};

// -ES- 1999/03/29 Added this
void R_DrawColumn8_BLF ()
{
#ifndef SMOOTHING
  R_DrawColumn8_CVersion();
#else
  int count;
  char* dest;
  fixed_t yfrac;
  fixed_t ystep;
  unsigned long col1, col2, col3, col4;
  unsigned long x1,x2,y1,y2;

  if (dc_iscale > 0x20000)
    R_DrawColumn8_CVersion();

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
  ystep = dc_iscale;
  yfrac = dc_texturemid + dc_yl*ystep - FRACUNIT/2;

  dc_xfrac &= 0xffff;

  // x position is constant
  x1 = (dc_xfrac >> (16-BLFshift));
  x2 = BLFmax - x1;

  // Inner loop that does the actual texture mapping, e.g. a DDA-lile scaling.
  do
  {
    col1 = dc_source[  (yfrac>>FRACBITS)&127];
    col2 = dc_source2[ (yfrac>>FRACBITS)&127];
    col3 = dc_source[ ((yfrac>>FRACBITS)+1)&127];
    col4 = dc_source2[((yfrac>>FRACBITS)+1)&127];

    // Get the texture sub-coordinates
    y1 = (yfrac >> (16-BLFshift)) & BLFmax;
    y2 = BLFmax - y1;

    col1 = BLFTab[x2][y2][col1]
         + BLFTab[x1][y2][col2]
         + BLFTab[x2][y1][col3]
         + BLFTab[x1][y1][col4];

    // Convert to usable RGB
    col1 &= 0xF80F0780;

    // Store pixel
    *dest = dc_colormap[rgb_8k[0][0][(0x1FFF & (col1>>7)) | (col1>>23)]];

    dest += SCREENWIDTH;
    yfrac += ystep;
  } while (--count);
#endif
};

// -KM- 1999/01/31 Low resolution Column drawer
void R_DrawColumn8_LowRes (void)
{ 
  int count;
  unsigned short* dest;
  unsigned short pixel;
  fixed_t frac;
  fixed_t fracstep;
 
  // Only render even columns
  if (dc_x & 1)
    return;

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
  dest = (unsigned short *) (ylookup[dc_yl] + columnofs[dc_x]);

  // Determine scaling, which is the only mapping to be done.
  fracstep = dc_iscale;
  frac = dc_texturemid + (dc_yl)*fracstep;

  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  if (count & 1)
  {
    pixel = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
    pixel |= pixel << 8;

    *dest = pixel;
          
    dest += SCREENWIDTH>>1;
    frac += fracstep;
    if (--count == 0)
      return;
  }
  fracstep <<= 1;
  do
  {
    // Re-map color indices from wall texture column
    // using a lighting/special effects LUT.
    pixel = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
    pixel |= pixel << 8;

    *dest = pixel;
    dest += SCREENWIDTH>>1;
    *dest = pixel;
    dest += SCREENWIDTH>>1;

    frac += fracstep;
  } while ((count-=2) > 0);
} 

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
  frac = dc_texturemid + dc_yl*fracstep;

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
// -KM- 1998/11/25 Modified rest of DOSDoom to work with this.
//
// -ES- 1998/02/12 New col2rgb format
void R_DrawTranslucentColumn8 ()
{ 
  int count;
  byte *dest;
  fixed_t frac;
  fixed_t fracstep;

  fixed_t fglevel, bglevel;
  unsigned long *fg2rgb, *bg2rgb;
  unsigned long fg;  // current colours

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
  frac = dc_texturemid + dc_yl*fracstep;

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

    The point of this format is that the two colours now can be added, and
    then be converted to a RGB table index very easily: First, we just set
    all the frac bits and the four upper zero bits to 1. It's now possible
    to get the RGB table index by ORing the current value >> 7 with the
    current value >> 23, and then mask away some high bits by anding it
    with 0x1FFF.

    */

    fg = (fg2rgb[dc_colormap[dc_source[(frac>>FRACBITS)&127]]] +
          bg2rgb[*dest]) & 0xF80F0780;

    *dest = rgb_8k[0][0][(0x1FFF & (fg>>7)) | (fg>>23)];

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
  frac = dc_texturemid + dc_yl*fracstep;

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

// -ES- 1998/02/12 New col2rgb format
void R_DrawTranslucentTranslatedColumn8 ()
{ 
  int count;
  byte* dest;
  fixed_t frac;
  fixed_t fracstep;
 
  fixed_t fglevel, bglevel;
  unsigned long *fg2rgb, *bg2rgb;
  unsigned long fg;  // current colours

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
  frac = dc_texturemid + dc_yl*fracstep;

  //
  // Inner loop that does the actual texture mapping,
  // e.g. a DDA-lile scaling. This is as fast as it gets.
  //
  do
  {
    //
    // This is R_DrawTranslucentColumn with an extra translation table lookup
    //
    fg = (fg2rgb[dc_colormap[dc_translation[dc_source[(frac>>FRACBITS)&127]]]] +
          bg2rgb[*dest]) & 0xF80F0780;

    *dest = rgb_8k[0][0][(0x1FFF & (fg>>7)) | (fg>>23)];
        
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

// -ES- 1999-04-07 Mipmapping test
void R_DrawSpan8_MIP (void)
{ 
  fixed_t xfrac;
  fixed_t yfrac;
  byte* dest;
  int count;
  int spot;

  unsigned long xmask, ymask;
  unsigned long i;
       
#ifdef DEVELOPERS 
  if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    I_Error( "R_DrawSpan: %i to %i at %i",ds_x1,ds_x2,ds_y);
#endif 

  xfrac = ds_xfrac;
  yfrac = ds_yfrac;
         
  dest = ylookup[ds_y] + columnofs[ds_x1];

  // We do not check for zero spans here?
  count = ds_x2 - ds_x1;

  xmask = 0xffff0000;
  for (i=abs(ds_xstep); i>FRACUNIT; i>>=1)
    xmask <<= 1;
  xmask &= 63*FRACUNIT;
  ymask = 0xffff0000;
  for (i=abs(ds_ystep); i>FRACUNIT; i>>=1)
    ymask <<= 1;
  ymask &= 63*FRACUNIT;
  do
  {
    // Current texture index in u,v.
    spot = ((yfrac&ymask)>>(16-6)) + ((xfrac&xmask)>>16);

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
        spot[i]=(unsigned long)(ds_source[spot[i]]);
        c += col2rgb8[level[i]>>26][spot[i]];
      }

      // -ES- 1999/03/13 Fixed the RGB conversion
      c &= 0xF80F0780;
      *dest++ = ds_colormap[rgb_8k[0][0][(0x1FFF & (c>>7)) | (c>>23)]];

      // Next step in u,v.
      xfrac += ds_xstep;
      yfrac += ds_ystep;
    }
    while (count--);
  }
}

//------------------------------------------------------------
// Bilinear Filtering
// 16-bit version originally written by Vitek Kavan vit.kavan@usa.net
// Improved & converted to 8-bit colour by -ES- 1999/02/12

void R_DrawSpan8_BLF (void)
{
    int                 count;
    unsigned long col1, col2, col3, col4;
    unsigned long x1,x2,y1,y2;
    unsigned char* dest = (ylookup[ds_y] + columnofs[ds_x1]);
    unsigned long xfrac = ds_xfrac - FRACUNIT/2;
    unsigned long yfrac = ds_yfrac - FRACUNIT/2;

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
      // Get the texture coordinates
      y1 = ((yfrac>>10)&(63*64));
      x1 = ((xfrac>>16)&63);
      y2 = (y1+64)&(63*64);
      x2 = (x1+1)&63;

      // Get the colours of the four corners
      col1 = ds_source[y1+x1];
      col2 = ds_source[y1+x2];
      col3 = ds_source[y2+x1];
      col4 = ds_source[y2+x2];

      // Get the texture sub-coordinates
      x1 = (xfrac >> (16-BLFshift)) & BLFmax;
      y1 = (yfrac >> (16-BLFshift)) & BLFmax;
      x2 = BLFmax - x1;
      y2 = BLFmax - y1;

      // Get the fixed-point RGB value
      col1 = BLFTab[x2][y2][col1]
           + BLFTab[x1][y2][col2]
           + BLFTab[x2][y1][col3]
           + BLFTab[x1][y1][col4];

      // Convert to usable RGB
      col1 &= 0xF80F0780;

      // Store pixel
      *dest++ = ds_colormap[rgb_8k[0][0][(0x1FFF & (col1>>7)) | (col1>>23)]];

      // Next step
      xfrac += ds_xstep;
      yfrac += ds_ystep;
    } while (count--);
}

// -KM- 1999/01/31 Low resolution span drawer for 386 etc
void R_DrawSpan8_LowRes (void)
{ 
  fixed_t xfrac;
  fixed_t yfrac;
  unsigned short* dest;
  unsigned short* dest2;
  unsigned short pixel;
  int count;
  int spot;
         
  // Render even rows only
  if (ds_y & 1)
    return;

#ifdef DEVELOPERS
  if (ds_x2 < ds_x1 || ds_x1<0 || ds_x2>=SCREENWIDTH || (unsigned)ds_y>SCREENHEIGHT)
    I_Error( "R_DrawSpan: %i to %i at %i",ds_x1,ds_x2,ds_y);
#endif 

  xfrac = ds_xfrac;
  yfrac = ds_yfrac;
         
  dest = (unsigned short *) (ylookup[ds_y] + columnofs[ds_x1]);
  dest2 = dest + (SCREENWIDTH>>1);

  // We do not check for zero spans here?
  count = ds_x2 - ds_x1;

  count >>= 1;
  ds_xstep <<= 1;
  ds_ystep <<= 1;
  do
  {
    // Current texture index in u,v.
    spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

    // Lookup pixel from flat texture tile,
    // re-index using light/colormap.
    // Render pixels four at a time
    pixel = ds_colormap[ds_source[spot]];
    pixel |= pixel << 8;
    *dest2++ = *dest++ = pixel;

    // Next step in u,v.
    xfrac += ds_xstep;
    yfrac += ds_ystep;  
  }
  while (count--);
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
 
 
