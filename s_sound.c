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
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: s_sound.c,v 1.6 1997/02/03 22:45:12 b1 Exp $";



#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "i_sound.h"
#include "lu_sound.h"
#include "s_sound.h"

#include "z_zone.h"
#include "m_random.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"
#include "m_argv.h"

#include "doomstat.h"


// Purpose?
const char snd_prefixen[]
= { 'P', 'P', 'A', 'S', 'S', 'S', 'M', 'M', 'M', 'S', 'S', 'S' };

#define S_MAX_VOLUME            15
#define S_MIN_VOLUME		0
// when to clip out sounds
// Does not fit the large outdoor areas. 1600 ** 2
#define S_CLIPPING_DIST         1600
#define S_CLIPPING_DIST2        (S_CLIPPING_DIST * S_CLIPPING_DIST)
// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000). 160 ** 2
#define S_CLOSE_DIST            160
#define S_CLOSE_DIST2		(S_CLOSE_DIST * S_CLOSE_DIST)
//(S_CLIPPING_DIST2-S_CLOSE_DIST2)
#define S_ATTENUATOR            (S_CLIPPING_DIST2 - S_CLOSE_DIST2)

// Adjustable by menu.

#define NORM_PITCH              128
#define NORM_PRIORITY           64
#define NORM_SEP                128

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

// percent attenuation from front to back
#define S_IFRACVOL              30

#define NA                      0
#define S_NUMCHANNELS           2


// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;

static boolean nosound;

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t*  sfxinfo;

    // origin of sound
    mobj_t*       origin;

    // handle of the sound being played
    int             handle;
    
} channel_t;


// the set of channels available
static channel_t*       channels;

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int             snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int             snd_MusicVolume = 15; 



// whether songs are mus_paused
static boolean          mus_paused;     

// music currently being played
static musicinfo_t*     mus_playing=0;

// following is set
//  by the defaults code in M_misc:
// number of channels available
//int                   numChannels;
//allegro has 256 virtual channels
int numChannels = 8;

static int              nextcleanup;

static inline int min(int a, int b)
{
 return (a < b) ? a : b;
}

static inline int max(int a, int b)
{
 return (a > b) ? a : b;
}

static inline int mid(int a, int b, int c)
{
 return max(a, min(b, c));
}

//
// Internals.
//
int
S_getChannel
( mobj_t*         origin,
  sfxinfo_t*    sfxinfo );


int
S_AdjustSoundParams
( mobj_t*       listener,
  mobj_t*       source,
  int*          vol,
  int*          sep,
  int*          pitch );

void S_StopChannel(int cnum);

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init
( int           sfxVolume,
  int           musicVolume )
{  
  int           i;

  nosound = M_CheckParm("-nosound");
  if (nosound) return;
  I_Printf("S_Init: default volumes: SFX: %d, Music: %d, CD: %d\n",
     sfxVolume, musicVolume, snd_CDMusicVolume);

  S_SetSfxVolume(sfxVolume);
  S_SetMusicVolume(musicVolume);
  S_SetCDMusicVolume(snd_CDMusicVolume);

  // Allocating the internal channels for mixing
  // (the maximum numer of sounds rendered
  // simultaneously) within zone memory.
  channels =
    (channel_t *) Z_Malloc(numChannels*sizeof(channel_t), PU_STATIC, 0);
  
  // Free all channels for use
  for (i=0 ; i<numChannels ; i++)
    channels[i].sfxinfo = 0;
  
  // no sounds are playing, and they are not mus_paused
  mus_paused = 0;

  // Note that sounds have not been cached (yet).
  for (i=1 ; i<NUMSFX ; i++) {
    S_sfx[i].lumpnum = -1;
    S_sfx[i].usefulness = -1;
  }
}




