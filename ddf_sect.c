//
// DOSDoom Definition File Codes (Sectors)
//
// By the DOSDoom Team
//
// Sector Setup and Parser Code
//
// -KM- 1998/09/27 Written.
//
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "p_local.h"
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

#define DDF_SectHashFunc(x) (x & 255)


static specialsector_t buffersect;
static const specialsector_t defaultSec = {
  0, // Trigger
  false, // secret
  NULL, // Lump check
  8, // Gravity
  FRICTION, // Friction
  false, // crush
  {mov_undefined, false}, // Floor
  {mov_undefined, true}, // Ceiling
  {lite_none}, // lights
  0, // damage
  0, // damagetime
  0, // exit
  "\0", // Colourmaplump
  -1, // Colourmap
  0, // SFX
  0, // viscosity
  false // Hash table
};

static specialsector_t *specialSecs[256] = { [0 ... 255] = NULL};

commandlist_t sectcommands[] =
{{"NUMBER"               , DDF_MainGetNumeric,      &buffersect.trigger},
 {"SECRET"               , DDF_MainGetBoolean,      &buffersect.secret},
 {"DAMAGE"               , DDF_MainGetNumeric,      &buffersect.damage},
 {"DAMAGETIME"           , DDF_MainGetTime,         &buffersect.damagetime},
 {"CRUSH"               , DDF_MainGetBoolean,         &buffersect.crush},
 {"FLOOR TYPE"          , DDF_MainGetMType,           &buffersect.f.type},
 {"FLOOR SPEED UP"      , DDF_MainGetFixed,           &buffersect.f.speed_up},
 {"FLOOR SPEED DOWN"    , DDF_MainGetFixed,           &buffersect.f.speed_down},
 {"FLOOR DEST REF"      , DDF_MainGetDestRef,         &buffersect.f.destref},
 {"FLOOR DEST OFFSET"   , DDF_MainGetFixed,           &buffersect.f.dest},
 {"FLOOR TEXTURE"       , DDF_MainGetString,          &buffersect.f.tex},
 {"FLOOR PAUSE TIME"    , DDF_MainGetTime,            &buffersect.f.wait},
 {"FLOOR WAIT TIME"     , DDF_MainGetTime,            &buffersect.f.prewait},
 {"FLOOR SFX START"     , DDF_MainLookupSound,        &buffersect.f.sfxstart},
 {"FLOOR SFX UP"        , DDF_MainLookupSound,        &buffersect.f.sfxup},
 {"FLOOR SFX DOWN"      , DDF_MainLookupSound,        &buffersect.f.sfxdown},
 {"FLOOR SFX STOP"      , DDF_MainLookupSound,        &buffersect.f.sfxstop},
 {"CEILING TYPE"        , DDF_MainGetMType,           &buffersect.c.type},
 {"CEILING SPEED UP"    , DDF_MainGetFixed,           &buffersect.c.speed_up},
 {"CEILING SPEED DOWN"  , DDF_MainGetFixed,           &buffersect.c.speed_down},
 {"CEILING DEST REF"    , DDF_MainGetDestRef,         &buffersect.c.destref},
 {"CEILING DEST OFFSET" , DDF_MainGetFixed,           &buffersect.c.dest},
 {"CEILING TEXTURE"     , DDF_MainGetString,          &buffersect.c.tex},
 {"CEILING PAUSE TIME"  , DDF_MainGetTime,            &buffersect.c.wait},
 {"CEILING WAIT TIME"   , DDF_MainGetTime,            &buffersect.c.prewait},
 {"CEILING SFX START"   , DDF_MainLookupSound,        &buffersect.c.sfxstart},
 {"CEILING SFX UP"      , DDF_MainLookupSound,        &buffersect.c.sfxup},
 {"CEILING SFX DOWN"    , DDF_MainLookupSound,        &buffersect.c.sfxdown},
 {"CEILING SFX STOP"    , DDF_MainLookupSound,        &buffersect.c.sfxstop},
 {"LIGHT TYPE"           , DDF_MainGetLighttype,    &buffersect.l.type},
 {"LIGHT LEVEL"          , DDF_MainGetNumeric,      &buffersect.l.light},
 {"LIGHT DARKTIME"       , DDF_MainGetTime,         &buffersect.l.darktime},
 {"LIGHT BRIGHTTIME"     , DDF_MainGetTime,         &buffersect.l.brighttime},
 {"LIGHT PROBABILITY"    , DDF_MainGetNumeric,      &buffersect.l.light},
 {"LIGHT SYNC"           , DDF_MainGetTime,         &buffersect.l.sync},
 {"EXIT"                 , DDF_MainGetExit,         &buffersect.exit},
 {"COLOURMAPLUMP"        , DDF_MainGetLumpName,     &buffersect.colourmaplump},
 {"COLOURMAP"            , DDF_MainGetNumeric,      &buffersect.colourmap},
 {"GRAVITY"              , DDF_MainGetNumeric,      &buffersect.gravity},
 {"FRICTION"             , DDF_MainGetFixed,        &buffersect.friction},
 {"VISCOSITY"            , DDF_MainGetFixed,        &buffersect.viscosity},
 {"SOUND"                , DDF_MainLookupSound,     &buffersect.sfx},
 {COMMAND_TERMINATOR     , NULL,                    NULL}};

