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

void DDF_MobjGetItemType(char *info, int commandref);
void DDF_MobjGetBpAmmo(char *info, int commandref);
void DDF_MobjGetBpAmmoLimit(char *info, int commandref);
void DDF_MobjGetBpArmour(char *info, int commandref);
void DDF_MobjGetBpKeys(char *info, int commandref);
void DDF_MobjGetBpWeapon(char *info, int commandref);
void DDF_MobjGetPlayer(char *info, int commandref);

// -KM- 1998/09/27 Major changes to sound handling
// -KM- 1998/11/25 Accuracy + Translucency are now fraction.  Added
// a spare attack for BFG.
// -KM- 1999/01/29 Merged in thing commands, so there is one list of
//  thing commands for all types of things (scenery, items, creatures + projectiles)
static commandlist_t attackcommands[] =
{
/*00*/  {"ATTACKTYPE"         , DDF_AtkCheckType,        NULL},
/*01*/  {"ATTACK SPECIAL"     , DDF_AtkGetSpecial,       NULL},
/*02*/  {"ACCURACY"           , DDF_MainGetFixed,        &bufferatk.accuracy},
/*03*/  {"ATTACK HEIGHT"      , DDF_MainGetFixed,        &bufferatk.height},
/*04*/  {"SHOTCOUNT"          , DDF_MainGetNumeric,      &bufferatk.count},
/*05*/  {"X OFFSET"           , DDF_MainGetFixed,        &bufferatk.xoffset},
/*06*/  {"Y OFFSET"           , DDF_MainGetFixed,        &bufferatk.yoffset},
/*07*/  {"DAMAGE"             , DDF_MainGetNumeric,      &bufferatk.damage},
/*08*/  {"DAMAGE MULTI"       , DDF_MainGetNumeric,      &bufferatk.damagemulti},
/*09*/  {"DAMAGE RANGE"       , DDF_MainGetNumeric,      &bufferatk.damagerange},
/*10*/  {"ATTACKRANGE"        , DDF_MainGetFixed,        &bufferatk.range},
/*11*/  {"TOO CLOSE RANGE"    , DDF_MainGetNumeric,      &bufferatk.tooclose},
/*12*/  {"NO TRACE CHANCE"    , DDF_MainGetNumeric,      &bufferatk.notracechance},
/*13*/  {"KEEP FIRING CHANCE" , DDF_MainGetNumeric,      &bufferatk.keepfirechance},
/*14*/  {"ASSAULT SPEED"      , DDF_MainGetNumeric,      &bufferatk.speed},
/*15*/  {"ATTEMPT SOUND"      , DDF_MainLookupSound,     &bufferatk.initsound},
/*16*/  {"ENGAGED SOUND"      , DDF_MainLookupSound,     &bufferatk.sound},

/*17*/  {"SPAWNED OBJECT"     , DDF_MainGetString,       &bufferatk.projectile},
/*18*/  {"SPAWN OBJECT STATE" , DDF_MainGetDirector,     &bufferatk.objinitstate},

/*19*/  {"PUFF"               , DDF_MainGetString,       &bufferatk.puff},

// MOBJ Commands:
/*20*/  {"MAPNUMBER"          , DDF_MainGetNumeric,      &buffermobj.doomednum},
/*21*/  {"SPAWNHEALTH"        , DDF_MainGetNumeric,      &buffermobj.spawnhealth},
/*22*/  {"RADIUS"             , DDF_MainGetFixed,        &buffermobj.radius},
/*23*/  {"HEIGHT"             , DDF_MainGetFixed,        &buffermobj.height},
/*24*/  {"MASS"               , DDF_MainGetNumeric,      &buffermobj.mass},
/*25*/  {"SPEED"              , DDF_MainGetFixed,        &buffermobj.speed},
/*26*/  {"FAST"               , DDF_MainGetFixed,        &buffermobj.fast},
/*27*/  {"SPECIAL"            , DDF_MainGetSpecial,      NULL},
/*28*/  {"PROJECTILE SPECIAL" , DDF_MainGetSpecial,      NULL},
/*29*/  {"RESPAWN TIME"       , DDF_MainGetTime,         &buffermobj.respawntime},
/*30*/  {"FUSE"               , DDF_MainGetTime,         &buffermobj.fuse},
/*31*/  {"LIFESPAN"           , DDF_MainGetTime,         &buffermobj.fuse},
/*32*/  {"PALETTE REMAP"      , DDF_MainGetNumeric,      &buffermobj.palremap},
/*33*/  {"TRANSLUCENCY"       , DDF_MainGetFixed,        &buffermobj.invisibility},
/*34*/  {"RESPAWN EFFECT"     , DDF_MainGetString,       &buffermobj.respawneffect},

/*35*/  {"BENEFIT TYPE"       , DDF_MobjGetItemType,     &buffermobj.benefittype},
/*36*/  {"BENEFIT AMOUNT"     , DDF_MainGetNumeric,      &buffermobj.benefitamount},
/*37*/  {"BENEFIT LIMIT"      , DDF_MainGetNumeric,      &buffermobj.limit},
/*38*/  {"BACKPACK AMMO"      , DDF_MobjGetBpAmmo,       NULL},
/*39*/  {"BACKPACK AMMOLIMIT" , DDF_MobjGetBpAmmoLimit,  NULL},
/*40*/  {"BACKPACK ARMOUR"    , DDF_MobjGetBpArmour,     NULL},
/*41*/  {"BACKPACK KEYS"      , DDF_MobjGetBpKeys,       NULL},
/*42*/  {"BACKPACK WEAPONS"   , DDF_MobjGetBpWeapon,     NULL},
/*43*/  {"PICKUP MESSAGE"     , DDF_MainReferenceString, &buffermobj.message},

/*44*/  {"PAINCHANCE"         , DDF_MainGetNumeric,      &buffermobj.painchance},
/*45*/  {"EXPLOD DAMAGE"      , DDF_MainGetNumeric,      &buffermobj.damage},
/*46*/  {"EXPLOSION DAMAGE"   , DDF_MainGetNumeric,      &buffermobj.damage},
/*47*/  {"EXPLOD DAMAGEMULTI" , DDF_MainGetNumeric,      &buffermobj.damagemulti},
/*48*/  {"EXPLOD DAMAGERANGE" , DDF_MainGetNumeric,      &buffermobj.damagerange},
/*49*/  {"REACTION TIME"      , DDF_MainGetTime,         &buffermobj.reactiontime},
/*50*/  {"JUMP HEIGHT"        , DDF_MainGetFixed,        &buffermobj.jumpheight},
/*51*/  {"MAX FALL"           , DDF_MainGetFixed,        &buffermobj.maxfall},
/*52*/  {"PAINCHANCE"         , DDF_MainGetNumeric,      &buffermobj.painchance},
/*53*/  {"MINATTACK CHANCE"   , DDF_MainGetNumeric,      &buffermobj.minatkchance},
/*54*/  {"SIDE"               , DDF_MainGetNumeric,      &buffermobj.side},
/*55*/  {"CLOSE ATTACK"       , DDF_MainRefAttack,       &buffermobj.closecombat},
/*56*/  {"RANGE ATTACK"       , DDF_MainRefAttack,       &buffermobj.rangeattack},
/*57*/  {"SPARE ATTACK"       , DDF_MainRefAttack,       &buffermobj.spareattack},
/*58*/  {"DROPITEM"           , DDF_MainGetString,       &buffermobj.dropitem},
/*59*/  {"CASTORDER"          , DDF_MainGetNumeric,      &buffermobj.castorder},
/*60*/  {"PLAYER"             , DDF_MobjGetPlayer,       &buffermobj.playernum},
/*61*/  {"BLOOD"              , DDF_MainGetString,       &buffermobj.blood},
/*62*/  {"GIB"                , DDF_MainGetString,       &buffermobj.gib},

/*63*/  {"PICKUP SOUND"       , DDF_MainLookupSound,     &buffermobj.activesound},
/*64*/  {"ACTIVE SOUND"       , DDF_MainLookupSound,     &buffermobj.activesound},
/*65*/  {"LAUNCH SOUND"       , DDF_MainLookupSound,     &buffermobj.seesound},
/*66*/  {"AMBIENT SOUND"      , DDF_MainLookupSound,     &buffermobj.seesound},
/*67*/  {"SIGHTING SOUND"     , DDF_MainLookupSound,     &buffermobj.seesound},
/*68*/  {"DEATH SOUND"        , DDF_MainLookupSound,     &buffermobj.deathsound},
/*69*/  {"PAIN SOUND"         , DDF_MainLookupSound,     &buffermobj.painsound},
/*70*/  {"STARTCOMBAT SOUND"  , DDF_MainLookupSound,     &buffermobj.attacksound},
/*71*/  {"WALK SOUND"         , DDF_MainLookupSound,     &buffermobj.walksound},

/*72*/  {"SPAWN STATES"       , DDF_MainLoadStates,      &buffermobj.spawnstate},
/*73*/  {"STATES"             , DDF_MainLoadStates,      &buffermobj.spawnstate},
/*74*/  {"CHASING STATES"     , DDF_MainLoadStates,      &buffermobj.seestate},
/*75*/  {"PAINSTATES"         , DDF_MainLoadStates,      &buffermobj.painstate},
/*76*/  {"DEATHSTATE"         , DDF_MainLoadStates,      &buffermobj.deathstate},
/*77*/  {"DEATHSTATES"        , DDF_MainLoadStates,      &buffermobj.deathstate},
/*78*/  {"OVERKILLSTATES"     , DDF_MainLoadStates,      &buffermobj.xdeathstate},
/*79*/  {"RESPAWN STATES"     , DDF_MainLoadStates,      &buffermobj.raisestate},
/*80*/  {"RESURRECT STATES"   , DDF_MainLoadStates,      &buffermobj.resstate},
/*81*/  {"MISSILE STATES"     , DDF_MainLoadStates,      &buffermobj.missilestate},
/*82*/  {"MELEE STATES"       , DDF_MainLoadStates,      &buffermobj.meleestate},
/*83*/  {"MEANDER STATES"     , DDF_MainLoadStates,      &buffermobj.meanderstate},
        {COMMAND_TERMINATOR   , NULL,                 NULL}
};
commandlist_t *thingcommands = &attackcommands[20];

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
  {"FACE TARGET",         AF_FACETARGET      },
  {"PLAYER ATTACK",       AF_PLAYER          }};

void DDF_AtkCreateAttack()
{
  attacktype_t* newattack;
  attacktype_t* currattack = NULL;
  mobjinfo_t* newmobj;
  mobjinfo_t* currmobj;

  if (replace)
  {
    newattack = replace;
    bufferatk.next = replace->next;
  }
  else
  {
    newattack = malloc(sizeof(attacktype_t));
  
    if (newattack == NULL)
      I_Error("DDF_AtkCreateAttack: Malloc error (ATTACK)");
  
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
  }

  memcpy(newattack, &bufferatk, sizeof(attacktype_t));
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
  buffermobj.respawntime = 12*TICRATE;
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
  buffermobj.respawntime = 12*TICRATE;
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
    attacks.filename = NULL;
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



