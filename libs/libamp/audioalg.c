// Allegro mixing code by KM

#include <allegro.h>
#include "audioalg.h"

#ifdef DEBUG
#include <stdio.h>
#endif

static AUDIOSTREAM *audio = 0, *newaudio;
static char *buffer;
static int instereo, newstereo;
static int stream_size, newstream_size;
static int stream_vol, newstream_vol;
static int going = FALSE;
// Allegro WIP doesn't need two channels
// Seperates stereo signals into two channels.  There better be
// enough space...
int SeperateStereo(long *source, long *dest, int count)
{
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
  if (audio) stop_audio_stream(audio);
  audio = NULL;
  going = FALSE;
  if (newaudio) AudioFlush();
  return 0;
}
END_OF_FUNCTION(AudioStop);


int AudioReady(void)
{
  buffer = NULL;
  if (audio && (buffer = get_audio_stream_buffer(audio)))
     return 1;
  return 0;
}
END_OF_FUNCTION(AudioReady);

static inline int StreamBufFinished(AUDIOSTREAM *stream)
{
  if (stream) {
    int pos = voice_get_position(stream->voice);
    int buf = 1 - stream->bufnum;
    if (buf) {
      if (pos < stream->len) return 1;
    } else {
      if (pos > stream->len) return 1;
    }
  }
  return 0;
}

int AudioDrainFinished(void)
{
  if (StreamBufFinished(audio))
    return 1;
  return 0;
}

int AudioBufferWrite(void *data)
{
  switch (instereo & 3) {
  // If stereo signal
  case 1:
  case 3:
    SeperateStereo((long *) data, (long *) buffer, stream_size);
    break;
  // Mono signal mix --> stereo
  case 2:
    SeperateMono((long *) data, (long *) buffer, stream_size);
    break;
  // Vanilla Mono :-(
  case 0:
    SignedFix((long *) data, (long *) buffer, stream_size);
    break;
  }
  free_audio_stream_buffer(audio);
  return 0;
}
END_OF_FUNCTION(AudioBufferWrite);

int AudioSetVol(int vol)
{
  stream_vol = newstream_vol = vol;
  voice_set_volume(audio->voice, vol);
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
    audio->samp->stereo = FALSE;
  } else if (stereo && !instereo) {
    audio->samp->stereo = TRUE;
  }
  instereo = stereo;
}

void AudioFlush(void)
{
  if (audio) stop_audio_stream(audio);
  audio = newaudio;
  instereo = newstereo;
  stream_size = newstream_size;
  stream_vol = newstream_vol;
  newaudio = NULL;
  newstereo = newstream_size = newstream_vol = 0;
  if (audio)
    going = TRUE;
}
END_OF_FUNCTION(AudioFlush);

void AudioInit_end(void);
int AudioInit(int bufsize, int freq, int stereo, int volume)
{
  static int unlocked = 1;
  int *_stereo, *_bufsize, *_vol;
  AUDIOSTREAM **_audio;

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
   _audio = &audio;
  } else {
   _stereo = &newstereo;
   _bufsize = &newstream_size;
   _vol = &newstream_vol;
   _audio = &newaudio;
  }
  *_stereo = stereo;
  *_bufsize = bufsize;
  *_vol = volume;

  *_audio = play_audio_stream(bufsize, 16, stereo? TRUE : FALSE, freq, volume, 128);
  if (!(*_audio)) return -1;
  going = TRUE;
  return 0;
}
END_OF_FUNCTION(AudioInit);

