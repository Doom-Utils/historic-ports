// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
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
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "r_sky.h"

planefunction_t		floorfunc;
planefunction_t		ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES	128	// must be a power of 2!

static visplane_t	*visplanes[MAXVISPLANES];
static visplane_t	*freetail;
static visplane_t	**freehead = &freetail;
visplane_t		*floorplane;
visplane_t		*ceilingplane;

// Hash function for visplanes, taken from Boom
// Lee Killough, the author of the code sayz:
//    Empirically verified to be fairly uniform.
// TODO:
//   Write a histogram function which prints the hash lists in the slots,
//   to make sure it really doesn't degenerate too bad.
#define visplane_hash(picnum, lightlevel, height) \
	((unsigned)((picnum) * 3 + (lightlevel) + (height) * 7) & \
	 (MAXVISPLANES - 1))

size_t			maxopenings;
short			*openings;
short			*lastopening;

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
short			floorclip[SCREENWIDTH];
short			ceilingclip[SCREENWIDTH];

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int			spanstart[SCREENHEIGHT];
int			spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t		**planezlight;
fixed_t			planeheight;

fixed_t			yslope[SCREENHEIGHT];
fixed_t			distscale[SCREENWIDTH];
fixed_t			basexscale;
fixed_t			baseyscale;

fixed_t			cachedheight[SCREENHEIGHT];
fixed_t			cacheddistance[SCREENHEIGHT];
fixed_t			cachedxstep[SCREENHEIGHT];
fixed_t			cachedystep[SCREENHEIGHT];

static fixed_t		xoffs;	// flat offset
static fixed_t		yoffs;

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes(void)
{
}

