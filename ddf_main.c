//
// DOSDoom Definition Files Code (main)
//
// By the DOSDoom Team
//
#define DDF_MAIN
#include "d_debug.h"
#include "dm_state.h"
#include "dstrings.h"
#include "i_system.h"
#include "lu_sound.h"
#include "m_argv.h"
#include "m_fixed.h"
#include "m_misc.h"
#include "p_mobj.h"
#include "z_zone.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

#include "p_action.h"

void DDF_MainTransferExisting(void);
void DDF_MainCleanUpLooseEnds(void);

int maxplayers = 8;

mobjinfo_t *specials[NUMMOBJSPEC];

backpack_t buffpack =
{
  NULL, // Ammo
  NULL, // Ammolimit
  { false, false, false, false, false, false }, // Keys
  NULL, // weapons
  0, // armour
  0, // armourclass
  NULL
};
mapstuff_t buffermap;
mobjinfo_t buffermobj;
mobjinfo_t* bufferreplacemobj;
langref_t bufflangref;
int statecount;
char *stateinfo[NUMSPLIT+1];
state_t *tempstates[255];
boolean terminated;

// -KM- 1998/11/25 All weapon related sprites are out
char **sprnames;
int MAXSPRITES = 256;
static char *oldsprnames[] = {
    "TROO","MISL","BBRN","BOSF","FIRE",NULL
};

// -KM- 1998/11/25 All weapon related states are out
state_t* states;
int MAXSTATES = 1024;
static state_t	oldstates[] = {
    {SPR_TROO,0,-1,{NULL},S_NULL,0,0},	// S_NULL
    {SPR_TROO,4,0,{A_Light0},S_NULL,0,0},	// S_LIGHTDONE

    // ------------------------------------------------------
    {SPR_MISL,32768,1,{NULL},S_ROCKET,0,0},	// S_ROCKET
    {SPR_MISL,32769,8,{A_Explode},S_EXPLODE2,0,0},	// S_EXPLODE1
    {SPR_MISL,32770,6,{NULL},S_EXPLODE3,0,0},	// S_EXPLODE2
    {SPR_MISL,32771,4,{NULL},S_NULL,0,0},	// S_EXPLODE3
    // ----------------------------------------------------------

    {SPR_BBRN,0,-1,{NULL},S_NULL,0,0},		// S_BRAIN
    {SPR_BBRN,1,36,{A_BrainPain},S_BRAIN,0,0},	// S_BRAIN_PAIN
    {SPR_BBRN,0,100,{A_BrainScream},S_BRAIN_DIE2,0,0},	// S_BRAIN_DIE1
    {SPR_BBRN,0,10,{NULL},S_BRAIN_DIE3,0,0},	// S_BRAIN_DIE2
    {SPR_BBRN,0,10,{NULL},S_BRAIN_DIE4,0,0},	// S_BRAIN_DIE3
    {SPR_BBRN,0,-1,{A_BrainDie},S_NULL,0,0},	// S_BRAIN_DIE4

    {SPR_TROO,0,10,{P_ActStandardLook},S_BRAINEYE,0,0},	// S_BRAINEYE
    {SPR_TROO,0,181,{A_BrainAwake},S_BRAINEYE1,0,0},	// S_BRAINEYESEE
    {SPR_TROO,0,150,{A_BrainSpit},S_BRAINEYE1,0,0},	// S_BRAINEYE1

    {SPR_BOSF,32768,3,{A_SpawnSound},S_SPAWN2,0,0},	// S_SPAWN1
    {SPR_BOSF,32769,3,{A_SpawnFly},S_SPAWN3,0,0},	// S_SPAWN2
    {SPR_BOSF,32770,3,{A_SpawnFly},S_SPAWN4,0,0},	// S_SPAWN3
    {SPR_BOSF,32771,3,{A_SpawnFly},S_SPAWN1,0,0},	// S_SPAWN4
    {SPR_FIRE,32768,4,{P_ActTrackerFollow},S_SPAWNFIRE2,0,0},	// S_SPAWNFIRE1
    {SPR_FIRE,32769,4,{P_ActTrackerFollow},S_SPAWNFIRE3,0,0},	// S_SPAWNFIRE2
    {SPR_FIRE,32770,4,{P_ActTrackerFollow},S_SPAWNFIRE4,0,0},	// S_SPAWNFIRE3
    {SPR_FIRE,32771,4,{P_ActTrackerFollow},S_SPAWNFIRE5,0,0},	// S_SPAWNFIRE4
    {SPR_FIRE,32772,4,{P_ActTrackerFollow},S_SPAWNFIRE6,0,0},	// S_SPAWNFIRE5
    {SPR_FIRE,32773,4,{P_ActTrackerFollow},S_SPAWNFIRE7,0,0},	// S_SPAWNFIRE6
    {SPR_FIRE,32774,4,{P_ActTrackerFollow},S_SPAWNFIRE8,0,0},	// S_SPAWNFIRE7
    {SPR_FIRE,32775,4,{P_ActTrackerFollow},S_NULL,0,0},		// S_SPAWNFIRE8

    {SPR_MISL,32769,10,{NULL},S_BRAINEXPLODE2,0,0},	// S_BRAINEXPLODE1
    {SPR_MISL,32770,10,{NULL},S_BRAINEXPLODE3,0,0},	// S_BRAINEXPLODE2
    {SPR_MISL,32771,10,{A_BrainExplode},S_NULL,0,0},	// S_BRAINEXPLODE3
};

// -KM- 1998/09/27 Change of SFX from enum to String
// -KM- 1998/10/29 String is converted to sfx_t at runtime. Yuk!
// -KM- 1998/11/25 All Weapon related things are out. Default visibilities are in.
mobjinfo_t oldmobjinfo[ORIG_NUMMOBJTYPES] = {
    {		// MT_BOSSBRAIN
	88,		// doomednum
	S_BRAIN,		// spawnstate
	250,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_BRAIN_PAIN,		// painstate
	255,		// painchance
	(sfx_t *) "BOSPN",		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BRAIN_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "BOSDTH",		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	10000000,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_SOLID|MF_SHOOTABLE,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_BOSSSPIT
	89,		// doomednum
	S_BRAINEYE,		// spawnstate
	1000,		// spawnhealth
	S_BRAINEYESEE,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_BOSSTARGET
	87,		// doomednum
	S_NULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_SPAWNSHOT
	-1,		// doomednum
	S_SPAWN1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	(sfx_t *) "BOSPIT",		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "FIRXPL",		// deathsound
	10*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	3,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_NOCLIP,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_SPAWNFIRE
	-1,		// doomednum
	S_SPAWNFIRE1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_ROCKET
	-1,		// doomednum
	S_ROCKET,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	(sfx_t *) "RLAUNC",		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_EXPLODE1,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "BAREXP",		// deathsound
	20*FRACUNIT,		// speed
	11*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	20,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    }
};

framedest_t framedestlist[] =
                   {{ "SPAWN",     &buffermobj.spawnstate,   FDF_SPAWN     },
                    { "CHASE",     &buffermobj.seestate,     FDF_CHASE     },
                    { "MELEE",     &buffermobj.meleestate,   FDF_MELEE     },
                    { "MISSILE",   &buffermobj.missilestate, FDF_MISSILE   },
                    { "PAIN",      &buffermobj.painstate,    FDF_PAIN      },
                    { "DEATH",     &buffermobj.deathstate,   FDF_DEATH     },
                    { "XDEATH",    &buffermobj.xdeathstate,  FDF_XDEATH    },
                    { "RESPAWN",   &buffermobj.raisestate,   FDF_RESPAWN   },
                    { "RESURRECT", &buffermobj.resstate,     FDF_RESURRECT },
                    { "MEANDER",   &buffermobj.meanderstate, FDF_MEANDER   },
                    { COMMAND_TERMINATOR, NULL, 0 }};

