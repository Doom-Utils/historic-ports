// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1999 by Priit Jaerv
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
// DESCRIPTION: DOOM graphics stuff for libvgl (FreeBSD).
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <machine/mouse.h>
#include <machine/console.h>
#include <vgl.h>
#include <signal.h>
#include <termios.h>

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "d_main.h"
#include "s_sound.h"
#include "i_sound.h"


static VGLBitmap	image;
static int		Xsize;
static int		Ysize;

static int		kbmode = 0;	// flag set when keyboard in raw mode
static struct termios	term;		// saved line discipline

extern boolean		showkey;
extern int		use_mmap;
extern int		sndupd_flag;
static int		snd_updated;

extern int		usemouse;	// flag set when mouse needed
static int		mouse_fd;	// mouse file descriptor

#define MOUSE_LEFT	0x01
#define MOUSE_MIDDLE	0x02
#define MOUSE_RIGHT	0x04

#ifdef FREEBSD_MOUSE_HACK
#define MOUSE_SYNC_1	0xf8		// mouse packet header bitmasks,
#define MOUSE_SYNC_2	0x80		// useful for syncing.
static int		mouse_level = 1;
extern char		*mousedev;
#else
const char		*mousedev = "/dev/sysmouse";
#endif // FREEBSD_MOUSE_HACK

//
//  Translates the raw console scancode into something doom recognizes
//
void xlatekey(unsigned char c, event_t *e)
{
	unsigned char	more;

	e->type = (c & 0x80) ? ev_keyup : ev_keydown;

	switch (c & 0x7F)
	{
	case 1:		e->data1 = KEY_ESCAPE;				break;
	case 2:		e->data1 = '1';					break;
	case 3:		e->data1 = '2';					break;
	case 4:		e->data1 = '3';					break;
	case 5:		e->data1 = '4';					break;
	case 6:		e->data1 = '5';					break;
	case 7:		e->data1 = '6';					break;
	case 8:		e->data1 = '7';					break;
	case 9:		e->data1 = '8';					break;
	case 10:	e->data1 = '9';					break;
	case 11:	e->data1 = '0';					break;
	case 12:	e->data1 = KEY_MINUS;				break;
	case 13:	e->data1 = KEY_EQUALS;				break;
	case 14:	e->data1 = KEY_BACKSPACE;			break;
	case 15:	e->data1 = KEY_TAB;				break;
	case 16:	e->data1 = 'q';					break;
	case 17:	e->data1 = 'w';					break;
	case 18:	e->data1 = 'e';					break;
	case 19:	e->data1 = 'r';					break;
	case 20:	e->data1 = 't';					break;
	case 21:	e->data1 = 'y';					break;
	case 22:	e->data1 = 'u';					break;
	case 23:	e->data1 = 'i';					break;
	case 24:	e->data1 = 'o';					break;
	case 25:	e->data1 = 'p';					break;
	case 28:	e->data1 = KEY_ENTER;				break;
	case 29:	e->data1 = KEY_RCTRL;				break;
	case 30:	e->data1 = 'a';					break;
	case 31:	e->data1 = 's';					break;
	case 32:	e->data1 = 'd';					break;
	case 33:	e->data1 = 'f';					break;
	case 34:	e->data1 = 'g';					break;
	case 35:	e->data1 = 'h';					break;
	case 36:	e->data1 = 'j';					break;
	case 37:	e->data1 = 'k';					break;
	case 38:	e->data1 = 'l';					break;
	case 41:	e->data1 = '`';					break;
	case 42:	e->data1 = KEY_RSHIFT;				break;
	case 44:	e->data1 = 'z';					break;
	case 45:	e->data1 = 'x';					break;
	case 46:	e->data1 = 'c';					break;
	case 47:	e->data1 = 'v';					break;
	case 48:	e->data1 = 'b';					break;
	case 49:	e->data1 = 'n';					break;
	case 50:	e->data1 = 'm';					break;
	case 54:	e->data1 = KEY_RSHIFT;				break;
	case 56:	e->data1 = KEY_RALT;				break;
	case 57:	e->data1 = ' ';					break;
//	case 58:	e->data1 = KEY_CAPSLOCK;			break;
	case 59:	e->data1 = KEY_F1;				break;
	case 60:	e->data1 = KEY_F2;				break;
	case 61:	e->data1 = KEY_F3;				break;
	case 62:	e->data1 = KEY_F4;				break;
	case 63:	e->data1 = KEY_F5;				break;
	case 64:	e->data1 = KEY_F6;				break;
	case 65:	e->data1 = KEY_F7;				break;
	case 66:	e->data1 = KEY_F8;				break;
	case 67:	e->data1 = KEY_F9;				break;
	case 68:	e->data1 = KEY_F10;				break;
	case 69:	e->data1 = KEY_PAUSE;				break;
	case 74:	e->data1 = KEY_MINUS;				break;
	case 78:	e->data1 = KEY_EQUALS;				break;
	case 86:	e->data1 = '<';					break;
	case 87:	e->data1 = KEY_F11;				break;
	case 88:	e->data1 = KEY_F12;				break;
	case 72:	e->data1 = KEY_UPARROW;				break;
	case 75:	e->data1 = KEY_LEFTARROW;			break;
	case 77:	e->data1 = KEY_RIGHTARROW;			break;
	case 80:	e->data1 = KEY_DOWNARROW;			break;
	default:	e->data1 = 0;					break;
	}

	// some keys are prefixed with 0xe0, handle those here
	if (c == 224)
	{
		while (read(fileno(stdin), &more, 1) != 1)
			/* nothing */ ;
		e->type = (more & 0x80) ? ev_keyup : ev_keydown;
		switch (more & 0x7F)
		{
		case 29:	e->data1 = KEY_RCTRL;			break;
		case 56:	e->data1 = KEY_RALT;			break;
		case 72:	e->data1 = KEY_UPARROW;			break;
		case 75:	e->data1 = KEY_LEFTARROW;		break;
		case 77:	e->data1 = KEY_RIGHTARROW;		break;
		case 80:	e->data1 = KEY_DOWNARROW;		break;
		default:	e->data1 = 0;				break;
		}
	}
}

