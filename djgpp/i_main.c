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
#include "doomdef.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void break_handler(int a)
{
 exit(a);
}

int
main
( int		argc,
  char**	argv ) 
{ 
    // Init Arguments
    myargc = argc; 
    myargv = argv; 

    signal(SIGINT, break_handler);
    printf("DOSDoom v%d.%d compiled on "__DATE__" at "__TIME__"\n", DOSDOOMVER / DOSDOOMVERFIX, DOSDOOMVER % DOSDOOMVERFIX);
    printf("DOSDoom homepage is at http://www.frag.com/dosdoom/\n");
    printf("DOOM is by id Software http://www.idsoftware.com/\n");
    printf("This version is neither distributed nor supported by id Software,\n so don't bug them about it!\n");
    sleep(2);

    // Run DOOM! -- Never returns...
    D_DoomMain ();

    return 0;
} 
