//
// DOSDoom Sound Interface Code 
//
// Based on the Doom Source Code
//
// Released by by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <math.h>

#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

//im using allegro sound code
#include <allegro.h>

#include "dm_defs.h"
#include "d_debug.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

//allegro has 256 virtual voices
#define VIRTUAL_VOICES 256
#define NUM_CHANNELS numChannels
#define SAMPLECOUNT 512

static int* channelhandles;
static int* channelstart;

extern boolean swapstereo;
extern int numChannels;

typedef struct doomsfx_s {
        unsigned short flags __attribute__((packed));
        unsigned short samplerate __attribute__((packed));
        unsigned long len __attribute__((packed));
        unsigned char  data[0] __attribute__((packed));
    } doomsfx_t;

//this function converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses
//it is need cuz allegro only loads samples from wavs and vocs
static inline SAMPLE *raw2SAMPLE(
  void *rawdata,            // Array of sample data
  int len,                  // Length, in samples
  unsigned short freq,      // Frequency: DOOM Default is 11025 (measured in Hz)
  int bits,                 // Bits per sample: DOOM Default is 8
  boolean stereo )          // if true, this sample has a stereo channel.  Can be used for cool fx
{
  SAMPLE *spl;

  spl=(SAMPLE *) ((byte *) rawdata + len * (bits / 8) * (stereo ? 2 : 1));
  spl->bits = bits;
  spl->freq = freq;
  spl->stereo = stereo;
  spl->len = len;
  spl->priority = 255;
  spl->loop_start = 0;
  spl->loop_end = len;
  spl->param = -1;
  spl->data = rawdata;

  return spl;
}

#ifdef DJGPP
void _unlock_dpmi_data(void* p, size_t len);
#endif

inline void unlock_sample(SAMPLE *spl)
{
         _unlock_dpmi_data(spl->data, spl->len * spl->bits/8 * ((spl->stereo) ? 2 : 1));
         _unlock_dpmi_data(spl, sizeof(SAMPLE));
}

#define CACHEBIT 0x8000
#define LOCKBIT  0x4000

//
// This function loads the sound data from the WAD lump,
//  for single sound.
// This now returns a SAMPLE*, not a void*
//
SAMPLE*
I_CacheSFX
( int           sfxnum )
{
    int                 bits;
    boolean             stereo;
    SAMPLE              *spl;
    doomsfx_t           *sfx;

    // Find the lump num
    if (S_sfx[sfxnum].lumpnum < 0)
      S_sfx[sfxnum].lumpnum = I_GetSfxLumpNum(&S_sfx[sfxnum]);

    // Cache the sound data
    sfx = (doomsfx_t *)W_CacheLumpNum( S_sfx[sfxnum].lumpnum, PU_SOUND );

    // Set the bits and stereo
    bits = S_sfx[sfxnum].bits;
    stereo = S_sfx[sfxnum].stereo;
    Debug_Printf("%.8s, %d bits%s@ %d Hz, %ld samples flags: %d\n",
                 S_sfx[sfxnum].name, bits, stereo ? " stereo " : " mono ", sfx->samplerate, sfx->len, sfx->flags);

    // Do a special check: some of Ultimate Doom's sounds are corrupted slightly
    if ((sfx->len * (bits / 8) * (stereo ? 2 : 1) + sizeof(doomsfx_t)) > W_LumpLength(S_sfx[sfxnum].lumpnum))
    {
      I_Printf("Warning! Sample '%.8s' has bad length! (%ld, should be %ld)\n", S_sfx[sfxnum].name, sfx->len,
               (W_LumpLength(S_sfx[sfxnum].lumpnum) - sizeof(doomsfx_t)) / (bits / 8) / (stereo ? 2 : 1));
      sfx->len = (W_LumpLength(S_sfx[sfxnum].lumpnum) - sizeof(doomsfx_t)) / (bits / 8) / (stereo ? 2 : 1);
    }

    // If the sound was cached from mem, CACHEBIT will still be set.
    // If the sound was reloaded from disk, CACHEBIT will be clear and we
    //  allocate space for the allegro SAMPLE header
    if (!(sfx->flags & CACHEBIT)) {
      sfx = Z_ReMalloc(sfx, sfx->len * (bits / 8) * (stereo ? 2 : 1) + sizeof(doomsfx_t) + sizeof(SAMPLE));
      sfx->flags |= CACHEBIT; // Mark as in use.
    }
    // Fills in the SAMPLE header
    spl = raw2SAMPLE(sfx->data, sfx->len, sfx->samplerate, bits, stereo);

    // Sound data is used on interrupt, so must be locked in physical memory.
    // Use LOCKBIT because only want one lock on it.
    if (!(sfx->flags & LOCKBIT)) {
      lock_sample(spl);
      sfx->flags |= LOCKBIT;
    }
    return spl;
}

