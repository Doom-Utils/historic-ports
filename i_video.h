//
// DOSDoom System Interface Video Code 
//
// Based on the DOOM Video Source Code
//
// Released by id Software (c) 1993-1996. (see DOOMLIC.TXT)
//
// -MH- 1998/07/02 Move I_DoomCode2ScanCode and I_ScanCode2DoomCode
//                 to i_system.c (and their relevant declarations to
//                 i_system.h. See i_system.c for details as to why.
//
#ifndef __I_VIDEO__
#define __I_VIDEO__

// For palette_color
#include <allegro.h>

#include "dm_type.h"

#ifdef __GNUG__
#pragma interface
#endif

void I_AutodetectBPP();
void I_GetResolution(void);

// Called by D_DoomMain,
// determines the hardware configuration
// and sets up the video mode
void I_InitGraphics (void);

void I_ShutdownGraphics(void);

// Takes full 8 bit values.
void I_SetPalette (byte* palette, int redness);

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

// Wait for vertical retrace or pause a bit.
void I_WaitVBL(int count);

void I_ReadScreen (byte* scr);

void I_BeginRead (void);
void I_EndRead (void);

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
