//
// DOSDoom Specials Lines & Floor Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/09/01 Lines.ddf
//
// -ACB- 1998/09/13 Cleaned Up.
//


#ifndef __P_SPEC__
#define __P_SPEC__

#include "ddf_main.h"

#define MAXSWITCHES 50
#define MAXBUTTONS 32
#define BUTTONTIME 35


#define GLOWSPEED 8
#define CEILSPEED FRACUNIT
#define FLOORSPEED FRACUNIT

#define STOPSPEED		0x1000
#define FRICTION		0xe800


typedef struct
{
  thinker_t thinker;
  sector_t* sector;

  litetype_e type;

  int count;

  int minlight;
  int maxlight;
  int direction;
  int darktime;
  int brighttime;
  int probability;
}
light_t;
// -ACB- 1998/09/13 Remove episode value (pointless under DDF)
typedef struct
{
  sfx_t* sfx;
  char name1[9];
  char name2[9];
}
switchlist_t;

typedef enum
{
  top,
  middle,
  bottom
}
bwhere_e;

typedef struct
{
  line_t* line;
  bwhere_e where;
  int btexture;
  int btimer;
  mobj_t* soundorg;
}
button_t;

// -KM- 1998/09/01 lines.ddf
typedef struct
{
  thinker_t thinker;
  movinPlane_t* type;
  sector_t* sector;

  // Ceiling is true
  boolean floorOrCeiling;

  fixed_t startheight;
  fixed_t destheight;
  fixed_t speed;
  boolean crush;

  // 1 = up, 0 = waiting at top, -1 = down
  int direction;
  int olddirection;

  int tag;

  // tics to wait at the top
  int waited;

  // (keep in case a door going down is reset)
  // when it reaches 0, start going down
  int wait;
  boolean sfxstarted;
  boolean completed;

  int newspecial;
  int texture;
}
secMove_t;

typedef enum
{
  ok,
  crushed,
  pastdest
}
result_e;

// End-level timer (-TIMER option)
extern boolean levelTimer;
extern int levelTimeCount;

extern int maxbuttons;
extern button_t* buttonlist;
extern int maxsecs;
extern secMove_t** activesecs;
void P_ResetActiveSecs(void);

// at game start
void P_InitPicAnims(void);
void P_InitSwitchList(void);

// at map load
void P_SpawnSpecials (int autotag);

// every tic
void P_UpdateSpecials (void);

// when needed
boolean P_UseSpecialLine (mobj_t* thing, line_t* line, int side);
boolean P_CrossSpecialLine (int linenum, int side, mobj_t* thing);
void P_ShootSpecialLine (mobj_t* thing, line_t* line);
void P_PlayerInSpecialSector (player_t* player);

// Utilities...
int twoSided (int sector, int line);
side_t* getSide (int currentSector, int line, int side);
sector_t* getSector (int currentSector, int line, int side);
sector_t* getNextSector (line_t* line, sector_t* sec);

// Info Needs....
fixed_t P_FindLowestFloorSurrounding(sector_t* sec);
fixed_t P_FindHighestFloorSurrounding(sector_t* sec);
fixed_t P_FindNextHighestFloor (sector_t* sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t* sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec);
fixed_t P_FindRaiseToTexture(sector_t*  sec); // -KM- 1998/09/01 New func, old inline
int P_FindSectorFromTag (int tag, int start);
int P_FindMinSurroundingLight (sector_t* sector, int max);

// start an action...
boolean EV_Lights(sector_t* sec, lighttype_t* type, void *null);

void P_AddActiveSector(secMove_t* sec);
void P_ChangeSwitchTexture (line_t* line, int useAgain);

void T_MoveSector( secMove_t* sec);
void T_Light(light_t* light);
result_e T_MovePlane (sector_t* sector, fixed_t speed, fixed_t dest,
                       boolean crush, int floorOrCeiling, int direction);

void EV_LightTurnOn (line_t* line, int bright);
boolean EV_DoDonut (sector_t* s1, sfx_t* sfx[4]);
boolean EV_Teleport (line_t* line, int side, mobj_t* thing, int delay,
                       mobjinfo_t* ineffectobj, mobjinfo_t* outeffectobj);
boolean EV_Manual ( line_t* line, mobj_t* thing, movinPlane_t* type);
boolean EV_DoSector (sector_t* sec, movinPlane_t* type, sector_t* model);

extern mobjinfo_t* bulletpuff;

#endif

