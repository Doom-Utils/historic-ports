/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: s_sound.c,v 1.10 1999/10/12 13:01:14 cphipps Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:  Platform-independent sound code
 *
 *-----------------------------------------------------------------------------*/

static const char
rcsid[] = "$Id: s_sound.c,v 1.10 1999/10/12 13:01:14 cphipps Exp $";

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#include "doomstat.h"
#include "s_sound.h"
#include "i_sound.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"
#include "lprintf.h"

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200<<FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).

#define S_CLOSE_DIST (160<<FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128
#define S_STEREO_SWING (96<<FRACBITS)

//jff 1/22/98 make sound enabling variables readable here
extern int snd_card, mus_card;
extern boolean nosfxparm, nomusicparm;
//jff end sound enabling variables readable here

typedef struct
{
  sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  void *origin;        // origin of sound
  int handle;          // handle of the sound being played
  int is_pickup;       // killough 4/25/98: whether sound is a player's weapon
} channel_t;

// the set of channels available
static channel_t *channels;

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int snd_MusicVolume = 15;

// whether songs are mus_paused
static boolean mus_paused;

// music currently being played
static musicinfo_t *mus_playing;

// following is set
//  by the defaults code in M_misc:
// number of channels available
int numChannels;

//jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Internals.
//

void S_StopChannel(int cnum);

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                        int *vol, int *sep, int *pitch);

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, int is_pickup);

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
  //jff 1/22/98 skip sound init if sound not enabled
  if (snd_card && !nosfxparm)
  {
    int i;
    
    lprintf(LO_CONFIRM, "S_Init: default sfx volume %d\n", sfxVolume);

    // Whatever these did with DMX, these are rather dummies now.
    I_SetChannels();

    S_SetSfxVolume(sfxVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    // CPhipps - calloc
    channels =
      (channel_t *) Z_Calloc(numChannels,sizeof(channel_t), PU_STATIC, 0);

    // Note that sounds have not been cached (yet).
    for (i=1 ; i<NUMSFX ; i++)
      S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
  }

  // CPhipps - music init reformatted
  if (mus_card && !nomusicparm) {
    S_SetMusicVolume(musicVolume);

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;
  }
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
  int cnum,mnum;

  // kill all playing sounds at start of level
  //  (trust me - a good idea)

  //jff 1/22/98 skip sound init if sound not enabled
  if (snd_card && !nosfxparm)
    for (cnum=0 ; cnum<numChannels ; cnum++)
      if (channels[cnum].sfxinfo)
        S_StopChannel(cnum);

  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  // start new music for the level
  mus_paused = 0;

  if (idmusnum!=-1)
    mnum = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
  else
    if (gamemode == commercial)
      mnum = mus_runnin + gamemap - 1;
    else
      {
        static const int spmus[] =     // Song - Who? - Where?
        {
          mus_e3m4,     // American     e4m1
          mus_e3m2,     // Romero       e4m2
          mus_e3m3,     // Shawn        e4m3
          mus_e1m5,     // American     e4m4
          mus_e2m7,     // Tim  e4m5
          mus_e2m4,     // Romero       e4m6
          mus_e2m6,     // J.Anderson   e4m7 CHIRON.WAD
          mus_e2m5,     // Shawn        e4m8
          mus_e1m9      // Tim          e4m9
        };

        if (gameepisode < 4)
          mnum = mus_e1m1 + (gameepisode-1)*9 + gamemap-1;
        else
          mnum = spmus[gamemap-1];
      }
  S_ChangeMusic(mnum, true);
}

void S_StartSoundAtVolume(void *origin_p, int sfx_id, int volume)
{
  int sep, pitch, priority, cnum, is_pickup;
  sfxinfo_t *sfx;
  mobj_t *origin = (mobj_t *) origin_p;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  is_pickup = sfx_id & PICKUP_SOUND || sfx_id == sfx_oof; // killough 4/25/98
  sfx_id &= ~PICKUP_SOUND;

  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > NUMSFX)
    I_Error("Bad sfx #: %d", sfx_id);

  sfx = &S_sfx[sfx_id];

  // Initialize sound parameters
  if (sfx->link)
    {
      pitch = sfx->pitch;
      priority = sfx->priority;
      volume += sfx->volume;

      if (volume < 1)
        return;

      if (volume > snd_SfxVolume)
        volume = snd_SfxVolume;
    }
  else
    {
      pitch = NORM_PITCH;
      priority = NORM_PRIORITY;
    }

  // Check to see if it is audible, modify the params
  // killough 3/7/98, 4/25/98: code rearranged slightly

  if (!origin || origin == players[displayplayer].mo)
    sep = NORM_SEP;
  else
    if (!S_AdjustSoundParams(players[displayplayer].mo, origin, &volume,
                             &sep, &pitch))
      return;
    else
      if ( origin->x == players[displayplayer].mo->x &&
           origin->y == players[displayplayer].mo->y)
        sep = NORM_SEP;

  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
    pitch += 8 - (M_Random()&15);
  else
    if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
      pitch += 16 - (M_Random()&31);

  if (pitch<0)
    pitch = 0;

  if (pitch>255)
    pitch = 255;

  // kill old sound
  for (cnum=0 ; cnum<numChannels ; cnum++)
    if (channels[cnum].sfxinfo && channels[cnum].origin == origin &&
        channels[cnum].is_pickup == is_pickup)
      {
        S_StopChannel(cnum);
        break;
      }

  // try to find a channel
  cnum = S_getChannel(origin, sfx, is_pickup);

  if (cnum<0)
    return;

  // get lumpnum if necessary
  // killough 2/28/98: make missing sounds non-fatal
  if (sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
    return;

  // increase the usefulness
  if (sfx->usefulness++ < 0)
    sfx->usefulness = 1;

  // Assigns the handle to one of the channels in the mix/output buffer.
  channels[cnum].handle = I_StartSound(sfx_id, volume, sep, pitch, priority);
}