// -KM- 1998/11/25 Translucency to fractional.
// -KM- 1998/12/16 Added individual flags for all.
specials_t specialslist[] =
                   {{ "FUZZY"         ,MF_SHADOW      ,0           ,0          },
                    { "SOLID"         ,MF_SOLID       ,0           ,0          },
                    { "ON CEILING"    ,MF_SPAWNCEILING+MF_NOGRAVITY,0,0        },
                    { "FLOATER"       ,MF_FLOAT+MF_NOGRAVITY,0     ,0          },
                    { "INERT"         ,MF_NOBLOCKMAP  ,0           ,0          },
                    { "TELEPORT TYPE" ,MF_NOGRAVITY   ,0           ,0          },
                    { "NO LINKS"      ,MF_NOBLOCKMAP+MF_NOSECTOR,0 ,0          },
                    { "DAMAGESMOKE"   ,MF_NOBLOOD     ,0           ,0          },
                    { "SHOOTABLE"     ,MF_SHOOTABLE   ,0           ,0          },
                    { "COUNT AS KILL" ,MF_COUNTKILL   ,0           ,0          },
                    { "COUNT AS ITEM" ,MF_COUNTITEM   ,0           ,0          },
                    { "SPECIAL"       ,MF_SPECIAL     ,0           ,0          },
                    { "NOSECTOR"      ,MF_NOSECTOR    ,0           ,0          },
                    { "NOBLOCKMAP"    ,MF_NOBLOCKMAP  ,0           ,0          },
                    { "SPAWNCEILING"  ,MF_SPAWNCEILING,0           ,0          },
                    { "NOGRAVITY"     ,MF_NOGRAVITY   ,0           ,0          },
                    { "DROPOFF"       ,MF_DROPOFF     ,0           ,0          },
                    { "PICKUP"        ,MF_PICKUP      ,0           ,0          },
                    { "NOCLIP"        ,MF_NOCLIP      ,0           ,0          },
                    { "SLIDER"        ,MF_SLIDE       ,0           ,0          },
                    { "FLOAT"         ,MF_FLOAT       ,0           ,0          },
                    { "TELEPORT"      ,MF_TELEPORT    ,0           ,0          },
                    { "MISSILE"       ,MF_MISSILE     ,0           ,0          },
                    { "DROPPED"       ,MF_DROPPED     ,0           ,0          },
                    { "CORPSE"        ,MF_CORPSE      ,0           ,0          },
                    { "STEALTH"       ,MF_STEALTH     ,0           ,0          },
                    { "NODEATHMATCH"  ,MF_NOTDMATCH   ,0           ,0          },
                    { "NO RESPAWN"    ,0              ,EF_NORESPAWN,0          },
                    { "NO RESURRECT"  ,0              ,EF_NORESURRECT,0        },
                    { "DISLOYAL"      ,0              ,EF_DISLOYALTYPE,0       },
                    { "TRIGGER HAPPY" ,0              ,EF_TRIGGERHAPPY,0       },
                    { "ATTACK HURTS"  ,0              ,EF_OWNATTACKHURTS,0     },
                    { "BOSSMAN"       ,0              ,EF_BOSSMAN,0            },
                    { "NEVERTARGETED" ,0              ,EF_NEVERTARGET,0        },
                    { "NOGRAV KILL"   ,0              ,EF_NOGRAVKILL,0         },
                    { "NO GRUDGE"     ,0              ,EF_NOGRUDGE,0           },
                    { "BOUNCE"        ,0              ,EF_BOUNCE,0             },
                    { "INVISIBLE"     ,0              ,0           ,INVISIBLE  },
                    { COMMAND_TERMINATOR,0,0,0 } };

int NUMMOBJTYPES;
int NUMSTATES;
int NUMSPRITES;

backpack_t* backpackhead;
commandlist_t* currentcmdlist;
// -KM- 1998/11/25 Always 10 weapon keys, 1 - 0
weaponkey_t weaponkey[10];
mobjinfo_t* mobjinfohead;
mapstuff_t* maphead = NULL;
boolean ddf_replace = false;

void DDF_MainInit()
{
  I_Printf("Doom Definition v%i.%02i\n",DDFVERSION/100,DDFVERSION%100);

  NUMMOBJTYPES = ORIG_NUMMOBJTYPES;
  NUMSTATES    = ORIG_NUMSTATES;
  NUMSPRITES   = ORIG_NUMSPRITES;

  DDF_LanguageInit();

  // Other DDF files depend on this, it goes first.
  DDF_SFXInit();

  // -KM- 1998/10/29 Depends on SFX
  DDF_MainTransferExisting();

  DDF_AttackInit();    // Attack types
  // -KM- 1998/11/25 Items.ddf must be loaded after weapons.ddf
  DDF_WeaponInit();    // Initialise Player Weapon Types

  DDF_MobjInit();      //   } - Initialise the moving objects (mobj's)

  DDF_LinedefInit();   // Map stuff: Linedefs
  DDF_SectorInit();    //            Sectors
  DDF_SWInit();        //            Switches
  DDF_AnimInit();      //            Animations.

  DDF_GameInit();      // ...and then the overall game.
  DDF_LevelInit();     // Sort out the levels....

  DDF_OldThingInit();  // temp function

//  DDF_MainCleanUpLooseEnds();
  ddf_replace = true;

#ifdef DEVELOPERS
  I_Printf("DDF Finished, Press a key:");
  getch();
#endif
}

// Temp procedure to transfer data to a linked list.
void DDF_MainTransferExisting()
{
 int i;

 mobjinfohead = &oldmobjinfo[0];

 for (i=0; i<(ORIG_NUMMOBJTYPES-1); i++)
 {
    // -KM- 1998/10/29 convert Strings to sfx
    oldmobjinfo[i].seesound = DDF_LookupSound((char *) oldmobjinfo[i].seesound);
    oldmobjinfo[i].attacksound = DDF_LookupSound((char *) oldmobjinfo[i].attacksound);
    oldmobjinfo[i].painsound = DDF_LookupSound((char *) oldmobjinfo[i].painsound);
    oldmobjinfo[i].deathsound = DDF_LookupSound((char *) oldmobjinfo[i].deathsound);
    oldmobjinfo[i].activesound = DDF_LookupSound((char *) oldmobjinfo[i].activesound);
    oldmobjinfo[i].walksound = DDF_LookupSound((char *) oldmobjinfo[i].walksound);

    oldmobjinfo[i].next = &oldmobjinfo[i+1];
 }
 // Do the last mobj
 oldmobjinfo[i].seesound = DDF_LookupSound((char *) oldmobjinfo[i].seesound);
 oldmobjinfo[i].attacksound = DDF_LookupSound((char *) oldmobjinfo[i].attacksound);
 oldmobjinfo[i].painsound = DDF_LookupSound((char *) oldmobjinfo[i].painsound);
 oldmobjinfo[i].deathsound = DDF_LookupSound((char *) oldmobjinfo[i].deathsound);
 oldmobjinfo[i].activesound = DDF_LookupSound((char *) oldmobjinfo[i].activesound);
 oldmobjinfo[i].walksound = DDF_LookupSound((char *) oldmobjinfo[i].walksound);

 sprnames = malloc(sizeof(*sprnames) * MAXSPRITES);
 for (i = 0; oldsprnames[i]; i++)
    sprnames[i] = oldsprnames[i];

 states = malloc(sizeof(*states) * MAXSTATES);
 memcpy(states, oldstates, sizeof(oldstates));
}

// -KM- 1999/01/29 Fixed #define system.
typedef struct
{
  char* name;
  char* value;
} define_t;

define_t* defines = NULL;
int       numDefines = 0;

void DDF_MainAddDefine(char* name, char* value)
{
   int i;
   for (i = 0; i < numDefines; i++)
   {
      if (!strcmp(defines[i].name, name))
        I_Error("\nRedefinition of '%s'\n", name);
   }

   defines = realloc(defines, (numDefines+1)*sizeof(define_t));
   defines[numDefines].name = name;
   defines[numDefines++].value = value;
}

