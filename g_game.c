//
// DOSDoom Game Handling Code
//
// Based on the Doom Source Code,
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -MH- 1998/07/02 Added key_flyup and key_flydown variables (no logic yet)
// -MH- 1998/08/18 Flyup and flydown logic
//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "d_debug.h"

#include "dm_defs.h" 
#include "dm_state.h"

#include "z_zone.h"
#include "f_finale.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "i_system.h"

#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"

#include "d_main.h"

#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

// Needs access to LFB.
#include "v_res.h"

#include "w_wad.h"

#include "p_local.h" 

#include "s_sound.h"

// Data.
#include "dstrings.h"
#include "lu_sound.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "r_sky.h"

#include "g_game.h"

#include "rad_trig.h"

#define SAVEGAMESIZE	0x50000
#define SAVESTRINGSIZE	24

boolean	G_CheckDemoStatus (void); 
void	G_ReadDemoTiccmd (ticcmd_t* cmd); 
void	G_WriteDemoTiccmd (ticcmd_t* cmd); 
void	G_PlayerReborn (int player); 
 
void	G_DoReborn (int playernum); 
 
void	G_DoLoadLevel (void); 
void	G_DoNewGame (void); 
void	G_DoLoadGame (void); 
void	G_DoPlayDemo (void); 
void	G_DoCompleted (void); 
void	G_DoVictory (void); 
void	G_DoWorldDone (void); 
void	G_DoSaveGame (void); 

// required for .. spawning a player, what else?.
void P_SpawnPlayer (mapthing_t* mthing); 
 
gameaction_t    gameaction; 
gamestate_t     gamestate; 
skill_t         gameskill = -1;
mapstuff_t*     currentmap = NULL; // currentmap
 
boolean         paused; 
boolean         sendpause;             	// send a pause event next tic 
boolean         sendsave;             	// send a save event next tic 
boolean         usergame;               // ok to save / end game 
 
boolean         timingdemo;             // if true, exit with report on completion 
boolean         nodrawers;              // for comparative timing purposes 
boolean         noblit;                 // for comparative timing purposes 
int             starttime;          	// for comparative timing purposes
// -KM- 1998/11/25 Exit time is the time when the level will actually finish
// after hitting the exit switch/killing the boss.  So that you see the
// switch change or the boss die.
int             exittime = 0x7fffffff;
 
boolean         viewactive; 
 
boolean         deathmatch;           	// only if started as net death 
boolean         netgame;                // only true if packets are broadcast 
boolean         playeringame[MAXPLAYERS]; 
player_t        players[MAXPLAYERS]; 
 
int             consoleplayer;          // player taking events and displaying 
int             displayplayer;          // view being displayed 
int             gametic; 
int             levelstarttic;          // gametic at level start 
int             totalkills, totalitems, totalsecret;    // for intermission

char            demoname[32]; 
boolean         demorecording; 
boolean         demoplayback; 
boolean		netdemo; 
boolean		newdemo;

// 98-7-10 KM Remove maxdemo limit
byte*		demobuffer = NULL;
int		demo_p;
int		maxdemo;
boolean         singledemo;            	// quit after playing a demo from cmdline 
 
boolean         precache = true;        // if true, load all graphics at start 
 
wbstartstruct_t wminfo;               	// parms for world map / intermission 
 
short		consistancy[MAXPLAYERS][BACKUPTICS]; 
 
byte*		savebuffer;
 
// 
// controls (have defaults) 
// 
int             key_right;
int		key_left;
int             key_lookup;
int             key_lookdown;
int             key_lookcenter;

int		key_up;
int		key_down; 
int             key_strafeleft;
int		key_straferight; 
int             key_fire;
int		key_use;
int		key_strafe;
int		key_speed;
int             key_nextweapon;
int             key_jump;
int             key_map;
int             key_180;
int             key_talk;

// -MH- 1998/07/10 Flying keys
int             key_flyup;
int             key_flydown;

#define MAXPLMOVE		(forwardmove[1]) 
 
#define TURBOTHRESHOLD	0x32

fixed_t		forwardmove[2] = {0x19, 0x32}; 
fixed_t         upwardmove[2] = {0x19, 0x32};     // -MH- 1998/08/18 Up/Down movement
fixed_t		sidemove[2] = {0x18, 0x28}; 
fixed_t		angleturn[3] = {640, 1280, 320};	// + slow turn 

#define SLOWTURNTICS	6 
 
#define NUMKEYS		512

boolean         gamekeydown[NUMKEYS]; 
int             turnheld;				// for accelerative turning 

//-------------------------------------------
// -KM-  1998/09/01 Analogue binding
// -ACB- 1998/09/06 Two-stage turning switch
//

int mouse_xaxis = AXIS_TURN;    // joystick values are used once
int mouse_yaxis = AXIS_FORWARD;
 
int joy_xaxis = AXIS_TURN;      // joystick values are repeated
int joy_yaxis = AXIS_FORWARD;
 
int  analogue[5] = {0, 0, 0, 0, 0};

boolean stageturn;              // Stage Turn Control

int forwardmovespeed;           // Speed controls
int angleturnspeed;
int sidemovespeed;

//--------------------------------------------
 
int		savegameslot; 
char		savedescription[32]; 
 
 
#define	BODYQUESIZE	32

mobj_t*		bodyque[BODYQUESIZE]; 
int		bodyqueslot; 
 
void*		statcopy;				// for statistics driver
 

// -KM- 1998/09/01 Made static.
static int CheckKey(int keynum)
{

#ifdef DEVELOPERS
  if ((keynum>>16)>NUMKEYS)
    I_Error("Invalid key!");
  else if ((keynum&0xffff)>NUMKEYS)
    I_Error("Invalid key!");
#endif

  if (gamekeydown[keynum>>16])
    return true;
  else if (gamekeydown[keynum&0xffff])
    return true;
  else
    return false;
}

 
int G_CmdChecksum (ticcmd_t* cmd) 
{ 
    int		i;
    int		sum = 0; 
	 
    for (i=0 ; i< sizeof(*cmd)/4 - 1 ; i++) 
	sum += ((int *)cmd)[i]; 
		 
    return sum; 
} 
 

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer. 
// If recording a demo, write it out 
//
// -ACB- 1998/07/02 Added Vertical angle checking for mlook.
// -ACB- 1998/07/10 Reformatted: I can read the code! :)
// -ACB- 1998/09/06 Apply speed controls to -KM-'s analogue controls
//
#define G_DefineGetSpeedDivisor(speed) ((8-speed)<<4)

