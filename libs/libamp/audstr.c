// Allegro mixing code by KM

#include <allegro.h>
#include "audioalg.h"
#include "stream.h"

#ifdef DEBUG
#include <stdio.h>
#endif

static AUDIOSTREAM2 *audio_left = 0, *audio_right = 0, *newaudio_left = 0, *newaudio_right = 0;
static char *left_buf, *right_buf;
static int instereo, newstereo;
static int stream_size, newstream_size;
static int stream_vol, newstream_vol;
static int going = FALSE;
#ifdef ALLEGRO_WIP
#define STEREO_SIGNAL long *dest
#else
#define STEREO_SIGNAL short *left, short *right
#endif

#ifdef ALLEGRO_WIP
// Allegro WIP doesn't need two channels
// Seperates stereo signals into two channels.  There better be
// enough space...
int SeperateStereo(long *source, long *dest, int count)
{
//  count >>= 1;
  while (count--) {
    *dest = (*source) + 0x80008000;
    dest++; source++;
  }
  return 0;
}
END_OF_FUNCTION(SeperateStereo);

// Simulates a stereo signal from a mono signal
int SeperateMono(long *source, long *dest, int count)
{
  int left, right;
 #ifdef DEBUG
  printf("SeperateMono: source: %p, left: %p, right: %p, count: %d\n",
    source, left, right, count);
#endif
  count >>= 1;
  while (count--) {  
    left = (*source) + 0x80008000;
    right = ~left;
    *dest = (left & 0xffff0000) + (right >> 16);
    dest ++;
    *dest = (left << 16) + (right & 0xffff);
    dest++; source++;
  }
  return 0;
}
END_OF_FUNCTION(SeperateMono);

#else
// Seperates stereo signals into two channels.  There better be
// enough space...
int SeperateStereo(long *source, short *left, short *right, int count)
{
  int i, b;
#ifdef DEBUG
  printf("SeperateStereo: source: %p, left: %p, right: %p, count: %d\n",
    source, left, right, count);
#endif
  for (i = count; i--;) {
    b = *source + 0x80008000;
    *left = b >> 16;
    *right = b & 0xffff;
    right++; left++; source++;
  }
  if (instereo & 2)
    for (i = count; i--;) {
      right--;
      *right = ~(*right);
    }
  return 0;
}
END_OF_FUNCTION(SeperateStereo);

// Simulates a stereo signal from a mono signal
int SeperateMono(long *source, long *left, long *right, int count)
{
#ifdef DEBUG
  printf("SeperateMono: source: %p, left: %p, right: %p, count: %d\n",
    source, left, right, count);
#endif
  count >>= 1;
  while (count--) {  
    *left = (*source) + 0x80008000;
    *right = ~((*source) + 0x80008000);
     right++; left++; source++;
  }
  return 0;
}
END_OF_FUNCTION(SeperateMono);
#endif

int SignedFix(long *source, long *mono, int count)
{
#ifdef DEBUG
  printf("SeperateStereo: source: %p, dest: %p, count: %d\n",
    source, mono, count);
#endif
  count >>= 1;
  while (count--) {
    *mono = (*source) + 0x80008000;
    mono++; source++;
  }
  return 0;
}
END_OF_FUNCTION(SignedFix);

int AudioStop(void)
{
  if (audio_left) stop_audio_stream2(audio_left);
  if (audio_right) stop_audio_stream2(audio_right);
  audio_right = audio_left = 0;
  going = FALSE;
  if (newaudio_left) AudioFlush();
  return 0;
}
END_OF_FUNCTION(AudioStop);


int AudioReady(void)
{
  left_buf = right_buf = 0;
  if (audio_left && (left_buf = get_audio_stream_buffer2(audio_left)))
#ifdef ALLEGRO_WIP
     return 1;
#else
  if (instereo)
     return (int) (right_buf = get_audio_stream_buffer2(audio_right));
  else return 1;
#endif
  return 0;
}
END_OF_FUNCTION(AudioReady);
static inline int in_buffer(AUDIOSTREAM2 *stream, int pointer, int buffer)
{
   int start = (int) stream->b[buffer] - (int) stream->b[0];
   int end = start + (stream->samp->len / stream->numbufs * stream->samp->bits/8 * ((stream->samp->stereo)?2:1));
   if ((pointer < end) && (pointer > start)) return 1;
   return 0;
}     

static inline int StreamBufFinished(AUDIOSTREAM2 *stream)
{
  if (stream) {
    int pos = voice_get_position(stream->voice);
    if (!in_buffer(stream, pos, stream->bufnum)) return 1;
  }
  return 0;
}

int AudioDrainFinished(void)
{
  if (StreamBufFinished(audio_left)) {
    if (audio_right) return StreamBufFinished(audio_right);
    return 1;
  }
  return 0;
}