//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//  Changed it to detect extra music eg D_E7M2 and change
// to that
void S_Start(void)
{
  int cnum;
  int mnum;
  char buffer[9];
  
  // start new music for the level
  if (nosound) return;
  mus_paused = 0;
  
  if (gamemission != doom) {
    sprintf(buffer, "D_MAP%02d", gamemap);
    mnum = W_CheckNumForName(buffer);
    if (mnum < 0) {
      mnum = mus_runnin + gamemap - 1;
      S_ChangeMusic(mnum, true);
    } else S_ChangeMusicbyName(buffer, true);
  }
  else
  {
    int spmus[]=
    {
      // Song - Who? - Where?
      
      mus_e3m4, // American     e4m1
      mus_e3m2, // Romero       e4m2
      mus_e3m3, // Shawn        e4m3
      mus_e1m5, // American     e4m4
      mus_e2m7, // Tim  e4m5
      mus_e2m4, // Romero       e4m6
      mus_e2m6, // J.Anderson   e4m7 CHIRON.WAD
      mus_e2m5, // Shawn        e4m8
      mus_e1m9  // Tim          e4m9
    };
    
    sprintf(buffer, "D_E%dM%d", gameepisode, gamemap);
    mnum = W_CheckNumForName(buffer);
    if (mnum < 0) {
      if (gameepisode == 4)
	mnum = spmus[gamemap-1];
      else
	mnum = mus_e1m1 + ((gameepisode-1)*9 + gamemap-1) % 36;
      S_ChangeMusic(mnum, true);
    } else S_ChangeMusicbyName(buffer, true);
  }   
  
  // HACK FOR COMMERCIAL
  //  if (commercial && mnum > mus_e3m9)        
  //      mnum -= mus_e3m9;
  
  
  // kill all playing sounds at start of level
  //  (trust me - a good idea)
  for (cnum=0 ; cnum<numChannels ; cnum++)
    if (channels[cnum].sfxinfo)
      S_StopChannel(cnum);
  
  nextcleanup = gametic + 15;
}       





int
S_StartSoundAtVolume
( mobj_t*         origin,
  int           sfx_id,
  int           volume )
{

  int           rc;
  int           sep;
  int           pitch;
  int           priority;
  sfxinfo_t*    sfx;
  int           cnum;
  
  if (nosound) return -1;
#ifdef DEVELOPERS
  // Debug.
  Debug_Printf(
	   "S_StartSoundAtVolume: playing sound %d (%s)\n",
	   sfx_id, S_sfx[sfx_id].name );
  
  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > NUMSFX)
    I_Error("Bad sfx #: %d", sfx_id);
#endif
  
  sfx = &S_sfx[sfx_id];
  
  // Initialize sound parameters
  if (sfx->link)
  {
    pitch = sfx->pitch;
    priority = sfx->priority;
    volume += sfx->volume;
    
    if (volume <= S_MIN_VOLUME)
      return -1;
    
    if (volume > S_MAX_VOLUME)
      volume = S_MAX_VOLUME;
  }     
  else
  {
    pitch = NORM_PITCH;
    priority = NORM_PRIORITY;
  }


  // Check to see if it is audible,
  //  and if not, modify the params
  if (origin && origin != players[consoleplayer].mo)
  {
    rc = S_AdjustSoundParams(players[consoleplayer].mo,
			     origin,
			     &volume,
			     &sep,
			     &pitch);
	
    if ( origin->x == players[consoleplayer].mo->x
	 && origin->y == players[consoleplayer].mo->y)
    {   
      sep       = NORM_SEP;
    }
    
    if (!rc)
      return -1;
  }     
  else
  {
    sep = NORM_SEP;
    volume <<= 4;
  }
  
  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup
      && sfx_id <= sfx_sawhit)
  {     
    pitch += 8 - (M_Random()&15);
    
    if (pitch<0)
      pitch = 0;
    else if (pitch>255)
      pitch = 255;
  }
  else if (sfx_id != sfx_itemup
	   && sfx_id != sfx_tink)
  {
    pitch += 16 - (M_Random()&31);
    
    if (pitch<0)
      pitch = 0;
    else if (pitch>255)
      pitch = 255;
  }

  // kill old sound
  if (origin)  S_StopSound(origin);

  // try to find a channel
  cnum = S_getChannel(origin, sfx);
  
  if (cnum<0)
    return -1;
#ifdef CACHESFX
  //
  // This is supposed to handle the loading/caching.
  // For some odd reason, the caching is done nearly
  //  each time the sound is needed?
  //
  
  // get lumpnum if necessary
  if (sfx->lumpnum < 0)
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

#ifndef LINUX
  // cache data if necessary
  sfx->data = I_CacheSFX(sfx_id);
#endif
  
  // increase the usefulness
  if (sfx->link && (sfx->link->usefulness++ < 0))
    sfx->link->usefulness = 1;
  else if (sfx->usefulness++ < 0)
    sfx->usefulness = 1;
#endif
  // Assigns the handle to one of the channels in the
  //  mix/output buffer.
  channels[cnum].handle = I_StartSound(sfx_id,
				       /*sfx->data,*/
				       volume,
				       sep,
				       pitch,
				       priority);
  return cnum;
}       

int
S_StartSound
( mobj_t*         origin,
  int           sfx_id )
{
  if (nosound) return -1;
  return S_StartSoundAtVolume(origin, sfx_id, S_MAX_VOLUME);
}




