// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_setup.c,v 1.21 1998/10/13 03:19:21 jim Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//  Do all the WAD I/O, get map description,
//  set up initial state and misc. LUTs.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: p_setup.c,v 1.21 1998/10/13 03:19:21 jim Exp $";

#include "doomstat.h"
#include "m_bbox.h"
#include "m_argv.h"
#include "g_game.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "lprintf.h" //jff 10/6/98 for debug outputs

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//

int      numvertexes;
vertex_t *vertexes;

int      numsegs;
seg_t    *segs;

int      numsectors;
sector_t *sectors;

int      numsubsectors;
subsector_t *subsectors;

int      numnodes;
node_t   *nodes;

int      numlines;
line_t   *lines;

int      numsides;
side_t   *sides;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.

int       bmapwidth, bmapheight;  // size in mapblocks

// killough 3/1/98: remove blockmap limit internally:
long      *blockmap;              // was short -- killough

// offsets in blockmap are from here
long      *blockmaplump;          // was short -- killough

fixed_t   bmaporgx, bmaporgy;     // origin of block map

mobj_t    **blocklinks;           // for thing chains

//
// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without the special effect, this could
// be used as a PVS lookup as well.
//

byte *rejectmatrix;

// Maintain single and multi player starting spots.

// 1/11/98 killough: Remove limit on deathmatch starts
mapthing_t *deathmatchstarts;      // killough
size_t     num_deathmatchstarts;   // killough

mapthing_t *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];

//
// P_LoadVertexes
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadVertexes (int lump)
{
  byte *data;
  int i;

  // Determine number of lumps:
  //  total lump length / vertex record length.
  numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);

  // Allocate zone memory for buffer.
  vertexes = Z_Malloc(numvertexes*sizeof(vertex_t),PU_LEVEL,0);

  // Load data into cache.
  data = W_CacheLumpNum(lump, PU_STATIC);

  // Copy and convert vertex coordinates,
  // internal representation as fixed.
  for (i=0; i<numvertexes; i++)
    {
      vertexes[i].x = SHORT(((mapvertex_t *) data)[i].x)<<FRACBITS;
      vertexes[i].y = SHORT(((mapvertex_t *) data)[i].y)<<FRACBITS;
    }

  // Free buffer memory.
  Z_Free (data);
}


//
// P_LoadSegs
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadSegs (int lump)
{
  int  i;
  byte *data;
  byte *vertchanged = Z_Malloc(numvertexes,PU_LEVEL,0); // phares 10/4/98
  line_t* line;     // phares 10/4/98
  int ptp_angle;    // phares 10/4/98
  int delta_angle;  // phares 10/4/98
  int dis;          // phares 10/4/98
  int dx,dy;        // phares 10/4/98
  int vnum1,vnum2;  // phares 10/4/98

  memset(vertchanged,0,numvertexes); // phares 10/4/98

  numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
  segs = Z_Malloc(numsegs*sizeof(seg_t),PU_LEVEL,0);
  memset(segs, 0, numsegs*sizeof(seg_t));
  data = W_CacheLumpNum(lump,PU_STATIC);

  // phares: 10/4/98: Vertchanged is an array that represents the vertices.
  // Mark those used by linedefs. A marked vertex is one that is not a
  // candidate for movement further down.

  line = lines;
  for (i = 0; i < numlines ; i++,line++)
    {
    vertchanged[line->v1 - vertexes] = 1;
    vertchanged[line->v2 - vertexes] = 1;
    }

  for (i=0; i<numsegs; i++)
    {
      seg_t *li = segs+i;
      mapseg_t *ml = (mapseg_t *) data + i;

      int side, linedef;
      line_t *ldef;

      li->v1 = &vertexes[SHORT(ml->v1)];
      li->v2 = &vertexes[SHORT(ml->v2)];

      li->angle = (SHORT(ml->angle))<<16;

// phares 10/4/98: In the case of a lineseg that was created by splitting
// another line, it appears that the line angle is inherited from the
// father line. Due to roundoff, the new vertex may have been placed 'off
// the line'. When you get close to such a line, and it is very short,
// it's possible that the roundoff error causes 'firelines', the thin
// lines that can draw from screen top to screen bottom occasionally. This
// is due to all the angle calculations that are done based on the line
// angle, the angles from the viewer to the vertices, and the viewer's
// angle in the world. In the case of firelines, the rounded-off position
// of one of the vertices determines one of these angles, and introduces
// an error in the scaling factor for mapping textures and determining
// where on the screen the ceiling and floor spans should be shown. For a
// fireline, the engine thinks the ceiling bottom and floor top are at the
// midpoint of the screen. So you get ceilings drawn all the way down to the
// screen midpoint, and floors drawn all the way up. Thus 'firelines'. The
// name comes from the original sighting, which involved a fire texture.
//
// To correct this, reset the vertex that was added so that it sits ON the
// split line.
//
// To know which of the two vertices was added, its number is greater than
// that of the last of the author-created vertices. If both vertices of the
// line were added by splitting, pick the higher-numbered one. Once you've
// changed a vertex, don't change it again if it shows up in another seg.
//
// To determine if there's an error in the first place, find the
// angle of the line between the two seg vertices. If it's one degree or more
// off, then move one vertex. This may seem insignificant, but one degree
// errors _can_ cause firelines.

      ptp_angle = R_PointToAngle2(li->v1->x,li->v1->y,li->v2->x,li->v2->y);
      dis = 0;
      delta_angle = (abs(ptp_angle-li->angle)>>ANGLETOFINESHIFT)*360/8192;

      if (delta_angle != 0)
        {
        dx = (li->v1->x - li->v2->x)>>FRACBITS;
        dy = (li->v1->y - li->v2->y)>>FRACBITS;
        dis = ((int) sqrt(dx*dx + dy*dy))<<FRACBITS;
        dx = finecosine[li->angle>>ANGLETOFINESHIFT];
        dy = finesine[li->angle>>ANGLETOFINESHIFT];
        vnum1 = li->v1 - vertexes; 
        vnum2 = li->v2 - vertexes; 
        if ((vnum2 > vnum1) && (vertchanged[vnum2] == 0))
          {
          li->v2->x = li->v1->x + FixedMul(dis,dx);
          li->v2->y = li->v1->y + FixedMul(dis,dy);
          vertchanged[vnum2] = 1; // this was changed
          }
        else if (vertchanged[vnum1] == 0)
          {
          li->v1->x = li->v2->x - FixedMul(dis,dx);
          li->v1->y = li->v2->y - FixedMul(dis,dy);
          vertchanged[vnum1] = 1; // this was changed
          }
        }

      li->offset = (SHORT(ml->offset))<<16;
      linedef = SHORT(ml->linedef);
      ldef = &lines[linedef];
      li->linedef = ldef;
      side = SHORT(ml->side);
      li->sidedef = &sides[ldef->sidenum[side]];
      li->frontsector = sides[ldef->sidenum[side]].sector;

      // killough 5/3/98: ignore 2s flag if second sidedef missing:
      if (ldef->flags & ML_TWOSIDED && ldef->sidenum[side^1]!=-1)
        li->backsector = sides[ldef->sidenum[side^1]].sector;
      else
        li->backsector = 0;
    }

  Z_Free (data);
  Z_Free(vertchanged); // phares 10/4/98
}


