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
//	Implements special effects:
//	Texture animation, height or lighting changes
//	 according to adjacent sectors, respective
//	 utility functions, etc.
//	Line Tag handling. Line and Sector triggers.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "z_zone.h"
#include "m_argv.h"
#include "m_random.h"
#include "m_swap.h"
#include "w_wad.h"
#include "r_local.h"
#include "p_local.h"
#include "p_specplus.h"
#include "g_game.h"
#include "s_sound.h"
#include "r_state.h"
#include "sounds.h"
#include "m_comdev.h"

static void P_SpawnScrollers(void);	// Initialize generalized scrolling

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
    boolean	istexture;
    int		picnum;
    int		basepic;
    int		numpics;
    int		speed;
} anim_t;

#pragma pack (1)	// necessary because loaded from WAD now
//
//      source animation definition
//
typedef struct
{
    signed char	istexture;	// if false, it is a flat
    char	endname[9];
    char	startname[9];
    int		speed;
} animdef_t;

#ifdef IRIX
#pragma pack (0)
#else
#pragma pack ()
#endif

#define MAXANIMS	32	// no longer a strict limit

static anim_t	*lastanim;
static anim_t	*anims;
static size_t	maxanims;

//
// not used anymore, instead we use customizable lists from PWAD's
//

#if 0
// Floor/ceiling animation sequences,
//  defined by first and last frame,
//  i.e. the flat (64x64 tile) name to
//  be used.
// The full animation sequence is given
//  using all the flats between the start
//  and end entry, in the order found in
//  the WAD file.
//
animdef_t	animdefs[] =
{
    {false,	"NUKAGE3",	"NUKAGE1",	8},
    {false,	"FWATER4",	"FWATER1",	8},
    {false,	"SWATER4",	"SWATER1", 	8},
    {false,	"LAVA4",	"LAVA1",	8},
    {false,	"BLOOD3",	"BLOOD1",	8},

    // DOOM II flat animations.
    {false,	"RROCK08",	"RROCK05",	8},
    {false,	"SLIME04",	"SLIME01",	8},
    {false,	"SLIME08",	"SLIME05",	8},
    {false,	"SLIME12",	"SLIME09",	8},

    {true,	"BLODGR4",	"BLODGR1",	8},
    {true,	"SLADRIP3",	"SLADRIP1",	8},

    {true,	"BLODRIP4",	"BLODRIP1",	8},
    {true,	"FIREWALL",	"FIREWALA",	8},
    {true,	"GSTFONT3",	"GSTFONT1",	8},
    {true,	"FIRELAVA",	"FIRELAV3",	8},
    {true,	"FIREMAG3",	"FIREMAG1",	8},
    {true,	"FIREBLU2",	"FIREBLU1",	8},
    {true,	"ROCKRED3",	"ROCKRED1",	8},

    {true,	"BFALL4",	"BFALL1",	8},
    {true,	"SFALL4",	"SFALL1",	8},
    {true,	"WFALL4",	"WFALL1",	8},
    {true,	"DBRAIN4",	"DBRAIN1",	8},

    {-1}
};
#endif

//
//      Animating line specials
//
// not used anymore, replaced with thinker based scrollers from Boom
//

#if 0
#define MAXLINEANIMS            64

extern short	numlinespecials;
extern line_t	*linespeciallist[MAXLINEANIMS];
#endif

//
// P_InitPicAnims
//
// Load the table of animation definitions, checking for existence of
// the start and end of each frame. If the start doesn't exist the sequence
// is skipped, if the last doesn't exist, XDoom exits.
//
// Wall/Flat animation sequences, defined by name of first and last frame,
// The full animation sequence is given using all lumps between the start
// and end entry, in the order found in the WAD file.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called ANIMATED rather than a static table in this module to
// allow wad designers to insert or modify animation sequences.
//
// Lump format is an array of byte packed animdef_t structures, terminated
// by a structure with istexture == -1. The lump can be generated from a
// text source file using SWANTBLS, distributed with the xwadtools utils.
// The standard list of switches and animations is contained in the example
// source text file defswani.dat also in the xwadtools util distribution.
//
// This was taken from Boom sources.
//
void P_InitPicAnims(void)
{
    int			i;
    animdef_t		*animdefs;	// pointer to animation lump
    int			lump = W_GetNumForName("ANIMATED");

    // read from wad lump instead of table
    animdefs = (animdef_t *)W_CacheLumpNum(lump, PU_CACHE);

    //	Init animation
    lastanim = anims;
    for (i = 0; animdefs[i].istexture != -1; i++)
    {
	// remove limit by array doubling
	if (lastanim >= anims + maxanims)
	{
	    size_t	newmax = maxanims ? maxanims * 2 : MAXANIMS;

	    anims = Z_Realloc(anims, newmax * sizeof(*anims), PU_STATIC,
			      (void *)0);
	    lastanim = anims + maxanims;
	    maxanims = newmax;
	}

	if (animdefs[i].istexture)
	{
	    // different episode ?
	    if (R_CheckTextureNumForName(animdefs[i].startname) == -1)
		continue;

	    lastanim->picnum = R_TextureNumForName(animdefs[i].endname);
	    lastanim->basepic = R_TextureNumForName(animdefs[i].startname);
	}
	else
	{
	    if (W_CheckNumForName(animdefs[i].startname) == -1)
		continue;

	    lastanim->picnum = R_FlatNumForName(animdefs[i].endname);
	    lastanim->basepic = R_FlatNumForName(animdefs[i].startname);
	}

	lastanim->istexture = animdefs[i].istexture;
	lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;

	if (lastanim->numpics < 2)
	    I_Error("P_InitPicAnims: bad cycle from %s to %s",
		     animdefs[i].startname,
		     animdefs[i].endname);

	lastanim->speed = LONG(animdefs[i].speed);
	lastanim++;
    }
}

//
// UTILITIES
//

//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t *getSide(int currentSector, int line, int side)
{
    return &sides[(sectors[currentSector].lines[line])->sidenum[side]];
}

//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t *getSector(int currentSector, int line, int side)
{
    return sides[(sectors[currentSector].lines[line])->sidenum[side]].sector;
}

//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
int twoSided(int sector, int line)
{
    return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
sector_t *getNextSector(line_t *line, sector_t *sec)
{
    if (!(line->flags & ML_TWOSIDED))
	return NULL;

    if (line->frontsector == sec)
	return line->backsector;

    return line->frontsector;
}

//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindLowestFloorSurrounding(sector_t *sec)
{
    int			i;
    sector_t		*other;
    fixed_t		floor = sec->floorheight;

    for (i = 0; i < sec->linecount; i++)
    {
	if ((other = getNextSector(sec->lines[i], sec)) &&
	    (other->floorheight < floor))
	        floor = other->floorheight;
    }
    return floor;
}

//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec)
{
    int			i;
    sector_t		*other;
    fixed_t		floor = -500 * FRACUNIT;

    for (i = 0; i < sec->linecount; i++)
    {
	if ((other = getNextSector(sec->lines[i], sec)) &&
	    (other->floorheight > floor))
	        floor = other->floorheight;
    }
    return floor;
}

//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
// Note: this should be doable w/o a fixed array.

#if 0
// 20 adjoining sectors max!
#define MAX_ADJOINING_SECTORS    	20

fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
    int			i;
    int			h;
    int			min;
    line_t		*check;
    sector_t		*other;
    fixed_t		height = currentheight;
    fixed_t		heightlist[MAX_ADJOINING_SECTORS];

    for (i = 0, h = 0; i < sec->linecount; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check, sec);

	if (!other)
	    continue;

	if (other->floorheight > height)
	    heightlist[h++] = other->floorheight;

	// Check for overflow. Exit.
	if (h >= MAX_ADJOINING_SECTORS)
	{
	    printf("Sector with more than 20 adjoining sectors\n" );
	    break;
	}
    }

    // Find lowest height in list
    if (!h)
	return currentheight;

    min = heightlist[0];

    // Range checking?
    for (i = 1; i < h; i++)
	if (heightlist[i] < min)
	    min = heightlist[i];

    return min;
}
#endif

