// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-2000 by Udo Munk
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
//	Do all the WAD I/O, get map description,
//	set up initial state and misc. LUTs.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <math.h>
#if defined(LINUX) || defined(SOLARIS) || defined(IRIX)
#include <alloca.h>
#endif

#include "z_zone.h"
#include "m_menu.h"
#include "m_swap.h"
#include "m_bbox.h"
#include "g_game.h"
#include "i_system.h"
#include "w_wad.h"
#include "doomdef.h"
#include "p_local.h"
#include "p_specplus.h"
#include "p_acs.h"
#include "s_sound.h"
#include "doomstat.h"

#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include "strcmp.h"
#endif

void	P_SpawnMapThing(mapthing_ext_t *mthing);

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int		numvertexes;
vertex_t	*vertexes;

int		numsegs;
seg_t		*segs;

int		numsectors;
sector_t	*sectors;

int		numsubsectors;
subsector_t	*subsectors;

int		numnodes;
node_t		*nodes;

int		numlines;
line_t		*lines;

int		numsides;
side_t		*sides;

int		has_behavior;	// set true if map contains BEHAVIOR lump

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int		bmapwidth;
int		bmapheight;	// size in mapblocks
short		*blockmap;	// int for larger maps
// offsets in blockmap are from here
short		*blockmaplump;
// origin of block map
fixed_t		bmaporgx;
fixed_t		bmaporgy;
// for thing chains
mobj_t		**blocklinks;

// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte		*rejectmatrix;

// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS	10

mapthing_ext_t	deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_ext_t	*deathmatch_p;
mapthing_ext_t	playerstarts[MAXPLAYERS];

//
// P_LoadVertexes
//
void P_LoadVertexes(int lump)
{
    byte		*data;
    int			i;
    mapvertex_t		*ml;
    vertex_t		*li;

    // show disk icon, we are loading data
    M_DrawIcon();

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, (void *)0);

    // Load data into cache.
    data = W_CacheLumpNum(lump, PU_STATIC);

    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i = 0; i < numvertexes; i++, li++, ml++)
    {
	li->x = SHORT(ml->x) << FRACBITS;
	li->y = SHORT(ml->y) << FRACBITS;
    }

    // Free buffer memory.
    Z_Free(data);
}

