// Music player code, by Kester Maddock, based on Docs by Vladimir Arnost
#ifndef __MUS_MUSIC__
#define __MUS_MUSIC__

#define PAUSE 1
#define PLAY 2
#define LOOP 4

// -----------    Local Types    --------------
struct MUSheader_s {
  
  //identifier "MUS" 0x1A; we will use 0x1a53554d
  long id; 
  
  // Length of the actual score in bytes
  unsigned short scoreLen;

  // Offset to start of score; doubles as header length. scoreStart + scoreLen = filesize
  unsigned short scoreStart;

  unsigned short channels;  // primary channels
  unsigned short channels2; // count secondary channels

  // Number of instruments used
  unsigned short instrCnt; // number of instruments
  unsigned short dummy;

  // Array of instruments used
  unsigned short *instruments;
};

struct MUShandle_s {
  // Every mus file has its header...
  struct MUSheader_s MUSheader;

  // The current byte we are accessing from MUSdata
  int datapointer;

  // The raw mus file is loaded into here
  unsigned char *MUSdata;

  // Bitfield: has a play bit, pause bit etc....
  int playControl; // uses the bit controls defined in i_music.h

  // The number of 140Hz tics until the next event
  signed int timer; 

  // The last volume used for each channel, used when the next note uses the same volume
  char lastVol[16];
};


// Initialise the Music player
void MUS_Init(void);

// Sets the volume for the music player
void MUS_SetMusicVolume(int volume);

// Pauses the specified song
void MUS_PauseSong(struct MUShandle_s *handle);

// Resumes the specified song (after being paused by I_PauseSong)
void MUS_ResumeSong(struct MUShandle_s *handle);

// Initialises the song to a ready to be played state.
// Returns the handle used by all the other functions
// data points to the MUS file which must be loaded into memory raw
struct MUShandle_s *MUS_RegisterSong(void *data);

// Gee, I wonder... if looping is nonzero, song will play forever...
void MUS_PlaySong(struct MUShandle_s *handle, int looping);

int MUS_HandleEvent(struct MUShandle_s *handle);

// Stops a song playing forever
void MUS_StopSong(struct MUShandle_s *handle);

// Frees the resources used by a song
void MUS_UnRegisterSong(struct MUShandle_s *handle);

// If the specified song is playing returns 1; if not playing returns 0
int MUS_QrySongPlaying(struct MUShandle_s *handle);

#endif