//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//  xoffs
//  yoffs
//
// BASIC PRIMITIVE
//
void R_MapPlane(int y, int x1, int x2)
{
    angle_t	angle;
    fixed_t	distance;
    fixed_t	length;
    unsigned	index;

#ifdef RANGECHECK
    if (x2 < x1
	|| x1 < 0
	|| x2 >= viewwidth
	|| (unsigned)y > viewheight)
    {
	I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
    }
#endif

    if (planeheight != cachedheight[y])
    {
	cachedheight[y] = planeheight;
	distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
	ds_xstep = cachedxstep[y] = FixedMul(distance, basexscale);
	ds_ystep = cachedystep[y] = FixedMul(distance, baseyscale);
    }
    else
    {
	distance = cacheddistance[y];
	ds_xstep = cachedxstep[y];
	ds_ystep = cachedystep[y];
    }

    length = FixedMul(distance, distscale[x1]);
    angle = (viewangle + xtoviewangle[x1]) >> ANGLETOFINESHIFT;
    ds_xfrac = viewx + FixedMul(finecosine[angle], length) + xoffs;
    ds_yfrac = -viewy - FixedMul(finesine[angle], length) + yoffs;

    if (fixedcolormap)
	ds_colormap = fixedcolormap;
    else
    {
	index = distance >> LIGHTZSHIFT;

	if (index >= MAXLIGHTZ)
	    index = MAXLIGHTZ - 1;

	ds_colormap = planezlight[index];
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc();
}

//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes(void)
{
    int		i;
    angle_t	angle;

    // opening / clipping determination
    for (i = 0; i < viewwidth; i++)
    {
	floorclip[i] = viewheight;
	ceilingclip[i] = -1;
    }

    // new code from Boom do handle dynamic visplanes
    for (i = 0; i < MAXVISPLANES; i++)
	for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead;)
	    freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset(cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(finecosine[angle], centerxfrac);
    baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}

//
// new_visplane()
// New function from Boom sources, to allocate a new visplane
//
static visplane_t *new_visplane(unsigned hash)
{
	visplane_t *check = freetail;

	if (!check)
	    check = Z_Malloc(sizeof(*check), PU_STATIC, (void *)0);
	else
	    if (!(freetail = freetail->next))
		freehead = &freetail;
	memset(check, 0, sizeof(*check));
	check->next = visplanes[hash];
	visplanes[hash] = check;
	return check;
}

//
// R_FindPlane
//
visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel,
			fixed_t xoffs, fixed_t yoffs)
{
    visplane_t	*check;
    unsigned	hash;

    if (picnum == skyflatnum)
    {
	height = 0;			// all skys map together
	lightlevel = 0;
    }

    hash = visplane_hash(picnum, lightlevel, height);

    for (check = visplanes[hash]; check; check = check->next)
    {
	if (height == check->height
	    && picnum == check->picnum
	    && lightlevel == check->lightlevel
	    && xoffs == check->xoffs	// offset checks
	    && yoffs == check->yoffs)
	{
	    return check;
	}
    }

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH;
    check->maxx = -1;
    check->xoffs = xoffs;	// save offsets
    check->yoffs = yoffs;

    memset(check->top, 0xff, sizeof(check->top));

    return check;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{
    int		intrl;
    int		intrh;
    int		unionl;
    int		unionh;
    int		x;

    if (start < pl->minx)
    {
	intrl = pl->minx;
	unionl = start;
    }
    else
    {
	unionl = pl->minx;
	intrl = start;
    }

    if (stop > pl->maxx)
    {
	intrh = pl->maxx;
	unionh = stop;
    }
    else
    {
	unionh = pl->maxx;
	intrh = stop;
    }

    for (x = intrl; x <= intrh; x++)
	if (pl->top[x] != 0xff)
	    break;

    if (x > intrh)
    {
	pl->minx = unionl;
	pl->maxx = unionh;
    }
    else
    {
	unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
	visplane_t *new_pl = new_visplane(hash);

	new_pl->height = pl->height;
	new_pl->picnum = pl->picnum;
	new_pl->lightlevel = pl->lightlevel;
	new_pl->xoffs = pl->xoffs;
	new_pl->yoffs = pl->yoffs;
	pl = new_pl;
	pl->minx = start;
	pl->maxx = stop;
	memset(pl->top, 0xff, sizeof(pl->top));
    }

    return pl;
}

//
// R_MakeSpans
//
void R_MakeSpans(int x, int t1, int b1, int t2, int b2)
{
    while (t1 < t2 && t1 <= b1)
    {
	R_MapPlane(t1, spanstart[t1], x - 1);
	t1++;
    }
    while (b1 > b2 && b1 >= t1)
    {
	R_MapPlane(b1, spanstart[b1], x - 1);
	b1--;
    }

    while (t2 < t1 && t2 <= b2)
    {
	spanstart[t2] = x;
	t2++;
    }
    while (b2 > b1 && b2 >= t2)
    {
	spanstart[b2] = x;
	b2--;
    }
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes(void)
{
    visplane_t	*pl;
    int		i;
    int		x;

    for (i = 0; i < MAXVISPLANES; i++)
    {
	for (pl = visplanes[i]; pl; pl = pl->next)
	{
	    if (pl->minx > pl->maxx)
		continue;

	    // sky flat
	    if (pl->picnum == skyflatnum)
	    {
		dc_iscale = pspriteiscale >> detailshift;

		// Sky is allways drawn full bright,
		//  i.e. colormaps[0] is used.
		// Because of this hack, sky is not affected
		//  by INVUL inverse mapping.
		//dc_colormap = colormaps;

		// I never liked that hack, this fixes it
		if (fixedcolormap)
		    dc_colormap = fixedcolormap;
		else
		    dc_colormap = colormaps;

		dc_texturemid = skytexturemid;
		dc_texheight = textureheight[skytexture] >> FRACBITS;

		for (x = pl->minx; x <= pl->maxx; x++)
		{
		    dc_yl = pl->top[x];
		    dc_yh = pl->bottom[x];

		    if (dc_yl <= dc_yh)
		    {
			dc_x = x;
			dc_source = R_GetColumn(skytexture,
				(viewangle + xtoviewangle[x]) >>
				ANGLETOSKYSHIFT);
			colfunc();
		    }
		}
	    }
	    else
	    {
		int stop;
		int light;

		// regular flat
		ds_source = W_CacheLumpNum(firstflat +
				flattranslation[pl->picnum],
				PU_STATIC);

		xoffs = pl->xoffs;
		yoffs = pl->yoffs;
		planeheight = abs(pl->height - viewz);
		light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;

		if (light >= LIGHTLEVELS)
		    light = LIGHTLEVELS - 1;

		if (light < 0)
		    light = 0;

		planezlight = zlight[light];

		pl->top[pl->maxx+1] = 0xff;
		pl->top[pl->minx-1] = 0xff;

		stop = pl->maxx + 1;

		for (x = pl->minx; x <= stop; x++)
		{
		    R_MakeSpans(x, pl->top[x - 1],
			pl->bottom[x - 1],
			pl->top[x],
			pl->bottom[x]);
		}

		Z_ChangeTag(ds_source, PU_CACHE);
	    }
	}
    }
}
