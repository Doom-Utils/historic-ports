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
//	Cheat sequence checking.
//
//-----------------------------------------------------------------------------
// -KM- 1998/07/21 Moved the cheat sequence here from st_stuff.c
//                 ST_Responder in st_stuff.c calls cht_Responder to check for
//                 cheat codes.  Also added NO_NIGHTMARE_CHEATS #define.
//                 if defined, there can be no cheating in nightmare. :-)
//                 Made all the cheat codes non global.
//
// -ACB- 1998/07/30 Naming Convention stuff, all procedures m_*.*.
//                  Added Touching Mobj "Cheat".
//


#include <stdlib.h>

#include "dstrings.h"
#include "m_fixed.h"
#include "ddf_main.h"
#include "g_game.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_menu.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_mobj.h"
#include "w_wad.h"
#include "z_zone.h"

#ifdef DJGPP
// CD Player controls.
#include "i_music.h"
#endif

//
// CHEAT SEQUENCE PACKAGE
//
// This is so hackers couldn't discover the cheat codes.
#define SCRAMBLE(a) (a)

static int		firsttime = 1;
static unsigned char	cheat_xlate_table[256];

// Massive bunches of cheat stuff to keep it from being easy to figure them out.
// Yeah, right...
static unsigned char	cheat_mus_seq[] =
{
'i','d','m','u','s',0xff
};

static unsigned char	cheat_choppers_seq[] =
{
'i','d','c','h','o','p','p','e','r','s',0xff
};

static unsigned char	cheat_god_seq[] =
{
'i','d','d','q','d',0xff
};

static unsigned char	cheat_ammo_seq[] =
{
'i','d','k','f','a',0xff
};

static unsigned char	cheat_ammonokey_seq[] =
{
'i','d','f','a',0xff
};


// Smashing Pumpkins Into Small Piles Of Putried Debris.
static unsigned char	cheat_noclip_seq[] =
{
'i','d','s','p','i','s','p','o','p','d',0xff
};

//
static unsigned char	cheat_commercial_noclip_seq[] =
{
'i','d','c','l','i','p',0xff
}; 


// -MH- 1998/06/17  added "idbeholdj" to give jetpack
// -ACB- 1998/07/15 added "idbeholdi" to give nightvision
static unsigned char   cheat_powerup_seq[NUMPOWERS+1][10] =
{
    {'i','d','b','e','h','o','l','d','v',0xff},
    {'i','d','b','e','h','o','l','d','s',0xff},
    {'i','d','b','e','h','o','l','d','i',0xff},
    {'i','d','b','e','h','o','l','d','r',0xff},
    {'i','d','b','e','h','o','l','d','a',0xff},
    {'i','d','b','e','h','o','l','d','l',0xff},
    {'i','d','b','e','h','o','l','d','j',0xff},
    {'i','d','b','e','h','o','l','d','n',0xff},
    {'i','d','b','e','h','o','l','d',0xff}
};


static unsigned char	cheat_clev_seq[] =
{
'i','d','c','l','e','v',0xff
};

// my position cheat
static unsigned char	cheat_mypos_seq[] =
{
'i','d','m','y','p','o','s',0xff
}; 

//
// DOSDoom Cheats
//

// CD Next Track
static unsigned char	cheat_cdnext_seq[] =
{
'c','d','n','e','x','t',0xff
};

// CD Previous Track
static unsigned char	cheat_cdprev_seq[] =
{
'c','d','p','r','e','v',0xff
};

// Kill all monsters
static unsigned char	cheat_killall_seq[] =
{
'i','d','k','i','l','l','a','l','l',0xff
};

// Suicide
static unsigned char	cheat_suicide_seq[] =
{
'i','d','s','u','i','c','i','d','e',0xff
};

// show player pos and level info
static unsigned char	cheat_showstats_seq[] =
{
'i','d','i','n','f','o',0xff
}; 

// give all the keys to me!
static unsigned char	cheat_keys_seq[] =
{
'i','d','u','n','l','o','c','k',0xff
}; 

