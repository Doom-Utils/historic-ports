/*************************************************************************
 *  playmus.c
 *
 *  Copyright (C) 1995-1996 Michael Heasley (mheasley@hmc.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <unistd.h>
#ifdef linux
#  include <signal.h>
#  include <errno.h>
#elif defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7) || defined(__FreeBSD__)
#  include <sys/signal.h>
#  include <errno.h>
#endif
#include <string.h>
#include <ctype.h>
#include "musserver.h"

void seqbuf_dump(void);
void reset_midi(void);
void note_off(int note, int channel, int volume);
void all_notes_off(void);
void note_on(int note, int channel, int volume);
void pitch_bend(int channel, int value);
void control_change(int controller, int channel, int value);
void patch_change(int patch, int channel);
void midi_wait(float time);
void midi_timer(int action);
void vol_change(int volume);
void cleanup(int status);
extern int qid;
extern int verbose;
extern int doomver;
extern int changevol;
extern char pproc[20];
extern int pcheck;

#define START 0
#define STOP 1
#define CONT 2

int returnval = 0;

static char *doom2names[] = {
        "D_RUNNIN", "D_STALKS", "D_COUNTD", "D_BETWEE", "D_DOOM", "D_THE_DA",
        "D_SHAWN", "D_DDTBLU", "D_IN_CIT", "D_DEAD", "D_STLKS2", "D_THEDA2",
        "D_DOOM2", "D_DDTBL2", "D_RUNNI2", "D_DEAD2", "D_STLKS3", "D_ROMERO",
        "D_SHAWN2", "D_MESSAG", "D_COUNT2", "D_DDTBL3", "D_AMPIE", "D_THEDA3",
        "D_ADRIAN", "D_MESSG2", "D_ROMER2", "D_TENSE", "D_SHAWN3", "D_OPENIN",
        "D_EVIL", "D_ULTIMA", "D_READ_M", "D_DM2TTL", "D_DM2INT" };

static char *doom1names[] = {
        "D_E1M1", "D_E1M2", "D_E1M3", "D_E1M4", "D_E1M5", "D_E1M6", "D_E1M7",
        "D_E1M8", "D_E1M9", "D_E2M1", "D_E2M2", "D_E2M3", "D_E2M4", "D_E2M5",
        "D_E2M6", "D_E2M7", "D_E2M8", "D_E2M9", "D_E3M1", "D_E3M2", "D_E3M3",
        "D_E3M4", "D_E3M5", "D_E3M6", "D_E3M7", "D_E3M8", "D_E3M9", "D_INTROA",
        "D_INTRO", "D_INTER", "D_VICTOR", "D_BUNNY" };

void quitmus()
{
  returnval = TERMINATED;
}

void do_nothing()
{
  signal(SIGHUP, do_nothing);
}

void get_mesg(int flags)
{
int result;
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
struct msgbuf *recv;
#elif defined(__FreeBSD__)
struct mymsg *recv;
#endif
int msize = 9;
int done = 0;
int paused = 0;
int x = 0;
FILE *tmp;


while (!done)
  {
  if (!paused)
    done = 1;
  recv = malloc(sizeof(long) + 9);
  result = msgrcv(qid, recv, msize, 0, flags + MSG_NOERROR);
  if (result > 0)
    {
    if (verbose >= 2)
      printf("ipc: errno = %d, result = %d, mtext = %s, qid = %d\n", errno, result, recv->mtext, qid);
    switch (recv->mtext[0])
      {
      case 'p':
	all_notes_off();
	midi_timer(STOP);
	paused = 1;
	done = 0;
	if (verbose)
	  printf("musserver: paused\n");
	break;
      case 'P':
	midi_timer(CONT);
	paused = 0;
	done = 1;
	if (verbose)
	  printf("musserver: resumed\n");
	break;
      case 'v': case 'V':
        if (changevol)
          vol_change(recv->mtext[1]);
        if (verbose)
          printf("musserver: volume change = %d\n", recv->mtext[1]);
        done = 0;
        break;
      case 'D': case 'd':
        for (x = 0; x < strlen(recv->mtext); x++)
          recv->mtext[x] = toupper(recv->mtext[x]);
        switch(doomver)
          {
          case 0: case 1:
            for (x = 0; x <= 31; x++)
              if (!strncmp(recv->mtext, doom1names[x], strlen(doom1names[x])))
                break;
            if (x == 32)
              x = 50;
            break;
          case 2:
            for (x = 0; x <= 34; x++)
              if (!strncmp(recv->mtext, doom2names[x], strlen(doom2names[x])))
                break;
            if (x == 35)
              x = 50;
            break;
          default:
            x = 50;
            break;
          }
        if (x != 50)
          returnval = 500 + x;
        else
          done = 0;
        break;
      }
    }
  else if (result == -1)
    switch (errno)
      {
      case EACCES:
        if (verbose)
          {
          printf("Could not receive IPC message: no read access permission\n");
          printf("Exiting...\n");
          }
        cleanup(1);
        break;
      case EFAULT:
        if (verbose)
          {
          printf("Could not receive IPC message: memory address is inaccessible\n");
          printf("Exiting...\n");
          }
        cleanup(1);
        break;
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
      case EIDRM:
        if (verbose)
          {
          printf("Could not receive IPC message: message queue has been removed\n");
          printf("Exiting...\n");
          }
        cleanup(1);
        break;
#endif
      case EINTR:
        if (verbose)
          printf("Could not receive IPC message: received an interrupt signal\n");
        break;
      case EINVAL:
        if (verbose)
          {
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
          printf("Could not receive IPC message: invalid message size or queue id\n");
#elif defined(__FreeBSD__)
          printf("Could not receive IPC message: message queue has been removed\n or invalid queue id\n");
#endif
          printf("Exiting...\n");
          }
        cleanup(1);
        break;
#ifdef __FreeBSD__
      case E2BIG:
        if (verbose)
          {
          printf("Could not receive IPC message: invalid message size or queue id\n");
          printf("Exiting...\n");
          }
        cleanup(1);
        break;
#endif
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
      case ENOMSG:
#elif defined(__FreeBSD__)
      case EAGAIN:
#endif
        break;
      }
  free(recv);
  }

/* Check to see if doom is still alive... */
  if (pcheck)
    {
    if ((tmp = fopen(pproc, "r")) != NULL)
      fclose(tmp);
    else
      {
      if (verbose)
        printf("musserver: parent process appears to be dead, exiting\n");
      cleanup(1);
      }
    }
}


