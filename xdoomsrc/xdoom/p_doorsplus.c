// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=4:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 2000 by Udo Munk
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
// DESCRIPTION: Scritable door animation code for XDoomPlus.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "z_zone.h"
#include "p_local.h"

//
// Switch force fields with id off
//
void EV_PlusForceField(int id, line_t *ln, mobj_t *thing, int flag)
{
	register int	line = -1;
	forcefield_t	*field;

	// find all lines with id
	while ((line = P_FindLineFromTag(id, line)) >= 0)
	{
		// force field must be 2s line
		if (!lines[line].flags & ML_TWOSIDED)
			continue;

		// make sure force field isn't open already
		if (sides[lines[line].sidenum[0]].midtexture == 0)
			continue;

		field = Z_Malloc(sizeof(*field), PU_LEVSPEC, (void *)0);
		field->frontsector = lines[line].frontsector;
		field->backsector = lines[line].backsector;
		field->line = &lines[line];
		field->oldflags = lines[line].flags;

		// used a switch?
		if (!flag)
		{
			field->status = ff_open;
			field->timer = FFWAITTICS;
		}
		// no, damaged with gun shot or deactivated with comm gadget or button
		else
		{
			field->status = ff_damaged;
			P_ChangeSwitchTexture(ln, 0);
		}
		P_AddThinker(&field->thinker);
		field->thinker.function.acv = T_ForceField;
	}
}
