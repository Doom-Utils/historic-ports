// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
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
// $Log:$
//
// DESCRIPTION:
//	System interface for sound.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_unix.c,v 1.5 1997/02/03 22:45:10 b1 Exp $";

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
#include <allegro.h>
#include <bcd.h>

#include "z_zone.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"

//cd-audio stuff
int cdaudio=0;
int cdtrack=-1;
int numtracks=0;
int starttrack=0;
volatile int cdcounter=0;

//allegro has 256 virtual voices
#define VIRTUAL_VOICES 256

#define SAMPLECOUNT 512
// The actual lengths of all sound effects.
int 		lengths[NUMSFX];

// Pitch to stepping lookup
int		steptable[256];

int DetectMusicCard(void);
int DetectSoundCard(void);

//this function converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses
//it is need cuz allegro only loads samples from wavs and vocs
SAMPLE *raw2SAMPLE(unsigned char *rawdata, int len, unsigned short freq)
  {
  SAMPLE *spl;

  spl=malloc(sizeof(SAMPLE));
  spl->bits = 8;
  spl->freq = freq;
  spl->len = len;
  spl->priority = 255;
  spl->loop_start = 0;
  spl->loop_end = len;
  spl->param = -1;
  spl->data=(void *)rawdata;

  return spl;
  }


