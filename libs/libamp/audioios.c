/* this file is a part of amp software, (C) tomislav uzelac 1996,1997

	Origional code by: tomislav uzelac
	Modified by:
	* Andrew Richards - moved code from audio.c

 */

#include "amp.h"
#include <sys/types.h>
#include <sys/stropts.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/audioio.h>
#include "audioIO.h"


/* declare these static to effectively isolate the audio device */

static audio_fd;
static audio_info_t auinfo;

/* audioOpen() */
/* should open the audio device and perform any special initialization */
/* returns the file descriptior of the audio device										 */


void
audioOpen(int frequency, int stereo, int volume)
{
	int res;

	if ((audio_fd = open("/dev/audio",O_RDWR))==-1) {
		die(" unable to open the audio device\n");
	}
	DB(audio, msg("Audio device opened on %d\n",audio_fd) );

	if (ioctl(audio_fd,AUDIO_GETINFO,&auinfo)<0)
		die("Unable to get audio info\n");

	auinfo.play.precision=16;
	auinfo.play.encoding=AUDIO_ENCODING_LINEAR;
	auinfo.play.channels=(stereo ? 2 : 1);
	DB(audio, msg("setting sample rate to %d Hz",frequency) );
	auinfo.play.sample_rate=frequency;
	if (ioctl(audio_fd,AUDIO_SETINFO,&auinfo)<0)
		die("Unable to set audio info\n");

	if (volume != -1)
		audioSetVolume(volume);
}


/* audioSetVolume */
/* sets the output volume 0-100 */

void
audioSetVolume(int volume)
{
	int res;

	DB(audio, msg("Setting volume to: %d\n",volume) );
	auinfo.play.gain=(volume*255)/100;
	if (ioctl(audio_fd,AUDIO_SETINFO,&auinfo)<0)
		warn("Unable to set sound volume\n");
}


/* audioFlush() */
/* should flush the audio device */

inline void
audioFlush()
{
	DB(audio, msg("audio: flush %d\n",audio_fd) );
}


/* audioClose() */
/* should close the audio device and perform any special shutdown			 */

void
audioClose()
{
	close(audio_fd);
}


/* audioWrite */
/* writes count bytes from buffer to the audio device */
/* returns the number of bytes actually written */

int inline
audioWrite(char *buffer, int count)
{
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
