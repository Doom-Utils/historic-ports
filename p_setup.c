//  
// DOSDoom Level Setup Code  
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#include <math.h>

#include "z_zone.h"

#include "m_swap.h"
#include "m_bbox.h"
#include "m_fixed.h"

#include "ddf_main.h"
#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "dm_defs.h"
#include "p_local.h"

#include "s_sound.h"

#include "dm_state.h"

void P_SpawnMapThing (mapthing_t* mthing);

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int		numvertexes;
vertex_t*	vertexes;

int		numsegs;
seg_t*		segs;

int		numsectors;
sector_t*	sectors;

int		numsubsectors;
subsector_t*	subsectors;

int		numnodes;
node_t*		nodes;

int		numlines;
line_t*		lines;

int		numsides;
side_t*		sides;


// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
// 23-6-98 KM Promotion of short* to int*
int		bmapwidth;
int		bmapheight;	// size in mapblocks
int*		blockmap;	// int for larger maps

// offsets in blockmap are from here
int*		blockmaplump;

// origin of block map
fixed_t		bmaporgx;
fixed_t		bmaporgy;

// for thing chains
mobj_t**	blocklinks;		


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*		rejectmatrix;


// Maintain single and multi player starting spots.
int		max_deathmatch_starts = 10;
mapthing_t*	deathmatchstarts;
mapthing_t*	deathmatch_p;
//mapthing_t	playerstarts[MAXPLAYERS];
mapthing_t*	playerstarts;





//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    byte*		data;
    int			i;
    mapvertex_t*	ml;
    vertex_t*		li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
	li->x = SHORT(ml->x)<<FRACBITS;
	li->y = SHORT(ml->y)<<FRACBITS;
    }

    // Free buffer memory.
    Z_Free (data);
}



//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    byte*		data;
    int			i;
    mapseg_t*		ml;
    seg_t*		li;
    line_t*		ldef;
    int			linedef;
    int			side;

    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
	li->v1 = &vertexes[SHORT(ml->v1)];
	li->v2 = &vertexes[SHORT(ml->v2)];
					
	li->angle = (SHORT(ml->angle))<<16;
	li->offset = (SHORT(ml->offset))<<16;
	linedef = SHORT(ml->linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = SHORT(ml->side);
	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;
	if (ldef-> flags & ML_TWOSIDED)
	    li->backsector = sides[ldef->sidenum[side^1]].sector;
	else
	    li->backsector = 0;
    }
	
    Z_Free (data);
}


//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    byte*		data;
    int			i;
    mapsubsector_t*	ms;
    subsector_t*	ss;
	
    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;
    
    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
	ss->numlines = SHORT(ms->numsegs);
	ss->firstline = SHORT(ms->firstseg);
    }
	
    Z_Free (data);
}



//
// P_LoadSectors
//
void P_LoadSectors (int lump)
{
    byte*		data;
    int			i;
    mapsector_t*	ms;
    sector_t*		ss;
	
    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);	
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ms = (mapsector_t *)data;
    ss = sectors;
    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
	ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
	ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
	ss->floorpic = R_FlatNumForName(ms->floorpic);
	ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
	ss->lightlevel = SHORT(ms->lightlevel);
	ss->special = SHORT(ms->special);
	ss->tag = SHORT(ms->tag);
	ss->thinglist = NULL;
    }
	
    Z_Free (data);
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    byte*	data;
    int		i;
    int		j;
    int		k;
    mapnode_t*	mn;
    node_t*	no;
	
    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    mn = (mapnode_t *)data;
    no = nodes;
    
    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;
	for (j=0 ; j<2 ; j++)
	{
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
void P_LoadThings (int lump)
{
    byte*		data;
    int			i;
    mapthing_t*		mt;
    int			numthings;
	
    data = W_CacheLumpNum (lump,PU_STATIC);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);
	
    mt = (mapthing_t *)data;
    for (i=0 ; i<numthings ; i++, mt++)
    {
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
//
void P_LoadLineDefs (int lump)
{
    byte*		data;
    int			i;
    maplinedef_t*	mld;
    line_t*		ld;
    vertex_t*		v1;
    vertex_t*		v2;
	
    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);	
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    mld = (maplinedef_t *)data;
    ld = lines;
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
	ld->flags = SHORT(mld->flags);
	ld->special = SHORT(mld->special);
	ld->tag = SHORT(mld->tag);
	v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
	v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;
	
	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv (ld->dy , ld->dx) > 0)
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

	if (ld->sidenum[0] != -1)
	    ld->frontsector = sides[ld->sidenum[0]].sector;
	else
	    ld->frontsector = 0;

	if (ld->sidenum[1] != -1)
	    ld->backsector = sides[ld->sidenum[1]].sector;
	else
	    ld->backsector = 0;
    }
	
    Z_Free (data);
}


