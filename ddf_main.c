//
// DOSDoom Definition Files Code (main)
//
// By the DOSDoom Team
//

#include "d_debug.h"
#include "dm_state.h"
#include "i_system.h"
#include "lu_sound.h"
#include "m_argv.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "z_zone.h"

#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

void DDF_MainTransferExisting(void);
void DDF_MainCleanUpLooseEnds(void);

mobjinfo_t *specials[NUMMOBJSPEC];

backpack_t buffpack;
mapstuff_t buffermap;
mobjinfo_t buffermobj;
mobjinfo_t* bufferreplacemobj;
langref_t bufflangref;
int statecount;
char *stateinfo[NUMSPLIT+1];
state_t *tempstates[255];
boolean terminated;

// -KM- 1998/11/25 All weapon related sprites are out
char **sprnames;
int MAXSPRITES = 256;
static char *oldsprnames[] = {
    "TROO","SHTG",/*"PUNG","PISG","PISF","SHTF","SHT2","CHGG","CHGF","MISG",
    "MISF","SAWG","PLSG","PLSF","BFGG","BFGF",*/"BLUD","PUFF",/*"PLSS","PLSE",*/
    "MISL",/*"BFS1","BFE1","BFE2",*/"IFOG",/*"PLAY",*/"BBRN","BOSF","POL5","FIRE"
};