void G_BuildTiccmd (ticcmd_t* cmd) 
{ 
    int		i; 
    boolean	strafe;
    int         vertangle; // -ACB- 1998/07/02 Look angle
    int		speed;
    int		tspeed; 
    int		forward;
    int         upward;    // -MH- 1998/08/18 Fly Up/Down movement
    int		side;
    ticcmd_t*	base;
    static boolean allow180=true;

    base = I_BaseTiccmd ();		// empty, or external driver
    memcpy (cmd,base,sizeof(*cmd)); 
	
    cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS];

    // -KM- 1998/12/21 If a drone player, do not accept input.
    if (gameflags.drone)
      return;

    vertangle = 0;

    strafe = CheckKey(key_strafe);
    speed = CheckKey(key_speed);

    if ((key_shifts&KB_CAPSLOCK_FLAG)==KB_CAPSLOCK_FLAG)
      speed=!speed;
      
    upward = forward = side = 0;

    //
    // -KM- 1998/09/01 use two stage accelerative turning on all devices
    //
    // -ACB- 1998/09/06 Allow stage turning to be switched off for
    //                  analogue devices...
    //
    if (CheckKey(key_right)||CheckKey(key_left)||(analogue[AXIS_TURN]&&stageturn))
      turnheld += ticdup; 
    else
      turnheld = 0; 

    if (turnheld < SLOWTURNTICS)
      tspeed = 2;             // slow turn
    else
      tspeed = speed;
    
    if (gameflags.freelook)
    {
      // -ACB- 1998/07/02 Use VertAngle for Look/up down.
      if (CheckKey(key_lookup))
        vertangle+=keylookspeed*64;
  
      // -ACB- 1998/07/02 Use VertAngle for Look/up down.
      if (CheckKey(key_lookdown))
        vertangle-=keylookspeed*64;
  
      // -ACB- 1998/07/02 Use CENTER flag to center the vertical look.
      if (CheckKey(key_lookcenter))
        cmd->extbuttons |= EBT_CENTER;
    }
     //let movement keys cancel each other out
    if (strafe) 
    { 
      if (CheckKey(key_right))
        side += sidemove[speed];

      if (CheckKey(key_left))
        side -= sidemove[speed];

      // -KM- 1998/09/01 Analogue binding
      // -ACB- 1998/09/06 Side Move Speed Control
      i = G_DefineGetSpeedDivisor(sidemovespeed);

      Debug_Printf("Side: %d\n",i);

      if (i)
        side += (analogue[AXIS_TURN] * sidemove[speed]) / i;
      else
        side += (analogue[AXIS_TURN] * sidemove[speed]);
    }
    else 
    {
      if (CheckKey(key_180))
      {
        if (allow180)
          cmd->angleturn -= ANG180>>16;

        allow180=false;
      }
      else
      {
          allow180=true;
      }

      if (CheckKey(key_right))
        cmd->angleturn -= angleturn[tspeed];

      if (CheckKey(key_left))
        cmd->angleturn += angleturn[tspeed];

      // -KM- 1998/09/01 Analogue binding
      // -ACB- 1998/09/06 Angle Turn Speed Control
      i = G_DefineGetSpeedDivisor(angleturnspeed);

      if (i)
        cmd->angleturn -= (analogue[AXIS_TURN]*angleturn[tspeed]) / i;
      else
        cmd->angleturn -= (analogue[AXIS_TURN]*angleturn[tspeed]);
    } 
 
    // -MH- 1998/08/18 Fly up
    if (gameflags.true3dgameplay)
    {
      if ((CheckKey(key_flyup)))
        upward += upwardmove[speed];

      // -MH- 1998/08/18 Fly down
      if ((CheckKey(key_flydown)))
        upward -= upwardmove[speed];

      i = G_DefineGetSpeedDivisor(forwardmovespeed);
      upward += analogue[AXIS_FLY]*upwardmove[speed] / (i ? i : 1);
    }

    if (CheckKey(key_up))
      forward += forwardmove[speed]; 

    if (CheckKey(key_down))
      forward -= forwardmove[speed];

    // -KM- 1998/09/01 Analogue binding
    // -ACB- 1998/09/06 Forward Move Speed Control
    i = G_DefineGetSpeedDivisor(forwardmovespeed);

    if (i)
      forward -= (analogue[AXIS_FORWARD]*forwardmove[speed])/i;
    else
      forward -= (analogue[AXIS_FORWARD]*forwardmove[speed]);

    // -ACB- 1998/09/06 Side Move Speed Control
    i = G_DefineGetSpeedDivisor(sidemovespeed);
    if (i)
      side += (analogue[AXIS_STRAFE]*sidemove[speed])/i;
    else
      side += (analogue[AXIS_STRAFE]*sidemove[speed]);

    if (CheckKey(key_straferight))
	side += sidemove[speed]; 

    if (CheckKey(key_strafeleft))
	side -= sidemove[speed];
    
    // buttons
    cmd->chatchar = HU_dequeueChatChar(); 
 
    if (CheckKey(key_fire))
      cmd->buttons |= BT_ATTACK; 
 
    if (CheckKey(key_use))
      cmd->buttons |= BT_USE;

    if (CheckKey(key_jump))
      cmd->extbuttons |= EBT_JUMP;

    // -KM- 1998/11/25 Weapon change key
    for (i=0 ; i<10 ; i++)
    {
      if (CheckKey('0'+i))
      { 
        cmd->buttons |= BT_CHANGE; 
        cmd->buttons |= i<<BT_WEAPONSHIFT; 
        break; 
      }
    }
 
    // -KM- 1998/09/01 More analogue binding
    if (gameflags.freelook)
      vertangle += analogue[AXIS_MLOOK]*keylookspeed*4;

    // -MH- 1998/08/18 Yep. More flying controls...
    if (upward > MAXPLMOVE)
      upward = MAXPLMOVE;
    else if (upward < -MAXPLMOVE)
      upward = -MAXPLMOVE;

    if (forward > MAXPLMOVE) 
      forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE) 
      forward = -MAXPLMOVE;

    if (side > MAXPLMOVE) 
      side = MAXPLMOVE;
    else if (side < -MAXPLMOVE) 
      side = -MAXPLMOVE;

    cmd->upwardmove += upward;
    cmd->forwardmove += forward; 
    cmd->sidemove += side;

    if (vertangle)
    {
      cmd->extbuttons |= EBT_VERTLOOK;
      cmd->vertangle = (vertangle>>8);
    }
    
    // special buttons
    if (sendpause) 
    { 
      sendpause = false; 
      cmd->buttons = BT_SPECIAL | BTS_PAUSE; 
    } 
 
    if (sendsave) 
    { 
      sendsave = false;

      // 98-7-10 KM Increase savegame slots
      if (netgame)
        cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot<<BTS_SAVESHIFT);
      else
        gameaction = ga_savegame;
    }

    // -KM- 1998/09/01 Guess what ? Analogue binding
    memset(analogue, 0, sizeof(analogue));
} 
 

