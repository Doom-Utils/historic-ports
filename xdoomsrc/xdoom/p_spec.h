// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-2000 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:  none
//	Implements special effects:
//	Texture animation, height or lighting changes
//	 according to adjacent sectors, respective
//	 utility functions, etc.
//
//-----------------------------------------------------------------------------

#ifndef __P_SPEC__
#define __P_SPEC__

// Base of the generalized linedef types
#define GenFloorBase		0x6000
#define GenCeilingBase		0x4000
#define GenDoorBase		0x3c00
#define GenLockedBase		0x3800
#define GenLiftBase		0x3400
#define GenStairsBase		0x3000
#define GenCrusherBase		0x2f80
#define GenLinedefBase		GenCrusherBase

#define TriggerType		0x0007
#define TriggerTypeShift	0

// bits and shift for generalized sector types
#define DAMAGE_MASK		0x60
#define DAMAGE_SHIFT		5
#define SECRET_MASK		0x80
#define SECRET_SHIT		7
#define FRICTION_MASK		0x100
#define FRICTION_SHIFT		8
#define PUSH_MASK		0x200
#define PUSH_SHIFT		9

// pure texture/type change for better generalized support
typedef enum
{
    trigChangeOnly,
    numChangeOnly
} change_e;

// names for the trigger type field of generalized linedefs
typedef enum
{
    WalkOnce,
    WalkMany,
    SwitchOnce,
    SwitchMany,
    GunOnce,
    GunMany,
    PushOnce,
    PushMany
} triggertype_e;

// names for the speed field of generalized linedefs
typedef enum
{
    SpeedSlow,
    SpeedNormal,
    SpeedFast,
    SpeedTurbo
} motionspeed_e;

// identify the special classes that can share sectors
typedef enum
{
    floor_special,
    ceiling_special,
    lighting_special
} special_e;

//
// End-level timer (-TIMER option)
//
extern	boolean levelTimer;
extern	int	levelTimeCount;

// Define values for map objects
#define MO_TELEPORTMAN		14

// at game start
void P_InitPicAnims(void);

// at map load
void P_SpawnSpecials(void);
void P_InitTagLists(void);

// every tic
void P_UpdateSpecials(void);

// the trigger functions
boolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side);
boolean P_UseSpecialLinePlus(mobj_t *thing, line_t *line, int side);
void	P_ShootSpecialLine(mobj_t *thing, line_t *line);
void	P_CrossSpecialLine(int linenum, int side, mobj_t *thing);
void    P_PlayerInSpecialSector(player_t *player);

// utility functions
int	twoSided(int sector, int line);
sector_t *getSector(int	currentSector, int line, int side);
sector_t *getNextSector(line_t *line, sector_t *sec);
side_t	*getSide(int currentSector, int line, int side);
int	P_FindSectorFromLineTag(line_t *line, int start);
int	P_FindSectorFromTag(int tag, int start);
int	P_FindLineFromLineTag(line_t *line, int start);
int	P_FindLineFromTag(int tag, int start);
int	P_SectorActive(special_e t, sector_t *sec);

// floor functions
fixed_t	P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);
fixed_t	P_FindNextHighestFloor(sector_t *sec, int currentheight);
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight);
sector_t *P_FindModelFloorSector(fixed_t floordestheight, int secnum);

// ceiling functions
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);
fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight);
fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight);
sector_t *P_FindModelCeilingSector(fixed_t ceildestheight, int secnum);

// lighting functions
int	P_FindMinSurroundingLight(sector_t *sector, int max);

// texture functions
fixed_t P_FindShortestUpperAround(int secnum);
fixed_t P_FindShortestTextureAround(int secnum);

//
// SPECIAL
//
int EV_DoDonut(line_t *line);

//
// P_LIGHTS
//
typedef struct
{
    thinker_t	thinker;
    sector_t	*sector;
    int		count;
    int		maxlight;
    int		minlight;
} fireflicker_t;

typedef struct
{
    thinker_t	thinker;
    sector_t	*sector;
    int		count;
    int		maxlight;
    int		minlight;
    int		maxtime;
    int		mintime;
} lightflash_t;

