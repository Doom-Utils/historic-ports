// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-2000 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright (C) 1998 by Andre Majorel
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
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#ifdef NORMALUNIX
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(LINUX) || defined(SOLARIS)
#include <alloca.h>
#endif
#define O_BINARY		0
#endif

#include "doomtype.h"
#include "m_swap.h"
#include "i_system.h"
#include "z_zone.h"

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif
#include "w_wad.h"

#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include "strcmp.h"
#endif

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t		*lumpinfo;
int			numlumps;

void			**lumpcache;

void strupr(char *s)
{
    while (*s) { *s = toupper(*s); s++; }
}

int filelength (int handle)
{
    struct stat	fileinfo;

    if (fstat(handle, &fileinfo) == -1)
	I_Error("Error fstating");

    return fileinfo.st_size;
}

void ExtractFileBase(char *path, char *dest)
{
    char	*src;
    int		length;

    src = path + strlen(path) - 1;

    // back up until a \ or the start
    while (src != path
	   && *(src-1) != '\\'
	   && *(src-1) != '/')
    {
	src--;
    }

    // copy up to eight characters
    memset (dest, 0, 8);
    length = 0;

    while (*src && *src != '.')
    {
	if (++length == 9)
	    I_Error("Filename base of %s >8 chars", path);

	*dest++ = toupper((int)*src++);
    }
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

// AYM 1998-08-29
// Modified so that if the file is not found and its name is not
// absolute, it's looked for in /usr/local/share/doom and
// /usr/share/doom (in that order so that local files override
// global ones).
//
// The intended use of this feature is the following : the users
// should put the IWAD (or a symlink to it) and their PWADs in
// one of the standard directories.
//
// This is just a quick hack to make it easier for me to use
// texture wads, etc. If we wanted to implement this seriously,
// we should look into :
// - having separate directories for Doom-specific files and
//   Doom II-specific files and maybe also a common directory,
// - possibly adding /opt/share/doom,
// - letting the user specify a list of directories to look up,
//   in the config file, through some environment variable à la
//   PATH and through a command-line option à la -I/-L.
//
// Also, got to convince TeamTNT to do the same in Boom :).
//

int			reloadlump;
char			*reloadname;

void W_AddFile(char *filename)
{
    wadinfo_t		header;
    lumpinfo_t		*lump_p;
    unsigned		i;
    int			handle;
    int			length;
    int			startlump;
    filelump_t		*fileinfo;
    filelump_t		singleinfo;
    int			storehandle;

    // open the file and add to directory

    // handle reload indicator.
    if (filename[0] == '~')
    {
	filename++;
	reloadname = filename;
	reloadlump = numlumps;
    }

    if ((handle = open(filename, O_RDONLY | O_BINARY)) == -1)
    {
        // AYM 1998-08-29
        if (errno == ENOENT && *filename != '/')
	{
	    const char *const path = "/usr/local/share/doom/";
            char *newname;

	    newname = malloc(strlen(path) + strlen(filename) + 1);
	    if (newname == NULL)
	        I_Error("W_AddFile(): not enough memory to build path\n");
            sprintf(newname, "%s%s", path, filename);
            handle = open(newname, O_RDONLY | O_BINARY);
	    free(newname);
	    if (handle == -1 && errno == ENOENT)
	    {
		const char *const path = "/usr/share/doom/";
		char *newname;

		newname = malloc(strlen(path) + strlen(filename) + 1);
		if (newname == NULL)
		    I_Error("W_AddFile(): not enough memory to build path\n");
		sprintf(newname, "%s%s", path, filename);
		handle = open(newname, O_RDONLY | O_BINARY);
		free(newname);
	    }
	}
	if (handle == -1)
        {
	    printf (" couldn't open %s\n", filename);
	    return;
        }
        // AYM 1998-08-29
    }

    printf (" adding %s\n",filename);
    startlump = numlumps;

    if (strcasecmp(filename + strlen(filename) - 3, "wad"))
    {
	// single lump file
	fileinfo = &singleinfo;
	singleinfo.filepos = 0;
	singleinfo.size = LONG(filelength(handle));
	ExtractFileBase(filename, singleinfo.name);
	numlumps++;
    }
    else
    {
	// WAD file
	read(handle, &header, sizeof(header));
	if (strncmp(header.identification, "IWAD", 4))
	{
	    // Homebrew levels?
	    if (strncmp(header.identification, "PWAD", 4))
	    {
		I_Error("Wad file %s doesn't have IWAD "
			"or PWAD id\n", filename);
	    }
	}
	header.numlumps = LONG(header.numlumps);
	header.infotableofs = LONG(header.infotableofs);
	length = header.numlumps * sizeof(filelump_t);
	fileinfo = (filelump_t *)alloca(length);
	lseek(handle, header.infotableofs, SEEK_SET);
	read(handle, fileinfo, length);
	numlumps += header.numlumps;
    }

    // Fill in lumpinfo
    lumpinfo = realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));

    if (!lumpinfo)
	I_Error("Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];

    storehandle = reloadname ? -1 : handle;

    for (i = startlump; i < numlumps; i++, lump_p++, fileinfo++)
    {
	lump_p->handle = storehandle;
	lump_p->position = LONG(fileinfo->filepos);
	lump_p->size = LONG(fileinfo->size);
	strncpy(lump_p->name, fileinfo->name, 8);
    }

    if (reloadname)
	close(handle);
}