//
// P_LoadSubsectors
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadSubsectors (int lump)
{
  byte *data;
  int  i;

  numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
  subsectors = Z_Malloc(numsubsectors*sizeof(subsector_t),PU_LEVEL,0);
  data = W_CacheLumpNum(lump, PU_STATIC);

  memset(subsectors, 0, numsubsectors*sizeof(subsector_t));

  for (i=0; i<numsubsectors; i++)
    {
      subsectors[i].numlines  = SHORT(((mapsubsector_t *) data)[i].numsegs );
      subsectors[i].firstline = SHORT(((mapsubsector_t *) data)[i].firstseg);
    }

  Z_Free (data);
}

//
// P_LoadSectors
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadSectors (int lump)
{
  byte *data;
  int  i;

  numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
  sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);
  memset (sectors, 0, numsectors*sizeof(sector_t));
  data = W_CacheLumpNum (lump,PU_STATIC);

  for (i=0; i<numsectors; i++)
    {
      sector_t *ss = sectors + i;
      const mapsector_t *ms = (mapsector_t *) data + i;

      ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
      ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
      ss->floorpic = R_FlatNumForName(ms->floorpic);
      ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
      ss->lightlevel = SHORT(ms->lightlevel);
      ss->special = SHORT(ms->special);
      ss->oldspecial = SHORT(ms->special);
      ss->tag = SHORT(ms->tag);
      ss->thinglist = NULL;
      ss->touching_thinglist = NULL;            // phares 3/14/98

      ss->nextsec = -1; //jff 2/26/98 add fields to support locking out
      ss->prevsec = -1; // stair retriggering until build completes

      // killough 3/7/98:
      ss->floor_xoffs = 0;
      ss->floor_yoffs = 0;      // floor and ceiling flats offsets
      ss->ceiling_xoffs = 0;
      ss->ceiling_yoffs = 0;
      ss->heightsec = -1;       // sector used to get floor and ceiling height
      ss->floorlightsec = -1;   // sector used to get floor lighting
      // killough 3/7/98: end changes

      // killough 4/11/98 sector used to get ceiling lighting:
      ss->ceilinglightsec = -1;

      // killough 4/4/98: colormaps:
      ss->bottommap = ss->midmap = ss->topmap = 0;
    }

  Z_Free (data);
}