typedef struct
{
    thinker_t	thinker;
    sector_t	*sector;
    int		count;
    int		minlight;
    int		maxlight;
    int		darktime;
    int		brighttime;
} strobe_t;

typedef struct
{
    thinker_t	thinker;
    sector_t	*sector;
    int		minlight;
    int		maxlight;
    int		direction;
} glow_t;

#define GLOWSPEED		8
#define STROBEBRIGHT		5
#define FASTDARK		15
#define SLOWDARK		35

void    P_SpawnFireFlicker(sector_t *sector);
void    T_LightFlash(lightflash_t *flash);
void    P_SpawnLightFlash(sector_t *sector);
void    T_StrobeFlash(strobe_t *flash);
void	P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, int inSync);
void    EV_StartLightStrobing(line_t *line);
void    EV_TurnTagLightsOff(line_t *line);
void	EV_LightTurnOn (line_t *line, int bright);
void    T_Glow(glow_t *g);
void    P_SpawnGlowingLight(sector_t *sector);
void	T_FireFlicker(fireflicker_t *flick);

//
// P_SWITCH
//
#pragma pack (1)		// necessary because loaded from WAD now
typedef struct
{
    char	name1[9];
    char	name2[9];
    short	episode;
} switchlist_t;
#ifdef IRIX
#pragma pack (0)
#else
#pragma pack ()
#endif

typedef enum
{
    top,
    middle,
    bottom
} bwhere_e;

typedef struct
{
    line_t	*line;
    bwhere_e	where;
    int		btexture;
    int		btimer;
    mobj_t	*soundorg;
} button_t;

 // max # of wall switches in a level
#define MAXSWITCHES		50

 // 4 players, 4 buttons each at once, max.
#define MAXBUTTONS		(MAXPLAYERS * 4)

 // 1 second, in ticks.
#define BUTTONTIME		35

extern button_t	buttonlist[MAXBUTTONS];

void	P_ChangeSwitchTexture(line_t *line, int useAgain);
void	P_InitSwitchList(void);

//
// P_PLATS
//
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
    blazeDWUS,
    genLift,
    genPerpetual,
    toggleUpDn
} plattype_e;

typedef struct
{
    thinker_t		thinker;
    sector_t		*sector;
    fixed_t		speed;
    fixed_t		low;
    fixed_t		high;
    int			wait;
    int			count;
    plat_e		status;
    plat_e		oldstatus;
    boolean		crush;
    int			tag;
    plattype_e		type;
    struct platlist	*list;
} plat_t;

// New limit-free plat structure, from Boom
typedef struct platlist
{
    plat_t		*plat;
    struct platlist	*next;
    struct platlist	**prev;
} platlist_t;

// names for the target field of the generalized lift
typedef enum
{
    F2LnF,
    F2NnF,
    F2LnC,
    LnF2HnF
} lifttarget_e;

#define PLATWAIT		3
#define PLATSPEED		FRACUNIT

// masks and shifts for generalized lift type fields
#define LiftTarget		0x0300
#define LiftDelay		0x00c0
#define LiftMonster		0x0020
#define LiftSpeed		0x0018

#define LiftTargetShift		8
#define LiftDelayShift		6
#define LiftMonsterShift	5
#define LiftSpeedShift		3

extern platlist_t	*activeplats;

void    T_PlatRaise(plat_t *plat);
int	EV_DoPlat(line_t *line, plattype_e type, int amount);
int	EV_DoGenLift(line_t *line);
void    P_AddActivePlat(plat_t *plat);
void    P_RemoveActivePlat(plat_t *plat);
void	P_RemoveAllActivePlats(void);
void    EV_StopPlat(line_t *line);
void    P_ActivateInStasis(int tag);

//
// P_DOORS
//
typedef enum
{
    normal,
    close30ThenOpen,
    close,
    open,
    raiseIn5Mins,
    blazeRaise,
    blazeOpen,
    blazeClose,
    genRaise,
    genBlazeRaise,
    genOpen,
    genBlazeOpen,
    genClose,
    genBlazeClose,
    genCdO,
    genBlazeCdO
} vldoor_e;