void keyboard_events()
{
	unsigned char	c;
	event_t		event;

	// read the scan codes send by the console driver and process them
	while (read(fileno(stdin), &c, 1) == 1)
	{
		xlatekey(c, &event);
		if (event.data1 > 0)
			D_PostEvent(&event);
	}
}

void mouse_events(void)
{
	event_t		event;
	mousestatus_t	status;
	char		buf[8];
	size_t		bufsize = sizeof(char);

#ifdef FREEBSD_MOUSE_HACK

	// On /dev/sysmouse this is a blocking read, so we're screwed.
	// It works directly on the PS/2 mouse device, however.

	while (read(mouse_fd, &buf[0], bufsize) > 0)
	{
		// check if we're in sync
		if ((buf[0] & MOUSE_SYNC_1) == MOUSE_SYNC_2)
		{

			// packet header matches, read rest of it
			if (read(mouse_fd, &buf[1], bufsize * 7) < 7) break;

			// now decode the packet. ***broken currently***
			event.type = ev_mouse;

			event.data1 = ((buf[0] & 4) ? 1 : 0) |
					((buf[0] & 2) ? 2 : 0) |
					((buf[0] & 1) ? 4 : 0);

			event.data2 = (int) (buf[1] + buf[3]);
			event.data3 = (int) (buf[2] + buf[4]);

			D_PostEvent(&event);
		}
		else
		{
			// we're out of sync, skip this byte
		}
	}
#else

	if (ioctl(mouse_fd, MOUSE_GETSTATUS, &status) < 0)
		return;

	if ((status.flags & MOUSE_POSCHANGED) ||
		(status.button != status.obutton))
	{
		event.type = ev_mouse;

		event.data1 = ((status.button & MOUSE_LEFT) ? 1 : 0) |
				((status.button & MOUSE_MIDDLE) ? 2 : 0) |
				((status.button & MOUSE_RIGHT) ? 4 : 0);

		event.data2 = status.dx << 2;
		event.data3 = -(status.dy << 2);

		D_PostEvent(&event);
	}
#endif

}

