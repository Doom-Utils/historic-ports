/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: i_network.h,v 1.4 1999/10/12 13:00:57 cphipps Exp $
 *
 *  New low level networking code for LxDoom, based in part on 
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
 *  Low level network interface. 
 *-----------------------------------------------------------------------------*/

boolean I_InitNetwork(void);
size_t I_GetPacket(packet_header_t* buffer, size_t buflen);
void I_SendPacket(packet_header_t* packet, size_t len);
#ifdef SERVER
void I_SendPacketTo(packet_header_t* packet, size_t len, struct sockaddr_in* to);
void I_InitSockets(int localport);
extern struct sockaddr_in sentfrom;
#endif

extern size_t sentbytes, recvdbytes;

/*
 * $Log: i_network.h,v $
 * Revision 1.4  1999/10/12 13:00:57  cphipps
 * Changed header to GPL, converted C++ comments to C
 *
 * Revision 1.3  1999/09/05 10:48:30  cphipps
 * Added sentfrom address so server can work out client addresses
 * when they connect.
 *
 * Revision 1.2  1999/04/01 09:38:09  cphipps
 * Add variables holding stats
 *
 * Revision 1.1  1999/03/29 11:55:09  cphipps
 * Initial revision
 *
 */
