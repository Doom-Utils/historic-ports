// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
// Copyright (C) 2000 by David Koppenhofer
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:
//	Rendering of moving objects, sprites.
//
//-----------------------------------------------------------------------------

#ifndef __R_THINGS__
#define __R_THINGS__

#ifdef __GNUG__
#pragma interface
#endif

// Constant arrays used for psprite clipping
//  and initializing clipping.
extern short		negonearray[SCREENWIDTH];
extern short		screenheightarray[SCREENWIDTH];

// vars for R_DrawMaskedColumn
extern short		*mfloorclip;
extern short		*mceilingclip;
extern fixed_t		spryscale;
extern fixed_t		sprtopscreen;

extern fixed_t		pspritescale;
extern fixed_t		pspriteiscale;

// *** PID BEGIN ***
// Have this function return a value denoting whether
// it drew any columns for a sprite.
// Return values are:
//  0 if nothing was drawn
//  1 if something was drawn
boolean R_DrawMaskedColumn(column_t *column);
// old code:
// void R_DrawMaskedColumn(column_t *column);
// *** PID END ***

void R_SortVisSprites(void);

void R_AddSprites(sector_t *sec);
void R_AddPSprites(void);
void R_DrawSprites(void);
void R_InitSprites(char **namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);

void R_ClipVisSprite(vissprite_t *vis, int xl, int xh);

#endif
