// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
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
// DESCRIPTION: ACS Script support for XDoomPlus
//
//-----------------------------------------------------------------------------

//
// Hexen sources from Raven Software have been examined to implement this.
// The P code interpreter implemented here is binary compatible to the one
// Raven wrote, but it is implemented different and from scratch.
//
// Major implementation differences are:
//	- Works on big endian systems too. Don't know why Raven didn't
//	  implement this, their acc compiler is endian aware.
//	- Strict argument checking, don't trust any bit on valid P code.
//	  Even if the compiler is bug free there still are some people
//	  alive, who can program object code without assemblers and
//	  compilers.
//	- Also trap illegal P codes and don't try to run a function at
//	  address 0 for illegal P codes, which will result in a crash.
//	- Test for stack overflow/underflow.
//	- Uses script accounting to detect runaway scripts and terminates them.
//	- Doesn't use state variables, the state of a script is dependend
//	  on the queue it is linked to. If it is not linked the script is
//	  inactive, means not started or has terminated. Also saves the
//	  time to walk the whole array with scripts to find the ones running,
//	  delayed, etc.
//	- Use one script control block only, to hold all informations necessary
//	  to manage the scripts.
//	- There is no such thing like the ACS store in Raven's interpreter
//	  which could run out of space, for holding informations about scripts
//	  started in other maps than the current one. Those scripts also are
//	  linked into a queue, which will be processed when another level is
//	  entered (not implemented yet).
//	- Doesn't use Push and Pop functions for the stack, the values on stack
//	  are accessed directely with a macro, is faster, use the saved time
//	  for consistency checking.
//

static const char
rcsid[] = "$Id:$";

#include <stdio.h>

#include "doomstat.h"
#include "r_defs.h"
#include "r_data.h"
#include "d_player.h"
#include "m_swap.h"
#include "m_random.h"
#include "m_argv.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_spec.h"
#include "p_specplus.h"
#include "p_mobj.h"
#include "p_acs.h"
#include "w_wad.h"
#include "z_zone.h"

// Instead of using Push()/Pop() functions the stack is accessed with
// the following macro, which is faster because it saves all the function
// calls. Of course the stack pointer must be handled manually then, but
// we (I?) can do that.
// The macro always works on the first script control block in the
// run queue, that is the current running script.
// Argument x is the offset from stack top, SVAL(1) is the top one,
// SVAL(0) is the next one above top, where a new value is pushed.
#define SVAL(x) (run_queue->stack[run_queue->sp - (x)])

static void	link_script(acs_t **, acs_t *);
static void	unlink_script(acs_t **, acs_t *);
static void	CmdNop(void);
static void	CmdNimp(void);
static void	CmdTerminate(void);
static void	CmdSuspend(void);
static void	CmdPushNumber(void);
static void	CmdLSpec1(void);
static void	CmdLSpec2(void);
static void	CmdLSpec3(void);
static void	CmdLSpec4(void);
static void	CmdLSpec5(void);
static void	CmdLSpec1Direct(void);
static void	CmdLSpec2Direct(void);
static void	CmdLSpec3Direct(void);
static void	CmdLSpec4Direct(void);
static void	CmdLSpec5Direct(void);
static void	CmdAdd(void);
static void	CmdSubtract(void);
static void	CmdMultiply(void);
static void	CmdDivide(void);
static void	CmdModulus(void);
static void	CmdEq(void);
static void	CmdNe(void);
static void	CmdLt(void);
static void	CmdGt(void);
static void	CmdLe(void);
static void	CmdGe(void);
static void	CmdAssignScriptVar(void);
static void	CmdAssignMapVar(void);
static void	CmdAssignWorldVar(void);
static void	CmdPushScriptVar(void);
static void	CmdPushMapVar(void);
static void	CmdPushWorldVar(void);
static void	CmdAddScriptVar(void);
static void	CmdAddMapVar(void);
static void	CmdAddWorldVar(void);
static void	CmdSubScriptVar(void);
static void	CmdSubMapVar(void);
static void	CmdSubWorldVar(void);
static void	CmdMulScriptVar(void);
static void	CmdMulMapVar(void);
static void	CmdMulWorldVar(void);
static void	CmdDivScriptVar(void);
static void	CmdDivMapVar(void);
static void	CmdDivWorldVar(void);
static void	CmdModScriptVar(void);
static void	CmdModMapVar(void);
static void	CmdModWorldVar(void);
static void	CmdIncScriptVar(void);
static void	CmdIncMapVar(void);
static void	CmdIncWorldVar(void);
static void	CmdDecScriptVar(void);
static void	CmdDecMapVar(void);
static void	CmdDecWorldVar(void);
static void	CmdGoto(void);
static void	CmdIfGoto(void);
static void	CmdDrop(void);
static void	CmdDelay(void);
static void	CmdDelayDirect(void);
static void	CmdRandom(void);
static void	CmdRandomDirect(void);
static void	CmdTagWait(void);
static void	CmdTagWaitDirect(void);
static void	CmdPolyWait(void);
static void	CmdPolyWaitDirect(void);
static void	CmdChangeFloor(void);
static void	CmdChangeFloorDirect(void);
static void	CmdChangeCeil(void);
static void	CmdChangeCeilDirect(void);
static void	CmdRestart(void);
static void	CmdAndLogical(void);
static void	CmdOrLogical(void);
static void	CmdAndBitwise(void);
static void	CmdOrBitwise(void);
static void	CmdEorBitwise(void);
static void	CmdNegateLogical(void);
static void	CmdLShift(void);
static void	CmdRShift(void);
static void	CmdUnaryMinus(void);
static void	CmdIfNotGoto(void);
static void	CmdLineSide(void);
static void	CmdScriptWait(void);
static void	CmdScriptWaitDirect(void);
static void	CmdClearLineSpecial(void);
static void	CmdCaseGoto(void);
static void	CmdBeginPrint(void);
static void	CmdEndPrint(void);
static void	CmdPrintString(void);
static void	CmdPrintNumber(void);
static void	CmdPrintCharacter(void);
static void	CmdPlayerCount(void);
static void	CmdGameType(void);
static void	CmdGameSkill(void);
static void	CmdTimer(void);
static void	CmdSectorSound(void);
static void	CmdAmbientSound(void);
static void	CmdSetLineTexture(void);
static void	CmdSetLineBlocking(void);
static void	CmdSetLineSpecial(void);
static void	CmdThingSound(void);

static acs_t	scripts[OPEN_SCRIPTS_BASE];	// script control blocks
static int	script_count;			// no. of scripts for the map
static int	map_vars[MAX_ACS_MAP_VARS];	// map variables
static int	world_vars[MAX_ACS_WORLD_VARS];	// world variables
static char	**acs_strings;			// strings
static int	string_count;			// no. of strings for the map
static byte	*code_base;			// code base of P code
static char	print_buffer[256];		// buffer for print P codes
static int	debug_flag;			// P code debugger flag

