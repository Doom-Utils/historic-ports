// File: i_music.c                   by Kester Maddock (dmaddock@xtra.co.nz)
// DOOM Music subsystems, 
// These routines provide MIDI Music to DOOM.

// For struct defines and function prototypes
#include "i_music.h"
#include "m_swap.h"

// For midi drivers and timer routines
#include "allegro.h"
void update_controllers(void);
MIDI *load_midi_data(char *dat);

// For memcpy routines
#include <string.h>

#include "m_argv.h"
#include "i_system.h"

#include <bcd.h>
#include "mus_mus.h"
#include "mus_mod.h"
#include "mus_mp3.h"

#include "doomstat.h"

#include "z_zone.h"

// Magic numbers: the numbers that identify what type of file it is we are trying
// to play.
#define MUS_MAGIC 0x1a53554d
#define MIDI_MAGIC 0x6468544d

// Speed of the MUS player system.  Default is BPS_TO_TIMER(140)
#define TIC_SPEED BPS_TO_TIMER(35)

// -----------    Local Types    --------------
typedef struct MusicList_s {
   // Type of Song
   SongTypes_t SongType;
   
   // Marks this handle as free/used
   int slotUsed;

   // Pointer to data
   void *data;

   // Timer
   signed int timer;
} MusicList_t;

// -----------  Local Variables  --------------

// The list of Registered songs, and its counter
MusicList_t *MusicList = NULL;
int MusicListLen = 0;

// handle of the song currently playing
int SongPlaying = -1;

// Type of song currently playing
SongTypes_t SongType = M_MUS;

// Sync music to display
boolean SongSync = false;

// music volume
extern int snd_MusicVolume;
int snd_CDMusicVolume;

// patches = 0 means midi patches are loaded dynamically.  (latency before song plays, while
// patches are loaded from disk.)
// patches = 1 means midi patches are loaded statically. (no latency, but more memory is used)
// patch memory is locked and can't be paged out, and if the patches are multimegabyte, can
// leave little memory for the game on low memory systems (8mb systems - mine :-( 
int patches;

// Some bytes that when sent to the MIDI device stops all its sound... useful for when
// a song stops to cut off hanging notes; also resets controllers to defaults
unsigned char all_sound_off[6] = {0xB0, 123, 0, 0xB0, 121, 0};

// CD-Audio Stuff
cdType_t cdaudio;
int cdtrack;
int cdnumtracks;
int cdcounter = 0;
boolean cdlooping = false;

// -----------  Function Definitions -------------

// If specified music is playing returns 1; else 0
int I_QrySongPlaying(int handle)
{
 if ((handle == SongPlaying) && (MusicList[handle].slotUsed))
 switch (MusicList[handle].SongType)
 {
  case M_MIDI:
    // MIDI music
    return (midi_pos != -1);

  case M_MUS:
    // Song is playing.
    return MUS_QrySongPlaying((struct MUShandle_s *) MusicList[handle].data);
    break;

  case M_MOD:
    return MOD_QrySongPlaying((JGMOD *) MusicList[handle].data);
    break;

  case M_MP3:
    return MP3_QrySongPlaying((struct MP3_handle_t *) MusicList[handle].data);
    break;

  case M_NONE:
    break;

  // Unimplemented song types
  default:
    return 0;
 }
 
 // Song is not playing
 return 0;
}

// Close down the music player
void I_ShutdownMusic(void)
{
  if (cdaudio)
    {
    bcd_stop();
    bcd_close();
    }
  // This is done automatically by Allegro, but...
  remove_int(MusicTicker);
}

// Changes the music volume range 0 - 15
void I_SetMusicVolume(int volume)
{
  // Limit the volume to sensible values
  volume = MIN(volume, 15);
  volume = MAX(volume, 0);

//  snd_MusicVolume = volume;
  volume <<= 4;
  switch ( SongType )
  {
    case M_MUS:
      MUS_SetMusicVolume(volume);
      break;
    case M_MIDI:
      // Allegro Set Volume. sfx_volume = don't care
      set_volume(-1, volume);
      break;
    case M_MOD:
      MOD_SetMusicVolume(volume);
      break;
    case M_MP3:
      MP3_SetMusicVolume(volume);
      break;
    default:
      break;
  }
}

// Changes the music volume range 0 - 15
void I_SetCDMusicVolume(int volume)
{
  // Limit the volume to sensible values
  volume = MIN(volume, 15);
  volume = MAX(volume, 0);

//  snd_CDMusicVolume = volume;
  if (cdaudio & CD_ON) bcd_set_volume(volume << 4);
}

