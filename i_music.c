// File: i_music.c                   by Kester Maddock (dmaddock@xtra.co.nz)
// DOOM Music subsystems,
// These routines provide MIDI Music to DOOM.

// For struct defines and function prototypes
#include "doomdef.h"
#include "i_music.h"
#include "doomdef.h"

// For midi drivers and timer routines
#include "allegro.h"
#ifdef NO_CONTROLLERS
void update_controllers(void);
void update_controllers(void) {};
#else
extern void update_controllers(void);
#endif

// For memcpy routines
#include <string.h>

// Uncoment one of these depending on your taste
#include <conio.h>
// #define cprintf printf

//#ifndef TEST_MUSICS
//#include "m_argv.h"
//#else
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
//#endif

//#include <bcd.h>
//#include "doomstat.h"

// In S_Sound.c, when a song is stopped it is removed from the queue, meaning
// only one song is ever playing at once, but what the heck...
#define MAX_MUS 4

// -----------    Local Types    --------------
struct MUSheader_s {
  long id; //identifier "MUS" 0x1A; we will use 0x1a53554d
  unsigned short scoreLen;
  unsigned short scoreStart;
  unsigned short channels;  // primary channels
  unsigned short channels2; // count secondary channels
  unsigned short instrCnt; // number of instruments
  unsigned short dummy;
  unsigned short *instruments;
};

struct MUSRegisteredList_s {
  struct MUSheader_s MUSheader;
  int datapointer;
  int slotUsed;
  int playControl; // uses the bit controls defined in i_music.h
  signed int timer; // tics to delay until next event.
  unsigned char *MUSdata;
  char lastVol[16];
};

// -----------  Local Variables  --------------

// Some bytes that when sent to the MIDI device stops all its sound... useful for when
// a song stops to cut off hanging notes; also resets controllers to defaults
static unsigned char all_sound_off[] = {0xB0, 123, 0, 0xB0, 121, 0};

// The list of Registered songs, and its counter
struct MUSRegisteredList_s MUSRegisteredList[MAX_MUS];
int MUSRListLen = MAX_MUS;

// Some place to copy the song's data to, with a known size that can be locked...
// pretty important if the data is touched from an interrupt.
static unsigned char MUSdata[65536];

// patches = 0 means midi patches are loaded dynamically.  (latency before song plays, while
// patches are loaded from disk.)
// patches = 1 means midi patches are loaded statically. (no latency, but more memory is used)
// patch memory is locked and can't be paged out, and if the patches are multimegabyte, can
// leave little memory for the game on low memory systems (8mb systems - mine :-(
static int patches = 0;

// handle of the song currently playing
int MUSplaying;

// music volume
extern int snd_MusicVolume;

// -----------  Local Prototypes  --------------

// Loads the MIDI patches for song 'handle' into memory
int MUS_LoadPatches(int handle);

// Reads a MIDI/MUS packed time from song 'handle'
long MUSgetTime(int handle);

// Plays the next event(s) in song 'handle'
void HandleMUSEvent(int handle);

// -----------  Function Definitions -------------

// If specified music is playing returns 1; else 0
int I_QrySongPlaying(int handle)
{
  if ((handle == MUSplaying) && (MUSRegisteredList[handle].slotUsed) &&
   ((MUSRegisteredList[handle].playControl & PLAY) == PLAY)) return 1;
  return 0;
}

// Close down the music player
void I_ShutdownMusic(void)
{
  // This is done automatically by Allegro, but...
  remove_int(MusicTicker);
  //Z_Free(MUSRegisteredList);
}

// Changes the music volume range 0 - 15
void I_SetMusicVolume(int volume)
{
  volume = MIN(volume, 15);
  volume = MAX(volume, 0);
  snd_MusicVolume = volume;
  // Allegro Set Volume.
  set_volume(-1, volume * 16);
}

// like the || button on ya stereo
void I_PauseSong(int handle)
{
//if (cdaudio==1)
//  {
//  bcd_pause();
// }
//else
//  {
  if ((handle >= 0) && (handle < MUSRListLen))  // Check handle in range
    if (MUSRegisteredList[handle].slotUsed == 1)  // Check music at slot
      MUSRegisteredList[handle].playControl |= PAUSE;
//  }
}