// Version which should do the same, but without using fixed array
fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
    int			i;
    sector_t		*other;
    fixed_t		height = currentheight;

    for (i = 0; i < sec->linecount; i++)
    {
      if ((other = getNextSector(sec->lines[i], sec)) &&
	  (other->floorheight > currentheight))
      {
	height = other->floorheight;
	while (++i < sec->linecount)
	{
	  if ((other = getNextSector(sec->lines[i], sec)) &&
	      (other->floorheight < height) &&
	      (other->floorheight > currentheight))
		height = other->floorheight;
	}
      }
    }
    return height;
}

//
// From Boom:
//
// Passed a sector and a floor height, returns the fixed point value
// of the largest floor height in a surrounding sector smaller than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight)
{
    sector_t	*other;
    int		i;

    for (i = 0; i < sec->linecount; i++)
	if ((other = getNextSector(sec->lines[i], sec)) &&
	    other->floorheight < currentheight)
	{
	    int height = other->floorheight;
	    while (++i < sec->linecount)
		if ((other = getNextSector(sec->lines[i], sec)) &&
		    other->floorheight > height &&
		    other->floorheight < currentheight)
		  height = other->floorheight;
	    return height;
	}
    return currentheight;
}

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec)
{
    int			i;
    sector_t		*other;
    fixed_t		height = MAXINT;

    for (i = 0; i < sec->linecount; i++)
    {
	if ((other = getNextSector(sec->lines[i], sec)) &&
	    (other->ceilingheight < height))
	        height = other->ceilingheight;
    }
    return height;
}

//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t	P_FindHighestCeilingSurrounding(sector_t *sec)
{
    int		i;
    sector_t	*other;
    fixed_t	height = 0;

    for (i = 0; i < sec->linecount; i++)
    {
	if ((other = getNextSector(sec->lines[i], sec)) &&
	    (other->ceilingheight > height))
	        height = other->ceilingheight;
    }
    return height;
}

//
// From Boom:
//
// Passed a floor height and a sector number, return a pointer to a
// sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL.
//
sector_t *P_FindModelFloorSector(fixed_t floordestheight, int secnum)
{
    int		i;
    sector_t	*sec = (sector_t *)0;
    int		linecount;

    sec = &sectors[secnum];
    linecount = sec->linecount;

    for (i = 0; i < (sec->linecount < linecount ? sec->linecount : linecount);
	 i++)
    {
	if (getSide(secnum, i, 0)->sector - sectors == secnum)
	    sec = getSector(secnum, i, 1);
	else
	    sec = getSector(secnum, i, 0);

	if (sec->floorheight == floordestheight)
	    return sec;
    }
    return (sector_t *)0;
}

//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
int P_FindSectorFromLineTag(line_t *line, int start)
{
    int	i;

    for (i = start + 1; i < numsectors; i++)
	if (sectors[i].tag == line->tag)
	    return i;

    return -1;
}

//
// Find sector from tag
// This is from Boom
//
int P_FindSectorFromTag(int tag, int start)
{
	start = start >= 0 ? sectors[start].nexttag :
		sectors[(unsigned)tag % (unsigned)numsectors].firsttag;
	while (start >= 0 && sectors[start].tag != tag)
		start = sectors[start].nexttag;
	return start;
}

//
// Find line from another lines tag
// This is from Boom
//
int P_FindLineFromLineTag(line_t *line, int start)
{
    start = start >= 0 ? lines[start].nexttag :
	lines[(unsigned)line->tag % (unsigned)numlines].firsttag;
    while (start >= 0 && lines[start].tag != line->tag)
	start = lines[start].nexttag;
    return start;
}

//
// Find line from tag
// This is needed for XDoomPlus, just written from function above
//
int P_FindLineFromTag(int tag, int start)
{
    start = start >= 0 ? lines[start].nexttag :
	lines[(unsigned)tag % (unsigned)numlines].firsttag;
    while (start >= 0 && lines[start].tag != tag)
	start = lines[start].nexttag;
    return start;
}

//
// Hash sector tags across the sectors and linedefs.
// Also taken from Boom.
//
void P_InitTagLists(void)
{
    register int i;

    // hash tags in sectors
    for (i = numsectors; --i >= 0;)	// Initially make all slots empty
	sectors[i].firsttag = -1;
    for (i = numsectors; --i >= 0;)	// Proceed from last to first sector
    {					// so that lower sector appears first
	int j = (unsigned) sectors[i].tag % (unsigned)numsectors; // hash func
	sectors[i].nexttag = sectors[j].firsttag;  // Prepend sector to chain
	sectors[j].firsttag = i;
    }

    // hash tags in linedefs
    for (i = numlines; --i >= 0;)	// Initially make all slots empty
	lines[i].firsttag = -1;
    for (i = numlines; --i >= 0;)	// Proceed from last to first linedef
    {					// so that lower linedef appears first
	int j = (unsigned) lines[i].tag % (unsigned)numlines; // hash func
	lines[i].nexttag = lines[j].firsttag; // Prepend linedef to chain
	lines[j].firsttag = i;
    }
}

//
// Find minimum light from an adjacent sector
//
int P_FindMinSurroundingLight(sector_t *sector, int max)
{
    int		i;
    int		min;
    sector_t	*check;

    min = max;
    for (i = 0; i < sector->linecount; i++)
    {
	if ((check = getNextSector(sector->lines[i], sector)) &&
	    (check->lightlevel < min))
	        min = check->lightlevel;
    }
    return min;
}

//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//

