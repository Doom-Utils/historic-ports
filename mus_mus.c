//
// DOSDoom MUS Support Code
//
// By the DOSDoom Team (Mostly -KM-)
// 
// These routines provide MUS Music to DOOM.
//

// For struct defines and function prototypes
#include "i_music.h"
#include "mus_mus.h"
#include "m_swap.h"
#include "i_alleg.h"

// For midi drivers and timer routines
//#include "allegro.h"
void update_controllers(void);

// For memcpy routines
#include <string.h>

#include "m_argv.h"
#include "i_system.h"
#include "z_zone.h"

#include "dm_state.h"

// Speed of the MUS player system.  Default is BPS_TO_TIMER(140)
#define MUS_TIC_SPEED BPS_TO_TIMER(140)

// -----------  Local Variables  --------------

// Some bytes that when sent to the MIDI device stops all its sound... useful for when
// a song stops to cut off hanging notes; also resets controllers to defaults
extern unsigned char all_sound_off[6];

// patches = 0 means midi patches are loaded dynamically.  (latency before song plays, while
// patches are loaded from disk.)
// patches = 1 means midi patches are loaded statically. (no latency, but more memory is used)
// patch memory is locked and can't be paged out, and if the patches are multimegabyte, can
// leave little memory for the game on low memory systems (8mb systems - mine :-( 
extern int patches;

// handle of the song currently playing
struct MUShandle_s *MUSplaying = 0;

// music volume
extern int snd_MusicVolume;


// -----------  Local Prototypes  --------------

// Loads the MIDI patches for song 'handle' into memory
int MUS_LoadPatches(struct MUShandle_s *handle);

// Reads a MIDI/MUS packed time from song 'handle'
long MUS_getTime(struct MUShandle_s *handle);

// -----------  Function Definitions -------------

// If specified music is playing returns 1; else 0
boolean MUS_QrySongPlaying(struct MUShandle_s *handle)
{
  // Song is playing.
  if (handle && ((handle->playControl & PLAY) == PLAY))
       return true;
 
 // Song is not playing
 return false;
}

// Changes the music volume range 0 - 255
void MUS_SetMusicVolume(int volume)
{
  // Allegro Set Volume. sfx_volume = don't care
  set_volume(-1, volume );
}

// like the || button on ya stereo
void MUS_PauseSong(struct MUShandle_s *handle)
{
  if (handle == NULL) return;  
  handle->playControl |= PAUSE;
}

