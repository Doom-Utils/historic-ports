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
// DESCRIPTION:
//		SCO OpenServer5, Unixware2 and Unixware7 VGA frame buffer module
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>

#ifdef SCOOS5
#include <sys/vtkd.h>
#include <sys/console.h>
#include <sys/event.h>
#include <mouse.h>
#endif

#if defined(SCOUW7) || defined(SCOUW2)
#include <sys/kd.h>
#include <sys/vt.h>
#include <sys/mouse.h>
#endif

#include <sys/sysi86.h>
#include <sys/v86.h>

#include "doomstat.h"
#include "d_main.h"
#include "m_argv.h"
#include "i_system.h"
#include "v_video.h"
#include "i_video.h"
#include "s_sound.h"
#include "i_sound.h"

// under Unixware use mouse device, under Openserver use event manager

#if defined(SCOUW2) || defined(SCOUW7)
#define MOUSE	"/dev/mouse"
#endif

extern void port_out(int, int);
extern int port_in(int);

static void rel_screen(int);
static void acq_screen(int);
static void keyboard_events(void);
static void xlatekey(unsigned char, event_t *);
static void mouse_events(void);

static int				grmode = 0;	// flag set when we are in graphics mode
static int				kbmode = 0;	// flag set when keyboard in raw mode
static int				oldmode;	// save current text mode here
static int				newmode;	// save current graphics mode here
static char				*screenmem;	// mapped video memory
static struct termios	term;		// saved line discipline
static char				spal[256][3]; // save palette here

extern int				sndupd_flag; // flag when we have to update sound
extern int				use_mmap;	 // flag when mmap'ed sound updates
static int				snd_updated; // flag when we have updated sound

static int				mouse_fd;	// file descriptor for mouse device
extern int				usemouse;	// flag if mouse support is wanted