// -KM- 1998/12/16 I_DeCacheSFX
//  This func is called to to unlock the sample data so it can
//  be released from mem.  If it is needed before it is released,
//  the cache routine will lock the version in memory.
void
I_DeCacheSFX
( int sfxnum )
{
  doomsfx_t* sfx;

  Debug_Printf("SFX Number %d '%s' decached from lump %d.\n", sfxnum, S_sfx[sfxnum].name, S_sfx[sfxnum].lumpnum);

  // Fill in the lump number
  if (S_sfx[sfxnum].lumpnum < 0)
    S_sfx[sfxnum].lumpnum = I_GetSfxLumpNum(&S_sfx[sfxnum]);
  // Set data from PU_SOUND to PU_CACHE
  sfx = (doomsfx_t *)W_CacheLumpNum( S_sfx[sfxnum].lumpnum, PU_CACHE );

  // Unlock the sample in memory so if it is purged a hunk of locked memory
  // is not left.
  if (sfx->flags & LOCKBIT) {
    unlock_sample(S_sfx[sfxnum].data);
    sfx->flags &= ~LOCKBIT;
  }
}

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
static inline int
addsfx
( int           sfxid,
  int           volume,
  int           step,
  int           seperation,
  boolean       looping)
{


    int         i;
    int         rc = -1;
    
    int         oldest = gametic;
    int         oldestnum = 0;
    int         slot;


    I_UpdateSound();

    // Loop all channels to find oldest SFX.
    for (i=0; (i<NUM_CHANNELS) && (channelhandles[i] >= 0); i++)
    {
      if (channelstart[i] < oldest)
      {
             oldestnum = i;
             oldest = channelstart[i];
      }
    }

    // If we found a channel, fine.
    // If not, we simply overwrite the oldest one.
    // Note the oldest one used to mean the sound playing for
    //  longest.  Now it is the volume, so the quietest sound
    //  will be replaced.  Probably won't be missed.
    slot = (i == NUM_CHANNELS)? oldestnum : i;

    // If the sound is still playing it must be stopped
    if ((channelhandles[slot] >= 0) && I_SoundIsPlaying(channelhandles[slot]))
      I_StopSound(channelhandles[slot]);

    // Record the volume
    channelstart[slot] = volume;

    // Start the sample
    rc = play_sample(S_sfx[sfxid].data, volume, seperation, step, looping);

    // Assign current handle number.
    // Check handle is valid (ie sound could be started)
    if (rc < 0) return rc;

    // Generate a unique handle and save it.
    // Handle: | 16 bits: sfxid | 16 bits: allegro handle |
    rc += sfxid << 16;
    channelhandles[slot] = rc;
    
    // Return the handle
    return rc;
}


//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
// -KM- 1999/02/04 Allocate channels.
void I_SetChannels(numchannels)
{
  channelhandles = Z_Malloc(sizeof(int) * numchannels, PU_STATIC, NULL);
  channelstart = Z_Malloc(sizeof(int) * numchannels, PU_STATIC, NULL);
  memset(channelhandles, -1, sizeof(int) * numchannels);
  memset(channelstart, 0, sizeof(int) * numchannels);
}       

 
void I_SetSfxVolume(int volume)
{
  // Volume is handled in S_Sound.c
}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//
// -KM- 1998/10/29 Handles links correctly
// -KM- 1998/12/16 If an sfx doesn't exist, use pistol sound.
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    int i;
    sprintf(namebuf, "ds%s", sfx->link ? sfx->link->name : sfx->name);
    i = W_CheckNumForName(namebuf);
    if (i == -1)
      i = W_CheckNumForName("DSPISTOL");
    return i;
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
// -KM- 1998/09/01 Added looping for plats/crushing ceilings
//                 changed the way pitch works
int
I_StartSound
( int           id,
  int           vol,
  int           sep,
  int           pitch,
  int           priority,
  boolean       looping)
{
    // Not much point in playing sound, is there...
    if (digi_card == DIGI_NONE) return -1;

    if (swapstereo)
      sep = 255 - sep;
    // Returns a handle
    id = addsfx( id, vol, pitch, sep, looping );
    if (id >= 0) voice_set_priority(id & 0xffff, 128 - priority);
    
    return id;
}