//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    byte*		data;
    int			i;
    mapsidedef_t*	msd;
    side_t*		sd;
	
    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);	
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
	sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
	sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
	sd->toptexture = R_TextureNumForName(msd->toptexture);
	sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
	sd->midtexture = R_TextureNumForName(msd->midtexture);
	sd->sector = &sectors[SHORT(msd->sector)];
    }
	
    Z_Free (data);
}


//
// P_LoadBlockMap
//

// 23-6-98 KM Brand Spanking New Blockmap code:
// I think there is a bug somewhere in P_GenerateBlockMap, so it
// is commented out, but since P_LoadBlockMap should be able
// to generate any level that doom can take, no problemo.

void P_LoadBlockMap (int lump)
{
    int		i;
    int		count;
    short*	bmap;
  	
    bmap = W_CacheLumpNum (lump, PU_STATIC);
    count = W_LumpLength (lump)/sizeof(short);
    blockmaplump = Z_Malloc(count * sizeof(int), PU_LEVEL, NULL);
    blockmap = blockmaplump+4;
  
  
    for (i = 0; i < 4; i++)
       blockmaplump[i] = (int) (unsigned short) SHORT(bmap[i]);
  
    bmaporgx = blockmaplump[0]<<FRACBITS;
    bmaporgy = blockmaplump[1]<<FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];
  
    for (; i < (bmapwidth * bmapheight); i++)
        blockmaplump[i] = (int) (unsigned short) SHORT(bmap[i]);

    for (; i<count ; i++)
	blockmaplump[i] = (int) (signed short) SHORT(bmap[i]);

    Z_Free(bmap);
}

