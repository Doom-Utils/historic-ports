// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// Linux OSS sound output
#ifdef LINUX
#include <linux/soundcard.h>
#endif

// SCO OS5 and Unixware OSS sound output
#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include <sys/soundcard.h>
#endif

// FreeBSD OSS sound support
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif

#include <math.h>
#include <sys/time.h>
#include <sys/types.h>

#if !defined(LINUX) && !defined(SCOOS5) && !defined(_AIX)
#include <sys/filio.h>
#endif

#include <sys/ioctl.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>

// for IPC between xdoom and musserver
#include <sys/ipc.h>

// for mmap()
#ifdef SNDMMAP
#include <sys/mman.h>
#endif

#include "z_zone.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "doomdef.h"
#include "searchp.h"
#include "cd_audio.h"

#if defined(SNDINTR) && defined(SNDMMAP)
#error cannot use mmaped buffer and interrupt
#endif

extern int	errno;

// Number of sound channels used as set in the config file
extern int	numChannels;

#ifdef SNDSERV
// fp and filename for external sound server process
static FILE	*sndserver = 0;
char		*sndserver_filename = "sndserver";
#endif

#ifdef MUSSERV
// fp, filename and IPC message id for external music server process
static FILE	*musserver = 0;
char		*musserver_filename = "musserver";
static int	msg_id = -1;
#endif

// Flags for the -nosound and -nomusic options
extern boolean	nosound;
extern boolean	nomusic;

// Flag for playing music from CD instead of using musserver
extern boolean cdaudio;

// Flag to signal CD audio support to not play a title
static int	playing_title;

// A flag to establish a protocol between
// synchronous mix buffer updates and asynchronous
// sound writes
int sndupd_flag = 0;

// interrupt and timer used for asynchronous sound writes
static int itimer = ITIMER_VIRTUAL;
static int sig = SIGVTALRM;

// The samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the sample rate of the raw data.
// Needed for calling the actual sound output.
#define SAMPLECOUNT	768	// don't touch, read comments in I_InitSound()!!
// It is 2 for 16bit, and 2 for two channels.
#define BUFMUL		4
#define MIXBUFFERSIZE	(SAMPLECOUNT * BUFMUL)

#define SAMPLERATE	11025	// Hz
#define SAMPLESIZE	2   	// 16bit

// The actual lengths of all sound effects.
static int 	lengths[NUMSFX];

// The actual output device and a flag for using 8bit samples.
static int	audio_fd;
static int	audio_8bit_flag;

// This is used if the driver does support mmap'able DMA buffers
int				use_mmap = 0;	// flag if driver supports mmap
#ifndef SNDSERV
static struct audio_buf_info	abinfo;		// arg for SNDCTL_DSP_GETOSPACE
static struct count_info	cinfo;		// arg for SNDCTL_DSP_GETOPTR
#endif
caddr_t				abuf;		// mmap DMA buffer here
unsigned int			abuf_size;	// size of the DMA buffer

// The global mixing buffer.
// Basically, samples from all active internal channels
//  are modifed and added, and stored in the buffer
//  that is submitted to the audio device.
static signed short	mixbuffer[MIXBUFFERSIZE];

// The channel step amount...
static unsigned int	channelstep[NUM_CHANNELS];
// ... and a 0.16 bit remainder of last step.
static unsigned int	channelstepremainder[NUM_CHANNELS];

// The channel data pointers, start and end.
static unsigned char	*channels[NUM_CHANNELS];
static unsigned char	*channelsend[NUM_CHANNELS];

// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
static int		channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
static int 		channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
static int		channelids[NUM_CHANNELS];

// Pitch to stepping lookup, unused.
static int		steptable[256];

// Volume lookups.
static int		vol_lookup[128 * 256];

// Hardware left and right channel volume lookup.
static int		*channelleftvol_lookup[NUM_CHANNELS];
static int		*channelrightvol_lookup[NUM_CHANNELS];

// Used in Music API
static int		looping = 0;
static int		musicdies = -1;

