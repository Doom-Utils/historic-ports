/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: d_client.c,v 1.11 2000/02/26 19:23:58 cph Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by Colin Phipps
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
 *    Network client. Passes information to/from server, staying 
 *    synchronised.
 *    Contains the main wait loop, waiting for network input or 
 *    time before doing the next tic.
 *    Rewritten for LxDoom, but based around bits of the old code.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
#include "z_zone.h"

#include "d_main.h"
#include "g_game.h"
#include "m_menu.h"

#include "protocol.h"
#include "i_network.h"
#include "i_system.h"
#include "i_main.h"
#include "i_video.h"

#include "lprintf.h"
#include <unistd.h>
#include <sys/types.h>
#include "SDL.h"

static boolean   server;
static int       remotetic; // Tic expected from the remote
#ifdef HAVE_NET
static int       remotesend; // Tic expected by the remote
#endif
ticcmd_t         netcmds[MAXPLAYERS][BACKUPTICS];
static ticcmd_t* localcmds;
#ifdef HAVE_NET
static unsigned          numqueuedpackets;
static packet_header_t** queuedpacket;
#endif
doomcom_t*      doomcom;        
int maketic;
int ticdup = 1;
#ifdef HAVE_NET
static int xtratics = 0;

void D_InitNetGame (void)
{
  int i;

  doomcom = Z_Malloc(sizeof *doomcom, PU_STATIC, NULL);
  if (!(server = netgame = I_InitNetwork())) {
    doomcom->consoleplayer = 0;
    doomcom->numnodes = 0; doomcom->numplayers = 1;
    localcmds = netcmds[consoleplayer];
  } else {
    // Get game info from server
    packet_header_t *packet = Z_Malloc(1000, PU_STATIC, NULL);
    struct setup_packet_s *sinfo = (void*)(packet+1);

    doomcom->numnodes = 1;
    do {
      while (!I_GetPacket(packet, 1000)) 
	SDL_Delay(10);
    } while (packet->type != PKT_SETUP);

    // Get info from the setup packet
    doomcom->consoleplayer = sinfo->yourplayer;
    doomcom->numplayers = sinfo->players;
    compatibility_level = sinfo->complevel;
    startskill = sinfo->skill;
    deathmatch = sinfo->deathmatch;
    startmap = sinfo->level;
    startepisode = sinfo->episode;
    ticdup = sinfo->ticdup;
    xtratics = sinfo->extratic;
    G_ReadOptions(sinfo->game_options);
    Z_Free(packet);
    localcmds = netcmds[consoleplayer];

    lprintf(LO_INFO, "\tjoined game as player %d/%d; %d WADs specified\n", 
	    doomcom->consoleplayer+1, doomcom->numplayers, sinfo->numwads);
    {
      int i = sinfo->numwads;
      char *p = sinfo->wadnames;

      while (i--) {
	D_AddFile(p, source_net);
	p += strlen(p) + 1;
      }
    }
  }

  for (i=0; i<doomcom->numplayers; i++)
    playeringame[i] = true;
  for (; i<MAXPLAYERS; i++)
    playeringame[i] = false;

  consoleplayer = displayplayer = doomcom->consoleplayer;
}
#else
void D_InitNetGame (void)
{
  int i;

  doomcom = Z_Malloc(sizeof *doomcom, PU_STATIC, NULL);
  doomcom->consoleplayer = 0;
  doomcom->numnodes = 0; doomcom->numplayers = 1;
  localcmds = netcmds[consoleplayer]; 

  for (i=0; i<doomcom->numplayers; i++)
    playeringame[i] = true;
  for (; i<MAXPLAYERS; i++)
    playeringame[i] = false;
        
  consoleplayer = displayplayer = doomcom->consoleplayer;
}
#endif

#ifdef HAVE_NET
void D_CheckNetGame(void)
{
  packet_header_t *packet = Z_Malloc(sizeof(packet_header_t)+1, PU_STATIC, NULL);

  if (server) {
    lprintf(LO_INFO, "D_CheckNetGame: waiting for server to signal game start\n");
    do {
      while (!I_GetPacket(packet, sizeof *packet)) {
	packet->tic = 0; packet->type = PKT_GO; 
	*(byte*)(packet+1) = consoleplayer;
	I_SendPacket(packet, 1 + sizeof *packet);
	SDL_Delay(100);
      }
    } while (packet->type != PKT_GO);
  }
  Z_Free(packet);
}