// unpauses a paused song
void I_ResumeSong(int handle)
{
//if (cdaudio==1)
//  {
//  bcd_resume();
//  }
//else
//  {
  if ((handle >= 0) && (handle < MUSRListLen))  // Check handle in range
    if (MUSRegisteredList[handle].slotUsed == 1)  // Check music at slot
      MUSRegisteredList[handle].playControl &= ~PAUSE;
//  }
}

// Places the song in the internal database, inits its paramaters and
// returns a handle to control the song later
int I_RegisterSong(void *data)
{
  int i;
  for (i = 0; i < MUSRListLen; i++) { // Find a free slot
    if (MUSRegisteredList[i].slotUsed == 0) break;
  }
  if (i == MUSRListLen) return -1; // No Handles Free.
  /*if (i == MUSRListLen) {
    MUSRListLen++;
    MUSRegisteredList =
      realloc(MUSRegisteredList, sizeof(struct MUSRegisteredList_s) * MUSRListLen);
  } */
  MUSRegisteredList[i].playControl = 0;
  MUSRegisteredList[i].slotUsed = 1;
  MUSRegisteredList[i].MUSdata = data;

  // Check if it IS a MUS file
  MUSRegisteredList[i].MUSheader.id = *((long *)data);
  if (MUSRegisteredList[i].MUSheader.id != 0x1a53554d) return -1;

  // Load the MUS file header
  MUSRegisteredList[i].MUSheader.scoreLen = *(((short *)data) + 2);
  MUSRegisteredList[i].MUSheader.scoreStart = *(((short *)data) + 3);
  MUSRegisteredList[i].MUSheader.channels = *(((short *)data) + 4);  // primary channels
  MUSRegisteredList[i].MUSheader.channels2 = *(((short *)data) + 5); // count secondary channels
  MUSRegisteredList[i].MUSheader.instrCnt = *(((short *)data) + 6); // number of instruments
  MUSRegisteredList[i].MUSheader.dummy = *(((short *)data) + 7);
  MUSRegisteredList[i].MUSheader.instruments = ((short *)data) + 8;
  MUSRegisteredList[i].datapointer = MUSRegisteredList[i].MUSheader.scoreStart;
  MUSRegisteredList[i].timer = 0;
  return i;
}

// like the > button on ya stereo
// if looping is nonzero, will play forever
void I_PlaySong(int handle, int looping)
{
  if ((handle >= 0) && (handle < MUSRListLen))  // Check handle in range
    if (MUSRegisteredList[handle].slotUsed == 1) {// Check music at slot
      if (patches == 0) MUS_LoadPatches(handle);
      // Because doom 2 title music leaves a note hanging...
      if (handle != MUSplaying) I_StopSong(MUSplaying);
      memcpy(MUSdata, MUSRegisteredList[handle].MUSdata,
         MUSRegisteredList[handle].MUSheader.scoreLen +
         MUSRegisteredList[handle].MUSheader.scoreStart);
      MUSRegisteredList[handle].playControl |= PLAY | ((looping == 0) ? 0 : LOOP);
      MUSplaying = handle;
    }
}

// like the # button on ya stereo
void I_StopSong(int handle)
{
  if ((handle >= 0) && (handle < MUSRListLen) && // Check handle in range
     (MUSRegisteredList[handle].slotUsed == 1)) { // Check music at slot
      MUSRegisteredList[handle].playControl &= ~PLAY;
      MUSplaying = -1;
      MUSRegisteredList[handle].datapointer
         = MUSRegisteredList[handle].MUSheader.scoreStart;
      for (  all_sound_off[0] = all_sound_off[3] = 0xb0;
         all_sound_off[0] <= 0xbf;
         all_sound_off[0]++, all_sound_off[3]++)
             midi_out(all_sound_off, sizeof(all_sound_off));
      update_controllers();
  }
}

// Marks the slot used as free, meaning a call to I_RegisterSong will
// reuse this spot
void I_UnRegisterSong(int handle)
{
  if ((handle >= 0) && (handle < MUSRListLen))  // Check handle in range
    if (MUSRegisteredList[handle].slotUsed == 1)  // Check music at slot
       MUSRegisteredList[handle].slotUsed = 0;
}

