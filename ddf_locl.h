//
// DOSDoom Definition Files (Local Header)
//
// By the DOSDoom Team
//
#ifndef __DDF_LOCAL__
#define __DDF_LOCAL__

#include "d_think.h"
#include "p_pspr.h"
#include "dm_defs.h"

#define DDFVERSION 100

#define COMMAND_TERMINATOR "DOH!"

// defines for parser stuff.
#define BACKSLASH   '\\'
#define COMMANDREAD '='
#define DEFSTART    '['
#define DEFSTOP     ']'
#define DIVIDE      ':'
#define REDIRECTOR  '#'
#define REMARKSTART '{'
#define REMARKSTOP  '}'
#define SEPERATOR   ','
#define SPACE       ' '
#define STRINGSTART '\"'
#define STRINGSTOP  '\"'
#define SUBSPACE    '_'
#define TERMINATOR  ';'

#define BUFFERSIZE 1024
#define NUMSPLIT 7 // - Number of sections a state is split info

// enum thats gives the parser's current status
typedef enum
{
  waiting_newdef,
  reading_newdef,
  reading_command,
  reading_data,
  reading_remark,
  reading_string
}
readstatus_t;

// enum thats describes the return value from DDF_MainProcessChar
typedef enum
{
  nothing,
  command_read,
  def_start,
  def_stop,
  remark_start,
  remark_stop,
  seperator,
  string_start,
  string_stop,
  terminator,
  ok_char
}
readchar_t;

//
// This structure forms the basis for the command checking, it hands back a code
// pointer and sometimes a pointer to a (int) numeric value. (used for info that gets
// its value directly from the file).
//
typedef struct
{
  char name[30];
  void (*routine)();
  void* data;
}
commandlist_t;

//
// This structure passes the information needed to DDF_MainReadFile, so that
// the reader uses the correct procedures when reading a file.
//
typedef struct
{
  char* message;                                  // message displayed
  char* filename;                                 // filename
  char* memfile;
  size_t memsize;
  void  (*DDF_MainCheckName)     (char *);        // name checking code
  void  (*DDF_MainCreateEntry)   ();              // create entries in table(s).
  int   (*DDF_MainCheckCmd)      (char *);        // create entries in table(s).
  void  (*DDF_MainFinishingCode) ();              // clean up code
  commandlist_t *cmdlist;                         // list of commands
}
readinfo_t;

//
// This structure forms the basis for referencing specials, the table is used to
// determine what flags/extendedflags/level-of-invisibility is used for a thing
//
typedef struct
{
  char* specialname;
  int flags;
  int extendedflags;
  int invisibility;
}
specials_t;

//
// This structure forms the basis for referencing specials for the control of
// a single level, the table is used to determine what flags are need to be set
// up the level.
//
typedef struct
{
  char *name;
  int flag;
}
specflags_t;

//
// This is a reference table, that determines what code pointer is placed in the
// states table entry.
//
typedef struct
{
  char* actionname;
  actionf_t action;
}
actioncode_t;

typedef enum
{
  FDF_SPAWN     = 0x10000,
  FDF_CHASE     = 0x20000,
  FDF_MELEE     = 0x40000,
  FDF_MISSILE   = 0x80000,
  FDF_PAIN      = 0x100000,
  FDF_DEATH     = 0x200000,
  FDF_XDEATH    = 0x400000,
  FDF_RESPAWN   = 0x800000,
  FDF_RESURRECT = 0x1000000,
  FDF_MEANDER   = 0x2000000
}
framedestflag_t;

typedef struct
{
  char* redirector;
  int* state;
  int subflag;
}
framedest_t;

// DDF_MAIN Code (Reading all files, main init & generic functions).
void DDF_MainReadFile(readinfo_t *readinfo);
readchar_t DDF_MainProcessChar(char character, char *buffer, readstatus_t status);

void DDF_MainGetAccuracy(char *info, int commandref);
void DDF_MainGetBoolean(char *info, int commandref);
void DDF_MainGetDirector(char *info, int commandref);
void DDF_MainGetFixed(char *info, int commandref);
void DDF_MainGetLumpName(char *info, int commandref);
void DDF_MainGetMusicName(char *info, int commandref);
void DDF_MainGetNumeric(char *info, int commandref);
void DDF_MainGetSpecial(char *info, int commandref);
void DDF_MainGetString(char *info, int commandref);
void DDF_MainGetTime(char *info, int commandref);
void DDF_MainGetMType(char *info, int c);
void DDF_MainGetDestRef(char *info, int c);

