// File: mod.c                   by Kester Maddock (dmaddock@xtra.co.nz)
// DOOM Music subsystems, 
// These routines provide MOD Music to DOOM.

// For struct defines and function prototypes
#include "i_music.h"
#include "mus_mod.h"

// For timer routines
#include "allegro.h"

// -----------  Local Variables  --------------

// handle of the song currently playing
extern JGMOD *of;

// -----------  Function Definitions -------------

// If specified music is playing returns 1; else 0
int MOD_QrySongPlaying(JGMOD *handle)
{
  // Song is playing.
  if (handle == of) return 1;  
  // Song is not playing
  return 0;
}

// Changes the music volume range 0 - 255
void MOD_SetMusicVolume(int volume)
{
  set_mod_volume(volume);
}

// like the || button on ya stereo
void MOD_PauseSong(JGMOD *handle)
{
   pause_mod();
}

// unpauses a paused song
void MOD_ResumeSong(JGMOD *handle)
{
   resume_mod();
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
JGMOD *MOD_RegisterSong(void *data)
{
  return load_mod(data);
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void MOD_PlaySong(JGMOD *handle, int looping)
{
  // Check handle is non NULL and not currently playing
  if (!handle || (handle == of)) return;
  
  // If a song is playing, stop it.
  if (of && (handle != of)) MOD_StopSong(of);
  
  play_mod(handle, looping);  
}

// like the # button on ya stereo
void MOD_StopSong(JGMOD *handle)
{
  if (!handle || (handle != of)) return;  
  stop_mod();
}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void MOD_UnRegisterSong(JGMOD *handle)
{
  if (!handle) return;  
  // We wouldn't want it to keep playing...
  MOD_StopSong(handle);
  destroy_mod(handle);
}

       
// Inits defaults, install MUSTicker etc...
// We can only use 16 channels, cause we have sfx too
void MOD_Init(void)
{
    install_mod(32);
}

void MOD_Shutdown(void)
{
    remove_mod();
}
