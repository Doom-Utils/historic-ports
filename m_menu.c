//  
// DOSDoom Main Menu Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// See M_Option.C for text built menus.
//
// -KM- 1998/07/21 Add support for message input.

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>

#include "dm_defs.h"
#include "dm_state.h"

#include "d_debug.h"
#include "d_main.h"
#include "ddf_main.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "lu_sound.h"
#include "m_argv.h"
#include "m_menu.h"
#include "m_option.h"
#include "m_swap.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_res.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

extern patch_t*		hu_font[HU_FONTSIZE];
extern boolean		message_dontfuckwithme;

extern boolean		chat_on;		// in heads-up code

//
// defaulted values
//
int			mouseSensitivity;       // has default

// Show messages has default, 0 = off, 1 = on
int			showMessages;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel;		
int			screenblocks;		// has default

int			darken_screen;

// temp for screenblocks (0-9)
int			screenSize;		

// -1 = no quicksave slot picked!
int			quickSaveSlot;
// 25-6-98 KM
int			quickSavePage;
// 25-6-98 KM Lots of save games... :-)
int			savegamePage = 0;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
char*			messageString;
// -KM- 1998/07/21  This string hold what the user has typed in
char*                   messageInputString;

// message x & y
int			messx;			
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;     

void    (*messageRoutine)(int response);

#define SAVESTRINGSIZE 	24

char* gammamsg[5];

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean			inhelpscreens;
boolean			menuactive;

#define SKULLXOFF		-32
#define LINEHEIGHT		16

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

// 98-7-10 KM New defines for slider left
// Part of savegame changes.
#define SLIDERLEFT -1
#define SLIDERRIGHT -2

//
// MENU TYPEDEFS
//
typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short	status;
    
    char	name[10];
    
    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void	(*routine)(int choice);
    
    // hotkey in menu
    char	alphaKey;			
} menuitem_t;



typedef struct menu_s
{
    short		numitems;	// # of menu items
    struct menu_s*	prevMenu;	// previous menu
    menuitem_t*		menuitems;	// menu items
    void		(*routine)();	// draw routine
    short		x;
    short		y;		// x,y of menu
    short		lastOn;		// last item user was on in menu
} menu_t;

short		itemOn;			// menu item skull is on
short		skullAnimCounter;	// skull animation counter
short		whichSkull;		// which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    skullName[2][9] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t*	currentMenu;                          

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
// 25-6-98 KM
void M_LoadSavePage(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, char *string);
void M_WriteTextTrans( int x, int y, int index, char* string);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartControlPanel(void);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);




//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'r'},
    {1,"M_QUITG",M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};


//
// EPISODE SELECT
//
/*
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep5,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'f'},
    {1,"M_EPI5", M_Episode,'h'}
};
*/
// -KM- 1998/12/16 This is generated dynamically.
menuitem_t* EpisodeMenu = NULL;
menu_t  EpiDef =
{
    0, //ep_end,		// # of menu items
    &MainDef,		// previous menu
    NULL,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    0			// lastOn
};

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {1,"M_JKILL",	M_ChooseSkill, 'i'},
    {1,"M_ROUGH",	M_ChooseSkill, 'h'},
    {1,"M_HURT",	M_ChooseSkill, 'h'},
    {1,"M_ULTRA",	M_ChooseSkill, 'u'},
    {1,"M_NMARE",	M_ChooseSkill, 'n'}
};

menu_t  NewDef =
{
    newg_end,		// # of menu items
    &EpiDef,		// previous menu
    NewGameMenu,	// menuitem_t ->
    M_DrawNewGame,	// drawing routine ->
    48,63,              // x,y
    hurtme		// lastOn
};



//
// OPTIONS MENU
//
enum
{
    endgame,
    messages,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"M_ENDGAM",	M_EndGame,'e'},
    {1,"M_MESSG",	M_ChangeMessages,'m'},
    {1,"M_DETAIL",	M_ChangeDetail,'g'},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s'},
    {-1,"",0},
    {2,"M_MSENS",	M_ChangeSensitivity,'m'},
    {-1,"",0},
    {1,"M_SVOL",	M_Sound,'s'}
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};

//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};

//
// SOUND VOLUME MENU
//
enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0}
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,64,
    0
};

