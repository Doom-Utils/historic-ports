// Allegro mixing code by KM

#ifndef __AUDIO_ALG_H
#define __AUDIO_ALG_H

int AudioStop(void);
int AudioClose(void);
void AudioFlush(void);
void AudioSetStereo(int stereo);
int AudioInit(int bufsize, int freq, int stereo, int volume);
int AudioReady(void);
int AudioDrainFinished(void);
int AudioBufferWrite(void *data);
int AudioSetVol(int vol);

#endif
