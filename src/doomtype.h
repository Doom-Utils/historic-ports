// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: doomtype.h,v 1.3 1998/05/03 23:24:33 killough Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//      Simple basic typedefs, isolated here to make it easier
//       separating modules.
//
//-----------------------------------------------------------------------------


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
// Fixed to use builtin bool type with C++.
#ifdef __cplusplus
typedef bool boolean;
#else
typedef enum {false, true} boolean;
#endif
typedef unsigned char byte;
#endif

#ifndef _WIN32 // proff: Visual C has no values.h so I have to add the values myself
#include <values.h>
#define MAXCHAR         ((char)0x7f)
#define MINCHAR         ((char)0x80)
#else // _WIN32

// proff 07/04/98: Added for CYGWIN32 compatibility
#ifdef _MSC_VER
#undef PATH_MAX
#define PATH_MAX 1024
#endif

#undef MAXCHAR
#define MAXCHAR         ((char)0x7f)
#undef MAXSHORT
#define MAXSHORT        ((short)0x7fff)
#undef MAXINT
#define MAXINT          ((int)0x7fffffff)       
#undef MAXLONG
#define MAXLONG         ((long)0x7fffffff)

#undef MINCHAR
#define MINCHAR         ((char)0x80)
#undef MINSHORT
#define MINSHORT        ((short)0x8000)
#undef MININT
#define MININT          ((int)0x80000000)       
#undef MINLONG
#define MINLONG         ((long)0x80000000)

#endif // _WIN32

#endif // __DOOMTYPE__

//----------------------------------------------------------------------------
//
// $Log: doomtype.h,v $
// Revision 1.3  1998/05/03  23:24:33  killough
// beautification
//
// Revision 1.2  1998/01/26  19:26:43  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:51  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