//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void P_CrossSpecialLine(int linenum, int side, mobj_t *thing)
{
    line_t	*line;
    int		ok;

    line = &lines[linenum];

    // Triggers that other things can activate
    if (!thing->player)
    {
	// Things that should NOT trigger specials...
	switch(thing->type)
	{
	  case MT_ROCKET:
	  case MT_PLASMA:
	  case MT_BFG:
	  case MT_TROOPSHOT:
	  case MT_HEADSHOT:
	  case MT_BRUISERSHOT:
	    return;
	    break;

	  default:
	    break;
	}

	ok = 0;
	switch(line->special)
	{
	  case 39:	// Teleport trigger
	  case 97:	// Teleport retrigger
	  case 125:	// Teleport Monster only trigger
	  case 126:	// Teleport monster only retrigger
	  case 4:	// Raise Door
	  case 10:	// Plat down-wait-up-stay trigger
	  case 88:	// Plat down-wait-up-stay retrigger
	  case 207:	// silent thing teleporters
	  case 208:
	  case 243:	// silent line teleporters
	  case 244:
	  case 262:
	  case 263:
	  case 264:
	  case 265:
	  case 266:
	  case 267:
	  case 268:
	  case 269:
	  case 334:	// UAC commdev message trigger
	  case 352:	// open door silent and stay open
	    ok = 1;
	    break;
	}
	if (!ok)
	    return;
    }

    // Triggers that players can't activate
    if (thing->player)
    {
	switch(line->special)
	{
	  case 125:	// teleport monster only
	  case 126:	// teleport monster only
	  case 264:	// teleport monster only
	  case 265:	// teleport monster only
	  case 266:	// teleport monster only
	  case 267:	// teleport monster only
	  case 268:	// teleport monster only
	  case 269:	// teleport monster only
	  case 352:	// open door silent and stay open
	    ok = 0;
	    break;
          default:
	    ok = 1;
            break;
	}
	if (!ok)
	    return;
    }

    // Note: could use some const's here.
    switch (line->special)
    {

// ===========================================================================
//		TRIGGERS.  All from here to RETRIGGERS
// ===========================================================================

      case 2:
	// Open Door
	EV_DoDoor(line, open);
	line->special = 0;
	break;

      case 3:
	// Close Door
	EV_DoDoor(line, close);
	line->special = 0;
	break;

      case 4:
	// Raise Door
	EV_DoDoor(line, normal);
	line->special = 0;
	break;

      case 5:
	// Raise Floor
	EV_DoFloor(line, raiseFloor);
	line->special = 0;
	break;

      case 6:
	// Fast Ceiling Crush & Raise
	EV_DoCeiling(line, fastCrushAndRaise);
	line->special = 0;
	break;

      case 8:
	// Build Stairs step 8
	EV_BuildStairs(line, build8);
	line->special = 0;
	break;

      case 10:
	// PlatDownWaitUp
	EV_DoPlat(line, downWaitUpStay, 0);
	line->special = 0;
	break;

      case 12:
	// Light Turn On - brightest near
	EV_LightTurnOn(line, 0);
	line->special = 0;
	break;

      case 13:
	// Light Turn On 255
	EV_LightTurnOn(line, 255);
	line->special = 0;
	break;

      case 16:
	// Close Door 30
	EV_DoDoor(line, close30ThenOpen);
	line->special = 0;
	break;

      case 17:
	// Start Light Strobing
	EV_StartLightStrobing(line);
	line->special = 0;
	break;

      case 19:
	// Lower Floor
	EV_DoFloor(line, lowerFloor);
	line->special = 0;
	break;

      case 22:
	// Raise Platform to nearest height and change texture
	EV_DoPlat(line, raiseToNearestAndChange, 0);
	line->special = 0;
	break;

      case 25:
	// Ceiling Crush and Raise
	EV_DoCeiling(line, crushAndRaise);
	line->special = 0;
	break;

      case 30:
	// Raise Floor to shortest texture height
	//  on either side of lines.
	EV_DoFloor(line, raiseToTexture);
	line->special = 0;
	break;

      case 35:
	// Lights Very Dark
	EV_LightTurnOn(line, 35);
	line->special = 0;
	break;

      case 36:
	// Lower Floor (TURBO)
	EV_DoFloor(line, turboLower);
	line->special = 0;
	break;

      case 37:
	// Lower Floor and change texture
	EV_DoFloor(line, lowerAndChange);
	line->special = 0;
	break;

      case 38:
	// Lower Floor To Lowest
	EV_DoFloor(line, lowerFloorToLowest);
	line->special = 0;
	break;

      case 39:
	// TELEPORT!
	EV_Teleport(line, side, thing);
	line->special = 0;
	break;

      case 40:
	// Raise Ceiling, Lower Floor
	EV_DoCeiling(line, raiseToHighest);
	EV_DoFloor(line, lowerFloorToLowest);
	line->special = 0;
	break;

      case 44:
	// Ceiling Crush
	EV_DoCeiling(line, lowerAndCrush);
	line->special = 0;
	break;

      case 52:
	// EXIT!
	G_ExitLevel();
	break;

      case 53:
	// Perpetual Platform Raise
	EV_DoPlat(line, perpetualRaise, 0);
	line->special = 0;
	break;

      case 54:
	// Platform Stop
	EV_StopPlat(line);
	line->special = 0;
	break;

      case 56:
	// Raise Floor Crush
	EV_DoFloor(line, raiseFloorCrush);
	line->special = 0;
	break;

      case 57:
	// Ceiling Crush Stop
	EV_CeilingCrushStop(line);
	line->special = 0;
	break;

      case 58:
	// Raise Floor 24
	EV_DoFloor(line, raiseFloor24);
	line->special = 0;
	break;

      case 59:
	// Raise Floor 24 and change texture
	EV_DoFloor(line, raiseFloor24AndChange);
	line->special = 0;
	break;

      case 104:
	// Turn Lights Off in sector(tag)
	EV_TurnTagLightsOff(line);
	line->special = 0;
	break;

      case 108:
	// Blazing Door Raise (faster than TURBO!)
	EV_DoDoor(line, blazeRaise);
	line->special = 0;
	break;

      case 109:
	// Blazing Door Open (faster than TURBO!)
	EV_DoDoor(line, blazeOpen);
	line->special = 0;
	break;

      case 100:
	// Build Stairs Turbo 16
	EV_BuildStairs(line, turbo16);
	line->special = 0;
	break;

      case 110:
	// Blazing Door Close (faster than TURBO!)
	EV_DoDoor(line, blazeClose);
	line->special = 0;
	break;

      case 119:
	// Raise Floor to nearest surr. Floor
	EV_DoFloor(line, raiseFloorToNearest);
	line->special = 0;
	break;

      case 121:
	// Blazing PlatDownWaitUpStay
	EV_DoPlat(line, blazeDWUS,0);
	line->special = 0;
	break;

      case 124:
	// Secret EXIT
	G_SecretExitLevel();
	break;

      case 125:
	// TELEPORT MonsterONLY
	EV_Teleport(line, side, thing);
	line->special = 0;
	break;

      case 130:
	// Raise Floor Turbo
	EV_DoFloor(line, raiseFloorTurbo);
	line->special = 0;
	break;

      case 141:
	// Silent Ceiling Crush & Raise
	EV_DoCeiling(line, silentCrushAndRaise);
	line->special = 0;
	break;

      case 142:
	// Raise Floor 512
	EV_DoFloor(line, raiseFloor512);
	line->special = 0;
	break;

      case 143:
	// Raise Platform 24 and change texture
	EV_DoPlat(line, raiseAndChange, 24);
	line->special = 0;
	break;

      case 144:
	// Raise Platform 32 and change texture
	EV_DoPlat(line, raiseAndChange, 32);
	line->special = 0;
	break;

      case 145:
	// Lower Ceiling to Floor
	EV_DoCeiling(line, lowerToFloor);
	line->special = 0;
	break;

      case 146:
	// Change Donut
	EV_DoDonut(line);
	line->special = 0;
	break;

      case 153:
	// Change Floor texture only, no motion
	EV_DoChange(line, trigChangeOnly);
	line->special = 0;
	break;

      case 199:
	// Lower Ceiling to lowest surrounding Ceiling
	EV_DoCeiling(line, lowerToLowest);
	line->special = 0;
	break;

      case 200:
	// Lower Ceiling to highest surrounding Floor
	EV_DoCeiling(line, lowerToMaxFloor);
	line->special = 0;
	break;

      case 207:
	// Silent Teleporter (normal kind)
	EV_SilentTeleport(line, side, thing);
	line->special = 0;
	break;

      case 219:
	// Lower Floor to next lowest Floor
	EV_DoFloor(line, lowerFloorToNearest);
	line->special = 0;
	break;

      case 227:
	// Raise Elevator to next highest Floor
	EV_DoElevator(line, elevateUp);
	line->special = 0;
	break;

      case 231:
	// Lower Elevator next Floor
	EV_DoElevator(line, elevateDown);
	line->special = 0;
	break;

      case 235:
	// Elevator to current Floor
	EV_DoElevator(line, elevateCurrent);
	line->special = 0;
	break;

      case 239:
	// Change Floor texture only (numeric), no motion
	EV_DoChange(line, numChangeOnly);
	line->special = 0;
	break;

      case 243:
	// Silent line Teleporter
	EV_SilentLineTeleport(line, side, thing, false);
	line->special = 0;
	break;

      case 262:
	// Silent line Teleporter, reversed
	EV_SilentLineTeleport(line, side, thing, true);
	line->special = 0;
	break;

      case 264:
	// Silent line Teleporter, monster only, reversed
	EV_SilentLineTeleport(line, side, thing, true);
	line->special = 0;
	break;

      case 266:
	// Silent line Teleporter, monster only
	EV_SilentLineTeleport(line, side, thing, false);
	line->special = 0;
	break;

      case 268:
	// Silent Teleporter, monster only
	EV_SilentTeleport(line, side, thing);
	line->special = 0;
	break;

      case 331:
	// Send new message to comdev
	M_CommNewMsg(line->tag);
	line->special = 0;
	break;

      case 334:
	// Send new message to comdev
	M_CommNewMsg(line->tag);
	line->special = 0;
	break;

      case 350:
      case 352:
	// Open Door silent
	EV_DoSilentDoor(line, open);
	line->special = 0;
	break;

// ===========================================================================
//		RETRIGGERS.  All from here till end.
// ===========================================================================

      case 72:
	// Ceiling Crush
	EV_DoCeiling(line, lowerAndCrush);
	break;

      case 73:
	// Ceiling Crush and Raise
	EV_DoCeiling(line, crushAndRaise);
	break;

      case 74:
	// Ceiling Crush Stop
	EV_CeilingCrushStop(line);
	break;

      case 75:
	// Close Door
	EV_DoDoor(line, close);
	break;

      case 76:
	// Close Door 30
	EV_DoDoor(line, close30ThenOpen);
	break;

      case 77:
	// Fast Ceiling Crush & Raise
	EV_DoCeiling(line, fastCrushAndRaise);
	break;

      case 79:
	// Lights Very Dark
	EV_LightTurnOn(line, 35);
	break;

      case 80:
	// Light Turn On - brightest near
	EV_LightTurnOn(line, 0);
	break;

      case 81:
	// Light Turn On 255
	EV_LightTurnOn(line, 255);
	break;

      case 82:
	// Lower Floor To Lowest
	EV_DoFloor(line, lowerFloorToLowest);
	break;

      case 83:
	// Lower Floor
	EV_DoFloor(line, lowerFloor);
	break;

      case 84:
	// Lower Floor and change texture
	EV_DoFloor(line, lowerAndChange);
	break;

      case 86:
	// Open Door
	EV_DoDoor(line, open);
	break;

      case 87:
	// Perpetual Platform Raise
	EV_DoPlat(line, perpetualRaise, 0);
	break;

      case 88:
	// PlatDownWaitUp
	EV_DoPlat(line, downWaitUpStay, 0);
	break;

      case 89:
	// Platform Stop
	EV_StopPlat(line);
	break;

      case 90:
	// Raise Door
	EV_DoDoor(line, normal);
	break;

      case 91:
	// Raise Floor
	EV_DoFloor(line, raiseFloor);
	break;

      case 92:
	// Raise Floor 24
	EV_DoFloor(line, raiseFloor24);
	break;

      case 93:
	// Raise Floor 24 and change texture
	EV_DoFloor(line, raiseFloor24AndChange);
	break;

      case 94:
	// Raise Floor Crush
	EV_DoFloor(line, raiseFloorCrush);
	break;

      case 95:
	// Raise Platform to nearest height and change texture
	// and change texture.
	EV_DoPlat(line, raiseToNearestAndChange, 0);
	break;

      case 96:
	// Raise Floor to shortest texture height
	// on either side of lines.
	EV_DoFloor(line, raiseToTexture);
	break;

      case 97:
	// TELEPORT!
	EV_Teleport(line, side, thing);
	break;

      case 98:
	// Lower Floor (TURBO)
	EV_DoFloor(line, turboLower);
	break;

      case 105:
	// Blazing Door Raise (faster than TURBO!)
	EV_DoDoor(line, blazeRaise);
	break;

      case 106:
	// Blazing Door Open (faster than TURBO!)
	EV_DoDoor(line, blazeOpen);
	break;

      case 107:
	// Blazing Door Close (faster than TURBO!)
	EV_DoDoor(line, blazeClose);
	break;

      case 120:
	// Blazing PlatDownWaitUpStay.
	EV_DoPlat(line, blazeDWUS, 0);
	break;

      case 126:
	// TELEPORT Monster ONLY.
	EV_Teleport(line, side, thing);
	break;

      case 128:
	// Raise To Nearest Floor
	EV_DoFloor(line, raiseFloorToNearest);
	break;

      case 129:
	// Raise Floor Turbo
	EV_DoFloor(line, raiseFloorTurbo);
	break;

      case 147:
	// Raise Floor 512
	EV_DoFloor(line, raiseFloor512);
	break;

      case 148:
	// Raise Platform 24 and change texture
	EV_DoPlat(line, raiseAndChange, 24);
	break;

      case 149:
	// Raise Platform 32 and change texture
	EV_DoPlat(line, raiseAndChange, 32);
	break;

      case 150:
	// Silent Ceiling Crush and Raise
	EV_DoCeiling(line, silentCrushAndRaise);
	break;

      case 151:
	// Raise Ceiling, lower Floor
	EV_DoCeiling(line, raiseToHighest);
	EV_DoFloor(line, lowerFloorToLowest);
	break;

      case 152:
	// Lower Ceiling to Floor
	EV_DoCeiling(line, lowerToFloor);
	break;

      case 154:
	// Texture change only, no motion
	EV_DoChange(line, trigChangeOnly);
	break;

      case 155:
	// Change Donut
	EV_DoDonut(line);
	break;

      case 156:
	// Start Light Strobing
	EV_StartLightStrobing(line);
	break;

      case 157:
	// Turn Lights Off in sector(tag)
	EV_TurnTagLightsOff(line);
	break;

      case 201:
	// Lower Ceiling to lowest surrounding ceiling
	EV_DoCeiling(line, lowerToLowest);
	break;

      case 202:
	// Lower Ceiling to highest surrounding Floor
	EV_DoCeiling(line, lowerToMaxFloor);
	break;

      case 208:
	// Silent Teleporter, normal kind
	EV_SilentTeleport(line, side, thing);
	break;

      case 212:
	// Toggle Platform between Floor and Ceiling instantly
	EV_DoPlat(line, toggleUpDn, 0);
	break;

      case 220:
	// Lower Floor to next lowest Floor
	EV_DoFloor(line, lowerFloorToNearest);
	break;

      case 228:
	// Raise Elevator next Floor
	EV_DoElevator(line, elevateUp);
	break;

      case 232:
	// Lower Elevator next Floor
	EV_DoElevator(line, elevateDown);
	break;

      case 236:
	// Elevator to current Floor
	EV_DoElevator(line, elevateCurrent);
	break;

      case 240:
	// Change Floor texture only (numeric), no motion
	EV_DoChange(line, numChangeOnly);
	break;

      case 244:
	// Silent line Teleporter
	EV_SilentLineTeleport(line, side, thing, false);
	break;

      case 256:
	// Build Stairs step 8
	EV_BuildStairs(line, build8);
	break;

      case 257:
	// Build Stairs step 16
	EV_BuildStairs(line, turbo16);
	break;

      case 263:
	// Silent line Teleporter, reversed
	EV_SilentLineTeleport(line, side, thing, true);
	break;

      case 265:
	// Silent line Teleporter, monster only, reversed
	EV_SilentLineTeleport(line, side, thing, true);
	break;

      case 267:
	// Silent line Teleporter, monster only
	EV_SilentLineTeleport(line, side, thing, false);
	break;

      case 269:
	// Silent Teleporter, monster only
	EV_SilentTeleport(line, side, thing);
	break;

      case 330:
	// Send new message to comdev
	M_CommNewMsg(line->tag);
	break;
    }
}

