//  
// DOSDoom Bounding-box Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#ifndef __M_BBOX__
#define __M_BBOX__

#include <values.h>

#include "m_fixed.h"


// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};	// bbox coordinates

// Bounding box functions.
void M_ClearBox (fixed_t*	box);

void
M_AddToBox
( fixed_t*	box,
  fixed_t	x,
  fixed_t	y );


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