//
// Safe ioctl, convenience.
//
void myioctl(int fd, int command, int	*arg)
{
    int		rc;

    rc = ioctl(fd, command, arg);
    if (rc < 0)
    {
	printf("ioctl(dsp,%d,arg) failed\n", command);
	printf("errno=%d\n", errno);
	exit(-1);
    }
}

// =========================================================================
//				SFX API
// =========================================================================

//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
void *getsfx(char *sfxname, int *len)
{
    unsigned char	*sfx;
    unsigned char	*paddedsfx;
    int			i;
    int			size;
    int			paddedsize;
    char		name[20];
    int			sfxlump;

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
    if (W_CheckNumForName(name) == -1)
      sfxlump = W_GetNumForName("dspistol");
    else
      sfxlump = W_GetNumForName(name);

    size = W_LumpLength(sfxlump);

    // Debug.
    // printf("." );
    // printf(" -loading  %s (lump %d, %d bytes)\n",
    //	     sfxname, sfxlump, size);
    // fflush(stdout);

    sfx = (unsigned char *)W_CacheLumpNum(sfxlump, PU_STATIC);

    // Pads the sound effect out to the mixing buffer size.
    // The original realloc would interfere with zone memory.
    paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;

    // Allocate from zone memory.
    paddedsfx = (unsigned char *)Z_Malloc(paddedsize + 8, PU_STATIC, (void *)0);

    // Now copy and pad.
    memcpy(paddedsfx, sfx, size);
    for (i = size; i < paddedsize + 8; i++)
        paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free(sfx);

    // Preserve padded length.
    *len = paddedsize;

    // Return allocated padded data.
    return (void *) (paddedsfx + 8);
}

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
int addsfx(int sfxid, int volume, int step, int seperation)
{
    static unsigned short	handlenums = 0;
    int		i;
    int		rc = -1;
    int		oldest = gametic;
    int		oldestnum = 0;
    int		slot;
    int		rightvol;
    int		leftvol;

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (sfxid == sfx_sawup
	 || sfxid == sfx_sawidl
	 || sfxid == sfx_sawful
	 || sfxid == sfx_sawhit
	 || sfxid == sfx_stnmov
	 || sfxid == sfx_pistol)
    {
	// Loop all channels, check.
	for (i = 0; i < numChannels; i++)
	{
	    // Active, and using the same SFX?
	    if ((channels[i]) && (channelids[i] == sfxid))
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
    for (i = 0; (i < numChannels) && (channels[i]); i++)
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
    if (i == numChannels)
	slot = oldestnum;
    else
	slot = i;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char *) S_sfx[sfxid].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[sfxid];

    // Assign current handle number.
    // Preserved so sounds can be stopped
    channelhandles[slot] = rc = handlenums++;

    // Set stepping???
    // Kinda getting the impression this is never used.
    channelstep[slot] = step;
    // ???
    channelstepremainder[slot] = 0;
    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    leftvol =
	volume - ((volume * seperation * seperation) >> 16);
    seperation = seperation - 257;
    rightvol =
	volume - ((volume * seperation * seperation) >> 16);

    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
	I_Error("rightvol out of bounds");

    if (leftvol < 0 || leftvol > 127)
	I_Error("leftvol out of bounds");

    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol * 256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol * 256];

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = sfxid;

    // You tell me.
    return rc;
}

//
// Called by S_Init.
//
void I_SetChannels(void)
{
  // Init internal lookups (raw data, mixing buffer, channels).
  // This function sets up internal lookups used during
  //  the mixing process.
  int		i;
  int		j;
  int		*steptablemid = steptable + 128;

  // This table provides step widths for pitch parameters.
  // I fail to see that this is currently used.
  for (i = -128; i < 128; i++)
    steptablemid[i] = (int)(pow(2.0, (i / 64.0)) * 65536.0);

  // Generates volume lookup tables
  //  which also turn the unsigned samples
  //  into signed samples.
  for (i = 0; i < 128; i++)
  {
    for (j = 0; j < 256; j++)
    {
	vol_lookup[i * 256 + j] = ((i * (j - 128) * 256) / 127) << 2;
    }
  }
}

