// Music player code, by Kester Maddock, based on Docs by Vladimir Arnost
#ifndef __I_MUSIC__
#define __I_MUSIC__

#define PAUSE 1
#define PLAY 2
#define LOOP 4

// Initialise the Music player
void I_InitMusic(void);

// Shutdown the Music player
void I_ShutdownMusic(void);

// Sets the volume for the music player
void I_SetMusicVolume(int volume);

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

// Stops a song playing forever
void I_StopSong(int handle);

// Frees the resources used by a song
void I_UnRegisterSong(int handle);

// Called every once in a while on interrupt to send data to the music
// device
void MusicTicker(void);

// If the specified song is playing returns 1; if not playing returns 0
int I_QrySongPlaying(int handle);

#endif

