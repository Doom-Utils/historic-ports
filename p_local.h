//  
// DOSDoom Local Header for play sim functions 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -ACB- 1998/07/27 Cleaned up, Can read it now :).
//

#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.h"
#endif

#define FLOATSPEED (FRACUNIT*4)  // Floating speed, should be info->floatspeed
#define VIEWHEIGHT (41*FRACUNIT) // Player Standard View Height
#define LOOKUPLIMIT   SHRT_MAX
#define LOOKDOWNLIMIT SHRT_MIN

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS	128
#define MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT	(FRACBITS+7)
#define MAPBMASK	(MAPBLOCKSIZE-1)
#define MAPBTOFRAC	(MAPBLOCKSHIFT-FRACBITS)

// player radius for movement checking
#define PLAYERRADIUS	16*FRACUNIT

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger, but we do not have any moving sectors nearby
#define MAXRADIUS       32*FRACUNIT

#define GRAVITY         (8192*grav)       
#define MAXMOVE		(100*FRACUNIT)
#define STEPMOVE        (10*FRACUNIT)
#define USERANGE	(64*FRACUNIT)
#define MELEERANGE	(64*FRACUNIT)
#define MISSILERANGE	(32*64*FRACUNIT)

// Weapon sprite speeds
#define LOWERSPEED		FRACUNIT*6
#define RAISESPEED		FRACUNIT*6

#define WPNLOWERSPEED FRACUNIT*6
#define WPNRAISESPEED FRACUNIT*6

#define WEAPONBOTTOM     128*FRACUNIT
#define WEAPONTOP        32*FRACUNIT

// follow a player exlusively for 3 seconds
#define	BASETHRESHOLD	 	100

extern mobj_t* RandomTarget;

//
// P_TICK
//

// both the head and tail of the thinker list
extern	thinker_t	thinkercap;	

void P_InitThinkers(void);
void P_AddThinker(thinker_t* thinker);
void P_RemoveThinker(thinker_t* thinker);

//
// P_ACTION
//
void P_ActPlayerAttack(mobj_t* playerobj, attacktype_t* attack);
void P_ActSlammedIntoObject(mobj_t *object, mobj_t* objecthit);
void P_ActMissileContact(mobj_t *object, mobj_t* objecthit);

//
// P_PSPR
//
void P_SetupPsprites (player_t* curplayer);
void P_SetPsprite(player_t* player, int position, int stnum);
void P_MovePsprites (player_t* curplayer);
void P_DropWeapon (player_t* player);
void P_BringUpWeapon (player_t* player);

//
// P_USER
//
void P_PlayerThink (player_t* player);

//
// P_MOBJ
//
#define ONFLOORZ		INT_MIN
#define ONCEILINGZ		INT_MAX

iteminque_t* itemquehead; // -ACB- 1998/07/30 Start Pointer the item-respawn-que.
mobj_t* mobjlisthead;     // -ACB- 1998/08/27 Start Pointer in the mobj list.

mobj_t* P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void 	P_RemoveMobj (mobj_t* th);
boolean	P_SetMobjState (mobj_t* mobj, statenum_t state);
void 	P_MobjThinker (mobj_t* mobj);
void	P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z, mobjinfo_t* puff);
void 	P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage, angle_t angle, mobjinfo_t* blood);
mobj_t* P_SpawnMissile (mobj_t* source, mobj_t* dest, mobjtype_t type);
void	P_SpawnPlayerMissile (mobj_t* source, mobjtype_t type);

// -ACB- 1998/08/02 New procedures for DDF etc...
void    P_MobjItemRespawn (void);
void    P_MobjRemoveMissile (mobj_t *missile);
mobj_t* P_MobjCreateObject (fixed_t x, fixed_t y, fixed_t z, mobjinfo_t *type);

//
// P_ENEMY
//
// Directions
typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS    
}
dirtype_t;

extern dirtype_t opposite[];
extern dirtype_t diags[];
extern fixed_t xspeed[8];
extern fixed_t yspeed[8];

