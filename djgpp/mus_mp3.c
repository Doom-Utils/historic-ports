// MP3 Code

#include "mus_mp3.h"
#include <allegro.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "z_zone.h"

struct MP3_handle_t *MP3_playing = 0;
static int loop = 0;

extern int A_DOWNMIX;
void MP3_Init(void)
{
  install_amp();
  amp_reverse_phase = get_config_int("sound", "amp_reverse_phase", 1);
  A_DOWNMIX = get_config_int("sound", "auto_downmix", 0);
}

// Returns true if MP3 Music is playing
int MP3_QrySongPlaying(struct MP3_handle_t *handle)
{
  return amp_playing;
}

// Changes the music volume range 0 - 255
void MP3_SetMusicVolume(int volume)
{
  // Allegro Set Volume. sfx_volume = don't care
  amp_setvolume(volume);
}

// like the || button on ya stereo
void MP3_PauseSong(void)
{
  amp_pause();
}

// unpauses a paused song
void MP3_ResumeSong(void)
{
  amp_resume();
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
struct MP3_handle_t *MP3_RegisterSong(void *data)
{
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
}

void PlayNext(struct MP3_handle_t *handle)
{
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
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void MP3_PlaySong(struct MP3_handle_t *handle, int looping)
{
  handle->playing = 0;
  PlayNext(handle);
  loop = looping;
}

// like the # button on ya stereo
void MP3_StopSong(struct MP3_handle_t *handle)
{
  if (handle && (handle == MP3_playing)) {
    unload_amp();
    MP3_playing = NULL;
  }

}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void MP3_UnRegisterSong(struct MP3_handle_t *handle)
{
  if (!handle) return;
  Z_Free(handle);
}

// Loads Data, checks to see if next mp3 in playlist should be played.
void MP3_Ticker(void)
{
  if (amp_decode() < 1) PlayNext(MP3_playing);
}
