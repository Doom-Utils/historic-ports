// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_misc.c,v 1.62 1998/09/07 20:19:07 jim Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//  Main loop menu stuff.
//  Default Config File.
//  PCX Screenshots.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_misc.c,v 1.62 1998/09/07 20:19:07 jim Exp $";

#include "doomstat.h"
#include "m_argv.h"
#include "g_game.h"
#include "m_menu.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "dstrings.h"
#include "m_misc.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t* hu_font[HU_FONTSIZE];

int M_DrawText(int x,int y,boolean direct,char* string)
  {
  int c;
  int w;

  while (*string)
    {
    c = toupper(*string) - HU_FONTSTART;
    string++;
    if (c < 0 || c> HU_FONTSIZE)
      {
      x += 4;
      continue;
      }
    
    w = SHORT (hu_font[c]->width);
    if (x+w > SCREENWIDTH)
      break;
    if (direct)
      V_DrawPatchDirect(x, y, 0, hu_font[c]);
    else
      V_DrawPatch(x, y, 0, hu_font[c]);
    x+=w;
    }

  return x;
  }


//
// M_WriteFile
//

#ifndef O_BINARY
#define O_BINARY 0
#endif

boolean M_WriteFile(char const* name,void* source,int length)
  {
  int handle;
  int count;
  
  handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

  if (handle == -1)
    return false;

  count = write (handle, source, length);
  close (handle);
  
  if (count < length)
    return false;
    
  return true;
  }


//
// M_ReadFile
//

int M_ReadFile(char const* name,byte** buffer)
  {
  int handle, count, length;
  struct stat fileinfo;
  byte   *buf;
  
  handle = open (name, O_RDONLY | O_BINARY, 0666);
  if (handle == -1)
    I_Error ("Couldn't read file %s", name);
  if (fstat (handle,&fileinfo) == -1)
    I_Error ("Couldn't read file %s", name);
  length = fileinfo.st_size;
  buf = Z_Malloc (length, PU_STATIC, NULL);
  count = read (handle, buf, length);
  close (handle);
  
  if (count < length)
    I_Error ("Couldn't read file %s", name);
    
  *buffer = buf;
  return length;
  }

//
// DEFAULTS
//

int config_help; //jff 3/3/98
int usemouse;
int usejoystick;

//jff 3/30/98
int screenshot_pcx;      // option to output screenshot as pcx or bmp

extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;

extern int joybfire;
extern int joybstrafe;
extern int joybuse;
extern int joybspeed;

extern int viewwidth;
extern int viewheight;

extern int mouseSensitivity_horiz,mouseSensitivity_vert;  // killough

extern int realtic_clock_rate;         // killough 4/13/98: adjustable timer
extern int leds_always_off;            // killough 3/6/98
extern int tran_filter_pct;            // killough 2/21/98

extern int screenblocks;
extern int showMessages;

extern boolean pause_init;             //jff 8/3/98 logical output controls

#ifdef LINUX
char*  mousetype;
char*  mousedev;
#endif

extern char* chat_macros[];

#define UL (-123456789) /* magic number for no min or max for parameter */