//
// G_DoLoadLevel 
//
extern gamestate_t wipegamestate; 
 
void G_DoLoadLevel (void) 
{ 
  int i;

  if (currentmap == NULL)
    I_Error("G_DoLoadLevel: No Current Map selected");

  // Set the sky map.
  //
  // First thing, we have a dummy sky texture name, a flat. The data is
  // in the WAD only because we look for an actual index, instead of simply
  // setting one.
  //
  // -ACB- 1998/08/09 Reference current map for sky name.
  //
  skyflatnum = R_FlatNumForName(SKYFLATNAME);
  skytexture = R_TextureNumForName(currentmap->sky);

  levelstarttic = gametic;        // for time calculation
    
  if (wipegamestate == GS_LEVEL) 
    wipegamestate = -1;             // force a wipe

  gamestate = GS_LEVEL; 

  for (i=0 ; i<MAXPLAYERS ; i++) 
  { 
    if (playeringame[i] && players[i].playerstate == PST_DEAD) 
      players[i].playerstate = PST_REBORN;

    memset(players[i].frags,0,sizeof(players[i].frags)); 
  } 

  // -KM- 1998/12/16 Make map flags actually do stuff.
  i = currentmap->flags;
  if (i & MPF_NOJUMPING)
    gameflags.jump = false;
  if (i & MPF_NOMLOOK)
    gameflags.freelook = false;
  if (i & MPF_ITEMRESPAWN)
    gameflags.itemrespawn = true;
  else if (i & MPF_NOITEMRESPN)
    gameflags.itemrespawn = false;
  if (i & MPF_FAST)
    gameflags.fastparm = true;
  if (i & MPF_RESMONSTER)
    gameflags.respawnsetting = RS_RESURRECT;
  else if (i & MPF_TELMONSTER)
    gameflags.respawnsetting = RS_TELEPORT;
  if (i & MPF_NOTRUE3D)
    gameflags.true3dgameplay = false;
  if (i & MPF_NORMBLOOD)
    gameflags.blood = false;
  if (i & MPF_NOCHEATS)
    gameflags.cheats = false;
  if (i & MPF_NOTRANSLUC)
    gameflags.trans = false;
  if (i & MPF_RESPAWN)
    gameflags.respawn = true;
  else if (i & MPF_NORESPAWN)
    gameflags.respawn = false;

  if (gameflags.blood)
    states[S_BLOOD3].tics = -1;
  else
    states[S_BLOOD3].tics = 8;

  //
  // Note: It should be noted that only the gameskill is
  // passed as the level is already defined in currentmap,
  // The method for changing currentmap, is using by
  // G_DeferedInitNew.
  //
  // -ACB- 1998/08/09 New P_SetupLevel
  // -KM- 1998/11/25 P_SetupLevel accepts the autotag
  //
  P_SetupLevel (gameskill, currentmap->autotag);

  // -KM- 1998/12/21 If a drone player, the display player is already
  //   set up.
  if (!gameflags.drone)
    displayplayer = consoleplayer;    // view the guy you are playing
  starttime = I_GetTime ();
  exittime = 0x7fffffff;
  gameaction = ga_nothing; 
  Z_CheckHeap ();
    
  // clear cmd building stuff
  memset (gamekeydown, 0, sizeof(gamekeydown));
  memset (analogue, 0, sizeof(analogue));
  sendpause = sendsave = paused = false; 
} 
 
 
//
// G_Responder  
// Get info needed to make ticcmd_ts for the players.
// 
boolean G_Responder (event_t* ev) 
{
    // 25-6-98 KM Allow spy mode for demos even in deathmatch
    if ((gamestate == GS_LEVEL) && (ev->type == ev_keydown)
        && (ev->data1 == KEYD_F12) && (demoplayback || !deathmatch) )
    {
	// spy mode 
	do 
	{ 
	    displayplayer++; 
	    if (displayplayer == MAXPLAYERS) 
		displayplayer = 0; 
	} while (!playeringame[displayplayer] && displayplayer != consoleplayer); 
	return true; 
    }
    
    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo && 
	(demoplayback || gamestate == GS_DEMOSCREEN) 
	) 
    { 
	if (ev->type == ev_keydown)
	{ 
	    M_StartControlPanel ();
            S_StartSound(NULL, sfx_swtchn);
	    return true; 
	} 
	return false; 
    } 
 
    if (gamestate == GS_LEVEL) 
    { 
	if (HU_Responder (ev)) 
	    return true;	// chat ate the event 
	if (ST_Responder (ev)) 
	    return true;	// status window ate it 
	if (AM_Responder (ev)) 
	    return true;	// automap ate it 
    } 
	 
    if (gamestate == GS_FINALE) 
    { 
	if (F_Responder (ev)) 
	    return true;	// finale ate the event 
    } 
	 
    switch (ev->type) 
    { 
      case ev_keydown: 
        if (ev->data1 == KEYD_PAUSE) 
	{ 
	    sendpause = true; 
	    return true; 
	} 
	if (ev->data1 <NUMKEYS) 
	    gamekeydown[ev->data1] = true;
	return true;    // eat key down events 
 
      case ev_keyup: 
	if (ev->data1 <NUMKEYS) 
	    gamekeydown[ev->data1] = false;
	return false;   // always let key up events filter down 
		 
      // -KM- 1998/09/01 Change mouse/joystick to analogue
      case ev_analogue:
        analogue[ev->data1] += ev->data2;
        analogue[ev->data3] += ev->data4;
	return true;    // eat events 
 
      default: 
	break; 
    } 
 
    return false; 
} 
 
 
 
