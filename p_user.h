// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_user.h,v 1.2 1998/05/10 23:38:38 killough Exp $
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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.
//
//-----------------------------------------------------------------------------

#ifndef __P_USER__
#define __P_USER__

#include "d_player.h"

void P_PlayerThink(player_t *player);
void P_CalcHeight(player_t *player);
void P_DeathThink(player_t *player);
void P_MovePlayer(player_t *player);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);

#endif // __P_USER__

//----------------------------------------------------------------------------
//
// $Log: p_user.h,v $
// Revision 1.2  1998/05/10  23:38:38  killough
// Add more prototypes
//
// Revision 1.1  1998/05/03  23:19:24  killough
// Move from obsolete p_local.h
//
//----------------------------------------------------------------------------