typedef struct
{
    thinker_t		thinker;
    vldoor_e		type;
    sector_t 		*sector;
    fixed_t		topheight;
    fixed_t		speed;

    // 1 = up, 0 = waiting at top, -1 = down
    int			direction;

    // tics to wait at the top
    int			topwait;

    // (keep in case a door going down is reset)
    // when it reaches 0, start going down
    int			topcountdown;

    // keep track of line door is triggered by
    line_t		*line;
} vldoor_t;

// names for kind of generalized doors
typedef enum
{
    OdCDoor,
    ODoor,
    CdODoor,
    CDoor
} doorkind_e;

// names for kind of generalized locked doors
typedef enum
{
    AnyKey,
    RCard,
    BCard,
    YCard,
    RSkull,
    BSkull,
    YSkull,
    AllKeys
} keykind_e;

// masks and shifts for generalized door type fields
#define DoorDelay		0x0300
#define DoorMonster		0x0080
#define DoorKind		0x0060
#define DoorSpeed		0x0018

#define DoorDelayShift		8
#define DoorMonsterShift	7
#define DoorKindShift		5
#define DoorSpeedShift		3

// masks and shifts for generalized locked door type fields
#define LockedNKeys		0x0200
#define LockedKey		0x01c0
#define LockedKind		0x0020
#define LockedSpeed		0x0018

#define LockedNKeysShift	9
#define LockedKeyShift		6
#define LockedKindShift		5
#define LockedSpeedShift	3

#define VDOORSPEED		FRACUNIT * 2
#define VDOORWAIT		150

void	EV_VerticalDoor(line_t *line, mobj_t *thing);
int	EV_DoDoor(line_t *line, vldoor_e type);
int	EV_DoGenDoor(line_t *line);
int	EV_DoSilentDoor(line_t *line, vldoor_e type);
int	EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing);
int	EV_DoGenLockedDoor(line_t *line);
void    T_VerticalDoor(vldoor_t *door);
void    P_SpawnDoorCloseIn30(sector_t *sec);
void	P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum);

//
//      Sliding doors...
//

// how many frames of animation
#define SNUMFRAMES		8

#define SDOORWAIT		35 * 3
#define SWAITTICS		4

// how many diff. types of anims
#define MAXSLIDEDOORS		5

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
    line_t	*line;
    int		frame;
    int		whichDoorIndex;
    int		timer;
    sector_t	*frontsector;
    sector_t	*backsector;
    sd_e	status;
} slidedoor_t;

typedef struct
{
    char	frontFrame1[9];
    char	frontFrame2[9];
    char	frontFrame3[9];
    char	frontFrame4[9];
    char	frontFrame5[9];
    char	frontFrame6[9];
    char	frontFrame7[9];
    char	frontFrame8[9];
    char	backFrame1[9];
    char	backFrame2[9];
    char	backFrame3[9];
    char	backFrame4[9];
    char	backFrame5[9];
    char	backFrame6[9];
    char	backFrame7[9];
    char	backFrame8[9];
} slidename_t;

typedef struct
{
    int             frontFrames[SNUMFRAMES];
    int             backFrames[SNUMFRAMES];
} slideframe_t;

void P_InitSlidingDoorFrames(void);
void EV_SlidingDoor(line_t *line, mobj_t *thing);
void T_SlidingDoor(slidedoor_t *door);

//
//      Force fields...
//
#define FFWAITTICS 10 * 35	// how long to wait until switch field back on

typedef enum
{
    ff_open,
    ff_waiting,
    ff_damaged
} ff_e;

typedef struct
{
    thinker_t	thinker;
    line_t	*line;
    short	oldflags;
    int		timer;
    sector_t	*frontsector;
    sector_t	*backsector;
    int		s0_texture;
    int		s1_texture;
    short	s0_lightlevel;
    short	s1_lightlevel;
    ff_e	status;
} forcefield_t;

void EV_ForceField(line_t *line, int flag);
void EV_PlusForceField(int id, line_t *ln, mobj_t *thing, int flag);
void T_ForceField(forcefield_t *field);

//
// P_CEILNG
//
typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerToLowest,
    lowerToMaxFloor,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise,
    genCeiling,
    genCeilingChg,
    genCeilingChg0,
    genCeilingChgT,
    genCrusher,
    genSilentCrusher
} ceiling_e;

