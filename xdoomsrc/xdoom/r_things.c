// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright (C) 1999 by Dennis Chao
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
//	Refresh of things, i.e. objects represented by sprites.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <stdlib.h>
// *** PID BEGIN ***
#include <string.h>

/* from m_menu.c */
void M_WriteText(int x, int y, char *string);
// *** PID END ***
 
#include "doomdef.h"
#include "m_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "r_local.h"
#include "doomstat.h"

#define MINZ				(FRACUNIT * 4)
#define BASEYCENTER			100

typedef struct
{
    int		x1;
    int		x2;

    int		column;
    int		topclip;
    int		bottomclip;
} maskdraw_t;

//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t		pspritescale;
fixed_t		pspriteiscale;

lighttable_t	**spritelights;

// constant arrays
//  used for psprite clipping and initializing clipping
short		negonearray[SCREENWIDTH];
short		screenheightarray[SCREENWIDTH];

//
// INITIALIZATION FUNCTIONS
//

// variables used to look up
//  and range check thing_t sprites patches
spritedef_t	*sprites;
int		numsprites;

#define MAX_SPRITE_FRAMES	29
spriteframe_t	sprtemp[MAX_SPRITE_FRAMES];
int		maxframe;
char		*spritename;

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void R_InstallSpriteLump(int lump, unsigned frame, unsigned rotation,
			 boolean flipped)
{
    int		r;

    if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
	I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

    if ((int)frame > maxframe)
	maxframe = frame;

    if (rotation == 0)
    {
	// the lump should be used for all rotations
#if 0
	if (sprtemp[frame].rotate == false)
	    I_Error("R_InitSprites: Sprite %s frame %c has "
		    "multip rot=0 lump", spritename, 'A'+frame);

	if (sprtemp[frame].rotate == true)
	    I_Error("R_InitSprites: Sprite %s frame %c has rotations "
		    "and a rot=0 lump", spritename, 'A'+frame);
#endif

	sprtemp[frame].rotate = false;
	for (r = 0; r < 8; r++)
	{
	    sprtemp[frame].lump[r] = lump - firstspritelump;
	    sprtemp[frame].flip[r] = (byte)flipped;
	}
	return;
    }

    // the lump is only used for one rotation
#if 0
    if (sprtemp[frame].rotate == false)
	I_Error("R_InitSprites: Sprite %s frame %c has rotations "
		"and a rot=0 lump", spritename, 'A'+frame);
#endif

    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;
#if 0
    if (sprtemp[frame].lump[rotation] != -1)
	I_Error("R_InitSprites: Sprite %s : %c : %c "
		"has two lumps mapped to it",
		spritename, 'A'+frame, '1'+rotation);
#endif

    sprtemp[frame].lump[rotation] = lump - firstspritelump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs(char **namelist)
{
    int		i;
    int		l;
    int		frame;
    int		rotation;
    int		start;
    int		end;
    int		patched;

    // set the number of sprite names
    numsprites = NUMSPRITES;

    sprites = Z_Malloc(numsprites * sizeof(*sprites), PU_STATIC, (void *)0);

    start = firstspritelump - 1;
    end = lastspritelump + 1;

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    // ^^^ and who says an int is 4 bytes long, eh?
    for (i = 0; i < numsprites; i++)
    {
	spritename = namelist[i];
	memset(sprtemp, -1, sizeof(sprtemp));

	maxframe = -1;

	// scan the lumps,
	//  filling in the frames for whatever is found
	for (l = start + 1; l < end; l++)
	{
	    // this compare is no good
	    //if (*(int *)lumpinfo[l].name == intname)
	    if (!memcmp(lumpinfo[l].name, spritename, 4))
	    {
		frame = lumpinfo[l].name[4] - 'A';
		rotation = lumpinfo[l].name[5] - '0';

		if (modifiedgame)
		    patched = W_GetNumForName(lumpinfo[l].name);
		else
		    patched = l;

		R_InstallSpriteLump(patched, frame, rotation, false);

		if (lumpinfo[l].name[6])
		{
		    frame = lumpinfo[l].name[6] - 'A';
		    rotation = lumpinfo[l].name[7] - '0';
		    R_InstallSpriteLump(l, frame, rotation, true);
		}
	    }
	}

	// check the frames that were found for completeness
	if (maxframe == -1)
	{
	    sprites[i].numframes = 0;
	    continue;
	}

	maxframe++;

	for (frame = 0; frame < maxframe; frame++)
	{
	    switch ((int)sprtemp[frame].rotate)
	    {
	      case -1:
		// no rotations were found for that frame at all
		I_Error("R_InitSprites: No patches found "
			"for %s frame %c", namelist[i], frame + 'A');
		break;

	      case 0:
		// only the first rotation is needed
		break;

	      case 1:
		// must have all 8 frames
#if 0
		for (rotation = 0; rotation < 8; rotation++)
		    if (sprtemp[frame].lump[rotation] == -1)
			I_Error("R_InitSprites: Sprite %s frame %c "
				"is missing rotations",
				 namelist[i], frame + 'A');
#endif
		break;
	    }
	}

	// allocate space for the frames present and copy sprtemp to it
	sprites[i].numframes = maxframe;
	sprites[i].spriteframes =
	    Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, (void *)0);
	memcpy(sprites[i].spriteframes, sprtemp,
	       maxframe * sizeof(spriteframe_t));
    }
}

