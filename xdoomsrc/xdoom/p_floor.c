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
//	General plane mover and floor mover action routines.
//	Floor motion, pure changer types, raising stairs, donuts, elevators.
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
// Move a plane (floor or ceiling) and check for crushing. Called
// every tick by all actions that move floors or ceilings.
//
// Passed the sector to move a plane in, the speed to move it at.
// the dest height it is to archive, whether it crushes obstacles,
// whether it moves a floor or ceiling, and the direction up or down
// to move.
//
// Returns a result_e:
//   ok - plane moved normally, has not archived destination yet
//   pastdest - plane moved normally and is now at destination height
//   crushed - plane encountered an obstacle, is holding until removed
//
result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest,
		     boolean crush, int floorOrCeiling, int direction)
{
    boolean	flag;
    fixed_t	lastpos;
    fixed_t	destheight;	// used to keep floors/ceilings
				// from moving thru each other

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
		    sector->floorheight = lastpos;
		    P_CheckSector(sector, crush);
		}
		return pastdest;
	    }
	    else
	    {
		lastpos = sector->floorheight;
		sector->floorheight -= speed;
		flag = P_CheckSector(sector, crush);
	    }
	    break;

	  case 1:
	    // UP
	    destheight = dest < sector->ceilingheight ?
			 dest : sector->ceilingheight;
	    if (sector->floorheight + speed > destheight)
	    {
		lastpos = sector->floorheight;
		sector->floorheight = destheight;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    sector->floorheight = lastpos;
		    P_CheckSector(sector, crush);
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
	    destheight = dest > sector->floorheight ?
			 dest : sector->floorheight;
	    if (sector->ceilingheight - speed < destheight)
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight = destheight;
		flag = P_CheckSector(sector, crush);
		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_CheckSector(sector, crush);
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
		}
		return pastdest;
	    }
	    else
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight += speed;
		flag = P_CheckSector(sector, crush);
	    }
	    break;
	}
	break;

    }
    return ok;
}

//
// Move a floor to it's destination (up or down).
// Called once per tick for each moving floor.
//
// Passed a floormove_t structure that contains all pertinent info
// about the move.
//
// Returns nothing.
//
void T_MoveFloor(floormove_t *floor)
{
    result_e	res;

    res = T_MovePlane(floor->sector,
		      floor->speed,
		      floor->floordestheight,
		      floor->crush, 0, floor->direction);

    // make the floor move sound
    if (!(leveltime & 7))
	S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_stnmov);

    // if destination height is reached
    if (res == pastdest)
    {
	if (floor->direction == 1)	// going up
	{
	    switch(floor->type)
	    {
	      case donutRaise:
		floor->sector->special = floor->newspecial;
		floor->sector->floorpic = floor->texture;
		break;

	      case genFloorChgT:
	      case genFloorChg0:
		floor->sector->special = floor->newspecial;
		floor->sector->oldspecial = floor->oldspecial;
	      case genFloorChg:
		floor->sector->floorpic = floor->texture;
		break;

	      default:
		break;
	    }
	}
	else if (floor->direction == -1) // going down
	{
	    switch(floor->type)
	    {
	      case lowerAndChange:
		floor->sector->special = floor->newspecial;
		floor->sector->oldspecial = floor->oldspecial;
		floor->sector->floorpic = floor->texture;

	      case genFloorChgT:
	      case genFloorChg0:
		floor->sector->special = floor->newspecial;
		floor->sector->oldspecial = floor->oldspecial;
	      case genFloorChg:
		floor->sector->floorpic = floor->texture;
		break;

	      default:
		break;
	    }
	}

	floor->sector->floordata = (void *)0;
	P_RemoveThinker(&floor->thinker);

	// implement stair retrigger lockout while still building
	// note this only applies to the retriggerable generalized stairs
	if (floor->sector->stairlock == -2) // if this sector is stairlocked
	{
	    sector_t *sec = floor->sector;

	    sec->stairlock = -1;	// thinker done, promote lock to -1

	    while (sec->prevsec != -1 && sectors[sec->prevsec].stairlock != -1)
		sec = &sectors[sec->prevsec]; // search for non-done thinker

	    if (sec->prevsec == -1)	// if all thinkers previous are done
	    {
		sec = floor->sector;	// search forward
		while (sec->nextsec != -1 &&
		       sectors[sec->nextsec].stairlock != -2)
		    sec = &sectors[sec->nextsec];
		if (sec->nextsec == -1) // if all thinkers ahead are done too
		{
		    while (sec->prevsec != -1) // clear all locks
		    {
			sec->stairlock = 0;
			sec = &sectors[sec->prevsec];
		    }
		    sec->stairlock = 0;
		}
	    }
	}

	// make floor stop sound
	S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_pstop);
    }
}

