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
//
// DESCRIPTION:
//      System interface, sound.
//
//-----------------------------------------------------------------------------

#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomdef.h"
#include <allegro.h>

#include <stdio.h>

#include "doomstat.h"
#include "lu_sound.h"



// Init at program start...
void I_InitSound();

// ... update sound buffer and audio device at runtime...
void I_UpdateSound(void);
void I_SubmitSound(void);

// ... shut down and relase at program termination.
void I_ShutdownSound(void);


//
//  SFX I/O
//

SAMPLE *I_CacheSFX(int sfxnum);
void I_DeCacheSFX(int sfxnum);

// Get raw data lump index for sound descriptor.
int I_GetSfxLumpNum (sfxinfo_t* sfxinfo );


// Starts a sound in a particular sound channel.
int
I_StartSound
( int           id,
  int           vol,
  int           sep,
  int           pitch,
  int           priority );

// Stops a sound channel.
void I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
boolean I_SoundIsPlaying(int handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void
I_UpdateSoundParams
( int           handle,
  int           vol,
  int           sep,
  int           pitch );

void I_SetSfxVolume(int volume);

#include "i_music.h"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