void I_InitGraphics(void)
{
	int				adapter;
	struct vt_mode	smode;
	struct termios	t;
#ifdef SCOOS5
	dmask_t			dmask = D_REL | D_BUTTON;
	struct devinfo	*devinf;
#endif

	// make sure that this signals bring us back into text mode
	signal(SIGINT, (void (*)(int))I_Quit);
	signal(SIGQUIT, (void (*)(int))I_Quit);
	signal(SIGHUP, (void (*)(int))I_Quit);
	signal(SIGTERM, (void (*)(int))I_Quit);

	// check if the console device is a VGA card
	if ((adapter = ioctl(fileno(stdin), CONS_CURRENT, (char *)0)) < 0)
		I_Error("Could not get display type, trying to run from xterm?\n");
	if (adapter != VGA)
		I_Error("Display adapter is not a VGA card\n");

	// setup signal handlers for virtual console switching
	smode.mode = VT_PROCESS;
	smode.waitv = 0;
	smode.relsig = SIGUSR1;
	smode.acqsig = SIGUSR2;
	smode.frsig = 0;
	if (ioctl(fileno(stdin), VT_SETMODE, &smode) < 0)
		I_Error("Could not setup screen switch signal handlers\n");
	signal(SIGUSR1, rel_screen);
	signal(SIGUSR2, acq_screen);

	// save users current text mode
	oldmode = ioctl(fileno(stdin), CONS_GET, (char *)0);

	// if root runs the program we can raise I/O priv to 3, not worth tho.
	if (getuid() == 0)
	{
#if defined(SCOUW2) || defined(SCOUW7)
		sysi86(SI86IOPL, 3);
#else	/* OS5 */
		sysi86(SI86V86, V86SC_IOPL, 0x3000);
#endif
	}

	// get I/O privileges for VGA card
	if (ioctl(fileno(stdin), VGA_IOPRIVL, 1) < 0)
		I_Error("VGA I/O privileges denied\n");

	// switch console into graphics mode, 320x200x256
	if (ioctl(fileno(stdin), SW_VGA13, (char *)0) < 0)
		I_Error("Could not switch into VGA graphics mode 13h\n");

	// map video memory into process space
	if ((screenmem = (char *)ioctl(fileno(stdin), MAPCONS, (char *)0)) == NULL)
		I_Error("Could not map video memory\n");

	// we are in graphics mode now
	grmode = 1;

	// switch keyboard into raw mode
	if (ioctl(fileno(stdin), KDSKBMODE, K_RAW) < 0)
		I_Error("Could not switch keyboard into raw mode\n");

#if defined(SCOUW2) || defined(SCOUW7)
	// disable AT&T queue mode, just to be sure
	ioctl(fileno(stdin), KDQUEMODE, (char *)0);
#endif

	// save line discipline and set into raw mode, non blocking
	tcgetattr(fileno(stdin), &term);
	t = term;

#ifdef SCOOS5
#define IMAXBEL 0		// missing under OS5??
#endif

	t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	t.c_iflag &= ~(IMAXBEL | IGNBRK | IGNCR | IGNPAR | BRKINT | INLCR |
				ICRNL | INPCK | ISTRIP | IXON | IUCLC | IXANY | IXOFF);
	t.c_cflag &= ~(CSIZE | PARENB);
	t.c_cflag |= CS8;
	t.c_oflag &= ~(OCRNL | OLCUC | ONLCR | OPOST);
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &t);

	// keyboard setup now
	kbmode = 1;

	// setup mouse device if user wants mouse support
	if (usemouse)
	{
#if defined(SCOUW2) || defined(SCOUW7)
		// under Unixware open mouse device
		if (mouse_fd = open(MOUSE, O_RDONLY) < 0)
		{
			printf("cannot open mouse device %s\n", MOUSE);
			usemouse = 0;
			return;
		}
#endif

#ifdef SCOOS5
		// under Openserver initialize event manager
		if (ev_init() < 0)
		{
			printf("cannot initialize event manager\n");
			usemouse = 0;
			return;
		}

		if ((mouse_fd = ev_open(&dmask)) < 0)
		{
			printf("cannot open event queue for mouse device\n");
			usemouse = 0;
			return;
		}

		devinf = (struct devinfo *)0;
		if ((devinf = ev_getdev(D_REL, devinf)) == (struct devinfo *)0)
		{
			printf("no mouse device attached to event queue\n");
			usemouse = 0;
			return;
		}
#endif
	}
}

void I_ShutdownGraphics(void)
{
	// restore text mode
	if (grmode)
	{
		ioctl(fileno(stdin), MODESWITCH | oldmode, (char *)0);
		grmode = 0;
	}

	// reset keyboard to xlate mode
	if (kbmode)
	{
		ioctl(fileno(stdin), KDSKBMODE, K_XLATE);

		// restore line discipline
		tcsetattr(fileno(stdin), TCSAFLUSH, &term);

		kbmode = 0;
	}

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
	memcpy(screenmem, screens[0], SCREENWIDTH * SCREENHEIGHT);

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
	int						i;

	//
	// First how this should be done with regrads to the system
	// documentation. This works, but under Unixware7 it's slow beyond
	// belief, the VGAIO ioctl() is no good, so we use self written
	// assembler functions for VGA port access. They are in asmmod-x86.s,
	// in case you need them for something else...
	//
#if 0
	struct port_io_arg		port;

	memset(&port, 0, sizeof(struct port_io_arg));

	// select palette index 0
	port.args[0].dir = OUT_ON_PORT;
	port.args[0].port = 0x3c8;
	port.args[0].data = 0;
	ioctl(0, VGAIO, &port);

	port.args[0].dir = port.args[1].dir = port.args[2].dir = OUT_ON_PORT;
	port.args[0].port = port.args[1].port = port.args[2].port = 0x3c9;

	// write palette
	for (i = 0; i < 256; i++)
	{
		port.args[0].data = gammatable[usegamma][*palette++] >> 2;
		port.args[1].data = gammatable[usegamma][*palette++] >> 2;
		port.args[2].data = gammatable[usegamma][*palette++] >> 2;
		ioctl(0, VGAIO, &port);
	}
#endif

	port_out(0, 0x3c8);
	for (i = 0; i < 256; i++)
	{
		port_out(gammatable[usegamma][*palette++] >> 2, 0x3c9);
		port_out(gammatable[usegamma][*palette++] >> 2, 0x3c9);
		port_out(gammatable[usegamma][*palette++] >> 2, 0x3c9);
	}
}