//
// DDF_SectAdd2HashTable
//
// Adds a sector definition to the hash table
//
// -KM-  1998/09/19 Procedure Written; stolen from linedefs :)
//
static void DDF_SectAdd2HashTable(specialsector_t* info)
{
  int slot = DDF_SectHashFunc(info->trigger);

  if (!specialSecs[slot])
  {
    specialSecs[slot] = malloc(sizeof(specialsector_t));
    memcpy(specialSecs[slot], info, sizeof(specialsector_t));
    specialSecs[slot][0].next = false;
  }
  else
  {
    int count;
    specialsector_t* index = specialSecs[slot];

    for (count = 1; index->next; count++, index++);
    {
      specialSecs[slot]
        = realloc(specialSecs[slot], (count + 1) * sizeof(specialsector_t));
    }

    memcpy(&specialSecs[slot][count], info, sizeof(specialsector_t));
    specialSecs[slot][count - 1].next = true;
    specialSecs[slot][count].next = false;
  }
}

//
// DDF_SectCheckNum
//
// Checks that the sector number is not already declared.
//
// -KM-  1998/09/19 Wrote Procedure. From ddf_line.c
//
void DDF_SectCheckNum(char *info)
{
  int i = 0, num, slot;

#ifdef DEVELOPERS
 if (info == NULL)
   I_Error("\nInfo has no info\n");
#endif

  num = atoi(info);
  slot = DDF_SectHashFunc(num);

  if (specialSecs[slot])
  {
    do
    {
      if (specialSecs[slot][i].trigger == num)
        I_Error("\nDDF_SectCheckNum: %d already declared\n",num);
    }
    while (specialSecs[slot][i++].next);
  }

  memcpy(&buffersect, &defaultSec, sizeof(buffersect));
  buffersect.trigger = num;
  buffersect.colourmap = -1;

  return;
}

void DDF_SectCreate(void)
{
  DDF_SectAdd2HashTable(&buffersect);
}

void DDF_ReadSectors(void *data, int size)
{
  readinfo_t sects;
   
  if (!data)
  {
    sects.message               = "DDF_InitSpecialSects";
    sects.filename              = "sectors.ddf";
    sects.memfile = NULL;
  } else {
    sects.message = NULL;
    sects.memfile = data;
    sects.memsize = size;
  }
  sects.DDF_MainCheckName     = DDF_SectCheckNum;
  sects.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  sects.DDF_MainCreateEntry   = DDF_SectCreate;
  sects.DDF_MainFinishingCode = DDF_SectCreate;
  sects.cmdlist               = sectcommands;
    
  DDF_MainReadFile(&sects);
}

void DDF_SectorInit()
{
  DDF_ReadSectors(NULL, 0);
}

//
// DDF_GetFromSectHashTable
//
// Returns the special sector properties from given specialtype
//
// -KM-  1998/09/19 Wrote Procedure
//
specialsector_t* DDF_GetFromSectHashTable(int trigger)
{
  int slot = DDF_SectHashFunc(trigger);
  int i = 0;

  // Find in hash table
  do
  {
    if (specialSecs[slot][i].trigger == trigger)
      return &specialSecs[slot][i];
  }
  while(specialSecs[slot][i++].next);

  I_Error("Unknown Special Sector type %d\n", trigger);
}

/*
  Need to implement:
DOOM:
  Secret Sectors.
  Lights.
  Damage.
  Doors.
  Exit.
DOSDOOM:
  Spawn MOBJ's randomly
  SFX
  Colourmap
  Gravity
*/
/*
	  case 1:
	    // FLICKERING LIGHTS
	    P_SpawnLightFlash (sector);
	    break;

	  case 2:
	    // STROBE FAST
	    P_SpawnStrobeFlash(sector,FASTDARK,0);
	    break;
	    
	  case 3:
	    // STROBE SLOW
	    P_SpawnStrobeFlash(sector,SLOWDARK,0);
	    break;
	    
	  case 4:
	    // STROBE FAST/DEATH SLIME
	    P_SpawnStrobeFlash(sector,FASTDARK,0);
	    sector->special = 4;
	    break;
	    
	  case 8:
	    // GLOWING LIGHT
	    P_SpawnGlowingLight(sector);
	    break;
	  case 9:
	    // SECRET SECTOR
	    totalsecret++;
	    break;
	    
	  case 10:
	    // DOOR CLOSE IN 30 SECONDS
	    P_SpawnDoorCloseIn30 (sector);
	    break;
	    
	  case 12:
	    // SYNC STROBE SLOW
	    P_SpawnStrobeFlash (sector, SLOWDARK, 1);
	    break;

	  case 13:
	    // SYNC STROBE FAST
	    P_SpawnStrobeFlash (sector, FASTDARK, 1);
	    break;

	  case 14:
	    // DOOR RAISE IN 5 MINUTES
	    P_SpawnDoorRaiseIn5Mins (sector, i);
	    break;
	    
	  case 17:
	    P_SpawnFireFlicker(sector);
	    break;
      case 5:
	// HELLSLIME DAMAGE
	if (!player->powers[pw_ironfeet])
	    if (!(leveltime&0x1f))
		P_DamageMobj (player->mo, NULL, NULL, 10);
	break;
	
      case 7:
	// NUKAGE DAMAGE
	if (!player->powers[pw_ironfeet])
	    if (!(leveltime&0x1f))
		P_DamageMobj (player->mo, NULL, NULL, 5);
	break;
	
      case 16:
	// SUPER HELLSLIME DAMAGE
      case 4:
	// STROBE HURT
	if (!player->powers[pw_ironfeet]
	    || (P_Random()<5) )
	{
	    if (!(leveltime&0x1f))
		P_DamageMobj (player->mo, NULL, NULL, 20);
	}
	break;
			
      case 9:
	// SECRET SECTOR
	player->secretcount++;
	sector->special = 0;
	break;
			
      case 11:
	// EXIT SUPER DAMAGE! (for E1M8 finale)
	player->cheats &= ~CF_GODMODE;

	if (!(leveltime&0x1f))
	    P_DamageMobj (player->mo, NULL, NULL, 20);

	if (player->health <= 10)
	    G_ExitLevel();
	break;
*/
