// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
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
//	Fixed point implementation.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>

#include "doomtype.h"
#include "i_system.h"

#ifdef __GNUG__
#pragma implementation "m_fixed.h"
#endif
#include "m_fixed.h"

#ifndef USE_INLINE
fixed_t FixedMul(fixed_t a, fixed_t b)
{
#if 1
    return (fixed_t)(((long long) a * (long long) b) >> FRACBITS);
#else
    // doesn't work
    return (fixed_t)(((double)a) * (((double)b)) / FRACUNIT);
#endif
}
#endif

//
// FixedDiv, C version.
//
#if !defined(USE_INLINE) || !defined(__GNUC__)
fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
	return (a ^ b) < 0 ? MININT : MAXINT;
    return FixedDiv2(a, b);
}
#endif

#ifndef USE_INLINE
fixed_t FixedDiv2(fixed_t a, fixed_t b)
{
    // using doubles on RISC CPU's can be faster than using 64bit
    // integers. then some compilers don't support long long,
    // go figure your self which is faster and what your compiler
    // supports
#if 1
    return (fixed_t)(((long long)a << 16) / ((long long)b));
#else
    double c;
    c = ((double)a) / ((double)b) * FRACUNIT;
#ifdef RANGECHECK
    if (c >= 2147483648.0 || c < -2147483648.0)
	I_Error("FixedDiv: divide by zero");
#endif

    return (fixed_t) c;
#endif
}

#endif