//
// P_LoadNodes
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadNodes (int lump)
{
  byte *data;
  int  i;

  numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
  nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);
  data = W_CacheLumpNum (lump, PU_STATIC);

  for (i=0; i<numnodes; i++)
    {
      node_t *no = nodes + i;
      mapnode_t *mn = (mapnode_t *) data + i;
      int j;

      no->x = SHORT(mn->x)<<FRACBITS;
      no->y = SHORT(mn->y)<<FRACBITS;
      no->dx = SHORT(mn->dx)<<FRACBITS;
      no->dy = SHORT(mn->dy)<<FRACBITS;

      for (j=0 ; j<2 ; j++)
        {
          int k;
          no->children[j] = SHORT(mn->children[j]);
          for (k=0 ; k<4 ; k++)
            no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }

  Z_Free (data);
}


//
// P_LoadThings
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadThings (int lump)
{
  int  i, numthings = W_LumpLength (lump) / sizeof(mapthing_t);
  byte *data = W_CacheLumpNum (lump,PU_STATIC);

  for (i=0; i<numthings; i++)
    {
      mapthing_t *mt = (mapthing_t *) data + i;

      // Do not spawn cool, new monsters if !commercial
      if (gamemode != commercial)
        switch(mt->type)
          {
          case 68:  // Arachnotron
          case 64:  // Archvile
          case 88:  // Boss Brain
          case 89:  // Boss Shooter
          case 69:  // Hell Knight
          case 67:  // Mancubus
          case 71:  // Pain Elemental
          case 65:  // Former Human Commando
          case 66:  // Revenant
          case 84:  // Wolf SS
            continue;
          }

      // Do spawn all other stuff.
      mt->x = SHORT(mt->x);
      mt->y = SHORT(mt->y);
      mt->angle = SHORT(mt->angle);
      mt->type = SHORT(mt->type);
      mt->options = SHORT(mt->options);

      P_SpawnMapThing (mt);
    }

  Z_Free (data);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//        ^^^
// ??? killough ???
// Does this mean secrets used to be linedef-based, rather than sector-based?
//
// killough 4/4/98: split into two functions, to allow sidedef overloading
//
// killough 5/3/98: reformatted, cleaned up

void P_LoadLineDefs (int lump)
{
  byte *data;
  int  i;

  numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
  lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);
  memset (lines, 0, numlines*sizeof(line_t));
  data = W_CacheLumpNum (lump,PU_STATIC);

  for (i=0; i<numlines; i++)
    {
      maplinedef_t *mld = (maplinedef_t *) data + i;
      line_t *ld = lines+i;
      vertex_t *v1, *v2;

      ld->flags = SHORT(mld->flags);
      ld->special = SHORT(mld->special);
      ld->tag = SHORT(mld->tag);
      v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
      v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
      ld->dx = v2->x - v1->x;
      ld->dy = v2->y - v1->y;

      ld->tranlump = -1;   // killough 4/11/98: no translucency by default

      ld->slopetype = !ld->dx ? ST_VERTICAL : !ld->dy ? ST_HORIZONTAL :
        FixedDiv(ld->dy, ld->dx) > 0 ? ST_POSITIVE : ST_NEGATIVE;

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

      // killough 4/4/98: support special sidedef interpretation below
      if (ld->sidenum[0] != -1 && ld->special)
        sides[*ld->sidenum].special = ld->special;
    }
  Z_Free (data);
}

// killough 4/4/98: delay using sidedefs until they are loaded
// killough 5/3/98: reformatted, cleaned up

void P_LoadLineDefs2(int lump)
{
  int i = numlines;
  register line_t *ld = lines;
  for (;i--;ld++)
    {
      ld->frontsector = ld->sidenum[0]!=-1 ? sides[ld->sidenum[0]].sector : 0;
      ld->backsector  = ld->sidenum[1]!=-1 ? sides[ld->sidenum[1]].sector : 0;
      switch (ld->special)
        {                       // killough 4/11/98: handle special types
          int lump, j;

        case 260:               // killough 4/11/98: translucent 2s textures
            lump = sides[*ld->sidenum].special; // translucency from sidedef
            if (!ld->tag)                       // if tag==0,
              ld->tranlump = lump;              // affect this linedef only
            else
              for (j=0;j<numlines;j++)          // if tag!=0,
                if (lines[j].tag == ld->tag)    // affect all matching linedefs
                  lines[j].tranlump = lump;
            break;
        }
    }
}

//
// P_LoadSideDefs
//
// killough 4/4/98: split into two functions

void P_LoadSideDefs (int lump)
{
  numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
  sides = Z_Malloc(numsides*sizeof(side_t),PU_LEVEL,0);
  memset(sides, 0, numsides*sizeof(side_t));
}

// killough 4/4/98: delay using texture names until
// after linedefs are loaded, to allow overloading.
// killough 5/3/98: reformatted, cleaned up

