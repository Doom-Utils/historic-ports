#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "multires.h"
#include "hu_stuff.h"
#include "doomstat.h"
#include "s_sound.h"
#include "r_main.h"
#include "i_video.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"
#include "m_menu.h"
#include "sounds.h"
#include "m_misc.h"

int optionsmenuon=0;
//submenus
void VideoOptions();
void GameplayOptions();
void CustomizeControls();
void Reset2Defaults();
void Reset2OrigDoom();

void Key2String(int key, char *deststring);

//Special function declarations
int screenSize,Respawn;
void ChangeScreenSize() {R_SetViewSize(screenSize+3,0);}
void ChangeSfxVol()     {S_SetSfxVolume(snd_SfxVolume);}
void ChangeMusVol()     {S_SetMusicVolume(snd_MusicVolume);}
void ChangeGamma()      {I_SetPalette(W_CacheLumpName("PLAYPAL",PU_CACHE),0);}

//now, the menus themselves
char YesNo[]="Off/On";
char TrnsO[]="Off/With Map/On"; //translucency options
char CrosO[]="None/Cross/Dot/Angle"; //crosshair options

struct menuinfo
  {
  int menusize;
  int menucenter;
  char menuname[80];
  };

struct optmenuitem
  {
  int type; //0 means plain text, 1 is change a switch, 2 is call a function,
            //3 is a slider, 4 is a key config
  char name[48];
  char *typenames;
  int numtypes;
  int dosdoomdefault;
  int origdoomdefault;
  int* switchvar;
  void (*routine)();
  };

struct specialkey
  {
  int keycode;
  char keystring[20];
  };

