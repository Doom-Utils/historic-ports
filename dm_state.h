//  
// DOSDoom Global State Variables
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -MH- 1998/07/02 "lookupdown" --> "true3dgameplay"
//

#ifndef __D_STATE__
#define __D_STATE__

// We need globally shared data structures,
//  for defining the global state variables.
#include "dm_data.h"
#include "d_net.h"

// We need the playr data structure as well.
#include "d_player.h"


#ifdef __GNUG__
#pragma interface
#endif

#ifndef DJGPP
//extern short palette_color[256];
//extern int key_shifts;
//extern int KB_CAPSLOCK_FLAG;
#endif


// ------------------------
// Command line parameters.
//
//extern  boolean	nomonsters;	// checkparm of -nomonsters
//extern  boolean	fastparm;	// checkparm of -fast

//extern  int respawnsetting;

extern  boolean	devparm;	// DEBUG: launched with -devparm
extern	boolean redrawsbar;

extern gameflags_t gameflags, settingflags;

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//

// Set if homebrew PWAD stuff has been added.
extern  boolean	modifiedgame;


// -------------------------------------------
// Language.
//extern  Language_t   language;


// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern  skill_t		startskill;
extern  char*           startmap;
extern  boolean         drone;

extern  boolean		autostart;

// Selected by user. 
extern  skill_t         gameskill;

// Netgame? Only true if >1 player.
extern  boolean	netgame;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern int deathmatch;
	
// -------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// These are not used, but should be (menu).
// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int snd_SfxVolume;      // maximum volume for sound
extern int snd_MusicVolume;    // maximum volume for music
extern int snd_CDMusicVolume; // max vol for cd-audio

// Current music/sfx card - index useless
//  w/o a reference LUT in a sound module.
// Ideally, this would use indices found
//  in: /usr/include/linux/soundcard.h
extern int snd_MusicDevice;
extern int snd_SfxDevice;
// Config file? Same disclaimer as above.
extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;


// -------------------------
// Status flags for refresh.
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern  boolean statusbaractive;

extern  boolean automapactive;	// In AutoMap mode?
extern  boolean	menuactive;	// Menu overlayed?
extern  boolean	paused;		// Game Pause?
extern  boolean		viewactive;

extern  boolean		nodrawers;
extern  boolean		noblit;

extern	int		viewwindowx;
extern	int		viewwindowy;
extern	int		viewheight;
extern	int		viewwidth;
extern	int		scaledviewwidth;

// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  angle_t	viewangleoffset;

// Player taking events, and displaying.
extern  int	consoleplayer;	
extern  int	displayplayer;
extern  int     maxplayers;


// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern  int	totalkills;
extern	int	totalitems;
extern	int	totalsecret;

// Timer, for scores.
extern  int	levelstarttic;	// gametic at level start
extern  int	leveltime;	// tics in game play for par

// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern  boolean	usergame;

//?
extern  boolean	demoplayback;
extern  boolean	demorecording;

// Quit after playing a demo from cmdline.
extern  boolean		singledemo;	

//?
extern  gamestate_t     gamestate;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern	int		gametic;

extern char**	player_names;
// Bookkeeping on players - state.
//extern	player_t	players[MAXPLAYERS];
extern	player_t*	players;

// Alive? Disconnected?
extern  boolean*		playeringame;
//extern  boolean		playeringame[MAXPLAYERS];

extern short		(*consistancy)[BACKUPTICS];

#define MAXHEALTH	200
#define MAXARMOUR	200
#define MAXSOULHEALTH	200

#define STANDARDARMOUR  100

#define NORMHEALTH	100
#define NORMARMOUR	0
#define NORMAMMO	50
#define BFGCELLS	40
#define SOULHEALTH	100
#define MEGAHEALTH	200

#define GREENARMOUR	1
#define BLUEARMOUR	2

#define CHEATARMOUR     MAXARMOUR
#define CHEATARMOURT    BLUEARMOUR



// Player spawn spots for deathmatch.
extern  int		max_deathmatch_starts;
extern  mapthing_t*     deathmatchstarts;
extern  mapthing_t*	deathmatch_p;

