//
// DOSDoom Definition File Code (Attack Types)
//
// By the DOSDoom Team
//
// Attacks Setup and Parser Code
//
// 1998/10/29 -KM- Finalisation of sound code.  SmartProjectile.
//
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "z_zone.h"

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

attacktype_t bufferatk;
attacktype_t *attackhead;
static attacktype_t *replace = NULL;

void DDF_AtkCheckType(char *info, int commandref);
void DDF_AtkGetSpecial(char *info, int commandref);

// -KM- 1998/09/27 Major changes to sound handling
// -KM- 1998/11/25 Accuracy + Translucency are now fraction.  Added
// a spare attack for BFG.
commandlist_t attackcommands[] =
 {{"ATTACKTYPE"         , DDF_AtkCheckType   ,  NULL},
  {"ATTACK SPECIAL"     , DDF_AtkGetSpecial  ,  NULL},
  {"ACCURACY"           , DDF_MainGetFixed,     &bufferatk.accuracy},
  {"ATTACK HEIGHT"      , DDF_MainGetFixed   ,  &bufferatk.height},
  {"HEIGHT"             , DDF_MainGetFixed   ,  &buffermobj.height},
  {"RADIUS"             , DDF_MainGetFixed   ,  &buffermobj.radius},
  {"SPEED"              , DDF_MainGetFixed   ,  &buffermobj.speed},
  {"FAST"               , DDF_MainGetFixed   ,  &buffermobj.fast},
  {"SHOTCOUNT"          , DDF_MainGetNumeric ,  &bufferatk.count},
  {"X OFFSET"           , DDF_MainGetFixed   ,  &bufferatk.xoffset},
  {"Y OFFSET"           , DDF_MainGetFixed   ,  &bufferatk.yoffset},
  {"DAMAGE"             , DDF_MainGetNumeric ,  &bufferatk.damage},
  {"DAMAGE MULTI"       , DDF_MainGetNumeric ,  &bufferatk.damagemulti},
  {"DAMAGE RANGE"       , DDF_MainGetNumeric ,  &bufferatk.damagerange},
  {"EXPLOD DAMAGE"      , DDF_MainGetNumeric ,  &buffermobj.damage},
  {"EXPLOD DAMAGEMULTI" , DDF_MainGetNumeric ,  &buffermobj.damagemulti},
  {"EXPLOD DAMAGERANGE" , DDF_MainGetNumeric ,  &buffermobj.damagerange},
  {"ATTACKRANGE"        , DDF_MainGetFixed ,  &bufferatk.range},
  {"TOO CLOSE RANGE"    , DDF_MainGetNumeric ,  &bufferatk.tooclose},
  {"NO TRACE CHANCE"    , DDF_MainGetNumeric ,  &bufferatk.notracechance},
  {"KEEP FIRING CHANCE" , DDF_MainGetNumeric ,  &bufferatk.keepfirechance},
  {"ASSAULT SPEED"      , DDF_MainGetNumeric ,  &bufferatk.speed},
  {"PROJECTILE SPECIAL" , DDF_MainGetSpecial ,  NULL},
  {"TRANSLUCENCY"       , DDF_MainGetFixed,     &buffermobj.invisibility},
  {"LAUNCH SOUND"       , DDF_MainLookupSound,  &buffermobj.seesound},
  {"ATTEMPT SOUND"      , DDF_MainLookupSound,  &bufferatk.initsound},
  {"ENGAGED SOUND"      , DDF_MainLookupSound,  &bufferatk.sound},
  {"ACTIVE SOUND"       , DDF_MainLookupSound,  &buffermobj.activesound},
  {"DEATH SOUND"        , DDF_MainLookupSound,  &buffermobj.deathsound},
  {"STATES"             , DDF_MainLoadStates ,  &buffermobj.spawnstate},
  {"DEATHSTATES"        , DDF_MainLoadStates ,  &buffermobj.deathstate},

  {"SPAWNED OBJECT"     , DDF_MainGetString ,   &bufferatk.projectile},
  {"SPARE ATTACK"       , DDF_MainRefAttack,       &buffermobj.spareattack},
  {"SPAWN OBJECT STATE" , DDF_MainGetDirector,  &bufferatk.objinitstate},

  {COMMAND_TERMINATOR   , NULL,                 NULL}};

// -KM- 1998/11/25 Added new attack type for BFG: Spray
char *attackclass[NUMATKCLASS] =
{
  "PROJECTILE",
  "SPAWNER",
  "TRIPLE SPAWNER",
  "FIXED SPREADER",
  "RANDOM SPREADER",
  "SHOT",
  "TRACKER",
  "CLOSECOMBAT",
  "SHOOTTOSPOT",
  "SKULLFLY",
  "SMARTPROJECTILE",
  "SPRAY"
};

specflags_t atkspecials[] =
 {{"SMOKING TRACER",      AF_TRACESMOKE      },
  {"REMOVE FAILED SPAWN", AF_SPAWNREMOVEFAIL },
  {"PRESTEP SPAWN",       AF_SPAWNPRESTEP    },
  {"SPAWN TELEFRAGS",     AF_SPAWNTELEFRAGS  },
  {"NEED SIGHT",          AF_NEEDSIGHT       },
  {"FACE TARGET",         AF_FACETARGET      }};

