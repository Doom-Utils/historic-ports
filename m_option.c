//
// DOSDoom Option Menu Modification
//
// Original Author: Chi Hoang
//
// -ACB- 1998/06/15 All functions are now m_* to follow doom standard.
//
// -MH-  1998/07/01 Shoot Up/Down becomes "True 3D Gameplay"
//                  Added keys for fly up and fly down
//
// -KM-  1998/07/10 Used better names :-) (Controls Menu)
//
// -ACB- 1998/07/10 Print screen is now possible in this menu
//
// -ACB- 1998/07/12 Removed Lost Soul/Spectre Ability Menu Entries
//
// -ACB- 1998/07/15 Changed menu structure for graphic titles
//
// -ACB- 1998/07/30 Remove M_SetRespawn and the newnmrespawn &
//                  respawnmonsters. Used new respawnsetting variable.
//
// -ACB- 1998/08/10 Edited the menu's to reflect the fact that currentmap
//                  flags can prevent changes.
//
// -ES-  1998/08/21 Added resolution options
//
// -ACB- 1998/08/29 Modified Resolution menus for user-friendlyness
//
// -ACB- 1998/09/06 MouseOptions renamed to AnalogueOptions
//
// -ACB- 1998/09/11 Cleaned up and used new white text colour for menu
//                  selections
// -KM- 1998/11/25 You can scroll backwards through the resolution list!
//
// -ES- 1998/11/28 Added faded teleportation option
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "dm_state.h"

#include "d_debug.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "i_video.h"
#include "i_music.h"
#include "i_system.h"
#include "lu_sound.h"
#include "m_menu.h"
#include "m_misc.h"
#include "m_option.h"
#include "p_local.h"
#include "r_main.h"
#include "s_sound.h"
#include "v_res.h"
#include "w_wad.h"
#include "z_zone.h"

#define OPTSHADE 15

int optionsmenuon=0;

// 98-7-10 KM For turning mouse/joystick on/off
extern int usejoystick;
extern int usemouse;

//submenus
static void M_StandardControlOptions();
static void M_ExtendedControlOptions();

static void M_VideoOptions();
static void M_GameplayOptions();
static void M_AnalogueOptions();

static void M_CalibrateJoystick(void);

static void M_ResetToDefaults();
static void M_ResetToOrigDoom();

static void M_Key2String(int key, char *deststring);

// -ACB- 1998/08/09 "Does Map allow these changes?" procedures.
static void M_ChangeMonsterRespawn(void);
static void M_ChangeItemRespawn(void);
static void M_ChangeStretchSky(void);
static void M_ChangeTransluc(void);
static void M_ChangeTrue3d(void);
static void M_ChangeAutoAim(void);
static void M_ChangeFastparm(void);
static void M_ChangeRespawn(void);

//Special function declarations
int menunormalfov,menuzoomedfov;
int screenSize;
static void M_ChangeScreenSize() {R_SetViewSize(screenSize+3,0);}
static void M_ChangeSfxVol()     {S_SetSfxVolume(snd_SfxVolume);}
static void M_ChangeMusVol()     {S_SetMusicVolume(snd_MusicVolume);}
static void M_ChangeCDMusVol()   {S_SetCDMusicVolume(snd_CDMusicVolume);}

static void M_ChangeNormalFOV()  {R_SetNormalFOV((ANG45/9)*(menunormalfov+1));}
static void M_ChangeZoomedFOV()  {R_SetZoomedFOV((ANG45/9)*(menuzoomedfov+1));}

static void M_SetCDAudio(void);

static void M_ChangeBlood(void);

static void M_ChangeGamma()      {I_SetPalette(W_CacheLumpName("PLAYPAL",PU_CACHE),0);}

// -ES- 1998/08/20
// -ACB- 1998/08/29 Resolution changes stuff, modified from -ES- code.
static int optwidth, oldwidth;
static int optheight, oldheight;
static int optbpp, oldbpp;
static int ressize;
static int testticker = -1;
extern boolean setresfailed;
static char setreserror[128];

// -ES- 1998/08/20 Added resolution options
// -ACB- 1998/08/29 Moved to top and tried different system
void R_ChangeResolution(int, int, int);

static void M_ResolutionOptions(void);
static void M_OptionSetResolution(void);
static void M_OptionTestResolution(void);
static void M_RestoreResSettings();
static void M_ChangeStoredRes(void);
//void M_ChangeStoredBpp(void) {optbpp = !optbpp;};

