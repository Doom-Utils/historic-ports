/*      midassound.c
 *
 * A Sound module for DOOM using MIDAS Digital Audio System. Based on
 * original i_sound.c from the source distribution.
 * 
 * Petteri Kangaslampi, pekangas@sci.fi
*/


/* Original file header: */
//-----------------------------------------------------------------------------
//
// i_sound.c
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

const char
midassound_rcsid[] = "$Id: midassound.c,v 1.6 1998/01/08 17:20:06 pekangas Exp $";

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <math.h>

/* We'll need to go below the MIDAS API level a bit */
#include "midas.h"
#include "midasdll.h"

#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "i_video.h"

#include "doomdef.h"


/*#define DEBUGPRINT*/

// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.

#define NUM_CHANNELS		8

#define SAMPLERATE		11025	// Hz
#define SAMPLESIZE		2   	// 16bit

// The actual lengths of all sound effects.
int 		lengths[NUMSFX];

// The channel data pointers, start and end.
unsigned char*	channels[NUM_CHANNELS];
unsigned char*	channelsend[NUM_CHANNELS];


// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
int		channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
int 		channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
int		channelids[NUM_CHANNELS];			

// Pitch to stepping lookup, unused.
int		steptable[256];

// Volume lookups.
int		vol_lookup[128*256];

// Hardware left and right channel volume lookup.
int*		channelleftvol_lookup[NUM_CHANNELS];
int*		channelrightvol_lookup[NUM_CHANNELS];

/* MIDAS sample play handles for all channels: */
MIDASsamplePlayHandle channelPlayHandles[NUM_CHANNELS];

static int wavonly = 0;
static int primarysound = 0;
static int nosound = 0;

static int soundInitialized = 0;



void MIDASerror(void)
{
    I_Error("MIDAS Error: %s", MIDASgetErrorMessage(MIDASgetLastError()));
}



/* Loads a sound and adds it to MIDAS - really returns a MIDAS
   sample handle */
void*
getsfx
( char*         sfxname,
  int*          len )
{
    unsigned char*      sfx;
    int                 size;
    char                name[20];
    int                 sfxlump;
    
    int         error;
    static sdSample smp;
    unsigned sampleHandle;

    
    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    sprintf(name, "ds%s", sfxname);

    // Now, there is a severe problem with the
    //  sound handling, in it is not (yet/anymore)
    //  gamemode aware. That means, sounds from
    //  DOOM II will be requested even with DOOM
    //  shareware.
    // The sound list is wired into sounds.c,
    //  which sets the external variable.
    // I do not do runtime patches to that
    //  variable. Instead, we will use a
    //  default sound for replacement.
    if ( W_CheckNumForName(name) == -1 )
      sfxlump = W_GetNumForName("dspistol");
    else
      sfxlump = W_GetNumForName(name);
    
    size = W_LumpLength( sfxlump );

    // Debug.
    // fprintf( stderr, "." );
    //fprintf( stderr, " -loading  %s (lump %d, %d bytes)\n",
    //	     sfxname, sfxlump, size );
    //fflush( stderr );
    
    sfx = (unsigned char*)W_CacheLumpNum( sfxlump, PU_STATIC );

    /* Add sound to MIDAS: */
    /* A hack below API level - yeahyeah, we should add support for preparing
       samples from memory to the API */
    
    /* Build Sound Device sample structure for the sample: */
    smp.sample = sfx+8;
    smp.samplePos = sdSmpConv;
    smp.sampleType = MIDAS_SAMPLE_8BIT_MONO;
    smp.sampleLength = size-8;
    /* The lump has 8 bytes of magic in the beginning? */

    /* No loop: */
    smp.loopMode = sdLoopNone;
    smp.loop1Start = smp.loop1End = 0;
    smp.loop1Type = loopNone;

    /* No loop 2: */
    smp.loop2Start = smp.loop2End = 0;
    smp.loop2Type = loopNone;

    /* Add the sample to the Sound Device: */
    if ( (error = midasSD->AddSample(&smp, 1, &sampleHandle)) != OK )
        I_Error("getsfx: AddSample failed: %s", MIDASgetErrorMessage(error));

    // Remove the cached lump.
    Z_Free( sfx );
    
    // Preserve padded length.
    *len = size-8;

    /* Return sample handle: (damn ugly) */
    return (void*) sampleHandle;
}