//
// LOAD GAME MENU
//
// 98-7-10 KM Note extra 2 slots per page
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {2,"", M_LoadSelect,'1'},
    {2,"", M_LoadSelect,'2'},
    {2,"", M_LoadSelect,'3'},
    {2,"", M_LoadSelect,'4'},
    {2,"", M_LoadSelect,'5'},
    {2,"", M_LoadSelect,'6'},
    {2,"", M_LoadSelect,'7'},
    {2,"", M_LoadSelect,'8'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,34,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {2,"", M_SaveSelect,'1'},
    {2,"", M_SaveSelect,'2'},
    {2,"", M_SaveSelect,'3'},
    {2,"", M_SaveSelect,'4'},
    {2,"", M_SaveSelect,'5'},
    {2,"", M_SaveSelect,'6'},
    {2,"", M_SaveSelect,'7'},
    {2,"", M_SaveSelect,'8'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,34,
    0
};

// 98-7-10 KM Chooses the page of savegames to view
void M_LoadSavePage(int choice)
{
  switch (choice)
  {
     case SLIDERLEFT:
          savegamePage -= 8;
          if (savegamePage < 0)
          {
            savegamePage = 0;
            return;
          }
          break;
     case SLIDERRIGHT:
          savegamePage += 8;
          if (savegamePage >= 65536)
          {
            savegamePage = 65536 - 8;
            return;
          }
          break;
  }
  S_StartSound(NULL,sfx_swtchn);
  M_ReadSaveStrings();
}
//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
// 98-7-10 KM Savegame slots increased
void M_ReadSaveStrings(void)
{
    int             handle;
    int             count;
    int             i;
    char    name[256];

    for (i = 0;i < (load_end - 1);i++)
    {
        if (netgame)
	  sprintf(name,NETSAVEDIR"/"SAVEGAMENAME"%04x.dsg",i + savegamePage);
        else
          sprintf(name,SAVEGAMEDIR"/"SAVEGAMENAME"%04x.dsg", i + savegamePage);

	handle = open (name, O_RDONLY | 0, 0666);
	if (handle == -1)
	{
	    strcpy(&savegamestrings[i][0],DDF_LanguageLookup("EmptySlot"));
	    LoadMenu[i].status = 0;
	    continue;
	}
	count = read (handle, &savegamestrings[i], SAVESTRINGSIZE);
	close (handle);
	LoadMenu[i].status = 1;
    }
}


//
// M_LoadGame & Cie.
//
// 98-7-10 KM Savegame slots increased
void M_DrawLoad(void)
{
    int             i;
    char	    pagenum[8];

    V_DrawPatchInDirect(72,8,0,W_CacheLumpName("M_LOADG",PU_CACHE));

    for (i = 0;i < (load_end - 1); i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);

	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    // -KM-  1998/06/25 This could quite possibly be replaced by some graphics...
    M_WriteText(LoadDef.x-4,LoadDef.y+LINEHEIGHT*i,"< Prev");

    sprintf(pagenum, "%d", (savegamePage / 8) + 1);

    M_WriteText(LoadDef.x + 94 - M_StringWidth(pagenum) / 2,
                      LoadDef.y+LINEHEIGHT*i, pagenum);

    M_WriteText(LoadDef.x + 192 - M_StringWidth("Next >") ,
                      LoadDef.y+LINEHEIGHT*i, "Next >");
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;
	
    V_DrawPatchInDirect (x-8,y+7,0,W_CacheLumpName("M_LSLEFT",PU_CACHE));
	
    for (i = 0;i < 24;i++)
    {
	V_DrawPatchInDirect (x,y+7,0,W_CacheLumpName("M_LSCNTR",PU_CACHE));
	x += 8;
    }

    V_DrawPatchInDirect (x,y+7,0,W_CacheLumpName("M_LSRGHT",PU_CACHE));
}



//
// User wants to load this game
//
// 98-7-10 KM Savegame slots increased
void M_LoadSelect(int choice)
{
    char    name[256];
	
    if (choice < 0)
    {
      M_LoadSavePage(choice);
      return;
    }
    if (netgame)
        sprintf(name,NETSAVEDIR"/"SAVEGAMENAME"%04x.dsg",choice + savegamePage);
    else
        sprintf(name,SAVEGAMEDIR"/"SAVEGAMENAME"%04x.dsg",choice + savegamePage);
    G_LoadGame (name);
    M_ClearMenus ();
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
	M_StartMessage(DDF_LanguageLookup("NoLoadInNetGame"),NULL,false);
	return;
    }
	
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
// 98-7-10 KM Savegame slots increased
void M_DrawSave(void)
{
    int             i;
    char	    pagenum[8];
	
    V_DrawPatchInDirect(72,8,0,W_CacheLumpName("M_SAVEG",PU_CACHE));

    for (i = 0;i < (load_end - 1); i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);

        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    // -KM-  1998/06/25 This could quite possibly be replaced by some graphics...
    M_WriteText(LoadDef.x-4,LoadDef.y+LINEHEIGHT*i,"< Prev");

    sprintf(pagenum, "%d", (savegamePage / 8) + 1);
    M_WriteText(LoadDef.x + 94 - M_StringWidth(pagenum) / 2,
                      LoadDef.y+LINEHEIGHT*i, pagenum);

    M_WriteText(LoadDef.x + 192 - M_StringWidth("Next >") ,
                      LoadDef.y+LINEHEIGHT*i,"Next >");

    if (saveStringEnter)
    {
	i = M_StringWidth(savegamestrings[saveSlot]);

	M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
// 98-7-10 KM Savegame slots increased
{
    G_SaveGame (slot + savegamePage,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
    {
	quickSaveSlot = slot;
        quickSavePage = savegamePage;
    }
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    if (choice < 0)
    {
      M_LoadSavePage(choice);
      return;
    }
    // we are going to be intercepting all chars
    saveStringEnter = 1;
    
    saveSlot = choice;
    strcpy(saveOldString,savegamestrings[choice]);
    if (!strcmp(savegamestrings[choice],DDF_LanguageLookup("EmptySlot")))
	savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
	M_StartMessage(DDF_LanguageLookup("SaveWhenNotPlaying"),NULL,false);
	return;
    }
	
    if (gamestate != GS_LEVEL)
	return;
	
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}



//
//      M_QuickSave
//
char    tempstring[80];

// 98-7-10 KM Savegame slots increased
void M_QuickSaveResponse(int ch)
{
    if (ch == 'y')
    {
        int tempsavepage = savegamePage;
        savegamePage = quickSavePage;
	M_DoSave(quickSaveSlot);
        savegamePage = tempsavepage;
	S_StartSound(NULL,sfx_swtchx);
    }
}

// 98-7-10 KM Savegame slots increased
void M_QuickSave(void)
{
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    if (gamestate != GS_LEVEL)
	return;
	
    if (quickSaveSlot < 0)
    {
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&SaveDef);
	quickSaveSlot = -2;	// means to pick a slot now
	return;
    }
    sprintf(tempstring,DDF_LanguageLookup("QuickSaveOver"),
             savegamestrings[quickSaveSlot]);

    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoad
//
// 98-7-10 KM Savegame slots increased
void M_QuickLoadResponse(int ch)
{
    if (ch == 'y')
    {
        int savepage = savegamePage;
        savegamePage = quickSavePage;
	M_LoadSelect(quickSaveSlot);
        savegamePage = savepage;
	S_StartSound(NULL,sfx_swtchx);
    }
}


// 98-7-10 KM Savegame slots increased
void M_QuickLoad(void)
{
    if (netgame)
    {
	M_StartMessage(DDF_LanguageLookup("NoQLoadInNet"),NULL,false);
	return;
    }
	
    if (quickSaveSlot < 0)
    {
	M_StartMessage(DDF_LanguageLookup("NoQuickSaveSlot"),NULL,false);
	return;
    }
    sprintf(tempstring,DDF_LanguageLookup("QuickLoad"),
             savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;
    // -KM- 1998/07/21 Erase the background behind the pic
    V_ClearPageBackground(0);
    if (W_CheckNumForName("HELP") < 0)
      V_DrawPatchInDirect (0,0,0,W_CacheLumpName("HELP1",PU_CACHE));
    else
      V_DrawPatchInDirect (0,0,0,W_CacheLumpName("HELP", PU_CACHE));
    return;
}



//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;
    // -KM- 1998/07/21 Erase the background behind the pic
    V_ClearPageBackground(0);
    if (W_CheckNumForName("HELP2") < 0)
	V_DrawPatchInDirect (0,0,0,W_CacheLumpName("CREDIT",PU_CACHE));
    else
	V_DrawPatchInDirect (0,0,0,W_CacheLumpName("HELP2",PU_CACHE));
    return;
}


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    V_DrawPatchInDirect(60,38,0,W_CacheLumpName("M_SVOL",PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),16,snd_SfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),16,snd_MusicVolume);
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

// 98-7-10 KM Use new defines
void M_SfxVol(int choice)
{
    switch(choice)
    {
      case SLIDERLEFT:
	if (snd_SfxVolume)
	    snd_SfxVolume--;
	break;
      case SLIDERRIGHT:
	if (snd_SfxVolume < 15)
	    snd_SfxVolume++;
	break;
    }
	
    S_SetSfxVolume(snd_SfxVolume /* *8 */);
}

void M_MusicVol(int choice)
// 98-7-10 KM Use new defines
{
    switch(choice)
    {
      case SLIDERLEFT:
	if (snd_MusicVolume)
	    snd_MusicVolume--;
	break;
      case SLIDERRIGHT:
	if (snd_MusicVolume < 15)
	    snd_MusicVolume++;
	break;
    }
	
    S_SetMusicVolume(snd_MusicVolume);
}




//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    V_DrawPatchInDirect(94,2,0,W_CacheLumpName("M_DOOM",PU_CACHE));
}


//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawPatchInDirect(96,14,0,W_CacheLumpName("M_NEWG",PU_CACHE));

    V_DrawPatchInDirect(54,38,0,W_CacheLumpName("M_SKILL",PU_CACHE));
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
	M_StartMessage(DDF_LanguageLookup("NewNetGame"),NULL,false);
	return;
    }
	
    //if (gamemode == commercial)
