/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: v_video.c,v 1.18 1999/10/27 18:38:03 cphipps Exp $
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
 *  Gamma correction LUT stuff.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------
 */

static const char
rcsid[] = "$Id: v_video.c,v 1.18 1999/10/27 18:38:03 cphipps Exp $";

#include "doomdef.h"
#include "r_main.h"
#include "m_bbox.h"
#include "w_wad.h"   /* needed for color translation lump lookup */
#include "v_video.h"
#include "i_video.h"

// Each screen is [SCREENWIDTH*SCREENHEIGHT];
byte *screens[6];
int  dirtybox[4];

/* jff 2/18/98 palette color ranges for translation
 * jff 4/24/98 now pointers set to predefined lumps to allow overloading
 * cphipps 10/99 - be consistent in using byte for pixel values
 */

const byte *cr_brick;
const byte *cr_tan;
const byte *cr_gray;
const byte *cr_green;
const byte *cr_brown;
const byte *cr_gold;
const byte *cr_red;
const byte *cr_blue;
const byte *cr_blue_status;
const byte *cr_orange;
const byte *cr_yellow;

//jff 4/24/98 initialize this at runtime
const byte *colrngs[10];

// Now where did these came from?
const byte gammatable[5][256] = // CPhipps - const
{
  {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
   17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
   33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
   49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
   65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
   81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
   97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
   113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255},

  {2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,24,25,26,27,29,30,31,
   32,33,34,36,37,38,39,40,41,42,44,45,46,47,48,49,50,51,52,54,55,
   56,57,58,59,60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,77,
   78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,
   99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
   115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,129,
   130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,
   146,147,148,148,149,150,151,152,153,154,155,156,157,158,159,160,
   161,162,163,163,164,165,166,167,168,169,170,171,172,173,174,175,
   175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,189,
   190,191,192,193,194,195,196,196,197,198,199,200,201,202,203,204,
   205,205,206,207,208,209,210,211,212,213,214,214,215,216,217,218,
   219,220,221,222,222,223,224,225,226,227,228,229,230,230,231,232,
   233,234,235,236,237,237,238,239,240,241,242,243,244,245,245,246,
   247,248,249,250,251,252,252,253,254,255},

  {4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,33,35,36,38,39,40,42,
   43,45,46,47,48,50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,69,
   70,72,73,74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,91,92,93,
   94,95,96,97,98,100,101,102,103,104,105,106,107,108,109,110,111,112,
   113,114,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
   129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,144,
   144,145,146,147,148,149,150,151,152,153,153,154,155,156,157,158,159,
   160,160,161,162,163,164,165,166,166,167,168,169,170,171,172,172,173,
   174,175,176,177,178,178,179,180,181,182,183,183,184,185,186,187,188,
   188,189,190,191,192,193,193,194,195,196,197,197,198,199,200,201,201,
   202,203,204,205,206,206,207,208,209,210,210,211,212,213,213,214,215,
   216,217,217,218,219,220,221,221,222,223,224,224,225,226,227,228,228,
   229,230,231,231,232,233,234,235,235,236,237,238,238,239,240,241,241,
   242,243,244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,
   255},

  {8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,47,49,50,52,53,55,
   57,58,60,61,63,64,65,67,68,70,71,72,74,75,76,77,79,80,81,82,84,85,
   86,87,88,90,91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
   108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
   125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,139,140,
   141,142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,
   155,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,
   169,170,171,172,173,173,174,175,176,176,177,178,179,180,180,181,182,
   183,183,184,185,186,186,187,188,189,189,190,191,192,192,193,194,195,
   195,196,197,197,198,199,200,200,201,202,202,203,204,205,205,206,207,
   207,208,209,210,210,211,212,212,213,214,214,215,216,216,217,218,219,
   219,220,221,221,222,223,223,224,225,225,226,227,227,228,229,229,230,
   231,231,232,233,233,234,235,235,236,237,237,238,238,239,240,240,241,
   242,242,243,244,244,245,246,246,247,247,248,249,249,250,251,251,252,
   253,253,254,254,255},

  {16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,66,68,69,71,73,75,76,
   78,80,81,83,84,86,87,89,90,92,93,94,96,97,98,100,101,102,103,105,106,
   107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,
   125,126,128,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
   142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,
   156,157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
   169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,181,
   182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,192,193,
   193,194,195,195,196,196,197,198,198,199,200,200,201,202,202,203,203,
   204,205,205,206,207,207,208,208,209,210,210,211,211,212,213,213,214,
   214,215,216,216,217,217,218,219,219,220,220,221,221,222,223,223,224,
   224,225,225,226,227,227,228,228,229,229,230,230,231,232,232,233,233,
   234,234,235,235,236,236,237,237,238,239,239,240,240,241,241,242,242,
   243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,
   251,252,252,253,254,254,255,255}
};

