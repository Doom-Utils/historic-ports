//
// DOSDoom Definition Files Code (main)
//
// By the DOSDoom Team
//

#ifndef __DDF_MAIN__
#define __DDF_MAIN__

// Needed for action function pointer handling.
#include "d_think.h"

#include "dm_defs.h"

extern int MAXSPRITES;
extern int MAXSTATES;

#define DDF_LineHashFunc(trignum) ((trignum) & 255)

//-------------------------------------------------------------------------
//-----------------OLD THING STUFF (TO BE DELETED)-------------------------
//-------------------------------------------------------------------------

// Sprite Names ENUM
// -KM- 1998/11/25 Removed externalised sprites, frames, and things
typedef enum
{
    SPR_TROO,
    SPR_MISL,
    SPR_BBRN,
    SPR_BOSF,
    SPR_FIRE,
    ORIG_NUMSPRITES
} spritenum_t;


// states enum(!?)
typedef enum
{
    S_NULL,
    S_LIGHTDONE,
    S_ROCKET,
    S_EXPLODE1,
    S_EXPLODE2,
    S_EXPLODE3,
    S_BRAIN,
    S_BRAIN_PAIN,
    S_BRAIN_DIE1,
    S_BRAIN_DIE2,
    S_BRAIN_DIE3,
    S_BRAIN_DIE4,
    S_BRAINEYE,
    S_BRAINEYESEE,
    S_BRAINEYE1,
    S_SPAWN1,
    S_SPAWN2,
    S_SPAWN3,
    S_SPAWN4,
    S_SPAWNFIRE1,
    S_SPAWNFIRE2,
    S_SPAWNFIRE3,
    S_SPAWNFIRE4,
    S_SPAWNFIRE5,
    S_SPAWNFIRE6,
    S_SPAWNFIRE7,
    S_SPAWNFIRE8,
    S_BRAINEXPLODE1,
    S_BRAINEXPLODE2,
    S_BRAINEXPLODE3,
    ORIG_NUMSTATES
}
statenum_t;

// Original MOBJ enum
typedef enum
{
    MT_BOSSBRAIN,
    MT_BOSSSPIT,
    MT_BOSSTARGET,
    MT_SPAWNSHOT,
    MT_SPAWNFIRE,
    MT_ROCKET,
    ORIG_NUMMOBJTYPES
}
mobjtype_t;

// special mobj types
typedef enum
{
  MOBJ_SPAWNSPOT,
  NUMMOBJSPEC
}
specialtype_t;

// State Struct
typedef struct
{
  int sprite;             // sprite ref
  long frame;             // frame ref
  long tics;              // duration in tics
  actionf_t action;       // routine to be performed
  int nextstate;          // next state ref
  long misc1, misc2;      // misc stuff
}
state_t;

// ------------------------------------------------------------------
// -------------------------------SFX--------------------------------
// ------------------------------------------------------------------
// -KM- 1998/10/29
typedef struct
{
  int num;
  int sounds[0];
} sfx_t;

// Bastard SFX are sounds that are hardcoded into the
// code.  They should be removed if at all possible

typedef struct
{
  sfx_t *s;
  char name[8];
} bastard_sfx_t;

extern bastard_sfx_t bastard_sfx[];

#define sfx_swtchn bastard_sfx[0].s
#define sfx_tink bastard_sfx[1].s
#define sfx_radio bastard_sfx[2].s
#define sfx_oof bastard_sfx[3].s
#define sfx_slop bastard_sfx[4].s
#define sfx_itmbk bastard_sfx[5].s
#define sfx_pstop bastard_sfx[6].s
#define sfx_stnmov bastard_sfx[7].s
#define sfx_pistol bastard_sfx[8].s
#define sfx_swtchx bastard_sfx[9].s
#define sfx_noway bastard_sfx[10].s
#define sfx_bossit bastard_sfx[11].s
#define sfx_bospn bastard_sfx[12].s
#define sfx_bosdth bastard_sfx[13].s
#define sfx_bospit bastard_sfx[14].s
#define sfx_telept bastard_sfx[15].s
#define sfx_boscub bastard_sfx[16].s
#define sfx_pldeth bastard_sfx[17].s
#define sfx_pdiehi bastard_sfx[18].s
#define sfx_shotgn bastard_sfx[19].s
#define sfx_barexp bastard_sfx[20].s
#define sfx_sgcock bastard_sfx[21].s
#define sfx_jpmove bastard_sfx[22].s
#define sfx_jpidle bastard_sfx[23].s
#define sfx_jprise bastard_sfx[24].s
#define sfx_jpdown bastard_sfx[25].s
#define sfx_jpflow bastard_sfx[26].s

