//
// DOSDoom Video Code
//
// Original Author: Chi Hoang
//
// By the DOSDoom Team
//
#ifndef __MULTIRES_H__
#define __MULTIRES_H__

#include "dm_type.h"
#include "dm_defs.h"
#include "r_data.h"

void V_MultiResInit(void);

// -ES- 1998/08/20 Added this
void V_InitResolution(void);

//
// okay, we're gonna replace v_video.c and r_draw.c with separate 8 and 16 bit
// color versions.  Thus, we need a whole load of function pointers
//

//
//start with v_video.h
//
#define CENTERY			(SCREENHEIGHT/2)

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

extern byte* screens[7]; //screens[5] and screens[6] are the double-buffers
extern byte gammatable[5][256];
extern int usegamma;

// -ES- 1998/08/20 Moved fuzz stuff here
extern int FUZZTABLE;

#define FUZZOFF	1
extern int fuzzoffset[];
extern int fuzzpos;

extern void (*V_Init) (void);

extern void
(*V_CopyRect)
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn );

extern void
(*V_DrawPatch)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

extern void
(*V_DrawPatchFlipped)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	);

extern void
(*V_DrawPatchDirect)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

extern void  //stretches to fill screen
(*V_DrawPatchInDirect)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

extern void  //stretches to fill screen
(*V_DrawPatchInDirectFlipped)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

extern void
(*V_DrawPatchTrans)
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch);

extern void  //stretches bitmap to full screen
(*V_DrawPatchInDirectTrans)
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch );


extern void
(*V_DrawPatchShrink)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

// Draw a linear block of pixels into the view buffer.
extern void
(*V_DrawBlock)
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		src );

// Reads a linear block of pixels into the view buffer.
extern void
(*V_GetBlock)
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest );

extern void
(*V_MarkRect)
( int		x,
  int		y,
  int		width,
  int		height );

extern void (*V_DarkenScreen)(int scrn);

extern void (*V_TextureBackScreen)(char *flatname);

//
// now with r_draw.h
//

extern lighttable_t*	dc_colormap;
extern int		dc_x;
extern int		dc_yl;
extern int		dc_yh;
extern fixed_t		dc_iscale;
extern fixed_t		dc_texturemid;
extern fixed_t          dc_translucency;

// first pixel in a column
extern byte*		dc_source;		

extern byte**		ylookup;
extern int*		columnofs;

extern void (*resinit_r_draw_c)(void);

// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
extern void 	(*R_DrawColumn) (void);

// The Spectre/Invisibility effect.
extern void 	(*R_DrawFuzzColumn) (void);

extern void (*R_DrawTranslucentColumn) (void);

extern void (*R_DrawTranslatedColumn) (void);
extern void (*R_DrawTranslucentTranslatedColumn) (void);

extern void (*R_VideoErase) ( unsigned ofs, int	count );

extern int		ds_y;
extern int		ds_x1;
extern int		ds_x2;

extern lighttable_t*	ds_colormap;

extern fixed_t		ds_xfrac;
extern fixed_t		ds_yfrac;
extern fixed_t		ds_xstep;
extern fixed_t		ds_ystep;

// start of a 64*64 tile image
extern byte*		ds_source;		

extern byte*		translationtables;
extern byte*		dc_translation;

// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
extern void (*R_DrawSpan) (void);

extern void (*R_InitBuffer) ( int width, int height );

// Initialize color translation tables,
//  for player rendering etc.
extern void (*R_InitTranslationTables) (void);

// Rendering function.
extern void (*R_FillBackScreen) (void);

// If the view size is not full screen, draws a border around it.
extern void (*R_DrawViewBorder) (void);

// -KM- 1998/07/21 Clears around the edge of a scaled pic
extern void V_ClearPageBackground(int scrn);

#endif
