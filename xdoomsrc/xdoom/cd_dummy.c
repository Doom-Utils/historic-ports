// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
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
// DESCRIPTION:
//	I used this dummy module to see what XDoom does in it's music
//	support. It was easier to write this dummy module than going
//	through the XDoom sources and figure when which function would
//	be called. This might be helpfull for anyone who wants to
//	implement a new CD audio support module and it can be used
//	as a template to start from.
//	All the functions do is just writing a text to stdout so one
//	can see, how the XDoom engine calls them.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include "cd_audio.h"

int CD_init()
{
    printf("CD audio: initialized\n");
    return 0;
}

void CD_shutdown()
{
    printf("CD audio: shutdown\n");
}

void CD_song(void * data)
{
    printf("CD audio: song: <%s>\n", (char *)data);
}

void CD_start(int handle)
{
    printf("CD audio: start: <%d>\n", handle);
}

void CD_stop(int handle)
{
    printf("CD audio: stop: <%d>\n", handle);
}

void CD_pause(int handle)
{
    printf("CD audio: pause: <%d>\n", handle);
}

void CD_resume(int handle)
{
    printf("CD audio: resume: <%d>\n", handle);
}

void CD_volume(int vol)
{
    printf("CD audio: volume: <%d>\n", vol);
}