#define mainmenusize 15
struct menuinfo mainmenuinfo={mainmenusize,200,""};
struct optmenuitem mainmenu[mainmenusize]=
 {{2,"Video Options",           NULL, 0, 0, 0, NULL,VideoOptions},
  {2,"Gameplay Options",        NULL, 0, 0, 0, NULL,GameplayOptions},
  {2,"Customize Controls",      NULL, 0, 0, 0, NULL,CustomizeControls},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {1,"Messages",                YesNo,2, 1, 1, &showMessages,NULL},
  {1,"Swap Stereo",             YesNo,2, 0, 0, &swapstereo,NULL},
  {3,"Sound Volume",            NULL, 16,12,12,&snd_SfxVolume,ChangeSfxVol},
  {3,"Music Volume",            NULL, 16,12,12,&snd_MusicVolume,ChangeMusVol},
  {3,"MouseSpeed",              NULL, 20,8, 8, &mouseSensitivity,NULL},
  {1,"FreeLook",                YesNo,2, 1, 0, &mlookon,NULL},
  {1,"Invert Mouse",            YesNo,2, 0, 0, &invertmouse,NULL},
  {3,"Mlook Speed",             NULL, 20,8, 0, &keylookspeed,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {2,"Reset to Recommended",    NULL, 0, 0, 0, NULL,Reset2Defaults},
  {2,"Reset to Original Doom",  NULL, 0, 0, 0, NULL,Reset2OrigDoom}};

#define vidoptionssize 7
struct menuinfo vidoptionsinfo={vidoptionssize,200,"Video Options"};
struct optmenuitem vidoptions[vidoptionssize]=
 {{3,"Screensize",              NULL, 9, 7, 7, &screenSize,ChangeScreenSize},
  {3,"Brightness",              NULL, 5, 0, 0, &usegamma,ChangeGamma},
  {1,"Translucency",            TrnsO,3, 2, 1, &transluc,NULL},
  {1,"Crosshair",               CrosO,4, 2, 0, &crosshair,NULL},
  {1,"Stretch Sky for Mlook",   YesNo,2, 1, 0, &stretchsky,NULL},
  {1,"Map Rotation",            YesNo,2, 1, 0, &rotatemap,NULL},
  {1,"Map Overlay",             YesNo,2, 1, 0, &newhud,NULL}};

#define playoptionssize 10
struct menuinfo playoptionsinfo={playoptionssize,200,"Gameplay Options"};
struct optmenuitem playoptions[playoptionssize]=
 {{3,"Gravity",                 NULL, 20,8, 8, &grav,NULL},
  {1,"Shoot up/down",           YesNo,2, 1, 0, &shootupdown,NULL},
  {1,"New Monster Respawn",     YesNo,2, 1, 0, &newnmrespawn,NULL},
  {1,"Item Respawning",         YesNo,2, 0, 0, &ItemRespawn,NULL},
  {1,"Random Infighting",       YesNo,2, 1, 0, &RandomInfight,NULL},
  {1,"Full Monster Infighting", YesNo,2, 0, 0, &TotalWar,NULL},
  {1,"Enhanced Monster AI",     YesNo,2, 1, 0, &NewAI,NULL},
  {1,"Less Accurate Monsters",  YesNo,2, 0, 0, &LessAccurateMon,NULL},
  {1,"Human Madness",           YesNo,2, 0, 0, &HumanMad,NULL},
  {1,"Human Explosions",        YesNo,2, 0, 0, &HumanExplode,NULL}};

#define keyconfigsize 19
struct menuinfo keyconfiginfo={keyconfigsize,110,"Customize Controls"};
struct optmenuitem keyconfig[keyconfigsize]=
 {{0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {0,"",                        NULL, 0, 0, 0, NULL,NULL},
  {4,"Attack",                  NULL, 0, KEYD_RCTRL, 0, &key_fire,NULL},
  {4,"Use",                     NULL, 0, ' ',0,&key_use,NULL},
  //not yet done
//  {4,"Change Weapon",           NULL, 0, '/',0,&key_nextweapon,NULL},
  {4,"Walk Forward",            NULL, 0, KEYD_UPARROW,0, &key_up,NULL},
  {4,"Backpedal",               NULL, 0, KEYD_DOWNARROW, 0, &key_down,NULL},

  {4,"Turn Left",               NULL, 0, KEYD_LEFTARROW, 0, &key_left,NULL},
  {4,"Turn Right",              NULL, 0, KEYD_RIGHTARROW, 0, &key_right,NULL},
  {4,"180 deg turn",            NULL, 0, 0, 0, &key_180,NULL},
  {4,"Run",                     NULL, 0, KEYD_RSHIFT, 0, &key_speed,NULL},
  {4,"Step Left",               NULL, 0, ',', 0, &key_strafeleft,NULL},
  {4,"Step Right",              NULL, 0, '.', 0, &key_straferight,NULL},

  {4,"Sidestep",                NULL, 0, KEYD_RALT, 0, &key_strafe,NULL},
  {4,"Look Up",                 NULL, 0, KEYD_PGUP, 0, &key_lookup,NULL},
  {4,"Look Down",               NULL, 0, KEYD_PGDN, 0, &key_lookdown,NULL},
  {4,"Center View",             NULL, 0, KEYD_HOME, 0, &key_lookcenter,NULL},
  {4,"Map Toggle",              NULL, 0, KEYD_TAB,0,&key_map,NULL},
  {4,"Multiplay Talk",          NULL, 0, 't',0,&key_talk,NULL},
  {4,"Jump",                    NULL, 0, 0, 0, &key_jump,NULL}};

char keystring1[]="Enter to change, Backspace to Clear";
char keystring2[]="Press a key for this action";

struct specialkey specialkeylist[]= //terminate on -1
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
  {KEYD_PGDN,       "PageDown"},
  {KEYD_PGUP,       "PageUp"},
  {KEYD_HOME,       "Home"},
  {KEYD_END,        "End"},
  {32,              "Space"},
  {'\'',            "\'"},
  {'`',             "Tilde"},
  {KEYD_MOUSE1,     "Mouse1"},
  {KEYD_MOUSE2,     "Mouse2"},
  {KEYD_MOUSE3,     "Mouse3"},
  {KEYD_JOY1,       "Joystick1"},
  {KEYD_JOY2,       "Joystick2"},
  {KEYD_JOY3,       "Joystick3"},
  {KEYD_JOY4,       "Joystick4"},
  {-1,              ""}};

struct optmenuitem* currmenu;
struct menuinfo* currmenuinfo;
int cursorpos,mainmenupos;

int keyscan;

void InitOptmenu()
  {
  optionsmenuon=0;
  screenSize=screenblocks-3;
  currmenu=mainmenu; currmenuinfo=&mainmenuinfo;
  cursorpos=mainmenupos=0;
  keyscan=0;
  }

void Opt_Drawer()
  {
  char tempstring[80];
  int curry,deltay;
  int i,j,k;

  //first, draw the title
  if (currmenu==mainmenu)
    V_DrawPatchInDirect (108,15,0,W_CacheLumpName("M_OPTTTL",PU_CACHE));
  else
    M_WriteText(160-(M_StringWidth(currmenuinfo->menuname)/2),22,currmenuinfo->menuname);

  //now, draw all the menuitems
  deltay=1+hu_font[0]->height;
  curry=40;
  if (currmenu==keyconfig)
    {
    if (keyscan)
      M_WriteText(160-(M_StringWidth(keystring2)/2),40,keystring2);
    else
      M_WriteText(160-(M_StringWidth(keystring1)/2),40,keystring1);
    }
  for (i=0;i<(currmenuinfo->menusize);i++)
    {
    M_WriteText((currmenuinfo->menucenter)-M_StringWidth(currmenu[i].name),curry,currmenu[i].name);
    if (i==cursorpos)
      M_WriteText((currmenuinfo->menucenter+5),curry,"*");
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
        if (k>=strlen(currmenu[i].typenames))
          I_Error("Invalid Switch Name");
        j=0;
        while ((currmenu[i].typenames[k]!='/')&&(k<strlen(currmenu[i].typenames)))
          {
          tempstring[j]=currmenu[i].typenames[k];
          j++; k++;
          }
        tempstring[j]=0;
        M_WriteText((currmenuinfo->menucenter)+15,curry,tempstring);
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
        Key2String(k,tempstring);
        M_WriteText((currmenuinfo->menucenter+15),curry,tempstring);
        break;
      }
    curry+=deltay;
    }
  }

