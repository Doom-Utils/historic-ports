/*************************************************************************
 *  readwad.c
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
#include <string.h>
#include <unistd.h>
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include "musserver.h"

static unsigned long lumpsize[35] = {	/* size in bytes of data lumps */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1};

static unsigned long lumppos[35] = {	/* position of lumps in wad file */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1};

static FILE *lumpfp[35];		/* FILE * */

extern unsigned char *musicdata;
extern struct opl_instr fm_instruments[175];
extern struct sbi_instrument fm_sbi[175];
extern void cleanup(int status);


#define WADHEADER 8	/* The wad header is 12 bytes, but we only care about
			   the last 4.  Thus we skip the first 8 */

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

unsigned long genmidipos;

#ifndef BIGEND
#define LONG(x) (x)
#define SHORT(x) (x)
#else
#define LONG(x) ((long)SwapLONG((unsigned long) (x)))
#define SHORT(x) ((short)SwapSHORT((unsigned short) (x)))
unsigned long SwapLONG(unsigned long x)
{
    return
        (x>>24)
        | ((x>>8) & 0xff00)
        | ((x<<8) & 0xff0000)
        | (x<<24);
}
unsigned short SwapSHORT(unsigned short x)
{
    return
        (x>>8) | (x<<8);
}
#endif

int readdir(FILE *wadfile, int version) {
    int inmus = 0;		/* flag */
    long count = 0;		/* counting variable */
    long dirpos, tmp;		/* position of directory in wad file */
    int lumpindex = 0;		/* index (0 to 34) of current lump */
    char lumpname[10];		/* name of current lump */
    int musiclump;		/* tells whether current lump contains music */
    int found = 0;

    /* skip first 8 bytes to find and read the wad directory offset */
    fseek(wadfile, WADHEADER, SEEK_SET);
    fread((char*)&tmp, 4, 1, wadfile);
    dirpos = LONG(tmp);
    
    /* seek to the name of the first wad directory entry */
    fseek(wadfile, dirpos + 8, SEEK_SET);
    
    /* loop to find positions of all the music entries */
    while (!inmus) {
	if (fread(&lumpname, 8, 1, wadfile) <= 0)
	    break;
	lumpname[8] = '\0';
	if (!strncmp(lumpname, "D_", 2)) {
	    musiclump = 1;
	    switch (version) {
	    case 0: case 1:
		for (lumpindex = 0; lumpindex <= 31; lumpindex++)
		    if (!strncmp(lumpname, doom1names[lumpindex],
				 strlen(doom1names[lumpindex])))
			break;
		if (lumpindex == 32) {
		    printf("musserver: unrecognized music entry (%s)\n",
			   lumpname);
		    musiclump = 0;
		}
		break;
          case 2:
		for (lumpindex = 0; lumpindex <= 34; lumpindex++)
		    if (!strncmp(lumpname, doom2names[lumpindex],
				 strlen(doom2names[lumpindex])))
			break;
		if (lumpindex >= 35) {
		    printf("musserver: unrecognized music entry (%s)\n",
			   lumpname);
		    musiclump = 0;
		}
		break;
	    }
	    
	    if (musiclump) {
		fseek(wadfile, -16, SEEK_CUR);
                fread((char*)&tmp, 4, 1, wadfile);
                lumppos[lumpindex] = LONG(tmp);
                fread((char*)&tmp, 4, 1, wadfile);
                lumpsize[lumpindex] = LONG(tmp);
		lumpfp[lumpindex] = wadfile;
		fseek(wadfile, 16, SEEK_CUR);
		++found;
	    }
	} else if (!strncmp(lumpname, "GENMIDI", 7)) {
	    fseek(wadfile, -16, SEEK_CUR);
            fread((char*)&tmp, 4, 1, wadfile);
            genmidipos = LONG(tmp);
	    fseek(wadfile, 20, SEEK_CUR);
	} else if (!strcmp(lumpname, "F_END")) {
	    break;
        }
	count++;
	if (feof(wadfile))
	    inmus = 1;
    }
    return found;
}


int readmus(int lumpnum)
{
struct mus_header {			/* header of music lump */
	char		id[4];
	unsigned short	music_size;
	unsigned short	header_size;
	unsigned short	channels;
	unsigned short	sec_channels;
	unsigned short	instrnum;
	unsigned short	dummy;
} header;
FILE *wadfile;
short tmp;

  wadfile = lumpfp[lumpnum];
  fseek(wadfile, lumppos[lumpnum], SEEK_SET);
  fread(&header, 1, sizeof(header), wadfile);
  tmp = header.music_size; header.music_size = SHORT(tmp);
  tmp = header.header_size; header.header_size = SHORT(tmp);


  fseek(wadfile, lumppos[lumpnum], SEEK_SET);

  fseek(wadfile, header.header_size, SEEK_CUR);

  musicdata = malloc(header.music_size);
  if (musicdata == NULL)
    {
    printf("musserver: could not allocate %d bytes for music data, exiting.\n", header.music_size);
    cleanup(1);
    }
  fread(musicdata, 1, header.music_size, wadfile);
  return header.music_size;
}

void read_genmidi(FILE *wadfile)
{
char header[9];

  fseek(wadfile, genmidipos, SEEK_SET);
  fread(&header, 1, 8, wadfile);
  if (strncmp(header, "#OPL_II#", 8))
    {
    printf("musserver: couldn't find GENMIDI entry in wadfile, exiting.\n");
    cleanup(1);
    }
  fread(&fm_instruments, sizeof(struct opl_instr), 175, wadfile);
}

static void readwad(char *s, int doomver) {
    char *waddir, *wadfilename;
    FILE *fp;

    if (!(waddir = getenv("DOOMWADDIR")))
        waddir = ".";

    wadfilename = malloc(2 + strlen(s) + strlen(waddir));
    if ((*s != '/') && (*s != '.'))
        sprintf(wadfilename, "%s/%s", waddir, s);
    else
        sprintf(wadfilename, "%s", s);
    fflush(stdout);
    fprintf(stderr, "musserver: trying external PWAD %s\n", wadfilename);
    if ((fp = fopen(wadfilename, "r"))) {
        int found;
        found = readdir(fp, doomver);
        fprintf(stderr, "%d soundtracks found in %s\n", found, wadfilename);
        if (!found)
            fclose(fp); /* don't need it */
    }
    free(wadfilename);
}

void read_extra_wads(int doomver) {
    pid_t ppid;
    char filename[100];
    FILE *fp;

    ppid = getppid();
    sprintf(filename, "/proc/%d/cmdline", (int)ppid);
    if ((fp = fopen(filename, "r"))) {
        char *s;
        static char buff[256];
        int len, active = 0;
        len = fread(buff, 1, 256, fp);
        fclose(fp);
        /*fprintf(stderr, "musserver: cmdline of parent is: "); */
        for (s = buff; s < buff+len;) {
            if (*s == '-')
                active = 0;
            if (active)
                readwad(s, doomver);
            else if (!strcmp(s, "-file"))
                active = 1;
            /*fprintf(stderr, " %s", s); */
            s += strlen(s) + 1;
        }
        /*fprintf(stderr, "\n"); */
    }
}