//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
int
addsfx
( int		sfxid,
  int		volume,
  int		step,
  int		seperation )
{
    static unsigned short	handlenums = 0;
 
    int		i;
    int		rc = -1;
    
    int		oldest = gametic;
    int		oldestnum = 0;
    int		slot;

    MIDASsamplePlayHandle playHandle;
    int vol;
    int pan;
    

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if ( sfxid == sfx_sawup
	 || sfxid == sfx_sawidl
	 || sfxid == sfx_sawful
	 || sfxid == sfx_sawhit
	 || sfxid == sfx_stnmov
	 || sfxid == sfx_pistol	 )
    {
	// Loop all channels, check.
	for (i=0 ; i<NUM_CHANNELS ; i++)
	{
	    // Active, and using the same SFX?
	    if ( (channels[i])
		 && (channelids[i] == sfxid) )
	    {
		// Reset.
		channels[i] = 0;
		// We are sure that iff,
		//  there will only be one.
		break;
	    }
	}
    }

    // Loop all channels to find oldest SFX.
    for (i=0; (i<NUM_CHANNELS) && (channels[i]); i++)
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
    if (i == NUM_CHANNELS)
	slot = oldestnum;
    else
	slot = i;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char *) S_sfx[sfxid].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[sfxid];

    // Reset current handle number, limited to 0..100.
    if (!handlenums)
	handlenums = 100;

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    channelhandles[slot] = rc = handlenums++;

    // Set stepping???
    // Kinda getting the impression this is never used.
    /*channelstep[slot] = step;*/
    // ???
    /*channelstepremainder[slot] = 0;*/
    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    /* Calculate MIDAS volume and panning from volume and separation: */
    vol = 4*volume;   /* original range 0-15(?), MIDAS range 0-64 */
    pan = (seperation-128) / 2;  /* original 0-255(?), MIDAS -64-64 */

    /* Clamp: */
    if ( vol > 64 ) vol = 64;
    if ( vol < 0 ) vol = 0;
    if ( pan < -64) pan = -64;
    if ( pan > 64) pan = 64;

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = sfxid;

    /* Play the sound: */
    if ( (playHandle = MIDASplaySample((MIDASsample) S_sfx[sfxid].data,
                                       slot, 0, SAMPLERATE, vol, pan)) == 0 )
        MIDASerror();

    // You tell me.
/*  [Petteri] We'll return the MIDAS sample playback handle instead
    return rc;*/
    return (int) playHandle;
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
  int		i;
  int		j;
    
  int*	steptablemid = steptable + 128;
  
  // Okay, reset internal mixing channels to zero.
  /*for (i=0; i<NUM_CHANNELS; i++)
  {
    channels[i] = 0;
  }*/

  // This table provides step widths for pitch parameters.
  // I fail to see that this is currently used.
  for (i=-128 ; i<128 ; i++)
    steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);
  
  
  // Generates volume lookup tables
  //  which also turn the unsigned samples
  //  into signed samples.
  for (i=0 ; i<128 ; i++)
    for (j=0 ; j<256 ; j++)
      vol_lookup[i*256+j] = (i*(j-128)*256)/127;
}	

 
void I_SetSfxVolume(int volume)
{
  // Identical to DOS.
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
  snd_SfxVolume = volume;
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
  // Internal state variable.
  snd_MusicVolume = volume;
  // Now set volume on output device.
  // Whatever( snd_MusciVolume );
}


//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
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
int
I_StartSound
( int		id,
  int		vol,
  int		sep,
  int		pitch,
  int		priority )
{

  // UNUSED
  priority = 0;

#ifdef DEBUGPRINT  
  fprintf(stderr, "I_StartSound: vol %i, sep %i, pitch %i, pri %i\n",
          vol, sep, pitch, priority);
#endif  
  
    // Returns a handle (not used).
    id = addsfx( id, vol, steptable[pitch], sep );

    return id;
}



void I_StopSound (int handle)
{
  // You need the handle returned by StartSound.
  // Would be looping all channels,
  //  tracking down the handle,
  //  an setting the channel to zero.

#ifdef DEBUGPRINT    
    fprintf(stderr, "I_StopSound\n");
#endif    
    /* [Petteri] We'll simply stop the sound with MIDAS (?): */
    if ( !MIDASstopSample((MIDASsamplePlayHandle) handle) )
        MIDASerror();
}


