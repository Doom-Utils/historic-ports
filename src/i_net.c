// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_net.c,v 1.4 1998/05/16 09:41:03 jim Exp $
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
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#ifndef _WIN32 // proff: Non windows networking

#include "z_zone.h"  /* memory allocation wrappers -- killough */

static const char
rcsid[] = "$Id: i_net.c,v 1.4 1998/05/16 09:41:03 jim Exp $";

#include "doomstat.h"
#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include <dpmi.h>
#include <sys/nearptr.h>

#ifdef __GNUG__
#pragma implementation "i_net.h"
#endif
#include "i_net.h"

void    NetSend (void);
boolean NetListen (void);

//
// NETWORKING
//

void    (*netget) (void);
void    (*netsend) (void);

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
  int                 i,j;
      
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

#else //_WIN32

static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <errno.h>

#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include "doomstat.h"

#ifdef __GNUG__
#pragma implementation "i_net.h"
#endif
#include "i_net.h"
#include "lprintf.h"





// For some odd reason...
#define ntohl(x) \
        ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                             (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                             (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                             (((unsigned long int)(x) & 0xff000000U) >> 24)))

#define ntohs(x) \
        ((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
                              (((unsigned short int)(x) & 0xff00) >> 8))) \
      
#define htonl(x) ntohl(x)
#define htons(x) ntohs(x)

void    NetSend (void);
boolean NetListen (void);


//char MsgText[2048];

//
// NETWORKING
//

// FIXME
//#define IPPORT_USERRESERVED 26000
#define IPPORT_USERRESERVED 5000

int    DOOMPORT =    (IPPORT_USERRESERVED +0x1d );

int            sendsocket;
int            insocket;

struct    sockaddr_in    sendaddress[MAXNETNODES];

void    (*netget) (void);
void    (*netsend) (void);

//
// UDPsocket
//
int UDPsocket (void)
   {
    int    s;
    
    // allocate a socket
    s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    if (s < 0)
    if (s == INVALID_SOCKET)
       {
        I_Error("Can't create socket:%08X %s\n", errno, strerror(errno));
       }
        
    return s;
   }

//
// BindToLocalPort
//
void BindToLocalPort( int s, int port )
   {
    int            v;
    struct sockaddr_in    address;
    
    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = port;

    v = bind(s, (void *)&address, sizeof(address));
    if (v == -1)
        I_Error ("BindToPort: bind: %s", strerror(errno));
   }


//
// PacketSend
//
void PacketSend (void)
{
    int        c;
    netbuffer->checksum = LONG(netbuffer->checksum);
                
    //printf ("sending %i\n",gametic);          
    c = sendto (sendsocket , (const char*)netbuffer, doomcom->datalength
                ,0,(void *)&sendaddress[doomcom->remotenode]
                ,sizeof(sendaddress[doomcom->remotenode]));

    netbuffer->checksum = LONG(netbuffer->checksum);
}


//
// PacketGet
//
void PacketGet (void)
{
    int            i;
    int            c;
    struct sockaddr_in    fromaddress;
    int            fromlen;
                
    fromlen = sizeof(fromaddress);
    c = recvfrom (insocket, (char *)netbuffer, sizeof(doomdata_t),0,(struct sockaddr *)&fromaddress, &fromlen );
    if (c == SOCKET_ERROR )
       {
        c = WSAGetLastError();
        if (c != WSAEWOULDBLOCK)
            I_Error ("GetPacket: Network error.");
        doomcom->remotenode = -1;        // no packet
        return;
       }

    {
    static int first=1;
//    if (first)
//        printf("len=%d:p=[0x%x 0x%x] \n", c, *(int*)&sw, *((int*)&sw+1));
    first = 0;
    }

    // find remote node number
    for (i=0 ; i<doomcom->numnodes ; i++)
    if ( fromaddress.sin_addr.s_addr == sendaddress[i].sin_addr.s_addr )
        break;

    if (i == doomcom->numnodes)
    {
    // packet is not from one of the players (new game broadcast)
    doomcom->remotenode = -1;        // no packet
    return;
    }
    
    doomcom->remotenode = i;            // good packet from a game player
    doomcom->datalength = c;
    
    // byte swap
    // byte swap
    netbuffer->checksum = LONG(netbuffer->checksum);
}