int usegamma;

/*
 * V_InitColorTranslation
 *
 * Loads the color translation tables from predefined lumps at game start
 * No return
 *
 * Used for translating text colors from the red palette range
 * to other colors. The first nine entries can be used to dynamically
 * switch the output of text color thru the HUlib_drawText routine
 * by embedding ESCn in the text to obtain color n. Symbols for n are
 * provided in v_video.h.
 *
 * cphipps - constness of crdef_t stuff fixed
 */

typedef struct {
  const char *name;
  const byte **map1, **map2;
} crdef_t;

// killough 5/2/98: table-driven approach
static const crdef_t crdefs[] = {
  {"CRBRICK",  &cr_brick,   &colrngs[CR_BRICK ]},
  {"CRTAN",    &cr_tan,     &colrngs[CR_TAN   ]},
  {"CRGRAY",   &cr_gray,    &colrngs[CR_GRAY  ]},
  {"CRGREEN",  &cr_green,   &colrngs[CR_GREEN ]},
  {"CRBROWN",  &cr_brown,   &colrngs[CR_BROWN ]},
  {"CRGOLD",   &cr_gold,    &colrngs[CR_GOLD  ]},
  {"CRRED",    &cr_red,     &colrngs[CR_RED   ]},
  {"CRBLUE",   &cr_blue,    &colrngs[CR_BLUE  ]},
  {"CRORANGE", &cr_orange,  &colrngs[CR_ORANGE]},
  {"CRYELLOW", &cr_yellow,  &colrngs[CR_YELLOW]},
  {"CRBLUE2",  &cr_blue_status, &cr_blue_status},
  {NULL}
};

// killough 5/2/98: tiny engine driven by table above
void V_InitColorTranslation(void)
{
  register const crdef_t *p;
  for (p=crdefs; p->name; p++)
    *p->map1 = *p->map2 = W_CacheLumpName(p->name);
}

//
// V_MarkRect
//
// Marks a rectangular portion of the screen specified by
// upper left origin and height and width dirty to minimize
// the amount of screen update necessary. No return.
//
void V_MarkRect(int x, int y, int width, int height)
{
  M_AddToBox(dirtybox, x, y);
  M_AddToBox(dirtybox, x+width-1, y+height-1);
}

//
// V_CopyRect
//
// Copies a source rectangle in a screen buffer to a destination
// rectangle in another screen buffer. Source origin in srcx,srcy,
// destination origin in destx,desty, common size in width and height.
// Source buffer specfified by srcscrn, destination buffer by destscrn.
//
// Marks the destination rectangle on the screen dirty.
//
// No return.
//
void V_CopyRect(int srcx, int srcy, int srcscrn, int width,
                int height, int destx, int desty, int destscrn )
{
  byte *src;
  byte *dest;

#ifdef RANGECHECK
  if (srcx<0
      ||srcx+width >SCREENWIDTH
      || srcy<0
      || srcy+height>SCREENHEIGHT
      ||destx<0||destx+width >SCREENWIDTH
      || desty<0
      || desty+height>SCREENHEIGHT)
    I_Error ("Bad V_CopyRect");
#endif

  V_MarkRect (destx, desty, width, height);

  src = screens[srcscrn]+SCREENWIDTH*srcy+srcx;
  dest = screens[destscrn]+SCREENWIDTH*desty+destx;

  for ( ; height>0 ; height--)
    {
      memcpy (dest, src, width);
      src += SCREENWIDTH;
      dest += SCREENWIDTH;
    }
}

