// ipxnet.c

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <process.h>
#include <unistd.h>
#include <values.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#include <dpmi.h>

#include "nullnet.h"

/*
=============================================================================

                              'NULL' PACKET DRIVER

=============================================================================
*/
int network_address;
int network_selector;
int mynode = -1;
node_c node;
/*===========================================================================*/

/*
====================
=
= InitNetwork
=
====================
*/
void InitNetwork(void)
{
}



/*
====================
=
= ShutdownNetwork
=
====================
*/

void ShutdownNetwork (void)
{
  int i;

  if (network_selector >= 0)
    __dpmi_free_dos_memory(network_selector);
}

unsigned short ShortSwap (unsigned short i)
{
     return ((i&255)<<8) + ((i>>8)&255);
}

// Sends the packet to another node
void node_c::SendPacket(int node)
{
  char     filename[256];
  packet_t packet;
  int file = -1;
  int i;

  packet.datalength = doomcom.datalength;
  packet.sender = mynode;
  memcpy(&packet.data, doomcom.data, sizeof(doomcom.data));

  if (node <= mynode)
    node --;

  sprintf(filename, "node%d.net", node);
  file = open(filename, O_BINARY|O_WRONLY|O_APPEND);
  while (file < 0)
  {
    __dpmi_yield();
    file = open(filename, O_BINARY|O_WRONLY|O_APPEND);
  }

  write(file, &packet, 2*sizeof(int) + doomcom.datalength);
  close(file);
}

int node_c::GetPacket(void)
{
  int i;
  packet_t packet;
  int file;
  char filename[256];
  sprintf(filename, "node%d.net", mynode);
  file = open(filename, O_BINARY|O_RDONLY);
  while (file < 0)
  {
    __dpmi_yield();
    file = open(filename, O_BINARY|O_RDONLY);
  }

  lseek(file, offset, SEEK_CUR);
  i = read(file, &packet, sizeof(int) * 2);
  if (i < sizeof(packet_t))
  {
    close(file);
    doomcom.remotenode = -1;
    return 0;
  }
  i = read(file, ((char *) &packet) + sizeof(int) * 2, packet.datalength);
  close(file);
  if (i < sizeof(packet_t))
  {
    doomcom.remotenode = -1;
    return 0;
  }
  if (packet.sender < mynode)
    packet.sender ++;
  doomcom.remotenode = packet.sender;
  doomcom.datalength = packet.datalength;
  memcpy(doomcom.data, &packet.data, doomcom.datalength);
  offset += sizeof(packet_t);
  if (offset > 0x80000)
  {
    char *old;
    int filesize;
    file = open(filename, O_BINARY|O_RDONLY);
    while (file < 0)
    {
      __dpmi_yield();
      file = open(filename, O_BINARY|O_RDONLY);
    }
    lseek(file, offset, SEEK_CUR);
    filesize = filelength(file) - offset;
    old = (char *) malloc(filesize);
    read(file, old, filesize);
    close(file);
    file = open(filename, O_BINARY|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    while (file < 0)
    {
      __dpmi_yield();
      file = open(filename, O_BINARY|O_RDONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    }
    write(file, old, filesize);
    close(file);
    offset = 0;
    free(old);
  }
  return 1;
}

node_c::node_c()
{
   real_doomcom =
     __dpmi_allocate_dos_memory((sizeof(doomcom)+15)>>4, &doomcom_selector) * 16;
   if (real_doomcom < 0)
     Error("Couldn't allocate DOS memory!\n");
   offset = 4;
}

node_c::~node_c()
{
  char filename[256];
  __dpmi_free_dos_memory(doomcom_selector);
  sprintf(filename, "node%d.net", mynode);
  unlink(filename);
}

void node_c::service(void)
{
   dosmemget(real_doomcom, sizeof(doomcom_t), &doomcom);
   if (doomcom.command == CMD_SEND)
          SendPacket (doomcom.remotenode);
	 else if (doomcom.command == CMD_GET)
          GetPacket ();
   dosmemput(&doomcom, sizeof(doomcom_t), real_doomcom);
}

int node_c::GetDoomcom(void)
{
  return real_doomcom;
}