//
// G_Ticker
// Make ticcmd_ts for the players.
//
// -ACB- 1998/08/10 Use DDF_LanguageLookup() for language specifics.
//
void G_Ticker (void) 
{ 
    int		i;
    int		buf; 
    ticcmd_t*	cmd;
    
    // do player reborns if needed
    for (i=0 ; i<MAXPLAYERS ; i++) 
	if (playeringame[i] && players[i].playerstate == PST_REBORN) 
	    G_DoReborn (i);

    if (exittime == leveltime)
    {
      gameaction = ga_completed;
      exittime = -1;
    }

    // do things to change the game state
    while (gameaction != ga_nothing) 
    { 
	switch (gameaction) 
	{ 
	  case ga_loadlevel: 
	    G_DoLoadLevel (); 
	    break; 
	  case ga_newgame: 
	    RAD_ResetRadiTriggers();
	    G_DoNewGame (); 
	    break; 
	  case ga_loadgame: 
	    RAD_ResetRadiTriggers();
	    G_DoLoadGame (); 
	    break; 
	  case ga_savegame: 
	    G_DoSaveGame (); 
	    break; 
	  case ga_playdemo: 
	    G_DoPlayDemo (); 
	    break; 
	  case ga_completed: 
	    G_DoCompleted (); 
	    break; 
	  case ga_victory: 
	    F_StartFinale (&currentmap->f[0], ga_nothing);
	    break; 
	  case ga_briefing:
	    F_StartFinale (&currentmap->nextlevel->f[1], ga_worlddone);
	    break; 
	  case ga_worlddone: 
	    RAD_ClearRadiTriggersTimers();
	    G_DoWorldDone (); 
	    break; 
	  case ga_screenshot: 
	    M_ScreenShot (); 
	    gameaction = ga_nothing; 
	    break; 
	  case ga_nothing: 
	    break; 
	} 
    }
    
    // get commands, check consistancy,
    // and build new consistancy check
    buf = (gametic/ticdup)%BACKUPTICS; 
 
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]) 
	{ 
	    cmd = &players[i].cmd; 
 
	    memcpy (cmd, &netcmds[i][buf], sizeof(ticcmd_t)); 
 
	    if (demoplayback) 
		G_ReadDemoTiccmd (cmd); 
	    if (demorecording) 
		G_WriteDemoTiccmd (cmd);
	    
	    // check for turbo cheats
	    if (cmd->forwardmove > TURBOTHRESHOLD 
		&& !(gametic&31) && ((gametic>>5)&3) == i )
	    {
		static char turbomessage[80];
		extern char *player_names[4];
		sprintf (turbomessage,DDF_LanguageLookup("IsTurbo"),player_names[i]);
		players[consoleplayer].message = turbomessage;
	    }
			
	    if (netgame && !netdemo && !(gametic%ticdup) ) 
	    { 
		if (gametic > BACKUPTICS
		    && consistancy[i][buf] != cmd->consistancy) 
		{ 
		    I_Error ("consistency failure (%i should be %i)",
			     cmd->consistancy, consistancy[i][buf]); 
		}
		if (players[i].mo) 
		    consistancy[i][buf] = players[i].mo->x; 
		else 
		    consistancy[i][buf] = rndindex; 
	    } 
	}
    }
    // check for special buttons
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]) 
	{ 
	    if (players[i].cmd.buttons & BT_SPECIAL) 
	    { 
		switch (players[i].cmd.buttons & BT_SPECIALMASK) 
		{ 
		  case BTS_PAUSE:
		    paused ^= 1; 
		    if (paused)
			S_PauseSound ();
		    else
			S_ResumeSound ();
		    break; 
					 
		  case BTS_SAVEGAME: 
		    if (!savedescription[0]) 
			strcpy (savedescription, "NET GAME"); 
		    savegameslot =  
			(players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT; 
		    gameaction = ga_savegame; 
		    break; 
		} 
	    } 
	}
    }
    
    // do main actions
    switch (gamestate) 
    { 
      case GS_LEVEL: 
	P_Ticker (); 
	ST_Ticker (); 
	AM_Ticker (); 
	HU_Ticker ();            
	break; 
	 
      case GS_INTERMISSION: 
	WI_Ticker (); 
	break; 
			 
      case GS_FINALE: 
	F_Ticker (); 
	break; 
 
      case GS_DEMOSCREEN: 
	D_PageTicker (); 
	break; 
    }        
} 
 
 
//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer 
// Called at the start.
// Called by the game initialization functions.
//
void G_InitPlayer (int player) 
{ 
    player_t*	p; 
 
    // set up the saved info         
    p = &players[player]; 
	 
    // clear everything else to defaults 
    G_PlayerReborn (player); 
	 
} 
 
 

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//
void G_PlayerFinishLevel (int player) 
{ 
    player_t*	p; 
	 
    p = &players[player]; 
	 
    memset (p->powers, 0, sizeof (p->powers)); 
    memset (p->cards, 0, sizeof (p->cards)); 
    p->mo->flags &= ~MF_SHADOW;		// cancel invisibility 
    p->extralight = 0;			// cancel gun flashes 
    p->fixedcolormap = 0;		// cancel ir gogles 
    p->damagecount = 0;			// no palette changes 
    p->bonuscount = 0; 
} 
 

//
// G_PlayerReborn
// Called after a player dies 
// almost everything is cleared and initialized 
//
void G_PlayerReborn (int player) 
{ 
    player_t*	p;
    boolean* w;
    int		i; 
    int		frags[MAXPLAYERS]; 
    int		killcount;
    int		itemcount;
    int		secretcount;
    int*        ammo, *ammolimit;

    // -ACB- 1998/07/20 Force update of the weapons stuff.
    weaponupdate = true;
	 
    memcpy (frags,players[player].frags,sizeof(frags)); 
    killcount = players[player].killcount; 
    itemcount = players[player].itemcount; 
    secretcount = players[player].secretcount; 
	 
    p = &players[player];
    // -KM- 1998/11/25 Save the weapon memory
    w = p->weaponowned;
    ammo = p->ammo;
    ammolimit = p->maxammo;
    memset (p, 0, sizeof(*p));
    for (i = 0; i < numweapons; i++)
       w[i] = weaponinfo[i].autogive;
    p->weaponowned = w;
    p->ammo = ammo;
    p->maxammo = ammolimit;

    memset(ammo, 0, NUMAMMO*sizeof(int));
 
    memcpy (players[player].frags, frags, sizeof(players[player].frags)); 
    players[player].killcount = killcount; 
    players[player].itemcount = itemcount; 
    players[player].secretcount = secretcount; 
 
    p->usedown = p->attackdown = true;	// don't do anything immediately 
    p->playerstate = PST_LIVE;       
    p->health = NORMHEALTH;
    p->readyweapon = p->pendingweapon = DDF_WeaponGetType("PISTOL");
    p->ammo[am_clip] = NORMAMMO;
	 
    for (i=0 ; i<4 ; i++)
	p->maxammo[i] = maxammo[i];
    for (i=4; i<NUMAMMO; i++)
        p->maxammo[i] = 0;
		 
}

//
// G_CheckSpot  
// Returns false if the player cannot be respawned
// at the given mapthing_t spot  
// because something is occupying it 
//
boolean G_CheckSpot ( int playernum, mapthing_t* mthing )
{ 
    fixed_t		x;
    fixed_t		y; 
    subsector_t*	ss; 
    unsigned		an; 
    mobj_t*		mo; 
    int			i;
	
    if (!players[playernum].mo)
    {
	// first spawn of level, before corpses
	for (i=0 ; i<playernum ; i++)
	    if (players[i].mo->x == mthing->x << FRACBITS
		&& players[i].mo->y == mthing->y << FRACBITS)
		return false;	
	return true;
    }
		
    x = mthing->x << FRACBITS; 
    y = mthing->y << FRACBITS; 
	 
    if (!P_CheckPosition (players[playernum].mo, x, y) ) 
	return false; 
 
    // flush an old corpse if needed 
    if (bodyqueslot >= BODYQUESIZE) 
	P_RemoveMobj (bodyque[bodyqueslot%BODYQUESIZE]); 
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo; 
    bodyqueslot++; 
	
    // spawn a teleport fog 
    ss = R_PointInSubsector (x,y); 
    an = ( ANG45 * (mthing->angle/45) ) >> ANGLETOFINESHIFT; 

    // temp fix for teleport effect
    mo = P_MobjCreateObject(x+20*finecosine[an], y+20*finesine[an],
                            ss->sector->floorheight,
                            DDF_MobjLookup("TELEPORT FLASH"));

    return true; 
} 