int I_SoundIsPlaying(int handle)
{
    // Ouch.
/*    return gametic < handle;*/
    int is;
    is = (int) MIDASgetSamplePlayStatus((MIDASsamplePlayHandle) handle);
#ifdef DEBUGPRINT    
    fprintf(stderr, "I_SoundIsPlaying: %i\n", is);
#endif    
    return is;
}



int I_QrySongPlaying(int handle);


void I_UpdateSound( void )
{
    /* Loop song if necessary: */
    I_QrySongPlaying(1);
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
  // Write it to DSP device.
/*  write(audio_fd, mixbuffer, SAMPLECOUNT*BUFMUL);*/
}



void
I_UpdateSoundParams
( int	handle,
  int	vol,
  int	sep,
  int	pitch)
{
    int mvol, mpan;

    pitch = pitch; /* how to use? */
    
    /* Calculate MIDAS volume and panning from volume and separation: */
    mvol = 4*vol;   /* original range 0-15(?), MIDAS range 0-64 */
    mpan = (sep-128) / 2;  /* original 0-255(?), MIDAS -64-64 */

    /* Clamp: */
    if ( mvol > 64 ) mvol = 64;
    if ( mvol < 0 ) mvol = 0;
    if ( mpan < -64) mpan = -64;
    if ( mpan > 64) mpan = 64;

    /* Set: */
    if ( !MIDASsetSampleVolume((MIDASsamplePlayHandle) handle, mvol) )
        MIDASerror();
    if ( !MIDASsetSamplePanning((MIDASsamplePlayHandle) handle, mpan) )
        MIDASerror();
}




void I_ShutdownSound(void)
{
    unsigned i;

    if ( !soundInitialized )
        return;

    soundInitialized = 0;

    fprintf(stderr, "I_ShutdownSound: Stopping sounds\n");
    for ( i = 0; i < NUM_CHANNELS; i++ )
        MIDASstopSample(channelPlayHandles[i]);
    
    fprintf(stderr, "I_ShutdownSound: Uninitializing MIDAS\n");
    if ( !MIDASstopBackgroundPlay() )
        MIDASerror();
    if ( !MIDAScloseChannels() )
        MIDASerror();
    if ( !MIDASclose() )
        MIDASerror();

    I_ShutdownMusic();
}




extern DWORD win;  /* window handle from win32video.c (actually HWND) */


void
I_InitSound()
{
    unsigned i;

    if ( soundInitialized )
        return;

    MIDASstartup();

    soundInitialized = 1;
    
    I_InitMusic();

    /* Get command line options: */
    wavonly = !!M_CheckParm("-wavonly");
    primarysound = !!M_CheckParm("-primarysound");
    nosound = !!M_CheckParm("-nosound");
    
    fprintf(stderr, "I_InitSound: Initializing MIDAS\n");
    
    MIDASsetOption(MIDAS_OPTION_MIXRATE, SAMPLERATE);
    MIDASsetOption(MIDAS_OPTION_MIXBUFLEN, 200);

    /* We'll need to initialize graphics here so that we can get a window
       handle for DirectSound: */
    I_InitGraphics();
    
    if ( !wavonly )
    {
        MIDASsetOption(MIDAS_OPTION_DSOUND_HWND, win);
        if ( primarysound )
            MIDASsetOption(MIDAS_OPTION_DSOUND_MODE, MIDAS_DSOUND_PRIMARY);
        else
            MIDASsetOption(MIDAS_OPTION_DSOUND_MODE, MIDAS_DSOUND_STREAM);
    }

    if ( nosound )
        MIDASsetOption(MIDAS_OPTION_FORCE_NO_SOUND, TRUE);  
    
    if ( !MIDASinit() )
        MIDASerror();
    if ( !MIDASstartBackgroundPlay(100) )
        MIDASerror();
    if ( !MIDASopenChannels(NUM_CHANNELS) )
        MIDASerror();

    for ( i = 0; i < NUM_CHANNELS; i++ )
        channelPlayHandles[i] = 0;

    /* Simply preload all sounds and throw them at MIDAS: */
    fprintf(stderr, "I_InitSound: Loading all sounds\n");
    for ( i = 1; i < NUMSFX; i++ )
    {
        if ( !S_sfx[i].link )
        {
            /* Fine, not an alias, just load it */
            S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
            /* getsfx() actually returns a MIDAS sample handle */
        }
        else
        {
            /* An alias sound */
            S_sfx[i].data = S_sfx[i].link->data;
            lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
        }
    }

  // Finished initialization.
  fprintf(stderr, "I_InitSound: sound module ready\n");  
}