//
// Handle regular and extended floor types
//
// Passed the line that activated the floor and the type of floor motion.
// Returns true if a thinker was created.
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

    // move all floors with the same tag as the linedef
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];

	// Don't start a second thinker on the same floor
	if (P_SectorActive(floor_special, sec))
	    continue;

	// new floor thinker
	rtn = 1;
	floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
	P_AddThinker(&floor->thinker);
	sec->floordata = floor;
	floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	floor->type = floortype;
	floor->crush = false;

	// setup the thinker according to the linedef type
	switch(floortype)
	{
	  case lowerFloor:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = P_FindHighestFloorSurrounding(sec);
	    break;

	  case lowerFloor24:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
	    break;

	  case lowerFloor32Turbo:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED * 4;
	    floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
	    break;

	  case lowerFloorToLowest:
	    floor->direction = -1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = P_FindLowestFloorSurrounding(sec);
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
	    floor->floordestheight = P_FindHighestFloorSurrounding(sec);
	    if (floor->floordestheight != sec->floorheight)
		floor->floordestheight += 8 * FRACUNIT;
	    break;

	  case raiseFloorCrush:
	    floor->crush = true;
	  case raiseFloor:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED;
	    floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
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
	    floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
	    break;

	  case raiseFloor32Turbo:
	    floor->direction = 1;
	    floor->sector = sec;
	    floor->speed = FLOORSPEED * 4;
	    floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
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
	    floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
	    sec->floorpic = line->frontsector->floorpic;
	    sec->special = line->frontsector->special;
	    sec->oldspecial = line->frontsector->oldspecial;
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
		      if (side->bottomtexture > 0 || !side->bottomtexture)
			  if (textureheight[side->bottomtexture] < minsize)
			      minsize = textureheight[side->bottomtexture];
		      side = getSide(secnum, i, 1);
		      if (side->bottomtexture > 0 || !side->bottomtexture)
			  if (textureheight[side->bottomtexture] < minsize)
			      minsize = textureheight[side->bottomtexture];
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
	    floor->floordestheight = P_FindLowestFloorSurrounding(sec);
	    floor->texture = sec->floorpic;
	    floor->newspecial = sec->special;
	    floor->oldspecial = sec->oldspecial;
	    sec = P_FindModelFloorSector(floor->floordestheight, sec - sectors);
	    if (sec)
	    {
		floor->texture = sec->floorpic;
		floor->newspecial = sec->special;
		floor->oldspecial = sec->oldspecial;
	    }
	    break;

	  default:
	    break;
	}
    }
    return rtn;
}