//jff 3/3/98 added min, max, and help string to all entries
//jff 4/10/98 added isstr field to specify whether value is string or int
default_t defaults[] =
{
    {"config_help",&config_help,1,
     0,1,0,ss_none,   "[0/1(1)] 1 to show help strings about each variable in config file"},   //jff 3/3/98

    {"orig_doom_compatibility",&default_compatibility,0,
     0,1,0,ss_none,   "[0/1(0)] 1 to make the game as close to original DOOM as practical"},    // killough

    {"default_skill", &defaultskill, 3, // jff 3/24/98 allow default skill setting
     1,5,0,ss_none,   "[1-5(3)] selects default skill 1=TYTD 2=NTR 3=HMP 4=UV 5=NM"},

    {"sound_card", &snd_card, -1,                            // jff 1/18/98 allow Allegro drivers
     -1,7,0,ss_none,   "[-1-7(-1)] code used by Allegro to select sounds driver, -1 is autodetect"}, 
    {"music_card", &mus_card, -1,                            //  to be set,  -1 = autodetect
     -1,9,0,ss_none,   "[-1-9(-1)] code used by Allegro to select music driver, -1 is autodetect"},  
    {"detect_voices", &detect_voices, 1,                    // jff 3/4/98 detect # voices
     0,1,0,ss_none,   "[0/1(1)] 1 enables voice detection prior to calling install sound"},  
    {"use_vsync",  &use_vsync, 1,             // killough 2/8/98
     0,1,0,ss_none,    "[0/1(1)] 1 to enable wait for vsync to avoid display tearing"},

    {"realtic_clock_rate", &realtic_clock_rate, 100,
     10,1000,0,ss_none, "[10/1000(100)] Percentage of normal speed (35 fps) realtic clock runs at"},

    {"pitched_sounds", &default_pitched_sounds, 0,
     0,1,0,ss_none,   "[0/1(0)] 1 to enable variable pitch in sound effects (from id's original code)"}, // killough 2/21/98

    {"translucency",&default_translucency, 1,
     0,1,0,ss_none,   "[0/1(1)] 1 to enable translucency for some things"},   // phares
    {"tran_filter_pct",&tran_filter_pct, 66,                        // killough 2/21/98
     0,100,0,ss_none, "[0-100(66)] set percentage of foreground/background translucency mix"},  
    {"max_player_corpse", &bodyquesize, 32,   // killough 2/8/98
     UL,UL,0,ss_none,   "[?-?(32)] number of dead bodies in view supported (negative value = no limit)"},

    {"demo_insurance",   &default_demo_insurance, 2,   // killough 3/31/98
     0,2,0,ss_none,      "[0-2(2)] 1=take special steps ensuring demo sync, 2=only during recordings"},

// phares 3/10/98: replaced with cheat code
//    {"variable_friction",&default_variable_friction, 1,
//     0,1,0,   "[0/1(1)] 1 to enable icy and sludgy floors"},        // phares

    {"weapon_recoil",&default_weapon_recoil, 1,
     0,1,0,ss_weap,   "[0/1(1)] 1 to enable recoil from weapon fire"},        // phares

// phares 3/10/98: replaced with cheat code
//    {"allow_pushers",&default_allow_pushers, 1,
//     0,1,0,   "[0/1(1)] 1 to enable wind/current sector type"},     // phares

    {"player_bobbing",&default_player_bobbing, 1,                   // phares 2/25/98
     0,1,0,ss_weap,   "[0/1(1)] 1 to enable player bobbing (view randomly moving up/down slightly)"},   

    {"monsters_remember",&default_monsters_remember, 1,              // killough 3/1/98
     0,1,0,ss_enem,   "[0/1(1)] 1 to enable monsters remembering enemies after killing others"},  

    {"sts_always_red",&sts_always_red, 0, // no color changes on status bar
     0,1,0,ss_stat,    "[0/1(0)] 1 to disable use of color on status bar"},

    {"sts_pct_always_gray",&sts_pct_always_gray, 1, // 2/23/98 chg default
     0,1,0,ss_stat,    "[0/1(1)] 1 to make percent signs on status bar always gray"},

    {"sts_traditional_keys", &sts_traditional_keys, 0,
     0,1,0,ss_stat,    "[0/1(0)] 1 to disable doubled card and skull key display on status bar"},    // killough 2/28/98

    {"traditional_menu", &traditional_menu, 0,
     0,1,0,ss_none,    "[0/1(0)] 1 to use Doom's main menu ordering"},   // killough 4/17/98

    {"leds_always_off", &leds_always_off, 0,
         0,1,0,ss_none,     "[0/1(0)] 1 to keep keyboard LEDs turned off"}, // killough 3/6/98

    //jff 4/3/98 allow unlimited sensitivity
    {"mouse_sensitivity_horiz",&mouseSensitivity_horiz, 5,
     0,UL,0,ss_none,  "[0-?(5)] adjust horizontal (x) mouse sensitivity"},  // killough
    //jff 4/3/98 allow unlimited sensitivity
    {"mouse_sensitivity_vert",&mouseSensitivity_vert, 5,
     0,UL,0,ss_none,  "[0-?(5)] adjust vertical (y) mouse sensitivity"},    // killough

    {"sfx_volume",&snd_SfxVolume, 8,
     0,15,0,ss_none,  "[0-15(8)] adjust sound effects volume"},

    {"music_volume",&snd_MusicVolume, 8,
     0,15,0,ss_none,  "[0-15(8)] adjust music volume"},

    {"show_messages",&showMessages, 1,
     0,1,0,ss_none,   "[0/1(1)] 1 to enable message display"},

#ifdef NORMALUNIX
    // killough 3/6/98: preserve autorun across games
    {"autorun", &autorun, 0,
         0,1,0,ss_none,    "[0/1(0)] 1 to enable autorun"},

    {"screenblocks",&screenblocks, 10,
     3,11,0,ss_none,   "[3-11(10)] initial play screen size"},  // killough 2/21/98: default to 10

//    {"detaillevel",&detailLevel, 0},   // obsolete -- killough

    {"usegamma",&usegamma, 3, //jff 3/6/98 fix erroneous upper limit in range
     0,4,0,ss_none,    "[0-4(3)] screen brightness (gamma correction)"},    // killough 1/18/98

// For key bindings, the values stored in the key_* variables       // phares
// are the internal Doom Codes. The values stored in the default.cfg
// file are the keyboard codes. I_ScanCode2DoomCode converts from
// keyboard codes to Doom Codes. I_DoomCode2ScanCode converts from
// Doom Codes to keyboard codes, and is only used when writing back 
// to default.cfg. For the printable keys (i.e. alphas, numbers)
// the Doom Code is the ascii code.

    {"key_right",        &key_right,           KEYD_RIGHTARROW,
     0,255,0,ss_keys,   "[0-255(77)] key to turn right"},
    {"key_left",         &key_left,            KEYD_LEFTARROW ,
     0,255,0,ss_keys,   "[0-255(75)] key to turn left"},
    {"key_up",           &key_up,              KEYD_UPARROW   ,
     0,255,0,ss_keys,   "[0-255(72)] key to move forward"},
    {"key_down",         &key_down,            KEYD_DOWNARROW ,
     0,255,0,ss_keys,   "[0-255(80)] key to move backward"},
    {"key_menu_right",   &key_menu_right,      KEYD_RIGHTARROW,// phares 3/7/98
     0,255,0,ss_keys,   "[0-255(77)] key to move right in a menu"},      //     |
    {"key_menu_left",    &key_menu_left,       KEYD_LEFTARROW ,//     V
     0,255,0,ss_keys,   "[0-255(75)] key to move left in a menu"},
    {"key_menu_up",      &key_menu_up,         KEYD_UPARROW   ,
     0,255,0,ss_keys,   "[0-255(72)] key to move up in a menu"},
    {"key_menu_down",    &key_menu_down,       KEYD_DOWNARROW ,
     0,255,0,ss_keys,   "[0-255(80)] key to move down in a menu"},
    {"key_menu_backspace",&key_menu_backspace, KEYD_BACKSPACE ,
     0,255,0,ss_keys,   "[0-255(239)] key to erase last character typed in a menu"},
    {"key_menu_escape",  &key_menu_escape,     KEYD_ESCAPE    ,
     0,255,0,ss_keys,   "[0-255(129)] key to leave a menu"},           // phares 3/7/98
    {"key_menu_enter",   &key_menu_enter,      KEYD_ENTER     ,
     0,255,0,ss_keys,   "[0-255(224)] key to select from menu or review past messages"},
    {"key_strafeleft",   &key_strafeleft,      ','            ,
     0,255,0,ss_keys,   "[0-255(179)] key to strafe left (sideways left)"},
    {"key_straferight",  &key_straferight,     '.'            ,
     0,255,0,ss_keys,   "[0-255(180)] key to strafe right (sideways right)"},

    {"key_fire",         &key_fire,            KEYD_RCTRL     ,
     0,255,0,ss_keys,   "[0-255(120)] key to fire current weapon"},
    {"key_use",          &key_use,             ' '            ,
     0,255,0,ss_keys,   "[0-255(185)] key to open a door, use a switch"},
    {"key_strafe",       &key_strafe,          KEYD_RALT      ,
     0,255,0,ss_keys,   "[0-255(121)] key to use with arrows to strafe"},
    {"key_speed",        &key_speed,           KEYD_RSHIFT    ,
     0,255,0,ss_keys,   "[0-255(54)] key to run (move fast)"},

    // phares 4/13/98:
    // key_escape is now set to KEYD_ESCAPE and left there. This ensures that
    // the player can always get at the menus.

//  {"key_escape",       &key_escape,          KEYD_ESCAPE    ,
//   0,255,0,   "[0-255(129)] key to start control panel (options)"},

    // phares 4/13/98:
    // key_help is now set to KEYD_F1 and left there. This ensures that the
    // player can always get to the help screen.

//  {"key_help",         &key_help,            KEYD_F1        ,
//   0,255,0,ss_keys,   "[0-255(59)] key to view help screen"},

    {"key_savegame",     &key_savegame,        KEYD_F2        ,
     0,255,0,ss_keys,   "[0-255(60)] key to save current game"},
    {"key_loadgame",     &key_loadgame,        KEYD_F3        ,
     0,255,0,ss_keys,   "[0-255(61)] key to restore from saved games"},
    {"key_soundvolume",  &key_soundvolume,     KEYD_F4        ,
     0,255,0,ss_keys,   "[0-255(62)] key to bring up sound control panel"},
    {"key_hud",          &key_hud,             KEYD_F5        ,
     0,255,0,ss_keys,   "[0-255(63)] key to adjust heads up display mode"},
    {"key_quicksave",    &key_quicksave,       KEYD_F6        ,
     0,255,0,ss_keys,   "[0-255(64)] key to to save to last slot saved"},
    {"key_endgame",      &key_endgame,         KEYD_F7        ,
     0,255,0,ss_keys,   "[0-255(65)] key to end the game"},
    {"key_messages",     &key_messages,        KEYD_F8        ,
     0,255,0,ss_keys,   "[0-255(66)] key to toggle message enable"},
    {"key_quickload",    &key_quickload,       KEYD_F9        ,
     0,255,0,ss_keys,   "[0-255(67)] key to load from quick saved game"},
    {"key_quit",         &key_quit,            KEYD_F10       ,
     0,255,0,ss_keys,   "[0-255(68)] key to quit game to DOS"},
    {"key_gamma",        &key_gamma,           KEYD_F11       ,
     0,255,0,ss_keys,   "[0-255(87)] key to adjust screen brightness (gamma correction)"},
    {"key_spy",          &key_spy,             KEYD_F12       ,
     0,255,0,ss_keys,   "[0-255(88)] key to view from another player's vantage"},
    {"key_pause",        &key_pause,           KEYD_PAUSE     ,
     0,255,0,ss_keys,   "[0-255(127)] key to pause the game"},
    {"key_autorun",      &key_autorun,         KEYD_CAPSLOCK  ,
     0,255,0,ss_keys,   "[0-255(58)] key to toggle always run mode"},
    {"key_chat",         &key_chat,            't'            ,
     0,255,0,ss_keys,   "[0-255(20)] key to enter a chat message"},
    {"key_backspace",    &key_backspace,       KEYD_BACKSPACE ,
     0,255,0,ss_keys,   "[0-255(239)] key to erase last character typed"},
    {"key_enter",        &key_enter,           KEYD_ENTER     ,
     0,255,0,ss_keys,   "[0-255(224)] key to select from menu or review past messages"},
    {"key_map",          &key_map,             KEYD_TAB       ,
     0,255,0,ss_keys,   "[0-255(143)] key to toggle automap display"},
    {"key_map_right",    &key_map_right,       KEYD_RIGHTARROW,// phares 3/7/98
     0,255,0,ss_keys,   "[0-255(77)] key to shift automap right"},      //     |
    {"key_map_left",     &key_map_left,        KEYD_LEFTARROW ,//     V
     0,255,0,ss_keys,   "[0-255(75)] key to shift automap left"},
    {"key_map_up",       &key_map_up,          KEYD_UPARROW   ,
     0,255,0,ss_keys,   "[0-255(72)] key to shift automap up"},
    {"key_map_down",     &key_map_down,        KEYD_DOWNARROW ,
     0,255,0,ss_keys,   "[0-255(80)] key to shift automap down"},
    {"key_map_zoomin",   &key_map_zoomin,      '='            ,
     0,255,0,ss_keys,   "[0-255(141)] key to enlarge automap"},
    {"key_map_zoomout",  &key_map_zoomout,     '-'            ,
     0,255,0,ss_keys,   "[0-255(202)] key to reduce automap"},
    {"key_map_gobig",    &key_map_gobig,       '0'            ,
     0,255,0,ss_keys,   "[0-255(139)] key to get max zoom for automap"},
    {"key_map_follow",   &key_map_follow,      'f'            ,
     0,255,0,ss_keys,   "[0-255(33)] key to toggle scrolling/moving with automap"},
    {"key_map_mark",     &key_map_mark,        'm'            ,
     0,255,0,ss_keys,   "[0-255(50)] key to drop a marker on automap"},
    {"key_map_clear",    &key_map_clear,       'c'            ,
     0,255,0,ss_keys,   "[0-255(46)] key to clear all markers on automap"},
    {"key_map_grid",     &key_map_grid,        'g'            ,
     0,255,0,ss_keys,   "[0-255(34)] key to toggle grid display over automap"},
    {"key_reverse",      &key_reverse,         '/'            ,
     0,255,0,ss_keys,   "[0-255(250)] key to spin 180 instantly"},
    {"key_zoomin",       &key_zoomin,          '='            ,
     0,255,0,ss_keys,   "[0-255(141)] key to enlarge display"},
    {"key_zoomout",      &key_zoomout,         '-'            ,
     0,255,0,ss_keys,   "[0-255(202)] key to reduce display"},
    {"key_chatplayer1",  &destination_keys[0], 'g'            ,
     0,255,0,ss_keys,   "[0-255(34)] key to chat with player 1"},
    {"key_chatplayer2",  &destination_keys[1], 'b'            ,
     0,255,0,ss_keys,   "[0-255(48)] key to chat with player 2"},
    {"key_chatplayer3",  &destination_keys[2], 'i'            ,
     0,255,0,ss_keys,   "[0-255(23)] key to chat with player 3"},
    {"key_chatplayer4",  &destination_keys[3], 'r'            ,
     0,255,0,ss_keys,   "[0-255(19)] key to chat with player 4"},
    {"key_weapontoggle", &key_weapontoggle,    '0'            ,
     0,255,0,ss_keys,   "[0-255(139)] key to toggle between two most preferred weapons with ammo"},
    {"key_weapon1",      &key_weapon1,         '1'            ,
     0,255,0,ss_keys,   "[0-255(130)] key to switch to weapon 1 (fist/chainsaw)"},
    {"key_weapon2",      &key_weapon2,         '2'            ,
     0,255,0,ss_keys,   "[0-255(131)] key to switch to weapon 2 (pistol)"},
    {"key_weapon3",      &key_weapon3,         '3'            ,
     0,255,0,ss_keys,   "[0-255(132)] key to switch to weapon 3 (supershotgun/shotgun)"},
    {"key_weapon4",      &key_weapon4,         '4'            ,
     0,255,0,ss_keys,   "[0-255(133)] key to switch to weapon 4 (chaingun)"},
    {"key_weapon5",      &key_weapon5,         '5'            ,
     0,255,0,ss_keys,   "[0-255(134)] key to switch to weapon 5 (rocket launcher)"},
    {"key_weapon6",      &key_weapon6,         '6'            ,
     0,255,0,ss_keys,   "[0-255(135)] key to switch to weapon 6 (plasma rifle)"},
    {"key_weapon7",      &key_weapon7,         '7'            ,
     0,255,0,ss_keys,   "[0-255(136)] key to switch to weapon 7 (bfg9000)"},         //    ^
    {"key_weapon8",      &key_weapon8,         '8'            ,
     0,255,0,ss_keys,   "[0-255(137)] key to switch to weapon 8 (chainsaw)"},        //    |
    {"key_weapon9",      &key_weapon9,         '9'            ,
     0,255,0,ss_keys,   "[0-255(138)] key to switch to weapon 9 (supershotgun)"},    // phares

    // killough 2/22/98: screenshot key
    {"key_screenshot",   &key_screenshot,      '*'            ,
     0,255,0,ss_keys,   "[0-255(183)] key to take a screenshot (devparm independent)"},
    // jff 3/30/98 add ability to take screenshots in BMP format
    {"screenshot_pcx",   &screenshot_pcx,       1            ,
     0,255,0,ss_none,   "[0/1(1)] 1 to take a screenshot in PCX format, 0 for BMP"}, 

#ifdef LINUX
    {"mousedev", (int*)&mousedev, (int)"/dev/ttyS0",
     UL,UL,1,ss_none,  "[(\"/dev/ttyS0\")] name of mouse device for Linux"},
    {"mousetype", (int*)&mousetype, (int)"microsoft",
     UL,UL,1,ss_none,   "[(\"microsoft\")] mousetype name for Linux"},
#endif
#endif

    {"use_mouse",&usemouse, 1,
     0,1,0,ss_none,    "[0/1(1)] 1 to enable use of mouse with DOOM"},
    //jff 3/8/98 allow -1 in mouse bindings to disable mouse function
    {"mouseb_fire",&mousebfire,0,
     -1,2,0,ss_keys,    "[-1-2(0)] mouse button number to use for fire"},
    {"mouseb_strafe",&mousebstrafe,1,
     -1,2,0,ss_keys,    "[-1-2(1)] mouse button number to use for strafing"},
    {"mouseb_forward",&mousebforward,2,
     -1,2,0,ss_keys,    "[-1-2(2)] mouse button number to use for forward motion"},
    //jff 3/8/98 end of lower range change for -1 allowed in mouse binding

    {"use_joystick",&usejoystick, 0,
     0,1,0,ss_none,    "[0/1(0)] 1 to enable use of joystick with BOOM"},
    {"joyb_fire",&joybfire,0,
     0,UL,0,ss_keys,   "[0-?(0)] joystick button number to use for fire"},
    {"joyb_strafe",&joybstrafe,1,
     0,UL,0,ss_keys,   "[0-?(1)] joystick button number to use for strafing"},
    {"joyb_speed",&joybspeed,2,
     0,UL,0,ss_keys,   "[0-?(2)] joystick button number to use for running"},
    {"joyb_use",&joybuse,3,
     0,UL,0,ss_keys,   "[0-?(3)] joystick button number to use for use/open"},

    {"snd_channels",&numChannels, 32,
     1,UL,0,ss_none,   "[1-?(32)] number of audio events simultaneously"},  // killough

    {"chatmacro0", (int *) &chat_macros[0], (int) HUSTR_CHATMACRO0,
     UL,UL,1,ss_chat,  "[(\"No\")] chat string associated with 0 key"},
    {"chatmacro1", (int *) &chat_macros[1], (int) HUSTR_CHATMACRO1,
     UL,UL,1,ss_chat,  "[(\"I'm ready to kick butt!\")] chat string associated with 1 key"},
    {"chatmacro2", (int *) &chat_macros[2], (int) HUSTR_CHATMACRO2,
     UL,UL,1,ss_chat,  "[(\"I'm OK.\")] chat string associated with 2 key"},
    {"chatmacro3", (int *) &chat_macros[3], (int) HUSTR_CHATMACRO3,
     UL,UL,1,ss_chat,  "[(\"I'm not looking too good!\")] chat string associated with 3 key"},
    {"chatmacro4", (int *) &chat_macros[4], (int) HUSTR_CHATMACRO4,
     UL,UL,1,ss_chat,  "[(\"Help!\")] chat string associated with 4 key"},
    {"chatmacro5", (int *) &chat_macros[5], (int) HUSTR_CHATMACRO5,
     UL,UL,1,ss_chat,  "[(\"You suck!\")] chat string associated with 5 key"},
    {"chatmacro6", (int *) &chat_macros[6], (int) HUSTR_CHATMACRO6,
     UL,UL,1,ss_chat,  "[(\"Next time, scumbag...\")] chat string associated with 6 key"},
    {"chatmacro7", (int *) &chat_macros[7], (int) HUSTR_CHATMACRO7,
     UL,UL,1,ss_chat,  "[(\"Come here!\")] chat string associated with 7 key"},
    {"chatmacro8", (int *) &chat_macros[8], (int) HUSTR_CHATMACRO8,
     UL,UL,1,ss_chat,  "[(\"I'll take care of it.\")] chat string associated with 8 key"},
    {"chatmacro9", (int *) &chat_macros[9], (int) HUSTR_CHATMACRO9,
     UL,UL,1,ss_chat,  "[(\"Yes\")] chat string associated with 9 key"},

  //jff 1/7/98 defaults for automap colors
  //jff 4/3/98 remove -1 in lower range, 0 now disables new map features
    {"mapcolor_back", &mapcolor_back, 247,  // black //jff 4/6/98 new black
     0,255,0,ss_auto,   "[0-255(247)] color used as background for automap"},
    {"mapcolor_grid", &mapcolor_grid, 104,  // dk gray
     0,255,0,ss_auto,   "[0-255(104] color used for automap grid lines"},
    {"mapcolor_wall", &mapcolor_wall, 23,   // red-brown
     0,255,0,ss_auto,   "[0-255(23)] color used for one side walls on automap"},
    {"mapcolor_fchg", &mapcolor_fchg, 55,   // lt brown
     0,255,0,ss_auto,   "[0-255(55)] color used for lines floor height changes across"},
    {"mapcolor_cchg", &mapcolor_cchg, 215,  // orange
     0,255,0,ss_auto,   "[0-255(215)] color used for lines ceiling height changes across"},
    {"mapcolor_clsd", &mapcolor_clsd, 208,  // white
     0,255,0,ss_auto,   "[0-255(208)] color used for lines denoting closed doors, objects"},
    {"mapcolor_rkey", &mapcolor_rkey, 175,  // red
     0,255,0,ss_auto,   "[0-255(175)] color used for red key sprites"},
    {"mapcolor_bkey", &mapcolor_bkey, 204,  // blue
     0,255,0,ss_auto,   "[0-255(204)] color used for blue key sprites"},
    {"mapcolor_ykey", &mapcolor_ykey, 231,  // yellow
     0,255,0,ss_auto,   "[0-255(231)] color used for yellow key sprites"},
    {"mapcolor_rdor", &mapcolor_rdor, 175,  // red
     0,255,0,ss_auto,   "[0-255(175)] color used for closed red doors"},
    {"mapcolor_bdor", &mapcolor_bdor, 204,  // blue
     0,255,0,ss_auto,   "[0-255(204)] color used for closed blue doors"},
    {"mapcolor_ydor", &mapcolor_ydor, 231,  // yellow
     0,255,0,ss_auto,   "[0-255(231)] color used for closed yellow doors"},
    {"mapcolor_tele", &mapcolor_tele, 119,  // dk green
     0,255,0,ss_auto,   "[0-255(119)] color used for teleporter lines"},
    {"mapcolor_secr", &mapcolor_secr, 252,  // purple
     0,255,0,ss_auto,   "[0-255(252)] color used for lines around secret sectors"},
    {"mapcolor_exit", &mapcolor_exit, 0,    // none
     0,255,0,ss_auto,   "[0-255(252)] color used for exit lines"},
    {"mapcolor_unsn", &mapcolor_unsn, 104,  // dk gray
     0,255,0,ss_auto,   "[0-255(104)] color used for lines not seen without computer map"},
    {"mapcolor_flat", &mapcolor_flat, 88,   // lt gray
     0,255,0,ss_auto,   "[0-255(88)] color used for lines with no height changes"},
    {"mapcolor_sprt", &mapcolor_sprt, 112,  // green
     0,255,0,ss_auto,   "[0-255(112)] color used as things"},
    {"mapcolor_hair", &mapcolor_hair, 208,  // white
     0,255,0,ss_auto,   "[0-255(208)] color used for dot crosshair denoting center of map"},
    {"mapcolor_sngl", &mapcolor_sngl, 208,  // white
     0,255,0,ss_auto,   "[0-255(208)] color used for the single player arrow"},
    {"mapcolor_ply1", mapcolor_plyr+0, 112, // green
     0,255,0,ss_auto,   "[0-255(112)] color used for the green player arrow"},
    {"mapcolor_ply2", mapcolor_plyr+1, 88,  // lt gray
     0,255,0,ss_auto,   "[0-255(88)] color used for the gray player arrow"},
    {"mapcolor_ply3", mapcolor_plyr+2, 64,  // brown
     0,255,0,ss_auto,   "[0-255(64)] color used for the brown player arrow"},
    {"mapcolor_ply4", mapcolor_plyr+3, 176, // red
     0,255,0,ss_auto,   "[0-255(176)] color used for the red player arrow"},
    //jff 3/9/98 add option to not show secrets til after found
    {"map_secret_after", &map_secret_after, 0, // show secret after gotten
     0,1,0,ss_auto,   "[0/1(0)] 1 to not show secret sectors till after entered"},
    //jff 1/7/98 end additions for automap

    //jff 2/16/98 defaults for color ranges in hud and status
    {"hudcolor_titl", &hudcolor_titl, 5,  // gold range
     0,9,0,ss_auto,     "[0-9(5)] color range used for automap level title"},
    {"hudcolor_xyco", &hudcolor_xyco, 3,  // green range
     0,9,0,ss_auto,     "[0-9(3)] color range used for automap coordinates"},
    {"hudcolor_mesg", &hudcolor_mesg, 6,  // red range
     0,9,0,ss_mess,     "[0-9(6)] color range used for messages during play"},
    {"hudcolor_chat", &hudcolor_chat, 5,  // gold range
     0,9,0,ss_mess,     "[0-9(5)] color range used for chat messages and entry"},
    {"hudcolor_list", &hudcolor_list, 5,  // gold range  //jff 2/26/98
     0,9,0,ss_mess,     "[0-9(5)] color range used for message review"},
    {"hud_msg_lines", &hud_msg_lines, 1,  // 1 line scrolling window
     1,16,0,ss_mess,    "[1-16(1)] number of messages in review display (1=disable)"},
    {"hud_list_bgon", &hud_list_bgon, 0,  // solid window bg ena //jff 2/26/98
     0,1,0,ss_mess,     "[0/1(1)] 1 enables background window behind message review"},
    {"hud_distributed", &hud_distributed, 0, // hud broken up into 3 displays //jff 3/4/98
     0,1,0,ss_none,     "[0/1(0)] 1 splits HUD into three 2 line displays"},
    {"health_red",    &health_red   , 25, // below is red
     0,200,0,ss_stat,   "[0-200(25)] amount of health for red to yellow transition"},
    {"health_yellow", &health_yellow, 50, // below is yellow
     0,200,0,ss_stat,   "[0-200(50)] amount of health for yellow to green transition"},
    {"health_green",  &health_green , 100,// below is green, above blue
     0,200,0,ss_stat,   "[0-200(100)] amount of health for green to blue transition"},
    {"armor_red",     &armor_red    , 25, // below is red
     0,200,0,ss_stat,   "[0-200(25)] amount of armor for red to yellow transition"},
    {"armor_yellow",  &armor_yellow , 50, // below is yellow
     0,200,0,ss_stat,   "[0-200(50)] amount of armor for yellow to green transition"},
    {"armor_green",   &armor_green  , 100,// below is green, above blue
     0,200,0,ss_stat,   "[0-200(100)] amount of armor for green to blue transition"},
    {"ammo_red",      &ammo_red     , 25, // below 25% is red
     0,100,0,ss_stat,   "[0-100(25)] percent of ammo for red to yellow transition"},
    {"ammo_yellow",   &ammo_yellow  , 50, // below 50% is yellow, above green
     0,100,0,ss_stat,   "[0-100(50)] percent of ammo for yellow to green transition"},

    //jff 2/16/98 HUD and status feature controls
    {"hud_active",    &hud_active, 2, // 0=off, 1=small, 2=full
     0,2,0,ss_none,    "[0-2(2)] 0 for HUD off, 1 for HUD small, 2 for full HUD"},
    //jff 2/23/98
    {"hud_displayed", &hud_displayed,  0, // whether hud is displayed
     0,1,0,ss_none,    "[0/1(0)] 1 to enable display of HUD"},
    {"hud_nosecrets", &hud_nosecrets,  0, // no secrets/items/kills HUD line
     0,1,0,ss_stat,    "[0/1(0)] 1 to disable display of kills/items/secrets on HUD"},

    // killough 2/8/98: weapon preferences set by user:
    {"weapon_choice_1", &weapon_preferences[0][0], 6,
     1,9,0,ss_weap,     "[1-9(6)] first choice for weapon (best)"},
    {"weapon_choice_2", &weapon_preferences[0][1], 9,
     1,9,0,ss_weap,     "[1-9(9)] second choice for weapon "},
    {"weapon_choice_3", &weapon_preferences[0][2], 4,
     1,9,0,ss_weap,     "[1-9(4)] third choice for weapon "},
    {"weapon_choice_4", &weapon_preferences[0][3], 3,
     1,9,0,ss_weap,     "[1-9(3)] fourth choice for weapon "},
    {"weapon_choice_5", &weapon_preferences[0][4], 2,
     1,9,0,ss_weap,     "[1-9(2)] fifth choice for weapon "},
    {"weapon_choice_6", &weapon_preferences[0][5], 8,
     1,9,0,ss_weap,     "[1-9(8)] sixth choice for weapon "},
    {"weapon_choice_7", &weapon_preferences[0][6], 5,
     1,9,0,ss_weap,     "[1-9(5)] seventh choice for weapon "},
    {"weapon_choice_8", &weapon_preferences[0][7], 7,
     1,9,0,ss_weap,     "[1-9(7)] eighth choice for weapon "},
    {"weapon_choice_9", &weapon_preferences[0][8], 1,
     1,9,0,ss_weap,     "[1-9(1)] ninth choice for weapon (worst)"},
};