//
// G_DeathMatchSpawnPlayer 
// Spawns a player at one of the random death match spots 
// called at level load and each death 
//
void G_DeathMatchSpawnPlayer (int playernum)
{
    int             i,j;
    int                         selections;

    selections = deathmatch_p - deathmatchstarts;
    if (selections < 4)
        I_Error ("Only %i deathmatch spots, 4 required", selections);

    for (j=0 ; j<20 ; j++)
    {
        i = P_Random() % selections;
        if (G_CheckSpot (playernum, &deathmatchstarts[i]) )
        {

//-CTF(JC)------------------------------------------------------------------
        if (playernum<4)
           deathmatchstarts[i].type = playernum+1;
        else
           deathmatchstarts[i].type = (4001+playernum)-4;
//--------------------------------------------------------------------------

            P_SpawnPlayer (&deathmatchstarts[i]);
            return;
        }
    }

    // no good spot, so the player will probably get stuck
    P_SpawnPlayer (&playerstarts[playernum]);
}

//
// G_DoReborn 
// 
void G_DoReborn (int playernum) 
{ 
    int i;
    int oldplayerstart;
	 
    if (!netgame)
    {
	// reload the level from scratch
	RAD_ResetRadiTriggers();
        weaponupdate = true; // -ACB- 1998/07/13 Update weapon widgets
	gameaction = ga_loadlevel;  
    }
    else
    {
	// respawn at the start

	// first dissasociate the corpse 
	players[playernum].mo->player = NULL;   
		 
	// spawn at random spot if in death match 
	if (deathmatch) 
	{ 
	    G_DeathMatchSpawnPlayer (playernum); 
	    return; 
	} 
		 
	if (G_CheckSpot (playernum, &playerstarts[playernum]) ) 
	{ 
	    P_SpawnPlayer (&playerstarts[playernum]); 
	    return; 
	}
	
        // try to spawn at one of the other players spots
        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (G_CheckSpot (playernum, &playerstarts[i]) )
            {
               oldplayerstart=playerstarts[i].type;
               if (playernum<4)
                  playerstarts[i].type = playernum+1; // fake as other player
               else
                 playerstarts[i].type = (4001+playernum)-4; // fake as other player

                P_SpawnPlayer (&playerstarts[i]);
                playerstarts[i].type = oldplayerstart;          // restore
                return;
            }
            // he's going to be inside something.  Too bad.
        }
        P_SpawnPlayer (&playerstarts[playernum]);
    }
} 
 
 
void G_ScreenShot (void) 
{ 
    gameaction = ga_screenshot; 
} 
 


// DOOM Par Times
int pars[4][10] = 
{ 
    {0}, 
    {0,30,75,120,90,165,180,180,30,165}, 
    {0,90,90,90,120,90,360,240,30,170}, 
    {0,90,45,90,150,90,90,165,30,135} 
}; 
/*
// -KM- 1998/11/25 Pars are external
// DOOM II Par Times
int cpars[32] =
{
    30,90,120,120,90,150,120,120,270,90,	//  1-10
    210,150,150,150,210,150,420,150,210,150,	// 11-20
    240,150,180,150,150,300,330,420,300,180,	// 21-30
    120,30					// 31-32
};
*/ 

//
// G_DoCompleted 
//
boolean		secretexit; 
extern char*	pagename; 
 
// -KM- 1998/11/25 Added time param which is the time to wait before
//  actually exiting level.
void G_ExitLevel (int time)
{ 
    currentmap->nextlevel = DDF_LevelGetNewMap(currentmap->nextmapname);
    exittime = leveltime + time;
} 

// -ACB- 1998/08/08 We don't have support for the german edition
//                  removed the check for map31.
void G_SecretExitLevel (int time)
{ 
    currentmap->nextlevel = DDF_LevelGetNewMap(currentmap->secretmapname);
    exittime = leveltime + time;
} 

void G_ExitToLevel(char* name, int time)
{
    currentmap->nextlevel = DDF_LevelGetNewMap(name);
    exittime = leveltime + time;
}

void G_DoCompleted (void) 
{ 
    int             i; 
	 
    gameaction = ga_nothing; 
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
	if (playeringame[i]) 
	    G_PlayerFinishLevel (i);        // take away cards and stuff 
	 
    if (automapactive) 
	AM_Stop (); 
			
    // -KM- 1998/12/16 New wminfo struct.
    wminfo.didsecret = players[consoleplayer].didsecret;

    wminfo.level = currentmap->name;
    wminfo.last = currentmap;
    wminfo.next = currentmap->nextlevel;
    wminfo.maxkills = totalkills; 
    wminfo.maxitems = totalitems; 
    wminfo.maxsecret = totalsecret; 
    wminfo.maxfrags = 0; 
    wminfo.partime = currentmap->partime;
    wminfo.pnum = consoleplayer; 
 
    for (i=0 ; i<MAXPLAYERS ; i++) 
    { 
	wminfo.plyr[i].in = playeringame[i]; 
	wminfo.plyr[i].skills = players[i].killcount; 
	wminfo.plyr[i].sitems = players[i].itemcount; 
	wminfo.plyr[i].ssecret = players[i].secretcount; 
	wminfo.plyr[i].stime = leveltime; 
	memcpy (wminfo.plyr[i].frags, players[i].frags, sizeof(wminfo.plyr[i].frags));
    } 
 
    gamestate = GS_INTERMISSION; 
    viewactive = false; 
    automapactive = false; 
 
    if (statcopy)
	memcpy (statcopy, &wminfo, sizeof(wminfo));
	
    WI_Start (&wminfo); 
} 


//
// G_WorldDone 
//
void G_WorldDone (void) 
{ 
    if (secretexit) 
	players[consoleplayer].didsecret = true;

    F_StartFinale(&currentmap->f[0], currentmap->nextlevel?ga_briefing:ga_nothing);

    /* if ( gamemission != doom )
    {
	switch (gamemap)
	{
	  case 15:
	  case 31:
	    if (!secretexit)
		break;
	  case 6:
	  case 11:
	  case 20:
	  case 30:
	    F_StartFinale ();
	    break;
	}
    }  */
} 
 
