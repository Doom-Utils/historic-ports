// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_swap.h,v 1.3 1998/05/03 23:14:03 killough Exp $
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
//      Endianess handling, swapping 16bit and 32bit.
//
//-----------------------------------------------------------------------------


#ifndef __M_SWAP__
#define __M_SWAP__

#ifdef __GNUG__
#pragma interface
#endif

// Endianess handling.
// WAD files are stored little endian.
//
// killough 5/1/98:
// Replaced old code with inlined code which works regardless of endianess.
//

// Swap 16bit, that is, MSB and LSB byte.

#ifdef __GNUC__
__inline__
#endif
static short SHORT(short x)
{
  return (((unsigned char *) &x)[1]<< 8) +
          ((unsigned char *) &x)[0];
}

// Swapping 32bit.

#ifdef __GNUC__
__inline__
#endif
static long LONG(long x)
{
  return (((unsigned char *) &x)[3]<<24) +
         (((unsigned char *) &x)[2]<<16) +
         (((unsigned char *) &x)[1]<< 8) +
          ((unsigned char *) &x)[0];
} 

#endif

//----------------------------------------------------------------------------
//
// $Log: m_swap.h,v $
// Revision 1.3  1998/05/03  23:14:03  killough
// Make endian independent, beautify
//
// Revision 1.2  1998/01/26  19:27:15  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:08  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
