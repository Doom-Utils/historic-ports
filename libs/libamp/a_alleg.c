/*
	Allegro code
	Written by: Ove Kaaven <ovek@arcticnet.no>

 */

/* Support for Allegro */

#include "amp.h"
#include "transform.h"

#include <stdio.h>
#include <string.h>
#include "audioIO.h"
#include "audio.h"

#include "allegro.h"
#include "libamp.h"

/* TODO: */
/*   Better buffer size control */

#define FULL_BUFFER

// I think this is for making an ocilloscope
#define TRACK_FRAME

/* Buffering scheme is based on 9*1024=16*576 */

#ifdef FULL_BUFFER

//#define STREAM_SAMP (32*576)
#define STREAM_SAMP (8*576)
#define BUF_SAMP STREAM_SAMP

#else

/* #define STREAM_SAMP (2*1024) */
#define STREAM_SAMP (100*576)
#define BUF_SAMP (STREAM_SAMP / 2)

#endif

//#define UPD_SAMP 1024
#define UPD_SAMP 1152

//                 Left(mono)      right(stereo) streams
static AUDIOSTREAM *audio_left = 0, *audio_right = 0;

//                     Left(mono)     right(stereo) buffer pointers
// Point to the actual data that is sent to the soundcard
static unsigned short *stream_left, *stream_right;

//         mixing frequency and stereo flag
static int freq,instereo;

// Mixing buffer.  Used to seperate the stereo signals
static unsigned short *buffer_left, *buffer_right;
// Points to where buffers are written, and buffers are read, and how much
// is used (for padding)
static unsigned buffer_head, buffer_tail, buffer_used;

// Ocilloscope data
#ifdef TRACK_FRAME
static unsigned last_frame,*buffer_frame,*stream_frame;
#endif

// Used for getting the current frame
unsigned short *amp_buf_left, *amp_buf_right;
int amp_buf_len;

int amp_dec_frame; /* (duplicate) definition here, so amp.exe will link */
int amp_reverse_phase; /* another duplicate */

// Dunno...
unsigned short *amp_play_left, *amp_play_right;
int amp_play_len;


// Allocates the Allegro Audiostream resources
int audioAlloc(void)
{
 // Stop the currently playing audio stream
 if (audio_left) stop_audio_stream(audio_left);
 if (audio_right) stop_audio_stream(audio_right);
 
 //  Start an Allegro audio stream, and check for errors.
 audio_left=play_audio_stream(STREAM_SAMP,16,freq,255,instereo?0:128);
 if (!audio_left) return(0);

 // If there is a right channel (stereo music) start another stream
 // panned hard right.
 if (instereo) {
  audio_right=play_audio_stream(STREAM_SAMP,16,freq,255,255);
  if (!audio_right) return(0);
 }
 
 // OK
 return(1);
}

// Allocates the mix buffer.  It is needed to seperate stereo
// messages.
int audioBufferAlloc(void)
{
 // Allocates left channel buffer. 72k
 buffer_left=malloc(BUF_SAMP*sizeof(short));
 if (!buffer_left) return(0);
 
 // Allocate the right channel buffer for stereo data. 72k
 if (instereo) {
  buffer_right=malloc(BUF_SAMP*sizeof(short));
  if (!buffer_right) return(0);
 }

#ifdef TRACK_FRAME
 buffer_frame=malloc(BUF_SAMP*sizeof(unsigned));
 if (!buffer_frame) return(0);
 
 stream_frame=malloc(2*STREAM_SAMP*sizeof(unsigned));
 if (!stream_frame) return(0);
 
 memset(stream_frame,-1,2*STREAM_SAMP*sizeof(unsigned));
#endif
 
 // OK
 return(1);
}

void
audioOpen(int frequency, int stereo, int volume)
{
 /* Set 1 or 2 channels */
 instereo=(stereo ? 1 : 0);
 DB(audio, msg("Setting stereo to %d\n",instereo) )

 /* Set the output frequency */
 freq=frequency;

 /* Allocate output channels */
 if (!audioAlloc())
  die("Unable to open audio device\n");

 /* Allocate output buffers */
 if (!audioBufferAlloc())
  die("Unable to allocate write buffer\n");

 // Clear the mix buffer head and tails
 buffer_head = buffer_tail = buffer_used = 0;
 
 // Audiostream buffers
 stream_left=stream_right=NULL;
 
 amp_buf_left=amp_buf_right=NULL;
 amp_buf_len=STREAM_SAMP;
 amp_play_left=amp_play_right=NULL;
 amp_play_len=UPD_SAMP;

 DB(audio, msg("Audio device opened\n"); )

 if (volume != -1)
  audioSetVolume(volume);
}