//      M_SetupNextMenu(&NewDef);
    //else
      M_SetupNextMenu(&EpiDef);
}


//
//      M_Episode
//
int     epi;

// -KM- 1998/12/16 Generates EpiDef menu dynamically.
void M_DrawEpisode(void)
{
    int i, j, k, e;
    char alpha;
    V_DrawPatchInDirect(54,38,0,W_CacheLumpName("M_EPISOD",PU_CACHE));

    if (!EpisodeMenu)
    {
      EpisodeMenu = Z_Malloc(sizeof(menuitem_t) * wi_nummaps, PU_STATIC, NULL);
      for (i = 0, e = 0; i < wi_nummaps; i++)
      {
         if (W_CheckNumForName(wi_maps[i].firstmap) == -1)
           continue;

         k = 0;
         EpisodeMenu[e].status = 1;
         strncpy(EpisodeMenu[e].name, wi_maps[i].graphicname, 8);
         EpisodeMenu[e].routine = M_Episode;
         alpha = EpisodeMenu[e].name[0];
         for (j = 0; j < e; j++)
         {
           if (alpha == EpisodeMenu[j].alphaKey)
           {
             for (; EpisodeMenu[e].name[k] && EpisodeMenu[e].name[k] != ' '; k++);
             k++;
             if (EpisodeMenu[e].name[k])
               alpha = EpisodeMenu[e].name[k];
             j = 0;
           }
         }
         EpisodeMenu[e].alphaKey = alpha;
         e++;
      }
      EpiDef.numitems = e;
      EpiDef.menuitems = EpisodeMenu;
    }
}

