/* this file is a part of amp software, (C) tomislav uzelac 1996,1997

	Origional code by: tomislav uzelac
	Modified by:
	* Dan Nelson - BSD mods.
	* Andrew Richards - moved code from audio.c and added mixer support etc

 */

/* Support for Linux and BSD sound devices */

#include "amp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "audioIO.h"

#ifdef HAVE_MACHINE_SOUNDCARD_H
#include <machine/soundcard.h>
#else
#include <linux/soundcard.h>
#endif

/* optimal fragment size */

int AUSIZ = 0;

/* declare these static to effectively isolate the audio device */

static int audio_fd;
static int mixer_fd;
static int volumeIoctl;

/* audioOpen() */
/* should open the audio device, perform any special initialization		 */
/* Set the frequency, no of channels and volume. Volume is only set if */
/* it is not -1 */

void
audioOpen(int frequency, int stereo, int volume)
{
	int supportedMixers, play_format=AFMT_S16_LE;

	if ((audio_fd = open ("/dev/dsp", O_WRONLY, 0)) == -1)
		die("Unable to open the audio device\n");
	DB(audio, msg("Audio device opened on %d\n",audio_fd); )

	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT,&play_format) < 0)
		die("Unable to set required audio format\n");
	if ((mixer_fd=open("/dev/mixer",O_RDWR)) == -1)
		warn("Unable to open mixer device\n");
	DB(audio, msg("Mixer device opened on %d\n",mixer_fd) );

	if (ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &supportedMixers) == -1) {
		warn("Unable to get mixer info assuming master volume\n");
		volumeIoctl=SOUND_MIXER_WRITE_VOLUME;
	} else {
		if ((supportedMixers & SOUND_MASK_PCM) != 0)
			volumeIoctl=SOUND_MIXER_WRITE_PCM;
		else
			volumeIoctl=0;
	}

	/* Set 1 or 2 channels */
	stereo=(stereo ? 1 : 0);
	DB(audio, msg("Setting stereo to %d\n",stereo) )
	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo) < 0)
		die("Unable to set stereo/mono\n");

	/* Set the output frequency */
	DB(audio, msg("Setting freq to %d Hz\n",frequency) )
	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &frequency) < 0)
		die("Unable to set frequency: %d\n",frequency);

	if (volume != -1)
		audioSetVolume(volume);

	if (ioctl(audio_fd, SNDCTL_DSP_GETBLKSIZE, &AUSIZ) == -1)
		die("Unable to get fragment size\n");
}


/* audioSetVolume - only code this if your system can change the volume while */
/*									playing. sets the output volume 0-100 */

void
audioSetVolume(int volume)
{
	DB(audio, msg("Setting volume to: %d\n",volume); )

	volume=(volume<<8)+volume;
	if ((mixer_fd != -1) && (volumeIoctl!=0))
		if (ioctl(mixer_fd, volumeIoctl, &volume) < 0)
			warn("Unable to set sound volume\n");
}


/* audioFlush() */
/* should flush the audio device */

inline void
audioFlush()
{
	DB(audio, msg("audio: flush %d\n",audio_fd) );

	if (ioctl(audio_fd, SNDCTL_DSP_RESET, 0) == -1)
		die("Unable to reset audio device\n");
}


/* audioClose() */
/* should close the audio device and perform any special shutdown */

void
audioClose()
{
	close(audio_fd);
	if (mixer_fd != -1)
		close(mixer_fd);
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
/* that the program is dependent on a file descriptor to work. Should really */
/* move the select's etc (with inlines of course) in here so that this is the */
/* ONLY file which has hardware dependent audio stuff in it										*/

int
getAudioFd()
{
	return(audio_fd);
}
