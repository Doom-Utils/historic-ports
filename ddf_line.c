//
// DOSDoom Definition File Codes (Line defs)
//
// By the DOSDoom Team
//
// Line Definitions Setup and Parser Code
//
// -KM- 1998/09/01 Written.
// -ACB- 1998/09/06 Beautification: cleaned up so I can read it :).
// -KM- 1998/10/29 New types of linedefs added: colourmap, sound, friction, gravity
//                  auto, singlesided, music, lumpcheck
//                  Removed sector movement to ddf_main.c, so can be accesed by
//                  ddf_sect.c
//
// TODO:  Original DOOM works fine, now add DOSDoom stuff.  Ideas:
//           Things stuff (spawn, ressurect)
//           Tips/Hints
//           Savegame
//           F_Finale stuff
//           Disable cheats :-)
//           CTF Stuff: Award Frags, Remove Keys.
//
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "p_local.h"
#include "z_zone.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

// -KM- 1999/01/29 Improved scrolling.
// Scrolling
typedef enum
{
  dir_none = 0,
  dir_vert = 1,
  dir_up   = 2,
  dir_horiz= 4,
  dir_left = 8
}
scrolldirs_e;

// Check for walk/push/shoot
static void DDF_LineGetTrigType(char *info, int c);

// Get player/monsters/missiles
static void DDF_LineGetActivators(char *info, int c);

// Get Red/Blue/Yellow
static void DDF_LineGetSecurity(char *info, int c);

// Check for scroll types
static void DDF_LineGetScroller(char *info, int c);

// My first attempt at a hash table!
linedeftype_t *specialLineDefs[256] = { [0 ... 255] = NULL};

static linedeftype_t bufferlinetype;
static fixed_t s_speed = FRACUNIT;
static scrolldirs_e s_dir = dir_none;
// -KM- 1998/09/27 Reversable linedefs.  Time functions.  More light functions
static const linedeftype_t defaultlinetype =
{
  0,                                      // Trigger num
  0,
  0,                                      // How triggered (walk/switch/shoot)
  0,                                      // OBJs that can trigger it
  KF_NONE,                               // No key needed
  NULL,
  -1,                                     // Can be activated repeatedly
  -1,                                     // Special type: don't change.
  false,                                  // Crushing

  {                                       // Floor:
    mov_undefined,                        //  Type
    false, false,
    -1, -1,               //  speed up/down
    ref_absolute,                         //  dest ref
    0,                                    //  dest
    "",                                   //  texture
    0, 0,                                 //  wait, prewait
    sfx_None, sfx_None, sfx_None, sfx_None                           // SFX start/up/down/stop
  },

  {                                       // Ceiling:
    mov_undefined,                        //  Type
    true, false,
    -1, -1,                 //  speed up/down
    ref_absolute,                         //  dest ref
    0,                                    //  dest
    "",                                   //  texture
    0, 0,                                 //  wait, prewait
    sfx_None, sfx_None, sfx_None, sfx_None                            //  SFX start/up/down/stop
  },

  {                                       // Donut
    false,                                //  dodonut
    sfx_None, sfx_None,                                 //  sfx down/stop inner circle
    sfx_None, sfx_None                                  //  sfx up/stop outer loop
  },

  {                                       // Teleport
    false,                                //  is a teleporter
    NULL,                                 //  effect object -> in
    NULL,                                 //  effect object -> off
    0,                                    // delay
  },

  {                                       // Lights:
   lite_none,                             //  lights action
   64                                     //  Set light to this level
  },

  0,                                      // Not an exit
  0, 0,                            // Not scrolling
  NULL,                                   // Security Message
  "\0",                                        // Colourmaplump
  -1,                                          // Colourmap
  0,                                           // Gravity
  0,                                           // Friction
  sfx_None,                                          // SFX
  "\0",                                              // Music
  false,                                              // Automatic line
  false,                                           // single sided
  false                                        // Hash table
};

