// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_swap.c,v 1.2 1997/12/29 19:51:04 pekangas Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log: m_swap.c,v $
// Revision 1.2  1997/12/29 19:51:04  pekangas
// Ported to WinNT/95 environment using Watcom C 10.6.
// Everything expect joystick support is implemented, but networking is
// untested. Crashes way too often with problems in FixedDiv().
// Compiles with no warnings at warning level 3, uses MIDAS 1.1.1.
//
// Revision 1.1.1.1  1997/12/28 12:59:08  pekangas
// Initial DOOM source release from id Software
//
//
// DESCRIPTION:
//	Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------

const char
m_swap_rcsid[] = "$Id: m_swap.c,v 1.2 1997/12/29 19:51:04 pekangas Exp $";


#ifdef __GNUG__
#pragma implementation "m_swap.h"
#endif
#include "m_swap.h"


// Not needed with big endian.
#ifndef __BIG_ENDIAN__

// Swap 16bit, that is, MSB and LSB byte.
unsigned short SwapSHORT(unsigned short x)
{
    // No masking with 0xFF should be necessary. 
    return (x>>8) | (x<<8);
}

// Swapping 32bit.
unsigned long SwapLONG( unsigned long x)
{
    return
	(x>>24)
	| ((x>>8) & 0xff00)
	| ((x<<8) & 0xff0000)
	| (x<<24);
}


#endif