boolean Opt_Responder (event_t *ev, int ch)
  {
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
    case KEYD_BACKSPACE:
      if (currmenu[cursorpos].type==4)
        *(currmenu[cursorpos].switchvar)=0;
      return true;
    case KEYD_DOWNARROW:
      do{
        cursorpos++;
        if (cursorpos>=(currmenuinfo->menusize))
          cursorpos=0;
        } while (currmenu[cursorpos].type==0);
      return true;
    case KEYD_UPARROW:
      do{
        cursorpos--;
        if (cursorpos<0)
          cursorpos=(currmenuinfo->menusize)-1;
        } while (currmenu[cursorpos].type==0);
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
        case 2:
          if (currmenu[cursorpos].routine!=NULL)
            currmenu[cursorpos].routine();
          S_StartSound(NULL,sfx_pistol);
          return true;
        case 1:
          (*(currmenu[cursorpos].switchvar))++;
          if ((*(currmenu[cursorpos].switchvar))>=currmenu[cursorpos].numtypes)
            (*(currmenu[cursorpos].switchvar))=0;
          S_StartSound(NULL,sfx_pistol);
          if (currmenu[cursorpos].routine!=NULL)
            currmenu[cursorpos].routine();
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
        }
      S_StartSound(NULL,sfx_swtchx);
      return true;
    }
  return false;
  }

void VideoOptions()
  {
  mainmenupos=cursorpos;
  cursorpos=0;
  currmenu=vidoptions; currmenuinfo=&vidoptionsinfo;
  }

void GameplayOptions()
  {
  mainmenupos=cursorpos;
  cursorpos=0;
  currmenu=playoptions; currmenuinfo=&playoptionsinfo;
  }

void CustomizeControls()
  {
  mainmenupos=cursorpos;
  cursorpos=2;
  currmenu=keyconfig; currmenuinfo=&keyconfiginfo;
  }

void Reset2Defaults()
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
  for (i=0;i<keyconfigsize;i++)
    if ((keyconfig[i].type==4))
      (*(keyconfig[i].switchvar))=keyconfig[i].dosdoomdefault;
  }

void Reset2OrigDoom()
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
  for (i=0;i<keyconfigsize;i++)
    if ((keyconfig[i].type==4))
      (*(keyconfig[i].switchvar))=keyconfig[i].dosdoomdefault;
  printf("abba\n");
  }

void Key2String(int key, char *deststring)
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