//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
boolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side)
{
    register int i;

    // Err...
    // Use the back sides of VERY SPECIAL lines...
    if (side)
    {
	switch(line->special)
	{
	  case 300:
	    // Sliding Door Open&Close
	    EV_SlidingDoor(line, thing);
	    break;

	  default:
	    return false;
	    break;
	}
    }

    // Switches that other things can activate.
    if (!thing->player)
    {
	// never open secret doors
	if (line->flags & ML_SECRET)
	    return false;

	switch(line->special)
	{
	  case 1: 	// Manual Door raise
	  case 32:	// Manual Blue
	  case 33:	// Manual Red
	  case 34:	// Manual Yellow
	  case 174:	// Teleporters
	  case 195:
	  case 209:
	  case 210:
	    break;

	  default:
	    return false;
	    break;
	}
    }

    // do something
    switch (line->special)
    {

// ------------------------------------------------------------------------
//				MANUALS
// ------------------------------------------------------------------------

      case 1:		// Vertical Door
      case 26:		// Blue Door/Locked
      case 27:		// Yellow Door /Locked
      case 28:		// Red Door /Locked

      case 31:		// Manual door open
      case 32:		// Blue locked door open
      case 33:		// Red locked door open
      case 34:		// Yellow locked door open

      case 117:		// Blazing door raise
      case 118:		// Blazing door open
	EV_VerticalDoor(line, thing);
	break;

	// Door Slide Open&Close
      case 300:
	 EV_SlidingDoor(line, thing);
	 break;

// ------------------------------------------------------------------------
//				SWITCHES
// ------------------------------------------------------------------------

      case 7:
	// Build Stairs step 8
	if (EV_BuildStairs(line, build8))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 9:
	// Change Donut
	if (EV_DoDonut(line))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 11:
	// Exit level
	P_ChangeSwitchTexture(line, 0);
	G_ExitLevel();
	break;

      case 14:
	// Raise Platform 32 and change texture
	if (EV_DoPlat(line, raiseAndChange, 32))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 15:
	// Raise Platform 24 and change texture
	if (EV_DoPlat(line, raiseAndChange, 24))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 18:
	// Raise Floor to next highest Floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 20:
	// Raise Platform next highest Floor and change texture
	if (EV_DoPlat(line, raiseToNearestAndChange, 0))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 21:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line, downWaitUpStay, 0))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 23:
	// Lower Floor to lowest
	if (EV_DoFloor(line, lowerFloorToLowest))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 29:
	// Raise Door
	if (EV_DoDoor(line, normal))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 41:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line, lowerToFloor))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 71:
	// Turbo Lower Floor
	if (EV_DoFloor(line, turboLower))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 49:
	// Ceiling Crush And Raise
	if (EV_DoCeiling(line, crushAndRaise))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 50:
	// Close Door
	if (EV_DoDoor(line, close))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 51:
	// Secret EXIT
	P_ChangeSwitchTexture(line, 0);
	G_SecretExitLevel();
	break;

      case 55:
	// Raise Floor Crush
	if (EV_DoFloor(line, raiseFloorCrush))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 101:
	// Raise Floor
	if (EV_DoFloor(line, raiseFloor))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 102:
	// Lower Floor to Surrounding Floor height
	if (EV_DoFloor(line, lowerFloor))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 103:
	// Open Door
	if (EV_DoDoor(line, open))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 111:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor(line, blazeRaise))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 112:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor(line, blazeOpen))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 113:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor(line, blazeClose))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 122:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line, blazeDWUS, 0))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 127:
	// Build Stairs Turbo 16
	if (EV_BuildStairs(line, turbo16))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 131:
	// Raise Floor Turbo
	if (EV_DoFloor(line, raiseFloorTurbo))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 133:
	// BlzOpenDoor Blue
      case 135:
	// BlzOpenDoor Red
      case 137:
	// BlzOpenDoor Yellow
	if (EV_DoLockedDoor(line, blazeOpen, thing))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 140:
	// Raise Floor 512
	if (EV_DoFloor(line, raiseFloor512))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 158:
	// Raise Floor to shortest lower texture
	if (EV_DoFloor(line, raiseToTexture))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 159:
	// Lower Floor to shortest lower texture and change texture
	if (EV_DoFloor(line, lowerAndChange))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 160:
	// Raise Floor 24 and change texture
	if (EV_DoFloor(line, raiseFloor24AndChange))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 161:
	// Raise Floor 24
	if (EV_DoFloor(line, raiseFloor24))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 162:
	// Perpetual Platform Raise
	if (EV_DoPlat(line, perpetualRaise, 0))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 163:
	// Stop Platform
	EV_StopPlat(line);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 164:
	// Fast Ceiling Crush & Raise
	if (EV_DoCeiling(line, fastCrushAndRaise))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 165:
	// Silent Ceiling Crush and Raise
	if (EV_DoCeiling(line, silentCrushAndRaise))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 166:
	// Raise Ceiling, lower Floor
	if (EV_DoCeiling(line, raiseToHighest) ||
	    EV_DoFloor(line, lowerFloorToLowest))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 167:
	// Lower Ceiling and Crush
	if (EV_DoCeiling(line, lowerAndCrush))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 168:
	// Stop Crushing Ceiling
	if (EV_CeilingCrushStop(line))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 169:
	// Light Turn On - brightest near
	EV_LightTurnOn(line, 0);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 170:
	// Lights Very Dark
	EV_LightTurnOn(line, 35);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 171:
	// Lights On Full
	EV_LightTurnOn(line, 255);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 172:
	// Start Light Strobing
	EV_StartLightStrobing(line);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 173:
	// Turn Lights Off in sector(tag)
	EV_TurnTagLightsOff(line);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 174:
	// TELEPORT
	if (EV_Teleport(line, side, thing))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 175:
	// Close Door, Open in 30 secs
	if (EV_DoDoor(line, close30ThenOpen))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 189:
	// Texture change only, no motion
	if (EV_DoChange(line, trigChangeOnly))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 203:
	// Lower Ceiling to lowest surrounding Ceiling
	if (EV_DoCeiling(line, lowerToLowest))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 204:
	// Lower Ceiling to highest surrounding Floor
	if (EV_DoCeiling(line, lowerToMaxFloor))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 209:
	// Silent Teleporter
	if (EV_SilentTeleport(line, side, thing))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 221:
	// Lower Floor to next lowest Floor
	if (EV_DoFloor(line, lowerFloorToNearest))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 229:
	// Raise Elevator to next highest Floor
	if (EV_DoElevator(line, elevateUp))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 233:
	// Lower Elevator to next lowest Floor
	if (EV_DoElevator(line, elevateDown))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 237:
	// Elevator to current Floor
	if (EV_DoElevator(line, elevateCurrent))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 241:
	// Change Floor texture only (numeric), no motion
	if (EV_DoChange(line, numChangeOnly))
	    P_ChangeSwitchTexture(line, 0);
	break;

      case 351:
	// Open Door silent
	if (EV_DoSilentDoor(line, open))
	    P_ChangeSwitchTexture(line, 0);
	break;