// The queues for the script scheduler. A script control block not
// linked into any queue indicates an inactive script, means not
// started so far or terminated.
static acs_t	*run_queue;	// runnable scripts linked here
static acs_t	*suspend_queue;	// suspended scripts linked here
static acs_t	*delayed_queue;	// delayed scripts linked here
static acs_t	*wait_queue;	// scripts waiting for some event linked here

// Array with function pointers for each P code. P codes not implemented
// yet will run into CmdNimp, which terminates the script.
// The P codes are Hexen compatible.
static void (*PCodeCmds[256])(void) =
{
	CmdNop,			//			0
	CmdTerminate,		// 			1
	CmdSuspend,		//			2
	CmdPushNumber,		//			3
	CmdLSpec1,		//			4
	CmdLSpec2,		//			5
	CmdLSpec3,		//			6
	CmdLSpec4,		//			7
	CmdLSpec5,		//			8
	CmdLSpec1Direct,	//			9
	CmdLSpec2Direct,	//			10
	CmdLSpec3Direct,	//			11
	CmdLSpec4Direct,	//			12
	CmdLSpec5Direct,	//			13
	CmdAdd,			//			14
	CmdSubtract,		//			15
	CmdMultiply,		//			16
	CmdDivide,		//			17
	CmdModulus,		//			18
	CmdEq,			//			19
	CmdNe,			//			20
	CmdLt,			//			21
	CmdGt,			//			22
	CmdLe,			//			23
	CmdGe,			//			24
	CmdAssignScriptVar,	//			25
	CmdAssignMapVar,	//			26
	CmdAssignWorldVar,	//			27
	CmdPushScriptVar,	//			28
	CmdPushMapVar,		//			29
	CmdPushWorldVar,	//			30
	CmdAddScriptVar,	//			31
	CmdAddMapVar,		//			32
	CmdAddWorldVar,		//			33
	CmdSubScriptVar,	//			34
	CmdSubMapVar,		//			35
	CmdSubWorldVar,		//			36
	CmdMulScriptVar,	//			37
	CmdMulMapVar,		//			38
	CmdMulWorldVar,		//			39
	CmdDivScriptVar,	//			40
	CmdDivMapVar,		//			41
	CmdDivWorldVar,		//			42
	CmdModScriptVar,	//			43
	CmdModMapVar,		//			44
	CmdModWorldVar,		//			45
	CmdIncScriptVar,	//			46
	CmdIncMapVar,		//			47
	CmdIncWorldVar,		//			48
	CmdDecScriptVar,	//			49
	CmdDecMapVar,		//			50
	CmdDecWorldVar,		//			51
	CmdGoto,		//			52
	CmdIfGoto,		//			53
	CmdDrop,		//			54
	CmdDelay,		//			55
	CmdDelayDirect,		//			56
	CmdRandom,		//			57
	CmdRandomDirect,	//			58
	CmdNimp,		// CmdThingCount	59
	CmdNimp,		// CmdThingCountDirect	60
	CmdTagWait,		//			61
	CmdTagWaitDirect,	//			62
	CmdPolyWait,		//			63
	CmdPolyWaitDirect,	//			64
	CmdChangeFloor,		//			65
	CmdChangeFloorDirect,	//			66
	CmdChangeCeil,		//			67
	CmdChangeCeilDirect,	//			68
	CmdRestart,		//			69
	CmdAndLogical,		//			70
	CmdOrLogical,		//			71
	CmdAndBitwise,		//			72
	CmdOrBitwise,		//			73
	CmdEorBitwise,		//			74
	CmdNegateLogical,	//			75
	CmdLShift,		//			76
	CmdRShift,		//			77
	CmdUnaryMinus,		//			78
	CmdIfNotGoto,		//			79
	CmdLineSide,		//			80
	CmdScriptWait,		//			81
	CmdScriptWaitDirect,	//			82
	CmdClearLineSpecial,	//			83
	CmdCaseGoto,		//			84
	CmdBeginPrint,		// 			85
	CmdEndPrint,		// 			86
	CmdPrintString,		//			87
	CmdPrintNumber,		//			88
	CmdPrintCharacter,	//			89
	CmdPlayerCount,		//			90
	CmdGameType,		//			91
	CmdGameSkill,		//			92
	CmdTimer,		//			93
	CmdSectorSound,		// 			94
	CmdAmbientSound,	//			95
	CmdNimp,		// CmdSoundSequence	96
	CmdSetLineTexture,	//			97
	CmdSetLineBlocking,	//			98
	CmdSetLineSpecial,	//			99
	CmdThingSound,		//			100
	CmdEndPrint,		// CmdEndPrintBold	101

// End of Hexen compatible P codes.
// Fill array up with traps, so that the P code interpreter
// won't crash the program with illegal P codes.

	CmdNimp,		// 102
	CmdNimp,		// 103
	CmdNimp,		// 104
	CmdNimp,		// 105
	CmdNimp,		// 106
	CmdNimp,		// 107
	CmdNimp,		// 108
	CmdNimp,		// 109
	CmdNimp,		// 110
	CmdNimp,		// 111
	CmdNimp,		// 112
	CmdNimp,		// 113
	CmdNimp,		// 114
	CmdNimp,		// 115
	CmdNimp,		// 116
	CmdNimp,		// 117
	CmdNimp,		// 118
	CmdNimp,		// 119
	CmdNimp,		// 120
	CmdNimp,		// 121
	CmdNimp,		// 122
	CmdNimp,		// 123
	CmdNimp,		// 124
	CmdNimp,		// 125
	CmdNimp,		// 126
	CmdNimp,		// 127
	CmdNimp,		// 128
	CmdNimp,		// 129
	CmdNimp,		// 130
	CmdNimp,		// 131
	CmdNimp,		// 132
	CmdNimp,		// 133
	CmdNimp,		// 134
	CmdNimp,		// 135
	CmdNimp,		// 136
	CmdNimp,		// 137
	CmdNimp,		// 138
	CmdNimp,		// 139
	CmdNimp,		// 140
	CmdNimp,		// 141
	CmdNimp,		// 142
	CmdNimp,		// 143
	CmdNimp,		// 144
	CmdNimp,		// 145
	CmdNimp,		// 146
	CmdNimp,		// 147
	CmdNimp,		// 148
	CmdNimp,		// 149
	CmdNimp,		// 150
	CmdNimp,		// 151
	CmdNimp,		// 152
	CmdNimp,		// 153
	CmdNimp,		// 154
	CmdNimp,		// 155
	CmdNimp,		// 156
	CmdNimp,		// 157
	CmdNimp,		// 158
	CmdNimp,		// 159
	CmdNimp,		// 160
	CmdNimp,		// 161
	CmdNimp,		// 162
	CmdNimp,		// 163
	CmdNimp,		// 164
	CmdNimp,		// 165
	CmdNimp,		// 166
	CmdNimp,		// 167
	CmdNimp,		// 168
	CmdNimp,		// 169
	CmdNimp,		// 170
	CmdNimp,		// 171
	CmdNimp,		// 172
	CmdNimp,		// 173
	CmdNimp,		// 174
	CmdNimp,		// 175
	CmdNimp,		// 176
	CmdNimp,		// 177
	CmdNimp,		// 178
	CmdNimp,		// 179
	CmdNimp,		// 180
	CmdNimp,		// 181
	CmdNimp,		// 182
	CmdNimp,		// 183
	CmdNimp,		// 184
	CmdNimp,		// 185
	CmdNimp,		// 186
	CmdNimp,		// 187
	CmdNimp,		// 188
	CmdNimp,		// 189
	CmdNimp,		// 190
	CmdNimp,		// 191
	CmdNimp,		// 192
	CmdNimp,		// 193
	CmdNimp,		// 194
	CmdNimp,		// 195
	CmdNimp,		// 196
	CmdNimp,		// 197
	CmdNimp,		// 198
	CmdNimp,		// 199
	CmdNimp,		// 200
	CmdNimp,		// 201
	CmdNimp,		// 202
	CmdNimp,		// 203
	CmdNimp,		// 204
	CmdNimp,		// 205
	CmdNimp,		// 206
	CmdNimp,		// 207
	CmdNimp,		// 208
	CmdNimp,		// 209
	CmdNimp,		// 210
	CmdNimp,		// 211
	CmdNimp,		// 212
	CmdNimp,		// 213
	CmdNimp,		// 214
	CmdNimp,		// 215
	CmdNimp,		// 216
	CmdNimp,		// 217
	CmdNimp,		// 218
	CmdNimp,		// 219
	CmdNimp,		// 220
	CmdNimp,		// 221
	CmdNimp,		// 222
	CmdNimp,		// 223
	CmdNimp,		// 224
	CmdNimp,		// 225
	CmdNimp,		// 226
	CmdNimp,		// 227
	CmdNimp,		// 228
	CmdNimp,		// 229
	CmdNimp,		// 230
	CmdNimp,		// 231
	CmdNimp,		// 232
	CmdNimp,		// 233
	CmdNimp,		// 234
	CmdNimp,		// 235
	CmdNimp,		// 236
	CmdNimp,		// 237
	CmdNimp,		// 238
	CmdNimp,		// 239
	CmdNimp,		// 240
	CmdNimp,		// 241
	CmdNimp,		// 242
	CmdNimp,		// 243
	CmdNimp,		// 244
	CmdNimp,		// 245
	CmdNimp,		// 246
	CmdNimp,		// 247
	CmdNimp,		// 248
	CmdNimp,		// 249
	CmdNimp,		// 250
	CmdNimp,		// 251
	CmdNimp,		// 252
	CmdNimp,		// 253
	CmdNimp,		// 254
	CmdNimp			// 255
};