// Called every so often to send events to the Music device
void MusicTicker(void)
{
  if (MUSplaying < 0) return; // No Music playing
  // Check there is music in the slot, (PLAY | PAUSE) == PLAY implies ~PAUSED
  if (MUSRegisteredList[MUSplaying].slotUsed &&
  ((MUSRegisteredList[MUSplaying].playControl & (PLAY | PAUSE)) == PLAY)) {
    if (MUSRegisteredList[MUSplaying].timer <= 0) HandleMUSEvent(MUSplaying);
    // Changing the speed of the interrupt takes load off the CPU and transfers
    // it to hardware that was designed for it.
    if (MUSRegisteredList[MUSplaying].timer != 0) {
      install_int_ex(MusicTicker, BPS_TO_TIMER(140)
        * MUSRegisteredList[MUSplaying].timer);
      MUSRegisteredList[MUSplaying].timer = 0;
    } else install_int_ex(MusicTicker, BPS_TO_TIMER(140));
  } // If play
}
END_OF_FUNCTION(MusicTicker)

// Why did I choose such long names? :-)
#define MUSDATA MUSdata
#define DP MUSRegisteredList[handle].datapointer

// Reads a MIDI/MUS packed value
long MUSgetTime(int handle)
{
  long ret_val;
  int i;
  // reads a variable length byte:
  // adds bytes until 4 bytes read or bit 7 not set
  for (i = 0, ret_val = 0;
   (MUSDATA[DP] & 128)
   && (i < 4);
   i++)
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
void HandleMUSEvent(int handle)
{
  unsigned char channel, lastevent = 0;
  long pitchwheel;
  unsigned char midimsg[4];
  do { // Do all events; until last event bit set

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
          MUSRegisteredList[handle].lastVol[channel] = MUSDATA[DP++];

        midimsg[2] = MUSRegisteredList[handle].lastVol[channel]; // Velocity
        midi_out(midimsg, 3);
        break;

      case 0x20: // Pitch Wheel
        pitchwheel = MUSDATA[DP++];
        pitchwheel *= 16384; // Scale to MIDI Pitch range
        pitchwheel /= 256;
        // Assemble to 14-bit MIDI pitch value
        midimsg[0] |= 0xE0;
        midimsg[1] = (pitchwheel & 7);
        midimsg[2] = (pitchwheel & 0x3F80) >> 7;
        midi_out(midimsg, 3);
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
          case 0: midimsg[0] |= 0xC0; midimsg[0] &= 0xCF; channel = 2; break;
          case 1: midimsg[1] = 0; break;
          case 2: midimsg[1] = 1; break;
          case 3: midimsg[1] = 7; break;
          case 4: midimsg[1] =0xa; break;
          case 5: midimsg[1] = 0xb; break;
          case 6: midimsg[1] = 0x5b; break;
          case 7: midimsg[1] = 0x5d; break;
          case 8: midimsg[1] = 0x40; break;
          case 9: midimsg[1] = 0x43; break;
        }
        midi_out(midimsg, channel);
        lastevent |= UPDATE_CONTROLLERS;
        break;

      default: // Running unhandled events (types 5 and 7) into an end of
               // song effectively stops the player from crashing
      case 0x60: // End of song
        // If the loop flag is set, change the datapointer to the start
        // of the score; else set the datapointer to the start of the
        // score and clear the play bit
        DP = MUSRegisteredList[handle].MUSheader.scoreStart;
        // Unless looping, stop play.
        if ((MUSRegisteredList[handle].playControl & LOOP) == 0) {
                MUSRegisteredList[handle].playControl &= ~PLAY;
                lastevent = 0x80;
        }
        break;

     } // Switch event_type

     // If the last_event bit was set we can leave
     //if (lastevent == 0x80) break;
  } while (!(lastevent & 0x80)); // for (;;)
  if (lastevent & UPDATE_CONTROLLERS) update_controllers();
  // And get the time until the next event
  if (DP != MUSRegisteredList[handle].MUSheader.scoreStart)
             MUSRegisteredList[handle].timer += MUSgetTime(handle);
  else MUSRegisteredList[handle].timer = 0; // Just Ended a song

}
END_OF_FUNCTION(HandleMUSEvent);

// Finds the MIDI patches needed for the specified file
// and tells the MIDI driver to load them.
int MUS_LoadPatches(int handle)
{
  char patches[128], drums[128];
  int i;
  // Set all patches to unrequired
  for (i = 0; i < 128; i++)
    patches[i] = drums[i] = FALSE;
  patches[0] = TRUE; // Always Load Piano
  // Step through the instrument list, marking the required patches
  for (i = 0; i < MUSRegisteredList[handle].MUSheader.instrCnt; i++) {
    // Check if it is a patch to be loaded...
    if (MUSRegisteredList[handle].MUSheader.instruments[i] <= 127) {
      patches[MUSRegisteredList[handle].MUSheader.instruments[i]] = TRUE;
    } else { // ... or a drum (patches 135 - 181) to be loaded.
      drums[MUSRegisteredList[handle].MUSheader.instruments[i] - 100] = TRUE;
    }
  }
  // Send them through the midi driver.
  return midi_driver->load_patches(patches, drums);
}

