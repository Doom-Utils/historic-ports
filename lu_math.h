//
// DOSDoom Mathematics LookUp Tables
//
// Based on the DOOM Video Source Code
//
// Released by id Software (c) 1993-1996. (see DOOMLIC.TXT)
//
// In the order of appearance:
//
//	int finetangent[4096] - Tangens LUT.
//	  Should work with BAM fairly well (12 of 16bit, effectively, by shifting).
//
//	int finesine[10240] - Sine lookup.
//	  Guess what, serves as cosine, too. Remarkable thing is,
//        how to use BAMs with this?
//
//	int tantoangle[2049] - ArcTan LUT,
//	  maps tan(angle) to angle fast. Gotta search.	
//    

#ifndef __TABLES__
#define __TABLES__

#include "m_fixed.h"
#include "dm_type.h"
#define FINEANGLES 8192
#define FINEMASK (FINEANGLES-1)

// 0x100000000 to 0x2000
#define ANGLETOFINESHIFT 19

// Effective size is 10240.
extern fixed_t finesine[5*FINEANGLES/4];

// Re-use data, is just PI/2 phase shift.
extern fixed_t* finecosine;

// Effective size is 4096.
extern fixed_t finetangent[FINEANGLES/2];

// Binary Angle Measument, BAM.
#define ANG45  0x20000000
#define ANG90  0x40000000
#define ANG180 0x80000000
#define ANG270 0xc0000000

#define SLOPERANGE 2048
#define SLOPEBITS  11
#define DBITS      (FRACBITS-SLOPEBITS)

// Effective size is 2049;
// The +1 size is to handle the case when x==y without additional checking.
extern angle_t		tantoangle[SLOPERANGE+1];

// Utility function, called by R_PointToAngle.
int SlopeDiv (unsigned num, unsigned den);

#endif