void S_StopSound(mobj_t *origin)
{

    int cnum;

    if (nosound) return;
    for (cnum=0 ; cnum<numChannels ; cnum++)
    {
	if (channels[cnum].sfxinfo && (channels[cnum].origin == origin))
	{
	    S_StopChannel(cnum);
	}
    }
}








extern int mus_pause_stop;
//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
  if (nosound) return;
    if (mus_playing && !mus_paused && mus_pause_stop)
    {
	I_PauseSong(mus_playing->handle);
	mus_paused = true;
    }
}

void S_ResumeSound(void)
{
  if (nosound) return;
    if (mus_playing && mus_paused && mus_pause_stop)
    {
	I_ResumeSong(mus_playing->handle);
	mus_paused = false;
    }
}


//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t* listener)
{
    int         audible;
    int         cnum;
    int         volume;
    int         sep;
    int         pitch;
    sfxinfo_t*  sfx;
    channel_t*  c;
    
  if (nosound) return;
#ifdef DJGPP
    // Clean up unused data.
    // This is currently not done for 16bit (sounds cached static).
    // DOS 8bit remains.  Note, I may try and implement SFX caching
    // again. - Kester
#ifdef CACHESFX
    if (gametic > nextcleanup)
    {
        int i;
	for (i=1 ; i<NUMSFX ; i++)
	{
            if (S_sfx[i].link) continue;
	    if (!S_sfx[i].usefulness)
	    {
 		    --S_sfx[i].usefulness;
                    I_DeCacheSFX(i);
            }
	}
	nextcleanup = gametic + 15;
    }
#endif
#endif
    for (cnum=0 ; cnum<numChannels ; cnum++)
    {
	c = &channels[cnum];
	sfx = c->sfxinfo;

	if (c->sfxinfo)
	{
	    if (I_SoundIsPlaying(c->handle))
	    {
		// initialize parameters
		volume = snd_SfxVolume;
		pitch = NORM_PITCH;
		sep = NORM_SEP;

		if (sfx->link)
		{
		    pitch = sfx->pitch;
		    volume += sfx->volume;
		    if (volume < 1)
		    {
			S_StopChannel(cnum);
			continue;
		    }
		    else if (volume > S_MAX_VOLUME)
		    {
			volume = S_MAX_VOLUME;
		    }
		}

		// check non-local sounds for distance clipping
		//  or modify their params
		if (c->origin && (listener != c->origin))
		{
		    audible = S_AdjustSoundParams(listener,
						  c->origin,
						  &volume,
						  &sep,
						  &pitch);
		    
		    if (!audible)
		    {
			S_StopChannel(cnum);
		    }
		    else
			I_UpdateSoundParams(c->handle, volume, sep, pitch);
		}
	    }
	    else
	    {
		// if channel is allocated but sound has stopped,
		//  free it
		S_StopChannel(cnum);
	    }
	}
    }
    I_UpdateSound();
}


void S_SetMusicVolume(int volume)
{
    // Clamp volume.  Errors suck.
    volume = mid(S_MIN_VOLUME, volume, S_MAX_VOLUME);
    I_SetMusicVolume(volume);
    snd_MusicVolume = volume;
}


void S_SetCDMusicVolume(int volume)
{
    volume = mid(S_MIN_VOLUME, volume, S_MAX_VOLUME);
    I_SetCDMusicVolume(volume);
    snd_CDMusicVolume = volume;
}

void S_SetSfxVolume(int volume)
{
    volume = mid(S_MIN_VOLUME, volume, S_MAX_VOLUME);
    I_SetSfxVolume(volume);
    snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
  if (nosound) return;
  S_ChangeMusic(m_id, false);
}

musicinfo_t playing_music = {"      ", 0, 0, -1};

void
S_ChangeMusicbyName
( char                 *name,
  int                   looping )
{
    musicinfo_t*        music;
    char                namebuf[9];

  if (nosound) return;
    music = &playing_music;
    name += 2;
    if (  (strncmp(music->name, name, 6) == 0) 
       && (mus_playing == music)) return;
    strncpy(music->name, name, 6);
    
    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    sprintf(namebuf, "D_%s", music->name);
    music->lumpnum = W_GetNumForName(namebuf);
    
    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum, PU_MUSIC);
    music->handle = I_RegisterSong(music->data);

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;
}
    
