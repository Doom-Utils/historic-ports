/*
	Amp Library interface
	 (audio.c + guicontrol.c)
	Adapted by Ove Kaaven <ovek@arcticnet.no>
 */


#include "amp.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef __DJGPP__
#ifndef __BEOS__
#include <sys/uio.h>
#endif
#endif

#ifndef __DJGPP__
#include <sys/socket.h>
#else
#include "audioalg.h"
#endif
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define AUDIO
#include "audio.h"
#include "formats.h"
#include "getbits.h"
#include "huffman.h"
#include "layer2.h"
#include "layer3.h"
#include "position.h"
#include "rtbuf.h"
#include "transform.h"
#include "controldata.h"
#include "guicontrol.h"

#include "libamp.h"
#include <allegro.h>

#ifndef __BEOS__
typedef int bool;
#endif

void calculate_t43(void);

char *AMP_buffer = 0;
unsigned int AMP_bufferHead, AMP_bufferRead, AMP_bufferTail, AMP_bufferSize, AMP_bufferDrain;
int AUDIO_BUFFER_SIZE;

// 1<<3 = 8 buffers
#define NUM_BUFFERS 3
// Number of times we can pop audio before I start downmixing...
#define FUCKER 0x1000
#define FUCK_TOLERANCE (4*FUCKER)

#define DRAIN_STOP 1
#define DRAIN_OFF  0
#define DRAIN_INITSWITCH -1
#define DRAIN_SWITCH -2

#define MID(min, val, max) MAX(min, MIN(val, max))

int amp_bitrate,amp_samprat;
int amp_mpg_ver,amp_layer,amp_stereo,amp_paused;
int amp_playing,amp_draining,amp_loaded=FALSE,amp_reverse_phase=FALSE;
int amp_looping;
int amp_dec_frame,amp_dec_time;
int amp_frame,amp_time;
struct AUDIO_HEADER amp_cur_header;

int amp_downmix(int mix)
{
  int a = A_DOWNMIX;
  int b = amp_reverse_phase << 1;
  A_DOWNMIX = mix;
  if (amp_stereo && !A_DOWNMIX) b |= 1;
  AudioSetStereo(b);
  return a;
}

static void get_amp_info(struct AUDIO_HEADER *header)
{
 /* show_header from dump.c */
 amp_bitrate=t_bitrate[header->ID][3-header->layer][header->bitrate_index];
 amp_samprat=t_sampling_frequency[header->ID][header->sampling_frequency];

 amp_layer=4-header->layer;
 if (header->ID==1) amp_mpg_ver=1; else amp_mpg_ver=2;

 if (header->mode==3) amp_stereo=0; else amp_stereo=1;
}

static int read_amp_frame(struct AUDIO_HEADER*header, char *buf)
{
 int i = 0;
 if (header->layer==1) {
  i = layer3_frame(header,amp_dec_frame, buf);
 } else if (header->layer==2)
  i = layer2_frame(header,amp_dec_frame, buf);
 amp_frame++;
 amp_dec_frame++;
 return i;
}

static int init_stop(void)
{
 amp_playing=FALSE;
 /* check for any data left to play in buffer */
 if (amp_draining < DRAIN_STOP) {
  AMP_bufferDrain = AMP_bufferHead + AUDIO_BUFFER_SIZE * sizeof(short) * 4;
  amp_draining=DRAIN_STOP;
  return(0);
 }
 /* nothing left to play, stop */
 AudioStop();
 return(-1);
}

static int fetch_header(struct AUDIO_HEADER*header)
{
 int g;

 if ((g=gethdr(header))!=0) {
  report_header_error(g);
  return(-1);
 }

 if (header->protection_bit==0) getcrc();
 get_amp_info(header);
 return(0);
}

static int init_rewind(struct AUDIO_HEADER*header)
{
 int i;
 fseek(in_file,0,SEEK_SET);
 i = fetch_header(header);
 return(i);
}

static int exec_replay(void)
{
 amp_draining=DRAIN_OFF;

 return(amp_playing=TRUE);
}

static int init_replay(void)
{
 int i;
 if (init_rewind(&amp_cur_header)<0) {
  i =init_stop();

 }
  i = exec_replay();

 return(i);
}

static int end_of_track(void)
{
 int i;
 if (amp_looping) {
  i = init_replay();
 } else {
  i = init_stop();
 }
 return i;
}

// Puts decoded audio into the soundcard
void amp_interrupt(void)
{
  // See whether we are actually playing anything
  if (!(amp_playing || (amp_draining == DRAIN_STOP))) return;

  // Check the audio device is ready for more data
  if (!AudioReady()) return;

  if ((AMP_bufferTail >= AMP_bufferDrain)
    && (amp_draining == DRAIN_INITSWITCH)) amp_draining = DRAIN_SWITCH;
  // Updates the mixing parameters in hardware
  if (amp_draining == DRAIN_SWITCH) {
    AudioFlush();
    amp_draining = DRAIN_OFF;
    if (!AudioReady()) return;
  }

  // Write the data
  AudioBufferWrite(&AMP_buffer[AMP_bufferTail % AMP_bufferSize]);

  // Update the tail
  AMP_bufferTail += AUDIO_BUFFER_SIZE * sizeof(short) * 
     (amp_stereo && !A_DOWNMIX ? 2 : 1);
}
END_OF_FUNCTION(amp_interrupt);