//
// GAME FUNCTIONS
//
static vissprite_t	*vissprites;
static vissprite_t	**vissprite_ptrs;
static size_t		num_vissprite;
static size_t		num_vissprite_alloc;
static size_t		num_vissprite_ptrs;

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites(char **namelist)
{
    int		i;

    for (i = 0; i < SCREENWIDTH; i++)
    {
	negonearray[i] = -1;
    }

    R_InitSpriteDefs(namelist);
}

//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites(void)
{
    num_vissprite = 0;
}

//
// R_NewVisSprite
//
vissprite_t *R_NewVisSprite(void)
{
    if (num_vissprite >= num_vissprite_alloc)
    {
	num_vissprite_alloc = num_vissprite_alloc ?
		num_vissprite_alloc * 2 : 128;
	vissprites = Z_Realloc(vissprites,
			       num_vissprite_alloc * sizeof(*vissprites),
			       PU_STATIC, (void *)0);
    }
    return vissprites + num_vissprite++;
}

//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
short		*mfloorclip;
short		*mceilingclip;

fixed_t		spryscale;
fixed_t		sprtopscreen;

// *** PID BEGIN ***
// Make a variable to keep track of the lowest dc_yl generated for a
// sprite.  This will be the rendered top of the sprite, because y-values
// increase down the screen.
int		lowest_dc_yl = 0;

// Have this function return a value denoting whether
// it drew any columns for a sprite.
// Return values are:
//  0 if nothing was drawn
//  1 if something was drawn
boolean R_DrawMaskedColumn(column_t *column)
// old code:
// void R_DrawMaskedColumn(column_t *column)
// *** PID END ***
{
    int		topscreen;
    int 	bottomscreen;
    fixed_t	basetexturemid;

// *** PID BEGIN ***
// Added this var to keep track of whether drawing occurred.
    boolean	drawing_occurred = 0;
// *** PID END ***

    basetexturemid = dc_texturemid;

    for (; column->topdelta != 0xff;)
    {
	// calculate unclipped screen coordinates
	//  for post
	topscreen = sprtopscreen + spryscale * column->topdelta;
	bottomscreen = topscreen + spryscale * column->length;

	dc_yl = (topscreen + FRACUNIT - 1) >> FRACBITS;
	dc_yh = (bottomscreen - 1) >> FRACBITS;

	if (dc_yh >= mfloorclip[dc_x])
	    dc_yh = mfloorclip[dc_x] - 1;
	if (dc_yl <= mceilingclip[dc_x])
	    dc_yl = mceilingclip[dc_x] + 1;

	if (dc_yl <= dc_yh)
	{
	    dc_source = (byte *)column + 3;
	    dc_texturemid = basetexturemid - (column->topdelta << FRACBITS);
	    // dc_source = (byte *)column + 3 - column->topdelta;

	    // Drawn by either R_DrawColumn
	    //  or (SHADOW) R_DrawFuzzColumn.
	    dc_texheight = 0;
	    colfunc();

// *** PID BEGIN ***
// This is where we tell if something was drawn.
            drawing_occurred = 1;

// Check to see if this dc_yl is higher on the screen (lower in value)
// than the other columns so far for this sprite.  If so, reset the
// lowest holder to the new value.
            if ( lowest_dc_yl > dc_yl ){
               lowest_dc_yl = dc_yl;
            }
// *** PID END ***

	}
	column = (column_t *)((byte *)column + column->length + 4);
    }

    dc_texturemid = basetexturemid;

// *** PID BEGIN ***
    return drawing_occurred;
// *** PID END ***
}

