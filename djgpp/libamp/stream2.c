/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves,
 *      1 Salisbury Road,
 *      Market Drayton,
 *      Shropshire,
 *      England, TF9 1AJ.
 *
 *      Audio stream functions (original version by Andrew Ellem).
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>

#include "allegro.h"
#include "stream.h"
//#include "internal.h"



void _unlock_dpmi_data(void *a, int s);
/* play_audio_stream:
 *  Creates a new audio stream and starts it playing. The length is the
 *  size of each transfer buffer, which should be at least 2k.
 */
AUDIOSTREAM2 *play_audio_stream2(int len, int bits, int stereo, int freq, int vol, int pan, int numbuffers)
{
   AUDIOSTREAM2 *stream;
   int i;

   stream = malloc(sizeof(AUDIOSTREAM2) + numbuffers * sizeof(void *));
   if (!stream)
      return NULL;

   stream->len = len;

   stream->samp = create_sample(bits, stereo, freq, len*numbuffers);
   if (!stream->samp) {
      free(stream);
      return NULL;
   }

   for (i = 0; i < numbuffers; i++)
     stream->b[i] = stream->samp->data + (len * bits/8 * ((stereo) ? 2 : 1)) * i;
   stream->bufnum = 0;
   stream->numbufs = numbuffers;

   if (bits == 16) {
      unsigned short *p = stream->samp->data;
      for (i=0; i < len*2 * ((stereo) ? 2 : 1); i++)
	 p[i] = 0x8000;
   }
   else {
      unsigned char *p = stream->samp->data;
      for (i=0; i < len*2 * ((stereo) ? 2 : 1); i++)
	 p[i] = 0x80;
   }

   _go32_dpmi_lock_data(stream, sizeof(AUDIOSTREAM2) + numbuffers * sizeof(void *));

   stream->voice = allocate_voice(stream->samp);
   if (stream->voice < 0) {
      destroy_sample(stream->samp);
      _unlock_dpmi_data(stream, sizeof(AUDIOSTREAM2) + numbuffers * sizeof(void *));
      free(stream);
      return NULL;
   }

   voice_set_playmode(stream->voice, PLAYMODE_LOOP);
   voice_set_volume(stream->voice, vol);
   voice_set_pan(stream->voice, pan);
   voice_start(stream->voice);

   return stream;
}



/* stop_audio_stream:
 *  Destroys an audio stream when it is no longer required.
 */
void stop_audio_stream2(AUDIOSTREAM2 *stream)
{
   voice_stop(stream->voice);
   deallocate_voice(stream->voice);

   destroy_sample(stream->samp);

   _unlock_dpmi_data(stream, sizeof(AUDIOSTREAM) + stream->numbufs * sizeof(void *));
   free(stream); 
}


static inline int in_buffer(AUDIOSTREAM2 *stream, int pointer, int buffer)
{
   int start = (int) stream->b[buffer] - (int) stream->b[0];
   int end = start + (stream->samp->len / stream->numbufs * stream->samp->bits/8 * ((stream->samp->stereo)?2:1));
   if ((pointer < end) && (pointer > start)) return 1;
   return 0;
}     

/* get_audio_stream_buffer:
 *  Returns a pointer to the next audio buffer, or NULL if the previous 
 *  data is still playing. This must be called at regular intervals while
 *  the stream is playing, and you must fill the return address with the
 *  appropriate number (the same length that you specified when you create
 *  the stream) of samples. Call free_audio_stream_buffer() after loading
 *  the new samples, to indicate that the data is now valid.
 */
void *get_audio_stream_buffer2(AUDIOSTREAM2 *stream)
{
   int pos = voice_get_position(stream->voice);
   int i;
   for (i = 0; i < stream->numbufs; i++) 
     if (in_buffer(stream, pos, i)) return stream->b[(i+1)%stream->numbufs];
   return NULL;
}



/* free_audio_stream_buffer:
 *  Indicates that a sample buffer previously returned by a call to
 *  get_audio_stream_buffer() has now been filled with valid data.
 */
void free_audio_stream_buffer2(AUDIOSTREAM2 *stream)
{
   stream->bufnum = (stream->bufnum + 1) % stream->numbufs;
}


