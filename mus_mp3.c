//
// DOSDoom MP3 Support Code
//
// By the DOSDoom Team (Mostly -KM-)
// 
// These routines provide MP3 Music to DOOM.
//

#include "mus_mp3.h"
#include "i_alleg.h"

#include "dm_type.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "z_zone.h"

#ifndef NOMP3
struct MP3_handle_t *MP3_playing = 0;
static int loop = 0;

extern int A_DOWNMIX;
#endif

void MP3_Init(void)
{
#ifndef NOMP3
  install_amp();
  amp_reverse_phase = get_config_int("sound", "amp_reverse_phase", 1);
  A_DOWNMIX = get_config_int("sound", "auto_downmix", 0);
#endif
}

// Returns true if MP3 Music is playing
boolean MP3_QrySongPlaying(struct MP3_handle_t *handle)
{
#ifndef NOMP3
  return amp_playing;
#else
  return 0;
#endif
}

// Changes the music volume range 0 - 255
void MP3_SetMusicVolume(int volume)
{
#ifndef NOMP3
  // Allegro Set Volume. sfx_volume = don't care
  amp_setvolume(volume);
#endif
}

// like the || button on ya stereo
void MP3_PauseSong(void)
{
#ifndef NOMP3
  amp_pause();
#endif
}

// unpauses a paused song
void MP3_ResumeSong(void)
{
#ifndef NOMP3
  amp_resume();
#endif
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
struct MP3_handle_t *MP3_RegisterSong(void *data)
{
#ifndef NOMP3
  char *tok;
  struct MP3_handle_t *rc;
  int i, num_mp3s;


  // Find the number of songs in the playlist, by cheating
  if (sscanf(data, "dosdoom_playlist %d", &num_mp3s) != 1)
    return NULL;

  // Get some mem
  rc = Z_Malloc(sizeof(struct MP3_handle_t) + sizeof(char *) * num_mp3s, PU_STATIC, NULL);
  rc->numplayers = num_mp3s;
  // Pre init values
  rc->playing = 0;
  // Skip header
  strtok(data, "\n");
  // Setup pointers to the file names
  for (i = 0; i < rc->numplayers; i++) {
    tok = strtok(NULL, "\n\r");
    rc->playlist[i] = tok;
  }

  // Check that all the MP3's exist...
  for (i = 0; i < rc->numplayers; i++)
     if (access(rc->playlist[i], R_OK)) {
       MP3_UnRegisterSong(rc);
       return NULL;
     }

  return rc;
#else
  return NULL;
#endif
}

void PlayNext(struct MP3_handle_t *handle)
{
#ifndef NOMP3
  if (!handle) return;
  if (handle->playing >= handle->numplayers) {
    if (loop) handle->playing = 0;
    else {
      MP3_playing = NULL;
      return;
    }
  }
  load_amp(handle->playlist[handle->playing], 0);
  handle->playing++;
  MP3_playing = handle;
#endif
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void MP3_PlaySong(struct MP3_handle_t *handle, int looping)
{
#ifndef NOMP3
  handle->playing = 0;
  PlayNext(handle);
  loop = looping;
#endif
}

// like the # button on ya stereo
void MP3_StopSong(struct MP3_handle_t *handle)
{
#ifndef NOMP3
  if (handle && (handle == MP3_playing)) {
    unload_amp();
    MP3_playing = NULL;
  }
#endif
}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void MP3_UnRegisterSong(struct MP3_handle_t *handle)
{
#ifndef NOMP3
  if (!handle) return;
  Z_Free(handle);
#endif
}

// Loads Data, checks to see if next mp3 in playlist should be played.
void MP3_Ticker(void)
{
#ifndef NOMP3
  if (amp_decode() < 1) PlayNext(MP3_playing);
#endif
}