void I_SetSfxVolume(int volume)
{
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
  snd_SfxVolume = volume;
}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t *sfx)
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
int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{

  // UNUSED
  priority = 0;

  if (nosound)
    return 0;

#ifdef SNDSERV
    if (sndserver)
    {
	fprintf(sndserver, "p%2.2x%2.2x%2.2x%2.2x\n", id, pitch, vol, sep);
	fflush(sndserver);
    }
#else
    // Debug.
    //printf("starting sound %d", id );

    // Returns a handle (not used).
    id = addsfx(id, vol, steptable[pitch], sep);

    //printf("/handle is %d\n", id);
#endif

    return id;
}

void I_StopSound(int handle)
{
  // Loop all channels, tracking down the handle,
  //  setting the channel to zero.
#ifndef SNDSERV
  int i;

  for (i = 0; i < numChannels; i++)
  {
    if (channelhandles[i] == handle)
    {
      channels[i] = 0;
      break;
    }
  }
#endif
}

int I_SoundIsPlaying(int handle)
{
#ifndef SNDSERV
    int i;

    for (i = 0; i < numChannels; i++)
    {
      if (channelhandles[i] == handle)
	return 1;
    }
#endif
    return 0;
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
void I_UpdateSound(void)
{
#ifndef SNDSERV
    // Debug. Count buffer misses with interrupt.
    static int misses = 0;

    // Flag. Will be set if the mixing buffer really got updated.
    int updated = 0;

    // Mix current sound data.
    // Data, from raw sound, for right and left.
    register unsigned int	sample;
    register int		dl;
    register int		dr;
    unsigned short		sdl;
    unsigned short		sdr;

    // Pointers in global mixbuffer, left, right, end.
    static signed short		*leftout;
    static signed short		*rightout;
    signed short		*leftend;
    unsigned char		*bothout;

    // Step in mixbuffer, left and right, thus two.
    int				step = 2;

    // Mixing channel index.
    int				chan;

    // Left and right channel
    //  are in global mixbuffer, alternating.
    if (!use_mmap)
    {
	leftout = mixbuffer;
	rightout = mixbuffer + 1;
	bothout = (unsigned char *)mixbuffer;
    }
    else
    {
	if (!rightout || ((char *)rightout >= (char *)abuf + abuf_size))
	{
	    leftout = (signed short *)abuf;
	    rightout = (signed short *)abuf + 1;
	}
	bothout = NULL;
    }

    // Determine end, for left channel only
    //  (right channel is implicit).
    if (!use_mmap)
	leftend = mixbuffer + SAMPLECOUNT * step;
    else
	leftend = leftout + SAMPLECOUNT * step;

    // We cannot write into the DMA buffer fragments, the sound card is
    // playing right now and we might have to wait.
    if (use_mmap)
    {
	int n1, n2;
	struct timeval tm;

	while (1)
	{
	    myioctl(audio_fd, SNDCTL_DSP_GETOPTR, (int *)&cinfo);
	    n1 = cinfo.ptr / abinfo.fragsize;
	    n2 = ((char *)leftout - (char *)abuf) / abinfo.fragsize;
	    if (n1 <= n2 || n1 > n2+2)
		  break;
	    tm.tv_sec = 0;
	    tm.tv_usec = 10000;
	    select(0, NULL, NULL, NULL, &tm);
	}
    }

    // Mix sounds into the mixing buffer.
    // Loop over step*SAMPLECOUNT.
    while (leftout != leftend)
    {
	// Reset left/right value.
	dl = 0;
	dr = 0;

	// Love thy L2 chache - made this a loop.
	// Now more channels could be set at compile time
	//  as well. Thus loop those  channels.
	// Uh hu, can be up to 64 channels now...
	for (chan = 0; chan < numChannels; chan++)
	{
	    // Check channel, if active.
	    if (channels[chan])
	    {
		// we are updating the mixer buffer, set flag
		updated++;
		// Get the raw data from the channel.
		sample = *channels[chan];
		// Add left and right part
		//  for this channel (sound)
		//  to the current data.
		// Adjust volume accordingly.
		dl += channelleftvol_lookup[chan][sample];
		dr += channelrightvol_lookup[chan][sample];
		// Increment index ???
		channelstepremainder[chan] += channelstep[chan];
		// MSB is next sample???
		channels[chan] += channelstepremainder[chan] >> 16;
		// Limit to LSB???
		channelstepremainder[chan] &= 65536 - 1;

		// Check whether we are done.
		if (channels[chan] >= channelsend[chan])
		    channels[chan] = 0;
	    }
	}

	// Clamp to range. Left hardware channel.
	if (!audio_8bit_flag)
	{
	    if (dl > 0x7fff)
	        *leftout = 0x7fff;
	    else if (dl < -0x8000)
	        *leftout = -0x8000;
	    else
	        *leftout = dl;

	    // Same for right hardware channel.
	    if (dr > 0x7fff)
	        *rightout = 0x7fff;
	    else if (dr < -0x8000)
	        *rightout = -0x8000;
	    else
	        *rightout = dr;
	}
	else
	{
	    // mix the channels together for 8bit sound
	    if (dl > 0x7fff)
		dl = 0x7fff;
	    else if (dl < -0x8000)
		dl = -0x8000;
	    sdl = dl ^ 0xfff8000;

	    if (dr > 0x7fff)
		dr = 0x7fff;
	    else if (dr < -0x8000)
		dr = -0x8000;
	    sdr = dr ^ 0xfff8000;

	    *bothout++ = (((sdr + sdl) / 2) >> 8);
	}

	// Increment current pointers in mixbuffer.
	leftout += step;
	rightout += step;
    }

    if (updated && !use_mmap)
    {
	sigset_t ss, os;

	sigemptyset(&ss);
	sigaddset(&ss, sig);
	sigprocmask(SIG_BLOCK, &ss, &os);

	// Debug check.
	if (sndupd_flag)
	{
	  misses += sndupd_flag;
	  sndupd_flag = 0;
	}

	if (misses > 10)
	{
	  printf("I_SoundUpdate: missed 10 buffer writes\n");
	  misses = 0;
	}

	// Increment flag for update.
	sndupd_flag++;

	sigprocmask(SIG_SETMASK, &os, (sigset_t *)0);
    }
#endif
}

//
// This is used to write out the mixbuffer
//  during each game loop update when using
//  synchronous sound updates.
//
void I_SubmitSound(void)
{
  int len = (audio_8bit_flag) ? SAMPLECOUNT : SAMPLECOUNT * BUFMUL;

  // do we have new sound data in the mixing buffer?
  if (sndupd_flag && !use_mmap)
  {
    sndupd_flag = 0;
    // write it to DSP device.
    write(audio_fd, mixbuffer, len);
  }
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
  // I fail too see that this is used.
  // Would be using the handle to identify
  //  on which channel the sound might be active,
  //  and resetting the channel parameters.

  // UNUSED.
  handle = vol = sep = pitch = 0;
}

void I_ShutdownSound(void)
{
#ifdef SNDSERV
  if (sndserver)
  {
    // Send a "quit" command.
    fprintf(sndserver, "q\n");
    fflush(sndserver);
  }

#else /* SNDSERV */

  // Wait till all pending sounds are finished.
  int done = 0;
  int i;

  if (nosound)
    return;

#ifdef SNDINTR
  // if using asynchrony sound updates stop the interrupt timer now
  I_SoundDelTimer();
#endif

  while (!done)
  {
    for(i = 0; i < numChannels && !channels[i]; i++)
      ;
    if (i == numChannels)
      done++;
    else
    {
	I_UpdateSound();
	I_SubmitSound();
	I_WaitVBL(1);
    }
  }

  I_WaitVBL(30);

  // Cleaning up, release the DSP device.
#ifdef SNDMMAP
  if (use_mmap)
  {
    memset(abuf, 0, abuf_size);
    i = 0;
    myioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &i);
    munmap(abuf, abuf_size);
    myioctl(audio_fd, SNDCTL_DSP_RESET, 0);
  }
#endif
  close(audio_fd);

#endif /* SNDSERV */

  // Done.
  return;
}

