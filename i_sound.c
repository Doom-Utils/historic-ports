//
// DOSDoom Sound Interface Code 
//
// Based on the Doom Source Code
//
// Released by by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <stdarg.h>

#include <math.h>

#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

//im using allegro sound code
#include "i_alleg.h"

#include <bcd.h>


#include "dm_defs.h"
#include "d_debug.h"
#include "i_alleg.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"


//allegro has 256 virtual voices
#define VIRTUAL_VOICES 256
#define NUM_CHANNELS 16
#define SAMPLECOUNT 512

// Pitch to stepping lookup
static int             steptable[256];

static int channelhandles[NUM_CHANNELS];
static int channelstart[NUM_CHANNELS];

extern boolean swapstereo;

/* static inline int absolute_freq(int freq, SAMPLE *spl)
{
   if (freq == 1000)
      return spl->freq;
   else
      return (spl->freq * freq) / 1000;
} -ACB- 1998/09/07 - Not used, remarked out to shut-up the compiler */

typedef struct doomsfx_s {
    	unsigned short flags __attribute__((packed));
        unsigned short samplerate __attribute__((packed));
        unsigned long len __attribute__((packed));
        unsigned char  data[0] __attribute__((packed));
    } doomsfx_t;

//this function converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses
//it is need cuz allegro only loads samples from wavs and vocs
//extern void lock_sample(SAMPLE *spl);
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

inline void unlock_sample(SAMPLE *spl)
{
 	 _unlock_dpmi_data(spl->data, spl->len * spl->bits/8 * ((spl->stereo) ? 2 : 1));
         _unlock_dpmi_data(spl, sizeof(SAMPLE));
}
#define CACHEBIT 0x8000
#define LOCKBIT  0x4000
#define BITSBIT    0x0001
#define STEREOBIT 0x0002