//
// Initialize when new game started
//
void P_ACSInitNewGame(void)
{
	// initialize world variables
	memset((void *)&world_vars[0], 0, sizeof(world_vars));

	// check option for debugging
	if (M_CheckParm("-pdebug"))
		debug_flag = 1;
	else
		debug_flag = 0;
}

//
// Load the ACS scripts for the map from BEHAVIOR lump and start
// all scripts declared OPEN
//
void P_LoadACScripts(int lump)
{
	int		i;
	int		*buffer;
	acs_header_t	*header;

	// initialize all script and map related memory
	memset((void *)&scripts[0], 0, sizeof(scripts));
	memset((void *)&map_vars[0], 0, sizeof(map_vars));

	// clear the queues
	run_queue = (acs_t *)0;
	delayed_queue = (acs_t *)0;
	suspend_queue = (acs_t *)0;
	wait_queue = (acs_t *)0;

	// read BEHAVIOR lump
	header = W_CacheLumpNum(lump, PU_LEVEL);
	code_base = (byte *)header;

	// check for valid marker, if invalid dump the scripts
	if (strncmp((char *)header, "ACS", 4))
	{
		Z_Free(header);
		return;
	}

	// get and check number of scripts
	buffer = (int *)((byte *)header + LONG(header->info));
	script_count = LONG(*buffer);
	buffer++;
	if (script_count == 0)
	{
		Z_Free(header);
		return;
	}

	// initialize the scripts, start OPEN declared ones
	for (i = 0; i < script_count; i++)
	{
		int	num;

		num = LONG(*buffer);		// script number
		buffer++;

		if (num >= OPEN_SCRIPTS_BASE * 2)
		{
			printf("Ignoring script %d, invalid script number\n",
				num);
			buffer += 2;
			continue;
		}

		if (num >= OPEN_SCRIPTS_BASE)
		{
			num -= OPEN_SCRIPTS_BASE;
			scripts[num].code = scripts[num].ip =
				(int *)((byte *)code_base + LONG(*buffer));
			buffer++;
			scripts[num].snum = num;
			link_script(&run_queue, &scripts[num]);
		}
		else
		{
			scripts[num].code = (int *)((byte *)code_base +
					     LONG(*buffer));
			buffer++;
			scripts[num].snum = num;
		}
		scripts[num].argc = LONG(*buffer);
		buffer++;
	}

	// initialize the strings
	string_count = LONG(*buffer);
	buffer++;
	acs_strings = (char **)buffer;
	for (i = 0; i < string_count; i++)
	{
		acs_strings[i] = (char *)(LONG(acs_strings[i]) +
				 (long)code_base);
	}

	printf("Loaded %d scripts and %d strings\n", script_count,
		string_count);
}

