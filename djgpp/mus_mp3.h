// MP3 support through dosamp

#ifndef __MUSIC_MP3
#define __MUSIC_MP3

#include "libamp/libamp.h"

struct MP3_handle_t {
  int numplayers;
  int playing;
  char *playlist[0];
};

void MP3_Init(void);

// Returns true if MP3 Music is playing
int MP3_QrySongPlaying(struct MP3_handle_t *handle);

// Changes the music volume range 0 - 15
void MP3_SetMusicVolume(int volume);

// like the || button on ya stereo
void MP3_PauseSong(void);

// unpauses a paused song
void MP3_ResumeSong(void);

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
struct MP3_handle_t *MP3_RegisterSong(void *data);

// like the > button on ya stereo
// if looping is nonzero, will play forever
void MP3_PlaySong(struct MP3_handle_t *handle, int looping);

// like the # button on ya stereo
void MP3_StopSong(struct MP3_handle_t *handle);

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void MP3_UnRegisterSong(struct MP3_handle_t *handle);

// Loads Data, checks to see if next mp3 in playlist should be played.
void MP3_Ticker(void);

#endif