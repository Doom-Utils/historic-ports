//  
// DOSDoom Fixed Point Stuff
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
#ifndef __M_FIXED__
#define __M_FIXED__

#ifdef __GNUG__
#pragma interface
#endif

#include <values.h>

//
// Fixed point, 32bit as <-16bits . 16bits->
//
#define FRACBITS		16
#define FRACUNIT		(1<<FRACBITS)

typedef int fixed_t;

//fixed_t FixedMul	(fixed_t a, fixed_t b);
//#define FixedMul(a,b) ((((long long)(a))*(b))>>FRACBITS)
// -KM- 1998/12/16 Fixed point functions inlined here for more speed.
static inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
  return ((((long long)(a))*(b))>>FRACBITS);
}

//
// FixedDiv, C version.
//
static inline fixed_t FixedDiv2 (fixed_t a, fixed_t b)
{
  return (((long long)a<<16) / ((long long)b));
}

static inline fixed_t FixedDiv (fixed_t a, fixed_t b )
{
    if ( (abs(a)>>14) >= abs(b))
	return (a^b)<0 ? MININT : MAXINT;
    return FixedDiv2 (a,b);
}

#endif