//
// The script scheduler, called every game tick
//
void P_RunScripts(void)
{
	acs_t	*script;
	acs_t	*next;

	// decrement timer for delayed scripts, if 0 make runnable
	if (delayed_queue)
	{
		for (script = delayed_queue; script;)
		{
			script->timer--;
			if (!script->timer)
			{
				next = script->next;
				unlink_script(&delayed_queue, script);
				link_script(&run_queue, script);
				script = next;
			}
			else
			{
				script = script->next;
			}
		}
	}

	// run all active scripts
	while (run_queue)
	{
		// debugging
		if (debug_flag)
		{
			printf("Script %d sp = %d, ip = %d, P code = %d\n",
			    run_queue->snum,
			    run_queue->sp,
			    (int) (run_queue->ip - run_queue->code),
			    *run_queue->ip);
		}

		// increase instruction count for the script, catch runaways
		run_queue->inst_count++;
		if (run_queue->inst_count > RUNAWAY)
		{
			printf("Script %d: runaway script terminated\n",
				run_queue->snum);
			unlink_script(&run_queue, run_queue);
			continue;
		}

		// check for stack overflow/underflow
		if (run_queue->sp >= STACK_SIZE || run_queue->sp < 0)
		{
			printf("Script %d: stack overflow\n", run_queue->snum);
			unlink_script(&run_queue, run_queue);
			continue;
		}

		// run next instruction for the active script
		PCodeCmds[*run_queue->ip++]();
	}
}

//
// Checks the wait queue for scripts waiting for this event and links
// those found into the run queue.
//
void P_CheckWaitingScripts(ev_wait event, int wait)
{
	acs_t	*script;
	acs_t	*next;

	// walk the wait queue
	if (wait_queue)
	{
		for (script = wait_queue; script;)
		{
			if ((script->wait_event == event) &&
			    (script->wait_value == wait))
			{
				next = script->next;
				unlink_script(&wait_queue, script);
				link_script(&run_queue, script);
				script = next;
			}
			else
			{
				script = script->next;
			}
		}
	}
}

//
// Start a script, this is called from the ACS_Execute line special
//
void P_StartACS(int snum, int map, byte *args, mobj_t *activator,
		line_t *line, int side)
{
	int	i;

	// run a script in this map?
	if (map && map != gamemap)
	{
		// no, other map, not implemented yet, ignore it
		printf("P_StartACS: cannot run script in another map yet\n");
		return;
	}

	// do we have that script?
	if (scripts[snum].snum != snum)
	{
		printf("P_StartACS: unknown script number %d\n", snum);
		return;
	}

	// is script linked into any queue?
	if (scripts[snum].queue)
	{
		// linked into suspend queue? if yes resume it
		if (scripts[snum].queue == &suspend_queue)
		{
			scripts[snum].inst_count = 0;
			unlink_script(&suspend_queue, &scripts[snum]);
			link_script(&run_queue, &scripts[snum]);
			return;
		}
		else
		{
			// script is in another queue, is not an error, ignore
			return;
		}
	}

	// script is not in any queue, so make it runnable from start address
	scripts[snum].activator = activator;
	scripts[snum].line = line;
	scripts[snum].side = side;
	scripts[snum].ip = scripts[snum].code;
	scripts[snum].inst_count = 0;

	for (i = 0; i < scripts[snum].argc; i++)
		scripts[snum].vars[i] = args[i];

	link_script(&run_queue, &scripts[snum]);
}

//
// Suspend a script, ACS_Suspend line special
//
void P_SuspendACS(int snum, int map)
{
	// suspend a script in this map?
	if (map && map != gamemap)
	{
		// no, other map, not implemented yet, ignore it
		printf("P_SuspendACS: cannot suspend script in another "
			"map yet\n");
		return;
	}

	// do we have that script?
	if (scripts[snum].snum != snum)
	{
		printf("P_SuspendACS: unknown script number %d\n", snum);
		return;
	}

	// if the script is running suspend it
	if (scripts[snum].queue && (scripts[snum].queue == &run_queue))
	{
		unlink_script(&run_queue, &scripts[snum]);
		link_script(&suspend_queue, &scripts[snum]);
		scripts[snum].inst_count = 0;
	}

	// also if it is delayed suspend it
	else if (scripts[snum].queue && (scripts[snum].queue == &delayed_queue))
	{
		unlink_script(&delayed_queue, &scripts[snum]);
		link_script(&suspend_queue, &scripts[snum]);
		scripts[snum].inst_count = 0;
	}
}

//
// Terminate a script, ACS_Terminate line special
//
void P_TerminateACS(int snum, int map)
{
	// suspend a script in this map?
	if (map && map != gamemap)
	{
		// no, other map, not implemented yet, ignore it
		printf("P_TerminateACS: cannot terminate script in another "
			"map yet\n");
		return;
	}

	// do we have that script?
	if (scripts[snum].snum != snum)
	{
		printf("P_TerminateACS: unknown script number %d\n", snum);
		return;
	}

	// if the script is running terminate it
	if (scripts[snum].queue && (scripts[snum].queue == &run_queue))
	{
		unlink_script(&run_queue, &scripts[snum]);
		scripts[snum].inst_count = 0;
	}
	// if the script is delayed terminate it
	else if (scripts[snum].queue && (scripts[snum].queue == &delayed_queue))
	{
		unlink_script(&delayed_queue, &scripts[snum]);
		scripts[snum].inst_count = 0;
	}
	// if the script is suspended terminate it
	else if (scripts[snum].queue && (scripts[snum].queue == &suspend_queue))
	{
		unlink_script(&suspend_queue, &scripts[snum]);
		scripts[snum].inst_count = 0;
	}
	// if the script is waiting terminate it
	else if (scripts[snum].queue && (scripts[snum].queue == &wait_queue))
	{
		unlink_script(&wait_queue, &scripts[snum]);
		scripts[snum].inst_count = 0;
	}
}

//
// Link and unlink scripts into/from the queues. Because scripts have
// no priority we always link them at the queue head. This is as fast
// as it gets, no need to walk the queues or keep a pointer to the
// end of a queue.
//
static void link_script(acs_t **queue, acs_t *script)
{
	// a script to link may not be linked into any queue already
	if (script->prev != (acs_t*)0 || script->next != (acs_t*)0 ||
	    script->queue != (acs_t**)0) {
		printf("Script %d: link_script: script already linked\n",
			script->snum);
		return;
	}

	script->queue = queue;
	script->next = *queue;
	if (*queue)
		(*queue)->prev = script;
	*queue = script;
}

static void unlink_script(acs_t **queue, acs_t *script)
{
	if (script->prev == (acs_t*)0)
		*queue = script->next;
	else
		script->prev->next = script->next;

	if (script->next)
		script->next->prev = script->prev;

	script->prev = script->next = (acs_t*)0;
	script->queue = (acs_t**)0;
}

// ----------------------------------------------------------------------
// Implemented P codes
// ----------------------------------------------------------------------

//
// Well, NOP
//
static void CmdNop(void)
{
}

//
// Not implemented P code trap
//
static void CmdNimp(void)
{
	printf("Script %d: not implemented P code %d\n", run_queue->snum,
		*(run_queue->ip - 1));
	unlink_script(&run_queue, run_queue);
}