void S_StartSound(void *origin, int sfx_id)
{
  S_StartSoundAtVolume(origin, sfx_id, snd_SfxVolume);
}

void S_StopSound(void *origin)
{
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  for (cnum=0 ; cnum<numChannels ; cnum++)
    if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
      {
        S_StopChannel(cnum);
        break;
      }
}


//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing && !mus_paused)
    {
      I_PauseSong(mus_playing->handle);
      mus_paused = true;
    }
}

void S_ResumeSound(void)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing && mus_paused)
    {
      I_ResumeSong(mus_playing->handle);
      mus_paused = false;
    }
}


//
// Updates music & sounds
//
void S_UpdateSounds(void* listener_p)
{
  mobj_t *listener = (mobj_t*) listener_p;
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  for (cnum=0 ; cnum<numChannels ; cnum++)
    {
      sfxinfo_t *sfx;
      channel_t *c = &channels[cnum];
      if ((sfx = c->sfxinfo))
        {
          if (I_SoundIsPlaying(c->handle))
            {
              // initialize parameters
              int volume = snd_SfxVolume;
              int pitch = NORM_PITCH;
              int sep = NORM_SEP;

              if (sfx->link)
                {
                  pitch = sfx->pitch;
                  volume += sfx->volume;
                  if (volume < 1)
                    {
                      S_StopChannel(cnum);
                      continue;
                    }
                  else
                    if (volume > snd_SfxVolume)
                      volume = snd_SfxVolume;
                }

              // check non-local sounds for distance clipping
              // or modify their params
              if (c->origin && listener_p != c->origin) { // killough 3/20/98
                if (!S_AdjustSoundParams(listener, c->origin,
                                         &volume, &sep, &pitch))
                  S_StopChannel(cnum);
                else
                  I_UpdateSoundParams(c->handle, volume, sep, pitch);
	      }
            }
          else   // if channel is allocated but sound has stopped, free it
            S_StopChannel(cnum);
        }
    }
}

void S_SetMusicVolume(int volume)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;
  if (volume < 0 || volume > 15)
    I_Error("Attempt to set music volume at %d", volume);
  I_SetMusicVolume(volume);
  snd_MusicVolume = volume;
}

void S_SetSfxVolume(int volume)
{
  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;
  if (volume < 0 || volume > 127)
    I_Error("Attempt to set sfx volume at %d", volume);
  snd_SfxVolume = volume;
}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;
  S_ChangeMusic(m_id, false);
}

void S_ChangeMusic(int musicnum, int looping)
{
  musicinfo_t *music;

  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (musicnum <= mus_None || musicnum >= NUMMUSIC)
    I_Error("Bad music number %d", musicnum);

  music = &S_music[musicnum];

  if (mus_playing == music)
    return;

  // shutdown old music
  S_StopMusic();

  // get lumpnum if neccessary
  if (!music->lumpnum)
    {
      char namebuf[9];
      sprintf(namebuf, "d_%s", music->name);
      music->lumpnum = W_GetNumForName(namebuf);
    }

  // load & register it
  music->data = W_CacheLumpNum(music->lumpnum);
  music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;
}


void S_StopMusic(void)
{
  //jff 1/22/98 return if music is not enabled
  if (!mus_card || nomusicparm)
    return;

  if (mus_playing)
    {
      if (mus_paused)
        I_ResumeSong(mus_playing->handle);

      I_StopSong(mus_playing->handle);
      I_UnRegisterSong(mus_playing->handle);
      W_UnlockLumpNum(mus_playing->lumpnum); // cph - release the music data

      mus_playing->data = 0;
      mus_playing = 0;
    }
}

