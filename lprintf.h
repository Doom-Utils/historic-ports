// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: lprintf.h,v 1.2 1998/09/09 21:21:43 phares Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1998 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
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
//    Declarations etc. for logical console output
//
//-----------------------------------------------------------------------------

#ifndef __LPRINTF__
#define __LPRINTF__

typedef enum                // Logical output levels
{
  LO_INFO=1,                // One of these is used in each physical output
  LO_CONFIRM=2,             // call. Which are output, or echoed to console
  LO_WARN=4,                // if output redirected is determined by the
  LO_ERROR=8,               // global masks: cons_output_mask,cons_error_mask.
  LO_FATAL=16,
  LO_DEBUG=32,
  LO_ALWAYS=64,
} OutputLevels;

#ifndef __GNUC__
#define __attribute(x)__
#endif

extern int lprintf(OutputLevels pri, const char *fmt, ...) __attribute__((format(printf,2,3)));
extern int cons_output_mask;
extern int cons_error_mask;

#endif

//----------------------------------------------------------------------------
//
// $Log: lprintf.h,v $
// Revision 1.2  1998/09/09  21:21:43  phares
// Added Log line
//
//
//----------------------------------------------------------------------------