//
// Terminate, ran into final closing bracket
//
static void CmdTerminate(void)
{
	acs_t	*script = run_queue;

	// unlink first script from the run queue
	unlink_script(&run_queue, script);

	// check if any scripts are waiting for this one to terminate
	P_CheckWaitingScripts(wait_script, script->snum);
}

//
// Suspend script
//
static void CmdSuspend(void)
{
	acs_t	*script;

	// unlink from run queue and link into suspend queue
	script = run_queue;
	script->inst_count = 0;
	unlink_script(&run_queue, run_queue);
	link_script(&suspend_queue, script);
}

//
// Push a number onto the stack
//
static void CmdPushNumber(void)
{
	SVAL(0) = *run_queue->ip++;
	run_queue->sp++;
}

//
// Call line special with 1 argument from stack
//
static void CmdLSpec1(void)
{
	byte	args[5];

	args[0] = SVAL(1);
	args[1] = 0;
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	run_queue->sp--;

	LineSpecials[*run_queue->ip++] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 2 arguments from stack
//
static void CmdLSpec2(void)
{
	byte	args[5];

	args[0] = SVAL(2);
	args[1] = SVAL(1);
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	run_queue->sp -= 2;

	LineSpecials[*run_queue->ip++] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 3 arguments from stack
//
static void CmdLSpec3(void)
{
	byte	args[5];

	args[0] = SVAL(3);
	args[1] = SVAL(2);
	args[2] = SVAL(1);
	args[3] = 0;
	args[4] = 0;
	run_queue->sp -= 3;

	LineSpecials[*run_queue->ip++] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 4 arguments from stack
//
static void CmdLSpec4(void)
{
	byte	args[5];

	args[0] = SVAL(4);
	args[1] = SVAL(3);
	args[2] = SVAL(2);
	args[3] = SVAL(1);
	args[4] = 0;
	run_queue->sp -= 4;

	LineSpecials[*run_queue->ip++] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 5 arguments from stack
//
static void CmdLSpec5(void)
{
	byte	args[5];

	args[0] = SVAL(5);
	args[1] = SVAL(4);
	args[2] = SVAL(3);
	args[3] = SVAL(2);
	args[4] = SVAL(1);
	run_queue->sp -= 5;

	LineSpecials[*run_queue->ip++] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 1 argument from P code
//
static void CmdLSpec1Direct(void)
{
	byte	args[5];
	int	spec;

	args[0] = run_queue->ip[1];
	args[1] = 0;
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	spec = run_queue->ip[0];
	run_queue->ip += 2;

	LineSpecials[spec] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 2 arguments from P code
//
static void CmdLSpec2Direct(void)
{
	byte	args[5];
	int	spec;

	args[0] = run_queue->ip[1];
	args[1] = run_queue->ip[2];
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	spec = run_queue->ip[0];
	run_queue->ip += 3;

	LineSpecials[spec] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 3 arguments from P code
//
static void CmdLSpec3Direct(void)
{
	byte	args[5];
	int	spec;

	args[0] = run_queue->ip[1];
	args[1] = run_queue->ip[2];
	args[2] = run_queue->ip[3];
	args[3] = 0;
	args[4] = 0;
	spec = run_queue->ip[0];
	run_queue->ip += 4;

	LineSpecials[spec] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 4 arguments from P code
//
static void CmdLSpec4Direct(void)
{
	byte	args[5];
	int	spec;

	args[0] = run_queue->ip[1];
	args[1] = run_queue->ip[2];
	args[2] = run_queue->ip[3];
	args[3] = run_queue->ip[4];
	args[4] = 0;
	spec = run_queue->ip[0];
	run_queue->ip += 5;

	LineSpecials[spec] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Call line special with 5 arguments from P code
//
static void CmdLSpec5Direct(void)
{
	byte	args[5];
	int	spec;

	args[0] = run_queue->ip[1];
	args[1] = run_queue->ip[2];
	args[2] = run_queue->ip[3];
	args[3] = run_queue->ip[4];
	args[4] = run_queue->ip[5];
	spec = run_queue->ip[0];
	run_queue->ip += 6;

	LineSpecials[spec] (&args[0], run_queue->line,
		run_queue->side,  run_queue->activator);
}

//
// Add the two topmost values, pop them and push result
//
static void CmdAdd(void)
{
	SVAL(2) = SVAL(2) + SVAL(1);
	run_queue->sp--;
}

//
// Subtract the two topmost values, pop them and push result
//
static void CmdSubtract(void)
{
	SVAL(2) = SVAL(2) - SVAL(1);
	run_queue->sp--;
}

//
// Multiply the two topmost values, pop them and push result
//
static void CmdMultiply(void)
{
	SVAL(2) = SVAL(2) * SVAL(1);
	run_queue->sp--;
}

//
// Divide the two topmost values, pop them and push result
//
static void CmdDivide(void)
{
	SVAL(2) = SVAL(2) / SVAL(1);
	run_queue->sp--;
}

//
// Modulo divide the two topmost values, pop them and push result
//
static void CmdModulus(void)
{
	SVAL(2) = SVAL(2) % SVAL(1);
	run_queue->sp--;
}

//
// Compare the two topmost values for equal, pop them and push result
//
static void CmdEq(void)
{
	SVAL(2) = (SVAL(2) == SVAL(1));
	run_queue->sp--;
}

//
// Compare the two topmost values for not equal, pop them and push result
//
static void CmdNe(void)
{
	SVAL(2) = (SVAL(2) != SVAL(1));
	run_queue->sp--;
}

//
// Compare the two topmost values for lower than, pop them and push result
//
static void CmdLt(void)
{
	SVAL(2) = (SVAL(2) < SVAL(1));
	run_queue->sp--;
}

//
// Compare the two topmost values for greater than, pop them and push result
//
static void CmdGt(void)
{
	SVAL(2) = (SVAL(2) > SVAL(1));
	run_queue->sp--;
}

//
// Compare the two topmost values for lower equal, pop them and push result
//
static void CmdLe(void)
{
	SVAL(2) = (SVAL(2) <= SVAL(1));
	run_queue->sp--;
}

//
// Compare the two topmost values for greater equal, pop them and push result
//
static void CmdGe(void)
{
	SVAL(2) = (SVAL(2) >= SVAL(1));
	run_queue->sp--;
}

//
// Assign script variable
//
static void CmdAssignScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdAssignScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++] = SVAL(1);
	run_queue->sp--;
}

//
// Assign map variable
//
static void CmdAssignMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdAssignMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++] = SVAL(1);
	run_queue->sp--;
}

//
// Assign world variable
//
static void CmdAssignWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdAssignWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++] = SVAL(1);
	run_queue->sp--;
}

//
// Push a script variable
//
static void CmdPushScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdPushScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	SVAL(0) = run_queue->vars[*run_queue->ip++];
	run_queue->sp++;
}

//
// Push a map variable
//
static void CmdPushMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdPushMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	SVAL(0) = map_vars[*run_queue->ip++];
	run_queue->sp++;
}

//
// Push a world variable
//
static void CmdPushWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdPushWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	SVAL(0) = world_vars[*run_queue->ip++];
	run_queue->sp++;
}