void
S_ChangeMusic
( int                   musicnum,
  int                   looping )
{
    musicinfo_t*        music;
    char                namebuf[9];

  if (nosound) return;
    if ( musicnum >= NUMMUSIC )
		I_Error("Bad music number %d", musicnum);
    
    if (musicnum >= 0) 
      music = &S_music[musicnum];
    else {
      music = &playing_music;
      music->lumpnum = -musicnum;
    }

    if (mus_playing == music)
	return;

    // shutdown old music
    S_StopMusic();
    // No music playing
    if (musicnum == mus_None) return;

    // get lumpnum if neccessary
    if ((!music->lumpnum) && (musicnum >= 0))
    {
	sprintf(namebuf, "d_%s", music->name);
	music->lumpnum = W_GetNumForName(namebuf);
    }

    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum, PU_MUSIC);
    music->handle = I_RegisterSong(music->data);

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;
}


void S_StopMusic(void)
{
  if (nosound) return;
    if (mus_playing)
    {
	if (mus_paused)
	    I_ResumeSong(mus_playing->handle);

	I_StopSong(mus_playing->handle);
	I_UnRegisterSong(mus_playing->handle);
	Z_ChangeTag(mus_playing->data, PU_CACHE);
	
	mus_playing->data = 0;
	mus_playing = 0;
    }
}




void S_StopChannel(int cnum)
{

    channel_t*  c = &channels[cnum];

  if (nosound) return;
    if (c->sfxinfo)
    {
	// stop the sound playing
	if (I_SoundIsPlaying(c->handle))
	{
	    I_StopSound(c->handle);
	}
#ifdef CACHESFX
	// degrade usefulness of sound data
        if (c->sfxinfo->link) c->sfxinfo->link->usefulness--;
	else c->sfxinfo->usefulness--;
#endif

	c->sfxinfo = 0;
    }
}



//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
// 2s here mean squared, eg S_CLOSE_DIST2 is S_CLOSE_DIST squared.
int
S_AdjustSoundParams
( mobj_t*       listener,
  mobj_t*       source,
  int*          vol,
  int*          sep,
  int*          pitch )
{
    fixed_t     approx_dist;
    fixed_t     adx;
    fixed_t     ady;
    angle_t     angle;

    // calculate the distance to sound origin
    //  and clip it if necessary
    adx = abs(listener->x - source->x)>>FRACBITS;
    ady = abs(listener->y - source->y)>>FRACBITS;

    // From _GG1_ p.428. Appox. eucledian distance fast.
//    approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);
    // Pythagoras.  Results are optimised: Square Root here cancels with square down there
    approx_dist = adx * adx + ady * ady;
    
    if (gamemap != 8
	&& approx_dist > S_CLIPPING_DIST2)
    {
	return 0;
    }
    
    // angle of source to listener
    angle = R_PointToAngle2(listener->x,
			    listener->y,
			    source->x,
			    source->y);

    if (angle > listener->angle)
	angle = angle - listener->angle;
    else
	angle = angle + (0xffffffff - listener->angle);

    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

    // volume calculation
    if (approx_dist < S_CLOSE_DIST2)
    {
	*vol = snd_SfxVolume << 4;
    }
    else if (gamemap == 8) // You can always hear the boss.
    {
	if (approx_dist > S_CLIPPING_DIST2)
	    approx_dist = S_CLIPPING_DIST2;

        // Kester's Physics Model (tm) v1.  Sounds cool.
        *vol = ((snd_SfxVolume << 4) * (S_CLIPPING_DIST2 - approx_dist)) / S_ATTENUATOR;
        *vol = mid(16, *vol, 255); // quietest volume is about 16
    }
    else
    {
        // Kester's Physics Model v1.
        *vol = ((snd_SfxVolume << 4) * (S_CLIPPING_DIST2 - approx_dist)) / S_ATTENUATOR;
        *vol = min(*vol, 255);
    }
    
    return (*vol > 0);
}




//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int
S_getChannel
( mobj_t*         origin,
  sfxinfo_t*    sfxinfo )
{
    // channel number to use
    int         cnum = -1, i;
    
    channel_t*  c = 0;

    // Find an open channel
    for (i = 0 ; i < numChannels ; i++)
    {
        if (sfxinfo->singularity && (sfxinfo == channels[i].sfxinfo))
          S_StopChannel(i);
	if ((cnum < 0) && (!channels[i].sfxinfo)) cnum = i;
        if (origin && (channels[i].origin == origin)) cnum = i;
    }

    // None available
    if (cnum == -1)// && (i == numChannels))
    {
	// Look for lower priority
	for (cnum=0 ; cnum<numChannels ; cnum++)
	    if (channels[cnum].sfxinfo->priority >= sfxinfo->priority) break;

	if (cnum == numChannels)
	{
	    // FUCK!  No lower priority.  Sorry, Charlie.    
	    return -1;
	}
	else
	{
	    // Otherwise, kick out lower priority.
	    S_StopChannel(cnum);
	}
    }

    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}




