//  
// DOSDoom Networking Code (OS independend parts)
//
// Based on the Doom Source Code,
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -MH- 1998/07/02 "shootupdown" --> "true3dgameplay"
//
// -ACB- 1998/07/25 DEVELOPERS define for debugging
//                  Extended Settings output more descriptive
//
// -ACB- 1998/09/06 Removed the startmap/startepisode stuff
//
// -KM- 1998/12/21 3 monitor view works.
// TODO: Make sure DDF/RTS files are the same on all PC's on a network
//

#include <stdlib.h>

#include "m_fixed.h"
#include "ddf_main.h"
#include "i_system.h"
#ifdef DJGPP
#include "i_music.h"
#endif
#include "i_video.h"
#include "i_net.h"
#include "g_game.h"
#include "d_debug.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "m_argv.h"
#include "m_menu.h"
#define	NCMD_EXIT		0x80000000
#define	NCMD_RETRANSMIT		0x40000000
#define	NCMD_SETUP		0x20000000
#define	NCMD_KILL		0x10000000	// kill game
#define	NCMD_CHECKSUM	 	0x0fffffff

 
doomcom_t*	doomcom;	
doomdata_t*	netbuffer;		// points inside doomcom


//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tick that hasn't had control made for it yet
// nettics[] has the maketics for all players 
//
// a gametic cannot be run until nettics[] > gametic for all players
//
#define	RESENDCOUNT	10
#define	PL_DRONE	0x80	// bit flag in doomdata->player

ticcmd_t	localcmds[BACKUPTICS];

ticcmd_t        netcmds[MAXPLAYERS][BACKUPTICS];
int         	nettics[MAXNETNODES];
boolean		nodeingame[MAXNETNODES];		// set false as nodes leave game
boolean		remoteresend[MAXNETNODES];		// set when local needs tics
int		resendto[MAXNETNODES];			// set when remote needs tics
int		resendcount[MAXNETNODES];

int		nodeforplayer[MAXPLAYERS];

int             maketic;
int		lastnettic;
int		skiptics;
int		ticdup;		
int		maxsend;	// BACKUPTICS/(2*ticdup)-1


void D_ProcessEvents (void); 
void G_BuildTiccmd (ticcmd_t *cmd); 
void D_DoAdvanceDemo (void);
 
boolean		reboundpacket;
doomdata_t	reboundstore;



//
//
//
int NetbufferSize (void)
{
    return (int)&(((doomdata_t *)0)->cmds[netbuffer->numtics]); 
}

//
// Checksum 
//
unsigned NetbufferChecksum (void)
{
    unsigned		c;
    int		i,l;

    c = 0x1234567;

    l = (NetbufferSize () - (int)&(((doomdata_t *)0)->retransmitfrom))/4;
    for (i=0 ; i<l ; i++)
	c += ((unsigned *)&netbuffer->retransmitfrom)[i] * (i+1);

    return c & NCMD_CHECKSUM;
}

//
//
//
int ExpandTics (int low)
{
    int	delta;
	
    delta = low - (maketic&0xff);
	
    if (delta >= -64 && delta <= 64)
	return (maketic&~0xff) + low;
    if (delta > 64)
	return (maketic&~0xff) - 256 + low;
    if (delta < -64)
	return (maketic&~0xff) + 256 + low;
		
    I_Error ("ExpandTics: strange value %i at maketic %i",low,maketic);
    return 0;
}



//
// HSendPacket
//
void
HSendPacket
 (int	node,
  int	flags )
{

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
    int i;
    int realretrans;
#endif

    netbuffer->checksum = NetbufferChecksum () | flags;

    if (!node)
    {
	reboundstore = *netbuffer;
	reboundpacket = true;
	return;
    }

    if (demoplayback)
	return;

    if (!netgame)
	I_Error ("Tried to transmit to another node");
		
    doomcom->command = CMD_SEND;
    doomcom->remotenode = node;
    doomcom->datalength = NetbufferSize ();

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
	if (netbuffer->checksum & NCMD_RETRANSMIT)
	    realretrans = ExpandTics (netbuffer->retransmitfrom);
	else
	    realretrans = -1;

	Debug_Printf("send (%i + %i, R %i) [%i] ",
		 ExpandTics(netbuffer->starttic),
		 netbuffer->numtics, realretrans, doomcom->datalength);
	
	for (i=0 ; i<doomcom->datalength ; i++)
	    Debug_Printf("%i ",((byte *)netbuffer)[i]);

	Debug_Printf("\n");
#endif

    I_NetCmd ();
}