//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
void R_DrawVisSprite(vissprite_t *vis, int x1, int x2)
{
    column_t		*column;
    int			texturecolumn;
    fixed_t		frac;
    patch_t		*patch;
    extern byte		*tranmap;

// *** PID BEGIN ***
// This flag tells whether any of the sprite was drawn in the function
// R_DrawMaskedColumn().  That function returns a boolean to denote
// if drawing took place.  This takes care of walls, ceilings, and
// floors blocking a sprite.
    boolean		drawing_occurred = 0;
// *** PID END ***


    patch = W_CacheLumpNum(vis->patch+firstspritelump, PU_CACHE);

    dc_colormap = vis->colormap;

    if (!dc_colormap)
    {
	// NULL colormap = shadow draw
	colfunc = fuzzcolfunc;
    }
    else if ((vis->mobjflags & MF_TRANSLUC) && tranmap)
    {
	colfunc = R_DrawTranslucentColumn;
    }
    else if (vis->mobjflags & MF_TRANSLATION)
    {
	colfunc = R_DrawTranslatedColumn;
	dc_translation = translationtables - 256 +
	    ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8));
    }
    else
	colfunc = R_DrawColumn;

    dc_iscale = abs(vis->xiscale) >> detailshift;
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid, spryscale);

// *** PID BEGIN ***
// Set the 'lowest' top of this sprite to the viewheight, initially.
    lowest_dc_yl = viewheight;
// *** PID END ***

    for (dc_x = vis->x1; dc_x <= vis->x2; dc_x++, frac += vis->xiscale)
    {
	texturecolumn = frac >> FRACBITS;
#ifdef RANGECHECK
	if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
	    I_Error("R_DrawSpriteRange: bad texturecolumn");
#endif
	column = (column_t *)((byte *)patch +
			      LONG(patch->columnofs[texturecolumn]));

// *** PID BEGIN ***
// If something's been drawn already, don't reset the flag, but
// call the drawing function.  Otherwise, grab the return value
// of the function, too. 0 == nothing drawn, 1 == something drawn.
        if ( drawing_occurred ){
           R_DrawMaskedColumn(column);
        } else {
           drawing_occurred = R_DrawMaskedColumn(column);
        }
// old code:
//        R_DrawMaskedColumn(column);
// *** PID END ***

    }

    colfunc = basecolfunc;

// *** PID BEGIN ***
// At this point, the flag 'drawing_occurred' will be 0 if no part of
// this sprite was drawn in R_DrawMaskedColumn(), or 1 if something
// was drawn.  Added it to the list of conditions to check for
// drawing the pid info.
// Also, don't use the fact that a vis has a pid to determine
// whether to draw; use the m_draw_pid_info flag instead.
    if (vis->m_draw_pid_info && vis->scale>18000 &&
       vis->x1>SCREENWIDTH/8 && vis->x2<SCREENWIDTH*7/8 &&
       drawing_occurred ) {

       char buf[16];
       sprintf(buf, "%d", vis->m_pid);

// Change y-placement of text to the top of the current sprite.
// Make sure the pid text doesn't extend onto the status bar.
       if ( lowest_dc_yl + 13 > viewheight ){
          lowest_dc_yl = viewheight - 13;
       }
       M_WriteText(vis->x1, lowest_dc_yl, buf);
       M_WriteText(vis->x1, lowest_dc_yl + 6, vis->m_pname);
    }
