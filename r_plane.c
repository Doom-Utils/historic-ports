// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_plane.c,v 1.8 1998/05/03 23:09:53 killough Exp $
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
//
// DESCRIPTION:
//      Here is a core component: drawing the floors and ceilings,
//       while maintaining a per column clipping list only.
//      Moreover, the sky areas have to be determined.
//
// MAXVISPLANES is no longer a limit on the number of visplanes,
// but a limit on the number of hash slots; larger numbers mean
// better performance usually but after a point they are wasted,
// and memory and time overheads creep in.
//
// For more information on visplanes, see:
//
// http://classicgaming.com/doom/editing/
//
// Lee Killough
//
//-----------------------------------------------------------------------------

#include "z_zone.h"  /* memory allocation wrappers -- killough */

static const char
rcsid[] = "$Id: r_plane.c,v 1.8 1998/05/03 23:09:53 killough Exp $";

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_things.h"
#include "r_sky.h"

#define MAXVISPLANES 128    /* must be a power of 2 */

static visplane_t *visplanes[MAXVISPLANES];   // killough
static visplane_t *freetail;                  // killough
static visplane_t **freehead = &freetail;     // killough
visplane_t *floorplane, *ceilingplane;

// killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))

size_t maxopenings;
short *openings,*lastopening;

// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1

short floorclip[MAX_SCREENWIDTH], ceilingclip[MAX_SCREENWIDTH];

// spanstart holds the start of a plane span; initialized to 0 at start

static int spanstart[MAX_SCREENHEIGHT];                // killough 2/8/98

//
// texture mapping
//

static lighttable_t **planezlight;
static fixed_t planeheight;

// killough 2/8/98: make variables static

static fixed_t basexscale, baseyscale;
static fixed_t cachedheight[MAX_SCREENHEIGHT];
static fixed_t cacheddistance[MAX_SCREENHEIGHT];
static fixed_t cachedxstep[MAX_SCREENHEIGHT];
static fixed_t cachedystep[MAX_SCREENHEIGHT];
static fixed_t xoffs,yoffs;    // killough 2/28/98: flat offsets

fixed_t yslope[MAX_SCREENHEIGHT], distscale[MAX_SCREENWIDTH];

//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
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

static void R_MapPlane(int y, int x1, int x2)
{
  angle_t angle;
  fixed_t distance, length;
  unsigned index;

#ifdef RANGECHECK
  if (x2 < x1 || x1<0 || x2>=viewwidth || (unsigned)y>viewheight)
    I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
#endif

  if (planeheight != cachedheight[y])
    {
      cachedheight[y] = planeheight;
      distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
      ds_xstep = cachedxstep[y] = FixedMul (distance,basexscale);
      ds_ystep = cachedystep[y] = FixedMul (distance,baseyscale);
    }
  else
    {
      distance = cacheddistance[y];
      ds_xstep = cachedxstep[y];
      ds_ystep = cachedystep[y];
    }

  length = FixedMul (distance,distscale[x1]);
  angle = (viewangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;

  // killough 2/28/98: Add offsets
  ds_xfrac =  viewx + FixedMul(finecosine[angle], length) + xoffs;
  ds_yfrac = -viewy - FixedMul(finesine[angle],   length) + yoffs;

  if (!(ds_colormap = fixedcolormap))
    {
      index = distance >> LIGHTZSHIFT;
      if (index >= MAXLIGHTZ )
        index = MAXLIGHTZ-1;
      ds_colormap = planezlight[index];
    }

  ds_y = y;
  ds_x1 = x1;
  ds_x2 = x2;

  R_DrawSpan();
}

//
// R_ClearPlanes
// At begining of frame.
//

void R_ClearPlanes(void)
{
  int i;
  angle_t angle;

  // opening / clipping determination
  for (i=0 ; i<viewwidth ; i++)
    floorclip[i] = viewheight, ceilingclip[i] = -1;

  for (i=0;i<MAXVISPLANES;i++)    // new code -- killough
    for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
      freehead = &(*freehead)->next;

  lastopening = openings;

  // texture calculation
  memset (cachedheight, 0, sizeof(cachedheight));

  // left to right mapping
  angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;

  // scale will be unit scale at SCREENWIDTH/2 distance
  basexscale = FixedDiv (finecosine[angle],centerxfrac);
  baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}

// New function, by Lee Killough

static visplane_t *new_visplane(unsigned hash)
{
  visplane_t *check = freetail;
  if (!check)
    check = calloc(1, sizeof *check);
  else
    if (!(freetail = freetail->next))
      freehead = &freetail;
  check->next = visplanes[hash];
  visplanes[hash] = check;
  return check;
}

//
// R_FindPlane
//
// killough 2/28/98: Add offsets

visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel,
                        fixed_t xoffs, fixed_t yoffs)
{
  visplane_t *check;
  unsigned hash;                      // killough

  if (picnum == skyflatnum)
    height = lightlevel = 0;          // all skys map together

  // New visplane algorithm uses hash table -- killough
  hash = visplane_hash(picnum,lightlevel,height);

  for (check=visplanes[hash]; check; check=check->next)  // killough
    if (height == check->height &&
        picnum == check->picnum &&
        lightlevel == check->lightlevel &&
        xoffs == check->xoffs &&      // killough 2/28/98: Add offset checks
        yoffs == check->yoffs)
      return check;

  check = new_visplane(hash);         // killough

  check->height = height;
  check->picnum = picnum;
  check->lightlevel = lightlevel;
  check->minx = SCREENWIDTH;
  check->maxx = -1;
  check->xoffs = xoffs;               // killough 2/28/98: Save offsets
  check->yoffs = yoffs;

  memset (check->top, 0xff, sizeof check->top);

  return check;
}