void P_LoadSideDefs2(int lump)
{
  byte *data = W_CacheLumpNum(lump,PU_STATIC);
  int  i;

  for (i=0; i<numsides; i++)
    {
      register mapsidedef_t *msd = (mapsidedef_t *) data + i;
      register side_t *sd = sides + i;
      register sector_t *sec;

      sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
      sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;

      // killough 4/4/98: allow sidedef texture names to be overloaded
      // killough 4/11/98: refined to allow colormaps to work as wall
      // textures if invalid as colormaps but valid as textures.

      sd->sector = sec = &sectors[SHORT(msd->sector)];
      switch (sd->special)
        {
        case 242:                       // variable colormap via 242 linedef
          sd->bottomtexture =
            (sec->bottommap =   R_ColormapNumForName(msd->bottomtexture)) < 0 ?
            sec->bottommap = 0, R_TextureNumForName(msd->bottomtexture): 0 ;
          sd->midtexture =
            (sec->midmap =   R_ColormapNumForName(msd->midtexture)) < 0 ?
            sec->midmap = 0, R_TextureNumForName(msd->midtexture)  : 0 ;
          sd->toptexture =
            (sec->topmap =   R_ColormapNumForName(msd->toptexture)) < 0 ?
            sec->topmap = 0, R_TextureNumForName(msd->toptexture)  : 0 ;
          break;

        case 260: // killough 4/11/98: apply translucency to 2s normal texture
          sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
            (sd->special = W_CheckNumForName(msd->midtexture)) < 0 ||
            W_LumpLength(sd->special) != 65536 ?
            sd->special=0, R_TextureNumForName(msd->midtexture) :
              (sd->special++, 0) : (sd->special=0);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;

        default:                        // normal cases
          sd->midtexture = R_TextureNumForName(msd->midtexture);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;
        }
    }
  Z_Free (data);
}

//
// jff 10/6/98
// New code added to speed up calculation of internal blockmap
// Algorithm is order of nlines*(ncols+nrows) not nlines*ncols*nrows
// 

#define blkshift 7               /* places to shift rel position for cell num */
#define blkmask ((1<<blkshift)-1)/* mask for rel position within cell */
#define blkmargin 0              /* size guardband around map used */
                                 // jff 10/8/98 use guardband>0 
                                 // jff 10/12/98 0 ok with + 1 in rows,cols

typedef struct linelist_t        // type used to list lines in each block
{
  long num;
  struct linelist_t *next;
} linelist_t;

//
// Subroutine to add a line number to a block list
// It simply returns if the line is already in the block
//

static void AddBlockLine
(
  linelist_t **lists,
  int *count,
  int *done,
  int blockno,
  long lineno
)
{
  linelist_t *l;

  if (done[blockno])
    return;

  l = malloc(sizeof(linelist_t));
  l->num = lineno;
  l->next = lists[blockno];
  lists[blockno] = l;
  count[blockno]++;
  done[blockno] = 1;
}

//
// Actually construct the blockmap lump from the level data
//
// This finds the intersection of each linedef with the column and
// row lines at the left and bottom of each blockmap cell. It then
// adds the line to all block lists touching the intersection.
//