int GetLocalAddress (void)
{
    char        hostname[1024];
    struct hostent*    hostentry;    // host information entry
    int            v;

    // get local address
    v = gethostname (hostname, sizeof(hostname));
    if (v == -1)
    I_Error ("GetLocalAddress : gethostname: errno %d",errno);
    
    hostentry = gethostbyname (hostname);
    if (!hostentry)
    I_Error ("GetLocalAddress : gethostbyname: couldn't get local host");
        
    return *(int *)hostentry->h_addr_list[0];
}


//
// I_InitNetwork
//
void I_InitNetwork (void)
   {
    boolean        trueval = true;
    int            i, err;
    int            p;
    struct hostent*    hostentry;    // host information entry
    WSADATA wsaData; 

    doomcom = malloc (sizeof (*doomcom) );
    memset (doomcom, 0, sizeof(*doomcom) );
    
    // set up for network
    i = M_CheckParm ("-dup");
    if (i && i< myargc-1)
       {
        doomcom->ticdup = myargv[i+1][0]-'0';
        if (doomcom->ticdup < 1)
            doomcom->ticdup = 1;
        if (doomcom->ticdup > 9)
            doomcom->ticdup = 9;
       }
    else
       doomcom-> ticdup = 1;
    
    if (M_CheckParm ("-extratic"))
       doomcom-> extratics = 1;
    else
       doomcom-> extratics = 0;
        
    p = M_CheckParm ("-port");
    if (p && p<myargc-1)
       {
        DOOMPORT = atoi (myargv[p+1]);
        lprintf (LO_INFO, "using alternate port %i\n",DOOMPORT);
       }
    
    // parse network game options,
    //  -net <consoleplayer> <host> <host> ...
    i = M_CheckParm ("-net");
    if (!i)
       {
        // single player game
        netgame = false;
        doomcom->id = DOOMCOM_ID;
        doomcom->numplayers = doomcom->numnodes = 1;
        doomcom->deathmatch = false;
        doomcom->consoleplayer = 0;
        return;
       }

    err = WSAStartup(0x0101, &wsaData); 
 
    netsend = PacketSend;
    netget = PacketGet;
    netgame = true;

    // parse player number and host list
    doomcom->consoleplayer = myargv[i+1][0]-'1';
    lprintf (LO_INFO, "console player number : %d\n", doomcom->consoleplayer);

    doomcom->numnodes = 1;    // this node for sure

    i++;
    i++;
    while (i < myargc && myargv[i][0] != '-')
       {
        sendaddress[doomcom->numnodes].sin_family = AF_INET;
        if (myargv[i][0] == '.')
           {
            lprintf (LO_INFO, "Node number %d address %s\n", doomcom->numnodes, myargv[i]+1);
            sendaddress[doomcom->numnodes].sin_addr.s_addr = inet_addr(myargv[i]+1);
           }
        else
           {
            lprintf (LO_INFO, "Node number %d hostname %s\n", doomcom->numnodes, myargv[i]);
            hostentry = gethostbyname(myargv[i]);
            if (!hostentry)
                I_Error ("gethostbyname: couldn't find %s", myargv[i]);
            sendaddress[doomcom->numnodes].sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];
           }
        i++;
	      if ((i < myargc) && (myargv[i][0] == ':'))
        {
            lprintf (LO_INFO, "Node number %d port %s\n", doomcom->numnodes, myargv[i]+1);
            sendaddress[doomcom->numnodes].sin_port = htons(atoi(myargv[i]+1));
            i++;
        }
        else
            sendaddress[doomcom->numnodes].sin_port = htons(DOOMPORT);
        doomcom->numnodes++;
       }
    
    lprintf (LO_INFO, "Total number of players : %d\n", (doomcom->numnodes));
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes;
    
// build message to receive
    insocket = UDPsocket();
    BindToLocalPort(insocket,htons(DOOMPORT));
    ioctlsocket(insocket, FIONBIO, (u_long *)&trueval);

    sendsocket = UDPsocket();
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

#endif //_WIN32

//----------------------------------------------------------------------------
//
// $Log: i_net.c,v $
// Revision 1.4  1998/05/16  09:41:03  jim
// formatted net files, installed temp switch for testing Stan/Lee's version
//
// Revision 1.3  1998/05/03  23:27:19  killough
// Fix #includes at the top, nothing else
//
// Revision 1.2  1998/01/26  19:23:26  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:07  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------