void I_InitSound(void)
{
#ifdef SNDSERV
  char buffer[2048];
  char *fn_snd;

  if (nosound)
    return;

  fn_snd = searchpath(sndserver_filename);

  // start sound process
  if (!access(fn_snd, X_OK))
  {
    sprintf(buffer, "%s", fn_snd);
    strcat(buffer, " -quiet");
    sndserver = popen(buffer, "w");
  }
  else
    printf("Could not start sound server [%s]\n", fn_snd);

#else /* SNDSERV */

  int i, j;

  if (nosound)
    return;

  // Secure and configure sound device first.
  printf("I_InitSound: ");

#ifdef SNDMMAP
  // don't use O_WRONLY, else mmap() will fail
  audio_fd = open("/dev/dsp", O_RDWR);
#else
  audio_fd = open("/dev/dsp", O_WRONLY);
#endif
  if (audio_fd < 0)
  {
    printf("Could not open /dev/dsp, no sound used.\n");
    nosound++;
    return;
  }

#if 0
  // this causes problems with various OSS releases, don't do it anymore
  myioctl(audio_fd, SNDCTL_DSP_RESET, 0);
#endif

  if (getenv("DOOM_SOUND_SAMPLEBITS") == NULL)
  {
    myioctl(audio_fd, SNDCTL_DSP_GETFMTS, &i);
    if (i &= AFMT_S16_LE)
    {
      // 12 buffers a 1K
      //  1) 12K is dividable by 4 and so the DMA buffer can be mmap'ed on x86.
      //  2) 12K is dividable by 768 sound samples * 4 bytes/sample.
      //  3) 768 * 4 = 3072 and 3 * 1024 = 3072, means 4 sound samples fit
      //     into 3 DMA buffer fragments. Has to fit somehow so we can write
      //     a contineous stream into the DMA buffers.
      j = 10 | (11 << 16);
      myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &j);
      myioctl(audio_fd, SNDCTL_DSP_GETOSPACE, (int *)&abinfo);
      abuf_size = abinfo.fragstotal * abinfo.fragsize;
      myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
      i = 1;
      myioctl(audio_fd, SNDCTL_DSP_STEREO, &i);
    }
    else
    {
      // 4 buffers a 1K, enough for lame old 8bit sound cards
      i = 10 | (3 << 16);
      myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
      i = AFMT_U8;
      myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
      audio_8bit_flag++;
    }
  }
  else
  {
    // 4 buffers a 1K, enough for lame old 8bit sound cards
    i = 10 | (3 << 16);
    myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
    i = AFMT_U8;
    myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
    audio_8bit_flag++;
  }

  i = SAMPLERATE;
  myioctl(audio_fd, SNDCTL_DSP_SPEED, &i);

  printf("configured %dbit audio device\n", (audio_8bit_flag) ? 8 : 16 );

