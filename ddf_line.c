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
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

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
  0, FRACUNIT,                            // Not scrolling
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
 {"SCROLL"              , DDF_LineGetScroller,        &bufferlinetype.scroller},
 {"SCROLLING SPEED"     , DDF_MainGetFixed,           &bufferlinetype.s_speed},
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
      bufferlinetype.scroller &= s_scroll[i].n >> 16;
      bufferlinetype.scroller |= s_scroll[i].n & 0xffff;
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
  DDF_LineAdd2HashTable(&bufferlinetype);
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

/*
 Linedef Types to define:
 2: Open Door W1
	EV_DoDoor(line,open);
 3: Close Door W1
	EV_DoDoor(line,close);
 4: Raise Door W1
	EV_DoDoor(line,normal);
 5: Raise Floor W1
	EV_DoFloor(line,raiseFloor);
 6: Fast Ceiling Crush & Raise W1
	EV_DoCeiling(line,fastCrushAndRaise);
 8: Build Stairs W1
	EV_BuildStairs(line,build8);
 10: PlatDownWaitUp W1
	EV_DoPlat(line,downWaitUpStay,0);
 12: Light Turn On - brightest near W1
	EV_LightTurnOn(line,0);
 13: Light Turn On 255 W1
	EV_LightTurnOn(line,255);
 16: Close Door 30 W1
	EV_DoDoor(line,close30ThenOpen);
 17: Start Light Strobing W1
	EV_StartLightStrobing(line);
 19: Lower Floor W1
	EV_DoFloor(line,lowerFloor);
 22: Raise floor to nearest height and change texture W1
	EV_DoPlat(line,raiseToNearestAndChange,0);
 25: Ceiling Crush and Raise W1
	EV_DoCeiling(line,crushAndRaise);
 30: Raise floor to shortest texture height on either side of lines W1
     Finds the shortest bottom texture around the target sector, and raises to it.
        EV_DoFloor(line, raiseToTexture);
 35: Lights Very Dark W1
	EV_LightTurnOn(line,35);
 36: Lower Floor (TURBO) W1
	EV_DoFloor(line,turboLower);
 37: LowerAndChange W1
	EV_DoFloor(line,lowerAndChange);
 38: Lower Floor To Lowest W1
	EV_DoFloor( line, lowerFloorToLowest );
 39: TELEPORT! W1
	EV_Teleport( line, side, thing );
 40: RaiseCeilingLowerFloor W1
	EV_DoCeiling( line, raiseToHighest );
	EV_DoFloor( line, lowerFloorToLowest );
 44: Ceiling Crush W1
	EV_DoCeiling( line, lowerAndCrush );
 52: EXIT! W1
	G_ExitLevel ();
 53: Perpetual Platform Raise W1
	EV_DoPlat(line,perpetualRaise,0);
 54: Platform Stop W1
	EV_StopPlat(line);
 56: Raise Floor Crush W1
	EV_DoFloor(line,raiseFloorCrush);
 57: Ceiling Crush Stop W1
	EV_CeilingCrushStop(line);
 58: Raise Floor 24 W1
	EV_DoFloor(line,raiseFloor24);
 59: Raise Floor 24 And Change W1
	EV_DoFloor(line,raiseFloor24AndChange);
 104: Turn lights off in sector(tag) W1
	EV_TurnTagLightsOff(line);
 108: Blazing Door Raise (faster than TURBO!) W1
	EV_DoDoor (line,blazeRaise);
 109: Blazing Door Open (faster than TURBO!) W1
	EV_DoDoor (line,blazeOpen);
 100: Build Stairs Turbo 16 W1
	EV_BuildStairs(line,turbo16);
 110: Blazing Door Close (faster than TURBO!) W1
	EV_DoDoor (line,blazeClose);
 119: Raise floor to nearest surr. floor W1
	EV_DoFloor(line,raiseFloorToNearest);
 121: Blazing PlatDownWaitUpStay W1
	EV_DoPlat(line,blazeDWUS,0);
 124: Secret EXIT W1
	G_SecretExitLevel ();
 125: TELEPORT MonsterONLY W1
	if (!thing->player)
	    EV_Teleport( line, side, thing );
 130: Raise Floor Turbo W1
	EV_DoFloor(line,raiseFloorTurbo);
 141: Silent Ceiling Crush & Raise W1
	EV_DoCeiling(line,silentCrushAndRaise);

	// RETRIGGERS.  All from here till end.
 72: Ceiling Crush WR
	EV_DoCeiling( line, lowerAndCrush );
 73: Ceiling Crush and Raise WR
	EV_DoCeiling(line,crushAndRaise);
 74: Ceiling Crush Stop WR
	EV_CeilingCrushStop(line);
 75: Close Door WR
	EV_DoDoor(line,close);
 76: Close Door 30 WR
	EV_DoDoor(line,close30ThenOpen);
 77: Fast Ceiling Crush & Raise WR
	EV_DoCeiling(line,fastCrushAndRaise);
 79: Lights Very Dark WR
	EV_LightTurnOn(line,35);
 80: Light Turn On - brightest near WR
	EV_LightTurnOn(line,0);
 81: Light Turn On 255 WR
	EV_LightTurnOn(line,255);
 82: Lower Floor To Lowest WR
	EV_DoFloor( line, lowerFloorToLowest );
 83: Lower Floor WR
	EV_DoFloor(line,lowerFloor);
 84: LowerAndChange WR
	EV_DoFloor(line,lowerAndChange);
 86: Open Door WR
	EV_DoDoor(line,open);
 87: Perpetual Platform Raise WR
	EV_DoPlat(line,perpetualRaise,0);
 88: PlatDownWaitUp WR
	EV_DoPlat(line,downWaitUpStay,0);
 89: Platform Stop WR
	EV_StopPlat(line);
 90: Raise Door WR
	EV_DoDoor(line,normal);
 91: Raise Floor WR
	EV_DoFloor(line,raiseFloor);
 92: Raise Floor 24 WR
	EV_DoFloor(line,raiseFloor24);
 93: Raise Floor 24 And Change WR
	EV_DoFloor(line,raiseFloor24AndChange);
 94: Raise Floor Crush WR
	EV_DoFloor(line,raiseFloorCrush);
 95: Raise floor to nearest height and change texture. WR
	EV_DoPlat(line,raiseToNearestAndChange,0);
 96: Raise floor to shortest texture height on either side of lines.
	EV_DoFloor(line,raiseToTexture);
 97: TELEPORT! WR
	EV_Teleport( line, side, thing );
 98: Lower Floor (TURBO) WR
	EV_DoFloor(line,turboLower);
 105: Blazing Door Raise (faster than TURBO!) WR
	EV_DoDoor (line,blazeRaise);
 106: Blazing Door Open (faster than TURBO!) WR
	EV_DoDoor (line,blazeOpen);
 107: Blazing Door Close (faster than TURBO!) WR
	EV_DoDoor (line,blazeClose);
 120: Blazing PlatDownWaitUpStay. WR
	EV_DoPlat(line,blazeDWUS,0);
 126: TELEPORT MonsterONLY.
	if (!thing->player)
	    EV_Teleport( line, side, thing );
 128: Raise To Nearest Floor WR
	EV_DoFloor(line,raiseFloorToNearest);
 129: Raise Floor Turbo WR
	EV_DoFloor(line,raiseFloorTurbo);
 24: RAISE FLOOR GR
	EV_DoFloor(line,raiseFloor);
	P_ChangeSwitchTexture(line,0);
 46: OPEN DOOR GR
	EV_DoDoor(line,open);
	P_ChangeSwitchTexture(line,1);
 47: RAISE FLOOR NEAR AND CHANGE GR
	EV_DoPlat(line,raiseToNearestAndChange,0);
	P_ChangeSwitchTexture(line,0);
 1: Vertical Door SR
 26: Blue Door/Locked SR
 27: Yellow Door /Locked SR
 28: Red Door /Locked SR
 31: Manual door open SR
 32: Blue locked door open SR
 33: Red locked door open SR
 34: Yellow locked door open SR
 117: Blazing door raise SR
 118: Blazing door open SR
	EV_VerticalDoor (line, thing);
 7: Build Stairs S1
	if (EV_BuildStairs(line,build8))
	    P_ChangeSwitchTexture(line,0);
 9: Change Donut S1
	if (EV_DoDonut(line))
	    P_ChangeSwitchTexture(line,0);
 11: Exit level S1
	P_ChangeSwitchTexture(line,0);
	G_ExitLevel ();
 14: Raise Floor 32 and change texture S1
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,0);
 15: Raise Floor 24 and change texture S1
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,0);
 18: Raise Floor to next highest floor S1
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,0);
 20: Raise Plat next highest floor and change texture S1
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,0);
 21: PlatDownWaitUpStay S1
	if (EV_DoPlat(line,downWaitUpStay,0))
	    P_ChangeSwitchTexture(line,0);
 23: Lower Floor to Lowest S1
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,0);
 29: Raise Door S1
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,0);
 41: Lower Ceiling to Floor S1
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,0);
 49: Ceiling Crush And Raise S1
	if (EV_DoCeiling(line,crushAndRaise))
	    P_ChangeSwitchTexture(line,0);
 50: Close Door S1
	if (EV_DoDoor(line,close))
	    P_ChangeSwitchTexture(line,0);
 51: Secret EXIT S1
	P_ChangeSwitchTexture(line,0);
	G_SecretExitLevel ();
 55: Raise Floor Crush S1
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,0);
 71: Turbo Lower Floor S1
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,0);
 101: Raise Floor S1
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,0);
 102: Lower Floor to Surrounding floor height S1
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,0);
 103: Open Door S1
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,0);
 111: Blazing Door Raise (faster than TURBO!) S1
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,0);
 112: Blazing Door Open (faster than TURBO!) S1
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,0);
 113: Blazing Door Close (faster than TURBO!) S1
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,0);
 122: Blazing PlatDownWaitUpStay S1
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,0);
 127: Build Stairs Turbo 16 S1
	if (EV_BuildStairs(line,turbo16))
	    P_ChangeSwitchTexture(line,0);
 131: Raise Floor Turbo S1
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,0);
 133: BlzOpenDoor BLUE S1
 135: BlzOpenDoor RED  S1
 137: BlzOpenDoor YELLOW S1
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,0);
 140: Raise Floor 512 S1
	if (EV_DoFloor(line,raiseFloor512))
	    P_ChangeSwitchTexture(line,0);
 42: Close Door SR
	if (EV_DoDoor(line,close))
	    P_ChangeSwitchTexture(line,1);
 43: Lower Ceiling to Floor SR
	if (EV_DoCeiling(line,lowerToFloor))
	    P_ChangeSwitchTexture(line,1);
 45: Lower Floor to Surrounding floor height SR
	if (EV_DoFloor(line,lowerFloor))
	    P_ChangeSwitchTexture(line,1);
 60: Lower Floor to Lowest SR
	if (EV_DoFloor(line,lowerFloorToLowest))
	    P_ChangeSwitchTexture(line,1);
 61: Open Door SR
	if (EV_DoDoor(line,open))
	    P_ChangeSwitchTexture(line,1);
 62: PlatDownWaitUpStay SR
	if (EV_DoPlat(line,downWaitUpStay,1))
	    P_ChangeSwitchTexture(line,1);
 63: Raise Door SR
	if (EV_DoDoor(line,normal))
	    P_ChangeSwitchTexture(line,1);
 64: Raise Floor to ceiling SR
	if (EV_DoFloor(line,raiseFloor))
	    P_ChangeSwitchTexture(line,1);
 66: Raise Floor 24 and change texture SR
	if (EV_DoPlat(line,raiseAndChange,24))
	    P_ChangeSwitchTexture(line,1);
 67: Raise Floor 32 and change texture SR
	if (EV_DoPlat(line,raiseAndChange,32))
	    P_ChangeSwitchTexture(line,1);
 65: Raise Floor Crush SR
	if (EV_DoFloor(line,raiseFloorCrush))
	    P_ChangeSwitchTexture(line,1);
 68: Raise Plat to next highest floor and change texture SR
	if (EV_DoPlat(line,raiseToNearestAndChange,0))
	    P_ChangeSwitchTexture(line,1);
 69: Raise Floor to next highest floor SR
	if (EV_DoFloor(line, raiseFloorToNearest))
	    P_ChangeSwitchTexture(line,1);
 70: Turbo Lower Floor SR
	if (EV_DoFloor(line,turboLower))
	    P_ChangeSwitchTexture(line,1);
 99: BlzOpenDoor BLUE
	if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,1);
 114: Blazing Door Raise (faster than TURBO!) SR
	if (EV_DoDoor (line,blazeRaise))
	    P_ChangeSwitchTexture(line,1);
 115: Blazing Door Open (faster than TURBO!) SR
	if (EV_DoDoor (line,blazeOpen))
	    P_ChangeSwitchTexture(line,1);
 116: Blazing Door Close (faster than TURBO!) SR
	if (EV_DoDoor (line,blazeClose))
	    P_ChangeSwitchTexture(line,1);
 123: Blazing PlatDownWaitUpStay SR
	if (EV_DoPlat(line,blazeDWUS,0))
	    P_ChangeSwitchTexture(line,1);
 132: Raise Floor Turbo SR
	if (EV_DoFloor(line,raiseFloorTurbo))
	    P_ChangeSwitchTexture(line,1);
 134: BlzOpenDoor RED
 136: BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,blazeOpen,thing))
	    P_ChangeSwitchTexture(line,1);
 138: Light Turn On
	EV_LightTurnOn(line,255);
	P_ChangeSwitchTexture(line,1);
 139: Light Turn Off SR
	EV_LightTurnOn(line,35);
	P_ChangeSwitchTexture(line,1);
 48: SCROLL + (Left)
 9000: SCROLL - (Right)
 9001: SCROLL + (Up)
 9002: SCROLL - (Down)
 9003: SCROLL - (Left & Up)
 9004: SCROLL - (Left & Down)
 9005: SCROLL - (Right & Up)
 9006: SCROLL - (Right & Down)

 Action pointers:
 EV_DoFloor(line_t* trig, floor_e floortype); // Add speed, amount

Need to implement:
  type:  Shoot, Walk, Use: How this line is triggered
  count: how many times this line can be triggered: 1 = once, -1 = any amount, 2... = twice
  type2: Monsters/Players/Projectiles can use this line.
  keys: red/yellow/blue key required to activate.
  Special: Special sector type to change to.
  SFX: Sound to trigger (overridden by other sfx's)

  Floor Specific
     Speed: How fast it moves
     Destination: Height to move to
     Texture: Texture to change to
     SFX: Sound to trigger
     Floor Sub Catagory: Plats
       Plat pause time: Time plat stays at destination before returning

  Ceiling Specific
     Speed: How fast it moves
     Destination: Height to move to
     SFX: Sound to trigger
     Ceiling Sub Catagory: Doors
       Door pause time: Time door stays open before closing.

  Light Specific:
     Type: Set, flash (random), strobe (non-random), flicker
     Destination: Light level to change to.
     SFX: Sound to trigger

  Donut Specific:
     DoDonut

  Exit Level Specific:
     Secret Level exit: true/false.

*/
