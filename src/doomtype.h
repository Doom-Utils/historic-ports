/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: doomtype.h,v 1.4 1999/10/12 13:00:56 cphipps Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
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
 *      Simple basic typedefs, isolated here to make it easier
 *       separating modules.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
/* Fixed to use builtin bool type with C++. */
#ifdef __cplusplus
typedef bool boolean;
#else
typedef enum {false, true} boolean;
#endif
typedef unsigned char byte;
#endif

typedef signed long long int_64_t; 
typedef unsigned long long uint_64_t; 

/* CPhipps - use limits.h instead of depreciated values.h */
#include <limits.h>
#endif

/*----------------------------------------------------------------------------
 *
 * $Log: doomtype.h,v $
 * Revision 1.4  1999/10/12 13:00:56  cphipps
 * Changed header to GPL, converted C++ comments to C
 *
 * Revision 1.3  1999/06/08 17:29:15  cphipps
 * Add typedefs for int_64_t types, to abstract 64 bit ints from the particular
 *  compiler type
 *
 * Revision 1.2  1999/01/25 22:44:51  cphipps
 * Use limits.h instead of depreciated values.h to get integer limit macros
 *
 * Revision 1.1  1998/09/13 16:49:50  cphipps
 * Initial revision
 *
 * Revision 1.3  1998/05/03  23:24:33  killough
 * beautification
 *
 * Revision 1.2  1998/01/26  19:26:43  phares
 * First rev with no ^Ms
 *
 * Revision 1.1.1.1  1998/01/19  14:02:51  rand
 * Lee's Jan 19 sources
 *
 *----------------------------------------------------------------------------*/