#ifdef SNDMMAP
  // figure if the driver does support mmap()
  if (!audio_8bit_flag)
  {
    myioctl(audio_fd, SNDCTL_DSP_GETCAPS, &i);
    if ((i & DSP_CAP_MMAP) && (i & DSP_CAP_TRIGGER))
    {

#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#define MAP_FILE 0
#endif

      // mmap the DMA buffer
      if ((abuf = mmap(NULL, abuf_size, PROT_WRITE, MAP_FILE | MAP_SHARED,
	  audio_fd, 0)) == (caddr_t)-1)
      {
        printf("I_InitSound: mmap() failed, errno=%d\n", errno);
        exit(-1);
      }
      // silence
      memset(abuf, 0, abuf_size);
      // set trigger
      i = 0;
      ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &i);
      // start DMA
      i = PCM_ENABLE_OUTPUT;
      myioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &i);
      use_mmap++;
      printf("I_InitSound: sound driver supports mmap()\n");
    }
    else
      printf("I_InitSound: sound driver doesn't support mmap()\n");
  }
#endif

#ifdef SNDINTR
  printf("I_InitSound: using %d microsecs timer\n", SOUND_INTERVAL);
  I_SoundSetTimer(SOUND_INTERVAL);
#endif

  // Initialize external data (all sounds) at start, keep static.
  printf("I_InitSound: ");

  // Do we have a sound lump for the chaingun?
  if ((W_CheckNumForName("dschgun") == -1) || M_CheckParm("-nochgun"))
  {
    // No, so link it to the pistol sound
    S_sfx[sfx_chgun].name = "pistol";
    printf("linking chaingun sound to pistol sound,");
  }
  else
  {
    printf("found chaingun sound,");
  }

  for (i = 1; i < NUMSFX; i++)
  {
    // Alias? Example is the chaingun sound linked to pistol.
    if (!S_sfx[i].link)
    {
      // Load data from WAD file.
      S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
    }
    else
    {
      // Previously loaded already?
      S_sfx[i].data = S_sfx[i].link->data;
      lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
    }
  }

  printf(" pre-cached all sound data\n");

  // Now initialize mixbuffer with zero.
  memset(mixbuffer, 0, MIXBUFFERSIZE);

  // Finished initialization.
  printf("I_InitSound: sound module ready\n");