int playmus(char *musdata, unsigned int mussize, int play_once)
{
unsigned char event;
unsigned char event1;
unsigned char event2;
unsigned char eventtype;
unsigned char channelnum;
unsigned char last = 0;
unsigned char notenum;
unsigned char notevol;
unsigned int pitchwheel;
unsigned char controlnum;
unsigned char controlval;
unsigned int ticks;
unsigned char tmpch;
unsigned int done = 0;
unsigned int lastvol[16];
unsigned int i;
unsigned int dticks;
double dtime;
double delaytime;
double curtime = 0.0;
char *muscopy;


signal(SIGHUP, do_nothing);
signal(SIGQUIT, quitmus);
signal(SIGINT, quitmus);
signal(SIGTERM, quitmus);
signal(SIGCONT, SIG_IGN);

returnval = 0;
muscopy = musdata;

if (play_once == 2)
  get_mesg(MSG_WAIT);


reset_midi();
midi_timer(START);


while(!done) {
  if (!returnval)
    get_mesg(IPC_NOWAIT);
  if (returnval != 0)
    return returnval;
  event = *musdata++;
  channelnum = event & 15;
  if (channelnum > 8)
    channelnum++;
  if (channelnum == 16)
    channelnum = 9;
  eventtype = (event >> 4) & 7;
  last = event & 128;

  switch (eventtype)
    {
    case 0:		/* note off */
      event1 = *musdata++;
      notenum = event1 & 127;
      note_off(notenum, channelnum, lastvol[channelnum]);
      break;

    case 1:		/* note on */
      event1 = *musdata++;
      notenum = event1 & 127;
      if (event1 & 128)
        {
        event2 = *musdata++;
        notevol = event2 & 127;
        lastvol[channelnum] = notevol;
        }
      note_on(notenum, channelnum, lastvol[channelnum]);
      break;

    case 2:		/* pitch wheel */
      event1 = *musdata++;
      pitchwheel = event1 / 2;
      pitch_bend(channelnum, pitchwheel);
      break;

    case 4:		/* midi controller change */
      event1 = *musdata++;
      controlnum = event1 & 127;
      event2 = *musdata++;
      controlval = event2 & 127;
      switch (controlnum)
        {
        case 0:		/* patch change */
          patch_change(controlval, channelnum);
          break;
        case 3:		/* volume */
          control_change(CTL_MAIN_VOLUME, channelnum, controlval);
          break;
        case 4:		/* pan */
          control_change(CTL_PAN, channelnum, controlval);
          break;
        }
      break;

    case 6:	/* end of music data */
        if (play_once)
          get_mesg(MSG_WAIT);
        else
          {
          musdata = muscopy;
          midi_timer(START);
          curtime = 0.0;
          last = 0;
          }
      break;

    case 3:	/* unknown, but contains data */
      musdata++;
      break;

    default:	/* unknown */
      break;
    }
  if (last)	/* next data portion is time data */
    {
    tmpch = *musdata++;
    ticks = tmpch & 127;
    while(tmpch & 128)
      {
      tmpch = *musdata++;
      ticks = (ticks * 128) + (tmpch & 127);
      }
    delaytime = (double)ticks / 1.4;
    dticks = (int)(delaytime / 10);
    for (i = 1; i < dticks; i++)
      {
      dtime = delaytime * (double)i / (double)dticks;
      midi_wait(curtime + dtime);
      if (!returnval)
        get_mesg(IPC_NOWAIT);
      if (returnval != 0)
        return returnval;
      }
    curtime += delaytime;
    midi_wait(curtime);
    }
  }
  return returnval;
}