char* DDF_MainGetDefine(char* name)
{
  int i;
  for (i = 0; i < numDefines; i++)
     if (!strcmp(defines[i].name, name))
       return defines[i].value;

  // Not a define.
  return name;
}

void DDF_MainCleanUpLooseEnds()
{
  int i, j;
  attacktype_t* curratk;
  mobjinfo_t* currmobj;
  boolean continueflag;

  char* name;

  // ------------
  // ATTACKS
  // ------------
  curratk = attackhead;

  while (curratk != NULL)
  {
    name = (char *) curratk->puff;
    if (name)
      curratk->puff = DDF_MobjLookup(name);
    else
      curratk->puff = DDF_MobjLookup("PUFF");

    if (curratk->attackstyle==ATK_SPAWNER || curratk->attackstyle==ATK_TRIPLESPAWNER)
    {
      name = (char *) curratk->projectile;

      if (!name)
        I_Error("DDF_MainCleanUp: No spawn object given for '%s'\n", curratk->name);

      curratk->projectile = DDF_MobjLookup(name);

      // This is bloody awful.....
      if (curratk->objinitstate & FDF_SPAWN)
        curratk->objinitstate -= (FDF_SPAWN - curratk->projectile->spawnstate);
      else if (curratk->objinitstate & FDF_CHASE)
        curratk->objinitstate -= (FDF_CHASE - curratk->projectile->seestate);
      else if (curratk->objinitstate & FDF_MELEE)
        curratk->objinitstate -= (FDF_MELEE - curratk->projectile->meleestate);
      else if (curratk->objinitstate & FDF_MISSILE)
        curratk->objinitstate -= (FDF_MISSILE - curratk->projectile->missilestate);
      else if (curratk->objinitstate & FDF_PAIN)
        curratk->objinitstate -= (FDF_PAIN - curratk->projectile->painstate);
      else if (curratk->objinitstate & FDF_DEATH)
        curratk->objinitstate -= (FDF_DEATH - curratk->projectile->deathstate);
      else if (curratk->objinitstate & FDF_XDEATH)
        curratk->objinitstate -= (FDF_XDEATH - curratk->projectile->xdeathstate);
      else if (curratk->objinitstate & FDF_RESPAWN)
        curratk->objinitstate -= (FDF_RESPAWN - curratk->projectile->raisestate);
      else if (curratk->objinitstate & FDF_RESURRECT)
        curratk->objinitstate -= (FDF_RESURRECT - curratk->projectile->resstate);
      else if (curratk->objinitstate & FDF_MEANDER) // -ACB- 1998/09/05 Meander stuff
        curratk->objinitstate -= (FDF_MEANDER - curratk->projectile->meanderstate);
      else
        curratk->objinitstate = curratk->projectile->spawnstate;
    }

    curratk = curratk->next;
  }

  // ------------
  // MOBJ TYPES
  // ------------
  currmobj = mobjinfohead;

  name = NULL;

  while (currmobj != NULL)
  {
    name = (char *) currmobj->dropitem;

    if (name)
      currmobj->dropitem = DDF_MobjLookup(name);

    name = (char *) currmobj->blood;
    if (name)
      currmobj->blood = DDF_MobjLookup(name);
    else
      currmobj->blood = DDF_MobjLookup("BLOOD");

    name = (char *) currmobj->respawneffect;
    if (name)
      currmobj->respawneffect = DDF_MobjLookup(name);
    else
    {
      if ((currmobj->flags & MF_SPECIAL))
        currmobj->respawneffect = DDF_MobjLookup("ITEM RESPAWN");
      else
        currmobj->respawneffect = DDF_MobjLookup("RESPAWN FLASH");
    }

    name = (char *) currmobj->gib;
    if (name)
      currmobj->gib = DDF_MobjLookup(name);
    else
      currmobj->gib = DDF_MobjLookup("GIB");

    currmobj = currmobj->next;
  }

  // ------------
  // LINES
  // ------------
  //
  // Yes, this is ACB's first attempt at a hash table search: come back
  // linked-list! all is forgiven!.
  //
  // -ACB- 1998/09/13 Search through and replace name with object info
  // pointer (teleport effects objects).
  //
  for (i=0; i<256; i++)
  {
    if (specialLineDefs[i])
    {
      j=0;
      continueflag = false;

      while (!continueflag)
      {
        if (specialLineDefs[i][j].t.teleport &&
             (char*)specialLineDefs[i][j].t.inspawnobj != NULL)
        {
          specialLineDefs[i][j].t.inspawnobj =
            DDF_MobjLookup((char *) specialLineDefs[i][j].t.inspawnobj);
        }

        if (specialLineDefs[i][j].t.teleport &&
             (char*)specialLineDefs[i][j].t.outspawnobj != NULL)
        {
          specialLineDefs[i][j].t.outspawnobj =
            DDF_MobjLookup((char *) specialLineDefs[i][j].t.outspawnobj);
        }

        if (!specialLineDefs[i][j].next)
          continueflag = true;

        j++;
      }
    }
  }

  //
  // Sprite names
  //
  sprnames[NUMSPRITES] = NULL;

  //hu_stuff.c
  //m_misc.c
  // -KM- 1999/01/29 Added chat macro defaults, gamma messages and talk keys back in.
  i = 0;

  while (strcmp(defaults[i].name,"chatmacro0"))
    i++;

  defaults[i++].defaultvalue=(int)chat_macros[0]=DDF_LanguageLookup("DefaultCHATMACRO0");
  defaults[i++].defaultvalue=(int)chat_macros[1]=DDF_LanguageLookup("DefaultCHATMACRO1");
  defaults[i++].defaultvalue=(int)chat_macros[2]=DDF_LanguageLookup("DefaultCHATMACRO2");
  defaults[i++].defaultvalue=(int)chat_macros[3]=DDF_LanguageLookup("DefaultCHATMACRO3");
  defaults[i++].defaultvalue=(int)chat_macros[4]=DDF_LanguageLookup("DefaultCHATMACRO4");
  defaults[i++].defaultvalue=(int)chat_macros[5]=DDF_LanguageLookup("DefaultCHATMACRO5");
  defaults[i++].defaultvalue=(int)chat_macros[6]=DDF_LanguageLookup("DefaultCHATMACRO6");
  defaults[i++].defaultvalue=(int)chat_macros[7]=DDF_LanguageLookup("DefaultCHATMACRO7");
  defaults[i++].defaultvalue=(int)chat_macros[8]=DDF_LanguageLookup("DefaultCHATMACRO8");
  defaults[i++].defaultvalue=(int)chat_macros[9]=DDF_LanguageLookup("DefaultCHATMACRO9");

  destination_keys = malloc(maxplayers*sizeof(*destination_keys) + 1);
  strncpy(destination_keys, DDF_LanguageLookup("ChatKeys"), maxplayers);

  //m_menu.c
  gammamsg[0]=DDF_LanguageLookup("GammaOff");
  gammamsg[1]=DDF_LanguageLookup("GammaLevelOne");
  gammamsg[2]=DDF_LanguageLookup("GammaLevelTwo");
  gammamsg[3]=DDF_LanguageLookup("GammaLevelThree");
  gammamsg[4]=DDF_LanguageLookup("GammaLevelFour");

  name = alloca(16);
  player_names = malloc(maxplayers*sizeof(*player_names));
  for (i = 0; i < maxplayers; i++)
  {
    sprintf(name, "Player%dName", i+1);
    player_names[i] = DDF_LanguageLookup(name);
  }

  players = malloc(maxplayers*sizeof(*players));
  memset(players, 0, sizeof(*players) * maxplayers);
  for (i = 0; i < maxplayers; i++)
  {
     players[i].weaponowned =
       malloc(numweapons * sizeof(boolean));
     // -KM- 1998/12/17 We want to realloc ammo and max ammo, not weaponowned!
     players[i].ammo =
       malloc(NUMAMMO * sizeof(int));
     players[i].maxammo = malloc(NUMAMMO * sizeof(int));
  }
}

