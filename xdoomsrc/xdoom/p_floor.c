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
//	Floor animation: raising stairs.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "p_spec.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"
#include "sounds.h"

//
// FLOORS
//

//
// Move a plane (floor or ceiling) and check for crushing
//
result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest,
		     boolean crush, int floorOrCeiling, int direction)
{
    boolean	flag;
    fixed_t	lastpos;

    switch(floorOrCeiling)
    {
      case 0:
	// FLOOR
	switch(direction)
	{
	  case -1:
	    // DOWN
	    if (sector->floorheight - speed < dest)
	    {
		lastpos = sector->floorheight;
		sector->floorheight = dest;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    sector->floorheight =lastpos;
		    P_CheckSector(sector, crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		lastpos = sector->floorheight;
		sector->floorheight -= speed;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    sector->floorheight = lastpos;
		    P_CheckSector(sector, crush);
		    return crushed;
		}
	    }
	    break;

	  case 1:
	    // UP
	    if (sector->floorheight + speed > dest)
	    {
		lastpos = sector->floorheight;
		sector->floorheight = dest;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    sector->floorheight = lastpos;
		    P_CheckSector(sector, crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		// COULD GET CRUSHED
		lastpos = sector->floorheight;
		sector->floorheight += speed;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    if (crush == true)
			return crushed;
		    sector->floorheight = lastpos;
		    P_CheckSector(sector, crush);
		    return crushed;
		}
	    }
	    break;
	}
	break;

      case 1:
	// CEILING
	switch(direction)
	{
	  case -1:
	    // DOWN
	    if (sector->ceilingheight - speed < dest)
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight = dest;
		flag = P_CheckSector(sector, crush);

		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_CheckSector(sector, crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		// COULD GET CRUSHED
		lastpos = sector->ceilingheight;
		sector->ceilingheight -= speed;
		flag = P_CheckSector(sector, crush);

		if (flag == true)
		{
		    if (crush == true)
			return crushed;
		    sector->ceilingheight = lastpos;
		    P_CheckSector(sector, crush);
		    return crushed;
		}
	    }
	    break;

	  case 1:
	    // UP
	    if (sector->ceilingheight + speed > dest)
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight = dest;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_CheckSector(sector, crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight += speed;
		flag = P_CheckSector(sector, crush);
// UNUSED
#if 0
		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_CheckSector(sector, crush);
		    return crushed;
		}
#endif
	    }
	    break;
	}
	break;

    }
    return ok;
}

//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void T_MoveFloor(floormove_t *floor)
{
    result_e	res;

    res = T_MovePlane(floor->sector,
		      floor->speed,
		      floor->floordestheight,
		      floor->crush, 0, floor->direction);

    if (!(leveltime & 7))
	S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_stnmov);

    if (res == pastdest)
    {
	floor->sector->floordata = (void  *)0;
	floor->sector->ceilingdata = (void  *)0;

	if (floor->direction == 1)
	{
	    switch(floor->type)
	    {
	      case donutRaise:
		floor->sector->special = floor->newspecial;
		floor->sector->floorpic = floor->texture;
	      default:
		break;
	    }
	}
	else if (floor->direction == -1)
	{
	    switch(floor->type)
	    {
	      case lowerAndChange:
		floor->sector->special = floor->newspecial;
		floor->sector->floorpic = floor->texture;
	      default:
		break;
	    }
	}
	P_RemoveThinker(&floor->thinker);

	S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_pstop);
    }
}