// -KM- 1998/11/25 All weapon related states are out
state_t* states;
int MAXSTATES = 1024;
static state_t	oldstates[] = {
    {SPR_TROO,0,-1,{NULL},S_NULL,0,0},	// S_NULL
    {SPR_SHTG,4,0,{A_Light0},S_NULL,0,0},	// S_LIGHTDONE

    //-------------------------------------
    //                FIST
    //-------------------------------------
/*
    {SPR_PUNG,0,1,{A_WeaponReady},S_PUNCH,0,0},	// S_PUNCH

    {SPR_PUNG,0,1,{A_Lower},S_PUNCHDOWN,0,0},	// S_PUNCHDOWN
    {SPR_PUNG,0,1,{A_Raise},S_PUNCHUP,0,0},	// S_PUNCHUP

    {SPR_PUNG,1,4,{NULL},S_PUNCH2,0,0},		// S_PUNCH1
    {SPR_PUNG,2,4,{A_Punch},S_PUNCH3,0,0},	// S_PUNCH2
    {SPR_PUNG,3,5,{NULL},S_PUNCH4,0,0},		// S_PUNCH3
    {SPR_PUNG,2,4,{NULL},S_PUNCH5,0,0},		// S_PUNCH4
    {SPR_PUNG,1,5,{A_ReFire},S_PUNCH,0,0},	// S_PUNCH5

    //-------------------------------------
    //             PISTOL
    //-------------------------------------
    {SPR_PISG,0,1,{A_WeaponReady},S_PISTOL,0,0},// S_PISTOL

    {SPR_PISG,0,1,{A_Lower},S_PISTOLDOWN,0,0},	// S_PISTOLDOWN
    {SPR_PISG,0,1,{A_Raise},S_PISTOLUP,0,0},	// S_PISTOLUP

    {SPR_PISG,0,4,{NULL},S_PISTOL2,0,0},	// S_PISTOL1
    {SPR_PISG,1,6,{A_FirePistol},S_PISTOL3,0,0},// S_PISTOL2
    {SPR_PISG,2,4,{NULL},S_PISTOL4,0,0},	// S_PISTOL3
    {SPR_PISG,1,5,{A_ReFire},S_PISTOL,0,0},	// S_PISTOL4

    {SPR_PISF,32768,7,{A_Light1},S_LIGHTDONE,0,0}, // S_PISTOLFLASH

    //-------------------------------------
    //          SHOTGUN
    //-------------------------------------
    {SPR_SHTG,0,1,{A_WeaponReady},S_SGUN,0,0},	// S_SGUN

    {SPR_SHTG,0,1,{A_Lower},S_SGUNDOWN,0,0},	// S_SGUNDOWN
    {SPR_SHTG,0,1,{A_Raise},S_SGUNUP,0,0},	// S_SGUNUP

    {SPR_SHTG,0,3,{NULL},S_SGUN2,0,0},	// S_SGUN1
    {SPR_SHTG,0,7,{A_FireShotgun},S_SGUN3,0,0},	// S_SGUN2
    {SPR_SHTG,1,5,{NULL},S_SGUN4,0,0},	// S_SGUN3
    {SPR_SHTG,2,5,{NULL},S_SGUN5,0,0},	// S_SGUN4
    {SPR_SHTG,3,4,{NULL},S_SGUN6,0,0},	// S_SGUN5
    {SPR_SHTG,2,5,{NULL},S_SGUN7,0,0},	// S_SGUN6
    {SPR_SHTG,1,5,{NULL},S_SGUN8,0,0},	// S_SGUN7
    {SPR_SHTG,0,3,{NULL},S_SGUN9,0,0},	// S_SGUN8
    {SPR_SHTG,0,7,{A_ReFire},S_SGUN,0,0},	// S_SGUN9

    {SPR_SHTF,32768,4,{A_Light1},S_SGUNFLASH2,0,0},	// S_SGUNFLASH1
    {SPR_SHTF,32769,3,{A_Light2},S_LIGHTDONE,0,0},	// S_SGUNFLASH2

    //-------------------------------------
    //           DOUBLE-BARREL
    //-------------------------------------
    {SPR_SHT2,0,1,{A_WeaponReady},S_DSGUN,0,0},	// S_DSGUN

    {SPR_SHT2,0,1,{A_Lower},S_DSGUNDOWN,0,0},	// S_DSGUNDOWN
    {SPR_SHT2,0,1,{A_Raise},S_DSGUNUP,0,0},	// S_DSGUNUP

    {SPR_SHT2,0,3,{NULL},S_DSGUN2,0,0},	// S_DSGUN1
    {SPR_SHT2,0,7,{A_FireShotgun2},S_DSGUN3,0,0},	// S_DSGUN2
    {SPR_SHT2,1,7,{NULL},S_DSGUN4,0,0},	// S_DSGUN3
    {SPR_SHT2,2,7,{A_CheckReload},S_DSGUN5,0,0},	// S_DSGUN4
    {SPR_SHT2,3,7,{A_OpenShotgun2},S_DSGUN6,0,0},	// S_DSGUN5
    {SPR_SHT2,4,7,{NULL},S_DSGUN7,0,0},	// S_DSGUN6
    {SPR_SHT2,5,7,{A_LoadShotgun2},S_DSGUN8,0,0},	// S_DSGUN7
    {SPR_SHT2,6,6,{NULL},S_DSGUN9,0,0},	// S_DSGUN8
    {SPR_SHT2,7,6,{A_CloseShotgun2},S_DSGUN10,0,0},	// S_DSGUN9
    {SPR_SHT2,0,5,{A_ReFire},S_DSGUN,0,0},	        // S_DSGUN10

    {SPR_SHT2,1,7,{NULL},S_DSNR2,0,0},	                // S_DSNR1
    {SPR_SHT2,0,3,{NULL},S_DSGUNDOWN,0,0},	        // S_DSNR2

    {SPR_SHT2,32776,5,{A_Light1},S_DSGUNFLASH2,0,0},	// S_DSGUNFLASH1
    {SPR_SHT2,32777,4,{A_Light2},S_LIGHTDONE,0,0},	// S_DSGUNFLASH2

    //-------------------------------------
    //           CHAINGUN
    //-------------------------------------
    {SPR_CHGG,0,1,{A_WeaponReady},S_CHAIN,0,0},	// S_CHAIN

    {SPR_CHGG,0,1,{A_Lower},S_CHAINDOWN,0,0},	// S_CHAINDOWN
    {SPR_CHGG,0,1,{A_Raise},S_CHAINUP,0,0},	// S_CHAINUP

    {SPR_CHGG,0,4,{A_FireCGun},S_CHAIN2,0,0},	// S_CHAIN1
    {SPR_CHGG,1,4,{A_FireCGun},S_CHAIN3,0,0},	// S_CHAIN2
    {SPR_CHGG,1,0,{A_ReFire},S_CHAIN,0,0},	// S_CHAIN3

    {SPR_CHGF,32768,5,{A_Light1},S_LIGHTDONE,0,0},	// S_CHAINFLASH1
    {SPR_CHGF,32769,5,{A_Light2},S_LIGHTDONE,0,0},	// S_CHAINFLASH2

    //-------------------------------------
    //           MISSILE LAUNCHER
    //-------------------------------------
    {SPR_MISG,0,1,{A_WeaponReady},S_MISSILE,0,0},	// S_MISSILE

    {SPR_MISG,0,1,{A_Lower},S_MISSILEDOWN,0,0},	// S_MISSILEDOWN
    {SPR_MISG,0,1,{A_Raise},S_MISSILEUP,0,0},	// S_MISSILEUP

    {SPR_MISG,1,8,{A_GunFlash},S_MISSILE2,0,0},	// S_MISSILE1
    {SPR_MISG,1,12,{A_FireMissile},S_MISSILE3,0,0},	// S_MISSILE2
    {SPR_MISG,1,0,{A_ReFire},S_MISSILE,0,0},	// S_MISSILE3

    {SPR_MISF,32768,3,{A_Light1},S_MISSILEFLASH2,0,0},	// S_MISSILEFLASH1
    {SPR_MISF,32769,4,{NULL},S_MISSILEFLASH3,0,0},	// S_MISSILEFLASH2
    {SPR_MISF,32770,4,{A_Light2},S_MISSILEFLASH4,0,0},	// S_MISSILEFLASH3
    {SPR_MISF,32771,4,{A_Light2},S_LIGHTDONE,0,0},	// S_MISSILEFLASH4

    //-------------------------------------
    //           CHAINSAW
    //-------------------------------------
    {SPR_SAWG,2,4,{A_WeaponReady},S_SAWB,0,0},	// S_SAW
    {SPR_SAWG,3,4,{A_WeaponReady},S_SAW,0,0},	// S_SAWB

    {SPR_SAWG,2,1,{A_Lower},S_SAWDOWN,0,0},	// S_SAWDOWN
    {SPR_SAWG,2,1,{A_Raise},S_SAWUP,0,0},	// S_SAWUP

    {SPR_SAWG,0,4,{A_Saw},S_SAW2,0,0},	// S_SAW1
    {SPR_SAWG,1,4,{A_Saw},S_SAW3,0,0},	// S_SAW2
    {SPR_SAWG,1,0,{A_ReFire},S_SAW,0,0},	// S_SAW3

    //-------------------------------------
    //           PLASMA-GUN
    //-------------------------------------
    {SPR_PLSG,0,1,{A_WeaponReady},S_PLASMA,0,0},	// S_PLASMA

    {SPR_PLSG,0,1,{A_Lower},S_PLASMADOWN,0,0},	// S_PLASMADOWN
    {SPR_PLSG,0,1,{A_Raise},S_PLASMAUP,0,0},	// S_PLASMAUP

    {SPR_PLSG,0,3,{A_FirePlasma},S_PLASMA2,0,0}, // S_PLASMA1
    {SPR_PLSG,1,20,{A_ReFire},S_PLASMA,0,0},	 // S_PLASMA2

    {SPR_PLSF,32768,4,{A_Light1},S_LIGHTDONE,0,0},	// S_PLASMAFLASH1
    {SPR_PLSF,32769,4,{A_Light1},S_LIGHTDONE,0,0},	// S_PLASMAFLASH2

    //-------------------------------------
    //           BFG9000
    //-------------------------------------
    {SPR_BFGG,0,1,{A_WeaponReady},S_BFG,0,0},	// S_BFG

    {SPR_BFGG,0,1,{A_Lower},S_BFGDOWN,0,0},	// S_BFGDOWN
    {SPR_BFGG,0,1,{A_Raise},S_BFGUP,0,0},	// S_BFGUP

    {SPR_BFGG,0,20,{A_BFGsound},S_BFG2,0,0},	// S_BFG1
    {SPR_BFGG,1,10,{A_GunFlash},S_BFG3,0,0},	// S_BFG2
    {SPR_BFGG,1,10,{A_FireBFG},S_BFG4,0,0},	// S_BFG3
    {SPR_BFGG,1,20,{A_ReFire},S_BFG,0,0},	// S_BFG4

    {SPR_BFGF,32768,11,{A_Light1},S_BFGFLASH2,0,0},	// S_BFGFLASH1
    {SPR_BFGF,32769,6,{A_Light2},S_LIGHTDONE,0,0},	// S_BFGFLASH2
*/

    {SPR_BLUD,2,8,{NULL},S_BLOOD2,0,0},	// S_BLOOD1
    {SPR_BLUD,1,8,{NULL},S_BLOOD3,0,0},	// S_BLOOD2
    {SPR_BLUD,0,8,{NULL},S_NULL,0,0},	// S_BLOOD3

    {SPR_PUFF,32768,4,{NULL},S_PUFF2,0,0},	// S_PUFF1
    {SPR_PUFF,1,4,{NULL},S_PUFF3,0,0},	// S_PUFF2
    {SPR_PUFF,2,4,{NULL},S_PUFF4,0,0},	// S_PUFF3
    {SPR_PUFF,3,4,{NULL},S_NULL,0,0},	// S_PUFF4

    // ------------------------------------------------------
/*
    {SPR_PLSS,32768,6,{NULL},S_PLASBALL2,0,0},	// S_PLASBALL
    {SPR_PLSS,32769,6,{NULL},S_PLASBALL,0,0},	// S_PLASBALL2
    {SPR_PLSE,32768,4,{NULL},S_PLASEXP2,0,0},	// S_PLASEXP
    {SPR_PLSE,32769,4,{NULL},S_PLASEXP3,0,0},	// S_PLASEXP2
    {SPR_PLSE,32770,4,{NULL},S_PLASEXP4,0,0},	// S_PLASEXP3
    {SPR_PLSE,32771,4,{NULL},S_PLASEXP5,0,0},	// S_PLASEXP4
    {SPR_PLSE,32772,4,{NULL},S_NULL,0,0},	// S_PLASEXP5 */
    {SPR_MISL,32768,1,{NULL},S_ROCKET,0,0},	// S_ROCKET
/*    {SPR_BFS1,32768,4,{NULL},S_BFGSHOT2,0,0},	// S_BFGSHOT
    {SPR_BFS1,32769,4,{NULL},S_BFGSHOT,0,0},	// S_BFGSHOT2
    {SPR_BFE1,32768,8,{NULL},S_BFGLAND2,0,0},	// S_BFGLAND
    {SPR_BFE1,32769,8,{NULL},S_BFGLAND3,0,0},	// S_BFGLAND2
    {SPR_BFE1,32770,8,{A_BFGSpray},S_BFGLAND4,0,0},	// S_BFGLAND3
    {SPR_BFE1,32771,8,{NULL},S_BFGLAND5,0,0},	// S_BFGLAND4
    {SPR_BFE1,32772,8,{NULL},S_BFGLAND6,0,0},	// S_BFGLAND5
    {SPR_BFE1,32773,8,{NULL},S_NULL,0,0},	// S_BFGLAND6
    {SPR_BFE2,32768,8,{NULL},S_BFGEXP2,0,0},	// S_BFGEXP
    {SPR_BFE2,32769,8,{NULL},S_BFGEXP3,0,0},	// S_BFGEXP2
    {SPR_BFE2,32770,8,{NULL},S_BFGEXP4,0,0},	// S_BFGEXP3
    {SPR_BFE2,32771,8,{NULL},S_NULL,0,0},	// S_BFGEXP4 */
    {SPR_MISL,32769,8,{A_Explode},S_EXPLODE2,0,0},	// S_EXPLODE1
    {SPR_MISL,32770,6,{NULL},S_EXPLODE3,0,0},	// S_EXPLODE2
    {SPR_MISL,32771,4,{NULL},S_NULL,0,0},	// S_EXPLODE3
    {SPR_IFOG,32768,6,{NULL},S_IFOG01,0,0},	// S_IFOG
    {SPR_IFOG,32769,6,{NULL},S_IFOG02,0,0},	// S_IFOG01
    {SPR_IFOG,32768,6,{NULL},S_IFOG2,0,0},	// S_IFOG02
    {SPR_IFOG,32769,6,{NULL},S_IFOG3,0,0},	// S_IFOG2
    {SPR_IFOG,32770,6,{NULL},S_IFOG4,0,0},	// S_IFOG3
    {SPR_IFOG,32771,6,{NULL},S_IFOG5,0,0},	// S_IFOG4
    {SPR_IFOG,32772,6,{NULL},S_NULL,0,0},	// S_IFOG5
/*    {SPR_PLAY,0,-1,{NULL},S_NULL,0,0},	// S_PLAY
    {SPR_PLAY,0,4,{NULL},S_PLAY_RUN2,0,0},	// S_PLAY_RUN1
    {SPR_PLAY,1,4,{NULL},S_PLAY_RUN3,0,0},	// S_PLAY_RUN2
    {SPR_PLAY,2,4,{NULL},S_PLAY_RUN4,0,0},	// S_PLAY_RUN3
    {SPR_PLAY,3,4,{NULL},S_PLAY_RUN1,0,0},	// S_PLAY_RUN4
    {SPR_PLAY,4,12,{NULL},S_PLAY,0,0},	// S_PLAY_ATK1
    {SPR_PLAY,32773,6,{NULL},S_PLAY_ATK1,0,0},	// S_PLAY_ATK2
    {SPR_PLAY,6,4,{NULL},S_PLAY_PAIN2,0,0},	// S_PLAY_PAIN
    {SPR_PLAY,6,4,{A_Pain},S_PLAY,0,0},	// S_PLAY_PAIN2
    {SPR_PLAY,7,10,{NULL},S_PLAY_DIE2,0,0},	// S_PLAY_DIE1
    {SPR_PLAY,8,10,{A_PlayerScream},S_PLAY_DIE3,0,0},	// S_PLAY_DIE2
    {SPR_PLAY,9,10,{A_Fall},S_PLAY_DIE4,0,0},	// S_PLAY_DIE3
    {SPR_PLAY,10,10,{NULL},S_PLAY_DIE5,0,0},	// S_PLAY_DIE4
    {SPR_PLAY,11,10,{NULL},S_PLAY_DIE6,0,0},	// S_PLAY_DIE5
    {SPR_PLAY,12,10,{NULL},S_PLAY_DIE7,0,0},	// S_PLAY_DIE6
    {SPR_PLAY,13,-1,{NULL},S_NULL,0,0},	// S_PLAY_DIE7
    {SPR_PLAY,14,5,{NULL},S_PLAY_XDIE2,0,0},	// S_PLAY_XDIE1
    {SPR_PLAY,15,5,{A_XScream},S_PLAY_XDIE3,0,0},	// S_PLAY_XDIE2
    {SPR_PLAY,16,5,{A_Fall},S_PLAY_XDIE4,0,0},	// S_PLAY_XDIE3
    {SPR_PLAY,17,5,{NULL},S_PLAY_XDIE5,0,0},	// S_PLAY_XDIE4
    {SPR_PLAY,18,5,{NULL},S_PLAY_XDIE6,0,0},	// S_PLAY_XDIE5
    {SPR_PLAY,19,5,{NULL},S_PLAY_XDIE7,0,0},	// S_PLAY_XDIE6
    {SPR_PLAY,20,5,{NULL},S_PLAY_XDIE8,0,0},	// S_PLAY_XDIE7
    {SPR_PLAY,21,5,{NULL},S_PLAY_XDIE9,0,0},	// S_PLAY_XDIE8
    {SPR_PLAY,22,-1,{NULL},S_NULL,0,0},	// S_PLAY_XDIE9
  */
    // ----------------------------------------------------------

    {SPR_PUFF,1,4,{NULL},S_SMOKE2,0,0},	// S_SMOKE1
    {SPR_PUFF,2,4,{NULL},S_SMOKE3,0,0},	// S_SMOKE2
    {SPR_PUFF,1,4,{NULL},S_SMOKE4,0,0},	// S_SMOKE3
    {SPR_PUFF,2,4,{NULL},S_SMOKE5,0,0},	// S_SMOKE4
    {SPR_PUFF,3,4,{NULL},S_NULL,0,0},	// S_SMOKE5

    // ----------------------------------------------------------

    {SPR_BBRN,0,-1,{NULL},S_NULL,0,0},		// S_BRAIN
    {SPR_BBRN,1,36,{A_BrainPain},S_BRAIN,0,0},	// S_BRAIN_PAIN
    {SPR_BBRN,0,100,{A_BrainScream},S_BRAIN_DIE2,0,0},	// S_BRAIN_DIE1
    {SPR_BBRN,0,10,{NULL},S_BRAIN_DIE3,0,0},	// S_BRAIN_DIE2
    {SPR_BBRN,0,10,{NULL},S_BRAIN_DIE4,0,0},	// S_BRAIN_DIE3
    {SPR_BBRN,0,-1,{A_BrainDie},S_NULL,0,0},	// S_BRAIN_DIE4

    {SPR_TROO,0,10,{P_ActStandardLook},S_BRAINEYE,0,0},	// S_BRAINEYE
    {SPR_TROO,0,181,{A_BrainAwake},S_BRAINEYE1,0,0},	// S_BRAINEYESEE
    {SPR_TROO,0,150,{A_BrainSpit},S_BRAINEYE1,0,0},	// S_BRAINEYE1

    {SPR_BOSF,32768,3,{A_SpawnSound},S_SPAWN2,0,0},	// S_SPAWN1
    {SPR_BOSF,32769,3,{A_SpawnFly},S_SPAWN3,0,0},	// S_SPAWN2
    {SPR_BOSF,32770,3,{A_SpawnFly},S_SPAWN4,0,0},	// S_SPAWN3
    {SPR_BOSF,32771,3,{A_SpawnFly},S_SPAWN1,0,0},	// S_SPAWN4
    {SPR_FIRE,32768,4,{P_ActTrackerFollow},S_SPAWNFIRE2,0,0},	// S_SPAWNFIRE1
    {SPR_FIRE,32769,4,{P_ActTrackerFollow},S_SPAWNFIRE3,0,0},	// S_SPAWNFIRE2
    {SPR_FIRE,32770,4,{P_ActTrackerFollow},S_SPAWNFIRE4,0,0},	// S_SPAWNFIRE3
    {SPR_FIRE,32771,4,{P_ActTrackerFollow},S_SPAWNFIRE5,0,0},	// S_SPAWNFIRE4
    {SPR_FIRE,32772,4,{P_ActTrackerFollow},S_SPAWNFIRE6,0,0},	// S_SPAWNFIRE5
    {SPR_FIRE,32773,4,{P_ActTrackerFollow},S_SPAWNFIRE7,0,0},	// S_SPAWNFIRE6
    {SPR_FIRE,32774,4,{P_ActTrackerFollow},S_SPAWNFIRE8,0,0},	// S_SPAWNFIRE7
    {SPR_FIRE,32775,4,{P_ActTrackerFollow},S_NULL,0,0},		// S_SPAWNFIRE8

    {SPR_MISL,32769,10,{NULL},S_BRAINEXPLODE2,0,0},	// S_BRAINEXPLODE1
    {SPR_MISL,32770,10,{NULL},S_BRAINEXPLODE3,0,0},	// S_BRAINEXPLODE2
    {SPR_MISL,32771,10,{A_BrainExplode},S_NULL,0,0},	// S_BRAINEXPLODE3

    {SPR_POL5,0,-1,{NULL},S_NULL,0,0}// S_GIBS - Crunched remains, keep -ACB-
    
};