void G_DoWorldDone (void) 
{        
    gamestate = GS_LEVEL;

    currentmap = currentmap->nextlevel;

    G_DoLoadLevel (); 
    gameaction = ga_nothing; 
    viewactive = true;
    secretexit = false;
} 
 


//
// G_InitFromSavegame
// Can be called by the startup code or the menu task. 
//
extern boolean setsizeneeded;
void R_ExecuteSetViewSize (void);

char	savename[256];

void G_LoadGame (char* name) 
{ 
    strcpy (savename, name); 
    gameaction = ga_loadgame; 
} 

#define VERSIONSIZE 16
void G_DoLoadGame (void) 
{ 
    int length;
    int lumplen;
    int i;
    int a,b,c;
    char vcheck[VERSIONSIZE];
    char *mapname;
    mapstuff_t* tempmap;
	 
    gameaction = ga_nothing; 
	 
    length = M_ReadFile (savename, &savebuffer); 
    save_p = savebuffer + SAVESTRINGSIZE;
    
    // skip the description field 
    memset (vcheck,0,sizeof(vcheck));

    sprintf (vcheck,"version %i",VERSION);

    if (strcmp (save_p, vcheck))
      I_Error("Bad Save Game Version: Read: %s", vcheck); // bad version

    save_p += VERSIONSIZE;

    // ---------------------------------------------------------------
    // Save the name of the map within the save game -ACB- 1998/08/29
    lumplen = *save_p++;

    mapname = Z_Malloc(sizeof(char)*(lumplen+1), PU_STATIC, NULL);
    memset (mapname,'\0',lumplen+1);
    memcpy (mapname, save_p, lumplen*sizeof(char));

    tempmap = DDF_LevelGetNewMap(mapname);

    if (!tempmap)
      I_Error("Bad Save Game Mapname: %s, %d",mapname,lumplen); // bad map name

    Z_Free (mapname);

    save_p += lumplen*sizeof(char);
    // --------------------------------------------------------------

#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

    gameskill = *save_p++;

    for (i=0 ; i<MAXPLAYERS ; i++) 
	playeringame[i] = *save_p++; 

    // load a base level
    G_InitNew (gameskill, tempmap);
 
    // get the times 
    a = *save_p++; 
    b = *save_p++; 
    c = *save_p++; 
    leveltime = (a<<16) + (b<<8) + c; 
	 
    // dearchive all the modifications
    P_UnArchivePlayers (); 
    P_UnArchiveWorld (); 
    P_UnArchiveThinkers (); 
    P_UnArchiveItemRespawnQue();
    P_UnArchiveSpecials (); 
 
    if (*save_p != 0x1d)
      I_Error ("Bad savegame");
    
    // done 
    Z_Free (savebuffer); 
 
    if (setsizeneeded)
      R_ExecuteSetViewSize ();
    
    // draw the pattern into the back screen
    R_FillBackScreen ();   
} 
 

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string 
//
void G_SaveGame (int slot, char* description)
{ 
    savegameslot = slot; 
    strcpy (savedescription, description); 
    sendsave = true; 
} 
 
void G_DoSaveGame (void) 
{
    char name[100];
    char name2[VERSIONSIZE];
    char* description;
    int length;
    int lumplen;
    int i;
	
    // 98-7-10 KM Increase savegame slots
    if (netgame)
        sprintf (name,NETSAVEDIR"/"SAVEGAMENAME"%04x.dsg",savegameslot);
    else
        sprintf (name,SAVEGAMEDIR"/"SAVEGAMENAME"%04x.dsg",savegameslot);

    description = savedescription;
    save_p = savebuffer = Z_Malloc(SAVEGAMESIZE, PU_STATIC, NULL);
    savegame_size = SAVEGAMESIZE;
	 
    if ((save_p - savebuffer) > (savegame_size + SAVESTRINGSIZE + VERSIONSIZE + 16))
    {
      savegame_size += SAVESTRINGSIZE + VERSIONSIZE + 16;
      length = save_p - savebuffer;
      savebuffer = Z_ReMalloc(savebuffer, savegame_size);
      save_p = savebuffer + length;
    }

    memcpy (save_p, description, SAVESTRINGSIZE);
    save_p += SAVESTRINGSIZE; 
    memset (name2,0,sizeof(name2)); 
    sprintf (name2,"version %i",VERSION); 
    memcpy (save_p, name2, VERSIONSIZE); 
    save_p += VERSIONSIZE; 

    // ---------------------------------------------------------------
    // Save the name of the map within the save game -ACB- 1998/08/29
    // (and remember to put the lumplen in! -ACB- 1998/09/14)
    lumplen = strlen(currentmap->name);

    *save_p++ = lumplen;

    memcpy (save_p, currentmap->name, lumplen*sizeof(char));
    save_p += lumplen*sizeof(char);
    // --------------------------------------------------------------
#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

    *save_p++ = gameskill;

    for (i=0 ; i<MAXPLAYERS ; i++) 
      *save_p++ = (byte) playeringame[i];

    *save_p++ = leveltime>>16; 
    *save_p++ = leveltime>>8; 
    *save_p++ = leveltime; 
 
    P_ArchivePlayers (); 
    P_ArchiveWorld (); 
    P_ArchiveThinkers (); 
    P_ArchiveItemRespawnQue();
    P_ArchiveSpecials (); 
	 
    *save_p++ = 0x1d;		// consistancy marker 
	 
    length = save_p - savebuffer;

    if (length > savegame_size)
      I_Error ("Savegame buffer overrun");

    M_WriteFile (name, savebuffer, length); 
    gameaction = ga_nothing; 
    savedescription[0] = 0;		 
	 
    players[consoleplayer].message = DDF_LanguageLookup("GameSaved");

    Z_Free(savebuffer);
    // draw the pattern into the back screen
    R_FillBackScreen ();	

} 
 
//
// G_InitNew
//
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should
// be set.
//
skill_t	d_skill;
mapstuff_t* d_newmap = NULL;
 
//
// G_DeferedInitNew
//
// This is the procedure that changes the currentmap
// at the start of the game and outside the normal
// progression of the game. All thats needed is the
// skill and the name (The name in the DDF File itself).
//
// 1998/08/09
//
boolean G_DeferedInitNew (skill_t skill, const char *mapname)
{
  d_newmap = DDF_LevelGetNewMap(mapname);

  if (!d_newmap)
    return true;

  d_skill = skill;

  gameaction = ga_newgame;
  return false;
} 

