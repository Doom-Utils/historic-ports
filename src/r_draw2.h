// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw2.h,v 1.1 1998/01/04 19:28:27 pekangas Exp $
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
// DESCRIPTION:
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __R_DRAW2__
#define __R_DRAW2__


#ifdef __GNUG__
#pragma interface
#endif


void resinit_r_draw_c16(void);

// The span blitting interface.
// Hook in assembler or system specific BLT
//  here.
void 	R_DrawColumn16 (void);
void 	R_DrawColumnLow16 (void);

// The Spectre/Invisibility effect.
void 	R_DrawFuzzColumn16 (void);

// Draw with color translation tables,
//  for player sprite rendering,
//  Green/Red/Blue/Indigo shirts.
void	R_DrawTranslatedColumn16 (void);

void
R_VideoErase16
( unsigned	ofs,
  int		count );

// Span blitting for rows, floor/ceiling.
// No Sepctre effect needed.
void 	R_DrawSpan16 (void);

// Low resolution mode, 160x200?
void 	R_DrawSpanLow16 (void);


void
R_InitBuffer16
( int		width,
  int		height );


// Initialize color translation tables,
//  for player rendering etc.
void	R_InitTranslationTables16 (void);



// Rendering function.
void R_FillBackScreen16 (void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder16 (void);



#endif
//-----------------------------------------------------------------------------
//
// $Log: r_draw2.h,v $
// Revision 1.1  1998/01/04 19:28:27  pekangas
// Initial revision
//
//
//-----------------------------------------------------------------------------
