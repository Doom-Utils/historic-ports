/* this file is a part of amp software, (C) tomislav uzelac 1996,1997

	Origional code by: Karl Anders Oygard
	Modified by:
	* Andrew Richards - moved code from audio.c

 */
#include "amp.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <dmedia/audio.h>
#include "audioIO.h"

/* declare these static to effectively isolate the audio device */

static ALport audioport;
static ALconfig audioconfig;


/* audioOpen() */
/* should open the audio device, perform any special initialization		 */
/* Set the frequency, no of channels and volume. Volume is only set if */
/* it is not -1 */


void
audioOpen(int frequency, int stereo, int volume)
{
	ALconfig audioconfig;
	audioconfig = ALnewconfig();
 
	if (!audioconfig)
		die("out of memory\n");
	else {
		long pvbuf[] = { AL_OUTPUT_COUNT, 0, AL_MONITOR_CTL, 0, AL_OUTPUT_RATE, 0};
		
		if (ALgetparams(AL_DEFAULT_DEVICE, pvbuf, 6) < 0)
			if (oserror() == AL_BAD_DEVICE_ACCESS)
				die("couldn't access audio device\n");
		
		if (pvbuf[1] == 0 && pvbuf[3] == AL_MONITOR_OFF) {
			long al_params[] = { AL_OUTPUT_RATE, 0};
			
			al_params[1] = frequency;
			ALsetparams(AL_DEFAULT_DEVICE, al_params, 2);
		} else
			if (pvbuf[5] != frequency)
				die("audio device is already in use with wrong sample output rate\n");
		
		/* ALsetsampfmt(audioconfig, AL_SAMPFMT_TWOSCOMP); this is the default */
		/* ALsetwidth(audioconfig, AL_SAMPLE_16); this is the default */
		
		if (!stereo) ALsetchannels(audioconfig, AL_MONO);
		/* else ALsetchannels(audioconfig, AL_STEREO); this is the default */

		ALsetqueuesize(audioconfig, AUSIZ * 2);
		
		audioport = ALopenport("amp", "w", audioconfig);
		if (audioport == (ALport) 0) {
			switch (oserror()) {
			case AL_BAD_NO_PORTS:
				die("system is out of ports\n");
				
			case AL_BAD_DEVICE_ACCESS:
				die("couldn't access audio device\n");
				
			case AL_BAD_OUT_OF_MEM:
				die("out of memory\n");
			}			 
			exit(-1);
		}
		ALsetfillpoint(audioport, AUSIZ);
	}
}


/* audioSetVolume - only code this if your system can change the volume while */
/*									playing. sets the output volume 0-100 */

void
audioSetVolume(int volume)
{
	long al_params[] = { AL_LEFT_SPEAKER_GAIN, 0, AL_RIGHT_SPEAKER_GAIN, 0};

	al_params[1] = al_params[3] = volume * 100 / 255;

	ALsetparams(AL_DEFAULT_DEVICE, al_params, 4);
}


/* audioFlush() */
/* should flush the audio device */

void
audioFlush()
{
	DB(audio, msg("audio: flush %d\n",audio_fd) );
}


/* audioClose() */
/* should close the audio device and perform any special shutdown			 */

void
audioClose()
{
int write_fd;

	/* wait for all samples to be played */

	write_fd = ALgetfd(audioport);
	if (write_fd >= 0) {
		fd_set write_fds;

		FD_ZERO(&write_fds);
		FD_SET(write_fd, &write_fds);

		ALsetfillpoint(audioport, AUSIZ * 2);
		select(write_fd + 1, NULL, &write_fds, NULL, NULL);
	}

	/* now close it */

	ALcloseport(audioport);
	DB(audio, msg("audio: closed %d\n",audio_fd) );
}


/* audioWrite */
/* writes count bytes from buffer to the audio device */
/* returns the number of bytes actually written */

int
audioWrite(char *buffer, int count)
{
	if (ALwritesamps(audioport, buffer, count / 2) == 0) {
		ALsetfillpoint(audioport, AUSIZ);
		return(count);
	} else
		return 0;
}

/* Let buffer.c have the audio descriptor so it can select on it. This
   means that the program is dependent on an file descriptor to
   work. Should really move the select's etc (with inlines of course) in
   here so that this is the ONLY file which has hardware dependent audio
   stuff in it. */

int
getAudioFd()
{
	return ALgetfd(audioport);
}

