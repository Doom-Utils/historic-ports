/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: d_server.c,v 1.11 2000/02/26 19:21:00 cph Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
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
 *  Network game server code
 *  New for LxDoom, but drawing ideas and code fragments from the 
 *  earlier net code
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVER

#include "doomtype.h"
#include "protocol.h"
#include "i_network.h"
#include "i_system.h"

#define MAXPLAYERS 4
#define BACKUPTICS 12

// Dummies to forfill l_udp.c unused client stuff
int M_CheckParm(const char* p) { p = NULL; return 1; }
int myargc;
char** myargv;

void I_Error(const char *error, ...) // killough 3/20/98: add const
{
  char errmsg[1000];
  va_list argptr;
  va_start(argptr,error);
  vsnprintf(errmsg,sizeof(errmsg),error,argptr);
  va_end(argptr);
  exit(-1);
}

int playerjoingame[MAXPLAYERS], playerleftgame[MAXPLAYERS];
#define playeringame(i) ((playerjoingame[i] < INT_MAX) && (playerleftgame[i] == INT_MAX))
struct sockaddr_in remoteaddr[MAXPLAYERS];

void BroadcastPacket(packet_header_t *packet, size_t len)
{
  int i;
  for (i=0; i<MAXPLAYERS; i++)
    if (playeringame(i))
      I_SendPacketTo(packet, len, &remoteaddr[i]);
}

byte def_game_options[GAME_OPTIONS_SIZE] = \
{ // cf g_game.c:G_WriteOptions()
  1, // monsters remember
  1, // friction
  1, // weapon recoil
  1, // pushers
  0, // reserved/unused
  1, // player bobbing
  0, 0, 0, // respawn, fast, nomonsters
  1, // demo insurance
  // 4 bytes of random number seed
}; // Zeroes to the end

int verbose;

void sig_handler(int signum)
{
  char buf[80];
  I_SigString(buf,80,signum);
  printf("Received signal: %s\n", buf);

  // Any signal is fatal
  exit(1);
}

void doexit(void)
{
  packet_header_t packet;

  // Send "downed" packet
  packet.type = PKT_DOWN; 
  packet.tic = 0; // Clients should not need the tic number, can't see a use for it
  BroadcastPacket(&packet, sizeof packet);
}