int numdefaults;
char* defaultfile;


//
// M_SaveDefaults
//

void M_SaveDefaults (void)
  {
  int   i;
  int   v;
  FILE* f;
  
  f = fopen (defaultfile, "w");
  if (!f)
    return; // can't write the file, but don't complain

  // 3/3/98 explain format of file

  if (config_help)
    {
    fprintf(f,"\n;BOOM.CFG format:\n");    
    fprintf(f,";[min-max(default)] description of variable\n");    
    fprintf(f,";variable   value\n");
    }

  for (i = 0 ; i < numdefaults ; i++)
    {
    if (config_help)
      fprintf(f,"\n%s\n",defaults[i].help); //jff 3/3/98 output help string

    if (!defaults[i].isstr) //jff 4/10/98 kill super-hack on pointer value
      {
      v = *defaults[i].location;

      if (!strncmp(defaults[i].name,"key_",4))    // killough
        v = I_DoomCode2ScanCode(v);

      // killough 3/6/98: use spaces instead of tabs for uniform justification
      fprintf (f,"%-25s %5i\n",defaults[i].name,v);
      }
    else
      {
      fprintf (f,"%-25s \"%s\"\n",defaults[i].name,
         * (char **) (defaults[i].location));
      }
    }
  
  fclose (f);
  }


