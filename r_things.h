// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_things.h,v 1.4 1998/05/03 22:46:19 killough Exp $
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
//      Rendering of moving objects, sprites.
//
//-----------------------------------------------------------------------------

#ifndef __R_THINGS__
#define __R_THINGS__

#ifdef __GNUG__
#pragma interface
#endif

// Constant arrays used for psprite clipping and initializing clipping.

extern short negonearray[MAX_SCREENWIDTH];         // killough 2/8/98:
extern short screenheightarray[MAX_SCREENWIDTH];   // change to MAX_*

// Vars for R_DrawMaskedColumn

extern short   *mfloorclip;
extern short   *mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t pspritescale;
extern fixed_t pspriteiscale;

void R_DrawMaskedColumn(column_t *column);
void R_SortVisSprites(void);
void R_AddSprites(sector_t *sec);
void R_AddPSprites(void);
void R_DrawSprites(void);
void R_InitSprites(char **namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);

void R_ClipVisSprite(vissprite_t *vis, int xl, int xh);

#endif

//----------------------------------------------------------------------------
//
// $Log: r_things.h,v $
// Revision 1.4  1998/05/03  22:46:19  killough
// beautification
//
// Revision 1.3  1998/02/09  03:23:27  killough
// Change array decl to use MAX screen width/height
//
// Revision 1.2  1998/01/26  19:27:49  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:09  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