// Player spawn spots.
extern  mapthing_t*      playerstarts;
//extern  mapthing_t      playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern  wbstartstruct_t		wminfo;	


// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern  int		maxammo[4];

// forces update of the avaliable weapons by the key.
extern boolean weaponupdate;


//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern char basedefault[1024];
extern FILE* debugfile;

// if true, load all graphics at level load
extern  boolean         precache;


// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern  gamestate_t     wipegamestate;

extern  int             mouseSensitivity;

//?
// debug flag to cancel adaptiveness
extern  boolean         singletics;	

extern  int             bodyqueslot;

// Needed to store the number of the dummy sky flat.
// Used for rendering,
//  as well as tracking projectiles etc.
extern int		skyflatnum;

// Netgame stuff (buffers and pointers, i.e. indices).

// This is for use in the network communication.
extern  doomcom_t*	doomcom;

// This points inside doomcom.
extern  doomdata_t*	netbuffer;	


extern  ticcmd_t	localcmds[BACKUPTICS];
extern	int		rndindex;

extern	int		maketic;
extern  int*             nettics;

extern  ticcmd_t        (*netcmds)[BACKUPTICS];
//extern  int             nettics[MAXNETNODES];

//extern  ticcmd_t        netcmds[MAXPLAYERS][BACKUPTICS];
extern	int		ticdup;

//misc stuff
extern boolean newhupd;
extern int lessaccuratemon;
extern int lessaccuratezom;
extern int rotatemap;
//extern int itemrespawn;
extern int showstats;
extern int novert;
extern boolean swapstereo;
extern int newhud;
extern int crosshair;
extern int screenblocks;
extern int infight;
//extern int stretchsky;
//extern int grav;
//extern int true3dgameplay;
extern int missileteleport; 
extern int teleportdelay;

//cd-audio stuff
typedef enum
{
  CD_OFF = 0,
  CD_ON,
  CD_ATMOS
}
cdType_t;

extern cdType_t cdaudio;
extern int cdtrack;
extern int cdnumtracks;
extern int cdcounter;

//okay, heres the resolution/hicolor:
extern int SCREENWIDTH;
extern int SCREENHEIGHT;
extern int SCREENDEPTH;
extern int SCREENPITCH;
// I_Video.c / V_Video*.c Precalc. Stuff
extern fixed_t DX,DY,DXI,DYI,DY2,DYI2;
extern fixed_t SCALEDWIDTH,SCALEDHEIGHT,X_OFFSET,Y_OFFSET;
extern fixed_t BASEYCENTER;
extern int retrace;                // Can be used if DBLBUF is not.
extern int BPP;                    //BYTES per pixel, 1=256 color, 2=hi color
extern int weirdaspect;            //1 means 8:5 aspect ratio, 0 means 4:3 aspect ratio
//extern short hicolortransmask1;    //mask for hi-color transparancy
//extern short hicolortransmask2;    //mask for hi-color transparancy
//extern char *translucencytable25;
//extern char *translucencytable50;
//extern char *translucencytable75;
//extern int transluc;
// -ES- 1998/11/28 Added these two
extern int faded_teleportation;
extern int wipe_method;
// -ES- 1998/11/29 Added translucency tables
extern long col2rgb16[65][256][2];
extern long col2rgb8[65][256];
extern unsigned char rgb_8k[16][32][16]; // 8K RGB table, for 8-bit translucency
extern unsigned long hicolortransmask3;
extern char hicolortransshift;

//mlook stuff
extern fixed_t keylookspeed;
extern int mlookon;
extern int invertmouse;

// -KM- 1998/09/01 Analogue binding stuff, These hold what axis they bind to.
extern int joy_xaxis;
extern int joy_yaxis;
extern int mouse_xaxis;
extern int mouse_yaxis;

//
// -ACB- 1998/09/06 Analogue binding:
//                   Two stage turning, angleturn control
//                   horzmovement control, vertmovement control
//                   strafemovediv;
//
extern boolean stageturn;  
extern int forwardmovespeed;
extern int angleturnspeed;
extern int sidemovespeed;

#endif
