/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: l_udp.c,v 1.11 2000/01/26 08:44:24 cphipps Exp $
 *
 *  New UDP networking code for LxDoom, based in part on 
 *  the original linuxdoom networking
 *  Copyright (C) 1993-1996 by id Software
 *  Copyright (C) 1999 by Colin Phipps
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
 *  Low level UDP network interface. This is shared between the server 
 *  and client, with SERVER defined for the former to select some extra
 *  functions. Handles socket creation, and packet send and receive.
 *
 *-----------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if defined(__BEOS__)
#define PF_INET AF_INET
#define IPPORT_USERRESERVED 5000
#else
#include <arpa/inet.h>
#endif

#include <../config.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "protocol.h"
#include "i_network.h"
#include "m_argv.h"
#include "lprintf.h"

int sendsocket, recvsocket;
struct sockaddr_in sendtoaddr;
size_t sentbytes, recvdbytes;

//
// UDPsocket
//
static int UDPsocket (void)
{
  int    s;
  
  // allocate a socket
  if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    I_Error("Can't create socket:%08X %s\n", errno, strerror(errno));
  
  return s;
}

//
// BindToLocalPort
//
static void BindToLocalPort( int s, int port )
{
  struct sockaddr_in    address;
  
  memset (&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  
  if (bind(s, (struct sockaddr *)&address, sizeof(address)) == -1)
    I_Error ("BindToPort: bind: %s", strerror(errno));
}

static int GetInAddr(const char* host, struct sockaddr_in *addr)
{
  char hostname[128], *p;

  addr->sin_family = AF_INET;
  if (strlen(host)>127) return 0;
  // Alternative port support
  strcpy(hostname, host); p = strchr(hostname, ':');
  if (p) {
    *p++=0; addr->sin_port = htons(atoi(p));
  } else addr->sin_port = htons(5030 /* Default server port */);

  if (isalpha(hostname[0])) {
    struct hostent*    hostentry;  
    if (!(hostentry = gethostbyname(hostname))) return 0;
    addr->sin_addr.s_addr = *(unsigned long int*)hostentry->h_addr;
  } else {
#ifdef HAVE_INET_ATON
    // dotted-quad ip address
    if (!inet_aton(hostname, &addr->sin_addr))
#endif
      return 0;
  }
  return 1;
}

static byte ChecksumPacket(const packet_header_t* buffer, size_t len)
{
  const byte* p = (void*)buffer; 
  byte sum = 0;

  while (p++, --len)
    sum += *p;

  return sum;
}

struct sockaddr_in sentfrom;

size_t I_GetPacket(packet_header_t* buffer, size_t buflen)
{
  size_t sfsize = sizeof(sentfrom);
  int n = recvfrom(recvsocket, buffer, buflen, 0, 
		   (struct sockaddr*)&sentfrom, &sfsize);
  if (n >= 0) recvdbytes += n;
  else if (errno != EWOULDBLOCK) perror("I_Getpacket:recvfrom");
  return ((n>0 && (buffer->checksum == ChecksumPacket(buffer, n))) ? n : 0);
}

void I_SendPacket(packet_header_t* packet, size_t len)
{
  packet->checksum = ChecksumPacket(packet, len);
  if (sendto(sendsocket, packet, len, 0, (struct sockaddr *)&sendtoaddr, 
	     sizeof sendtoaddr) < 0)
    perror("I_SendPacket: sendto");
  else sentbytes+=len;
}

void I_SendPacketTo(packet_header_t* packet, size_t len, struct sockaddr_in* to)
{
  packet->checksum = ChecksumPacket(packet, len);
  if (sendto(sendsocket, packet, len, 0, (struct sockaddr *)to, sizeof *to)<0)
    perror("I_SendPacketTo: sendto");
  else sentbytes+=len;
}

void I_InitSockets(int localport)
{
  boolean        trueval = true;

  // Open sockets
  sendsocket = UDPsocket();
  recvsocket = UDPsocket(); BindToLocalPort(recvsocket, localport);
#ifdef _WIN32
  ioctlsocket(recvsocket, FIONBIO, (u_long *)&trueval);
#elif defined(__BEOS__)
  setsockopt(recvsocket, SOL_SOCKET, SO_NONBLOCK, &trueval, sizeof(trueval));
#else
  ioctl (recvsocket, FIONBIO, &trueval);
#endif

}

boolean I_InitNetwork(void)
{
  int p, localport = 5029;
  struct { packet_header_t head; short port; char myaddr[200]; } initpacket;

  // Get local & remote network addresses
  if (!(p=M_CheckParm("-net"))) return false;
  if (++p>=myargc) return false;
  if (!GetInAddr(myargv[p], &sendtoaddr)) 
    I_Error("I_InitNetwork: Unable to locate server.\n");
  if ((p=M_CheckParm("-port")) && (++p<myargc)) 
    localport = atoi(myargv[p]);
  if (gethostname(initpacket.myaddr, 200)<0) 
    strcpy(initpacket.myaddr, "too.long");

  I_InitSockets(localport);
  // Send init packet
  initpacket.port = htons(localport);
  initpacket.head.type = PKT_INIT; initpacket.head.tic = 0;
  I_SendPacket(&initpacket.head, sizeof(initpacket));
  return 1;
}

/*
 * $Log: l_udp.c,v $
 * Revision 1.11  2000/01/26 08:44:24  cphipps
 * Support filio.h so Solaris can get the non-blocking socket ioctl
 *
 * Revision 1.10  1999/12/26 17:46:18  cphipps
 * Always set socket to non-blocking, using an ioctl if nothing better
 *
 * Revision 1.9  1999/12/18 15:48:10  cphipps
 * Use autoconf script to detect inet_aton
 *
 * Revision 1.8  1999/10/31 16:25:22  cphipps
 * Change i_system.h include to lprintf.h, where I_Error is now
 *
 * Revision 1.7  1999/10/12 13:01:11  cphipps
 * Changed header to GPL
 *
 * Revision 1.6  1999/09/05 13:57:38  cphipps
 * Added pointer cast for libc5
 *
 * Revision 1.5  1999/09/05 10:49:52  cphipps
 * I_GetPacket stores the address that sent the packet, used
 * by server during startup
 * Init packet no longer contains the full machine address, instead
 * just the port, and the hostname just for display
 * Failure to lookup local hostname is no longer a fatal error
 *
 * Revision 1.4  1999/04/01 09:46:05  cphipps
 * Sort out default port numbers
 * Add accounting (count sent/rec'ved bytes)
 *
 * Revision 1.3  1999/03/30 06:52:49  cphipps
 * Add cast to sendto(2) parameter to satisfy fussy libc5 headers
 * Fix endianness of port send in PKT_INIT packet by client
 *
 * Revision 1.2  1999/03/29 12:11:38  cphipps
 * Fix endianness of port numbers
 *
 * Revision 1.1  1999/03/29 11:55:30  cphipps
 * Initial revision
 *
 */