//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload(void)
{
    wadinfo_t		header;
    int			lumpcount;
    lumpinfo_t		*lump_p;
    unsigned		i;
    int			handle;
    int			length;
    filelump_t		*fileinfo;

    if (!reloadname)
	return;

    if ((handle = open(reloadname, O_RDONLY | O_BINARY)) == -1)
	I_Error("W_Reload: couldn't open %s", reloadname);

    read(handle, &header, sizeof(header));
    lumpcount = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = lumpcount * sizeof(filelump_t);
    fileinfo = (filelump_t *)alloca(length);
    lseek(handle, header.infotableofs, SEEK_SET);
    read(handle, fileinfo, length);

    // Fill in lumpinfo
    lump_p = &lumpinfo[reloadlump];

    for (i = reloadlump; i < reloadlump+lumpcount; i++, lump_p++, fileinfo++)
    {
	if (lumpcache[i])
	    Z_Free(lumpcache[i]);

	lump_p->position = LONG(fileinfo->filepos);
	lump_p->size = LONG(fileinfo->size);
    }

    close(handle);
}

//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
void W_InitMultipleFiles (char **filenames)
{
    int		size;

    // open all the files, load headers, and count lumps
    numlumps = 0;

    // will be realloced as lumps are added
    lumpinfo = malloc(1);

    for (; *filenames; filenames++)
	W_AddFile(*filenames);

    if (!numlumps)
	I_Error("W_InitFiles: no files found");

    // merge the sprites and flats namespaces
    W_MergeLumps("S_START", "S_END");
    W_MergeLumps("F_START", "F_END");

    // set up caching
    size = numlumps * sizeof(*lumpcache);
    lumpcache = malloc(size);

    if (!lumpcache)
	I_Error("Couldn't allocate lumpcache");

    memset(lumpcache, 0, size);

    W_InitLumpHash();
}

//
// Finds a marker if it's like S_START, but also if it's SS_START.
// This is from Boom.
//
static int IsMarker(char *marker, char *name)
{
    return !strncasecmp(name, marker, 8) ||
	   (*name == *marker && !strncasecmp(name + 1, marker, 7));
}

//
// Merge lumps between start and end markers.
// Basically from Boom too, but modified.
//
void W_MergeLumps(char *start, char *end)
{
    lumpinfo_t	*newlumpinfo;
    int		oldlumps;
    int		newlumps;
    int		in_block = 0;
    int		i;

    newlumpinfo = (lumpinfo_t *)alloca(numlumps * sizeof(lumpinfo_t));
    oldlumps = newlumps = 0;

    for (i = 0; i < numlumps; i++)
    {
	// process lumps in global namespace
	if (!in_block)
	{
	    // check for start of block
            if (IsMarker(start, lumpinfo[i].name))
	    {
		in_block = 1;
		if (!newlumps)
		{
		    newlumps++;
		    memset(newlumpinfo[0].name, 0, 8);
		    strcpy(newlumpinfo[0].name, start);
		    newlumpinfo[0].handle = -1;
		    newlumpinfo[0].position = newlumpinfo[0].size = 0;
		}
	    }
	    // else copy it
	    else
	    {
		lumpinfo[oldlumps++] = lumpinfo[i];
	    }
	}
	// process lumps in sprites or flats namespace
	else
	{
	    // check for end of block
	    if (IsMarker(end, lumpinfo[i].name))
	    {
		in_block = 0;
	    }
	    else if (i && lumpinfo[i].handle != lumpinfo[i-1].handle)
	    {
		in_block = 0;
		lumpinfo[oldlumps++] = lumpinfo[i];
	    }
	    else
	    {
		newlumpinfo[newlumps++] = lumpinfo[i];
	    }
	}
    }

    // now copy the merged lumps to the end of the old list
    if (newlumps)
    {
	if (oldlumps + newlumps > numlumps)
	    lumpinfo = realloc(lumpinfo, (oldlumps + newlumps) *
				 sizeof(lumpinfo_t));
	memcpy(lumpinfo + oldlumps, newlumpinfo, sizeof(lumpinfo_t) * newlumps);

	numlumps = oldlumps + newlumps;

	memset(lumpinfo[numlumps].name, 0, 8);
	strcpy(lumpinfo[numlumps].name, end);
	lumpinfo[numlumps].handle = -1;
	lumpinfo[numlumps].position = lumpinfo[numlumps].size = 0;
	numlumps++;
    }
}

