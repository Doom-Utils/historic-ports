//
// DOSDoom MOD Support Code
//
// By the DOSDoom Team (Mostly -KM-)
// 
// These routines provide MOD Music to DOOM.
//

// For struct defines and function prototypes
// For timer routines
#include <allegro.h>

#include "i_music.h"
#include "mus_mod.h"

// -----------  Local Variables  --------------

// handle of the song currently playing
#ifndef NOMOD
extern JGMOD *of;
#endif
// -----------  Function Definitions -------------

// If specified music is playing returns 1; else 0
boolean MOD_QrySongPlaying(JGMOD *handle)
{
#ifndef NOMOD
  // Song is playing.
  if (handle == of) return true;
#endif
  // Song is not playing
  return false;
}

// Changes the music volume range 0 - 255
void MOD_SetMusicVolume(int volume)
{
#ifndef NOMOD
  set_mod_volume(volume);
#endif
}

// like the || button on ya stereo
void MOD_PauseSong(JGMOD *handle)
{
#ifndef NOMOD
   pause_mod();
#endif
}

// unpauses a paused song
void MOD_ResumeSong(JGMOD *handle)
{
#ifndef NOMOD
   resume_mod();
#endif
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
JGMOD *MOD_RegisterSong(void *data)
{
#ifndef NOMOD
  return load_mod(data);
#else
  return NULL;
#endif
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void MOD_PlaySong(JGMOD *handle, int looping)
{
#ifndef NOMOD
  // Check handle is non NULL and not currently playing
  if (!handle || (handle == of)) return;
  
  // If a song is playing, stop it.
  if (of && (handle != of)) MOD_StopSong(of);
  
  play_mod(handle, looping);
#endif
}

// like the # button on ya stereo
void MOD_StopSong(JGMOD *handle)
{
#ifndef NOMOD
  if (!handle || (handle != of)) return;  
  stop_mod();
#endif
}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void MOD_UnRegisterSong(JGMOD *handle)
{
#ifndef NOMOD
  if (!handle) return;  
  // We wouldn't want it to keep playing...
  MOD_StopSong(handle);
  destroy_mod(handle);
#endif
}

       
// Inits defaults, install MUSTicker etc...
// We can only use 16 channels, cause we have sfx too
void MOD_Init(void)
{
#ifndef NOMOD
    install_mod(32);
#endif
}

void MOD_Shutdown(void)
{
#ifndef NOMOD
    remove_mod();
#endif
}
