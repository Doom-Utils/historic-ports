// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=4:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1999-2000 by Udo Munk
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
// DESCRIPTION: This handles the line specials for XDoomPlus.
//
//-----------------------------------------------------------------------------

//
// Hexen sources from Raven Software have been examined to implement this.
//
// The Hexen line special is a byte only, so we use an array with 256
// function pointers for the specials, instead of one huge case.
// The array is at the end of the file, to avoid all that forward
// declarations neccessary otherwise.
//

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>

#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "r_defs.h"
#include "m_argv.h"
#include "p_spec.h"
#include "p_specplus.h"
#include "p_acs.h"
#include "p_local.h"
#include "g_game.h"

// ------------------------------------------------------------------
// Line Special Function
// ------------------------------------------------------------------

//
// Function that does nothing, used for unimplemented specials
//
static void Fn_None(byte *args, line_t *ln, int side, mobj_t *thing)
{
}

//
// Run script
//
static void Fn_ACSExecute(byte *args, line_t *ln, int side, mobj_t *thing)
{
	P_StartACS(args[0], args[1], &args[2], thing, ln, side);
}

//
// Run script if player has the right keys
//
static void Fn_ACSExecuteLock(byte *args, line_t *ln, int side, mobj_t *thing)
{
	int	anykey;

	// only players have keys
	if (!thing->player)
		return;

	anykey = thing->player->cards[it_redcard] |
			 thing->player->cards[it_bluecard] |
			 thing->player->cards[it_yellowcard] |
			 thing->player->cards[it_redskull] |
			 thing->player->cards[it_blueskull] |
			 thing->player->cards[it_yellowskull];

	if ((args[4] == Key_Card_Red && thing->player->cards[it_redcard]) ||
		(args[4] == Key_Card_Blue && thing->player->cards[it_bluecard]) ||
		(args[4] == Key_Card_Yellow && thing->player->cards[it_yellowcard]) ||
		(args[4] == Key_Skull_Red && thing->player->cards[it_redskull]) ||
		(args[4] == Key_Skull_Blue && thing->player->cards[it_blueskull]) ||
		(args[4] == Key_Skull_Yellow && thing->player->cards[it_yellowskull]) ||
		(args[4] == Key_Any && anykey))
		P_StartACS(args[0], args[1], &args[2], thing, ln, side);
}

//
// Suspend script
//
static void Fn_ACSSuspend(byte *args, line_t *ln, int side, mobj_t *thing)
{
	P_SuspendACS(args[0], args[1]);
}

//
// Terminate script
//
static void Fn_ACSTerminate(byte *args, line_t *ln, int side, mobj_t *thing)
{
	P_TerminateACS(args[0], args[1]);
}

//
// Thrust thing
//
static void Fn_ThrustThing(byte *args, line_t *ln, int side, mobj_t *thing)
{
	if (thing)
	{
		angle_t	angle = BYTEANGLE(args[0]) >> ANGLETOFINESHIFT;

		thing->momx = args[1] * finecosine[angle];
		thing->momy = args[1] * finesine[angle];
	}
}

//
// Damage thing that touchs the line. If the damage value in arg1 is not
// set (0), it is a sure kill.
//
static void Fn_ThingDamage(byte *args, line_t *ln, int side, mobj_t *thing)
{
	if (thing)
	{
		// don't damage a player when god cheat active
		if (thing->player && ((thing->player->cheats & CF_GODMODE) ==
		    CF_GODMODE))
			return;

		if (args[0])
			P_DamageMobj(thing, (mobj_t *)0, (mobj_t *)0, args[0]);
		else
			P_DamageMobj(thing, (mobj_t *)0, (mobj_t *)0, 10000);
	}
}

//
// Remove things with TID in arg1
//
static void Fn_ThingRemove(byte *args, line_t *ln, int side, mobj_t *thing)
{
	mobj_t	*mobj = P_FindMobjByTid((mobj_t *)0, args[0]);

	while (mobj)
	{
		mobj_t	*next = P_FindMobjByTid(mobj, args[0]);

		P_RemoveMobj(mobj);
		mobj = next;
	}
}

//
// Destroy things with TID in arg1
//
static void Fn_ThingDestroy(byte *args, line_t *ln, int side, mobj_t *thing)
{
	mobj_t	*mobj = P_FindMobjByTid((mobj_t *)0, args[0]);

	while (mobj)
	{
		mobj_t	*next = P_FindMobjByTid(mobj, args[0]);

		if (mobj->flags & MF_SHOOTABLE)
			P_DamageMobj(mobj, (mobj_t *)0, (mobj_t *)0, 10000);
		mobj = next;
	}
}

//
// Switch force fields off with line special id in arg1
//
static void Fn_FieldOff(byte *args, line_t *ln, int side, mobj_t *thing)
{
	EV_PlusForceField((int)args[0], ln, thing, 1);
}

//
// Normal level ending with exit switch
//
static void Fn_ExitNormal(byte *args, line_t *ln, int side, mobj_t *thing)
{
	P_ChangeSwitchTexture(ln, 0);
	G_ExitLevel();
}