void G_DoNewGame (void) 
{
  int i;
  extern int quickSaveSlot;

  demoplayback = false;
  netdemo = false;
  netgame = false;
  deathmatch = false;
  quickSaveSlot = -1;

  for (i=1;i<MAXPLAYERS;i++) playeringame[i]=false;

  gameflags.fastparm = false;
  gameflags.nomonsters = false;
  consoleplayer = 0;
  G_InitNew (d_skill, d_newmap);
  gameaction = ga_nothing;
} 

// The sky texture to be used instead of the F_SKY1 dummy.
extern  int	skytexture;

//
// G_InitNew
//
// -ACB- 1998/07/12 Removed Lost Soul/Spectre Ability stuff
// -ACB- 1998/08/10 Inits new game without the need for gamemap or episode.
// -ACB- 1998/09/06 Removed remarked code.
// -KM- 1998/12/21 Added mapstuff param so no need for defered init new
//   which was conflicting with net games.
//
void G_InitNew (skill_t skill, mapstuff_t* map)
{ 
  int i;

  if (paused)
  { 
    paused = false; 
    S_ResumeSound (); 
  } 
	
  currentmap = map;

  if (skill > sk_nightmare)
    skill = sk_nightmare;

  // This was quite messy with SPECIAL and commented parts.
  // Supposedly hacks to make the latest edition work.                 
  M_ClearRandom (); 
	 	 			 
  // force players to be initialized upon first level load         
  for (i=0 ; i<MAXPLAYERS ; i++) 
     players[i].playerstate = PST_REBORN; 
 
  usergame = true;                // will be set false if a demo 
  paused = false; 
  demoplayback = false; 
  automapactive = false; 
  viewactive = true; 
  gameskill = skill; 
 
  viewactive = true;

  memcpy(&gameflags, &settingflags, sizeof(gameflags_t));
  if (skill == sk_nightmare)
  {
     gameflags.fastparm = true;
     gameflags.respawn = true;
#ifdef NO_NIGHTMARE_CHEATS
     gameflags.cheats = false;
#endif
  }

  G_DoLoadLevel (); 
} 
 
//
// DEMO RECORDING 
// 
#define DEMOMARKER		0x80

//
// G_ReadDemoTiccmd
//
// A demo file is essentially a stream of ticcmds: every tic,
// the ticcmd holds all the info for movement for a player on
// that tic. This means that a demo merely replays the movements
// and actions of the player.
//
// This function gets the actions from the demobuffer and gives
// them to ticcmd to be played out. Its worth a note that this
// is the reason demos desync when played on two different
// versions, since any alteration to the gameplay could give
// a different reaction to a player action and therefore the
// game is different to the original.
//  
void G_ReadDemoTiccmd(ticcmd_t* cmd) 
{
  // 98-7-10 KM Demolimit removed
  if (demobuffer[demo_p] == DEMOMARKER)
  {
    // end of demo data stream
    G_CheckDemoStatus ();
    return;
  }

  // -ACB- 1998/07/11 Added additional ticcmd stuff to demo
  // -MH-  1998/08/18 Added same for fly up/down
  //                  Keep all upward stuff before all forward stuff, to
  //                  keep consistent. Will break existing demos. Damn.
/*  cmd->vertangle   = (signed char)demobuffer[demo_p++];
  cmd->upwardmove  = (signed char)demobuffer[demo_p++];
  cmd->forwardmove = (signed char)demobuffer[demo_p++];
  cmd->sidemove    = (signed char)demobuffer[demo_p++];
  cmd->angleturn   = (unsigned char)demobuffer[demo_p++]<<8;
  cmd->buttons     = (unsigned char)demobuffer[demo_p++];
  cmd->extbuttons  = (unsigned char)demobuffer[demo_p++];*/
  memcpy(cmd, demobuffer + demo_p, sizeof(*cmd));
  demo_p += sizeof(*cmd);
} 

//
// G_WriteDemoTiccmd
//
// A demo file is essentially a stream of ticcmds: every tic,
// the ticcmd holds all the info for movement for a player on
// that tic. This means that a demo merely replays the movements
// and actions of the player.
//
// This function writes the ticcmd to the demobuffer and
// then get G_ReadDemoTiccmd to read it, so that whatever is
// recorded is played out. 
//
void G_WriteDemoTiccmd (ticcmd_t* cmd) 
{ 
  if (gamekeydown['q'])           // press q to end demo recording 
    G_CheckDemoStatus (); 

  // 98-7-10 KM Demolimit removed
  if (demo_p > maxdemo - 16)
  {
    // no more space
    maxdemo += 0x1000;
    demobuffer = Z_ReMalloc(demobuffer, maxdemo);
  }

  // -ACB- 1998/07/11 Added additional ticcmd stuff to demo
  // -MH-  1998/08/18 Added same for fly up/down
  //                  Keep all upward stuff before all forward stuff, to
  //                  keep consistent. Will break existing demos. Damn.
  memcpy(demobuffer + demo_p, cmd, sizeof(*cmd));
/*  demobuffer[demo_p++] = cmd->vertangle;
  demobuffer[demo_p++] = cmd->upwardmove;
  demobuffer[demo_p++] = cmd->forwardmove; 
  demobuffer[demo_p++] = cmd->sidemove;
  demobuffer[demo_p++] =(cmd->angleturn+128)>>8; 
  demobuffer[demo_p++] = cmd->buttons; 
  demobuffer[demo_p++] = cmd->extbuttons;

  demo_p -= 7;                           */

  G_ReadDemoTiccmd (cmd);         // make SURE it is exactly the same
} 

//
// G_RecordDemo 
// 
// 98-7-10 KM Demolimit removed
//
void G_RecordDemo (char* name) 
{ 
    usergame = false; 
    strcpy (demoname, name); 
    strcat (demoname, ".lmp"); 
    maxdemo = 0x20000;
    if (!demobuffer)
      demobuffer = Z_Malloc (maxdemo, PU_STATIC, NULL);
    else
      demobuffer = Z_ReMalloc (demobuffer, maxdemo);
	
    demorecording = true; 
} 
 
