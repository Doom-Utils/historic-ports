/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: l_soundsrv.c,v 1.16 1999/10/12 13:01:11 cphipps Exp $
 *
 *  Sound server for LxDoom, based on the sound server released with the 
 *   original linuxdoom sources.
 *  Copyright (C) 1993-1996 by id Software
 *  Copyright (C) 1999 by Colin Phipps
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Interface to the main program. Initially accepts the sound 
 *  data on stdin. Then accepts sound commands on stdin, adding 
 *  sounds to the playing sound list. Calls the low level sound
 *  output functions at regular intervals.
 *-----------------------------------------------------------------------------
 */

static const char
rcsid[] = "$Id: l_soundsrv.c,v 1.16 1999/10/12 13:01:11 cphipps Exp $";

#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#define SNDSERV
#include "sounds.h"
#include "l_soundgen.h"
#include "l_soundsrv.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define USE_SELECT

#ifndef USE_SELECT
#include <sys/ioctl.h>
#endif

static int snd_verbose;

int  I_GetTime_RealTime (void)
{
  static struct timeval tv;
  static struct timezone tz;
  static int basetime = 0;

  gettimeofday(&tv, &tz);

  if (!basetime) basetime = tv.tv_sec;

  return (tv.tv_sec * 35 + (tv.tv_usec * 35) / 1000000);
}

static void I_GetData(void) 
{
  switch (pass_by) {
  case SP_IPC:
    {
      struct shmid_ds shminfo;
      int             shmid = shmget(snd_ipc_key, sizeof(snd_ipc_t), 0);
      snd_ipc_t*      buf;
      int i;
      int count = 10000; // In milliseconds
      
      if (shmid == -1) {
	fprintf(stderr, "Failed to get shared memory\n");
	exit(-1);
      }
      
      buf = (snd_ipc_t*)shmat(shmid, NULL, 0);

      if (snd_verbose)
	fprintf(stderr, "Attached shared memory\n");
      
      for (i=1 ; i<NUMSFX ; i++) { 
	// Alias? Example is the chaingun sound linked to pistol.
	if (!S_sfx[i].link) {
	  // Load data via IPC.
	  buf->req_num = i;
	  while (buf->srv_num != i) {
	    usleep(1000);
	    if (!--count) goto cleanup;
	  }
	  count = 1000; // Only 1 second timeout once connected
	  lengths[i] = buf->datalen;
	  S_sfx[i].data = (unsigned char*)I_PadSfx(buf->data, &lengths[i]);
	} else {
	  // Previously loaded already?
	  S_sfx[i].data = S_sfx[i].link->data;
	  lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
	}
      }
    cleanup:
      buf->req_num = NUMSFX; // Signal done;
      fprintf(stderr, "Detaching shared memory\n");
      shmdt((char*)buf);
      shmctl(shmid, IPC_RMID, &shminfo);
      if (!count) {
	fprintf(stderr, "I_GetData: Timeout on IPC\n");
	exit(-1);
      }
      break;
    }
  case SP_PIPE:
    {
      // This has been generalised to accept sound data not matching that expected.
      // The only restriction is that there are fewer than NUMSFX sounds.
      int i;
      unsigned long numsounds;
      
      fread(&numsounds, sizeof(numsounds), 1, stdin);

      for (i=0; i<numsounds; i++) {
	snd_pass_t sfxpass;
	void* databuf = NULL;
	void* crapbuf = NULL;
	
	fread(&sfxpass, sizeof(sfxpass), 1, stdin);
	
	if (sfxpass.sfxid != i) {
	  fprintf(stderr, "I_GetData: Sound ID mismatch\n");
	  exit(-1);
	}
	
	// If we are within the bounds of our array...
	if (i < NUMSFX)
	  if (sfxpass.link == -1) {
	    if (sfxpass.datalen) {
	      databuf = malloc(lengths[i] = sfxpass.datalen);
	      fread(databuf, 1, sfxpass.datalen, stdin);
	      
	      S_sfx[i].data = (unsigned char*)I_PadSfx(databuf, &lengths[i]);
	    } else {
	      // No data available. Make it safe
	      S_sfx[i].data = S_sfx[0].data;
	      lengths[i] = 0;
	    }
	  } else {
	    unsigned int link_num = (unsigned int)(sfxpass.link);
	    S_sfx[i].data = S_sfx[link_num].data;
	    lengths[i]=lengths[link_num];
	  }
	else if (sfxpass.datalen) {
	  // We cannot hold this data, but must clear it from the pipe
	  crapbuf = malloc(sfxpass.datalen);
	  fread(crapbuf, 1, sfxpass.datalen, stdin);
	  free(crapbuf);
	}
      }
      break;
    }
  }
}