// ------------------------------------------------------------------
// Support functions like in p_spec.c, different for XDoomPlus
// ------------------------------------------------------------------

//
// Initialize the Scrollers
//
static void P_SpawnScrollers(void)
{
	int		i;
	line_t	*ld = lines;

	for (i = 0; i < numlines; i++, ld++)
    {
		switch (ld->special)
		{
			case 100:	// scroll wall left
				Add_Scroller(sc_side, FRACUNIT * ld->args[0], 0, -1,
				lines[i].sidenum[0], 0);
				break;

			case 101:	// scroll wall right
				Add_Scroller(sc_side, FRACUNIT * -ld->args[0], 0, -1,
				lines[i].sidenum[0], 0);
				break;

			case 102:	// scroll wall up
				Add_Scroller(sc_side, 0, FRACUNIT * ld->args[0], -1,
				lines[i].sidenum[0], 0);
				break;

			case 103:	// scroll wall down
				Add_Scroller(sc_side, 0, FRACUNIT * -ld->args[0], -1,
				lines[i].sidenum[0], 0);
				break;
		}
    }
}

//
// Like P_SpawnSpecials() in p_spec.c
//
void P_SpawnSpecialsPlus(void)
{
	int		i;

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
		int     time;

		time = atoi(myargv[i + 1]) * 60 * 35;
		levelTimer = true;
		levelTimeCount = time;
	}

	P_InitTagLists();
	P_SpawnScrollers();
}

