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
//	Lookup tables.
//	Do not try to look them up :-).
//	In the order of appearance:
//
//	int finetangent[4096]	- Tangens LUT.
//	 Should work with BAM fairly well (12 of 16bit,
//      effectively, by shifting).
//
//	int finesine[10240]		- Sine lookup.
//	 Guess what, serves as cosine, too.
//	 Remarkable thing is, how to use BAMs with this?
//
//	int tantoangle[2049]	- ArcTan LUT,
//	  maps tan(angle) to angle fast. Gotta search.
//
//-----------------------------------------------------------------------------

#ifndef __TABLES__
#define __TABLES__

#include <math.h>

#ifndef PI
#ifdef M_PI
#define PI			M_PI
#else
#define PI			3.14159265358979323846
#endif
#endif

#include "m_fixed.h"

#define FINEANGLES		8192
#define FINEMASK		(FINEANGLES - 1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT	19

// Effective size is 10240.
extern  fixed_t		finesine[5 * FINEANGLES / 4];

// Re-use data, is just PI/2 pahse shift.
extern  fixed_t		*finecosine;

// Effective size is 4096.
extern fixed_t		finetangent[FINEANGLES / 2];

// Binary Angle Measument, BAM.
#define ANG45			0x20000000
#define ANG90			0x40000000
#define ANG180			0x80000000
#define ANG270			0xc0000000

#define SLOPERANGE		2048
#define SLOPEBITS		11
#define DBITS			(FRACBITS - SLOPEBITS)

typedef unsigned angle_t;

// Effective size is 2049;
// The +1 size is to handle the case when x==y
//  without additional checking.
extern angle_t		tantoangle[SLOPERANGE + 1];

// Utility function,
//  called by R_PointToAngle.
int SlopeDiv(unsigned num, unsigned den);

#endif
