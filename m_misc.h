// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_misc.h,v 1.4 1998/05/05 19:56:06 phares Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//
//    
//-----------------------------------------------------------------------------


#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"
//
// MISC
//



boolean M_WriteFile (char const* name,void* source,int length);

int M_ReadFile (char const* name,byte** buffer);

void M_ScreenShot (void);

void M_LoadDefaults (void);

void M_SaveDefaults (void);


int M_DrawText (int x,int y,boolean direct,char* string);

// phares 4/21/98:
// Moved from m_misc.c so m_menu.c could see it.

typedef struct
{
  char* name;
  int*  location;
  int   defaultvalue;
  int   minvalue;         // jff 3/3/98 minimum allowed value
  int   maxvalue;         // jff 3/3/98 maximum allowed value
  int   isstr;            // jff 4/10/98 whether defaultvalue is int or str
  int   setupscreen;      // phares 4/19/98: setup screen where this appears
  char* help;             // jff 3/3/98 description of parameter
  int   scantranslate;    // PC scan code hack
  int   untranslated;     // lousy hack
} default_t;

#endif

//----------------------------------------------------------------------------
//
// $Log: m_misc.h,v $
// Revision 1.4  1998/05/05  19:56:06  phares
// Formatting and Doc changes
//
// Revision 1.3  1998/04/22  13:46:17  phares
// Added Setup screen Reset to Defaults
//
// Revision 1.2  1998/01/26  19:27:12  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