static commandlist_t linedefcommands[] =
{{"TRIGGER"             , DDF_MainGetNumeric,         &bufferlinetype.trignum},
 {"NEWTRIGGER"          , DDF_MainGetNumeric,         &bufferlinetype.newtrignum},
 {"TYPE"                , DDF_LineGetTrigType,        &bufferlinetype.type},
 {"ACTIVATORS"          , DDF_LineGetActivators,      &bufferlinetype.obj},
 {"KEYS"                , DDF_LineGetSecurity,        &bufferlinetype.keys},
 {"FAILED MESSAGE"      , DDF_MainGetString,          &bufferlinetype.failedmessage},
 {"COUNT"               , DDF_MainGetNumeric,         &bufferlinetype.count},
 {"SECSPECIAL"          , DDF_MainGetNumeric,         &bufferlinetype.specialtype},
 {"CRUSH"               , DDF_MainGetBoolean,         &bufferlinetype.crush},
 {"FLOOR TYPE"          , DDF_MainGetMType,           &bufferlinetype.f.type},
 {"FLOOR SPEED UP"      , DDF_MainGetFixed,           &bufferlinetype.f.speed_up},
 {"FLOOR SPEED DOWN"    , DDF_MainGetFixed,           &bufferlinetype.f.speed_down},
 {"FLOOR DEST REF"      , DDF_MainGetDestRef,         &bufferlinetype.f.destref},
 {"FLOOR DEST OFFSET"   , DDF_MainGetFixed,           &bufferlinetype.f.dest},
 {"FLOOR TEXTURE"       , DDF_MainGetString,          &bufferlinetype.f.tex},
 {"FLOOR PAUSE TIME"    , DDF_MainGetTime,            &bufferlinetype.f.wait},
 {"FLOOR WAIT TIME"     , DDF_MainGetTime,            &bufferlinetype.f.prewait},
 {"FLOOR SFX START"     , DDF_MainLookupSound,        &bufferlinetype.f.sfxstart},
 {"FLOOR SFX UP"        , DDF_MainLookupSound,        &bufferlinetype.f.sfxup},
 {"FLOOR SFX DOWN"      , DDF_MainLookupSound,        &bufferlinetype.f.sfxdown},
 {"FLOOR SFX STOP"      , DDF_MainLookupSound,        &bufferlinetype.f.sfxstop},
 {"CEILING TYPE"        , DDF_MainGetMType,           &bufferlinetype.c.type},
 {"CEILING SPEED UP"    , DDF_MainGetFixed,           &bufferlinetype.c.speed_up},
 {"CEILING SPEED DOWN"  , DDF_MainGetFixed,           &bufferlinetype.c.speed_down},
 {"CEILING DEST REF"    , DDF_MainGetDestRef,         &bufferlinetype.c.destref},
 {"CEILING DEST OFFSET" , DDF_MainGetFixed,           &bufferlinetype.c.dest},
 {"CEILING TEXTURE"     , DDF_MainGetString,          &bufferlinetype.c.tex},
 {"CEILING PAUSE TIME"  , DDF_MainGetTime,            &bufferlinetype.c.wait},
 {"CEILING WAIT TIME"   , DDF_MainGetTime,            &bufferlinetype.c.prewait},
 {"CEILING SFX START"   , DDF_MainLookupSound,        &bufferlinetype.c.sfxstart},
 {"CEILING SFX UP"      , DDF_MainLookupSound,        &bufferlinetype.c.sfxup},
 {"CEILING SFX DOWN"    , DDF_MainLookupSound,        &bufferlinetype.c.sfxdown},
 {"CEILING SFX STOP"    , DDF_MainLookupSound,        &bufferlinetype.c.sfxstop},
 {"DONUT"               , DDF_MainGetBoolean,         &bufferlinetype.d.dodonut},
 {"DONUT IN SFX"        , DDF_MainLookupSound,        &bufferlinetype.d.d_sfxin},
 {"DONUT IN SFXSTOP"    , DDF_MainLookupSound,        &bufferlinetype.d.d_sfxinstop},
 {"DONUT OUT SFX"       , DDF_MainLookupSound,        &bufferlinetype.d.d_sfxout},
 {"DONUT OUT SFXSTOP"   , DDF_MainLookupSound,        &bufferlinetype.d.d_sfxoutstop},
 {"TELEPORT"            , DDF_MainGetBoolean,         &bufferlinetype.t.teleport},
 {"TELEPORT DELAY"      , DDF_MainGetTime,            &bufferlinetype.t.delay},
 {"TELEIN EFFECTOBJ"    , DDF_MainGetString,          &bufferlinetype.t.inspawnobj},
 {"TELEOUT EFFECTOBJ"   , DDF_MainGetString,          &bufferlinetype.t.outspawnobj},
 {"LIGHT TYPE"          , DDF_MainGetLighttype,       &bufferlinetype.l.type},
 {"LIGHT LEVEL"         , DDF_MainGetNumeric,         &bufferlinetype.l.light},
 {"LIGHT DARK TIME"     , DDF_MainGetTime,            &bufferlinetype.l.darktime},
 {"LIGHT BRIGHT TIME"   , DDF_MainGetTime,            &bufferlinetype.l.brighttime},
 {"LIGHT PROBABILITY"   , DDF_MainGetNumeric,         &bufferlinetype.l.light},
 {"LIGHT SYNC"          , DDF_MainGetTime,            &bufferlinetype.l.sync},
 {"EXIT"                , DDF_MainGetExit,            &bufferlinetype.e_exit},
 {"SCROLL"              , DDF_LineGetScroller,        &s_dir},
 {"SCROLLING SPEED"     , DDF_MainGetFixed,           &s_speed},
 {"SCROLL XSPEED"       , DDF_MainGetFixed,           &bufferlinetype.s_xspeed},
 {"SCROLL YSPEED"       , DDF_MainGetFixed,           &bufferlinetype.s_yspeed},
 {"COLOURMAPLUMP"        , DDF_MainGetLumpName,     &bufferlinetype.colourmaplump},
 {"COLOURMAP"            , DDF_MainGetNumeric,      &bufferlinetype.colourmap},
 {"GRAVITY"              , DDF_MainGetNumeric,      &bufferlinetype.gravity},
 {"FRICTION"             , DDF_MainGetFixed,      &bufferlinetype.friction},
 {"SOUND"                , DDF_MainLookupSound,     &bufferlinetype.sfx},
 {"MUSIC"                , DDF_MainGetMusicName,    &bufferlinetype.music},
 {"AUTO"                 , DDF_MainGetBoolean,      &bufferlinetype.autoline},
 {"SINGLESIDED"          , DDF_MainGetBoolean,      &bufferlinetype.singlesided},
 {"LUMPEXISTS"           , DDF_MainGetString,       &bufferlinetype.lumpcheck},
 {COMMAND_TERMINATOR    , NULL,                       NULL}};