void I_StartTic(void)
{
	keyboard_events();

	if (usemouse)
		mouse_events();
}

// ------------------------------------------------------------------------
//				signal handler for VT switching
// ------------------------------------------------------------------------

//
// signal handler for when user screen flips away from us
//
static void rel_screen(int val)
{
	int i;
#if 0
	struct port_io_arg		port;
#endif

#if defined (SCOUW7) || defined(SCOUW2)
#define VT_TRUE 1		// missing somehow, dunno
#endif

	signal(SIGUSR1, rel_screen);

	// save the current graphics mode
	newmode = ioctl(fileno(stdin), CONS_GET, (char *)0);

	// save the current color palette
#if 0
	memset(&port, 0, sizeof(struct port_io_arg));

	// select palette index 0
	port.args[0].dir = OUT_ON_PORT;
	port.args[0].port = 0x3c8;
	port.args[0].data = 0;
	ioctl(0, VGAIO, &port);

	port.args[0].dir = port.args[1].dir = port.args[2].dir = IN_ON_PORT;
	port.args[0].port = port.args[1].port = port.args[2].port = 0x3c9;

	// save current palette
	for (i = 0; i < 256; i++)
	{
		ioctl(0, VGAIO, &port);
		spal[i][0] = port.args[0].data;
		spal[i][1] = port.args[1].data;
		spal[i][2] = port.args[2].data;
	}
#endif

	port_out(0, 0x3c8);
	for (i = 0; i < 256; i++)
	{
		spal[i][0] = port_in(0x3c9);
		spal[i][1] = port_in(0x3c9);
		spal[i][2] = port_in(0x3c9);
	}

    // pause the music
    S_PauseSound();

	// restore text mode
	ioctl(fileno(stdin), MODESWITCH | oldmode, (char *)0);

	// release VT
	ioctl(0, VT_RELDISP, VT_TRUE);

	// wait until user switches back to us
	pause();
}

//
// signal handler for when user screen flips back to us
//
static void acq_screen(int val)
{
	int i;
#if 0
	struct port_io_arg		port;
#endif

	signal(SIGUSR2, acq_screen);

	// restore graphics mode
	ioctl(fileno(stdin), MODESWITCH | newmode, (char *)0);

	// restore the saved color palette
#if 0
	memset(&port, 0, sizeof(struct port_io_arg));

	// select palette index 0
	port.args[0].dir = OUT_ON_PORT;
	port.args[0].port = 0x3c8;
	port.args[0].data = 0;
	ioctl(0, VGAIO, &port);

	port.args[0].dir = port.args[1].dir = port.args[2].dir = OUT_ON_PORT;
	port.args[0].port = port.args[1].port = port.args[2].port = 0x3c9;

	// restore saved palette
	for (i = 0; i < 256; i++)
	{
		port.args[0].data = spal[i][0];
		port.args[1].data = spal[i][1];
		port.args[2].data = spal[i][2];
		ioctl(0, VGAIO, &port);
	}
#endif

	port_out(0, 0x3c8);
	for (i = 0; i < 256; i++)
	{
		port_out(spal[i][0], 0x3c9);
		port_out(spal[i][1], 0x3c9);
		port_out(spal[i][2], 0x3c9);
	}

    // resume the music
	S_ResumeSound();

	// acknowledge VT switch
	ioctl(0, VT_RELDISP, VT_ACKACQ);
}

// ------------------------------------------------------------------------
//				keyboard event handling
// ------------------------------------------------------------------------

