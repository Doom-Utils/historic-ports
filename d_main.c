// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
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
// $Log:$
//
// DESCRIPTION:
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//
//-----------------------------------------------------------------------------


static const char rcsid[] = "$Id: d_main.c,v 1.8 1997/02/03 22:45:09 b1 Exp $";

#define	BGCOLOR		7
#define	FGCOLOR		8


#ifdef NORMALUNIX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "doomdef.h"
#include "doomstat.h"

#include "dstrings.h"
#include "sounds.h"


#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "multires.h"

#include "f_finale.h"
#include "f_wipe.h"

#include "m_argv.h"
#include "m_misc.h"
#include "m_menu.h"

#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"

#include "g_game.h"

#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"

#include "p_setup.h"
#include "r_local.h"


#include "d_main.h"
#include "allegvid.h"
#include "dehacked.h"

//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
void D_DoomLoop (void);


char*		wadfiles[MAXWADFILES];


boolean		devparm;	// started game with -devparm
boolean         nomonsters;	// checkparm of -nomonsters
boolean         respawnparm;	// checkparm of -respawn
boolean         fastparm;	// checkparm of -fast

boolean         drone;

boolean		singletics = false; // debug flag to cancel adaptiveness



//extern int soundVolume;
//extern  int	sfxVolume;
//extern  int	musicVolume;

extern  boolean	inhelpscreens;

skill_t		startskill;
int             startepisode;
int		startmap;
boolean		autostart;

FILE*		debugfile;

boolean		advancedemo;




char		wadfile[1024];		// primary wad file
char		mapdir[1024];           // directory of development maps
char		basedefault[1024];      // default file

mobj_t* RandomTarget;

int newnmrespawn=0;
int LessAccurateMon=0;
int rotatemap=0;
int ItemRespawn=0;
int showstats=0;
int novert=0;
int swapstereo=0;
int RandomInfight=0;
int TotalWar=0;
int NewAI=0;
int HumanMad=0;
int HumanExplode=0;
int crosshair=0;
int stretchsky=0;
int grav=8;
int shootupdown=0;

void D_CheckNetGame (void);
void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t* cmd);
void D_DoAdvanceDemo (void);


//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
//
event_t         events[MAXEVENTS];
int             eventhead;
int 		eventtail;


//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent (event_t* ev)
{
    events[eventhead] = *ev;
    eventhead = (++eventhead)&(MAXEVENTS-1);
}


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*	ev;
	
    // IF STORE DEMO, DO NOT ACCEPT INPUT
    if ( ( gamemode == commercial )
	 && (W_CheckNumForName("map01")<0) )
      return;
	
    for ( ; eventtail != eventhead ; eventtail = (++eventtail)&(MAXEVENTS-1) )
    {
	ev = &events[eventtail];
	if (M_Responder (ev))
	    continue;               // menu ate the event
	G_Responder (ev);
    }
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t     wipegamestate = GS_DEMOSCREEN;
extern  boolean setsizeneeded;
extern  int             showMessages;
void R_ExecuteSetViewSize (void);
boolean			redrawsbar=false;