void DDF_AtkCreateAttack()
{
  attacktype_t* newattack;
  attacktype_t* currattack;
  mobjinfo_t* newmobj;
  mobjinfo_t* currmobj;

  if (replace)
    newattack = replace;
  else
  {
    newattack = malloc(sizeof(attacktype_t));
  
    if (newattack == NULL)
      I_Error("DDF_AtkCreateAttack: Malloc error (ATTACK)");
  }
  memcpy(newattack, &bufferatk, sizeof(attacktype_t));

  if (attackhead == NULL)
  {
    attackhead = newattack;
  }
  else
  {
    currattack = attackhead;

    while (currattack->next != NULL)
      currattack = currattack->next;

    currattack->next = newattack;
  }

  newattack->next = NULL;

  // do we have a new mobj also?
  if (buffermobj.spawnstate)
  {
    newmobj = malloc(sizeof(mobjinfo_t));

    if (newmobj == NULL)
      I_Error("DDF_AtkCreateAttack: Malloc error (MOBJ)");

    memcpy(newmobj, &buffermobj, sizeof(mobjinfo_t));

    // Malloc enough space for attack name and 2 characters: "__"; the
    // intention is that any attacks that create a mobj have names with the
    // double underscore prefix and the attack name -ACB- 1998/08/12
    newmobj->name = malloc(strlen(newattack->name)+(3*sizeof(char)));

    if (newmobj->name == NULL)
      I_Error("DDF_AtkCreateAttack: Malloc error (Mobj->name)");

    sprintf(newmobj->name,"__%s",newattack->name);

    if (newattack->attackstyle==ATK_SPREADER ||
         newattack->attackstyle==ATK_PROJECTILE ||
          newattack->attackstyle==ATK_SMARTPROJECTILE ||
           newattack->attackstyle==ATK_RANDOMSPREAD ||
            newattack->attackstyle==ATK_SHOOTTOSPOT)
    {
//      newmobj->flags |= MF_NOBLOCKMAP+MF_MISSILE+MF_DROPOFF+MF_NOGRAVITY;
      newmobj->mass = 100;
    }

    newattack->projectile = newmobj;

    currmobj = mobjinfohead;

    while (currmobj->next != NULL)
      currmobj = currmobj->next;

    currmobj->next = newmobj;
  }

  memset (&bufferatk, 0, sizeof(attacktype_t));
  memset (&buffermobj, 0, sizeof(mobjinfo_t));
  buffermobj.invisibility = VISIBLE;
  buffermobj.fast = FRACUNIT;
  bufferatk.accuracy = FRACUNIT;
}

void DDF_AtkGetSpecial(char *info, int commandref)
{
  int i;

  i=0;

  while (strcmp(info,atkspecials[i].name) &&
           strcmp(atkspecials[i].name,COMMAND_TERMINATOR))
  {
    i++;
  }

  if (!strcmp(atkspecials[i].name,COMMAND_TERMINATOR))
    I_Error("\n\tDDF_LevelGetSpecials: Unknown Special on Level: %s",bufferatk.name);

  bufferatk.flags += atkspecials[i].flag;
}

void DDF_AtkCheckType(char *info, int commandref)
{
  int i;

  i=0;

  while (i != NUMATKCLASS && strcmp(info,attackclass[i]))
    i++;

  if (i==NUMATKCLASS)
    I_Error("\n\tDDF_AtkCheckType: No such attack type '%s'\n", info);

  bufferatk.attackstyle = i;
}

//
// DDF_AtkCheckName
//
// Checks the attack name, makes sure it is unique.
//
void DDF_AtkCheckName(char *info)
{
  attacktype_t* entry;

#ifdef DEVELOPERS
  if (info == NULL)
    I_Error("Info has no info\n");
#endif

  entry = attackhead;
  replace = NULL;

  while (entry != NULL)
  {
    if (entry->name != NULL)
    {
      if (!strcmp(info,entry->name))
      {
        if (ddf_replace)
          replace = entry;
        else
          I_Error("DDF_AtkCheckName: '%s' already declared\n",info);
      }
    }

   entry = entry->next;
  }

  if ((bufferatk.name = malloc(sizeof(char)*(strlen(info)+1)))==NULL)
    I_Error("DDF_AtkCheckName: Unable to allocate memory\n");

  strcpy(bufferatk.name,info);

  return;

}

void DDF_ReadAtks(void *data, int size)
{
  readinfo_t attacks;

  memset (&bufferatk, 0, sizeof(attacktype_t));
  memset (&buffermobj, 0, sizeof(mobjinfo_t));
  buffermobj.invisibility = VISIBLE;
  buffermobj.fast = FRACUNIT;
  bufferatk.accuracy = FRACUNIT;

  if (!data)
  {
    attacks.message               = "DDF_InitAttacks";
    attacks.filename              = "attacks.ddf";
    attacks.memfile = NULL;
  } else {
    attacks.message = NULL;
    attacks.memfile = data;
    attacks.memsize = size;
  }
  attacks.DDF_MainCheckName     = DDF_AtkCheckName;
  attacks.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  attacks.DDF_MainCreateEntry   = DDF_AtkCreateAttack;
  attacks.DDF_MainFinishingCode = DDF_AtkCreateAttack;
  attacks.cmdlist               = attackcommands;
 
  DDF_MainReadFile(&attacks);
}

void DDF_AttackInit()
{
  attackhead = NULL;

  DDF_ReadAtks(NULL, 0);
}



