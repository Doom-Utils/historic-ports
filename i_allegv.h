//
// DOSDoom Allegro Video Code
//
// By the DOSDoom Team 
//
#ifndef __allegvid_h__
#define __allegvid_h__

#include <allegro.h>
#include "dm_type.h"

extern void (*flipscreens) (void);
extern boolean doublebufferflag;

boolean enter_graphics_mode();

extern boolean forcevga;

#endif

