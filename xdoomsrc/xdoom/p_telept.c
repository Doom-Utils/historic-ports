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
//	Teleportation.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "doomdef.h"
#include "s_sound.h"
#include "p_local.h"
#include "sounds.h"
#include "r_state.h"

extern void P_CalcHeight(player_t *player);

//
// TELEPORTATION
//
int EV_Teleport(line_t *line, int side, mobj_t *thing)
{
    int		i;
    mobj_t	*m;
    mobj_t	*fog;
    unsigned	an;
    thinker_t	*thinker;
    sector_t	*sector;
    fixed_t	oldx;
    fixed_t	oldy;
    fixed_t	oldz;

    // don't teleport missiles
    if (thing->flags & MF_MISSILE)
	return 0;

    // Don't teleport if hit back of line,
    //  so you can get out of teleporter.
    if (side == 1)
	return 0;

    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {
	for (thinker = thinkercap.next; thinker != &thinkercap;
		 thinker = thinker->next)
	{
		// not a mobj
		if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
		    continue;

		m = (mobj_t *)thinker;

		// not a teleportman
		if (m->type != MT_TELEPORTMAN)
		    continue;

		sector = m->subsector->sector;
		// wrong sector
		if (sector-sectors != i)
		    continue;

		oldx = thing->x;
		oldy = thing->y;
		oldz = thing->z;

		if (!P_TeleportMove(thing, m->x, m->y))
		    return 0;

		thing->z = thing->floorz;
		if (thing->player)
		    thing->player->viewz = thing->z + thing->player->viewheight;

		// spawn teleport fog at source and destination
		fog = P_SpawnMobj(oldx, oldy, oldz, MT_TFOG);
		S_StartSound(fog, sfx_telept);
		an = m->angle >> ANGLETOFINESHIFT;
		fog = P_SpawnMobj(m->x + 20 * finecosine[an],
				  m->y + 20 * finesine[an],
				  thing->z, MT_TFOG);

		// emit sound, where?
		S_StartSound(fog, sfx_telept);

		// don't move for a bit
		if (thing->player)
		    thing->reactiontime = 18;

		thing->angle = m->angle;
		thing->momx = thing->momy = thing->momz = 0;
		return 1;
	}
    }
    return 0;
}

//
// Silent TELEPORTATION
// Taken from Boom sources, by Lee Killough
//
int EV_SilentTeleport(line_t *line, int side, mobj_t *thing)
{
    int		i;
    mobj_t	*m;
    thinker_t	*th;

    // don't teleport missiles
    // don't teleport if hit back of line
    //   so one can get out of teleporter
    if (side || thing->flags & MF_MISSILE)
	return 0;

    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	    if (th->function.acp1 == (actionf_p1)P_MobjThinker &&
		(m = (mobj_t *)th)->type == MT_TELEPORTMAN &&
		 m->subsector->sector - sectors == i)
	    {
		// Height of thing above ground, in case of mid-air teleports
		fixed_t z = thing->z - thing->floorz;

		// Get the angle between the exit thing and source linedef.
		// Rotate 90 degrees, so that walking perpendicularly across
		// teleporter lindedef causes thing to exit in the direction
		// indicated by the exit thing.
		angle_t angle =
		  R_PointToAngle2(0, 0, line->dx, line->dy) - m->angle + ANG90;

		// sine, cosine of angle adjustment
		fixed_t s = finesine[angle >> ANGLETOFINESHIFT];
		fixed_t c = finecosine[angle >> ANGLETOFINESHIFT];

		// Momentum of thing crossing teleporter linedef
		fixed_t momx = thing->momx;
		fixed_t momy = thing->momy;

		// whether this is a player, and if so, pointer to its player_t
		player_t *player = thing->player;

		// attempt to teleport, aborting if blocked
		if (!P_TeleportMove(thing, m->x, m->y))
		    return 0;

		// rotate thing according to difference in angles
		thing-> angle += angle;

		// adjust z position to be same height above ground as before
		thing->z = z + thing->floorz;

		// rotate thing's momentum to come out of exit just like entered
		thing->momx = FixedMul(momx, c) - FixedMul(momy, s);
		thing->momy = FixedMul(momy, c) + FixedMul(momx, s);

		// adjust player's view, in case there has been a height change
		// voodoo dolls are excluded by making sure player->mo == thing
		if (player && player->mo == thing)
		{
		    // save the current deltaviewheight, used in stepping
		    fixed_t deltaviewheight = player->deltaviewheight;

		    // clear deltaviewheight, since we don't want any changes
		    player->deltaviewheight = 0;

		    // set player's view according to the newly set parameters
		    P_CalcHeight(player);

		    // reset the delta to have the same dynamics as before
		    player->deltaviewheight = deltaviewheight;
		}
		return 1;
	    }
    return 0;
}