//
// P_LoadSegs
//
void P_LoadSegs(int lump)
{
    byte		*data;
    int			i;
    mapseg_t		*ml;
    seg_t		*li;
    line_t		*ldef;
    int			linedef;
    int			side;

    // show disk icon, we are loading data
    M_DrawIcon();

    numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, (void *)0);
    memset(segs, 0, numsegs * sizeof(seg_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    ml = (mapseg_t *)data;
    li = segs;
    for (i = 0; i < numsegs; i++, li++, ml++)
    {
	li->v1 = &vertexes[SHORT(ml->v1)];
	li->v2 = &vertexes[SHORT(ml->v2)];

	li->angle = (SHORT(ml->angle)) << 16;
	li->offset = (SHORT(ml->offset)) << 16;
	linedef = SHORT(ml->linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = SHORT(ml->side);
	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;
	if (ldef->flags & ML_TWOSIDED && ldef->sidenum[side ^ 1] != -1)
	    li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
	else
	    li->backsector = 0;
    }

    Z_Free(data);
}

//
// P_LoadSubsectors
//
void P_LoadSubsectors(int lump)
{
    byte		*data;
    int			i;
    mapsubsector_t	*ms;
    subsector_t		*ss;

    // show disk icon, we are loading data
    M_DrawIcon();

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL,
			  (void *)0);
    data = W_CacheLumpNum(lump, PU_STATIC);

    ms = (mapsubsector_t *)data;
    memset(subsectors, 0, numsubsectors * sizeof(subsector_t));
    ss = subsectors;

    for (i = 0; i < numsubsectors; i++, ss++, ms++)
    {
	ss->numlines = SHORT(ms->numsegs);
	ss->firstline = SHORT(ms->firstseg);
    }

    Z_Free(data);
}

//
// P_LoadSectors
//
void P_LoadSectors(int lump)
{
    byte		*data;
    int			i;
    mapsector_t		*ms;
    sector_t		*ss;

    // show disk icon, we are loading data
    M_DrawIcon();

    numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
    sectors = Z_Malloc(numsectors * sizeof(sector_t), PU_LEVEL, (void *)0);
    memset(sectors, 0, numsectors * sizeof(sector_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    ms = (mapsector_t *)data;
    ss = sectors;
    for (i = 0; i < numsectors; i++, ss++, ms++)
    {
	ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
	ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
	ss->floorpic = R_FlatNumForName(ms->floorpic);
	ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
	ss->lightlevel = SHORT(ms->lightlevel);
	ss->special = SHORT(ms->special);
	ss->oldspecial = SHORT(ms->special);
	ss->tag = SHORT(ms->tag);
	ss->thinglist = NULL;
	ss->touching_thinglist = NULL;

	// lockout stair retriggering until build completes
	ss->nextsec = -1;
	ss->prevsec = -1;

	// floor offsets
	ss->floor_xoffs = 0;
	ss->floor_yoffs = 0;

	// ceiling offsets
	ss->ceiling_xoffs = 0;
	ss->ceiling_yoffs = 0;

	// transfers
	ss->heightsec = -1;	  // sector used to get floor and ceiling height
	ss->floorlightsec = -1;   // sector used to get floor lighting
	ss->ceilinglightsec = -1; // scetor used to get ceiling lighting
	ss->bottommap =		  // colormaps coming from sidedefs
	ss->midmap = ss->topmap = 0;
	ss->sky = 0;		  // sky textures coming from sidedefs
    }

    Z_Free(data);
}

//
// P_LoadNodes
//
void P_LoadNodes(int lump)
{
    byte	*data;
    int		i;
    int		j;
    int		k;
    mapnode_t*	mn;
    node_t*	no;

    // show disk icon, we are loading data
    M_DrawIcon();

    numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
    nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, (void *)0);
    data = W_CacheLumpNum(lump, PU_STATIC);

    mn = (mapnode_t *)data;
    no = nodes;

    for (i = 0; i < numnodes; i++, no++, mn++)
    {
	no->x = SHORT(mn->x) << FRACBITS;
	no->y = SHORT(mn->y) << FRACBITS;
	no->dx = SHORT(mn->dx) << FRACBITS;
	no->dy = SHORT(mn->dy) << FRACBITS;
	for (j = 0; j < 2; j++)
	{
	    no->children[j] = SHORT(mn->children[j]);
	    for (k = 0; k < 4; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
	}
    }

    Z_Free(data);
}

//
// P_LoadThings, Doom style.
// Inside the engine now we use Hexen style mapthings, so
// we need to convert the Doom style mapthings here.
//
void P_LoadThings(int lump)
{
    byte		*data;
    int			i;
    mapthing_t		*mt;
    mapthing_ext_t	mt_ext;
    int			numthings;
    boolean		spawn;
    int			flags;

    // show disk icon, we are loading data
    M_DrawIcon();

    // make sure the unused data is zero
    memset((void *)&mt_ext, 0, sizeof(mt_ext));

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_t);

    mt = (mapthing_t *)data;
    for (i = 0; i < numthings; i++, mt++)
    {
	spawn = true;

	// Do not spawn cool, new monsters if !commercial
	if ( gamemode != commercial)
	{
	    switch(mt->type)
	    {
	      case 68:	// Arachnotron
	      case 64:	// Archvile
	      case 88:	// Boss Brain
	      case 89:	// Boss Shooter
	      case 69:	// Hell Knight
	      case 67:	// Mancubus
	      case 71:	// Pain Elemental
	      case 65:	// Former Human Commando
	      case 66:	// Revenant
	      case 84:	// Wolf SS
		spawn = false;
		break;
	    }
	}
	if (spawn == false)
	    break;

	// Do spawn all other stuff.
	mt_ext.x = SHORT(mt->x);
	mt_ext.y = SHORT(mt->y);
	mt_ext.angle = SHORT(mt->angle);
	mt_ext.type = SHORT(mt->type);

	// convert flags
	flags = SHORT(mt->options);
	mt_ext.flags = flags & 15;
	mt_ext.flags |= (MTF_SINGLE | MTF_COOPERATIVE | MTF_DEATHMATCH);
	if (flags & 16)
		mt_ext.flags &= ~MTF_SINGLE;
	if (flags & 32)
		mt_ext.flags &= ~MTF_DEATHMATCH;
	if (flags & 64)
		mt_ext.flags &= ~MTF_COOPERATIVE;

	P_SpawnMapThing(&mt_ext);
    }

    Z_Free(data);
}

//
// P_LoadThingsExt
// Same as above but Hexen style
//
void P_LoadThingsExt(int lump)
{
    byte		*data;
    int			i;
    mapthing_ext_t	*mt;
    int			numthings;

    // show disk icon, we are loading data
    M_DrawIcon();

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_ext_t);
    mt = (mapthing_ext_t *)data;

    for (i = 0; i < numthings; i++, mt++)
    {
	mt->tid = SHORT(mt->tid);
	mt->x = SHORT(mt->x);
	mt->y = SHORT(mt->y);
	mt->z = SHORT(mt->z);
	mt->angle = SHORT(mt->angle);
	mt->type = SHORT(mt->type);
	mt->flags = SHORT(mt->flags);

	P_SpawnMapThing(mt);
    }

    Z_Free(data);
}

//
// P_LoadLineDefs, Doom style
// Split into two functions, to allow sidedef overloading
//
void P_LoadLineDefs(int lump)
{
    byte		*data;
    int			i;
    maplinedef_t	*mld;
    line_t		*ld;
    vertex_t		*v1;
    vertex_t		*v2;
    short		mask = stripextbits ? 0x001F : 0xFFFF;

    // show disk icon, we are loading data
    M_DrawIcon();

    numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
    lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, (void *)0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    mld = (maplinedef_t *)data;
    ld = lines;
    for (i = 0; i < numlines; i++, mld++, ld++)
    {
	ld->flags = SHORT(mld->flags) & mask;
	ld->special = SHORT(mld->special);
	ld->tag = SHORT(mld->tag);
	v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
	v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;

	ld->tranlump = -1;	// no translucency by default;

	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv(ld->dy, ld->dx) > 0)
		ld->slopetype = ST_POSITIVE;
	    else
		ld->slopetype = ST_NEGATIVE;
	}

	if (v1->x < v2->x)
	{
	    ld->bbox[BOXLEFT] = v1->x;
	    ld->bbox[BOXRIGHT] = v2->x;
	}
	else
	{
	    ld->bbox[BOXLEFT] = v2->x;
	    ld->bbox[BOXRIGHT] = v1->x;
	}

	if (v1->y < v2->y)
	{
	    ld->bbox[BOXBOTTOM] = v1->y;
	    ld->bbox[BOXTOP] = v2->y;
	}
	else
	{
	    ld->bbox[BOXBOTTOM] = v2->y;
	    ld->bbox[BOXTOP] = v1->y;
	}

	ld->sidenum[0] = SHORT(mld->sidenum[0]);
	ld->sidenum[1] = SHORT(mld->sidenum[1]);

	// support special sidedef interpretation below, from Boom
	if (ld->sidenum[0] != -1 && ld->special)
	    sides[*ld->sidenum].special = ld->special;
	else
	    sides[*ld->sidenum].special = 0;
    }

    Z_Free(data);
}

