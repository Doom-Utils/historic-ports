
// sounds.h

#ifndef __SOUNDSH__
#define __SOUNDSH__

#define MAX_SND_DIST 	1600
#define MAX_CHANNELS	32

typedef struct sfxinfo_struct	sfxinfo_t;

struct sfxinfo_struct
{
    // up to 6-character name
    char*	name;

    // Sfx singularity (only one at a time)
    int		singularity;

    // Sfx priority
    int		priority;

    // referenced sound if a link
    sfxinfo_t*	link;

    // pitch if a link
    int		pitch;

    // volume if a link
    int		volume;

    // sound data
    SAMPLE*	data;

    // this is checked every second to see if sound
    // can be thrown out (if 0, then decrement, if -1,
    // then throw out, if > 0, then it is in use)
    int		usefulness;

    // lump number of sfx
    int		lumpnum;		
};



// Music identifiers

typedef enum
{
        mus_None,
        mus_e1m1,
	mus_e1m2,
	mus_e1m3,
	mus_e1m4,
	mus_e1m5,
	mus_e1m6,
	mus_e1m7,
	mus_e1m8,
	mus_e1m9,

	mus_e2m1,
	mus_e2m2,
	mus_e2m3,
	mus_e2m4,
	mus_e2m5,
	mus_e2m6,
	mus_e2m7,
	mus_e2m8,
	mus_e2m9,

	mus_e3m1,
	mus_e3m2,
	mus_e3m3,
	mus_e3m4,
	mus_e3m5,
	mus_e3m6,
	mus_e3m7,
	mus_e3m8,
	mus_e3m9,

	mus_e4m1,
	mus_e4m2,
	mus_e4m3,
	mus_e4m4,
	mus_e4m5,
	mus_e4m6,
	mus_e4m7,
	mus_e4m8,
	mus_e4m9,

	mus_e5m1,
	mus_e5m2,
	mus_e5m3,
	mus_e5m4,
	mus_e5m5,
	mus_e5m6,
	mus_e5m7,
	mus_e5m8,
	mus_e5m9,

	mus_e6m1,
	mus_e6m2,
	mus_e6m3,

	mus_titl,
	mus_intr,
	mus_cptd,
	NUMMUSIC
} musicenum_t;

//
// MusicInfo struct.
//
typedef struct
{
    // up to 6-character name
    char*	name;

    // lump number of music
    int		lumpnum;
    
    // music data
    void*	data;

    // music handle once registered
    int handle;
    
} musicinfo_t;


typedef struct
{
	long id;
	unsigned short priority;
	char *name;
	mobj_t *mo;
	int distance;
} ChanInfo_t;


/*typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t*	sfxinfo;

    // origin of sound
    void*	origin;

    // handle of the sound being played
    SAMPLE*		handle;
    
} channel_t;*/


typedef	struct
{
	int channelCount;
	int musicVolume;
	int soundVolume;
	ChanInfo_t chan[8];
} SoundInfo_t;

// Sound identifiers

typedef enum
{
  sfx_None          ,
  sfx_gldhit        ,
  sfx_gntful        ,
  sfx_gnthit        ,
  sfx_gntpow        ,
  sfx_gntact        ,
  sfx_gntuse        ,
  sfx_phosht        ,
  sfx_phopow        ,
  sfx_lobsht        ,
  sfx_lobhit        ,
  sfx_lobpow        ,
  sfx_hrnsht        ,
  sfx_hrnhit        ,
  sfx_hrnpow        ,
  sfx_ramphit       ,
  sfx_ramrain       ,
  sfx_bowsht        ,
  sfx_stfhit        ,
  sfx_stfpow        ,
  sfx_impsit        ,
  sfx_impat1        ,
  sfx_impat2        ,
  sfx_impdth        ,
  sfx_impact        ,
  sfx_imppai        ,
  sfx_mumsit        ,
  sfx_mumat1        ,
  sfx_mumat2        ,
  sfx_mumdth        ,
  sfx_mumact        ,
  sfx_mumpai        ,
  sfx_mumhed        ,
  sfx_bstsit        ,
  sfx_bstatk        ,
  sfx_bstdth        ,
  sfx_bstact        ,
  sfx_bstpai        ,
  sfx_clksit        ,
  sfx_clkatk        ,
  sfx_clkdth        ,
  sfx_clkact        ,
  sfx_clkpai        ,
  sfx_snksit        ,
  sfx_snkatk        ,
  sfx_snkdth        ,
  sfx_snkact        ,
  sfx_snkpai        ,
  sfx_kgtsit        ,
  sfx_kgtatk        ,
  sfx_kgtat2        ,
  sfx_kgtdth        ,
  sfx_kgtact        ,
  sfx_kgtpai        ,
  sfx_wizsit        ,
  sfx_wizatk        ,
  sfx_wizdth        ,
  sfx_wizact        ,
  sfx_wizpai        ,
  sfx_minsit        ,
  sfx_minat1        ,
  sfx_minat2        ,
  sfx_minat3        ,
  sfx_mindth        ,
  sfx_minact        ,
  sfx_minpai        ,
  sfx_hedsit        ,
  sfx_hedat1        ,
  sfx_hedat2        ,
  sfx_hedat3        ,
  sfx_heddth        ,
  sfx_hedact        ,
  sfx_hedpai        ,
  sfx_sorzap        ,
  sfx_sorrise       ,
  sfx_sorsit        ,
  sfx_soratk        ,
  sfx_soract        ,
  sfx_sorpai        ,
  sfx_sordsph       ,
  sfx_sordexp       ,
  sfx_sordbon       ,
  sfx_sbtsit        ,
  sfx_sbtatk        ,
  sfx_sbtdth        ,
  sfx_sbtact        ,
  sfx_sbtpai        ,
  sfx_plroof        ,
  sfx_plrpai        ,
  sfx_plrdth        ,
  sfx_gibdth        ,
  sfx_plrwdth       ,
  sfx_plrcdth       ,
  sfx_itemup        ,
  sfx_wpnup         ,
  sfx_telept        ,
  sfx_doropn        ,
  sfx_dorcls        ,
  sfx_dormov        ,
  sfx_artiup        ,
  sfx_switch        ,
  sfx_pstart        ,
  sfx_pstop         ,
  sfx_stnmov        ,
  sfx_chicpai       ,
  sfx_chicatk       ,
  sfx_chicdth       ,
  sfx_chicact       ,
  sfx_chicpk1       ,
  sfx_chicpk2       ,
  sfx_chicpk3       ,
  sfx_keyup         ,
  sfx_ripslop       ,
  sfx_newpod        ,
  sfx_podexp        ,
  sfx_bounce        ,
  sfx_volsht        ,
  sfx_volhit        ,
  sfx_burn          ,
// { "splash", false,  64, 0, -1, -1, 0 },
  sfx_gloop         ,
  sfx_respawn       ,
  sfx_blssht        ,
  sfx_blshit        ,
  sfx_chat          ,
  sfx_artiuse       ,
  sfx_gfrag         ,
  sfx_waterfl       ,
                
  // Monophonic sounds

  sfx_wind          ,
  sfx_amb1          ,
  sfx_amb2          ,
  sfx_amb3          ,
  sfx_amb4          ,
  sfx_amb5          ,
  sfx_amb7          ,
  sfx_amb8          ,
  sfx_amb9          ,
  sfx_amb10         ,
  sfx_amb11         ,
  NUMSFX
} sfxenum_t;

// Identifiers for the game sound/music

extern sfxinfo_t        S_sfx[];
extern musicinfo_t      S_music[];


#endif