void M_VerifyNightmare(int ch)
{
    int i;
    if (ch != 'y')
	return;
		
    // -KM- 1998/12/17 Clear the intermission.
    WI_MapInit(NULL);
    for (i = 0; strcmp(wi_maps[i].graphicname, EpisodeMenu[epi].name); i++);
    if (G_DeferedInitNew(nightmare, wi_maps[i].firstmap))
    {
      // 23-6-98 KM Fixed this.
      M_SetupNextMenu(&EpiDef);
      M_StartMessage(DDF_LanguageLookup("EpisodeNonExist"), NULL, false);
      return;
    }

   M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    int i;
    if (choice == nightmare)
    {
	M_StartMessage(DDF_LanguageLookup("NightMareCheck"),M_VerifyNightmare,true);
	return;
    }
    // -KM- 1998/12/17 Clear the intermission
    WI_MapInit(NULL);
    for (i = 0; strcmp(wi_maps[i].graphicname, EpisodeMenu[epi].name); i++);
    if (G_DeferedInitNew(choice, wi_maps[i].firstmap))
    {
      // 23-6-98 KM Fixed this.
      M_SetupNextMenu(&EpiDef);
      M_StartMessage(DDF_LanguageLookup("EpisodeNonExist"), NULL, false);
      return;
    }

    M_ClearMenus ();
}

void M_Episode(int choice)
{
    epi = choice;
    M_SetupNextMenu(&NewDef);
}



//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
    V_DrawPatchInDirect (108,15,0,W_CacheLumpName("M_OPTTTL",PU_CACHE));
	
    V_DrawPatchInDirect (OptionsDef.x + 175,OptionsDef.y+LINEHEIGHT*detail,0,
		       W_CacheLumpName(detailNames[detailLevel],PU_CACHE));

    V_DrawPatchInDirect (OptionsDef.x + 120,OptionsDef.y+LINEHEIGHT*messages,0,
		       W_CacheLumpName(msgNames[showMessages],PU_CACHE));

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(mousesens+1),
		 10,mouseSensitivity);
	
    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
		 9,screenSize);
}

void M_Options(int choice)
{
  optionsmenuon=1;
}



//
//      Toggle messages on/off
//
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;
	
    if (!showMessages)
	players[consoleplayer].message = DDF_LanguageLookup("MessagesOff");
    else
	players[consoleplayer].message = DDF_LanguageLookup("MessagesOn");

    message_dontfuckwithme = true;
}


//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
    if (ch != 'y')
	return;
		
    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }
	
    if (netgame)
    {
	M_StartMessage(DDF_LanguageLookup("EndNetGame"),NULL,false);
	return;
    }
	
    M_StartMessage(DDF_LanguageLookup("EndGameCheck"),M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}




//
// M_QuitDOOM
//
bastard_sfx_t     quitsounds[] =
{
  {0, "pldeth"},
  {0, "dmpain"},
  {0, "popain"},
  {0, "slop"},
  {0, "telept"},
  {0, "posit1"},
  {0, "posit3"},
  {0, "sgtatk"},
  {0, "vilact"},
  {0, "getpow"},
  {0, "boscub"},
  {0, "slop"},
  {0, "skeswg"},
  {0, "kntdth"},
  {0, "bspact"},
  {0, "sgtatk"}
};


// -KM- 1998/12/16 Handle sfx that don't exist in this
//   version.
void M_QuitResponse(int ch)
{
    if (ch != 'y')
	return;
    if (!netgame)
    {
        int i, j = 0;
        if (!quitsounds[0].s)
        {
          char name[9];
          for (i = sizeof(quitsounds) / sizeof(quitsounds[0]); i--;)
          {
             sprintf(name, "DS%s", quitsounds[i].name);
             if (W_CheckNumForName(name) != -1)
               quitsounds[i].s = DDF_LookupSound(quitsounds[i].name);
          }
        }
        i = sizeof(quitsounds) / sizeof(quitsounds[0]);
	//if ((gamemode == commercial) || (gamemode == dosdoom))
        while (!quitsounds[((gametic>>2)+j)%i].s) j++;
        S_StartSound(NULL, quitsounds[((gametic>>2)+j)%i].s);
	//else
	//    S_StartSound(NULL,quitsounds[(gametic>>2)&7]);
	I_WaitVBL(105);
    }
    I_Quit ();
}




