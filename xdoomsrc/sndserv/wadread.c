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
//	WAD and Lump I/O, the second.
//	This time for soundserver only.
//	Welcome to Department of Redundancy Department. Again :-).
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "soundsrv.h"
#include "sounds.h"
#include "wadread.h"

#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include "strcmp.h"
#endif

#define strcmpi strcasecmp

int		*sfxlengths;
int		got_chgun;

typedef struct wadinfo_struct
{
    char	identification[4];		                 
    int		numlumps;
    int		infotableofs;
} wadinfo_t;

typedef struct filelump_struct
{
    int		filepos;
    int		size;
    char	name[8];
} filelump_t;

typedef struct lumpinfo_struct
{
    int		handle;
    int		filepos;
    int		size;
    char	name[8];
} lumpinfo_t;

lumpinfo_t	*lumpinfo;		                                
int		numlumps;
int		_numlumps;

void		**lumpcache;

//
// Something new.
// This version of w_wad.c does handle endianess.
//
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

// Way too many of those...
static void derror(char *msg)
{
    fprintf(stderr, "\nwadread error: %s\n", msg);
    exit(-1);
}

void strupr(char *s)
{
    while (*s)
	*s++ = toupper(*s);
}

int filelength(int handle)
{
    struct stat	fileinfo;
  
    if (fstat(handle, &fileinfo) == -1)
	fprintf(stderr, "Error fstating\n");

    return fileinfo.st_size;
}

int findlump(char *lumpname)
{
  int i;

  for (i = 0; i < _numlumps; i++)
    if (!strncmp(lumpname, lumpinfo[i].name, 8))
      return(i);
  return(-1);
}

void openwad(char *wadname)
{
    int		wadfile;
    int		tableoffset;
    int		tablelength;
    int		tablefilelength;
    int		i, j, k = 0;
    int		isiwad = 0;
    wadinfo_t	header;
    filelump_t	*filetable;

    // open and read the wadfile header
    wadfile = open(wadname, O_RDONLY);

    if (wadfile < 0)
	derror("Could not open wadfile");

    read(wadfile, &header, sizeof header);

    if (!strncmp(header.identification, "IWAD", 4))
    {
	isiwad = 1;
	numlumps = _numlumps = LONG(header.numlumps);
    }
    else if (!strncmp(header.identification, "PWAD", 4))
    {
	isiwad = 0;
	_numlumps = LONG(header.numlumps);
    }
    else
	derror("wadfile has weirdo header");

    tableoffset = LONG(header.infotableofs);
    tablelength = _numlumps * sizeof(lumpinfo_t);
    tablefilelength = _numlumps * sizeof(filelump_t);
    if (isiwad)
	lumpinfo = (lumpinfo_t *) malloc(tablelength);
    filetable = (filelump_t *) ((char*)lumpinfo + tablelength - tablefilelength);

    // get the lumpinfo table
    lseek(wadfile, tableoffset, SEEK_SET);
    read(wadfile, filetable, tablefilelength);

    // process the table to make the endianness right and shift it down
    for (i = 0; i < _numlumps; i++)
    {
	if (strncmp(filetable[i].name, "DS", 2))
	    continue;

	// merge pwads
	if (!isiwad)
	{
	    // uh, this is a GROSS HACK for the sfx lumps in xdoom.wad
	    if ((j = findlump(filetable[i].name)) != -1)
	    {
		if (!strncmp(filetable[i].name, "DSCHGUN", 7))
		{
		    k = sfx_chgun;
		    got_chgun++;
		} else if (!strncmp(filetable[i].name, "DSBEEP", 6))
		{
		    k = sfx_beep;
		} else
		    continue;
	    }
	}
	else
		k = i;

	//printf("adding sfx %.8s from file %s\n", filetable[i].name, wadname);

	strncpy(lumpinfo[k].name, filetable[i].name, 8);
	lumpinfo[k].handle = wadfile;
	lumpinfo[k].filepos = LONG(filetable[i].filepos);
	lumpinfo[k].size = LONG(filetable[i].size);
	// fprintf(stderr, "lump [%.8s] exists\n", lumpinfo[k].name);
    }
}

void *loadlump(char *lumpname, int *size)
{
    int		i;
    void	*lump;

    for (i = 0; i < numlumps; i++)
    {
	if (!strncasecmp(lumpinfo[i].name, lumpname, 8))
	    break;
    }

    if (i == numlumps)
    {
	// fprintf(stderr,
	//   "Could not find lumpname [%s]\n", lumpname);
	lump = 0;
    }
    else
    {
	lump = (void *) malloc(lumpinfo[i].size);
	lseek(lumpinfo[i].handle, lumpinfo[i].filepos, SEEK_SET);
	read(lumpinfo[i].handle, lump, lumpinfo[i].size);
	*size = lumpinfo[i].size;
    }

    return lump;
}

void *getsfx(char *sfxname, int *len)
{
    unsigned char	*sfx;
    unsigned char	*paddedsfx;
    int			i;
    int			size;
    int			paddedsize;
    char		name[20];

    sprintf(name, "ds%s", sfxname);

    sfx = (unsigned char *) loadlump(name, &size);

    // pad the sound effect out to the mixing buffer size
    paddedsize = ((size-8 + (SAMPLECOUNT-1)) / SAMPLECOUNT) * SAMPLECOUNT;
    paddedsfx = (unsigned char *) realloc(sfx, paddedsize+8);
    for (i = size; i < paddedsize+8; i++)
	paddedsfx[i] = 128;

    *len = paddedsize;
    return (void *) (paddedsfx + 8);
}

// This is the same hack than done in musserver, read the -file
// PWAD list from /proc/<pid>/cmdline.
// The game engine should tell the external processes which PWAD's
// to load, this hack is not portable, but it is better than
// nothing. One day I might do it the right way...

void read_extra_wads(void)
{
    pid_t	ppid;
    char	filename[1024];
    char	*doomwaddir;
    FILE	*fp;

    if (!(doomwaddir = getenv("DOOMWADDIR")))
	doomwaddir = ".";

    // merging with other PWADs doesn't work correct, disabled
    return;
    /* NOTREACHED */

    // load xdoom.wad, new chaingun sfx
    sprintf(filename, "%s/xdoom.wad", doomwaddir);
    if (!access(filename, R_OK))
	openwad(filename);

    ppid = getppid();
    sprintf(filename, "/proc/%d/cmdline", (int)ppid);

    if ((fp = fopen(filename, "r")))
    {
	char		*s;
	static char	buff[256];
	int		len, active = 0;

	len = fread(buff, 1, 256, fp);
	fclose(fp);
	for (s = buff; s < buff+len;)
	{
	    if (*s == '-')
		active = 0;
	    if (active)
		openwad(s);
	    else if (!strcmp(s, "-file"))
		active = 1;
	    s += strlen(s) + 1;
	}
    }
}
