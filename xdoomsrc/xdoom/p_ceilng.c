// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
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
// DESCRIPTION:  Ceiling aninmation (lowering, crushing, raising)
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "doomstat.h"
#include "r_state.h"
#include "sounds.h"
#include "z_zone.h"

//
// CEILINGS
//
ceilinglist_t	*activeceilings;

//
// T_MoveCeiling
//
void T_MoveCeiling(ceiling_t *ceiling)
{
    result_e	res;

    switch(ceiling->direction)
    {
      case 0:
	// IN STASIS
	break;
      case 1:
	// UP
	res = T_MovePlane(ceiling->sector,
			  ceiling->speed,
			  ceiling->topheight,
			  false, 1, ceiling->direction);

	if (!(leveltime & 7))
	{
	    switch(ceiling->type)
	    {
	      case silentCrushAndRaise:
		break;
	      default:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_stnmov);
		break;
	    }
	}

	if (res == pastdest)
	{
	    switch(ceiling->type)
	    {
	      case raiseToHighest:
		P_RemoveActiveCeiling(ceiling);
		break;

	      case silentCrushAndRaise:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_pstop);
	      case fastCrushAndRaise:
	      case crushAndRaise:
		ceiling->direction = -1;
		break;

	      default:
		break;
	    }
	}
	break;

      case -1:
	// DOWN
	res = T_MovePlane(ceiling->sector,
			  ceiling->speed,
			  ceiling->bottomheight,
			  ceiling->crush,1,ceiling->direction);

	if (!(leveltime & 7))
	{
	    switch(ceiling->type)
	    {
	      case silentCrushAndRaise:
		break;
	      default:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_stnmov);
	    }
	}

	if (res == pastdest)
	{
	    switch(ceiling->type)
	    {
	      case silentCrushAndRaise:
		S_StartSound((mobj_t *)&ceiling->sector->soundorg,
			     sfx_pstop);
	      case crushAndRaise:
		ceiling->speed = CEILSPEED;
	      case fastCrushAndRaise:
		ceiling->direction = 1;
		break;

	      case lowerAndCrush:
	      case lowerToFloor:
		P_RemoveActiveCeiling(ceiling);
		break;

	      default:
		break;
	    }
	}
	else // ( res != pastdest )
	{
	    if (res == crushed)
	    {
		switch(ceiling->type)
		{
		  case silentCrushAndRaise:
		  case crushAndRaise:
		  case lowerAndCrush:
		    ceiling->speed = CEILSPEED / 8;
		    break;

		  default:
		    break;
		}
	    }
	}
	break;
    }
}

//
// EV_DoCeiling
// Move a ceiling up/down and all around!
//
int EV_DoCeiling(line_t *line, ceiling_e type)
{
    int		secnum;
    int		rtn;
    sector_t	*sec;
    ceiling_t	*ceiling;

    secnum = -1;
    rtn = 0;

    //	Reactivate in-stasis ceilings...for certain types.
    switch(type)
    {
      case fastCrushAndRaise:
      case silentCrushAndRaise:
      case crushAndRaise:
	P_ActivateInStasisCeiling(line);
      default:
	break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];
	if (sec->ceilingdata)
	    continue;

	// new door thinker
	rtn = 1;
	ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, (void *)0);
	P_AddThinker(&ceiling->thinker);
	sec->ceilingdata = ceiling;
	ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
	ceiling->sector = sec;
	ceiling->crush = false;

	switch(type)
	{
	  case fastCrushAndRaise:
	    ceiling->crush = true;
	    ceiling->topheight = sec->ceilingheight;
	    ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
	    ceiling->direction = -1;
	    ceiling->speed = CEILSPEED * 2;
	    break;

	  case silentCrushAndRaise:
	  case crushAndRaise:
	    ceiling->crush = true;
	    ceiling->topheight = sec->ceilingheight;
	  case lowerAndCrush:
	  case lowerToFloor:
	    ceiling->bottomheight = sec->floorheight;
	    if (type != lowerToFloor)
		ceiling->bottomheight += 8 * FRACUNIT;
	    ceiling->direction = -1;
	    ceiling->speed = CEILSPEED;
	    break;

	  case raiseToHighest:
	    ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
	    ceiling->direction = 1;
	    ceiling->speed = CEILSPEED;
	    break;
	}

	ceiling->tag = sec->tag;
	ceiling->type = type;
	P_AddActiveCeiling(ceiling);
    }
    return rtn;
}

//
// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active ceilings. It also avoids spending as much
// time searching for active ceilings. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.
//

//
// Add an active ceiling
//
void P_AddActiveCeiling(ceiling_t *ceiling)
{
    ceilinglist_t	*list = Z_Malloc(sizeof(*list), PU_STATIC, (void *)0);

    list->ceiling = ceiling;
    ceiling->list = list;
    if ((list->next = activeceilings))
	list->next->prev = &list->next;
    list->prev = &activeceilings;
    activeceilings = list;
}

//
// Remove a ceiling's thinker
//
void P_RemoveActiveCeiling(ceiling_t *ceiling)
{
    ceilinglist_t	*list = ceiling->list;

    ceiling->sector->ceilingdata = (void *)0;
    P_RemoveThinker(&ceiling->thinker);
    if ((*list->prev = list->next))
	list->next->prev = list->prev;
    Z_Free(list);
}

//
// Remove all ceilings from the active ceiling list
//
void P_RemoveAllActiveCeilings(void)
{
    while (activeceilings)
    {
	ceilinglist_t	*next = activeceilings->next;
	Z_Free(activeceilings);
	activeceilings = next;
    }
}

//
// Restart a ceiling that's in-stasis
//
void P_ActivateInStasisCeiling(line_t *line)
{
    ceilinglist_t	*cl;

    for (cl = activeceilings; cl; cl = cl->next)
    {
	ceiling_t	*ceiling = cl->ceiling;

	if (ceiling->tag == line->tag && ceiling->direction == 0)
	{
	    ceiling->direction = ceiling->olddirection;
	    ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;
	}
    }
}

//
// EV_CeilingCrushStop
// Stop a ceiling from crushing!
//
int EV_CeilingCrushStop(line_t *line)
{
    ceilinglist_t	*cl;
    int			rtn = 0;

    for (cl = activeceilings; cl; cl = cl->next)
    {
	ceiling_t	*ceiling = cl->ceiling;

	if (ceiling->tag == line->tag && ceiling->direction != 0)
	{
	    ceiling->olddirection = ceiling->direction;
	    ceiling->thinker.function.acv = (actionf_v)0;
	    ceiling->direction = 0;		// in-stasis
	    rtn = 1;
	}
    }

    return rtn;
}