static void keyboard_events(void)
{
	unsigned char	c;
	event_t			event;

	// read the scan codes send by the console driver and process them
	while (read(fileno(stdin), &c, 1) == 1)
	{
		xlatekey(c, &event);
		if (event.data1 > 0)
			D_PostEvent(&event);
	}
}

//
// The following scan code translation routines are good enough for Doom,
// for other programs you might want to use 'showkey' to figure out the
// scan codes send by the SCO console drivers. The 'showkey' program is
// included in the xdoom source distribution in the contrib directory.
//

#if defined(SCOOW2) || defined(SCOUW7)
static void xlatekey(unsigned char c, event_t *e)
{
	unsigned char	more;

	e->type = (c & 0x80) ? ev_keyup : ev_keydown;

	switch (c & 0x7F)
	{
	case 1:		e->data1 = KEY_ESCAPE;				break;
	case 2:		e->data1 = '1';						break;
	case 3:		e->data1 = '2';						break;
	case 4:		e->data1 = '3';						break;
	case 5:		e->data1 = '4';						break;
	case 6:		e->data1 = '5';						break;
	case 7:		e->data1 = '6';						break;
	case 8:		e->data1 = '7';						break;
	case 9:		e->data1 = '8';						break;
	case 10:	e->data1 = '9';						break;
	case 11:	e->data1 = '0';						break;
	case 12:	e->data1 = KEY_MINUS;				break;
	case 13:	e->data1 = KEY_EQUALS;				break;
	case 14:	e->data1 = KEY_BACKSPACE;			break;
	case 15:	e->data1 = KEY_TAB;					break;
	case 16:	e->data1 = 'q';						break;
	case 17:	e->data1 = 'w';						break;
	case 18:	e->data1 = 'e';						break;
	case 19:	e->data1 = 'r';						break;
	case 20:	e->data1 = 't';						break;
	case 21:	e->data1 = 'y';						break;
	case 22:	e->data1 = 'u';						break;
	case 23:	e->data1 = 'i';						break;
	case 24:	e->data1 = 'o';						break;
	case 25:	e->data1 = 'p';						break;
	case 28:	e->data1 = KEY_ENTER;				break;
	case 29:	e->data1 = KEY_RCTRL;				break;
	case 30:	e->data1 = 'a';						break;
	case 31:	e->data1 = 's';						break;
	case 32:	e->data1 = 'd';						break;
	case 33:	e->data1 = 'f';						break;
	case 34:	e->data1 = 'g';						break;
	case 35:	e->data1 = 'h';						break;
	case 36:	e->data1 = 'j';						break;
	case 37:	e->data1 = 'k';						break;
	case 38:	e->data1 = 'l';						break;
	case 41:	e->data1 = '`';						break;
	case 42:	e->data1 = KEY_RSHIFT;				break;
	case 44:	e->data1 = 'z';						break;
	case 45:	e->data1 = 'x';						break;
	case 46:	e->data1 = 'c';						break;
	case 47:	e->data1 = 'v';						break;
	case 48:	e->data1 = 'b';						break;
	case 49:	e->data1 = 'n';						break;
	case 50:	e->data1 = 'm';						break;
	case 54:	e->data1 = KEY_RSHIFT;				break;
	case 56:	e->data1 = KEY_RALT;				break;
	case 57:	e->data1 = ' ';						break;
	case 58:	e->data1 = KEY_CAPSLOCK;			break;
	case 59:	e->data1 = KEY_F1;					break;
	case 60:	e->data1 = KEY_F2;					break;
	case 61:	e->data1 = KEY_F3;					break;
	case 62:	e->data1 = KEY_F4;					break;
	case 63:	e->data1 = KEY_F5;					break;
	case 64:	e->data1 = KEY_F6;					break;
	case 65:	e->data1 = KEY_F7;					break;
	case 66:	e->data1 = KEY_F8;					break;
	case 67:	e->data1 = KEY_F9;					break;
	case 68:	e->data1 = KEY_F10;					break;
	case 69:	e->data1 = KEY_PAUSE;				break;
	case 74:	e->data1 = KEY_MINUS;				break;
	case 78:	e->data1 = KEY_EQUALS;				break;
	case 86:	e->data1 = '<';						break;
	case 87:	e->data1 = KEY_F11;					break;
	case 88:	e->data1 = KEY_F12;					break;
	case 72:	e->data1 = KEY_UPARROW;				break;
	case 75:	e->data1 = KEY_LEFTARROW;			break;
	case 77:	e->data1 = KEY_RIGHTARROW;			break;
	case 80:	e->data1 = KEY_DOWNARROW;			break;
	default:	e->data1 = 0;						break;
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
		default:	e->data1 = 0;					break;
		}
	}
}
#endif