#ifdef USE_SELECT
static fd_set		fdset;
static fd_set		scratchset;
#endif

int main(int argc, const char** argv)
{
#ifndef USE_SELECT
    int         trueval = 1;
#else
    struct timeval	zerowait = { 0, 0 };
#endif
    int		done = 0;
    int		rc, nrc;
    int		sndnum;
    int		handle = 0;
    int         badcmd = 0;
    
    unsigned char	commandbuf[10];
    
    int 	pitch, vol, sep;
    int         thistime, lasttime = I_GetTime_RealTime();

    // Debugging output?
    snd_verbose = 0;
    if (argc >= 3)
      if (!stricmp(argv[2], "-devparm"))
	snd_verbose = 1;
    if (argc >= 4)
      if (!stricmp(argv[3], "-devparm"))
	snd_verbose = 1;

    pass_by = SP_PIPE; // Use pipe to pass sound data to sound server
    if (argc >= 3)
      if (!stricmp(argv[2], IPC_OPT_STR)) {
	pass_by = SP_IPC; // Unless optioned
	if (snd_verbose)
	  fprintf(stderr, "Using ipc\n");
      }
    if (argc >= 4)
      if (!stricmp(argv[3], IPC_OPT_STR)) {
	pass_by = SP_IPC; // Unless optioned
	if (snd_verbose)
	  fprintf(stderr, "Using ipc\n");
      }

    I_InitSoundGen((argv[1] != NULL) ? argv[1] : "/dev/dsp");

    usleep(200000);
    // get sound data
    I_GetData();

    //    I_InitMusic();

    if (snd_verbose)
      fprintf(stderr, "ready\n");
    
#ifdef USE_SELECT
    // parse commands and play sounds until done
    FD_ZERO(&fdset);
    FD_SET(STDIN_FILENO, &fdset);
#endif

    while (!done) 
      {
        // Sync at 35Hz
        while (lasttime == (thistime = I_GetTime_RealTime())) {
	  usleep(1000);
        }
      
	lasttime = thistime;

	do {
#ifdef USE_SELECT
	  scratchset = fdset;
	  rc = select(FD_SETSIZE, &scratchset, 0, 0, &zerowait);
	  
	  if (rc > 0)
#endif 
	  {
	    //	fprintf(stderr, "select is true\n");
	    // got a command
	    rc = 1;
	    nrc = read(STDIN_FILENO, commandbuf, 1);

	    if (nrc <= 0) {
#ifdef USE_SELECT
	      done = 1;
	      if (snd_verbose) 
		fprintf(stderr, "select true but no data: exiting\n");
#endif
	      rc = 0;
	    }
	    else {
	      if (snd_verbose)
		fprintf(stderr, "cmd: %c", commandbuf[0]);
	      
	      switch (commandbuf[0]) {
	      case 'p':
		// play a new sound effect
		read(STDIN_FILENO, commandbuf, 9);
		
		if (snd_verbose) {
		  commandbuf[9]=0;
		  fprintf(stderr, "%s\n", commandbuf);
		}
		
		commandbuf[0] -=
		  commandbuf[0]>='a' ? 'a'-10 : '0';
		commandbuf[1] -=
		  commandbuf[1]>='a' ? 'a'-10 : '0';
		commandbuf[2] -=
		  commandbuf[2]>='a' ? 'a'-10 : '0';
		commandbuf[3] -=
		  commandbuf[3]>='a' ? 'a'-10 : '0';
		commandbuf[4] -=
		  commandbuf[4]>='a' ? 'a'-10 : '0';
		commandbuf[5] -=
		  commandbuf[5]>='a' ? 'a'-10 : '0';
		commandbuf[6] -=
		  commandbuf[6]>='a' ? 'a'-10 : '0';
		commandbuf[7] -=
		  commandbuf[7]>='a' ? 'a'-10 : '0';
		
		//	p<snd#><pitch><vol><sep>
		sndnum = (commandbuf[0]<<4) + commandbuf[1];
		pitch = (commandbuf[2]<<4) + commandbuf[3];
		vol = (commandbuf[4]<<4) + commandbuf[5];
		sep = (commandbuf[6]<<4) + commandbuf[7];
		
		if (sndnum < NUMSFX)
		  handle = I_AddSfx(sndnum, vol, pitch, sep);
		// returns the handle
		//	outputushort(handle);
		break;
		
	      case 'q':
		read(STDIN_FILENO, commandbuf, 1);
		done = 1; rc = 0;
		break;
		
	      case 's':
		{
		  int fd;
		  read(STDIN_FILENO, commandbuf, 3);
		  commandbuf[2] = 0;
		  fd = open((char*)commandbuf, O_CREAT|O_WRONLY, 0644);
		  commandbuf[0] -= commandbuf[0]>='a' ? 'a'-10 : '0';
		  commandbuf[1] -= commandbuf[1]>='a' ? 'a'-10 : '0';
		  sndnum = (commandbuf[0]<<4) + commandbuf[1];
		  write(fd, S_sfx[sndnum].data, lengths[sndnum]);
		  close(fd);
		}
	      break;
	      
	      default:
		if (!badcmd) // cph - first char of bad command sequence
		  fprintf(stderr, "sndserver: Bad command:");
		badcmd += 2;
		fprintf(stderr, "`%c' %d",commandbuf[0], commandbuf[0]);
		break;
	      }
	      // cph - carriage return after bad command line
	      if (badcmd)
		if (!(badcmd>>=1)) putc('\n',stderr);
	    }
	  }
#ifdef USE_SELECT
	  else if (rc < 0) {
	    exit(0);
	  }
#endif
	} while (rc > 0);
	
	
	I_UpdateSound();
	I_SubmitSound();
	
      }
    
    I_EndSoundGen();
    return 0;
}