/* Music using Windows MIDI device - yuck */

static int nomusic = 0;
static char musName[512];
static char midName[512];
static int	looping=0;
static int	musicdies=-1;
static int playing = 0;
static int muslen;


void I_InitMusic(void)
{
    char *temp;

    fprintf(stderr, "I_InitMusic\n");
    
    nomusic = !!M_CheckParm("-nomusic");

    /* Create temporary file names: */    
    temp = getenv("TEMP");
    if ( temp == NULL )
        temp = ".";
    strcpy(musName, temp);

    while ( musName[strlen(musName)-1] == '\\' )
        musName[strlen(musName)-1] = 0;

    strcat(musName, "\\");
    strcpy(midName, musName);
    strcat(musName, "doomtemp.mus");
    strcat(midName, "doomtemp.mid");
}


void I_ShutdownMusic(void)
{
    if ( playing )
        I_StopSong(1);
    
    remove(musName);
    remove(midName);
}



int SendMCI(char *str, char *retStr, int retLen)
{
    int res;
    char errorStr[256];

    res = mciSendString(str, retStr, retLen, NULL);
    if ( res )
    {
        mciGetErrorString(res, errorStr, 255);
        fprintf(stderr, "MCI error: %s\n", errorStr);
    }
    
    return res;
}


void I_PlaySong(int handle, int _looping)
{
    char command[768];
    char resp[32];
        
    if ( nomusic || (handle == -1) )
    {
        playing = 0;
        return;
    }

    handle = handle;
    looping = _looping;
    playing = 0;

    sprintf(command, "open %s alias doomMusic", midName);
    if ( SendMCI(command, NULL, 0) )
        return;
    
    if ( SendMCI("play doomMusic from 0", NULL, 0) )
        return;

    if ( SendMCI("set doomMusic time format ms", NULL, 0) )
        return;

    if ( SendMCI("status doomMusic length", resp, 31) )
        return;

    muslen = atoi(resp);

    musicdies = gametic + (muslen * TICRATE) / 1000;

    playing = 1;
}

void I_PauseSong (int handle)
{
    if ( !playing )
        return;
    
    handle = handle;
}

void I_ResumeSong (int handle)
{
    if ( !playing )
        return;
    
    handle = handle;
}

void I_StopSong(int handle)
{
    handle = handle;
    
    if ( !playing )
        return;

    playing = 0;

    SendMCI("close doomMusic", NULL, 0);
}

void I_UnRegisterSong(int handle)
{
    handle = handle;
    remove(midName);
}

int musicLump;
extern int convert( const char *mus, const char *mid, int nodisplay, int div,
                    int size, int nocomp, int *ow );


int I_RegisterSong(void* data)
{
    FILE *f;
    int ow = 2;

    if ( (f = fopen(musName, "wb")) == NULL )
        I_Error("Unable to open temporary MUS file %s", musName);
    if ( fwrite(data, W_LumpLength(musicLump), 1, f) != 1 )
        I_Error("Unable to write temporary MUS file");
    fclose(f);

    convert(musName, midName, 1, 89, 0, 1, &ow);

    remove(musName);
  
    return 1;
}

// Is the song playing?
int I_QrySongPlaying(int handle)
{
    handle = handle;
    if ( !playing )
        return 0;

    if ( looping )
    {
        if ( gametic >= musicdies )
        {
            SendMCI("play doomMusic from 0", NULL, 0);
            musicdies = gametic + (muslen * TICRATE) / 1000;
        }
        return 1;
    }

    return musicdies > gametic;
}




/*
 * $Log: midassound.c,v $
 * Revision 1.6  1998/01/08 17:20:06  pekangas
 * MIDAS is now uninitialized on an I_Error exit
 *
 * Revision 1.5  1998/01/07 18:44:18  pekangas
 * Fixed a bunch of Visual C warnings, changed name to NTDOOM and
 * made assembler optional
 *
 * Revision 1.4  1998/01/05 16:29:58  pekangas
 * Removed yet another debug message (oops)
 *
 * Revision 1.3  1998/01/05 16:29:05  pekangas
 * Removed MCI debug messages
 *
 * Revision 1.2  1998/01/05 16:26:07  pekangas
 * Added music using Win32 MIDI MCI device
 *
 * Revision 1.1  1997/12/29 19:49:10  pekangas
 * Initial revision
 *
*/