void I_ShutdownGraphics(void)
{
	// reset keyboard to xlate mode
	if (kbmode)
	{
		ioctl(fileno(stdin), KDSKBMODE, K_XLATE);

		// restore line discipline
		tcsetattr(fileno(stdin), TCSAFLUSH, &term);

		kbmode = 0;
	}

	if (usemouse)
		close(mouse_fd);

	VGLEnd();
}

//
// I_StartTic
//
void I_StartTic(void)
{
	keyboard_events();

	if (usemouse)
		mouse_events();
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
}

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    static int	lasttic;
    int		tics;
    int		i;

    // draws little dots on the bottom of the screen
    if (devparm)
    {
	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20)
	    tics = 20;

	for (i = 0; i < tics * 2; i += 2)
	    screens[0][(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0xff;
	for ( ; i < 20 * 2; i += 2)
	    screens[0][(SCREENHEIGHT - 1) * SCREENWIDTH + i] = 0x0;
    }

    // draw the image
    VGLBitmapCopy(&image, 0, 0, VGLDisplay, 0, 0, Xsize, Ysize);
}

//
// I_ReadScreen
//
void I_ReadScreen(byte* scr)
{
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette(byte* palette)
{
    register int	i;
    byte		red[256];
    byte		green[256];
    byte		blue[256];

    // set the palette entries
    for (i = 0; i < 256; i++)
    {
	red[i] = gammatable[usegamma][*palette++] >> 2;
	green[i] = gammatable[usegamma][*palette++] >> 2;
	blue[i] = gammatable[usegamma][*palette++] >> 2;
    }

    VGLSetPalette(red, green, blue);
}

void I_InitGraphics(void)
{
    static int		firsttime = 1;
    struct termios	t;

    if (!firsttime)
	return;
    firsttime = 0;

    if (geteuid())
        I_Error("You can only run this program as root, sorry.\n");

    signal(SIGINT, (void (*)(int))I_Quit);
    signal(SIGHUP, (void (*)(int))I_Quit);
    signal(SIGTERM, (void (*)(int))I_Quit);
    signal(SIGQUIT, (void (*)(int))I_Quit);

    Xsize = SCREENWIDTH;
    Ysize = SCREENHEIGHT;

    VGLInit(SW_VGA_CG320);

    image.Type = MEMBUF;
    image.Bitmap = screens[0];
    image.Xsize = Xsize; image.Ysize = Ysize;

    // switch keyboard into raw mode
    if (ioctl(fileno(stdin), KDSKBMODE, K_RAW) < 0)
	I_Error("Could not switch keyboard into raw mode\n");

    // save line discipline and set into raw mode, non blocking
    tcgetattr(fileno(stdin), &term);
    t = term;

    t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    t.c_iflag &= ~(IMAXBEL | IGNBRK | IGNCR | IGNPAR | BRKINT | INLCR |
		   ICRNL | INPCK | ISTRIP | IXON | IXANY | IXOFF);
    t.c_cflag &= ~(CSIZE | PARENB);
    t.c_cflag |= CS8;
    t.c_oflag &= ~(ONLCR | OPOST);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &t);

    // keyboard setup now
    kbmode = 1;

    if (usemouse)
    {
	if ((mouse_fd = open(mousedev, O_RDONLY)) < 0)
	{
		printf("cannot open mouse device %s\n", mousedev);
		usemouse = 0;
	}
#ifdef FREEBSD_MOUSE_HACK
	else ioctl(mouse_fd, MOUSE_SETLEVEL, &mouse_level);
#endif
    }
}

//
// I_StartFrame
//
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
