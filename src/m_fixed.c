// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_fixed.c,v 1.3 1997/12/29 21:17:11 pekangas Exp $
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
// $Log: m_fixed.c,v $
// Revision 1.3  1997/12/29 21:17:11  pekangas
// Fixed to compile with Visual C
// Some Win95 fixes
//
// Revision 1.2  1997/12/29 19:51:01  pekangas
// Ported to WinNT/95 environment using Watcom C 10.6.
// Everything expect joystick support is implemented, but networking is
// untested. Crashes way too often with problems in FixedDiv().
// Compiles with no warnings at warning level 3, uses MIDAS 1.1.1.
//
// Revision 1.1.1.1  1997/12/28 12:59:06  pekangas
// Initial DOOM source release from id Software
//
//
// DESCRIPTION:
//	Fixed point implementation.
//
//-----------------------------------------------------------------------------


const char
m_fixed_rcsid[] = "$Id: m_fixed.c,v 1.3 1997/12/29 21:17:11 pekangas Exp $";

#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#ifdef __GNUG__
#pragma implementation "m_fixed.h"
#endif
#include "m_fixed.h"

/* [Petteri] */
#include <assert.h>   


// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
#if defined(__WATCOMC__) || defined(_MSC_VER)
    /* [Petteri] FIXME: inline assembler #pragma would be a better idea: */
    double da = ((double) a) * (1.0 / ((double) FRACUNIT));
    double db = ((double) b) * (1.0 / ((double) FRACUNIT));
    return (fixed_t) (da * db * ((double) FRACUNIT));
#else
    return ((long long) a * (long long) b) >> FRACBITS;
#endif    
}



//
// FixedDiv, C version.
//

fixed_t
FixedDiv
( fixed_t	a,
  fixed_t	b )
{
    if ( (abs(a)>>14) >= abs(b))
	return (a^b)<0 ? MININT : MAXINT;
    return FixedDiv2 (a,b);
}



fixed_t
FixedDiv2
( fixed_t	a,
  fixed_t	b )
{
#if 0
    long long c;
    c = ((long long)a<<16) / ((long long)b);
    return (fixed_t) c;
#endif

    double c;

    c = ((double)a) / ((double)b) * FRACUNIT;

    assert ( c < 2147483648.0 && c >= -2147483648.0 );
    
    if (c >= 2147483648.0 || c < -2147483648.0)
	I_Error("FixedDiv: divide by zero");
    return (fixed_t) c;
}
