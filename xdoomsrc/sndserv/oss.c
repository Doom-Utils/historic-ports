// Emacs style mode select   -*- C++ -*- 
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:
//	Soundserver for UNIX's with 4Front OSS sound driver.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef LINUX
#include <linux/soundcard.h>
#endif

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif

#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include <sys/soundcard.h>
#endif

#include <sys/ioctl.h>

#include "soundsrv.h"

int	audio_fd;
int	audio_8bit_flag;

void myioctl(int fd, int command, int *arg)
{   
    int		rc;
    extern int	errno;
    
    rc = ioctl(fd, command, arg);  
    if (rc < 0)
    {
	fprintf(stderr, "ioctl(dsp,%d,arg) failed\n", command);
	fprintf(stderr, "errno=%d\n", errno);
	exit(-1);
    }
}

void I_InitMusic(void)
{
}

void
I_InitSound(int samplerate, int samplesize)
{
    int i;

    audio_fd = open("/dev/dsp", O_WRONLY);
    if (audio_fd<0)
    {
        fprintf(stderr, "Could not open /dev/dsp\n");
	return;
    }

#if 0
    // causes problems with old sound drivers, don't do it
    myioctl(audio_fd, SNDCTL_DSP_RESET, 0);
#endif

    if (getenv("DOOM_SOUND_SAMPLEBITS") == NULL)
    {
        myioctl(audio_fd, SNDCTL_DSP_GETFMTS, &i);
        if (i &= AFMT_S16_LE)
        {
            myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
            i = 11 | (2<<16);                                           
            myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
	    i = 1;    
	    myioctl(audio_fd, SNDCTL_DSP_STEREO, &i);
            fprintf(stderr, "sndserver: Using 16 bit sound card\n");
        }
        else
        {
	    i = AFMT_U8;
            myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
            i = 10 | (2<<16);                                           
            myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
            audio_8bit_flag++;
            fprintf(stderr, "sndserver: Using 8 bit sound card\n");
        }
    }
    else
    {
        i = AFMT_U8;
        myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
        i = 10 | (2<<16);                                           
        myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
        audio_8bit_flag++;
        fprintf(stderr, "sndserver: Using 8 bit sound card\n");
    }

    i = 11025;
    myioctl(audio_fd, SNDCTL_DSP_SPEED, &i);
}

void I_SubmitOutputBuffer(void *samples, int samplecount)
{
    if (audio_fd >= 0)
    {
	if (!audio_8bit_flag)
	    write(audio_fd, samples, samplecount*4);
	else
	    write(audio_fd, samples, samplecount);
    }
}

void I_ShutdownSound(void)
{
    if (audio_fd >= 0)
	close(audio_fd);
}

void I_ShutdownMusic(void)
{
}
