// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=4:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1998, 1999 by Udo Munk
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
// DESCRIPTION: Voodoo 3DFX frame buffer module
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <glide.h>
#include <vgamouse.h>
#include <signal.h>
#include <fcntl.h>
#include <assert.h>

#include <sys/ioctl.h>
#include <sys/kd.h>

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

static void keyboard_events(void);
static int xlatekey(int);
static void mouse_events(int, int, int, int, int, int, int);

//
// Select one of the possible render modes.
// If you don't have a SGI Octane or a 1000MHz Pentium CPU, drawing
// anti-aliased pixels is to slow and the result doesn't look that
// much better than simply doubling the pixels anyway.
//
#define RENDER_LFB				// fast direct frame buffer writes
// #define RENDER_AAP				// anti-aliased pixels

//
// Define one of the frame buffer formats, 24bit colors (888) is pretty
// hefty and should be used on systems better than Pentium 200 only.
// Both formats look the same because the Doom engine uses 256 color
// textures only anyway, so 555 is the default because it's a lot faster.
//
#define FRAMEBUFFER_FORMAT GR_LFB_SRC_FMT_555
// #define FRAMEBUFFER_FORMAT GR_LFB_SRC_FMT_888

// pixel format and bytes per pixel in the frame buffer
#if (FRAMEBUFFER_FORMAT == GR_LFB_SRC_FMT_888)
typedef FxU32 pixel_t;
#define BPP 4
#else
typedef FxU16 pixel_t;
#define BPP 2
#endif

//
// The resolution we use on the 3DFX
//
#define DFX_WIDTH   640
#define DFX_HEIGHT  400

// colormap
#ifdef RENDER_LFB
static pixel_t colors[256];
#else
static float colors[3][256];
#endif

// image to write to frame buffer
#ifdef RENDER_LFB
static pixel_t image[DFX_WIDTH * DFX_HEIGHT];
#endif

// vertex for anti-aliased pixel rendering
#ifdef RENDER_AAP
static GrVertex vertex;
#endif

// line discipline
static struct termios term;

// mouse handling
extern int		usemouse;
extern char		*mousedev;
extern char		*mousetype;
static int		mouse_type;

// Joystick handling
extern int		usejoystick;

// sound handling
extern int	use_mmap;
extern int	sndupd_flag;
static int	snd_updated;

void I_InitGraphics(void)
{
	char			version[80];
	struct termios	t;
	int				i;

	// make sure that signals bring us back into text mode
	signal(SIGINT, (void (*)(int))I_Quit);
	signal(SIGQUIT, (void (*)(int))I_Quit);
	signal(SIGHUP, (void (*)(int))I_Quit);
	signal(SIGTERM, (void (*)(int))I_Quit);

	// get and print glide version used
	grGlideGetVersion(version);
	printf("Using glide version %s\n", version);

	// initialize glide
	grGlideInit();
	grSstSelect(0);
	assert(grSstWinOpen(0, GR_RESOLUTION_640x400, GR_REFRESH_75Hz,
			GR_COLORFORMAT_ARGB, GR_ORIGIN_UPPER_LEFT, 2, 1));

	// setup renderering engine
	grDitherMode(GR_DITHER_DISABLE);

#ifdef RENDER_AAP
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_LOCAL_ITERATED, GR_COMBINE_OTHER_NONE, FXFALSE);

	grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
		GR_BLEND_ZERO, GR_BLEND_ZERO);

	vertex.a = 255.0f;
#endif

	// switch keyboard into scan code mode
	ioctl(fileno(stdin), KDSKBMODE, K_MEDIUMRAW);

	// save line discipline and set into raw mode, non blocking
	tcgetattr(fileno(stdin), &term);
	t = term;
	t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	t.c_iflag &= ~(IMAXBEL | IGNBRK | IGNCR | IGNPAR | BRKINT | INLCR |
				ICRNL | INPCK | ISTRIP | IXON | IUCLC | IXANY | IXOFF);
	t.c_cflag &= ~(CSIZE | PARENB);
	t.c_cflag |= CS8;
	t.c_oflag &= ~(OCRNL | OLCUC | ONLCR | OPOST);
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = 0;
	tcsetattr(fileno(stdin), TCSAFLUSH, &t);

	// check if user wants mouse and set it up
	if (usemouse)
	{
		mouse_type = MOUSE_NONE;
		for (i = 0; mousetypes[i].name != NULL; i++)
		{
			if (!strcmp(mousetype, mousetypes[i].name))
				mouse_type = mousetypes[i].type;
		}

		mouse_init(mousedev, mouse_type, MOUSE_DEFAULTSAMPLERATE);
		mouse_setxrange(0, DFX_WIDTH - 1);
		mouse_setyrange(0, DFX_HEIGHT - 1);
		mouse_setwrap(MOUSE_NOWRAP);
		mouse_seteventhandler(mouse_events);
	}

