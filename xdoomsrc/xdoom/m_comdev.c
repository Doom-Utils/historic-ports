// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
// $Id:$
//
// Copyright (C) 1999 by Udo Munk
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
// DESCRIPTION: Handheld communication device for XDoom
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "doomdef.h"
#include "doomstat.h"
#include "m_comdev.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "s_sound.h"
#include "sounds.h"

#define CX (SCREENWIDTH - 200) / 2 + 15
#define CY 30

extern void M_WriteText(int, int, char *);

boolean		commdevmsg;			// flag set if new message
boolean		commdevactive;			// flag set if device active
static char	*comtxt;			// pointer to COMTXT lump
static int	comtxt_size;			// size of the lump
char		*comdev_lastmsg;		// pointer to message string

//
// Search for comdev_lastmsg in the COMTXT lump and display message if found.
// If not found display error message.
//
static void M_CommMsg()
{
	int	flag = 0;
	int	line = 0;
	char	*p = comtxt;
	int	size = comtxt_size;

	while ((p = memchr(p, '#', size)))
	{
		p++;
		if (!memcmp(p, comdev_lastmsg, strlen(comdev_lastmsg)))
		{
			flag++;
			break;
		}
		size -= strlen(p) + 2;
	}

	if (!flag)
	{
		M_WriteText(CX, CY, "DEVICE MALFUNCTION");
		M_WriteText(CX, CY+10, comdev_lastmsg);
	}
	else
	{
		while (*p != '\n')
			p++;
		p++;
		while (*p != '\n')
		{
			M_WriteText(CX, CY + line * 10, p);
			p += strlen(p) + 1;
			line++;
			if (line >= 10)
				break;
		}
	}
}

//
// Called with the tag of a trigger, activate message icon and update
// the message to display in the gadget.
//
void M_CommNewMsg(int num)
{
	static char msg[10];

	commdevmsg = true;
	sprintf(&msg[0], "%d", num);
	comdev_lastmsg = &msg[0];
	S_StartSound(NULL, sfx_beep);
}

//
// Called with a string which is a label in COMTXT lump. Activate message
// icon and update the message to display in the gadget.
//
void M_CommNewMsgTxt(char *s)
{
	static char msg[40];

	commdevmsg = true;
	strcpy(&msg[0], s);
	comdev_lastmsg = &msg[0];
	S_StartSound(NULL, sfx_beep);
}

//
// Called by the game loop, display an icon if new message received and
// display the gadget if the device is activated.
//
void M_CommDevice()
{
	static int lump = 0;

	if (lump == 0)
	{
		lump = W_CheckNumForName("COMTXT");
		if (lump != -1)
		{
			comtxt = W_CacheLumpNum(lump, PU_STATIC);
			comtxt_size = W_LumpLength(lump);
		}
	}

	if (commdevmsg && (lump != -1))
	{
		V_DrawPatch(5, 5, 0,
			W_CacheLumpName("STCDROM", PU_CACHE));
	}

	if (commdevactive)
	{
		V_DrawPatch((SCREENWIDTH - 200) / 2, 10, 0,
			W_CacheLumpName("COMDEV", PU_CACHE));
		if (lump == -1)
			M_WriteText(CX, CY, "NO OS BOOTED");
		else
			M_CommMsg();
	}
}