//
// HANDLE FLOOR TYPES
//
int EV_DoFloor(line_t *line, floor_e floortype)
{
    int			secnum;
    int			rtn;
    int			i;
    sector_t		*sec;
    floormove_t		*floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];

	// ALREADY MOVING?  IF SO, KEEP GOING...
	if (sec->floordata)
	    continue;

	// new floor thinker
	rtn = 1;
	floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
	P_AddThinker(&floor->thinker);
	sec->floordata = floor;
	floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	floor->type = floortype;
	floor->crush = false;

	switch(floortype)
	{
	  case lowerFloor:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight =
		P_FindHighestFloorSurrounding(sec);
	    break;

	  case lowerFloorToLowest:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight =
		P_FindLowestFloorSurrounding(sec);
	    break;

	  case lowerFloorToNearest:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight =
		P_FindNextLowestFloor(sec, floor->sector->floorheight);
	    break;

	  case turboLower:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED * 4;
	    floor->floordestheight =
		P_FindHighestFloorSurrounding(sec);
	    if (floor->floordestheight != sec->floorheight)
		floor->floordestheight += 8 * FRACUNIT;
	    break;

	  case raiseFloorCrush:
	    floor->crush = true;
	  case raiseFloor:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight =
		P_FindLowestCeilingSurrounding(sec);
	    if (floor->floordestheight > sec->ceilingheight)
		floor->floordestheight = sec->ceilingheight;
	    floor->floordestheight -= (8 * FRACUNIT) *
		(floortype == raiseFloorCrush);
	    break;

	  case raiseFloorTurbo:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED * 4;
	    floor->floordestheight =
		P_FindNextHighestFloor(sec, sec->floorheight);
	    break;

	  case raiseFloorToNearest:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight =
		P_FindNextHighestFloor(sec, sec->floorheight);
	    break;

	  case raiseFloor24:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = floor->sector->floorheight +
		24 * FRACUNIT;
	    break;

	  case raiseFloor512:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = floor->sector->floorheight +
		512 * FRACUNIT;
	    break;

	  case raiseFloor24AndChange:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = floor->sector->floorheight +
		24 * FRACUNIT;
	    sec->floorpic = line->frontsector->floorpic;
	    sec->special = line->frontsector->special;
	    break;

	  case raiseToTexture:
	  {
	      int	minsize = MAXINT;
	      side_t	*side;

	      floor->direction = 1;
	      floor->sector = sec;
	      floor->speed = FLOORSPEED;
	      for (i = 0; i < sec->linecount; i++)
	      {
		  if (twoSided (secnum, i) )
		  {
		      side = getSide(secnum, i, 0);
		      if (side->bottomtexture >= 0)
			  if (textureheight[side->bottomtexture] <
			      minsize)
			      minsize =
				  textureheight[side->bottomtexture];
		      side = getSide(secnum, i, 1);
		      if (side->bottomtexture >= 0)
			  if (textureheight[side->bottomtexture] <
			      minsize)
			      minsize =
				  textureheight[side->bottomtexture];
		  }
	      }
	      floor->floordestheight =
		  floor->sector->floorheight + minsize;
	  }
	  break;

	  case lowerAndChange:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight =
		P_FindLowestFloorSurrounding(sec);
	    floor->texture = sec->floorpic;

	    for (i = 0; i < sec->linecount; i++)
	    {
		if ( twoSided(secnum, i) )
		{
		    if (getSide(secnum, i, 0)->sector-sectors == secnum)
		    {
			sec = getSector(secnum, i, 1);

			if (sec->floorheight == floor->floordestheight)
			{
			    floor->texture = sec->floorpic;
			    floor->newspecial = sec->special;
			    break;
			}
		    }
		    else
		    {
			sec = getSector(secnum, i, 0);

			if (sec->floorheight == floor->floordestheight)
			{
			    floor->texture = sec->floorpic;
			    floor->newspecial = sec->special;
			    break;
			}
		    }
		}
	    }
	  default:
	    break;
	}
    }
    return rtn;
}

//
// MOVE AN ELEVATOR TO IT'S DESTINATION (UP OR DOWN)
//
// Called once per tick for each moving floor.
// Passed an elevator_t structure that contains all pertinent info
// about the move. Supports parallel floor/ceiling motion.
//
// This is from Boom.
//
void T_MoveElevator(elevator_t *elevator)
{
    result_e	res;

    if (elevator->direction < 0)	// moving down
    {
	res = T_MovePlane(elevator->sector, elevator->speed,	// move floor
			  elevator->ceilingdestheight, 0, 1,
			  elevator->direction);
	if (res == ok || res == pastdest)	// don't move ceil if blocked
	    T_MovePlane(elevator->sector, elevator->speed,	// move ceil
			elevator->floordestheight, 0, 0,
			elevator->direction
	    );
    }
    else				// moving down
    {
	res = T_MovePlane(elevator->sector, elevator->speed,
			  elevator->floordestheight, 0, 0,
			  elevator->direction);
	if (res == ok || res == pastdest)	// don't move floor if blocked
	    T_MovePlane(elevator->sector, elevator->speed,
			elevator->ceilingdestheight, 0, 1,
			elevator->direction);
    }

    // make floor move sound
    if (!(leveltime & 7))
	S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_stnmov);

    if (res == pastdest)	// if destination height archived
    {
	elevator->sector->floordata = (void *)0;
	elevator->sector->ceilingdata = (void *)0;
	P_RemoveThinker(&elevator->thinker);

	// make flor stop sound
	S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_pstop);
    }
}