int DDF_MainCheckCommand(char *info);
void DDF_MainLoadStates(char *info, int commandref);
void DDF_MainLookupSound(char *info, int commandref);
void DDF_MainMobjCheckName(char *info);
void DDF_MainRefAttack(char *info, int commandref);
void DDF_MainReferenceString(char *info, int commandref);

// -KM- 1998/09/27 Two new funcs shared by sectors + linedefs.
void DDF_MainGetLighttype(char *info, int c);
void DDF_MainGetExit(char *info, int c);

void DDF_DummyFunction(char *info);

// DDF_ANIM Code
void DDF_AnimInit(void);

// DDF_ATK Code
void DDF_AttackInit(void);

// DDF_GAME Code
void DDF_GameInit(void);

// DDF_LANG Code
void DDF_LanguageInit(void);

// DDF_LEVL Code
void DDF_LevelInit(void);

// DDF_LINE Code
void DDF_LinedefInit(void);

// DDF_MOBJ Code  (Moving Objects)
void DDF_MobjCreatureInit(void);
void DDF_MobjItemInit(void);
void DDF_MobjSceneInit(void);

// DDF_SECT Code
void DDF_SectorInit(void);

// DDF_SFX Code
void DDF_SFXInit(void);

// DDF_SWTH Code
// -KM- 1998/07/31 Switch and Anim ddfs.
void DDF_SWInit(void);

// DDF_WEAP Code
void DDF_WeaponInit(void);
extern weaponinfo_t bufferweapon;

// Virtual Function
void (*DDF_ReadFunction) (char *info, int commandref);

// Global stuff that is used by the parser.
extern backpack_t buffpack;
extern mapstuff_t buffermap;
extern mobjinfo_t buffermobj;
extern mobjinfo_t* bufferreplacemobj;
extern langref_t bufflangref;
extern int statecount;
extern char *stateinfo[NUMSPLIT+1];
extern state_t *tempstates[255];
extern boolean terminated;
extern boolean ddf_replace;

extern framedest_t framedestlist[];

// Action Routine pointers
void A_Light0();
void A_Light1();
void A_Light2();

void A_WeaponReady();
void A_FireWeapon();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();

void A_FirePistol();

void A_FireShotgun();

void A_FireShotgun2();
void A_CheckReload();
void A_SFXWeapon1();
void A_SFXWeapon2();
void A_SFXWeapon3();

void A_FireCGun();
void A_GunFlash();

void A_FireMissile();

void A_FirePlasma();

void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();

void A_Saw();

// Generally needed for remaining things...
void A_Explode();
void A_Pain();
void A_Scream();
void A_PlayerScream();
void A_XScream();
void A_Fall();

// Needed for the bossbrain.
void A_BrainPain();
void A_BrainScream();
void A_BrainDie();
void A_BrainAwake();
void A_BrainSpit();
void A_SpawnSound();
void A_SpawnFly();
void A_BrainExplode();

// MISC
void DDF_OldThingInit(void);

// Visibility Actions
void P_ActAlterTransluc();
void P_ActAlterVisibility();
void P_ActBecomeLessVisible();
void P_ActBecomeMoreVisible();

// Sound Actions
void P_ActMakeAmbientSound();
void P_ActMakeAmbientSoundRandom();
void P_ActMakeCloseAttemptSound();
void P_ActMakeDyingSound();
void P_ActMakeOverKillSound();
void P_ActMakePainSound();
void P_ActMakeRangeAttemptSound();

// Explosion Damage Actions
void P_ActSetDamageExplosion();
void P_ActVaryingDamageExplosion();

// Stand-by / Looking Actions
void P_ActStandardLook();
void P_ActPlayerSupportLook();

// Meander, aimless movement actions.
void P_ActStandardMeander();
void P_ActPlayerSupportMeander();

// Chasing Actions
void P_ActResurrectChase();
void P_ActStandardChase();
void P_ActWalkSoundChase();

// Attacking Actions
void P_ActComboAttack();
void P_ActMeleeAttack();
void P_ActRangeAttack();
void P_ActSpareAttack();
void P_ActRefireCheck();

// Miscellanous
void P_ActFaceTarget();
void P_ActMakeIntoCorpse();
void P_ActResetSpreadCount();

// Projectiles
void P_ActFixedHomingProjectile();
void P_ActRandomHomingProjectile();
void P_ActLaunchOrderedSpread();
void P_ActLaunchRandomSpread();
void P_ActCreateSmokeTrail();

// Trackers
void P_ActEffectTracker();
void P_ActTrackerActive();
void P_ActTrackerFollow();
void P_ActTrackerStart();

#endif // __DDF_LOCAL__

