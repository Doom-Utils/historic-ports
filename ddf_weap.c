//
// DOSDoom Definition File Codes (Weapons)
//
// By the DOSDoom Team
//
// Player Weapons Setup and Parser Code
//
// -KM- 1998/11/25 File Written
//
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "z_zone.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

static void DDF_WGetAmmo(char* info, int c);
static void DDF_WGetBindKey(char* info, int c);
static void DDF_WGetWeapon(char* info, int c);

static weaponinfo_t bufferweapon;
static weaponinfo_t* replace;
commandlist_t weaponcommands[] =
 {{"AMMOTYPE"           , DDF_WGetAmmo   ,  &bufferweapon.ammo},
  {"AMMOPERSHOT"        , DDF_MainGetNumeric, &bufferweapon.ammopershot},
  {"UPSTATE"            , DDF_MainLoadStates ,  &bufferweapon.upstate},
  {"DOWNSTATE"            , DDF_MainLoadStates ,  &bufferweapon.downstate},
  {"READYSTATE"            , DDF_MainLoadStates ,  &bufferweapon.readystate},
  {"ATTACKSTATE"            , DDF_MainLoadStates ,  &bufferweapon.atkstate},
  {"FLASHSTATE"            , DDF_MainLoadStates ,  &bufferweapon.flashstate},
  {"RELOADSTATE"            , DDF_MainLoadStates ,  &bufferweapon.reloadstate},
  {"AUTOMATIC"              , DDF_MainGetBoolean, &bufferweapon.autofire},
  {"FREE"                   , DDF_MainGetBoolean, &bufferweapon.autogive},
  {"ATTACK"                 , DDF_MainRefAttack, &bufferweapon.attack},
  {"BINDKEY"                , DDF_WGetBindKey, NULL},
  {"PRIORITY"               , DDF_MainGetNumeric, &bufferweapon.priority},
  {"UPGRADES"               , DDF_WGetWeapon, &bufferweapon.replaces},
  {"IDLE SOUND"             , DDF_MainLookupSound, &bufferweapon.idle},
  {"ENGAGED SOUND"             , DDF_MainLookupSound, &bufferweapon.engaged},
  {"HIT SOUND"             , DDF_MainLookupSound, &bufferweapon.hit},
  {"START SOUND"             , DDF_MainLookupSound, &bufferweapon.start},
  {"SOUND1"             , DDF_MainLookupSound, &bufferweapon.sound1},
  {"SOUND2"             , DDF_MainLookupSound, &bufferweapon.sound2},
  {"SOUND3"             , DDF_MainLookupSound, &bufferweapon.sound3},
  {"NOTHRUST"           , DDF_MainGetBoolean, &bufferweapon.nothrust},
  {"FEEDBACK"           , DDF_MainGetBoolean, &bufferweapon.feedback},
  {"KICK"               , DDF_MainGetFixed,   &bufferweapon.kick},
  {"CLIPSIZE"           , DDF_MainGetNumeric, &bufferweapon.clip},
  {COMMAND_TERMINATOR   , NULL,                 NULL}};

struct
{
  char* name;
  ammotype_t val;
} ammotypes[] = {
 {"NOAMMO", am_noammo},
 {"BULLETS", am_clip},
 {"SHELLS", am_shell},
 {"ROCKETS", am_misl},
 {"CELLS", am_cell}
};

int NUMAMMO = 4;
int numweapons = 0;
weaponinfo_t* weaponinfo = NULL;

static void DDF_WGetAmmo(char* info, int c)
{
   int i;
   for (i = sizeof(ammotypes)/sizeof(ammotypes[0]); i--;)
   {
     if (!strcmp(info, ammotypes[i].name))
     {
       *(ammotype_t *)weaponcommands[c].data = ammotypes[i].val;
       return;
     }
   }
   i = atoi (info);
   if (!i)
     I_Error("\nUnknown Ammo type '%s'\n", info);
   else
   {
     *(int *)weaponcommands[c].data = i;
     if (i >= NUMAMMO)
       NUMAMMO = i + 1;
   }
}

static void DDF_WGetBindKey(char* info, int c)
{
   int key = atoi(info);
   weaponkey_t* w = &weaponkey[key];

   w->choice = realloc(w->choice, (1+w->numchoices)*sizeof(int));
   if (replace)
   {
      int i, j;
      int oldweapon = replace - weaponinfo;
      for (i = 0; i < 10; i++)
      {
         for (j = 0; j < weaponkey[i].numchoices; j++)
         {
           if (weaponkey[i].choice[j] == oldweapon)
             weaponkey[i].choice[j] = -1;
         }
      }
      w->choice[w->numchoices++] = oldweapon;
   } else
     w->choice[w->numchoices++] = numweapons;
}

static void DDF_WGetName(char* info)
{
  int i;

#ifdef DEVELOPERS
  if (!info)
    I_Error("\ninfo has no info!\n");
#endif
  replace = NULL;

  for (i = 0; i < numweapons; i++)
  {
     if (!strcmp(weaponinfo[i].name, info))
     {
       if (ddf_replace)
         replace = &weaponinfo[i];
       else
         I_Error("\nRedeclaration of Weapon type '%s'\n", info);
     }
  }

  bufferweapon.name = strdup(info);
}

static void DDF_WCreate(void)
{
  state_t* st;

  // Clean up states:  after redirect attack state to readystate
  st = &states[bufferweapon.atkstate];
  while (st->nextstate != bufferweapon.atkstate) st++;

  st->nextstate = bufferweapon.readystate;

  // And reload state to ready state if it exists.
  if (bufferweapon.reloadstate)
  {
    st = &states[bufferweapon.reloadstate];
    while (st->nextstate != bufferweapon.reloadstate) st++;
  
    st->nextstate = bufferweapon.readystate;
  }
  if (replace)
    memcpy(replace, &bufferweapon, sizeof(weaponinfo_t));
  else
  {
    weaponinfo = realloc(weaponinfo, (numweapons+1)*sizeof(weaponinfo_t));
  
    memcpy(&weaponinfo[numweapons++], &bufferweapon, sizeof(weaponinfo_t));
  }
  memset(&bufferweapon, 0, sizeof(weaponinfo_t));
  bufferweapon.replaces = -1;
  bufferweapon.clip = 1;
}

void DDF_ReadWeapons(void *data, int size)
{
  readinfo_t weapons;
  bufferweapon.replaces = -1;
  bufferweapon.clip = 1;

  if (!data)
  {
    weapons.message               = "DDF_InitWeapons";
    weapons.filename              = "weapons.ddf";
    weapons.memfile = NULL;
  } else {
    weapons.message = NULL;
    weapons.memfile = data;
    weapons.memsize = size;
    weapons.filename = NULL;
  }
  weapons.DDF_MainCheckName     = DDF_WGetName;
  weapons.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  weapons.DDF_MainCreateEntry   = DDF_WCreate;
  weapons.DDF_MainFinishingCode = DDF_WCreate;
  weapons.cmdlist               = weaponcommands;

  DDF_MainReadFile(&weapons);
}

void DDF_WeaponInit()
{
  DDF_ReadWeapons(NULL, 0);
}

weapontype_t DDF_WeaponGetType(char* name)
{
  int i;
  for (i = 0; i < numweapons; i++)
  {
    if (!strcmp(name, weaponinfo[i].name))
      return i;
  }
  return -1;
}

static void DDF_WGetWeapon(char* info, int c)
{
  *(weapontype_t*) weaponcommands[c].data = DDF_WeaponGetType(info);
}