//
// P_LoadLineDefsExt
// Same as P_LoadLineDefs, but Hexen style
//
void P_LoadLineDefsExt(int lump)
{
    byte		*data;
    int			i;
    maplinedef_ext_t	*mld;
    line_t		*ld;
    vertex_t		*v1;
    vertex_t		*v2;

    // show disk icon, we are loading data
    M_DrawIcon();

    numlines = W_LumpLength(lump) / sizeof(maplinedef_ext_t);
    lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, (void *)0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    mld = (maplinedef_ext_t *)data;
    ld = lines;
    for (i = 0; i < numlines; i++, mld++, ld++)
    {
	int	j;

	for (j = 0; j < 5; j++)
	    ld->args[j] = mld->args[j];

        ld->flags = SHORT(mld->flags);
	ld->special = mld->special;

	v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
	v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;

	ld->tranlump = -1;

	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv(ld->dy, ld->dx) > 0)
		ld->slopetype = ST_POSITIVE;
	    else
		ld->slopetype = ST_NEGATIVE;
	}

	if (v1->x < v2->x)
	{
	    ld->bbox[BOXLEFT] = v1->x;
	    ld->bbox[BOXRIGHT] = v2->x;
	}
	else
	{
	    ld->bbox[BOXLEFT] = v2->x;
	    ld->bbox[BOXRIGHT] = v1->x;
	}

	if (v1->y < v2->y)
	{
	    ld->bbox[BOXBOTTOM] = v1->y;
	    ld->bbox[BOXTOP] = v2->y;
	}
	else
	{
	    ld->bbox[BOXBOTTOM] = v2->y;
	    ld->bbox[BOXTOP] = v1->y;
	}

	ld->sidenum[0] = SHORT(mld->sidenum[0]);
	ld->sidenum[1] = SHORT(mld->sidenum[1]);

	// support special sidedef interpretation below, from Boom
	if (ld->sidenum[0] != -1 && ld->special)
	    sides[*ld->sidenum].special = ld->special;
	else
	    sides[*ld->sidenum].special = 0;

	// Hexen linedefs don't have a tag, but a linedef special to set
	// the line id and some other specials, which set an id in their
	// first argument. Here we just copy the id set by the special
	// into the tag field.
	if (ld->special == 121 ||	// set line identification
	    ld->special == 208)		// translucent line
	    ld->tag = ld->args[0];
	else
	    ld->tag = 0;
    }

    Z_Free(data);
}