//
// BUILD A STAIRCASE!
//
int EV_BuildStairs(line_t *line, stair_e type)
{
    int			secnum;
    int			height;
    int			i;
    int			newsecnum;
    int			texture;
    int			ok;
    int			rtn;

    sector_t		*sec;
    sector_t		*tsec;

    floormove_t		*floor;

    fixed_t		stairsize;
    fixed_t		speed;

    secnum = -1;
    rtn = 0;
    stairsize = speed = 0;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];

	// ALREADY MOVING?  IF SO, KEEP GOING...
	if (sec->floordata)
	    continue;

	// new floor thinker
	rtn = 1;
	floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
	P_AddThinker(&floor->thinker);
	sec->floordata = floor;
	floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	floor->direction = 1;
	floor->sector = sec;
	switch(type)
	{
	  case build8:
	    speed = FLOORSPEED / 4;
	    stairsize = 8 * FRACUNIT;
	    break;

	  case turbo16:
	    speed = FLOORSPEED * 4;
	    stairsize = 16 * FRACUNIT;
	    break;
	}
	floor->speed = speed;
	height = sec->floorheight + stairsize;
	floor->floordestheight = height;

	texture = sec->floorpic;

	// Find next sector to raise
	// 1.	Find 2-sided line with same sector side[0]
	// 2.	Other side is the next sector to raise
	do
	{
	    ok = 0;
	    for (i = 0; i < sec->linecount; i++)
	    {
		if (!((sec->lines[i])->flags & ML_TWOSIDED))
		    continue;

		tsec = (sec->lines[i])->frontsector;
		newsecnum = tsec-sectors;

		if (secnum != newsecnum)
		    continue;

		tsec = (sec->lines[i])->backsector;
		newsecnum = tsec - sectors;

		if (tsec->floorpic != texture)
		    continue;

		height += stairsize;

		if (tsec->floordata)
		    continue;

		sec = tsec;
		secnum = newsecnum;
		floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);

		P_AddThinker(&floor->thinker);

		sec->floordata = floor;
		floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
		floor->direction = 1;
		floor->sector = sec;
		floor->speed = speed;
		floor->floordestheight = height;
		ok = 1;
		break;
	    }
	} while(ok);
    }
    return rtn;
}

//
// Handle elevator linedef types, can move floor and ceiling in parallel.
// This is from Boom.
//
int EV_DoElevator(line_t *line, elevator_e elevtype)
{
    int		secnum;
    int		rtn;
    sector_t	*sec;
    elevator_t	*elevator;

    secnum = -1;
    rtn = 0;

    // act on all sectors with the same tag as the triggering linedef
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];

	// if either floor or ceiling is already activated, skip it
	if (sec->floordata || sec->ceilingdata)
	    continue;

	// create and initialize new elevator thinker
	rtn = 1;
	elevator = Z_Malloc(sizeof(*elevator), PU_LEVSPEC, (void *)0);
	P_AddThinker(&elevator->thinker);
	sec->floordata = elevator;
	sec->ceilingdata = elevator;
	elevator->thinker.function.acp1 = (actionf_p1)T_MoveElevator;
	elevator->type = elevtype;

	// set up the fields according to the type of elevator action
	switch (elevtype)
	{
	    // elevator down to next floor
	    case elevateDown:
		elevator->direction = -1;
		elevator->sector = sec;
		elevator->speed = ELEVATORSPEED;
		elevator->floordestheight =
		    P_FindNextLowestFloor(sec, sec->floorheight);
		elevator->ceilingdestheight =
		    elevator->floordestheight + sec->ceilingheight
		    - sec->floorheight;
		break;

	    // elevator up to next floor
	    case elevateUp:
		elevator->direction = 1;
		elevator->sector = sec;
		elevator->speed = ELEVATORSPEED;
		elevator->floordestheight =
		    P_FindNextHighestFloor(sec, sec->floorheight);
		elevator->ceilingdestheight =
		    elevator->floordestheight + sec->ceilingheight
		    - sec->floorheight;
		break;

	    // elevator to floor height of activating switch's front sector
	    case elevateCurrent:
		elevator->sector = sec;
		elevator->speed = ELEVATORSPEED;
		elevator->floordestheight = line->frontsector->floorheight;
		elevator->ceilingdestheight =
		    elevator->floordestheight + sec->ceilingheight
		    - sec->floorheight;
		elevator->direction =
		    elevator->floordestheight > sec->floorheight ? 1 : -1;
		break;
	}
    }
    return rtn;
}