//
// R_CheckPlane
//
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{
  int intrl, intrh, unionl, unionh, x;

  if (start < pl->minx)
    intrl   = pl->minx, unionl = start;
  else
    unionl  = pl->minx,  intrl = start;

  if (stop  > pl->maxx)
    intrh   = pl->maxx, unionh = stop;
  else
    unionh  = pl->maxx, intrh  = stop;

  for (x=intrl ; x <= intrh && pl->top[x] == 0xffff; x++)
    ;

  if (x > intrh)
    pl->minx = unionl, pl->maxx = unionh;
  else
    {
      unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
      visplane_t *new_pl = new_visplane(hash);

      new_pl->height = pl->height;
      new_pl->picnum = pl->picnum;
      new_pl->lightlevel = pl->lightlevel;
      new_pl->xoffs = pl->xoffs;           // killough 2/28/98
      new_pl->yoffs = pl->yoffs;
      pl = new_pl;
      pl->minx = start;
      pl->maxx = stop;
      memset(pl->top, 0xff, sizeof pl->top);
    }

  return pl;
}

//
// R_MakeSpans
//

static void R_MakeSpans(int x, int t1, int b1, int t2, int b2)
{
  for (; t1 < t2 && t1 <= b1; t1++)
    R_MapPlane(t1, spanstart[t1], x-1);
  for (; b1 > b2 && b1 >= t1; b1--)
    R_MapPlane(b1, spanstart[b1] ,x-1);
  while (t2 < t1 && t2 <= b2)
    spanstart[t2++] = x;
  while (b2 > b1 && b2 >= t2)
    spanstart[b2--] = x;
}

// New function, by Lee Killough

static void do_draw_plane(visplane_t *pl)
{
  register int x;
  if (pl->minx <= pl->maxx)
    if (pl->picnum == skyflatnum)            // sky flat
      {
        // Sky is always drawn full bright, i.e. colormaps[0] is used.
        // Because of this hack, sky is not affected by INVUL inverse mapping.

        dc_colormap = fullcolormap;          // killough 3/20/98
        dc_texturemid = skytexturemid;
        dc_texheight = textureheight[skytexture]>>FRACBITS; // killough
        dc_iscale = pspriteiscale;

        for (x = pl->minx; x <= pl->maxx; x++)
          if ((dc_yl = pl->top[x]) <= (dc_yh = pl->bottom[x]))
            {
              dc_x = x;
              dc_source = R_GetColumn(skytexture,
                          (viewangle + xtoviewangle[x]) >> ANGLETOSKYSHIFT);
              colfunc();
            }
      }
    else      // regular flat
      {
        int stop, light;

        ds_source = W_CacheLumpNum(firstflat + flattranslation[pl->picnum],
                                   PU_STATIC);

        xoffs = pl->xoffs;  // killough 2/28/98: Add offsets
        yoffs = pl->yoffs;
        planeheight = abs(pl->height-viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;

        if (light >= LIGHTLEVELS)
          light = LIGHTLEVELS-1;

        if (light < 0)
          light = 0;

        stop = pl->maxx + 1;
        planezlight = zlight[light];
        pl->top[pl->minx-1] = pl->top[stop] = 0xffff;

        for (x = pl->minx ; x <= stop ; x++)
          R_MakeSpans(x,pl->top[x-1],pl->bottom[x-1],pl->top[x],pl->bottom[x]);

        Z_ChangeTag (ds_source, PU_CACHE);
      }
}

//
// RDrawPlanes
// At the end of each frame.
//

void R_DrawPlanes (void)
{
  visplane_t *pl;
  int i;
  for (i=0;i<MAXVISPLANES;i++)
    for (pl=visplanes[i]; pl; pl=pl->next)
      do_draw_plane(pl);
}

//----------------------------------------------------------------------------
//
// $Log: r_plane.c,v $
// Revision 1.8  1998/05/03  23:09:53  killough
// Fix #includes at the top
//
// Revision 1.7  1998/04/27  01:48:31  killough
// Program beautification
//
// Revision 1.6  1998/03/23  03:38:26  killough
// Use 'fullcolormap' for fully-bright F_SKY1
//
// Revision 1.5  1998/03/02  11:47:13  killough
// Add support for general flats xy offsets
//
// Revision 1.4  1998/02/09  03:16:03  killough
// Change arrays to use MAX height/width
//
// Revision 1.3  1998/02/02  13:28:40  killough
// performance tuning
//
// Revision 1.2  1998/01/26  19:24:45  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:03  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