// -KM- 1998/09/27 Change of SFX from enum to String
// -KM- 1998/10/29 String is converted to sfx_t at runtime. Yuk!
// -KM- 1998/11/25 All Weapon related things are out. Default visibilities are in.
mobjinfo_t oldmobjinfo[ORIG_NUMMOBJTYPES] = {
/*
    {		// MT_PLAYER
	-1,		// doomednum
	S_PLAY,		// spawnstate
	100,		// spawnhealth
	S_PLAY_RUN1,		// seestate
	sfx_None,		// seesound
	0,		// reactiontime
	sfx_None,		// attacksound
	S_PLAY_PAIN,		// painstate
	255,		// painchance
	(sfx_t *) "PLPAIN",		// painsound
	S_PLAY_ATK1,		// meleestate
	S_PLAY_ATK1,		// missilestate
	S_PLAY_DIE1,		// deathstate
	S_PLAY_XDIE1,		// xdeathstate
	(sfx_t *) "PLDETH",		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	56*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
        MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,		// flags
	S_NULL,		// raisestate
//        castorder: 1, // castorder
        name: "The Hero",
        invisibility: VISIBLE
    },            */

    {		// MT_SMOKE
	-1,		// doomednum
	S_SMOKE1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_BOSSBRAIN
	88,		// doomednum
	S_BRAIN,		// spawnstate
	250,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_BRAIN_PAIN,		// painstate
	255,		// painchance
	(sfx_t *) "BOSPN",		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BRAIN_DIE1,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "BOSDTH",		// deathsound
	0,		// speed
	16*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	10000000,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_SOLID|MF_SHOOTABLE,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_BOSSSPIT
	89,		// doomednum
	S_BRAINEYE,		// spawnstate
	1000,		// spawnhealth
	S_BRAINEYESEE,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_BOSSTARGET
	87,		// doomednum
	S_NULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_SPAWNSHOT
	-1,		// doomednum
	S_SPAWN1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	(sfx_t *) "BOSPIT",		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "FIRXPL",		// deathsound
	10*FRACUNIT,		// speed
	6*FRACUNIT,		// radius
	32*FRACUNIT,		// height
	100,		// mass
	3,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY|MF_NOCLIP,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_SPAWNFIRE
	-1,		// doomednum
	S_SPAWNFIRE1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_ROCKET
	-1,		// doomednum
	S_ROCKET,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	(sfx_t *) "RLAUNC",		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_EXPLODE1,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "BAREXP",		// deathsound
	20*FRACUNIT,		// speed
	11*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	20,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },
/*
    {		// MT_PLASMA
	-1,		// doomednum
	S_PLASBALL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	(sfx_t *) "PLASMA",		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_PLASEXP,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "FIRXPL",		// deathsound
	25*FRACUNIT,		// speed
	13*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	5,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE
    },

    {		// MT_BFG
	-1,		// doomednum
	S_BFGSHOT,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_BFGLAND,		// deathstate
	S_NULL,		// xdeathstate
	(sfx_t *) "RXPLOD",		// deathsound
	25*FRACUNIT,		// speed
	13*FRACUNIT,		// radius
	8*FRACUNIT,		// height
	100,		// mass
	100,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE
    },
  */
    {		// MT_PUFF
	-1,		// doomednum
	S_PUFF1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOCLIP|MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: (2*VISIBLE)/3,
        fast: FRACUNIT
    },

    {		// MT_BLOOD
	-1,		// doomednum
	S_BLOOD1,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	FRACUNIT,		// radius 20
	FRACUNIT,		// height 15
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
        MF_CORPSE|MF_DROPOFF,             // flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_IFOG
	-1,		// doomednum
	S_IFOG,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },

    {		// MT_TELEPORTMAN
	14,		// doomednum
	S_NULL,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOSECTOR,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE,
        fast: FRACUNIT
    },
/*
    {		// MT_EXTRABFG
	-1,		// doomednum
	S_BFGEXP,		// spawnstate
	1000,		// spawnhealth
	S_NULL,		// seestate
	sfx_None,		// seesound
	8,		// reactiontime
	sfx_None,		// attacksound
	S_NULL,		// painstate
	0,		// painchance
	sfx_None,		// painsound
	S_NULL,		// meleestate
	S_NULL,		// missilestate
	S_NULL,		// deathstate
	S_NULL,		// xdeathstate
	sfx_None,		// deathsound
	0,		// speed
	20*FRACUNIT,		// radius
	16*FRACUNIT,		// height
	100,		// mass
	0,		// damage
	sfx_None,		// activesound
        NULL,                   // enemyattackmissile        
	MF_NOBLOCKMAP|MF_NOGRAVITY,		// flags
	S_NULL,		// raisestate
        invisibility: VISIBLE
    }*/
};

framedest_t framedestlist[] =
                   {{ "SPAWN",     &buffermobj.spawnstate,   FDF_SPAWN     },
                    { "CHASE",     &buffermobj.seestate,     FDF_CHASE     },
                    { "MELEE",     &buffermobj.meleestate,   FDF_MELEE     },
                    { "MISSILE",   &buffermobj.missilestate, FDF_MISSILE   },
                    { "PAIN",      &buffermobj.painstate,    FDF_PAIN      },
                    { "DEATH",     &buffermobj.deathstate,   FDF_DEATH     },
                    { "XDEATH",    &buffermobj.xdeathstate,  FDF_XDEATH    },
                    { "RESPAWN",   &buffermobj.raisestate,   FDF_RESPAWN   },
                    { "RESURRECT", &buffermobj.resstate,     FDF_RESURRECT },
                    { "MEANDER",   &buffermobj.meanderstate, FDF_MEANDER   },
                    { COMMAND_TERMINATOR, NULL, 0 }};

// -KM- 1998/11/25 Translucency to fractional.
// -KM- 1998/12/16 Added individual flags for all.
specials_t specialslist[] =
                   {{ "FUZZY"         ,MF_SHADOW      ,0           ,0          },
                    { "SOLID"         ,MF_SOLID       ,0           ,0          },
                    { "ON CEILING"    ,MF_SPAWNCEILING+MF_NOGRAVITY,0,0        },
                    { "FLOATER"       ,MF_FLOAT+MF_NOGRAVITY,0     ,0          },
                    { "INERT"         ,MF_NOBLOCKMAP  ,0           ,0          },
                    { "TELEPORT TYPE" ,MF_NOGRAVITY   ,0           ,0          },
                    { "NO LINKS"      ,MF_NOBLOCKMAP+MF_NOSECTOR,0 ,0          },
                    { "DAMAGESMOKE"   ,MF_NOBLOOD     ,0           ,0          },
                    { "SHOOTABLE"     ,MF_SHOOTABLE   ,0           ,0          },
                    { "COUNT AS KILL" ,MF_COUNTKILL   ,0           ,0          },
                    { "COUNT AS ITEM" ,MF_COUNTITEM   ,0           ,0          },
                    { "SPECIAL"       ,MF_SPECIAL     ,0           ,0          },
                    { "NOSECTOR"      ,MF_NOSECTOR    ,0           ,0          },
                    { "NOBLOCKMAP"    ,MF_NOBLOCKMAP  ,0           ,0          },
                    { "SPAWNCEILING"  ,MF_SPAWNCEILING,0           ,0          },
                    { "NOGRAVITY"     ,MF_NOGRAVITY   ,0           ,0          },
                    { "DROPOFF"       ,MF_DROPOFF     ,0           ,0          },
                    { "PICKUP"        ,MF_PICKUP      ,0           ,0          },
                    { "NOCLIP"        ,MF_NOCLIP      ,0           ,0          },
                    { "SLIDER"        ,MF_SLIDE       ,0           ,0          },
                    { "FLOAT"         ,MF_FLOAT       ,0           ,0          },
                    { "TELEPORT"      ,MF_TELEPORT    ,0           ,0          },
                    { "MISSILE"       ,MF_MISSILE     ,0           ,0          },
                    { "DROPPED"       ,MF_DROPPED     ,0           ,0          },
                    { "CORPSE"        ,MF_CORPSE      ,0           ,0          },
                    { "STEALTH"       ,MF_STEALTH     ,0           ,0          },
                    { "NODEATHMATCH"  ,MF_NOTDMATCH   ,0           ,0          },
                    { "NO RESPAWN"    ,0              ,EF_NORESPAWN,0          },
                    { "NO RESURRECT"  ,0              ,EF_NORESURRECT,0        },
                    { "DISLOYAL"      ,0              ,EF_DISLOYALTYPE,0       },
                    { "TRIGGER HAPPY" ,0              ,EF_TRIGGERHAPPY,0       },
                    { "ATTACK HURTS"  ,0              ,EF_OWNATTACKHURTS,0     },
                    { "BOSSMAN"       ,0              ,EF_BOSSMAN,0            },
                    { "NEVERTARGETED" ,0              ,EF_NEVERTARGET,0        },
                    { "NOGRAV KILL"   ,0              ,EF_NOGRAVKILL,0         },
                    { "NO GRUDGE"     ,0              ,EF_NOGRUDGE,0           },
                    { "INVISIBLE"     ,0              ,0           ,INVISIBLE  },
                    { COMMAND_TERMINATOR,0,0,0 } };