void D_Display (void)
{
    static  boolean		viewactivestate = false;
    static  boolean		menuactivestate = false;
    static  boolean		inhelpscreensstate = false;
    static  boolean		fullscreen = false;
    static  gamestate_t		oldgamestate = -1;
    static  int			borderdrawcount;
    int				nowtime;
    int				tics;
    int				wipestart;
    int				y;
    boolean			done;
    boolean			wipe;
    boolean			redrawsbar;

    if (nodrawers)
	return;                    // for comparative timing / profiling

    if (doublebufferflag==1)
      {
      int i,j;
      byte *blah;

      j=SCREENWIDTH*BPP; blah=screens[0]+j*viewwindowy;
      for (i=0;i<viewheight;i++)
        {
        ylookup[i]=blah; blah+=j;
        }
      }
		    
    // change the view size if needed
    if (setsizeneeded)
    {
	R_ExecuteSetViewSize ();
	oldgamestate = -1;                      // force background redraw
	borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
	wipe = true;
	wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
	wipe = false;

    if (gamestate == GS_LEVEL && gametic)
	HU_Erase();
    
    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
	if (!gametic)
	    break;
	if (automapactive==2)
	    AM_Drawer ();
	if (wipe || (viewheight != SCREENHEIGHT && fullscreen) )
	    redrawsbar = true;
	if (inhelpscreensstate && !inhelpscreens)
	    redrawsbar = true;              // just put away the help screen
	ST_Drawer (viewheight == SCREENHEIGHT, redrawsbar );
        redrawsbar = false;
	fullscreen = viewheight == SCREENHEIGHT;
	break;

      case GS_INTERMISSION:
	WI_Drawer ();
	break;

      case GS_FINALE:
	F_Drawer ();
	break;

      case GS_DEMOSCREEN:
	D_PageDrawer ();
	break;
    }
    
    // draw buffered stuff to screen
    I_UpdateNoBlit ();
    
    // draw the view directly
    if (gamestate == GS_LEVEL && gametic && automapactive != 2 )
      {
      R_RenderPlayerView (&players[displayplayer]);
      if (automapactive)
        AM_Drawer ();
      }

    if (gamestate == GS_LEVEL && gametic)
	HU_Drawer ();
    
    // clean up border stuff
    if (gamestate != oldgamestate && gamestate != GS_LEVEL)
	I_SetPalette (W_CacheLumpName ("PLAYPAL",PU_CACHE),0);

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
	viewactivestate = false;        // view was not active
	R_FillBackScreen ();    // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
   if (gamestate == GS_LEVEL && automapactive != 2 )
    {
	if (menuactive || menuactivestate || !viewactivestate)
	    borderdrawcount = 3;
	if (borderdrawcount)
	{
	    R_DrawViewBorder ();    // erase old menu stuff
	    borderdrawcount--;
	}

    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    inhelpscreensstate = inhelpscreens;
    oldgamestate = wipegamestate = gamestate;
    
    // draw pause pic
    if (paused)
    {
	if (automapactive)
	    y = 4;
	else
	    y = viewwindowy+4;
	V_DrawPatchDirect(viewwindowx+(scaledviewwidth-68)/2,
			  y,0,W_CacheLumpName ("M_PAUSE", PU_CACHE));
    }


    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation


    // normal update
    if (!wipe)
    {
	I_FinishUpdate ();              // page flip or blit buffer
	return;
    }
    
    // wipe update
    wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    wipestart = I_GetTime () - 1;

    do
    {
	do
	{
	    nowtime = I_GetTime ();
	    tics = nowtime - wipestart;
	} while (!tics);
	wipestart = nowtime;
	done = wipe_ScreenWipe(wipe_Melt
			       , 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
	I_UpdateNoBlit ();
	M_Drawer ();                            // menu is drawn even on top of wipes
	I_FinishUpdate ();                      // page flip or blit buffer
    } while (!done);
}



//
//  D_DoomLoop
//
extern  boolean         demorecording;

void D_DoomLoop (void)
{
    if (demorecording)
	G_BeginRecording ();
		
    if (M_CheckParm ("-debugfile"))
    {
	char    filename[20];
	sprintf (filename,"debug%i.txt",consoleplayer);
	printf ("debug output to: %s\n",filename);
	debugfile = fopen (filename,"w");
    }
	
    I_InitGraphics ();

    while (1)
    {
	// frame syncronous IO operations
	I_StartFrame ();                
	
	// process one or more tics
	if (singletics)
	{
	    I_StartTic ();
	    D_ProcessEvents ();
	    G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
	    if (advancedemo)
		D_DoAdvanceDemo ();
	    M_Ticker ();
	    G_Ticker ();
	    gametic++;
	    maketic++;
	}
	else
	{
	    TryRunTics (); // will run at least one tic
	}
		
	S_UpdateSounds (players[consoleplayer].mo);// move positional sounds

	// Update display, next frame, with current state.
	D_Display ();

   //check cd player - if curr track is done, go to next track
   if (!nosound)
     I_CheckCD();

#ifndef SNDSERV
	// Sound mixing for the buffer is snychronous.
       if (!nosound)
	I_UpdateSound();
#endif	
	// Synchronous sound output is explicitly called.
#ifndef SNDINTR
	// Update sound output.
       if (!nosound)
	I_SubmitSound();
#endif
    }
}



//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char                    *pagename;


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
	D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
    V_DrawPatchInDirect (0,0, 0, W_CacheLumpName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
 void D_DoAdvanceDemo (void)
{
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    paused = false;
    gameaction = ga_nothing;

    if ( gamemode == retail )
      demosequence = (demosequence+1)%7;
    else
      demosequence = (demosequence+1)%6;
    
    switch (demosequence)
    {
      case 0:
	if ( gamemode == commercial )
	    pagetic = 35 * 11;
	else
	    pagetic = 170;
	gamestate = GS_DEMOSCREEN;
	pagename = "TITLEPIC";
	if ( gamemode == commercial )
	  S_StartMusic(mus_dm2ttl);
	else
	  S_StartMusic (mus_intro);
	break;
      case 1:
	G_DeferedPlayDemo ("demo1");
	break;
      case 2:
	pagetic = 200;
	gamestate = GS_DEMOSCREEN;
	pagename = "CREDIT";
	break;
      case 3:
	G_DeferedPlayDemo ("demo2");
	break;
      case 4:
	gamestate = GS_DEMOSCREEN;
	if ( gamemode == commercial)
	{
	    pagetic = 35 * 11;
	    pagename = "TITLEPIC";
	    S_StartMusic(mus_dm2ttl);
	}
	else
	{
	    pagetic = 200;

	    if ( gamemode == retail )
	      pagename = "CREDIT";
	    else
	      pagename = "HELP2";
	}
	break;
      case 5:
	G_DeferedPlayDemo ("demo3");
	break;
        // THE DEFINITIVE DOOM Special Edition demo
      case 6:
	G_DeferedPlayDemo ("demo4");
	break;
    }
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
    gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo ();
}




//      print title for every printed line
char            title[128];



//
// D_AddFile
//
void D_AddFile (char *file)
{
    int     numwadfiles;
    char    *newfile;
	
    for (numwadfiles = 0 ; wadfiles[numwadfiles] ; numwadfiles++)
	;

    newfile = malloc (strlen(file)+1);
    strcpy (newfile, file);
	
    wadfiles[numwadfiles] = newfile;
}

//
// IdentifyVersion
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWAD's).
//
void IdentifyVersion (void)
{

    char*	doom1wad;
    char*	doomwad;
    char*	doomuwad;
    char*	doom2wad;

    char*	doom2fwad;
    char*	plutoniawad;
    char*	tntwad;

#define MHFX_DOOMWADDIR
#ifdef MHFX_DOOMWADDIR
int p;
#endif

#ifdef NORMALUNIX
    char *home;
    char *doomwaddir;
    doomwaddir = getenv("DOOMWADDIR");
    if (!doomwaddir)
	doomwaddir = ".";

#ifdef MHFX_DOOMWADDIR
    p=M_CheckParm("-gwaddir");
    if (p && p<myargc-1)
      {
      doomwaddir=(char *)malloc(strlen(myargv[p+1])+1);
      strcpy(doomwaddir,myargv[p+1]);
      }
#endif

    // Commercial.
    doom2wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom2wad, "%s/doom2.wad", doomwaddir);

    // Retail.
    doomuwad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doomuwad, "%s/doomu.wad", doomwaddir);
    
    // Registered.
    doomwad = malloc(strlen(doomwaddir)+1+8+1);
    sprintf(doomwad, "%s/doom.wad", doomwaddir);
    
    // Shareware.
    doom1wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom1wad, "%s/doom1.wad", doomwaddir);

     // Bug, dear Shawn.
    // Insufficient malloc, caused spurious realloc errors.
    plutoniawad = malloc(strlen(doomwaddir)+1+/*9*/12+1);
    sprintf(plutoniawad, "%s/plutonia.wad", doomwaddir);

    tntwad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(tntwad, "%s/tnt.wad", doomwaddir);


    // French stuff.
    doom2fwad = malloc(strlen(doomwaddir)+1+10+1);
    sprintf(doom2fwad, "%s/doom2f.wad", doomwaddir);

#ifdef DJGPP
    home = getenv("DOSDOOM");
    if (!home)
      sprintf(basedefault,"default.cfg");
    else
      sprintf(basedefault, "%s\\default.cfg", home);
#else
    home=getenv("HOME");
    if (!home)
      I_Error("Please set $HOME to your home directory");
    sprintf(basedefault,"%s/.doomrc",home);
#endif

#endif

    if (M_CheckParm ("-shdev"))
    {
	gamemode = shareware;
	devparm = true;
	D_AddFile (DEVDATA"doom1.wad");
	D_AddFile (DEVMAPS"data_se/texture1.lmp");
	D_AddFile (DEVMAPS"data_se/pnames.lmp");
#ifdef DJGPP
	strcpy (basedefault,DEVDATA"default.cfg");
#else
	strcpy (basedefault,DEVDATA".doomrc");
#endif
	return;
    }

    if (M_CheckParm ("-regdev"))
    {
	gamemode = registered;
	devparm = true;
	D_AddFile (DEVDATA"doom.wad");
	D_AddFile (DEVMAPS"data_se/texture1.lmp");
	D_AddFile (DEVMAPS"data_se/texture2.lmp");
	D_AddFile (DEVMAPS"data_se/pnames.lmp");
#ifdef DJGPP
	strcpy (basedefault,DEVDATA"default.cfg");
#else
	strcpy (basedefault,DEVDATA".doomrc");
#endif
	return;
    }

    if (M_CheckParm ("-comdev"))
    {
	gamemode = commercial;
	devparm = true;
	/* I don't bother
	if(plutonia)
	    D_AddFile (DEVDATA"plutonia.wad");
	else if(tnt)
	    D_AddFile (DEVDATA"tnt.wad");
	else*/
	    D_AddFile (DEVDATA"doom2.wad");
	    
	D_AddFile (DEVMAPS"cdata/texture1.lmp");
	D_AddFile (DEVMAPS"cdata/pnames.lmp");
#ifdef DJGPP
	strcpy (basedefault,DEVDATA"default.cfg");
#else
	strcpy (basedefault,DEVDATA".doomrc");
#endif
	return;
    }

    //new stuff - check for wad specifiers
    if (M_CheckParm("-doom2f"))
      {
      if (!access(doom2fwad,R_OK))
        {
        //Raven: Add gamemission for all commercial games...
        gamemode=commercial; language=french; gamemission=doom2;
        printf("French version");
        D_AddFile(doom2fwad); return;
        }
      else
        I_Error("Unable to find Doom2 French Wad!\n");
      }
    if (M_CheckParm("-doom2"))
      {
      if (!access(doom2wad,R_OK))
        {
        //Raven: Add gamemission for all commercial games...
        gamemode=commercial; gamemission=doom2;
        D_AddFile(doom2wad); return;
        }
      else
        I_Error("Unable to find Doom2 Wad!\n");
      }
    if (M_CheckParm("-plutonia"))
      {
      if (!access(plutoniawad,R_OK))
        {
        //Raven: Add gamemission for all commercial games...
        gamemode=commercial; gamemission=pack_plut;
        D_AddFile(plutoniawad); return;
        }
      else
        I_Error("Unable to find Plutonia Wad!\n");
      }
    if (M_CheckParm("-tnt"))
      {
      if (!access(tntwad,R_OK))
        {
        //Raven: Add gamemission for all commercial games...
        gamemode=commercial; gamemission=pack_tnt;
        D_AddFile(tntwad); return;
        }
      else
        I_Error("Unable to find TNT Wad!\n");
      }
    if (M_CheckParm("-udoom"))
      {
      if (!access(doomuwad,R_OK))
        {
        gamemode=retail;
        D_AddFile(doomuwad); return;
        }
      else if (!access(doomwad,R_OK))
        {
        gamemode=retail;
        D_AddFile(doomwad); return;
        }
      else
        I_Error("Unable to find Ultimate Doom Wad!\n");
      }
    if (M_CheckParm("-doom"))
      {
      if (!access(doomwad,R_OK))
        {
        gamemode=registered;
        D_AddFile(doomwad); return;
        }
      else
        I_Error("Unable to find Doom Wad!\n");
      }
    if (M_CheckParm("-shareware"))
      {
      if (!access(doom1wad,R_OK))
        {
        gamemode=shareware;
        D_AddFile(doom1wad); return;
        }
      else
        I_Error("Unable to find Shareware Wad!\n");
      }
    //end of new stuff


    if ( !access (doom2fwad,R_OK) )
    {
        //Raven: Add gamemission for all commercial games...
	gamemode = commercial; gamemission=doom2;
	// C'est ridicule!
	// Let's handle languages in config files, okay?
	language = french;
	printf("French version\n");
	D_AddFile (doom2fwad);
	return;
    }

    if ( !access (doom2wad,R_OK) )
    {
        //Raven: Add gamemission for all commercial games...
	gamemode = commercial; gamemission=doom2;
	D_AddFile (doom2wad);
	return;
    }

    if ( !access (plutoniawad, R_OK ) )
    {
        //Raven: Add gamemission for all commercial games...
      gamemode = commercial; gamemission=pack_plut;
      D_AddFile (plutoniawad);
      return;
    }

    if ( !access ( tntwad, R_OK ) )
    {
        //Raven: Add gamemission for all commercial games...
      gamemode = commercial; gamemission=pack_tnt;
      D_AddFile (tntwad);
      return;
    }

    if ( !access (doomuwad,R_OK) )
    {
      gamemode = retail;
      D_AddFile (doomuwad);
      return;
    }

    if ( !access (doomwad,R_OK) )
    {  //changes to retail if appropriate after w_initmultiplefiles
      gamemode = registered;
      D_AddFile (doomwad);
      return;
    }

    if ( !access (doom1wad,R_OK) )
    {
      gamemode = shareware;
      D_AddFile (doom1wad);
      return;
    }

    printf("Game mode indeterminate.\n");
    gamemode = indetermined;

    // We don't abort. Let's see what the PWAD contains.
    //exit(1);
    //I_Error ("Game mode indeterminate\n");
}