//
// G_BeginRecording
//
// -ACB- 1998/07/02 Changed the code to record as version 0.65 (065),
//                  All of the additional DOSDoom features are stored in
//                  the demo.
//
// -KM-  1998/07/10 Removed the demo limit.
//
// -ACB- 1998/07/12 Removed Lost Soul/Spectre Ability Check
//
void G_BeginRecording (void)
{
  int i,j;
  char *tempstring;

  demo_p = 0;

  i = strlen(currentmap->name);

  demobuffer[demo_p++] = DEMOVERSION;

  if (gameskill == -1)
    gameskill = startskill;

  //---------------------------------------------------------
  // -ACB- 1998/09/03 Record Level Name In Demo
  demobuffer[demo_p++] = i;

  tempstring = currentmap->name;

  for (j=0; j<i; j++)
  {
    demobuffer[demo_p+j] = tempstring[j];
    Debug_Printf("%c\n",demobuffer[demo_p+j]);
  }

  demo_p += i;
  //---------------------------------------------------------

  demobuffer[demo_p++] = gameskill;
  demobuffer[demo_p++] = deathmatch;
//  demobuffer[demo_p++] = fastparm;
//  demobuffer[demo_p++] = nomonsters;
  demobuffer[demo_p++] = consoleplayer;
  //demobuffer[demo_p++] = lessaccuratezom;
  //demobuffer[demo_p++] = lessaccuratemon;
//  demobuffer[demo_p++] = grav;
//  demobuffer[demo_p++] = true3dgameplay;
  //demobuffer[demo_p++] = missileteleport;
  //demobuffer[demo_p++] = crosshair;
  //demobuffer[demo_p++] = stretchsky;
  //demobuffer[demo_p++] = rotatemap;
  //demobuffer[demo_p++] = newhud;
//  demobuffer[demo_p++] = respawnsetting;
//  demobuffer[demo_p++] = itemrespawn;
  //demobuffer[demo_p++] = infight;
//  demobuffer[demo_p++] = invertmouse;
//  demobuffer[demo_p++] = keylookspeed;
//  demobuffer[demo_p++] = transluc;

  memcpy(&demobuffer[demo_p], &gameflags, sizeof(gameflags_t));
  demo_p += sizeof(gameflags_t);

  for (i=0 ; i<MAXPLAYERS ; i++)
    demobuffer[demo_p++] = playeringame[i];

} 
 
//
// G_PlayDemo 
//
char*	defdemoname; 
 
void G_DeferedPlayDemo (char* name) 
{ 
    defdemoname = name; 
    gameaction = ga_playdemo; 
} 

//
// G_DoPlayDemo
// Sets up the system to play a demo.
//
// -ACB- 1998/07/02 Change the code only to play version 0.65 demos.
// -KM-  1998/07/10 Displayed error message on screen and make demos limitless
// -ACB- 1998/07/12 Removed Lost Soul/Spectre Ability Check
// -ACB- 1998/07/12 Removed error message (became bloody annoying...)
//
void G_DoPlayDemo (void) 
{
  skill_t skill;
  int i,j;
  int demversion;
  char mapname[30];
  mapstuff_t* newmap;

  gameaction = ga_nothing;
  demobuffer = W_CacheLumpName (defdemoname, PU_STATIC);
  demo_p = 0;
  demversion= demobuffer[demo_p++];

  if (demversion != DEMOVERSION)
  {
    gameaction = ga_nothing;
    return;
  }
  else
  {
    //------------------------------------------------------
    // -ACB- 1998/09/03 Read the Level Name from the demo.
    i = demobuffer[demo_p++];
    memset(mapname,'\0',sizeof(char)*(i+1));

    for (j=0; j<i; j++)
      mapname[j] = demobuffer[demo_p+j];

    demo_p+= i;
    //------------------------------------------------------

    skill           = demobuffer[demo_p++];
    deathmatch      = demobuffer[demo_p++];
//    fastparm        = demobuffer[demo_p++];
//    nomonsters      = demobuffer[demo_p++];
    consoleplayer   = demobuffer[demo_p++];
    //lessaccuratezom = demobuffer[demo_p++];
    //lessaccuratemon = demobuffer[demo_p++];
//    grav            = demobuffer[demo_p++];
//    true3dgameplay  = demobuffer[demo_p++];
    //missileteleport = demobuffer[demo_p++];
    //crosshair       = demobuffer[demo_p++];
    //stretchsky      = demobuffer[demo_p++];
    //rotatemap       = demobuffer[demo_p++];
    //newhud          = demobuffer[demo_p++];
//    respawnsetting  = demobuffer[demo_p++];
//    itemrespawn     = demobuffer[demo_p++];
    //infight         = demobuffer[demo_p++];
//    mlookon         = demobuffer[demo_p++];
//    invertmouse     = demobuffer[demo_p++];
//    keylookspeed    = demobuffer[demo_p++];
//    transluc        = demobuffer[demo_p++];

    memcpy(&gameflags, &demobuffer[demo_p], sizeof(gameflags_t));
    demo_p += sizeof(gameflags_t);

    for (i=0 ; i<MAXPLAYERS ; i++)
      playeringame[i] = demobuffer[demo_p++];
  }

  //----------------------------------------------------------------
  // -ACB- 1998/09/03 Setup the given mapname; fail if map does not
  // exist.
  newmap = DDF_LevelGetNewMap(mapname);

  if (newmap==NULL)
  {
    gameaction = ga_nothing;
    return;
  }

  //----------------------------------------------------------------

  if (playeringame[1])
  {
    netgame = true;
    netdemo = true;
  }

  // don't spend a lot of time in loadlevel
  precache = false;
  G_InitNew (skill, newmap);
  precache = true;
  usergame = false;
  demoplayback = true;
} 

//
// G_TimeDemo 
//
void G_TimeDemo (char* name) 
{ 	 
    nodrawers = M_CheckParm ("-nodraw"); 
    noblit = M_CheckParm ("-noblit"); 
    timingdemo = true; 
    singletics = true; 

    defdemoname = name; 
    gameaction = ga_playdemo; 
} 
 
 
// 
// G_CheckDemoStatus 
//
//Called after a death or level completion to allow demos to be cleaned up, 
//Returns true if a new demo loop action will take place 
// 
//-KM- 1998/07/10 Reformed code for limitless demo
//
boolean G_CheckDemoStatus (void)
{
    int             endtime;
    int             i;
	 
    if (timingdemo) 
    { 
      float fps;

      endtime = I_GetTime ();
      fps=((float)(gametic*TICRATE))/(endtime-starttime);
      I_Error ("timed %i gametics in %i realtics, which equals %f fps",gametic,
		  endtime-starttime,fps);
    } 
	 
    if (demoplayback) 
    { 
	if (singledemo) 
	    I_Quit (); 
			 
	Z_ChangeTag (demobuffer, PU_CACHE); 
	demoplayback = false; 
	netdemo = false;
	netgame = false;
	deathmatch = false;

        for (i=1;i<MAXPLAYERS;i++) playeringame[i]=0;

	gameflags.fastparm = false;
	gameflags.nomonsters = false;
	consoleplayer = 0;
	D_AdvanceDemo (); 
	return true; 
    } 
 
    if (demorecording) 
    { 
	demobuffer[demo_p++] = DEMOMARKER; 
	M_WriteFile (demoname, demobuffer, demo_p);
	Z_Free (demobuffer); 
	demorecording = false; 
	I_Error ("Demo %s recorded",demoname); 
    } 
	 
    return false; 
} 

 
 
