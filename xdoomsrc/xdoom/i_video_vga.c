// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=4:
//-----------------------------------------------------------------------------
//
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
// DESCRIPTION: Linux svgalib frame buffer module
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <string.h>
#include <signal.h>
#include <vga.h>
#include <vgakeyboard.h>
#include <vgamouse.h>

#include "doomstat.h"
#include "d_main.h"
#include "m_argv.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "i_sound.h"
#include "i_lmouse.h"

#ifdef USE_JOYSTICK
#include "i_joy.h"
#endif

static void keyboard_events(int, int);
static void mouse_events(int, int, int, int, int, int, int);
static int xlatekey(int);

// sound handling flags
extern int	use_mmap;
extern int	sndupd_flag;
static int	snd_updated;

// colormap
static struct
{
	int		r;
	int		g;
	int		b;
} colors[256];

// mouse handling
extern int		usemouse;
extern char		*mousedev;
extern char		*mousetype;
static int		mouse_type;

// Joystick handling
extern int		usejoystick;

void I_InitGraphics(void)
{
	int i;

	// make sure that signals bring us back into text mode
	signal(SIGINT, (void(*)(int))I_Quit);
	signal(SIGQUIT, (void(*)(int))I_Quit);
	signal(SIGHUP, (void(*)(int))I_Quit);
	signal(SIGTERM, (void(*)(int))I_Quit);

	// init VGA card
	if (vga_init() != 0)
		I_Error("Could not initialize graphics console\n");
	if (vga_setmode(G320x200x256) != 0)
		I_Error("Could not switch to graphics mode\n");

	// init keyboard
	keyboard_init();
	keyboard_seteventhandler(keyboard_events);

	// init mouse
	if (usemouse)
	{
		mouse_type = MOUSE_NONE;
		for (i = 0; mousetypes[i].name != NULL; i++)
		{
			if (!strcmp(mousetype, mousetypes[i].name))
				mouse_type = mousetypes[i].type;
		}

		mouse_init(mousedev, mouse_type, MOUSE_DEFAULTSAMPLERATE);
		mouse_setxrange(0, SCREENWIDTH - 1);
		mouse_setyrange(0, SCREENHEIGHT - 1);
		mouse_setwrap(MOUSE_NOWRAP);
		mouse_seteventhandler(mouse_events);
	}

#ifdef USE_JOYSTICK
	// init joystick
	if (usejoystick)
		I_InitJoystick();
#endif
}

void I_ShutdownGraphics(void)
{
	keyboard_close();
	if (usemouse)
		mouse_close();

	if (vga_getcurrentmode() != TEXT)
		vga_setmode(TEXT);
}

void I_StartFrame(void)
{
	// update sound if necessary
#if !defined(SNDINTR) && !defined(SNDSERV)
	if (sndupd_flag && !use_mmap)
	{
		I_SubmitSound();
		snd_updated = 1;
	}
	else
		snd_updated = 0;
#endif
}

void I_FinishUpdate(void)
{
	static int		lasttic;
	int				tics;
	int				i;

	// draws little dots on the bottom of the screen, a simple fps meter
	if (devparm)
	{
		i = I_GetTime();
		tics = i - lasttic;
		lasttic = i;
		if (tics > 20)
			tics = 20;

		for (i = 0; i < tics * 2; i += 2)
			screens[0][(SCREENHEIGHT - 2) * SCREENWIDTH + i + 3] = 0xff;
		for (; i < 20 * 2; i += 2)
			screens[0][(SCREENHEIGHT - 2) * SCREENWIDTH + i + 3] = 0x0;
	}

	// blit frame
	memcpy(vga_getgraphmem(), screens[0], SCREENWIDTH * SCREENHEIGHT);

	// sleep a bit if there was no sound update
	if (!snd_updated)
	{
		I_WaitVBL(1);
	}
}

void I_UpdateNoBlit(void)
{
	// empty
}

void I_ReadScreen(byte *scr)
{
	memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette(byte *palette)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		colors[i].r = gammatable[usegamma][*palette++] >> 2;
		colors[i].g	= gammatable[usegamma][*palette++] >> 2;
		colors[i].b = gammatable[usegamma][*palette++] >> 2;
	}
	vga_setpalvec(0, 256, (int *)colors);
}

void I_StartTic (void)
{
	keyboard_update();

	if (usemouse)
		mouse_update();

#ifdef USE_JOYSTICK
	if (usejoystick)
		joystick_events();
#endif
}

// ------------------------------------------------------------------------
//						keyboard event handling
// ------------------------------------------------------------------------