//
// M_LoadDefaults
//

#define NUMCHATSTRINGS 10 // phares 4/13/98

extern byte scantokey[128];

void M_LoadDefaults (void)
  {
  int   i;
  int   len;
  FILE* f;
  char  def[80];
  char  strparm[100];
  char* newstring = NULL;   // killough
  int   parm;
  boolean isstring;
    
  // set everything to base values

  numdefaults = sizeof(defaults)/sizeof(defaults[0]);
  for (i = 0 ; i < numdefaults ; i++)
    {
    *defaults[i].location = defaults[i].defaultvalue;

    // phares 4/13/98:
    // provide default strings with their own malloced memory so that when
    // we leave this routine, that's what we're dealing with whether there
    // was a config file or not, and whether there were chat definitions
    // in it or not. This provides consistency later on when/if we need to
    // edit these strings (i.e. chat macros in the Chat Strings Setup screen).

    if (defaults[i].isstr)
      *defaults[i].location = (int)strdup((char *)*defaults[i].location);
    }
    
  // check for a custom default file

  i = M_CheckParm ("-config");
  if (i && i < myargc-1)
    {
    defaultfile = myargv[i+1];
    //jff 8/3/98 use logical output routine
    lprintf (LO_CONFIRM," default file: %s\n",defaultfile);
    }
  else
    defaultfile = basedefault;
    
  // read the file in, overriding any set defaults

  f = fopen (defaultfile, "r");
  if (f)
    {
    while (!feof(f))
      {
      isstring = false;
      if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
        {

        //jff 3/3/98 skip lines not starting with an alphanum

        if (!isalnum(def[0]))
          continue;

        if (strparm[0] == '"')
          {

          // get a string default

          isstring = true;
          len = strlen(strparm);
          newstring = (char *) malloc(len);
          strparm[len-1] = 0; // clears trailing double-quote mark
          strcpy(newstring, strparm+1); // clears leading double-quote mark
          }

/*  redundant because of the very definition of %i in scanf -- killough:
    else if (strparm[0] == '0' && strparm[1] == 'x')
        sscanf(strparm+2, "%x", &parm);
*/
        else
          {
          sscanf(strparm, "%i", &parm);
          if (!strncmp(def,"key_",4))    /* killough */
            parm = I_ScanCode2DoomCode(parm);
          }

        for (i = 0 ; i < numdefaults ; i++)
          if (!strcmp(def, defaults[i].name))
            {
            if (!isstring)
              {

              //jff 3/4/98 range check numeric parameters

              if ((defaults[i].minvalue==UL || defaults[i].minvalue<=parm) &&
                  (defaults[i].maxvalue==UL || defaults[i].maxvalue>=parm))
                *defaults[i].location = parm;
              }
            else
              {
              free((char *)*defaults[i].location);    // phares 4/13/98
              *defaults[i].location = (int) newstring;
              }
            break;
            }
        }
      }
    
    fclose (f);
    }

    //jff 3/4/98 redundant range checks for hud deleted here
  }