//
// HGetPacket
// Returns false if no packet is waiting
//
boolean HGetPacket (void)
{

#ifdef DEVELOPERS
    int realretrans;
    int i;
#endif

    if (reboundpacket)
    {
	*netbuffer = reboundstore;
	doomcom->remotenode = 0;
	reboundpacket = false;
	return true;
    }

    if (!netgame)
	return false;

    if (demoplayback)
	return false;
		
    doomcom->command = CMD_GET;
    I_NetCmd ();
    
    if (doomcom->remotenode == -1)
	return false;

    if (doomcom->datalength != NetbufferSize ())
    {

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
	Debug_Printf("bad packet length %i\n",doomcom->datalength);
#endif

	return false;
    }
	
    if (NetbufferChecksum () != (netbuffer->checksum&NCMD_CHECKSUM) )
    {

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
	Debug_Printf("bad packet checksum %i\n",doomcom->datalength);
#endif

	return false;
    }

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
			
	if (netbuffer->checksum & NCMD_SETUP)
	    Debug_Printf("setup packet\n");
	else
	{
	    if (netbuffer->checksum & NCMD_RETRANSMIT)
		realretrans = ExpandTics (netbuffer->retransmitfrom);
	    else
		realretrans = -1;
	    
	    Debug_Printf("get %i = (%i + %i, R %i)[%i] ",
		     doomcom->remotenode,
		     ExpandTics(netbuffer->starttic),
		     netbuffer->numtics, realretrans, doomcom->datalength);

	    for (i=0 ; i<doomcom->datalength ; i++)
		Debug_Printf("%i ",((byte *)netbuffer)[i]);
	    Debug_Printf("\n");
	}
#endif

    return true;	
}


//
// GetPackets
//
char    exitmsg[80];

void GetPackets (void)
{
    int		netconsole;
    int		netnode;
    ticcmd_t	*src, *dest;
    int		realend;
    int		realstart;
				 
    while ( HGetPacket() )
    {
	if (netbuffer->checksum & NCMD_SETUP)
	    continue;		// extra setup packet
			
	netconsole = netbuffer->player & ~PL_DRONE;
        if (netbuffer->player & PL_DRONE)
           doomcom->drone |= 1<<netconsole;

	netnode = doomcom->remotenode;

	// to save bytes, only the low byte of tic numbers are sent
	// Figure out what the rest of the bytes are
	realstart = ExpandTics (netbuffer->starttic);		
	realend = (realstart+netbuffer->numtics);
	
	// check for exiting the game
	if (netbuffer->checksum & NCMD_EXIT)
	{
	    if (!nodeingame[netnode])
		continue;
	    nodeingame[netnode] = false;
	    playeringame[netconsole] = false;

            // -KM- 1998/12/21 netconsole is 0 based, add 1.
            sprintf(exitmsg, "Player %d left the game", netconsole + 1);
	    players[consoleplayer].message = exitmsg;

	    if (demorecording)
		G_CheckDemoStatus ();
	    continue;
	}
	
	// check for a remote game kill
	if (netbuffer->checksum & NCMD_KILL)
	    I_Error ("Killed by network driver");

	nodeforplayer[netconsole] = netnode;
	
	// check for retransmit request
	if ( resendcount[netnode] <= 0 
	     && (netbuffer->checksum & NCMD_RETRANSMIT) )
	{
	    resendto[netnode] = ExpandTics(netbuffer->retransmitfrom);

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
            Debug_Printf("retransmit from %i\n", resendto[netnode]);
#endif

	    resendcount[netnode] = RESENDCOUNT;
	}
	else
	    resendcount[netnode]--;
	
	// check for out of order / duplicated packet		
	if (realend == nettics[netnode])
	    continue;
			
	if (realend < nettics[netnode])
	{

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
                Debug_Printf("out of order packet (%i + %i)\n" ,
			 realstart,netbuffer->numtics);
#endif

	    continue;
	}
	
	// check for a missed packet
	if (realstart > nettics[netnode])
	{
	    // stop processing until the other system resends the missed tics

// -ACB- 1998/07/17 Use DEVELOPERS define
#ifdef DEVELOPERS
                Debug_Printf(
			 "missed tics from %i (%i - %i)\n",
			 netnode, realstart, nettics[netnode]);
#endif

	    remoteresend[netnode] = true;
	    continue;
	}

	// update command store from the packet
        {
	    int		start;

	    remoteresend[netnode] = false;
		
	    start = nettics[netnode] - realstart;		
	    src = &netbuffer->cmds[start];

	    while (nettics[netnode] < realend)
	    {
		dest = &netcmds[netconsole][nettics[netnode]%BACKUPTICS];
		nettics[netnode]++;
		*dest = *src;
		src++;
	    }
	}
    }
}