#define sfx_None (sfx_t*) NULL

// ------------------------------------------------------------------
// --------------------MOVING OBJECT INFORMATION---------------------
// ------------------------------------------------------------------
// -KM- 1998/10/29 sfx_t SFXs
// -KM- 1998/11/25 Added weapons, changed invisibility + accuracy.
typedef struct mobjinfo_s
{
  int doomednum;
  int spawnstate;
  int spawnhealth;
  int seestate;
  sfx_t* seesound;
  int reactiontime;
  sfx_t* attacksound;
  int painstate;
  int painchance;
  sfx_t* painsound;
  int meleestate;
  int missilestate;
  int deathstate;
  int xdeathstate;
  sfx_t* deathsound;
  int speed;
  int radius;
  int height;
  int mass;
  int damage;
  sfx_t* activesound;
  int attackmissile;
  int flags;
  int raisestate;
  int extendedflags;
  int damagerange;
  int damagemulti;
  int type;
  int benefittype;
  int benefitweapon;
  int benefitamount;
  int benefitammo;
  int limit;
  int castorder;
  int respawntime;
  fixed_t invisibility;
  int minatkchance;
  sfx_t* walksound;
  int meanderstate;
  int resstate;
  int palremap;
  char *name;
  char *message;
  fixed_t jumpheight;
  fixed_t maxfall;
  fixed_t fast;
  int fuse;
  int side;
  int playernum;
  struct backpack_s* backpackinfo;
  struct attacktype_s* closecombat;
  struct attacktype_s* rangeattack;
  struct attacktype_s* spareattack;
  struct mobjinfo_s* dropitem;
  struct mobjinfo_s* blood;
  struct mobjinfo_s* gib;
  struct mobjinfo_s* respawneffect;
  struct mobjinfo_s* next;
}
mobjinfo_t;

// ------------------------------------------------------------------
// --------------------ATTACK TYPE STRUCTURES------------------------
// ------------------------------------------------------------------
typedef struct attacktype_s
{
  char *name;
  int attackstyle;
  sfx_t* initsound;
  sfx_t* sound;
  fixed_t accuracy;
  int xoffset;
  int yoffset;
  int height;
  int range;
  int speed;
  int flags;
  int count;
  int tooclose;
  int damage;
  int damagerange;
  int damagemulti;
  int objinitstate;
  int notracechance;
  int keepfirechance;
  struct mobjinfo_s *projectile;
  struct mobjinfo_s *aimspot;
  struct mobjinfo_s *puff;
  struct attacktype_s *next;
}
attacktype_t;

// -KM- 1998/11/25 Added BFG SPRAY attack type.
typedef enum
{
  ATK_PROJECTILE,
  ATK_SPAWNER,
  ATK_TRIPLESPAWNER,
  ATK_SPREADER,
  ATK_RANDOMSPREAD,
  ATK_SHOT,
  ATK_TRACKER,
  ATK_CLOSECOMBAT,
  ATK_SHOOTTOSPOT,
  ATK_SKULLFLY,
  ATK_SMARTPROJECTILE,
  ATK_SPRAY,
  NUMATKCLASS
}
attackstyle_t;

typedef enum
{
  AF_TRACESMOKE      = 1,
  AF_SPAWNREMOVEFAIL = 2,
  AF_SPAWNPRESTEP    = 4,
  AF_SPAWNTELEFRAGS  = 8,
  AF_NEEDSIGHT       = 16,
  AF_FACETARGET      = 32,
  AF_PLAYER          = 64
}
attackflags_t;

typedef enum
{
  PERFECT  = 0,
  HIGH     = 17,
  MEDIUM   = 18,
  LOW      = 19,
  PATHETIC = 20
}
accuracy_t;

