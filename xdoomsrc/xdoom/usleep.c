// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
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
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <signal.h>
#include <sys/types.h>

#ifdef SCOOS5
#include <sys/itimer.h>
#endif

#if defined(SCOUW2) || defined(SCOUW7) || defined(LINUX) || defined(FREEBSD) || defined(_AIX)
#include <sys/time.h>
#endif

#include "usleep.h"

volatile static int waiting;

static void getalrm(int i)
{
    waiting = 0;
}

void usleep(unsigned t)
{
    struct itimerval	it, ot;
    struct sigaction	ac, oc;
    sigset_t		ss, os, zs;
    long		nt;

    waiting = 1;

    sigemptyset(&zs);
    sigemptyset(&ss);
    sigaddset(&ss, SIGALRM);

    ac.sa_handler = getalrm;
    ac.sa_flags = SA_RESTART;
    sigemptyset(&ac.sa_mask);
    sigaction(SIGALRM, &ac, &oc);

    it.it_value.tv_sec = t / 1000000;
    it.it_value.tv_usec = t % 1000000;
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &it, &ot)) {
	sigaction(SIGALRM, &oc, (struct sigaction *)0);
	return /*error*/;
    }

    sigprocmask(SIG_BLOCK, &ss, &os);
    while (waiting)
	sigsuspend(&zs);
    sigprocmask(SIG_SETMASK, &os, (sigset_t *)0);

    if (ot.it_value.tv_sec + ot.it_value.tv_usec > 0) {
      nt = ((ot.it_value.tv_sec * 1000000L) + ot.it_value.tv_usec) - t;
      if (nt > 0) {
	ot.it_value.tv_sec = nt / 1000000;
	ot.it_value.tv_usec = nt % 1000000;
      }
    }

    sigaction(SIGALRM, &oc, (struct sigaction *)0);
    setitimer(ITIMER_REAL, &ot, (struct itimerval *)0);
}