// like the || button on ya stereo
void I_PauseSong(int handle)
{
 // CD Music
 if (cdaudio & CD_ON) bcd_pause();

 if ((handle >= 0) && (handle < MusicListLen) && // Check handle in range
    (MusicList[handle].slotUsed == 1))  // Check music at slot
 switch (MusicList[handle].SongType) 
 {
  // MIDI Music
  case M_MIDI:
    midi_pause();
    break;
  
  // MUS Music
  case M_MUS:
    MUS_PauseSong((struct MUShandle_s *) MusicList[handle].data);
    break;

  // MOD, S3M, XM Music
  case M_MOD:
    MOD_PauseSong((JGMOD *) MusicList[handle].data);
    break;

  case M_MP3:
    MP3_PauseSong();
    break;

  case M_NONE:
    break;
  
  // Unimplemented music types
  default:
    break;
 }
 I_SetMusicVolume(0);
}

// unpauses a paused song
void I_ResumeSong(int handle)
{
 // CD Music
 if (cdaudio & CD_ON) bcd_resume();

 if ((handle >= 0) && (handle < MusicListLen) && // Check handle in range
    (MusicList[handle].slotUsed == 1))  // Check music at slot
 switch(MusicList[handle].SongType) 
 {
  // MIDI Music
  case M_MIDI:
    midi_resume();
    break;

  // MUS Music
  case M_MUS:
    MUS_ResumeSong((struct MUShandle_s *) MusicList[handle].data);
    break;
  
  // MOD X3M XM Music
  case M_MOD:
    MOD_ResumeSong((JGMOD *) MusicList[handle].data);
    break;

  case M_MP3:
    MP3_ResumeSong();

  case M_NONE:
    break;
  
  // Unimplemented music types
  default:
    break;
 }
 I_SetMusicVolume(snd_MusicVolume);
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
int I_RegisterSong(void *data)
{
  int slot;

  // Can't play midi music and cd music at the same time :-)
  if (cdaudio == CD_ON) return -1;

  // Find a free slot
  for (slot = MusicListLen; slot--;)
    if (MusicList[slot].slotUsed == 0) break;
  
  // No Handles Free.  
  if (slot < 0) {
    slot = MusicListLen++;
    MusicList = Z_ReMalloc(MusicList, sizeof(MusicList_t) * MusicListLen);
  }
     
  MusicList[slot].data = (void *) MUS_RegisterSong(data);
  MusicList[slot].SongType = M_MUS;
  if (MusicList[slot].data == NULL) {
    MusicList[slot].data = (void *) load_midi_data(data);
    MusicList[slot].SongType = M_MIDI;
  }
  if (MusicList[slot].data == NULL) {
    MusicList[slot].data = (void *) MOD_RegisterSong(data);
    MusicList[slot].SongType = M_MOD;
  }
  if (MusicList[slot].data == NULL) {
    MusicList[slot].data = (void *) MP3_RegisterSong(data);
    MusicList[slot].SongType = M_MP3;
  }
  if (MusicList[slot].data == NULL) return -1;
  MusicList[slot].slotUsed = 1;
  MusicList[slot].timer = 0;
  return slot;
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void I_PlaySong(int handle, int looping)
{
  // CD-Audio
  if (cdaudio == CD_ON) return;

  if ((handle >= 0) && (handle < MusicListLen) && // Check handle in range
     (MusicList[handle].slotUsed == 1))  // Check music at slot
  {
  // Because doom 2 title music leaves a note hanging...
  if (handle != SongPlaying) I_StopSong(SongPlaying);

  switch(MusicList[handle].SongType)  
  {
    // MIDI Music
    case M_MIDI:
      play_midi( (MIDI *) MusicList[handle].data, looping);
      break;

    // MUS Music
    case M_MUS:
      MUS_PlaySong((struct MUShandle_s *) MusicList[handle].data, looping);
      install_int_ex(MusicTicker, TIC_SPEED);
      break;
  
    case M_MOD:
      MOD_PlaySong((JGMOD *) MusicList[handle].data, looping);
      break; 

    case M_MP3:
      MP3_PlaySong((struct MP3_handle_t *) MusicList[handle].data, looping);
      break;

    case M_NONE:
      break;

    // Unimplemented
    default:
      break;
  } // switch
  
  SongPlaying = handle;
  SongType = MusicList[SongPlaying].SongType;

  // Because each song type has it's own volume
  I_SetMusicVolume(snd_MusicVolume);
  SongSync = true;
  I_PauseSong(SongPlaying);
  
  } // rangecheck
}

// For Syncing to display
void I_SongSync(void)
{
 if (SongSync) {
   SongSync = false;
   I_ResumeSong(SongPlaying);
 }
}

// like the # button on ya stereo
void I_StopSong(int handle)
{
  if ((handle >= 0) && (handle < MusicListLen) && // Check handle in range
     (MusicList[handle].slotUsed == 1)) { // Check music at slot
  remove_int(MusicTicker);
  switch(MusicList[handle].SongType)
  {
    case M_MIDI:
      stop_midi();
      break;

    case M_MUS:
      MUS_StopSong((struct MUShandle_s *) MusicList[handle].data);
      break;
    
    case M_MOD:
      MOD_StopSong((JGMOD *) MusicList[handle].data);
      break;

    case M_MP3:
      MP3_StopSong((struct MP3_handle_t *) MusicList[handle].data);
      break;

    case M_NONE:
      break;

    default:
      break;
  } 
  SongPlaying = -1;
  SongType = M_NONE;
  }
}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void I_UnRegisterSong(int handle)
{
  if ((handle >= 0) && (handle < MusicListLen)  &&// Check handle in range
     (MusicList[handle].slotUsed == 1))  { // Check music at slot
  switch (MusicList[handle].SongType)
  {
    // MIDI Music
    case M_MIDI:
      destroy_midi((MIDI *) MusicList[handle].data);
      break;

    // MUS Music
    case M_MUS: 
      MUS_UnRegisterSong((struct MUShandle_s *) MusicList[handle].data);
      break;

    case M_MOD:
      MOD_UnRegisterSong((JGMOD *) MusicList[handle].data);
      break;

    case M_MP3:
      MP3_UnRegisterSong((struct MP3_handle_t *) MusicList[handle].data);
      break;

    case M_NONE:
      break;

    default:
      break;
  }
  // Mark this slot as free.
  MusicList[handle].slotUsed = 0;  
  }
}

// Called every so often to send events to the Music device
void MusicTicker(void)
{
  // Check that a song is actually playing...
  if ((SongPlaying < 0) || !MusicList[SongPlaying].slotUsed)
      return; // No Music playing
  switch (SongType) 
  {
    case M_MUS:
       // If the timer has run out (which it always has) send some notes to the hardware
       // and get a new time  
       if (MusicList[SongPlaying].timer <= 0) 
	MusicList[SongPlaying].timer = MUS_HandleEvent((struct MUShandle_s *)
	   MusicList[SongPlaying].data);
       else MusicList[SongPlaying].timer -= TIC_SPEED;
      break;
  
    case M_NONE:  
      break;

    default:
      break;
  }
  // Changing the speed of the interrupt takes load off the CPU and transfers
  // it to hardware that was designed for it. (Programmable Interval Timer)
  if (MusicList[SongPlaying].timer > 0) {
    install_int_ex(MusicTicker, MusicList[SongPlaying].timer);
    MusicList[SongPlaying].timer = 0;
  } else install_int_ex(MusicTicker, TIC_SPEED);
}
END_OF_FUNCTION(MusicTicker)

void CD_Next(void)
{
     int play = 0;
     // All this modulo arithmatic shit means the cd loops
     // Finds the next audio track: won't play data tracks
     if (!cdlooping)
     for (play = 0; play <= cdnumtracks; play++)
       if (bcd_track_is_audio(((cdtrack + play) % cdnumtracks) + 1)) {
	 cdtrack = ((cdtrack + play) % cdnumtracks) + 1;
	 break;
       }
     if (play > cdnumtracks) return;
     bcd_play_track(cdtrack);
     cdcounter = TICRATE * 30;
}

void CD_Prev(void)
{
     int play;
     // All this modulo arithmatic means the cd loops
     // Searches backward to find the next audio track: won't play data
     // tracks
     for (play = cdnumtracks - 2; play; play--)
       if (bcd_track_is_audio(((cdtrack + play) % cdnumtracks) + 1)) {
	 cdtrack = ((cdtrack + play) % cdnumtracks) + 1;
	 break;
       }
     if (play < 0) return;
     bcd_play_track(cdtrack);
     cdcounter = TICRATE * 30; // 30 seconds.
}

void CD_Play(int track, boolean looping)
{
  cdlooping = false;
  cdtrack = track - 1;
  CD_Next();
  cdlooping = looping;
}

static void CheckCD(void)
{
  if ((cdaudio & CD_ON) && (cdcounter <= 0)) {

    // Find out if the disc has been changed
    int play = bcd_disc_changed();
    // This means the cd is only checked once per second
    cdcounter = TICRATE;
    if (play == BCD_DISC_CHANGED) {
      cdnumtracks = bcd_get_audio_info();
      cdtrack = 0;
      return;
    } else if (play == BCD_DISC_UNKNOWN) {
      cdtrack = 0;
      cdnumtracks = 0;
      return;
    }

    // Find out if the current track has finished playing
    play = bcd_now_playing();
    if (!play) CD_Next();
  }
}

void I_MusicTicker2(void)
{
  CheckCD();
  if ((SongPlaying < 0) || !MusicList[SongPlaying].slotUsed)
      return; // No Music playing
  switch (SongType) 
  {
    case M_MP3:
       // mp3 support goes here... 
       MP3_Ticker();
       break;
    default:
       break;
  }
}  

// Inits defaults, install MUSTicker etc...
void I_InitMusic(void)
{
  int i;
  
  MusicList = Z_Malloc(sizeof(MusicList_t), PU_MUSIC, NULL);
  MusicListLen = 1;
  // Mark all slots free
  for (i = MusicListLen; i--;) 
    MusicList[i].slotUsed = 0;

  // No song playing
  SongPlaying = -1;
  SongType = M_NONE;

  // Load the MIDI patches, if the -patches parameter is present.
  // Pros: You don't have to wait that second when the song changes
  //       Some broken songs that have a corrupted instrument list will 
  //       work properly
  // Cons: It can take a lot of memory, if you are using Allegro's software 
  //       wavetable sytnth.  A decent patch bank (Creative Labs' 2mbgmgs.sf2) 
  //       is about 3mb, and this can cause problems on systems (like mine) 
  //       with only 8 mb of ram.  It is not helped by the fact that DJ's 
  //       malloc (though extremely fast) rounds memory request up to a power 
  //       of 2.  In this size region, 3 mb rounds to 4mbs, and DOOM's default 
  //       internal heap rounds to 8 mbs.  This is probably why large
  //       heap sizes fail.  They are only effective in sizes of 2 ^ x mbs.

  patches = get_config_int("sound", "cache_all_patches", false);
  if (M_CheckParm("-patches")) patches = true;
  if (patches) {
    I_Printf ( "I_InitMusic: Loading General MIDI Patch Set.\n\r");
    load_midi_patches();
  } else I_Printf ( "I_InitMusic: Patches will be loaded dynamically.\n\r");
  cdaudio = get_config_int("sound", "cdaudio", CD_OFF);
  if (M_CheckParm("-cdaudio")) cdaudio = CD_ON;
  if (M_CheckParm("-cdatmosphere")) cdaudio = CD_ON | CD_ATMOS;

  if (cdaudio & CD_ON)
  {
    int starttrack = 1;
    if (bcd_open()==0) {
      I_Printf ( "Unable to access CD-ROM drive!  Defaulting to normal music\n\r");
      cdaudio = CD_OFF;
    } else if ((cdnumtracks=bcd_get_audio_info())==0) {
      I_Printf ( "Error getting audio info - Insert a music CD!\n\r");
      cdtrack = 0;
    } else {
      for (i=1;i<=cdnumtracks;i++)
	if (bcd_track_is_audio(i))
	{
	  starttrack=i;
	  break;
	}
      if (starttrack>cdnumtracks) {
	  I_Printf ( "No audio tracks! - Insert a music CD!\n\r");
	  cdtrack = 0;
      } else {
	  I_Printf ( "Starting CD-Audio from track %d\n\r",starttrack);
	  cdtrack=starttrack - 1;
	  if (bcd_audio_busy()) bcd_stop();
      }
    }
  }


  // If memory is paged out to disk, it can not be paged in in an interrupt context, causing
  // the machine to bomb out to dos.
  LOCK_VARIABLE(MusicList);
  LOCK_VARIABLE(MusicListLen);
  LOCK_VARIABLE(SongType);
  LOCK_VARIABLE(SongPlaying);
  LOCK_FUNCTION(MusicTicker);

  MUS_Init();
  MOD_Init();
  MP3_Init();

  // Call the MusicTicker 140 times per second (by default) to send music to the MIDI device
  install_int_ex(MusicTicker, TIC_SPEED);

  // Reset the MIDI device to a default state: 
	       //(all sound off, reset controllers)
  for (  all_sound_off[0] = all_sound_off[3] = 0xb0; 
	 all_sound_off[0] <= 0xbf; 
	 all_sound_off[0]++, all_sound_off[3]++)
	     midi_out(all_sound_off, sizeof(all_sound_off));
  update_controllers();
}