// ------------------------------------------------------------------
// -----------------------WEAPON HANDLING----------------------------
// ------------------------------------------------------------------

// Ammunition types defined.
typedef enum
{
  am_noammo= -1,// Unlimited for chainsaw / fist.
  am_clip,	// Pistol / chaingun ammo.
  am_shell,	// Shotgun / double barreled shotgun.
  am_misl,	// Missile launcher.
  am_cell	// Plasma rifle, BFG.
}
ammotype_t;
extern int NUMAMMO;


// The defined weapons,
//
// including a marker indicating
// user has not changed weapon.
//
// -KM- 1998/11/25 Weapons externalised, except for none and change.
typedef enum
{
  // No pending weapon change.
  wp_nochange = -2,
  wp_none = -1
}
weapontype_t;

// -KM- 1998/11/25 Number of weapons defined.
extern int numweapons;

// Weapon info: sprite frames, ammunition use.
typedef struct weaponinfo_s
{
  // Type of ammo this weapon uses.
  ammotype_t ammo;
  // The state to use when raising the weapon
  int upstate;
  // The state to use when lowering the weapon (if changing weapon)
  int downstate;
  // The state that the weapon is ready to fire in.
  int readystate;
  // The state that shows the weapon 'firing'
  int atkstate;
  // The state that shows the muzzle flash
  int flashstate;
  // Not sure what Andy meant to do with this...
  int reloadstate;
  // Ammo used per shot.
  int ammopershot;
  // Weapon's name
  char* name;
  // If true, this is an automatic weapon.  If false it is semiautomatic.
  boolean autofire;
  // The player gets this weapon on spawn.  (Fist + Pistol)
  boolean autogive;
  // This weapon gives feedback on hit (chainsaw
  boolean feedback;
  // Amount of kick this weapon gives
  fixed_t kick;
  // This weapon upgrades a previous one. (Berserk -> Fist)
  int replaces;
  // This affects how it will be selected if out of ammo
  int priority;
  // Sounds.
  // Played at the start of every readystate
  sfx_t* idle;
  // Played while the trigger is held (chainsaw)
  sfx_t* engaged;
  // Played while the trigger is held and it is pointed at a target.
  sfx_t* hit;
  // Played when the weapon is selected
  sfx_t* start;
  // Misc sounds
  sfx_t* sound1;
  sfx_t* sound2;
  sfx_t* sound3;
  // This close combat weapon should not throw the target away (chainsaw)
  boolean nothrust;
  // Amount of shots in a clip
  int clip;
  // Attack type used.
  struct attacktype_s *attack;
}
weaponinfo_t;

// -KM- 1998/11/25 Dynamic number of choices, 10 keys.
typedef struct
{
  int numchoices;
  int* choice;
  int choiceon;
  boolean available;
}
weaponkey_t;

// ------------------------------------------------------------------
// --------------------------ANIMATIONS------------------------------
// ------------------------------------------------------------------

//
// source animation definition
//
// -KM- 98/07/31 Anims.ddf
//
typedef struct
{
  boolean istexture;	// if false, it is a flat
  char endname[9];
  char startname[9];
  int speed;
}
animdef_t;

// ------------------------------------------------------------------
// ---------------MAP STRUCTURES AND CONFIGURATION-------------------
// ------------------------------------------------------------------
// -KM- 1998/11/25 Added generalised Finale type.
typedef struct
{
  // Text
  char *text;
  char text_back[9];
  char text_flat[9];
  int  text_speed;
  int  text_wait;

  // Pic
  int  numpics;
  int  picwait;
  char* pics;

  // Cast
  boolean docast;

  // Bunny
  boolean dobunny;

  // Music
  char music[9];
} finale_t;

typedef struct mapstuff_s
{
  char *name;
  char *description;
  char graphicname[9];
  char lump[9];
  char sky[9];
  char music[9];
  char surround[9];
  int flags;
  int partime;
  struct wi_map_s *interscr;
  struct mapstuff_s *next;      // next in the list
  struct mapstuff_s *nextlevel; // next level to go
  char *nextmapname;            // name of the next normal level
  char *secretmapname;          // name of the secret level
  // -KM- 1998/11/25 All lines with this trigger will be activated at
  //   the level start. (MAP07)
  int autotag;
  // -KM- 1998/11/25 Generalised finales.
  finale_t f[2];
}
mapstuff_t;