// *** PID END ***

}

//
// R_ProjectSprite
// Generates a vissprite for a thing
//  if it might be visible.
//
void R_ProjectSprite(mobj_t *thing)
{
    fixed_t		tr_x;
    fixed_t		tr_y;
    fixed_t		gxt;
    fixed_t		gyt;
    fixed_t		tx;
    fixed_t		tz;
    fixed_t		xscale;
    int			x1;
    int			x2;
    spritedef_t		*sprdef;
    spriteframe_t	*sprframe;
    int			lump;
    unsigned		rot;
    boolean		flip;
    int			index;
    vissprite_t		*vis;
    angle_t		ang;
    fixed_t		iscale;

    // transform the origin point
    tr_x = thing->x - viewx;
    tr_y = thing->y - viewy;

    gxt = FixedMul(tr_x, viewcos);
    gyt = -FixedMul(tr_y, viewsin);

    tz = gxt - gyt;

    // thing is behind view plane?
    if (tz < MINZ)
	return;

    xscale = FixedDiv(projection, tz);

    gxt = -FixedMul(tr_x, viewsin);
    gyt = FixedMul(tr_y, viewcos);
    tx = -(gyt + gxt);

    // too far off the side?
    if (abs(tx) > (tz << 2))
	return;

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned)thing->sprite >= numsprites)
	I_Error("R_ProjectSprite: invalid sprite number %i ",
		thing->sprite);
#endif

    sprdef = &sprites[thing->sprite];

#ifdef RANGECHECK
    if ((thing->frame & FF_FRAMEMASK) >= sprdef->numframes)
	I_Error("R_ProjectSprite: invalid sprite frame %i : %i ",
		thing->sprite, thing->frame);
#endif

    sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
	// choose a different rotation based on player view
	ang = R_PointToAngle(thing->x, thing->y);
	rot = (ang-thing->angle + (unsigned)(ANG45 / 2) * 9) >> 29;
	lump = sprframe->lump[rot];
	flip = (boolean)sprframe->flip[rot];
    }
    else
    {
	// use single rotation for all views
	lump = sprframe->lump[0];
	flip = (boolean)sprframe->flip[0];
    }

    // calculate edges of the shape
    tx -= spriteoffset[lump];
    x1 = (centerxfrac + FixedMul(tx, xscale)) >> FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
	return;

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FixedMul(tx, xscale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;

    // store information in a vissprite
    vis = R_NewVisSprite();

// *** PID BEGIN ***
// Set the pid and name in the vissprite.
    vis->m_pid = thing->m_pid;
    memcpy(vis->m_pname, thing->m_pname, 8);
// Also set flag that tells whether to draw the pid info or not.
    vis->m_draw_pid_info = thing->m_draw_pid_info;
// *** PID END ***

    vis->mobjflags = thing->flags;
    vis->scale = xscale << detailshift;
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    vis->gzt = thing->z + spritetopoffset[lump];
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth - 1 : x2;
    iscale = FixedDiv(FRACUNIT, xscale);

    if (flip)
    {
	vis->startfrac = spritewidth[lump] - 1;
	vis->xiscale = -iscale;
    }
    else
    {
	vis->startfrac = 0;
	vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale * (vis->x1 - x1);
    vis->patch = lump;

    // get light level
    if (thing->flags & MF_SHADOW)
    {
	// shadow draw
	vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
	// fixed map
	vis->colormap = fixedcolormap;
    }
    else if (thing->frame & FF_FULLBRIGHT)
    {
	// full bright
	vis->colormap = colormaps;
    }
    else
    {
	// diminished light
	index = xscale >> (LIGHTSCALESHIFT - detailshift);

	if (index >= MAXLIGHTSCALE)
	    index = MAXLIGHTSCALE - 1;

	vis->colormap = spritelights[index];
    }
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites(sector_t *sec)
{
    mobj_t		*thing;
    int			lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
	return;

    // Well, now it will be done.
    sec->validcount = validcount;

    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT) + extralight;

    if (lightnum < 0)
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS - 1];
    else
	spritelights = scalelight[lightnum];

    // Handle all things in sector.
    for (thing = sec->thinglist; thing; thing = thing->snext)
	R_ProjectSprite(thing);
}

