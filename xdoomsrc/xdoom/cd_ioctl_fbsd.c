// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1999 by Priit Jaerv
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
//	This CD audio module uses the FreeBSD ioctl()
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
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/cdio.h>

#include "cd_audio.h"

#define DRIVE			"/dev/cdrom"	// the device to use
#define STARTTRACK		1

#define CDROM_DATA_TRACK	4
#define CDROM_AUDIO_COMPLETED	19

void start_timer(void);
void stop_timer(void);
void handle_timer(int);

static int fd;				// filedescriptor for the CD drive
static int track = STARTTRACK - 1;	// current playing track
static struct ioc_toc_header toc_header;// TOC header
static struct ioc_play_track ti;	// track/index to play
static struct ioc_vol volume;		// volume
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
    if (ioctl(fd, CDIOCSTART) != 0) {
	printf("CD audio (ioctl): can't start drive motor\n");
	return 1;	// failed, use musserver
    }

    // get the TOC header, so we know how many audio tracks
    if (ioctl(fd, CDIOREADTOCHEADER, &toc_header) != 0) {
	printf("CD audio (ioctl): can't read TOC header\n");
	return 1;	// failed, use musserver
    } else {
	printf("CD audio (ioctl): using tracks %d - %d\n",
		(int) toc_header.starting_track,
		(int) toc_header.ending_track);
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
    ioctl(fd, CDIOCSTOP);
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
    if (++track > toc_header.ending_track)
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
    struct ioc_read_toc_entry toc;
    struct cd_toc_entry buffer;
    int i = 0;

    while (i < 30) {
	    // get toc entry for the track
	    toc.starting_track = track;
	    toc.address_format = CD_MSF_FORMAT;
	    toc.data_len = sizeof(buffer);
	    toc.data = &buffer;
	    ioctl(fd, CDIOREADTOCENTRYS, &toc);

	    // is this an audio track?
	    if (buffer.control & CDROM_DATA_TRACK) {
		// nope, try next track
		CD_song((void *)"next");
		i++;
		continue;
	    }

	    // found audio track
	    break;
    }

    // all data track? forget this CD
    if (i == 30) {
	printf("CD audio (ioctl): CD has no audio tracks, can't play it\n");
	return;
    }

    ti.start_track = track;
    ti.start_index = 0;
    ti.end_track = track;
    ti.end_index = 0;
    ioctl(fd, CDIOCPLAYTRACKS, &ti);
    start_timer();
}

//
// XDoom calls this function to stop playing the current song.
//
void CD_stop(int handle)
{
    stop_timer();
    ioctl(fd, CDIOCSTOP);
}

//
// XDoom calls this function to pause the current playing song.
//
void CD_pause(int handle)
{
    stop_timer();
    ioctl(fd, CDIOCPAUSE);
}

//
// XDoom calls this function to resume playing the paused song.
//
void CD_resume(int handle)
{
    ioctl(fd, CDIOCRESUME);
    start_timer();
}

//
// XDoom calls this function whenever the volume for the music is changed.
// The ioctl() interface uses a volume setting of 0 - 255, so the values
// XDoom sends are mapped into this valid range.
//
void CD_volume(int newvol)
{
    if ((newvol >= 0) && (newvol <= 15)) {
	volume.vol[0] =  newvol * 255 / 15;
	volume.vol[1] =  newvol * 255 / 15;
	volume.vol[2] =  0;
	volume.vol[3] =  0;
	ioctl(fd, CDIOCSETVOL, &volume);
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
    struct ioc_read_subchannel s;
    struct cd_sub_channel_info data;

    bzero(&s, sizeof(s));
    s.data = &data;
    s.data_len = sizeof(data);
    s.address_format = CD_MSF_FORMAT;
    s.data_format = CD_CURRENT_POSITION;

    ioctl(fd, CDIOCREADSUBCHANNEL, &s);

    if (s.data->header.audio_status == CDROM_AUDIO_COMPLETED) {
    	ti.start_track = track;
    	ti.start_index = 0;
    	ti.end_track = track;
    	ti.end_index = 0;
	ioctl(fd, CDIOCPLAYTRACKS, &ti);
    }
}
