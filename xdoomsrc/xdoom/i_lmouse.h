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
// DESCRIPTION:
//	This are the mouse types supported with Linux svgalib 1.3.1
//
//-----------------------------------------------------------------------------

#ifndef __LMOUSE__
#define __LMOUSE__

struct {
	char	*name;
	int	type;
} mousetypes[] = {
#ifdef MOUSE_MICROSOFT
	{ "microsoft",		MOUSE_MICROSOFT },
#endif
#ifdef MOUSE_MOUSESYSTEMS
	{ "mousesystems",	MOUSE_MOUSESYSTEMS },
#endif
#ifdef MOUSE_MMSERIES
	{ "mmseries",		MOUSE_MMSERIES },
#endif
#ifdef MOUSE_LOGITECH
	{ "logitech",		MOUSE_LOGITECH },
#endif
#ifdef MOUSE_BUSMOUSE
	{ "busmouse",		MOUSE_BUSMOUSE },
#endif
#ifdef MOUSE_PS2
	{ "ps2",		MOUSE_PS2 },
#endif
#ifdef MOUSE_LOGIMAN
	{ "logiman",		MOUSE_LOGIMAN },
#endif
#ifdef MOUSE_GPM
	{ "gpm",		MOUSE_GPM },
#endif
#ifdef MOUSE_SPACEBALL
	{ "spaceball",		MOUSE_SPACEBALL },
#endif
#ifdef MOUSE_INTELLIMOUSE
	{ "intellimouse",	MOUSE_INTELLIMOUSE },
#endif
#ifdef MOUSE_IMPS2
	{ "imps2",		MOUSE_IMPS2 },
#endif
	{ NULL,			0 }
};

#endif