void P_CreateBlockMap()
{
  int xorg,yorg;                 // blockmap origin (lower left)
  int nrows,ncols;               // blockmap dimensions
  linelist_t **blocklists=NULL;  // array of pointers to lists of lines
  int *blockcount=NULL;          // array of counters of line lists
  int *blockdone=NULL;           // array keeping track of blocks/line
  int NBlocks;                   // number of cells = nrows*ncols
  long linetotal=0;              // total length of all blocklists
  int i,j;
  int map_minx=INT_MAX;          // init for map limits search
  int map_miny=INT_MAX;
  int map_maxx=INT_MIN;
  int map_maxy=INT_MIN;

  // scan for map limits, which the blockmap must enclose

  for (i=0;i<numvertexes;i++)
  {
    fixed_t t;

    if ((t=vertexes[i].x) < map_minx)
      map_minx = t;
    else if (t > map_maxx)
      map_maxx = t;
    if ((t=vertexes[i].y) < map_miny)
      map_miny = t;
    else if (t > map_maxy)
      map_maxy = t;
  }
  map_minx >>= FRACBITS;    // work in map coords, not fixed_t
  map_maxx >>= FRACBITS;
  map_miny >>= FRACBITS;
  map_maxy >>= FRACBITS;

  // set up blockmap area to enclose level plus margin

  xorg = map_minx-blkmargin;
  yorg = map_miny-blkmargin;
  ncols = (map_maxx+blkmargin-xorg+1+blkmask)>>blkshift;  //jff 10/12/98
  nrows = (map_maxy+blkmargin-yorg+1+blkmask)>>blkshift;  //+1 needed for
  NBlocks = ncols*nrows;                                  //map exactly 1 cell

  // create the array of pointers on NBlocks to blocklists
  // also create an array of linelist counts on NBlocks
  // finally make an array in which we can mark blocks done per line

  blocklists = malloc(NBlocks*sizeof(linelist_t *));
  memset(blocklists,0,NBlocks*sizeof(linelist_t *));
  blockcount = malloc(NBlocks*sizeof(int));
  memset(blockcount,0,NBlocks*sizeof(int));
  blockdone = malloc(NBlocks*sizeof(int));

  // initialize each blocklist, and enter the trailing -1 in all blocklists
  // note the linked list of lines grows backwards

  for (i=0;i<NBlocks;i++)
  {
    blocklists[i] = malloc(sizeof(linelist_t));
    blocklists[i]->num = -1;
    blocklists[i]->next = NULL;
    blockcount[i]++;
  }

  // For each linedef in the wad, determine all blockmap blocks it touches,
  // and add the linedef number to the blocklists for those blocks

  for (i=0;i<numlines;i++)
  {
    int x1 = lines[i].v1->x>>FRACBITS;         // lines[i] map coords
    int y1 = lines[i].v1->y>>FRACBITS;
    int x2 = lines[i].v2->x>>FRACBITS;
    int y2 = lines[i].v2->y>>FRACBITS;
    int dx = x2-x1;
    int dy = y2-y1;
    int vert = !dx;                            // lines[i] slopetype
    int horiz = !dy;
    int spos = (dx^dy) > 0;
    int sneg = (dx^dy) < 0;
    int bx,by;                                 // block cell coords
    int minx = x1>x2? x2 : x1;                 // extremal lines[i] coords
    int maxx = x1>x2? x1 : x2;
    int miny = y1>y2? y2 : y1;
    int maxy = y1>y2? y1 : y2;

    // no blocks done for this linedef yet

    memset(blockdone,0,NBlocks*sizeof(int));

    // The line always belongs to the blocks containing its endpoints

    bx = (x1-xorg)>>blkshift;
    by = (y1-yorg)>>blkshift;
    AddBlockLine(blocklists,blockcount,blockdone,by*ncols+bx,i);
    bx = (x2-xorg)>>blkshift;
    by = (y2-yorg)>>blkshift;
    AddBlockLine(blocklists,blockcount,blockdone,by*ncols+bx,i);


    // For each column, see where the line along its left edge, which 
    // it contains, intersects the Linedef i. Add i to each corresponding
    // blocklist.

    if (!vert)    // don't interesect vertical lines with columns
    {
      for (j=0;j<ncols;j++)
      {
        // intersection of Linedef with x=xorg+(j<<blkshift)
        // (y-y1)*dx = dy*(x-x1)
        // y = dy*(x-x1)+y1*dx;

        int x = xorg+(j<<blkshift);       // (x,y) is intersection
        int y = (dy*(x-x1))/dx+y1;
        int yb = (y-yorg)>>blkshift;      // block row number
        int yp = (y-yorg)&blkmask;        // y position within block

        if (yb<0 || yb>nrows-1)     // outside blockmap, continue
          continue;

        if (x<minx || x>maxx)       // line doesn't touch column
          continue;

        // The cell that contains the intersection point is always added

        AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j,i);

        // if the intersection is at a corner it depends on the slope
        // (and whether the line extends past the intersection) which 
        // blocks are hit

        if (yp==0)        // intersection at a corner
        {
          if (sneg)       //   \ - blocks x,y-, x-,y
          {
            if (yb>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(yb-1)+j,i);
            if (j>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j-1,i);
          }
          else if (spos)  //   / - block x-,y-
          {
            if (yb>0 && j>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(yb-1)+j-1,i);
          }
          else if (horiz) //   - - block x-,y
          {
            if (j>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j-1,i);
          }
        }
        else if (j>0 && minx<x) // else not at corner: x-,y
          AddBlockLine(blocklists,blockcount,blockdone,ncols*yb+j-1,i);
      }
    }

    // For each row, see where the line along its bottom edge, which 
    // it contains, intersects the Linedef i. Add i to all the corresponding
    // blocklists.

    if (!horiz)
    {
      for (j=0;j<nrows;j++)
      {
        // intersection of Linedef with y=yorg+(j<<blkshift)
        // (x,y) on Linedef i satisfies: (y-y1)*dx = dy*(x-x1)
        // x = dx*(y-y1)/dy+x1;

        int y = yorg+(j<<blkshift);       // (x,y) is intersection
        int x = (dx*(y-y1))/dy+x1;
        int xb = (x-xorg)>>blkshift;      // block column number
        int xp = (x-xorg)&blkmask;        // x position within block

        if (xb<0 || xb>ncols-1)   // outside blockmap, continue
          continue;

        if (y<miny || y>maxy)     // line doesn't touch row
          continue;

        // The cell that contains the intersection point is always added

        AddBlockLine(blocklists,blockcount,blockdone,ncols*j+xb,i);

        // if the intersection is at a corner it depends on the slope
        // (and whether the line extends past the intersection) which 
        // blocks are hit

        if (xp==0)        // intersection at a corner
        {
          if (sneg)       //   \ - blocks x,y-, x-,y
          {
            if (j>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb,i);
            if (xb>0 && minx<x)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*j+xb-1,i);
          }
          else if (vert)  //   | - block x,y-
          {
            if (j>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb,i);
          }
          else if (spos)  //   / - block x-,y-
          {
            if (xb>0 && j>0 && miny<y)
              AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb-1,i);
          }
        }
        else if (j>0 && miny<y) // else not on a corner: x,y-
          AddBlockLine(blocklists,blockcount,blockdone,ncols*(j-1)+xb,i);
      }
    }
  }

  // Add initial 0 to all blocklists
  // count the total number of lines (and 0's and -1's)

  memset(blockdone,0,NBlocks*sizeof(int));
  for (i=0,linetotal=0;i<NBlocks;i++)
  {
    AddBlockLine(blocklists,blockcount,blockdone,i,0);
    linetotal += blockcount[i];
  }

  // Create the blockmap lump

  blockmaplump = Z_Malloc(sizeof(*blockmaplump) * (4+NBlocks+linetotal),
                          PU_LEVEL, 0);
  // blockmap header

  blockmaplump[0] = bmaporgx = xorg << FRACBITS;
  blockmaplump[1] = bmaporgy = yorg << FRACBITS;
  blockmaplump[2] = bmapwidth  = ncols;
  blockmaplump[3] = bmapheight = nrows;

  // offsets to lists and block lists

  for (i=0;i<NBlocks;i++)
  {
    linelist_t *bl = blocklists[i];
    long offs = blockmaplump[4+i] =   // set offset to block's list
      (i? blockmaplump[4+i-1] : 4+NBlocks) + (i? blockcount[i-1] : 0);

    // add the lines in each block's list to the blockmaplump
    // delete each list node as we go

    while (bl)
    {
      linelist_t *tmp = bl->next;
      blockmaplump[offs++] = bl->num;
      free(bl);
      bl = tmp;
    }
  }

  // free all temporary storage

  free (blocklists);
  free (blockcount);
  free (blockdone);
}