typedef enum
{
  MPF_NOJUMPING   = 1,             // Disable Jumping
  MPF_NOMLOOK     = 2,             // Disable Freelook
  MPF_NOTRANSLUC  = 4,             // Disable Translucency
  MPF_NOCHEATS    = 8,             // Disable Cheats
  MPF_ITEMRESPAWN = 16,            // Force Item Respawn
  MPF_NOITEMRESPN = 32,            // Disable Item Respawn
  MPF_FAST        = 64,            // Force Fast Monsters
  MPF_RESMONSTER  = 128,           // Force Resurrect Monsters
  MPF_TELMONSTER  = 256,           // Force Teleport Monsters
  MPF_STRETCHSKY  = 512,           // Force Stretch Sky
  MPF_NORMALSKY   = 1024,          // Force Normal Sky
  MPF_NOTRUE3D    = 2048,          // Disable True 3D Gameplay
  MPF_NOSTOMP     = 4096,          // Monsters cannot stomp players
  MPF_NORMBLOOD   = 8192,          // Use Normal Blood activity
  MPF_RESPAWN     = 16384,         // Respawn ON
  MPF_NORESPAWN   = 32768,         // Respawn OFF
  MPF_AUTOAIMOFF  = 0x30000,       // Autoaim on
  MPF_AUTOAIMON   = 0x10000,
  MPF_AUTOAIMMLOOK= 0x20000
}
mapsettings_t;

// ------------------------------------------------------------------
// -------------------------INTERMISSIONS----------------------------
// ------------------------------------------------------------------
typedef enum
{
  WI_NORMAL,
  WI_LEVEL
} animtype_t;

typedef struct
{
  int x;
  int y;
} point_t;

struct patch_s;

typedef struct
{
  // Tics on this frame
  int tics;
  // Position on screen where this goes
  point_t pos;

  // Name of pic to display.
  char pic[9];
  struct patch_s* p;
} wi_frame_t;

typedef struct wi_anim_s
{
  animtype_t type;
  char level[9];

  int numframes;
  wi_frame_t* frames;

  // Countdown number of tics
  int count;
  int frameon;
} wi_anim_t;

typedef struct
{
  point_t pos;
  char name[9];
} mappos_t;

typedef struct wi_map_s
{
  char* name;

  wi_anim_t* anims;
  int numanims;

  mappos_t* mappos;
  boolean* mapdone;
  int nummaps;

  char background[9];
  char splatpic[9];
  char yah[2][9];
  char music[9];

  sfx_t* percent;
  sfx_t* done;
  sfx_t* endmap;
  sfx_t* nextmap;

  char firstmap[9];
  char graphicname[9];
  char** titlepics;
  int numtitlepics;
  char titlemusic[9];
  int titletics;
} wi_map_t;

wi_map_t* DDF_GameGetName(char* name);
extern wi_map_t* wi_maps;
extern int wi_nummaps;

// ------------------------------------------------------------------
// ---------------------------LANGUAGES------------------------------
// ------------------------------------------------------------------
typedef struct langref_s
{
 char* refname;
 char* string;
 struct langref_s *next;
}
langref_t;

// ------------------------------------------------------------------
// ------------------------BENEFIT TYPES-----------------------------
// ------------------------------------------------------------------
// -KM- 1998/11/25 Weapon generalised.
typedef enum
{
  AMMO_TYPE = -2,       // AMMO
  WEAPON = -1,           // Weapon
  KEY_BLUECARD,
  KEY_REDCARD,
  KEY_YELLOWCARD,
  KEY_BLUESKULL,
  KEY_REDSKULL,
  KEY_YELLOWSKULL,
  POWERUP_ACIDSUIT,
  POWERUP_ARMOUR,
  POWERUP_AUTOMAP,
  POWERUP_BACKPACK,
  POWERUP_BERSERK,
  POWERUP_HEALTH,
  POWERUP_HEALTHARMOUR,
  POWERUP_INVULNERABLE,
  POWERUP_JETPACK,
  POWERUP_LIGHTGOGGLES,
  POWERUP_NIGHTVISION,
  POWERUP_PARTINVIS,
  NUMOFBENEFITS
}
benefit_t;

