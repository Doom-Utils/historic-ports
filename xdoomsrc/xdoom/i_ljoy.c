// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
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
// DESCRIPTION: Linux x86 Joystick support module
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>

#include "d_main.h"
#include "d_event.h"

#define JDEVICE		"/dev/js0"
#define XTHRES		0x40			// threshold for x axis
#define YTHRES		0x40			// threshold for y axis

extern int			usejoystick;
static int			joy_fd;
static struct JS_DATA_TYPE	jdata;
static int			jleft, jright, jup, jdown;

//
// read axis and button state from joystick device
//
static int read_joystick(void)
{
	if (read(joy_fd, &jdata, JS_RETURN) != JS_RETURN) {
		printf("joystick_events: cannot read from device\n");
		usejoystick = 0;
		return 0;
	}
	return 1;
}

//
// initialze joystick device
//
void I_InitJoystick(void)
{
	// open the device
	if ((joy_fd = open(JDEVICE, O_RDONLY)) < 0)
		printf("I_InitJoystick: cannot open joystick device %s\n",
			JDEVICE);

	// get the axis values (assuming neutral position) and setup diffs
	if (read_joystick()) {
		jleft = jdata.x - XTHRES;
		jright = jdata.x + XTHRES;
		jup = jdata.y - YTHRES;
		jdown = jdata.y + YTHRES;
	}
}

//
// event handling routine for joystick device
//
void joystick_events(void)
{
	event_t ev;

	if (!read_joystick())
		return;

	ev.type = ev_joystick;
	ev.data1 = jdata.buttons;
	ev.data2 = (jdata.x < jleft) ? -1 : ((jdata.x > jright) ? 1 : 0);
	ev.data3 = (jdata.y < jup) ? -1 : ((jdata.y > jdown) ? 1 : 0);

	D_PostEvent(&ev);
}