//
// This function is from Boom
// Used for sidedef overloading
//
void P_LoadLineDefs2(int lump)
{
    int		i = numlines;
    line_t	*ld = lines;

    // show disk icon, we are loading data
    M_DrawIcon();

    for (; i--; ld++)
    {
	// fix common wad errors (missing sidedefs)
	if (ld->sidenum[0] == -1)
	{
	    ld->sidenum[0] = 0;	// substitute dummy sidedef for missing right
	    printf("P_LoadLineDefs2: linedef %d missing first sidedef\n",
		   numlines - i);
	}

	// clear 2s flag for missing left side
	if ((ld->sidenum[1] == -1) && (ld->flags & ML_TWOSIDED))
	{
	    ld->flags &= ~ML_TWOSIDED;
	    printf("P_LoadLineDefs2: linedef %d has two-sided flag set, "
		   "but no second sidedef\n", numlines -i);
	}

	// set front and back sector of the line
	ld->frontsector = ld->sidenum[0] != -1 ?
		sides[ld->sidenum[0]].sector : 0;
	ld->backsector = ld->sidenum[1] != -1 ?
		sides[ld->sidenum[1]].sector : 0;

	// handle special types
	switch (ld->special)
	{
	    int		lump, j;

	    case 260:		// translucent 2s textures
		lump = sides[*ld->sidenum].special; // translucency from sidedef
		if (!ld->tag)
		    ld->tranlump = lump;	// affect this linedef only
		else
		    for (j = 0; j < numlines; j++)
		      if (lines[j].tag == ld->tag) // affect all matching lines
			lines[j].tranlump = lump;
	    break;
	}
    }
}

//
// Same as above for sidedef overloading, but for XDoomPlus
//
void P_LoadLineDefs2Ext(int lump)
{
    int		i = numlines;
    line_t	*ld = lines;

    // show disk icon, we are loading data
    M_DrawIcon();

    for (; i--; ld++)
    {
	// set front and back sector of the line
	ld->frontsector = ld->sidenum[0] != -1 ?
		sides[ld->sidenum[0]].sector : 0;
	ld->backsector = ld->sidenum[1] != -1 ?
		sides[ld->sidenum[1]].sector : 0;

	// handle special types
	switch (ld->special)
	{
	    int		lump, j;

	    case 208:		// translucent 2s textures, ZDoom
				// first argument is tag, second controls
				// how opaque the texture is, not supported
				// yet.
		lump = sides[*ld->sidenum].special; // translucency from sidedef
		if (!ld->tag)
		    ld->tranlump = 0;	// affect this linedef only
		else
		    for (j = 0; j < numlines; j++)
		      if (lines[j].tag == ld->tag) // affect all matching lines
			lines[j].tranlump = 0;
	    break;
	}
    }
}

