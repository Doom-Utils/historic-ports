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
//	BSP traversal, handling of LineSegs for rendering.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <string.h>

#include "doomdef.h"
#include "m_bbox.h"
#include "i_system.h"
#include "r_main.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_things.h"
#include "doomstat.h"
#include "r_state.h"

seg_t		*curline;
side_t		*sidedef;
line_t		*linedef;
sector_t	*frontsector;
sector_t	*backsector;

drawseg_t	*drawsegs;
unsigned	maxdrawsegs;
drawseg_t	*ds_p;

int		doorclosed;

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs(void)
{
    ds_p = drawsegs;
}

//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef	struct
{
    short first;
    short last;
} cliprange_t;

// From Boom:
//
// 1/11/98: Lee Killough
//
// This fixes many strange venetian blind crashes, which occurred when a scan
// line had too many "posts" of alternating non-transparent and transparent
// regions. Using a doubly-linked list to represent the posts is one way to
// do it, but it has increased overhead and poor spatial locality, which hurts
// cache performance on modern machines. Since the maximum number of posts
// theoretically possible is a function of screen width, a static limit is
// okay in this case. It used to be 32, which was way too small.
//
// This limit was frequently mistaken for the visplane limit in some Doom
// editing FAQs, where visplanes were said to "double" if a pillar or other
// object split the view's space into two pieces horizontally. That did not
// have anything to do with visplanes, but it had everything to do with these
// clip posts.
//#define MAXSEGS		32
#define MAXSEGS (SCREENWIDTH / 2 + 1)

// newend is one past the last valid seg
static cliprange_t	*newend;
static cliprange_t	solidsegs[MAXSEGS];

//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
//
void R_ClipSolidWallSegment(int first, int last)
{
    cliprange_t		*next;
    cliprange_t		*start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first - 1)
	start++;

    if (first < start->first)
    {
	if (last < start->first - 1)
	{
	    // Post is entirely visible (above start),
	    //  so insert a new clippost.
	    R_StoreWallRange(first, last);
	    memmove(start + 1, start, (++newend - start) * sizeof(*start));
	    start->first = first;
	    start->last = last;
	    return;
	}

	// There is a fragment above *start.
	R_StoreWallRange(first, start->first - 1);

	// Now adjust the clip size.
	start->first = first;
    }

    // Bottom contained in start?
    if (last <= start->last)
	return;

    next = start;
    while (last >= (next + 1)->first - 1)
    {
	// There is a fragment between two posts.
	R_StoreWallRange(next->last + 1, (next + 1)->first - 1);
	next++;

	if (last <= next->last)
	{
	    // Bottom is contained in next.
	    // Adjust the clip size.
	    start->last = next->last;
	    goto crunch;
	}
    }

    // There is a fragment after *next.
    R_StoreWallRange(next->last + 1, last);

    // Adjust the clip size.
    start->last = last;

    // Remove start + 1 to next from the clip list,
    // because start now covers their area.
  crunch:
    if (next == start)
    {
	// Post just extended past the bottom of one post.
	return;
    }

    while (next++ != newend)
    {
	// Remove a post.
	*++start = *next;
    }

    newend = start + 1;
}

//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
void R_ClipPassWallSegment(int first, int last)
{
    cliprange_t		*start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first - 1)
	start++;

    if (first < start->first)
    {
	if (last < start->first - 1)
	{
	    // Post is entirely visible (above start).
	    R_StoreWallRange(first, last);
	    return;
	}

	// There is a fragment above *start.
	R_StoreWallRange(first, start->first - 1);
    }

    // Bottom contained in start?
    if (last <= start->last)
	return;

    while (last >= (start + 1)->first - 1)
    {
	// There is a fragment between two posts.
	R_StoreWallRange(start->last + 1, (start + 1)->first - 1);
	start++;

	if (last <= start->last)
	    return;
    }

    // There is a fragment after *next.
    R_StoreWallRange(start->last + 1, last);
}

//
// R_ClearClipSegs
//
void R_ClearClipSegs(void)
{
    solidsegs[0].first = -0x7fff;
    solidsegs[0].last = -1;
    solidsegs[1].first = viewwidth;
    solidsegs[1].last = 0x7fff;
    newend = solidsegs + 2;
}