#ifdef USE_JOYSTICK
	// check if user wants joystick and set it up
	if (usejoystick)
		I_InitJoystick();
#endif
}

void I_ShutdownGraphics(void)
{
	// shutdown glide and return to text mode
	grGlideShutdown();

	// reset keyboard to xlate mode
	ioctl(fileno(stdin), KDSKBMODE, K_XLATE);

	// restore line discipline
	tcsetattr(fileno(stdin), TCSAFLUSH, &term);

	// flush stdout
	fflush(stdout);

	// flush stdin
	tcflush(fileno(stdin), TCIOFLUSH);
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
	int				x, y;
	static int		lasttic;
	int				tics;
	int				i;
	unsigned char	*src = (unsigned char *) screens[0];
#ifdef RENDER_LFB
	pixel_t			pixel;
	pixel_t			*dst1 = &image[0];
	pixel_t			*dst2 = &image[SCREENWIDTH*2];
	const int		step = SCREENWIDTH*2;
#endif

	// draws little dots on the bottom of the screen, a simple fps meter
	if (devparm)
	{
		i = I_GetTime();
		tics = i - lasttic;
		lasttic = i;
		if (tics > 20)
			tics = 20;

		for (i = 0; i < tics * 2 ;i += 2)
			screens[0][(SCREENHEIGHT - 2) * SCREENWIDTH + i + 3] = 0xff;
		for (; i < 20 * 2; i += 2)
			screens[0][(SCREENHEIGHT - 2) * SCREENWIDTH + i + 3] = 0x0;
	}

#ifdef RENDER_LFB
	// convert the engine rendered image to RGB image, double size
	y = SCREENHEIGHT - 1;
	do
	{
		x = SCREENWIDTH - 1;
		do {
			pixel = colors[*src++];
			*dst1++ = pixel;
			*dst1++ = pixel;
			*dst2++ = pixel;
			*dst2++ = pixel;
		} while (x--);
		dst1 += step;
		dst2 += step;
	} while (y--);

	// write image into back buffer
	grLfbWriteRegion(GR_BUFFER_BACKBUFFER, 0, 0, FRAMEBUFFER_FORMAT,
		DFX_WIDTH, DFX_HEIGHT, DFX_WIDTH*BPP, &image[0]);
#else
	// convert every pixel to a vertex and draw anti-aliased vertices
	grBufferClear(0, 0, GR_ZDEPTHVALUE_FARTHEST);
	for (y = 0; y < SCREENHEIGHT; y++)
	{
		for (x = 0; x < SCREENWIDTH; x++)
		{
			vertex.x = (float)(x * 2 + 1);
			vertex.y = (float)(y * 2 + 1);
			vertex.r = colors[0][*src];
			vertex.g = colors[1][*src];
			vertex.b = colors[2][*src];
			grAADrawPoint(&vertex);
			src++;
		}
	}
#endif

	// swap buffers and make new frame visible
	grBufferSwap(1);

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
	register int	i;

#ifdef RENDER_LFB
	// build LUT with 16 or 32bit RGB colors for palette
	for (i = 0; i < 256; i++)
	{
#if (FRAMEBUFFER_FORMAT == GR_LFB_SRC_FMT_888)
		colors[i] = (gammatable[usegamma][*palette] << 16) |
					(gammatable[usegamma][*(palette + 1)] << 8) |
					(gammatable[usegamma][*(palette + 2)]);
#else
		colors[i] = ((gammatable[usegamma][*palette] >> 3) << 10) |
					((gammatable[usegamma][*(palette + 1)] >> 3) << 5) |
					(gammatable[usegamma][*(palette + 2)] >> 3);
#endif
		palette += 3;
	}
#else
	// built color table for vertices
	for (i = 0; i < 256; i++)
	{
		colors[0][i] = (float) gammatable[usegamma][*palette];
		colors[1][i] = (float) gammatable[usegamma][*(palette + 1)];
		colors[2][i] = (float) gammatable[usegamma][*(palette + 2)];
		palette += 3;
	}
#endif
}

void I_StartTic(void)
{
	keyboard_events();

	if (usemouse)
	{
		mouse_update();
	}

#ifdef USE_JOYSTICK
	if (usejoystick)
	{
		joystick_events();
	}
#endif
}

// ------------------------------------------------------------------------
//				keyboard event handling
// ------------------------------------------------------------------------

static void keyboard_events(void)
{
	unsigned char		c;
	event_t				event;

	while (read(fileno(stdin), &c, 1) == 1)
	{
		event.type = (c & 0x80) ? ev_keyup : ev_keydown;
		event.data1 = xlatekey((int)(c & 0x7F));
		if (event.data1 > 0)
			D_PostEvent(&event);
	}
}

static int xlatekey(int c)
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