#endif /* SNDSERV */
}

//
// Routines for interrupt driven, asynchrony sound output.
//

// Interrupt handler.
void I_HandleSoundTimer(int ignore)
{
  sigset_t ss, os;
  int len = (audio_8bit_flag) ? SAMPLECOUNT : SAMPLECOUNT * BUFMUL;
  static int missed = 0;

  // Debug.
  //printf("%c", '+' ); fflush(stdout);

  sigemptyset(&ss);
  sigaddset(&ss, sig);
  sigprocmask(SIG_BLOCK, &ss, &os);

  // do we have new sound data in the mixing buffer not played yet?
  if (sndupd_flag || missed)
  {
#if 0	// theoretically this should work, pratically it doesn't work well
    int i;

    // check how much data still needs to be played by the driver
    myioctl(audio_fd, SNDCTL_DSP_GETODELAY, &i);

    // if that is more than 2 * len and we haven't missed interrupts yet,
    // we can afford to miss this one
    // with this the writes to the device won't block that often
    if ((i >= len * 2) && !missed)
    {
      missed++;
      sigprocmask(SIG_SETMASK, &os, (sigset_t *)0);
      return;
    }
#endif

    // write data to DSP device
    write(audio_fd, mixbuffer, len);
    sndupd_flag = missed = 0;
  }

  sigprocmask(SIG_SETMASK, &os, (sigset_t *)0);

  // UNUSED, but required.
  ignore = 0;

  return;
}

// Set the interrupt. Set duration in millisecs.
int I_SoundSetTimer(int duration_of_tick)
{
  struct itimerval	value;
  struct sigaction	act;
  int res;

  act.sa_handler = I_HandleSoundTimer;
  act.sa_flags = SA_RESTART;
  sigemptyset(&act.sa_mask);
  sigaction(sig, &act, (struct sigaction *)0);

  value.it_interval.tv_sec    = 0;
  value.it_interval.tv_usec   = duration_of_tick;
  value.it_value.tv_sec       = 0;
  value.it_value.tv_usec      = duration_of_tick;
  res = setitimer (itimer, &value, (struct itimerval *)0);

  // Debug.
  if ( res == -1 )
    printf("I_SoundSetTimer: interrupt n.a.\n");

  return res;
}

// Remove the interrupt. Set duration to zero.
void I_SoundDelTimer(void)
{
  struct sigaction    act;

  // Debug.
  if (I_SoundSetTimer(0) == -1)
    printf("I_SoundDelTimer: failed to remove interrupt. Doh!\n");

  act.sa_handler = SIG_DFL;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(sig, &act, (struct sigaction *)0);
}

// =========================================================================
//				MUSIC API
// =========================================================================

//
// Music done now, we'll use Michael Heasley's musserver and
// optionally CD music.
//

void I_InitMusic(void)
{
#ifdef MUSSERV
  char buffer[2048];
  char *fn_mus;
#endif

  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    // if initializing works we assume we can play a CD, if not
    // just use the default music
    if (CD_init() != 0)
    {
      cdaudio = 0;
    }
  }

#ifdef MUSSERV
  fn_mus = searchpath(musserver_filename);

  // now try to start the music server process
  if (!access(fn_mus, X_OK))
  {
    sprintf(buffer, "%s", fn_mus);
    strcat(buffer, " -t 3");
    printf("Starting music server [%s]\n", buffer);
    musserver = popen(buffer, "w");
    msg_id = msgget(53075, IPC_CREAT | 0777);
  }
  else
    printf("Could not start music server [%s]\n", fn_mus);
