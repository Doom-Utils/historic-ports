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
//	Switches, buttons. Two-state animation. Exits.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "i_system.h"
#include "doomdef.h"
#include "p_local.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "doomstat.h"
#include "r_state.h"
#include "m_swap.h"
#include "m_comdev.h"
#include "w_wad.h"
#include "z_zone.h"

extern int	has_behavior;

//
// not used anymore, switch list is loaded from PWAD's now
//

#if 0
//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
switchlist_t alphSwitchList[] =
{
    // Doom shareware episode 1 switches
    {"SW1BRCOM",	"SW2BRCOM",	1},
    {"SW1BRN1",		"SW2BRN1",	1},
    {"SW1BRN2",		"SW2BRN2",	1},
    {"SW1BRNGN",	"SW2BRNGN",	1},
    {"SW1BROWN",	"SW2BROWN",	1},
    {"SW1COMM",		"SW2COMM",	1},
    {"SW1COMP",		"SW2COMP",	1},
    {"SW1DIRT",		"SW2DIRT",	1},
    {"SW1EXIT",		"SW2EXIT",	1},
    {"SW1GRAY",		"SW2GRAY",	1},
    {"SW1GRAY1",	"SW2GRAY1",	1},
    {"SW1METAL",	"SW2METAL",	1},
    {"SW1PIPE",		"SW2PIPE",	1},
    {"SW1SLAD",		"SW2SLAD",	1},
    {"SW1STARG",	"SW2STARG",	1},
    {"SW1STON1",	"SW2STON1",	1},
    {"SW1STON2",	"SW2STON2",	1},
    {"SW1STONE",	"SW2STONE",	1},
    {"SW1STRTN",	"SW2STRTN",	1},

    // Doom registered episodes 2&3 switches
    {"SW1BLUE",		"SW2BLUE",	2},
    {"SW1CMT",		"SW2CMT",	2},
    {"SW1GARG",		"SW2GARG",	2},
    {"SW1GSTON",	"SW2GSTON",	2},
    {"SW1HOT",		"SW2HOT",	2},
    {"SW1LION",		"SW2LION",	2},
    {"SW1SATYR",	"SW2SATYR",	2},
    {"SW1SKIN",		"SW2SKIN",	2},
    {"SW1VINE",		"SW2VINE",	2},
    {"SW1WOOD",		"SW2WOOD",	2},

    // Doom II switches
    {"SW1PANEL",	"SW2PANEL",	3},
    {"SW1ROCK",		"SW2ROCK",	3},
    {"SW1MET2",		"SW2MET2",	3},
    {"SW1WDMET",	"SW2WDMET",	3},
    {"SW1BRIK",		"SW2BRIK",	3},
    {"SW1MOD1",		"SW2MOD1",	3},
    {"SW1ZIM",		"SW2ZIM",	3},
    {"SW1STON6",	"SW2STON6",	3},
    {"SW1TEK",		"SW2TEK",	3},
    {"SW1MARB",		"SW2MARB",	3},
    {"SW1SKULL",	"SW2SKULL",	3},

    {"\0",		"\0",		0}
};
#endif

static int	*switchlist;
static int	numswitches;
static int	max_numswitches;

button_t        buttonlist[MAXBUTTONS];

//
// P_InitSwitchList()
//
// Only called at game initialization in order to list the set of switches
// and buttons known to the engine. This enables their texture to change
// when activated, and in the case of buttons, change back after a timeout.
//
// This routine modified to read its data from a predefined lump or
// PWAD lump called SWITCHES rather than a static table in this module to
// allow wad designers to insert or modify switches.
//
// Lump format is an array of byte packed switchlist_t structures, terminated
// by a structure with episode == 0. The lump can be generated from a
// text source file using SWANTBLS, distributed with the xwadtools utils.
// The standard list of switches and animations is contained in the example
// source text file defswani.dat.
//
// This was taken from Boom sources.
//
void P_InitSwitchList(void)
{
    int			i;
    int			index;
    int			episode;
    switchlist_t	*alphSwitchList;	// pointer to switch table
    int			lump = W_GetNumForName("SWITCHES");

    episode = 1;

    if (gamemode == registered || gamemode == retail)
	episode = 2;
    else
	if (gamemode == commercial)
	    episode = 3;

    // read the switch table from a predefined lump
    alphSwitchList = (switchlist_t *)W_CacheLumpNum(lump, PU_CACHE);

    for (index = 0, i = 0; ; i++)
    {
	if (index + 1 >= max_numswitches)
	    switchlist = Z_Realloc(switchlist, sizeof(*switchlist) *
				   (max_numswitches = max_numswitches ?
				   max_numswitches * 2 : 50), PU_STATIC,
				   (void *)0);

	if (SHORT(alphSwitchList[i].episode) <= episode)
	{
	    if (!SHORT(alphSwitchList[i].episode))
		break;
	    switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name1);
	    switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name2);
	}
    }

    numswitches = index / 2;
    switchlist[index] = -1;
}

//
// Start a button counting down till it turns off.
//
void P_StartButton(line_t *line, bwhere_e w, int texture, int time)
{
    int		i;

    // See if button is already pressed
    for (i = 0; i < MAXBUTTONS; i++)
    {
	if (buttonlist[i].btimer && buttonlist[i].line == line)
	{
	    return;
	}
    }

    for (i = 0; i < MAXBUTTONS; i++)
    {
	if (!buttonlist[i].btimer)
	{
	    buttonlist[i].line = line;
	    buttonlist[i].where = w;
	    buttonlist[i].btexture = texture;
	    buttonlist[i].btimer = time;
	    buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
	    return;
	}
    }

    I_Error("P_StartButton: no button slots left!");
}

//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
void P_ChangeSwitchTexture(line_t *line, int useAgain)
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;
    int     sound;

    if (!useAgain)
	line->special = 0;

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    sound = sfx_swtchn;

    // EXIT SWITCH?
    if ((line->special == 11 && !has_behavior) ||
	(line->special == 243 && has_behavior))
	sound = sfx_swtchx;

    for (i = 0; i < numswitches * 2; i++)
    {
	if (switchlist[i] == texTop)
	{
	    S_StartSound(buttonlist->soundorg, sound);
	    sides[line->sidenum[0]].toptexture = switchlist[i ^ 1];

	    if (useAgain)
		P_StartButton(line,top,switchlist[i], BUTTONTIME);

	    return;
	}
	else
	{
	    if (switchlist[i] == texMid)
	    {
		S_StartSound(buttonlist->soundorg, sound);
		sides[line->sidenum[0]].midtexture = switchlist[i ^ 1];

		if (useAgain)
		    P_StartButton(line, middle, switchlist[i], BUTTONTIME);

		return;
	    }
	    else
	    {
		if (switchlist[i] == texBot)
		{
		    S_StartSound(buttonlist->soundorg, sound);
		    sides[line->sidenum[0]].bottomtexture = switchlist[i ^ 1];

		    if (useAgain)
			P_StartButton(line, bottom, switchlist[i], BUTTONTIME);

		    return;
		}
	    }
	}
    }
}