//
// V_DrawBlock
//
// Draw a linear block of pixels into the view buffer. 
//
// The bytes at src are copied in linear order to the screen rectangle
// at x,y in screenbuffer scrn, with size width by height.
//
// The destination rectangle is marked dirty.
//
// No return.
// 
// CPhipps - modified  to take the patch translation flags. For now, only stretching is 
//  implemented, to support highres in the menus
//
void V_DrawBlock(int x, int y, int scrn, int width, int height, 
		 const byte *src, enum patch_translation_e flags)
{
  byte *dest;

#ifdef RANGECHECK
  if (x<0
      ||x+width >((flags & VPT_STRETCH) ? 320 : SCREENWIDTH)
      || y<0
      || y+height>((flags & VPT_STRETCH) ? 200 : SCREENHEIGHT))
    I_Error ("Bad V_DrawBlock");

  if (flags & (VPT_TRANS | VPT_FLIP))
    I_Error("Bad V_DrawBlock (flags=%u)",flags);
#endif

  if (flags & VPT_STRETCH) {
    byte   *dest;
    int     s_width;
    fixed_t dx = (320 << FRACBITS) / SCREENWIDTH;
    
    x = (x * SCREENWIDTH) / 320; y = (y * SCREENHEIGHT) / 200;
    s_width = (width * SCREENWIDTH) / 320; height = (height * SCREENHEIGHT) / 200;
    
    if (!scrn)
      V_MarkRect (x, y, width, height);

    dest = screens[scrn] + y*SCREENWIDTH+x;
    // x & y no longer needed
    
    while (height--) {
      const byte *const src_row = src + width * ((height * 200) / SCREENHEIGHT);
      byte       *const dst_row = dest + SCREENWIDTH * height;
      fixed_t           tx;
      
      for (x=0, tx=0; x<s_width; x++, tx+=dx)
	dst_row[x] = src_row[tx >> FRACBITS];
    }
  } else {
    V_MarkRect (x, y, width, height);
    
    dest = screens[scrn] + y*SCREENWIDTH+x;

    while (height--) {
      memcpy (dest, src, width);
      src += width;
      dest += SCREENWIDTH;
    }
  }
}

/*
 * V_DrawBackground tiles a 64x64 patch over the entire screen, providing the
 * background for the Help and Setup screens, and plot text betwen levels.
 * cphipps - used to have M_DrawBackground, but that was used the framebuffer 
 * directly, so this is my code from the equivalent function in f_finale.c
 */

void V_DrawBackground(const char* flatname)
{
  /* erase the entire screen to a tiled background */
  const byte *src;
  int         x,y;
  int         lump;
  
  // killough 4/17/98: 
  src = W_CacheLumpNum(lump = firstflat + R_FlatNumForName(flatname));
  
  V_DrawBlock(0, 0, 0, 64, 64, src, 0);
  
  for (y=0 ; y<SCREENHEIGHT ; y+=64)
    for (x=y ? 0 : 64; x<SCREENWIDTH ; x+=64)
      V_CopyRect(0, 0, 0, ((SCREENWIDTH-x) < 64) ? (SCREENWIDTH-x) : 64, 
		 ((SCREENHEIGHT-y) < 64) ? (SCREENHEIGHT-y) : 64, x, y, 0);
  W_UnlockLumpNum(lump);
}

//
// V_GetBlock
//
// Gets a linear block of pixels from the view buffer.
//
// The pixels in the rectangle at x,y in screenbuffer scrn with size
// width by height are linearly packed into the buffer dest.
// No return
//

void V_GetBlock(int x, int y, int scrn, int width, int height, byte *dest)
{
  byte *src;

#ifdef RANGECHECK
  if (x<0
      ||x+width >SCREENWIDTH
      || y<0
      || y+height>SCREENHEIGHT)
    I_Error ("Bad V_GetBlock");
#endif

  src = screens[scrn] + y*SCREENWIDTH+x;

  while (height--)
    {
      memcpy (dest, src, width);
      src += SCREENWIDTH;
      dest += width;
    }
}

//
// V_Init
//
// Allocates the 4 full screen buffers in low DOS memory
// No return
//

void V_Init (void)
{
  int  i;
  // CPhipps - allocate only 2 screens all the time, the rest can be allocated as and when needed
#define PREALLOCED_SCREENS 2

  // CPhipps - no point in "stick these in low dos memory on PCs" anymore
  // Allocate the screens individually, so I_InitGraphics can release screens[0]
  //  if e.g. it wants a MitSHM buffer instead

  for (i=0 ; i<PREALLOCED_SCREENS ; i++)
    screens[i] = Z_Calloc(SCREENWIDTH*SCREENHEIGHT, 1, PU_STATIC, NULL);
  for (; i<4; i++) // Clear the rest (paranoia)
    screens[i] = NULL;
}

