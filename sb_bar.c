
// SB_bar.c

#include "DoomDef.h"
#include "P_local.h"
#include "soundst.h"

// Macros

#define CHEAT_ENCRYPT(a) \
	((((a)&1)<<5)+ \
	(((a)&2)<<1)+ \
	(((a)&4)<<4)+ \
	(((a)&8)>>3)+ \
	(((a)&16)>>3)+ \
	(((a)&32)<<2)+ \
	(((a)&64)>>2)+ \
	(((a)&128)>>4))

// Types

typedef struct Cheat_s
{
	void (*func)(player_t *player, struct Cheat_s *cheat);
	byte *sequence;
	byte *pos;
	int args[2];
	int currentArg;
} Cheat_t;

// Private Functions

static void DrawSoundInfo(void);
static void ShadeLine(int x, int y, int height, int shade);
static void ShadeChain(void);
static void DrINumber(signed int val, int x, int y);
static void DrBNumber(signed int val, int x, int y);
static void DrawCommonBar(void);
static void DrawMainBar(void);
static void DrawInventoryBar(void);
static void DrawFullScreenStuff(void);
static boolean HandleCheats(byte key);
static boolean CheatAddKey(Cheat_t *cheat, byte key, boolean *eat);
static void CheatGodFunc(player_t *player, Cheat_t *cheat);
static void CheatNoClipFunc(player_t *player, Cheat_t *cheat);
static void CheatWeaponsFunc(player_t *player, Cheat_t *cheat);
static void CheatPowerFunc(player_t *player, Cheat_t *cheat);
static void CheatHealthFunc(player_t *player, Cheat_t *cheat);
static void CheatKeysFunc(player_t *player, Cheat_t *cheat);
static void CheatSoundFunc(player_t *player, Cheat_t *cheat);
static void CheatTickerFunc(player_t *player, Cheat_t *cheat);
static void CheatArtifact1Func(player_t *player, Cheat_t *cheat);
static void CheatArtifact2Func(player_t *player, Cheat_t *cheat);
static void CheatArtifact3Func(player_t *player, Cheat_t *cheat);
static void CheatWarpFunc(player_t *player, Cheat_t *cheat);
static void CheatChickenFunc(player_t *player, Cheat_t *cheat);
static void CheatMassacreFunc(player_t *player, Cheat_t *cheat);
static void CheatIDKFAFunc(player_t *player, Cheat_t *cheat);
static void CheatIDDQDFunc(player_t *player, Cheat_t *cheat);

// Public Data

extern  int     screenblocks;

boolean DebugSound; // debug flag for displaying sound info

boolean inventory;
int curpos;
int inv_ptr;
int ArtifactFlash;

// Private Data

static int HealthMarker;
static int ChainWiggle;
static player_t *CPlayer;
int playpalette;

patch_t *PatchLTFACE;
patch_t *PatchRTFACE;
patch_t *PatchBARBACK;
patch_t *PatchCHAIN;
patch_t *PatchSTATBAR;
patch_t *PatchLIFEGEM;
//patch_t *PatchEMPWEAP;
//patch_t *PatchLIL4BOX;
patch_t *PatchLTFCTOP;
patch_t *PatchRTFCTOP;
//patch_t *PatchARMORBOX;
//patch_t *PatchARTIBOX;
patch_t *PatchSELECTBOX;
//patch_t *PatchKILLSPIC;
//patch_t *PatchMANAPIC;
//patch_t *PatchPOWERICN;
patch_t *PatchINVLFGEM1;
patch_t *PatchINVLFGEM2;
patch_t *PatchINVRTGEM1;
patch_t *PatchINVRTGEM2;
patch_t *PatchINumbers[10];
patch_t *PatchNEGATIVE;
patch_t *PatchSmNumbers[10];
patch_t *PatchBLACKSQ;
patch_t *PatchINVBAR;
patch_t *PatchARMCLEAR;
patch_t *PatchCHAINBACK;
//byte *ShadeTables;
extern byte *screens[7];
int FontBNumBase;
int spinbooklump;
int spinflylump;

static byte CheatLookup[256];

