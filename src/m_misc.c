/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: m_misc.c,v 1.43 2000/03/17 20:50:30 cph Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Main loop menu stuff.
 *  Default Config File.
 *  PCX Screenshots.
 *
 *-----------------------------------------------------------------------------*/

static const char
rcsid[] = "$Id: m_misc.c,v 1.43 2000/03/17 20:50:30 cph Exp $";

#include "doomstat.h"
#include "m_argv.h"
#include "g_game.h"
#include "m_menu.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "dstrings.h"
#include "m_misc.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_joy.h"
#include "lprintf.h"

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

  while (*string) {
    c = toupper(*string) - HU_FONTSTART;
    string++;
    if (c < 0 || c> HU_FONTSIZE) {
      x += 4;
      continue;
    }
    
    w = SHORT (hu_font[c]->width);
    if (x+w > SCREENWIDTH)
      break;

    // proff/nicolas 09/20/98 -- changed for hi-res
    // CPhipps - patch drawing updated, reformatted
    V_DrawMemPatch(x, y, 0, hu_font[c], NULL, VPT_STRETCH);
    x+=w;
  }

  return x;
}

//
// M_WriteFile
//

boolean M_WriteFile(char const* name,void* source,int length)
{
  int handle;
  int count;
  
  handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

  if (handle == -1)
    return false;

  count = write (handle, source, length);
  close (handle);
  
  if (count < length) {
    unlink(name); // CPhipps - no corrupt data files around, they only confuse people.
    return false;
  }
    
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
  if (fstat (handle,&fileinfo) == -1) {
    close(handle);
    I_Error ("Couldn't read file %s", name);
  }

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

int usemouse;

//jff 3/30/98
int screenshot_pcx;      // option to output screenshot as pcx or bmp

extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;

extern int viewwidth;
extern int viewheight;

extern int mouseSensitivity_horiz,mouseSensitivity_vert;  // killough

extern int realtic_clock_rate;         // killough 4/13/98: adjustable timer
extern int leds_always_off;            // killough 3/6/98
extern int tran_filter_pct;            // killough 2/21/98

extern int screenblocks;
extern int showMessages;

#ifndef DJGPP
const char* snd_device;
int         mus_pause_opt; // 0 = kill music, 1 = pause, 2 = continue
#endif

extern const char* chat_macros[];

// CPhipps - new stuff. Autoloaded wads, and endoom and misc X options
extern const char* auto_load_wads;
extern int endoom_mode;
int X_opt;

#define UL (-123456789) /* magic number for no min or max for parameter */

//jff 3/3/98 added min, max, and help string to all entries
//jff 4/10/98 added isstr field to specify whether value is string or int
// CPhipps - const
const default_t defaults[] =
{
  {"Misc settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"compatibility_level",{&default_compatibility_level},
   {MAX_COMPATIBILITY_LEVEL-1},0,MAX_COMPATIBILITY_LEVEL-1,
   def_int,ss_none}, // compatibility level" - CPhipps  
  {"realtic_clock_rate",{&realtic_clock_rate},{100},0,UL,
   def_int,ss_none}, // percentage of normal speed (35 fps) realtic clock runs at
  {"max_player_corpse", {&bodyquesize}, {32},-1,UL,   // killough 2/8/98
   def_int,ss_none}, // number of dead bodies in view supported (-1 = no limit)
  {"demo_insurance",{&default_demo_insurance},{2},0,2,  // killough 3/31/98
   def_int,ss_none}, // 1=take special steps ensuring demo sync, 2=only during recordings
  {"leds_always_off",{&leds_always_off},{0},0,1,
   def_bool,ss_none}, // keep keyboard LEDs turned off (DOS) // killough 3/6/98
  {"endoom_mode", {&endoom_mode},{5},0,7, // CPhipps - endoom flags
   def_hex, ss_none}, // 0, +1 for colours, +2 for non-ascii chars, +4 for skip-last-line
  
  {"Files",{NULL},{0},UL,UL,def_none,ss_none},
  // jff 3/30/98 add ability to take screenshots in BMP format
  {"screenshot_pcx",{&screenshot_pcx},{0},0,1,
   def_int,ss_none}, //1 to take a screenshot in PCX format, 0 for BMP"}, 
  {"auto_load",{NULL,&auto_load_wads},{0,"boomlump.wad"},UL,UL, 
   def_str,ss_none}, // files to load automatically, separated by ;'s
  
  {"Game settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"default_skill",{&defaultskill},{3},1,5, // jff 3/24/98 allow default skill setting
   def_int,ss_none}, // selects default skill 1=TYTD 2=NTR 3=HMP 4=UV 5=NM
  {"weapon_recoil",{&default_weapon_recoil},{1},0,1,
   def_bool,ss_weap}, // enables recoil from weapon fire   // phares
  {"player_bobbing",{&default_player_bobbing},{1},0,1,         // phares 2/25/98
   def_bool,ss_weap}, // enables player bobbing (view randomly moving up/down slightly)
  {"monsters_remember",{&default_monsters_remember},{1},0,1,   // killough 3/1/98
   def_bool,ss_enem}, // enables monsters remembering enemies after killing others
  {"sts_always_red",{&sts_always_red},{0},0,1, // no color changes on status bar
   def_bool,ss_stat},
  {"sts_pct_always_gray",{&sts_pct_always_gray},{1},0,1, // 2/23/98 chg default
   def_bool,ss_stat}, // makes percent signs on status bar always gray
  {"sts_traditional_keys",{&sts_traditional_keys},{0},0,1,  // killough 2/28/98
   def_bool,ss_stat}, // disables doubled card and skull key display on status bar
  {"traditional_menu",{&traditional_menu},{0},0,1, 
   def_bool,ss_none}, // force use of Doom's main menu ordering // killough 4/17/98
  {"show_messages",{&showMessages},{1},0,1,
   def_bool,ss_none}, // enables message display
  {"autorun",{&autorun},{0},0,1,  // killough 3/6/98: preserve autorun across games
   def_bool,ss_none},

  {"Sound settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"sound_card",{&snd_card},{-1},-1,7,       // jff 1/18/98 allow Allegro drivers
   def_int,ss_none}, // select sounds driver (DOS), -1 is autodetect, 0 is none; in Linux, non-zero enables sound 
  {"music_card",{&mus_card},{-1},-1,9,       //  to be set,  -1 = autodetect
   def_int,ss_none}, // select music driver (DOS), -1 is autodetect, 0 is none"; in Linux, non-zero enables music
  {"pitched_sounds",{&default_pitched_sounds},{0},0,1, // killough 2/21/98
   def_bool,ss_none}, // enables variable pitch in sound effects (from id's original code)
  {"sfx_volume",{&snd_SfxVolume},{8},0,15, def_int,ss_none}, 
  {"music_volume",{&snd_MusicVolume},{8},0,15, def_int,ss_none},
  {"mus_pause_opt",{&mus_pause_opt},{0},0,2, // CPhipps - music pausing
   def_int, ss_none}, // 0 = kill music when paused, 1 = pause music, 2 = let music continue
  {"sounddev", {NULL,&snd_device}, {0,"/dev/dsp"},UL,UL,
   def_str,ss_none}, // sound output device (UNIX)
  {"snd_channels",{&numChannels},{32},1,UL,
   def_int,ss_none}, // number of audio events simultaneously // killough
  {"detect_voices",{&detect_voices},{1},0,1,// jff 3/4/98 detect # voices
   def_int,ss_none}, // enables detection of no. of sound voices (DOS) 

  {"Video settings",{NULL},{0},UL,UL,def_none,ss_none},
  // CPhipps - default screensize for targets that support high-res
  {"screen_width",{&desired_screenwidth},{320}, 320, 1600, 
   def_int,ss_none},
  {"screen_height",{&desired_screenheight},{200},200,1200,
   def_int,ss_none},  
  {"use_vsync",{&use_vsync},{1},0,1,             // killough 2/8/98
   def_bool,ss_none}, // enable wait for vsync to avoid display tearing (fullscreen)
  {"translucency",{&default_translucency},{1},0,1,   // phares
   def_bool,ss_none}, // enables translucency
  {"tran_filter_pct",{&tran_filter_pct},{66},0,100,         // killough 2/21/98
   def_int,ss_none}, // set percentage of foreground/background translucency mix  
  {"screenblocks",{&screenblocks},{10},3,11,  // killough 2/21/98: default to 10
   def_int,ss_none},
  {"usegamma",{&usegamma},{3},0,4, //jff 3/6/98 fix erroneous upper limit in range
   def_int,ss_none}, // gamma correction level // killough 1/18/98
  {"X_options",{&X_opt},{0},0,3, // CPhipps - misc X options
   def_hex,ss_none}, // X options, see l_video_x.c  

  {"Mouse settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"use_mouse",{&usemouse},{1},0,1,
   def_bool,ss_none}, // enables use of mouse with DOOM
  //jff 4/3/98 allow unlimited sensitivity
  {"mouse_sensitivity_horiz",{&mouseSensitivity_horiz},{10},0,UL,
   def_int,ss_none}, /* adjust horizontal (x) mouse sensitivity killough/mead */
  //jff 4/3/98 allow unlimited sensitivity
  {"mouse_sensitivity_vert",{&mouseSensitivity_vert},{10},0,UL,
   def_int,ss_none}, /* adjust vertical (y) mouse sensitivity killough/mead */
  //jff 3/8/98 allow -1 in mouse bindings to disable mouse function
  {"mouseb_fire",{&mousebfire},{0},-1,MAX_MOUSEB,
   def_int,ss_keys}, // mouse button number to use for fire
  {"mouseb_strafe",{&mousebstrafe},{1},-1,MAX_MOUSEB,
   def_int,ss_keys}, // mouse button number to use for strafing
  {"mouseb_forward",{&mousebforward},{2},-1,MAX_MOUSEB,
   def_int,ss_keys}, // mouse button number to use for forward motion
  //jff 3/8/98 end of lower range change for -1 allowed in mouse binding
  
// For key bindings, the values stored in the key_* variables       // phares
// are the internal Doom Codes. The values stored in the default.cfg
// file are the keyboard codes.
// CPhipps - now they're the doom codes, so default.cfg can be portable

  {"Key bindings",{NULL},{0},UL,UL,def_none,ss_none},
  {"key_right",       {&key_right},          {KEYD_RIGHTARROW},
   0,MAX_KEY,def_key,ss_keys}, // key to turn right
  {"key_left",        {&key_left},           {KEYD_LEFTARROW} ,
   0,MAX_KEY,def_key,ss_keys}, // key to turn left
  {"key_up",          {&key_up},             {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys}, // key to move forward
  {"key_down",        {&key_down},           {KEYD_DOWNARROW},
   0,MAX_KEY,def_key,ss_keys}, // key to move backward
  {"key_menu_right",  {&key_menu_right},     {KEYD_RIGHTARROW},// phares 3/7/98
   0,MAX_KEY,def_key,ss_keys}, // key to move right in a menu  //     |
  {"key_menu_left",   {&key_menu_left},      {KEYD_LEFTARROW} ,//     V
   0,MAX_KEY,def_key,ss_keys}, // key to move left in a menu
  {"key_menu_up",     {&key_menu_up},        {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys}, // key to move up in a menu
  {"key_menu_down",   {&key_menu_down},      {KEYD_DOWNARROW} ,
   0,MAX_KEY,def_key,ss_keys}, // key to move down in a menu
  {"key_menu_backspace",{&key_menu_backspace},{KEYD_BACKSPACE} ,
   0,MAX_KEY,def_key,ss_keys}, // delete key in a menu
  {"key_menu_escape", {&key_menu_escape},    {KEYD_ESCAPE}    ,
   0,MAX_KEY,def_key,ss_keys}, // key to leave a menu      ,   // phares 3/7/98
  {"key_menu_enter",  {&key_menu_enter},     {KEYD_ENTER}     ,
   0,MAX_KEY,def_key,ss_keys}, // key to select from menu
  {"key_strafeleft",  {&key_strafeleft},     {','}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to strafe left
  {"key_straferight", {&key_straferight},    {'.'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to strafe right
  
  {"key_fire",        {&key_fire},           {KEYD_RCTRL}     ,
   0,MAX_KEY,def_key,ss_keys}, // duh
  {"key_use",         {&key_use},            {' '}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to open a door, use a switch
  {"key_strafe",      {&key_strafe},         {KEYD_RALT}      ,
   0,MAX_KEY,def_key,ss_keys}, // key to use with arrows to strafe
  {"key_speed",       {&key_speed},          {KEYD_RSHIFT}    ,
   0,MAX_KEY,def_key,ss_keys}, // key to run
  
  {"key_savegame",    {&key_savegame},       {KEYD_F2}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to save current game
  {"key_loadgame",    {&key_loadgame},       {KEYD_F3}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to restore from saved games
  {"key_soundvolume", {&key_soundvolume},    {KEYD_F4}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to bring up sound controls
  {"key_hud",         {&key_hud},            {KEYD_F5}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to adjust HUD
  {"key_quicksave",   {&key_quicksave},      {KEYD_F6}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to to quicksave
  {"key_endgame",     {&key_endgame},        {KEYD_F7}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to end the game
  {"key_messages",    {&key_messages},       {KEYD_F8}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle message enable
  {"key_quickload",   {&key_quickload},      {KEYD_F9}        ,
   0,MAX_KEY,def_key,ss_keys}, // key to load from quicksave
  {"key_quit",        {&key_quit},           {KEYD_F10}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to quit game
  {"key_gamma",       {&key_gamma},          {KEYD_F11}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to adjust gamma correction
  {"key_spy",         {&key_spy},            {KEYD_F12}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to view from another coop player's view
  {"key_pause",       {&key_pause},          {KEYD_PAUSE}     ,
   0,MAX_KEY,def_key,ss_keys}, // key to pause the game
  {"key_autorun",     {&key_autorun},        {KEYD_CAPSLOCK}  ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle always run mode
  {"key_chat",        {&key_chat},           {'t'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to enter a chat message
  {"key_backspace",   {&key_backspace},      {KEYD_BACKSPACE} ,
   0,MAX_KEY,def_key,ss_keys}, // backspace key
  {"key_enter",       {&key_enter},          {KEYD_ENTER}     ,
   0,MAX_KEY,def_key,ss_keys}, // key to select from menu or see last message
  {"key_map",         {&key_map},            {KEYD_TAB}       ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle automap display
  {"key_map_right",   {&key_map_right},      {KEYD_RIGHTARROW},// phares 3/7/98
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap right   //     |
  {"key_map_left",    {&key_map_left},       {KEYD_LEFTARROW} ,//     V
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap left
  {"key_map_up",      {&key_map_up},         {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap up
  {"key_map_down",    {&key_map_down},       {KEYD_DOWNARROW} ,
   0,MAX_KEY,def_key,ss_keys}, // key to shift automap down
  {"key_map_zoomin",  {&key_map_zoomin},      {'='}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to enlarge automap
  {"key_map_zoomout", {&key_map_zoomout},     {'-'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to reduce automap
  {"key_map_gobig",   {&key_map_gobig},       {'0'}           ,
   0,MAX_KEY,def_key,ss_keys},  // key to get max zoom for automap
  {"key_map_follow",  {&key_map_follow},      {'f'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle follow mode
  {"key_map_mark",    {&key_map_mark},        {'m'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to drop a marker on automap
  {"key_map_clear",   {&key_map_clear},       {'c'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to clear all markers on automap
  {"key_map_grid",    {&key_map_grid},        {'g'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle grid display over automap
  {"key_map_rotate",  {&key_map_rotate},      {'r'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle rotating the automap to match the player's orientation
  {"key_map_overlay", {&key_map_overlay},     {'o'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle overlaying the automap on the rendered display
  {"key_reverse",     {&key_reverse},         {'/'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to spin 180 instantly
  {"key_zoomin",      {&key_zoomin},          {'='}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to enlarge display
  {"key_zoomout",     {&key_zoomout},         {'-'}           ,
   0,MAX_KEY,def_key,ss_keys}, // key to reduce display
  {"key_chatplayer1", {&destination_keys[0]}, {'g'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 1
  // killough 11/98: fix 'i'/'b' reversal
  {"key_chatplayer2", {&destination_keys[1]}, {'i'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 2
  {"key_chatplayer3", {&destination_keys[2]}, {'b'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 3
  {"key_chatplayer4", {&destination_keys[3]}, {'r'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to chat with player 4
  {"key_weapontoggle",{&key_weapontoggle},    {'0'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to toggle between two most preferred weapons with ammo
  {"key_weapon1",     {&key_weapon1},         {'1'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 1 (fist/chainsaw)
  {"key_weapon2",     {&key_weapon2},         {'2'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 2 (pistol)
  {"key_weapon3",     {&key_weapon3},         {'3'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 3 (supershotgun/shotgun)
  {"key_weapon4",     {&key_weapon4},         {'4'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 4 (chaingun)
  {"key_weapon5",     {&key_weapon5},         {'5'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 5 (rocket launcher)
  {"key_weapon6",     {&key_weapon6},         {'6'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 6 (plasma rifle)
  {"key_weapon7",     {&key_weapon7},         {'7'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 7 (bfg9000)         //    ^
  {"key_weapon8",     {&key_weapon8},         {'8'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 8 (chainsaw)        //    |
  {"key_weapon9",     {&key_weapon9},         {'9'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to switch to weapon 9 (supershotgun)    // phares
  
  // killough 2/22/98: screenshot key
  {"key_screenshot",  {&key_screenshot},      {'*'}            ,
   0,MAX_KEY,def_key,ss_keys}, // key to take a screenshot
  
  {"Joystick settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"use_joystick",{&usejoystick},{0},0,2,
   def_int,ss_none}, // number of joystick to use (0 for none)
  {"joy_left",{&joyleft},{0},  UL,UL,def_int,ss_none},
  {"joy_right",{&joyright},{0},UL,UL,def_int,ss_none},
  {"joy_up",  {&joyup},  {0},  UL,UL,def_int,ss_none},
  {"joy_down",{&joydown},{0},  UL,UL,def_int,ss_none},
  {"joyb_fire",{&joybfire},{0},0,UL,
   def_int,ss_keys}, // joystick button number to use for fire
  {"joyb_strafe",{&joybstrafe},{1},0,UL,
   def_int,ss_keys}, // joystick button number to use for strafing
  {"joyb_speed",{&joybspeed},{2},0,UL,
   def_int,ss_keys}, // joystick button number to use for running
  {"joyb_use",{&joybuse},{3},0,UL,
   def_int,ss_keys}, // joystick button number to use for use/open

  {"Chat macros",{NULL},{0},UL,UL,def_none,ss_none},
  {"chatmacro0", {0,&chat_macros[0]}, {0,HUSTR_CHATMACRO0},UL,UL,
   def_str,ss_chat}, // chat string associated with 0 key
  {"chatmacro1", {0,&chat_macros[1]}, {0,HUSTR_CHATMACRO1},UL,UL,
   def_str,ss_chat}, // chat string associated with 1 key
  {"chatmacro2", {0,&chat_macros[2]}, {0,HUSTR_CHATMACRO2},UL,UL,
   def_str,ss_chat}, // chat string associated with 2 key
  {"chatmacro3", {0,&chat_macros[3]}, {0,HUSTR_CHATMACRO3},UL,UL,
   def_str,ss_chat}, // chat string associated with 3 key
  {"chatmacro4", {0,&chat_macros[4]}, {0,HUSTR_CHATMACRO4},UL,UL,
   def_str,ss_chat}, // chat string associated with 4 key
  {"chatmacro5", {0,&chat_macros[5]}, {0,HUSTR_CHATMACRO5},UL,UL,
   def_str,ss_chat}, // chat string associated with 5 key
  {"chatmacro6", {0,&chat_macros[6]}, {0,HUSTR_CHATMACRO6},UL,UL,
   def_str,ss_chat}, // chat string associated with 6 key
  {"chatmacro7", {0,&chat_macros[7]}, {0,HUSTR_CHATMACRO7},UL,UL,
   def_str,ss_chat}, // chat string associated with 7 key
  {"chatmacro8", {0,&chat_macros[8]}, {0,HUSTR_CHATMACRO8},UL,UL,
   def_str,ss_chat}, // chat string associated with 8 key
  {"chatmacro9", {0,&chat_macros[9]}, {0,HUSTR_CHATMACRO9},UL,UL,
   def_str,ss_chat}, // chat string associated with 9 key
  
  {"Automap settings",{NULL},{0},UL,UL,def_none,ss_none},
  //jff 1/7/98 defaults for automap colors
  //jff 4/3/98 remove -1 in lower range, 0 now disables new map features
  {"mapcolor_back", {&mapcolor_back}, {247},0,255,  // black //jff 4/6/98 new black
   def_colour,ss_auto}, // color used as background for automap
  {"mapcolor_grid", {&mapcolor_grid}, {104},0,255,  // dk gray
   def_colour,ss_auto}, // color used for automap grid lines
  {"mapcolor_wall", {&mapcolor_wall}, {23},0,255,   // red-brown
   def_colour,ss_auto}, // color used for one side walls on automap
  {"mapcolor_fchg", {&mapcolor_fchg}, {55},0,255,   // lt brown
   def_colour,ss_auto}, // color used for lines floor height changes across
  {"mapcolor_cchg", {&mapcolor_cchg}, {215},0,255,  // orange
   def_colour,ss_auto}, // color used for lines ceiling height changes across
  {"mapcolor_clsd", {&mapcolor_clsd}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for lines denoting closed doors, objects
  {"mapcolor_rkey", {&mapcolor_rkey}, {175},0,255,  // red
   def_colour,ss_auto}, // color used for red key sprites
  {"mapcolor_bkey", {&mapcolor_bkey}, {204},0,255,  // blue
   def_colour,ss_auto}, // color used for blue key sprites
  {"mapcolor_ykey", {&mapcolor_ykey}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for yellow key sprites
  {"mapcolor_rdor", {&mapcolor_rdor}, {175},0,255,  // red
   def_colour,ss_auto}, // color used for closed red doors
  {"mapcolor_bdor", {&mapcolor_bdor}, {204},0,255,  // blue
   def_colour,ss_auto}, // color used for closed blue doors
  {"mapcolor_ydor", {&mapcolor_ydor}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for closed yellow doors
  {"mapcolor_tele", {&mapcolor_tele}, {119},0,255,  // dk green
   def_colour,ss_auto}, // color used for teleporter lines
  {"mapcolor_secr", {&mapcolor_secr}, {252},0,255,  // purple
   def_colour,ss_auto}, // color used for lines around secret sectors
  {"mapcolor_exit", {&mapcolor_exit}, {0},0,255,    // none
   def_colour,ss_auto}, // color used for exit lines
  {"mapcolor_unsn", {&mapcolor_unsn}, {104},0,255,  // dk gray
   def_colour,ss_auto}, // color used for lines not seen without computer map
  {"mapcolor_flat", {&mapcolor_flat}, {88},0,255,   // lt gray
   def_colour,ss_auto}, // color used for lines with no height changes
  {"mapcolor_sprt", {&mapcolor_sprt}, {112},0,255,  // green
   def_colour,ss_auto}, // color used as things
  {"mapcolor_hair", {&mapcolor_hair}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for dot crosshair denoting center of map
  {"mapcolor_sngl", {&mapcolor_sngl}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for the single player arrow
  {"mapcolor_me",   {&mapcolor_me}, {112},0,255, // green
   def_colour,ss_auto}, // your (player) colour
  //jff 3/9/98 add option to not show secrets til after found
  {"map_secret_after", {&map_secret_after}, {0},0,1, // show secret after gotten
   def_bool,ss_auto}, // prevents showing secret sectors till after entered
  //jff 1/7/98 end additions for automap
  {"automapmode", {(int*)&automapmode}, {0}, 0, 31, // CPhipps - remember automap mode
   def_hex,ss_none}, // automap mode
  
  {"Heads-up display settings",{NULL},{0},UL,UL,def_none,ss_none},
  //jff 2/16/98 defaults for color ranges in hud and status
  {"hudcolor_titl", {&hudcolor_titl}, {5},0,9,  // gold range
   def_int,ss_auto}, // color range used for automap level title
  {"hudcolor_xyco", {&hudcolor_xyco}, {3},0,9,  // green range
   def_int,ss_auto}, // color range used for automap coordinates
  {"hudcolor_mesg", {&hudcolor_mesg}, {6},0,9,  // red range
   def_int,ss_mess}, // color range used for messages during play
  {"hudcolor_chat", {&hudcolor_chat}, {5},0,9,  // gold range
   def_int,ss_mess}, // color range used for chat messages and entry
  {"hudcolor_list", {&hudcolor_list}, {5},0,9,  // gold range  //jff 2/26/98
   def_int,ss_mess}, // color range used for message review
  {"hud_msg_lines", {&hud_msg_lines}, {1},1,16,  // 1 line scrolling window
   def_int,ss_mess}, // number of messages in review display (1=disable)
  {"hud_list_bgon", {&hud_list_bgon}, {0},0,1,  // solid window bg ena //jff 2/26/98
   def_bool,ss_mess}, // enables background window behind message review
  {"hud_distributed",{&hud_distributed},{0},0,1, // hud broken up into 3 displays //jff 3/4/98
   def_bool,ss_none}, // splits HUD into three 2 line displays

  {"health_red",    {&health_red}   , {25},0,200, // below is red
   def_int,ss_stat}, // amount of health for red to yellow transition
  {"health_yellow", {&health_yellow}, {50},0,200, // below is yellow
   def_int,ss_stat}, // amount of health for yellow to green transition
  {"health_green",  {&health_green} , {100},0,200,// below is green, above blue
   def_int,ss_stat}, // amount of health for green to blue transition
  {"armor_red",     {&armor_red}    , {25},0,200, // below is red
   def_int,ss_stat}, // amount of armor for red to yellow transition
  {"armor_yellow",  {&armor_yellow} , {50},0,200, // below is yellow
   def_int,ss_stat}, // amount of armor for yellow to green transition
  {"armor_green",   {&armor_green}  , {100},0,200,// below is green, above blue
   def_int,ss_stat}, // amount of armor for green to blue transition
  {"ammo_red",      {&ammo_red}     , {25},0,100, // below 25% is red
   def_int,ss_stat}, // percent of ammo for red to yellow transition
  {"ammo_yellow",   {&ammo_yellow}  , {50},0,100, // below 50% is yellow, above green
   def_int,ss_stat}, // percent of ammo for yellow to green transition
  
  //jff 2/16/98 HUD and status feature controls
  {"hud_active",    {&hud_active}, {2},0,2, // 0=off, 1=small, 2=full
   def_int,ss_none}, // 0 for HUD off, 1 for HUD small, 2 for full HUD
  //jff 2/23/98
  {"hud_displayed", {&hud_displayed},  {0},0,1, // whether hud is displayed
   def_bool,ss_none}, // enables display of HUD
  {"hud_nosecrets", {&hud_nosecrets},  {0},0,1, // no secrets/items/kills HUD line
   def_bool,ss_stat}, // disables display of kills/items/secrets on HUD
  
  {"Weapon preferences",{NULL},{0},UL,UL,def_none,ss_none},
  // killough 2/8/98: weapon preferences set by user:
  {"weapon_choice_1", {&weapon_preferences[0][0]}, {6}, 0,9,
   def_int,ss_weap}, // first choice for weapon (best)
  {"weapon_choice_2", {&weapon_preferences[0][1]}, {9}, 0,9,
   def_int,ss_weap}, // second choice for weapon 
  {"weapon_choice_3", {&weapon_preferences[0][2]}, {4}, 0,9,
   def_int,ss_weap}, // third choice for weapon 
  {"weapon_choice_4", {&weapon_preferences[0][3]}, {3}, 0,9,
   def_int,ss_weap}, // fourth choice for weapon 
  {"weapon_choice_5", {&weapon_preferences[0][4]}, {2}, 0,9,
   def_int,ss_weap}, // fifth choice for weapon 
  {"weapon_choice_6", {&weapon_preferences[0][5]}, {8}, 0,9,
   def_int,ss_weap}, // sixth choice for weapon 
  {"weapon_choice_7", {&weapon_preferences[0][6]}, {5}, 0,9,
   def_int,ss_weap}, // seventh choice for weapon 
  {"weapon_choice_8", {&weapon_preferences[0][7]}, {7}, 0,9,
   def_int,ss_weap}, // eighth choice for weapon 
  {"weapon_choice_9", {&weapon_preferences[0][8]}, {1}, 0,9,
   def_int,ss_weap}, // ninth choice for weapon (worst)
};

int numdefaults;
static const char* defaultfile; // CPhipps - static, const

//
// M_SaveDefaults
//

void M_SaveDefaults (void)
  {
  int   i;
  FILE* f;
  
  f = fopen (defaultfile, "w");
  if (!f)
    return; // can't write the file, but don't complain

  // 3/3/98 explain format of file

  fprintf(f,"# Doom config file\n");
  fprintf(f,"# Format:\n");
  fprintf(f,"# variable   value\n");

  for (i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].type == def_none) {
      // CPhipps - pure headers
      fprintf(f, "\n# %s\n", defaults[i].name);
    } else
    // CPhipps - modified for new default_t form
    if (!IS_STRING(defaults[i])) //jff 4/10/98 kill super-hack on pointer value
      {
      // CPhipps - remove keycode hack
      // killough 3/6/98: use spaces instead of tabs for uniform justification
      if (defaults[i].type == def_hex)
	fprintf (f,"%-25s 0x%x\n",defaults[i].name,*(defaults[i].location.pi));
      else
	fprintf (f,"%-25s %5i\n",defaults[i].name,*(defaults[i].location.pi));
      }
    else
      {
      fprintf (f,"%-25s \"%s\"\n",defaults[i].name,*(defaults[i].location.ppsz));
      }
    }
  
  fclose (f);
  }


//
// M_LoadDefaults
//

#define NUMCHATSTRINGS 10 // phares 4/13/98

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
    if (defaults[i].location.ppsz) 
      *defaults[i].location.ppsz = defaults[i].defaultvalue.psz;
    if (defaults[i].location.pi)
      *defaults[i].location.pi = defaults[i].defaultvalue.i;

    // phares 4/13/98:
    // provide default strings with their own malloced memory so that when
    // we leave this routine, that's what we're dealing with whether there
    // was a config file or not, and whether there were chat definitions
    // in it or not. This provides consistency later on when/if we need to
    // edit these strings (i.e. chat macros in the Chat Strings Setup screen).

    if (IS_STRING(defaults[i]))
      *(defaults[i].location.ppsz) = strdup(*(defaults[i].location.ppsz));
    }
    
  // check for a custom default file

  i = M_CheckParm ("-config");
  if (i && i < myargc-1)
    {
    defaultfile = myargv[i+1];
    }
  else
    defaultfile = basedefault;

  if (i || devparm ) // if debug or -config was specified
    lprintf (LO_CONFIRM, " default file: %s\n",defaultfile);

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

        if (strparm[0] == '"') {
          // get a string default

          isstring = true;
          len = strlen(strparm);
          newstring = (char *) malloc(len);
          strparm[len-1] = 0; // clears trailing double-quote mark
          strcpy(newstring, strparm+1); // clears leading double-quote mark
	} else if ((strparm[0] == '0') && (strparm[1] == 'x')) {
	  // CPhipps - allow ints to be specified in hex
	  sscanf(strparm+2, "%x", &parm);
	} else {
          sscanf(strparm, "%i", &parm);
	  // Keycode hack removed
	}

        for (i = 0 ; i < numdefaults ; i++)
          if ((defaults[i].type != def_none) && !strcmp(def, defaults[i].name))
            {
	    // CPhipps - safety check
            if (isstring != IS_STRING(defaults[i])) {
	      lprintf(LO_WARN, "M_LoadDefaults: Type mismatch reading %s\n", defaults[i].name);
	      continue;
	    }
            if (!isstring)
              {

              //jff 3/4/98 range check numeric parameters

              if ((defaults[i].minvalue==UL || defaults[i].minvalue<=parm) &&
                  (defaults[i].maxvalue==UL || defaults[i].maxvalue>=parm))
                *(defaults[i].location.pi) = parm;
              }
            else
              {
              free((char*)*(defaults[i].location.ppsz));    // phares 4/13/98
              *(defaults[i].location.ppsz) = newstring;
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


// CPhipps - nasty but better than nothing
static boolean screenshot_write_error;

//
// WritePCXfile
//
// CPhipps - static, const parameters, formatting
static void WritePCXfile(const char* filename, const byte* data,
		  int width, int height, const byte* palette)
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
    else {
      *pack++ = 0xc1;
      *pack++ = *data++;
    }
    
  // write the palette

  *pack++ = 0x0c; // palette ID byte
  for (i = 0 ; i < 768 ; i++)
    *pack++ = gammatable[usegamma][*palette++];   // killough
    
  // write output file

  length = pack - (byte *)pcx;
  screenshot_write_error = !M_WriteFile (filename, pcx, length);

  Z_Free (pcx);
}

// jff 3/30/98 types and data structures for BMP output of screenshots
//
// killough 5/2/98:
// Changed type names to avoid conflicts with endianess functions

#define BI_RGB 0L

typedef unsigned long dword_t;
typedef long     long_t;
typedef unsigned char ubyte_t;

#if defined(__MWERKS__)
#pragma options align=packed
#endif

typedef struct tagBITMAPFILEHEADER
  {
  unsigned short  bfType;
  dword_t bfSize;
  unsigned short  bfReserved1;
  unsigned short  bfReserved2;
  dword_t bfOffBits;
  } __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
  {
  dword_t biSize;
  long_t  biWidth;
  long_t  biHeight;
  unsigned short  biPlanes;
  unsigned short  biBitCount;
  dword_t biCompression;
  dword_t biSizeImage;
  long_t  biXPelsPerMeter;
  long_t  biYPelsPerMeter;
  dword_t biClrUsed;
  dword_t biClrImportant;
  } __attribute__ ((packed)) BITMAPINFOHEADER;

#if defined(__MWERKS__)
#pragma options align=reset
#endif
  
// jff 3/30/98 binary file write with error detection
// CPhipps - static, const on parameter
static void SafeWrite(const void *data, size_t size, size_t number, FILE *st)
{
  if (fwrite(data,size,number,st)<number)
    screenshot_write_error = true; // CPhipps - made non-fatal
}

//
// WriteBMPfile
// jff 3/30/98 Add capability to write a .BMP file (256 color uncompressed)
//

// CPhipps - static, const on parameters
static void WriteBMPfile(const char* filename, const byte* data, 
			 int width, int height, const byte* palette)
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
  if (st!=NULL) {
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
    for (i=0;i<768;i+=3) {
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

// CPhipps - modified to use its own buffer for the image
//         - checks for the case where no file can be created (doesn't occur on POSIX systems, would on DOS)
//         - track errors better
//         - split into 2 functions

//
// M_DoScreenShot
// Takes a screenshot into the names file

void M_DoScreenShot (const char* fname)
{
  byte       *linear;
  const byte *pal;
  int        pplump = W_GetNumForName("PLAYPAL");

  screenshot_write_error = false;

  // munge planar buffer to linear
  // CPhipps - use a malloc()ed buffer instead of screens[2]
  I_ReadScreen(linear = malloc(SCREENWIDTH * SCREENHEIGHT));

  // killough 4/18/98: make palette stay around (PU_CACHE could cause crash)
  pal = W_CacheLumpNum (pplump);
    
  // save the pcx file
  //jff 3/30/98 write pcx or bmp depending on mode

  (screenshot_pcx ? WritePCXfile : WriteBMPfile)
    (fname, linear, SCREENWIDTH, SCREENHEIGHT, pal);

  // cph - free the palette
  W_UnlockLumpNum(pplump);
  free(linear);

  // 1/18/98 killough: replace "SCREEN SHOT" acknowledgement with sfx

  if (screenshot_write_error)
    doom_printf("M_ScreenShot: Error writing screenshot");
}

void M_ScreenShot(void)
{
  static int shot;
  char       lbmname[32];
  int        startshot;
  
  screenshot_write_error = false;

  if (access(".",2)) screenshot_write_error = true;

  startshot = shot; // CPhipps - prevent infinite loop
    
  do                                         //jff 3/30/98 pcx or bmp?  
    sprintf(lbmname,"DOOM%02d.%.3s", shot++,screenshot_pcx? "PCX":"BMP");
  while (!access(lbmname,0) && (shot != startshot) && (shot < 10000));

  if (!access(lbmname,0)) screenshot_write_error = true;

  if (screenshot_write_error) {
    doom_printf ("M_ScreenShot: Couldn't create a PCX"); 
    // killough 4/18/98
    return;
  }

  M_DoScreenShot(lbmname); // cph

  S_StartSound(NULL,gamemode==commercial ? sfx_radio : sfx_tink); 
}

//----------------------------------------------------------------------------
//
// $Log: m_misc.c,v $
// Revision 1.43  2000/03/17 20:50:30  cph
// Commit mead's improved mouse stuff
//
// Revision 1.42  1999/11/01 00:13:44  cphipps
// Remove uint_t bizarreness
//
// Revision 1.41  1999/10/27 18:35:50  cphipps
// Made W_CacheLump* return a const pointer
//
// Revision 1.40  1999/10/12 13:01:12  cphipps
// Changed header to GPL
//
// Revision 1.39  1999/09/06 19:41:53  cphipps
// Use macros set by autoconf setup for default locations of sound and music servers
//
// Revision 1.38  1999/09/05 20:12:51  cphipps
// Removed help strings from the config file (depreciated, help goes in boom.cfg.5 now)
// Removed help strings from the defaults array, turned them into comments instead
//
// Revision 1.37  1999/05/16 08:47:16  cphipps
// Minor change to ease compiling on Solaris
//
// Revision 1.36  1999/04/01 14:29:19  cphipps
// Add Xwindows options variable to config file
//
// Revision 1.35  1999/03/26 11:08:57  cphipps
// Store full automap mode in config file instead of just follow mode
// Remove unneeded extern decls
//
// Revision 1.34  1999/03/24 09:47:04  cphipps
// Remove multiplayer colours variables, added single mapcolor_me variable instead
//
// Revision 1.33  1999/03/22 17:11:25  cphipps
// Remove player colour config options
//
// Revision 1.32  1999/03/13 10:17:46  cphipps
// Add the new automap keys to config file
//
// Revision 1.31  1999/01/25 22:33:25  cphipps
// Fix type in default sound server
//
// Revision 1.30  1999/01/23 16:37:58  cphipps
// Change sound/music servers to be in /usr/local/games explicitely
// by default.
//
// Revision 1.29  1999/01/23 07:44:35  cphipps
// Make default sound/music server path not start with ./, so they
// can be found via path too.
//
// Revision 1.28  1999/01/12 18:58:38  cphipps
// Screenshot handling broken up into 2 functions
//
// Revision 1.27  1999/01/07 10:34:23  cphipps
// Alignment options for Metrowerks compiler
//
// Revision 1.26  1999/01/01 16:53:54  cphipps
// Add variable to control aspects of ENDOOM display
//
// Revision 1.25  1999/01/01 16:14:56  cphipps
// Free the screenshot buffer
//
// Revision 1.25  1999/01/01 16:01:18  cphipps
// Revision 1.24  1998/12/31 20:27:59  cphipps
// New wad lump handling
//
// Revision 1.23  1998/12/31 11:09:01  cphipps
// Patch drawing updated
//
// Revision 1.22  1998/12/28 21:26:22  cphipps
// Reformat M_ScreenShot
// Make it allocate memory for the image on its own, instead of using spare screens
// Error handling improved some more
//
// Revision 1.21  1998/12/28 13:13:10  cphipps
// Add multiplayer colours variable to config file
// Fix upper limit of compatibilit_level variable
//
// Revision 1.20  1998/12/26 11:55:48  cphipps
// Modified for new default_compatibility variable
//
// Revision 1.19  1998/12/25 18:40:14  cphipps
// Fix i/b chat keys reversal, thanks to MBF
//
// Revision 1.18  1998/12/24 17:45:25  cphipps
// Overhaul defaults[] table for new default_t struct
// Reorder defaults[] array, add section headers
// Modify M_LoadDefaults and M_SaveDefaults for the new struct
// Add ability to read/write ints in hex
// Preserve automap follow mode in config file
// Change auto_load_wads to auto_load, since deh/bex files will also be supported
//
// Revision 1.17  1998/12/19 20:44:06  cphipps
// Report error on failed screenshots
//
// Revision 1.16  1998/12/16 22:34:55  cphipps
// Added default screen size vars to config file
//
// Revision 1.15  1998/12/16 21:24:40  cphipps
// Fix joystick parameters comments to be clearly comments
// Add auto_load_wads variable
//
// Revision 1.14  1998/12/07 09:57:18  cphipps
// Add option so player can control what happens to the music when the game
// is paused
//
// Revision 1.13  1998/12/02 09:14:26  cphipps
// Print config file name if devparm
//
// Revision 1.12  1998/11/17 07:47:06  cphipps
// Hi-res changes
//
// Revision 1.11  1998/11/03 12:15:06  cphipps
// Fixed bad example sound device string
//
// Revision 1.10  1998/10/27 18:14:15  cphipps
// Boom v2.02 updates - logical console output stuff
//
// Revision 1.9  1998/10/20 18:13:00  cphipps
// Add music server to config file
//
// Revision 1.8  1998/10/20 06:58:37  cphipps
// dprintf -> doom_printf
//
// Revision 1.7  1998/10/16 23:08:05  cphipps
// Release a variable so m_menu.c can see it
//
// Revision 1.6  1998/10/16 20:56:14  cphipps
// Added const, static to some variables
//
// Revision 1.5  1998/10/16 20:27:32  cphipps
// Made defaults table const
// Added some static's and const's o the screenshot code
//
// Revision 1.4  1998/10/15 18:48:52  cphipps
// Remove redundant linux mouse type/device vars
//
// Revision 1.3  1998/10/10 20:30:46  cphipps
// Added sound device to config file
//
// Revision 1.2  1998/09/19 11:05:21  cphipps
// Added joystick calibration variables
//
// Revision 1.1  1998/09/13 16:49:50  cphipps
// Initial revision
//
// Revision 1.4  1998/09/13 15:46:20  cphipps
// *** empty log message ***
//
// Revision 1.3  1998/09/13 15:31:16  cphipps
// Added soundserver path to config file
//
// Revision 1.2  1998/09/12 19:31:17  cphipps
// Removed ^M's
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

