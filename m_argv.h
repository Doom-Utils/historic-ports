// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_argv.h,v 1.3 1998/05/01 14:26:18 killough Exp $
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
//  Nil.
//    
//-----------------------------------------------------------------------------


#ifndef __M_ARGV__
#define __M_ARGV__

//
// MISC
//
extern int  myargc;
extern char **myargv;

// Returns the position of the given parameter in the arg list (0 if not found).
int M_CheckParm(const char *check);

#endif

//----------------------------------------------------------------------------
//
// $Log: m_argv.h,v $
// Revision 1.3  1998/05/01  14:26:18  killough
// beautification
//
// Revision 1.2  1998/01/26  19:27:05  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:02:58  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
