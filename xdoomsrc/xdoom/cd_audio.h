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
//	This module defines the prototypes for every function all
//	CD audio support modules must implement. This functions
//	are called from the XDoom sound support module i_sound.c
//	and this builds an abstract layer to implement CD audio
//	support with various different methods on various UNIX OS's.
//
//-----------------------------------------------------------------------------

#ifndef __CDAUDIO__
#define __CDAUDIO__

int	CD_init(void);
void	CD_shutdown(void);
void	CD_song(void *);
void	CD_start(int);
void	CD_stop(int);
void	CD_pause(int);
void	CD_resume(int);
void	CD_volume(int);

#endif