// ------------------------------------------------------------------
// ---------------------BACKPACK INFORMATION-------------------------
// ------------------------------------------------------------------
// -KM- 1998/11/25 Weapons generalised.
typedef struct backpack_s
{
  int* ammo;
  int* ammolimit;
  int cards[NUMCARDS];
  int* weapons;
  int armour;
  int armourclass;
  struct backpack_s *next;
}
backpack_t;

// ------------------------------------------------------------------
// ------------------------LINEDEF TYPES-----------------------------
// ------------------------------------------------------------------

// Triggers (What the line triggers)
typedef enum
{
  line_none,
  line_shootable,
  line_walkable,
  line_pushable
}
trigger_e;

// Triggers (What object types can cause the line to be triggered)
typedef enum
{
  trig_player = 1,
  trig_monster = 2,
  trig_projectile = 4
}
trigacttype_e;

// Height Info Reference
typedef enum
{
  ref_absolute,                  // Absolute from current position
  ref_floorHeight,               // Measure from floor height
  ref_ceilingHeight,             // Measure from ceiling height
  ref_LowestSurroundingCeiling,  // Measure from surrounding ceiling height
  ref_HighestSurroundingCeiling,
  ref_LowestSurroundingFloor,
  ref_HighestSurroundingFloor,
  ref_NextHighestFloor,
  ref_LowestLoTexture
}
heightref_e;

// Light Specials
typedef enum
{
  lite_none,
  lite_set,
  lite_fireflicker,
  lite_glow,
  lite_flash,
  lite_strobe
}
litetype_e;

// Movement type
typedef enum
{
  mov_undefined,
  mov_Once,
  mov_MoveWaitReturn,
  mov_Continuous,
  mov_Plat,
  mov_Stairs,
  mov_Stop
}
movetype_e;

// Security type: requires certain key
typedef enum
{
  KF_NONE        = 0x0,
  KF_BLUECARD    = 0x1,
  KF_YELLOWCARD  = 0x2,
  KF_REDCARD     = 0x4,
  KF_BLUESKULL   = 0x8,
  KF_YELLOWSKULL = 0x10,
  KF_REDSKULL    = 0x20,
  KF_REQUIRESALL = 0x3F
}
keys_e;

// Moving Planes (Ceilings, floors and doors)
typedef struct
{
  // Type of floor: raise/lower/etc
  movetype_e       type;

  // True for a ceiling, false for a floor
  boolean          floorOrCeiling;
  boolean          crush;

  // How fast the plane moves.
  int              speed_up;
  int              speed_down;

  // This refers to what the dest. height refers to.
  // 0 = absolute, 1 = surrounding ceiling
  heightref_e      destref;

  // Destination height.
  int              dest;

  // Floor texture to change to.
  char             *tex; // memory is allocated by DDF_MainGetString

  // PLAT/DOOR Specific
  // Time to wait before returning.
  int wait;
  int prewait;

  // Up/Down/Stop sfx
  sfx_t* sfxstart, *sfxup, *sfxdown, *sfxstop;
}
movinPlane_t;

// -KM- 1998/09/27 Generalisation of light types for lines + sectors
typedef struct
{
  litetype_e type;  
  int light; // Light level to change to.
  int darktime;
  int brighttime;
  int sync;
}
lighttype_t;