//
// P_LoadSideDefs
//
void P_LoadSideDefs(int lump)
{
    // show disk icon, we are loading data
    M_DrawIcon();

    numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc(numsides * sizeof(side_t), PU_LEVEL, (void *)0);
    memset(sides, 0, numsides * sizeof(side_t));
}

//
// This function is from Boom
// Delay using texture names until after linedefs are loaded, to allow
// overloading.
//
void P_LoadSideDefs2(int lump)
{
    byte		*data;
    int			i;
    mapsidedef_t	*msd;
    side_t		*sd;

    // show disk icon, we are loading data
    M_DrawIcon();

    data = W_CacheLumpNum(lump, PU_STATIC);

    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i = 0; i < numsides; i++, msd++, sd++)
    {
	sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
	sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;
	sd->sector = &sectors[SHORT(msd->sector)];

	switch (sd->special)
	{
	    // variable colormap via 242 linedef
	    case 242:
		sd->bottomtexture =
		    (sd->sector->bottommap =
		     R_ColormapNumForName(msd->bottomtexture)) < 0 ?
		     sd->sector->bottommap = 0,
		     R_TextureNumForName(msd->bottomtexture) : 0;
		sd->midtexture =
		    (sd->sector->midmap =
		     R_ColormapNumForName(msd->midtexture)) < 0 ?
		     sd->sector->midmap = 0,
		     R_TextureNumForName(msd->midtexture) : 0;
		sd->toptexture =
		    (sd->sector->topmap =
		     R_ColormapNumForName(msd->toptexture)) < 0 ?
		     sd->sector->topmap = 0,
		     R_TextureNumForName(msd->toptexture) : 0;
		break;

	    // apply translucency to 2s normal texture
	    case 260:
		sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
		  (sd->special = W_CheckNumForName(msd->midtexture)) < 0
		  || W_LumpLength(sd->special) != 65536 ?
		  sd->special = 0, R_TextureNumForName(msd->midtexture) :
		    (sd->special++, 0) : (sd->special = 0);
		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		break;

	    // normal case
	    default:
		sd->midtexture = R_TextureNumForName(msd->midtexture);
		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		break;
	}
    }

    Z_Free(data);
}

//
// Same as above, sidedef overloading, but for XDoomPlus
//
void P_LoadSideDefs2Ext(int lump)
{
    byte		*data;
    int			i;
    mapsidedef_t	*msd;
    side_t		*sd;

    // show disk icon, we are loading data
    M_DrawIcon();

    data = W_CacheLumpNum(lump, PU_STATIC);

    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i = 0; i < numsides; i++, msd++, sd++)
    {
	sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
	sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;
	sd->sector = &sectors[SHORT(msd->sector)];

	switch (sd->special)
	{
	    // normal case
	    default:
		sd->midtexture = R_TextureNumForName(msd->midtexture);
		sd->toptexture = R_TextureNumForName(msd->toptexture);
		sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
		break;
	}
    }

    Z_Free(data);
}