// -KM- 1998/11/25 Added weapon functions.
actioncode_t actions[] = {{ "ALTERTRANSLUC"    , {P_ActAlterTransluc}           },
                          { "ALTERVISIBILITY"  , {P_ActAlterVisibility}         },
                          { "LESSVISIBLE"      , {P_ActBecomeLessVisible}       },
                          { "MOREVISIBLE"      , {P_ActBecomeMoreVisible}       },
                          { "CLOSEATTEMPTSND"  , {P_ActMakeCloseAttemptSound}   },
                          { "COMBOATTACK"      , {P_ActComboAttack}             },
                          { "FACETARGET"       , {P_ActFaceTarget}              },
                          { "MAKESOUND"        , {P_ActMakeAmbientSound}        },
                          { "MAKESOUNDRANDOM"  , {P_ActMakeAmbientSoundRandom}  },
                          { "MAKEDEATHSOUND"   , {P_ActMakeDyingSound}          },
                          { "MAKEDEAD"         , {P_ActMakeIntoCorpse}          },
                          { "MAKEOVERKILLSOUND", {P_ActMakeOverKillSound}       },
                          { "MAKEPAINSOUND"    , {P_ActMakePainSound}           },
                          { "CLOSE COMBAT"     , {P_ActMeleeAttack}             },
                          { "RANGE ATTACK"     , {P_ActRangeAttack}             },
                          { "SPARE ATTACK"     , {P_ActSpareAttack}             },
                          { "RANGEATTEMPTSND"  , {P_ActMakeRangeAttemptSound}   },
                          { "REFIRE CHECK"     , {P_ActRefireCheck}             },
                          { "LOOKOUT"          , {P_ActStandardLook}            },
                          { "SUPPORT LOOKOUT"  , {P_ActPlayerSupportLook}       },
                          { "CHASE"            , {P_ActStandardChase}           },
                          { "RESCHASE"         , {P_ActResurrectChase}          },
                          { "WALKSOUND CHASE"  , {P_ActWalkSoundChase}          },
                          { "MEANDER"          , {P_ActStandardMeander}         },
                          { "SUPPORT MEANDER"  , {P_ActPlayerSupportMeander}    },
                          { "EXPLOSIONDAMAGE"  , {P_ActSetDamageExplosion}      },
                          { "VARIEDEXPDAMAGE"  , {P_ActVaryingDamageExplosion}  },
                          { "TRACER"           , {P_ActFixedHomingProjectile}   },
                          { "RANDOM TRACER"    , {P_ActRandomHomingProjectile}  },
                          { "RESET SPREADER"   , {P_ActResetSpreadCount}        },
                          { "SMOKING"          , {P_ActCreateSmokeTrail}        },
                          { "TRACKERACTIVE"    , {P_ActTrackerActive}           },
                          { "TRACKERFOLLOW"    , {P_ActTrackerFollow}           },
                          { "TRACKERSTART"     , {P_ActTrackerStart}            },
                          { "EFFECTTRACKER"    , {P_ActEffectTracker}           },
                          { "WEAPON RAISE"     , {A_Raise}                      },
                          { "WEAPON LOWER"     , {A_Lower}                      },
                          { "WEAPON READY"     , {A_WeaponReady}                },
                          { "WEAPON SHOOT"     , {A_FireWeapon}                 },
                          { "WEAPON REFIRE"    , {A_ReFire}                     },
                          { "WEAPON LIGHT0"    , {A_Light0}                     },
                          { "WEAPON LIGHT1"    , {A_Light1}                     },
                          { "WEAPON LIGHT2"    , {A_Light2}                     },
                          { "WEAPON CHECKRELOAD",{A_CheckReload}               },
                          { "WEAPON FLASH"     , {A_GunFlash}                  },
                          { "WEAPON SOUND1"      , {A_SFXWeapon1}                },
                          { "WEAPON SOUND2"    , {A_SFXWeapon2}              },
                          { "WEAPON SOUND3"     , {A_SFXWeapon3}               },
                          { "NOTHING"          , {NULL}                         },
                          { COMMAND_TERMINATOR , {NULL}                         }};

//
// PSPRITE ACTIONS for weapons.
// This struct controls the weapon animations.
//
// -ACB- 1998/06/25 Put this in logical order.
//
// -KM- 1998/11/25 Externalised in DDF.
/*
weaponinfo_t	weaponinfo[NUMWEAPONS] =
{
    // TYPE 0 - fist
    { am_noammo, S_PUNCHUP, S_PUNCHDOWN, S_PUNCH, S_PUNCH1, S_NULL, NULL },

    // TYPE 1 - pistol
    { am_clip, S_PISTOLUP, S_PISTOLDOWN, S_PISTOL, S_PISTOL1, S_PISTOLFLASH, NULL },

    // TYPE 2 - shotgun
    { am_shell, S_SGUNUP, S_SGUNDOWN, S_SGUN, S_SGUN1, S_SGUNFLASH1, NULL },

    // TYPE 3 - super shotgun
    { am_shell, S_DSGUNUP, S_DSGUNDOWN, S_DSGUN, S_DSGUN1, S_DSGUNFLASH1, NULL },

    // TYPE 4 - chaingun
    { am_clip, S_CHAINUP, S_CHAINDOWN, S_CHAIN, S_CHAIN1, S_CHAINFLASH1, NULL },

    // TYPE 5 - missile launcher
    { am_misl, S_MISSILEUP,S_MISSILEDOWN,S_MISSILE,S_MISSILE1,S_MISSILEFLASH1, NULL},

    // TYPE 6 - plasma rifle
    { am_cell, S_PLASMAUP, S_PLASMADOWN, S_PLASMA, S_PLASMA1, S_PLASMAFLASH1, NULL },

    // TYPE 7 - bfg 9000
    { am_cell, S_BFGUP, S_BFGDOWN, S_BFG, S_BFG1, S_BFGFLASH1, NULL },

    // TYPE 8 - chainsaw
    { am_noammo, S_SAWUP, S_SAWDOWN, S_SAW, S_SAW1, S_NULL, NULL }
};
*/
int NUMMOBJTYPES;
int NUMSTATES;
int NUMSPRITES;

backpack_t* backpackhead;
commandlist_t* currentcmdlist;
// -KM- 1998/11/25 Always 10 weapon keys, 1 - 0
weaponkey_t weaponkey[10];
mobjinfo_t* mobjinfohead;
mapstuff_t* maphead = NULL;
boolean ddf_replace = false;

void DDF_MainInit()
{
  I_Printf("Doom Definition v%i.%02i\n",DDFVERSION/100,DDFVERSION%100);

  NUMMOBJTYPES = ORIG_NUMMOBJTYPES;
  NUMSTATES    = ORIG_NUMSTATES;
  NUMSPRITES   = ORIG_NUMSPRITES;

  DDF_LanguageInit();

  // Other DDF files depend on this, it goes first.
  DDF_SFXInit();

  // -KM- 1998/10/29 Depends on SFX
  DDF_MainTransferExisting();

  DDF_AttackInit();    // Attack types
  // -KM- 1998/11/25 Items.ddf must be loaded after weapons.ddf
  DDF_WeaponInit();    // Initialise Player Weapon Types

  DDF_MobjItemInit();      //  }
  DDF_MobjSceneInit();     //   } - Initialise the moving objects (mobj's)
  DDF_MobjCreatureInit();  //  }


  DDF_LinedefInit();   // Map stuff: Linedefs
  DDF_SectorInit();    //            Sectors
  DDF_SWInit();        //            Switches
  DDF_AnimInit();      //            Animations.

  DDF_GameInit();      // ...and then the overall game.
  DDF_LevelInit();     // Sort out the levels....

  DDF_OldThingInit();  // temp function

//  DDF_MainCleanUpLooseEnds();
  ddf_replace = true;

#ifdef DEVELOPERS
  I_Printf("DDF Finished, Press a key:");
  getch();
#endif
}

// Temp procedure to transfer data to a linked list.
void DDF_MainTransferExisting()
{
 int i;

 mobjinfohead = &oldmobjinfo[0];

 for (i=0; i<(ORIG_NUMMOBJTYPES-1); i++)
 {
    // -KM- 1998/10/29 convert Strings to sfx
    oldmobjinfo[i].seesound = DDF_LookupSound((char *) oldmobjinfo[i].seesound);
    oldmobjinfo[i].attacksound = DDF_LookupSound((char *) oldmobjinfo[i].attacksound);
    oldmobjinfo[i].painsound = DDF_LookupSound((char *) oldmobjinfo[i].painsound);
    oldmobjinfo[i].deathsound = DDF_LookupSound((char *) oldmobjinfo[i].deathsound);
    oldmobjinfo[i].activesound = DDF_LookupSound((char *) oldmobjinfo[i].activesound);
    oldmobjinfo[i].walksound = DDF_LookupSound((char *) oldmobjinfo[i].walksound);

    oldmobjinfo[i].next = &oldmobjinfo[i+1];
 }
 // Do the last mobj
 oldmobjinfo[i].seesound = DDF_LookupSound((char *) oldmobjinfo[i].seesound);
 oldmobjinfo[i].attacksound = DDF_LookupSound((char *) oldmobjinfo[i].attacksound);
 oldmobjinfo[i].painsound = DDF_LookupSound((char *) oldmobjinfo[i].painsound);
 oldmobjinfo[i].deathsound = DDF_LookupSound((char *) oldmobjinfo[i].deathsound);
 oldmobjinfo[i].activesound = DDF_LookupSound((char *) oldmobjinfo[i].activesound);
 oldmobjinfo[i].walksound = DDF_LookupSound((char *) oldmobjinfo[i].walksound);

 sprnames = malloc(sizeof(*sprnames) * MAXSPRITES);
 for (i = 0; oldsprnames[i]; i++)
    sprnames[i] = oldsprnames[i];

 states = malloc(sizeof(*states) * MAXSTATES);
 memcpy(states, oldstates, sizeof(oldstates));
}

typedef struct
{
  char* name;
  char* value;
} define_t;

define_t* defines = NULL;
int       numDefines = 0;

void DDF_MainAddDefine(char* name, char* value)
{
   int i;
   for (i = 0; i < numDefines; i++)
   {
      if (!strcmp(defines[i].name, name))
        I_Error("\nRedefinition of '%s'\n", name);
   }

   defines = realloc(defines, (numDefines+1)*sizeof(define_t));
   defines[numDefines].name = name;
   defines[numDefines].value = value;
}

char* DDF_MainGetDefine(char* name)
{
  int i;
  for (i = 0; i < numDefines; i++)
     if (!strcmp(defines[i].name, name))
       return defines[i].value;

  // Not a define.
  return name;
}

