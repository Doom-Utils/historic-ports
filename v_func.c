//
// DOSDoom Video Virtual Functions Code
//
// Original Author: Chi Hoang
//
// By the DOSDoom Team
//

//
// These are where the function pointers are actually delared (not extern)
//

#include "dm_type.h"
#include "dm_defs.h"

// Needed because we are refering to patches.
#include "r_data.h"

// Allocates buffer screens, call before R_Init.
void (*V_Init) (void);

void
(*V_CopyRect)
( int		srcx,
  int		srcy,
  int		srcscrn,
  int		width,
  int		height,
  int		destx,
  int		desty,
  int		destscrn );

void
(*V_DrawPatchTrans)
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch);

void  //stretches bitmap to full screen
(*V_DrawPatchInDirectTrans)
( int		x,
  int		y,
  int           index,
  int		scrn,
  patch_t*	patch );

void
(*V_DrawPatch)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch);

void
(*V_DrawPatchFlipped)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch	);

void
(*V_DrawPatchDirect)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void
(*V_DrawPatchInDirect)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void
(*V_DrawPatchInDirectFlipped)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

void
(*V_DrawPatchShrink)
( int		x,
  int		y,
  int		scrn,
  patch_t*	patch );

// Draw a linear block of pixels into the view buffer.
void
(*V_DrawBlock)
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		src );

// Reads a linear block of pixels into the view buffer.
void
(*V_GetBlock)
( int		x,
  int		y,
  int		scrn,
  int		width,
  int		height,
  byte*		dest );


void
(*V_MarkRect)
( int		x,
  int		y,
  int		width,
  int		height );

void (*V_DarkenScreen)(int scrn);

// 98-7-10 KM Reduce code redundancy
void (*V_TextureBackScreen)(char *flatname);

void (*resinit_r_draw_c)(void);

// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
void 	(*R_DrawColumn) (void);

// The Spectre/Invisibility effect.
void 	(*R_DrawFuzzColumn) (void);

// -KM- 1998/11/25 All the 75%/50%/25% funcs have been removed.
void (*R_DrawTranslucentColumn) (void);
void (*R_DrawTranslucentTranslatedColumn) (void);
void (*R_DrawTranslatedColumn) (void);

void
(*R_VideoErase)
( unsigned	ofs,
  int		count );

// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
void 	(*R_DrawSpan) (void);


void
(*R_InitBuffer)
( int		width,
  int		height );


// Initialize color translation tables,
//  for player rendering etc.
void	(*R_InitTranslationTables) (void);



// Rendering function.
void (*R_FillBackScreen) (void);

// If the view size is not full screen, draws a border around it.
void (*R_DrawViewBorder) (void);