// unpauses a paused song
void MUS_ResumeSong(struct MUShandle_s *handle)
{
  if (handle == NULL) return;  
  handle->playControl &= ~PAUSE;
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
struct MUShandle_s *MUS_RegisterSong(void *data)
{
  struct MUShandle_s *rc;
  
  // If there is no midi device there is not much point in trying to
  // play music....
  if (midi_card == MIDI_NONE) return 0;

  rc = Z_Malloc(sizeof(struct MUShandle_s), PU_STATIC, NULL);
  
  // Check if it IS a MUS file
  rc->MUSheader.id = LONG(*((long *)data));
  if (rc->MUSheader.id != 0x1a53554d) return 0;

  // Setup default values
  rc->playControl = 0;
  rc->MUSdata = data;
  
  // Load the MUS file header
  rc->MUSheader.scoreLen = SHORT(*(((short *)data) + 2));
  rc->datapointer = rc->MUSheader.scoreStart = SHORT(*(((short *)data) + 3));
  rc->MUSheader.channels = SHORT(*(((short *)data) + 4));  // primary channels
  rc->MUSheader.channels2 = SHORT(*(((short *)data) + 5)); // count secondary channels
  rc->MUSheader.instrCnt = SHORT(*(((short *)data) + 6)); // number of instruments
  rc->MUSheader.dummy = SHORT(*(((short *)data) + 7));
  rc->MUSheader.instruments = ((short *)data) + 8;
  rc->timer = 0;
  
  _go32_dpmi_lock_data(rc->MUSdata, 
     rc->MUSheader.scoreLen + rc->MUSheader.scoreStart);
  _go32_dpmi_lock_data(rc, sizeof(*rc));

  return rc;
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void MUS_PlaySong(struct MUShandle_s *handle, int looping)
{
  if (handle == NULL) return;  
  
  // Because doom 2 title music leaves a note hanging...
  if (handle != MUSplaying) MUS_StopSong(MUSplaying);

  // Dynamic Patch loading required
  if (patches == 0) MUS_LoadPatches(handle);

  // Set the flags for PLAY
  handle->playControl |= PLAY | ((looping == 0) ? 0 : LOOP);
  MUSplaying = handle;
}

// like the # button on ya stereo
void MUS_StopSong(struct MUShandle_s *handle)
{
  if (handle == NULL) return;  

  // Set the flags for not PLAY
  handle->playControl &= ~PLAY;

  // Rewind the song to the start
  handle->datapointer 
	 = handle->MUSheader.scoreStart;

  // Kill all sound, reset all controllers
  for (  all_sound_off[0] = all_sound_off[3] = 0xb0; 
	 all_sound_off[0] <= 0xbf; 
	 all_sound_off[0]++, all_sound_off[3]++)
	     midi_out(all_sound_off, sizeof(all_sound_off));
  update_controllers();
  
  if (handle == MUSplaying)
    MUSplaying = 0;
}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void MUS_UnRegisterSong(struct MUShandle_s *handle)
{
  if (handle == NULL) return;  
  // We wouldn't want it to keep playing...
  MUS_StopSong(handle);
  
  _unlock_dpmi_data(handle->MUSdata, 
     handle->MUSheader.scoreStart + handle->MUSheader.scoreLen);
  _unlock_dpmi_data(handle, sizeof(*handle));

  Z_Free(handle);
}

// Why did I choose such long names? :-)
#define MUSDATA handle->MUSdata
#define DP handle->datapointer

// Reads a MIDI/MUS packed value
long MUSgetTime(struct MUShandle_s *handle)
{
  long ret_val;
  int i;

  // reads a variable length byte:
  // maximum 4 bytes read or bit 7 not set
  for (i = 4, ret_val = 0; 
   (MUSDATA[DP] & 128) 
   && i--;
   )
  {
    ret_val += MUSDATA[DP++] & 127;
    ret_val <<= 7;
  }
  ret_val += MUSDATA[DP++] & 127;
  return ret_val;
};
END_OF_FUNCTION(MUSgetTime);
 
#define UPDATE_CONTROLLERS 0x40

// Where the real work gets done.  Takes a MUS encoded event, decodes it,
// reencodes it as a MIDI event, then sends it to the MIDI driver (in Allegro)
// (which decodes it...)
int MUS_HandleEvent(struct MUShandle_s *handle)
{
  unsigned char channel, lastevent = 0;
  long pitchwheel;
  unsigned char midimsg[4];
  
  if (handle == NULL) return 0;  
  // (PLAY | PAUSE) != PLAY implies !PAUSED
  if ((handle->playControl & (PLAY | PAUSE)) != PLAY) return MUS_TIC_SPEED;

  // Do all events; until last event bit set
  do { 
	 
    // Store the last_event bit for comparison later
    lastevent |= MUSDATA[DP] & 0x80;
	 
    // Isolate the channel
    midimsg[0] = channel = MUSDATA[DP] & 0xF;
    
    // Switch channels 16 (MUS drum) & 10 (MIDI Drum)
    if (midimsg[0] == 0xF) { midimsg[0] = 0x9; channel = 0x9; }
    else if (midimsg[0] == 0x9) { midimsg[0] = 0xF; channel = 0xF; }
	 
    // Check the event
    switch (MUSDATA[DP++] & 0x70) {
      
      case 0: // Note Off
	midimsg[0] |= 0x80; // Note off
	midimsg[1] = MUSDATA[DP++]; // Note Number
	midimsg[2] = 0x40; // Velocity (64)
	midi_out(midimsg, 3);
	break;
	 
      case 0x10: // Note On
	midimsg[0] |= 0x90; // Note On
	midimsg[1] = (MUSDATA[DP] & 0x7F); // Note Number
	
	// Check for volume data
	if ((MUSDATA[DP++] & 0x80) == 0x80) 
	  handle->lastVol[channel] = MUSDATA[DP++];
	
	midimsg[2] = handle->lastVol[channel]; // Velocity
	midi_out(midimsg, 3);
	break;
      
      case 0x20: // Pitch Wheel

	// Scale to MIDI Pitch range
	pitchwheel = MUSDATA[DP++];
	pitchwheel *= 16384; 
	pitchwheel /= 256;           
	
	// Assemble to 14-bit MIDI pitch value
	midimsg[0] |= 0xE0;
	midimsg[1] = (pitchwheel & 7);
	midimsg[2] = (pitchwheel & 0x3F80) >> 7;
	midi_out(midimsg, 3);

	// Update the controllers (later)
	lastevent |= UPDATE_CONTROLLERS;
	break;
       
      case 0x30: // System event
	
	// A control change event with no extra data byte
	midimsg[0] |= 0xB0;
	
	switch (MUSDATA[DP++]) {
	  case 10: midimsg[1] = 0x78; break;
	  case 11: midimsg[1] = 0x7b; break;
	  case 12: midimsg[1] = 0x7e; break;
	  case 13: midimsg[1] = 0x7f; break;
	  case 14: midimsg[1] = 0x79; break;
	}
	midimsg[2] = 0;
	midi_out(midimsg, 3);
	lastevent |= UPDATE_CONTROLLERS;
	break;
	 
     case 0x40: // Controller Change
	// Map MUS controllers to MIDI controllers
	channel = 3;  // 3 data bytes
	midimsg[3] = MUSDATA[DP++];
	midimsg[2] = MUSDATA[DP++]; 
	midimsg[1] = midimsg[2];
	midimsg[0] |= 0xb0;  // probably control change
	switch (midimsg[3]) {
	  
	  // Program Change
	  case 0: midimsg[0] = 0xC0 + (midimsg[0] & 0xF); channel = 2; break;
	  
	  // Bank change
	  case 1: midimsg[1] = 0; break;

	  case 2: midimsg[1] = 1; break;

	  // Volume controller
	  case 3: midimsg[1] = 7; break;

	  // Pan controller
	  case 4: midimsg[1] =0xa; break;

	  case 5: midimsg[1] = 0xb; break;
	  case 6: midimsg[1] = 0x5b; break;
	  case 7: midimsg[1] = 0x5d; break;
	  case 8: midimsg[1] = 0x40; break;
	  case 9: midimsg[1] = 0x43; break;

	  // Uh Oh....
	  default:
	     channel = -1; break;
	}
	if (channel > 0) midi_out(midimsg, channel);
	lastevent |= UPDATE_CONTROLLERS;
	break;
	
      default: // Running unhandled events (types 5 and 7) into an end of
	       // song effectively stops the player from crashing.
	       // If we run into a event type 5 of 7 we will gracefully loop to
	       // start.

      case 0x60: // End of song

	// If the loop flag is set, change the datapointer to the start
	// of the score; else set the datapointer to the start of the 
	// score and clear the play bit
	DP = handle->MUSheader.scoreStart;

	// Unless looping, stop play.
	if ((handle->playControl & LOOP) == 0) {
		handle->playControl &= ~PLAY;
		lastevent = 0x80;
	}
	break;
       
     } // Switch event_type
     
     // If the last_event bit was set we can leave
  } while (!(lastevent & 0x80));

  // Do we update the controllers? This condenses streams of controller data into 
  // a few updates.  Also makes controllers work with Allegro
  if (lastevent & UPDATE_CONTROLLERS) update_controllers();     

  // And get the time until the next event
  if (DP != handle->MUSheader.scoreStart) 
	     handle->timer = MUSgetTime(handle);
  else handle->timer = 0; // Just Ended a song
  
  return handle->timer * MUS_TIC_SPEED;  
}
END_OF_FUNCTION(MUS_HandleEvent);

// Finds the MIDI patches needed for the specified file
// and tells the MIDI driver to load them.
static void MUS_LoadPatchesSlow(struct MUShandle_s *handle, char *patches, char *drums)
{
  unsigned char channel, lastevent = 0;

  // Set all patches to unrequired
  for (channel = 128; channel--;)
    patches[channel] = drums[channel] = FALSE;

  patches[0] = TRUE; // Always Load Piano
  
  // Do all events; until last event bit set
  do { 
	 
    // Store the last_event bit for comparison later
    lastevent = MUSDATA[DP] & 0x80;
	 
    // Isolate the channel
    channel = MUSDATA[DP] & 0xF;
    
    // Check the event
    switch (MUSDATA[DP++] & 0x70) {
      
      case 0x30: // System event
      case 0x20: // Pitch Wheel
      case 0: // Note Off; ignore
	DP++;
	break;
	 
      case 0x10: // Note On; is it a drum?
	if (channel == 0xf) {
	  drums[MUSDATA[DP] & 0x7F] = TRUE;
        }
	// Check for volume data
	if ((MUSDATA[DP++] & 0x80) == 0x80) 
	  DP++;
	break;
	 
     case 0x40: // Controller Change
	if (MUSDATA[DP++] == 0) {
		patches[MUSDATA[DP]] = TRUE;
	}
	DP++;
	break;
	
      default: // Running unhandled events (types 5 and 7) into an end of
	       // song effectively stops the player from crashing.
	       // If we run into a event type 5 of 7 we will gracefully loop to
	       // start.

      case 0x60: // End of song

	// If the loop flag is set, change the datapointer to the start
	// of the score; else set the datapointer to the start of the 
	// score and clear the play bit
	DP = handle->MUSheader.scoreStart;
	lastevent = 0x40;
	break;
       
     } // Switch event_type
     if (lastevent & 0x80)
	     MUSgetTime(handle);
     
     // If the last_event bit was set we can leave
  } while (!(lastevent & 0x40));
}

// Finds the MIDI patches needed for the specified file
// and tells the MIDI driver to load them.
int MUS_LoadPatches(struct MUShandle_s *handle)
{
  char patches[128], drums[128];
  int i;

  if (handle == NULL) return 1;  
  
  // Set all patches to unrequired
  for (i = 128; i--;)   
    patches[i] = drums[i] = FALSE;

  patches[0] = TRUE; // Always Load Piano

  // Step through the instrument list, marking the required patches
  for (i = handle->MUSheader.instrCnt; i--;) {

    // Check if it is a patch to be loaded...
    if (handle->MUSheader.instruments[i] <= 127) {
      patches[handle->MUSheader.instruments[i]] = TRUE;

    } else if (handle->MUSheader.instruments[i] <= 181) { 

      // ... or a drum (patches 135 - 181) to be loaded.
      drums[handle->MUSheader.instruments[i] - 100] = TRUE;

    } else {
      
      // This file is screwed !
      // Load them the hard way
      MUS_LoadPatchesSlow(handle, patches, drums);
      break;
    }
  }

  // Send them through the midi driver.
  return midi_driver->load_patches(patches, drums);
}


       
// Inits defaults, install MUSTicker etc...
void MUS_Init(void)
{
  LOCK_FUNCTION(MUSgetTime);
  LOCK_FUNCTION(MUS_HandleEvent);
}