/*
 * $Log: l_soundsrv.c,v $
 * Revision 1.16  1999/10/12 13:01:11  cphipps
 * Changed header to GPL
 *
 * Revision 1.15  1999/09/06 19:40:27  cphipps
 * Include config.h for autoconf stuff
 *
 * Revision 1.14  1999/01/27 19:57:48  cphipps
 * Fix headers to avoid FreeBSD warnings
 *
 * Revision 1.13  1999/01/03 13:00:32  cphipps
 * Improve bad command diagnostics
 *
 * Revision 1.12  1998/12/25 17:17:13  cphipps
 * Remove redundant 'waitingtofinish' code
 *
 * Revision 1.11  1998/12/02 20:31:49  cphipps
 * Sounds above the NUMSFX limit are correctly flushed from the pipe now
 *
 * Revision 1.10  1998/12/01 23:12:32  cphipps
 * Read the number of sounds from the pipe
 * Clamp sound numbers to NUMSFX
 *
 * Revision 1.9  1998/10/20 07:26:23  cphipps
 * Add sys/time.h for glibc systems
 *
 * Revision 1.8  1998/10/15 18:47:00  cphipps
 * Added #define SNDSERV so it will compile without errors even if unused
 *
 * Revision 1.7  1998/10/15 11:47:46  cphipps
 * Use STDIN_FILENO macro instead of 0 for portability
 *
 * Revision 1.6  1998/10/15 10:44:47  cphipps
 * Use select
 * Make diagnostics selectable with -devparm
 *
 * Revision 1.5  1998/10/13 17:54:54  cphipps
 * Both IPC and pipe data passing is supported, selected by command line param
 *
 * Revision 1.4  1998/10/11 08:18:38  cphipps
 * Switch to passing sound data via the pipe, instead of IPC
 *
 * Revision 1.3  1998/10/10 19:55:29  cphipps
 * Accept command-line parameter of sound device.
 *
 * Revision 1.2  1998/09/27 13:27:39  cphipps
 * Get rid of select() call, use non-blocking IO instead on stdin.
 * Add timeout on IPC
 *
 * Revision 1.1  1998/09/20 16:09:22  cphipps
 * Initial revision
 *
 */
