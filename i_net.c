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
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>

#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include "doomstat.h"

#ifdef __GNUG__
#pragma implementation "i_net.h"
#endif
#include "i_net.h"

void	NetSend (void);
boolean NetListen (void);


//
// NETWORKING
//

void	(*netget) (void);
void	(*netsend) (void);


//
// PacketSend
//
void PacketSend (void)
{
    __dpmi_regs r;
				
    __dpmi_int(doomcom->intnum,&r);
}


//
// PacketGet
//
void PacketGet (void)
{
    __dpmi_regs r;
				
    __dpmi_int(doomcom->intnum,&r);

}

//
// I_InitNetwork
//
void I_InitNetwork (void)
{
    int			i,j;
	
    // set up for network
			    
    // parse network game options,
    //  -net <consoleplayer> <host> <host> ...
    i = M_CheckParm ("-net");
    if (!i)
      {
      // single player game
      doomcom = malloc (sizeof (*doomcom) );
      memset (doomcom, 0, sizeof(*doomcom) );

      netgame = false;
      doomcom->id = DOOMCOM_ID;
      doomcom->numplayers = doomcom->numnodes = 1;
      doomcom->deathmatch = false;
      doomcom->consoleplayer = 0;
      doomcom->extratics=0;
      doomcom->ticdup=1;
      return;
      }

    doomcom=(doomcom_t *)(__djgpp_conventional_base+atoi(myargv[i+1]));

    doomcom->ticdup=1;
    if (M_CheckParm ("-extratic"))
      doomcom-> extratics = 1;
    else
      doomcom-> extratics = 0;

    j = M_CheckParm ("-dup");
    if (j && j< myargc-1)
      {
      doomcom->ticdup = myargv[j+1][0]-'0';
      if (doomcom->ticdup < 1)
        doomcom->ticdup = 1;
      if (doomcom->ticdup > 9)
        doomcom->ticdup = 9;
      }
    else
      doomcom-> ticdup = 1;

    netsend = PacketSend;
    netget = PacketGet;
    netgame = true;    
}


void I_NetCmd (void)
{
    if (doomcom->command == CMD_SEND)
    {
	netsend ();
    }
    else if (doomcom->command == CMD_GET)
    {
	netget ();
    }
    else
	I_Error ("Bad net cmd: %i\n",doomcom->command);
}