// Decodes encoded audio
int amp_decode(void)
{
  static int poll_size = -666;
  static int urgency = 1;
  static int fuck_count = 0;

  if (poll_size == -666) poll_size = AUDIO_BUFFER_SIZE * sizeof(short) * 2;

  if (!amp_playing) {
    if (amp_draining) {
      if (AMP_bufferTail > AMP_bufferDrain) {
  	     /* nothing left to play, stop */
	     amp_draining=DRAIN_OFF; AudioStop();
        poll_size = -666;
        fuck_count = 0;
        urgency = 1;
        return -1;
      }
    } else {
      return -1;
    }
  }

  if (AMP_bufferTail >= AMP_bufferRead) {
    int a = AMP_bufferRead;
    AMP_bufferRead = AMP_bufferTail + AUDIO_BUFFER_SIZE * sizeof(short) * 2;
    poll_size += AMP_bufferRead - a;
  }

  // This code checks to see if data is read before it is written
  // If it does this more than FUCK_TOLERANCE, it starts downmixing
  // the data.
  if (AMP_bufferTail >= AMP_bufferHead) {
    AMP_bufferHead = AMP_bufferTail + (AMP_bufferSize >> NUM_BUFFERS);
    memset(&AMP_buffer[AMP_bufferTail % AMP_bufferSize], 0,
           (AMP_bufferHead - AMP_bufferTail));
    AMP_bufferRead = AMP_bufferTail + AUDIO_BUFFER_SIZE * sizeof(short) * 2;
    fuck_count += FUCKER;
    if (fuck_count > FUCK_TOLERANCE)
    {
      amp_downmix(TRUE);
      fuck_count = 0;
    }
  } else if (fuck_count > 0) fuck_count --;

  // This reads enough data
  if (poll_size > 0) {
   int i = 576 << 1, j;
//   if (poll_size > (AUDIO_BUFFER_SIZE * sizeof(short) * 8)) urgency++;
   for (j = urgency; j--;)
    if ((amp_draining < DRAIN_STOP) && amp_playing && !amp_paused) {
      i = read_amp_frame(&amp_cur_header, &AMP_buffer[AMP_bufferHead % AMP_bufferSize])
		   * sizeof(short) /* * (amp_stereo && !A_DOWNMIX ? 2 : 1) */;
      AMP_bufferHead += i;
      poll_size -= i;
      if (fetch_header(&amp_cur_header)<0) {
         poll_size = -666;
         fuck_count = 0;
         urgency = 1;
	      return(end_of_track());
      }
    } else {
      if (amp_stereo && !A_DOWNMIX) i <<= 1;
      memset(&AMP_buffer[AMP_bufferHead % AMP_bufferSize], 0x00000000, i);
      AMP_bufferHead += i;
      poll_size -= i;
    }
  } else urgency = 1;
  
  amp_dec_time=amp_dec_frame*1152/amp_samprat;
  return(1);
}

int install_amp(void)
{
 static bool is_installed=FALSE;

 if (!is_installed) {
/* initialise_decoder */
  premultiply();
  imdct_init();
  calculate_t43();

/* args */
  AUDIO_BUFFER_SIZE=1152 * 2;
  AMP_bufferSize = AUDIO_BUFFER_SIZE * sizeof(short) * 2 * 32;
  AMP_buffer = malloc(AMP_bufferSize);
  memset(AMP_buffer, 0x00000000, AMP_bufferSize);
  AMP_bufferHead = AMP_bufferTail = AMP_bufferRead = AMP_bufferDrain = 0;
  A_DUMP_BINARY=FALSE;
  A_QUIET=TRUE;
  A_FORMAT_WAVE=FALSE;
  A_SHOW_CNT=FALSE;
  A_SET_VOLUME=255;
  A_SHOW_TIME=0;
  A_AUDIO_PLAY=TRUE;
  A_WRITE_TO_FILE=FALSE;
  A_MSG_STDOUT=FALSE;
  A_DOWNMIX=FALSE;

  LOCK_VARIABLE(AMP_bufferSize);
  LOCK_VARIABLE(AMP_bufferHead);
  LOCK_VARIABLE(AMP_bufferTail);
  LOCK_VARIABLE(AMP_bufferRead);
  LOCK_VARIABLE(AMP_bufferDrain);
  LOCK_VARIABLE(amp_stereo);
  LOCK_VARIABLE(amp_playing);
  LOCK_VARIABLE(amp_draining);
  LOCK_VARIABLE(A_DOWNMIX);
  LOCK_VARIABLE(AUDIO_BUFFER_SIZE);
  _go32_dpmi_lock_data(AMP_buffer, AMP_bufferSize);

  LOCK_FUNCTION(amp_interrupt);

  install_timer();
  install_int_ex(amp_interrupt, BPS_TO_TIMER(280));

  is_installed=TRUE;
 }

 return(is_installed);
}

