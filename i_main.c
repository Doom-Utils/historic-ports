// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
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
// $Log:$
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_main.c,v 1.4 1997/02/03 22:45:10 b1 Exp $";


#include "m_argv.h"
#include "d_main.h"
#include "dm_defs.h"

#include <stdio.h>
#include <signal.h>

void break_handler(int a)
{
 exit(a);
}

int VERSION       = 200;
int VERSIONFIX    = 100;
int DOSDOOMVER    = 0x651;
int DOSDOOMVERFIX = 0x1000;

int main ( int argc, char** argv )
{ 
    // Init Arguments
    myargc = argc; 
    myargv = argv; 

    signal(SIGINT, break_handler);

    // Run DOOM! -- Never returns...
    D_DoomMain ();

    return 0;
} 