//
// SCREEN SHOTS
//

typedef struct
  {
  char     manufacturer;
  char     version;
  char     encoding;
  char     bits_per_pixel;

  unsigned short  xmin;
  unsigned short  ymin;
  unsigned short  xmax;
  unsigned short  ymax;
    
  unsigned short  hres;
  unsigned short  vres;

  unsigned char palette[48];
    
  char     reserved;
  char     color_planes;
  unsigned short  bytes_per_line;
  unsigned short  palette_type;
    
  char     filler[58];
  unsigned char data;   // unbounded
} pcx_t;


//
// WritePCXfile
//
void WritePCXfile(char* filename,byte* data,int width,int height,byte* palette)
  {
  int    i;
  int    length;
  pcx_t* pcx;
  byte*  pack;
  
  pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

  pcx->manufacturer = 0x0a; // PCX id
  pcx->version = 5;         // 256 color
  pcx->encoding = 1;        // uncompressed
  pcx->bits_per_pixel = 8;  // 256 color
  pcx->xmin = 0;
  pcx->ymin = 0;
  pcx->xmax = SHORT(width-1);
  pcx->ymax = SHORT(height-1);
  pcx->hres = SHORT(width);
  pcx->vres = SHORT(height);
  memset (pcx->palette,0,sizeof(pcx->palette));
  pcx->color_planes = 1;        // chunky image
  pcx->bytes_per_line = SHORT(width);
  pcx->palette_type = SHORT(2); // not a grey scale
  memset (pcx->filler,0,sizeof(pcx->filler));

  // pack the image

  pack = &pcx->data;
  
  for (i = 0 ; i < width*height ; i++)
    if ( (*data & 0xc0) != 0xc0)
      *pack++ = *data++;
    else
      {
      *pack++ = 0xc1;
      *pack++ = *data++;
      }
    
  // write the palette

  *pack++ = 0x0c; // palette ID byte
  for (i = 0 ; i < 768 ; i++)
    *pack++ = gammatable[usegamma][*palette++];   // killough
    
  // write output file

  length = pack - (byte *)pcx;
  M_WriteFile (filename, pcx, length);

  Z_Free (pcx);
  }