//
// V_DrawMemPatch
//
// CPhipps - unifying patch drawing routine, handles all cases and combinations
//  of stretching, flipping and translating
//
// This function is big, hopefully not too big that gcc can't optimise it well. 
// In fact it packs pretty well, there is no big performance lose for all this merging; 
// the inner loops themselves are just the same as they always were 
// (indeed, laziness of the people who wrote the 'clones' of the original V_DrawPatch
//  means that their inner loops weren't so well optimised, so merging code may even speed them).
//
void V_DrawMemPatch(int x, int y, int scrn, const patch_t *patch, 
		    const byte *trans, enum patch_translation_e flags)
{
  y -= SHORT(patch->topoffset);
  x -= SHORT(patch->leftoffset);

#ifdef HIGHRES
  // CPhipps - auto-no-stretch if not high-res
  if (flags & VPT_STRETCH)
    if ((SCREENWIDTH==320) && (SCREENHEIGHT==200))
      flags &= ~VPT_STRETCH;
#endif

  // CPhipps - null translation pointer => no translation
  if (!trans)
    flags &= ~VPT_TRANS;

#ifdef RANGECHECK
  if (x<0
      ||x+SHORT(patch->width) > ((flags & VPT_STRETCH) ? 320 : SCREENWIDTH)
      || y<0
      || y+SHORT(patch->height) > ((flags & VPT_STRETCH) ? 200 :  SCREENHEIGHT))
    // killough 1/19/98: improved error message:
    I_Error("Patch origin %d,%d exceeds LFB\n"
            "Bad V_DrawMemPatch (flags=%u)", x, y, flags);
#endif

  if (!(flags & VPT_STRETCH)) {
    int             col;
    const column_t *column;
    byte           *desttop = screens[scrn]+y*SCREENWIDTH+x;
    unsigned int    w = SHORT(patch->width);
    
    if (!scrn)
      V_MarkRect (x, y, w, SHORT(patch->height));
    
    w--; // CPhipps - note: w = width-1 now, speeds up flipping
    
    for (col=0 ; col<=w ; desttop++, col++) {
      column = (column_t *)((byte *)patch + 
			    LONG(patch->columnofs[(flags & VPT_FLIP) ? w-col : col]));
      
	// step through the posts in a column
      while (column->topdelta != 0xff ) {
	// killough 2/21/98: Unrolled and performance-tuned
	
	register const byte *source = (byte *)column + 3;
	register byte *dest = desttop + column->topdelta*SCREENWIDTH;
	register int count = column->length;
	
	if (!(flags & VPT_TRANS)) {
	  if ((count-=4)>=0)
	    do {
	      register byte s0,s1;
	      s0 = source[0];
	      s1 = source[1];
	      dest[0] = s0;
	      dest[SCREENWIDTH] = s1;
	      dest += SCREENWIDTH*2;
	      s0 = source[2];
	      s1 = source[3];
	      source += 4;
	      dest[0] = s0;
	      dest[SCREENWIDTH] = s1;
	      dest += SCREENWIDTH*2;
	    } while ((count-=4)>=0);
	  if (count+=4)
	    do {
	      *dest = *source++;
	      dest += SCREENWIDTH;
	    } while (--count);
	  column = (column_t *)(source+1); //killough 2/21/98 even faster
	} else {
	  // CPhipps - merged translation code here
	  if ((count-=4)>=0)
	    do {
	      register byte s0,s1;
	      s0 = source[0];
	      s1 = source[1];
	      s0 = trans[s0];
	      s1 = trans[s1];
	      dest[0] = s0;
	      dest[SCREENWIDTH] = s1;
	      dest += SCREENWIDTH*2;
	      s0 = source[2];
	      s1 = source[3];
	      s0 = trans[s0];
	      s1 = trans[s1];
	      source += 4;
	      dest[0] = s0;
	      dest[SCREENWIDTH] = s1;
	      dest += SCREENWIDTH*2;
	    } while ((count-=4)>=0);
	  if (count+=4)
	    do {
	      *dest = trans[*source++];
	      dest += SCREENWIDTH;
	    } while (--count);
	  column = (column_t *)(source+1);
	}
      }
    }
  }  
#ifdef HIGHRES
  else {
    // CPhipps - move stretched patch drawing code here
    //         - reformat initialisers, move variables into inner blocks
    
    byte *desttop;
    int   col;
    int   w = (SHORT( patch->width ) << 16) - 1; // CPhipps - -1 for faster flipping
    int   stretchx, stretchy;
    int   DX  = (SCREENWIDTH<<16)  / 320;
    int   DXI = (320<<16)          / SCREENWIDTH;
    int   DY  = (SCREENHEIGHT<<16) / 200;
    register int DYI = (200<<16)   / SCREENHEIGHT;
    int   DY2, DYI2;
    
    stretchx = ( x * DX ) >> 16;
    stretchy = ( y * DY ) >> 16;
    DY2  = DY / 2;
    DYI2 = DYI* 2;
    
    if (!scrn)
      V_MarkRect ( stretchx, stretchy, (SHORT( patch->width ) * DX ) >> 16,
		   (SHORT( patch->height) * DY ) >> 16 );
    
    desttop = screens[scrn] + stretchy * SCREENWIDTH +  stretchx;
    
    for ( col = 0; col <= w; x++, col+=DXI, desttop++ ) {
      const column_t *column;
      {
	unsigned int d = patch->columnofs[(flags & VPT_FLIP) ? ((w - col)>>16): (col>>16)];
	column = (column_t*)((byte*)patch + LONG(d));
      }
      
      while ( column->topdelta != 0xff ) {
	register const byte *source = ( byte* ) column + 3;
	register byte       *dest = desttop + (( column->topdelta * DY ) >> 16 ) * SCREENWIDTH;
	register int         count  = ( column->length * DY ) >> 16;
	register int         srccol = 0x8000;
	
	if (flags & VPT_TRANS)
	  while (count--) {
	    *dest  =  trans[source[srccol>>16]];
	    dest  +=  SCREENWIDTH;
	    srccol+=  DYI;
	  }
	else
	  while (count--) {
	    *dest  =  source[srccol>>16];
	    dest  +=  SCREENWIDTH;
	    srccol+=  DYI;
	  }
	column = ( column_t* ) (( byte* ) column + ( column->length ) + 4 );
      }
    }
  }
#endif
}

