// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: lprintf.c,v 1.2 1998/09/14 18:49:49 jim Exp $
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
//  Provides a logical console output routine that allows what is
//  output to console normally and when output is redirected to
//  be controlled..
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: lprintf.c,v 1.2 1998/09/14 18:49:49 jim Exp $";
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "lprintf.h"

int cons_error_mask = -1-LO_INFO; // all but LO_INFO when redir'd
int cons_output_mask = -1;        // all output enabled

#define MAX_MESSAGE_SIZE 1024

int lprintf(OutputLevels pri, const char *s, ...)
{
  int r=0;
  static char msg[MAX_MESSAGE_SIZE];
  int lvl=pri;

  va_list v;
  va_start(v,s);
  vsprintf(msg,s,v);                      // print message in buffer
  va_end(v);

  if (lvl&cons_output_mask)               // mask output as specified
    r=fprintf(stdout,"%s",msg);
  if (!isatty(1) && lvl&cons_error_mask)  // if stdout redirected 
    r=fprintf(stderr,"%s",msg);           // select output at console

  return r;
}

//----------------------------------------------------------------------------
//
// $Log: lprintf.c,v $
// Revision 1.2  1998/09/14  18:49:49  jim
// fix log comments
//
// Revision 1.1  1998/09/07  20:10:56  jim
// Logical output routine added
//
//
//----------------------------------------------------------------------------