// -KM- 1998/12/16 This loads the ddf file into memory for parsing.
void* DDF_MainCacheFile(readinfo_t* readinfo)
{
  FILE* file;
  void *memfile;
  char *filename;
  int p;
  size_t size;

  if (!readinfo->filename)
    I_Error("DDF_MainReadFile: No file to read");

  p = M_CheckParm("-ddf");
  if (p && p < myargc-1)
  {
    filename = alloca(strlen(myargv[p+1]) + 2 + strlen(readinfo->filename));
    sprintf(filename, "%s/%s", myargv[p+1], readinfo->filename);
  } else
    filename = readinfo->filename;

  if ((file = fopen( filename,"rb" ))==NULL)
    I_Error("DDF_MainReadFile: Unable to open: '%s'",filename);

#ifdef DEVELOPERS
  File_Printf("\nDDF Parser Output:\n");
#endif

  fseek(file,0,SEEK_END);                         // get to the end of the file
  size = ftell(file);                             // get the size
  fseek(file,0,SEEK_SET);                         // reset to beginning
  memfile = malloc(size*sizeof(char)+1);          // malloc the size

  if (!memfile)
    I_Error("DDF_MainReadFile: Malloc Failed on memfile\n");

  memset (memfile,'\0',size*sizeof(char)+1);
  fread ((char*)memfile,sizeof(char),size,file);

  fclose (file); // close the file
  readinfo->memsize = size;
  return memfile;
}

//
// Description of the DDF Parser:
//
// The DDF Parser is a simple reader that is very limited in error checking,
// however it can adapt to most tasks, as is required for the variety of stuff
// need to be loaded in order to configure the DOSDoom Engine.
//
// The parser will read an ascii file, character by character an interpret each
// depending in which mode it is in; Unless an error is encountered or a called
// procedure stops the parser, it will read everything until EOF is encountered.
//
// When the parser function is called, a pointer to a readinfo_t is passed and
// contains all the info needed, it contains:
//
// * message               - message displayed on the screen, if NULL nothing displayed
// * filename              - filename to be read, returns error if NULL
// * DDF_MainCheckName     - function called when a def has been just been started
// * DDF_MainCheckCmd      - function called when we need to check a command
// * DDF_MainCreateEntry   - function called when a def has been completed
// * DDF_MainFinishingCode - function called when EOF is read
// * currentcmdlist        - Current list of commands
//
// Also when commands are referenced, they use currentcmdlist, which is a pointer
// to a list of entries, the entries are formatted like this:
//
// * name - name of command
// * routine - function called to interpret info
// * numeric - void pointer to an value (possibly used by routine)
//
// name is compared with the read command, to see if it matchs.
// routine called to interpret info, if command name matches read command.
// numeric is used if a numeric value needs to be changed, by routine.
//
// The different parser modes are:
//  waiting_newdef
//  reading_newdef
//  reading_command
//  reading_data
//  reading_remark
//  reading_string
//
// 'waiting_newdef' is only set at the start of the code, At this point every
// character with the exception of DEFSTART is ignored. When DEFSTART is
// encounted, the parser will switch to reading_newdef. DEFSTART the parser
// will only switches modes and sets firstgo to false.
//
// 'reading_newdef' reads all alphanumeric characters and the '_' character - which
// substitudes for a space character (whitespace is ignored) - until DEFSTOP is read.
// DEFSTOP passes the read string to DDF_MainCheckName and then clears the string.
// Mode reading_command is now set. All read stuff is passed to char *buffer.
//
// 'reading_command' picks out all the alphabetic characters and passes them to
// buffer as soon as COMMANDREAD is encountered; DDF_MainReadCmd looks through
// for a matching command, if none is found a fatal error is returned. If a matching
// command is found, this function returns a command reference number to command ref
// and sets the mode to reading_data. if DEFSTART is encountered the procedure will
// clear the buffer, run DDF_MainCreateEntry (called this as it reflects that in Items
// & Scenery if starts a new mobj type, in truth it can do anything procedure wise) and
// then switch mode to reading_newdef.
//
// 'reading_data' passes alphanumeric characters, plus a few other characters that
// are also needed. It continues to feed buffer until a SEPERATOR or a TERMINATOR is
// found. The difference between SEPERATOR and TERMINATOR is that a TERMINATOR refs
// the cmdlist to find the routine to use and then sets the mode to reading_command,
// whereas SEPERATOR refs the cmdlist to find the routine and a looks for more data
// on the same command. This is how the multiple states and specials are defined.
//
// 'reading_remark' does not process any chars except REMARKSTOP, everything else is
// ignored. This mode is only set when REMARKSTART is found, when this happens the
// current mode is held in formerstatus, which is restored when REMARKSTOP is found.
//
// 'reading_string' is set when the parser is going through data (reading_data) and
// encounters STRINGSTART and only stops on a STRINGSTOP. When reading_string,
// everything that is an ASCII char is read (which the exception of STRINGSTOP) and
// passed to the buffer. REMARKS are ignored in when reading_string and the case is
// take notice of here.
//
// The maximum size of BUFFER is set in the BUFFERSIZE define.
//
// DDF_MainReadFile & DDF_MainProcessChar handle the main processing of the file, all
// the procedures in the other DDF files (which the exceptions of the Inits) are
// called directly or indirectly. DDF_MainReadFile handles to opening, closing and
// calling of procedures, DDF_MainProcessChar makes sense from the character read
// from the file.
//

//
// DDF_MainReadFile
//
// -ACB- 1998/08/10 Added the string reading code
//
void DDF_MainReadFile (readinfo_t *readinfo)
{
  char *buffer;
  char character;
  char *memfile;
  char *memfileptr;
  readstatus_t status, formerstatus;
  readchar_t response;
  int commandref;
  int size;
  boolean firstgo;

#ifdef DEVELOPERS
  char charcount = 0;
#endif

  statecount = NUMSTATES;
  status = waiting_newdef;
  formerstatus = 0;
  commandref = NULL;
  firstgo = true;

  if (readinfo->message)
    I_Printf("  %s",readinfo->message);

  if (!readinfo->memfile && !readinfo->filename)
    I_Error("DDF_MainReadFile: No file to read");

  if (!readinfo->memfile)
    readinfo->memfile = DDF_MainCacheFile(readinfo);

  // Get the current list from readinfo
  currentcmdlist = readinfo->cmdlist;

  buffer = malloc (sizeof(char)*BUFFERSIZE);

  if (buffer == NULL)
    I_Error("DDF_MainReadFile: Malloc for BUFFER failed\n");

  memset(buffer,'\0',sizeof(sizeof(char)*BUFFERSIZE));

  memfileptr = memfile = readinfo->memfile;
  size = readinfo->memsize;

  // -ACB- 1998/09/12 Copy file to memory: Read until end. Speed optimisation.
  while (memfileptr < &memfile[size])
  {
    // -KM- 1998/12/16 Added #define command to ddf files.
    if (!strncasecmp(memfileptr, "#DEFINE", 7))
    {
      char* name;
      char* value;
      boolean line = false;
      memfileptr += 8;
      name = memfileptr;
      while (*memfileptr != ' ' && memfileptr < &memfile[size]) memfileptr++;
      if (memfileptr < &memfile[size])
      {
        *memfileptr++ = 0;
        value = memfileptr;
      }
      else
        I_Error("\n#DEFINE '%s' as what?!\n", name);
      while (memfileptr < &memfile[size])
      {
        if (*memfileptr == '\r')
          *memfileptr = ' ';
        if (*memfileptr == '\\')
          line = true;
        if (*memfileptr == '\n' && !line)
          break;
        memfileptr++;
      }
      *memfileptr++ = 0;
      DDF_MainAddDefine(name, value);
      buffer[0] = '\0';
      status = reading_command;
      terminated = false;
      continue;
    }

    character = *memfileptr++;

    response = DDF_MainProcessChar(character, buffer, status);

    switch (response)
    {
      case remark_start:
        formerstatus = status;
        status = reading_remark;
        break;

      case remark_stop:
        status = formerstatus;
        break;

      case command_read:
        strupr(buffer);

        //
        // Confused..The Function sets up the command reference and returns
        // a pointer to the reading function. This function is the one that looks at
        // the string read from the file. got it?
        //
        // -ACB- 1998/08/10 Returns commandref, not function.
        //
        commandref = readinfo->DDF_MainCheckCmd(buffer);
        DDF_ReadFunction = currentcmdlist[commandref].routine;
        buffer[0] = '\0';
        status = reading_data;
        break;

      case def_start:
        I_Printf(".");
        if (firstgo)
        {
          firstgo = false;
          status=reading_newdef;
        }
        else
        {
          readinfo->DDF_MainCreateEntry();
          buffer[0] = '\0';
          status=reading_newdef;
        }
        break;

      case def_stop:
        strupr(buffer);
        readinfo->DDF_MainCheckName(buffer);
        buffer[0] = '\0';
        status=reading_command;
        break;

      case seperator:
        DDF_ReadFunction(DDF_MainGetDefine(buffer),commandref);
        buffer[0] = '\0';
        break;

      // -ACB- 1998/08/10 String Handling
      case string_start:
        status = reading_string;
        break;

      // -ACB- 1998/08/10 String Handling
      case string_stop:
        status = reading_data;
        break;

      case terminator:
        terminated = true;
        DDF_ReadFunction(DDF_MainGetDefine(buffer),commandref);
        buffer[0] = '\0';
        status = reading_command;
        terminated = false;
        break;

      case nothing:
        break;

      case ok_char:
#ifdef DEVELOPERS
        charcount++;
        File_Printf("%c",character);
        if (charcount == 75)
        {
          charcount = 0;
          File_Printf("\n");
        }
#endif
        break;

      default:
        break;
    }

  }

  // if firstgo is true, nothing was defined
  if (!firstgo) readinfo->DDF_MainFinishingCode();

  free(buffer);
  if (defines)
  {
     free(defines);
     numDefines = 0;
     defines = NULL;
  }
  if (readinfo->filename)
  {
    free(memfile);
    I_Printf("\n");
  }
}