//
// -ACB- 1998/07/19 Removed offensive messages selection (to some people);
//			  Better Random Selection.
// -KM- 1998/07/21 Reinstated counting quit messages, so adding them to dstrings.c
//                   is all you have to do.  Using P_Random for the random number
//                   automatically kills the demo sync...
// -KM- 1998/07/31 Removed Limit. So there.
static char *endstring = NULL;
void M_QuitDOOM(int choice)
{
  char* DOSY;

  int num_quitmessages;
  int longest_quitmessage = 0;

  DOSY = DDF_LanguageLookup("PressToQuit");

  // Count the quit messages
  for (num_quitmessages = 0; endmsg[num_quitmessages]; num_quitmessages++)
    if (!endstring && strlen(endmsg[num_quitmessages]) > longest_quitmessage)
      longest_quitmessage = strlen(endmsg[num_quitmessages]);

  if (!endstring)
      endstring = Z_Malloc(longest_quitmessage + 3 + strlen(DOSY), PU_STATIC, NULL);

  //
  // We pick index 0 which is language sensitive,
  // or one at random, between 1 and maximum number.
  //
//  if (language != english )
    sprintf(endstring,"%s\n\n%s", endmsg[0],DOSY );
//  else
//    sprintf(endstring,"%s\n\n%s", endmsg[ gametic % num_quitmessages ],DOSY);
  
  M_StartMessage(endstring,M_QuitResponse,true);
}




// 98-7-10 KM Use new defines
void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
      case SLIDERLEFT:
	if (mouseSensitivity)
	    mouseSensitivity--;
	break;
      case SLIDERRIGHT:
	if (mouseSensitivity < 9)
	    mouseSensitivity++;
	break;
    }
}




void M_ChangeDetail(int choice)
{
    choice = 0;
//    detailLevel = 1 - detailLevel;
    detailLevel=0;

    // FIXME - does not work. Remove anyway?
//    fprintf( stderr, "M_ChangeDetail: low detail mode n.a.\n");

    return;
    
    /*R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DETAILHI;
    else
	players[consoleplayer].message = DETAILLO;*/
}




// 98-7-10 KM Use new defines
void M_SizeDisplay(int choice)
{
    switch(choice)
    {
      case SLIDERLEFT:
	if (screenSize > 0)
	{
	    screenblocks--;
	    screenSize--;
	}
	break;
      case SLIDERRIGHT:
	if (screenSize < 8)
	{
	    screenblocks++;
	    screenSize++;
	}
	break;
    }
	

    R_SetViewSize (screenblocks, 0);
}




//
//      Menu Functions
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int		xx;
    int		i;

    xx = x;
    V_DrawPatchInDirect (xx,y,0,W_CacheLumpName("M_THERML",PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
	V_DrawPatchInDirect (xx,y,0,W_CacheLumpName("M_THERMM",PU_CACHE));
	xx += 8;
    }
    V_DrawPatchInDirect (xx,y,0,W_CacheLumpName("M_THERMR",PU_CACHE));

    V_DrawPatchInDirect ((x+8) + thermDot*8,y,
		       0,W_CacheLumpName("M_THERMO",PU_CACHE));
}



void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchInDirect (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
		       W_CacheLumpName("M_CELL1",PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchInDirect (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
		       W_CacheLumpName("M_CELL2",PU_CACHE));
}


void
M_StartMessage
( char*		string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}

// -KM- 1998/07/21 Call M_StartMesageInput to start a message that needs a
//                  string input.  (You can convert it to a number if you want to.)
//                 string:  The prompt, eg "What is your name\n\n" must be either
//                  static or globally visible.
//                 routine: Format is void routine(char *s)  Routine will be called
//                  with a pointer to the input in s.  s will be NULL if the user
//                  pressed ESCAPE to cancel the input.
//                  String is allocated by Z_Malloc, it is your responsibility to
//                  Z_Free it.
void M_StartMessageInput (char* string, void* routine)
{
	messageLastMenuActive = menuactive;
        messageToPrint = 2;
        messageString = string;
        messageRoutine = routine;
        messageNeedsInput = true;
        menuactive = true;
        return;
}


void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    int             i;
    int             w = 0;
    int             c;
	
    for (i = 0;i < strlen(string);i++)
    {
	c = toupper(string[i]) - HU_FONTSTART;
	if (c < 0 || c >= HU_FONTSIZE)
	    w += 4;
	else
	    w += SHORT (hu_font[c]->width);
    }
		
    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    int             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);
	
    h = height;
    for (i = 0;i < strlen(string);i++)
	if (string[i] == '\n')
	    h += height;
		
    return h;
}