// maximum fixed_t units to move object to avoid hiccups
#define FUDGEFACTOR	10

//
// Silent linedef-based TELEPORTATION
// Primarily for rooms-over-rooms etc., this is the complete
// player-preserving kind of teleporter.
// Taken from Boom sources, by Lee Killough
//
int EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing,
			  boolean reverse)
{
    int		i;
    line_t	*l;

    if (side || thing->flags & MF_MISSILE)
	return 0;

    for (i = -1; (i = P_FindLineFromLineTag(line, i)) >= 0;)
	if ((l = lines + i) != line && l->backsector)
	{
	    // get the thing's position along the source linedef
	    fixed_t pos = abs(line->dx) > abs(line->dy) ?
		FixedDiv(thing->x - line->v1->x, line->dx) :
		FixedDiv(thing->y - line->v1->y, line->dy);

	    // Get the angle between the two linedefs, for rotating
	    // orientation and momentum. Rotate 180 degrees, and flip
	    // the position across the exit linedef, if reversed.
	    angle_t angle = (reverse ? pos = FRACUNIT - pos, 0 : ANG180) +
			    R_PointToAngle2(0, 0, l->dx, l->dy) -
			    R_PointToAngle2(0, 0, line->dx, line->dy);

	    // interpolate position across the exit linedef
	    fixed_t x = l->v2->x - FixedMul(pos, l->dx);
	    fixed_t y = l->v2->y - FixedMul(pos, l->dy);

	    // sine, cosine of angle adjustment
	    fixed_t s = finesine[angle >> ANGLETOFINESHIFT];
	    fixed_t c = finecosine[angle >> ANGLETOFINESHIFT];

	    // maximum distance thing can be moved away from interpolated
	    // exit, to ensure that it is on the correct side of the exit
	    // linedef
	    int fudge = FUDGEFACTOR;

	    // Whether this is a player, and if so, a pointer to its player_t.
	    // Voodoo dolls are excluded by making sure thing->player->mo ==
	    // thing.
	    player_t *player = thing->player && thing->player->mo == thing ?
			       thing->player : (player_t *)0;

	    // whether walking towards first side of exit linedef steps down
	    int stepdown =
		l->frontsector->floorheight < l->backsector->floorheight;

	    // height of thing above ground
	    fixed_t z = thing->z - thing->floorz;

	    // Side to exit the linedef on positionally.
	    //
	    // Notes:
	    //
	    // This flag concerns exit positions, not momentum. Due to
	    // roundoff error, the thing can land on either the left or
	    // the right side of the exit linedef, and steps must be
	    // taken to make sure it does not end up on the wrong side.
	    //
	    // Exit momentum is always towards side 1 in a reversed
	    // teleporter, and always towards side 0 otherwise.
	    //
	    // Exiting positionally on side 1 is always safe, as far
	    // as avoiding oscillations and stuck-in-wall problems,
	    // but may not be optimum for non-reversed teleporters.
	    //
	    // Exiting on side 1 slightly improves player viewing
	    // when going down a step on a non-reverse teleporter.
	    int side = reverse || (player && stepdown);

	    // make sure we are on correct side of exit linedef
	    while (P_PointOnLineSide(x, y, l) != side && --fudge >= 0)
		if (abs(l->dx) > abs(l->dy))
		    y -= l->dx < 0 != side ? -1 : 1;
		else
		    x += l->dy < 0 != side ? -1 : 1;

	    // attempt to teleport, aborting if blocked
	    if (!P_TeleportMove(thing, x, y))
		return 0;

	    // Adjust z position to be same height above ground as before.
	    // Ground level at the exit is measured as the hight of the
	    // two floor heights at the exit linedef.
	    thing->z = z + sides[l->sidenum[stepdown]].sector->floorheight;

	    // rotate thing's orientation according to difference in
	    // linedef angles
	    thing-> angle += angle;

	    // momentum of thing crossing teleporter linedef
	    x = thing->momx;
	    y = thing->momy;

	    // rotate thing's momentum to come out of exit just like it entered
	    thing->momx = FixedMul(x, c) - FixedMul(y, s);
	    thing->momy = FixedMul(y, c) + FixedMul(x, s);

	    // adjust a player's view, in case there has been a height change
	    if (player)
	    {
		// save the current deltaviewheight, used in stepping
		fixed_t deltaviewheight = player->deltaviewheight;

		// clear deltaviewheight, since we don't want any changes now
		player->deltaviewheight = 0;

		// set player's view according to the newly set parameters
		P_CalcHeight(player);

		// reset the delta to have the same dynamics as before
		player->deltaviewheight = deltaviewheight;
	    }

	    return 1;
	}

	return 0;
}