// CPhipps - some simple, useful wrappers for that function, for drawing patches from wads

// CPhipps - GNU C only suppresses generating a copy of a function if it is 
// static inline; other compilers have different behaviour.
// This inline is _only_ for the function below
#ifdef __GNUC__
inline
#endif
void V_DrawNumPatch(int x, int y, int scrn, int lump, 
			   const byte *trans, enum patch_translation_e flags)
{
  V_DrawMemPatch(x, y, scrn, (const patch_t*)W_CacheLumpNum(lump), 
		 trans, flags);
  W_UnlockLumpNum(lump);
}

void V_DrawNamePatch(int x, int y, int scrn, const char *name, 
		     const byte *trans, enum patch_translation_e flags)
{
  V_DrawNumPatch(x, y, scrn, W_GetNumForName(name), trans, flags);
}

// CPhipps -
// V_PatchToBlock
//
// Returns a simple bitmap which contains the patch. See-through parts of the 
// patch will be undefined (in fact black for now)

byte *V_PatchToBlock(const char* name, const byte *trans, 
			      enum patch_translation_e flags, 
			      unsigned short* width, unsigned short* height)
{
  byte          *oldscr = screens[1];
  byte          *block;
  const patch_t *patch;

  screens[1] = calloc(SCREENWIDTH*SCREENHEIGHT, 1);

  patch = W_CacheLumpName(name);
  V_DrawMemPatch(SHORT(patch->leftoffset), SHORT(patch->topoffset), 
		  1, patch, trans, flags);

#ifdef RANGECHECK
  if (flags & VPT_STRETCH) 
    I_Error("V_PatchToBlock: Stretching not supported");
#endif

  *width = SHORT(patch->width); *height = SHORT(patch->height);

  W_UnlockLumpName(name);

  V_GetBlock(0, 0, 1, *width, *height, 
	     block = malloc((long)(*width) * (*height)));

  free(screens[1]);
  screens[1] = oldscr;
  return block;
}