// ------------------------------------------------------------------------
//				 BUTTONS
// ------------------------------------------------------------------------

      case 42:
	// Close Door
	if (EV_DoDoor(line, close))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 43:
	// Lower Ceiling to Floor
	if (EV_DoCeiling(line, lowerToFloor))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 45:
	// Lower Floor to surrounding Floor height
	if (EV_DoFloor(line, lowerFloor))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 60:
	// Lower Floor to lowest
	if (EV_DoFloor(line, lowerFloorToLowest))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 61:
	// Open Door
	if (EV_DoDoor(line, open))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 62:
	// PlatDownWaitUpStay
	if (EV_DoPlat(line, downWaitUpStay, 1))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 63:
	// Raise Door
	if (EV_DoDoor(line, normal))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 64:
	// Raise Floor to Ceiling
	if (EV_DoFloor(line, raiseFloor))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 66:
	// Raise Platform 24 and change texture
	if (EV_DoPlat(line, raiseAndChange, 24))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 67:
	// Raise Platform 32 and change texture
	if (EV_DoPlat(line, raiseAndChange, 32))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 65:
	// Raise Floor Crush
	if (EV_DoFloor(line, raiseFloorCrush))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 68:
	// Raise Platform to next highest Floor and change texture
	if (EV_DoPlat(line, raiseToNearestAndChange, 0))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 69:
	// Raise Floor to next highest Floor
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 70:
	// Turbo Lower Floor
	if (EV_DoFloor(line, turboLower))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 78:
	// Change Floor texture only (numeric), no motion
	if (EV_DoChange(line, numChangeOnly))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 114:
	// Blazing Door Raise (faster than TURBO!)
	if (EV_DoDoor(line, blazeRaise))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 115:
	// Blazing Door Open (faster than TURBO!)
	if (EV_DoDoor(line, blazeOpen))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 116:
	// Blazing Door Close (faster than TURBO!)
	if (EV_DoDoor(line, blazeClose))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 123:
	// Blazing PlatDownWaitUpStay
	if (EV_DoPlat(line, blazeDWUS, 0))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 132:
	// Raise Floor Turbo
	if (EV_DoFloor(line, raiseFloorTurbo))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 99:
	// BlzOpenDoor Blue
      case 134:
	// BlzOpenDoor Red
      case 136:
	// BlzOpenDoor Yellow
	if (EV_DoLockedDoor(line, blazeOpen, thing))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 138:
	// Light Turn On
	EV_LightTurnOn(line, 255);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 139:
	// Light Turn Off
	EV_LightTurnOn(line, 35);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 176:
	// Raise Floor to shortest lower texture
	if (EV_DoFloor(line, raiseToTexture))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 177:
	// Lower Floor to shortest lower texture and change texture
	if (EV_DoFloor(line, lowerAndChange))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 178:
	// Raise Floor 512
	if (EV_DoFloor(line, raiseFloor512))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 179:
	// Raise Floor 24 and change texture
	if (EV_DoFloor(line, raiseFloor24AndChange))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 180:
	// Raise Floor 24
	if (EV_DoFloor(line, raiseFloor24))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 181:
	// Perpetual Platform Raise
	if (EV_DoPlat(line, perpetualRaise, 0))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 182:
	// Stop Platform
	EV_StopPlat(line);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 183:
	// Fast Ceiling Crush & Raise
	if (EV_DoCeiling(line, fastCrushAndRaise))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 184:
	// Ceiling Crush and Raise
	if (EV_DoCeiling(line, crushAndRaise))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 185:
	// Silent Ceiling Crush and Raise
	if (EV_DoCeiling(line, silentCrushAndRaise))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 186:
	// Raise Ceiling, lower Floor
	if (EV_DoCeiling(line, raiseToHighest) ||
	    EV_DoFloor(line, lowerFloorToLowest))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 187:
	// Lower Ceiling and Crush
	if (EV_DoCeiling(line, lowerAndCrush))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 188:
	// Stop Crushing Ceiling
	if (EV_CeilingCrushStop(line))
	    P_ChangeSwitchTexture(line, 1);
	break;
	
      case 190:
	// Texture change only, no motion
	if (EV_DoChange(line, trigChangeOnly))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 191:
	// Change Donut
	if (EV_DoDonut(line))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 192:
	// Light Turn On - brightest near
	EV_LightTurnOn(line, 0);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 193:
	// Start Light Strobing
	EV_StartLightStrobing(line);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 194:
	// Turn Lights Off in sector(tag)
	EV_TurnTagLightsOff(line);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 195:
	// TELEPORT
	if (EV_Teleport(line, side, thing))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 196:
	// Close Door, Open in 30 secs
	if (EV_DoDoor(line, close30ThenOpen))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 205:
	// Lower Ceiling to lowest surrounding Ceiling
	if (EV_DoCeiling(line, lowerToLowest))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 206:
	// Lower Ceiling to highest surrounding Floor
	if (EV_DoCeiling(line, lowerToMaxFloor))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 210:
	// Silent Teleporter
	if (EV_SilentTeleport(line, side, thing))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 211:
	// Toggle Platform between Floor and Ceiling instantly
	if (EV_DoPlat(line, toggleUpDn, 0))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 222:
	// Lower Floor to next lowest Floor
	if (EV_DoFloor(line, lowerFloorToNearest))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 230:
	// Raise Elevator to next highest Floor
	if (EV_DoElevator(line, elevateUp))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 234:
	// Lower Elevator to next lowest Floor
	if (EV_DoElevator(line, elevateDown))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 238:
	// Elevator to current Floor
	if (EV_DoElevator(line, elevateCurrent))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 258:
	// Build Stairs step 8
	if (EV_BuildStairs(line, build8))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 259:
	// Build Stairs step 16
	if (EV_BuildStairs(line, turbo16))
	    P_ChangeSwitchTexture(line, 1);
	break;

      case 321:
	// Switch Force Field Off, then On again
	EV_ForceField(line, 0);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 332:
	// Comdev linked to access terminal and activated Teleporter
	M_CommNewMsgTxt("TELEPORT");
	// find all 2s sided lines with same tag and make it player teleport
	for (i = -1; (i = P_FindLineFromLineTag(line, i)) >= 0;)
	{
	    // teleporter must be 2s line
	    if ( !lines[i].flags & ML_TWOSIDED )
		continue;

	    // make it a player teleporter
	    lines[i].special = 97;
	}
	// terminal is usable once
	line->special = 0;
	break;

      case 333:
	// Comdev linked to access terminal and switched Force Field off
	M_CommNewMsgTxt("FIELD");
	EV_ForceField(line, 1);
	// terminal is usable once
	line->special = 0;
	break;
    }

    return true;
}