//
// The OpenServer5 console driver is a pretty weird thing, probably
// for backwards compatibility to Xenix, SCO UNIX and OpenDeskTop.
// If you don't understand the following stuff, the xdoom source
// distribution includes a program 'showkey' in the contrib directory,
// which was used to examine the 'scan codes' send by the OS5 console
// driver.
//
// Also please note that I was using a german PS/2 keyboard when I used
// the showkey program, the scan codes for your keyboard might be all
// different, I don't know, I don't have your keyboard. The mapkey for
// this keyboard is /usr/lib/keyboard/ps.ibm.ger, so you could try that,
// if you don't want to change the scan code mapping in this source.
//
// All this mess comes because the scan codes can be mapped with mapkey,
// which is a stupid idea, if I want raw scan codes, I want that and not
// what I get right now. It might be possible to fix this mess with
// loading an own map table into the driver, which uses the real keyboard
// scan codes, as documented. Not sure though, if this would fix the
// missing key up events and multiple usage of the same key up value, for
// different keys. However, it can be done different somehow as the Xsco
// sever demonstrates, is only a question of finding out what needs to
// get done...
//

#ifdef SCOOS5
static void xlatekey(unsigned char c, event_t *e)
{
	unsigned char			more;
    static unsigned char	saved = '\0';

	switch (c)
	{
	case 1:		e->data1 = KEY_ESCAPE; e->type = ev_keydown;	break;
	case 252:	e->data1 = KEY_ESCAPE; e->type = ev_keyup;		break;
	case 28:	e->data1 = KEY_ENTER; e->type = ev_keydown;		break;
	case 163:	e->data1 = KEY_ENTER; e->type = ev_keyup;		break;
	case 15:	e->data1 = KEY_TAB; e->type = ev_keydown;		break;
	case 197:	e->data1 = KEY_TAB; e->type = ev_keyup;			break;
	case 14:	e->data1 = KEY_BACKSPACE; e->type = ev_keydown;
				saved = KEY_BACKSPACE;							break;
	case 72:	e->data1 = KEY_UPARROW; e->type = ev_keydown;	break;
	case 200:	e->data1 = KEY_UPARROW; e->type = ev_keyup;		break;
	case 80:	e->data1 = KEY_DOWNARROW; e->type = ev_keydown;	break;
	case 208:	e->data1 = KEY_DOWNARROW; e->type = ev_keyup;	break;
	case 75:	e->data1 = KEY_LEFTARROW; e->type = ev_keydown;	break;
	case 203:	e->data1 = KEY_LEFTARROW; e->type = ev_keyup;	break;
	case 77:	e->data1 = KEY_RIGHTARROW; e->type = ev_keydown; break;
	case 205:	e->data1 = KEY_RIGHTARROW; e->type = ev_keyup;	break;
	case 29:	e->data1 = KEY_RCTRL; e->type = ev_keydown;		break;
	case 165:	e->data1 = KEY_RCTRL; e->type = ev_keyup;		break;
	case 56:	e->data1 = KEY_RALT; e->type = ev_keydown;		break;
	case 184:	e->data1 = KEY_RALT; e->type = ev_keyup;		break;
	case 42:	e->data1 = KEY_RSHIFT; e->type = ev_keydown;	break;
	case 172:	e->data1 = KEY_RSHIFT; e->type = ev_keyup;		break;
	case 54:	e->data1 = KEY_RSHIFT; e->type = ev_keydown;	break;
	case 182:	e->data1 = KEY_RSHIFT; e->type = ev_keyup;		break;
	case 58:	e->data1 = KEY_CAPSLOCK; e->type = ev_keydown;	break;
	case 186:	e->data1 = KEY_CAPSLOCK; e->type = ev_keyup;	break;
	case 59:	e->data1 = KEY_F1; e->type = ev_keydown;
				saved = KEY_F1;									break;
	case 60:	e->data1 = KEY_F2; e->type = ev_keydown;
				saved = KEY_F2;									break;
	case 61:	e->data1 = KEY_F3; e->type = ev_keydown;		break;
	case 189:	e->data1 = KEY_F3; e->type = ev_keyup;			break;
	case 62:	e->data1 = KEY_F4; e->type = ev_keydown;		break;
	case 190:	e->data1 = KEY_F4; e->type = ev_keyup;			break;
	case 63:	e->data1 = KEY_F5; e->type = ev_keydown;		break;
	case 191:	e->data1 = KEY_F5; e->type = ev_keyup;			break;
	case 64:	e->data1 = KEY_F6; e->type = ev_keydown;		break;
	case 179:	e->data1 = KEY_F6; e->type = ev_keyup;			break;
	case 65:	e->data1 = KEY_F7; e->type = ev_keydown;		break;
	case 180:	e->data1 = KEY_F7; e->type = ev_keyup;			break;
	case 66:	e->data1 = KEY_F8; e->type = ev_keydown;		break;
	case 164:	e->data1 = KEY_F8; e->type = ev_keyup;			break;
	case 67:	e->data1 = KEY_F9; e->type = ev_keydown;		break;
	case 195:	e->data1 = KEY_F9; e->type = ev_keyup;			break;
	case 68:	e->data1 = KEY_F10; e->type = ev_keydown;
				saved = KEY_F10;								break;
	case 87:	e->data1 = KEY_F11; e->type = ev_keydown;		break;
	case 215:	e->data1 = KEY_F11; e->type = ev_keyup;			break;
	case 88:	e->data1 = KEY_F12; e->type = ev_keydown;		break;
	case 216:	e->data1 = KEY_F12; e->type = ev_keyup;			break;
	case 2:		e->data1 = '1'; e->type = ev_keydown;			break;
	case 233:	e->data1 = '1'; e->type = ev_keyup;				break;
	case 3:		e->data1 = '2'; e->type = ev_keydown;			break;
	case 226:	e->data1 = '2'; e->type = ev_keyup;				break;
	case 4:		e->data1 = '3'; e->type = ev_keydown;			break;
	case 228:	e->data1 = '3'; e->type = ev_keyup;				break;
	case 5:		e->data1 = '4'; e->type = ev_keydown;
				saved = '4';									break;
	case 6:		e->data1 = '5'; e->type = ev_keydown;			break;
	case 229:	e->data1 = '5'; e->type = ev_keyup;				break;
	case 7:		e->data1 = '6'; e->type = ev_keydown;			break;
	case 231:	e->data1 = '6'; e->type = ev_keyup;				break;
	case 8:		e->data1 = '7'; e->type = ev_keydown;			break;
	case 234:	e->data1 = '7'; e->type = ev_keyup;				break;
	case 9:		e->data1 = '8'; e->type = ev_keydown;			break;
	case 137:	e->data1 = '8'; e->type = ev_keyup;				break;
	case 10:	e->data1 = '9'; e->type = ev_keydown;			break;
	case 232:	e->data1 = '9'; e->type = ev_keyup;				break;
	case 11:	e->data1 = '0'; e->type = ev_keydown;			break;
	case 239:	e->data1 = '0'; e->type = ev_keyup;				break;
	case 12:	e->data1 = KEY_MINUS; e->type = ev_keydown;		break;
	case 238:	e->data1 = KEY_MINUS; e->type = ev_keyup;		break;
	case 13:	e->data1 = KEY_EQUALS; e->type = ev_keydown;	break;
	case 236:	e->data1 = KEY_EQUALS; e->type = ev_keyup;		break;
	case 86:	e->data1 = '<'; e->type = ev_keydown;
				saved = '<';									break;
	case 16:	e->data1 = 'q'; e->type = ev_keydown;			break;
	case 201:	e->data1 = 'q'; e->type = ev_keyup;				break;
	case 17:	e->data1 = 'w'; e->type = ev_keydown;			break;
	case 230:	e->data1 = 'w'; e->type = ev_keyup;				break;
	case 18:	e->data1 = 'e'; e->type = ev_keydown;			break;
	case 198:	e->data1 = 'e'; e->type = ev_keyup;				break;
	case 19:	e->data1 = 'r'; e->type = ev_keydown;			break;
	case 244:	e->data1 = 'r'; e->type = ev_keyup;				break;
	case 20:	e->data1 = 't'; e->type = ev_keydown;			break;
	case 246:	e->data1 = 't'; e->type = ev_keyup;				break;
	case 21:	e->data1 = 'y'; e->type = ev_keydown;			break;
	case 242:	e->data1 = 'y'; e->type = ev_keyup;				break;
	case 22:	e->data1 = 'u'; e->type = ev_keydown;			break;
	case 251:	e->data1 = 'u'; e->type = ev_keyup;				break;
	case 23:	e->data1 = 'i'; e->type = ev_keydown;			break;
	case 249:	e->data1 = 'i'; e->type = ev_keyup;				break;
	case 24:	e->data1 = 'o'; e->type = ev_keydown;			break;
	case 255:	e->data1 = 'o'; e->type = ev_keyup;				break;
	case 25:	e->data1 = 'p'; e->type = ev_keydown;
				saved = 'p';									break;
	case 30:	e->data1 = 'a'; e->type = ev_keydown;			break;
	case 158:	e->data1 = 'a'; e->type = ev_keyup;				break;
	// *** FUNNY, for the key s on my keyboard the driver sends nuffin
	case 32:	e->data1 = 'd'; e->type = ev_keydown;			break;
	case 225:	e->data1 = 'd'; e->type = ev_keyup;				break;
	case 33:	e->data1 = 'f'; e->type = ev_keydown;			break;
	case 237:	e->data1 = 'f'; e->type = ev_keyup;				break;
	case 34:	e->data1 = 'g'; e->type = ev_keydown;			break;
	case 243:	e->data1 = 'g'; e->type = ev_keyup;				break;
	case 35:	e->data1 = 'h'; e->type = ev_keydown;			break;
	case 250:	e->data1 = 'h'; e->type = ev_keyup;				break;
	case 36:	e->data1 = 'j'; e->type = ev_keydown;			break;
	case 241:	e->data1 = 'j'; e->type = ev_keyup;				break;
	case 37:	e->data1 = 'k'; e->type = ev_keydown;			break;
	case 209:	e->data1 = 'k'; e->type = ev_keyup;				break;
	case 38:	e->data1 = 'l'; e->type = ev_keydown;			break;
	case 170:	e->data1 = 'l'; e->type = ev_keyup;				break;
	case 44:	e->data1 = 'z'; e->type = ev_keydown;
				saved = 'z';									break;
	case 45:	e->data1 = 'x'; e->type = ev_keydown;			break;
	case 161:	e->data1 = 'x'; e->type = ev_keyup;				break;
	case 46:	e->data1 = 'c'; e->type = ev_keydown;			break;
	case 171:	e->data1 = 'c'; e->type = ev_keyup;				break;
	case 47:	e->data1 = 'v'; e->type = ev_keydown;
				saved = 'v';									break;
	// *** hum, key b sends a key down code, but no key up code, grr
	// *** same with n
	case 50:	e->data1 = 'm'; e->type = ev_keydown;			break;
	case 167:	e->data1 = 'm'; e->type = ev_keyup;				break;
	// *** , no key up code
	// *** . no key up code
	case 57:	e->data1 = ' '; e->type = ev_keydown;			break;
	case 185:	e->data1 = ' '; e->type = ev_keyup;				break;

	//
	// wow, 188 is send for releasing F2 and z, what a crap
    // more fun with 214
	// more fun with 196
	// more fun with 187
	//
	case 187:
	case 196:
	case 214:
	case 188:	e->data1 = saved; e->type = ev_keyup;
				saved = '\0';									break;

	// some keys are prefixed with 0xe0, handle those here
	case 224:
		// then again, releasing '4' also sends 224, yuck
		if (saved)
		{
			e->data1 = saved; e->type = ev_keyup;
			saved = '\0';
			break;
		}
		 while (read(fileno(stdin), &more, 1) != 1)
			/* nothing */ ;
		switch (more)
		{
		case 29:	e->data1 = KEY_RCTRL; e->type = ev_keydown;		break;
		case 165:	e->data1 = KEY_RCTRL; e->type = ev_keyup;		break;
		case 56:	e->data1 = KEY_RALT; e->type = ev_keydown;		break;
		case 184:	e->data1 = KEY_RALT; e->type = ev_keyup;		break;
		case 72:	e->data1 = KEY_UPARROW; e->type = ev_keydown;	break;
		case 200:	e->data1 = KEY_UPARROW; e->type = ev_keyup;		break;
		case 80:	e->data1 = KEY_DOWNARROW; e->type = ev_keydown;	break;
		case 208:	e->data1 = KEY_DOWNARROW; e->type = ev_keyup;	break;
		case 75:	e->data1 = KEY_LEFTARROW; e->type = ev_keydown;	break;
		case 203:	e->data1 = KEY_LEFTARROW; e->type = ev_keyup;	break;
		case 77:	e->data1 = KEY_RIGHTARROW; e->type = ev_keydown; break;
		case 205:	e->data1 = KEY_RIGHTARROW; e->type = ev_keyup;	break;
		default:	e->data1 = 0;									break;
		}
	break;

	default:	e->data1 = 0;									break;
	}
}
#endif