//
// P_LoadBlockMap
//
void P_LoadBlockMap(int lump)
{
    int		i;
    int		count;

    // show disk icon, we are loading data
    M_DrawIcon();

    blockmaplump = W_CacheLumpNum(lump, PU_LEVEL);
    blockmap = blockmaplump + 4;
    count = W_LumpLength(lump) / 2;

    for (i = 0; i < count; i++)
	blockmaplump[i] = SHORT(blockmaplump[i]);

    bmaporgx = blockmaplump[0] << FRACBITS;
    bmaporgy = blockmaplump[1] << FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];

    // clear out mobj chains
    count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = Z_Malloc(count, PU_LEVEL, (void *)0);
    memset(blocklinks, 0, count);
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines(void)
{
    line_t		**linebuffer;
    int			i;
    int			j;
    int			total;
    line_t		*li;
    sector_t		*sector;
    subsector_t		*ss;
    seg_t		*seg;
    fixed_t		bbox[4];
    int			block;

    // look up sector number for each subsector
    ss = subsectors;
    for (i = 0; i < numsubsectors; i++, ss++)
    {
	seg = &segs[ss->firstline];
	ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    total = 0;
    for (i = 0; i < numlines; i++, li++)
    {
	total++;
	li->frontsector->linecount++;

	if (li->backsector && li->backsector != li->frontsector)
	{
	    li->backsector->linecount++;
	    total++;
	}
    }

    // build line tables for each sector
    linebuffer = Z_Malloc(total * 4, PU_LEVEL, (void *)0);
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
	M_ClearBox(bbox);
	sector->lines = linebuffer;
	li = lines;
	for (j = 0; j < numlines; j++, li++)
	{
	    if (li->frontsector == sector || li->backsector == sector)
	    {
		*linebuffer++ = li;
		M_AddToBox(bbox, li->v1->x, li->v1->y);
		M_AddToBox(bbox, li->v2->x, li->v2->y);
	    }
	}
	if (linebuffer - sector->lines != sector->linecount)
	    I_Error("P_GroupLines: miscounted");

	// set the degenmobj_t to the middle of the bounding box
	sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
	sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

	// adjust bounding box to map blocks
	block = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
	block = block >= bmapheight ? bmapheight - 1 : block;
	sector->blockbox[BOXTOP] = block;

	block = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXBOTTOM] = block;

	block = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
	block = block >= bmapwidth ? bmapwidth - 1 : block;
	sector->blockbox[BOXRIGHT] = block;

	block = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXLEFT] = block;
    }
}