// jff 3/30/98 types and data structures for BMP output of screenshots
//
// killough 5/2/98:
// Changed type names to avoid conflicts with endianess functions

#define BI_RGB 0L

typedef unsigned short uint_t;
typedef unsigned long dword_t;
typedef long     long_t;
typedef unsigned char ubyte_t;

typedef struct tagBITMAPFILEHEADER
  {
  uint_t  bfType;
  dword_t bfSize;
  uint_t  bfReserved1;
  uint_t  bfReserved2;
  dword_t bfOffBits;
  } __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
  {
  dword_t biSize;
  long_t  biWidth;
  long_t  biHeight;
  uint_t  biPlanes;
  uint_t  biBitCount;
  dword_t biCompression;
  dword_t biSizeImage;
  long_t  biXPelsPerMeter;
  long_t  biYPelsPerMeter;
  dword_t biClrUsed;
  dword_t biClrImportant;
  } __attribute__ ((packed)) BITMAPINFOHEADER;

// jff 3/30/98 binary file write with error detection
void SafeWrite(void *data,size_t size,size_t number,FILE *st)
  {
  if (fwrite(data,size,number,st)<number)
    I_Error("Out of disk space writing screenshot");
  }

//
// WriteBMPfile
// jff 3/30/98 Add capability to write a .BMP file (256 color uncompressed)
//