// jff 10/6/98
// End new code added to speed up calculation of internal blockmap

#if 0   /*jff 10/6/98 this code superseded by faster blockmap routine above */

// killough 5/3/98: tests whether a linedef is inside a block.
static boolean IsLineDefInside(const line_t *l, int x, int y)
{
  int dx = l->dx >> FRACBITS;
  int dy = l->dy >> FRACBITS;
  int a = (l->v1->x >> FRACBITS) - x;
  int b = (l->v1->y >> FRACBITS) - y;

  if (abs(a*2+dx)-abs(dx) > MAPBLOCKUNITS ||
      abs(b*2+dy)-abs(dy) > MAPBLOCKUNITS )
    return 0;

  a *= dy;
  b *= dx;
  a -= b;
  b = dx + dy;
  b <<= MAPBTOFRAC-1;
  if (((a-b)^(a+b)) < 0)
    return 1;
  dy -= dx;
  dy <<= MAPBTOFRAC-1;
  b = a+dy;
  a -= dy;
  return (a^b) < 0;
}

static void P_CreateBlockMap(void)
{
  long minx = LONG_MAX, miny = LONG_MAX, maxx = LONG_MIN, maxy = LONG_MIN;
  long i, tot, *bmap, nalloc, bx, by, count;

  // First find limits of map

  for (i=0; i<numlines; i++)
    {
      vertex_t *v;
      fixed_t t;
      v = lines[i].v1;
      t = v->x >> FRACBITS;
      if (t < minx)
        minx = t;
      else
        if (t > maxx)
          maxx = t;
      t = v->y >> FRACBITS;
      if (t < miny)
        miny = t;
      else
        if (t > maxy)
          maxy = t;
      v = lines[i].v2;
      t = v->x >> FRACBITS;
      if (t < minx)
        minx = t;
      else
        if (t > maxx)
          maxx = t;
      t = v->y >> FRACBITS;
      if (t < miny)
        miny = t;
      else
        if (t > maxy)
          maxy = t;
    }

  bmaporgx = minx << FRACBITS;
  bmaporgy = miny << FRACBITS;
  bmapwidth  = ((maxx-minx) >> MAPBTOFRAC) + 1;
  bmapheight = ((maxy-miny) >> MAPBTOFRAC) + 1;
  count = tot = bmapwidth * bmapheight + 4;
  nalloc = tot + numlines*2 + 2;
  bmap = malloc(sizeof(*bmap) * nalloc);

  // Brute-force algorithm for now -- a better way would be a Bresenham-like
  // traversal of the linedef, through the blocks it touches, moving either
  // horizontally or vertically one block at each step. killough 3/30/98

  minx += MAPBLOCKUNITS/2;
  miny += MAPBLOCKUNITS/2;
  for (i=4, by=0; by<bmapheight; by++, miny += MAPBLOCKUNITS)
    {
      long x = minx;
      for (bx=0; bx<bmapwidth; bx++, i++, x += MAPBLOCKUNITS)
        {
          int j;
          bmap[i] = count;
          if (count+numlines+2 >= nalloc)
            bmap = realloc(bmap, sizeof(*bmap) * (nalloc*=2));
          bmap[count++] = 0;
          for (j=0; j<numlines; j++)
            if (IsLineDefInside(lines+j, x, miny))
              bmap[count++] = j;
          bmap[count++] = -1;
        }
    }
  blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);
  memcpy(blockmaplump, bmap, count * sizeof(*bmap));
  free(bmap);
}