//
// Add TOS to script variable
//
static void CmdAddScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdAddScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++] += SVAL(1);
	run_queue->sp--;
}

//
// Add TOS to map variable
//
static void CmdAddMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdAddMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++] += SVAL(1);
	run_queue->sp--;
}

//
// Add TOS to world variable
//
static void CmdAddWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdAddWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++] += SVAL(1);
	run_queue->sp--;
}

//
// Subtract TOS from script variable
//
static void CmdSubScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdSubScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++] -= SVAL(1);
	run_queue->sp--;
}

//
// Subtract TOS from map variable
//
static void CmdSubMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdSubMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++] -= SVAL(1);
	run_queue->sp--;
}

//
// Subtract TOS from world variable
//
static void CmdSubWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdSubWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++] -= SVAL(1);
	run_queue->sp--;
}

//
// Multiply TOS with script variable
//
static void CmdMulScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdMulScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++] *= SVAL(1);
	run_queue->sp--;
}

//
// Multiply TOS with map variable
//
static void CmdMulMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdMulMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++] *= SVAL(1);
	run_queue->sp--;
}

//
// Multiply TOS with world variable
//
static void CmdMulWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdMulWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++] *= SVAL(1);
	run_queue->sp--;
}

//
// Divide script variable by TOS
//
static void CmdDivScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdDivScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++] /= SVAL(1);
	run_queue->sp--;
}

//
// Divide map variable by TOS
//
static void CmdDivMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdDivMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++] /= SVAL(1);
	run_queue->sp--;
}

//
// Divide world variable by TOS
//
static void CmdDivWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdDivWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++] /= SVAL(1);
	run_queue->sp--;
}

//
// Modulo divide script variable by TOS
//
static void CmdModScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdModScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++] %= SVAL(1);
	run_queue->sp--;
}

//
// Modulo divide map variable by TOS
//
static void CmdModMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdModMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++] %= SVAL(1);
	run_queue->sp--;
}

//
// Modulo divide world variable by TOS
//
static void CmdModWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdModWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++] %= SVAL(1);
	run_queue->sp--;
}

//
// Increment script variable
//
static void CmdIncScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdIncScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++]++;
}

//
// Increment map variable
//
static void CmdIncMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdIncMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++]++;
}

//
// Increment world variable
//
static void CmdIncWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdIncWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++]++;
}

