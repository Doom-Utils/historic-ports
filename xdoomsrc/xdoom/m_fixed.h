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
//	Fixed point arithemtics, implementation.
//
//-----------------------------------------------------------------------------

#ifndef __M_FIXED__
#define __M_FIXED__

#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

//
// Fixed point, 32bit as 16.16.
//
#define FRACBITS		16
#define FRACUNIT		(1 << FRACBITS)

typedef int fixed_t;

//
// Assembler inline functions for x86 + GNU C
//
#if defined(__GNUC__) && defined(i386) && defined(USE_INLINE)

// use cdq instruction for a fast abs() inline assembler version
// SCO as doesn't understand the cdq instruction, so we use .byte 0x99
#define abs(x) \
	({int _s,_t=(x); asm(".byte 0x99" : "=d" (_s) : "a" (_t)); (_t^_s)-_s;})

static inline fixed_t FixedMul(fixed_t _a, fixed_t _b) {
	fixed_t _result;
	asm ("
	    imull %2
	    shrdl  $16, %%edx, %0" :
	    "=a,=a" (_result) :
	    "0,0" (_a),
	    "m,r" (_b) :
	    "%edx", "%cc");
	return _result;
}

// again, use .byte 0x99 for cdq for SCO assembler
static inline fixed_t FixedDiv2(fixed_t _a, fixed_t _b) {
	fixed_t _result;
	asm ("
	    .byte 0x99
	    shldl $16, %%eax, %%edx
	    sall  $16, %%eax
	    idivl %2" :
	    "=a,=a" (_result) :
	    "0,0" (_a),
	    "m,r" (_b) :
	    "%edx", "%cc");
	return _result;
}

static inline fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
	return (a ^ b) < 0 ? MININT : MAXINT;
    return FixedDiv2(a, b);
}

//
// Assembler inline functions for x86 + SCO C
//
#elif (defined(SCOUW7) || defined(SCOOS5)) && defined(USE_INLINE)

asm fixed_t FixedMul(fixed_t a, fixed_t b)
{
%	mem a, b;
	push  %edx
	movl  a, %eax
	movl  b, %edx
	imull %edx
	shrdl $16, %edx, %eax
	pop   %edx
}

asm fixed_t FixedDiv2(fixed_t a, fixed_t b)
{
%	mem a, b;
	push  %edx
	movl  a, %eax
	.byte 0x99	// cdq, not implemented in SCO as
	shldl $16, %eax, %edx
	sall  $16, %eax
	idivl b
	pop   %edx
}

fixed_t FixedDiv(fixed_t a, fixed_t b);

#else

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);
fixed_t FixedDiv2(fixed_t a, fixed_t b);

#endif

#endif