void DDF_MainCleanUpLooseEnds()
{
  int i, j;
  attacktype_t* curratk;
  mobjinfo_t* currmobj;
  boolean continueflag;

  char* name;

  // ------------
  // ATTACKS
  // ------------
  curratk = attackhead;

  while (curratk != NULL)
  {
    if (curratk->attackstyle==ATK_SPAWNER || curratk->attackstyle==ATK_TRIPLESPAWNER)
    {
      name = (char *) curratk->projectile;

      if (!name)
        I_Error("DDF_MainCleanUp: No spawn object given for %s", curratk->name);

      curratk->projectile = DDF_MobjLookup(name);

      // This is bloody awful.....
      if (curratk->objinitstate & FDF_SPAWN)
        curratk->objinitstate -= (FDF_SPAWN - curratk->projectile->spawnstate);
      else if (curratk->objinitstate & FDF_CHASE)
        curratk->objinitstate -= (FDF_CHASE - curratk->projectile->seestate);
      else if (curratk->objinitstate & FDF_MELEE)
        curratk->objinitstate -= (FDF_MELEE - curratk->projectile->meleestate);
      else if (curratk->objinitstate & FDF_MISSILE)
        curratk->objinitstate -= (FDF_MISSILE - curratk->projectile->missilestate);
      else if (curratk->objinitstate & FDF_PAIN)
        curratk->objinitstate -= (FDF_PAIN - curratk->projectile->painstate);
      else if (curratk->objinitstate & FDF_DEATH)
        curratk->objinitstate -= (FDF_DEATH - curratk->projectile->deathstate);
      else if (curratk->objinitstate & FDF_XDEATH)
        curratk->objinitstate -= (FDF_XDEATH - curratk->projectile->xdeathstate);
      else if (curratk->objinitstate & FDF_RESPAWN)
        curratk->objinitstate -= (FDF_RESPAWN - curratk->projectile->raisestate);
      else if (curratk->objinitstate & FDF_RESURRECT)
        curratk->objinitstate -= (FDF_RESURRECT - curratk->projectile->resstate);
      else if (curratk->objinitstate & FDF_MEANDER) // -ACB- 1998/09/05 Meander stuff
        curratk->objinitstate -= (FDF_MEANDER - curratk->projectile->meanderstate);
      else
        curratk->objinitstate = curratk->projectile->spawnstate;
    }

    curratk = curratk->next;
  }

  // ------------
  // MOBJ TYPES
  // ------------
  currmobj = mobjinfohead;

  name = NULL;

  while (currmobj != NULL)
  {
    name = (char *) currmobj->dropitem;

    if (!name)
    {
      currmobj = currmobj->next;
      continue;
    }

    currmobj->dropitem = DDF_MobjLookup(name);
    currmobj = currmobj->next;

  }

  // ------------
  // LINES
  // ------------
  //
  // Yes, this is ACB's first attempt at a hash table search: come back
  // linked-list! all is forgiven!.
  //
  // -ACB- 1998/09/13 Search through and replace name with object info
  // pointer (teleport effects objects).
  //
  for (i=0; i<256; i++)
  {
    if (specialLineDefs[i])
    {
      j=0;
      continueflag = false;

      while (!continueflag)
      {
        if (specialLineDefs[i][j].t.teleport &&
             (char*)specialLineDefs[i][j].t.inspawnobj != NULL)
        {
          specialLineDefs[i][j].t.inspawnobj =
            DDF_MobjLookup((char *) specialLineDefs[i][j].t.inspawnobj);
        }

        if (specialLineDefs[i][j].t.teleport &&
             (char*)specialLineDefs[i][j].t.outspawnobj != NULL)
        {
          specialLineDefs[i][j].t.outspawnobj =
            DDF_MobjLookup((char *) specialLineDefs[i][j].t.outspawnobj);
        }

        if (!specialLineDefs[i][j].next)
          continueflag = true;

        j++;
      }
    }
  }
  sprnames[NUMSPRITES] = NULL;
}

// -KM- 1998/12/16 This loads the ddf file into memory for parsing.
void* DDF_MainCacheFile(readinfo_t* readinfo)
{
  FILE* file;
  void *memfile;
  char *filename;
  int p;
  size_t size;

  if (!readinfo->filename)
    I_Error("DDF_MainReadFile: No file to read");

  p = M_CheckParm("-ddf");
  if (p && p < myargc-1)
  {
    filename = alloca(strlen(myargv[p+1]) + 2 + strlen(readinfo->filename));
    sprintf(filename, "%s/%s", myargv[p+1], readinfo->filename);
  } else
    filename = readinfo->filename;

  if ((file = fopen( filename,"rb" ))==NULL)
    I_Error("DDF_MainReadFile: Unable to open: '%s'",filename);

#ifdef DEVELOPERS
  File_Printf("\nDDF Parser Output:\n");
#endif

  fseek(file,0,SEEK_END);                         // get to the end of the file
  size = ftell(file);                             // get the size
  fseek(file,0,SEEK_SET);                         // reset to beginning
  memfile = malloc(size*sizeof(char)+1);          // malloc the size

  if (!memfile)
    I_Error("DDF_MainReadFile: Malloc Failed on memfile\n");

  memset (memfile,'\0',size*sizeof(char)+1);
  fread ((char*)memfile,sizeof(char),size,file);

  fclose (file); // close the file
  readinfo->memsize = size;
  return memfile;
}

//
// Description of the DDF Parser:
//
// The DDF Parser is a simple reader that is very limited in error checking,
// however it can adapt to most tasks, as is required for the variety of stuff
// need to be loaded in order to configure the DOSDoom Engine.
//
// The parser will read an ascii file, character by character an interpret each
// depending in which mode it is in; Unless an error is encountered or a called
// procedure stops the parser, it will read everything until EOF is encountered.
//
// When the parser function is called, a pointer to a readinfo_t is passed and
// contains all the info needed, it contains:
//
// * message               - message displayed on the screen, if NULL nothing displayed
// * filename              - filename to be read, returns error if NULL
// * DDF_MainCheckName     - function called when a def has been just been started
// * DDF_MainCheckCmd      - function called when we need to check a command
// * DDF_MainCreateEntry   - function called when a def has been completed
// * DDF_MainFinishingCode - function called when EOF is read
// * currentcmdlist        - Current list of commands
//
// Also when commands are referenced, they use currentcmdlist, which is a pointer
// to a list of entries, the entries are formatted like this:
//
// * name - name of command
// * routine - function called to interpret info
// * numeric - void pointer to an value (possibly used by routine)
//
// name is compared with the read command, to see if it matchs.
// routine called to interpret info, if command name matches read command.
// numeric is used if a numeric value needs to be changed, by routine.
//
// The different parser modes are:
//  waiting_newdef
//  reading_newdef
//  reading_command
//  reading_data
//  reading_remark
//  reading_string
//
// 'waiting_newdef' is only set at the start of the code, At this point every
// character with the exception of DEFSTART is ignored. When DEFSTART is
// encounted, the parser will switch to reading_newdef. DEFSTART the parser
// will only switches modes and sets firstgo to false.
//
// 'reading_newdef' reads all alphanumeric characters and the '_' character - which
// substitudes for a space character (whitespace is ignored) - until DEFSTOP is read.
// DEFSTOP passes the read string to DDF_MainCheckName and then clears the string.
// Mode reading_command is now set. All read stuff is passed to char *buffer.
//
// 'reading_command' picks out all the alphabetic characters and passes them to
// buffer as soon as COMMANDREAD is encountered; DDF_MainReadCmd looks through
// for a matching command, if none is found a fatal error is returned. If a matching
// command is found, this function returns a command reference number to command ref
// and sets the mode to reading_data. if DEFSTART is encountered the procedure will
// clear the buffer, run DDF_MainCreateEntry (called this as it reflects that in Items
// & Scenery if starts a new mobj type, in truth it can do anything procedure wise) and
// then switch mode to reading_newdef.
//
// 'reading_data' passes alphanumeric characters, plus a few other characters that
// are also needed. It continues to feed buffer until a SEPERATOR or a TERMINATOR is
// found. The difference between SEPERATOR and TERMINATOR is that a TERMINATOR refs
// the cmdlist to find the routine to use and then sets the mode to reading_command,
// whereas SEPERATOR refs the cmdlist to find the routine and a looks for more data
// on the same command. This is how the multiple states and specials are defined.
//
// 'reading_remark' does not process any chars except REMARKSTOP, everything else is
// ignored. This mode is only set when REMARKSTART is found, when this happens the
// current mode is held in formerstatus, which is restored when REMARKSTOP is found.
//
// 'reading_string' is set when the parser is going through data (reading_data) and
// encounters STRINGSTART and only stops on a STRINGSTOP. When reading_string,
// everything that is an ASCII char is read (which the exception of STRINGSTOP) and
// passed to the buffer. REMARKS are ignored in when reading_string and the case is
// take notice of here.
//
// The maximum size of BUFFER is set in the BUFFERSIZE define.
//
// DDF_MainReadFile & DDF_MainProcessChar handle the main processing of the file, all
// the procedures in the other DDF files (which the exceptions of the Inits) are
// called directly or indirectly. DDF_MainReadFile handles to opening, closing and
// calling of procedures, DDF_MainProcessChar makes sense from the character read
// from the file.
//

//
// DDF_MainReadFile
//
// -ACB- 1998/08/10 Added the string reading code
//
void DDF_MainReadFile (readinfo_t *readinfo)
{
  char *buffer;
  char character;
  char *memfile;
  char *memfileptr;
  readstatus_t status, formerstatus;
  readchar_t response;
  int commandref;
  int size;
  boolean firstgo;

#ifdef DEVELOPERS
  char charcount = 0;
#endif

  statecount = NUMSTATES;
  status = waiting_newdef;
  formerstatus = NULL;
  commandref = NULL;
  firstgo = true;

  if (readinfo->message)
    I_Printf("  %s",readinfo->message);

  if (!readinfo->memfile && !readinfo->filename)
    I_Error("DDF_MainReadFile: No file to read");

  if (!readinfo->memfile)
    readinfo->memfile = DDF_MainCacheFile(readinfo);

  // Get the current list from readinfo
  currentcmdlist = readinfo->cmdlist;

  buffer = malloc (sizeof(char)*BUFFERSIZE);

  if (buffer == NULL)
    I_Error("DDF_MainReadFile: Malloc for BUFFER failed\n");

  memset(buffer,'\0',sizeof(sizeof(char)*BUFFERSIZE));

  memfileptr = memfile = readinfo->memfile;
  size = readinfo->memsize;

  // -ACB- 1998/09/12 Copy file to memory: Read until end. Speed optimisation.
  while (memfileptr < &memfile[size])
  {
    // -KM- 1998/12/16 Added #define command to ddf files.
    if (!strcmp(memfileptr, "#DEFINE"))
    {
      char* name = memfileptr;
      char* value;
      while (*memfileptr != ' ' && memfileptr < &memfile[size]) memfileptr++;
      if (memfileptr < &memfile[size])
        value = memfileptr;
      else
        I_Error("\n#DEFINE '%s' as what?!\n", name);
      while (*memfileptr != ' ' && memfileptr < &memfile[size]) memfileptr++;
      DDF_MainAddDefine(name, value);
      buffer[0] = '\0';
      status = reading_command;
      terminated = false;
      continue;
    }

    character = *memfileptr++;

    response = DDF_MainProcessChar(character, buffer, status);

    switch (response)
    {
      case remark_start:
        formerstatus = status;
        status = reading_remark;
        break;

      case remark_stop:
        status = formerstatus;
        break;

      case command_read:
        strupr(buffer);

        //
        // Confused..The Function sets up the command reference and returns
        // a pointer to the reading function. This function is the one that looks at
        // the string read from the file. got it?
        //
        // -ACB- 1998/08/10 Returns commandref, not function.
        //
        commandref = readinfo->DDF_MainCheckCmd(buffer);
        DDF_ReadFunction = currentcmdlist[commandref].routine;
        buffer[0] = '\0';
        status = reading_data;
        break;

      case def_start:
        I_Printf(".");
        if (firstgo)
        {
          firstgo = false;
          status=reading_newdef;
        }
        else
        {
          readinfo->DDF_MainCreateEntry();
          buffer[0] = '\0';
          status=reading_newdef;
        }
        break;

      case def_stop:
        strupr(buffer);
        readinfo->DDF_MainCheckName(buffer);
        buffer[0] = '\0';
        status=reading_command;
        break;

      case seperator:
        DDF_ReadFunction(DDF_MainGetDefine(buffer),commandref);
        buffer[0] = '\0';
        break;

      // -ACB- 1998/08/10 String Handling
      case string_start:
        status = reading_string;
        break;

      // -ACB- 1998/08/10 String Handling
      case string_stop:
        status = reading_data;
        break;

      case terminator:
        terminated = true;
        DDF_ReadFunction(DDF_MainGetDefine(buffer),commandref);
        buffer[0] = '\0';
        status = reading_command;
        terminated = false;
        break;

      case nothing:
        break;

      case ok_char:
#ifdef DEVELOPERS
        charcount++;
        File_Printf("%c",character);
        if (charcount == 75)
        {
          charcount = 0;
          File_Printf("\n");
        }
#endif
        break;

      default:
        break;
    }

  }

  // if firstgo is true, nothing was defined
  if (!firstgo) readinfo->DDF_MainFinishingCode();

  I_Printf("\n");
  free(buffer);
  free(memfile);
}