//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
int      gametime;

void NetUpdate (void)
{
    int             nowtime;
    int             newtics;
    int				i,j;
    int				realstart;
    int				gameticdiv;
    
    // check time
    nowtime = I_GetTime ()/ticdup;
    newtics = nowtime - gametime;
    gametime = nowtime;
	
    // -ACB- 1998/07/17 removed goto - educational note: GOTO's are best avoided.
    if (newtics <= 0) 	// nothing new to update
    {
     GetPackets ();
     return;
    }

    if (skiptics <= newtics)
    {
	newtics -= skiptics;
	skiptics = 0;
    }
    else
    {
	skiptics -= newtics;
	newtics = 0;
    }
	
    netbuffer->player = consoleplayer;
    if (gameflags.drone)
      netbuffer->player |= PL_DRONE;
    
    // build new ticcmds for console player
    gameticdiv = gametic/ticdup;
    for (i=0 ; i<newtics ; i++)
    {
	I_StartTic ();
	D_ProcessEvents ();
	if (maketic - gameticdiv >= BACKUPTICS/2-1)
	    break;          // can't hold any more
	
	//printf ("mk:%i ",maketic);
	G_BuildTiccmd (&localcmds[maketic%BACKUPTICS]);
	maketic++;
    }


    if (singletics)
	return;         // singletic update is syncronous
    
    // send the packet to the other nodes
    for (i=0 ; i<doomcom->numnodes ; i++)
	if (nodeingame[i])
	{
	    netbuffer->starttic = realstart = resendto[i];
	    netbuffer->numtics = maketic - realstart;
	    if (netbuffer->numtics > BACKUPTICS)
		I_Error ("NetUpdate: netbuffer->numtics > BACKUPTICS");

	    resendto[i] = maketic - doomcom->extratics;

	    for (j=0 ; j< netbuffer->numtics ; j++)
		netbuffer->cmds[j] = 
		    localcmds[(realstart+j)%BACKUPTICS];
					
	    if (remoteresend[i])
	    {
		netbuffer->retransmitfrom = nettics[i];
		HSendPacket (i, NCMD_RETRANSMIT);
	    }
	    else
	    {
		netbuffer->retransmitfrom = 0;
		HSendPacket (i, 0);
	    }
	}
    
    // -ACB- 1998/07/17 removed goto - educational note: GOTO's are best avoided.
    GetPackets ();
}



//
// CheckAbort
//
void CheckAbort (void)
{
    event_t *ev;
    int		stoptic;
	
    stoptic = I_GetTime () + 2; 
    while (I_GetTime() < stoptic) 
	I_StartTic (); 
	
    I_StartTic ();
    for ( ; eventtail != eventhead 
	      ; eventtail = (++eventtail)&(MAXEVENTS-1) ) 
    { 
	ev = &events[eventtail]; 
	if (ev->type == ev_keydown && ev->data1 == KEYD_ESCAPE)
	    I_Error ("Network game synchronization aborted.");
    } 
}