boolean D_NetGetWad(const char* name)
{
#if defined(HAVE_WAIT_H)
  size_t psize = sizeof(packet_header_t) + strlen(name) + 500;
  packet_header_t *packet;
  boolean done = false;

  if (!server || strchr(name, '/')) return false; // If it contains path info, reject

  do {
    // Send WAD request to remote
    packet = Z_Malloc(psize, PU_STATIC, NULL);
    packet->type = PKT_WAD; packet->tic = 0;
    *(byte*)(packet+1) = consoleplayer;
    strcpy(1+(byte*)(packet+1), name);
    I_SendPacket(packet, sizeof(packet_header_t) + strlen(name) + 2);
    
    SDL_Delay(10);
  } while (!I_GetPacket(packet, psize) || (packet->type != PKT_WAD));
  Z_Free(packet);

  if (!strcasecmp((void*)(packet+1), name)) {
    pid_t pid;
    int   rv;
    byte *p = (byte*)(packet+1) + strlen(name) + 1;

    /* Automatic wad file retrieval using wget (supports http and ftp, using URLs)
     * Unix systems have all these commands handy, this kind of thing is easy
     * Any windo$e port will have some awkward work replacing these.
     */
    /* cph - caution here. This is data from an untrusted source. 
     * Don't pass it via a shell. */
    if ((pid = fork()) == -1)
      perror("fork");
    else if (!pid) {
      /* Child chains to wget, does the download */
      execlp("wget", "wget", p, NULL);
    }
    /* This is the parent, i.e. main LxDoom process */
    wait(&rv);
    if (!(done = !access(name, R_OK))) {
      if (!strcmp(p+strlen(p)-4, ".zip")) {
	p = strrchr(p, '/')+1; 
	if ((pid = fork()) == -1)
	  perror("fork");
	else if (!pid) {
	  /* Child executes decompressor */
	  execlp("unzip", "unzip", p, name, NULL);
	}
	/* Parent waits for the file */
	wait(&rv);
	done = !!access(name, R_OK);
      }
      /* Add more decompression protocols here as desired */
    }
    Z_Free(buffer);
  }
  return done;
#else /* HAVE_WAIT_H */
  return false;
#endif
}

void NetUpdate(void)
{
  if (server) { // Receive network packets
    size_t recvlen;
    packet_header_t *packet = Z_Malloc(10000, PU_STATIC, NULL);
    while ((recvlen = I_GetPacket(packet, 10000))) {
      switch(packet->type) {
      case PKT_TICS:
	{
	  byte *p = (void*)(packet+1);
	  int tics = *p++;
	  if (packet->tic > remotetic) { // Missed some
	    packet->type = PKT_RETRANS;
	    packet->tic = remotetic;
	    *(byte*)(packet+1) = consoleplayer;
	    I_SendPacket(packet, sizeof(*packet)+1);
	  } else {
	    if (packet->tic + tics <= remotetic) break; // Will not improve things
	    remotetic = packet->tic;
	    while (tics--) {
	      int players = *p++;
	      while (players--) {
		int n = *p++;
		GetTicSwap(&netcmds[n][remotetic%BACKUPTICS], (void*)p);
		p += sizeof(ticcmd_t);
	      }
	      remotetic++;
	    }
	  }
	}
	break;
      case PKT_RETRANS: // Resend request
	remotesend = packet->tic;
	break;
      case PKT_DOWN: // Server downed
	{
	  int j;
	  for (j=0; j<MAXPLAYERS; j++)
	    if (j != consoleplayer) playeringame[j] = false;
	  server = false;
	  doom_printf("Server is down\nAll other players are no longer in the game\n");
	}
	break;
      case PKT_EXTRA: // Misc stuff
      case PKT_QUIT: // Player quit
	// Queue packet to be processed when its tic time is reached
	queuedpacket = Z_Realloc(queuedpacket, ++numqueuedpackets * sizeof *queuedpacket, 
				 PU_STATIC, NULL);
	queuedpacket[numqueuedpackets-1] = Z_Malloc(recvlen, PU_STATIC, NULL);
	memcpy(queuedpacket[numqueuedpackets-1], packet, recvlen);
	break;
      default: // Other packet, unrecognised or redundant
	break;
      }
    }
    Z_Free(packet);
  }
  D_BuildNewTiccmds();
  { // Send the tics to the server
    if (server && maketic > remotesend) {
      int sendtics;
      remotesend -= xtratics;
      sendtics = maketic - remotesend;
      {
	size_t pkt_size = sizeof(packet_header_t) + 2 + sendtics * sizeof(ticcmd_t);
	packet_header_t *packet = Z_Malloc(pkt_size, PU_STATIC, NULL);
	
	packet->tic = maketic - sendtics;
	packet->type = PKT_TICC;
	*(byte*)(packet+1) = sendtics;
	*(((byte*)(packet+1))+1) = consoleplayer;
	{
	  ticcmd_t *tic = (void*)(((char*)(packet+1)) +2);
	  while (sendtics--) GetTicSwap(tic++, &localcmds[remotesend++%BACKUPTICS]);
	}
	I_SendPacket(packet, pkt_size);
	Z_Free(packet);
      }
    }
  }
}
#endif

void D_BuildNewTiccmds()
{
    static int lastmadetic;
    int newtics = I_GetTime() - lastmadetic;
    lastmadetic += newtics;
    while (newtics--)
    {
      I_StartTic();
      D_ProcessEvents();
      if (maketic - gametic > BACKUPTICS/2) break;
      G_BuildTiccmd(&localcmds[maketic%BACKUPTICS]);
      maketic++;
    }
}