// show me the ammo!
static unsigned char	cheat_loaded_seq[] =
{
'i','d','l','o','a','d','e','d',0xff
}; 

// removes weapons, ammos and keys
static unsigned char	cheat_takeall_seq[] =
{
'i','d','t','a','k','e','a','l','l',0xff
};

// toggles the shootable flag on the player mobj
// -ACB- 1998/08/26 Don't target player cheat
/* static unsigned char	cheat_noshoot_seq[] =
{
'i','d','d','u','f','u','s',0xff
}; */

// give weapons
static unsigned char	cheat_giveweapon_seq[8][8] =
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
static cheatseq_t	cheat_mus = { cheat_mus_seq, 0 };
static cheatseq_t	cheat_god = { cheat_god_seq, 0 };
static cheatseq_t	cheat_ammo = { cheat_ammo_seq, 0 };
static cheatseq_t	cheat_ammonokey = { cheat_ammonokey_seq, 0 };
static cheatseq_t	cheat_noclip = { cheat_noclip_seq, 0 };
static cheatseq_t	cheat_commercial_noclip = { cheat_commercial_noclip_seq, 0 };
static cheatseq_t       cheat_powerup[NUMPOWERS+1] =
{
    { cheat_powerup_seq[0], 0 },
    { cheat_powerup_seq[1], 0 },
    { cheat_powerup_seq[2], 0 },
    { cheat_powerup_seq[3], 0 },
    { cheat_powerup_seq[4], 0 },
    { cheat_powerup_seq[5], 0 },
    { cheat_powerup_seq[6], 0 },
    { cheat_powerup_seq[7], 0 }, // -MH- 1998/06/17  added "give jetpack" cheat
    { cheat_powerup_seq[8], 0 }  // -ACB- 1998/07/15  added "give nightvision" cheat
};

static cheatseq_t	cheat_choppers = { cheat_choppers_seq, 0 };
static cheatseq_t	cheat_clev = { cheat_clev_seq, 0 };
static cheatseq_t	cheat_mypos = { cheat_mypos_seq, 0 };

//new cheats
static cheatseq_t	cheat_cdnext = { cheat_cdnext_seq, 0 };
static cheatseq_t	cheat_cdprev = { cheat_cdprev_seq, 0 };
static cheatseq_t	cheat_killall = { cheat_killall_seq, 0 };
static cheatseq_t	cheat_showstats = { cheat_showstats_seq, 0 };
static cheatseq_t	cheat_suicide = { cheat_suicide_seq, 0 };
static cheatseq_t	cheat_keys = { cheat_keys_seq, 0 };
static cheatseq_t	cheat_loaded = { cheat_loaded_seq, 0 };
static cheatseq_t	cheat_takeall = { cheat_takeall_seq, 0 };
//static cheatseq_t	cheat_noshoot = { cheat_noshoot_seq, 0 };

static cheatseq_t	cheat_giveweapon[8] =
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

//for cdaudio
static char msgstring[40];

//
// Called in M_CheatResponder module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
int M_CheckCheat (cheatseq_t* cht, char key)
{
    int i;
    int rc = 0;

    if (firsttime)
    {
	firsttime = 0;
	for (i=0;i<256;i++) cheat_xlate_table[i] = SCRAMBLE(i);
    }

    if (!cht->p)
	cht->p = cht->sequence; // initialize if first time

    if (*cht->p == 0)
	*(cht->p++) = key;
    else if
	(cheat_xlate_table[(unsigned char)key] == *cht->p) cht->p++;
    else
	cht->p = cht->sequence;

    if (*cht->p == 1)
	cht->p++;
    else if (*cht->p == 0xff) // end of sequence character
    {
	cht->p = cht->sequence;
	rc = 1;
    }

    return rc;
}