#endif

//
// P_LoadBlockMap
//
// killough 3/1/98: substantially modified to work
// towards removing blockmap limit (a wad limitation)
//
// killough 3/30/98: Rewritten to remove blockmap limit,
// though current algorithm is brute-force and unoptimal.
//

void P_LoadBlockMap (int lump)
{
  long count;

  if (M_CheckParm("-blockmap") || (count = W_LumpLength(lump)/2) >= 0x10000)
    P_CreateBlockMap();
  else
    {
      long i;
      short *wadblockmaplump = W_CacheLumpNum (lump, PU_LEVEL);
      blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);

      // killough 3/1/98: Expand wad blockmap into larger internal one,
      // by treating all offsets except -1 as unsigned and zero-extending
      // them. This potentially doubles the size of blockmaps allowed,
      // because Doom originally considered the offsets as always signed.

      blockmaplump[0] = SHORT(wadblockmaplump[0]);
      blockmaplump[1] = SHORT(wadblockmaplump[1]);
      blockmaplump[2] = (long)(SHORT(wadblockmaplump[2])) & 0xffff;
      blockmaplump[3] = (long)(SHORT(wadblockmaplump[3])) & 0xffff;

      for (i=4 ; i<count ; i++)
        {
          short t = SHORT(wadblockmaplump[i]);          // killough 3/1/98
          blockmaplump[i] = t == -1 ? -1l : (long) t & 0xffff;
        }

      Z_Free(wadblockmaplump);

      bmaporgx = blockmaplump[0]<<FRACBITS;
      bmaporgy = blockmaplump[1]<<FRACBITS;
      bmapwidth = blockmaplump[2];
      bmapheight = blockmaplump[3];
    }

  // clear out mobj chains
  count = sizeof(*blocklinks)* bmapwidth*bmapheight;
  blocklinks = Z_Malloc (count,PU_LEVEL, 0);
  memset (blocklinks, 0, count);
  blockmap = blockmaplump+4;
}

//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
// killough 5/3/98: reformatted, cleaned up

void P_GroupLines (void)
{
  line_t **linebuffer;
  register line_t *li;
  int i, total = 0;

  // look up sector number for each subsector
  for (i=0; i<numsubsectors; i++)
    subsectors[i].sector = segs[subsectors[i].firstline].sidedef->sector;

  // count number of lines in each sector
  for (i=0,li=lines; i<numlines; i++, li++)
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
  linebuffer = Z_Malloc(total*4, PU_LEVEL, 0);

  for (i=0; i<numsectors; i++)
    {
      fixed_t  bbox[4];
      sector_t *sector = sectors+i;
      int      block, j;

      M_ClearBox(bbox);
      sector->lines = linebuffer;
      for (j=0, li=lines; j<numlines; j++, li++)
        if (li->frontsector == sector || li->backsector == sector)
          {
            *linebuffer++ = li;
            M_AddToBox (bbox, li->v1->x, li->v1->y);
            M_AddToBox (bbox, li->v2->x, li->v2->y);
          }

      if (linebuffer - sector->lines != sector->linecount)
        I_Error("P_GroupLines: miscounted");

      // set the degenmobj_t to the middle of the bounding box
      sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
      sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;

      // adjust bounding box to map blocks
      block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
      block = block >= bmapheight ? bmapheight-1 : block;
      sector->blockbox[BOXTOP]=block;

      block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
      block = block < 0 ? 0 : block;
      sector->blockbox[BOXBOTTOM]=block;

      block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
      block = block >= bmapwidth ? bmapwidth-1 : block;
      sector->blockbox[BOXRIGHT]=block;

      block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
      block = block < 0 ? 0 : block;
      sector->blockbox[BOXLEFT]=block;
    }
}


//
// P_SetupLevel
//
// killough 5/3/98: reformatted, cleaned up

