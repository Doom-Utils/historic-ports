// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
// $Id:$
//
// Copyright (C) 1997 by Udo Munk
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

#include <ctype.h>
#include "strcmp.h"

int strcasecmp(const char *a, const char *b)
{
    const char *p;
    const char *q;

    for (p = a, q = b; *p && *q; p++, q++)
    {
      int diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    if (*p) return 1;		// p was longer than q
    if (*q) return -1;		// p was shorter than q
    return 0;			// exact match
}

int strncasecmp(const char *a, const char *b, int n)
{
    const char *p;
    const char *q;

    for (p = a, q = b; /*NOTHING*/; p++, q++)
    {
      int diff;
      if (p == a + n) return 0;		// match up to n characters
      if (!(*p && *q)) return *p - *q;
      diff = tolower(*p) - tolower(*q);
      if (diff) return diff;
    }
    /*NOTREACHED*/
}