int AudioBufferWrite(void *data)
{
  switch (instereo & 3) {
#ifdef ALLEGRO_WIP
  // If stereo signal
  case 1:
  case 3:
    SeperateStereo((long *) data, (long *) left_buf, stream_size);
    break;
  // Mono signal mix --> stereo
  case 2:
    SeperateMono((long *) data, (long *) left_buf, stream_size);
    break;
#else
  // If stereo signal
  case 1:
  case 3:
    SeperateStereo((long *) data,
                  (short *) left_buf,
                  (short *) right_buf,
                         stream_size);
    free_audio_stream_buffer2(audio_right);
    break;
  // Mono signal mix --> stereo
  case 2:
    SeperateMono((long *) data, (long *) left_buf,
                                (long *) right_buf,
                                stream_size);
    free_audio_stream_buffer2(audio_right);
    break;
#endif
  // Vanilla Mono :-(
  case 0:
    SignedFix((long *) data,
                 (long *) left_buf, stream_size);
    break;
  }
  free_audio_stream_buffer2(audio_left);
  return 0;
}
END_OF_FUNCTION(AudioBufferWrite);

int AudioSetVol(int vol)
{
  stream_vol = newstream_vol = vol;
  voice_set_volume(audio_left->voice, vol);
#ifndef ALLEGRO_WIP
  if (audio_right)
    voice_set_volume(audio_right->voice, vol);
#endif
  return 0;
}
END_OF_FUNCTION(AudioSetVol);

int AudioClose(void)
{
  return AudioStop();
}
END_OF_FUNCTION(AudioClose);

void AudioSetStereo(int stereo)
{
  if (!stereo && instereo) {
#ifdef ALLEGRO_WIP
    audio_left->samp->stereo = FALSE;
#else
    stop_audio_stream2(audio_right);
    audio_right = 0;
    voice_set_pan(audio_left->voice, 128);
#endif
  } else if (stereo && !instereo) {
#ifdef ALLEGRO_WIP
    audio_left->samp->stereo = TRUE;
#else
    audio_right=play_audio_stream2(stream_size, 16, audio_left->samp->freq, stream_vol, 255, 2);
    voice_set_pan(audio_left->voice, 0);
#endif
  }
  instereo = stereo;
}

void AudioFlush(void)
{
  if (audio_left) stop_audio_stream2(audio_left);
  if (audio_right) stop_audio_stream2(audio_right);
  audio_left = newaudio_left;
  audio_right = newaudio_right;
  instereo = newstereo;
  stream_size = newstream_size;
  stream_vol = newstream_vol;
  newaudio_left = newaudio_right = 0;
  newstereo = newstream_size = newstream_vol = 0;
  if (audio_left)
    going = TRUE;
}
END_OF_FUNCTION(AudioFlush);

void AudioInit_end(void);
int AudioInit(int bufsize, int freq, int stereo, int volume)
{
  static int unlocked = 1;
  int *_stereo, *_bufsize, *_vol;
  AUDIOSTREAM2 **_left, **_right;

  if (unlocked) {
    LOCK_VARIABLE(instereo);
    LOCK_VARIABLE(stream_size);
    LOCK_VARIABLE(stream_vol);

    LOCK_VARIABLE(newstereo);
    LOCK_VARIABLE(newstream_size);
    LOCK_VARIABLE(newstream_vol);

    LOCK_FUNCTION(SeperateStereo);
    LOCK_FUNCTION(SeperateMono);
    LOCK_FUNCTION(SignedFix);

    LOCK_FUNCTION(AudioStop);
    LOCK_FUNCTION(AudioInit);
    LOCK_FUNCTION(AudioReady);
    LOCK_FUNCTION(AudioBufferWrite);
    LOCK_FUNCTION(AudioSetVol);
    LOCK_FUNCTION(AudioClose);
    LOCK_FUNCTION(AudioFlush);
    unlocked = 0;
  }

  if (going == FALSE) {
   _stereo = &instereo;
   _bufsize = &stream_size;
   _vol = &stream_vol;
   _left = &audio_left;
   _right = &audio_right;
  } else {
   _stereo = &newstereo;
   _bufsize = &newstream_size;
   _vol = &newstream_vol;
   _left = &newaudio_left;
   _right = &newaudio_right;
  }
  *_stereo = stereo;
  *_bufsize = bufsize;
  *_vol = volume;

#ifdef ALLEGRO_WIP
  *_left = play_audio_stream2(bufsize, 16, stereo? TRUE : FALSE, freq, volume, 128, 2);
  *_right = 0;
#else
  *_left = play_audio_stream2(bufsize, 16, freq, volume, stereo?0:128, 2);
  if (stereo) {
    *_right = play_audio_stream2(bufsize, 16, freq, volume, 255, 2);
    if (!(*_right)) return -1;
  } else *_right = 0;
#endif
  if (!(*_left)) return -1;
  going = TRUE;
  return 0;
}
END_OF_FUNCTION(AudioInit);