void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
  int   i;
  char  lumpname[9];
  int   lumpnum;

  totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
  wminfo.partime = 180;
  for (i=0; i<MAXPLAYERS; i++)
    players[i].killcount = players[i].secretcount = players[i].itemcount = 0;

  // Initial height of PointOfView will be set by player think.
  players[consoleplayer].viewz = 1;

  // Make sure all sounds are stopped before Z_FreeTags.
  S_Start();

  Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1);

  P_InitThinkers();

  // if working with a devlopment map, reload it
  //    W_Reload ();     killough 1/31/98: W_Reload obsolete

  // find map name
  if (gamemode == commercial)
    sprintf(lumpname, "map%02d", map);           // killough 1/24/98: simplify
  else
    sprintf(lumpname, "E%dM%d", episode, map);   // killough 1/24/98: simplify

  lumpnum = W_GetNumForName(lumpname);

  leveltime = 0;

  // note: most of this ordering is important

  // killough 3/1/98: P_LoadBlockMap call moved down to below
  // killough 4/4/98: split load of sidedefs into two parts,
  // to allow texture names to be used in special linedefs

  P_LoadVertexes  (lumpnum+ML_VERTEXES);
  P_LoadSectors   (lumpnum+ML_SECTORS);
  P_LoadSideDefs  (lumpnum+ML_SIDEDEFS);             // killough 4/4/98
  P_LoadLineDefs  (lumpnum+ML_LINEDEFS);             //       |
  P_LoadSideDefs2 (lumpnum+ML_SIDEDEFS);             //       |
  P_LoadLineDefs2 (lumpnum+ML_LINEDEFS);             // killough 4/4/98
  P_LoadBlockMap  (lumpnum+ML_BLOCKMAP);             // killough 3/1/98
  P_LoadSubsectors(lumpnum+ML_SSECTORS);
  P_LoadNodes     (lumpnum+ML_NODES);
  P_LoadSegs      (lumpnum+ML_SEGS);

  rejectmatrix = W_CacheLumpNum(lumpnum+ML_REJECT,PU_LEVEL);
  P_GroupLines();

  bodyqueslot = 0;

// phares 8/10/98: Clear body queue so the corpses from previous games are
// not assumed to be from this one. The mobj_t's belonging to these corpses
// are cleared in the normal freeing of zoned memory between maps, so all
// we have to do here is clear the pointers to them.

  if (bodyque)
    for (i = 0 ; i < bodyquesize ; i++)
      bodyque[i] = 0;

  deathmatch_p = deathmatchstarts;
  P_LoadThings(lumpnum+ML_THINGS);

  // if deathmatch, randomly spawn the active players
  if (deathmatch)
    for (i=0; i<MAXPLAYERS; i++)
      if (playeringame[i])
        {
          players[i].mo = NULL;
          G_DeathMatchSpawnPlayer(i);
        }

  // killough 3/26/98: Spawn icon landings:
  if (gamemode==commercial)
    P_SpawnBrainTargets();

  // clear special respawning que
  iquehead = iquetail = 0;

  // set up world state
  P_SpawnSpecials();

  // preload graphics
  if (precache)
    R_PrecacheLevel();
}

//
// P_Init
//
void P_Init (void)
{
  P_InitSwitchList();
  P_InitPicAnims();
  R_InitSprites(sprnames);
}

//----------------------------------------------------------------------------
//
// $Log: p_setup.c,v $
// Revision 1.21  1998/10/13  03:19:21  jim
// Rand's segadjust chosen, Blockmap tweak
//
// Revision 1.18  1998/10/05  21:29:21  phares
// Fixed firelines
//
// Revision 1.17  1998/08/11  19:32:07  phares
// DM Weapon bug fix
//
// Revision 1.16  1998/05/07  00:56:49  killough
// Ignore translucency lumps that are not exactly 64K long
//
// Revision 1.15  1998/05/03  23:04:01  killough
// beautification
//
// Revision 1.14  1998/04/12  02:06:46  killough
// Improve 242 colomap handling, add translucent walls
//
// Revision 1.13  1998/04/06  04:47:05  killough
// Add support for overloading sidedefs for special uses
//
// Revision 1.12  1998/03/31  10:40:42  killough
// Remove blockmap limit
//
// Revision 1.11  1998/03/28  18:02:51  killough
// Fix boss spawner savegame crash bug
//
// Revision 1.10  1998/03/20  00:30:17  phares
// Changed friction to linedef control
//
// Revision 1.9  1998/03/16  12:35:36  killough
// Default floor light level is sector's
//
// Revision 1.8  1998/03/09  07:21:48  killough
// Remove use of FP for point/line queries and add new sector fields
//
// Revision 1.7  1998/03/02  11:46:10  killough
// Double blockmap limit, prepare for when it's unlimited
//
// Revision 1.6  1998/02/27  11:51:05  jim
// Fixes for stairs
//
// Revision 1.5  1998/02/17  22:58:35  jim
// Fixed bug of vanishinb secret sectors in automap
//
// Revision 1.4  1998/02/02  13:38:48  killough
// Comment out obsolete reload hack
//
// Revision 1.3  1998/01/26  19:24:22  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/26  05:02:21  killough
// Generalize and simplify level name generation
//
// Revision 1.1.1.1  1998/01/19  14:03:00  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