//
// DDF_MainProcessChar
//
// 1998/08/10 Added String reading code.
//
readchar_t DDF_MainProcessChar(char character, char *buffer, readstatus_t status)
{
   int len;
   char concat[2] = "\0";

   // -ACB- 1998/08/11 Used for detecting formatting in a string
   static char formatchar = false;

   len = strlen(buffer);

   if (len > BUFFERSIZE)
     I_Error("DDF_MainProcessChar: Read Buffer Size Exceeded, Size: %d\n", BUFFERSIZE);

   // With the exception of reading_string, whitespace is ignored and
   // a SUBSPACE is replaced by a space.
   if (status != reading_string)
   {
     if (isspace(character))
       return nothing;

     if (character == SUBSPACE)
       character = SPACE;
   }
   else // check for formatting char in a string
   {
     if (character == '\\')
     {
       formatchar = true;
       return nothing;
     }
   }

   switch (status)
   {
     case reading_remark:
       if (character == REMARKSTOP)
         return remark_stop;
       else
         return nothing;
       break;

    case waiting_newdef:
      if (character == REMARKSTART) // -ACB- 1998/07/31 Missed this out first time
        return remark_start;
      else if (character == DEFSTART)
        return def_start;
      else
        return nothing;
      break;

    case reading_newdef:
      if (character == REMARKSTART)
      {
        return remark_start;
      }
      else if (character == DEFSTOP)
      {
        return def_stop;
      }
      else if ( (isalnum(character)) || (character == SPACE))
      {
        sprintf(concat,"%c",character);
        strcat(buffer,concat);
        return ok_char;
      }
      return nothing;
      break;

   case reading_command:
     if (character == REMARKSTART)
     {
       return remark_start;
     }
     else if (character == COMMANDREAD)
     {
       return command_read;
     }
     else if (character == DEFSTART)
     {
       return def_start;
     }
     else if (isalnum(character) || character == SPACE)
     {
       sprintf(concat,"%c",character);
       strcat(buffer,concat);
       return ok_char;
     }

     return nothing;
     break;

   // -ACB- 1998/08/10 Check for string start
   case reading_data:
     if (character == STRINGSTART)
     {
       return string_start;
     }
     if (character == REMARKSTART)
     {
       return remark_start;
     }
     else if (character == TERMINATOR)
     {
       return terminator;
     }
     else if (character == SEPERATOR)
     {
       return seperator;
     }
     // Sprite Data - more than a few exceptions....
     else if (isalnum(character) || character == SPACE || character == '-' ||
              character == ':' || character == '.' || character == '[' ||
              character == ']' || character == '\\' || character == '!' ||
              character == '#' )
     {
       sprintf(concat,"%c",character);
       strupr(concat);
       strcat(buffer,concat);
       return ok_char;
     }
     break;

   case reading_string: // -ACB- 1998/08/10 New string handling
     // -KM- 1999/01/29 Fixed nasty bug where \" would be recognised as
     //  string end over quote mark.  One of the level text used this.
     if (formatchar)
     {
       // -ACB- 1998/08/11 Formatting check: Carriage-return.
       if (character == 'n')
       {
         sprintf(concat,"%c",'\n');
         strcat(buffer,concat);
         formatchar = false;
         return ok_char;
       }
       // -KM- 1998/10/29 Also recognise quote
       else if (character == '\"')
       {
         sprintf(concat,"%c",'\"');
         strcat(buffer,concat);
         formatchar = false;
         return ok_char;
       }
     }
     else if (character == STRINGSTOP)
     {
       return string_stop;
     }
     // -KM- 1998/10/29 Removed ascii check, allow foreign characters („)
     else
     {
       sprintf(concat,"%c",character);
       strcat(buffer,concat);
       return ok_char;
     }
     break;

   default:  // doh!
     break;
  }

  return nothing;
}

//
// DDF_MainGetNumeric
//
// Get numeric value directly from the file
//
void DDF_MainGetNumeric(char *info, int commandref)
{
  int i;

  // -KM- 1999/01/29 strtol accepts hex and decimal.
  i = strtol(info, NULL, 0); // straight conversion - no messin'

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetNumeric: Integer not specified\n");

  // data is a void pointer, use memcpy to transfer info -ACB- 1998/07/31
  memcpy(currentcmdlist[commandref].data, &i, sizeof(int));

  return;
}


//
// DDF_MainGetBoolean
//
// Get true/false from the file
//
// -KM- 1998/09/01 Gets a true/false value
//
void DDF_MainGetBoolean(char *info, int commandref)
{
  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetBoolean: Target not specified! (Internal bug)\n");

  if (!strcasecmp(info, "TRUE"))
    *(boolean *) currentcmdlist[commandref].data = true;
  else if (!strcasecmp(info, "FALSE"))
    *(boolean *) currentcmdlist[commandref].data = false;
#ifdef DEVELOPERS
  else
    I_Error("Boolean is neither True nor False??!!\n");
#endif
}

//
// DDF_MainGetString
//
// Get String value directly from the file
//
// -KM- 1998/07/31 Needed a string argument.  Based on DDF_MainGetNumeric.
//
void DDF_MainGetString(char *info, int commandref)
{
  char *string;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetString: Destination not specified\n");

  string = strdup(info);

  if (string==NULL)
    I_Error("\nDDF_MainGetString: Malloc Failed\n");

  memcpy((char *)currentcmdlist[commandref].data, &string, sizeof(char *));

  return;
}

//
// DDF_MainLookupSound
//
// Lookup the sound specificed.
//
// -ACB- 1998/07/08 Checked the S_sfx table for sfx names.
// -ACB- 1998/07/18 Removed to the need set *currentcmdlist[commandref].data to -1
// -KM- 1998/09/27 Fixed this func because of sounds.ddf
void DDF_MainLookupSound(char *info, int commandref)
{
#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainLookupSound: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainLookupSound: Integer not specified\n");
#endif

  // -KM- 1998/10/29 sfx_t finished
  *(sfx_t **) currentcmdlist[commandref].data = DDF_LookupSound(info);

  return;
}