//
// P_UseSpecialLinePlus
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
// This version is for XDoomPlus.
//
boolean P_UseSpecialLinePlus(mobj_t *thing, line_t *line, int side)
{
    int		flags = MLPLUS_GET_SPAC(line->flags);

    // Missiles don't use lines, that should never happen
    if (thing->flags & MF_MISSILE)
	return false;

    // Check monsters
    if (!thing->player)
    {
	// never open secret doors
	if (line->flags & ML_SECRET)
	    return false;
    }

    // otherwise let player and monster use line
    if (flags == SPAC_USE)
    {
	LineSpecials[line->special] (&line->args[0], line, side, thing);
	if (!(line->flags & MLPLUS_REPEAT_SPECIAL))
	    line->special = 0;
    }
    else
    {
	// player makes "oof" sound, if trying to use a not usable special
	if (thing->player)
	    S_StartSound(thing, sfx_noway);
    }

    return true;
}

//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void P_ShootSpecialLine(mobj_t *thing, line_t *line)
{
    int		ok;

    //	Impacts that other things can activate.
    if (!thing->player)
    {
	ok = 0;
	switch(line->special)
	{
	  case 46:
	    // Open Door impact
	    ok = 1;
	    break;
	}
	if (!ok)
	    return;
    }

    switch(line->special)
    {
      case 24:
	// Raise Floor
	EV_DoFloor(line, raiseFloor);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 46:
	// Open Door
	EV_DoDoor(line, open);
	P_ChangeSwitchTexture(line, 1);
	break;

      case 47:
	// Raise Platform near and change texture
	EV_DoPlat(line, raiseToNearestAndChange, 0);
	P_ChangeSwitchTexture(line, 0);
	break;

      case 197:
	// EXIT
	P_ChangeSwitchTexture(line, 0);
	G_ExitLevel();
	break;

      case 198:
	// Secret EXIT
	P_ChangeSwitchTexture(line, 0);
	G_SecretExitLevel();
	break;

      case 322:
	// Damaged lenses of Force Field, switch it Off
	EV_ForceField(line, 1);
	break;
    }
}