// From Boom:
//
// killough 1/18/98 -- This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// It assumes that Doom has already ruled out a door being closed because
// of front-back closure (e.g. front floor is taller than back ceiling).
int R_DoorClosed(void)
{
    return

    // if door is closed because back is shut:
    backsector->ceilingheight <= backsector->floorheight

    // preserve a kind of transparent door/lift special effect:
    && (backsector->ceilingheight >= frontsector->ceilingheight ||
	curline->sidedef->toptexture)

    && (backsector->floorheight <= frontsector->floorheight ||
	curline->sidedef->bottomtexture)

    // properly render skies (consider door "open" if both ceilings are sky):
    && (backsector->ceilingpic != skyflatnum ||
        frontsector->ceilingpic != skyflatnum);
}

//
// From Boom sources:
//
// For one this is used to modify floor and ceiling lighting.
// Then, if player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
// Similar for ceiling, only reflected.
//
sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec,
		     int *floorlightlevel, int *ceilinglightlevel,
		     boolean back)
{
    if (floorlightlevel)
	*floorlightlevel = sec->floorlightsec == -1 ?
	sec->lightlevel : sectors[sec->floorlightsec].lightlevel;

    if (ceilinglightlevel)
	*ceilinglightlevel = sec->ceilinglightsec == -1 ?
	sec->lightlevel : sectors[sec->ceilinglightsec].lightlevel;

    if (sec->heightsec != -1)
    {
	sector_t *s = &sectors[sec->heightsec];
	int	 heightsec = viewplayer->mo->subsector->sector->heightsec;
	int	 underwater = heightsec != -1 && viewz <=
			      sectors[heightsec].floorheight;

	// replace the sector beeing drawn with a copy to be hacked
	*tempsec = *sec;

	// replace floor and ceiling height with other sector's heights
	tempsec->floorheight = s->floorheight;
	tempsec->ceilingheight = s->ceilingheight;

	// prevent sudden light changes from non-water sectors
	if ((underwater && (tempsec->floorheight = sec->floorheight,
			   tempsec->ceilingheight = s->floorheight - 1, !back))
			   || viewz <= s->floorheight)
	{
	    // head below floor hack
	    tempsec->floorpic = s->floorpic;
	    tempsec->floor_xoffs = s->floor_xoffs;
	    tempsec->floor_yoffs = s->floor_yoffs;

	    if (underwater)
	    {
		if (s->ceilingpic == skyflatnum)
		{
		    tempsec->floorheight = tempsec->ceilingheight + 1;
		    tempsec->ceilingpic = tempsec->floorpic;
		    tempsec->ceiling_xoffs = tempsec->floor_xoffs;
		    tempsec->ceiling_yoffs = tempsec->floor_yoffs;
		}
		else
		{
		    tempsec->ceilingpic = s->ceilingpic;
		    tempsec->ceiling_xoffs = s->ceiling_xoffs;
		    tempsec->ceiling_yoffs = s->ceiling_yoffs;
		}
	    }

	    tempsec->lightlevel = s->lightlevel;

	    if (floorlightlevel)
		*floorlightlevel = s->floorlightsec == -1 ?
		s->lightlevel : sectors[s->floorlightsec].lightlevel;

	    if (ceilinglightlevel)
		*ceilinglightlevel = s->ceilinglightsec == -1 ?
		s->lightlevel : sectors[s->ceilinglightsec].lightlevel;
	}
	else if (heightsec != -1 && viewz >= sectors[heightsec].ceilingheight
		 && sec->ceilingheight > s->ceilingheight)
	{
	    // head above ceiling hack
	    tempsec->ceilingheight = s->ceilingheight;
	    tempsec->floorheight = s->ceilingheight + 1;
	    tempsec->floorpic = tempsec->ceilingpic = s->ceilingpic;
	    tempsec->floor_xoffs = tempsec->ceiling_xoffs = s->ceiling_xoffs;
	    tempsec->floor_yoffs = tempsec->ceiling_yoffs = s->ceiling_yoffs;

	    if (s->floorpic != skyflatnum)
	    {
		tempsec->ceilingheight = sec->ceilingheight;
		tempsec->floorpic = s->floorpic;
		tempsec->floor_xoffs = s->floor_xoffs;
		tempsec->floor_yoffs = s->floor_yoffs;
	    }

	    tempsec->lightlevel = s->lightlevel;

	    if (floorlightlevel)
		*floorlightlevel = s->floorlightsec == -1 ?
		s->lightlevel : sectors[s->floorlightsec].lightlevel;

	    if (ceilinglightlevel)
		*ceilinglightlevel = s->ceilinglightsec == -1 ?
		s->lightlevel : sectors[s->ceilinglightsec].lightlevel;
	}
	sec = tempsec;	// use other sector
    }

    return sec;
}

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
void R_AddLine(seg_t *line)
{
    int			x1;
    int			x2;
    angle_t		angle1;
    angle_t		angle2;
    angle_t		span;
    angle_t		tspan;
    static sector_t	tempsec;

    curline = line;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle(line->v1->x, line->v1->y);
    angle2 = R_PointToAngle(line->v2->x, line->v2->y);

    // Clip to view edges.
    // OPTIMIZE: make constant out of 2 * clipangle (FIELDOFVIEW).
    span = angle1 - angle2;

    // Back side? I.e. backface culling?
    if (span >= ANG180)
	return;

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;

    tspan = angle1 + clipangle;
    if (tspan > 2 * clipangle)
    {
	tspan -= 2 * clipangle;

	// Totally off the left edge?
	if (tspan >= span)
	    return;

	angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2 * clipangle)
    {
	tspan -= 2 * clipangle;

	// Totally off the left edge?
	if (tspan >= span)
	    return;
	angle2 = -clipangle;
    }

    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    // Does not cross a pixel?
    if (x1 >= x2)
	return;

    backsector = line->backsector;

    // Single sided line?
    if (!backsector)
	goto clipsolid;

    // hack for invisible ceilings / deep water
    backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

    // Closed door.
    doorclosed = 0;
    if (backsector->ceilingheight <= frontsector->floorheight
	|| backsector->floorheight >= frontsector->ceilingheight)
	goto clipsolid;

    // This fixes the automap floor height bug
    if ((doorclosed = R_DoorClosed()))
	goto clipsolid;

    // Window.
    if (backsector->ceilingheight != frontsector->ceilingheight
	|| backsector->floorheight != frontsector->floorheight)
	goto clippass;

    // Reject empty lines used for triggers
    //  and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides,
    // and no middle texture.
    if (backsector->ceilingpic == frontsector->ceilingpic
	&& backsector->floorpic == frontsector->floorpic
	&& backsector->lightlevel == frontsector->lightlevel
	&& curline->sidedef->midtexture == 0
	// take floor and ceiling offsets into account
	&& backsector->floor_xoffs == frontsector->floor_xoffs
	&& backsector->floor_yoffs == frontsector->floor_yoffs
	&& backsector->ceiling_xoffs == frontsector->ceiling_xoffs
	&& backsector->ceiling_yoffs == frontsector->ceiling_yoffs
	// consider altered lighting
	&& backsector->floorlightsec == frontsector->floorlightsec
	&& backsector->ceilinglightsec == frontsector->ceilinglightsec
       )
    {
	return;
    }

  clippass:
    R_ClipPassWallSegment(x1, x2 - 1);
    return;

  clipsolid:
    R_ClipSolidWallSegment(x1, x2 - 1);
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
int	checkcoord[12][4] =
{
    {3, 0, 2, 1},
    {3, 0, 2, 0},
    {3, 1, 2, 0},
    {0},
    {2, 0, 2, 1},
    {0, 0, 0, 0},
    {3, 1, 3, 0},
    {0},
    {2, 0, 3, 1},
    {2, 1, 3, 1},
    {2, 1, 3, 0}
};

boolean R_CheckBBox(fixed_t *bspcoord)
{
    int			boxx;
    int			boxy;
    int			boxpos;

    fixed_t		x1;
    fixed_t		y1;
    fixed_t		x2;
    fixed_t		y2;

    angle_t		angle1;
    angle_t		angle2;
    angle_t		span;
    angle_t		tspan;

    cliprange_t		*start;

    int			sx1;
    int			sx2;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (viewx <= bspcoord[BOXLEFT])
	boxx = 0;
    else if (viewx < bspcoord[BOXRIGHT])
	boxx = 1;
    else
	boxx = 2;

    if (viewy >= bspcoord[BOXTOP])
	boxy = 0;
    else if (viewy > bspcoord[BOXBOTTOM])
	boxy = 1;
    else
	boxy = 2;

    boxpos = (boxy << 2) + boxx;
    if (boxpos == 5)
	return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    // check clip list for an open space
    angle1 = R_PointToAngle(x1, y1) - viewangle;
    angle2 = R_PointToAngle(x2, y2) - viewangle;

    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
	return true;

    tspan = angle1 + clipangle;

    if (tspan > 2 * clipangle)
    {
	tspan -= 2 * clipangle;

	// Totally off the left edge?
	if (tspan >= span)
	    return false;

	angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2 * clipangle)
    {
	tspan -= 2 * clipangle;

	// Totally off the left edge?
	if (tspan >= span)
	    return false;

	angle2 = -clipangle;
    }

    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
    sx1 = viewangletox[angle1];
    sx2 = viewangletox[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
	return false;
    sx2--;

    start = solidsegs;
    while (start->last < sx2)
	start++;

    if (sx1 >= start->first && sx2 <= start->last)
    {
	// The clippost contains the new span.
	return false;
    }

    return true;
}

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
void R_Subsector(int num)
{
    int			count;
    seg_t		*line;
    subsector_t		*sub;
    sector_t		tempsec;
    int			floorlightlevel;
    int			ceilinglightlevel;

#ifdef RANGECHECK
    if (num >= numsubsectors)
	I_Error("R_Subsector: ss %i with numss = %i",
		 num,
		 numsubsectors);
#endif

    sscount++;
    sub = &subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &segs[sub->firstline];

    // for floor/ceiling lighting different from sector and fake ceiling
    frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel,
			     &ceilinglightlevel, false);

    floorplane = frontsector->floorheight < viewz ||
		 (frontsector->heightsec != -1 &&
		  sectors[frontsector->heightsec].ceilingpic == skyflatnum) ?
		 R_FindPlane(frontsector->floorheight,
			     frontsector->floorpic == skyflatnum &&
			     frontsector->sky & PL_SKYFLAT ?
			     frontsector->sky : frontsector->floorpic,
			     floorlightlevel,
			     frontsector->floor_xoffs,
			     frontsector->floor_yoffs) : NULL;

    ceilingplane = frontsector->ceilingheight > viewz ||
		   frontsector->ceilingpic == skyflatnum ||
		   (frontsector->heightsec != -1 &&
		    sectors[frontsector->heightsec].floorpic == skyflatnum) ?
		   R_FindPlane(frontsector->ceilingheight,
			       frontsector->ceilingpic == skyflatnum &&
			       frontsector->sky & PL_SKYFLAT ?
			       frontsector->sky : frontsector->ceilingpic,
			       ceilinglightlevel,
			       frontsector->ceiling_xoffs,
			       frontsector->ceiling_yoffs) : NULL;

    R_AddSprites(sub->sector, (floorlightlevel + ceilinglightlevel) / 2);

    while (count--)
	R_AddLine(line++);
}

//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
void R_RenderBSPNode(int bspnum)
{
    node_t	*bsp;
    int		side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
	if (bspnum == -1)
	    R_Subsector(0);
	else
	    R_Subsector(bspnum & ~NF_SUBSECTOR);
	return;
    }

    bsp = &nodes[bspnum];

    // Decide which side the view point is on.
    side = R_PointOnSide(viewx, viewy, bsp);

    // Recursively divide front space.
    R_RenderBSPNode(bsp->children[side]);

    // Possibly divide back space.
    if (R_CheckBBox(bsp->bbox[side ^ 1]))
	R_RenderBSPNode(bsp->children[side ^ 1]);
}