static struct
{
  char *s;
  int  n;
}

s_scroll[] =
{
 {"NONE",   dir_none              | dir_none},
 {"UP",    (dir_vert  | dir_up  ) | ((~dir_none) << 16)},
 {"DOWN",  (dir_vert            ) | ((~dir_up  ) << 16)},
 {"LEFT",  (dir_horiz | dir_left) | ((~dir_none) << 16)},
 {"RIGHT", (dir_horiz           ) | ((~dir_left) << 16)}
},

s_keys[] =
{
 {"NONE",         KF_NONE},
 {"BLUE CARD",    KF_BLUECARD},
 {"YELLOW CARD",  KF_YELLOWCARD},
 {"RED CARD",     KF_REDCARD},
 {"BLUE SKULL",   KF_BLUESKULL},
 {"YELLOW SKULL", KF_YELLOWSKULL},
 {"RED SKULL",    KF_REDSKULL},
 {"REQUIRES ALL", KF_REQUIRESALL}
},

s_trigger[] =
{
 {"WALK",  line_walkable},
 {"PUSH",  line_pushable},
 {"SHOOT", line_shootable}
},

s_activators[] =
{
 {"PLAYER" , trig_player},
 {"MONSTER", trig_monster},
 {"MISSILE", trig_projectile}
};


