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
//	WAD I/O functions.
//
//-----------------------------------------------------------------------------

#ifndef __W_WAD__
#define __W_WAD__

#ifdef __GNUG__
#pragma interface
#endif

//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];
    int			numlumps;
    int			infotableofs;
} wadinfo_t;

typedef struct
{
    int			filepos;
    int			size;
    char		name[8];
} filelump_t;

//
// WADFILE I/O related stuff.
//
typedef struct
{
    char	name[8];
    int		handle;
    int		position;
    int		size;
    int		index;
    int		next;
} lumpinfo_t;

extern	void		**lumpcache;
extern	lumpinfo_t	*lumpinfo;
extern	int		numlumps;

void    W_InitMultipleFiles(char **filenames);
void    W_Reload(void);
void	W_MergeLumps(char *start, char *end);
void	W_InitLumpHash(void);

int	W_CheckNumForName(char *name);
int	W_GetNumForName(char *name);
int	W_CheckLumpName(int lump, char *name);

int	W_LumpLength(int lump);
void    W_ReadLump(int lump, void *dest);

void	*W_CacheLumpNum(int lump, int tag);
void	*W_CacheLumpName(char *name, int tag);

#endif
