// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_map.h,v 1.2 1998/05/07 00:53:07 killough Exp $
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
// DESCRIPTION:
//      Map functions
//
//-----------------------------------------------------------------------------

#ifndef __P_MAP__
#define __P_MAP__

#include "r_defs.h"
#include "d_player.h"

#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes the spider demon
// is larger, but we do not have any moving sectors nearby
#define MAXRADIUS       (32*FRACUNIT)

// killough 3/15/98: add fourth argument to P_TryMove
boolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, boolean dropoff);

boolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y);
void    P_SlideMove(mobj_t *mo);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);
void    P_UseLines(player_t *player);
boolean P_ChangeSector(sector_t *sector, boolean crunch);
fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance);
void    P_LineAttack(mobj_t *t1, angle_t angle, fixed_t distance,
                     fixed_t slope, int damage );
void    P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage);
boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);

//jff 3/19/98 P_CheckSector(): new routine to replace P_ChangeSector()
boolean P_CheckSector(sector_t *sector, boolean crunch);
void    P_DelSeclist(msecnode_t *);                         // phares 3/16/98
void    P_CreateSecNodeList(mobj_t*,fixed_t,fixed_t);       // phares 3/14/98
int     P_GetMoveFactor(mobj_t* mo);                        // phares  3/6/98
boolean Check_Sides(mobj_t *, int, int);                    // phares


// If "floatok" true, move would be ok if within "tmfloorz - tmceilingz".
extern boolean floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;
extern line_t *ceilingline;
extern mobj_t *linetarget;     // who got hit (or NULL)
extern msecnode_t *sector_list;                             // phares 3/16/98
extern fixed_t tmbbox[4];         // phares 3/20/98

#endif // __P_MAP__

//----------------------------------------------------------------------------
//
// $Log: p_map.h,v $
// Revision 1.2  1998/05/07  00:53:07  killough
// Add more external declarations
//
// Revision 1.1  1998/05/03  22:19:23  killough
// External declarations formerly in p_local.h
//
//
//----------------------------------------------------------------------------