typedef struct
{
    thinker_t		thinker;
    ceiling_e		type;
    sector_t		*sector;
    fixed_t		bottomheight;
    fixed_t		topheight;
    fixed_t		speed;
    fixed_t		oldspeed;
    boolean		crush;

    // support for ceiling changers
    int			newspecial;
    int			oldspecial;
    short		texture;

    // 1 = up, 0 = waiting, -1 = down
    int			direction;
    int			olddirection;

    // ID
    int			tag;

    struct ceilinglist	*list;
} ceiling_t;

// from Boom to remove limit on ceilings
typedef struct ceilinglist
{
    ceiling_t		*ceiling;
    struct ceilinglist	*next;
    struct ceilinglist	**prev;
} ceilinglist_t;

// names for the target field of generalized ceilings
typedef enum
{
    CtoHnC,
    CtoLnC,
    CtoNnC,
    CtoHnF,
    CtoF,
    CbyST,
    Cby24,
    Cby32
} ceilingtarget_e;

// names for the changer type field of generalized ceilings
typedef enum
{
    CNoCgh,
    CChgZero,
    CChgTxt,
    CChgTyp
} ceilingchange_e;

// names for the change model field of generalized ceilings
typedef enum
{
    CTriggerModel,
    CNumericModel
} ceilingmodel_t;

#define CEILSPEED		FRACUNIT
#define CEILWAIT		150

// masks and shifts for generalized ceiling type fields
#define CeilingCrush		0x1000
#define CeilingChange		0x0c00
#define CeilingTarget		0x0380
#define CeilingDirection	0x0040
#define CeilingModel		0x0020
#define CeilingSpeed		0x0018

#define CeilingCrushShift	12
#define CeilingChangeShift	10
#define CeilingTargetShift	 7
#define CeilingDirectionShift	 6
#define CeilingModelShift	 5
#define CeilingSpeedShift	 3

extern ceilinglist_t		*activeceilings;

int	EV_DoCeiling(line_t *line, ceiling_e type);
int	EV_DoGenCeiling(line_t *line);
void	T_MoveCeiling (ceiling_t *ceiling);
void	P_AddActiveCeiling(ceiling_t *c);
void	P_RemoveActiveCeiling(ceiling_t *c);
void	P_RemoveAllActiveCeilings(void);
int	EV_CeilingCrushStop(line_t *line);
int	P_ActivateInStasisCeiling(line_t *line);

//
// P_FLOOR
//
#define FLOORSPEED		FRACUNIT
#define ELEVATORSPEED		(FRACUNIT * 4)

typedef enum
{
    // lower floor to highest surrounding floor
    lowerFloor,

    // lower floor to lowest surrounding floor
    lowerFloorToLowest,

    // lower floor to next lowest floor
    lowerFloorToNearest,

    // lower floor 24 absolute
    lowerFloor24,

    // lower floor 32 absolute
    lowerFloor32Turbo,

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
    raiseFloor32Turbo,
    raiseFloor24AndChange,
    raiseFloorCrush,

     // raise to next highest floor, turbo-speed
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512,

    // types for generalized floor movers
    genFloor,
    genFloorChg,
    genFloorChg0,
    genFloorChgT,

    // stair builders
    buildStair,
    genBuildStair
} floor_e;

typedef enum
{
    build8,	// slowly build by 8
    turbo16	// quickly build by 16
} stair_e;

typedef enum
{
    ok,
    crushed,
    pastdest
} result_e;

typedef enum
{
    elevateUp,
    elevateDown,
    elevateCurrent
} elevator_e;

typedef struct
{
    thinker_t	thinker;
    floor_e	type;
    boolean	crush;
    sector_t	*sector;
    int		direction;
    int		newspecial;
    int		oldspecial;
    short	texture;
    fixed_t	floordestheight;
    fixed_t	speed;
} floormove_t;

typedef struct
{
    thinker_t	thinker;
    elevator_e	type;
    sector_t	*sector;
    int		direction;
    fixed_t	floordestheight;
    fixed_t	ceilingdestheight;
    fixed_t	speed;
} elevator_t;