//
// DDF_MainCheckCommand
//
// Check command exists, and then return reference.
//
int DDF_MainCheckCommand(char *info)
{
  int i;

  i=0;

  while (strcmp(info,currentcmdlist[i].name)
        && strcmp(currentcmdlist[i].name,COMMAND_TERMINATOR))
  {
    i++;
  }

  if (!strcmp(currentcmdlist[i].name,COMMAND_TERMINATOR))
    I_Error("\nDDF_MainCheckCommand: Unknown Command - %s\n", info);

  return i;
}

//
// DDF_MainMobjCheckName
//
// Check the Names is unique to any other thing, then give it to buffermobj.
//
void DDF_MainMobjCheckName(char *info)
{
  mobjinfo_t* entry;

  if (info == NULL)
    I_Error("Info has no info\n");

  entry = mobjinfohead;
  bufferreplacemobj = NULL;

  while (entry != NULL)
  {
    if (entry->name != NULL)
    {
      if (!strcmp(info,entry->name))
      {
        if (ddf_replace)
          bufferreplacemobj = entry;
        else
          I_Error("DDF_MainCheckName: '%s' already declared\n",info);
      }
    }

   entry = entry->next;
  }

  if ((buffermobj.name = malloc(sizeof(char)*(strlen(info)+1)))==NULL)
    I_Error("DDF_MainCheckName: Unable to allocate memory\n");

  strcpy(buffermobj.name,info);

  return;
}

//
// DDF_MainGetSpecial
//
// Compares info the the entries in specialslist: if found apply attribs for it
//
void DDF_MainGetSpecial(char *info, int commandref)
{
  int i;

  i=0;

  while ( strcmp(info,specialslist[i].specialname) &&
          strcmp(specialslist[i].specialname, COMMAND_TERMINATOR) )
  {
    i++;
  }

  if (!strcmp(specialslist[i].specialname, COMMAND_TERMINATOR))
    I_Error("\n\tDDF_MainGetSpecial: No such special '%s'",info);

  if (specialslist[i].flags)
    buffermobj.flags |= specialslist[i].flags;

  if (specialslist[i].extendedflags)
    buffermobj.extendedflags |= specialslist[i].extendedflags;
// -KM- 1998/11/25 Removed translucency test.
}

//
// DDF_MainSplitIntoState
//
// Small procedure that takes the info and splits it into relevant stuff
//
// -KM- 1998/12/21 Rewrote procedure, much cleaner now.
int DDF_MainSplitIntoState(char *info)
{
  char *temp;
  int i;

  for (i = NUMSPLIT; i--; )
     stateinfo[i] = NULL;

  for (temp = strtok(info, ":"), i = 0; temp && i < NUMSPLIT; temp = strtok(NULL, ":"), i++)
  {
    if (temp[0] == REDIRECTOR)
    {
       stateinfo[2] = NULL; // signify that we have found redirector
       stateinfo[0] = strdup(temp + 1);

       temp = strtok(NULL, ":");
       if (temp)
         stateinfo[1] = strdup(temp);
       else
         stateinfo[1] = NULL;

       return -1;
    }

    stateinfo[i] = strdup(temp);
  }
  return i;
}

//
// DDF_MainLoadStates
//
// Quite frankly this is not the greatest code written, loads the states table
//
void DDF_MainLoadStates(char *info, int commandref)
{
  int count,i,j;
  char *backupinfo;

  backupinfo = info;
  count = statecount - NUMSTATES;

  if ((tempstates[count]=malloc(sizeof(state_t)))==NULL)
    I_Error("DDF_ItemLoadSprites: Unable to allocate memory!\n");

  tempstates[count]->nextstate = statecount + 1;

  // Split the state info into component parts
  // -ACB- 1998/07/26 New Procedure, for cleaner code.
  i = DDF_MainSplitIntoState(info);
  if (i < 5 && i >=0)
    I_Error("\n\tDDF_MainLoadStates: Bad state '%s'\n", info);

  if (stateinfo[0]==NULL)
    I_Error("\n\tDDF_MainLoadStates: Stateinfo is NULL\n");

  //--------------------------------------------------
  //----------------REDIRECTOR HANDLING---------------
  //--------------------------------------------------
  if(stateinfo[2]==NULL)
  {
    if (!terminated)
      I_Printf("\n\tDDF_MainLoadStates: Warning - redirector terminates\n");

    // Copy Tempstates to states table......
    // -KM- 1999/01/29 Ouch, nast bug. > should be >=
    if (statecount >= MAXSTATES)
    {
      MAXSTATES = statecount + 32;
      states = realloc(states, sizeof(*states) * MAXSTATES);
    }

    // Use Numeric ref
    memcpy(currentcmdlist[commandref].data, &NUMSTATES, sizeof(int));

    for (i = 0; i <= (count-1); i++)
    {
      states[NUMSTATES+i].sprite    = tempstates[i]->sprite;
      states[NUMSTATES+i].frame     = tempstates[i]->frame;
      states[NUMSTATES+i].tics      = tempstates[i]->tics;
      states[NUMSTATES+i].nextstate = tempstates[i]->nextstate;
      states[NUMSTATES+i].action    = tempstates[i]->action;
      states[NUMSTATES+i].misc1     = tempstates[i]->misc1;
      states[NUMSTATES+i].misc2     = tempstates[i]->misc2;

      free(tempstates[i]);
    }

    if (!strcmp(stateinfo[0],"REMOVE"))
    {
      states[statecount-1].nextstate = S_NULL;
    }
    else
    {
      i=j=0;

      //
      // This checks the redirector name and if one is found, gives     -ACB-
      // a subflag an places it in the nextstate reference, this is       |
      // dereferences when the object is finalised (in DDF_MOBJ.C),       |
      // so this only for object defines. Any offset (like chase state   \|/
      // + one frame) is added to the state, this will be sorted when
      // the program derefs the flag.
      //
      while(strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector) &&
             strcmp(framedestlist[i].redirector,stateinfo[0]))
      {
        i++;
      }

      if (!strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector))
        I_Error("\n\tDDF_MainLoadStates: Unknown Redirector %s\n",stateinfo[0]);

      j = framedestlist[i].subflag;

      i=0;

      if (stateinfo[1] != NULL)
      {
        i = atoi(stateinfo[1])-1;

        if (i < 0)
          i = 0;
      }

      states[statecount-1].nextstate = j + i;

                                                                      /* /|\ */
      // End of sprite referencing                                        |
      //                                                                  |
      //                                                                -ACB-
    }

    NUMSTATES = statecount;
    return;
  }

  //--------------------------------------------------
  //----------------SPRITE NAME HANDLING--------------
  //--------------------------------------------------
  if (strlen(stateinfo[0]) != 4)
    I_Error("\nDDF_MainLoadStates: Sprite Names must be 4 Characters long '%s'.\n",
            stateinfo[0]);

  i=0;
  while (i != NUMSPRITES && strcmp(stateinfo[0],sprnames[i]))
    i++;

  if (i==NUMSPRITES)
  {
    if ((sprnames[NUMSPRITES] = strdup(stateinfo[0]))==NULL)
      I_Error("DDF_MainSpriteName: Unable to allocate enough memory!\n");

    tempstates[count]->sprite = NUMSPRITES;
    NUMSPRITES++;

    // -KM- 1999/01/29 > should have been >=
    if (NUMSPRITES >= MAXSPRITES)
    {
      MAXSPRITES += 32;
      sprnames = realloc(sprnames, sizeof(*sprnames) * MAXSPRITES);
    }
  }
  else
  {
    tempstates[count]->sprite = i;
  }

  //--------------------------------------------------
  //--------------SPRITE INDEX HANDLING---------------
  //--------------------------------------------------

  j = stateinfo[1][0];                  // look at the first character

  if ( j < 'A' || j > ']' )             // check for bugger up
    I_Error("\nDDF_MainLoadStates: Illegal Sprite Index %c\n", j);

  tempstates[count]->frame = (long) (j-65);

  //--------------------------------------------------
  //------------STATE TIC COUNT HANDLING--------------
  //--------------------------------------------------
  tempstates[count]->tics = atol(stateinfo[2]);

  //--------------------------------------------------
  //------------STATE BRIGHTNESS LEVEL----------------
  //--------------------------------------------------
  if (!strcmp("BRIGHT",stateinfo[3]))
    tempstates[count]->frame = tempstates[count]->frame + FF_FULLBRIGHT;
  else if ( strcmp("NORMAL",stateinfo[3]) )
    I_Error("\n\tDDF_MainLoadStates: Lighting is not BRIGHT or NORMAL\n");

  //--------------------------------------------------
  //------------STATE ACTION CODE HANDLING------------
  //--------------------------------------------------

  // Get Action Code Ref (Using remainder of the string).
  // Go through all the actions, end if terminator or action found
  i=0;
  while (strcmp(actions[i].actionname,stateinfo[4]) &&
          strcmp(actions[i].actionname,COMMAND_TERMINATOR))
  {
    i++;
  }

  if (!strcmp(actions[i].actionname,COMMAND_TERMINATOR))
    I_Error("\n\tDDF_MainLoadStates: Unknown Command: %s\n",stateinfo[4]);

  tempstates[count]->action = actions[i].action;

  //--------------------------------------------------
  //--------------------------------------------------
  // Misc1 + 2
  // -KM- 1999/01/29 Use strtol instead of atoi.
  if (stateinfo[5])
    tempstates[count]->misc1 = strtol(stateinfo[5], NULL, 0);
  else
    tempstates[count]->misc1 = 0;
  if (stateinfo[6])
    tempstates[count]->misc2 = strtol(stateinfo[6], NULL, 0);
  else
    tempstates[count]->misc2 = 0;

  // Terminator Found - Pass the states to the main state table, free up mem
  if (terminated)
  {
    // data is a void pointer, use memcpy to transfer info -ACB- 1998/07/31
    memcpy(currentcmdlist[commandref].data, &NUMSTATES, sizeof(int));

    // -KM- 1999/01/29 Again > should be >=
    if (statecount >= MAXSTATES)
    {
      MAXSTATES = statecount + 32;
      states = realloc(states, sizeof(*states) * MAXSTATES);
    }

    for (i = 0; i <= count; i++)
    {
      states[NUMSTATES+i].sprite    = tempstates[i]->sprite;
      states[NUMSTATES+i].frame     = tempstates[i]->frame;
      states[NUMSTATES+i].tics      = tempstates[i]->tics;
      states[NUMSTATES+i].nextstate = tempstates[i]->nextstate;
      states[NUMSTATES+i].action    = tempstates[i]->action;
      states[NUMSTATES+i].misc1     = tempstates[i]->misc1;
      states[NUMSTATES+i].misc2     = tempstates[i]->misc2;

      free(tempstates[i]);
    }

    states[statecount].nextstate = NUMSTATES;

    NUMSTATES = statecount + 1;
  }

  info = backupinfo;
  statecount++;
}