void M_GetCheatParam (cheatseq_t* cht, char* buffer)
{

    unsigned char *p, c;

    p = cht->sequence;
    while (*(p++) != 1);
    
    do
    {
	c = *p;
	*(buffer++) = c;
	*(p++) = 0;
    }
    while (c && *p!=0xff );

    if (*p==0xff)
	*buffer = 0;

}

static void M_ChangeLevelCheat(char *string)
{
  // User pressed <ESC>
  if (!string) return;

  if (G_DeferedInitNew(gameskill,string))
  {
    players[consoleplayer].message = DDF_LanguageLookup("ImpossibleChange");
    return;
  }

  players[consoleplayer].message = DDF_LanguageLookup("LevelChange");
  Z_Free(string);
}

static void M_ChangeCDMusicCheat(char *string)
{
  int track;
  if (!string) return;
  track = atoi(string);
  if (!track)
  {
    players[consoleplayer].message = DDF_LanguageLookup("ImpossibleChange");
    Z_Free(string);
    return;
  }
  // -KM- 1998/12/17 Don't play this track forever...
  CD_Play(track, false);
  sprintf(msgstring,DDF_LanguageLookup("CDPlayTrack"),cdtrack);
  players[consoleplayer].message = msgstring;
  Z_Free(string);
}

static void M_ChangeMusicCheat(char *string)
{
  if (string)
  {
    char musString[strlen(string) + 3];
    sprintf(musString, "D_%s", string);
    if (W_CheckNumForName(musString) >= 0)
    {
      S_ChangeMusic(musString, true);
      players[consoleplayer].message = DDF_LanguageLookup("MusChange");
    }
    else
#ifdef DJGPP
    if (cdaudio)
    {
      M_ChangeCDMusicCheat(string);
      return;
    }
    else
#endif
    {
      players[consoleplayer].message = DDF_LanguageLookup("ImpossibleChange");
    }

    Z_Free(string);
  }
}