//
// D_ArbitrateNetStart
//
// -ACB- 1998/07/25 Cleaned up the extended settings; Output more descriptive.
//
void D_ArbitrateNetStart (void)
{
    int		i;
    boolean	gotinfo[MAXNETNODES];
    byte buffer=0; //-jc-
	
    autostart = true;
    memset (gotinfo,0,sizeof(gotinfo));

    if (doomcom->consoleplayer)
    {
	// listen for setup info from key player
	I_Printf (DDF_LanguageLookup("ListenNet"));
	while (1)
	{
	    CheckAbort ();

	    if (!HGetPacket ())
		continue;

	    if (netbuffer->checksum & NCMD_SETUP)
	    {
                if (netbuffer->player != DEMOVERSION)
		    I_Error ("DOSDoom Versions are incompatiable!");

		// Read Ticcmd for all the gameplay settings
                settingflags.true3dgameplay = netbuffer->cmds->chatchar&1;
                settingflags.itemrespawn = netbuffer->cmds->chatchar&2;
                settingflags.respawnsetting = netbuffer->cmds->chatchar&4;
                settingflags.respawn = netbuffer->cmds->chatchar&8;
                settingflags.fastparm = netbuffer->cmds->chatchar&16;
                settingflags.blood = netbuffer->cmds->chatchar&32;
                settingflags.jump = netbuffer->cmds->chatchar&64;
                settingflags.freelook = netbuffer->cmds->chatchar&128;
                settingflags.trans=netbuffer->cmds->buttons&1;

                settingflags.grav=netbuffer->cmds->angleturn;

		startskill = netbuffer->retransmitfrom&31;

                deathmatch = netbuffer->retransmitfrom >> 6;
		settingflags.nomonsters = netbuffer->retransmitfrom & 0x20;

                // -ACB- 1998/07/25 Display the current settings
                I_Printf("             True3D: %s\n",
                                       settingflags.true3dgameplay ? "On" : "Off" );
                I_Printf("   Enemy Respawning: ");
                if (settingflags.respawnsetting == RS_RESURRECT)
                  I_Printf("Resurrection ");
                else if (settingflags.respawnsetting == RS_TELEPORT)
                  I_Printf("Teleport     ");
                I_Printf(settingflags.respawn?"On\n":"Off\n");
        
                // Shows enabled even if not set with altdeath
                I_Printf("       Item Respawn: %s\n",
                                       settingflags.itemrespawn ? "On" : "Off" );
        
                I_Printf("      Gravity Level: %d\n",settingflags.grav );
                I_Printf("           Fastparm: %s\n", settingflags.fastparm?"On":"Off");
                I_Printf("              Blood: %s\n", settingflags.blood?"On":"Off");
                I_Printf("            Jumping: %s\n", settingflags.jump?"On":"Off");
                I_Printf("           Freelook: %s\n", settingflags.freelook?"On":"Off");
                I_Printf("       Translucency: %s\n", settingflags.trans?"On":"Off");
                I_Printf("           Monsters: %s\n", settingflags.nomonsters?"Off":"On");

		return;
	    } else if (netbuffer->player & PL_DRONE)
            {
              doomcom->drone |= 1<<(netbuffer->player&0x7f);
            }
	}
    }
    else
    {
	// key player, send the setup info
	I_Printf (DDF_LanguageLookup("SendNet"));
        // -ACB- 1998/07/25 Display the current settings
        // -KM- 1998/12/21 Moved Display Here, so it only gets displayed once.
        I_Printf("                   True3D: %s\n",
                               settingflags.true3dgameplay ? "On" : "Off" );
                //                         :
        I_Printf(" %13s Respawning: ", settingflags.respawnsetting == RS_RESURRECT?"Resurrection":"Teleport");
        I_Printf(settingflags.respawn?"On\n":"Off\n");
        // Shows enabled even if not set with altdeath
        I_Printf("       Item Respawn: %s\n",
                               settingflags.itemrespawn ? "On" : "Off" );
        I_Printf("      Gravity Level: %d\n",settingflags.grav );
        I_Printf("           Fastparm: %s\n", settingflags.fastparm?"On":"Off");
        I_Printf("              Blood: %s\n", settingflags.blood?"On":"Off");
        I_Printf("            Jumping: %s\n", settingflags.jump?"On":"Off");
        I_Printf("           Freelook: %s\n", settingflags.freelook?"On":"Off");
        I_Printf("       Translucency: %s\n", settingflags.trans?"On":"Off");
        I_Printf("           Monsters: %s\n", settingflags.nomonsters?"Off":"On");
	do
	{
	    CheckAbort ();
	    for (i=0 ; i<doomcom->numnodes ; i++)
	    {
                buffer=settingflags.true3dgameplay?1:0;
                buffer|=settingflags.itemrespawn?2:0;
                buffer|=settingflags.respawnsetting?4:0;
                buffer|=settingflags.respawn?8:0;
                buffer|=settingflags.fastparm?16:0;
                buffer|=settingflags.blood?32:0;
                buffer|=settingflags.jump?64:0;
                buffer|=settingflags.freelook?128:0;
                netbuffer->cmds->chatchar=buffer;

                buffer=settingflags.trans?1:0;
                netbuffer->cmds->buttons=buffer;

                netbuffer->cmds->angleturn=settingflags.grav;

		netbuffer->retransmitfrom = startskill;

                if (deathmatch)
                  netbuffer->retransmitfrom |= (deathmatch<<6);
		if (settingflags.nomonsters)
		    netbuffer->retransmitfrom |= 0x20;

//		netbuffer->starttic = startepisode * 64 + startmap;
		netbuffer->player = DEMOVERSION;


                // Fake 1 tic if not using "-oldset" this will hold the
                // extended startup information.
                netbuffer->numtics = 1; //-JC- WAS 0

		HSendPacket (i, NCMD_SETUP);


	    }

	    while (HGetPacket ())
	    {
		gotinfo[netbuffer->player&0x7f] = true;
                if (netbuffer->player & PL_DRONE)
                  doomcom->drone |= 1<<(netbuffer->player&0x7f);
	    }

	    for (i=1 ; i<doomcom->numnodes ; i++)
		if (!gotinfo[i])
		    break;

	} while (i < doomcom->numnodes);
    }
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
void D_CheckNetGame (void)
{
    int             i;
	
    for (i=0 ; i<MAXNETNODES ; i++)
    {
	nodeingame[i] = false;
       	nettics[i] = 0;
	remoteresend[i] = false;	// set when local needs tics
	resendto[i] = 0;		// which tic to start sending
    }
	
    // I_InitNetwork sets doomcom and netgame
    I_InitNetwork ();
    if (doomcom->id != DOOMCOM_ID)
	I_Error ("Doomcom buffer invalid!");
    
    netbuffer = &doomcom->data;
    consoleplayer = displayplayer = doomcom->consoleplayer;

    if (netgame)
	D_ArbitrateNetStart ();

    I_Printf ("startskill %i  deathmatch: %i ", startskill, deathmatch);
	
    // read values out of doomcom
    ticdup = doomcom->ticdup;
    maxsend = BACKUPTICS/(2*ticdup)-1;
    if (maxsend<1)
	maxsend = 1;
			
    for (i=0 ; i<doomcom->numplayers ; i++)
	playeringame[i] = true;
    for (i=0 ; i<doomcom->numnodes ; i++)
	nodeingame[i] = true;

    i = M_CheckParm("-viewangle");
    if (i && i < myargc-1)
    {
      settingflags.viewangleoffset = 0xB60B60 * atoi(myargv[i+1]);
      if (netgame)
      {
        settingflags.drone = true;
        doomcom->drone |= 1<<consoleplayer;
      }
      // -KM- 1998/12/21 Find the display player.
      //  DOSDoom has enough network players for
      //  more than one persone with 3 monitors each.
      if (i < myargc-2)
        displayplayer = atoi(myargv[i+2]);
      else
        displayplayer = 0;
    }

    I_Printf ("player %i of %i (%i nodes)\n",
	    consoleplayer+1, doomcom->numplayers, doomcom->numnodes);
}


//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame (void)
{
    int             i, j;
		
    if (!netgame || !usergame || consoleplayer == -1 || demoplayback)
	return;
	
    // send a bunch of packets for security
    netbuffer->player = consoleplayer;
    if (gameflags.drone)
      netbuffer->player |= PL_DRONE;
    netbuffer->numtics = 0;
    for (i=0 ; i<4 ; i++)
    {
	for (j=1 ; j<doomcom->numnodes ; j++)
	    if (nodeingame[j])
		HSendPacket (j, NCMD_EXIT);
	I_WaitVBL (1);
    }
}



//
// TryRunTics
//
int	frameon;
int	frameskip[MAXPLAYERS];
int	oldnettics;

extern	boolean	advancedemo;

void TryRunTics (void)
{
    int		i;
    int		lowtic;
    int		entertic;
    static int	oldentertics;
    int		realtics;
    int		availabletics;
    int		counts;
    int		numplaying;
    
    // get real tics		
    entertic = I_GetTime ()/ticdup;
    realtics = entertic - oldentertics;
    oldentertics = entertic;
    
    // get available tics
    NetUpdate ();
	
    lowtic = MAXINT;
    numplaying = 0;
    for (i=0 ; i<doomcom->numnodes ; i++)
    {
	if (nodeingame[i])
	{
	    numplaying++;
	    if (nettics[i] < lowtic)
		lowtic = nettics[i];
	}
    }
    availabletics = lowtic - gametic/ticdup;
    
    // decide how many tics to run
    if (realtics < availabletics-1)
	counts = realtics+1;
    else if (realtics < availabletics)
	counts = realtics;
    else
	counts = availabletics;
    
    if (counts < 1)
	counts = 1;
		
    frameon++;

#ifdef DEVELOPERS
        Debug_Printf("=======real: %i  avail: %i  game: %i\n",
                       realtics, availabletics, counts);
#endif

    if (!demoplayback)
    {	
	// ideally nettics[0] should be 1 - 3 tics above lowtic
	// if we are consistantly slower, speed up time
	for (i=0 ; i<MAXPLAYERS ; i++)
	    if (playeringame[i])
		break;
        // the key player does not adapt
	if (consoleplayer != i)
	{
            if (nettics[0] <= nettics[nodeforplayer[i]])
                gametime--;
                        frameskip[frameon&3] = (oldnettics > nettics[nodeforplayer[i]]);
            oldnettics = nettics[0];
            if (frameskip[0] && frameskip[1] && frameskip[2] && frameskip[3])
               skiptics = 1;
	}
    }// demoplayback
	
    // wait for new tics if needed
    while (lowtic < gametic/ticdup + counts)	
    {
	NetUpdate ();   
	lowtic = MAXINT;
	
	for (i=0 ; i<doomcom->numnodes ; i++)
	    if (nodeingame[i] && nettics[i] < lowtic)
		lowtic = nettics[i];
	
	if (lowtic < gametic/ticdup)
	    I_Error ("TryRunTics: lowtic < gametic");
				
	// don't stay in here forever -- give the menu a chance to work
	if (I_GetTime ()/ticdup - entertic >= 20)
	{
	    M_Ticker ();
	    return;
	}
        I_MusicTicker2();
    }
    
    // run the count * ticdup dics
    while (counts--)
    {
	for (i=0 ; i<ticdup ; i++)
	{
	    if (gametic/ticdup > lowtic)
		I_Error ("gametic>lowtic");
	    if (advancedemo)
		D_DoAdvanceDemo ();
	    M_Ticker ();
	    G_Ticker ();
	    gametic++;
	    
	    // modify command for duplicated tics
	    if (i != ticdup-1)
	    {
		ticcmd_t	*cmd;
		int			buf;
		int			j;
				
		buf = (gametic/ticdup)%BACKUPTICS; 
		for (j=0 ; j<MAXPLAYERS ; j++)
		{
		    cmd = &netcmds[j][buf];
		    cmd->chatchar = 0;
		    if (cmd->buttons & BT_SPECIAL)
			cmd->buttons = 0;
		}
	    }
	}
	NetUpdate ();	// check for new console commands
        I_MusicTicker2();
    }
}