//
// DDF_MainGetLumpName
//
// Gets the string and checks the length to see if is not more than 8.
//
void DDF_MainGetLumpName(char *info, int commandref)
{
  char *lumpname;

  lumpname = (char *) currentcmdlist[commandref].data;

  if (strlen(info) > 8)
    I_Error("\nDDF_MainGetLumpName: Lump name %s too big\n", info);

  if (lumpname == NULL)
    I_Error("\nDDF_MainGetLumpName: Destination not specified\n");

  strcpy(lumpname, info);

  return;
}

//
// DDF_MainGetMusicName
//
// Gets the string and checks the length is not bigger than 6, if
// this is the case D_ is prefixed and then it is stored.
//
void DDF_MainGetMusicName(char *info, int commandref)
{
  char *lumpname;

  lumpname = (char *) currentcmdlist[commandref].data;

  if (strlen(info) > 6)
    I_Error("\nDDF_MainGetMusicName: Music name too big\n");

  if (lumpname == NULL)
    I_Error("\nDDF_MainGetMusicName: Destination not specified\n");

  strcpy(lumpname, "D_");
  strcat(lumpname, info);

  return;
}

void DDF_MainReferenceString(char *info, int commandref)
{
  char *string;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainReferenceString: Destination not specified\n");

  string = DDF_LanguageLookup(info);

  memcpy((char *)currentcmdlist[commandref].data, &string, sizeof(char *));
}

void DDF_MainRefAttack(char *info, int commandref)
{
  attacktype_t* currattack;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainRefAttack: Destination not specified\n");

  currattack = attackhead;

  while (currattack != NULL && strcmp(currattack->name,info))
    currattack = currattack->next;

  if (currattack == NULL)
    I_Error("\n\tDDF_MainRefAttack: Attack - '%s' - does not exist\n",info);

  memcpy((attacktype_t *)currentcmdlist[commandref].data,
           &currattack, sizeof(attacktype_t *));
}

//
// DDF_MainGetDirector
//
void DDF_MainGetDirector (char *info, int commandref)
{
  int i,j;
  char *director;
  char *offsetstring;

  offsetstring = strchr(info,DIVIDE);        // find DIVIDE

  i = strlen(info) - strlen(offsetstring);

  if (i <= 1)
    I_Error("\n\tDDF_MainGetDirector: Nothing after divide\n");

  director = malloc(sizeof(char)*(i+1));
  memset(director,'\0',sizeof(char)*(i+1));

  if (director == NULL)
    I_Error("DDF_MainGetDirector:  Malloc failure");

  director = strncpy(director,info,i);
  offsetstring++; // step over divide

  i=0;

  while(strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector) &&
         strcmp(framedestlist[i].redirector,director))
  {
    i++;
  }

  if (!strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector))
    I_Error("\n\tDDF_MainGetDirector: Unknown Director %s\n",director);

  j = framedestlist[i].subflag;

  i=0;

  if (offsetstring != NULL)
  {
    i = atoi(offsetstring)-1;

    if (i < 0)
      i = 0;
  }

  i += j;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetNumeric: Integer not specified\n");

  memcpy(currentcmdlist[commandref].data, &i, sizeof(int));
}
void DDF_MainGetAccuracy(char *info, int commandref)
{
  int i = 0;

  // this is cheap...
  if (!(strcmp(info, "HIGH")))
    i = HIGH;
  else if (!(strcmp(info, "MEDIUM")))
    i = MEDIUM;
  else if (!(strcmp(info, "LOW")))
    i = LOW;
  else if (!(strcmp(info, "PATHETIC")))
    i = PATHETIC;

  if (!i)
    I_Error("\n\tDDF_AtkGetAccuracy: Unknown accuracy level - %s",info);

  *(int *)currentcmdlist[commandref].data = i;
}
//
// DDF_MainGetFixed
//
// This procedure gets a number and translates it into a fixed
// number (in Doom Terms). The translated value can have a decimal point...
//
// -ACB- 1998/09/12 Procedure Written.
//
static inline fixed_t DDF_MainGetFixedHelper(char *info, int fixed)
{
  char *temp;
  fixed_t i = 0, f = 0;
  int j;

  // Get the integer part
  // -KM- 1999/01/29 Use strtol instead of atoi.  Don't know how this works
  //  in hex for fixed point...
  i = strtol(strtok(info, "."), NULL, 0) * fixed;

  // Get the decimal part
  temp = strtok(NULL, ".");
  if (temp)
  {
    f = (fixed * strtol(temp, NULL, 0));
    // -KM- 1998/11/25 Fixed major bug, that resulted in incorrect fracs
    //   for anything with more than one decimal place, eg 0.3 worked, but
    //   0.33333 would not.
    for (j = strlen(temp); j > 0; j--)
      f /= 10;
  }
  return i + f;
}