int main(int argc, char** argv)
{
  int localport = 5030, numplayers = 2, xtratics = 0, ticdup = 1;
  int exectics = 0; // gametics completed
  struct setup_packet_s setupinfo = { 2, 0, 1, 1, 1, 0, 3};
  char**wadname = NULL;
  char**wadget = NULL;
  int numwads = 0;
  {
    int opt;
    byte *gameopt = setupinfo.game_options;

    memcpy(gameopt, &def_game_options, sizeof (setupinfo.game_options));
    while ((opt = getopt(argc, argv, "p:e:l:adrfns:c:N:x:t:vw:")) != EOF)
      switch (opt) {
      case 't':
	if (optarg) ticdup = atoi(optarg);
	break;
      case 'x':
	if (optarg) xtratics = atoi(optarg);
	break;
      case 'p':
	if (optarg) localport = atoi(optarg);
	break;
      case 'e':
	if (optarg) setupinfo.episode = atoi(optarg);
	break;
      case 'l':
	if (optarg) setupinfo.level = atoi(optarg);
	break;
      case 'a':
	setupinfo.deathmatch = 2;
	break;
      case 'd':
	setupinfo.deathmatch = 1;
	break;
      case 'r':
	setupinfo.game_options[6] = 1;
	break;
      case 'f':
	setupinfo.game_options[7] = 1;
	break;
      case 'n':
	setupinfo.game_options[8] = 1;
	break;
      case 's':
	if (optarg) setupinfo.skill = atoi(optarg);
	break;
      case 'N':
	if (optarg) setupinfo.players = numplayers = atoi(optarg);
	break;
      case 'v':
	verbose++;
	break;
      case 'w':
	if (optarg) {
	  char *p;
	  wadname = realloc(wadname, ++numwads * sizeof *wadname);
	  wadget  = realloc(wadget ,   numwads * sizeof *wadget );
	  wadname[numwads-1] = strdup(optarg);
	  if ((p = strchr(wadname[numwads-1], ','))) {
	    *p++ = 0; wadget[numwads-1] = p;
	  } else wadget[numwads-1] = NULL;
	}
	break;
      }
  }

  setupinfo.ticdup = ticdup; setupinfo.extratic = xtratics;
  { /* Random number seed 
     * Mirrors the corresponding code in G_ReadOptions */
    int rngseed = time(NULL);
    setupinfo.game_options[13] = rngseed & 0xff;
    rngseed >>= 8;
    setupinfo.game_options[12] = rngseed & 0xff;
    rngseed >>= 8;
    setupinfo.game_options[11] = rngseed & 0xff;
    rngseed >>= 8;
    setupinfo.game_options[10] = rngseed & 0xff;
  }
              
  I_InitSockets(localport);

  printf("Listening on port %d, waiting for %d players\n", localport, numplayers);

  { // no players initially
    int i;
    for (i=0; i<MAXPLAYERS; i++)
      playerjoingame[i] = INT_MAX; playerleftgame[i] = 0;

    // Print wads
    for (i=0; i<numwads; i++)
      printf("Wad %s (%s)\n", wadname[i], wadget[i]);
  }

  // Exit and signal handling
  atexit(doexit); // heh
  signal(SIGTERM, sig_handler);
  signal(SIGINT , sig_handler);
  signal(SIGQUIT, sig_handler);
  signal(SIGKILL, sig_handler);
  signal(SIGHUP , sig_handler);
  
  {
    int remoteticfrom[MAXPLAYERS] = { 0, 0, 0, 0 };
    int remoteticto[MAXPLAYERS] = { 0, 0, 0, 0 };
    int curplayers = 0;
    boolean ingame = false;
    ticcmd_t netcmds[MAXPLAYERS][BACKUPTICS];

    while (1) {
      {
	packet_header_t *packet = malloc(10000);
	size_t len;
	
	usleep(10000);
	while ((len = I_GetPacket(packet, 10000))) {
	  if (verbose>2) printf("Received packet:");
	  switch (packet->type) {
	  case PKT_INIT:
	    printf("INIT\n");
	    if (!ingame) {
	      {
		int n;
		struct setup_packet_s *sinfo = (void*)(packet+1);
		const char *rname = (void*)((short*)(packet+1)+1);
		
		// Add player to the game
		for (n=0; n<MAXPLAYERS; n++)
		  if (playerjoingame[n] == INT_MAX) break;

		if (n == MAXPLAYERS) break; // Full game
		playerjoingame[n] = 0;
		remoteaddr[n] = sentfrom;
		remoteaddr[n].sin_port = *(short*)(packet+1);

		if (!memchr(rname,0,1000)) rname = "Invalid";
		printf("%s(%s:%u) joined\n", rname,
		       inet_ntoa(remoteaddr[n].sin_addr), 
		       ntohs(remoteaddr[n].sin_port));
		{
		  int i;
		  size_t extrabytes = 0;
		  // Send setup packet
		  packet->type = PKT_SETUP;
		  packet->tic = 0;
		  memcpy(sinfo, &setupinfo, sizeof setupinfo);
		  sinfo->yourplayer = n;
		  sinfo->numwads = numwads;
		  for (i=0; i<numwads; i++) {
		    strcpy(sinfo->wadnames + extrabytes, wadname[i]);
		    extrabytes += strlen(wadname[i]) + 1;
		  }
		  I_SendPacketTo(packet, sizeof *packet + sizeof setupinfo + extrabytes, 
				 remoteaddr+n);
		  usleep(10000);
		  I_SendPacketTo(packet, sizeof *packet + sizeof setupinfo + extrabytes, 
				 remoteaddr+n);
		}
	      }
	    }
	    break;
	  case PKT_GO:
	    if (!ingame) {
	      int from = *(byte*)(packet+1);

	      if (playerleftgame[from] == INT_MAX) break;
	      playerleftgame[from] = INT_MAX;
	      if (++curplayers == numplayers) {
		ingame=true;
		printf("All players joined, beginning game.\n");
		packet->type = PKT_GO; packet->tic = 0;
		BroadcastPacket(packet, sizeof *packet);
		usleep(10000);
		BroadcastPacket(packet, sizeof *packet);
		usleep(100000);
	      }
	    }
	    break;
	  case PKT_TICC:
	    {
	      byte tics = *(byte*)(packet+1);
	      int from = *(((byte*)(packet+1))+1);

	      if (verbose>2)
		printf("tics %d - %d from %d\n", packet->tic, packet->tic + tics - 1, from);
	      if (packet->tic > remoteticfrom[from]) {
		// Missed tics, so request a resend
		packet->tic = remoteticfrom[from];
		packet->type = PKT_RETRANS;
		I_SendPacketTo(packet, sizeof *packet, remoteaddr+from);
	      } else {
		ticcmd_t *newtic = (void*)(((byte*)(packet+1))+2);
		if (packet->tic + tics < remoteticfrom[from]) break; // Won't help
		remoteticfrom[from] = packet->tic;
		while (tics--)
		  netcmds[from][remoteticfrom[from]++%BACKUPTICS] =  *newtic++;
	      }
	    }
	    break;
	  case PKT_RETRANS:
	    {
	      int from = *(byte*)(packet+1);
	      if (verbose>2) printf("%d requests resend from %d\n", from, packet->tic);
	      remoteticto[from] = packet->tic;
	    }
	    break;
	  case PKT_QUIT:
	    { 
	      int from = *(byte*)(packet+1);

	      if (verbose>2) printf("%d quits at %d\n", from, packet->tic);
	      if (playerleftgame[from] == INT_MAX) { // In the game
		playerleftgame[from] = packet->tic;
		if (ingame && !--curplayers) exit(0); // All players have exited
	      }
	    }
	    // Fall through and broadcast it
	  case PKT_EXTRA:
	    BroadcastPacket(packet, len);
	    if (packet->type == PKT_EXTRA) {
	      if (verbose>2) printf("misc from %d\n", *(((byte*)(packet+1))+1));
	    }
	    break;
	  case PKT_WAD:
	    {
	      int i;
	      int from = *(byte*)(packet+1);
	      char *name = 1 + (char*)(packet+1);
	      size_t size = sizeof(packet_header_t);
	      packet_header_t *reply;

	      if (verbose) printf("Request for %s ", name);
	      for (i=0; i<numwads; i++)
		if (!strcasecmp(name, wadname[i]))
		  break;

	      if ((i==numwads) || !wadget[i]) {
		if (verbose) printf("n/a\n");
		*(char*)(packet+1) = 0;
		I_SendPacketTo(packet, size+1, remoteaddr + from); 
	      } else {
		size += strlen(wadname[i]) + strlen(wadget[i]) + 2;
		reply = malloc(size);
		reply->type = PKT_WAD; reply->tic = 0;
		strcpy((char*)(reply+1), wadname[i]);
		strcpy((char*)(reply+1) + strlen(wadname[i]) + 1, wadget[i]);
		printf("sending %s\n", wadget[i]);
		I_SendPacketTo(reply, size, remoteaddr + from);
		free(reply);
	      }
	    }
	    break;
	  default:
	    printf("Unrecognised packet type %d\n", packet->type);
	    break;
	  }
	}
	free(packet);
      }

      if (ingame) { // Run some tics
	int lowtic = INT_MAX;
	int i;
	for (i=0; i<MAXPLAYERS; i++) 
	  if (playeringame(i))
	    if (remoteticfrom[i]<lowtic)
	      lowtic = remoteticfrom[i];

	if (verbose>1) printf("%d new tics can be run\n", lowtic - exectics);

	if (lowtic > exectics) 
	  exectics = lowtic; // count exec'ed tics
	// Now send all tics up to lowtic
	for (i=0; i<MAXPLAYERS; i++) 
	  if (playeringame(i)) {
	    int tics;
	    if (lowtic <= remoteticto[i]) continue;
	    remoteticto[i] -= xtratics;
	    tics = lowtic - remoteticto[i]; 
	    {
	      packet_header_t *packet = malloc(sizeof(packet_header_t) + 1 +
				 tics * (1 + numplayers * (1 + sizeof(ticcmd_t))));
	      byte *p = (void*)(packet+1);
	      packet->type = PKT_TICS; packet->tic = remoteticto[i] - xtratics;
	      *p++ = tics;
	      if (verbose>1) printf("sending %d tics to %d\n", tics, i);
	      while (tics--) {
		int j, playersthistic = 0;
		byte *q = p++;
		for (j=0; j<MAXPLAYERS; j++)
		  if ((playerjoingame[j] < remoteticto[i]) && 
		      (playerleftgame[j] > remoteticto[i])) {
		    *p++ = j;
		    memcpy(p, &netcmds[j][remoteticto[i]%BACKUPTICS], sizeof(ticcmd_t));
		    p += sizeof(ticcmd_t);
		    playersthistic++;
		  }
		*q = playersthistic;
		remoteticto[i]++;
	      }
	      I_SendPacketTo(packet, p - ((byte*)packet), remoteaddr+i);
	      free(packet);
	    }
	  }
      }
    }
  }
}