void P_GenerateBlockMap(int min_x, int min_y, int max_x, int max_y)
{
    int	 	count;
    int**	bmap;
    int*	bmaplen;
    int*	maxbmaplen;
    int		i, j;
    int		size;

    int		x0, x1, y0, y1;
    int		blocknum;

    int		blockstep;
    boolean	column, row;

    int		x_sign, y_sign;
    fixed_t	slope, x_jump, next_x, this_x;
    int		lastblock;

    line_t*	line;

    int		pointer_p;
    int		blockmap_p;

    bmaporgx = min_x - 8;
    bmaporgy = min_y - 8;
    bmapwidth = (max_x - bmaporgx + 127) / 128;
    bmapheight = (max_y - bmaporgy + 127) / 128;
    bmaporgx <<= FRACBITS;
    bmaporgy <<= FRACBITS;

    count = bmapwidth * bmapheight;

    bmap = Z_Malloc(count * sizeof(int *), PU_STATIC, NULL);
    bmaplen = Z_Malloc(count * sizeof(int), PU_STATIC, NULL);
    maxbmaplen = Z_Malloc(count * sizeof(int), PU_STATIC, NULL);
    size = count * sizeof(int *);
    for (i = count; i--;)
    {
       bmap[i] = Z_Malloc(sizeof(int) * 16, PU_STATIC, NULL);
       bmap[i][0] = 0;
       bmaplen[i] = 1;
       maxbmaplen[i] = 16;
       size += sizeof(int);
    }

    for (i = 0; i < numlines; i++)
    {
      line = &lines[i];
      x0 = (line->v1->x >> FRACBITS) - (bmaporgx >> FRACBITS);
      x1 = (line->v2->x >> FRACBITS) - (bmaporgx >> FRACBITS);
      y0 = (line->v1->y >> FRACBITS) - (bmaporgy >> FRACBITS);
      y1 = (line->v2->y >> FRACBITS) - (bmaporgy >> FRACBITS);

      blocknum = (y0 / 128) * bmapwidth + (x0 / 128);
      if (bmaplen[blocknum] == maxbmaplen[blocknum])
      {
        bmap[blocknum] = Z_ReMalloc(bmap[blocknum], (maxbmaplen[blocknum] + 16) * sizeof(int));
        maxbmaplen[blocknum] += 16;
      }
      size += sizeof(int);
      bmap[blocknum][bmaplen[blocknum]] = i;
      bmaplen[blocknum]++;

      // Check if this line spans multiple blocks.
      column = ((x0 / 128) == (x1 / 128));
      row = ((y0 / 128) == (y1 / 128));
      switch(column * 2 + row)
      {
        // Entirely in one block
        case 3:
             break;
        case 2:
             blockstep = (y1 >= y0) ? bmapwidth : -bmapwidth;
             for (j = 0; j < abs((y1 / 128) - (y0 / 128)); j++)
             {
                blocknum += blockstep;
                if (bmaplen[blocknum] == maxbmaplen[blocknum])
                {
                  bmap[blocknum] = Z_ReMalloc(bmap[blocknum], (maxbmaplen[blocknum] + 16) * sizeof(int));
                  maxbmaplen[blocknum] += 16;
                }
                size += sizeof(int);
                bmap[blocknum][bmaplen[blocknum]] = i;
                bmaplen[blocknum]++;
             }
             break;
        case 1:
             blockstep = (x1 >= x0) ? 1 : -1;
             for (j = 0; j < abs((x1 / 128) - (x0 / 128)); j++)
             {
                blocknum += blockstep;
                if (bmaplen[blocknum] == maxbmaplen[blocknum])
                {
                  bmap[blocknum] = Z_ReMalloc(bmap[blocknum], (maxbmaplen[blocknum] + 16) * sizeof(int));
                  maxbmaplen[blocknum] += 16;
                }
                size += sizeof(int);
                bmap[blocknum][bmaplen[blocknum]] = i;
                bmaplen[blocknum]++;
             }
             break;
         default:
             y_sign = (y1 >= y0) ? 1 : -1;
             x_sign = (x1 >= x0) ? 1 : -1;

             slope = ((y1-y0)<<FRACBITS)/(x1-x0);
             x_jump = y_sign * FixedDiv(128 << FRACBITS, slope);
             if (y_sign < 0)
               next_x = (x0 << FRACBITS) + FixedDiv(((y0 / 128) * 128 - 1 - y0)<<FRACBITS, slope);
             else
               next_x = (x0 << FRACBITS) + FixedDiv(((y0 / 128) * 128 + 128 - y0)<<FRACBITS, slope);

             lastblock = next_x / (128 << FRACBITS) - (x0 / 128) + blocknum;
             if (lastblock > blocknum)
             {
               for (j = blocknum + x_sign; j <= lastblock; j += x_sign)
               {
                  if (bmaplen[j] == maxbmaplen[j])
                  {
                    bmap[j] = Z_ReMalloc(bmap[j], (maxbmaplen[j] + 16) * sizeof(int));
                    maxbmaplen[j] += 16;
                  }
                  size += sizeof(int);
                  bmap[j][bmaplen[j]] = i;
                  bmaplen[j]++;
               }
	     }

             do
             {
               this_x = next_x;
               next_x = this_x + x_jump;
               if ((x_sign * next_x) > (x_sign * x1))
               {
                  j = lastblock + y_sign * bmapwidth;
                  lastblock = j + next_x/(128 << FRACBITS) - this_x/(128 << FRACBITS);
                  for (; j <= lastblock; j += x_sign)
                  {
                     if (bmaplen[j] == maxbmaplen[j])
                     {
                       bmap[j] = Z_ReMalloc(bmap[j], (maxbmaplen[j] + 16) * sizeof(int));
                       maxbmaplen[j] += 16;
                     }
                     size += sizeof(int);
                     bmap[j][bmaplen[j]] = i;
                     bmaplen[j]++;
                  }
               }
             } while ((next_x>>FRACBITS) != x1);
             break;
      }
    }


    size += sizeof(int) * (4 + count);
    blockmaplump = Z_Malloc(size, PU_LEVEL, &blockmaplump);

    blockmaplump[0] = min_x - 8;
    blockmaplump[1] = min_y - 8;
    blockmaplump[2] = bmapwidth;
    blockmaplump[3] = bmapheight;

    blockmap = blockmaplump + 4;
    pointer_p = 4;
    blockmap_p = 4 + count;

    for (i = 0; i < count; i++)
    {
	blockmaplump[pointer_p] = blockmap_p;
        for (j = 0; j < bmaplen[i]; j++)
           blockmaplump[blockmap_p + j] = bmap[i][j];
        blockmaplump[blockmap_p + j] = -1;
        blockmap_p += bmaplen[i] + 1;
        pointer_p++;
        Z_Free(bmap[i]);
    }
    Z_Free(bmap);
    Z_Free(bmaplen);
    Z_Free(maxbmaplen);
}

void P_DoBlockMap (int lump)
{
    vertex_t*	v;
    int i, x, y;
    int min_x = vertexes[0].x >> FRACBITS, min_y = vertexes[0].y >> FRACBITS;
    int max_x = vertexes[0].x >> FRACBITS, max_y = vertexes[0].y >> FRACBITS;
    int map_width, map_height;
    int count;
    for (v = vertexes, i = numvertexes; i--; v++)
    {
       x = v->x >> FRACBITS;
       y = v->y >> FRACBITS;
       if (x < min_x) min_x = x;
       else if (x > max_x) max_x = x;
       if (y < min_y) min_y = y;
       else if (y > max_y) max_y = y;
    }
    map_width = max_x - min_x;
    map_height = max_y - min_y;
//    if ((map_width < 32768) && (map_height < 32768))
    P_LoadBlockMap(lump);
//    else
//      P_GenerateBlockMap(min_x, min_y, max_x, max_y);

     // clear out mobj chains
     count = sizeof(*blocklinks)* bmapwidth*bmapheight;
     blocklinks = Z_Malloc (count,PU_LEVEL, NULL);
     memset (blocklinks, 0, count);
}