//now, the menus themselves
static char YesNo[]="Off/On";                   // basic on/off
//static char ImPer[]="Perfect/Imperfect";        // Targeting: imperfect/perfect
static char CrosO[]="None/Cross/Dot/Angle";     // crosshair options
//static char Infig[]="None/Low/High";            // monster infighting
static char Respw[]="Teleport/Resurrect";  // monster respawning
//static char MTele[]="Off/On/Silent";            // missile teleport
//static char TelDl[]="Normal/Fast/Quickest";     // player teleport
// -KM- 1998/09/27  Analogue binding. Add Fly axis.
static char Axis[]="Turn/Forward/Strafe/MLook/Fly/Disable";
static char CD[]="Off/On/Atmosphere/Atmosphere";
static char* Resolutions = NULL;
static char Colours[]="8 bit/16 bit";
// -ES- 1998/11/28 Wipe and Faded teleportation options
static char FadeT[]="Off/On, w flash/On, wo flash";
static char WipeM[]="Fade/Melt";
static char AAim[]="Off/On/Mlook";

typedef struct menuinfo_s
{
  int menusize;
  int menucenter;
  int titleposx;
  char menutitlename[9];
}
menuinfo_t;

typedef struct optmenuitem_s
{
  int type; //0 means plain text, 1 is change a switch, 2 is call a function,
            //3 is a slider, 4 is a key config
  char name[48];
  const char *typenames;
  int numtypes;
  int dosdoomdefault;
  int origdoomdefault;
  int* switchvar;
  void (*routine)();
  char* help;
}
optmenuitem_t;

typedef struct specialkey_s
{
  int keycode;
  char keystring[20];
}
specialkey_t;

typedef struct
{
  int width;
  int height;
}
screenmode_t;

// -KM- 1998/11/25 Removed extreme aspect modes.
screenmode_t screenres[] = {
   { 320,   200}, //0.625
   { 320,   240}, //0.75
   { 360,   200}, //0.5555555
   { 360,   240}, //0.666
   { 360,   270}, //0.75
   { 360,   360}, //1.0
   { 376,   282}, //0.75
   { 376,   308}, //0.819
   { 400,   300}, //0.75
   { 512,   384}, //0.75
   { 640,   400}, //0.625
   { 640,   480}, //0.75
   { 800,   600}, //0.75
   { 1024,  768}, //0.75
   { 1280,  1024},//0.8
   { 1600,  1200}, //0.75
   { -1, -1}
};


#define mainmenusize 17

// -ACB- 1998/07/15 Altered menu structure
menuinfo_t mainmenuinfo={mainmenusize,164,108,"M_OPTTTL"};

optmenuitem_t mainmenu[mainmenusize]=
 {{2,"Standard Controls",       NULL, 0, 0, 0, NULL,M_StandardControlOptions,"Controls"},
  {2,"Extended Controls",       NULL, 0, 0, 0, NULL,M_ExtendedControlOptions,"Controls2"},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {2,"Video Options",           NULL, 0, 0, 0, NULL,M_VideoOptions, "VideoOptions"},
  {2,"Gameplay Options",        NULL, 0, 0, 0, NULL,M_GameplayOptions, "GameplayOptions"},
  {2,"Analogue Options",        NULL, 0, 0, 0, NULL,M_AnalogueOptions, "AnalogueOptions"},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Messages",                YesNo,2, 1, 1, &showMessages,NULL,"Messages"},
  {1,"Swap Stereo",             YesNo,2, 0, 0, (int *) &swapstereo,NULL,"SwapStereo"},
  {3,"Sound Volume",            NULL, 16,12,12,&snd_SfxVolume,M_ChangeSfxVol,NULL},
  {3,"Music Volume",            NULL, 16,12,12,&snd_MusicVolume,M_ChangeMusVol, NULL},
  {3,"CD Music Volume",         NULL, 16,12,12,&snd_CDMusicVolume,M_ChangeCDMusVol, NULL},
  {1,"CD Audio",                CD, 3, 0, 0, (int *) &cdaudio, M_SetCDAudio, "CDOptions"},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {2,"Reset to Recommended",    NULL, 0, 0, 0, NULL,M_ResetToDefaults, "ResetDOSDoom"},
  {2,"Reset to Original Doom",  NULL, 0, 0, 0, NULL,M_ResetToOrigDoom, "ResetDoom"}};

#define vidoptionssize 18

// -ACB- 1998/07/15 Altered menu structure
menuinfo_t vidoptionsinfo={vidoptionssize,150,77,"M_VIDEO"};