// Linedef structure itself
typedef struct linedeftype_s
{
  // The value you put into level editors
  int trignum;

  // Linedef will change to this.
  int newtrignum;

  // Determines whether line is shootable/walkable/pushable
  trigger_e type;

  // Determines whether line is acted on by monsters/players/projectiles
  trigacttype_e obj;

  // Keys required to use
  keys_e keys;

  char* lumpcheck;

  // Number of times this line can be triggered. -1 = Any amount
  int count;

  // Special sector type to change to.  Used to turn off acid
  int specialtype;

  // Crush.  If true, players will be crushed.  If false, obj will stop(/return)
  boolean crush;

  // Floor
  movinPlane_t f;

  // Ceiling
  movinPlane_t c;

  // DONUT SPECIFIC
  struct
  {
    boolean dodonut;            // Do Donut?
    sfx_t* d_sfxin, *d_sfxinstop;   // SFX for inner donut parts
    sfx_t* d_sfxout, *d_sfxoutstop; // SFX for outer donut parts
  }d;

  // TELEPORT SPECIFIC
  struct
  {
    boolean teleport;        // If true, teleport activator
    mobjinfo_t* inspawnobj;  // effect object spawned when going in...
    mobjinfo_t* outspawnobj; // effect object spawned when going out...
    int delay;               // Teleport delay
  }t;

  // LIGHT SPECIFIC
  // Things may be added here; start strobing/flashing glowing lights.
  lighttype_t l;

  // EXIT SPECIFIC
  int e_exit; // Exit Switch: 0 = no, 1 = yes, 2 = secret level.

  // SCROLLER SPECIFIC
  fixed_t s_xspeed;
  fixed_t s_yspeed;

  // -ACB- 1998/09/11 Message handling
  char* failedmessage;

  // Colourmap changing
  char colourmaplump[9];
  int  colourmap;

  // Gravity
  int gravity;
  int friction;

  sfx_t* sfx;

  char music[9];

  boolean autoline;

  boolean singlesided;

  // HASH TABLE
  boolean next;
}
linedeftype_t;

// -KM- 1998/09/27 Sectors.ddf stuff
typedef struct
{
  // The number you put in the wad file
  int trigger;

  // This sector gives you secret count
  boolean secret;

  char* lumpcheck;

  // Gravity
  int gravity;
  fixed_t friction;

  boolean crush;

  // Movement
  movinPlane_t f, c;

  // Lighting
  lighttype_t l;

  // Slime
  int damage;
  int damagetime;

  // Exit.  Also disables god mode.
  int exit;

  // Colourmap changing
  char colourmaplump[8];
  int  colourmap;

  // SFX
  sfx_t* sfx;

  fixed_t viscosity;

  // Internal Hash tabling
  boolean next;
}
specialsector_t;

// ------------------------------------------------------------------
// -------------------------EXTERNALISATIONS-------------------------
// ------------------------------------------------------------------

extern int NUMSTATES;
extern int NUMSPRITES;

extern state_t* states;
extern char **sprnames;

// -KM- 1998/11/25 Dynamic number of weapons, always 10 weapon keys.
extern weaponinfo_t *weaponinfo;
extern weaponkey_t weaponkey[10];

extern animdef_t *animdefs;

extern backpack_t* backpackhead; // -ACB- 1998/09/14 Backpack type list..

extern mobjinfo_t *specials[NUMMOBJSPEC];
extern mobjinfo_t *mobjinfohead;

extern mapstuff_t *maphead;
extern mapstuff_t *currentmap;

extern attacktype_t *attackhead;

extern linedeftype_t* specialLineDefs[256];

void DDF_MainInit(void);
void DDF_MainCleanUpLooseEnds();
sfx_t* DDF_LookupSound(char *name);
char* DDF_LanguageLookup(const char *refname);
mobjinfo_t* DDF_MobjLookup(const char *refname);
// -KM- 1998/11/25 Looks up a weapon by name
weapontype_t DDF_WeaponGetType(char* name);
mapstuff_t* DDF_LevelGetNewMap(const char *refname);
linedeftype_t* DDF_GetFromLineHashTable(int trignum);
// -KM- 1998/09/27 Sectors.ddf
specialsector_t* DDF_GetFromSectHashTable(int trigger);

// -KM- 1998/12/16 If you have a ddf file to add, call
//  this, passing the data and the size of the data in
//  memory.
void DDF_ReadAnims(void *data, int size);
void DDF_ReadAtks(void *data, int size);
void DDF_ReadGames(void *data, int size);
void DDF_ReadLangs(void *data, int size);
void DDF_ReadLevels(void *data, int size);
void DDF_ReadLines(void *data, int size);
void DDF_ReadThings(void *data, int size);
void DDF_ReadCreatures(void* data, int size);
void DDF_ReadItems(void* data, int size);
void DDF_ReadScenery(void *data, int size);
void DDF_ReadSectors(void *data, int size);
void DDF_ReadSFX(void* data, int size);
void DDF_ReadSW(void *data, int size);
void DDF_ReadWeapons(void *data, int size);


#endif