void I_StopSound (int handle)
{
  // You need the handle returned by StartSound.
  // Would be looping all channels,
  //  tracking down the handle,
  //  an setting the channel to zero.
  int i;
  if (handle >= 0) {
    for (i = NUM_CHANNELS; i--;) {
      if (channelhandles[i] == handle) {
        channelhandles[i] = -1;
        //channels[i] = 0;
        if (voice_check(handle & 0xffff) == S_sfx[handle >> 16].data)
          deallocate_voice(handle & 0xffff);
          //voice_stop(handle & 0xffff);
      } /* if channelhandles[i] == handle */
    } /* for (i = num_channels; i--;) */
  } /* if handle >= 0 */
}


boolean I_SoundIsPlaying(int handle)
{
    int i;

    // If handle < 0, voice was not played in the first place
    if (handle >= 0) {

      // Find a channel with the same handle...
      for (i = NUM_CHANNELS; i--;)
        if (channelhandles[i] == handle) break;
      if (i < 0) return false;

      // If the sample hasn't been overwritten...
      if (voice_check(handle & 0xffff) != S_sfx[handle >> 16].data)
        return false;

      // If the sample hasn't finished playing...
      // Sample is playing.
      return ((voice_get_position(handle & 0xffff) == -1 )? false : true);
    }

    // Sample is not playing
    return false;
}




//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
//  allegro does this now
//
void I_UpdateSound( void )
{
  int i;
  for (i = NUM_CHANNELS; i--;) 
    if (!I_SoundIsPlaying(channelhandles[i])) I_StopSound(channelhandles[i]);
  I_MusicTicker2();
}


// 
// This would be used to write out the mixbuffer
//  during each game loop update.
// Updates sound buffer and audio device at runtime. 
// It is called during Timer interrupt with SNDINTR.
// Mixing now done synchronous, and
//  only output be done asynchronous?
//

void
I_SubmitSound(void)
{
  //this should no longer be necessary cuz allegro is doing all the sound mixing now
}



// -KM- 1998/09/01 Pitching changed
void
I_UpdateSoundParams
( int   handle,
  int   vol,
  int   sep,
  int   pitch)
{
  // -KM- 1999/01/31 Fixed swapstereo
  if (swapstereo)
    sep = 255 - sep;
  // Separation, that is, orientation/stereo.
  //  range is: 1 - 256 (Allegro : 0 - 255)
  // -KM- 1999/01/31 Added ability for pitch to be changed once set.
  //   (for Doppler effect.)
  if ((handle >= 0) && I_SoundIsPlaying(handle)) {
    voice_set_volume(handle & 0xffff, vol);
    voice_set_pan(handle & 0xffff, sep);
    voice_set_frequency(handle & 0xffff,
      (S_sfx[handle >> 16].data->freq * pitch) / 1000);
  }
}

// -KM- 1998/09/28 Added a timeout to this loop.
void I_ShutdownSound(void)
{    
  // Wait till all pending sounds are finished.
  boolean done = false, i;
  int exittic = I_GetTime() + 70;
    
  // Stop all sounds from looping, else this function will never end.
  for ( i = NUM_CHANNELS; i--;)
    if (channelhandles[i] >= 0)
      voice_set_playmode(channelhandles[i] & 0xffff, PLAYMODE_PLAY);

  // Wait for exit SFX to finish
  while ( !done && I_GetTime() < exittic)
  {
    done = true;
    I_UpdateSound();
    for( i = NUM_CHANNELS; i--;) 
      if (I_SoundIsPlaying(channelhandles[i])) done = false;
  }

  remove_sound();

  // Done.
  I_ShutdownMusic();
  return;
}

// -KM- 1998/09/27 Returns a value to disable sound if no sound card exists.
boolean
I_InitSound()
{
  // Secure and configure sound device first.
  I_Printf( "I_InitSound: ");
  // This isn't as bad as it looks, will try config file first
  if (install_sound(DIGI_AUTODETECT,MIDI_AUTODETECT,NULL)==-1)
  {
    I_Printf( "ALLEGRO SOUND INIT ERROR!!!!:\n\r");
    I_Printf( "   %s\n\r", allegro_error);
    return true;
  } else { // Kester.  Useful information (for people who _think_ they
           // have wavetable enabled and wonder why it sounds the same :-)
    I_Printf( " configured audio device:\n\r" );
    I_Printf( " SFX   : %s\n\r", digi_driver->desc);
    I_Printf( " Music : %s\n\r", midi_driver->desc);
  }

  I_SetChannels(numChannels);

  //music stuff
  I_InitMusic();

  if (M_CheckParm("-swapstereo"))
    swapstereo=true;

  // Finished initialization.
  I_Printf( "I_InitSound: sound module ready\n\r");

  return false;
}



