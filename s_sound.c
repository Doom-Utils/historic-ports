//
// DOSDoom Sound FX Handling Code
//
// Based on the Doom Source Code,
//
// Released by Id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -KM- 1998/09/27 Sounds.ddf. nosound is now global.  Reduced the pitching a little.
//
#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "i_sound.h"
#include "lu_sound.h"
#include "s_sound.h"

#include "z_zone.h"
#include "m_random.h"
#include "w_wad.h"

#include "d_debug.h"
#include "dm_defs.h"
#include "p_local.h"
#include "m_argv.h"

#include "dm_state.h"

// -KM- 1999/01/31 VSOUND, the speed of sound. Guessed at 500.
#define VSOUND                    (500<<16)

#define S_MAX_VOLUME            15
#define S_MIN_VOLUME		0
// when to clip out sounds
// Does not fit the large outdoor areas. 1600 ** 2
// -KM- 1998/09/27 Moved S_CLIPPING_DIST to s_sound.h
#define S_CLIPPING_DIST2        (S_CLIPPING_DIST * S_CLIPPING_DIST)
// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
#define S_CLOSE_DIST2		(S_CLOSE_DIST * S_CLOSE_DIST)
//(S_CLIPPING_DIST2-S_CLOSE_DIST2)
#define S_ATTENUATOR            (S_CLIPPING_DIST2 - S_CLOSE_DIST2)

// Adjustable by menu.

#define NORM_PITCH              128
#define NORM_PRIORITY           64
#define NORM_SEP                128

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;

// -KM- 1998/09/01 Pitch shifting
extern unsigned char rndtable[];
static int srndindex = 0;

// If true, sound system is disabled/not working.
// Changed to false if sound init ok.
boolean nosound = true;

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t*      sfxinfo;

    // origin of sound
    mobj_t*         origin;

    // pitch sound was started at
    // -KM- 1999/01/31 Record for Doppler shift.
    int             pitch;

    // handle of the sound being played
    int             handle;
    
} channel_t;


// the set of channels available
static channel_t*       channels;

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int             snd_SfxVolume = 8;

// Maximum volume of music.
int             snd_MusicVolume = 8;



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


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init( int sfxVolume, int musicVolume )
{  
  int           i;

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
  for (i=1 ; i<numsfx ; i++)
  {
    S_sfx[i].lumpnum = -1;
    S_sfx[i].usefulness = -1;
  }
}

//
// S_Start
//
// Pre-level startup code. Kills playing sounds at start of level,
// determines music if any and changes music.
//
// -ACB- 1998/08/09 Uses currentmap to select music used.
//
void S_Start(void)
{
  int cnum;

  // start new music for the level
  if (nosound)
    return;

  mus_paused = 0;

  S_ChangeMusic(currentmap->music, true);

  // kill all playing sounds at start of level (trust me - a good idea)
  for (cnum=0; cnum<numChannels; cnum++)
  {
    if (channels[cnum].sfxinfo)
      S_StopChannel(cnum);
  }

  nextcleanup = gametic + 15;
}       

//
// Calculates the Doppler Shift to apply due to two moving objects
// Doppler shift: f = f0 * VSOUND / (VSOUND - v)
//   where f is the observed frequency
//         f0 is the emitted frequency
//         VSOUND is the speed of sound
//         v is the relative velocity (objects moving away have a negative velocity)
//
// -KM- 1999/01/31
//
static inline fixed_t S_DopplerShift(mobj_t* listener, mobj_t* source)
{
  // The objects' speed relative to each other
  fixed_t speed;
  // Angle between the two objects
  angle_t angle;
  angle_t sangle, langle;
  // Angle between angle and an object's angle
  angle_t incidence;

  angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

  langle = R_PointToAngle2(0, 0, listener->momx, listener->momy);

  incidence = (langle - angle) >> ANGLETOFINESHIFT;
  speed = FixedMul(P_AproxDistance(listener->momx, listener->momy), finecosine[incidence]);

  if (source->thinker.function.acv)
  {
    sangle = R_PointToAngle2(0, 0, source->momx, source->momy);
    incidence = (sangle - angle) >> ANGLETOFINESHIFT;
    speed += FixedMul(P_AproxDistance(source->momx, source->momy), finecosine[incidence]);
  }

  return FixedDiv(VSOUND, VSOUND - speed);
}

//
// S_StartSoundAtVolume
//
// -ACB- 1998/08/10 Altered Error Messages 
//
// -KM- 1998/09/01 Looping support
int S_StartSoundAtVolume (mobj_t* origin, int sfx_id, int volume)
{

  int           rc;
  int           sep;
  int           pitch;
  int           priority;
  sfxinfo_t*    sfx;
  int           cnum;
// -KM- 1998/09/01 SFX Looping
  boolean       looping = false;
  
  if (nosound) return -1;

  // Debug.
  Debug_Printf("playing sound %d (%s)\n", sfx_id, S_sfx[sfx_id].name );
  
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

  pitch += ((rndtable[srndindex++] - 128) / 3) + 872;
  srndindex &= 255;

  if (origin && sfx->looping)
     looping = true;
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
    volume = min(volume, S_MAX_VOLUME);
    // -KM- 1998/07/31 use full range of volume
    volume *= 17;
  }
  
  // try to find a channel
  cnum = S_getChannel(origin, sfx);
  
  if (cnum<0)
    return -1;

  channels[cnum].pitch = pitch;
  if (origin && origin != players[consoleplayer].mo)
    pitch = (pitch * S_DopplerShift(players[consoleplayer].mo, origin)) >> 16;

  //
  // This handles the loading/caching.
  //
  
  // get lumpnum if necessary
  if (sfx->lumpnum < 0)
    sfx->lumpnum = I_GetSfxLumpNum(sfx);

