/*************************************************************************
 *  musserver.c
 *
 *  Copyright (C) 1995-1997 Michael Heasley (mheasley@hmc.edu)
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "musserver.h"

#if defined(SCOUW2)
#include "usleep.h"
#endif

extern int use_dev;
extern int seqfd;
extern int mixfd;
unsigned char *musicdata;
int doomver;
int verbose = 0;
int qid;
int changevol = 1;
FILE *infile;
char pproc[20];
#if !defined(SCOOS5)
int pcheck = 1;
#else
int pcheck = 0;
#endif

extern int readdir(FILE *wadfile, int version);
extern int readmus(int lumpnum);
extern int playmus(char *musdata, unsigned int mussize, int play_once);
extern void seq_setup(int pref_dev, int dev_num);
extern void read_genmidi(FILE *wadfile);
extern void list_devs(void);
extern void cleanup_midi(void);
extern void read_extra_wads(int doomver);
extern void readwad(char *s, int doomver);
extern void fmload(void);


void show_help(void)
{
  printf("Usage: musserver [options]\n\n");
#ifdef AWE32_SYNTH_SUPPORT
  printf("  -a               Use AWE32 synth device for music playback\n");
#endif
  printf("  -c               Do not check whether the parent process is alive\n");
  printf("  -d directory     Look in 'directory' for DOOM wad file\n");
  printf("  -f               Use FM synth device for music playback\n");
  printf("  -h               Print this message and exit\n");
  printf("  -l               List detected music devices and exit\n");
  printf("  -m               Use general midi device for music playback\n");
  printf("  -t number        Timeout after 'number' seconds when getting IPC message\n");
  printf("                   queue id\n");
  printf("  -u number        Use device of type 'number' where 'number' is the type\n");
  printf("                   reported by 'musserver -l'.  Requires -f or -m option.\n");
  printf("  -V               Ignore volume change messages from Doom\n");
  printf("  -v               Verbose\n\n");
}
 
void cleanup(int status)
{
struct msqid_ds *dummy;

  cleanup_midi();
  dummy = malloc(sizeof(struct msqid_ds));
  msgctl(qid, IPC_RMID, dummy);
  free(dummy);
  fclose(infile);
  exit(status);
}

int main(int argc, char **argv)
{
int done = 0;
int lump = -1;
int intro = 0;
int introa = 0;
int result;
char *wadfilename;
char *waddir;
unsigned int musicsize;
int x;
int outtro = 31;
int opt_dev = 0;
int num_dev = -1;
int playarg;
unsigned int timeout = 1500;
pid_t ppid;


  ppid = getppid();
  sprintf(pproc, "/proc/%d", (int)ppid);
  if (verbose > 1)
    printf("ppid %d %s\n", ppid, pproc);
  ppid = getpid();
  if (verbose > 1) 
    printf("pid %d %s\n", ppid, pproc);
  waddir = getenv("DOOMWADDIR");
  if (waddir == NULL)
    waddir = ".";

  while ((x = getopt(argc, argv, "acd:fhlmt:u:Vv")) != -1)
    switch (x)
      {
#ifdef AWE32_SYNTH_SUPPORT
      case 'a':
        opt_dev = AWE32_SYNTH;
        break;
#endif
      case 'c':
	pcheck = 0;
	break;
      case 'd':
        waddir = optarg;
        break;
      case 'f':
        opt_dev = FM_SYNTH;
        break;
      case 'h':
        show_help();
        exit(0);
        break;
      case 'l':
        list_devs();
        break;
      case 'm':
        opt_dev = EXT_MIDI;
        break;
      case 't':
        timeout = 5 * atoi(optarg);
        break;
      case 'u':
        num_dev = atoi(optarg);
        break;
      case 'V':
        changevol = 0;
        break;
      case 'v':
        verbose++;
        break;
      case '?': case ':':
        show_help();
        exit(1);
        break;
      }


  if (!opt_dev)
    opt_dev = DEFAULT_DEV;

#ifdef DEFAULT_TYPE
  if (num_dev == -1)
    num_dev = DEFAULT_TYPE;
#endif
    
  if (verbose > 1)
    printf("musserver version %s\n", MUS_VERSION);

  wadfilename = malloc(strlen(waddir) + strlen("/plutonia.wad") + 1);

  /*
   * carefull here, the order for looking at the wad files must be the
   * same as in xdoom and sndserver, else it might play the wrong tunes
   */
  sprintf(wadfilename, "%s/doom2.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 2;
    goto found;
  }

  sprintf(wadfilename, "%s/doomu.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 1;
    goto found;
  }

  sprintf(wadfilename, "%s/doom.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 1;
    goto found;
  }

  sprintf(wadfilename, "%s/doom1.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 0;
    goto found;
  }

  sprintf(wadfilename, "%s/plutonia.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 2;
    goto found;
  }

  sprintf(wadfilename, "%s/tnt.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 2;
    goto found;
  }

  sprintf(wadfilename, "%s/doom2f.wad", waddir);
  infile = fopen(wadfilename, "r");
  if (infile != NULL) {
    doomver = 2;
    goto found;
  }

  printf("musserver: game mode indeterminate.\n");
  exit(1);

  found:
  if (verbose)
    switch (doomver)
      {
      case 0:
        printf("Playing music for shareware DOOM\n");
        break;
      case 1:
        printf("Playing music for registered DOOM\n");
        break;
      case 2:
        printf("Playing music for DOOM II\n");
        break;
      }

  switch(doomver)
    {
    case 0: case 1:
      intro = 28;
      introa = 27;
      break;
    case 2:
      intro = 33;
      introa = 100;
      outtro = 100;
      break;
    }

/* read the wadfile, get music data */
  readdir(infile, doomver);
  if (doomver)	/*  if not shareware doom, scan for external PWADs */
     read_extra_wads(doomver);

  seq_setup(opt_dev, num_dev);

  if (use_dev == FM_SYNTH)
    {
    read_genmidi(infile);
    fmload();
    }

  qid = -1;
  x = 0;
  if (verbose)
    printf("getting message queue id...\n");
  while (qid == -1)
    {
    x++;
    qid = msgget((key_t)53075, 0);
    if (verbose > 1)
      printf("qid: %d\n",qid);
    if (x > timeout)
      qid = -2;
    if (qid == -1)
      {
      if (errno == ENOENT)
        usleep(200000);
      else
        qid = -2;
      }
    }
  if (qid == -2)
    {
    printf("musserver: could not get IPC message queue id, exiting.\n");
    cleanup(1);
    }

  printf("musserver: using %s\nmusserver: ready\n", wadfilename);
  free(wadfilename);

  playarg = 2;
  if (verbose)
    printf("Waiting for first message from Doom...\n");
  while (!done)
    {
    qid = msgget((key_t)53075, 0);
    if ((lump == outtro) || (lump == intro) || (lump == introa))
      playarg = 1;
    else if (lump >= 0)
      playarg = 0;
    else
      lump = intro;
    if ((verbose) && (playarg != 2))
      printf("Playing music resource number %d\n", lump + 1);
    musicsize = readmus(lump);
    result = playmus(musicdata, musicsize, playarg);
    free(musicdata);
    switch (result)
      {
      case TERMINATED:
        done = 1;
        if (verbose)
          printf("Terminated\n");
        break;
      default:
        if (result >= 500)
          lump = result - 500;
        else
          {
          done = 1;
          printf("musserver: unknown error in music playing, exiting\n");
          }
        break;
      }
    if (playarg == 2)
      playarg = 1;
    }

  cleanup(0);

  return 0;
}