// Inits defaults, install MUSTicker etc...
void I_InitMusic(void)
{
  int i;
  // Mark all slots free
  for (i = 0; i < MUSRListLen; i++)
    MUSRegisteredList[i].slotUsed = 0;
  // No song playing
  MUSplaying = -1;
  install_timer(); // Could be installed elsewhere
  // #defines are for if this module is not to be linked with DOOM.  Module can be linked
  // with the seperate MUSplay utility as a command line tool to play .MUS files.
#ifndef TEST_MUSICS
  // Load the MIDI patches, if the -patches parameter is present.
  // Pros: You don't have to wait that second when the song changes
  //       Some broken songs that have a corrupted instrument list will work properly
  // Cons: It can take a lot of memory, if you are using Allegro's software wavetable
  //       synth.  A decent patch bank (Creative Labs' 2mbgmgs.sf2) is about 3mb, and
  //       this can cause problems on systems (like mine) with only 8 mb of ram.  It
  //       is not helped by the fact that DJ's malloc (though extremely fast) rounds
  //       memory request up to a power of 2.  In this size region, 3 mb rounds to 4mbs,
  //       and DOOM's default internal heap rounds to 8 mbs.  This is probably why large
  //       heap sizes fail.  They are only effective in sizes of 2 ^ x mbs.
  if (M_CheckParm("-patches")) {
    cprintf("I_InitMusic: Loading General MIDI Patch Set.\n\r");
    load_midi_patches();
    patches = 1;
  } else cprintf("I_InitMusic: Patches will be loaded dynamically.\n\r");
#endif

  // If memory is paged out to disk, it can not be paged in in an interrupt context, causing
  // the machine to bomb out to dos.
  LOCK_VARIABLE(MUSRegisteredList);
  LOCK_VARIABLE(MUSRListLen);
  LOCK_VARIABLE(MUSdata);
  LOCK_FUNCTION(MusicTicker);
  LOCK_FUNCTION(MUSgetTime);
  LOCK_FUNCTION(HandleMUSEvent);

  // Call the MusicTicker 140 times per second (by default) to send music to the MIDI device
  install_int_ex(MusicTicker, BPS_TO_TIMER(140));

  // Reset the MIDI device to a default state:
               //(all sound off, reset controllers)
  for (  all_sound_off[0] = all_sound_off[3] = 0xb0;
         all_sound_off[0] <= 0xbf;
         all_sound_off[0]++, all_sound_off[3]++)
             midi_out(all_sound_off, sizeof(all_sound_off));
  update_controllers();
}

#ifdef TEST_MUSICS
int snd_MusicVolume;
void help(void);

int main(int argc, char **argv)
{
  int handle;
  char *data;
  int key;
  if (argc == 2) {
    allegro_init();
    i_love_bill = TRUE; // My machine doesn't work without it.  As you might have guessed,
    // my machine is pretty shit. :-(
    handle = install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
    I_InitMusic();
    load_midi_patches();

    if (handle < 0) {
        printf("ERROR: '%s'", allegro_error);
        return handle;
    }
    handle = _open(argv[1], O_RDONLY);
    if (handle < 0) { printf("open %s failed: error no %d\n", argv[1], handle);
                      return handle; }
    data = malloc(filelength(handle));
    if (data == NULL) { printf("out of memory\n"); return -1; }
    _read(handle, data, filelength(handle));
    _close(handle);

    handle = I_RegisterSong(data);
    I_PlaySong(handle, 1);
    printf("DOOM Music Systems Test Programme by Kester Maddock\n");
    printf("\nPress a key to stop...\n");
    while(!kbhit());
    getch();
    I_ShutdownMusic();
    free(data);
  } else help();
  return 0;
}

void help(void)
{
    printf("DOOM Music Systems Test Programme by Kester Maddock\n");
    printf("Usage: MUSplay <filename>\n");
    printf("       <filename> is a .mus file\n");
}

#endif