//
// DDF_LineAdd2HashTable
//
// Adds a line definition to the hash table
//
// -KM-  1998/09/01 Procedure Written.
// -ACB- 1998/09/06 Remarked, Renamed & Reformatted.
//
static void DDF_LineAdd2HashTable(linedeftype_t* info)
{
  int slot = DDF_LineHashFunc(info->trignum);

  if (!specialLineDefs[slot])
  {
    specialLineDefs[slot] = malloc(sizeof(linedeftype_t));
    memcpy(specialLineDefs[slot], info, sizeof(linedeftype_t));
    specialLineDefs[slot][0].next = false;
  }
  else
  {
    int count;
    linedeftype_t* index = specialLineDefs[slot];

    for (count = 1; index->next; count++, index++);
    {
      specialLineDefs[slot]
        = realloc(specialLineDefs[slot], (count + 1) * sizeof(linedeftype_t));
    }

    memcpy(&specialLineDefs[slot][count], info, sizeof(linedeftype_t));
    specialLineDefs[slot][count - 1].next = true;
    specialLineDefs[slot][count].next = false;
  }
}



//
// DDF_LineGetScroller
//
// Check for scroll types
//
void DDF_LineGetScroller(char *info, int c)
{
  int i;
  for (i = sizeof(s_scroll) / sizeof(s_scroll[0]); i--;)
  {
    if (!strcasecmp(info, s_scroll[i].s))
    {
      s_dir &= s_scroll[i].n >> 16;
      s_dir |= s_scroll[i].n & 0xffff;
      return;
    }
  }
  I_Error("\nUnknown scroll direction %s\n", info);
}

//
// DDF_LineGetSecurity
//
// Get Red/Blue/Yellow
//
void DDF_LineGetSecurity(char *info, int c)
{
  int i;
  boolean required = false;

  if (info[0] == '+')
  {
    required = true;
    info++;
  }

  for (i = sizeof(s_keys) / sizeof(s_keys[0]); i--;)
  {
    if (!strcasecmp(info, s_keys[i].s))
    {
      bufferlinetype.keys |= s_keys[i].n;
      if (required)
        bufferlinetype.keys |= s_keys[i].n << 8;
      return;
    }
  }

  I_Error("\nUnknown key type %s\n", info);
}

//
// DDF_LineGetTrigType
//
// Check for walk/push/shoot
//
void DDF_LineGetTrigType(char *info, int c)
{
  int i;

  for (i = sizeof(s_trigger) / sizeof(s_trigger[0]); i--;)
  {
    if (!strcasecmp(info, s_trigger[i].s))
    {
      bufferlinetype.type = s_trigger[i].n;
      return;
    }
  }

  I_Error("\nUnknown Trigger type %s\n", info);
}

//
// DDF_LineGetActivators
//
// Get player/monsters/missiles
//
void DDF_LineGetActivators(char *info, int c)
{
  int i;
  for (i = sizeof(s_activators) / sizeof(s_activators[0]); i--;)
  {

    if (!strcasecmp(info, s_activators[i].s))
    {
      bufferlinetype.obj |= s_activators[i].n;
      return;
    }
  }

  I_Error("\nUnknown Activator type %s\n", info);
}

//
// DDF_LinedefCheckNum
//
// Checks that the linedef number is not already declared.
//
// -KM-  1998/09/01 Wrote Procedure.
// -ACB- 1998/09/06 Remarked and reformatted.
//
void DDF_LinedefCheckNum(char *info)
{
  int i = 0, num, slot;

#ifdef DEVELOPERS
 if (info == NULL)
   I_Error("\nInfo has no info\n");
#endif

  num = atoi(info);
  slot = DDF_LineHashFunc(num);

  if (specialLineDefs[slot])
  {
    do
    {
      if (specialLineDefs[slot][i].trignum == num)
        I_Error("\nDDF_LinedefCheckNum: %d already declared\n",num);
    }
    while (specialLineDefs[slot][i++].next);
  }

  memcpy(&bufferlinetype, &defaultlinetype, sizeof(bufferlinetype));
  bufferlinetype.trignum = num;

  return;
}

