// Music player code, by Kester Maddock, based on Docs by Vladimir Arnost
#ifndef __I_MUSIC__
#define __I_MUSIC__

#include "doomtype.h"

#define PAUSE 1
#define PLAY 2
#define LOOP 4

// Type of music file playing:
typedef enum {
 M_NONE = -1,
 M_MUS,
 M_MIDI,
 M_MP3,
 M_MOD,
 M_CDAUDIO
} SongTypes_t;

// Initialise the Music player
void I_InitMusic(void);

// Shutdown the Music player
void I_ShutdownMusic(void);

// Sets the volume for the music (midi) player
void I_SetMusicVolume(int volume);
// Sets the volume for the cd player
void I_SetCDMusicVolume(int volume);

// Pauses the specified song
void I_PauseSong(int handle);

// Resumes the specified song (after being paused by I_PauseSong)
void I_ResumeSong(int handle);

// Initialises the song to a ready to be played state.
// Returns the handle used by all the other functions
// data points to the MUS file which must be loaded into memory raw
int I_RegisterSong(void *data);

// Gee, I wonder... if looping is nonzero, song will play forever...
void I_PlaySong(int handle, int looping);
// Indicates the sync.  Song can start now...
void I_SongSync(void);

// Stops a song playing forever
void I_StopSong(int handle);

// Frees the resources used by a song
void I_UnRegisterSong(int handle);

// Called every once in a while on interrupt to send data to the music
// device
void MusicTicker(void);
// Called every once in a while not on interrupt to send data to the music
// device.  Not called on interrupt so that data can be streamed from disk.
// Mainly for mp3 support
void I_MusicTicker2(void);


// If the specified song is playing returns 1; if not playing returns 0
int I_QrySongPlaying(int handle);

void I_SongSync(void);
void CD_Next(void);
void CD_Prev(void);
void CD_Play(int track, boolean looping);
#endif