//
//      Write a string using the hu_font
//
void M_WriteText (int x, int y, char* string)
{
  char* ch;
  int w;
  int c;
  int cx;
  int cy;

  ch = string;
  cx = x;
  cy = y;
	
  while(1)
  {
    c = *ch++;

    if (!c)
      break;

    if (c == '\n')
    {
      cx = x;
      cy += 12;
      continue;
    }
		
    if (c < 128)
      c = toupper(c);

    c -= HU_FONTSTART;

    if (c < 0 || c>= HU_FONTSIZE)
    {
      cx += 4;
      continue;
    }
		
    w = SHORT (hu_font[c]->width);

    if (cx+w > SCREENWIDTH)
      break;

    V_DrawPatchInDirect(cx, cy, 0, hu_font[c]);
    cx+=w;
  }
}

//
//   Write a string using the hu_font and index translator.
//
void M_WriteTextTrans (int x, int y, int index, char* string)
{
  int w;
  char*	ch;
  int c;
  int cx;
  int cy;

  ch = string;
  cx = x;
  cy = y;
	
  while(1)
  {
    c = *ch++;

    if (!c)
      break;

    if (c == '\n')
    {
      cx = x;
      cy += 12;
      continue;
    }
		
    if (c < 128)
      c = toupper(c);

    c -= HU_FONTSTART;

    if (c < 0 || c>= HU_FONTSIZE)
    {
      cx += 4;
      continue;
    }
		
    w = SHORT (hu_font[c]->width);

    if (cx+w > SCREENWIDTH)
      break;

    V_DrawPatchInDirectTrans(cx, cy, index, 0, hu_font[c]);
    cx+=w;
  }
}

//
// CONTROL PANEL
//