//
// DDF_MainProcessChar
//
// 1998/08/10 Added String reading code.
//
readchar_t DDF_MainProcessChar(char character, char *buffer, readstatus_t status)
{
   int len;
   char concat[2] = "\0";

   // -ACB- 1998/08/11 Used for detecting formatting in a string
   static char formatchar = false;

   len = strlen(buffer);

   if (len > BUFFERSIZE)
     I_Error("DDF_MainProcessChar: Read Buffer Size Exceeded, Size: %d\n", BUFFERSIZE);

   // With the exception of reading_string, whitespace is ignored and
   // a SUBSPACE is replaced by a space.
   if (status != reading_string)
   {
     if (isspace(character))
       return nothing;

     if (character == SUBSPACE)
       character = SPACE;
   }
   else // check for formatting char in a string
   {
     if (character == '\\')
     {
       formatchar = true;
       return nothing;
     }
   }

   switch (status)
   {
     case reading_remark:
       if (character == REMARKSTOP)
         return remark_stop;
       else
         return nothing;
       break;

    case waiting_newdef:
      if (character == REMARKSTART) // -ACB- 1998/07/31 Missed this out first time
        return remark_start;
      else if (character == DEFSTART)
        return def_start;
      else
        return nothing;
      break;

    case reading_newdef:
      if (character == REMARKSTART)
      {
        return remark_start;
      }
      else if (character == DEFSTOP)
      {
        return def_stop;
      }
      else if ( (isalnum(character)) || (character == SPACE))
      {
        sprintf(concat,"%c",character);
        strcat(buffer,concat);
        return ok_char;
      }
      return nothing;
      break;

   case reading_command:
     if (character == REMARKSTART)
     {
       return remark_start;
     }
     else if (character == COMMANDREAD)
     {
       return command_read;
     }
     else if (character == DEFSTART)
     {
       return def_start;
     }
     else if (isalnum(character) || character == SPACE)
     {
       sprintf(concat,"%c",character);
       strcat(buffer,concat);
       return ok_char;
     }

     return nothing;
     break;

   // -ACB- 1998/08/10 Check for string start
   case reading_data:
     if (character == STRINGSTART)
     {
       return string_start;
     }
     if (character == REMARKSTART)
     {
       return remark_start;
     }
     else if (character == TERMINATOR)
     {
       return terminator;
     }
     else if (character == SEPERATOR)
     {
       return seperator;
     }
     // Sprite Data - more than a few exceptions....
     else if (isalnum(character) || character == SPACE || character == '-' ||
              character == ':' || character == '.' || character == '[' ||
              character == ']' || character == '\\' || character == '!' ||
              character == '#' )
     {
       sprintf(concat,"%c",character);
       strupr(concat);
       strcat(buffer,concat);
       return ok_char;
     }
     break;

   case reading_string: // -ACB- 1998/08/10 New string handling
     if (character == STRINGSTOP)
     {
       return string_stop;
     }
     else if (formatchar)
     {
       // -ACB- 1998/08/11 Formatting check: Carriage-return.
       if (character == 'n')
       {
         sprintf(concat,"%c",'\n');
         strcat(buffer,concat);
         formatchar = false;
         return ok_char;
       }
       // -KM- 1998/10/29 Also recognise quote
       else if (character == '\"')
       {
         sprintf(concat,"%c",'\"');
         strcat(buffer,concat);
         formatchar = false;
         return ok_char;
       }
     }
     // -KM- 1998/10/29 Removed ascii check, allow foreign characters („)
     else
     {
       sprintf(concat,"%c",character);
       strcat(buffer,concat);
       return ok_char;
     }
     break;

   default:  // doh!
     break;
  }

  return nothing;
}

//
// DDF_MainGetNumeric
//
// Get numeric value directly from the file
//
void DDF_MainGetNumeric(char *info, int commandref)
{
  int i;

  i = atoi(info); // straight conversion - no messin'

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetNumeric: Integer not specified\n");

  // data is a void pointer, use memcpy to transfer info -ACB- 1998/07/31
  memcpy(currentcmdlist[commandref].data, &i, sizeof(int));

  return;
}


//
// DDF_MainGetBoolean
//
// Get true/false from the file
//
// -KM- 1998/09/01 Gets a true/false value
//
void DDF_MainGetBoolean(char *info, int commandref)
{
  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetBoolean: Target not specified! (Internal bug)\n");

  if (!strcasecmp(info, "TRUE"))
    *(boolean *) currentcmdlist[commandref].data = true;
  else if (!strcasecmp(info, "FALSE"))
    *(boolean *) currentcmdlist[commandref].data = false;
#ifdef DEVELOPERS
  else
    I_Error("Boolean is neither True nor False??!!\n");
#endif
}

//
// DDF_MainGetString
//
// Get String value directly from the file
//
// -KM- 1998/07/31 Needed a string argument.  Based on DDF_MainGetNumeric.
//
void DDF_MainGetString(char *info, int commandref)
{
  char *string;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetString: Destination not specified\n");

  string = strdup(info);

  if (string==NULL)
    I_Error("\nDDF_MainGetString: Malloc Failed\n");

  memcpy((char *)currentcmdlist[commandref].data, &string, sizeof(char *));

  return;
}

//
// DDF_MainLookupSound
//
// Lookup the sound specificed.
//
// -ACB- 1998/07/08 Checked the S_sfx table for sfx names.
// -ACB- 1998/07/18 Removed to the need set *currentcmdlist[commandref].data to -1
// -KM- 1998/09/27 Fixed this func because of sounds.ddf
void DDF_MainLookupSound(char *info, int commandref)
{
#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainLookupSound: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainLookupSound: Integer not specified\n");
#endif

  // -KM- 1998/10/29 sfx_t finished
  *(sfx_t **) currentcmdlist[commandref].data = DDF_LookupSound(info);

  return;
}

//
// DDF_MainCheckCommand
//
// Check command exists, and then return reference.
//
int DDF_MainCheckCommand(char *info)
{
  int i;

  i=0;

  while (strcmp(info,currentcmdlist[i].name)
        && strcmp(currentcmdlist[i].name,COMMAND_TERMINATOR))
  {
    i++;
  }

  if (!strcmp(currentcmdlist[i].name,COMMAND_TERMINATOR))
    I_Error("\nDDF_MainCheckCommand: Unknown Command - %s\n", info);

  return i;
}

//
// DDF_MainMobjCheckName
//
// Check the Names is unique to any other thing, then give it to buffermobj.
//
void DDF_MainMobjCheckName(char *info)
{
  mobjinfo_t* entry;

  if (info == NULL)
    I_Error("Info has no info\n");

  entry = mobjinfohead;
  bufferreplacemobj = NULL;

  while (entry != NULL)
  {
    if (entry->name != NULL)
    {
      if (!strcmp(info,entry->name))
      {
        if (ddf_replace)
          bufferreplacemobj = entry;
        else
          I_Error("DDF_MainCheckName: '%s' already declared\n",info);
      }
    }

   entry = entry->next;
  }

  if ((buffermobj.name = malloc(sizeof(char)*(strlen(info)+1)))==NULL)
    I_Error("DDF_MainCheckName: Unable to allocate memory\n");

  strcpy(buffermobj.name,info);

  return;
}

//
// DDF_MainGetSpecial
//
// Compares info the the entries in specialslist: if found apply attribs for it
//
void DDF_MainGetSpecial(char *info, int commandref)
{
  int i;

  i=0;

  while ( strcmp(info,specialslist[i].specialname) &&
          strcmp(specialslist[i].specialname, COMMAND_TERMINATOR) )
  {
    i++;
  }

  if (!strcmp(specialslist[i].specialname, COMMAND_TERMINATOR))
    I_Error("\n\tDDF_MainGetSpecial: No such special %s",info);

  if (specialslist[i].flags)
    buffermobj.flags |= specialslist[i].flags;

  if (specialslist[i].extendedflags)
    buffermobj.extendedflags |= specialslist[i].extendedflags;
// -KM- 1998/11/25 Removed translucency test.
}

//
// DDF_MainSplitIntoState
//
// Small procedure that takes the info and splits it into relevant stuff
//
// -KM- 1998/12/21 Rewrote procedure, much cleaner now.
int DDF_MainSplitIntoState(char *info)
{
  char *temp;
  int i;

  for (i = NUMSPLIT; i--; )
     stateinfo[i] = NULL;

  for (temp = strtok(info, ":"), i = 0; temp && i < NUMSPLIT; temp = strtok(NULL, ":"), i++)
  {
    if (temp[0] == REDIRECTOR)
    {
       stateinfo[2] = NULL; // signify that we have found redirector
       stateinfo[0] = strdup(temp + 1);

       temp = strtok(NULL, ":");
       if (temp)
         stateinfo[1] = strdup(temp);
       else
         stateinfo[1] = NULL;

       return -1;
    }

    stateinfo[i] = strdup(temp);
  }
  return i;
}