//
// This function loads the sound data from the WAD lump,
//  for single sound.
// This now returns a SAMPLE*, not a void*
//
SAMPLE*
getsfx
( char*         sfxname,
  int*          len )
{
    unsigned char*      sfx;
    unsigned char*      paddedsfx;
    int                 i;
    int                 size;
    int                 paddedsize;
    char                name[20];
    int                 sfxlump;

    
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

    // Pads the sound effect out to the mixing buffer size.
    // The original realloc would interfere with zone memory.
    paddedsize = ((size-8 + (SAMPLECOUNT-1)) / SAMPLECOUNT) * SAMPLECOUNT;

    // Allocate from zone memory.
    paddedsfx = (unsigned char*)Z_Malloc( paddedsize+8, PU_STATIC, 0 );
    // ddt: (unsigned char *) realloc(sfx, paddedsize+8);
    // This should interfere with zone memory handling,
    //  which does not kick in in the soundserver.

    // Now copy and pad.
    memcpy(  paddedsfx, sfx, size );
    for (i=size ; i<paddedsize+8 ; i++)
        paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free( sfx );
    
    // Preserve padded length.
    *len = paddedsize;


    // Return allocated padded data.
    return raw2SAMPLE(paddedsfx + 8,*len,*((unsigned short *)(paddedsfx+2)));
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

SAMPLE*
I_StartSound
( int		id,
  int		vol,
  int		sep,
  int		pitch,
  int		priority )
  {
  int i;

  // UNUSED
  priority = 0;

  //handle chainsaw
  if ( id == sfx_sawup
    || id == sfx_sawidl
	 || id == sfx_sawful
	 || id == sfx_sawhit
	 || id == sfx_stnmov
	 || id == sfx_pistol	 )
    {
    // Loop all channels, check.
    for (i=0;i<VIRTUAL_VOICES;i++)
      {
      if (voice_check(i)==S_sfx[id].data)
        {
        stop_sample(S_sfx[id].data);
        break;
        }
      }
    }
  pitch=(pitch-128)/8+128;
  if (swapstereo==0)
    play_sample(S_sfx[id].data,vol*16,sep,pitch*1000/128,0);
  else
    play_sample(S_sfx[id].data,vol*16,255-sep,pitch*1000/128,0);

  // Returns a handle
  return S_sfx[id].data;
  }


void I_StopSound (SAMPLE* handle)
  {
  stop_sample(handle);
  }


int I_SoundIsPlaying(SAMPLE* handle)
  {
  int i;

  for (i=0;i<VIRTUAL_VOICES;i++)
    {
    if (voice_check(i)==handle)
      return TRUE;
    }
  return FALSE;
  }

void
I_UpdateSoundParams
( SAMPLE*	handle,
  int	vol,
  int	sep,
  int	pitch)
  {
  pitch=(pitch-128)/8+128;
  if (swapstereo==0)
    adjust_sample(handle,vol*16,sep,pitch*1000/128,0);
  else
    adjust_sample(handle,vol*16,255-sep,pitch*1000/128,0);
  }

void I_SetSfxVolume(int volume)
{
  // Identical to DOS.
  // Basically, this should propagate
  //  the menu/config file setting
  //  to the state variable used in
  //  the mixing.
  snd_SfxVolume = volume;
  set_volume(snd_SfxVolume*16,-1);
}


//  allegro does these two things now
void I_UpdateSound(void) { }
void I_SubmitSound(void) { }

void I_ShutdownSound(void)
{    
  // Wait till all pending sounds are finished.
  int done = 0;
    
  while (!done)  //fixme
  {

//    for( i=0 ; i<8 && !channels[i] ; i++);
    
    // FIXME. No proper channel output.
    //if (i==8)
    done=1;
  }

  remove_sound();

  if (cdaudio==1)
    {
    bcd_stop();
    bcd_close();
    }
  // Done.
  I_ShutdownMusic();
  return;
}


void
I_InitSound()
{
  int i;
  int*  steptablemid = steptable + 128;

  if (M_CheckParm("-cdaudio"))
    {
    if (bcd_open()==0)
      printf ("Unable to access CD-ROM drive!  Defaulting to normal music\n");
    else
      {
      if ((numtracks=bcd_get_audio_info())==0)
        printf ("Error getting audio info - Defaulting to normal music!\n");
      else
        {
        for (i=1;i<=numtracks;i++)
          if (bcd_track_is_audio(i)==1)
            {
            starttrack=i;
            break;
            }
        if (starttrack>numtracks)
          printf ("No audio tracks! - Defaulting to normal music!\n");
        else
          {
          cdaudio=1;
          cdtrack=starttrack;
          printf ("Starting CD-Audio from track %d\n",cdtrack);
          }
        }
      }
    }

  if (M_CheckParm("-ilovebill"))
    i_love_bill=true; //kill that stupid windoze warning message
  // Secure and configure sound device first.
  fprintf( stderr, "I_InitSound: ");
  if (install_sound(DetectSoundCard(),DetectMusicCard(),NULL)==-1)
    fprintf(stderr,"ALLEGRO SOUND INIT ERROR!!!!\n");
  else
    fprintf(stderr, " configured audio device\n" );

  // Initialize external data (all sounds) at start, keep static.
  fprintf( stderr, "I_InitSound: ");
  
  for (i=1 ; i<NUMSFX ; i++)
  { 
    // Alias? Example is the chaingun sound linked to pistol.
    if (!S_sfx[i].link)
    {
      // Load data from WAD file.
      S_sfx[i].data = getsfx( S_sfx[i].name, &lengths[i] );
    }	
    else
    {
      // Previously loaded already?
      S_sfx[i].data = S_sfx[i].link->data;
      lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
    }
  }

  fprintf( stderr, " pre-cached all sound data\n");
  
  // This table provides step widths for pitch parameters.
  for (i=-128 ; i<128 ; i++)
    steptablemid[i] = (int)(pow(2.0, (i/64.0))*65536.0);

  //cd-audio start
  if (bcd_audio_busy())
    {
    bcd_stop();
    delay(1000);
    }
  bcd_play_track(cdtrack);

  //music stuff
  I_InitMusic();

  if (M_CheckParm("-swapstereo"))
    swapstereo=1;

  // Finished initialization.
  fprintf(stderr, "I_InitSound: sound module ready\n");

}


extern int snd_musicdevice;
extern int snd_sfxdevice;

int DetectMusicCard(void)
  {
  if (cdaudio==1)
    return MIDI_NONE;
  if (M_CheckParm("-digmid"))
    return MIDI_DIGMID;
  if (M_CheckParm("-readmus"))
    {
    switch (snd_musicdevice)
      {
      case 0: //nosound
        return MIDI_NONE; break;
      case 2: //adlib
        return MIDI_ADLIB; break;
      case 3: //sound blaster
        return MIDI_OPL2; break;
      case 4: //pro audio spectrum
        return MIDI_NONE; break;
      case 5: //gus
        return MIDI_GUS; break;
      case 6: //wave blaster
        return MIDI_NONE; break;
      case 7: //roland sound canvas
        return MIDI_NONE; break;
      case 8: //general midi
        return MIDI_MPU; break;
      case 9: //awe 32
        return MIDI_AWE32; break;
      }
    }
  return MIDI_AUTODETECT;
  }

int DetectSoundCard(void)
  {
  if (M_CheckParm("-readsfx"))
    {
    switch (snd_sfxdevice)
      {
      case 0: //nosound
        return DIGI_NONE; break;
      case 3: //sound blaster
        return DIGI_SB; break;
      case 4: //pro audio spectrum
        return DIGI_NONE; break;
      case 5: //gus
        return DIGI_GUS; break;
      }
    }
  return DIGI_AUTODETECT;
  }

//check cd player - if curr track is done, go to next track
void I_CheckCD()
  {
  int temptrack;

  if ((cdaudio==1)&&(cdcounter>2000)&&(!paused))
    {
    if (bcd_audio_busy()==0)
      {
      temptrack=cdtrack+1;
      if (temptrack>(numtracks+starttrack-1))
        temptrack=starttrack;
      cdtrack=temptrack;
      bcd_play_track(cdtrack);
      cdcounter=0;
      }
    }
  }