boolean M_CheatResponder (event_t *ev)
{
  int i;
#ifdef NOCHEATS
  return false
#endif

  //mobj_t* playermobj;

  // if a user keypress...
  if (ev->type == ev_keydown)
  {
    if (!netgame)
    {
     if (!(gameflags.cheats))
       return false;

      // 'dufus' - toggle the shootable flag on the player mobj
      // -ACB- 1998/08/26 Don't target player cheat
      /* if (M_CheckCheat(&cheat_noshoot, ev->data1))
      {
        playermobj = players[consoleplayer].mo;

        if (playermobj->info->extendedflags & EF_NEVERTARGET)
        {
	  players[consoleplayer].message = DDF_LanguageLookup("PlayerTargetingOff");
          playermobj->info->extendedflags &= ~EF_NEVERTARGET;
        }
        else
        {
	  players[consoleplayer].message = DDF_LanguageLookup("PlayerTargetingOn");
          playermobj->info->extendedflags |= EF_NEVERTARGET;
        }
      }  */

      // 'dqd' cheat for toggleable god mode
      if (M_CheckCheat(&cheat_god, ev->data1))
      {
	players[consoleplayer].cheats ^= CF_GODMODE;
	if (players[consoleplayer].cheats & CF_GODMODE)
	{
	  if (players[consoleplayer].mo)
	    players[consoleplayer].mo->health = NORMHEALTH;
	  
	  players[consoleplayer].health = NORMHEALTH;
	  players[consoleplayer].message = DDF_LanguageLookup("GodModeOn");
	}
	else 
	  players[consoleplayer].message = DDF_LanguageLookup("GodModeOff");
      }
      // 'fa' cheat for killer fucking arsenal
      //
      // -ACB- 1998/06/26 removed backpack from this as backpack is variable
      //
      else if (M_CheckCheat(&cheat_ammonokey, ev->data1))
      {
        // -ACB- 1998/06/27 update weapons widget
        weaponupdate = true;

        players[consoleplayer].armorpoints = CHEATARMOUR;
	players[consoleplayer].armortype = CHEATARMOURT;
	
        // -KM- 1998/11/25 Weapons generalised
	for (i=0;i<numweapons;i++)
	  players[consoleplayer].weaponowned[i] = true;
	
	for (i=0;i<NUMAMMO;i++)
	  players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];
	
	players[consoleplayer].message = DDF_LanguageLookup("AmmoAdded");
      }
      // 'kfa' cheat for key full ammo
      //
      // -ACB- 1998/06/26 removed backpack from this as backpack is variable
      //
      else if (M_CheckCheat(&cheat_ammo, ev->data1))
      {
        // -ACB- 1998/06/27 update weapons widget
        weaponupdate = true;

        players[consoleplayer].armorpoints = CHEATARMOUR;
	players[consoleplayer].armortype = CHEATARMOURT;
	
        for (i=0;i<numweapons;i++)
	  players[consoleplayer].weaponowned[i] = true;
	
        for (i=0;i<NUMAMMO;i++)
	  players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];
	
	for (i=0;i<NUMCARDS;i++)
	  players[consoleplayer].cards[i] = true;

        // refresh to add all stuff to status bar
        newhupd = true;

	players[consoleplayer].message = DDF_LanguageLookup("VeryHappyAmmo");
      }
      else if (M_CheckCheat(&cheat_keys, ev->data1))
      {
        for (i=0;i<NUMCARDS;i++)
	  players[consoleplayer].cards[i] = true;

        // refresh to remove all stuff from status bar
        newhupd = true;

        players[consoleplayer].message = DDF_LanguageLookup("UnlockCheat");
      }
      else if (M_CheckCheat(&cheat_loaded, ev->data1))
      {
        for (i=0;i<NUMAMMO;i++)
	  players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];

        players[consoleplayer].message = DDF_LanguageLookup("LoadedCheat");
      }
      else if (M_CheckCheat(&cheat_takeall, ev->data1))
      {
        // -ACB- 1998/06/27 update weapons widget
        weaponupdate = true;

        for (i=0;i<NUMAMMO;i++)
	  players[consoleplayer].ammo[i] = 0;

        for (i=0;i<NUMCARDS;i++)
	  players[consoleplayer].cards[i] = false;

        // -KM- 1998/11/25 Weapons generalised
        for (i=0;i<numweapons;i++)
          players[consoleplayer].weaponowned[i] = weaponinfo[i].autogive;
        players[consoleplayer].ammo[am_clip] = 50;

	players[consoleplayer].armorpoints = 0;
	players[consoleplayer].armortype   = 0;

        players[consoleplayer].pendingweapon = DDF_WeaponGetType("PISTOL");

        // refresh to remove all stuff from status bar
        newhupd = true;

        // -ACB- 1998/08/26 Stuff removed language reference
        players[consoleplayer].message = DDF_LanguageLookup("StuffRemoval");
      }
      else if (M_CheckCheat(&cheat_suicide, ev->data1))
      {
        P_DamageMobj(players[consoleplayer].mo,NULL,players[consoleplayer].mo,10000);

        // -ACB- 1998/08/26 Suicide language reference
        players[consoleplayer].message = DDF_LanguageLookup("SuicideCheat");
      }
      // -ACB- 1998/08/27 Used Mobj linked-list code, much cleaner.
      else if (M_CheckCheat(&cheat_killall, ev->data1))
      {
        int killcount=0;
        mobj_t* currmobj;

        currmobj = mobjlisthead;

        while (currmobj != NULL)
        {
          if ((currmobj->flags & MF_COUNTKILL) && (currmobj->health > 0))
          {
             P_DamageMobj(currmobj,NULL,NULL,10000);
             killcount++;
          }
          currmobj = currmobj->next;
        }

        sprintf(msgstring,DDF_LanguageLookup("MonstersKilled"),killcount);
        players[consoleplayer].message = msgstring;
      }
      // Simplified, accepting both "noclip" and "idspispopd".
      // no clipping mode cheat
      else if ( M_CheckCheat(&cheat_noclip, ev->data1) 
		|| M_CheckCheat(&cheat_commercial_noclip,ev->data1) )
      {	
	players[consoleplayer].cheats ^= CF_NOCLIP;
	
	if (players[consoleplayer].cheats & CF_NOCLIP)
	  players[consoleplayer].message = DDF_LanguageLookup("ClipOn");
	else
	  players[consoleplayer].message = DDF_LanguageLookup("ClipOff");
      }

      // 'behold?' power-up cheats
      for (i=0;i<NUMPOWERS;i++)
      {
	if (M_CheckCheat(&cheat_powerup[i], ev->data1))
	{
	  if (!players[consoleplayer].powers[i])
            P_GivePower(&players[consoleplayer], i, 100, 100 );
	  else if (i!=pw_strength)
	    players[consoleplayer].powers[i] = 1;
	  else
	    players[consoleplayer].powers[i] = 0;
	  
	  players[consoleplayer].message = DDF_LanguageLookup("BeholdUsed");
	}
      }
      
      // 'behold' power-up menu
      if (M_CheckCheat(&cheat_powerup[NUMPOWERS], ev->data1))
      {
	players[consoleplayer].message = DDF_LanguageLookup("BeholdNote");
      }

      // 'give?' power-up cheats
      for (i=1;i<8;i++)
      {
	if (M_CheckCheat(&cheat_giveweapon[i], ev->data1))
	{
         // -ACB- 1998/06/27 update weapons widget
         weaponupdate = true;

         players[consoleplayer].weaponowned[i] = true;
         players[consoleplayer].ammo[weaponinfo[i].ammo] = maxammo[weaponinfo[i].ammo];
	}
      } 

      // 'choppers' invulnerability & chainsaw
      if (M_CheckCheat(&cheat_choppers, ev->data1))
      {
        // -ACB- 1998/06/27 update weapons widget
        weaponupdate = true;

        players[consoleplayer].weaponowned[DDF_WeaponGetType("CHAINSAW")] = true;
	players[consoleplayer].powers[pw_invulnerability] = true;
	players[consoleplayer].message = DDF_LanguageLookup("CHOPPERSNote");
      }

      // 'mypos' for player position
      else if (M_CheckCheat(&cheat_mypos, ev->data1))
      {
	sprintf(msgstring, "ang=0x%x;x,y=(0x%x,0x%x)",
		players[consoleplayer].mo->angle,
		players[consoleplayer].mo->x,
		players[consoleplayer].mo->y);
	players[consoleplayer].message = msgstring;
      }
    
      // 'clev' change-level cheat
      if (M_CheckCheat(&cheat_clev, ev->data1))
        M_StartMessageInput(DDF_LanguageLookup("LevelQ"), (void *) M_ChangeLevelCheat);

      else if (M_CheckCheat(&cheat_showstats, ev->data1))
        showstats=!showstats;

      // 'mus' cheat for changing music
      else if (M_CheckCheat(&cheat_mus, ev->data1))
        M_StartMessageInput(DDF_LanguageLookup("MusicQ"), (void *) M_ChangeMusicCheat);

//CD cheat codes
#ifdef DJGPP
        else if (M_CheckCheat(&cheat_cdnext, ev->data1))
        {
         if (!cdaudio)
          players[consoleplayer].message = DDF_LanguageLookup("CDdisabled");
         else
         {
          CD_Next();
          sprintf(msgstring, DDF_LanguageLookup("CDPlayTrack"), cdtrack);
          players[consoleplayer].message = msgstring;
         }
        }
        else if (M_CheckCheat(&cheat_cdprev, ev->data1))
        {
         if (!cdaudio)
          players[consoleplayer].message = DDF_LanguageLookup("CDdisabled");
         else
         {
          CD_Prev();
          sprintf(msgstring, DDF_LanguageLookup("CDPlayTrack"), cdtrack);
          players[consoleplayer].message = msgstring;
         }
       }
      }
#endif
  }
  return false;
}

