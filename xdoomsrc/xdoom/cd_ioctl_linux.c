// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1998, 1999 by Udo Munk
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
//	This CD audio module uses the Linux ioctl()
//	interface, to play audio CD's.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include "cd_audio.h"

#define DRIVE "/dev/cdrom"		// the device to use
#define STARTTRACK 1

void start_timer(void);
void stop_timer(void);
void handle_timer(int);

static int fd;				// filedescriptor for the CD drive
static int track = STARTTRACK - 1;	// current playing track
static struct cdrom_tochdr toc_header;	// TOC header
static struct cdrom_ti ti;		// track/index to play
static struct cdrom_volctrl volume;	// volume
static struct itimerval value, ovalue;	// timer for sub-channel control
static struct sigaction act, oact;

//
// This function is called once by XDoom to initialize the CD audio
// system.
//
// This function should return 0 if it was possible to initialize
// CD audio successfull. It should return another value if there
// was any problem, in this case XDoom will fall back to playing
// music with musserver.
//
int CD_init()
{
    printf("CD audio (ioctl): initializing...\n");

    // try to open the CD drive
    if ((fd = open(DRIVE, O_RDONLY)) < 0) {
	printf("CD audio (ioctl): can't open %s\n", DRIVE);
	return 1;	// failed, use musserver
    }

    // turn drives motor on, some drives won't work without
    if (ioctl(fd, CDROMSTART) != 0) {
	printf("CD audio (ioctl): can't start drive motor\n");
	return 1;	// failed, use musserver
    }

    // get the TOC header, so we know how many audio tracks
    if (ioctl(fd, CDROMREADTOCHDR, &toc_header) != 0) {
	printf("CD audio (ioctl): can't read TOC header\n");
	return 1;	// failed, use musserver
    } else {
	printf("CD audio (ioctl): using tracks %d - %d\n",
		(int) toc_header.cdth_trk0,
		(int) toc_header.cdth_trk1);
    }

    printf("CD audio (ioctl): ready\n");

    return 0;		// good lets play CD audio...
}

//
// This function is called once by XDoom if the program terminates.
// Just stop playing and close the drive...
//
void CD_shutdown()
{
    stop_timer();
    ioctl(fd, CDROMSTOP);
    close(fd);
}

//
// XDoom registers the next song to play with the resource names found
// in IWAD files. One could think up an algorithm to map CD tracks to
// the sound tracks XDoom uses, but I don't have a good idea for that in
// the moment, so this implementation here is rather simple. It just
// plays one track after the other and in case you start a new game
// the CD is played from the beginning.
//
void CD_song(void *data)
{
    if (++track > toc_header.cdth_trk1)
	track = STARTTRACK;

    // check for a new game started
    if ((strcmp((char *)data, "runnin") == 0) ||
        (strcmp((char *)data, "e1m1") == 0) ||
        (strcmp((char *)data, "e2m1") == 0) ||
        (strcmp((char *)data, "e3m1") == 0) ||
        (strcmp((char *)data, "e4m1") == 0))
	track = STARTTRACK;
}

//
// XDoom calls this function to start playing the song registered before.
// We'll deal with CD's containing data tracks only correctly by
// ignoring it.
//
void CD_start(int handle)
{
    struct cdrom_tocentry toc;
    int i = 0;

    while (i < 30) {
	    // get toc entry for the track
	    toc.cdte_track = track;
	    toc.cdte_format = CDROM_MSF;
	    ioctl(fd, CDROMREADTOCENTRY, &toc);

	    // is this an audio track?
	    if (toc.cdte_ctrl & CDROM_DATA_TRACK) {
		// nope, try next track
		CD_song((void *)"next");
		i++;
		continue;
	    }

	    // found audio track
	    break;
    }

    // all data track? forget about this CD
    if (i == 30) {
	printf("CD audio (ioctl): CD has no audio tracks, can't play it\n");
	return;
    }

    ti.cdti_trk0 = track;
    ti.cdti_ind0 = 0;
    ti.cdti_trk1 = track;
    ti.cdti_ind1 = 0;
    ioctl(fd, CDROMPLAYTRKIND, &ti);
    start_timer();
}

//
// XDoom calls this function to stop playing the current song.
//
void CD_stop(int handle)
{
    stop_timer();
    ioctl(fd, CDROMSTOP);
}

//
// XDoom calls this function to pause the current playing song.
//
void CD_pause(int handle)
{
    stop_timer();
    ioctl(fd, CDROMPAUSE);
}

//
// XDoom calls this function to resume playing the paused song.
//
void CD_resume(int handle)
{
    ioctl(fd, CDROMRESUME);
    start_timer();
}

//
// XDoom calls this function whenever the volume for the music is changed.
// The ioctl() interface uses a volume setting of 0 - 255, so the values
// XDoom sends are mapped into this valid range.
//
void CD_volume(int vol)
{
    if ((vol >= 0) && (vol <= 15)) {
	volume.channel0 =  vol * 255 / 15;
	volume.channel1 =  vol * 255 / 15;
	volume.channel2 =  0;
	volume.channel3 =  0;
	ioctl(fd, CDROMVOLCTRL, &volume);
    }
}

//
// start a 1 second timer
//
void start_timer()
{
    act.sa_handler = handle_timer;
    act.sa_flags = SA_RESTART;
    sigemptyset(&act.sa_flags);
    sigaction(SIGALRM, &act, &oact);

    value.it_interval.tv_sec	= 1;
    value.it_interval.tv_usec	= 0;
    value.it_value.tv_sec	= 1;
    value.it_value.tv_usec	= 0;
    setitimer(ITIMER_REAL, &value, &ovalue);
}

//
// stop the timer
//
void stop_timer()
{
    sigaction(SIGALRM, &oact, (struct sigaction *)0);
    setitimer(ITIMER_REAL, &ovalue, (struct itimerval *)0);
}

//
// Check the CD drives sub-channel information, if the track is completed
// and if so, restart it.
//
void handle_timer(int val)
{
    static struct cdrom_subchnl subchnl;

    subchnl.cdsc_format = CDROM_MSF;	// get result in MSF format
    ioctl(fd, CDROMSUBCHNL, &subchnl);

    if (subchnl.cdsc_audiostatus == CDROM_AUDIO_COMPLETED) {
	ti.cdti_trk0 = track;
	ti.cdti_ind0 = 0;
	ti.cdti_trk1 = track;
	ti.cdti_ind1 = 0;
	ioctl(fd, CDROMPLAYTRKIND, &ti);
    }
}
