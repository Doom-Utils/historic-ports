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
//	System interface, sound.
//
//-----------------------------------------------------------------------------

#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomdef.h"

// UNIX hack, to be removed.
#ifdef SNDSERV
#include <stdio.h>
extern FILE* sndserver;
extern char* sndserver_filename;
#endif

//#include "doomstat.h"
#include "sounds.h"



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

// Get raw data lump index for sound descriptor.
int I_GetSfxLumpNum (sfxinfo_t* sfxinfo );


// Starts a sound in a particular sound channel.
SAMPLE*
I_StartSound
( int		id,
  int		vol,
  int		sep,
  int		pitch,
  int		priority );


// Stops a sound channel.
void I_StopSound(SAMPLE* handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int I_SoundIsPlaying(SAMPLE* handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void
I_UpdateSoundParams
( SAMPLE* handle,
  int		vol,
  int		sep,
  int		pitch );

void I_SetSfxVolume(int volume);

//check cd player - if curr track is done, go to next track
void I_CheckCD();
#include "i_music.h"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
