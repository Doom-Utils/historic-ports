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
//	Cheat code checking.
//
//-----------------------------------------------------------------------------


#ifndef __M_CHEAT__
#define __M_CHEAT__

//
// CHEAT SEQUENCE PACKAGE
//
/*
#define SCRAMBLE(a) \
((((a)&1)<<7) + (((a)&2)<<5) + ((a)&4) + (((a)&8)<<1) \
 + (((a)&16)>>1) + ((a)&32) + (((a)&64)>>5) + (((a)&128)>>7))
*/
#define SCRAMBLE(a) (a)

typedef struct
{
    unsigned char*	sequence;
    unsigned char*	p;
    
} cheatseq_t;

extern cheatseq_t	cheat_mus;
extern cheatseq_t	cheat_god;
extern cheatseq_t	cheat_ammo;
extern cheatseq_t	cheat_ammonokey;
extern cheatseq_t	cheat_noclip;
extern cheatseq_t	cheat_commercial_noclip;

extern cheatseq_t	cheat_powerup[7];

extern cheatseq_t	cheat_choppers;
extern cheatseq_t	cheat_clev;
extern cheatseq_t	cheat_mypos;
extern cheatseq_t       cheat_amap;

//new cheats
extern cheatseq_t	cheat_cdnext;
extern cheatseq_t	cheat_cdprev;
extern cheatseq_t	cheat_killall;

int
cht_CheckCheat
( cheatseq_t*		cht,
  char			key );


void
cht_GetParam
( cheatseq_t*		cht,
  char*			buffer );


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