//
// V_SetPalette
//
// CPhipps - New function to set the palette to palette number pal.
// Handles loading of PLAYPAL and calls I_SetPalette

void V_SetPalette(unsigned short pal)
{
  I_SetPalette(pal);
}

// 
// V_FillRect
//
// CPhipps - New function to fill a rectangle with a given colour

void V_FillRect(int scrn, int x, int y, int width, int height, byte colour)
{
  byte* dest = screens[scrn] + x + y*SCREENWIDTH;
  while (height--) {
    memset(dest, colour, width);
    dest += SCREENWIDTH;
  }
}

//----------------------------------------------------------------------------
//
// $Log: v_video.c,v $
// Revision 1.18  1999/10/27 18:38:03  cphipps
// Updated for W_Cache'd lumps being properly const
// Made colour translation tables be referenced by const byte*'s
// Updated various V_* functions for this change
//
// Revision 1.17  1999/10/27 11:59:49  cphipps
// Added V_DrawBackground, which draws a tiled flat over the screen
// (taken from M_DrawBackground and similar code in f_finale.c)
//
// Revision 1.16  1999/10/12 13:01:15  cphipps
// Changed header to GPL
//
// Revision 1.15  1999/10/02 12:34:45  cphipps
// Remove RANGECHECK checks which erroneously thought 4 was the highest
//  screen number
// Fixed bad V_GetBlock RANGECHECK error message
//
// Revision 1.14  1999/09/05 16:07:05  cphipps
// Fixed bug with flipped high-res patch drawing, thanks to Gady Kozma <gady@math.tau.ac.il>
//
// Revision 1.13  1999/08/30 12:48:35  cphipps
// Added V_FillRect, to fill a rectangle on screen to a given colour
//
// Revision 1.12  1999/03/26 11:58:10  cphipps
// Remove I_AllocLow() call
// Allocate screens separately
//
// Revision 1.11  1999/02/04 21:38:23  cphipps
// Extra pointer in screens[] ready for status bar scaling
//
// Revision 1.10  1999/02/01 09:10:13  cphipps
// Pass palette number to I_SetPalette
//
// Revision 1.9  1999/01/13 08:00:49  cphipps
// Fix inlining for nono-gnu compilers
//
// Revision 1.8  1999/01/01 13:34:53  cphipps
// Added i_video.h
// Fix palette handling function
//
// Revision 1.7  1998/12/31 20:19:52  cphipps
// Added palette handling function
//
// Revision 1.6  1998/12/31 14:13:19  cphipps
// Merged all patch drawing functions into 1 function, flags selectnig stretching, translating
// etc.
// Merged block drawing functions similarly.
// New V_PatchToBlock function to convert patches to simple bitmap block images.
// Added const to many parameters.
//
// Revision 1.5  1998/12/28 21:24:57  cphipps
// Don't allocate screens[2 to 3] in startup
// Made gamma correction tables const
//
// Revision 1.4  1998/12/24 20:41:11  cphipps
// Stretched block drawing routine added
// Added const to some paramters
//
// Revision 1.3  1998/12/22 21:14:01  cphipps
// Fixed a few missing SHORT()'s
//
// Revision 1.2  1998/11/17 12:26:39  cphipps
// Updated version from PrBoom v2.02 incorporated into LxDoom
//
// Revision 1.10  1998/05/06  11:12:48  jim
// Formattted v_video.*
//
// Revision 1.9  1998/05/03  22:53:16  killough
// beautification, simplify translation lookup
//
// Revision 1.8  1998/04/24  08:09:39  jim
// Make text translate tables lumps
//
// Revision 1.7  1998/03/02  11:41:58  killough
// Add cr_blue_status for blue statusbar numbers
//
// Revision 1.6  1998/02/24  01:40:12  jim
// Tuned HUD font
//
// Revision 1.5  1998/02/23  04:58:17  killough
// Fix performance problems
//
// Revision 1.4  1998/02/19  16:55:00  jim
// Optimized HUD and made more configurable
//
// Revision 1.3  1998/02/17  23:00:36  jim
// Added color translation machinery and data
//
// Revision 1.2  1998/01/26  19:25:08  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:05  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------