void P_NoiseAlert(mobj_t* target, mobj_t* emmiter);
void P_NewChaseDir(mobj_t* actor);
boolean P_CreateAggression(mobj_t *actor);
boolean P_CheckMeleeRange(mobj_t* actor);
boolean P_CheckMissileRange(mobj_t* actor);
boolean P_Move(mobj_t* actor);
boolean P_LookForPlayers (mobj_t* actor, boolean allaround);

//
// P_MAPUTL
//
#define MAXINTERCEPTS	128
#define PT_ADDLINES	1
#define PT_ADDTHINGS	2
#define PT_EARLYOUT	4

typedef struct
{
    fixed_t	x;
    fixed_t	y;
    fixed_t	dx;
    fixed_t	dy;
    
} divline_t;

typedef struct
{
    fixed_t	frac;		// along trace line
    boolean	isaline;

    union
    {
     mobj_t* thing;
     line_t* line;
    } d;

} intercept_t;

typedef boolean (*traverser_t) (intercept_t *in);

extern int		maxintercepts;
extern intercept_t*	intercepts;
extern int		intercept_p;

extern fixed_t		opentop;
extern fixed_t 		openbottom;
extern fixed_t		openrange;
extern fixed_t		lowfloor;
extern divline_t trace;

fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int 	P_PointOnLineSide (fixed_t x, fixed_t y, line_t* line);
int 	P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t* line);
void 	P_MakeDivline (line_t* li, divline_t* dl);
fixed_t P_InterceptVector (divline_t* v2, divline_t* v1);
int 	P_BoxOnLineSide (fixed_t* tmbox, line_t* ld);
void 	P_LineOpening (line_t* linedef);
void    P_UnsetThingPosition (mobj_t* thing);
void    P_SetThingPosition (mobj_t* thing);
boolean P_BlockLinesIterator    (int x, int y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator   (int x, int y, boolean(*func)(mobj_t*) );

boolean P_PathTraverse
( fixed_t x1,
  fixed_t y1,
  fixed_t x2,
  fixed_t	y2,
  int		flags,
  boolean	(*trav) (intercept_t *));

//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean floatok;
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;

extern mobj_t* linetarget;	// who got hit (or NULL)
extern line_t* ceilingline;

boolean P_MapCheckBlockingLine(mobj_t* thing, mobj_t* spawnthing);
mobj_t* P_MapFindCorpse(mobj_t* thing);
mobj_t* P_MapTargetAutoAim(mobj_t* source, angle_t angle, fixed_t distance);
mobj_t* P_MapTargetTheory(mobj_t* source);

fixed_t P_AimLineAttack(mobj_t* t1, angle_t angle, fixed_t distance);
boolean P_ChangeSector(sector_t* sector, boolean crunch);
boolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);
boolean P_CheckSight(mobj_t* t1, mobj_t* t2);
void	P_SlideMove(mobj_t* mo);
void    P_RadiusAttack(mobj_t* spot, mobj_t* source, int damage);
boolean P_TeleportMove(mobj_t* thing, fixed_t x, fixed_t y);
boolean P_TryMove(mobj_t* thing, fixed_t x, fixed_t y);
void 	P_UseLines(player_t* player);

void P_LineAttack
         (mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int damage );

//
// P_SETUP
//
// 23-6-98 KM Short*s changed to int*s, for bigger, better blockmaps
//
extern byte*		rejectmatrix;	// for fast sight rejection
extern int*		blockmaplump;	// offsets in blockmap are from here
extern int*		blockmap;
extern int		bmapwidth;
extern int		bmapheight;	// in mapblocks
extern fixed_t		bmaporgx;
extern fixed_t		bmaporgy;	// origin of block map
extern mobj_t**		blocklinks;	// for thing chains

//
// P_INTER
//
extern int		maxammo[4];
//extern int		clipammo[NUMAMMO];

void P_TouchSpecialThing(mobj_t* special, mobj_t* toucher);
void P_DamageMobj(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage);
void P_KillMobj(mobj_t* source, mobj_t* target);

//
// P_SPEC
//
#include "p_spec.h"

#endif	// __P_LOCAL__

