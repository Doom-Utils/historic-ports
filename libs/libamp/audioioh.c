/* this file is a part of amp software, (C) tomislav uzelac 1996,1997

	Origional code by: Lutz Vieweg
	Modified by:
	* Andrew Richards - moved code from audio.c

 */


#include "amp.h"
#include <sys/audio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/lock.h>
#include <unistd.h>
#include <stdio.h>
#include "audioIO.h"

/* declare these static to effectively isolate the audio device */

static int audio_fd;


/* audioOpen() */
/* should open the audio device, perform any special initialization		 */
/* Set the frequency, no of channels and volume. Volume is only set if */
/* it is not -1 */

void
audioOpen(int frequency, int stereo, int volume)
{
	int flags;
	int failed = 0;
	
	if ((audio_fd = open("/dev/audio",O_RDWR))==-1)
		die(" unable to open the audio device\n");

	DB(audio, msg("Audio device opened on %d\n",audio_fd); )
	
	if ((flags = fcntl (audio_fd, F_GETFL, 0)) < 0) {
		die("unable to set non-blocking mode for /dev/audio\n");
	}
	flags |= O_NDELAY;
	if (fcntl (audio_fd, F_SETFL, flags) < 0) {
		die("unable to set non-blocking mode for /dev/audio\n");
	}
	
	if ( ioctl(audio_fd, AUDIO_SET_DATA_FORMAT, AUDIO_FORMAT_LINEAR16BIT) < 0 ||
			 ioctl(audio_fd, AUDIO_SET_CHANNELS, stereo ? 2 : 1) < 0 ||
			 ioctl(audio_fd, AUDIO_SET_OUTPUT, AUDIO_OUT_SPEAKER | AUDIO_OUT_HEADPHONE
						 | AUDIO_OUT_LINE) < 0 ||
			 ioctl(audio_fd, AUDIO_SET_SAMPLE_RATE, frequency) < 0) {
		failed = -1;		
	}
	if (volume != -1) {
		struct audio_describe description;
		struct audio_gains gains;
		float fvolume = (float)volume / 100.0f;
		if (ioctl(audio_fd, AUDIO_DESCRIBE, &description)) {
			failed = -1;
		}
		if (ioctl (audio_fd, AUDIO_GET_GAINS, &gains)) {
			failed = -1;
		}
		
		gains.transmit_gain = (int)((float)description.min_transmit_gain +
																(float)(description.max_transmit_gain
																				- description.min_transmit_gain)
																* fvolume);
		
		/* gains.monitor_gain = description.min_monitor_gain; */ /* don't monitor ! */

		if (ioctl (audio_fd, AUDIO_SET_GAINS, &gains)) {
			failed = -1;
		}
	}
	
	if (ioctl(audio_fd, AUDIO_SET_TXBUFSIZE, 4096 * 8)) {
		failed = -1;
	}
	if (failed)
		die(" unable to setup /dev/audio\n");
}
	

/* audioSetVolume - only code this if your system can change the volume while */
/*									playing. sets the output volume 0-100 */

void
audioSetVolume(int volume)
{
	struct audio_describe description;
	struct audio_gains gains;
	int failed = 0;
	float fvolume = ((float)volume) / 100.0f;
	if (ioctl(audio_fd, AUDIO_DESCRIBE, &description)) {
		failed = -1;
	}
	if (ioctl (audio_fd, AUDIO_GET_GAINS, &gains)) {
		failed = -1;
}

	gains.transmit_gain = (int)((float)description.min_transmit_gain +
													(float)(description.max_transmit_gain
																- description.min_transmit_gain)
													* fvolume);
	if (ioctl (audio_fd, AUDIO_SET_GAINS, &gains)) {
		failed = -1;
	}
	
	/* could evaluate "failed" here - but who cares? */ 

	DB(audio, msg("volume set to %d%%\n",volume); )

}

/* audioFlush() */
/* should flush the audio device */

inline void
audioFlush()
{
	DB(audio, msg("audio: flush %d\n",audio_fd) );
}


/* audioClose() */
/* should close the audio device and perform any special shutdown */

void
audioClose()
{
	close(audio_fd);
	DB(audio, msg("audio: closed %d\n",audio_fd) );
}


/* audioWrite */
/* writes count bytes from buffer to the audio device */
/* returns the number of bytes actually written */

inline int
audioWrite(char *buffer, int count)
{
	DB(audio, msg("audio: Writing %d bytes to audio descriptor %d\n",count,getAudioFd()) );
	return(write(audio_fd,buffer,count));
}


/* Let buffer.c have the audio descriptor so it can select on it. This means	*/
/* that the program is dependent on an file descriptor to work. Should really */
/* move the select's etc (with inlines of course) in here so that this is the */
/* ONLY file which has hardware dependent audio stuff in it										*/

int
getAudioFd()
{
	return(audio_fd);
}

/*
	Try to set the priority of this process to a value which
	allows us to play without buffering, thus saving memory
	and avoiding cache-misses.
	If we cannot get any priority high enough to allow for
	undisturbed replay (because we don't have sufficient
	priviledges), return a zero, otherwise, return a one.
*/
int audioSetPriority(void) {
	
	/* try to lock process in physical memory, just ignore if this fails */
	plock(PROCSHLIBLOCK);
	
	/* try to set a realtime-priority of 64 */
	if (-1 != rtprio(0, 64)) {
		DB(audio, msg("using real-time priority\n"); )
		return 1; 
	}
	
	/* try to set a nice-level of -20 */
	if (-1 != nice(-20)) {
		DB(audio, msg("using nice-level -20\n"); )
		return 1; 
	}
	
	DB(audio, msg("using buffered output\n"); )
	return 0; /* need to use a buffer */
}