//
// $Log: d_server.c,v $
// Revision 1.11  2000/02/26 19:21:00  cph
// Remove server stats file; be safe with error message printing
//
// Revision 1.10  2000/01/26 08:51:53  cphipps
// Fix random number seed (cures SIGBUS on Sparc, removes endianness ambiguity
//
// Revision 1.9  1999/10/31 16:31:37  cphipps
// Changed signal handler to use function from l_system.c to get the signal
// name. Added #include for that.
//
// Revision 1.8  1999/10/12 13:01:09  cphipps
// Changed header to GPL
//
// Revision 1.7  1999/09/05 10:50:40  cphipps
// Server deduces remote address of a new client from the packet
// sent-from address, instead of the startup packet contents.
//
// Revision 1.6  1999/04/02 11:21:21  cphipps
// Wait for a PKT_GO from each player before starting the game
//
// Revision 1.5  1999/04/02 10:55:44  cphipps
// Send PKT_GO packets when the game starts
// Extra packets sent in case of packet loss in game startup
//
// Revision 1.4  1999/04/01 21:57:32  cphipps
// Add wad file specifying/listing to the server
//
// Revision 1.3  1999/04/01 10:11:37  cphipps
// Added exit/signal handling
// Fix counting of players leaving (again :/)
//
// Revision 1.2  1999/03/29 12:12:35  cphipps
// Misc endian fixes
//
// Revision 1.1  1999/03/29 11:54:34  cphipps
// Initial revision
//
//
