/*
 * RS/6000 audio routines added by George L. Coulouris, 4/97
 * glc5@tc.cornell.edu
 */

#include "amp.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/audio.h>
#include <fcntl.h>

static audio_fd;
static audio_init a_init;
static audio_change a_change;
static audio_control a_control;

void
audioOpen(int frequency, int stereo, int volume)
{
if ((audio_fd = open ("/dev/paud0/1", O_WRONLY, 0)) == -1)
	die("unable to open the audio device\n");

DB(audio, msg("Audio device opened on %d\n",audio_fd) );

a_init.srate=frequency;
a_init.channels = (stereo ? 2 : 1);
a_init.bits_per_sample=16;
a_init.mode=PCM;
a_init.flags=FIXED | BIG_ENDIAN | TWOS_COMPLEMENT;
a_init.operation=PLAY;

a_change.balance=0x3fff0000;
a_change.balance_delay=0;
a_change.volume=  (long) ( (volume/100.0) * 0x7fffffff );

a_change.volume_delay=0;
a_change.monitor=AUDIO_IGNORE;
a_change.input=AUDIO_IGNORE;
a_change.output=OUTPUT_1;

a_control.ioctl_request=AUDIO_CHANGE;
a_control.position=0;
a_control.request_info = &a_change;

ioctl(audio_fd, AUDIO_INIT, &a_init);
ioctl(audio_fd, AUDIO_CONTROL, &a_control);

a_control.ioctl_request=AUDIO_START;

ioctl(audio_fd, AUDIO_CONTROL, &a_control);

}


void audioSetVolume(int volume)
{
a_change.volume=  (long) ( (volume/100.0) * 0x7fffffff );
ioctl(audio_fd, AUDIO_CONTROL, &a_control);
}

void audioFlush()
{
}

void audioClose()
{
close(audio_fd);
}

int inline
audioWrite(char *buffer, int count)
{
	return(write(audio_fd,buffer,count));
}

int
getAudioFd()
{
	return(audio_fd);
}