//
// Hash function used for lump names.
// Taken from Boom, written by Lee Killough.
//
unsigned W_LumpNameHash(char *s)
{
    unsigned	hash;

    (void) ((hash =            toupper(s[0]), s[1]) &&
	    (hash = hash * 3 + toupper(s[1]), s[2]) &&
	    (hash = hash * 2 + toupper(s[2]), s[3]) &&
	    (hash = hash * 2 + toupper(s[3]), s[4]) &&
	    (hash = hash * 2 + toupper(s[4]), s[5]) &&
	    (hash = hash * 2 + toupper(s[5]), s[6]) &&
	    (hash = hash * 2 + toupper(s[6]),
	     hash = hash * 2 + toupper(s[7]))
	   );

    return hash;
}

//
// Initialize the lump hash chains
//
void W_InitLumpHash(void)
{
    int		i;

    for (i = 0; i < numlumps; i++)
	lumpinfo[i].index = -1;

    for (i = 0; i < numlumps; i++)
    {
	int	j = W_LumpNameHash(lumpinfo[i].name) % (unsigned)numlumps;

	lumpinfo[i].next = lumpinfo[j].index;
	lumpinfo[j].index = i;
    }
}

//
// W_InitFile
// Just initialize from a single file.
//
void W_InitFile(char *filename)
{
    char	*names[2];

    names[0] = filename;
    names[1] = NULL;
    W_InitMultipleFiles(names);
}

//
// W_NumLumps
//
int W_NumLumps(void)
{
    return numlumps;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//
// Rewritten by Lee Killough for Boom, to use hash table for performance.
//
int W_CheckNumForName(char *name)
{
    register int	i;

    i = lumpinfo[W_LumpNameHash(name) % (unsigned)numlumps].index;
    while (i >= 0 && strncasecmp(lumpinfo[i].name, name, 8))
	i = lumpinfo[i].next;

    return i;
}

//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName(char *name)
{
    int	i;

    i = W_CheckNumForName(name);

    if (i == -1)
      I_Error("W_GetNumForName: %s not found!", name);

    return i;
}

//
// W_CheckLumpName
// Checks if the lump has the name, used to find BEHAVIOR lump
// for a map.
//
int W_CheckLumpName(int lump, char *name)
{
    if (lump >= numlumps)
	return 0;

    return (!strncasecmp(lumpinfo[lump].name, name, 8));
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
    if (lump >= numlumps)
	I_Error("W_LumpLength: %i >= numlumps", lump);

    return lumpinfo[lump].size;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(int lump, void *dest)
{
    int		c;
    lumpinfo_t	*l;
    int		handle;

    if (lump >= numlumps)
	I_Error("W_ReadLump: %i >= numlumps", lump);

    l = lumpinfo + lump;

    if (l->handle == -1)
    {
	// reloadable file, so use open / read / close
	if ((handle = open(reloadname, O_RDONLY | O_BINARY)) == -1)
	    I_Error("W_ReadLump: couldn't open %s", reloadname);
    }
    else
	handle = l->handle;

    lseek(handle, l->position, SEEK_SET);
    c = read(handle, dest, l->size);

    if (c < l->size)
	I_Error("W_ReadLump: only read %i of %i on lump %i",
		 c, l->size, lump);

    if (l->handle == -1)
	close (handle);
}

//
// W_CacheLumpNum
//
void *W_CacheLumpNum(int lump, int tag)
{
    byte	*ptr;

    if ((unsigned)lump >= numlumps)
	I_Error("W_CacheLumpNum: %i >= numlumps", lump);

    if (!lumpcache[lump])
    {
	// read the lump in

	//printf("cache miss on lump %i\n", lump);
	ptr = Z_Malloc(W_LumpLength(lump), tag, &lumpcache[lump]);
	W_ReadLump(lump, lumpcache[lump]);
    }
    else
    {
	//printf("cache hit on lump %i\n",lump);
	Z_ChangeTag(lumpcache[lump], tag);
    }

    return lumpcache[lump];
}

//
// W_CacheLumpName
//
void *W_CacheLumpName(char *name, int tag)
{
    return W_CacheLumpNum(W_GetNumForName(name), tag);
}

//
// W_Profile
//
int		info[2500][10];
int		profilecount;

void W_Profile(void)
{
    int		i;
    memblock_t	*block;
    void	*ptr;
    char	ch;
    FILE	*f;
    int		j;
    char	name[9];

    for (i = 0; i < numlumps; i++)
    {
	ptr = lumpcache[i];
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;

    f = fopen("waddump.txt", "w");
    name[8] = 0;

    for (i = 0; i < numlumps; i++)
    {
	memcpy(name, lumpinfo[i].name, 8);

	for (j = 0; j < 8; j++)
	    if (!name[j])
		break;

	for (; j < 8; j++)
	    name[j] = ' ';

	fprintf(f, "%s ", name);

	for (j = 0; j < profilecount; j++)
	    fprintf(f, "    %c", info[i][j]);

	fprintf(f, "\n");
    }
    fclose(f);
}