//
// The function pointer array for all line specials
//
SpecFunc LineSpecials[256] =
{
//
// Hexen like specials.
//
	Fn_None,			// 0 no action at all
	Fn_None,			// 1 Poly Start Line, does nothing
	Fn_None,			// 2
	Fn_None,			// 3
	Fn_None,			// 4
	Fn_None,			// 5
	Fn_None,			// 6
	Fn_None,			// 7
	Fn_None,			// 8
	Fn_None,			// 9
	Fn_None,			// 10
	Fn_None,			// 11
	Fn_None,			// 12
	Fn_None,			// 13
	Fn_None,			// 14
	Fn_None,			// 15
	Fn_None,			// 16
	Fn_None,			// 17
	Fn_None,			// 18
	Fn_None,			// 19
	Fn_None,			// 20
	Fn_None,			// 21
	Fn_None,			// 22
	Fn_None,			// 23
	Fn_None,			// 24
	Fn_None,			// 25
	Fn_None,			// 26
	Fn_None,			// 27
	Fn_None,			// 28
	Fn_None,			// 29
	Fn_None,			// 30
	Fn_None,			// 31
	Fn_None,			// 32
	Fn_None,			// 33
	Fn_None,			// 34
	Fn_None,			// 35
	Fn_None,			// 36
	Fn_None,			// 37
	Fn_None,			// 38
	Fn_None,			// 39
	Fn_None,			// 40
	Fn_None,			// 41
	Fn_None,			// 42
	Fn_None,			// 43
	Fn_None,			// 44
	Fn_None,			// 45
	Fn_None,			// 46
	Fn_None,			// 47
	Fn_None,			// 48
	Fn_None,			// 49
	Fn_None,			// 50
	Fn_None,			// 51
	Fn_None,			// 52
	Fn_None,			// 53
	Fn_None,			// 54
	Fn_None,			// 55
	Fn_None,			// 56
	Fn_None,			// 57
	Fn_None,			// 58
	Fn_None,			// 59
	Fn_None,			// 60
	Fn_None,			// 61
	Fn_None,			// 62
	Fn_None,			// 63
	Fn_None,			// 64
	Fn_None,			// 65
	Fn_None,			// 66
	Fn_None,			// 67
	Fn_None,			// 68
	Fn_None,			// 69
	Fn_None,			// 70
	Fn_None,			// 71
	Fn_ThrustThing,		// 72
	Fn_ThingDamage,		// 73
	Fn_None,			// 74
	Fn_None,			// 75
	Fn_None,			// 76
	Fn_None,			// 77
	Fn_None,			// 78
	Fn_None,			// 79
	Fn_ACSExecute,		// 80
	Fn_ACSSuspend,		// 81
	Fn_ACSTerminate,	// 82
	Fn_ACSExecuteLock,	// 83
	Fn_None,			// 84
	Fn_None,			// 85
	Fn_None,			// 86
	Fn_None,			// 87
	Fn_None,			// 88
	Fn_None,			// 89
	Fn_None,			// 90
	Fn_None,			// 91
	Fn_None,			// 92
	Fn_None,			// 93
	Fn_None,			// 94
	Fn_None,			// 95
	Fn_None,			// 96
	Fn_None,			// 97
	Fn_None,			// 98
	Fn_None,			// 99
	Fn_None,			// 100 Scroll Texture Left, level init
	Fn_None,			// 101 Scroll Texture Right, level init
	Fn_None,			// 102 Scroll Texture Up, level init
	Fn_None,			// 103 Scroll Texture Down, level init
	Fn_None,			// 104
	Fn_None,			// 105
	Fn_None,			// 106
	Fn_None,			// 107
	Fn_None,			// 108
	Fn_None,			// 109
	Fn_None,			// 110
	Fn_None,			// 111
	Fn_None,			// 112
	Fn_None,			// 113
	Fn_None,			// 114
	Fn_None,			// 115
	Fn_None,			// 116
	Fn_None,			// 117
	Fn_None,			// 118
	Fn_None,			// 119
	Fn_None,			// 120
	Fn_None,			// 121 Set Line Ident., level init
	Fn_None,			// 122
	Fn_None,			// 123
	Fn_None,			// 124
	Fn_None,			// 125
	Fn_None,			// 126
	Fn_None,			// 127
	Fn_None,			// 128
	Fn_None,			// 129
	Fn_None,			// 130
	Fn_None,			// 131
	Fn_ThingRemove,		// 132
	Fn_ThingDestroy,	// 133
	Fn_None,			// 134
	Fn_None,			// 135
	Fn_None,			// 136
	Fn_None,			// 137
	Fn_None,			// 138
	Fn_None,			// 139
	Fn_None,			// 140
//
// End of Hexen like specials.
//
	Fn_None,			// 141
	Fn_None,			// 142
	Fn_None,			// 143
	Fn_None,			// 144
//
// Start of XDoomPlus specials.
//
	Fn_FieldOff,		// 145
	Fn_None,			// 146
	Fn_None,			// 147
	Fn_None,			// 148
	Fn_None,			// 149
	Fn_None,			// 150
	Fn_None,			// 151
	Fn_None,			// 152
	Fn_None,			// 153
	Fn_None,			// 154
	Fn_None,			// 155
	Fn_None,			// 156
	Fn_None,			// 157
	Fn_None,			// 158
	Fn_None,			// 159
	Fn_None,			// 160
	Fn_None,			// 161
	Fn_None,			// 162
	Fn_None,			// 163
	Fn_None,			// 164
	Fn_None,			// 165
	Fn_None,			// 166
	Fn_None,			// 167
	Fn_None,			// 168
	Fn_None,			// 169
	Fn_None,			// 170
	Fn_None,			// 171
	Fn_None,			// 172
	Fn_None,			// 173
	Fn_None,			// 174
	Fn_None,			// 175
//
// End of XDoomPlus specials.
//
	Fn_None,			// 176
	Fn_None,			// 177
	Fn_None,			// 178
	Fn_None,			// 179
	Fn_None,			// 180
	Fn_None,			// 181
	Fn_None,			// 182
	Fn_None,			// 183
	Fn_None,			// 184
	Fn_None,			// 185
	Fn_None,			// 186
	Fn_None,			// 187
	Fn_None,			// 188
	Fn_None,			// 189
//
// Start of ZDoom specials from here until end.
// Don't implement any of those without making
// sure it's ZDoom compatible!
//
	Fn_None,			// 190
	Fn_None,			// 191
	Fn_None,			// 192
	Fn_None,			// 193
	Fn_None,			// 194
	Fn_None,			// 195
	Fn_None,			// 196
	Fn_None,			// 197
	Fn_None,			// 198
	Fn_None,			// 199
	Fn_None,			// 200
	Fn_None,			// 201
	Fn_None,			// 202
	Fn_None,			// 203
	Fn_None,			// 204
	Fn_None,			// 205
	Fn_None,			// 206
	Fn_None,			// 207
	Fn_None,			// 208 Translucent lines, level init
	Fn_None,			// 209
	Fn_None,			// 210
	Fn_None,			// 211
	Fn_None,			// 212
	Fn_None,			// 213
	Fn_None,			// 214
	Fn_None,			// 215
	Fn_None,			// 216
	Fn_None,			// 217
	Fn_None,			// 218
	Fn_None,			// 219
	Fn_None,			// 220
	Fn_None,			// 221
	Fn_None,			// 222
	Fn_None,			// 223
	Fn_None,			// 224
	Fn_None,			// 225
	Fn_None,			// 226
	Fn_None,			// 227
	Fn_None,			// 228
	Fn_None,			// 229
	Fn_None,			// 230
	Fn_None,			// 231
	Fn_None,			// 232
	Fn_None,			// 233
	Fn_None,			// 234
	Fn_None,			// 235
	Fn_None,			// 236
	Fn_None,			// 237
	Fn_None,			// 238
	Fn_None,			// 239
	Fn_None,			// 240
	Fn_None,			// 241
	Fn_None,			// 242
	Fn_ExitNormal,		// 243
	Fn_None,			// 244
	Fn_None,			// 245
	Fn_None,			// 246
	Fn_None,			// 247
	Fn_None,			// 248
	Fn_None,			// 249
	Fn_None,			// 250
	Fn_None,			// 251
	Fn_None,			// 252
	Fn_None,			// 253
	Fn_None,			// 254
	Fn_None				// 255
};