//
// This function loads the sound data from the WAD lump,
//  for single sound.
// This now returns a SAMPLE*, not a void*
//
SAMPLE*
I_CacheSFX
( int		sfxnum )
{
    int			bits;
    boolean		stereo;
    SAMPLE		*spl;
    doomsfx_t		*sfx;

    if (S_sfx[sfxnum].lumpnum < 0)
      S_sfx[sfxnum].lumpnum = I_GetSfxLumpNum(&S_sfx[sfxnum]);

    sfx = (doomsfx_t *)W_CacheLumpNum( S_sfx[sfxnum].lumpnum, PU_SOUND );

    bits = S_sfx[sfxnum].bits; //(sfx->flags & BITSBIT) ? 8 : 16;
    stereo = S_sfx[sfxnum].stereo; //(sfx->flags & STEREOBIT) ? false : true;
    Debug_Printf("%.8s, %d bits%s@ %d Hz, %ld samples flags: %d\n",
                 S_sfx[sfxnum].name, bits, stereo ? " stereo " : " mono ", sfx->samplerate, sfx->len, sfx->flags);
    if ((sfx->len * (bits / 8) * (stereo ? 2 : 1) + sizeof(doomsfx_t)) > W_LumpLength(S_sfx[sfxnum].lumpnum))
    {
      I_Printf("Warning! Sample '%.8s' has bad length! (%ld, should be %ld)\n", S_sfx[sfxnum].name, sfx->len,
               (W_LumpLength(S_sfx[sfxnum].lumpnum) - sizeof(doomsfx_t)) / (bits / 8) / (stereo ? 2 : 1));
      sfx->len = (W_LumpLength(S_sfx[sfxnum].lumpnum) - sizeof(doomsfx_t)) / (bits / 8) / (stereo ? 2 : 1);
    }

    // Return allocated padded data.
    if (!(sfx->flags & CACHEBIT)) {
      sfx = Z_ReMalloc(sfx, sfx->len * (bits / 8) * (stereo ? 2 : 1) + sizeof(doomsfx_t) + sizeof(SAMPLE));
      sfx->flags |= CACHEBIT; // Mark as in use.
    }
    spl = raw2SAMPLE(sfx->data, sfx->len, sfx->samplerate, bits, stereo);

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
  if (S_sfx[sfxnum].lumpnum < 0)
    S_sfx[sfxnum].lumpnum = I_GetSfxLumpNum(&S_sfx[sfxnum]);
  sfx = (doomsfx_t *)W_CacheLumpNum( S_sfx[sfxnum].lumpnum, PU_CACHE );

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

    // Tales from the cryptic.
    // If we found a channel, fine.
    // If not, we simply overwrite the first one, 0.
    // Probably only happens at startup.
    slot = (i == NUM_CHANNELS)? oldestnum : i;
    if ((channelhandles[slot] >= 0) && 
	I_SoundIsPlaying(channelhandles[slot]))
      I_StopSound(channelhandles[slot]);

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // volume is adjusted to global volume
    // Not done in allegro, because that effects
    // Midi volume as well

    channelstart[slot] = volume;

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    //channelids[slot] = sfxid;

    
    //this is where the sound actually gets played.  i kept the previous code
    //because i dont want to risk breaking anything
    //max num in vol seems to be 8, i mul by 28 and not 31 just to be safe
    //               Data               Volume  Pan         Pitch Loop
    rc = play_sample(S_sfx[sfxid].data, volume, seperation, step, looping);

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    if (rc < 0) return rc;
    rc += sfxid << 16;
    channelhandles[slot] = rc;
    
    // You tell me.
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
void I_SetChannels()
{
  // Init internal lookups (raw data, mixing buffer, channels).
  // This function sets up internal lookups used during
  //  the mixing process. 
  int           i;
    
  int*  steptablemid = steptable + 128;
  
  // Okay, reset internal mixing channels to zero.
  /*for (i=0; i<NUM_CHANNELS; i++)
  {
    channels[i] = 0;
  }*/

  // This table provides step widths for pitch parameters.
  // I fail to see that this is currently used.
  //for (i=-128 ; i<128 ; i++)
    //steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);
  for (i=-128 ; i<128 ; i++)
    steptablemid[i] = i * i * i / 4000 + 1000;
}       

 
void I_SetSfxVolume(int volume)
{
  // Identical to DOS.
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
//  snd_SfxVolume = volume;
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

  // Separation, that is, orientation/stereo.
  //  range is: 1 - 256 (Allegro : 0 - 255)
  if ((handle >= 0) && I_SoundIsPlaying(handle)) {
    voice_set_volume(handle & 0xffff, vol);
    voice_set_pan(handle & 0xffff, sep);
//    voice_set_frequency(handle & 0xffff,
//       absolute_freq(pitch,
//       S_sfx[handle >> 16].data));
  }
}

// -KM- 1998/09/28 Added a timeout to this loop.
void I_ShutdownSound(void)
{    
  // Wait till all pending sounds are finished.
  boolean done = false, i;
  int exittic = I_GetTime() + 175;
    
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

  // Initialize external data (all sounds) at start, keep static.
  I_Printf( "I_InitSound: ");
  // Not much point in loading sounds if you don't have a sound card...
  // If you don't have a sound card you probably have a 386 or something
  // and want all the mem you can get :-)
/*  if (digi_card != DIGI_NONE)
    for (i = 1 ; i < numsfx; i++)
    {
        S_sfx[i].data = I_CacheSFX(i);
    }*/

  I_Printf( " pre-cached all sound data\n\r");
  
  // This table provides step widths for pitch parameters.
/*  for (i=-128 ; i<128 ; i++)
    steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);
*/
  I_SetChannels();

  //music stuff
  I_InitMusic();

  if (M_CheckParm("-swapstereo"))
    swapstereo=true;


  // Finished initialization.
  I_Printf( "I_InitSound: sound module ready\n\r");

  return false;
}