int replay_amp(void)
{
 if (!amp_loaded) return(-1);
 return(init_replay());
}

int seek_amp_abs(int frame)
{
 if (!amp_loaded) return(-1);

 if (frame>amp_dec_frame) {
  amp_dec_frame+=ffwd(&amp_cur_header,frame-amp_dec_frame);
  exec_replay();
  return(0);
 } else
 if (frame<amp_dec_frame) {
  if (frame<(amp_dec_frame/2)) {
   if (init_rewind(&amp_cur_header)<0) return(-1);
   amp_dec_frame+=ffwd(&amp_cur_header,frame);
  } else {
   amp_dec_frame-=rew(&amp_cur_header,amp_dec_frame-frame);
  }
  exec_replay();
  return(0);
 } else {
  exec_replay();
  return(0);
 }
}

int seek_amp_rel(int framecnt)
{
 int frame;

 frame=amp_frame+framecnt;
 if (frame<0) frame=0;
 return(seek_amp_abs(frame));
}

void amp_setvolume(int vol)
{
  vol = MID(0, vol, 255);
  AudioSetVol(vol);
}

void amp_pause(void)
{
 amp_paused = 1;
}

void amp_resume(void)
{
 amp_paused = 0;
}

int load_amp(char*filename,int loop)
{
 int g, firsttime = 1;
 if (amp_loaded) {
   AMP_bufferDrain = AMP_bufferHead + AUDIO_BUFFER_SIZE * sizeof(short) * 2;
   memset(&AMP_buffer[AMP_bufferHead % AMP_bufferSize], 0,
     AUDIO_BUFFER_SIZE * sizeof(short) * 2);
   AMP_bufferHead = AMP_bufferDrain;
   amp_draining = DRAIN_INITSWITCH;
   fclose(in_file);
   firsttime = 0;
 }

 if ((in_file=fopen(filename,"rb"))==NULL) {
  warn("Could not open file: %s\n",filename);
  return(0);
 }

/* initialise_globals */
  append=data=nch=0;
  f_bdirty=TRUE;
  bclean_bytes=0;
  memset(s,0,sizeof s);
  memset(res,0,sizeof res);

/* load MPEG header */
 fetch_header(&amp_cur_header);
 get_amp_info(&amp_cur_header);

/* setup_audio */
 if (amp_stereo && !A_DOWNMIX)
   g = 1;
 else
   g = 0;
 AudioInit(AUDIO_BUFFER_SIZE, amp_samprat, g | (amp_reverse_phase << 1),A_SET_VOLUME);

 amp_looping=loop;
 amp_paused = 0;
 amp_dec_time=amp_dec_frame=0;
 amp_time=amp_frame=0;
 if (firsttime) {
    int cnt = AMP_bufferHead + AUDIO_BUFFER_SIZE * sizeof(short) * 4;
    if (!AMP_bufferRead)
      AMP_bufferRead = AUDIO_BUFFER_SIZE * sizeof(short) * 2;
    while (AMP_bufferHead < cnt) {
     AMP_bufferHead += 
       read_amp_frame(&amp_cur_header, 
                  	 &AMP_buffer[AMP_bufferHead % AMP_bufferSize])
	                   * sizeof(short) ;
     if (fetch_header(&amp_cur_header)<0) 
       return(end_of_track());
    }
 }
 return(amp_playing=amp_loaded=TRUE);
}


int unload_amp(void)
{
 if (!amp_loaded) return(0);
 amp_loaded=amp_playing=FALSE; amp_draining=DRAIN_OFF;
 AudioClose();
 fclose(in_file);
 memset(AMP_buffer, 0, AMP_bufferSize);
 AMP_bufferHead = AMP_bufferTail = AMP_bufferRead = AMP_bufferDrain = 0;
 return(1);
}

void report_header_error(int err)
{
#if 0
	switch (err) {
		case GETHDR_ERR: die("error reading mpeg bitstream. exiting.\n");
					break;
		case GETHDR_NS : warn("this is a file in MPEG 2.5 format, which is not defined\n");
				 warn("by ISO/MPEG. It is \"a special Fraunhofer format\".\n");
				 warn("amp does not support this format. sorry.\n");
					break;
		case GETHDR_FL1: warn("ISO/MPEG layer 1 is not supported by amp (yet).\n");
					break;
		case GETHDR_FF : warn("free format bitstreams are not supported. sorry.\n");
					break;  
		case GETHDR_SYN: warn("oops, we're out of sync.\n");
					break;
		case GETHDR_EOF: 
		default:                ; /* some stupid compilers need the semicolon */
	}
#endif
}

