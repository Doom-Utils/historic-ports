//
// DOSDoom Sound FX Handling Code
//
// Based on the Doom Source Code,
//
// Released by Id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//

#ifndef __S_SOUND__
#define __S_SOUND__


#ifdef __GNUG__
#pragma interface
#endif

#include "p_mobj.h"

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void
S_Init
( int		sfxVolume,
  int		musicVolume );




//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);


//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//  Returns channel playing on, for stopping
// Sounds in the finale
int
S_StartSound
( mobj_t*		origin,
  sfx_t*		sound_id );

#ifdef DJGPP
void S_DoSound(void);
#endif

// Will start a sound at a given volume.
int
S_StartSoundAtVolume
( mobj_t*		origin,
  int		sound_id,
  int		volume );


// Stop sound for thing at <origin> and Channel num
void S_StopSound(mobj_t* origin);
void S_StopChannel(int cnum);


// Start music using <music_id> from sounds.h
//void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h,
//  and set whether looping
//void S_ChangeMusic(int music_id, int looping);

void S_ChangeMusic(char *name, int looping);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);


//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t* listener);

void S_SetMusicVolume(int volume);
void S_SetCDMusicVolume(int volume);
void S_SetSfxVolume(int volume);


extern int numChannels;

#define S_CLIPPING_DIST         1600
#define S_CLOSE_DIST            160

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
