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
//	This CD audio module just does nothing at all and is used as
//	a stub to compile on systems, for which no working support
//	module was written yet.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "cd_audio.h"

int CD_init()
{
    return 1;	// make sure it will play the default musserver music!
}

void CD_shutdown()
{
}

void CD_song(void *data)
{
    data = data;
}

void CD_start(int handle)
{
    handle = handle;
}

void CD_stop(int handle)
{
    handle = handle;
}

void CD_pause(int handle)
{
    handle = handle;
}

void CD_resume(int handle)
{
    handle = handle;
}

void CD_volume(int vol)
{
    vol = vol;
}