//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void P_PlayerInSpecialSector(player_t *player)
{
    sector_t	*sector;

    sector = player->mo->subsector->sector;

    // Falling, not all the way down yet?
    if (player->mo->z != sector->floorheight)
	return;

    // Has hitten ground.
    switch (sector->special)
    {
      case 5:
	// HELLSLIME DAMAGE
	if (!player->powers[pw_ironfeet])
	    if (!(leveltime&0x1f))
		P_DamageMobj(player->mo, NULL, NULL, 10);
	break;

      case 7:
	// NUKAGE DAMAGE
	if (!player->powers[pw_ironfeet])
	    if (!(leveltime&0x1f))
		P_DamageMobj(player->mo, NULL, NULL, 5);
	break;

      case 16:
	// SUPER HELLSLIME DAMAGE
      case 4:
	// STROBE HURT
	if (!player->powers[pw_ironfeet] || (P_Random()<5))
	{
	    if (!(leveltime&0x1f))
		P_DamageMobj(player->mo, NULL, NULL, 20);
	}
	break;

      case 9:
	// SECRET SECTOR
	player->secretcount++;
	sector->special = 0;
	break;

      case 11:
	// EXIT SUPER DAMAGE! (for E1M8 finale)
	player->cheats &= ~CF_GODMODE;

	if (!(leveltime&0x1f))
	    P_DamageMobj(player->mo, NULL, NULL, 20);

	if (player->health <= 10)
	    G_ExitLevel();
	break;

      default:
	I_Error("P_PlayerInSpecialSector: "
		"unknown special %i",
		sector->special);
	break;
    }
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
boolean		levelTimer;
int		levelTimeCount;

void P_UpdateSpecials(void)
{
    anim_t	*anim;
    int		pic;
    int		i;
    //line_t	*line;

    //	LEVEL TIMER
    if (levelTimer == true)
    {
	levelTimeCount--;
	if (!levelTimeCount)
	    G_ExitLevel();
    }

    //	ANIMATE FLATS AND TEXTURES GLOBALLY
    for (anim = anims; anim < lastanim; anim++)
    {
	for (i = anim->basepic; i < anim->basepic + anim->numpics; i++)
	{
	    pic = anim->basepic +
		  ((leveltime / anim->speed + i) % anim->numpics);
	    if (anim->istexture)
		texturetranslation[i] = pic;
	    else
		flattranslation[i] = pic;
	}
    }

    // This was the original implementation from id for scrolling.
    // It is now replaced with generalized scrolling from Boom.
#if 0
    //	ANIMATE LINE SPECIALS
    for (i = 0; i < numlinespecials; i++)
    {
	line = linespeciallist[i];
	switch (line->special)
	{
	  case 48:
	    // EFFECT FIRSTCOL SCROLL +
	    sides[line->sidenum[0]].textureoffset += FRACUNIT;
	    break;
	}
    }
#endif

    //	DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
	if (buttonlist[i].btimer)
	{
	    buttonlist[i].btimer--;
	    if (!buttonlist[i].btimer)
	    {
		switch (buttonlist[i].where)
		{
		  case top:
		    sides[buttonlist[i].line->sidenum[0]].toptexture =
			buttonlist[i].btexture;
		    break;

		  case middle:
		    sides[buttonlist[i].line->sidenum[0]].midtexture =
			buttonlist[i].btexture;
		    break;

		  case bottom:
		    sides[buttonlist[i].line->sidenum[0]].bottomtexture =
			buttonlist[i].btexture;
		    break;
		}
		S_StartSound((mobj_t *)&buttonlist[i].soundorg, sfx_swtchn);
		memset(&buttonlist[i], 0, sizeof(button_t));
	    }
	}
}

//
// Special Stuff that can not be categorized
//
int EV_DoDonut(line_t *line)
{
    sector_t		*s1;
    sector_t		*s2;
    sector_t		*s3;
    int			secnum;
    int			rtn;
    int			i;
    floormove_t		*floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	s1 = &sectors[secnum];

	// ALREADY MOVING?  IF SO, KEEP GOING...
	if (s1->floordata)
	    continue;

	rtn = 1;
	s2 = getNextSector(s1->lines[0], s1);
	for (i = 0; i < s2->linecount; i++)
	{
	    if ((!s2->lines[i]->flags & ML_TWOSIDED) ||
		(s2->lines[i]->backsector == s1))
		continue;
	    s3 = s2->lines[i]->backsector;

	    //	Spawn rising slime
	    floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
	    P_AddThinker(&floor->thinker);
	    s2->floordata = floor;
	    floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	    floor->type = donutRaise;
	    floor->crush = false;
	    floor->direction = 1;
	    floor->sector = s2;
	    floor->speed = FLOORSPEED / 2;
	    floor->texture = s3->floorpic;
	    floor->newspecial = 0;
	    floor->floordestheight = s3->floorheight;

	    //	Spawn lowering donut-hole
	    floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
	    P_AddThinker(&floor->thinker);
	    s1->floordata = floor;
	    floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	    floor->type = lowerFloor;
	    floor->crush = false;
	    floor->direction = -1;
	    floor->sector = s1;
	    floor->speed = FLOORSPEED / 2;
	    floor->floordestheight = s3->floorheight;
	    break;
	}
    }
    return rtn;
}

//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//


//
// not used anymore, replaced with thinker based scrollers from Boom
//
#if 0
short		numlinespecials;
line_t		*linespeciallist[MAXLINEANIMS];
#endif

// Parses command line parameters.
void P_SpawnSpecials(void)
{
    sector_t	*sector;
    int		i;
    int		episode;

    episode = 1;
    if (W_CheckNumForName("texture2") >= 0)
	episode = 2;

    // See if -TIMER needs to be used.
    levelTimer = false;

    i = M_CheckParm("-avg");
    if (i && deathmatch)
    {
	levelTimer = true;
	levelTimeCount = 20 * 60 * 35;
    }

    i = M_CheckParm("-timer");
    if (i && deathmatch)
    {
	int	time;
	time = atoi(myargv[i+1]) * 60 * 35;
	levelTimer = true;
	levelTimeCount = time;
    }

    //	Init special SECTORs.
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
	if (!sector->special)
	    continue;

	switch (sector->special)
	{
	  case 1:
	    // FLICKERING LIGHTS
	    P_SpawnLightFlash(sector);
	    break;

	  case 2:
	    // STROBE FAST
	    P_SpawnStrobeFlash(sector, FASTDARK, 0);
	    break;

	  case 3:
	    // STROBE SLOW
	    P_SpawnStrobeFlash(sector, SLOWDARK, 0);
	    break;

	  case 4:
	    // STROBE FAST/DEATH SLIME
	    P_SpawnStrobeFlash(sector, FASTDARK, 0);
	    sector->special = 4;
	    break;

	  case 8:
	    // GLOWING LIGHT
	    P_SpawnGlowingLight(sector);
	    break;

	  case 9:
	    // SECRET SECTOR
	    totalsecret++;
	    break;

	  case 10:
	    // DOOR CLOSE IN 30 SECONDS
	    P_SpawnDoorCloseIn30(sector);
	    break;

	  case 12:
	    // SYNC STROBE SLOW
	    P_SpawnStrobeFlash(sector, SLOWDARK, 1);
	    break;

	  case 13:
	    // SYNC STROBE FAST
	    P_SpawnStrobeFlash(sector, FASTDARK, 1);
	    break;

	  case 14:
	    // DOOR RAISE IN 5 MINUTES
	    P_SpawnDoorRaiseIn5Mins(sector, i);
	    break;

	  case 17:
	    P_SpawnFireFlicker(sector);
	    break;
	}
    }

    // Id's implementation of scrolling is replaced with generalized
    // scrolling from Boom.
#if 0
    //	Init line EFFECTs
    numlinespecials = 0;
    for (i = 0; i < numlines; i++)
    {
	switch(lines[i].special)
	{
	  case 48:
	    // EFFECT FIRSTCOL SCROLL+
	    linespeciallist[numlinespecials] = &lines[i];
	    numlinespecials++;
	    break;
	}
    }