//
// R_DrawPSprite
//
void R_DrawPSprite(pspdef_t *psp)
{
    fixed_t		tx;
    int			x1;
    int			x2;
    spritedef_t		*sprdef;
    spriteframe_t	*sprframe;
    int			lump;
    boolean		flip;
    vissprite_t		*vis;
    vissprite_t		avis;

    // decide which patch to use
#ifdef RANGECHECK
    if ((unsigned)psp->state->sprite >= numsprites)
	I_Error("R_ProjectSprite: invalid sprite number %i ",
		psp->state->sprite);
#endif

    sprdef = &sprites[psp->state->sprite];

#ifdef RANGECHECK
    if ((psp->state->frame & FF_FRAMEMASK) >= sprdef->numframes)
	I_Error("R_ProjectSprite: invalid sprite frame %i : %i ",
		psp->state->sprite, psp->state->frame);
#endif

    sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    // calculate edges of the shape
    tx = psp->sx - 160 * FRACUNIT;

    tx -= spriteoffset[lump];
    x1 = (centerxfrac + FixedMul(tx, pspritescale)) >> FRACBITS;

    // off the right side
    if (x1 > viewwidth)
	return;

    tx += spritewidth[lump];
    x2 = ((centerxfrac + FixedMul(tx, pspritescale)) >> FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;

    // store information in a vissprite
    vis = &avis;
    vis->mobjflags = 0;
    vis->texturemid = (BASEYCENTER << FRACBITS) + FRACUNIT / 2 -
			(psp->sy - spritetopoffset[lump]);
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth - 1 : x2;
    vis->scale = pspritescale << detailshift;

// *** PID BEGIN ***
// Don't use the value of m_pid to determine whether to draw;
// use the flag instead.
    vis->m_draw_pid_info = false;
// *** PID END ***
 
    if (flip)
    {
	vis->xiscale = -pspriteiscale;
	vis->startfrac = spritewidth[lump] - 1;
    }
    else
    {
	vis->xiscale = pspriteiscale;
	vis->startfrac = 0;
    }

    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale * (vis->x1 - x1);

    vis->patch = lump;

    if (viewplayer->powers[pw_invisibility] > 4 * 32
	|| viewplayer->powers[pw_invisibility] & 8)
    {
	// shadow draw
	vis->colormap = NULL;
    }
    else if (fixedcolormap)
    {
	// fixed color
	vis->colormap = fixedcolormap;
    }
    else if (psp->state->frame & FF_FULLBRIGHT)
    {
	// full bright
	vis->colormap = colormaps;
    }
    else
    {
	// local light
	vis->colormap = spritelights[MAXLIGHTSCALE - 1];
    }

    R_DrawVisSprite(vis, vis->x1, vis->x2);
}

//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites(void)
{
    int		i;
    int		lightnum;
    pspdef_t	*psp;

    // get light level
    lightnum =
	(viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) +
	extralight;

    if (lightnum < 0)
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS - 1];
    else
	spritelights = scalelight[lightnum];

    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    for (i = 0, psp = viewplayer->psprites; i < NUMPSPRITES; i++, psp++)
    {
	if (psp->state)
	    R_DrawPSprite(psp);
    }
}

//
// R_SortVisSprites
// This is from the Boom sources:
// rewritten by Lee Killough to avoid using unnecessary
// linked lists and to use faster sorting algorithm.
//
#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