#ifdef HAVE_NET
void D_NetSendMisc(netmisctype_t type, size_t len, void* data) 
{
  if (server) {
    size_t size = sizeof(packet_header_t) + 3*sizeof(int) + len;
    packet_header_t *packet = Z_Malloc(size, PU_STATIC, NULL);
    int *p = (void*)(packet+1);
    
    packet->tic = gametic;
    packet->type = PKT_EXTRA;
    *p++ = type; *p++ = consoleplayer; *p++ = len;
    memcpy(p, data, len);
    I_SendPacket(packet, size);
    
    Z_Free(packet);
  }
}

static void CheckQueuedPackets(void)
{
  int i;
  for (i=0; i<numqueuedpackets; i++)
    if (queuedpacket[i]->tic <= gametic)
      switch (queuedpacket[i]->type) {
      case PKT_QUIT: // Player quit the game
	{
	  int pn = *(byte*)(queuedpacket[i]+1);
	  playeringame[pn] = false;
	  doom_printf("Player %d left the game\n", pn);
	}
	break;
      case PKT_EXTRA:
	{
	  int *p = (int*)(queuedpacket[i]+1);
	  size_t len = LONG(*(p+2));
	  switch (*p) {
	  case nm_plcolour:
	    G_ChangedPlayerColour(LONG(*(p+1)), LONG(*(p+3)));
	    break;
	  case nm_savegamename:
	    if (len < SAVEDESCLEN) {
	      memcpy(savedescription, p+3, len);
	      // Force terminating 0 in case
	      savedescription[len] = 0;
	    }
	    break;
	  }
	}
	break;
      default: // Should not be queued
	break;
      }

  { // Requeue remaining packets
    int newnum = 0;
    packet_header_t **newqueue = NULL;

    for (i=0; i<numqueuedpackets; i++)
      if (queuedpacket[i]->tic > gametic) {
	newqueue = Z_Realloc(newqueue, ++newnum * sizeof *newqueue, 
			     PU_STATIC, NULL);
	newqueue[newnum-1] = queuedpacket[i];
      } else Z_Free(queuedpacket[i]);

    Z_Free(queuedpacket);
    numqueuedpackets = newnum; queuedpacket = newqueue;
  }
}
#endif

void TryRunTics (void)
{
  int runtics;
  int entertime = I_GetTime();

  // Wait for tics to run
  while (1) {
    if (I_GetTime() - entertime > 5) {
      M_Ticker(); return;
    }
#ifdef HAVE_NET
    NetUpdate();
#else
    D_BuildNewTiccmds();
#endif
    runtics = (server ? remotetic : maketic) - gametic;
    if (!runtics) {
      SDL_Delay(1);
      if (I_GetTime() - entertime > 10) {
	M_Ticker(); return;
      }
    } else break;
  }

  while (runtics--) {
#ifdef HAVE_NET
    if (server) CheckQueuedPackets();
#endif
    if (advancedemo)
      D_DoAdvanceDemo ();
    M_Ticker ();
    G_Ticker ();
    gametic++;
#ifdef HAVE_NET
    NetUpdate(); // Keep sending our tics to avoid stalling remote nodes
#else
   D_BuildNewTiccmds();
#endif
  }
}

#ifdef HAVE_NET
void D_QuitNetGame (void)
{
  byte buf[1 + sizeof(packet_header_t)];
  packet_header_t *packet = (void*)buf;
  int i;

  if (!server) return;
  buf[sizeof(packet_header_t)] = consoleplayer;
  packet->type = PKT_QUIT; packet->tic = gametic;

  for (i=0; i<4; i++) {
    I_SendPacket(packet, 1 + sizeof(packet_header_t));
    SDL_Delay(10);
  }
}
#endif
//
// $Log: d_client.c,v $
// Revision 1.11  2000/02/26 19:23:58  cph
// Don't trust server path data, avoid system(3) calls with it
//
// Revision 1.10  1999/11/01 17:12:28  cphipps
// Added i_main.h
//
// Revision 1.9  1999/10/31 16:36:06  cphipps
// Update include files for i_* changes
//
// Revision 1.8  1999/10/12 13:01:09  cphipps
// Changed header to GPL
//
// Revision 1.7  1999/10/03 06:40:33  cphipps
// Improved D_NetGetWad
// - retransmits the packet to the server until it gets a reply
// - used wget(1) to do the download, which supports http in addition to ftp
//
// Revision 1.6  1999/08/21 09:17:44  cphipps
// Reduced time delay in TryRunTics
//
// Revision 1.5  1999/04/02 11:21:34  cphipps
// Send PKT_GO to server saying when we are ready
//
// Revision 1.4  1999/04/02 10:54:45  cphipps
// Split netgame startup between 2 functions:
// D_InitNetGame gets the startup packet and wad list
// D_CheckNetGame waits for the game to start
//
// Revision 1.3  1999/04/01 22:19:43  cphipps
// Working PKT_WAD implementation, adds wad files to the game and downloads them as
// specified by the server, as needed
//
// Revision 1.2  1999/04/01 10:12:57  cphipps
// Fix a couple of memory leaks (d'oh)
// Fix PKT_DOWN handling
// Call NetUpdate() more regularly
// Rearrange packet-waiting loop
//
// Revision 1.1  1999/03/29 11:54:47  cphipps
// Initial revision
//
//