static void keyboard_events (int scancode, int press)
{
	event_t		event;

	if (press == KEY_EVENTPRESS)
		event.type = ev_keydown;
	else
		event.type = ev_keyup;
	event.data1 = xlatekey(scancode);
	if (event.data1 > 0)
		D_PostEvent(&event);
}

static int xlatekey (int c)
{
	int rc;

	switch (c)
	{
	case 1:		rc = KEY_ESCAPE;				break;
	case 2:		rc = '1';						break;
	case 3:		rc = '2';						break;
	case 4:		rc = '3';						break;
	case 5:		rc = '4';						break;
	case 6:		rc = '5';						break;
	case 7:		rc = '6';						break;
	case 8:		rc = '7';						break;
	case 9:		rc = '8';						break;
	case 10:	rc = '9';						break;
	case 11:	rc = '0';						break;
	case 12:	rc = KEY_MINUS;					break;
	case 13:	rc = KEY_EQUALS;				break;
	case 14:	rc = KEY_BACKSPACE;				break;
	case 15:	rc = KEY_TAB;					break;
	case 16:	rc = 'q';						break;
	case 17:	rc = 'w';						break;
	case 18:	rc = 'e';						break;
	case 19:	rc = 'r';						break;
	case 20:	rc = 't';						break;
	case 21:	rc = 'y';						break;
	case 22:	rc = 'u';						break;
	case 23:	rc = 'i';						break;
	case 24:	rc = 'o';						break;
	case 25:	rc = 'p';						break;
	case 28:	rc = KEY_ENTER;					break;
	case 29:	rc = KEY_RCTRL;					break;
	case 30:	rc = 'a';						break;
	case 31:	rc = 's';						break;
	case 32:	rc = 'd';						break;
	case 33:	rc = 'f';						break;
	case 34:	rc = 'g';						break;
	case 35:	rc = 'h';						break;
	case 36:	rc = 'j';						break;
	case 37:	rc = 'k';						break;
	case 38:	rc = 'l';						break;
	case 41:	rc = '`';						break;
	case 42:	rc = KEY_RSHIFT;				break;
	case 44:	rc = 'z';						break;
	case 45:	rc = 'x';						break;
	case 46:	rc = 'c';						break;
	case 47:	rc = 'v';						break;
	case 48:	rc = 'b';						break;
	case 49:	rc = 'n';						break;
	case 50:	rc = 'm';						break;
	case 54:	rc = KEY_RSHIFT;				break;
	case 56:	rc = KEY_RALT;					break;
	case 57:	rc = ' ';						break;
	case 58:	rc = KEY_CAPSLOCK;				break;
	case 59:	rc = KEY_F1;					break;
	case 60:	rc = KEY_F2;					break;
	case 61:	rc = KEY_F3;					break;
	case 62:	rc = KEY_F4;					break;
	case 63:	rc = KEY_F5;					break;
	case 64:	rc = KEY_F6;					break;
	case 65:	rc = KEY_F7;					break;
	case 66:	rc = KEY_F8;					break;
	case 67:	rc = KEY_F9;					break;
	case 68:	rc = KEY_F10;					break;
	case 74:	rc = KEY_MINUS;					break;
	case 78:	rc = KEY_EQUALS;				break;
	case 86:	rc = '<';						break;
	case 87:	rc = KEY_F11;					break;
	case 88:	rc = KEY_F12;					break;
	case 97:	rc = KEY_RCTRL;					break;
	case 100:	rc = KEY_RALT;					break;
	case 72:
	case 103:	rc = KEY_UPARROW;				break;
	case 75:
	case 105:	rc = KEY_LEFTARROW;				break;
	case 77:
	case 106:	rc = KEY_RIGHTARROW;			break;
	case 80:
	case 108:	rc = KEY_DOWNARROW;				break;
	case 111:	rc = KEY_BACKSPACE;				break;
	case 119:	rc = KEY_PAUSE;					break;
	default:	rc = 0;							break;
	}
	return rc;
}

// ------------------------------------------------------------------------
//				mouse event handling
// ------------------------------------------------------------------------

static void mouse_events(int button, int dx, int dy, int dz,
						 int rdx, int rdy, int rdz)
{
	event_t		event;

	event.type = ev_mouse;
	event.data1 = ((button & MOUSE_LEFTBUTTON) ? 1 : 0) |
				  ((button & MOUSE_MIDDLEBUTTON) ? 2 : 0) |
				  ((button & MOUSE_RIGHTBUTTON) ? 4 : 0);

	event.data2 = dx;
	event.data3 = -dy;

	D_PostEvent(&event);
}
