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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: st_stuff.c,v 1.6 1997/02/03 22:45:13 b1 Exp $";

#include <stdio.h>
#ifdef DJGPP
#include <i_music.h>
#endif

#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_random.h"
#include "w_wad.h"

#include "doomdef.h"

#include "g_game.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"

#include "p_local.h"
#include "p_inter.h"
#include "p_mobj.h"

#include "am_map.h"
#include "m_cheat.h"

#include "s_sound.h"

// Needs access to LFB.
#include "v_res.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "lu_sound.h"
#include "i_allegv.h"

//
// STATUS BAR DATA
//


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS		1
#define STARTBONUSPALS		9
#define NUMREDPALS			8
#define NUMBONUSPALS		4
// Radiation suit, green shift.
#define RADIATIONPAL		13

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY		96

// For Responder
#define ST_TOGGLECHAT		KEYD_ENTER

// Location of status bar
#define ST_X				(((SCREENWIDTH-320)/2)+0)
#define ST_X2				(((SCREENWIDTH-320)/2)+104)

#define ST_FX  			(((SCREENWIDTH-320)/2)+143)
#define ST_FY  			(SCREENHEIGHT-(200-169))

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH		(tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES		5
#define ST_NUMSTRAIGHTFACES	3
#define ST_NUMTURNFACES		2
#define ST_NUMSPECIALFACES		3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES		2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET		(ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET		(ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET		(ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET		(ST_EVILGRINOFFSET + 1)
#define ST_GODFACE			(ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE			(ST_GODFACE+1)

#define ST_FACESX			(((SCREENWIDTH-320)/2)+143)
#define ST_FACESY			(SCREENHEIGHT-(200-168))

#define ST_EVILGRINCOUNT		(2*TICRATE)
#define ST_STRAIGHTFACECOUNT	(TICRATE/2)
#define ST_TURNCOUNT		(1*TICRATE)
#define ST_OUCHCOUNT		(1*TICRATE)
#define ST_RAMPAGEDELAY		(2*TICRATE)

#define ST_MUCHPAIN			20


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH		3	
#define ST_AMMOX			(((SCREENWIDTH-320)/2)+44)
#define ST_AMMOY			(SCREENHEIGHT-(200-171))

// HEALTH number pos.
#define ST_HEALTHWIDTH		3	
#define ST_HEALTHX			(((SCREENWIDTH-320)/2)+90)
#define ST_HEALTHY			(SCREENHEIGHT-(200-171))

// Weapon pos.
#define ST_ARMSX			(((SCREENWIDTH-320)/2)+111)
#define ST_ARMSY			(SCREENHEIGHT-(200-172))
#define ST_ARMSBGX			(((SCREENWIDTH-320)/2)+104)
#define ST_ARMSBGY			(SCREENHEIGHT-(200-168))
#define ST_ARMSXSPACE		12
#define ST_ARMSYSPACE		10

// Frags pos.
#define ST_FRAGSX			(((SCREENWIDTH-320)/2)+138)
#define ST_FRAGSY			(SCREENHEIGHT-(200-171))
#define ST_FRAGSWIDTH		2

// ARMOR number pos.
#define ST_ARMORWIDTH		3
#define ST_ARMORX			(((SCREENWIDTH-320)/2)+221)
#define ST_ARMORY			(SCREENHEIGHT-(200-171))

// Key icon positions.
#define ST_KEY0WIDTH		8
#define ST_KEY0HEIGHT		5
#define ST_KEY0X			(((SCREENWIDTH-320)/2)+239)
#define ST_KEY0Y			(SCREENHEIGHT-(200-171))
#define ST_KEY1WIDTH		ST_KEY0WIDTH
#define ST_KEY1X			(((SCREENWIDTH-320)/2)+239)
#define ST_KEY1Y			(SCREENHEIGHT-(200-181))
#define ST_KEY2WIDTH		ST_KEY0WIDTH
#define ST_KEY2X			(((SCREENWIDTH-320)/2)+239)
#define ST_KEY2Y			(SCREENHEIGHT-(200-191))

// Ammunition counter.
#define ST_AMMO0WIDTH		3
#define ST_AMMO0HEIGHT		6
#define ST_AMMO0X			(((SCREENWIDTH-320)/2)+288)
#define ST_AMMO0Y			(SCREENHEIGHT-(200-173))
#define ST_AMMO1WIDTH		ST_AMMO0WIDTH
#define ST_AMMO1X			(((SCREENWIDTH-320)/2)+288)
#define ST_AMMO1Y			(SCREENHEIGHT-(200-179))
#define ST_AMMO2WIDTH		ST_AMMO0WIDTH
#define ST_AMMO2X			(((SCREENWIDTH-320)/2)+288)
#define ST_AMMO2Y			(SCREENHEIGHT-(200-191))
#define ST_AMMO3WIDTH		ST_AMMO0WIDTH
#define ST_AMMO3X			(((SCREENWIDTH-320)/2)+288)
#define ST_AMMO3Y			(SCREENHEIGHT-(200-185))

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH		3
#define ST_MAXAMMO0HEIGHT		5
#define ST_MAXAMMO0X		(((SCREENWIDTH-320)/2)+314)
#define ST_MAXAMMO0Y		(SCREENHEIGHT-(200-173))
#define ST_MAXAMMO1WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X		(((SCREENWIDTH-320)/2)+314)
#define ST_MAXAMMO1Y		(SCREENHEIGHT-(200-179))
#define ST_MAXAMMO2WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X		(((SCREENWIDTH-320)/2)+314)
#define ST_MAXAMMO2Y		(SCREENHEIGHT-(200-191))
#define ST_MAXAMMO3WIDTH		ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X		(((SCREENWIDTH-320)/2)+314)
#define ST_MAXAMMO3Y		(SCREENHEIGHT-(200-185))

// pistol
#define ST_WEAPON0X			(((SCREENWIDTH-320)/2)+110)
#define ST_WEAPON0Y			(SCREENHEIGHT-(200-172))

// shotgun
#define ST_WEAPON1X			(((SCREENWIDTH-320)/2)+122)
#define ST_WEAPON1Y			(SCREENHEIGHT-(200-172))

// chain gun
#define ST_WEAPON2X			(((SCREENWIDTH-320)/2)+134)
#define ST_WEAPON2Y			(SCREENHEIGHT-(200-172))

// missile launcher
#define ST_WEAPON3X			(((SCREENWIDTH-320)/2)+110)
#define ST_WEAPON3Y			(SCREENHEIGHT-(200-181))

// plasma gun
#define ST_WEAPON4X			(((SCREENWIDTH-320)/2)+122)
#define ST_WEAPON4Y			(SCREENHEIGHT-(200-181))

 // bfg
#define ST_WEAPON5X			(((SCREENWIDTH-320)/2)+134)
#define ST_WEAPON5Y			(SCREENHEIGHT-(200-181))

// WPNS title
#define ST_WPNSX			(((SCREENWIDTH-320)/2)+109)
#define ST_WPNSY			(SCREENHEIGHT-(200-191))

 // DETH title
#define ST_DETHX			(((SCREENWIDTH-320)/2)+109)
#define ST_DETHY			(SCREENHEIGHT-(200-191))

//Incoming messages window location
//UNUSED
// #define ST_MSGTEXTX	   (viewwindowx)
// #define ST_MSGTEXTY	   (viewwindowy+viewheight-18)
#define ST_MSGTEXTX			0
#define ST_MSGTEXTY			0
// Dimensions given in characters.
#define ST_MSGWIDTH			52
// Or shall I say, in lines?
#define ST_MSGHEIGHT		1

#define ST_OUTTEXTX			0
#define ST_OUTTEXTY			6

// Width, in characters again.
#define ST_OUTWIDTH			52 
 // Height, in lines. 
#define ST_OUTHEIGHT		1

#define ST_MAPWIDTH	\
    (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

#define ST_MAPTITLEX \
    (SCREENWIDTH - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY		0
#define ST_MAPHEIGHT		1

	    
// main player in game
static player_t*	plyr; 

// ST_Start() has just been called
static boolean		st_firsttime;

// used to execute ST_Init() only once
static int		veryfirsttime = 1;

// lump number for PLAYPAL
static int		lu_palette;

// used for timing
static unsigned int	st_clock;

// used for making messages go away
static int		st_msgcounter=0;

// used when in chat 
static st_chatstateenum_t	st_chatstate;

// whether in automap or first-person
static st_stateenum_t	st_gamestate;

// whether left-side main status bar is active
static boolean		st_statusbaron;

// whether status bar chat is active
static boolean		st_chat;

// value of st_chat before message popped up
static boolean		st_oldchat;

// whether chat window has the cursor on
static boolean		st_cursoron;

// !deathmatch
static boolean		st_notdeathmatch; 

// !deathmatch && st_statusbaron
static boolean		st_armson;

// !deathmatch
static boolean		st_fragson; 

// main bar left
static patch_t*		sbar;

// 0-9, tall numbers
static patch_t*		tallnum[10];

// tall % sign
static patch_t*		tallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t*		shortnum[10];

// 3 key-cards, 3 skulls
static patch_t*		keys[NUMCARDS]; 

// face status patches
static patch_t*		faces[ST_NUMFACES];

// face background
static patch_t*		faceback;

 // main bar right
static patch_t*		armsbg;

// weapon ownership patches
static patch_t*		arms[6][2]; 

// ready-weapon widget
static st_number_t	w_ready;

 // in deathmatch only, summary of frags stats
static st_number_t	w_frags;

// health widget
static st_percent_t	w_health;

// arms background
static st_binicon_t	w_armsbg; 


// weapon ownership widgets
static st_multicon_t	w_arms[6];

// face status widget
static st_multicon_t	w_faces; 

// keycard widgets
static st_multicon_t	w_keyboxes[3];

// armor widget
static st_percent_t	w_armor;

// ammo widgets
static st_number_t	w_ammo[4];

// max ammo widgets
static st_number_t	w_maxammo[4]; 



 // number of frags so far in deathmatch
static int	st_fragscount;

// used to use appopriately pained face
static int	st_oldhealth = -1;

// used for evil grin
static boolean	oldweaponsowned[NUMWEAPONS]; 

 // count until face changes
static int	st_facecount = 0;

// current face index, used by w_faces
static int	st_faceindex = 0;

// holds key-type for each key box on bar
static int	keyboxes[3]; 

// a random number per tick
static int	st_randomnumber;  



// Massive bunches of cheat shit
//  to keep it from being easy to figure them out.
// Yeah, right...
unsigned char	cheat_mus_seq[] =
{
'i','d','m','u','s',1,0,0,0xff
};

unsigned char	cheat_choppers_seq[] =
{
'i','d','c','h','o','p','p','e','r','s',0xff
};

unsigned char	cheat_god_seq[] =
{
'i','d','d','q','d',0xff
};

unsigned char	cheat_ammo_seq[] =
{
'i','d','k','f','a',0xff
};

unsigned char	cheat_ammonokey_seq[] =
{
'i','d','f','a',0xff
};


// Smashing Pumpkins Into Small Piles Of Putried Debris.
unsigned char	cheat_noclip_seq[] =
{
'i','d','s','p','i','s','p','o','p','d',0xff
};

//
unsigned char	cheat_commercial_noclip_seq[] =
{
'i','d','c','l','i','p',0xff
}; 



unsigned char	cheat_powerup_seq[7][10] =
{
    {'i','d','b','e','h','o','l','d','v',0xff},
    {'i','d','b','e','h','o','l','d','s',0xff},
    {'i','d','b','e','h','o','l','d','i',0xff},
    {'i','d','b','e','h','o','l','d','r',0xff},
    {'i','d','b','e','h','o','l','d','a',0xff},
    {'i','d','b','e','h','o','l','d','l',0xff},
    {'i','d','b','e','h','o','l','d',0xff}
};


unsigned char	cheat_clev_seq[] =
{
'i','d','c','l','e','v',1,0,0,0xff
};


// my position cheat
unsigned char	cheat_mypos_seq[] =
{
'i','d','m','y','p','o','s',0xff
}; 

//
// DOSDoom Cheats
//

// CD Next Track
unsigned char	cheat_cdnext_seq[] =
{
'c','d','n','e','x','t',0xff
};

// CD Previous Track
unsigned char	cheat_cdprev_seq[] =
{
'c','d','p','r','e','v',0xff
};

// Kill all monsters
unsigned char	cheat_killall_seq[] =
{
'i','d','k','i','l','l','a','l','l',0xff
};

// Suicide
unsigned char	cheat_suicide_seq[] =
{
'i','d','s','u','i','c','i','d','e',0xff
};

// show player pos and level info
unsigned char	cheat_showstats_seq[] =
{
'i','d','i','n','f','o',0xff
}; 

// give all the keys to me!
unsigned char	cheat_keys_seq[] =
{
'i','d','u','n','l','o','c','k',0xff
}; 

// show me the ammo!
unsigned char	cheat_loaded_seq[] =
{
'i','d','l','o','a','d','e','d',0xff
}; 

// removes weapons, ammos and keys
unsigned char	cheat_takeall_seq[] =
{
'i','d','t','a','k','e','a','l','l',0xff
}; 

// give weapons
unsigned char	cheat_giveweapon_seq[8][8] =
{
        {'i','d','g','i','v','e',0xff},     // not used
        {'i','d','g','i','v','e','1',0xff}, // chainsaw
        {'i','d','g','i','v','e','2',0xff}, // shotgun
        {'i','d','g','i','v','e','3',0xff}, // double-barrel shotgun
        {'i','d','g','i','v','e','4',0xff}, // chaingun
        {'i','d','g','i','v','e','5',0xff}, // missile launcher
        {'i','d','g','i','v','e','6',0xff}, // plasma rifle
        {'i','d','g','i','v','e','7',0xff}, // BFG9000
}; 

// Now what?
cheatseq_t	cheat_mus = { cheat_mus_seq, 0 };
cheatseq_t	cheat_god = { cheat_god_seq, 0 };
cheatseq_t	cheat_ammo = { cheat_ammo_seq, 0 };
cheatseq_t	cheat_ammonokey = { cheat_ammonokey_seq, 0 };
cheatseq_t	cheat_noclip = { cheat_noclip_seq, 0 };
cheatseq_t	cheat_commercial_noclip = { cheat_commercial_noclip_seq, 0 };

cheatseq_t	cheat_powerup[7] =
{
    { cheat_powerup_seq[0], 0 },
    { cheat_powerup_seq[1], 0 },
    { cheat_powerup_seq[2], 0 },
    { cheat_powerup_seq[3], 0 },
    { cheat_powerup_seq[4], 0 },
    { cheat_powerup_seq[5], 0 },
    { cheat_powerup_seq[6], 0 }
};

cheatseq_t	cheat_choppers = { cheat_choppers_seq, 0 };
cheatseq_t	cheat_clev = { cheat_clev_seq, 0 };
cheatseq_t	cheat_mypos = { cheat_mypos_seq, 0 };

//new cheats
cheatseq_t	cheat_cdnext = { cheat_cdnext_seq, 0 };
cheatseq_t	cheat_cdprev = { cheat_cdprev_seq, 0 };
cheatseq_t	cheat_killall = { cheat_killall_seq, 0 };
cheatseq_t	cheat_showstats = { cheat_showstats_seq, 0 };
cheatseq_t	cheat_suicide = { cheat_suicide_seq, 0 };
cheatseq_t	cheat_keys = { cheat_keys_seq, 0 };
cheatseq_t	cheat_loaded = { cheat_loaded_seq, 0 };
cheatseq_t	cheat_takeall = { cheat_takeall_seq, 0 };

cheatseq_t	cheat_giveweapon[8] =
{
    { cheat_giveweapon_seq[0], 0 },
    { cheat_giveweapon_seq[1], 0 },
    { cheat_giveweapon_seq[2], 0 },
    { cheat_giveweapon_seq[3], 0 },
    { cheat_giveweapon_seq[4], 0 },
    { cheat_giveweapon_seq[5], 0 },
    { cheat_giveweapon_seq[6], 0 },
    { cheat_giveweapon_seq[7], 0 },
};


// 
extern char*	mapnames[];

//for cdaudio
char tmpmsgstring[40];

//
// STATUS BAR CODE
//
void ST_Stop(void);

void ST_refreshBackground(void)
{

    if (st_statusbaron)
    {
	V_DrawPatch(ST_X, 0, BG, sbar);

	if (netgame)
	    V_DrawPatch(ST_FX, 0, BG, faceback);

   if (doublebufferflag==1)
     {
     V_CopyRect(ST_X, 0, BG, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y, 5);
     V_CopyRect(ST_X, 0, BG, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y, 6);
     }
   else
     {
     V_CopyRect(ST_X, 0, BG, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y, FG);
     }
    }

}


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder (event_t* ev)
{
  int		i;
    
  // Filter automap on/off.
  if (ev->type == ev_keyup
      && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
  {
    switch(ev->data1)
    {
      case AM_MSGENTERED:
	st_gamestate = AutomapState;
	st_firsttime = true;
	break;
	
      case AM_MSGEXITED:
	//	fprintf(stderr, "AM exited\n");
	st_gamestate = FirstPersonState;
	break;
    }
  }

  // if a user keypress...
  else if (ev->type == ev_keydown)
  {
    if (!netgame)
    {
      // b. - enabled for more debug fun.
      // if (gameskill != sk_nightmare) {
      
      // 'dqd' cheat for toggleable god mode
      if (cht_CheckCheat(&cheat_god, ev->data1))
      {
	plyr->cheats ^= CF_GODMODE;
	if (plyr->cheats & CF_GODMODE)
	{
	  if (plyr->mo)
	    plyr->mo->health = NORMHEALTH;
	  
	  plyr->health = NORMHEALTH;
	  plyr->message = STSTR_DQDON;
	}
	else 
	  plyr->message = STSTR_DQDOFF;
      }
      // 'fa' cheat for killer fucking arsenal
      else if (cht_CheckCheat(&cheat_ammonokey, ev->data1))
      {
	if (!plyr->backpack)
	{
	    for (i=0 ; i<NUMAMMO ; i++)
		plyr->maxammo[i] *= 2;
	    plyr->backpack = true;
	}
	plyr->armorpoints = CHEATARMOUR;
	plyr->armortype = CHEATARMOURT;
	
	for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	plyr->message = STSTR_FAADDED;
      }
      // 'kfa' cheat for key full ammo
      else if (cht_CheckCheat(&cheat_ammo, ev->data1))
      {
        if (!plyr->backpack)
	{
	    for (i=0 ; i<NUMAMMO ; i++)
		plyr->maxammo[i] *= 2;
	    plyr->backpack = true;
	}
        plyr->armorpoints = CHEATARMOUR;
	plyr->armortype = CHEATARMOURT;
	
        for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = true;
	
        for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];
	
	for (i=0;i<NUMCARDS;i++)
	  plyr->cards[i] = true;

        // refresh to add all stuff to status bar
        st_firsttime = true;

	plyr->message = STSTR_KFAADDED;
      }
      else if (cht_CheckCheat(&cheat_keys, ev->data1))
      {
        for (i=0;i<NUMCARDS;i++)
	  plyr->cards[i] = true;

        // refresh to remove all stuff from status bar
        st_firsttime = true;

        plyr->message = "Unlock any door!";
      }
      else if (cht_CheckCheat(&cheat_loaded, ev->data1))
      {
        for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = plyr->maxammo[i];

        plyr->message = "Loaded!";
      }
      else if (cht_CheckCheat(&cheat_takeall, ev->data1))
      {
        for (i=0;i<NUMAMMO;i++)
	  plyr->ammo[i] = 0;

        for (i=0;i<NUMCARDS;i++)
	  plyr->cards[i] = false;

        for (i=0;i<NUMWEAPONS;i++)
	  plyr->weaponowned[i] = false;

        plyr->weaponowned[wp_pistol] = true;
        plyr->ammo[am_clip] = 50;

	plyr->armorpoints = 0;
	plyr->armortype   = 0;

        if (plyr->backpack)
	{
	    for (i=0 ; i<NUMAMMO ; i++)
		plyr->maxammo[i] /= 2;
	    plyr->backpack = false;
	}

        plyr->pendingweapon = wp_pistol;

        // refresh to remove all stuff from status bar
        st_firsttime = true;

        plyr->message = "All Stuff Removed!";
      }
      else if (cht_CheckCheat(&cheat_suicide, ev->data1))
      {
        P_DamageMobj(plyr->mo,NULL,plyr->mo,10000);
        plyr->message = "Loser!";
      }
      else if (cht_CheckCheat(&cheat_killall, ev->data1))
      {
        int killcount=0;
        thinker_t* currentthinker;

        currentthinker = thinkercap.next;
        while (currentthinker != &thinkercap)
          {
          if ( (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
             && ((((((mobj_t *)currentthinker)->flags)&MF_COUNTKILL)==MF_COUNTKILL)||(((mobj_t *)currentthinker)->type==MT_SKULL))
             && ((((mobj_t *)currentthinker)->health)>0) )
               {
               P_DamageMobj((mobj_t *)currentthinker,NULL,NULL,10000);
               killcount++;
               }
          currentthinker = currentthinker->next;
          }
        sprintf(tmpmsgstring,"%d Monsters Killed",killcount);
        plyr->message = tmpmsgstring;
        }
      // Simplified, accepting both "noclip" and "idspispopd".
      // no clipping mode cheat
      else if ( cht_CheckCheat(&cheat_noclip, ev->data1) 
		|| cht_CheckCheat(&cheat_commercial_noclip,ev->data1) )
      {	
	plyr->cheats ^= CF_NOCLIP;
	
	if (plyr->cheats & CF_NOCLIP)
	  plyr->message = STSTR_NCON;
	else
	  plyr->message = STSTR_NCOFF;
      }

      // 'behold?' power-up cheats
      for (i=0;i<6;i++)
      {
	if (cht_CheckCheat(&cheat_powerup[i], ev->data1))
	{
	  if (!plyr->powers[i])
	    P_GivePower( plyr, i);
	  else if (i!=pw_strength)
	    plyr->powers[i] = 1;
	  else
	    plyr->powers[i] = 0;
	  
	  plyr->message = STSTR_BEHOLDX;
	}
      }
      
      // 'behold' power-up menu
      if (cht_CheckCheat(&cheat_powerup[6], ev->data1))
      {
	plyr->message = STSTR_BEHOLD;
      }

      // 'give?' power-up cheats
      for (i=1;i<8;i++)
      {
	if (cht_CheckCheat(&cheat_giveweapon[i], ev->data1))
	{
         switch (i)
         {
          case 1: // chainsaw
            plyr->weaponowned[wp_chainsaw] = true;
            plyr->message = GOTCHAINSAW;
            break;

          case 2: // shotgun
            plyr->weaponowned[wp_shotgun] = true;
            plyr->ammo[am_shell] = maxammo[am_shell];
            plyr->message = GOTSHOTGUN;
            break;

          case 3: // supershotgun
            plyr->weaponowned[wp_supershotgun] = true;
            plyr->ammo[am_shell] = maxammo[am_shell];
            plyr->message = GOTSHOTGUN2;
            break;

          case 4: // chaingun
            plyr->weaponowned[wp_chaingun] = true;
            plyr->ammo[am_clip] = maxammo[am_clip];
            plyr->message = GOTCHAINGUN;
            break;

          case 5: // missile
            plyr->weaponowned[wp_missile] = true;
            plyr->ammo[am_misl] = maxammo[am_misl];
            plyr->message = GOTLAUNCHER;
            break;

          case 6: // plasma rifle
            plyr->weaponowned[wp_plasma] = true;
            plyr->ammo[am_cell] = maxammo[am_cell];
            plyr->message = GOTPLASMA;
            break;

          case 7: // BFG9000
            plyr->weaponowned[wp_bfg] = true;
            plyr->ammo[am_cell] = maxammo[am_cell];
            plyr->message = GOTBFG9000;
            break;

          
          default: // this should not happen
            break;
         }
	}
      }

      // 'choppers' invulnerability & chainsaw
      if (cht_CheckCheat(&cheat_choppers, ev->data1))
      {
	plyr->weaponowned[wp_chainsaw] = true;
	plyr->powers[pw_invulnerability] = true;
	plyr->message = STSTR_CHOPPERS;
      }

      // 'mypos' for player position
      else if (cht_CheckCheat(&cheat_mypos, ev->data1))
      {
	static char	buf[ST_MSGWIDTH];
	sprintf(buf, "ang=0x%x;x,y=(0x%x,0x%x)",
		players[consoleplayer].mo->angle,
		players[consoleplayer].mo->x,
		players[consoleplayer].mo->y);
	plyr->message = buf;
      }
    }
    
    // 'clev' change-level cheat
    if (cht_CheckCheat(&cheat_clev, ev->data1))
    {
      char		buf[9];
      int		epsd;
      int		map;
      cht_GetParam(&cheat_clev, buf);

        if (gamemission != doom) // - Kester
        {
          epsd = 1;
          map = strtol(buf, NULL, 10);
          sprintf(buf, "MAP%02d", map);
        } else {
          epsd = strtol(buf, NULL, 16);
          map = epsd & 15;
          epsd >>= 4;
	  sprintf(buf, "E%xM%d", epsd, map);
        }

        if (W_CheckNumForName(buf) < 0) return false; // - Kester
      
        // So be it.
        plyr->message = STSTR_CLEV;
        G_DeferedInitNew(gamemission, gameskill, epsd, map);
    }    
  else if (cht_CheckCheat(&cheat_showstats, ev->data1))
      {
      showstats=!showstats;
      }
      // 'mus' cheat for changing music
  else if (cht_CheckCheat(&cheat_mus, ev->data1))
      {

#ifdef DJGPP
      if (cdaudio)
        {
        char buf[3];
        int temptrack;

        cht_GetParam(&cheat_mus, buf);
        temptrack=(buf[0]-'0')*10 + buf[1]-'0';
        CD_Play(temptrack, true);
        sprintf(tmpmsgstring,"Playing Track %d on CD",cdtrack);
        plyr->message = tmpmsgstring;
        }
      else
#endif

        {
        char	buf[3];
        int		musnum;
	
        plyr->message = STSTR_MUS;
        cht_GetParam(&cheat_mus, buf);
	
        if (gamemode == commercial)
          {
          musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;
	  
          if (((buf[0]-'0')*10 + buf[1]-'0') > 35)
            plyr->message = STSTR_NOMUS;
         else
            S_ChangeMusic(musnum, 1);
          }
        else
          {
          musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');
	  
          if (((buf[0]-'1')*9 + buf[1]-'1') > 31)
            plyr->message = STSTR_NOMUS;
          else
            S_ChangeMusic(musnum, 1);
          }
        }
      }

//the new cheat codes
#ifdef DJGPP
  else if (cht_CheckCheat(&cheat_cdnext, ev->data1))
        {
         if (!cdaudio)
          plyr->message = "CD Audio disabled";
         else
         {
          CD_Next();
          sprintf(tmpmsgstring,"Playing Track %d on CD",cdtrack);
          plyr->message = tmpmsgstring;
         }
        }
  else if (cht_CheckCheat(&cheat_cdprev, ev->data1))
        {
         if (!cdaudio)
          plyr->message = "CD Audio disabled";
         else
         {
          CD_Prev();
          sprintf(tmpmsgstring,"Playing Track %d on CD",cdtrack);
          plyr->message = tmpmsgstring;
         }
        }
#endif

  }
  return false;
}



int ST_calcPainOffset(void)
{
    int		health;
    static int	lastcalc;
    static int	oldhealth = -1;
    
    health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {
	lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
	oldhealth = health;
    }
    return lastcalc;
}

void ST_drawWidgets(boolean refresh)
{
    int		i;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch;

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 

    STlib_updateNum(&w_ready, refresh);

    for (i=0;i<4;i++)
    {
	STlib_updateNum(&w_ammo[i], refresh);
	STlib_updateNum(&w_maxammo[i], refresh);
    }

    STlib_updatePercent(&w_health, refresh);
    STlib_updatePercent(&w_armor, refresh);

    STlib_updateBinIcon(&w_armsbg, refresh);

    for (i=0;i<6;i++)
	STlib_updateMultIcon(&w_arms[i], refresh);

    STlib_updateMultIcon(&w_faces, refresh);

    for (i=0;i<3;i++)
	STlib_updateMultIcon(&w_keyboxes[i], refresh);

    STlib_updateNum(&w_frags, refresh);

}


//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget(void)
{
    int		i;
    angle_t	badguyangle;
    angle_t	diffang;
    static int	lastattackdown = -1;
    static int	priority = 0;
    boolean	doevilgrin;

    if (priority < 10)
    {
	// dead
	if (!plyr->health)
	{
	    priority = 9;
	    st_faceindex = ST_DEADFACE;
	    st_facecount = 1;
	}
    }

    if (priority < 9)
    {
	if (plyr->bonuscount)
	{
	    // picking up bonus
	    doevilgrin = false;

	    for (i=0;i<NUMWEAPONS;i++)
	    {
		if (oldweaponsowned[i] != plyr->weaponowned[i])
		{
		    doevilgrin = true;
		    oldweaponsowned[i] = plyr->weaponowned[i];
		}
	    }
	    if (doevilgrin) 
	    {
		// evil grin if just picked up weapon
		priority = 8;
		st_facecount = ST_EVILGRINCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
	    }
	}

    }
  
    if (priority < 8)
    {
	if (plyr->damagecount
	    && plyr->attacker
	    && plyr->attacker != plyr->mo)
	{
	    // being attacked
	    priority = 7;
	    
	    if (plyr->health - st_oldhealth > ST_MUCHPAIN)
	    {
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
	    }
	    else
	    {
		badguyangle = R_PointToAngle2(plyr->mo->x,
					      plyr->mo->y,
					      plyr->attacker->x,
					      plyr->attacker->y);
		
		if (badguyangle > plyr->mo->angle)
		{
		    // whether right or left
		    diffang = badguyangle - plyr->mo->angle;
		    i = diffang > ANG180; 
		}
		else
		{
		    // whether left or right
		    diffang = plyr->mo->angle - badguyangle;
		    i = diffang <= ANG180; 
		} // confusing, aint it?

		
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset();
		
		if (diffang < ANG45)
		{
		    // head-on    
		    st_faceindex += ST_RAMPAGEOFFSET;
		}
		else if (i)
		{
		    // turn face right
		    st_faceindex += ST_TURNOFFSET;
		}
		else
		{
		    // turn face left
		    st_faceindex += ST_TURNOFFSET+1;
		}
	    }
	}
    }
  
    if (priority < 7)
    {
	// getting hurt because of your own damn stupidity
	if (plyr->damagecount)
	{
	    if (plyr->health - st_oldhealth > ST_MUCHPAIN)
	    {
		priority = 7;
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
	    }
	    else
	    {
		priority = 6;
		st_facecount = ST_TURNCOUNT;
		st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
	    }

	}

    }
  
    if (priority < 6)
    {
	// rapid firing
	if (plyr->attackdown)
	{
	    if (lastattackdown==-1)
		lastattackdown = ST_RAMPAGEDELAY;
	    else if (!--lastattackdown)
	    {
		priority = 5;
		st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
		st_facecount = 1;
		lastattackdown = 1;
	    }
	}
	else
	    lastattackdown = -1;

    }
  
    if (priority < 5)
    {
	// invulnerability
	if ((plyr->cheats & CF_GODMODE)
	    || plyr->powers[pw_invulnerability])
	{
	    priority = 4;

	    st_faceindex = ST_GODFACE;
	    st_facecount = 1;

	}

    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
	st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
	st_facecount = ST_STRAIGHTFACECOUNT;
	priority = 0;
    }

    st_facecount--;

}

void ST_updateWidgets(void)
{
    static int	largeammo = 1994; // means "n/a"
    int		i;

    // must redirect the pointer if the ready weapon has changed.
    //  if (w_ready.data != plyr->readyweapon)
    //  {
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
	w_ready.num = &largeammo;
    else
	w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
    //{
    // static int tic=0;
    // static int dir=-1;
    // if (!(tic&15))
    //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
    // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
    //   dir = 1;
    // tic++;
    // }
    w_ready.data = plyr->readyweapon;

    // if (*w_ready.on)
    //  STlib_updateNum(&w_ready, true);
    // refresh weapon change
    //  }

    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
	keyboxes[i] = plyr->cards[i] ? i : -1;

	if (plyr->cards[i+3])
	    keyboxes[i] = i+3;
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !deathmatch;
    
    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch; 

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 
    st_fragscount = 0;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (i != consoleplayer)
	    st_fragscount += plyr->frags[i];
	else
	    st_fragscount -= plyr->frags[i];
    }

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
	st_chat = st_oldchat;

}

void ST_Ticker (void)
{

    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;

}

static int st_palette = 0;

void ST_doPaletteStuff(void)
{

    int		palette;
    byte*	pal;
    int		cnt;
    int		bzc;
    int redness;

    redness=0;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
	// slowly fade the berzerk out
  	bzc = 12 - (plyr->powers[pw_strength]>>6);

	if (bzc > cnt)
	    cnt = bzc;
    }
	
    if (cnt)
    {
	palette = (cnt+7)>>3;
	
	if (palette >= NUMREDPALS)
	    palette = NUMREDPALS-1;

   redness=palette;
   palette += STARTREDPALS;
    }

    else if (plyr->bonuscount)
    {
	palette = (plyr->bonuscount+7)>>3;

	if (palette >= NUMBONUSPALS)
	    palette = NUMBONUSPALS-1;

	palette += STARTBONUSPALS;
    }

    else if ( plyr->powers[pw_ironfeet] > 4*32
	      || plyr->powers[pw_ironfeet]&8)
	palette = RADIATIONPAL;
    else
	palette = 0;

    if (palette != st_palette)
    {
	st_palette = palette;
	pal = (byte *) W_CacheLumpNum (lu_palette, PU_CACHE)+palette*768;
	I_SetPalette (pal,redness);
    }

}


void ST_doRefresh(void)
{

    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);

}

void ST_diffDraw(void)
{


    // update all widgets
    ST_drawWidgets(false);
}

void ST_Drawer (boolean fullscreen, boolean refresh)
{
  
    st_statusbaron = (!fullscreen) || automapactive;
    st_firsttime = st_firsttime || refresh;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // If just after ST_Start(), refresh all
    if (st_firsttime||newhupd)
      {
      newhupd=false;
      ST_doRefresh();
      }
    // Otherwise, update as little as possible
    else ST_diffDraw();

}

void ST_loadGraphics(void)
{

    int		i;
    int		j;
    int		facenum;
    
    char	namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
	sprintf(namebuf, "STTNUM%d", i);
	tallnum[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

	sprintf(namebuf, "STYSNUM%d", i);
	shortnum[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?
    tallpercent = (patch_t *) W_CacheLumpName("STTPRCNT", PU_STATIC);

    // key cards
    for (i=0;i<NUMCARDS;i++)
    {
	sprintf(namebuf, "STKEYS%d", i);
	keys[i] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);
    }

    // arms background
    armsbg = (patch_t *) W_CacheLumpName("STARMS", PU_STATIC);

    // arms ownership widgets
    for (i=0;i<6;i++)
    {
	sprintf(namebuf, "STGNUM%d", i+2);

	// gray #
	arms[i][0] = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

	// yellow #
	arms[i][1] = shortnum[i+2]; 
    }

    // face backgrounds for different color players
    sprintf(namebuf, "STFB%d", consoleplayer);
    faceback = (patch_t *) W_CacheLumpName(namebuf, PU_STATIC);

    // status bar background bits
    sbar = (patch_t *) W_CacheLumpName("STBAR", PU_STATIC);

    // face states
    facenum = 0;
    for (i=0;i<ST_NUMPAINFACES;i++)
    {
	for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
	{
	    sprintf(namebuf, "STFST%d%d", i, j);
	    faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
	}
	sprintf(namebuf, "STFTR%d0", i);	// turn right
	faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
	sprintf(namebuf, "STFTL%d0", i);	// turn left
	faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
	sprintf(namebuf, "STFOUCH%d", i);	// ouch!
	faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
	sprintf(namebuf, "STFEVL%d", i);	// evil grin ;)
	faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
	sprintf(namebuf, "STFKILL%d", i);	// pissed off
	faces[facenum++] = W_CacheLumpName(namebuf, PU_STATIC);
    }
    faces[facenum++] = W_CacheLumpName("STFGOD0", PU_STATIC);
    faces[facenum++] = W_CacheLumpName("STFDEAD0", PU_STATIC);

}

void ST_loadData(void)
{
    lu_palette = W_GetNumForName ("PLAYPAL");
    ST_loadGraphics();
}

void ST_unloadGraphics(void)
{

    int i;

    // unload the numbers, tall and short
    for (i=0;i<10;i++)
    {
	Z_ChangeTag(tallnum[i], PU_CACHE);
	Z_ChangeTag(shortnum[i], PU_CACHE);
    }
    // unload tall percent
    Z_ChangeTag(tallpercent, PU_CACHE); 

    // unload arms background
    Z_ChangeTag(armsbg, PU_CACHE); 

    // unload gray #'s
    for (i=0;i<6;i++)
	Z_ChangeTag(arms[i][0], PU_CACHE);
    
    // unload the key cards
    for (i=0;i<NUMCARDS;i++)
	Z_ChangeTag(keys[i], PU_CACHE);

    Z_ChangeTag(sbar, PU_CACHE);
    Z_ChangeTag(faceback, PU_CACHE);

    for (i=0;i<ST_NUMFACES;i++)
	Z_ChangeTag(faces[i], PU_CACHE);

    // Note: nobody ain't seen no unloading
    //   of stminus yet. Dude.
    

}

void ST_unloadData(void)
{
    ST_unloadGraphics();
}

void ST_initData(void)
{

    int		i;

    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_clock = 0;
    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
	oldweaponsowned[i] = plyr->weaponowned[i];

    for (i=0;i<3;i++)
	keyboxes[i] = -1;

    STlib_init();

}



void ST_createWidgets(void)
{

    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready,
		  ST_AMMOX,
		  ST_AMMOY,
		  tallnum,
		  &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
		  &st_statusbaron,
		  ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // health percentage
    STlib_initPercent(&w_health,
		      ST_HEALTHX,
		      ST_HEALTHY,
		      tallnum,
		      &plyr->health,
		      &st_statusbaron,
		      tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg,
		      ST_ARMSBGX,
		      ST_ARMSBGY,
		      armsbg,
		      &st_notdeathmatch,
		      &st_statusbaron);

    // weapons owned
    for(i=0;i<6;i++)
    {
	STlib_initMultIcon(&w_arms[i],
			   ST_ARMSX+(i%3)*ST_ARMSXSPACE,
			   ST_ARMSY+(i/3)*ST_ARMSYSPACE,
			   arms[i], (int *) &plyr->weaponowned[i+1],
			   &st_armson);
    }

    // frags sum
    STlib_initNum(&w_frags,
		  ST_FRAGSX,
		  ST_FRAGSY,
		  tallnum,
		  &st_fragscount,
		  &st_fragson,
		  ST_FRAGSWIDTH);

    // faces
    STlib_initMultIcon(&w_faces,
		       ST_FACESX,
		       ST_FACESY,
		       faces,
		       &st_faceindex,
		       &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
		      ST_ARMORX,
		      ST_ARMORY,
		      tallnum,
		      &plyr->armorpoints,
		      &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0],
		       ST_KEY0X,
		       ST_KEY0Y,
		       keys,
		       &keyboxes[0],
		       &st_statusbaron);
    
    STlib_initMultIcon(&w_keyboxes[1],
		       ST_KEY1X,
		       ST_KEY1Y,
		       keys,
		       &keyboxes[1],
		       &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[2],
		       ST_KEY2X,
		       ST_KEY2Y,
		       keys,
		       &keyboxes[2],
		       &st_statusbaron);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0],
		  ST_AMMO0X,
		  ST_AMMO0Y,
		  shortnum,
		  &plyr->ammo[0],
		  &st_statusbaron,
		  ST_AMMO0WIDTH);

    STlib_initNum(&w_ammo[1],
		  ST_AMMO1X,
		  ST_AMMO1Y,
		  shortnum,
		  &plyr->ammo[1],
		  &st_statusbaron,
		  ST_AMMO1WIDTH);

    STlib_initNum(&w_ammo[2],
		  ST_AMMO2X,
		  ST_AMMO2Y,
		  shortnum,
		  &plyr->ammo[2],
		  &st_statusbaron,
		  ST_AMMO2WIDTH);
    
    STlib_initNum(&w_ammo[3],
		  ST_AMMO3X,
		  ST_AMMO3Y,
		  shortnum,
		  &plyr->ammo[3],
		  &st_statusbaron,
		  ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0],
		  ST_MAXAMMO0X,
		  ST_MAXAMMO0Y,
		  shortnum,
		  &plyr->maxammo[0],
		  &st_statusbaron,
		  ST_MAXAMMO0WIDTH);

    STlib_initNum(&w_maxammo[1],
		  ST_MAXAMMO1X,
		  ST_MAXAMMO1Y,
		  shortnum,
		  &plyr->maxammo[1],
		  &st_statusbaron,
		  ST_MAXAMMO1WIDTH);

    STlib_initNum(&w_maxammo[2],
		  ST_MAXAMMO2X,
		  ST_MAXAMMO2Y,
		  shortnum,
		  &plyr->maxammo[2],
		  &st_statusbaron,
		  ST_MAXAMMO2WIDTH);
    
    STlib_initNum(&w_maxammo[3],
		  ST_MAXAMMO3X,
		  ST_MAXAMMO3Y,
		  shortnum,
		  &plyr->maxammo[3],
		  &st_statusbaron,
		  ST_MAXAMMO3WIDTH);

}

static boolean	st_stopped = true;


void ST_Start (void)
{

    if (!st_stopped)
	ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;

}

void ST_Stop (void)
{
    if (st_stopped)
	return;

    I_SetPalette (W_CacheLumpNum (lu_palette, PU_CACHE),0);

    st_stopped = true;
}

void ST_Init (void)
{
    veryfirsttime = 0;
    ST_loadData();
    screens[4] = (byte *) Z_Malloc(SCREENWIDTH*ST_HEIGHT*BPP, PU_STATIC, 0);
}
