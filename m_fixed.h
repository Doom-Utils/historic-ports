// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: m_fixed.h,v 1.5 1998/05/10 23:42:22 killough Exp $
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
//      Fixed point arithemtics, implementation.
//
//-----------------------------------------------------------------------------

#ifndef __M_FIXED__
#define __M_FIXED__

#ifdef __GNUG__
#pragma interface
#endif

#ifndef __GNUC__
#define __inline__
#define __attribute__(x)
#endif

#include "i_system.h"

//
// Fixed point, 32bit as 16.16.
//

#define FRACBITS 16
#define FRACUNIT (1<<FRACBITS)

typedef int fixed_t;

//
// Absolute Value
//

// killough 5/10/98: In djgpp, use inlined assembly for performance
// Use x86 cdq instruction to generate fast abs(), avoiding branches.

#ifdef DJGPP
#define abs(x) ({int _s,_t=(x); asm(" cdq": "=d" (_s): "a" (_t)); (_t^_s)-_s;})
#endif // DJGPP

//
// Fixed Point Multiplication
//

#ifdef DJGPP

// killough 5/10/98: In djgpp, use inlined assembly for performance

__inline__ static fixed_t FixedMul(fixed_t a, fixed_t b)
{
  fixed_t result;

  asm("  imull %2 ;"
      "  shrdl $16,%%edx,%0 ;"
      : "=a,=a" (result)           // eax is always the result
      : "0,0" (a),                 // eax is also first operand
        "m,r" (b)                  // second operand can be mem or reg
      : "%edx", "%cc"              // edx and condition codes clobbered
      );

  return result;
}

#else // DJGPP

__inline__ static fixed_t FixedMul(fixed_t a, fixed_t b)
{
  return (fixed_t)((long long) a*b >> FRACBITS);
}

#endif // DJGPP

//
// Fixed Point Division
//

#ifdef DJGPP

// killough 5/10/98: In djgpp, use inlined assembly for performance

__inline__ static fixed_t FixedDiv(fixed_t a, fixed_t b)
{
  fixed_t result;

  if (abs(a) >> 14 >= abs(b))
    return (a^b)<0 ? MININT : MAXINT;

  asm(" movl %0, %%edx ;"
      " sall $16,%%eax ;"
      " sarl $16,%%edx ;"
      " idivl %2 ;"
      : "=a,=a" (result)    // eax is always the result
      : "0,0" (a),          // eax is also the first operand
        "m,r" (b)           // second operand can be mem or reg (not imm)
      : "%edx", "%cc"       // edx and condition codes are clobbered
      );

  return result;
}

#else // DJGPP

__inline__ static fixed_t FixedDiv(fixed_t a, fixed_t b)
{
  return (abs(a)>>14) >= abs(b) ? (a^b)<0 ? MININT : MAXINT :
    (fixed_t)(((long long) a << FRACBITS) / b);
}

#endif // DJGPP

#endif

//----------------------------------------------------------------------------
//
// $Log: m_fixed.h,v $
// Revision 1.5  1998/05/10  23:42:22  killough
// Add inline assembly for djgpp (x86) target
//
// Revision 1.4  1998/04/27  01:53:37  killough
// Make gcc extensions #ifdef'ed
//
// Revision 1.3  1998/02/02  13:30:35  killough
// move fixed point arith funcs to m_fixed.h
//
// Revision 1.2  1998/01/26  19:27:09  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:53  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------

