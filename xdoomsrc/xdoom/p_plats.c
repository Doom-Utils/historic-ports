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
//	Plats (i.e. elevator platforms) code, raising/lowering.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "i_system.h"
#include "z_zone.h"
#include "m_random.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"
#include "sounds.h"

platlist_t	*activeplats;

//
// Move a plat up and down
//
void T_PlatRaise(plat_t *plat)
{
    result_e	res;

    switch(plat->status)
    {
      case up:
	res = T_MovePlane(plat->sector,
			  plat->speed,
			  plat->high,
			  plat->crush, 0, 1);

	if (plat->type == raiseAndChange
	    || plat->type == raiseToNearestAndChange)
	{
	    if (!(leveltime & 7))
		S_StartSound((mobj_t *)&plat->sector->soundorg,
			     sfx_stnmov);
	}

	if (res == crushed && (!plat->crush))
	{
	    plat->count = plat->wait;
	    plat->status = down;
	    S_StartSound((mobj_t *)&plat->sector->soundorg,
			 sfx_pstart);
	}
	else
	{
	    if (res == pastdest)
	    {
		plat->count = plat->wait;
		plat->status = waiting;
		S_StartSound((mobj_t *)&plat->sector->soundorg,
			     sfx_pstop);

		switch(plat->type)
		{
		  case blazeDWUS:
		  case downWaitUpStay:
		    P_RemoveActivePlat(plat);
		    break;

		  case raiseAndChange:
		  case raiseToNearestAndChange:
		    P_RemoveActivePlat(plat);
		    break;

		  default:
		    break;
		}
	    }
	}
	break;

      case	down:
	res = T_MovePlane(plat->sector, plat->speed, plat->low, false, 0, -1);

	if (res == pastdest)
	{
	    plat->count = plat->wait;
	    plat->status = waiting;
	    S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstop);
	}
	break;

      case	waiting:
	if (!--plat->count)
	{
	    if (plat->sector->floorheight == plat->low)
		plat->status = up;
	    else
		plat->status = down;
	    S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstart);
	}
      case	in_stasis:
	break;
    }
}

//
// Do Platforms
//  "amount" is only used for SOME platforms.
//
int EV_DoPlat(line_t *line, plattype_e type, int amount)
{
    plat_t	*plat;
    int		secnum;
    int		rtn;
    sector_t	*sec;

    secnum = -1;
    rtn = 0;

    //	Activate all <type> plats that are in_stasis
    switch(type)
    {
      case perpetualRaise:
	P_ActivateInStasis(line->tag);
	break;

      default:
	break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];

	if (sec->floordata)
	    continue;

	// Find lowest & highest floors around sector
	rtn = 1;
	plat = Z_Malloc(sizeof(*plat), PU_LEVSPEC, (void *)0);
	P_AddThinker(&plat->thinker);

	plat->type = type;
	plat->sector = sec;
	plat->sector->floordata = plat;
	plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
	plat->crush = false;
	plat->tag = line->tag;

	switch(type)
	{
	  case raiseToNearestAndChange:
	    plat->speed = PLATSPEED / 2;
	    sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
	    plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
	    plat->wait = 0;
	    plat->status = up;
	    // NO MORE DAMAGE, IF APPLICABLE
	    sec->special = 0;

	    S_StartSound((mobj_t *)&sec->soundorg, sfx_stnmov);
	    break;

	  case raiseAndChange:
	    plat->speed = PLATSPEED / 2;
	    sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
	    plat->high = sec->floorheight + amount * FRACUNIT;
	    plat->wait = 0;
	    plat->status = up;

	    S_StartSound((mobj_t *)&sec->soundorg, sfx_stnmov);
	    break;

	  case downWaitUpStay:
	    plat->speed = PLATSPEED * 4;
	    plat->low = P_FindLowestFloorSurrounding(sec);

	    if (plat->low > sec->floorheight)
		plat->low = sec->floorheight;

	    plat->high = sec->floorheight;
	    plat->wait = 35 * PLATWAIT;
	    plat->status = down;
	    S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);
	    break;

	  case blazeDWUS:
	    plat->speed = PLATSPEED * 8;
	    plat->low = P_FindLowestFloorSurrounding(sec);

	    if (plat->low > sec->floorheight)
		plat->low = sec->floorheight;

	    plat->high = sec->floorheight;
	    plat->wait = 35 * PLATWAIT;
	    plat->status = down;
	    S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);
	    break;

	  case perpetualRaise:
	    plat->speed = PLATSPEED;
	    plat->low = P_FindLowestFloorSurrounding(sec);

	    if (plat->low > sec->floorheight)
		plat->low = sec->floorheight;

	    plat->high = P_FindHighestFloorSurrounding(sec);

	    if (plat->high < sec->floorheight)
		plat->high = sec->floorheight;

	    plat->wait = 35 * PLATWAIT;
	    plat->status = P_Random() & 1;

	    S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);
	    break;
	}
	P_AddActivePlat(plat);
    }
    return rtn;
}

//
// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active plats. It also avoids spending as much
// time searching for active plats. Previously a
// fixed-size array was used. with NULL indicating
// empty entries, while now a double-linked list
// is used.
// This sources were taken from Boom.
//

//
// P_ActivateInStasis()
//
// Activate a plat that as been put in statis
// (stopped perpetual floor, instant floor/ceil toggle)
//
// Passed the tag of the plat that should be reactivated
// Returns nothing
//
void P_ActivateInStasis(int tag)
{
    platlist_t	*pl;

    for (pl = activeplats; pl; pl = pl->next)
    {
	plat_t	*plat = pl->plat;

	if (plat->tag == tag && plat->status == in_stasis)
	{
	    plat->status = plat->oldstatus;
	    plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;
	}
    }
}

//
// EV_StopPlat()
//
// Handler for "stop perpetual floor" linedef type
//
// Passed the linedef that stopped plat
// Returns nothing
//
void EV_StopPlat(line_t *line)
{
    platlist_t	*pl;

    for (pl = activeplats; pl; pl = pl->next)
    {
	plat_t	*plat = pl->plat;

	if (plat->status != in_stasis && plat->tag == line->tag)
	{
	    plat->oldstatus = plat->status;
	    plat->status = in_stasis;
	    plat->thinker.function.acv = (actionf_v)0;
	}
    }
}

//
// P_AddActivePlat()
//
// Add a plat to the head of the active plat list
//
// Passed a pointer to the plat to add
// returns nothing
void P_AddActivePlat(plat_t *plat)
{
    platlist_t	*list = Z_Malloc(sizeof(*list), PU_STATIC, (void *)0);

    list->plat = plat;
    plat->list = list;
    if ((list->next = activeplats))
	list->next->prev = &list->next;
    list->prev = &activeplats;
    activeplats = list;
}

//
// P_RemoveActivePlat()
//
// Remove a plat from the active plat list
//
// Passed a pointer to the plat to remove
// Returns nothing
//
void P_RemoveActivePlat(plat_t *plat)
{
    platlist_t	*list = plat->list;

    plat->sector->floordata = (void *)0;
    P_RemoveThinker(&plat->thinker);
    if ((*list->prev = list->next))
	list->next->prev = list->prev;
    Z_Free(list);
}

//
// P_RemoveAllActivePlats()
//
// Remove all plats from the active plat list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActivePlats(void)
{
    while (activeplats)
    {
	platlist_t	*next = activeplats->next;

	Z_Free(activeplats);
	activeplats = next;
    }
}