// ------------------------------------------------------------------------
//						mouse event handling
// ------------------------------------------------------------------------

#if defined(SCOUW2) || defined(SCOUW7)
static void mouse_events(void)
{
	event_t				event;
	struct mouseinfo	m;

	if (ioctl(mouse_fd, MOUSEIOCREAD, &m) < 0)
	{
		printf("cannot get mouse status\n");
		return;
	}

	if ((m.status&MOVEMENT) || (m.status&BUT1CHNG) ||
		(m.status&BUT2CHNG) || (m.status&BUT3CHNG))
	{
		event.type = ev_mouse;
		event.data1 = ((m.status & BUT1STAT) ? 1 : 0) |
					   ((m.status & BUT2STAT) ? 2 : 0) |
					   ((m.status & BUT3STAT) ? 4 : 0);

		event.data2 = m.xmotion << 2;
		event.data3 = -(m.ymotion << 2);

		D_PostEvent(&event);
	}
}
#endif

#ifdef SCOOS5
static void mouse_events(void)
{
	event_t		event;
	EVENT		*evp;

	while ((evp = ev_read()) != (EVENT *)0)
	{
		event.type = ev_mouse;
		event.data1 = ((EV_BUTTONS(*evp) & BUTTON3) ? 1 : 0) |
					   ((EV_BUTTONS(*evp) & BUTTON2) ? 2 : 0) |
					   ((EV_BUTTONS(*evp) & BUTTON1) ? 4 : 0);

		event.data2 = EV_DX(*evp) << 2;
		event.data3 = EV_DY(*evp) << 2;

		ev_pop();

		D_PostEvent(&event);
	}
}
#endif
