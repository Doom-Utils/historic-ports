// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
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
// DESCRIPTION:
//	Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------


#ifndef __M_SWAP__
#define __M_SWAP__


#ifdef __GNUG__
#pragma interface
#endif


// Endianess handling.
// WAD files are stored little endian.

// Not needed with little endian.
#ifdef __BIG_ENDIAN__
// Swap 16bit, that is, MSB and LSB byte.
static inline unsigned short SHORT(unsigned short x)
{
    // No masking with 0xFF should be necessary.
    return (x>>8) | (x<<8);
}

// Swapping 32bit.
static inline unsigned long LONG( unsigned long x)
{
    return
	(x>>24)
	| ((x>>8) & 0xff00)
	| ((x<<8) & 0xff0000)
	| (x<<24);
}
#else
#define SHORT(x)	(x)
#define LONG(x)         (x)
#endif




#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
