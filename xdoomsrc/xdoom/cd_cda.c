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
//	This CD audio support module uses Ti Kan's xmcd player, actually it uses
//	the command line version cda, which is part of the package. Advantage of
//	this player is, it's available for many different UNIX's and CD-ROM
//	drives. For informations about this fine program visit:
//		http://sunsite.unc.edu/~cddb/xmcd/welcome.html
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "cd_audio.h"
#include "searchp.h"

#define STARTTRACK 2	// the Quake CD starts on track 2 with audio
#define MAXTRACK 11	// are there audio CD's with less than 11 tracks?

static int track = STARTTRACK - 1;
static char buf[20];
static char cda[2048];

int callcda(int, char *, char *);

//
// This function is called once by XDoom to initialize the CD audio
// system. For cda we need to start it's daemon and we set it into
// repeat mode, in case the audio CD's end is reached.
//
// This function should return 0 if it was possible to initialize
// CD audio successfull. It should return another value if there
// was any problem, in this case XDoom will fall back to playing
// music with musserver.
//
int CD_init()
{
    printf("CD audio (cda): initializing...\n");
    if (getenv("XMCD_LIBDIR") == NULL) {
	printf("CD audio (cda): XMCD_LIBDIR not set\n");
	return 1;
    }

    strcpy(&cda[0], searchpath("cda"));	// find the cda binary for faster use

    if (callcda(1, "on", (char *)0) != 0) {
	printf("CD audio (cda): can't execute cda\n");
	return 1;
    }

    callcda(1, "repeat", "on");

    printf("CD audio (cda): ready\n");

    return 0;
}

//
// This function is called once by XDoom if the program terminates. For
// cda we shutdown it's daemon. If XDoom crashes we are in trouble...
//
void CD_shutdown()
{
    callcda(0, "off", (char *)0);
}

//
// XDoom registers the next song to play with the resource names found
// in IWAD files. One could think up an algorithm to map CD tracks to
// the sound tracks XDoom uses, but I don't have a good idea for that in
// the moment, so this implementation here is rather simple. It just
// plays one track after the other and in case you start a new game
// the CD is played from the beginning. Please note that the starting
// track is set to track 2. This is because of the Quake CD, track 1
// contains the program and the songs start with track 2. If you don't
// like this change the define STARTTRACK above.
//
void CD_song(void * data)
{
    if (++track > MAXTRACK)
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
//
void CD_start(int handle)
{
    sprintf(buf, "%d", track);
    callcda(1, "play", buf);
}

//
// XDoom calls this function to stop playing the current song.
//
void CD_stop(int handle)
{
    callcda(1, "stop", (char *)0);
}

//
// XDoom calls this function to pause the current playing song.
//
void CD_pause(int handle)
{
    callcda(1, "pause", (char *)0);
}

//
// XDoom calls this function to resume playing the paused song.
//
void CD_resume(int handle)
{
    callcda(1, "play", (char *)0);
}

//
// XDoom calls this function whenever the volume for the music is changed.
// Cda uses a volume setting of 0 - 100%, so the values XDoom sends
// are mapped into this valid range.
//
void CD_volume(int vol)
{
    if ((vol >= 0) && (vol <= 15)) {
        sprintf(buf, "%d", vol * 100 / 15);
        callcda(0, "volume", buf);
    }
}

// -------------------------------------------------------------------------

//
// This function calls the cda binary as fast as possible and it allows
// to wait until it terminates or just return, so XDoom won't hang until
// the work was done.
// This function should return 0 if it was successfull and another return
// value if there were problems.
//
int callcda(int wait_flag, char *arg1, char *arg2)
{
    pid_t pid, rpid;
    struct itimerval	value;
    struct sigaction    act;
    int status;

    // fork child process and execute cda
    if ((pid = fork()) == 0) {

	value.it_interval.tv_sec    = 0;
	value.it_interval.tv_usec   = 0;
	value.it_value.tv_sec       = 0;
	value.it_value.tv_usec      = 0;
	setitimer (ITIMER_VIRTUAL, &value, (struct itimerval *)0);
	setitimer (ITIMER_REAL, &value, (struct itimerval *)0);

	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, (struct sigaction *)0);
	sigaction(SIGVTALRM, &act, (struct sigaction *)0);
	sigaction(SIGUSR1, &act, (struct sigaction *)0);
	sigaction(SIGUSR2, &act, (struct sigaction *)0);

	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
        execl(cda, cda, arg1, arg2, (char *)0);
	_exit(-1);

    } else if (pid < 0) {
	return 1;
    } else {
	if (wait_flag) {
	    while ((rpid = wait(&status)) != pid)
		;
	    return(WEXITSTATUS(status));
	}
    }

    return 0;
}