#endif
}

void I_ShutdownMusic (void)
{
  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    CD_shutdown();
  }

#ifdef MUSSERV
  if (musserver)
  {
    // send a "quit" command.
    if (msg_id != -1)
      msgctl(msg_id, IPC_RMID, (struct msqid_ds *) 0);
  }
#endif
}

void I_SetMusicVolume (int volume)
{
  // Internal state variable.
  snd_MusicVolume = volume;
  // Now set volume on output device.
  // Whatever(snd_MusciVolume);

  if (nosound || nomusic)
    return;

#ifdef MUSSERV
  if (msg_id != -1)
  {
    struct {
      long msg_type;
      char msg_text[12];
    } msg_buffer;

    msg_buffer.msg_type = 6;
    memset(msg_buffer.msg_text, 0, 12);
    msg_buffer.msg_text[0] = 'v';
    msg_buffer.msg_text[1] = volume;
    msgsnd(msg_id, (struct msgbuf *) &msg_buffer, 12, IPC_NOWAIT);
  }
#endif
}

void I_SetCdVolume(int volume)
{
  snd_CdVolume = volume;

  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    CD_volume(volume);
  }
}

void I_PlaySong(int handle, int looping)
{
  // UNUSED.
  handle = 0;

  looping = 0;
  musicdies = gametic + TICRATE*30;

  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    if (!playing_title)
	CD_start(handle);
  }
}

void I_PauseSong(int handle)
{
  // UNUSED.
  handle = 0;

  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    CD_pause(handle);
  }

#ifdef MUSSERV
  if (msg_id != -1)
  {
    struct {
      long msg_type;
      char msg_text[12];
    } msg_buffer;

    msg_buffer.msg_type = 6;
    memset(msg_buffer.msg_text, 0, 12);
    msg_buffer.msg_text[0] = 'p';
    msgsnd(msg_id, (struct msgbuf *) &msg_buffer, 12, IPC_NOWAIT);
  }
#endif
}

void I_ResumeSong(int handle)
{
  // UNUSED.
  handle = 0;

  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    CD_resume(handle);
  }
#ifdef MUSSERV
  if (msg_id != -1)
  {
    struct {
      long msg_type;
      char msg_text[12];
    } msg_buffer;

    msg_buffer.msg_type = 6;
    memset(msg_buffer.msg_text, 0, 12);
    msg_buffer.msg_text[0] = 'P';
    msgsnd(msg_id, (struct msgbuf *) &msg_buffer, 12, IPC_NOWAIT);
  }
#endif
}

void I_StopSong(int handle)
{
  // UNUSED.
  handle = 0;

  looping = musicdies = 0;

  if (nosound || nomusic)
    return;

  if (cdaudio)
  {
    if (!playing_title)
	CD_stop(handle);
    else
	playing_title = 0;
  }
}

void I_UnRegisterSong(int handle)
{
  // UNUSED.
  handle = 0;
}

int I_RegisterSong(void *data)
{
#ifdef MUSSERV
  struct {
	long msg_type;
	char msg_text[12];
  } msg_buffer;
#endif

  if (nosound || nomusic)
  {
    data = NULL;
    return 1;
  }

  if (cdaudio)
  {
    // don't play CD for title tunes
    if ((strcmp((char *)data, "intro") == 0) ||
        (strcmp((char *)data, "dm2ttl") == 0))
    {
	playing_title = 1;
    }
    else
    {
	CD_song(data);
	data = NULL;
        return 1;
    }
  }

#ifdef MUSSERV
  if (msg_id != -1)
  {
    msg_buffer.msg_type = 6;
    memset(msg_buffer.msg_text, 0, 12);
    sprintf(msg_buffer.msg_text, "d_%s", (char *) data);
    msgsnd(msg_id, (struct msgbuf *) &msg_buffer, 12, IPC_NOWAIT);
  }
#else
  data = NULL;
#endif

  return 1;
}

// Is the song playing?
int I_QrySongPlaying(int handle)
{
  // UNUSED.
  handle = 0;
  return looping || musicdies > gametic;
}