void S_StopChannel(int cnum)
{
  int i;
  channel_t *c = &channels[cnum];

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return;

  if (c->sfxinfo)
    {
      // stop the sound playing
      if (I_SoundIsPlaying(c->handle))
        I_StopSound(c->handle);

      // check to see
      //  if other channels are playing the sound
      for (i=0 ; i<numChannels ; i++)
        if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
          break;

      // degrade usefulness of sound data
      c->sfxinfo->usefulness--;
      c->sfxinfo = 0;
    }
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

int S_AdjustSoundParams(mobj_t *listener, mobj_t *source,
                        int *vol, int *sep, int *pitch)
{
  fixed_t adx, ady,approx_dist;
  angle_t angle;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return 0;

  // calculate the distance to sound origin
  //  and clip it if necessary
  adx = abs(listener->x - source->x);
  ady = abs(listener->y - source->y);

  // From _GG1_ p.428. Appox. eucledian distance fast.
  approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

  if (!approx_dist)  // killough 11/98: handle zero-distance as special case
    {
      *sep = NORM_SEP;
      *vol = snd_SfxVolume;
      return *vol > 0;
    }

  if (approx_dist > S_CLIPPING_DIST)
    return 0;

  // angle of source to listener
  angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

  if (angle <= listener->angle)
    angle += 0xffffffff;
  angle -= listener->angle;
  angle >>= ANGLETOFINESHIFT;

  // stereo separation
  *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

  // volume calculation
  if (approx_dist < S_CLOSE_DIST)
    *vol = snd_SfxVolume;
  else
    // distance effect
    *vol = (snd_SfxVolume * ((S_CLIPPING_DIST-approx_dist)>>FRACBITS))
      / S_ATTENUATOR;

  return (*vol > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
// killough 4/25/98: made static, added is_pickup argument

static int S_getChannel(void *origin, sfxinfo_t *sfxinfo, int is_pickup)
{
  // channel number to use
  int cnum;
  channel_t *c;

  //jff 1/22/98 return if sound is not enabled
  if (!snd_card || nosfxparm)
    return -1;

  // Find an open channel
  for (cnum=0; cnum<numChannels && channels[cnum].sfxinfo; cnum++)
    if (origin && channels[cnum].origin == origin &&
        channels[cnum].is_pickup == is_pickup)
      {
        S_StopChannel(cnum);
        break;
      }

    // None available
  if (cnum == numChannels)
    {      // Look for lower priority
      for (cnum=0 ; cnum<numChannels ; cnum++)
        if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
          break;
      if (cnum == numChannels)
        return -1;                  // No lower priority.  Sorry, Charlie.
      else
        S_StopChannel(cnum);        // Otherwise, kick out lower priority.
    }

  c = &channels[cnum];              // channel is decided to be cnum.
  c->sfxinfo = sfxinfo;
  c->origin = origin;
  c->is_pickup = is_pickup;         // killough 4/25/98
  return cnum;
}

//----------------------------------------------------------------------------
//
// $Log: s_sound.c,v $
// Revision 1.10  1999/10/12 13:01:14  cphipps
// Changed header to GPL
//
// Revision 1.9  1999/03/01 07:51:29  cphipps
// Fix obvious stupid stuff in S_SetMusicVolume, like bad volume range
// and volume being set to 127 before any change. Thanks to Rafael for
// pointing this out
//
// Revision 1.8  1999/01/01 13:03:14  cphipps
// Fix releasing music lump problem
//
// Revision 1.7  1999/01/01 10:30:24  cphipps
// Music no longer retained in memory
// Wad lump handling updated
//
// Revision 1.6  1998/12/24 10:25:59  cphipps
// Minor fixes from MBF
// Reformatted S_Init slightly
//
// Revision 1.5  1998/10/27 19:19:35  cphipps
// Fix typo
//
// Revision 1.4  1998/10/27 18:56:15  cphipps
// Logical output console stuff for Boom v2.02 update patched in
//
// Revision 1.3  1998/10/20 14:45:28  cphipps
// Pass length to I_RegisterSong
//
// Revision 1.2  1998/10/16 22:00:57  cphipps
// Removed hanging else
//
// Revision 1.1  1998/09/13 16:49:50  cphipps
// Initial revision
//
// Revision 1.11  1998/05/03  22:57:06  killough
// beautification, #include fix
//
// Revision 1.10  1998/04/27  01:47:28  killough
// Fix pickups silencing player weapons
//
// Revision 1.9  1998/03/23  03:39:12  killough
// Fix spy-mode sound effects
//
// Revision 1.8  1998/03/17  20:44:25  jim
// fixed idmus non-restore, space bug
//
// Revision 1.7  1998/03/09  07:32:57  killough
// ATTEMPT to support hearing with displayplayer's hears
//
// Revision 1.6  1998/03/04  07:46:10  killough
// Remove full-volume sound hack from MAP08
//
// Revision 1.5  1998/03/02  11:45:02  killough
// Make missing sounds non-fatal
//
// Revision 1.4  1998/02/02  13:18:48  killough
// stop incorrect looping of music (e.g. bunny scroll)
//
// Revision 1.3  1998/01/26  19:24:52  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/23  01:50:49  jim
// Added music/sound options, and enables
//
// Revision 1.1.1.1  1998/01/19  14:03:04  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
