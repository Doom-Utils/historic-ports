//  
// DOSDoom Moving Object Header
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// IMPORTANT NOTE: Altering anything within the mobj_t will most likely
//                 require changes to p_saveg.c and the save-game object
//                 (savegmobj_t); if you experience any problems with
//                 savegames, check here!
//

#ifndef __P_MOBJ__
#define __P_MOBJ__

#include "m_fixed.h"

#include "d_think.h"
#include "dm_data.h"
#include "ddf_main.h"
#include "lu_math.h"

#ifdef __GNUG__
#pragma interface
#endif

//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost always set
// from state_t structures.
//
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
//
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.  
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
    // Call P_TouchSpecialThing when touched.
    MF_SPECIAL		= 1,

    // Blocks.
    MF_SOLID		= 2,

    // Can be hit.
    MF_SHOOTABLE	= 4,

    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR		= 8,

    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP	= 16,                    

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH		= 32,

    // Will try to attack right back.
    MF_JUSTHIT		= 64,

    // Will take at least one step before attacking.
    MF_JUSTATTACKED	= 128,

    // On level spawning (initial position),
    // hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING	= 256,

    // Don't apply gravity (every tic), that is, object will float,
    // keeping current height or changing it actively.
    MF_NOGRAVITY	= 512,

    // Movement flags. This allows jumps from high places.
    MF_DROPOFF		= 0x400,

    // For players, will pick up items.
    MF_PICKUP		= 0x800,

    // Player cheat. Object is not checked when moving, no clipping is used.
    MF_NOCLIP		= 0x1000,

    // Player: keep info about sliding along walls.
    MF_SLIDE		= 0x2000,

    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT		= 0x4000,

    // Don't cross lines
    //   ??? or look at heights on teleport.
    MF_TELEPORT		= 0x8000,

    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE		= 0x10000,	

    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED		= 0x20000,

    // Use fuzzy draw (shadow demons or spectres),
    // temporary player invisibility powerup.
    MF_SHADOW		= 0x40000,

    // Flag: don't bleed when shot (use puff),
    // barrels and shootable furniture shall not bleed.
    MF_NOBLOOD		= 0x80000,

    // Don't stop moving halfway off a step,
    // that is, have dead bodies slide down all the way.
    MF_CORPSE		= 0x100000,

    // Floating to a height for a move, ???
    // don't auto float to target's height.
    MF_INFLOAT		= 0x200000,

    // On kill, count this enemy object
    // towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL	= 0x400000,
    
    // On picking up, count this item object
    // towards intermission item total.
    MF_COUNTITEM	= 0x800000,

    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF_SKULLFLY		= 0x1000000,

    // Don't spawn this object
    // in death match mode (e.g. key cards).
    MF_NOTDMATCH    	= 0x2000000,

    MF_STEALTH          = 0x4000000,

    // Used so bots know they have picked up their
    // target item.
    MF_JUSTPICKEDUP     = 0x8000000

} mobjflag_t;


typedef enum
{
    // Act like a big ugly bossman (ignores certain types of damage and
    // makes start and death sound at full volume regardless of location).
    EF_BOSSMAN        = 1,

    // Used when varying visibility levels
    EF_LESSVIS        = 2,

    // This thing does not respawn
    EF_NORESPAWN      = 4,

    // double the chance of object using range attack
    EF_NOGRAVKILL     = 8,

    // This thing is not loyal to its own type, fights its own
    EF_DISLOYALTYPE   = 16,

    // This thing can be hurt by another thing with same attack
    EF_OWNATTACKHURTS = 32,

    // Used for tracing (homing) projectiles, its the first time
    // this projectile has been checked for tracing if set.
    EF_FIRSTCHECK     = 64,

    // This projectile can trace, but if this is set it will not.
    EF_NOTRACE        = 128,

    // double the chance of object using range attack
    EF_TRIGGERHAPPY   = 256,

    // not targeted by other monsters for damaging them
    EF_NEVERTARGET    = 512,

    // Normally most monsters will follow a target which caused them
    // damage for a length of time, even if another object inflicted
    // pain upon them; with this enabled, they will not hold the grudge
    // and switch targets to the other object that has caused them the
    // more recent pain.
    EF_NOGRUDGE       = 1024,

    // This object is dummy, used for carring a dummy set of co-ordinates for
    // use as a target.
    EF_DUMMYMOBJ      = 2048,

    // Archvile cannot resurrect this monster
    EF_NORESURRECT    = 4096,

    // Object bounces
    EF_BOUNCE         = 8192

}
mobjextendedflag_t;

// Map Object definition.
typedef struct mobj_s
{
  // List: thinker links.
  thinker_t thinker;

  // Info for drawing: position.
  fixed_t x;
  fixed_t y;
  fixed_t z;

  // More list: links in sector (if needed)
  struct mobj_s* snext;
  struct mobj_s* sprev;

  //More drawing info: to determine current sprite.
  angle_t angle;      // orientation
  spritenum_t sprite; // used to find patch_t and flip value
  int frame;	      // might be ORed with FF_FULLBRIGHT

  // Interaction info, by BLOCKMAP.
  // Links in blocks (if needed).
  struct mobj_s* bnext;
  struct mobj_s* bprev;
    
  struct subsector_s* subsector;

  // The closest interval over all contacted Sectors.
  fixed_t floorz;
  fixed_t ceilingz;

  // For movement checking.
  fixed_t radius;
  fixed_t height;

  // Momentums, used to update position.
  fixed_t momx;
  fixed_t momy;
  fixed_t momz;

  // This is the current speed of the object.
  // if fastparm, it is already calculated.
  fixed_t speed;
  int fuse;

  // If == validcount, already checked.
  int validcount;

  int type;
  mobjinfo_t* info;
    
  int tics;	// state tic counter
  fixed_t fasttics; // -KM- 1998/12/16 This is tics for fastparm.
  state_t* state;
  int flags;
  int health;

  // Movement direction, movement generation (zig-zagging).
  int movedir;	        // 0-7
  int movecount;        // when 0, select a new dir

  // Reaction time: if non 0, don't attack yet.
  // Used by player to freeze a bit after teleporting.
  int reactiontime;

  // If >0, the target will be chased
  // no matter what (even if shot)
  int threshold;

  // Additional info record for player avatars only.
  struct player_s* player;

  // Player number last looked for.
  int lastlook;

  // For respawning.
  mapthing_t spawnpoint;

  // source of the mobj, used for projectiles
  struct mobj_s* source;

  // target of the mobj
  struct mobj_s* target;

  // current spawned fire of the mobj
  struct mobj_s* tracer;

  // if exists, we are supporting this object
  struct mobj_s* supportobj;
  int side;
    
  // DOSDoom Specifics
  byte playxtra;                      //-jc- Colours for players 5-8
  fixed_t origheight;

  // -KM- 1998/11/25 Made fixed.
  fixed_t invisibility;                  // 0 = invisible, FRACUNIT = visible
  fixed_t deltainvis;

  int extendedflags;                  // New flags

  backpack_t* backpackinfo;           // used if object is a BACKPACK type.

  fixed_t vertangle;                  // looking up or down.....

  attacktype_t* currentattack;        // current attack to be made

  int spreadcount;                    // spread count for Ordered spreaders

  struct mobj_s* next;                // next in the linked list
  struct mobj_s* prev;                // previous in the linked list
}
mobj_t;

// Item-in-Respawn-que Structure -ACB- 1998/07/30
typedef struct iteminque_s
{
  mapthing_t info;
  int time;
  struct iteminque_s* next;
  struct iteminque_s* prev;
}
iteminque_t;


#endif