void DDF_LinedefCreate(void)
{
  bufferlinetype.c.crush = bufferlinetype.f.crush = bufferlinetype.crush;
  // -KM- 1999/01/29 Convert old style scroller to new.
  if (s_dir & dir_vert)
  {
    if (s_dir & dir_up)
      bufferlinetype.s_yspeed = s_speed;
    else
      bufferlinetype.s_yspeed = -s_speed;
  }
  if (s_dir & dir_horiz)
  {
    if (s_dir & dir_left)
      bufferlinetype.s_xspeed = s_speed;
    else
      bufferlinetype.s_xspeed = -s_speed;
  }
  DDF_LineAdd2HashTable(&bufferlinetype);
  s_dir = dir_none;
  s_speed = FRACUNIT;
}

void DDF_ReadLines(void *data, int size)
{
  readinfo_t lines;

  if (!data)
  {
    lines.message               = "DDF_InitSpecialLines";
    lines.filename              = "lines.ddf";
    lines.memfile = NULL;
  } else {
    lines.message = NULL;
    lines.memfile = data;
    lines.memsize = size;
    lines.filename = NULL;
  }

  lines.DDF_MainCheckName     = DDF_LinedefCheckNum;
  lines.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  lines.DDF_MainCreateEntry   = DDF_LinedefCreate;
  lines.DDF_MainFinishingCode = DDF_LinedefCreate;
  lines.cmdlist               = linedefcommands;
    
  DDF_MainReadFile(&lines);
}
//
// DDF_LinedefInit
//
void DDF_LinedefInit()
{
  DDF_ReadLines(NULL, 0);
}

//
// DDF_GetSecHeightReference
//
// Finds a sector height, using the reference provided; will select
// the approriate method of obtaining this value, if it cannot
// get it directly.
//
// -KM-  1998/09/01 Wrote Procedure.
// -ACB- 1998/09/06 Remarked and Reformatted.
//
fixed_t DDF_GetSecHeightReference(heightref_e ref, sector_t *sec)
{
  switch (ref)
  {
    case ref_absolute:
      return 0;

    case ref_floorHeight:
      return sec->floorheight;

    case ref_ceilingHeight:
      return sec->ceilingheight;

    case ref_LowestSurroundingCeiling:
      return P_FindLowestCeilingSurrounding(sec);

    case ref_HighestSurroundingCeiling:
      return P_FindHighestCeilingSurrounding(sec);

    case ref_LowestSurroundingFloor:
      return P_FindLowestFloorSurrounding(sec);

    case ref_HighestSurroundingFloor:
      return P_FindHighestFloorSurrounding(sec);

    case ref_NextHighestFloor:
      return P_FindNextHighestFloor(sec, sec->floorheight);

    case ref_LowestLoTexture:
      return P_FindRaiseToTexture(sec);

    default:
      I_Error("DDF_GetSecHeightReference: undefined reference %d\n", ref);
  }
}

//
// DDF_GetFromLineHashTable
//
// Returns the special linedef properties from given specialtype
//
// -KM-  1998/09/01 Wrote Procedure
// -ACB- 1998/09/06 Remarked and Reformatted....
//
linedeftype_t* DDF_GetFromLineHashTable(int trignum)
{
  int slot = DDF_LineHashFunc(trignum);
  int i = 0;

  // Find in hash table
  do
  {
    if (specialLineDefs[slot][i].trignum == trignum)
      return &specialLineDefs[slot][i];
  }
  while(specialLineDefs[slot][i++].next);

  I_Error("Unknown Special LineDef type %d\n", trignum);
}


