//  
// DOSDoom Pseudo-Random Number Code (via LUT)
//
// Based on the Doom Source code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//

#ifndef __M_RANDOM__
#define __M_RANDOM__


#include "dm_type.h"



// Returns a number from 0 to 255,
// from a lookup table.
int M_Random (void);

// As M_Random, but used only by the play simulation.
int P_Random (void);
int J_Random (void);

// Fix randoms for demos.
void M_ClearRandom (void);


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