#endif

    // Init other misc stuff

    P_RemoveAllActivePlats();
    P_RemoveAllActiveCeilings();

    for (i = 0; i < MAXBUTTONS; i++)
	memset(&buttonlist[i], 0, sizeof(button_t));

    P_InitTagLists();	// create xref tables for tags
    P_SpawnScrollers();
    P_InitSlidingDoorFrames();

    for (i = 0; i < numlines; i++)
    {
	switch (lines[i].special)
	{
	    int sec;
	    int s;

	    // floor lighting independently (e.g. lava)
	    case 213:
		sec = sides[*lines[i].sidenum].sector - sectors;
		for (s = -1; (s = P_FindSectorFromLineTag(lines + i, s)) >= 0;)
		    sectors[s].floorlightsec = sec;
		break;

	    // ceiling lighting independently
	    case 261:
		sec = sides[*lines[i].sidenum].sector - sectors;
		for (s = -1; (s = P_FindSectorFromLineTag(lines + i, s)) >= 0;)
		    sectors[s].ceilinglightsec = sec;
		break;
	}
    }
}

//
// The following code handles scrollers with thinkers and the code was
// taken from TeamTNT's Boom engine and integrated into XDoom.
//

//
// This function, with the help of r_plane.c and r_bsp.c, supports generalized
// scrolling floors and walls, with optional mobj-carrying properties, e.g.
// conveyor belts, rivers, etc. A linedef with a special type affects all
// tagged sectors the same way, by creating scrolling and/or object-carrying
// properties. Multiple linedefs may be used on the same sector and are
// cumulative, although the special case of scrolling a floor and carrying
// things on it, requires only one linedef. The linedef's direction determines
// the scrolling direction, and the linedef's length determines the scrolling
// speed. This was designed so that an edge around the sector could be used to
// control the direction of the sector's scrolling, which is usually what is
// desired.
//
void T_Scroll(scroll_t *s)
{
    fixed_t	dx = s->dx, dy = s->dy;

    if (s->control != -1) {
	// compute scroll amounts based on a sectors height change
	fixed_t height = sectors[s->control].floorheight +
		sectors[s->control].ceilingheight;
	fixed_t delta = height - s->last_height;

	s->last_height = height;
	dx = FixedMul(dx, delta);
	dy = FixedMul(dy, delta);
    }

    // add acceleration
    if (s->accel) {
	s->vdx = dx += s->vdx;
	s->vdy = dy += s->vdy;
    }

    // no-op if both (x, y) offsets 0
    if (!(dx | dy))
	return;

    switch (s->type) {
	side_t		*side;
	sector_t	*sec;
	fixed_t		height;
	msecnode_t	*node;
	mobj_t		*thing;

	// scoll wall texture
	case sc_side:
	    side = sides + s->affectee;
	    side->textureoffset += dx;
	    side->rowoffset += dy;
	    break;

	// scroll floor texture
	case sc_floor:
	    sec = sectors + s->affectee;
	    sec->floor_xoffs += dx;
	    sec->floor_yoffs += dy;
	    break;

	// scroll ceiling texture
	case sc_ceiling:
	    sec = sectors + s->affectee;
	    sec->ceiling_xoffs += dx;
	    sec->ceiling_yoffs += dy;
	    break;

	// carry things on floor
	case sc_carry:
	    sec = sectors + s->affectee;
	    height = sec->floorheight;
	    for (node = sec->touching_thinglist; node; node = node->m_snext)
	    {
	      thing = node->m_thing;
	      if (!(thing->flags & MF_NOCLIP) &&
		  !(thing->flags & MF_NOGRAVITY) && !(thing->z > height))
	      {
		thing->momx += dx;
		thing->momy += dy;
	      }
	    }
	    break;
    }
}

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: non-zero if this is an accelerative effect
//
void Add_Scroller(int type, fixed_t dx, fixed_t dy, int control,
			 int affectee, int accel)
{
	scroll_t	*s = Z_Malloc(sizeof(*s), PU_LEVSPEC, (void *)0);

	s->thinker.function.acp1 = (actionf_p1)T_Scroll;
	s->type = type;
	s->dx = dx;
	s->dy = dy;
	s->accel = accel;
	s->vdx = s->vdy = 0;

	if ((s->control = control) != -1)
	    s->last_height = sectors[control].floorheight +
		sectors[control].ceilingheight;

	s->affectee = affectee;

	P_AddThinker(&s->thinker);
}

// Amount (dx, dy) vector linedef is shifted right to get scroll amount
#define SCROLL_SHIFT	5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// This is so scrolling floors and objects on them can move at same speed.
#define CARRYFACTOR ((fixed_t)(FRACUNIT * .09375))

//
// Initialize the generalized scrollers
//
static void P_SpawnScrollers(void)
{
	int	i;
	int	s;
	line_t	*l = lines;

	for (i = 0; i < numlines; i++, l++)
	{
	    fixed_t	dx = l->dx >> SCROLL_SHIFT; // direction and speed
	    fixed_t	dy = l->dy >> SCROLL_SHIFT; // of scrolling
	    int		control = -1;		// no control sector
	    int		accel = 0;		// no acceleration
	    int		special = l->special;
	    fixed_t	x, y, d;

	    // types 245-249 are the same as 250-254 except that the
	    // first side's sector's heights cause scrolling when they
	    // change, and this linedef controls the direction and speed
	    // of the scrolling.
	    if (special >= 245 && special <= 249)
	    {
		special += 250 - 245;
		control = sides[*l->sidenum].sector - sectors;
	    }
	    // Types 214-218 are the same as 250-254 but are accelerative.
	    else if (special >= 214 && special <= 218)
	    {
		accel = 1;
		special += 250 - 214;
		control = sides[*l->sidenum].sector - sectors;
	    }

	    switch (special) {
	    case 48:	// scroll wall left
		Add_Scroller(sc_side, FRACUNIT, 0, -1, lines[i].sidenum[0],
			     accel);
		break;

	    case 85:	// scroll wall right
		Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0],
			     accel);
		break;

	    case 255:	// scroll wall according to sidedef offsets
		s = lines[i].sidenum[0];
		Add_Scroller(sc_side, -sides[s].textureoffset,
			     sides[s].rowoffset, -1, s, accel);
		break;

	    // Scroll tagged wall. Scroll ammount is rotated with respect to
	    // wall's linedef first, so that scrolling towards the wall in a
	    // perpendicular direction is translated into vertical motion,
	    // while scrolling along the wall in a parallel direction is
	    // translated into horizontal motion.
	    case 254:
		for (s = -1; (s = P_FindLineFromLineTag(l, s)) >= 0;)
		  if (s != i)
		  {
		    x = abs(lines[s].dx);
		    y = abs(lines[s].dy);
		    if (y > x)
		      d = x, x = y, y = d;
		    d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y, x)
			>> DBITS] + ANG90) >> ANGLETOFINESHIFT]);
		    x = -FixedDiv(FixedMul(dy, lines[s].dy) +
			FixedMul(dx, lines[s].dx), d);
		    y = -FixedDiv(FixedMul(dx, lines[s].dy) -
			FixedMul(dy, lines[s].dx), d);
		    Add_Scroller(sc_side, x, y, control, lines[s].sidenum[0],
				 accel);
		  }
		break;

	    case 250:	// scroll effect ceiling
		for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
		    Add_Scroller(sc_ceiling, -dx, dy, control, s, accel);
		break;

	    case 251:	// scroll effect floor
		for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
		    Add_Scroller(sc_floor, -dx, dy, control, s, accel);
		break;

	    case 252:	// carry objects on floor, but doesn't scroll
		x = FixedMul(dx, CARRYFACTOR);
		y = FixedMul(dy, CARRYFACTOR);
		for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;)
		    Add_Scroller(sc_carry, x, y, control, s, accel);
		break;

	    case 253:	// scroll and carry objects on floor
		x = FixedMul(dx, CARRYFACTOR);
		y = FixedMul(dy, CARRYFACTOR);
		for (s = -1; (s = P_FindSectorFromLineTag(l, s)) >= 0;) {
		    Add_Scroller(sc_floor, -dx, dy, control, s, accel);
		    Add_Scroller(sc_carry, x, y, control, s, accel);
		}
		break;
	    }
	}
}