//
// Decrement script variable
//
static void CmdDecScriptVar(void)
{
	if (*run_queue->ip >= MAX_ACS_SCRIPT_VARS)
	{
		printf("Script %d: CmdDecScriptVar: no such script "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	run_queue->vars[*run_queue->ip++]--;
}

//
// Decrement map variable
//
static void CmdDecMapVar(void)
{
	if (*run_queue->ip >= MAX_ACS_MAP_VARS)
	{
		printf("Script %d: CmdDecMapVar: no such map "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	map_vars[*run_queue->ip++]--;
}

//
// Decrement world variable
//
static void CmdDecWorldVar(void)
{
	if (*run_queue->ip >= MAX_ACS_WORLD_VARS)
	{
		printf("Script %d: CmdDecWorldVar: no such world "
			"variable %d\n", run_queue->snum, *run_queue->ip);
		unlink_script(&run_queue, run_queue);
		return;
	}

	world_vars[*run_queue->ip++]--;
}

//
// Goto statement
//
static void CmdGoto(void)
{
	run_queue->ip = (int *)(code_base + *run_queue->ip);
}

//
// If goto dependend on TOS
//
static void CmdIfGoto(void)
{
	if (SVAL(1))
		run_queue->ip = (int *)(code_base + *run_queue->ip);
	else
		run_queue->ip++;
	run_queue->sp--;
}

//
// Drop TOS
//
static void CmdDrop(void)
{
	run_queue->sp--;
}

//
// Delay script by timer on TOS
//
static void CmdDelay(void)
{
	acs_t	*script;

	run_queue->timer = SVAL(1);
	run_queue->sp--;

	// unlink from run queue and link into delayed queue
	script = run_queue;
	script->inst_count = 0;
	unlink_script(&run_queue, script);
	link_script(&delayed_queue, script);
}

//
// Delay script by timer in next instruction
//
static void CmdDelayDirect(void)
{
	acs_t	*script;

	run_queue->timer = *run_queue->ip++;

	// unlink from run queue and link into delayed queue
	script = run_queue;
	script->inst_count = 0;
	unlink_script(&run_queue, script);
	link_script(&delayed_queue, script);
}

//
// Push a random number on stack, in the range of the two topmost values
//
static void CmdRandom(void)
{
	int	low, high;

	high = SVAL(1);
	low = SVAL(2);
	SVAL(2) = low + (P_Random() % (high - low + 1));
	run_queue->sp--;
}

//
// Push a random number on stack, in the range of the values
// in the next two instructions
//
static void CmdRandomDirect(void)
{
	int	low, high;

	low = *run_queue->ip++;
	high = *run_queue->ip++;
	SVAL(0) = low + (P_Random() % (high - low + 1));
	run_queue->sp++;
}

//
// Unlink script from run queue and link into wait queue.
// Event is waiting for tag, the wait value is on TOS.
//
static void CmdTagWait(void)
{
	acs_t	*script;

	script = run_queue;
	script->wait_event = wait_sector;
	script->wait_value = SVAL(1);
	run_queue->sp--;
	unlink_script(&run_queue, script);
	link_script(&wait_queue, script);
}

//
// Unlink script from run queue and link into wait queue.
// Event is waiting for tag, the wait value is in next instruction.
//
static void CmdTagWaitDirect(void)
{
	acs_t	*script;

	script = run_queue;
	script->wait_event = wait_sector;
	script->wait_value = *run_queue->ip++;
	unlink_script(&run_queue, script);
	link_script(&wait_queue, script);
}

//
// Unlink script from run queue and link into wait queue.
// Event is waiting for poly, the wait value is on TOS.
//
static void CmdPolyWait(void)
{
	acs_t	*script;

	script = run_queue;
	script->wait_event = wait_poly;
	script->wait_value = SVAL(1);
	run_queue->sp--;
	unlink_script(&run_queue, script);
	link_script(&wait_queue, script);
}

//
// Unlink script from run queue and link into wait queue.
// Event is waiting for poly, the wait value is in next instruction.
//
static void CmdPolyWaitDirect(void)
{
	acs_t	*script;

	script = run_queue;
	script->wait_event = wait_poly;
	script->wait_value = *run_queue->ip++;
	unlink_script(&run_queue, script);
	link_script(&wait_queue, script);
}

// Change Floor texture
// TOS		= string index with flatname
// TOS - 1	= sector tag, where to change floor texture
//
static void CmdChangeFloor(void)
{
	int	tag;
	int	flat;
	int	secnum = -1;

	if (SVAL(1) >= string_count || SVAL(1) < 0)
	{
		printf("Script %d: CmdChangeFloor: no such string %d\n",
			run_queue->snum, SVAL(1));
		unlink_script(&run_queue, run_queue);
		return;
	}

	flat = R_FlatNumForName(acs_strings[SVAL(1)]);;
	tag = SVAL(2);
	run_queue->sp -= 2;

	while ((secnum = P_FindSectorFromTag(tag, secnum)) >= 0)
		sectors[secnum].floorpic = flat;
}

//
// Change Floor texture
// ip		= sector tag, where to change floor texture
// ip + 1	= string index with flat name
//
static void CmdChangeFloorDirect(void)
{
	int	tag;
	int	flat;
	int	secnum = -1;

	tag = *run_queue->ip++;
	flat = *run_queue->ip++;

	if (flat >= string_count || flat < 0)
	{
		printf("Script %d: CmdChangeFloorDirect: no such string %d\n",
			run_queue->snum, flat);
		unlink_script(&run_queue, run_queue);
		return;
	}

	flat = R_FlatNumForName(acs_strings[flat]);

	while ((secnum = P_FindSectorFromTag(tag, secnum)) >= 0)
		sectors[secnum].floorpic = flat;
}

//
// Change Ceiling texture
// TOS		= string index with flatname
// TOS - 1	= sector tag, where to change ceiling texture
//
static void CmdChangeCeil(void)
{
	int	tag;
	int	flat;
	int	secnum = -1;

	if (SVAL(1) >= string_count || SVAL(1) < 0)
	{
		printf("Script %d: CmdChangeCeil: no such string %d\n",
			run_queue->snum, SVAL(1));
		unlink_script(&run_queue, run_queue);
		return;
	}

	flat = R_FlatNumForName(acs_strings[SVAL(1)]);;
	tag = SVAL(2);
	run_queue->sp -= 2;

	while ((secnum = P_FindSectorFromTag(tag, secnum)) >= 0)
		sectors[secnum].ceilingpic = flat;
}

//
// Change Ceiling texture
// ip		= sector tag, where to change ceiling texture
// ip + 1	= string index with flat name
//
static void CmdChangeCeilDirect(void)
{
	int	tag;
	int	flat;
	int	secnum = -1;

	tag = *run_queue->ip++;
	flat = *run_queue->ip++;

	if (flat >= string_count || flat < 0)
	{
		printf("Script %d: CmdChangeCeilDirect: no such string %d\n",
			run_queue->snum, flat);
		unlink_script(&run_queue, run_queue);
		return;
	}

	flat = R_FlatNumForName(acs_strings[flat]);

	while ((secnum = P_FindSectorFromTag(tag, secnum)) >= 0)
		sectors[secnum].ceilingpic = flat;
}

//
// Restart, means run script from the beginning again
//
static void CmdRestart(void)
{
	run_queue->ip = run_queue->code;
}

//
// Logical AND of the two topmost values, pop them and push result
//
static void CmdAndLogical(void)
{
	SVAL(2) = SVAL(2) && SVAL(1);
	run_queue->sp--;
}

//
// Logical OR of the two topmost values, pop them and push result
//
static void CmdOrLogical(void)
{
	SVAL(2) = SVAL(2) || SVAL(1);
	run_queue->sp--;
}

//
// Bitwise AND of the two topmost values, pop them and push result
//
static void CmdAndBitwise(void)
{
	SVAL(2) = SVAL(2) & SVAL(1);
	run_queue->sp--;
}

//
// Bitwise OR of the two topmost values, pop them and push result
//
static void CmdOrBitwise(void)
{
	SVAL(2) = SVAL(2) | SVAL(1);
	run_queue->sp--;
}

//
// Bitwise Exclusive OR of the two topmost values, pop them and push result
//
static void CmdEorBitwise(void)
{
	SVAL(2) = SVAL(2) ^ SVAL(1);
	run_queue->sp--;
}

//
// Negate TOS
//
static void CmdNegateLogical(void)
{
	SVAL(1) = !SVAL(1);
}

//
// Shift left, shift is TOS, value to shift is TOS - 1
//
static void CmdLShift(void)
{
	SVAL(2) = SVAL(2) << SVAL(1);
	run_queue->sp--;
}

//
// Shift right, shift is TOS, value to shift is TOS - 1
//
static void CmdRShift(void)
{
	SVAL(2) = SVAL(2) >> SVAL(1);
	run_queue->sp--;
}

//
// Unary minus of TOS
//
static void CmdUnaryMinus(void)
{
	SVAL(1) = -SVAL(1);
}

//
// If not goto, dependend on TOS
//
static void CmdIfNotGoto(void)
{
	if (SVAL(1))
		run_queue->ip++;
	else
		run_queue->ip = (int *)(code_base + *run_queue->ip);
	run_queue->sp--;
}

//
// Push line side on stack, from which the line was triggered
//
static void CmdLineSide(void)
{
	SVAL(0) = run_queue->side;
	run_queue->sp++;
}

//
// Link script into wait queue, wait value is TOS
//
static void CmdScriptWait(void)
{
	acs_t	*script;

	run_queue->wait_value = SVAL(1);
	run_queue->wait_event = wait_script;
	run_queue->sp--;
	script = run_queue;
	script->inst_count = 0;
	unlink_script(&run_queue, script);
	link_script(&wait_queue, script);
}

//
// Link script into wait queue, wait value is next instruction
//
static void CmdScriptWaitDirect(void)
{
	acs_t	*script;

	run_queue->wait_value = *run_queue->ip++;
	run_queue->wait_event = wait_script;
	script = run_queue;
	script->inst_count = 0;
	unlink_script(&run_queue, script);
	link_script(&wait_queue, script);
}

//
// Clear line special
//
static void CmdClearLineSpecial(void)
{
	if (run_queue->line)
		run_queue->line->special = 0;
}

//
// Branch for case statement
//
static void CmdCaseGoto(void)
{
	if (SVAL(1) == *run_queue->ip++)
	{
		run_queue->ip = (int *)(code_base + *run_queue->ip);
		run_queue->sp--;
	}
	else
	{
		run_queue->ip++;
	}
}

//
// Initialize print buffer
//
static void CmdBeginPrint(void)
{
	*print_buffer = 0;
}

//
// Print buffer is formatted, print message
//
static void CmdEndPrint(void)
{
	player_t	*player;

	if (run_queue->activator && run_queue->activator->player)
		player = run_queue->activator->player;
	else
		player = &players[consoleplayer];

	player->message = &print_buffer[0];
}

//
// Print string
//
static void CmdPrintString(void)
{
	if (SVAL(1) >= string_count || SVAL(1) < 0)
	{
		printf("Script %d: CmdPrintString: no such string %d\n",
			run_queue->snum, SVAL(1));
		unlink_script(&run_queue, run_queue);
		return;
	}

	strcat(&print_buffer[0], acs_strings[SVAL(1)]);
	run_queue->sp--;
}

//
// Print number
//
static void CmdPrintNumber(void)
{
	char	tmp[16];

	sprintf(tmp, "%d", SVAL(1));
	run_queue->sp--;
	strcat(&print_buffer[0], tmp);
}

//
// Print character
//
static void CmdPrintCharacter(void)
{
	char	*end;

	end = &print_buffer[0] + strlen(print_buffer);
	*end++ = SVAL(1);
	run_queue->sp--;
	*end = '\0';
}

//
// Push number of players on stack
//
static void CmdPlayerCount(void)
{
	int	i;
	int	count = 0;

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			count++;

	SVAL(0) = count;
	run_queue->sp++;
}

//
// Push game type on stack
//
static void CmdGameType(void)
{
	int	gametype;

	if (!netgame)
		gametype = GAME_SINGLE_PLAYER;
	else if (deathmatch)
		gametype = GAME_NET_DEATHMATCH;
	else if (altcoop)
		gametype = GAME_NET_ALTCOOP;
	else
		gametype = GAME_NET_COOPERATIVE;

	SVAL(0) = gametype;
	run_queue->sp++;
}

//
// Push game skill on stack
//
static void CmdGameSkill(void)
{
	SVAL(0) = gameskill;
	run_queue->sp++;
}

//
// Push time played this level on stack
//
static void CmdTimer(void)
{
	SVAL(0) = leveltime;
	run_queue->sp++;
}

//
// Play a sound in a sector
// TOS		= volume
// TOS + 1	= string index for the sound
//
static void CmdSectorSound(void)
{
	mobj_t	*mobj = (mobj_t*)0;

	if (SVAL(2) >= string_count || SVAL(2) < 0)
	{
		printf("Script %d: CmdSectorSound: no such string %d\n",
			run_queue->snum, SVAL(2));
		unlink_script(&run_queue, run_queue);
		return;
	}

	if (run_queue->line)
		mobj = (mobj_t *)&run_queue->line->frontsector->soundorg;

	S_StartSoundAtVolume(mobj, S_GetSoundID(acs_strings[SVAL(2)]), SVAL(1));
	run_queue->sp -= 2;
}

//
// Play sound that all players hear at same volume
//
// TOS		= volume
// TOS + 1	= string index for the sound
//
static void CmdAmbientSound(void)
{
	if (SVAL(2) >= string_count || SVAL(2) < 0)
	{
		printf("Script %d: CmdAmbientSound: no such string %d\n",
			run_queue->snum, SVAL(2));
		unlink_script(&run_queue, run_queue);
		return;
	}

	S_StartSoundAtVolume((mobj_t*)0, S_GetSoundID(acs_strings[SVAL(2)]),
		SVAL(1));
	run_queue->sp -= 2;
}

//
// Set line texture
//
// TOS		= string index for texture name
// TOS + 1	= position (upper, normal, lower)
// TOS + 2	= side
// TOS + 3	= line tag
//
static void CmdSetLineTexture(void)
{
	int	texture;
	int	position;
	int	side;
	int	tag;
	int	line = -1;
	side_t	*sidedef;

	if (SVAL(1) >= string_count || SVAL(1) < 0)
	{
		printf("Script %d: CmdSetLineTexture: no such string %d\n",
			run_queue->snum, SVAL(1));
		unlink_script(&run_queue, run_queue);
		return;
	}

	texture = R_TextureNumForName(acs_strings[SVAL(1)]);
	position = SVAL(2);
	side = SVAL(3) ? 1 : 0;
	tag = SVAL(4);
	run_queue->sp -= 4;

	while ((line = P_FindLineFromTag(tag, line)) >= 0)
	{
		sidedef = sides + lines[line].sidenum[side];

		switch (position)
		{
			case TEXTURE_TOP:
				sidedef->toptexture = texture;
				break;
			case TEXTURE_MIDDLE:
				sidedef->midtexture = texture;
				break;
			case TEXTURE_BOTTOM:
				sidedef->bottomtexture = texture;
				break;
			default:
				break;
		}
	}
}

//
// Set line blocking
//
// TOS		= flag, 1 set line blocking, 0 set line unblocking
// TOS + 1	= line tag
//
static void CmdSetLineBlocking(void)
{
	int	line = -1;

	while ((line = P_FindLineFromTag(SVAL(2), line)) >= 0)
	{
		if (SVAL(1))
			lines[line].flags |= ML_BLOCKING;
		else
			lines[line].flags &=
				~(ML_BLOCKING | ML_BLOCKEVERYTHING);
	}
	run_queue->sp -= 2;
}

//
// Set Line Special
//
// TOS 1 - 5	= arguments 1 - 5 for line args
// TOS 6	= new special
// TOS 7	= line tag
//
static void CmdSetLineSpecial(void)
{
	int	ln = -1;

	while ((ln = P_FindLineFromTag(SVAL(7), ln)) >= 0)
	{
		line_t	*line = &lines[ln];

		line->special = SVAL(6);
		line->args[0] = SVAL(5);
		line->args[1] = SVAL(4);
		line->args[2] = SVAL(3);
		line->args[3] = SVAL(2);
		line->args[4] = SVAL(1);
	}
	run_queue->sp -= 7;
}

//
// Play sound at a thing
//
// TOS		= volume
// TOS + 1	= string index with sound to play
// TOS + 2	= thing id
//
static void CmdThingSound(void)
{
	mobj_t	*mobj = (mobj_t*)0;

	if (SVAL(2) >= string_count || SVAL(2) < 0)
	{
		printf("Script %d: CmdThingSound: no such string %d\n",
			run_queue->snum, SVAL(2));
		unlink_script(&run_queue, run_queue);
		return;
	}

	while ((mobj = P_FindMobjByTid(mobj, SVAL(3))) != (mobj_t*)0)
		S_StartSoundAtVolume(mobj, S_GetSoundID(acs_strings[SVAL(2)]),
				     SVAL(1));

	run_queue->sp -= 3;
}