void WriteBMPfile(char* filename,byte* data,int width,int height,byte* palette)
  {
  int i,wid;
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  int fhsiz,ihsiz;
  FILE *st;
  char zero=0;
  ubyte_t c;

  fhsiz = sizeof(BITMAPFILEHEADER);
  ihsiz = sizeof(BITMAPINFOHEADER);
  wid = 4*((width+3)/4);
  //jff 4/22/98 add endian macros
  bmfh.bfType = SHORT(19778);
  bmfh.bfSize = LONG(fhsiz+ihsiz+256L*4+width*height);
  bmfh.bfReserved1 = SHORT(0);
  bmfh.bfReserved2 = SHORT(0);
  bmfh.bfOffBits = LONG(fhsiz+ihsiz+256L*4);

  bmih.biSize = LONG(ihsiz);
  bmih.biWidth = LONG(width);
  bmih.biHeight = LONG(height);
  bmih.biPlanes = SHORT(1);
  bmih.biBitCount = SHORT(8);
  bmih.biCompression = LONG(BI_RGB);
  bmih.biSizeImage = LONG(wid*height);
  bmih.biXPelsPerMeter = LONG(0);
  bmih.biYPelsPerMeter = LONG(0);
  bmih.biClrUsed = LONG(256);
  bmih.biClrImportant = LONG(256);

  st = fopen(filename,"wb");
  if (st!=NULL)
    {
    // write the header
    SafeWrite(&bmfh.bfType,sizeof(bmfh.bfType),1,st);
    SafeWrite(&bmfh.bfSize,sizeof(bmfh.bfSize),1,st);
    SafeWrite(&bmfh.bfReserved1,sizeof(bmfh.bfReserved1),1,st);
    SafeWrite(&bmfh.bfReserved2,sizeof(bmfh.bfReserved2),1,st);
    SafeWrite(&bmfh.bfOffBits,sizeof(bmfh.bfOffBits),1,st);

    SafeWrite(&bmih.biSize,sizeof(bmih.biSize),1,st);
    SafeWrite(&bmih.biWidth,sizeof(bmih.biWidth),1,st);
    SafeWrite(&bmih.biHeight,sizeof(bmih.biHeight),1,st);
    SafeWrite(&bmih.biPlanes,sizeof(bmih.biPlanes),1,st);
    SafeWrite(&bmih.biBitCount,sizeof(bmih.biBitCount),1,st);
    SafeWrite(&bmih.biCompression,sizeof(bmih.biCompression),1,st);
    SafeWrite(&bmih.biSizeImage,sizeof(bmih.biSizeImage),1,st);
    SafeWrite(&bmih.biXPelsPerMeter,sizeof(bmih.biXPelsPerMeter),1,st);
    SafeWrite(&bmih.biYPelsPerMeter,sizeof(bmih.biYPelsPerMeter),1,st);
    SafeWrite(&bmih.biClrUsed,sizeof(bmih.biClrUsed),1,st);
    SafeWrite(&bmih.biClrImportant,sizeof(bmih.biClrImportant),1,st);

    // write the palette, in blue-green-red order, gamma corrected
    for (i=0;i<768;i+=3)
      {
      c=gammatable[usegamma][palette[i+2]];
      SafeWrite(&c,sizeof(char),1,st);
      c=gammatable[usegamma][palette[i+1]];
      SafeWrite(&c,sizeof(char),1,st);
      c=gammatable[usegamma][palette[i+0]];
      SafeWrite(&c,sizeof(char),1,st);
      SafeWrite(&zero,sizeof(char),1,st);
      }

    for (i = 0 ; i < height ; i++)
      SafeWrite(data+(height-1-i)*width,sizeof(byte),wid,st);

    fclose(st);
    }
  }


//
// M_ScreenShot
//
// Modified by Lee Killough so that any number of shots can be taken,
// the code is faster, and no annoying "screenshot" message appears.

void M_ScreenShot (void)
  {
  static int shot;
  char lbmname[32];

  // munge planar buffer to linear

  byte *linear = screens[2], *pal;

  I_ReadScreen (linear);
    
  if (access(".",2))
    {
    dprintf ("M_ScreenShot: Couldn't create a PCX"); // killough 4/18/98
    return;
    }

  do                                         //jff 3/30/98 pcx or bmp?  
    sprintf(lbmname,"DOOM%02d.%.3s", shot++,screenshot_pcx? "PCX":"BMP");
  while (!access(lbmname,0));

  // killough 4/18/98: make palette stay around (PU_CACHE could cause crash)

  pal = W_CacheLumpName ("PLAYPAL", PU_STATIC);
    
  // save the pcx file
  //jff 3/30/98 write pcx or bmp depending on mode

  (screenshot_pcx ? WritePCXfile : WriteBMPfile)
    (lbmname, linear, SCREENWIDTH, SCREENHEIGHT, pal);

  // killough 4/18/98: now you can mark it PU_CACHE

  Z_ChangeTag(pal, PU_CACHE);

  // 1/18/98 killough: replace "SCREEN SHOT" acknowledgement with sfx
  // players[consoleplayer].message = "screen shot";

  S_StartSound(NULL,gamemode==commercial ? sfx_radio : sfx_tink); 
  }