//
// Merge sort by Lee Killough
//
static void msort(vissprite_t **s, vissprite_t **t, int n)
{
    if (n >= 16)
    {
	int		n1 = n / 2, n2 = n - n1;
	vissprite_t	**s1 = s, **s2 = s + n1, **d = t;

	msort(s1, t, n1);
	msort(s2, t, n2);

	while ((*s1)->scale > (*s2)->scale ?
	    (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

	if (n2)
	    bcopyp(d, s2, n2);
	else
	    bcopyp(d, s1, n1);

	bcopyp(s, t, n);
    }
    else
    {
	int		i;
	int		j;
	vissprite_t	*temp;

	for (i = 1; i < n; i++)
	{
	    temp = s[i];
	    if (s[i - 1]->scale < temp->scale)
	    {
		j = i;
		while ((s[j] = s[j - 1])->scale < temp->scale && --j)
		    ;
		s[j] = temp;
	    }
	}
    }
}

void R_SortVisSprites(void)
{
    if (num_vissprite)
    {
	int	i = num_vissprite;

	if (num_vissprite_ptrs < num_vissprite * 2)
	{
	    if (vissprite_ptrs)
	    {
		Z_ChangeTag(vissprite_ptrs, PU_CACHE);
		Z_Free(vissprite_ptrs);
	    }
	    vissprite_ptrs = Z_Malloc((num_vissprite_ptrs =
		num_vissprite_alloc * 2) * sizeof(*vissprite_ptrs),
		PU_STATIC, (void *)0);
	}

	while (--i >= 0)
	    vissprite_ptrs[i] = vissprites + i;

	msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

//
// R_DrawSprite
//
void R_DrawSprite(vissprite_t *spr)
{
    drawseg_t		*ds;
    short		clipbot[SCREENWIDTH];
    short		cliptop[SCREENWIDTH];
    int			x;
    int			r1;
    int			r2;
    fixed_t		scale;
    fixed_t		lowscale;
    int			silhouette;

    for (x = spr->x1; x <= spr->x2; x++)
	clipbot[x] = cliptop[x] = -2;

    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    // for (ds = ds_p - 1; ds >= drawsegs; ds--) old buggy code
    for (ds = ds_p; ds-- > drawsegs;)
    {
	// determine if the drawseg obscures the sprite
	if (ds->x1 > spr->x2
	    || ds->x2 < spr->x1
	    || (!ds->silhouette
		&& !ds->maskedtexturecol))
	{
	    // does not cover sprite
	    continue;
	}

	r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
	r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

	if (ds->scale1 > ds->scale2)
	{
	    lowscale = ds->scale2;
	    scale = ds->scale1;
	}
	else
	{
	    lowscale = ds->scale1;
	    scale = ds->scale2;
	}

	if (scale < spr->scale
	    || (lowscale < spr->scale
		 && !R_PointOnSegSide(spr->gx, spr->gy, ds->curline)))
	{
	    // masked mid texture?
	    if (ds->maskedtexturecol)
		R_RenderMaskedSegRange(ds, r1, r2);
	    // seg is behind sprite
	    continue;
	}

	// clip this piece of the sprite
	silhouette = ds->silhouette;

	if (spr->gz >= ds->bsilheight)
	    silhouette &= ~SIL_BOTTOM;

	if (spr->gzt <= ds->tsilheight)
	    silhouette &= ~SIL_TOP;

	if (silhouette == 1)
	{
	    // bottom sil
	    for (x = r1; x <= r2; x++)
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
	}
	else if (silhouette == 2)
	{
	    // top sil
	    for (x = r1; x <= r2; x++)
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	}
	else if (silhouette == 3)
	{
	    // both
	    for (x = r1; x <= r2; x++)
	    {
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	    }
	}
    }

    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1; x <= spr->x2; x++)
    {
	if (clipbot[x] == -2)
	    clipbot[x] = viewheight;

	if (cliptop[x] == -2)
	    cliptop[x] = -1;
    }

    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite(spr, spr->x1, spr->x2);
}

//
// R_DrawMasked
//
void R_DrawMasked(void)
{
    int			i;
    drawseg_t		*ds;

    R_SortVisSprites();

    // draw all vissprites back to front
    for (i = num_vissprite; --i >= 0;)
	R_DrawSprite(vissprite_ptrs[i]);

    // render any remaining masked mid textures
    // for (ds = ds_p - 1; ds >= drawsegs; ds--) old buggy code
    for (ds = ds_p; ds-- > drawsegs;)
	if (ds->maskedtexturecol)
	    R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset)
	R_DrawPlayerSprites();
}