//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed.
// Returns true if any sector changes.
//
// This is from Boom.
//
int EV_DoChange(line_t *line, change_e changetype)
{
    int		secnum;
    int		rtn;
    sector_t	*sec;
    sector_t	*secm;

    secnum = -1;
    rtn = 0;

    // change all sectors with same tag as the linedef
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];
	rtn = 1;

	// handle trigger or numeric change type
	switch(changetype)
	{
	    case trigChangeOnly:
		sec->floorpic = line->frontsector->floorpic;
		sec->special = line->frontsector->special;
		sec->oldspecial = line->frontsector->oldspecial;
		break;

	    case numChangeOnly:
		secm = P_FindModelFloorSector(sec->floorheight, secnum);
		if (secm)	// if no model, no change
		{
		    sec->floorpic = secm->floorpic;
		    sec->special = secm->special;
		    sec->oldspecial = secm->oldspecial;
		}
		break;

	    default:
		break;
	}
    }
    return rtn;
}

//
// Move an elevator to it's destination (up or down)
//
// Called once per tick for each moving floor.
//
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
    else				// moving up
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

	// make floor stop sound
	S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_pstop);
    }
}

//
// Build a staircase
//
// Handles staircase building. A sequence of sectors chosen by algorithm
// rise at a speed indicated to a height that increases by the stepsize
// each step.
//
// Passed the linedef triggering the stairs and the type of stair rise.
// Returns true if any thinkers are created.
//
int EV_BuildStairs(line_t *line, stair_e type)
{
    int			secnum;
    int			osecnum;
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

    // start a stair at each sector tagged the same as the linedef
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];

	// don't start a stair if the first step's floor is already moving
	if (P_SectorActive(floor_special, sec))
	    continue;

	// create new floor thinker for first step
	rtn = 1;
	floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
	P_AddThinker(&floor->thinker);
	sec->floordata = floor;
	floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	floor->direction = 1;
	floor->sector = sec;
	floor->type = buildStair;

	// set up the speed and stepsize according to the stairs type
	switch(type)
	{
	  default:
	  case build8:
	    speed = FLOORSPEED / 4;
	    stairsize = 8 * FRACUNIT;
	    floor->crush = false;
	    break;

	  case turbo16:
	    speed = FLOORSPEED * 4;
	    stairsize = 16 * FRACUNIT;
	    floor->crush = true;
	    break;
	}
	floor->speed = speed;
	height = sec->floorheight + stairsize;
	floor->floordestheight = height;

	texture = sec->floorpic;
	osecnum = secnum;	// preserve loop index

	// Find next sector to raise
	// 1.	Find 2-sided line with same sector side[0] (lowest numbered)
	// 2.	Other side is the next sector to raise
	// 3.	Unless already moving, or different texture, then stop building
	do
	{
	    ok = 0;
	    for (i = 0; i < sec->linecount; i++)
	    {
		if (!((sec->lines[i])->flags & ML_TWOSIDED))
		    continue;

		tsec = (sec->lines[i])->frontsector;
		newsecnum = tsec - sectors;

		if (secnum != newsecnum)
		    continue;

		tsec = (sec->lines[i])->backsector;
		if (!tsec)
		    continue;
		newsecnum = tsec - sectors;

		// if sectors floor is different texture, look for another
		if (tsec->floorpic != texture)
		    continue;

		if (democompat)
		    height += stairsize;

		// if sectors floor already moving, look for another
		if (P_SectorActive(floor_special, tsec))
		    continue;

		if (!democompat)
		    height += stairsize;

		sec = tsec;
		secnum = newsecnum;

		// create and initialize a thinker for the next step
		floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, (void *)0);
		P_AddThinker(&floor->thinker);

		sec->floordata = floor;
		floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
		floor->direction = 1;
		floor->sector = sec;
		floor->speed = speed;
		floor->floordestheight = height;
		floor->type = buildStair;
		floor->crush = type == build8 ? false : true;
		ok = 1;
		break;
	    }
	} while(ok);		// continue until no next step is found
	secnum = osecnum;	// restore loop index
    }
    return rtn;
}

//
// From Boom:
//
// Handle elevator linedef types, can move floor and ceiling in parallel.
//
// Passed the linedef that triggered the elevator and the elevator action
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

	    default:
		break;
	}
    }
    return rtn;
}