//
// killough 10/98
//
// Remove slime trails.
//
// Slime trails are inherent to Doom's coordinate system -- i.e. there is
// nothing that a node builder can do to prevent slime trails ALL of the time,
// because it's a product of the integer coodinate system, and just because
// two lines pass through exact integer coordinates, doesn't necessarily mean
// that they will intersect at integer coordinates. Thus we must allow for
// fractional coordinates if we are to be able to split segs with node lines,
// as a node builder must do when creating a BSP tree.
//
// A wad file does not allow fractional coordinates, so node builders are out
// of luck except that they can try to limit the number of splits (they might
// also be able to detect the degree of roundoff error and try to avoid splits
// with a high degree of roundoff error). But we can use fractional coordinates
// here, inside the engine. It's like the difference between square inches and
// square miles, in terms of granularity.
//
// For each vertex of every seg, check to see whether it's also a vertex of
// the linedef associated with the seg (i.e, it's an endpoint). If it's not
// an endpoint, and it wasn't already moved, move the vertex towards the
// linedef by projecting it using the law of cosines. Formula:
//
//      2        2                         2        2
//    dx  x0 + dy  x1 + dx dy (y0 - y1)  dy  y0 + dx  y1 + dx dy (x0 - x1)
//   {---------------------------------, ---------------------------------}
//                  2     2                            2     2
//                dx  + dy                           dx  + dy
//
// (x0,y0) is the vertex being moved, and (x1,y1)-(x1+dx,y1+dy) is the
// reference linedef.
//
// Segs corresponding to orthogonal linedefs (exactly vertical or horizontal
// linedefs), which comprise at least half of all linedefs in most wads, don't
// need to be considered, because they almost never contribute to slime trails
// (because then any roundoff error is parallel to the linedef, which doesn't
// cause slime). Skipping simple orthogonal lines lets the code finish quicker.
//
//
void P_RemoveSlimeTrails(void)
{
    register int i;
    byte *hit = (byte *)alloca(numvertexes);	// Hitlist for vertices

    memset(hit, 0, numvertexes);		// Clear hitlist
    for (i = 0; i < numsegs; i++)		// Go through each seg
    {
	const line_t *l = segs[i].linedef;	// The parent linedef
	if (l->dx && l->dy)		// We can ignore orthogonal lines
	{
	    vertex_t *v = segs[i].v1;
	    do
		if (!hit[v - vertexes])	// If we haven't processed vertex
		{
		    hit[v - vertexes] = 1;	// Mark this vertex as processed
		    // Exclude endpoints of linedefs
		    if (v != l->v1 && v != l->v2)
		    { // Project the vertex back onto the parent linedef
		      long long dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
		      long long dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
		      long long dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
		      long long s = dx2 + dy2;
		      int x0 = v->x, y0 = v->y, x1 = l->v1->x, y1 = l->v1->y;
		      v->x = (dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s;
		      v->y = (dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s;
		    }
	        }  // Obsfucated C contest entry:   :)
	    while ((v != segs[i].v2) && (v = segs[i].v2));
	}
    }
}

//
// P_SetupLevel
//
void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
    int			i;
    char		lumpname[9];
    int			lumpnum;
    extern msecnode_t	*headsecnode;

    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;
    for (i = 0; i < MAXPLAYERS; i++)
    {
	players[i].killcount = players[i].secretcount
	    = players[i].itemcount = 0;
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1;

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start();

#if 0 // UNUSED
    if (debugfile)
    {
	Z_FreeTags(PU_LEVEL, MAXINT);
	Z_FileDumpHeap(debugfile);
    }
    else
#endif
	Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    // reset sector node list, the allocated nodes were purged above
    headsecnode = NULL;

    // UNUSED W_Profile();
    P_InitThinkers();

    // if working with a devlopment map, reload it
    W_Reload();

    // find map name
    if (gamemode == commercial)
    {
	if (map < 10)
	    sprintf(lumpname, "map0%i", map);
	else
	    sprintf(lumpname, "map%i", map);
    }
    else
    {
	lumpname[0] = 'E';
	lumpname[1] = '0' + episode;
	lumpname[2] = 'M';
	lumpname[3] = '0' + map;
	lumpname[4] = 0;
    }

    lumpnum = W_GetNumForName(lumpname);

    // check if this is a Hexen style map with BEHAVIOR
    has_behavior = W_CheckLumpName(lumpnum + ML_BEHAVIOR, "BEHAVIOR");

    leveltime = 0;

    // note: most of this ordering is important
    P_LoadBlockMap(lumpnum + ML_BLOCKMAP);
    P_LoadVertexes(lumpnum + ML_VERTEXES);
    P_LoadSectors(lumpnum + ML_SECTORS);

    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

    if (!has_behavior)
        P_LoadLineDefs(lumpnum + ML_LINEDEFS);
    else
	P_LoadLineDefsExt(lumpnum + ML_LINEDEFS);

    if (!has_behavior)
	P_LoadSideDefs2(lumpnum + ML_SIDEDEFS);
    else
	P_LoadSideDefs2Ext(lumpnum + ML_SIDEDEFS);

    if (!has_behavior)
	P_LoadLineDefs2(lumpnum + ML_LINEDEFS);
    else
	P_LoadLineDefs2Ext(lumpnum + ML_LINEDEFS);

    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);

    rejectmatrix = W_CacheLumpNum(lumpnum + ML_REJECT, PU_LEVEL);
    P_GroupLines();

    P_RemoveSlimeTrails();	// remove slime trails from wad

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;

    // clear special respawning queue
    iquehead = iquetail = 0;

    // clear tid queue slots
    for (i = 0; i < 256; i++)
	tid_list[i] = (mobj_t *)0;

    if (!has_behavior)
	P_LoadThings(lumpnum + ML_THINGS);
    else
	P_LoadThingsExt(lumpnum + ML_THINGS);

    // load scripts from BEHAVIOR
    if (has_behavior)
	P_LoadACScripts(lumpnum + ML_BEHAVIOR);

    // if deathmatch, randomly spawn the active players
    if (deathmatch)
    {
	for (i = 0; i < MAXPLAYERS; i++)
	    if (playeringame[i])
	    {
		players[i].mo = NULL;
		G_DeathMatchSpawnPlayer(i);
	    }
    }

    // set up world state
    if (!has_behavior)
	P_SpawnSpecials();
    else
	P_SpawnSpecialsPlus();

    // build subsector connect matrix
    //	UNUSED P_ConnectSubsectors();

    // preload graphics
    if (precache)
	R_PrecacheLevel();

    //printf("free memory: 0x%x\n", Z_FreeMemory());
}

//
// P_Init
//
void P_Init(void)
{
    P_InitSwitchList();
    P_InitPicAnims();
    R_InitSprites(sprnames);
}
