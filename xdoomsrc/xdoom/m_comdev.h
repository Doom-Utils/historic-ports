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
// DESCRIPTION: Handheld communication device for XDoom
//
//-----------------------------------------------------------------------------

#ifndef __COMDEV__
#define __COMDEV__

extern boolean	commdevmsg;
extern boolean	commdevactive;
extern char	*comdev_lastmsg;

void M_CommDevice(void);
void M_CommNewMsg(int);
void M_CommNewMsgTxt(char *);

#endif