void DDF_MainGetFixed(char *info, int commandref)
{
#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetFixed: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetFixed: Integer not specified\n");
#endif
  *(fixed_t *)currentcmdlist[commandref].data = DDF_MainGetFixedHelper(info, FRACUNIT);
}

// -KM- 1998/09/27 You can end a number with T to specify tics; ie 35T means 35 tics
// while 3.5 means 3.5 seconds.
void DDF_MainGetTime(char *info, int commandref)
{
#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetTime: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetTime: Integer not specified\n");
#endif
  if (strchr(info, 'T'))
    DDF_MainGetNumeric(info, commandref);
  else
    *(int *)currentcmdlist[commandref].data = DDF_MainGetFixedHelper(info, TICRATE);
}
/*
void DDF_MainGetFixed(char *info, int commandref)
{
  int total = 0;
  int divnum = 0;
  int i, j, k;
  char *string;
  char *newstring;

#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetFixed: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetFixed: Integer not specified\n");
#endif

  string = strchr(info,'.');        // find decimal point

  if (!string) // no decimal point
  {
    *(int *)currentcmdlist[commandref].data = strtol(info, NULL, 0)<<FRACBITS;
    return;
  }

  i = strlen(info) - strlen(string);

  string++; // step over decimal point

  if (!string) // nothing after the decimal point
  {
    *(int *)currentcmdlist[commandref].data = strtol(info, NULL, 0)<<FRACBITS;
    return;
  }

  for (j=0; j<(i+1); j++)
  {
    divnum = 1;

    for (k=0; k<(j+1); k++) // calc division number (10 to the power of (j+1))
     divnum *= 10;

    total += FixedDiv((string[j]-'0')<<FRACBITS, divnum);

  }

  newstring = malloc(sizeof(char)*(i+1));

  if (!newstring)
    I_Error("DDF_MainGetFixed: Malloc failure on newstring");

  strncpy(newstring, info, i);
  j = atoi(newstring)<<FRACBITS;

  // Check for negative number.
  if (newstring[0] == '-')
    total = j - total;
  else
    total = j + total;

  *(int *)currentcmdlist[commandref].data = total;

  free(string);
}

//
// DDF_MainGetTime
//
// This procedure gets a number and translates it into the number of
// tics (using the TICRATE define), also allowed is decimal point values.
//
// -ACB- 1998/09/12 Procedure Written.
//
void DDF_MainGetTime(char *info, int commandref)
{
  int total = 0;
  int divnum = 0;
  int i, j, k;
  char *string;
  char *newstring;

#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetTime: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetTime: Integer not specified\n");
#endif

  string = strchr(info,'.');        // find decimal point

  if (!string) // no decimal point
  {
    *(int *)currentcmdlist[commandref].data = strtol(info, NULL, 0)*TICRATE;
    return;
  }

  i = strlen(info) - strlen(string);

  string++; // step over decimal point

  if (!string) // nothing after the decimal point
  {
    *(int *)currentcmdlist[commandref].data = strtol(info, NULL, 0)*TICRATE;
    return;
  }

  for (j=0; j<(i+1); j++)
  {
    divnum = 1;

    for (k=0; k<(j+1); k++) // calc division number (10 to the power of (j+1))
     divnum *= 10;

    total += ((string[j]-'0')*TICRATE)/divnum;

  }

  newstring = malloc(sizeof(char)*(i+1));

  if (!newstring)
    I_Error("DDF_MainGetTime: Malloc failure on newstring");

  strncpy(newstring, info, i);
  j += atoi(newstring)*TICRATE;

  // Check for negative number. Time is always a positive value...
  if (newstring[0] == '-')
    j = 0 - j;

  total += j;

  *(int *)currentcmdlist[commandref].data = total;

  free(string);
}
*/

// DDF_DummyFunction
void DDF_DummyFunction(char *info) {return;};

//
// DDF_OldThingInit
//
// Temporary code that adjusts the current info, so the games plays as Doom
// a.k.a. The most hacked code going
//
void DDF_OldThingInit()
{
  // -KM- 1999/01/29 Removed fixed old things.
  specials[MOBJ_SPAWNSPOT]   = &oldmobjinfo[MT_BOSSTARGET];
}

// -KM- 1998/10/29 Add stuff from ddf_lines.c that is also shared by ddf_sect.c

static struct
{
  char *s;
  int  n;
}

s_exit[] =
{
 {"NONE", 0},
 {"EXIT", 1},
 {"SECRET", 2}
},

s_lite[] =
{
  {"SET", lite_set},
  {"STROBE", lite_strobe},
  {"FLASH", lite_flash},
  {"GLOW", lite_glow},
  {"FLICKER", lite_fireflicker}
},

s_movement[] =
{
  {"MOVE"          , mov_Once},
  {"MOVEWAITRETURN", mov_MoveWaitReturn},
  {"CONTINUOUS"    , mov_Continuous},
  {"PLAT"          , mov_Plat},
  {"BUILDSTAIRS"   , mov_Stairs},
  {"STOP"          , mov_Stop}
},

s_reference[] =
{
  {"ABSOLUTE",             ref_absolute},
  {"FLOOR",                ref_floorHeight},
  {"CEILING",              ref_ceilingHeight},
  {"LOSURROUNDINGCEILING", ref_LowestSurroundingCeiling},
  {"HISURROUNDINGCEILING", ref_HighestSurroundingCeiling},
  {"LOSURROUNDINGFLOOR",   ref_LowestSurroundingFloor},
  {"HISURROUNDINGFLOOR",   ref_HighestSurroundingFloor},
  {"NEXTHIGHESTFLOOR",     ref_NextHighestFloor},
  {"LOWESTBOTTOMTEXTURE",  ref_LowestLoTexture}
};
//
// DDF_LineGetExit
//
// Get the exit type
//
void DDF_MainGetExit(char *info, int c)
{
  int i;

  for (i = sizeof(s_exit) / sizeof(s_exit[0]); i--;)
  {
    if (!strcasecmp(info, s_exit[i].s))
    {
      *(int *)currentcmdlist[c].data = s_exit[i].n;
      return;
    }
  }

  I_Error("\nUnknown Exit type: %s\n", info);
}

//
// DDF_LineGetLighttype
//
// Get the light type
//
void DDF_MainGetLighttype(char *info, int c)
{
  int i;

  for (i = sizeof(s_lite) / sizeof(s_lite[0]); i--;)
  {
    if (!strcasecmp(info, s_lite[i].s))
    {
      *(int *)currentcmdlist[c].data = s_lite[i].n;
      return;
    }
  }

  I_Error("\nUnknown light type: %s\n", info);
}

//
// DDF_LineGetMType
//
// Get movement types: MoveWaitReturn etc
//
void DDF_MainGetMType(char *info, int c)
{
  int i;

  for (i = sizeof(s_movement) / sizeof(s_movement[0]); i--;)
  {
    if (!strcasecmp(info, s_movement[i].s))
    {
      *(int *)currentcmdlist[c].data = s_movement[i].n;
      return;
    }
  }

  I_Error("\nUnknown Movement type %s\n", info);
}

//
// DDF_LineGetDestRef
//
// Get surroundingsectorceiling/floorheight etc
//
void DDF_MainGetDestRef(char *info, int c)
{
  int i;

  for (i = sizeof(s_reference) / sizeof(s_reference[0]); i--;)
  {
    if (!strcasecmp(info, s_reference[i].s))
    {
      *(int *)currentcmdlist[c].data = s_reference[i].n;
      return;
    }
  }

  I_Error("\nUnknown Reference Point %s\n", info);
}

void DDF_MobjGetPlayer(char *info, int commandref)
{
  DDF_MainGetNumeric(info, commandref);
  if (*(int *)currentcmdlist[commandref].data > maxplayers)
    maxplayers = *(int *)currentcmdlist[commandref].data;
}


