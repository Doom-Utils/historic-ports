// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1998 by Udo Munk
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
//	See if file is in any directory in the PATH environment variable.
//	If yes return a complete pathname, if not found just return the
//	filename.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

char *searchpath(char *file)
{
	char		*path;
	char		*dir;
	static char	b[2048];
	struct stat	s;
	char		pb[2048];

	// get PATH, if not set just return filename, might be in cwd
	if ((path = getenv("PATH")) == NULL)
		return(file);

	// we have to do this because strtok() is destructive
	strcpy(pb, path);

	// get first directory
	dir = strtok(pb, ":");

	// loop over the directories in PATH and see if the file is there
	while (dir)
	{
		// build filename from directory/filename
		strcpy(b, dir);
		strcat(b, "/");
		strcat(b, file);
		if (stat(b, &s) == 0) {
			return b ;	// yep, there it is
		}

		// get next directory
		dir = strtok(NULL, ":");
	}

	// hm, not found, just return filename, again, might be in cwd
	return file ;
}
