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
 *      Main header file for the Allegro library.
 *      This should be included by everyone and everything.
 *
 *      See readme.txt for copyright information.
 */


typedef struct AUDIOSTREAM2
{
   int voice;                          /* the voice we are playing on */
   SAMPLE *samp;                       /* the sample we are using */
   int bufnum;                         /* which buffer is currently playing */
   int numbufs;                        /* number of buffers */
   int len;                            /* buffer length */
   void *b[0];                         /* array of buffers */
} AUDIOSTREAM2;

AUDIOSTREAM2 *play_audio_stream2(int len, int bits, int stereo, int freq, int vol, int pan, int numbuffers);
void stop_audio_stream2(AUDIOSTREAM2 *stream);
void *get_audio_stream_buffer2(AUDIOSTREAM2 *stream);
void free_audio_stream_buffer2(AUDIOSTREAM2 *stream);