/*
void DDF_MainSplitIntoState(char *info)
{
  char *temp;
  char *remaininginfo;
  int i,j;

  remaininginfo = info;

  temp = strchr(remaininginfo,DIVIDE);        // find DIVIDE

  if (remaininginfo[0] == REDIRECTOR)
  {
     stateinfo[2] = NULL; // signify that we have found redirector
     remaininginfo++;

     if (temp == NULL)
     {
       stateinfo[0] = remaininginfo;      // copy it to the info
       stateinfo[1] = NULL;
       return;
     }

     i = strlen(remaininginfo) - strlen(temp);

     stateinfo[0]=malloc(sizeof(char)*(i+1));

     if (stateinfo[0]==NULL)
       I_Error("\n\tDDF_MainSplitIntoState: Malloc Failed\n");

     memset(stateinfo[0],'\0',sizeof(char)*(i+1));
     strncat(stateinfo[0],remaininginfo,i);      // copy it to the info
     remaininginfo+=(i+1);

     stateinfo[1] = &remaininginfo[0];
     return;
  }

  for (j=0; j<(NUMSPLIT-1); j++)
  {
    temp = strchr(remaininginfo,DIVIDE);        // find DIVIDE

    i = strlen(remaininginfo) - strlen(temp);

    stateinfo[j]=malloc(sizeof(char)*(i+1));

    if (stateinfo[j]==NULL)
      I_Error("\n\tDDF_SplitIntoStateInfo: Malloc Failed\n");

    memset(stateinfo[j],'\0',sizeof(char)*(i+1));
    strncat(stateinfo[j],remaininginfo,i);      // copy it to the info

    remaininginfo+=(i+1);
  }

  temp = (remaininginfo+1); // step over the DIVIDE.

  if (temp==NULL)
    I_Error("DDF_SplitIntoStateInfo: Nothing to split up\n");

  i = strlen(remaininginfo);

  stateinfo[NUMSPLIT-1]=malloc(sizeof(char)*(i+1));
  memset(stateinfo[NUMSPLIT-1],'\0',sizeof(char)*(i+1));

  if (stateinfo[NUMSPLIT-1]==NULL)
    I_Error("\n\tDDF_SplitIntoStateInfo: Malloc Failed\n");

  strcpy(stateinfo[NUMSPLIT-1],remaininginfo);      // copy it to the info

  return;
}
*/

//
// DDF_MainLoadStates
//
// Quite frankly this is not the greatest code written, loads the states table
//
void DDF_MainLoadStates(char *info, int commandref)
{
  int count,i,j;
  char *backupinfo;

  backupinfo = info;
  count = statecount - NUMSTATES;

  if ((tempstates[count]=malloc(sizeof(state_t)))==NULL)
    I_Error("DDF_ItemLoadSprites: Unable to allocate memory!\n");

  tempstates[count]->nextstate = statecount + 1;

  // Split the state info into component parts
  // -ACB- 1998/07/26 New Procedure, for cleaner code.
  i = DDF_MainSplitIntoState(info);
  if (i < 5 && i >=0)
    I_Error("\n\tDDF_MainLoadStates: Bad state '%s'\n", info);

  if (stateinfo[0]==NULL)
    I_Error("\n\tDDF_MainLoadStates: Stateinfo is NULL\n");

  //--------------------------------------------------
  //----------------REDIRECTOR HANDLING---------------
  //--------------------------------------------------
  if(stateinfo[2]==NULL)
  {
    if (!terminated)
      I_Printf("\n\tDDF_MainLoadStates: Warning - redirector terminates\n");

    // Copy Tempstates to states table......
    if (statecount > MAXSTATES)
    {
      MAXSTATES = statecount + 32;
      states = realloc(states, sizeof(*states) * MAXSTATES);
    }

    // Use Numeric ref
    memcpy(currentcmdlist[commandref].data, &NUMSTATES, sizeof(int));

    for (i = 0; i <= (count-1); i++)
    {
      states[NUMSTATES+i].sprite    = tempstates[i]->sprite;
      states[NUMSTATES+i].frame     = tempstates[i]->frame;
      states[NUMSTATES+i].tics      = tempstates[i]->tics;
      states[NUMSTATES+i].nextstate = tempstates[i]->nextstate;
      states[NUMSTATES+i].action    = tempstates[i]->action;
      states[NUMSTATES+i].misc1     = tempstates[i]->misc1;
      states[NUMSTATES+i].misc2     = tempstates[i]->misc2;

      free(tempstates[i]);
    }

    if (!strcmp(stateinfo[0],"REMOVE"))
    {
      states[statecount-1].nextstate = S_NULL;
    }
    else
    {
      i=j=0;

      //
      // This checks the redirector name and if one is found, gives     -ACB-
      // a subflag an places it in the nextstate reference, this is       |
      // dereferences when the object is finalised (in DDF_MOBJ.C),       |
      // so this only for object defines. Any offset (like chase state   \|/
      // + one frame) is added to the state, this will be sorted when
      // the program derefs the flag.
      //
      while(strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector) &&
             strcmp(framedestlist[i].redirector,stateinfo[0]))
      {
        i++;
      }

      if (!strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector))
        I_Error("\n\tDDF_MainLoadStates: Unknown Redirector %s\n",stateinfo[0]);

      j = framedestlist[i].subflag;

      i=0;

      if (stateinfo[1] != NULL)
      {
        i = atoi(stateinfo[1])-1;

        if (i < 0)
          i = 0;
      }

      states[statecount-1].nextstate = j + i;

                                                                      /* /|\ */
      // End of sprite referencing                                        |
      //                                                                  |
      //                                                                -ACB-
    }

    NUMSTATES = statecount;
    return;
  }

  //--------------------------------------------------
  //----------------SPRITE NAME HANDLING--------------
  //--------------------------------------------------
  if (strlen(stateinfo[0]) != 4)
    I_Error("\nDDF_MainLoadStates: Sprite Names must be 4 Characters long '%s'.\n",
            stateinfo[0]);

  i=0;
  while (i != NUMSPRITES && strcmp(stateinfo[0],sprnames[i]))
    i++;

  if (i==NUMSPRITES)
  {
    if ((sprnames[NUMSPRITES] = strdup(stateinfo[0]))==NULL)
      I_Error("DDF_MainSpriteName: Unable to allocate enough memory!\n");

    tempstates[count]->sprite = NUMSPRITES;
    NUMSPRITES++;

    if (NUMSPRITES > MAXSPRITES)
    {
      MAXSPRITES += 32;
      sprnames = realloc(sprnames, sizeof(*sprnames) * MAXSPRITES);
    }
  }
  else
  {
    tempstates[count]->sprite = i;
  }

  //--------------------------------------------------
  //--------------SPRITE INDEX HANDLING---------------
  //--------------------------------------------------

  j = stateinfo[1][0];                  // look at the first character

  if ( j < 'A' || j > ']' )             // check for bugger up
    I_Error("\nDDF_MainLoadStates: Illegal Sprite Index %c\n", j);

  tempstates[count]->frame = (long) (j-65);

  //--------------------------------------------------
  //------------STATE TIC COUNT HANDLING--------------
  //--------------------------------------------------
  tempstates[count]->tics = atol(stateinfo[2]);

  //--------------------------------------------------
  //------------STATE BRIGHTNESS LEVEL----------------
  //--------------------------------------------------
  if (!strcmp("BRIGHT",stateinfo[3]))
    tempstates[count]->frame = tempstates[count]->frame + FF_FULLBRIGHT;
  else if ( strcmp("NORMAL",stateinfo[3]) )
    I_Error("\n\tDDF_MainLoadStates: Lighting is not BRIGHT or NORMAL\n");

  //--------------------------------------------------
  //------------STATE ACTION CODE HANDLING------------
  //--------------------------------------------------

  // Get Action Code Ref (Using remainder of the string).
  // Go through all the actions, end if terminator or action found
  i=0;
  while (strcmp(actions[i].actionname,stateinfo[4]) &&
          strcmp(actions[i].actionname,COMMAND_TERMINATOR))
  {
    i++;
  }

  if (!strcmp(actions[i].actionname,COMMAND_TERMINATOR))
    I_Error("\n\tDDF_MainLoadStates: Unknown Command: %s\n",stateinfo[4]);

  tempstates[count]->action = actions[i].action;

  //--------------------------------------------------
  //--------------------------------------------------
  // Misc1 + 2
  if (stateinfo[5])
    tempstates[count]->misc1 = atoi(stateinfo[5]);
  else
    tempstates[count]->misc1 = 0;
  if (stateinfo[6])
    tempstates[count]->misc2 = atoi(stateinfo[6]);
  else
    tempstates[count]->misc2 = 0;

  // Terminator Found - Pass the states to the main state table, free up mem
  if (terminated)
  {
    // data is a void pointer, use memcpy to transfer info -ACB- 1998/07/31
    memcpy(currentcmdlist[commandref].data, &NUMSTATES, sizeof(int));

    if (statecount > MAXSTATES)
    {
      MAXSTATES = statecount + 32;
      states = realloc(states, sizeof(*states) * MAXSTATES);
    }

    for (i = 0; i <= count; i++)
    {
      states[NUMSTATES+i].sprite    = tempstates[i]->sprite;
      states[NUMSTATES+i].frame     = tempstates[i]->frame;
      states[NUMSTATES+i].tics      = tempstates[i]->tics;
      states[NUMSTATES+i].nextstate = tempstates[i]->nextstate;
      states[NUMSTATES+i].action    = tempstates[i]->action;
      states[NUMSTATES+i].misc1     = tempstates[i]->misc1;
      states[NUMSTATES+i].misc2     = tempstates[i]->misc2;

      free(tempstates[i]);
    }

    states[statecount].nextstate = NUMSTATES;

    NUMSTATES = statecount + 1;
  }

  info = backupinfo;
  statecount++;
}

//
// DDF_MainGetLumpName
//
// Gets the string and checks the length to see if is not more than 8.
//
void DDF_MainGetLumpName(char *info, int commandref)
{
  char *lumpname;

  lumpname = (char *) currentcmdlist[commandref].data;

  if (strlen(info) > 8)
    I_Error("\nDDF_MainGetLumpName: Lump name %s too big\n", info);

  if (lumpname == NULL)
    I_Error("\nDDF_MainGetLumpName: Destination not specified\n");

  strcpy(lumpname, info);

  return;
}

//
// DDF_MainGetMusicName
//
// Gets the string and checks the length is not bigger than 6, if
// this is the case D_ is prefixed and then it is stored.
//
void DDF_MainGetMusicName(char *info, int commandref)
{
  char *lumpname;

  lumpname = (char *) currentcmdlist[commandref].data;

  if (strlen(info) > 6)
    I_Error("\nDDF_MainGetMusicName: Music name too big\n");

  if (lumpname == NULL)
    I_Error("\nDDF_MainGetMusicName: Destination not specified\n");

  strcpy(lumpname, "D_");
  strcat(lumpname, info);

  return;
}

void DDF_MainReferenceString(char *info, int commandref)
{
  char *string;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainReferenceString: Destination not specified\n");

  string = DDF_LanguageLookup(info);

  memcpy((char *)currentcmdlist[commandref].data, &string, sizeof(char *));
}

void DDF_MainRefAttack(char *info, int commandref)
{
  attacktype_t* currattack;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainRefAttack: Destination not specified\n");

  currattack = attackhead;

  while (currattack != NULL && strcmp(currattack->name,info))
    currattack = currattack->next;

  if (currattack == NULL)
    I_Error("\n\tDDF_MainRefAttack: Attack - %s - does not exist\n",info);

  memcpy((attacktype_t *)currentcmdlist[commandref].data,
           &currattack, sizeof(attacktype_t *));
}