// names for the target field of generalized floors
typedef enum
{
    FtoHnF,
    FtoLnF,
    FtoNnF,
    FtoLnC,
    FtoC,
    FbyST,
    Fby24,
    Fby32
} floortarget_e;

// names for the changer type field of generalized floors
typedef enum
{
    FNoChg,
    FChgZero,
    FChgTxt,
    FChgTyp
} floorchange_e;

// names for the change model field of generalized floors
typedef enum
{
    FTriggerModel,
    FNumericModel
} floormodel_t;

// masks and shifts for generalized floor type fields
#define FloorCrush		0x1000
#define FloorChange		0x0c00
#define FloorTarget		0x0380
#define FloorDirection		0x0040
#define FloorModel		0x0020
#define FloorSpeed		0x0018

#define FloorCrushShift		12
#define FloorChangeShift	10
#define FloorTargetShift	 7
#define FloorDirectionShift	 6
#define FloorModelShift		 5
#define FloorSpeedShift		 3

// masks and shift for generalized stair type fields
#define StairIgnore		0x0200
#define StairDirection		0x0100
#define StairStep		0x00c0
#define StairMonster		0x0020
#define StairSpeed		0x0018

#define StairIgnoreShift	9
#define StairDirectionShift	8
#define StairStepShift		6
#define StairMonsterShift	5
#define StairSpeedShift		3

// define masks and shift for generalized crusher type fields
#define CrusherSilent		0x0040
#define CrusherMonster		0x0020
#define CrusherSpeed		0x0018

#define CrusherSilentShift	6
#define CrusherMonsterShift	5
#define CrusherSpeedShift	3

result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest,
		boolean	crush, int floorOrCeiling, int direction);
int	EV_BuildStairs(line_t *line, stair_e type);
int	EV_DoGenStairs(line_t *line);
int	EV_DoFloor(line_t *line, floor_e floortype);
int	EV_DoGenFloor(line_t *line);
int	EV_DoChange(line_t *line, change_e changetype);
void	T_MoveFloor(floormove_t *floor);
int	EV_DoElevator(line_t *line, elevator_e elevtype);
void	T_MoveElevator(elevator_t *elevator);
int	EV_DoGenCrusher(line_t *line);

//
// P_TELEPT
//
int	EV_Teleport(line_t *line, int side, mobj_t *thing);
int	EV_SilentTeleport(line_t *line, int side, mobj_t *thing);
int	EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing,
			      boolean reverse);

//
// SCROLLING
//
typedef struct
{
    thinker_t	thinker;	// Thinker structure for scrolling
    fixed_t	dx, dy;		// (dx, dy) scroll speeds
    int		affectee;	// no. of affected sidedefs...
    int		control;	// Control sector, -1 if none
    fixed_t	last_height;	// Last known height of control sector
    fixed_t	vdx, vdy;	// Accumulated velocity if accelerative
    int		accel;		// Whether it's accelerative
    enum			// Type of scroll effect
    {
	sc_side,		// scroll walls
	sc_floor,		// scroll floor
	sc_ceiling,		// scroll ceiling
	sc_carry
    } type;
} scroll_t;

void	T_Scroll(scroll_t *scroll);
void	Add_Scroller(int type, fixed_t dx, fixed_t dy, int control,
		     int affectee, int accel);
//
// PUSH/PULL EFFECTS
//
typedef struct {
    thinker_t thinker;		// Thinker structure for Pusher
    enum
    {
	p_push,
	p_pull,
	p_wind,
	p_current
    } type;
    mobj_t	*source;	// Point source if point pusher
    int		x_mag;		// X Strength
    int		y_mag;		// Y Strength
    int		magnitude;	// Vector strength for point pusher
    int		radius;		// Effective radius for point pusher
    int		x;		// X of point source if point pusher
    int		y;		// Y of point source if point pusher
    int		affectee;	// Number of affected sector
} pusher_t;

void	T_Pusher(pusher_t *pusher);

//
// FRICTION EFFECT
//
typedef struct {
    thinker_t	thinker;	// Thinker structure for friction
    int		friction;	// friction value (E800 = normal)
    int		movefactor;	// inertia factor when adding to momentum
    int		affectee;	// Number of affected sector
} friction_t;

void	T_Friction(friction_t *friction);

#endif