//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    line_t**		linebuffer;
    int			i;
    int			j;
    int			total;
    line_t*		li;
    sector_t*		sector;
    subsector_t*	ss;
    seg_t*		seg;
    fixed_t		bbox[4];
    int			block;
	
    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
	seg = &segs[ss->firstline];
	ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    total = 0;
    for (i=0 ; i<numlines ; i++, li++)
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
    linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
	M_ClearBox (bbox);
	sector->lines = linebuffer;
	li = lines;
	for (j=0 ; j<numlines ; j++, li++)
	{
	    if (li->frontsector == sector || li->backsector == sector)
	    {
		*linebuffer++ = li;
		M_AddToBox (bbox, li->v1->x, li->v1->y);
		M_AddToBox (bbox, li->v2->x, li->v2->y);
	    }
	}
	if (linebuffer - sector->lines != sector->linecount)
	    I_Error ("P_GroupLines: miscounted");
			
	// set the degenmobj_t to the middle of the bounding box
	sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
	sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
        // -KM- 1999/01/31 Identify dummy mobjs for Doppler.
        sector->soundorg.thinker.function.acv = NULL;
		
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
// Sets up the current level using the skill passed and the
// information in currentmap.
//
// -ACB- 1998/08/09 Use currentmap to ref lump and par time
// -KM- 1998/11/25 Added autotag.
//
void P_SetupLevel (skill_t skill, int autotag)
{
  int i;
  int lumpnum;
	
  totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;

  wminfo.partime = currentmap->partime;

  for (i=0 ; i<maxplayers ; i++)
    players[i].killcount = players[i].secretcount = players[i].itemcount = 0;

  // Initial height of PointOfView
  // will be set by player think.
  players[consoleplayer].viewz = 1;

  Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

  P_InitThinkers ();

  // if working with a devlopment map, reload it
  W_Reload ();
	   
  lumpnum = W_GetNumForName (currentmap->lump);
	
  leveltime = 0;

  // -ACB- 1998/08/27 NULL the head pointers for the linked lists....
  itemquehead = NULL;
  mobjlisthead = NULL;

  // note: most of this ordering is important
  // 23-6-98 KM, eg, Sectors must be loaded before sidedefs,
  // Sidedefs + Vertexes must be loaded before LineDefs,
  // LineDefs + Vertexes must be loaded before BlockMap
  P_LoadVertexes (lumpnum+ML_VERTEXES);
  P_LoadSectors (lumpnum+ML_SECTORS);
  P_LoadSideDefs (lumpnum+ML_SIDEDEFS);
  P_LoadLineDefs (lumpnum+ML_LINEDEFS);
  P_DoBlockMap (lumpnum+ML_BLOCKMAP);
  P_LoadSubsectors (lumpnum+ML_SSECTORS);
  P_LoadNodes (lumpnum+ML_NODES);
  P_LoadSegs (lumpnum+ML_SEGS);
	
  rejectmatrix = W_CacheLumpNum (lumpnum+ML_REJECT,PU_LEVEL);
  P_GroupLines ();

  bodyqueslot = 0;
  deathmatch_p = deathmatchstarts;
  P_LoadThings (lumpnum+ML_THINGS);

  // if deathmatch, randomly spawn the active players
  if (deathmatch)
  {
    for (i=0 ; i<maxplayers ; i++)
    {
      if (playeringame[i])
      {
        players[i].mo = NULL;
        G_DeathMatchSpawnPlayer (i);
      }
    }
  }
	
  // set up world state
  P_SpawnSpecials (autotag);

  // preload graphics
  if (precache)
    R_PrecacheLevel ();

  // 25-6-98 -KM- Moved here so music plays while the rest of the level loads.
  S_Start ();

}

//
// P_Init
//
void P_Init (void)
{
    deathmatchstarts =
       Z_Malloc(sizeof(mapthing_t) * max_deathmatch_starts, PU_STATIC, NULL);
    playerstarts = Z_Malloc(maxplayers*sizeof(*playerstarts), PU_STATIC, NULL);
    P_InitSwitchList ();
    P_InitPicAnims ();
    R_InitSprites (sprnames);
}