//
// DDF_MainGetDirector
//
void DDF_MainGetDirector (char *info, int commandref)
{
  int i,j;
  char *director;
  char *offsetstring;

  offsetstring = strchr(info,DIVIDE);        // find DIVIDE

  i = strlen(info) - strlen(offsetstring);

  if (i <= 1)
    I_Error("\n\tDDF_MainGetDirector: Nothing after divide\n");

  director = malloc(sizeof(char)*(i+1));
  memset(director,'\0',sizeof(char)*(i+1));

  if (director == NULL)
    I_Error("DDF_MainGetDirector:  Malloc failure");

  director = strncpy(director,info,i);
  offsetstring++; // step over divide

  i=0;

  while(strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector) &&
         strcmp(framedestlist[i].redirector,director))
  {
    i++;
  }

  if (!strcmp(COMMAND_TERMINATOR,framedestlist[i].redirector))
    I_Error("\n\tDDF_MainGetDirector: Unknown Director %s\n",director);

  j = framedestlist[i].subflag;

  i=0;

  if (offsetstring != NULL)
  {
    i = atoi(offsetstring)-1;

    if (i < 0)
      i = 0;
  }

  i += j;

  if (currentcmdlist[commandref].data == NULL)
    I_Error("\nDDF_MainGetNumeric: Integer not specified\n");

  memcpy(currentcmdlist[commandref].data, &i, sizeof(int));
}
void DDF_MainGetAccuracy(char *info, int commandref)
{
  int i = 0;

  // this is cheap...
  if (!(strcmp(info, "HIGH")))
    i = HIGH;
  else if (!(strcmp(info, "MEDIUM")))
    i = MEDIUM;
  else if (!(strcmp(info, "LOW")))
    i = LOW;
  else if (!(strcmp(info, "PATHETIC")))
    i = PATHETIC;

  if (!i)
    I_Error("\n\tDDF_AtkGetAccuracy: Unknown accuracy level - %s",info);

  *(int *)currentcmdlist[commandref].data = i;
}
//
// DDF_MainGetFixed
//
// This procedure gets a number and translates it into a fixed
// number (in Doom Terms). The translated value can have a decimal point...
//
// -ACB- 1998/09/12 Procedure Written.
//
static inline fixed_t DDF_MainGetFixedHelper(char *info, int fixed)
{
  char *temp;
  fixed_t i = 0, f = 0;
  int j;

  // Get the integer part
  i = atoi(strtok(info, ".")) * fixed;

  // Get the decimal part
  temp = strtok(NULL, ".");
  if (temp)
  {
    f = (fixed * atoi(temp));
    // -KM- 1998/11/25 Fixed major bug, that resulted in incorrect fracs
    //   for anything with more than one decimal place, eg 0.3 worked, but
    //   0.33333 would not.
    for (j = strlen(temp); j > 0; j--)
      f /= 10;
  }
  return i + f;
}

void DDF_MainGetFixed(char *info, int commandref)
{
#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetFixed: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetFixed: Integer not specified\n");
#endif
  *(fixed_t *)currentcmdlist[commandref].data = DDF_MainGetFixedHelper(info, FRACUNIT);
}

// -KM- 1998/09/27 You can end a number with T to specify tics; ie 35T means 35 tics
// while 3.5 means 3.5 seconds.
void DDF_MainGetTime(char *info, int commandref)
{
#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetTime: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetTime: Integer not specified\n");
#endif
  if (strchr(info, 'T'))
    DDF_MainGetNumeric(info, commandref);
  else
    *(int *)currentcmdlist[commandref].data = DDF_MainGetFixedHelper(info, TICRATE);
}
/*
void DDF_MainGetFixed(char *info, int commandref)
{
  int total = 0;
  int divnum = 0;
  int i, j, k;
  char *string;
  char *newstring;

#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetFixed: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetFixed: Integer not specified\n");
#endif

  string = strchr(info,'.');        // find decimal point

  if (!string) // no decimal point
  {
    *(int *)currentcmdlist[commandref].data = atoi(info)<<FRACBITS;
    return;
  }

  i = strlen(info) - strlen(string);

  string++; // step over decimal point

  if (!string) // nothing after the decimal point
  {
    *(int *)currentcmdlist[commandref].data = atoi(info)<<FRACBITS;
    return;
  }

  for (j=0; j<(i+1); j++)
  {
    divnum = 1;

    for (k=0; k<(j+1); k++) // calc division number (10 to the power of (j+1))
     divnum *= 10;

    total += FixedDiv((string[j]-'0')<<FRACBITS, divnum);

  }

  newstring = malloc(sizeof(char)*(i+1));

  if (!newstring)
    I_Error("DDF_MainGetFixed: Malloc failure on newstring");

  strncpy(newstring, info, i);
  j = atoi(newstring)<<FRACBITS;

  // Check for negative number.
  if (newstring[0] == '-')
    total = j - total;
  else
    total = j + total;

  *(int *)currentcmdlist[commandref].data = total;

  free(string);
}

//
// DDF_MainGetTime
//
// This procedure gets a number and translates it into the number of
// tics (using the TICRATE define), also allowed is decimal point values.
//
// -ACB- 1998/09/12 Procedure Written.
//
void DDF_MainGetTime(char *info, int commandref)
{
  int total = 0;
  int divnum = 0;
  int i, j, k;
  char *string;
  char *newstring;

#ifdef DEVELOPERS
  if (!info)
    I_Error("DDF_MainGetTime: info clobbered\n");

  if (currentcmdlist[commandref].data == NULL)
    I_Error("DDF_MainGetTime: Integer not specified\n");
#endif

  string = strchr(info,'.');        // find decimal point

  if (!string) // no decimal point
  {
    *(int *)currentcmdlist[commandref].data = atoi(info)*TICRATE;
    return;
  }

  i = strlen(info) - strlen(string);

  string++; // step over decimal point

  if (!string) // nothing after the decimal point
  {
    *(int *)currentcmdlist[commandref].data = atoi(info)*TICRATE;
    return;
  }

  for (j=0; j<(i+1); j++)
  {
    divnum = 1;

    for (k=0; k<(j+1); k++) // calc division number (10 to the power of (j+1))
     divnum *= 10;

    total += ((string[j]-'0')*TICRATE)/divnum;

  }

  newstring = malloc(sizeof(char)*(i+1));

  if (!newstring)
    I_Error("DDF_MainGetTime: Malloc failure on newstring");

  strncpy(newstring, info, i);
  j += atoi(newstring)*TICRATE;

  // Check for negative number. Time is always a positive value...
  if (newstring[0] == '-')
    j = 0 - j;

  total += j;

  *(int *)currentcmdlist[commandref].data = total;

  free(string);
}
*/

// DDF_DummyFunction
void DDF_DummyFunction(char *info) {return;};

//
// DDF_OldThingInit
//
// Temporary code that adjusts the current info, so the games plays as Doom
// a.k.a. The most hacked code going
//
void DDF_OldThingInit()
{
  // PRE-DDF Hack
//  specials[MOBJ_PLAYER]      = &oldmobjinfo[MT_PLAYER];
  specials[MOBJ_BLOOD]       = &oldmobjinfo[MT_BLOOD];
  specials[MOBJ_PUFF]        = &oldmobjinfo[MT_PUFF];
  specials[MOBJ_RESPAWNFOG]  = &oldmobjinfo[MT_IFOG];
  specials[MOBJ_SMOKE]       = &oldmobjinfo[MT_SMOKE];
  specials[MOBJ_SPAWNSPOT]   = &oldmobjinfo[MT_BOSSTARGET];
  specials[MOBJ_TELEPOS]     = &oldmobjinfo[MT_TELEPORTMAN];
}

// -KM- 1998/10/29 Add stuff from ddf_lines.c that is also shared by ddf_sect.c

static struct
{
  char *s;
  int  n;
}

s_exit[] =
{
 {"NONE", 0},
 {"EXIT", 1},
 {"SECRET", 2}
},

s_lite[] =
{
  {"SET", lite_set},
  {"STROBE", lite_strobe},
  {"FLASH", lite_flash},
  {"GLOW", lite_glow},
  {"FLICKER", lite_fireflicker}
},

s_movement[] =
{
  {"MOVE"          , mov_Once},
  {"MOVEWAITRETURN", mov_MoveWaitReturn},
  {"CONTINUOUS"    , mov_Continuous},
  {"BUILDSTAIRS"   , mov_Stairs},
  {"STOP"          , mov_Stop}
},

s_reference[] =
{
  {"ABSOLUTE",             ref_absolute},
  {"FLOOR",                ref_floorHeight},
  {"CEILING",              ref_ceilingHeight},
  {"LOSURROUNDINGCEILING", ref_LowestSurroundingCeiling},
  {"HISURROUNDINGCEILING", ref_HighestSurroundingCeiling},
  {"LOSURROUNDINGFLOOR",   ref_LowestSurroundingFloor},
  {"HISURROUNDINGFLOOR",   ref_HighestSurroundingFloor},
  {"NEXTHIGHESTFLOOR",     ref_NextHighestFloor},
  {"LOWESTBOTTOMTEXTURE",  ref_LowestLoTexture}
};
//
// DDF_LineGetExit
//
// Get the exit type
//
void DDF_MainGetExit(char *info, int c)
{
  int i;

  for (i = sizeof(s_exit) / sizeof(s_exit[0]); i--;)
  {
    if (!strcasecmp(info, s_exit[i].s))
    {
      *(int *)currentcmdlist[c].data = s_exit[i].n;
      return;
    }
  }

  I_Error("\nUnknown Exit type: %s\n", info);
}

//
// DDF_LineGetLighttype
//
// Get the light type
//
void DDF_MainGetLighttype(char *info, int c)
{
  int i;

  for (i = sizeof(s_lite) / sizeof(s_lite[0]); i--;)
  {
    if (!strcasecmp(info, s_lite[i].s))
    {
      *(int *)currentcmdlist[c].data = s_lite[i].n;
      return;
    }
  }

  I_Error("\nUnknown light type: %s\n", info);
}

//
// DDF_LineGetMType
//
// Get movement types: MoveWaitReturn etc
//
void DDF_MainGetMType(char *info, int c)
{
  int i;

  for (i = sizeof(s_movement) / sizeof(s_movement[0]); i--;)
  {
    if (!strcasecmp(info, s_movement[i].s))
    {
      *(int *)currentcmdlist[c].data = s_movement[i].n;
      return;
    }
  }

  I_Error("\nUnknown Movement type %s\n", info);
}

//
// DDF_LineGetDestRef
//
// Get surroundingsectorceiling/floorheight etc
//
void DDF_MainGetDestRef(char *info, int c)
{
  int i;

  for (i = sizeof(s_reference) / sizeof(s_reference[0]); i--;)
  {
    if (!strcasecmp(info, s_reference[i].s))
    {
      *(int *)currentcmdlist[c].data = s_reference[i].n;
      return;
    }
  }

  I_Error("\nUnknown Reference Point %s\n", info);
}