//
// M_Responder
//
// -KM- 1998/09/01 Analogue binding, and hat support
boolean M_Responder (event_t* ev)
{
    int             ch = -1;
    int             i;
    static  int     analogue_wait = 0;
	
    if (ev->type == ev_analogue && analogue_wait < I_GetTime())
    {
        if (ev->data4 < 0)
        {
            ch = KEYD_UPARROW;
            analogue_wait = I_GetTime() + 5;
        } else if (ev->data4 > 0)
        {
            ch = KEYD_DOWNARROW;
            analogue_wait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            ch = KEYD_LEFTARROW;
            analogue_wait = I_GetTime() + 5;
        } else if (ev->data2 > 0)
        {
            ch = KEYD_RIGHTARROW;
            analogue_wait = I_GetTime() + 5;
        }
    } else if (ev->type == ev_keydown)
    {
	ch = ev->data1;
        if (menuactive)
        {
          switch (ch)
          {
                  case KEYD_JOY1:
                  case KEYD_MOUSE1:
                       ch = KEYD_ENTER;
                       break;
                  case KEYD_JOY2:
                  case KEYD_MOUSE2:
                       ch = KEYD_ESCAPE;
                       break;
                  case KEYD_HATN:
                       ch = KEYD_UPARROW;
                       break;
                  case KEYD_HATE:
                       ch = KEYD_RIGHTARROW;
                       break;
                  case KEYD_HATS:
                       ch = KEYD_DOWNARROW;
                       break;
                  case KEYD_HATW:
                       ch = KEYD_LEFTARROW;
                       break;
                  default:
                       break;
          }
        }
    }

    if ((ev->type == ev_keyup) || (ch == -1)) return false;

    // 25-6-98 KM Optimised here.
    if ((devparm && (ch == KEYD_F1)) || (ch == KEYD_PRTSCR))
    {
      G_ScreenShot ();
      return true;
    }

    // Take care of any messages that need input
    // -KM- 1998/07/21 Message Input
    if (messageToPrint == 1)
    {
	if (messageNeedsInput == true &&
	    !(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEYD_ESCAPE))
	    return false;
		
        messageToPrint = 0;
        // -KM- 1998/07/31 Moved this up here to fix bugs.
	menuactive = messageLastMenuActive;
	if (messageRoutine)
	    messageRoutine(ch);
			
        S_StartSound(NULL,sfx_swtchx);
	return true;
    } else if (messageToPrint == 2)
    {
        static int   messageLength;
        static int   messageP;
        if (!messageInputString)
        {
          messageInputString = Z_Malloc(32, PU_STATIC, NULL);
          messageLength = 32;
          messageP = 0;
          memset(messageInputString, 0, messageLength);
        }
        if (ch == KEYD_ENTER)
        {
   	  menuactive = messageLastMenuActive;
    	  messageToPrint = 0;
   	  if (messageRoutine)
   	    ((void (*)(char *)) messageRoutine)(messageInputString);
   			
          messageInputString = NULL;
   	  menuactive = false;
   	  S_StartSound(NULL,sfx_swtchx);
   	  return true;
        }
        if (ch == KEYD_ESCAPE)
        {
          menuactive = messageLastMenuActive;
          messageToPrint = 0;
          if (messageRoutine)
            ((void (*)(char *)) messageRoutine)(NULL);
          menuactive = false;
          Z_Free(messageInputString);
          messageInputString = NULL;
          S_StartSound(NULL, sfx_swtchx);
          return true;
        }
        if (ch == KEYD_BACKSPACE)
        {
          if (messageP > 0)
          {
            messageP--;
            messageInputString[messageP] = 0;
          }
          return true;
        }
        ch = toupper(ch);
        if (ch == '-') ch = '_';
        if (ch != 32)
    	  if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
    	    return true;
        if ((ch >= 32) && (ch <= 127) &&
    	  (M_StringWidth(messageInputString) < 300))
        {
    	  messageInputString[messageP++] = ch;
    	  messageInputString[messageP] = 0;
        }
        if (messageP == (messageLength - 1))
          messageInputString = Z_ReMalloc(messageInputString, ++messageLength);
        return true;
    }

    // new options menu on - use that responder
    if (optionsmenuon)
      return M_OptResponder(ev,ch);
    
    // Save Game string input
    if (saveStringEnter)
    {
	switch(ch)
	{
	  case KEYD_BACKSPACE:
	    if (saveCharIndex > 0)
	    {
		saveCharIndex--;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
				
	  case KEYD_ESCAPE:
	    saveStringEnter = 0;
	    strcpy(&savegamestrings[saveSlot][0],saveOldString);
	    break;
				
	  case KEYD_ENTER:
	    saveStringEnter = 0;
	    if (savegamestrings[saveSlot][0])
		M_DoSave(saveSlot);
	    break;
				
	  default:
	    ch = toupper(ch);
	    if (ch != 32)
		if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
		    break;
	    if (ch >= 32 && ch <= 127 &&
		saveCharIndex < SAVESTRINGSIZE-1 &&
		M_StringWidth(savegamestrings[saveSlot]) <
		(SAVESTRINGSIZE-2)*8)
	    {
		savegamestrings[saveSlot][saveCharIndex++] = ch;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
	}
	return true;
    }
    
    // F-Keys
    if (!menuactive)
    {
	switch(ch)
	{
	  case KEYD_MINUS:         // Screen size down
	    if (automapactive || chat_on)
		return false;
            // 98-7-10 KM Use new defines
	    M_SizeDisplay(SLIDERLEFT);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
				
	  case KEYD_EQUALS:        // Screen size up
	    if (automapactive || chat_on)
		return false;
            // 98-7-10 KM Use new defines
	    M_SizeDisplay(SLIDERRIGHT);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
				
	  case KEYD_F1:            // Help key
	    M_StartControlPanel ();

	    //if ( gamemode == retail )
	    //  currentMenu = &ReadDef2;
	    //else
	      currentMenu = &ReadDef1;
	    
	    itemOn = 0;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEYD_F2:            // Save
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_SaveGame(0);
	    return true;
				
	  case KEYD_F3:            // Load
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_LoadGame(0);
	    return true;
				
	  case KEYD_F4:            // Sound Volume
	    M_StartControlPanel ();
	    currentMenu = &SoundDef;
	    itemOn = sfx_vol;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEYD_F5:            // Detail toggle, now loads options menu
            // -KM- 1998/07/31 F5 now loads options menu, detail is obsolete.
	    S_StartSound(NULL,sfx_swtchn);
            M_StartControlPanel();
            M_Options(0);
	    return true;
				
	  case KEYD_F6:            // Quicksave
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickSave();
	    return true;
				
	  case KEYD_F7:            // End game
	    S_StartSound(NULL,sfx_swtchn);
	    M_EndGame(0);
	    return true;
				
	  case KEYD_F8:            // Toggle messages
	    M_ChangeMessages(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
				
	  case KEYD_F9:            // Quickload
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickLoad();
	    return true;
				
	  case KEYD_F10:           // Quit DOOM
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuitDOOM(0);
	    return true;
				
	  case KEYD_F11:           // gamma toggle
	    usegamma++;
	    if (usegamma > 4)
		usegamma = 0;
	    players[consoleplayer].message = gammamsg[usegamma];
	    I_SetPalette (W_CacheLumpName ("PLAYPAL",PU_CACHE),0);
	    return true;
				
	}
    }

    
    // Pop-up menu?
    if (!menuactive)
    {
	if (ch == KEYD_ESCAPE)
	{
	    M_StartControlPanel ();
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
	return false;
    }

    
    // Keys usable within menu
    switch (ch)
    {
      case KEYD_DOWNARROW:
	do
	{
	    if (itemOn+1 > currentMenu->numitems-1)
		itemOn = 0;
	    else itemOn++;
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);
	return true;
		
      case KEYD_UPARROW:
	do
	{
	    if (!itemOn)
		itemOn = currentMenu->numitems-1;
	    else itemOn--;
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);
	return true;

      case KEYD_LEFTARROW:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
            // 98-7-10 KM Use new defines
	    currentMenu->menuitems[itemOn].routine(SLIDERLEFT);
	}
	return true;
		
      case KEYD_RIGHTARROW:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
            // 98-7-10 KM Use new defines
	    currentMenu->menuitems[itemOn].routine(SLIDERRIGHT);
	}
	return true;

      case KEYD_ENTER:
	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
	    currentMenu->lastOn = itemOn;
            currentMenu->menuitems[itemOn].routine(itemOn);
            S_StartSound(NULL,sfx_pistol);
	}
	return true;
		
      case KEYD_ESCAPE:
	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	S_StartSound(NULL,sfx_swtchx);
	return true;
		
      case KEYD_BACKSPACE:
	currentMenu->lastOn = itemOn;
	if (currentMenu->prevMenu)
	{
	    currentMenu = currentMenu->prevMenu;
	    itemOn = currentMenu->lastOn;
	    S_StartSound(NULL,sfx_swtchn);
	}
	return true;
	
      default:
	for (i = itemOn+1;i < currentMenu->numitems;i++)
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
	for (i = 0;i <= itemOn;i++)
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
	break;
	
    }

    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
	return;
    
    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
  static short x;
  static short y;
  short i;
  short max;
  int start;

  inhelpscreens = false;

  // 1998/07/10 KM Darken screen added for all messages (quit messages)
  if (!menuactive)
    return;

  if (darken_screen) V_DarkenScreen(0);
    
  // Horiz. & Vertically center string and print it.
  if (messageToPrint)
  {
    // -KM- 1998/06/25 Remove limit.
    // -KM- 1998/07/21 Add space for input
    // -ACB- 1998/06/09 More space for message.
    // -KM- 1998/07/31 User input in different colour.
    char *string;
    char *Mstring;
    int len;
    int input_start;

    // Reserve space for prompt
    len = input_start = strlen(messageString);

    // Reserve space for what the user typed in
    len += messageInputString ? strlen(messageInputString) : 0;

    // Reserve space for '_' cursor
    len += (messageToPrint == 2) ? 1 : 0;

    // Reserve space for NULL Terminator
    len++;

    string = alloca(len);
    Mstring = alloca(len);

    strcpy(Mstring, messageString);

    if (messageToPrint == 2)
    {
      if (messageInputString)
        strcpy(Mstring + strlen(messageString), messageInputString);

      Mstring[i = strlen(Mstring)] = '_';
      Mstring[i + 1] = 0;
    }

    start = 0;

    y = 100 - M_StringHeight(Mstring)/2;

    while(*(Mstring+start))
    {
      for (i = 0; i < strlen(Mstring+start); i++)
      {
        if (*(Mstring+start+i) == '\n')
        {
          // -ACB- 1998/06/09 set sufficent space.
 	  memset(string,0,strlen(Mstring));
	  strncpy(string,Mstring+start,i);
	  start += (i+1);
	  break;
        }
      }

      if (i == strlen(Mstring+start))
      {
 	strcpy(string,Mstring+start);
 	start += i;
      }
				
      x = 160 - M_StringWidth(string)/2;

      // -KM- 1998/07/31 Colour should be a define or something...
      // -ACB- 1998/09/01 Colour is now a define
      if (start > input_start)
        M_WriteTextTrans(x,y,15,string);
      else
        M_WriteText(x,y,string);

      y += SHORT(hu_font[0]->height);
    }

    return;
  }

  // new options menu enable, use that drawer instead
  if (optionsmenuon)
  {
    M_OptDrawer();
    return;
  }

  if (currentMenu->routine)
    currentMenu->routine();         // call Draw routine
    
  // DRAW MENU
  x = currentMenu->x;
  y = currentMenu->y;
  max = currentMenu->numitems;

  for (i=0;i<max;i++)
  {
    if (currentMenu->menuitems[i].name[0])
    {
      V_DrawPatchInDirect(x,y,0,
        W_CacheLumpName(currentMenu->menuitems[i].name,PU_CACHE));
    }

    y += LINEHEIGHT;
  }

  // DRAW SKULL
  V_DrawPatchInDirect(x+SKULLXOFF, currentMenu->y-5+itemOn*LINEHEIGHT, 0,
		            W_CacheLumpName(skullName[whichSkull],PU_CACHE));

}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;
    // if (!netgame && usergame && paused)
    //       sendpause = true;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
void M_Ticker (void)
{
  if (optionsmenuon)
  {
    M_OptTicker();
    return;
  }

  if (--skullAnimCounter <= 0)
    {
	whichSkull ^= 1;
	skullAnimCounter = 8;
    }
}


//
// M_Init
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.
//    if (W_CheckNumForName("M_EPI4") < 0)
//      EpiDef.numitems -= 2;
//    else if (W_CheckNumForName("M_EPI5") < 0)
//      EpiDef.numitems--;

    if (W_CheckNumForName("HELP2") < 0)
    {
	// This is used because DOOM 2 had only one HELP
	//  page. I use CREDIT as second page now, but
	//  kept this hack for educational purposes.
	MainMenu[readthis] = MainMenu[quitdoom];
	MainDef.numitems--;
	MainDef.y += 8;
	NewDef.prevMenu = &MainDef;
	ReadDef1.routine = M_DrawReadThis1;
	ReadDef1.x = 330;
	ReadDef1.y = 165;
	ReadMenu1[0].routine = M_FinishReadThis;
    }

  M_InitOptmenu();
    
}