// Sets the volume.  Not good, cause also affects digital sfx volume too.
void
audioSetVolume(int volume)
{
 DB(audio, msg("Setting volume to: %d\n",volume); )
 set_volume(volume*2,-1);
}

// Stops the audiostream
inline void
audioStop()
{
 DB(audio, msg("audio: stop\n") );
 if (audio_left) stop_audio_stream(audio_left);
 if (audio_right) stop_audio_stream(audio_right);
 
 // Reset the buffer FIFO
 amp_buf_left=amp_buf_right=NULL;
 audio_left=audio_right=NULL;
 buffer_head=buffer_tail=buffer_used=0;
}

inline void
audioStart()
{
 if (!audioAlloc())
  die("Unable to open audio device\n");

#ifdef TRACK_FRAME
 memset(stream_frame,-1,2*STREAM_SAMP*sizeof(unsigned));
#endif
}

// This will stop and start the mixbuffer.  Causing a pop.
inline void
audioFlush()
{
 DB(audio, msg("audio: flush\n") );
 audioStop();
 audioStart();
}

void
audioClose()
{
 // Free Allegro resources
 if (audio_left) stop_audio_stream(audio_left);
 if (audio_right) stop_audio_stream(audio_right);

 // Free misc data
 free(buffer_left);
 free(buffer_right);

#ifdef TRACK_FRAME
 free(buffer_frame);
 free(stream_frame);
#endif
 
 DB(audio, msg("audio: closed stream\n") );
}

// Finds the buffers to update the sound.
// Returns TRUE when it is OK to update.
int audioReady(void)
{
 // Finds the current buffer, for left(mono) buffer
 if (!stream_left)
  stream_left=get_audio_stream_buffer(audio_left);
 if (!stream_left) return(0);
 
 // Finds the current stereo buffer, right channel
 if (instereo) {
  if (!stream_right)
   stream_right=get_audio_stream_buffer(audio_right);
  if (!stream_right) return(0);
 }
 
 // OK
 return(1);
}

inline int audioUsedSpace(void)
{
 return(buffer_used);
}

int audioSpaceRemaining(void)
{
 return(BUF_SAMP-audioUsedSpace());
}

int
audioWrite(char *buffer, int count)
{
 while (!audioReady()) __dpmi_yield();
 audioBufferWrite(buffer,count);
 return(count);
}

void
audioBufferTransfer(void)
{
 while (audioReady()&&(buffer_used>=STREAM_SAMP)) {
  
  // Copy the seperated data to its audio stream
  memcpy(stream_left,buffer_left+buffer_tail,STREAM_SAMP*sizeof(short));
  
  // Copy the stereo channel as well
  if (instereo) 
    memcpy(stream_right,buffer_right+buffer_tail,STREAM_SAMP*sizeof(short));

#ifdef TRACK_FRAME
  memcpy(stream_frame+STREAM_SAMP*audio_left->bufnum,buffer_frame+buffer_tail,STREAM_SAMP*sizeof(unsigned));
#endif
  
  // Mark the audio buffers as full, for Allegro
  free_audio_stream_buffer(audio_left);
  if (instereo) free_audio_stream_buffer(audio_right);
  
  // Save the pointers for getting the current frame
  amp_buf_left=stream_left;
  if (instereo) amp_buf_right=stream_right; 
  else amp_buf_right=amp_buf_left;

#ifndef TRACK_FRAME
  amp_play_left=amp_buf_left; amp_play_right=amp_buf_right;
#endif
  
  // Reset the buffer pointers for next time.
  stream_left=stream_right=NULL;
  buffer_tail=(buffer_tail+STREAM_SAMP)%BUF_SAMP;
  buffer_used-=STREAM_SAMP;
 }
}

unsigned
audioBufferGetFrame(void)
{
#ifdef TRACK_FRAME
 int stream_pos,frame=-1;
 unsigned block;

 if (amp_buf_left) {
  amp_play_left=amp_play_right=NULL;
  if (audio_left&&((stream_pos=voice_get_position(audio_left->voice))>=0)) {
   block=stream_pos/UPD_SAMP;
   amp_play_left=(short*)(audio_left->samp->data)+block*UPD_SAMP;
   if (instereo)
    amp_play_right=(short*)(audio_right->samp->data)+block*UPD_SAMP;
   else
    amp_play_right=amp_play_left;
   frame=stream_frame[stream_pos];
  } else frame=last_frame+1;
 } /* -1 = preparing to play */
 return(frame);
#else
 return(amp_dec_frame);
#endif
}

int
audioIsPlaying(void)
{
 if (audioUsedSpace()) return(TRUE);
 if (audioReady()) return(FALSE);
 return(TRUE);
}