#ifndef LINUX
  // cache data if necessary
  sfx->data = I_CacheSFX(sfx_id);
#endif
  
  // increase the usefulness
  if (sfx->link)
  {
    if (sfx->link->usefulness++ < 0)
      sfx->link->usefulness = 1;
  } else if (sfx->usefulness++ < 0)
    sfx->usefulness = 1;
  // Assigns the handle to one of the channels in the
  //  mix/output buffer.
  channels[cnum].handle = I_StartSound(sfx_id,
				       /*sfx->data,*/
				       volume,
				       sep,
				       pitch,
				       priority,
                                       looping);
  return cnum;
}       

int S_StartSound(mobj_t* origin, sfx_t* sound_id)
{
  int rnd;
  if (nosound) return -1;
  // -KM- 1998/11/25 Fixed this, added origin check
  if (!sound_id)
  {
    if (origin)
      S_StopSound(origin);
    return -1;
  }
  // -KM- 1999/01/31 Using P_Random here means demos and net games get out of
  //  sync.
  rnd = rndtable[srndindex++];
  srndindex &= 255;
  return S_StartSoundAtVolume(origin, sound_id->sounds[rnd%sound_id->num],
                              snd_SfxVolume);
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
// -KM- 1998/09/01 Pitch shifting
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
  // Clean up unused data.
  // This is currently not done for 16bit (sounds cached static).
  // DOS 8bit remains.  Note, I may try and implement SFX caching
  // again. - Kester
  if (gametic > nextcleanup)
  {
      int i;
      for (i=1 ; i<numsfx ; i++)
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
		pitch = c->pitch;
		sep = NORM_SEP;

		if (sfx->link)
		{
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
		    
                    pitch = (pitch * S_DopplerShift(listener, c->origin)) >> 16;

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
/*
void S_StartMusic(int m_id)
{
  if (nosound) return;
  S_ChangeMusic(m_id, false);
}
*/
musicinfo_t playing_music = {"        ", 0, 0, -1};

//
// S_ChangeMusic
//
// This changes the music by the lump, not by an enum reference, this is
// used for DDF defined levels and the new music cheat.
//
// -ACB- 1998/08/09 Removed "D_" addition (unnecessary). although all music
//                  should be begin with "D_", all refs should ideally include
//                  it.
//
void S_ChangeMusic (char *name, int looping)
{
    musicinfo_t* music;

    if (nosound)
      return;

    music = &playing_music;
    if ((strcmp(music->name, name) == 0) && (mus_playing == music))
      return;

    // shutdown old music
    S_StopMusic();

    strcpy(music->name, name);

    // get lumpnum if neccessary
    music->lumpnum = W_GetNumForName(music->name);
    
    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum, PU_MUSIC);
    music->handle = I_RegisterSong(music->data);

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;
}

//
// S_ChangeMusic
//
// Change the current music playing, use enum as a reference.
//
/*
void S_ChangeMusic (int musicnum, int looping)
{
    musicinfo_t*        music;
    char                namebuf[9];

    if (nosound)
      return;

    if ( musicnum >= NUMMUSIC )
      I_Error("Bad music number %d", musicnum);
    
    if (musicnum >= 0)
    {
      music = &S_music[musicnum];
    }
    else
    {
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
*/

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
	// degrade usefulness of sound data
        if (c->sfxinfo->link) c->sfxinfo->link->usefulness--;
	else c->sfxinfo->usefulness--;

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
    // Pythagoras.  Results are optimised:
    //                                    Square Root here cancels with square
    //                                    down there
    approx_dist = adx * adx + ady * ady;
    
   /* if (currentmap->flags & MPF_BOSSLEVEL
	&& approx_dist > S_CLIPPING_DIST2)
    {
	return 0;
    } */
    
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
        // -KM- 1998/07/31 Use full range of volume
	*vol *= 17;
    }
    /* else if (gamemap == 8) // You can always hear the boss.
    {
	if (approx_dist > S_CLIPPING_DIST2)
	  approx_dist = S_CLIPPING_DIST2;

        // Kester's Physics Model (tm) v1.1  Sounds cool.
        // -KM- 1998/07/31 Use full dynamic range
        *vol = ((*vol * 17) * (S_CLIPPING_DIST2 - approx_dist)) / S_ATTENUATOR;
        *vol = mid(16, *vol, 255); // quietest volume is about 16
    }     */
    else
    {
        // Kester's Physics Model v1.1
        // -KM- 1998/07/31 Use Full dynamic range
        *vol = ((*vol * 17) * (S_CLIPPING_DIST2 - approx_dist)) / S_ATTENUATOR;
        *vol = min(*vol, 255);
    }
    
    return (*vol > 0);
}




//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int S_getChannel (mobj_t* origin, sfxinfo_t* sfxinfo)
{
    // channel number to use
    int         cnum = -1, i;
    int         channel_priority = 0;
    
    channel_t*  c = 0;

    // Find an open channel
    // -KM- 1998/12/16 New SFX code.
    for (i = 0 ; i < numChannels ; i++)
    {
        // -KM- 1998/12/17 Fixed new SFX code: added check against
        //   channels[i].sfxinfo == NULL
        if (channels[i].origin == origin && channels[i].sfxinfo)
        {
           if (channels[i].sfxinfo->singularity == sfxinfo->singularity)
           {
             if (sfxinfo->singularity)
             {
               cnum = i;
               channel_priority = 1;
               S_StopChannel(i);
               break;
             }
           } /*else if (origin && channel_priority <= 1)
           {
             cnum = i;
             channel_priority = 1;
           }   */
        }
	if ((!channel_priority) && (!channels[i].sfxinfo)) cnum = i;
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