void ApplyResponseFile (char *filename, int i)
  {
#define MAXARGVS        200

  FILE *          handle;
  int             size;
  int             k;
  int             index;
  int             indexinfile;
  char    *infile;
  char    *file;
  char    *moreargs[20];
  char    *firstargv;
			
  // READ THE RESPONSE FILE INTO MEMORY
  handle = fopen (filename,"rb");
  if (!handle)
    {
    printf ("\nNo such response file!");
    exit(1);
    }
  printf("Found response file %s!\n",filename);
  fseek (handle,0,SEEK_END);
  size = ftell(handle);
  fseek (handle,0,SEEK_SET);
  file = malloc (size);
  fread (file,size,1,handle);
  fclose (handle);

  // KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
  for (index = 0,k = i+1; k < myargc; k++)
    moreargs[index++] = myargv[k];
			
  firstargv = myargv[0];
  myargv = malloc(sizeof(char *)*MAXARGVS);
  memset(myargv,0,sizeof(char *)*MAXARGVS);
  myargv[0] = firstargv;
			
  infile = file;
  indexinfile = k = 0;
  indexinfile++;  // SKIP PAST ARGV[0] (KEEP IT)
  do{
    myargv[indexinfile++] = infile+k;
    while(k < size &&
     ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
       k++;
    *(infile+k) = 0;
    while(k < size &&
     ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
       k++;
    } while(k < size);
			
    for (k = 0;k < index;k++)
	myargv[indexinfile++] = moreargs[k];
    myargc = indexinfile;
	
    // DISPLAY ARGS
    printf("%d command-line args:\n",myargc);
    for (k=1;k<myargc;k++)
	printf("%s\n",myargv[k]);
  }
//
// Find a Response File
//
void FindResponseFile (void)
{
    int             i;

    if ( !access ("dosdoom.cmd", R_OK ) )
      {
      ApplyResponseFile("dosdoom.cmd",0);
      }

    for (i = 1;i < myargc;i++)
	if (myargv[i][0] == '@')
	{
        ApplyResponseFile(&(myargv[i][1]),i);
        break;
	}
}


//
// D_DoomMain
//
void D_DoomMain (void)
{
    int             p;
    char                    file[256];

#ifdef DJGPP
    set_config_file("dosdoom.cfg");
#endif
    FindResponseFile ();
	
    IdentifyVersion ();
	
    setbuf (stdout, NULL);
    modifiedgame = false;
	
    nomonsters = M_CheckParm ("-nomonsters");
    respawnparm = M_CheckParm ("-respawn");
    fastparm = M_CheckParm ("-fast");
    devparm = M_CheckParm ("-devparm");
    if (M_CheckParm ("-altdeath"))
	deathmatch = 2;
    else if (M_CheckParm ("-deathmatch"))
	deathmatch = 1;

    if (M_CheckParm ("-newnmrespawn"))
      newnmrespawn=1;
    if (M_CheckParm ("-lessaccurate"))
      LessAccurateMon=1;
    if (M_CheckParm ("-rotatemap"))
      rotatemap=1;
    if (M_CheckParm ("-itemrespawn"))
      ItemRespawn=1;
    if (M_CheckParm ("-stretchsky"))
      stretchsky=1;

    //mlook stuff
    if (M_CheckParm ("-mlook"))
      mlookon=1;
    if (M_CheckParm ("-invertmouse"))
      invertmouse=1;
    p=M_CheckParm("-vspeed");
    if (p && p<myargc-1)
      keylookspeed=atoi(myargv[p+1])/64;


    if (M_CheckParm ("-french"))
      language=french;
    if (M_CheckParm ("-german1"))
      language=german1;
    if (M_CheckParm ("-german2"))
      language=german2;
    if (M_CheckParm ("-turkish"))
      language=turkish;
    if (M_CheckParm ("-spanish"))
      language=spanish;
    if (M_CheckParm ("-dutch"))
      language=dutch;
    if (M_CheckParm ("-swedish"))
      {
      language=swedish;
      if (access("swedish.wad",0)==0) D_AddFile("swedish.wad");
        else I_Error("swedish.WAD Could not be added\n");
      }
    initstrings();
    p = M_CheckParm ("-deh"); //put this near the front, b4 any text is printed
    if (p)
    {
	// the parms after p are dehfile names,
	// until end of parms or another - preceded parm
	modifiedgame = true;            // homebrew levels
	while (++p != myargc && myargv[p][0] != '-')
	    loaddeh(myargv[p]);
    }
    applystrings();

	//Raven: Modified to tell the difference between doom2, tnt and plutonia
	switch ( gamemode )
	{
		case retail:
			sprintf (title,UDOOMSTART,
				VERSION/100,VERSION%100);
			break;
		case shareware:
			sprintf (title,SHAREDOOMSTART,
				VERSION/100,VERSION%100);
			break;
		case registered:
			sprintf (title,REGDOOMSTART,  //fixme! - might be udoom
				VERSION/100,VERSION%100);
			break;
		case commercial:
            //Raven: Main change here...
			switch(gamemission)
			{
				case doom2:
					sprintf (title,DOOM2START,
						VERSION/100,VERSION%100);
					break;
				case pack_plut:
					sprintf (title,PLUTSTART,
						VERSION/100,VERSION%100);
					break;
				case pack_tnt:
					sprintf (title,TNTSTART,
						VERSION/100,VERSION%100);
					break;
				case doom:	//Raven: shouldn't occur, but is needed
				case none:	//Raven: shouldn't occur, but is needed
                	break;
			}
            break;
		default:
			sprintf (title,PUBDOOMSTART,
				VERSION/100,VERSION%100);
			break;
	}    
    printf ("%s\n",title);
    printf ("                                (DosDoom v%i.%i)\n",DOSDOOMVER/100,DOSDOOMVER%100);

    if (devparm)
	printf(D_DEVSTR);
    
    if (M_CheckParm("-cdrom"))
    {
	printf(D_CDROM);
	mkdir("c:\\doomdata",0);
#ifdef DJGPP
	strcpy (basedefault,"c:/doomdata/default.cfg");
#else
	strcpy (basedefault,"~/.doomrc");
#endif
    }	
    
    // turbo option
    if ( (p=M_CheckParm ("-turbo")) )
    {
	int     scale = 200;
	extern int forwardmove[2];
	extern int sidemove[2];
	
	if (p<myargc-1)
	    scale = atoi (myargv[p+1]);
	if (scale < 10)
	    scale = 10;
	if (scale > 400)
	    scale = 400;
	printf (TURBOSCLSTR,scale);
	forwardmove[0] = forwardmove[0]*scale/100;
	forwardmove[1] = forwardmove[1]*scale/100;
	sidemove[0] = sidemove[0]*scale/100;
	sidemove[1] = sidemove[1]*scale/100;
    }

    //check resolution
    multires_setbpp();
    multires_setres();

    // add any files specified on the command line with -file wadfile
    // to the wad list
    //
    // convenience hack to allow -wart e m to add a wad file
    // prepend a tilde to the filename so wadfile will be reloadable
    p = M_CheckParm ("-wart");
    if (p)
    {
	myargv[p][4] = 'p';     // big hack, change to -warp

	// Map name handling.
	switch (gamemode )
	{
	  case shareware:
	  case retail:
	  case registered:
	    sprintf (file,"~"DEVMAPS"E%cM%c.wad",
		     myargv[p+1][0], myargv[p+2][0]);
	    printf("Warping to Episode %s, Map %s.\n",
		   myargv[p+1],myargv[p+2]);
	    break;
	    
	  case commercial:
	  default:
	    p = atoi (myargv[p+1]);
	    if (p<10)
	      sprintf (file,"~"DEVMAPS"cdata/map0%i.wad", p);
	    else
	      sprintf (file,"~"DEVMAPS"cdata/map%i.wad", p);
	    break;
	}
	D_AddFile (file);
    }
	
    //--------------------------------------------------------------------------
    //-CTF(JC) Add Extra player backgrounds for 8 player support.
    if (access("playback.wad",0)==0) D_AddFile("playback.wad");
      else I_Error("PLAYBACK.WAD Could not be added\n");

   //--------------------------------------------------------------------------
    p = M_CheckParm ("-file");
    if (p)
    {
	// the parms after p are wadfile/lump names,
	// until end of parms or another - preceded parm
	modifiedgame = true;            // homebrew levels
	while (++p != myargc && myargv[p][0] != '-')
	    D_AddFile (myargv[p]);
    }

    p = M_CheckParm ("-playdemo");

    if (!p)
	p = M_CheckParm ("-timedemo");

    if (p && p < myargc-1)
    {
	sprintf (file,"%s.lmp", myargv[p+1]);
	D_AddFile (file);
	printf("Playing demo %s.lmp.\n",myargv[p+1]);
    }
    
    // get skill / episode / map from parms
    startskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

		
    p = M_CheckParm ("-skill");
    if (p && p < myargc-1)
    {
	startskill = myargv[p+1][0]-'1';
	autostart = true;
    }

    p = M_CheckParm ("-episode");
    if (p && p < myargc-1)
    {
	startepisode = myargv[p+1][0]-'0';
	startmap = 1;
	autostart = true;
    }
	
    p = M_CheckParm ("-timer");
    if (p && p < myargc-1 && deathmatch)
    {
	int     time;
	time = atoi(myargv[p+1]);
	printf("Levels will end after %d minute",time);
	if (time>1)
	    printf("s");
	printf(".\n");
    }

    p = M_CheckParm ("-avg");
    if (p && p < myargc-1 && deathmatch)
	printf("Austin Virtual Gaming: Levels will end after 20 minutes\n");

    p = M_CheckParm ("-warp");
    if (p && p < myargc-1)
    {
	if (gamemode == commercial)
	    startmap = atoi (myargv[p+1]);
	else
	{
	    startepisode = myargv[p+1][0]-'0';
	    startmap = myargv[p+2][0]-'0';
	}
	autostart = true;
    }
    
    // init subsystems
    printf (V_INITSTR);
    V_Init ();

    printf (M_LDEFSTR);
    M_LoadDefaults ();              // load before initing other systems

    printf (Z_INITSTR);
    Z_Init ();

    printf (W_INITSTR);
    W_InitMultipleFiles (wadfiles);
    if (gamemode==registered)
      {
      if (W_CheckNumForName("e4m1")!=-1)
        {
        gamemode=retail;
        printf ("Ultimate Doom!!!!!!!\n");
        }
      //else
//        printf ("Registered Doom!!!!!!!!!\n");
      }

    // Check for -file in shareware
    if (modifiedgame)
    {
	// These are the lumps that will be checked in IWAD,
	// if any one is not present, execution will be aborted.
	char name[23][8]=
	{
	    "e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
	    "e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
	    "dphoof","bfgga0","heada1","cybra1","spida1d1"
	};
	int i;
	
	if ( gamemode == shareware)
	    I_Error("\nYou cannot -file with the shareware "
		    "version. Register!");

	// Check for fake IWAD with right name,
	// but w/o all the lumps of the registered version. 
	if (gamemode == registered)
	    for (i = 0;i < 23; i++)
		if (W_CheckNumForName(name[i])<0)
		    I_Error("\nThis is not the registered version.");
    }
    
    // Iff additonal PWAD files are used, print modified banner
    if (modifiedgame)
    {
    printf("%s",MODMSG);
    getchar ();
    }
	

    // Check and print which version is executed.
    switch ( gamemode )
    {
      case shareware:
      case indetermined:
	printf (SWMSG);
	break;
      case registered:
      case retail:
        printf (NOSWMSG);
        break;
      case commercial:
	printf (NOSWMSG2);
	break;
	
      default:
	// Ouch.
	break;
    }

    printf (M_INITSTR);
    M_Init ();

    printf (R_INITSTR);
    R_Init ();

    printf (P_INITSTR);
    P_Init ();

    printf (I_INITSTR);
    I_Init ();

    printf (D_CHKNETSTR);
    D_CheckNetGame ();

    printf (S_INITSTR);
    S_Init (snd_SfxVolume /**8*/, snd_MusicVolume  /**8*/ );

    printf (HU_INITSTR);
    HU_Init ();

    printf (ST_INITSTR);
    ST_Init ();

    // check for a driver that wants intermission stats
    p = M_CheckParm ("-statcopy");
    if (p && p<myargc-1)
    {
	// for statistics driver
	extern  void*	statcopy;                            

	statcopy = (void*)atoi(myargv[p+1]);
	printf ("External statistics registered.\n");
    }
    
    // start the apropriate game based on parms
    p = M_CheckParm ("-record");

    if (p && p < myargc-1)
    {
	G_RecordDemo (myargv[p+1]);
	autostart = true;
    }
	
    p = M_CheckParm ("-playdemo");
    if (p && p < myargc-1)
    {
	singledemo = true;              // quit after one demo
	G_DeferedPlayDemo (myargv[p+1]);
	D_DoomLoop ();  // never returns
    }
	
    p = M_CheckParm ("-timedemo");
    if (p && p < myargc-1)
    {
	G_TimeDemo (myargv[p+1]);
	D_DoomLoop ();  // never returns
    }
	
    p = M_CheckParm ("-loadgame");
    if (p && p < myargc-1)
    {
	if (M_CheckParm("-cdrom"))
	    sprintf(file, "c:\\doomdata\\"SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
	else
	    sprintf(file, SAVEGAMENAME"%c.dsg",myargv[p+1][0]);
	G_LoadGame (file);
    }
	

    if ( gameaction != ga_loadgame )
    {
	if (autostart || netgame)
	    G_InitNew (startskill, startepisode, startmap);
	else
	    D_StartTitle ();                // start up intro loop

    }

    D_DoomLoop ();  // never returns
}
