//  
// DOSDoom Networking Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

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

#include "dm_state.h"

#ifdef __GNUG__
#pragma implementation "i_net.h"
#endif
#include "i_net.h"

void	NetSend (void);
boolean NetListen (void);

int real_doomcom;
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
				
    movedata(_my_ds(), (int) doomcom, _dos_ds, real_doomcom, sizeof(*doomcom));
    __dpmi_int(doomcom->intnum,&r);
    movedata(_dos_ds, real_doomcom, _my_ds(), (int) doomcom, sizeof(*doomcom));
}


//
// PacketGet
//
void PacketGet (void)
{
    __dpmi_regs r;

    movedata(_my_ds(), (int) doomcom, _dos_ds, real_doomcom, sizeof(*doomcom));
    __dpmi_int(doomcom->intnum,&r);
    movedata(_dos_ds, real_doomcom, _my_ds(), (int) doomcom, sizeof(*doomcom));

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

   // doomcom=(doomcom_t *)(__djgpp_conventional_base+atoi(myargv[i+1]));
    doomcom = (doomcom_t *) malloc(sizeof(*doomcom));
    real_doomcom = atoi(myargv[i+1]);
    movedata(_dos_ds, real_doomcom, _my_ds(), (int) doomcom, sizeof(*doomcom));

    doomcom->ticdup=1;
    if (M_CheckParm ("-extratic"))
      doomcom->extratics = 1;
    else
      doomcom->extratics = 0;

    j = M_CheckParm ("-dup");
    if (j && j< myargc-1)
      {
      doomcom->ticdup = atoi(myargv[j+1]);
      if (doomcom->ticdup < 1)
        doomcom->ticdup = 1;
      if (doomcom->ticdup > 9)
        doomcom->ticdup = 9;
      }
    else
      doomcom->ticdup = 1;

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