//----------------------------------------------------------------------------
//
// $Log: m_misc.c,v $
// Revision 1.62  1998/09/07  20:19:07  jim
// Added logical output routine
//
// Revision 1.60  1998/06/03  20:32:12  jim
// Fixed mispelling of key_chat string
//
// Revision 1.59  1998/05/21  12:12:28  jim
// Removed conditional from net code
//
// Revision 1.58  1998/05/16  09:41:15  jim
// formatted net files, installed temp switch for testing Stan/Lee's version
//
// Revision 1.57  1998/05/12  12:47:04  phares
// Removed OVER_UNDER code
//
// Revision 1.56  1998/05/05  19:56:01  phares
// Formatting and Doc changes
//
// Revision 1.55  1998/05/05  16:29:12  phares
// Removed RECOIL and OPT_BOBBING defines
//
// Revision 1.54  1998/05/03  23:05:19  killough
// Fix #includes, remove external decls duplicated elsewhere, fix LONG() conflict
//
// Revision 1.53  1998/04/23  13:07:27  jim
// Add exit line to automap
//
// Revision 1.51  1998/04/22  13:46:12  phares
// Added Setup screen Reset to Defaults
//
// Revision 1.50  1998/04/19  01:13:50  killough
// Fix freeing memory before use in savegame code
//
// Revision 1.49  1998/04/17  10:35:50  killough
// Add traditional_menu option for main menu
//
// Revision 1.48  1998/04/14  08:18:11  killough
// replace obsolete adaptive_gametic with realtic_clock_rate
//
// Revision 1.47  1998/04/13  21:36:33  phares
// Cemented ESC and F1 in place
//
// Revision 1.46  1998/04/13  12:30:02  phares
// Resolved Z_Free error msg when no boom.cfg file
//
// Revision 1.45  1998/04/12  22:55:33  phares
// Remaining 3 Setup screens
//
// Revision 1.44  1998/04/10  23:21:41  jim
// fixed string/int differentiation by value
//
// Revision 1.43  1998/04/10  06:37:54  killough
// Add adaptive gametic timer option
//
// Revision 1.42  1998/04/06  11:05:00  jim
// Remove LEESFIXES, AMAP bdg->247
//
// Revision 1.41  1998/04/06  04:50:00  killough
// Support demo_insurance=2
//
// Revision 1.40  1998/04/05  00:51:13  phares
// Joystick support, Main Menu re-ordering
//
// Revision 1.39  1998/04/03  14:45:49  jim
// Fixed automap disables at 0, mouse sens unbounded
//
// Revision 1.38  1998/03/31  10:44:31  killough
// Add demo insurance option
//
// Revision 1.37  1998/03/31  00:39:44  jim
// Screenshots in BMP format added
//
// Revision 1.36  1998/03/25  16:31:23  jim
// Fixed bad default value for defaultskill
//
// Revision 1.34  1998/03/23  15:24:17  phares
// Changed pushers to linedef control
//
// Revision 1.33  1998/03/20  00:29:47  phares
// Changed friction to linedef control
//
// Revision 1.32  1998/03/11  17:48:16  phares
// New cheats, clean help code, friction fix
//
// Revision 1.31  1998/03/10  07:06:30  jim
// Added secrets on automap after found only option
//
// Revision 1.30  1998/03/09  18:29:12  phares
// Created separately bound automap and menu keys
//
// Revision 1.29  1998/03/09  11:00:20  jim
// allowed -1 in mouse bindings and map functions
//
// Revision 1.28  1998/03/09  07:35:18  killough
// Rearrange order of cfg options, add capslock options
//
// Revision 1.27  1998/03/06  21:41:04  jim
// fixed erroneous range for gamma in config
//
// Revision 1.26  1998/03/05  00:57:47  jim
// Scattered HUD
//
// Revision 1.25  1998/03/04  11:55:42  jim
// Add range checking, help strings to BOOM.CFG
//
// Revision 1.24  1998/03/02  15:34:15  jim
// Added Rand's HELP screen as lump and loaded and displayed it
//
// Revision 1.23  1998/03/02  11:36:44  killough
// clone defaults, add sts_traditional_keys
//
// Revision 1.22  1998/02/27  19:22:05  jim
// Range checked hud/sound card variables
//
// Revision 1.21  1998/02/27  08:10:02  phares
// Added optional player bobbing
//
// Revision 1.20  1998/02/26  22:58:39  jim
// Added message review display to HUD
//
// Revision 1.19  1998/02/24  22:00:57  killough
// turn translucency back on by default
//
// Revision 1.18  1998/02/24  08:46:05  phares
// Pushers, recoil, new friction, and over/under work
//
// Revision 1.17  1998/02/23  14:21:14  jim
// Merged HUD stuff, fixed p_plats.c to support elevators again
//
// Revision 1.16  1998/02/23  04:40:48  killough
// Lots of new options
//
// Revision 1.14  1998/02/20  21:57:00  phares
// Preliminarey sprite translucency
//
// Revision 1.13  1998/02/20  18:46:58  jim
// cleanup of HUD control
//
// Revision 1.12  1998/02/19  16:54:33  jim
// Optimized HUD and made more configurable
//
// Revision 1.11  1998/02/18  11:56:11  jim
// Fixed issues with HUD and reduced screen size
//
// Revision 1.9  1998/02/15  03:21:20  phares
// Jim's comment: Fixed bug in automap from mistaking framebuffer index for mark color
//
// Revision 1.8  1998/02/15  03:17:56  phares
// User-defined keys
//
// Revision 1.6  1998/02/09  03:04:12  killough
// Add weapon preferences, player corpse, vsync options
//
// Revision 1.5  1998/02/02  13:37:26  killough
// Clone compatibility flag, for TNTCOMP to work
//
// Revision 1.4  1998/01/26  19:23:49  phares
// First rev with no ^Ms
//
// Revision 1.3  1998/01/26  04:59:07  killough
// Fix DOOM 1 screenshot acknowledgement
//
// Revision 1.2  1998/01/21  16:56:16  jim
// Music fixed, defaults for cards added
//
// Revision 1.1.1.1  1998/01/19  14:02:57  rand
// Lee's Jan 19 sources
//
//
//----------------------------------------------------------------------------