// Toggle god mode
static byte CheatGodSeq[] =
{
	CHEAT_ENCRYPT('q'),
	CHEAT_ENCRYPT('u'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('n'),
	0xff
};

// Toggle no clipping mode
static byte CheatNoClipSeq[] =
{
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('y'),
	0xff
};

// Get all weapons and ammo
static byte CheatWeaponsSeq[] =
{
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('b'),
	CHEAT_ENCRYPT('o'),
	0xff
};

// Toggle tome of power
static byte CheatPowerSeq[] =
{
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('h'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('z'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('m'),
	0xff, 0
};

// Get full health
static byte CheatHealthSeq[] =
{
	CHEAT_ENCRYPT('p'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('e'),
	0xff
};

// Get all keys
static byte CheatKeysSeq[] =
{
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('l'),
	0xff, 0
};

// Toggle sound debug info
static byte CheatSoundSeq[] =
{
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('e'),
	0xff
};

// Toggle ticker
static byte CheatTickerSeq[] =
{
	CHEAT_ENCRYPT('t'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('r'),
	0xff, 0
};

// Get an artifact 1st stage (ask for type)
static byte CheatArtifact1Seq[] =
{
	CHEAT_ENCRYPT('g'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('e'),
	0xff
};

// Get an artifact 2nd stage (ask for count)
static byte CheatArtifact2Seq[] =
{
	CHEAT_ENCRYPT('g'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('e'),
	0, 0xff, 0
};

// Get an artifact final stage
static byte CheatArtifact3Seq[] =
{
	CHEAT_ENCRYPT('g'),
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('e'),
	0, 0, 0xff
};

// Warp to new level
static byte CheatWarpSeq[] =
{
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('n'),
	CHEAT_ENCRYPT('g'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('g'),
	CHEAT_ENCRYPT('e'),
	0, 0, 0xff, 0
};

// Save a screenshot
static byte CheatChickenSeq[] =
{
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('l'),
	CHEAT_ENCRYPT('e'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('o'),
	CHEAT_ENCRYPT('o'),
	0xff, 0
};

// Kill all monsters
static byte CheatMassacreSeq[] =
{
	CHEAT_ENCRYPT('m'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('s'),
	CHEAT_ENCRYPT('a'),
	CHEAT_ENCRYPT('c'),
	CHEAT_ENCRYPT('r'),
	CHEAT_ENCRYPT('e'),
	0xff, 0
};

static byte CheatIDKFASeq[] =
{
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('k'),
	CHEAT_ENCRYPT('f'),
	CHEAT_ENCRYPT('a'),
	0xff, 0
};

static byte CheatIDDQDSeq[] =
{
	CHEAT_ENCRYPT('i'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('d'),
	CHEAT_ENCRYPT('q'),
	CHEAT_ENCRYPT('d'),
	0xff, 0
};

static Cheat_t Cheats[] =
{
	{ CheatGodFunc, CheatGodSeq, NULL, 0, 0, 0 },
	{ CheatNoClipFunc, CheatNoClipSeq, NULL, 0, 0, 0 },
	{ CheatWeaponsFunc, CheatWeaponsSeq, NULL, 0, 0, 0 },
	{ CheatPowerFunc, CheatPowerSeq, NULL, 0, 0, 0 },
	{ CheatHealthFunc, CheatHealthSeq, NULL, 0, 0, 0 },
	{ CheatKeysFunc, CheatKeysSeq, NULL, 0, 0, 0 },
	{ CheatSoundFunc, CheatSoundSeq, NULL, 0, 0, 0 },
	{ CheatTickerFunc, CheatTickerSeq, NULL, 0, 0, 0 },
	{ CheatArtifact1Func, CheatArtifact1Seq, NULL, 0, 0, 0 },
	{ CheatArtifact2Func, CheatArtifact2Seq, NULL, 0, 0, 0 },
	{ CheatArtifact3Func, CheatArtifact3Seq, NULL, 0, 0, 0 },
	{ CheatWarpFunc, CheatWarpSeq, NULL, 0, 0, 0 },
	{ CheatChickenFunc, CheatChickenSeq, NULL, 0, 0, 0 },
	{ CheatMassacreFunc, CheatMassacreSeq, NULL, 0, 0, 0 },
	{ CheatIDKFAFunc, CheatIDKFASeq, NULL, 0, 0, 0 },
	{ CheatIDDQDFunc, CheatIDDQDSeq, NULL, 0, 0, 0 },
	{ NULL, NULL, NULL, 0, 0, 0 } // Terminator
};

//---------------------------------------------------------------------------
//
// PROC SB_Init
//
//---------------------------------------------------------------------------

void SB_Init(void)
{
	int i;
	int startLump;

	PatchLTFACE = W_CacheLumpName("LTFACE", PU_STATIC);
	PatchRTFACE = W_CacheLumpName("RTFACE", PU_STATIC);
	PatchBARBACK = W_CacheLumpName("BARBACK", PU_STATIC);
	PatchINVBAR = W_CacheLumpName("INVBAR", PU_STATIC);
	PatchCHAIN = W_CacheLumpName("CHAIN", PU_STATIC);
	if(deathmatch)
	{
		PatchSTATBAR = W_CacheLumpName("STATBAR", PU_STATIC);
	}
	else
	{
		PatchSTATBAR = W_CacheLumpName("LIFEBAR", PU_STATIC);
	}
	if(!netgame)
	{ // single player game uses red life gem
		PatchLIFEGEM = W_CacheLumpName("LIFEGEM2", PU_STATIC);
	}
	else
	{
		PatchLIFEGEM = W_CacheLumpNum(W_GetNumForName("LIFEGEM0")
			+ consoleplayer, PU_STATIC);
	}
	PatchLTFCTOP = W_CacheLumpName("LTFCTOP", PU_STATIC);
	PatchRTFCTOP = W_CacheLumpName("RTFCTOP", PU_STATIC);
	PatchSELECTBOX = W_CacheLumpName("SELECTBOX", PU_STATIC);
	PatchINVLFGEM1 = W_CacheLumpName("INVGEML1", PU_STATIC);
	PatchINVLFGEM2 = W_CacheLumpName("INVGEML2", PU_STATIC);
	PatchINVRTGEM1 = W_CacheLumpName("INVGEMR1", PU_STATIC);
	PatchINVRTGEM2 = W_CacheLumpName("INVGEMR2", PU_STATIC);
	PatchBLACKSQ    =   W_CacheLumpName("BLACKSQ", PU_STATIC);
	PatchARMCLEAR = W_CacheLumpName("ARMCLEAR", PU_STATIC);
	PatchCHAINBACK = W_CacheLumpName("CHAINBACK", PU_STATIC);
	startLump = W_GetNumForName("IN0");
	for(i = 0; i < 10; i++)
	{
		PatchINumbers[i] = W_CacheLumpNum(startLump+i, PU_STATIC);
	}
	PatchNEGATIVE = W_CacheLumpName("NEGNUM", PU_STATIC);
	FontBNumBase = W_GetNumForName("FONTB16");
	startLump = W_GetNumForName("SMALLIN0");
	for(i = 0; i < 10; i++)
	{
		PatchSmNumbers[i] = W_CacheLumpNum(startLump+i, PU_STATIC);
	}
	playpalette = W_GetNumForName("PLAYPAL");
	spinbooklump = W_GetNumForName("SPINBK0");
	spinflylump = W_GetNumForName("SPFLY0");
	for(i = 0; i < 256; i++)
	{
		CheatLookup[i] = CHEAT_ENCRYPT(i);
	}
}

//---------------------------------------------------------------------------
//
// PROC SB_Ticker
//
//---------------------------------------------------------------------------

void SB_Ticker(void)
{
	int delta;
	int curHealth;

	if(leveltime&1)
	{
		ChainWiggle = P_Random()&1;
	}
	curHealth = players[consoleplayer].mo->health;
	if(curHealth < 0)
	{
		curHealth = 0;
	}
	if(curHealth < HealthMarker)
	{
		delta = (HealthMarker-curHealth)>>2;
		if(delta < 1)
		{
			delta = 1;
		}
		else if(delta > 8)
		{
			delta = 8;
		}
		HealthMarker -= delta;
	}
	else if(curHealth > HealthMarker)
	{
		delta = (curHealth-HealthMarker)>>2;
		if(delta < 1)
		{
			delta = 1;
		}
		else if(delta > 8)
		{
			delta = 8;
		}
		HealthMarker += delta;
	}
}

//---------------------------------------------------------------------------
//
// PROC DrINumber
//
// Draws a three digit number.
//
//---------------------------------------------------------------------------

static void DrINumber(signed int val, int x, int y)
{
	patch_t *patch;
	int oldval;

	oldval = val;
	if(val < 0)
	{
		if(val < -9)
		{
                        V_DrawPatchInDirect(x+1, y+1, 0, W_CacheLumpName("LAME", PU_CACHE));
		}
		else
		{
			val = -val;
                        V_DrawPatchInDirect(x+18, y, 0, PatchINumbers[val]);
                        V_DrawPatchInDirect(x+9,  y, 0, PatchNEGATIVE);
		}
		return;
	}
	if(val > 99)
	{
		patch = PatchINumbers[val/100];
                V_DrawPatchInDirect(x, y, 0, patch);
	}
	val = val%100;
	if(val > 9 || oldval > 99)
	{
		patch = PatchINumbers[val/10];
                V_DrawPatchInDirect(x+9, y, 0, patch);
	}
	val = val%10;
	patch = PatchINumbers[val];
        V_DrawPatchInDirect(x+18, y, 0, patch);
}

//---------------------------------------------------------------------------
//
// PROC DrBNumber
//
// Draws a three digit number using FontB
//
//---------------------------------------------------------------------------

static void DrBNumber(signed int val, int x, int y)
{
	patch_t *patch;
	int xpos;
	int oldval;

	oldval = val;
	xpos = x;
	if(val < 0)
	{
		val = 0;
	}
	if(val > 99)
	{
		patch = W_CacheLumpNum(FontBNumBase+val/100, PU_CACHE);
		V_DrawPatchInDirect(xpos+6-patch->width/2, y, 0, patch);
	}
	val = val%100;
	xpos += 12;
	if(val > 9 || oldval > 99)
	{
		patch = W_CacheLumpNum(FontBNumBase+val/10, PU_CACHE);
		V_DrawPatchInDirect(xpos+6-patch->width/2, y, 0, patch);
	}
	val = val%10;
	xpos += 12;
	patch = W_CacheLumpNum(FontBNumBase+val, PU_CACHE);
	V_DrawPatchInDirect(xpos+6-patch->width/2, y, 0, patch);
}

//---------------------------------------------------------------------------
//
// PROC DrSmallNumber
//
// Draws a small two digit number.
//
//---------------------------------------------------------------------------

static void DrSmallNumber(int val, int x, int y)
{
	patch_t *patch;

	if(val == 1)
	{
		return;
	}
	if(val > 9)
	{
		patch = PatchSmNumbers[val/10];
                V_DrawPatchInDirect(x, y, 0,patch);
	}
	val = val%10;
	patch = PatchSmNumbers[val];
        V_DrawPatchInDirect(x+4, y, 0,patch);
}

//---------------------------------------------------------------------------
//
// PROC ShadeLine
//
//---------------------------------------------------------------------------

static void ShadeLine(int x, int y, int height, int shade)
{
	byte *dest;
	byte *shades;

	shades = colormaps+9*256+shade*2*256;
	dest = screens[1]+y*SCREENWIDTH+x;
	while(height--)
	{
		*(dest) = *(shades+*dest);
		dest += SCREENWIDTH;
	}
}

//---------------------------------------------------------------------------
//
// PROC ShadeChain
//
//---------------------------------------------------------------------------

static void ShadeChain(void)
{
	int i;

	for(i = 0; i < 16; i++)
	{
		ShadeLine(277+i, 190, 10, i/2);
		ShadeLine(19+i, 190, 10, 7-(i/2));
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawSoundInfo
//
// Displays sound debugging information.
//
//---------------------------------------------------------------------------

static void DrawSoundInfo(void)
{
	int i;
	SoundInfo_t s;
	ChanInfo_t *c;
	char text[32];
	int x;
	int y;
	int xPos[7] = {1, 75, 112, 156, 200, 230, 260};

	if(leveltime&16)
	{
		MN_DrTextA("*** SOUND DEBUG INFO ***", xPos[0], 20);
	}
//	S_GetChannelInfo(&s);
	if(s.channelCount == 0)
	{
		return;
	}
	x = 0;
	MN_DrTextA("NAME", xPos[x++], 30);
	MN_DrTextA("MO.T", xPos[x++], 30);
	MN_DrTextA("MO.X", xPos[x++], 30);
	MN_DrTextA("MO.Y", xPos[x++], 30);
	MN_DrTextA("ID", xPos[x++], 30);
	MN_DrTextA("PRI", xPos[x++], 30);
	MN_DrTextA("DIST", xPos[x++], 30);
	for(i = 0; i < s.channelCount; i++)
	{
		c = &s.chan[i];
		x = 0;
		y = 40+i*10;
		if(c->mo == NULL)
		{ // Channel is unused
			MN_DrTextA("------", xPos[0], y);
			continue;
		}
		sprintf(text, "%s", c->name);
		M_ForceUppercase(text);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->type);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->x>>FRACBITS);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->mo->y>>FRACBITS);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->id);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->priority);
		MN_DrTextA(text, xPos[x++], y);
		sprintf(text, "%d", c->distance);
		MN_DrTextA(text, xPos[x++], y);
	}
	UpdateState |= I_FULLSCRN;
	BorderNeedRefresh = true;
}

//---------------------------------------------------------------------------
//
// PROC SB_Drawer
//
//---------------------------------------------------------------------------

char patcharti[][10] =
{
	{"ARTIBOX"},    // none
	{"ARTIINVU"},   // invulnerability
	{"ARTIINVS"},   // invisibility
	{"ARTIPTN2"},   // health
	{"ARTISPHL"},   // superhealth
	{"ARTIPWBK"},   // tomeofpower
	{"ARTITRCH"},   // torch
	{"ARTIFBMB"},   // firebomb
	{"ARTIEGGC"},   // egg
	{"ARTISOAR"},   // fly
	{"ARTIATLP"}    // teleport
};

char ammopic[][10] =
{
	{"INAMGLD"},
	{"INAMBOW"},
	{"INAMBST"},
	{"INAMRAM"},
	{"INAMPNX"},
	{"INAMLOB"}
};

int SB_state = -1;
static int oldarti = 0;
static int oldartiCount = 0;
static int oldfrags = -9999;
static int oldammo = -1;
static int oldarmor = -1;
static int oldweapon = -1;
static int oldhealth = -1;
static int oldlife = -1;
static int oldkeys = -1;

int playerkeys = 0;

extern boolean automapactive;

void SB_Drawer(void)
{
	int frame;
	static boolean hitCenterFrame;

	// Sound info debug stuff
	if(DebugSound == true)
	{
		DrawSoundInfo();
	}
	CPlayer = &players[consoleplayer];

/*        if(viewheight == SCREENHEIGHT && !automapactive)
        {
		DrawFullScreenStuff();
                return;
                SB_state = -1;
        }
	else
	{
		if(SB_state == -1)
		{
                        V_DrawPatchInDirect(0, 158, 0,PatchBARBACK);
			if(players[consoleplayer].cheats&CF_GODMODE)
			{
                                V_DrawPatchInDirect(16, 167, 0, W_CacheLumpName("GOD1", PU_CACHE));
                                V_DrawPatchInDirect(287, 167, 0, W_CacheLumpName("GOD2", PU_CACHE));
			}
			oldhealth = -1;
		}
		DrawCommonBar();
		if(!inventory)
		{
			if(SB_state != 0)
			{
				// Main interface
                                V_DrawPatchInDirect(34, 160, 0, PatchSTATBAR);
				oldarti = 0;
				oldammo = -1;
				oldarmor = -1;
				oldweapon = -1;
				oldfrags = -9999; //can't use -1, 'cuz of negative frags
				oldlife = -1;
				oldkeys = -1;
			}
			DrawMainBar();
			SB_state = 0;
		}
		else
		{
			if(SB_state != 1)
			{
                                V_DrawPatchInDirect(34, 160, 0, PatchINVBAR);
			}
			DrawInventoryBar();
			SB_state = 1;
		}
	} */

        DrawFullScreenStuff();

	SB_PaletteFlash();

	// Flight icons
	if(CPlayer->powers[pw_flight])
	{
		if(CPlayer->powers[pw_flight] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_flight]&16))
		{
			frame = (leveltime/3)&15;
			if(CPlayer->mo->flags2&MF2_FLY)
			{
				if(hitCenterFrame && (frame != 15 && frame != 0))
				{
                                        V_DrawPatchInDirect(20, 17, 0, W_CacheLumpNum(spinflylump+15, 
						PU_CACHE));
				}
				else
				{
                                        V_DrawPatchInDirect(20, 17, 0, W_CacheLumpNum(spinflylump+frame, 
						PU_CACHE));
					hitCenterFrame = false;
				}
			}
			else
			{
				if(!hitCenterFrame && (frame != 15 && frame != 0))
				{
                                        V_DrawPatchInDirect(20, 17, 0, W_CacheLumpNum(spinflylump+frame, 
						PU_CACHE));
					hitCenterFrame = false;
				}
				else
				{
                                        V_DrawPatchInDirect(20, 17, 0, W_CacheLumpNum(spinflylump+15, 
						PU_CACHE));
					hitCenterFrame = true;
				}
			}
			BorderTopRefresh = true;
			UpdateState |= I_MESSAGES;
		}
		else
		{
			BorderTopRefresh = true;
			UpdateState |= I_MESSAGES;
		}
	}

	if(CPlayer->powers[pw_weaponlevel2] && !CPlayer->chickenTics)
	{
		if(CPlayer->powers[pw_weaponlevel2] > BLINKTHRESHOLD
			|| !(CPlayer->powers[pw_weaponlevel2]&16))
		{
			frame = (leveltime/3)&15;
                        V_DrawPatchInDirect(300, 17, 0, W_CacheLumpNum(spinbooklump+frame, PU_CACHE));
			BorderTopRefresh = true;
			UpdateState |= I_MESSAGES;
		}
		else
		{
			BorderTopRefresh = true;
			UpdateState |= I_MESSAGES;
		}
	}
}

// sets the new palette based upon current values of player->damagecount
// and player->bonuscount
void SB_PaletteFlash(void)
{
	static int sb_palette = 0;
	int palette;
	byte *pal;

	CPlayer = &players[consoleplayer];

	if(CPlayer->damagecount)
	{
		palette = (CPlayer->damagecount+7)>>3;
		if(palette >= NUMREDPALS)
		{
			palette = NUMREDPALS-1;
		}
		palette += STARTREDPALS;
	}
	else if(CPlayer->bonuscount)
	{
		palette = (CPlayer->bonuscount+7)>>3;
		if(palette >= NUMBONUSPALS)
		{
			palette = NUMBONUSPALS-1;
		}
		palette += STARTBONUSPALS;
	}
	else
	{
		palette = 0;
	}
	if(palette != sb_palette)
	{
		sb_palette = palette;
		pal = (byte *)W_CacheLumpNum(playpalette, PU_CACHE)+palette*768;
		I_SetPalette(pal);
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawCommonBar
//
//---------------------------------------------------------------------------

void DrawCommonBar(void)
{
	int chainY;
	int healthPos;

        V_DrawPatchInDirect(0, 148, 0, PatchLTFCTOP);
        V_DrawPatchInDirect(290, 148, 0, PatchRTFCTOP);

	if(oldhealth != HealthMarker)
	{
		oldhealth = HealthMarker;
		healthPos = HealthMarker;
		if(healthPos < 0)
		{
			healthPos = 0;
		}
		if(healthPos > 100)
		{
			healthPos = 100;
		}
		healthPos = (healthPos*256)/100;
		chainY = (HealthMarker == CPlayer->mo->health) ? 191 : 191+ChainWiggle;
                V_DrawPatchInDirect(0, 190, 0, PatchCHAINBACK);
                V_DrawPatchInDirect(2+(healthPos%17), chainY, 0, PatchCHAIN);
                V_DrawPatchInDirect(17+healthPos, chainY, 0,PatchLIFEGEM);
                V_DrawPatchInDirect(0, 190, 0, PatchLTFACE);
                V_DrawPatchInDirect(276, 190, 0, PatchRTFACE);
		ShadeChain();
		UpdateState |= I_STATBAR;
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawMainBar
//
//---------------------------------------------------------------------------

void DrawMainBar(void)
{
	int i;
	int temp;

	// Ready artifact
	if(ArtifactFlash)
	{
                V_DrawPatchInDirect(180, 161, 0, PatchBLACKSQ);
                V_DrawPatchInDirect(182, 161, 0, W_CacheLumpNum(W_GetNumForName("useartia")
			+ ArtifactFlash - 1, PU_CACHE));
		ArtifactFlash--;
		oldarti = -1; // so that the correct artifact fills in after the flash
		UpdateState |= I_STATBAR;
	}
	else if(oldarti != CPlayer->readyArtifact
		|| oldartiCount != CPlayer->inventory[inv_ptr].count)
	{
                V_DrawPatchInDirect(180, 161, 0, PatchBLACKSQ);
		if(CPlayer->readyArtifact > 0)
		{
                        V_DrawPatchInDirect(179,160, 0, W_CacheLumpName(patcharti[CPlayer->readyArtifact],
				PU_CACHE));
			DrSmallNumber(CPlayer->inventory[inv_ptr].count, 201, 182);
		}
		oldarti = CPlayer->readyArtifact;
		oldartiCount = CPlayer->inventory[inv_ptr].count;
		UpdateState |= I_STATBAR;
	}

	// Frags
	if(deathmatch)
	{
		temp = 0;
		for(i = 0; i < MAXPLAYERS; i++)
		{
			temp += CPlayer->frags[i];
		}
		if(temp != oldfrags)
		{
                        V_DrawPatchInDirect(57, 171, 0, PatchARMCLEAR);
			DrINumber(temp, 61, 170);
			oldfrags = temp;
			UpdateState |= I_STATBAR;
		}
	}
	else
	{
		temp = HealthMarker;
		if(temp < 0)
		{
			temp = 0;
		}
		else if(temp > 100)
		{
			temp = 100;
		}
		if(oldlife != temp)
		{
			oldlife = temp;
                        V_DrawPatchInDirect(57, 171, 0, PatchARMCLEAR);
			DrINumber(temp, 61, 170);
			UpdateState |= I_STATBAR;
		}
	}

	// Keys
	if(oldkeys != playerkeys)
	{
		if(CPlayer->keys[key_yellow])
		{
                        V_DrawPatchInDirect(153, 164, 0, W_CacheLumpName("ykeyicon", PU_CACHE));
		}
		if(CPlayer->keys[key_green])
		{
                        V_DrawPatchInDirect(153, 172, 0, W_CacheLumpName("gkeyicon", PU_CACHE));
		}
		if(CPlayer->keys[key_blue])
		{
                        V_DrawPatchInDirect(153, 180, 0, W_CacheLumpName("bkeyicon", PU_CACHE));
		}
		oldkeys = playerkeys;
		UpdateState |= I_STATBAR;
	}
	// Ammo
	temp = CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];
	if(oldammo != temp || oldweapon != CPlayer->readyweapon)
	{
                V_DrawPatchInDirect(108, 161, 0, PatchBLACKSQ);
		if(temp && CPlayer->readyweapon > 0 && CPlayer->readyweapon < 7)
		{
			DrINumber(temp, 109, 162);
                        V_DrawPatchInDirect(111, 172, 0, W_CacheLumpName(
				ammopic[CPlayer->readyweapon-1], PU_CACHE));
		}
		oldammo = temp;
		oldweapon = CPlayer->readyweapon;
		UpdateState |= I_STATBAR;
	}

	// Armor
	if(oldarmor != CPlayer->armorpoints)
	{
                V_DrawPatchInDirect(224, 171, 0, PatchARMCLEAR);
		DrINumber(CPlayer->armorpoints, 228, 170);
		oldarmor = CPlayer->armorpoints;
		UpdateState |= I_STATBAR;
	}
}

//---------------------------------------------------------------------------
//
// PROC DrawInventoryBar
//
//---------------------------------------------------------------------------

void DrawInventoryBar(void)
{
	int i;
	int x;

	x = inv_ptr-curpos;
	UpdateState |= I_STATBAR;
        V_DrawPatchInDirect(34, 160, 0, PatchINVBAR);
	for(i = 0; i < 7; i++)
	{
		//V_DrawPatch(50+i*31, 160, W_CacheLumpName("ARTIBOX", PU_CACHE));
		if(CPlayer->inventorySlotNum > x+i
			&& CPlayer->inventory[x+i].type != arti_none)
		{
                        V_DrawPatchInDirect(50+i*31, 160, 0, W_CacheLumpName(
				patcharti[CPlayer->inventory[x+i].type], PU_CACHE));
			DrSmallNumber(CPlayer->inventory[x+i].count, 69+i*31, 182);
		}
	}
        V_DrawPatchInDirect(50+curpos*31, 189, 0, PatchSELECTBOX);
	if(x != 0)
	{
                V_DrawPatchInDirect(38, 159, 0, !(leveltime&4) ? PatchINVLFGEM1 :
			PatchINVLFGEM2);
	}
	if(CPlayer->inventorySlotNum-x > 7)
	{
                V_DrawPatchInDirect(269, 159, 0, !(leveltime&4) ?
			PatchINVRTGEM1 : PatchINVRTGEM2);
	}
}

void DrawFullScreenStuff(void)
{
	int i;
	int x;
	int temp;

	UpdateState |= I_FULLSCRN;

        V_DrawPatchInDirect(240, 180, 0, W_CacheLumpName("_KEYS" , PU_CACHE));
        V_DrawPatchInDirect(250, 180, 0, W_CacheLumpName("_LIFE" , PU_CACHE));
        V_DrawPatchInDirect(285, 180, 0, W_CacheLumpName("_ARMOR", PU_CACHE));
        V_DrawPatchInDirect(285, 160, 0, W_CacheLumpName("_AMMO" , PU_CACHE));

// KEYS
        if(CPlayer->keys[key_yellow])
          {
           V_DrawPatchInDirect(241, 181, 0, W_CacheLumpName("ykeyicon", PU_CACHE));
          }
        if(CPlayer->keys[key_green])
          {
           V_DrawPatchInDirect(241, 188, 0, W_CacheLumpName("gkeyicon", PU_CACHE));
          }
        if(CPlayer->keys[key_blue])
          {
           V_DrawPatchInDirect(241, 195, 0, W_CacheLumpName("bkeyicon", PU_CACHE));
          }



// HEALTH 
                if(CPlayer->mo->health > 0)
                  { DrINumber(CPlayer->mo->health, 255, 188); }
                else
                  { DrINumber(0, 255, 188); }

// ARMOR
                DrINumber(CPlayer->armorpoints, 255+35, 188);
		UpdateState |= I_STATBAR;

// AMMO
                temp = CPlayer->ammo[wpnlev1info[CPlayer->readyweapon].ammo];
		if(temp && CPlayer->readyweapon > 0 && CPlayer->readyweapon < 7)
		{
                            DrINumber(temp, 255+35, 188-20);
		}
		UpdateState |= I_STATBAR;





        if(deathmatch)
	{
		temp = 0;
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(playeringame[i])
			{
				temp += CPlayer->frags[i];
			}
		}
		DrINumber(temp, 45, 185);
	}


	if(!inventory)
	{
		if(CPlayer->readyArtifact > 0)
		{
                        V_DrawPatchInDirect(0, 170, 0, W_CacheLumpName("ARTIBOX",
				PU_CACHE));
                        V_DrawPatchInDirect(0, 170, 0,
                                W_CacheLumpName(patcharti[CPlayer->readyArtifact], PU_CACHE));
                        DrSmallNumber(CPlayer->inventory[inv_ptr].count, 21, 193);
		}
	}
	else
	{
		x = inv_ptr-curpos;
		for(i = 0; i < 7; i++)
		{
                        V_DrawPatchInDirect(50+i*31, 168, 0, W_CacheLumpName("ARTIBOX",
				PU_CACHE));
			if(CPlayer->inventorySlotNum > x+i
				&& CPlayer->inventory[x+i].type != arti_none)
			{
                                V_DrawPatchInDirect(50+i*31, 168, 0, W_CacheLumpName(
					patcharti[CPlayer->inventory[x+i].type], PU_CACHE));
				DrSmallNumber(CPlayer->inventory[x+i].count, 69+i*31, 190);
			}
		}

                V_DrawPatchInDirect(50+curpos*31, 197, 0, PatchSELECTBOX);
		if(x != 0)
		{
                        V_DrawPatchInDirect(38, 167, 0, !(leveltime&4) ? PatchINVLFGEM1 :
				PatchINVLFGEM2);
		}
		if(CPlayer->inventorySlotNum-x > 7)
		{
                        V_DrawPatchInDirect(269, 167, 0, !(leveltime&4) ?
				PatchINVRTGEM1 : PatchINVRTGEM2);
		}
	}
}

//--------------------------------------------------------------------------
//
// FUNC SB_Responder
//
//--------------------------------------------------------------------------

boolean SB_Responder(event_t *event)
{
	if(event->type == ev_keydown)
	{
		if(HandleCheats(event->data1))
		{ // Need to eat the key
			return(true);
		}
	}
	return(false);
}

//--------------------------------------------------------------------------
//
// FUNC HandleCheats
//
// Returns true if the caller should eat the key.
//
//--------------------------------------------------------------------------

static boolean HandleCheats(byte key)
{
	int i;
	boolean eat;

	if(netgame || gameskill == sk_nightmare)
	{ // Can't cheat in a net-game, or in nightmare mode
		return(false);
	}
	if(players[consoleplayer].health <= 0)
	{ // Dead players can't cheat
		return(false);
	}
	eat = false;
	for(i = 0; Cheats[i].func != NULL; i++)
	{
		if(CheatAddKey(&Cheats[i], key, &eat))
		{
			Cheats[i].func(&players[consoleplayer], &Cheats[i]);
			S_StartSound(NULL, sfx_dorcls);
		}
	}
	return(eat);
}

//--------------------------------------------------------------------------
//
// FUNC CheatAddkey
//
// Returns true if the added key completed the cheat, false otherwise.
//
//--------------------------------------------------------------------------

static boolean CheatAddKey(Cheat_t *cheat, byte key, boolean *eat)
{
	if(!cheat->pos)
	{
		cheat->pos = cheat->sequence;
		cheat->currentArg = 0;
	}
	if(*cheat->pos == 0)
	{
		*eat = true;
		cheat->args[cheat->currentArg++] = key;
		cheat->pos++;
	}
	else if(CheatLookup[key] == *cheat->pos)
	{
		cheat->pos++;
	}
	else
	{
		cheat->pos = cheat->sequence;
		cheat->currentArg = 0;
	}
	if(*cheat->pos == 0xff)
	{
		cheat->pos = cheat->sequence;
		cheat->currentArg = 0;
		return(true);
	}
	return(false);
}

//--------------------------------------------------------------------------
//
// CHEAT FUNCTIONS
//
//--------------------------------------------------------------------------

static void CheatGodFunc(player_t *player, Cheat_t *cheat)
{
	player->cheats ^= CF_GODMODE;
	if(player->cheats&CF_GODMODE)
	{
		P_SetMessage(player, TXT_CHEATGODON, false);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATGODOFF, false);
	}
        SB_state = -1;
}

static void CheatNoClipFunc(player_t *player, Cheat_t *cheat)
{
	player->cheats ^= CF_NOCLIP;
	if(player->cheats&CF_NOCLIP)
	{
		P_SetMessage(player, TXT_CHEATNOCLIPON, false);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATNOCLIPOFF, false);
	}
}

static void CheatWeaponsFunc(player_t *player, Cheat_t *cheat)
{
	int i;
	//extern boolean *WeaponInShareware;

	player->armorpoints = 200;
	player->armortype = 2;
	if(!player->backpack)
	{
		for(i = 0; i < NUMAMMO; i++)
		{
			player->maxammo[i] *= 2;
		}
		player->backpack = true;
	}
	for(i = 0; i < NUMWEAPONS-1; i++)
	{
		player->weaponowned[i] = true;
	}
	if(shareware)
	{
		player->weaponowned[wp_skullrod] = false;
		player->weaponowned[wp_phoenixrod] = false;
		player->weaponowned[wp_mace] = false;
	}
	for(i = 0; i < NUMAMMO; i++)
	{
		player->ammo[i] = player->maxammo[i];
	}
	P_SetMessage(player, TXT_CHEATWEAPONS, false);
}

static void CheatPowerFunc(player_t *player, Cheat_t *cheat)
{
	if(player->powers[pw_weaponlevel2])
	{
		player->powers[pw_weaponlevel2] = 0;
		P_SetMessage(player, TXT_CHEATPOWEROFF, false);
	}
	else
	{
		P_UseArtifact(player, arti_tomeofpower);
		P_SetMessage(player, TXT_CHEATPOWERON, false);
	}
}

static void CheatHealthFunc(player_t *player, Cheat_t *cheat)
{
	if(player->chickenTics)
	{
		player->health = player->mo->health = MAXCHICKENHEALTH;
	}
	else
	{
		player->health = player->mo->health = MAXHEALTH;
	}
	P_SetMessage(player, TXT_CHEATHEALTH, false);
}

static void CheatKeysFunc(player_t *player, Cheat_t *cheat)
{
	extern int playerkeys;

	player->keys[key_yellow] = true;
	player->keys[key_green] = true;
	player->keys[key_blue] = true;
	playerkeys = 7; // Key refresh flags
	P_SetMessage(player, TXT_CHEATKEYS, false);
}

static void CheatSoundFunc(player_t *player, Cheat_t *cheat)
{
	DebugSound = !DebugSound;
	if(DebugSound)
	{
		P_SetMessage(player, TXT_CHEATSOUNDON, false);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATSOUNDOFF, false);
	}
}

static void CheatTickerFunc(player_t *player, Cheat_t *cheat)
{
	extern int DisplayTicker;

	DisplayTicker = !DisplayTicker;
	if(DisplayTicker)
	{
		P_SetMessage(player, TXT_CHEATTICKERON, false);
	}
	else
	{
		P_SetMessage(player, TXT_CHEATTICKEROFF, false);
	}
}

static void CheatArtifact1Func(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, TXT_CHEATARTIFACTS1, false);
}

static void CheatArtifact2Func(player_t *player, Cheat_t *cheat)
{
	P_SetMessage(player, TXT_CHEATARTIFACTS2, false);
}

static void CheatArtifact3Func(player_t *player, Cheat_t *cheat)
{
	int i;
	int j;
	artitype_t type;
	int count;

	type = cheat->args[0]-'a'+1;
	count = cheat->args[1]-'0';
	if(type == 26 && count == 0)
	{ // All artifacts
		for(i = arti_none+1; i < NUMARTIFACTS; i++)
		{
			if(shareware && (i == arti_superhealth
				|| i == arti_teleport))
			{
				continue;
			}
			for(j = 0; j < 16; j++)
			{
				P_GiveArtifact(player, i, NULL);
			}
		}
		P_SetMessage(player, TXT_CHEATARTIFACTS3, false);
	}
	else if(type > arti_none && type < NUMARTIFACTS
		&& count > 0 && count < 10)
	{
		if(shareware && (type == arti_superhealth || type == arti_teleport))
		{
			P_SetMessage(player, TXT_CHEATARTIFACTSFAIL, false);
			return;
		}
		for(i = 0; i < count; i++)
		{
			P_GiveArtifact(player, type, NULL);
		}
		P_SetMessage(player, TXT_CHEATARTIFACTS3, false);
	}
	else
	{ // Bad input
		P_SetMessage(player, TXT_CHEATARTIFACTSFAIL, false);
	}
}

static void CheatWarpFunc(player_t *player, Cheat_t *cheat)
{
	int episode;
	int map;

	episode = cheat->args[0]-'0';
	map = cheat->args[1]-'0';
	if(M_ValidEpisodeMap(episode, map))
	{
		G_DeferedInitNew(gameskill, episode, map);
		P_SetMessage(player, TXT_CHEATWARP, false);
	}
}

static void CheatChickenFunc(player_t *player, Cheat_t *cheat)
{
	extern boolean P_UndoPlayerChicken(player_t *player);

	if(player->chickenTics)
	{
		if(P_UndoPlayerChicken(player))
		{
			P_SetMessage(player, TXT_CHEATCHICKENOFF, false);
		}
	}
	else if(P_ChickenMorphPlayer(player))
	{
		P_SetMessage(player, TXT_CHEATCHICKENON, false);
	}
}

static void CheatMassacreFunc(player_t *player, Cheat_t *cheat)
{
	P_Massacre();
	P_SetMessage(player, TXT_CHEATMASSACRE, false);
}

static void CheatIDKFAFunc(player_t *player, Cheat_t *cheat)
{
	int i;
	if(player->chickenTics)
	{
		return;
	}
	for(i = 1; i < 8; i++)
	{
		player->weaponowned[i] = false;
	}
	player->pendingweapon = wp_staff;
	P_SetMessage(player, TXT_CHEATIDKFA, true);
}

static void CheatIDDQDFunc(player_t *player, Cheat_t *cheat)
{
	P_DamageMobj(player->mo, NULL, player->mo, 10000);
	P_SetMessage(player, TXT_CHEATIDDQD, true);
}
