// ipxnet.h


typedef struct
{
     char data[512];
} doomdata_t;


#include "DoomNet.h"

//===========================================================================

#define NUMPACKETS      10              // max outstanding packets before loss

// setupdata_t is used as doomdata_t during setup
/*
typedef struct
{
     short     gameid;                       // so multiple games can setup at once
     short     drone;
     short     nodesfound;
     short     nodeswanted;
} setupdata_t;
*/


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long LONG;

typedef struct
{
     WORD cs;
     WORD ip;
} nodeadr_t;

// time is used by the communication driver to sequence packets returned
// to DOOM when more than one is waiting

typedef struct
{
     int                 sender;
     int                 datalength;
     doomdata_t          data;
} packet_t;

class node_c
{
  int         real_doomcom;
  int         doomcom_selector;
  // in queue packet list
  int         offset;

public:
  doomcom_t   doomcom;

  int GetDoomcom(void);

  // sendpacket // sends a packet to another node
  void SendPacket(int node);

  // _recievepacket // recieves a packet from another node, adds to queue
  void _recievepacket(void);

  // getpacket // gets next packet.
  int GetPacket(void);

  void service(void);

  node_c();
  ~node_c();
};

extern int network_address;
extern int network_selector;
extern int mynode;
extern node_c node;

extern	  int	myargc;

extern	  char  **myargv;
extern    char  *argused;

void Error (char *error, ...);


void InitNetwork (void);
void ShutdownNetwork (void);
int CheckParm (char *check);

void PrintAddress (nodeadr_t *adr, char *str);
void _receivepacket(void);

