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
/*#define STROBEBRIGHT 5
#define FASTDARK 15
#define SLOWDARK 35

#define VDOORSPEED FRACUNIT*2
#define VDOORWAIT 150
#define CEILWAIT 150
*/
#define CEILSPEED FRACUNIT
#define FLOORSPEED FRACUNIT

#define STOPSPEED		0x1000
#define FRICTION		0xe800
/*
typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int count;
  int maxlight;
  int minlight;
}
fireflicker_t;

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int count;
  int maxlight;
  int minlight;
  int maxtime;
  int mintime;
}
lightflash_t;

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int count;
  int minlight;
  int maxlight;
  int darktime;
  int brighttime;
}
strobe_t;

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int minlight;
  int maxlight;
  int direction;
}
glow_t;
*/


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


/*
typedef struct
{
  thinker_t thinker;
  linedeftype_t* type;
  sector_t* sector;
  fixed_t startheight;
  fixed_t destheight;
  fixed_t speed;
  boolean crush;

  // 1 = up, 0 = waiting at top, -1 = down
  int direction;
  int olddirection;

  int tag;

  // tics to wait at the top
  int topwait;

  // (keep in case a door going down is reset)
  // when it reaches 0, start going down
  int topcountdown;
  boolean sfxstarted;
  boolean completed;
}
ceiling_t;

typedef struct
{
  thinker_t thinker;
  linedeftype_t* type;
  sector_t* sector;
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
floormove_t;
*/
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
//void P_SpawnFireFlicker (sector_t* sector);
//void P_SpawnLightFlash (sector_t* sector);
//void P_SpawnStrobeFlash (sector_t* sector, int fastOrSlow, int inSync);
//void P_SpawnGlowingLight(sector_t* sector);
boolean EV_Lights(sector_t* sec, lighttype_t* type, void *null);
//void P_SpawnDoorCloseIn30 (sector_t* sec);
//void P_SpawnDoorRaiseIn5Mins (sector_t* sec, int secnum);

//void P_AddActiveCeiling(ceiling_t* c);
//void P_RemoveActiveCeiling(ceiling_t* c);
//void P_AddActiveFloor(floormove_t* floor);
//void P_RemoveActiveFloor(floormove_t* floor);
//boolean P_StasifyFloor(line_t* line);
//boolean	P_StasifyCeiling(line_t* line);
//boolean P_ActivateInStasis(int tag);
///boolean P_ActivateInStasisCeiling(line_t* line);
void P_AddActiveSector(secMove_t* sec);
void P_ChangeSwitchTexture (line_t* line, int useAgain);

void T_MoveSector( secMove_t* sec);
//void T_LightFlash (lightflash_t* flash);
//void T_StrobeFlash (strobe_t* flash);
//void T_Glow(glow_t* g);
void T_Light(light_t* light);
//void T_MoveCeiling (ceiling_t* ceiling);
result_e T_MovePlane (sector_t* sector, fixed_t speed, fixed_t dest,
                       boolean crush, int floorOrCeiling, int direction);

//void EV_StartLightStrobing(line_t* line);
//void EV_TurnTagLightsOff(line_t* line);
void EV_LightTurnOn (line_t* line, int bright);
boolean EV_DoDonut (sector_t* s1, sfx_t* sfx[4]);
//boolean EV_Ceiling (line_t* line, mobj_t* thing, linedeftype_t* type);
//boolean EV_DoCeiling (line_t* line, linedeftype_t* type);
//boolean EV_DoFloor (line_t* line, linedeftype_t* type);
boolean EV_Teleport (line_t* line, int side, mobj_t* thing, int delay,
                       mobjinfo_t* ineffectobj, mobjinfo_t* outeffectobj);
boolean EV_Manual ( line_t* line, mobj_t* thing, movinPlane_t* type);
boolean EV_DoSector (sector_t* sec, movinPlane_t* type, sector_t* model);

/*
typedef enum
{
    normal,
    close30ThenOpen,
    close,
    open,
    raiseIn5Mins,
    blazeRaise,
    blazeOpen,
    blazeClose

} vldoor_e;

int
EV_DoDoor
( line_t*	line,
  vldoor_e	type );

int
EV_DoLockedDoor
( line_t*	line,
  vldoor_e	type,
  mobj_t*	thing );

void    T_VerticalDoor (vldoor_t* door);

typedef enum
{
    sd_opening,
    sd_waiting,
    sd_closing

} sd_e;

typedef enum
{
    sdt_openOnly,
    sdt_closeOnly,
    sdt_openAndClose

} sdt_e;

typedef struct
{
    thinker_t	thinker;
    sdt_e	type;
    line_t*	line;
    int		frame;
    int		whichDoorIndex;
    int		timer;
    sector_t*	frontsector;
    sector_t*	backsector;
    sd_e	 status;

} slidedoor_t;

typedef struct
{
    char	frontFrame1[9];
    char	frontFrame2[9];
    char	frontFrame3[9];
    char	frontFrame4[9];
    char	backFrame1[9];
    char	backFrame2[9];
    char	backFrame3[9];
    char	backFrame4[9];
    
} slidename_t;

typedef struct
{
    int             frontFrames[4];
    int             backFrames[4];

} slideframe_t;

// how many frames of animation
#define SNUMFRAMES		4

#define SDOORWAIT		35*3
#define SWAITTICS		4

// how many diff. types of anims
#define MAXSLIDEDOORS	5                            

void P_InitSlidingDoorFrames(void);

void
EV_SlidingDoor
( line_t*	line,
  mobj_t*	thing );

typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise

} ceiling_e;

typedef struct
{
    thinker_t	thinker;
    ceiling_e	type;
    sector_t*	sector;
    fixed_t	bottomheight;
    fixed_t	topheight;
    fixed_t	speed;
    boolean	crush;

    // 1 = up, 0 = waiting, -1 = down
    int		direction;

    // ID
    int		tag;                   
    int		olddirection;
    
} ceiling_t;

typedef enum
{
    // lower floor to highest surrounding floor
    lowerFloor,
    
    // lower floor to lowest surrounding floor
    lowerFloorToLowest,
    
    // lower floor to highest surrounding floor VERY FAST
    turboLower,
    
    // raise floor to lowest surrounding CEILING
    raiseFloor,
    
    // raise floor to next highest surrounding floor
    raiseFloorToNearest,

    // raise floor to shortest height texture around it
    raiseToTexture,
    
    // lower floor to lowest surrounding floor
    //  and change floorpic
    lowerAndChange,
  
    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,

     // raise to next highest floor, turbo-speed
    raiseFloorTurbo,       
    donutRaise,
    raiseFloor512
    
} floor_e;

typedef enum
{
    build8,	// slowly build by 8
    turbo16	// quickly build by 16
    
} stair_e;

int
EV_BuildStairs
( line_t*	line,
  stair_e	type );

int
EV_DoFloor
( line_t*	line,
  floor_e	floortype );

typedef enum
{
    up,
    down,
    waiting,
    in_stasis

} plat_e;

typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS

} plattype_e;

typedef struct
{
    thinker_t	thinker;
    sector_t*	sector;
    fixed_t	speed;
    fixed_t	low;
    fixed_t	high;
    int		wait;
    int		count;
    plat_e	status;
    plat_e	oldstatus;
    boolean	crush;
    int		tag;
    plattype_e	type;
    
} plat_t;



#define PLATWAIT		3
#define PLATSPEED		FRACUNIT
void    T_PlatRaise(plat_t*	plat);
*/
#endif