void printout(void)
{
int j;

	if (nch==2)
		j=32 * 18 * 2;
	else
		j=32 * 18;

	if (A_WRITE_TO_FILE) {
#ifndef NO_BYTE_SWAPPING
	short *ptr = (short*) sample_buffer;
	int i = j;

		for (;i>=0;--i)
			ptr[i] = ptr[i] << 8 | ptr[i] >> 8;
#endif

		fwrite(sample_buffer, sizeof(short), j, out_file);
	}


	if (A_AUDIO_PLAY) {
		if (AUDIO_BUFFER_SIZE==0)
			audioWrite((char*)sample_buffer, j * sizeof(short));
		else
			audioBufferWrite((char*)sample_buffer, j * sizeof(short));
	    }

}

int AUDIO_BUFFER_SIZE;

int
audioBufferOpen(int frequency, int stereo, int volume)
{
	audioOpen(frequency, stereo, volume);
	return(0);
}

void
audioBufferClose()
{
	audioClose();
}

// Splits stereo data into seperate spaces.
// Then throws it at the soundcard.
// 16bit samples must be handled in words.
// This is probably *very* slow
void
audioBufferWrite(char *buffer, int count)
{
 int asmp,smp,cnt;

 DB(audio, msg("audio: Writing %d bytes to audio stream\n",count) );

 // Find the number of (16bit) samples
 asmp=count/sizeof(short);
 
 // Find the number of samples (stereo samples take two)
 smp=instereo?asmp/2:asmp;
 while (smp) {
  cnt=MIN(smp,BUF_SAMP-buffer_head);
  /* block until free buffer space, if necessary */
  while ((buffer_used+cnt)>BUF_SAMP) {
   while (!audioReady()) __dpmi_yield();
   audioBufferTransfer();
  }
  /* split channels */
  if (instereo) {
   if (amp_reverse_phase)
    asm volatile ("
     0:
     lodsw
     addw $0x8000,%%ax
     movw %%ax,(%%edi)
     lodsw
     addw $0x8000,%%ax
      notw %%ax
     movw %%ax,(%%ebx)
     addl $2,%%edi
     addl $2,%%ebx
     decl %%ecx
     jnz 0b
    " :
      : "S" (buffer), "D" (buffer_left+buffer_head), "b" (buffer_right+buffer_head), "c" (cnt)
      : "ax", "ebx", "ecx", "esi", "edi", "memory" );
   else // Normal Stereo
    asm volatile ("
     0:
     lodsw
     addw $0x8000,%%ax
     movw %%ax,(%%edi)
     lodsw
     addw $0x8000,%%ax
     movw %%ax,(%%ebx)
     addl $2,%%edi
     addl $2,%%ebx
     decl %%ecx
     jnz 0b
    " :
      : "S" (buffer), "D" (buffer_left+buffer_head), "b" (buffer_right+buffer_head), "c" (cnt)
      : "ax", "ebx", "ecx", "esi", "edi", "memory" );
  } else { // Mono 
   asm volatile ("
    0:
    lodsw
    addw $0x8000,%%ax
    stosw
    decl %%ecx
    jnz 0b
   " :
     : "S" (buffer), "D" (buffer_left+buffer_head), "c" (cnt)
     : "ax", "ecx", "esi", "edi", "memory" );
  }
#ifdef TRACK_FRAME
  asm volatile ("
   rep
   stosl
  " :
    : "a" (amp_dec_frame), "D" (buffer_frame+buffer_head), "c" (cnt)
    : "ecx", "edi", "memory" );
  last_frame=amp_dec_frame;
#endif
  
  // Buffer_head points to where new data will be added
  buffer_head=(buffer_head+cnt)%BUF_SAMP;
  buffer_used+=cnt; smp-=cnt;
 }

 // Chuck it at the soundcard
 audioBufferTransfer();
}

// Fills the remaining buffer with 0
void audioBufferPad(void)
{
 int req,cnt;

 // If the buffer's not full already...
 if ((buffer_used)&&(buffer_used<STREAM_SAMP)) {
  
  // Find the number of samples to pad
  req=STREAM_SAMP-buffer_used;
  
  // Fill the left (mono) buffer
  for (cnt=req; cnt--;)
   buffer_left[buffer_head+cnt]=0x8000;
  
  // Fill the right buffer if in stereo
  if (instereo)
    for (cnt=req; cnt--;)
     buffer_right[buffer_head+cnt]=0x8000;

#ifdef TRACK_FRAME
  for (cnt=0; cnt<req; cnt++)
   buffer_frame[buffer_head+cnt]=last_frame+1;
#endif
  
  // Mark the buffer_head
  buffer_head=(buffer_head+req)%BUF_SAMP;
  buffer_used+=req;
 }
}

void audioBufferFlush(void)
{
 audioFlush();
}

