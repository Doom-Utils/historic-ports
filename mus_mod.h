// Music player code, by Kester Maddock.
#ifndef __MOD_MUSIC__
#define __MOD_MUSIC__

#ifndef NOMOD
#include "mus_lmod.h"
#else
#define JGMOD void
#endif
#include "dm_type.h"

// -----------    Local Functions    --------------

// Initialise the Music player
void MOD_Init(void);

// Sets the volume for the music player
void MOD_SetMusicVolume(int volume);

// Pauses the specified song
void MOD_PauseSong(JGMOD *handle);

// Resumes the specified song (after being paused by I_PauseSong)
void MOD_ResumeSong(JGMOD *handle);

// Initialises the song to a ready to be played state.
// Returns the handle used by all the other functions
// data points to the MUS file which must be loaded into memory raw
JGMOD *MOD_RegisterSong(void *data);

// Gee, I wonder... if looping is nonzero, song will play forever...
void MOD_PlaySong(JGMOD *handle, int looping);

int MOD_HandleEvent(JGMOD *handle);

// Stops a song playing forever
void MOD_StopSong(JGMOD *handle);

// Frees the resources used by a song
void MOD_UnRegisterSong(JGMOD *handle);

// If the specified song is playing returns 1; if not playing returns 0
boolean MOD_QrySongPlaying(JGMOD *handle);

#endif