// -ES- 1999/03/29 New fov stuff
optmenuitem_t vidoptions[vidoptionssize]=
 {{2,"Set Resolution",          NULL, 0, 0, 0, NULL,M_ResolutionOptions, "ChangeRes"},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {3,"Screensize",              NULL, 9, 7, 7, &screenSize,M_ChangeScreenSize,NULL},
  {3,"Brightness",              NULL, 5, 0, 0, &usegamma,M_ChangeGamma,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {3,"Field Of View",           NULL, 35, 17, 17, &menunormalfov,M_ChangeNormalFOV,NULL},
  {3,"Zoomed Field of View",    NULL, 35, 1, 1, &menuzoomedfov,M_ChangeZoomedFOV,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Translucency",            YesNo,2, 1, 0, (int *) &settingflags.trans,M_ChangeTransluc,NULL},
  {1,"Faded teleportation",     FadeT,3, 2, 0, &faded_teleportation,NULL,NULL},
  {1,"Wipe method",             WipeM,2, 1, 0, &wipe_method,NULL,NULL},
  {1,"Crosshair",               CrosO,4, 2, 0, &crosshair,NULL,NULL},
  {1,"Stretch Sky",             YesNo,2, 1, 0, (int *) &settingflags.stretchsky,M_ChangeStretchSky,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Vertical Retrace",        YesNo,2, 1, 0, &retrace,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Map Rotation",            YesNo,2, 1, 0, &rotatemap,NULL,NULL},
  {1,"Map Overlay",             YesNo,2, 1, 0, &newhud,NULL,NULL}};

#define resoptionssize 11
menuinfo_t resoptionsinfo={resoptionssize,150,77,"M_VIDEO"};

optmenuitem_t resoptions[resoptionssize]=
 {{0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Change Size",             NULL, 0, 0, 0, &ressize,M_ChangeStoredRes,NULL},
  {1,"Change Depth",            Colours, 2, 0, 0, &optbpp, NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {2,"Set Resolution",          NULL, 0, 0, 0, NULL,M_OptionSetResolution,NULL},
  {2,"Test Resolution",         NULL, 0, 0, 0, NULL,M_OptionTestResolution,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL}};
  
// -ACB- 1998/06/15 Added new mouse menu
// -KM- 1998/09/01 Changed to an analogue menu.  Must change those names
#define analogueoptionssize 15

// -ACB- 1998/07/15 Altered menu structure
menuinfo_t analogueoptionsinfo={analogueoptionssize,150,75,"M_MSETTL"};

optmenuitem_t analogueoptions[analogueoptionssize]=
 {{1,"Invert Mouse",            YesNo,2, 0, 0, &invertmouse,NULL,NULL},
  {1,"Mouse X Axis",            Axis, 6, AXIS_TURN, AXIS_TURN, &mouse_xaxis, NULL,NULL},
  {1,"Mouse Y Axis",            Axis, 6, AXIS_FORWARD, AXIS_MLOOK, &mouse_yaxis, NULL,NULL},
  {3,"MouseSpeed",              NULL, 20,8, 8, &mouseSensitivity,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {3,"FreeLook Speed",          NULL, 20,8, 0, &keylookspeed,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {1,"Two-Stage Turning",       YesNo,2, 0, 0, (int *)&stageturn,NULL,NULL},
  {3,"Turning Speed",           NULL, 9, 0, 0, &angleturnspeed,NULL,NULL},
  {3,"Side Move Speed",         NULL, 9, 0, 0, &sidemovespeed,NULL,NULL},
  {3,"Forward Move Speed",      NULL, 9, 0, 0, &forwardmovespeed,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Joystick X Axis",         Axis, 6, AXIS_TURN, AXIS_TURN, &joy_xaxis, NULL,NULL},
  {1,"Joystick Y Axis",         Axis, 6, AXIS_FORWARD, AXIS_FORWARD, &joy_yaxis, NULL,NULL},
  {2,"Calibrate Joystick",      NULL, 0, 0, 0, NULL, M_CalibrateJoystick,NULL}};
  
#define playoptionssize 10

// -ACB- 1998/07/15 Altered menu structure
menuinfo_t playoptionsinfo={playoptionssize,160,46,"M_GAMEPL"};

// -KM- 1998/07/21 Change blood to switch
optmenuitem_t playoptions[playoptionssize]=
 {{3,"Gravity",                 NULL, 20,8, 8, &settingflags.grav,NULL,"Gravity"},
  {1,"Blood",                   YesNo,2 ,0, 0, (int *) &settingflags.blood, M_ChangeBlood,"Blood"},
  {1,"True 3D Gameplay",        YesNo,2, 1, 0, (int *) &settingflags.true3dgameplay, M_ChangeTrue3d,"True3d"},
  {1,"AutoAiming",              AAim, 3, 1, 1, (int *) &settingflags.autoaim, M_ChangeAutoAim, NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},              
  {1,"Enemy Respawn Mode",      Respw,2, 1, 0, (int *) &settingflags.respawnsetting,M_ChangeMonsterRespawn,NULL},
  {1,"Item Respawn",            YesNo,2, 0, 0, (int *) &settingflags.itemrespawn,M_ChangeItemRespawn,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {1,"Fast Monsters",           YesNo,2, 0, 0, (int *) &settingflags.fastparm,M_ChangeFastparm,NULL},
  {1,"Respawn",                 YesNo,2, 0, 0, (int *) &settingflags.respawn,M_ChangeRespawn,NULL}
};
  
#define stdkeyconfigsize 15

// -ACB- 1998/07/15 Altered menuinfo struct
menuinfo_t stdkeyconfiginfo={stdkeyconfigsize,110,98,"M_CONTRL"};

// -KM- 1998/07/10 Used better names :-)
optmenuitem_t stdkeyconfig[stdkeyconfigsize]=
 {{0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {4,"Attack",                  NULL, 0, KEYD_RCTRL, 0, &key_fire,NULL,NULL},
  {4,"Use",                     NULL, 0, ' ',0,&key_use,NULL,NULL},
  {4,"Walk Forward",            NULL, 0, KEYD_UPARROW,0, &key_up,NULL,NULL},
  {4,"Walk Backwards",          NULL, 0, KEYD_DOWNARROW, 0, &key_down,NULL,NULL},
  {4,"Turn Left",               NULL, 0, KEYD_LEFTARROW, 0, &key_left,NULL,NULL},
  {4,"Turn Right",              NULL, 0, KEYD_RIGHTARROW, 0, &key_right,NULL,NULL},
  {4,"Fly Up",                  NULL, 0, KEYD_INSERT, 0, &key_flyup,NULL,NULL},
  {4,"Fly Down",                NULL, 0, KEYD_DELETE, 0, &key_flydown,NULL,NULL},
  {4,"Run",                     NULL, 0, KEYD_RSHIFT, 0, &key_speed,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {4,"Strafe Left",             NULL, 0, ',', 0, &key_strafeleft,NULL,NULL},
  {4,"Strafe Right",            NULL, 0, '.', 0, &key_straferight,NULL,NULL},
  {4,"Strafe",                  NULL, 0, KEYD_RALT, 0, &key_strafe,NULL,NULL}};

#define extkeyconfigsize 10

// -ACB- 1998/07/15 Altered menuinfo struct
// -ES- 1999/03/28 Added Zoom Key
menuinfo_t extkeyconfiginfo={extkeyconfigsize,110,98,"M_CONTRL"};

optmenuitem_t extkeyconfig[extkeyconfigsize]=
 {{0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL,NULL},
  {4,"Look Up",                 NULL, 0, KEYD_PGUP, 0, &key_lookup,NULL,NULL},
  {4,"Look Down",               NULL, 0, KEYD_PGDN, 0, &key_lookdown,NULL,NULL},
  {4,"Center View",             NULL, 0, KEYD_HOME, 0, &key_lookcenter,NULL,NULL},
  {4,"Zoom in/out",             NULL, 0, KEYD_TILDE,  0, &key_zoom,NULL,NULL},
  {4,"180 degree turn",         NULL, 0, 0, 0, &key_180,NULL,NULL},
  {4,"Jump",                    NULL, 0, 0, 0, &key_jump,NULL,NULL},
  {4,"Map Toggle",              NULL, 0, KEYD_TAB,0,&key_map,NULL,NULL},
  {4,"Multiplay Talk",          NULL, 0, 't',0,&key_talk,NULL,NULL}};

char keystring1[]="Enter to change, Backspace to Clear";
char keystring2[]="Press a key for this action";

specialkey_t specialkeylist[]= //terminate on -1
 {{KEYD_RIGHTARROW, "Right Arrow"},
  {KEYD_LEFTARROW,  "Left Arrow"},
  {KEYD_UPARROW,    "Up Arrow"},
  {KEYD_DOWNARROW,  "Down Arrow"},
  {KEYD_ESCAPE,     "Escape"},
  {KEYD_ENTER,      "Enter"},
  {KEYD_TAB,        "Tab"},
  {KEYD_F1,         "F1"},
  {KEYD_F2,         "F2"},
  {KEYD_F3,         "F3"},
  {KEYD_F4,         "F4"},
  {KEYD_F5,         "F5"},
  {KEYD_F6,         "F6"},
  {KEYD_F7,         "F7"},
  {KEYD_F8,         "F8"},
  {KEYD_F9,         "F9"},
  {KEYD_F10,        "F10"},
  {KEYD_F11,        "F11"},
  {KEYD_F12,        "F12"},
  {KEYD_BACKSPACE,  "Backspace"},
  {KEYD_EQUALS,     "Equals"},
  {KEYD_MINUS,      "Minus"},
  {KEYD_RSHIFT,     "Shift"},
  {KEYD_RCTRL,      "Ctrl"},
  {KEYD_RALT,       "Alt"},
  {KEYD_INSERT,     "Insert"},
  {KEYD_DELETE,     "Delete"},
  {KEYD_PGDN,       "PageDown"},
  {KEYD_PGUP,       "PageUp"},
  {KEYD_HOME,       "Home"},
  {KEYD_END,        "End"},
  {32,              "Space"},
  {'\'',            "\'"},
  {KEYD_TILDE,             "Tilde"},
  {KEYD_MOUSE1,     "Mouse1"},
  {KEYD_MOUSE2,     "Mouse2"},
  {KEYD_MOUSE3,     "Mouse3"},
  {-1,              ""}};

optmenuitem_t* currmenu;
menuinfo_t* currmenuinfo;
int cursorpos,mainmenupos,numberofentries,keyscan;

void M_InitOptmenu()
{
  optionsmenuon=0;
  screenSize=screenblocks-3;
  currmenu=mainmenu; currmenuinfo=&mainmenuinfo;
  cursorpos=mainmenupos=0;
  keyscan=0;
  numberofentries=mainmenusize;
  // Restore the config setting.
  M_ChangeBlood();
}

void M_OptTicker(void)
{
  if (setresfailed)
  {
    sprintf(setreserror, DDF_LanguageLookup("ModeSelErr"), optwidth, optheight, 1<<((optbpp + 1) * 8));
    M_StartMessage(setreserror, NULL, false);
    testticker = -1;
    setresfailed = false;
  }
  if (testticker > 0)
  {
    testticker --;
  } else if (!testticker)
  {
    testticker --;
    M_RestoreResSettings();
  }
}
void M_OptDrawer()
{
  char tempstring[80];
  int curry,deltay,menutop;
  int i,j,k;

  // -ACB- 1998/06/15 Calculate height for menu and then center it.
  menutop = 68 - ((numberofentries*hu_font[0]->height)/2);

  V_DrawPatchInDirect(currmenuinfo->titleposx,menutop,0,
                       W_CacheLumpName(currmenuinfo->menutitlename,PU_CACHE));

  //now, draw all the menuitems
  deltay=1+hu_font[0]->height;

  curry=menutop + 25;

  // here we hack again........
  if (currmenu==stdkeyconfig || currmenu==extkeyconfig)
  {
    if (keyscan)
    {
      M_WriteText(160-(M_StringWidth(keystring2)/2),curry,keystring2);
    }
    else
    {
      M_WriteText(160-(M_StringWidth(keystring1)/2),curry,keystring1);
    }
  }
  else if (currmenu==resoptions)
  {
    i = curry;
    sprintf(tempstring,"Current Resolution:");
    M_WriteText(160-(M_StringWidth(tempstring)/2), i, tempstring);

    i += deltay;
    sprintf(tempstring,"%d x %d in %d-bit mode",
              SCREENWIDTH, SCREENHEIGHT, (BPP<2?8:16));
    M_WriteTextTrans(160-(M_StringWidth(tempstring)/2), i, OPTSHADE, tempstring);

    i = curry + (deltay * (resoptionssize-2));
    sprintf(tempstring,"Selected Resolution:");
    M_WriteText(160-(M_StringWidth(tempstring)/2), i, tempstring);

    i += deltay;
    sprintf(tempstring,"%d x %d in %d-bit mode",
              optwidth, optheight, (optbpp<1?8:16));
    M_WriteTextTrans(160-(M_StringWidth(tempstring)/2), i, OPTSHADE, tempstring);
  }

  for (i=0;i<(currmenuinfo->menusize);i++)
  {
    M_WriteText((currmenuinfo->menucenter)-M_StringWidth(currmenu[i].name),
                  curry,currmenu[i].name);

    // -ACB- 1998/07/15 Menu Cursor is colour indexed.
    if (i==cursorpos)
    {
      M_WriteText((currmenuinfo->menucenter+5),curry,"*");
      if (currmenu[i].help)
      {
        char* help = DDF_LanguageLookup(currmenu[i].help);
        M_WriteText(160-(M_StringWidth(help)/2),200-deltay,help);
      }
    }

    switch (currmenu[i].type)
      {
      case 1: //a switch
        k=0;
        for (j=0;j<(*(currmenu[i].switchvar));j++)
          {
          while ((currmenu[i].typenames[k]!='/')&&(k<strlen(currmenu[i].typenames)))
            k++;
          k++;
          }
        if (k<strlen(currmenu[i].typenames))
        {
        j=0;
        while ((currmenu[i].typenames[k]!='/')&&(k<strlen(currmenu[i].typenames)))
        {
          tempstring[j]=currmenu[i].typenames[k];
          j++; k++;
        }
        tempstring[j]=0;
        } else sprintf(tempstring, "Invalid");
        M_WriteTextTrans((currmenuinfo->menucenter)+15,curry, OPTSHADE, tempstring);
        break;
      case 3:  //a slider
        V_DrawPatchShrink ((currmenuinfo->menucenter+15),curry,0,W_CacheLumpName("M_THERML",PU_CACHE));
        for (j=0;j<9;j++)
          V_DrawPatchShrink ((currmenuinfo->menucenter+15)+8+8*j,curry,0,W_CacheLumpName("M_THERMM",PU_CACHE));
        V_DrawPatchShrink ((currmenuinfo->menucenter+15)+80,curry,0,W_CacheLumpName("M_THERMR",PU_CACHE));
        V_DrawPatchShrink ((currmenuinfo->menucenter+15)+8+(64*(*(currmenu[i].switchvar))/(currmenu[i].numtypes-1)),
                           curry,0,W_CacheLumpName("M_THERMO",PU_CACHE));
        break;
      case 4:  //a keycode
        k=(*(currmenu[i].switchvar));
        M_Key2String(k,tempstring);
        M_WriteTextTrans((currmenuinfo->menucenter+15),curry, OPTSHADE,tempstring);
        break;
      }
    curry+=deltay;
   }
}

boolean M_OptResponder (event_t *ev, int ch)
{
  // Scan for keycodes
  if (keyscan)
  {
    if (ev->type==ev_keydown)
    {
      int* blah;
      keyscan=0;

      blah=(currmenu[cursorpos].switchvar);
      if (((*blah)>>16)==ev->data1)
      {
        (*blah)&=0xffff;
        return true;
      }
      if (((*blah)&0xffff)==ev->data1)
      {
        (*blah)>>=16;
        return true;
      }

      if (((*blah)&0xffff)==0)
        *blah=ev->data1;
      else if (((*blah)>>16)==0)
        *blah|=ev->data1<<16;
      else
      {
        *blah>>=16;
        *blah|=ev->data1<<16;
      }
      return true;
    }
    else
      return false;
  }

  switch (ch)
  {
/*    case KEYD_F1:
      if (!devparm)
        break;
    case KEYD_PRTSCR: // -ACB- 1998/07/10 Allow printing screen in text menu
      G_ScreenShot();
      return true;
*/
    case KEYD_BACKSPACE:
      if (currmenu[cursorpos].type==4)
        *(currmenu[cursorpos].switchvar)=0;
      return true;
    case KEYD_DOWNARROW:
      do
      {
        cursorpos++;
        if (cursorpos>=(currmenuinfo->menusize))
          cursorpos=0;
      } while (currmenu[cursorpos].type==0);
      S_StartSound(NULL, sfx_pstop);
      return true;
    case KEYD_UPARROW:
      do
      {
        cursorpos--;
        if (cursorpos<0)
          cursorpos=(currmenuinfo->menusize)-1;
      } while (currmenu[cursorpos].type==0);
      S_StartSound(NULL, sfx_pstop);
      return true;
    case KEYD_LEFTARROW:
      if (currmenu[cursorpos].type==3)
      {
        if ((*(currmenu[cursorpos].switchvar))>0)
        {
          (*(currmenu[cursorpos].switchvar))--;
          S_StartSound(NULL,sfx_stnmov);
        }
        if (currmenu[cursorpos].routine!=NULL)
          currmenu[cursorpos].routine();
        return true;
      }
      else if (currmenu[cursorpos].type==1)
      {
        (*(currmenu[cursorpos].switchvar))--;
        if ((*(currmenu[cursorpos].switchvar))<0)
          (*(currmenu[cursorpos].switchvar))=currmenu[cursorpos].numtypes-1;
        S_StartSound(NULL,sfx_pistol);
        if (currmenu[cursorpos].routine!=NULL)
          currmenu[cursorpos].routine();
        return true;
      }
      break;
    case KEYD_RIGHTARROW:
    case KEYD_ENTER:
      switch (currmenu[cursorpos].type)
      {
        case 0:
          return false;
        case 1:
          (*(currmenu[cursorpos].switchvar))++;
          if ((*(currmenu[cursorpos].switchvar))>=currmenu[cursorpos].numtypes)
            (*(currmenu[cursorpos].switchvar))=0;
          S_StartSound(NULL,sfx_pistol);
          if (currmenu[cursorpos].routine!=NULL)
            currmenu[cursorpos].routine();
          return true;
        case 2:
          if (currmenu[cursorpos].routine!=NULL)
            currmenu[cursorpos].routine();
          S_StartSound(NULL,sfx_pistol);
          return true;
        case 3:
          if ((*(currmenu[cursorpos].switchvar))<(currmenu[cursorpos].numtypes-1))
          {
            (*(currmenu[cursorpos].switchvar))++;
            S_StartSound(NULL,sfx_stnmov);
          }
          if (currmenu[cursorpos].routine!=NULL)
            currmenu[cursorpos].routine();
          return true;
        case 4:
          keyscan=1;
          return true;
      }
      I_Error("Invalid menu type!");
    case KEYD_ESCAPE:
      if (currmenu==mainmenu)
        optionsmenuon=0;
      else
      {
        currmenu=mainmenu; currmenuinfo=&mainmenuinfo;
        cursorpos=mainmenupos;
        numberofentries=mainmenusize;
      }
      S_StartSound(NULL,sfx_swtchx);
      return true;
  }
  return false;
}

static void M_VideoOptions()
{
  mainmenupos=cursorpos;
  cursorpos=0;
  currmenu=vidoptions; currmenuinfo=&vidoptionsinfo;
  numberofentries=vidoptionssize;
}

// -ES- 1998/08/20 Added resolution menu
static void M_ResolutionOptions()
{

  ressize=0;
  optbpp = BPP-1;

  if (!Resolutions)
  {
    char temp[16];
    while (screenres[ressize].width != -1) ressize++;
  
    Resolutions = Z_Malloc(ressize*10, PU_STATIC, NULL);

    sprintf(Resolutions, "%dx%d", screenres[0].width, screenres[0].height);
    for (ressize = 1; screenres[ressize].width != -1; ressize++)
    {
      sprintf(temp, "/%dx%d", screenres[ressize].width, screenres[ressize].height);
      strcat(Resolutions, temp);
    }

    resoptions[3].typenames = Resolutions;
    resoptions[3].numtypes = ressize;

    ressize = 0;
  }
  //--------------------------------------------------------------------
  // -ACB- 1998/08/29 Default the size
  while (screenres[ressize].width != -1)
  {
    if (SCREENWIDTH == screenres[ressize].width &&
         SCREENHEIGHT == screenres[ressize].height)
    {
      optwidth = SCREENWIDTH;
      optheight = SCREENHEIGHT;
      break;
    }
    ressize++;
  }

  if (screenres[ressize].width == -1)
  {
    optwidth = 320;
    optheight = 200;
    ressize = 0;
  }
  //--------------------------------------------------------------------

  mainmenupos=cursorpos;
  cursorpos=3;
  currmenu=resoptions; currmenuinfo=&resoptionsinfo;
  numberofentries=resoptionssize;
}

static void M_AnalogueOptions()
{
  mainmenupos=cursorpos;
  cursorpos=0;
  currmenu=analogueoptions; currmenuinfo=&analogueoptionsinfo;
  numberofentries=analogueoptionssize;
}

static void M_GameplayOptions()
{
  if (netgame) return;

  mainmenupos=cursorpos;
  cursorpos=0;
  currmenu=playoptions; currmenuinfo=&playoptionsinfo;
  numberofentries=playoptionssize;

}

static void M_StandardControlOptions()
{
  mainmenupos=cursorpos;
  cursorpos=2;
  currmenu=stdkeyconfig; currmenuinfo=&stdkeyconfiginfo;
  numberofentries=stdkeyconfigsize;
}

static void M_ExtendedControlOptions()
{
  mainmenupos=cursorpos;
  cursorpos=2;
  currmenu=extkeyconfig; currmenuinfo=&extkeyconfiginfo;
  numberofentries=extkeyconfigsize;
}

static void M_ResetToDefaults()
{
  int i;

  for (i=0;i<mainmenusize;i++)
    if ((mainmenu[i].type==1)||(mainmenu[i].type==3))
      (*(mainmenu[i].switchvar))=mainmenu[i].dosdoomdefault;

  for (i=0;i<vidoptionssize;i++)
    if ((vidoptions[i].type==1)||(vidoptions[i].type==3))
      (*(vidoptions[i].switchvar))=vidoptions[i].dosdoomdefault;

  for (i=0;i<playoptionssize;i++)
    if ((playoptions[i].type==1)||(playoptions[i].type==3))
      (*(playoptions[i].switchvar))=playoptions[i].dosdoomdefault;

  for (i=0;i<stdkeyconfigsize;i++)
    if ((stdkeyconfig[i].type==4))
         (*(stdkeyconfig[i].switchvar))=stdkeyconfig[i].dosdoomdefault;
}

static void M_ResetToOrigDoom()
{
  int i;

  for (i=0;i<mainmenusize;i++)
    if ((mainmenu[i].type==1)||(mainmenu[i].type==3))
      (*(mainmenu[i].switchvar))=mainmenu[i].origdoomdefault;

  for (i=0;i<vidoptionssize;i++)
    if ((vidoptions[i].type==1)||(vidoptions[i].type==3))
      (*(vidoptions[i].switchvar))=vidoptions[i].origdoomdefault;

  for (i=0;i<playoptionssize;i++)
    if ((playoptions[i].type==1)||(playoptions[i].type==3))
      (*(playoptions[i].switchvar))=playoptions[i].origdoomdefault;

  for (i=0;i<stdkeyconfigsize;i++)
    if ((stdkeyconfig[i].type==4))
         (*(stdkeyconfig[i].switchvar))=stdkeyconfig[i].dosdoomdefault;

}

static void M_Key2String(int key, char *deststring)
  {
  int key1,key2;
  char key2string[100];
  int j;

  if (((key&0xffff)==0)&&((key>>16)!=0))
    I_Error("key problem!");

  if (key==0)
    {
    strcpy(deststring,"---");
    return;
    }
  key1=key&0xffff; key2=key>>16;

  //first do key 1
  if ((toupper(key1)>=',')&&(toupper(key1)<=']'))
  {
    deststring[0]=toupper(key1);
    deststring[1]=0;
  }
  else
  {
    if (key1 >= KEYD_JOYBASE)
      sprintf (deststring, "Joystick %d", key1 - KEYD_JOYBASE + 1);
    else
      sprintf (deststring,"Keycode %d",key1);
    j=0; while (specialkeylist[j].keycode!=-1)
      {
      if (specialkeylist[j].keycode==key1)
        {
        strcpy(deststring,specialkeylist[j].keystring);
        break;
        }
      j++;
      }
    }

  if (key2==0)
    return;

  //now, do key 2
  if ((toupper(key2)>=',')&&(toupper(key2)<=']'))
  {
    key2string[0]=toupper(key2);
    key2string[1]=0;
  }
  else
  {
    if (key2 >= KEYD_JOYBASE)
      sprintf (key2string, "Joystick %d", key2 - KEYD_JOYBASE + 1);
    else
      sprintf (key2string,"Keycode %d",key2);
    j=0; while (specialkeylist[j].keycode!=-1)
      {
      if (specialkeylist[j].keycode==key2)
        {
        strcpy(key2string,specialkeylist[j].keystring);
        break;
        }
      j++;
      }
    }
  strcat(deststring," or ");
  strcat(deststring,key2string);
  return;
  }

// -KM- 1998/07/21 Change blood to a boolean
// -ACB- 1998/08/09 Check map setting allows this
static void M_ChangeBlood(void)
{
 if (!(currentmap->flags & MPF_NORMBLOOD))
   gameflags.blood = settingflags.blood;
}

// 98-7-10 KM Recalibration of Joystick
static void M_CalibrateJoystick(void)
{
  I_CalibrateJoystick(0);
}

// -ACB- 1998/08/09 New DDF settings, check that map allows the settings
static void M_ChangeMonsterRespawn(void)
{
  if (!(currentmap->flags & (MPF_RESMONSTER|MPF_TELMONSTER)))
    gameflags.respawnsetting = settingflags.respawnsetting;
}

static void M_ChangeItemRespawn(void)
{
  if (!(currentmap->flags & (MPF_ITEMRESPAWN|MPF_NOITEMRESPN)))
    gameflags.itemrespawn = settingflags.itemrespawn;
}

static void M_ChangeStretchSky(void)
{
  if (!(currentmap->flags & (MPF_NORMALSKY|MPF_STRETCHSKY)))
    gameflags.stretchsky = settingflags.stretchsky;
}

static void M_ChangeTransluc(void)
{
  if (!(currentmap->flags & MPF_NOTRANSLUC))
    gameflags.trans = settingflags.trans;
}

static void M_ChangeTrue3d(void)
{
  if (!(currentmap->flags & MPF_NOTRUE3D))
    gameflags.true3dgameplay = settingflags.true3dgameplay;
}

static void M_ChangeAutoAim(void)
{
  if (!(currentmap->flags & MPF_AUTOAIMOFF))
    gameflags.autoaim = settingflags.autoaim;
}

static void M_ChangeRespawn(void)
{
  if ((gameskill != sk_nightmare) && !(currentmap->flags & (MPF_RESPAWN|MPF_NORESPAWN)))
    gameflags.respawn = settingflags.respawn;
}

static void M_ChangeFastparm(void)
{
  if ((gameskill != sk_nightmare) && !(currentmap->flags & MPF_FAST))
    gameflags.fastparm = settingflags.fastparm;
}
// -ACB- 1998/08/29 Resolution Changes...
//
// New idea based on -ES- original work:
// so long as it does not borrow from quake
//
static void M_ChangeStoredRes(void)
{
  optwidth = screenres[ressize].width;
  optheight = screenres[ressize].height;
}

static void M_OptionSetResolution()
{
  oldwidth = SCREENWIDTH;
  oldheight = SCREENHEIGHT;
  oldbpp = BPP;
  R_ChangeResolution(optwidth, optheight, optbpp+1);
}

static void M_OptionTestResolution()
{
  oldwidth = SCREENWIDTH;
  oldheight = SCREENHEIGHT;
  oldbpp = BPP;

  R_ChangeResolution(optwidth, optheight, optbpp+1);
  testticker = TICRATE * 5;
}

static void M_RestoreResSettings()
{
  R_ChangeResolution(oldwidth, oldheight, oldbpp);
}


static void M_SetCDAudio(void)
{
 static int oldcdaudio = -1;

 if (oldcdaudio == -1)
   oldcdaudio = -1;

 if (cdaudio == 2)
 {
   if (oldcdaudio < cdaudio)
    cdaudio = 3;
   else
    cdaudio = 1;
 }
 oldcdaudio = cdaudio;

 I_StartCDAudio();
}

