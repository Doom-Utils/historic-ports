//  
// DOSDoom Misc System Interface Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/07/21 i_love_bill config & P2 detection.
//
// -ES- 1998/08/05 New CPU detecting code
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

#include <winalleg.h>
#include <allegro.h>

#include "dm_defs.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_debug.h"
#include "d_net.h"
#include "d_main.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_menu.h"
#include "w_wad.h"
#include "z_zone.h"

// -ES- 1998/08/05 Need R_DrawColumn/R_DrawSpan prototypes
#include "r_draw1.h"
#include "r_draw2.h"

void I_CalibrateJoystick(int ch);

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

#define BGCOLOR		4
#define FGCOLOR		0xf

int     mb_used = 10;

char oldkeystate[128];
volatile int mselapsed=0;
int mousepresent = -1;

cputype_t cpu;

extern boolean demorecording;
extern boolean graphicsmode;
extern int usejoystick;
extern int usemouse;

//
// -MH- 1998/07/02  USABLE_EXTKEY, I_DoomCode2ScanCode, I_ScanCode2DoomCode
// -MH- 1998/08/18  Placed up here so defined _before_ use
//
// Doom seems to have some of the special keys usable as control keys and
// others not. Whether or not a special key is is determined by the two
// functions mentioned above and a LONG if statement further down in this
// module. For example, if the functions have them but they are not in the
// if-statement, the DEFAULT.CFG logic can handle them but Doom can't; that
// is to say, you can't assign them in the CONTROLS menu.
//
// I have changed the contents of the if-statement into a macro to make it
// more manageable and I have move the functions from i_video.c to here so
// that they are all defined together. Who knows what the hell they were
// doing in i_video.c since they are nothing to do with video logic.
//
// I have cleaned up the functions and the macro and made these changes
// because the menu won't recognise some of the keys especially insert and
// delete. I guess we should really do same for function keys as well, but
// one possible bug-introducer at a time, eh :-)
//

// Extended keys that can be used in menus
#define USABLE_EXTKEY(i)      \
        (  (i==0x48)          \
         ||(i==0x50)          \
         ||(i==0x4b)          \
         ||(i==0x4d)          \
         ||(i==0x0e)          \
         ||(i==0x2a)          \
         ||(i==KEY_RCONTROL)  \
         ||(i==KEY_ALTGR)     \
         ||(i==KEY_PGUP)      \
         ||(i==KEY_PGDN)      \
         ||(i==KEY_HOME)      \
         ||(i==KEY_END)       \
         ||(i==KEY_INSERT)    \
         ||(i==KEY_DEL)       \
         ||(i==KEY_PRTSCR))

int I_ScanCode2DoomCode (int a)
{
  switch (a)
  {
    case 0x48:         return KEYD_UPARROW;
    case 0x50:         return KEYD_DOWNARROW;
    case 0x4b:         return KEYD_LEFTARROW;
    case 0x4d:         return KEYD_RIGHTARROW;
    case 0x0e:         return KEYD_BACKSPACE;
    case 0x2a:         return KEYD_RSHIFT;
    case KEY_RCONTROL: return KEYD_RCTRL;
    case KEY_ALTGR:    return KEYD_RALT;
    case KEY_PGUP:     return KEYD_PGUP;
    case KEY_PGDN:     return KEYD_PGDN;
    case KEY_HOME:     return KEYD_HOME;
    case KEY_END:      return KEYD_END;
    case KEY_INSERT:   return KEYD_INSERT;
    case KEY_DEL:      return KEYD_DELETE;
    case KEY_PRTSCR:   return KEYD_PRTSCR;  // -MH- 1998/08/18 Added "Print Screen"
  }

  if ((key_shifts & KB_NUMLOCK_FLAG) && (a == KEY_5_PAD))
      return '5';

  if (a>=0x100)
    return a;
  else if (key_ascii_table[a]>8)
    return key_ascii_table[a];
  else
    return a+0x80;
}

int I_DoomCode2ScanCode (int a)
{
  int i;

  switch (a)
  {
    case KEYD_UPARROW:    return 0x48;
    case KEYD_DOWNARROW:  return 0x50;
    case KEYD_LEFTARROW:  return 0x4b;
    case KEYD_RIGHTARROW: return 0x4d;
    case KEYD_BACKSPACE:  return 0x0e;
    case KEYD_RSHIFT:     return 0x2a;
    case KEYD_RALT:       return KEY_ALTGR;
    case KEYD_PGUP:       return KEY_PGUP;
    case KEYD_PGDN:       return KEY_PGDN;
    case KEYD_HOME:       return KEY_HOME;
    case KEYD_END:        return KEY_END;
    case KEYD_INSERT:     return KEY_INSERT;
    case KEYD_DELETE:     return KEY_DEL;
    case KEYD_PRTSCR:     return KEY_PRTSCR;  // -MH- 1998/08/18 Added "Print Screen"
  }

  if (a>=0x100)
    return a;
  else if (a>=0x80)
    return (a-0x80);
  else
  {
    for (i=0;i<128;i++)
      if (key_ascii_table[i]==a)
        return i;
    return 0;
  }
}


void I_Tactile (int on, int off, int total)
{
  // UNUSED.
  on = off = total = 0;
}

static ticcmd_t emptycmd = {
 0, // vertangle
 0, // upwardmove
 0, // forwardmove
 0, // sidemove
 0, // angleturn
 0, // consistancy
 0, // chatchar
 0, // buttons
 0  // extbuttons
 };

ticcmd_t* I_BaseTiccmd(void)
{
    return &emptycmd;
}

// -KM- 1998/09/01 Handles keyboard
static void Keyboard_Event()
{
  event_t event;
  char keystate[128];
  int i;

  //key presses
  for (i=0;i<128;i++)
    keystate[i]=key[i];

  for (i=0;i<128;i++)
  {
    char normkey,extkey,oldnormkey,oldextkey;

    normkey=keystate[i]&KB_NORMAL; extkey=keystate[i]&KB_EXTENDED;
    oldnormkey=oldkeystate[i]&KB_NORMAL; oldextkey=oldkeystate[i]&KB_EXTENDED;

    if ((normkey!=0)&&(oldnormkey==0))
    {
      event.type=ev_keydown;
      event.data1=I_ScanCode2DoomCode(i);
      D_PostEvent(&event);
    }
    if ((normkey==0)&&(oldnormkey!=0))
    {
      event.type=ev_keyup;
      event.data1=I_ScanCode2DoomCode(i);
      D_PostEvent(&event);
    }
    if ((extkey!=0)&&(oldextkey==0))
    {
      if USABLE_EXTKEY(i)              /* -MH- 1998/08/18 Fixed keys for up/down etc */
      {
        event.type=ev_keydown;
        event.data1=I_ScanCode2DoomCode(i);
        D_PostEvent(&event);
      }
      else if (i==0x7b)
      {
        event.type=ev_keydown;
        event.data1=KEYD_PAUSE;
        D_PostEvent(&event);
        key[0x7b]=0;break;
        event.type=ev_keyup;
        event.data1=KEYD_PAUSE;
        D_PostEvent(&event);
      }
    }
    if ((extkey==0)&&(oldextkey!=0))
    {
      if USABLE_EXTKEY(i)
      {
        event.type=ev_keyup;
        event.data1=I_ScanCode2DoomCode(i);
        D_PostEvent(&event);
      }
    }
  }

  memcpy(oldkeystate,keystate,128);
}

// -KM- 1998/09/01 Handles Mouse
// -KM- 1998/12/21 Boosted mouse speed 10 times.
static void Mouse_Event(boolean enabled)
{
 event_t event;
 int xmickeys, ymickeys;
 static int lastbuttons=0;
 int buttons = 0;
 // No Mouse detected
 if (mousepresent == -1) return;

 if (enabled)
 {
   get_mouse_mickeys(&xmickeys,&ymickeys);
  
   event.type=ev_analogue;
  
   event.data1=mouse_xaxis;
   event.data2=xmickeys*(mouseSensitivity+5);
  
   event.data3=mouse_yaxis;
   event.data4=-ymickeys*(mouseSensitivity+5);
   if (invertmouse)
     event.data4 = -event.data4;
   if ((xmickeys!=0)||(ymickeys!=0))
     D_PostEvent(&event);
  
   //now, do buttons
   buttons=mouse_b;
 }
 if (buttons!=lastbuttons)
 {
   int j=1, i;
   for (i=0;i<3;i++)
   {
     if ((buttons&j) && !(lastbuttons&j))
     {
       event.type=ev_keydown;
       event.data1=KEYD_MOUSE1+i;
       D_PostEvent(&event);
     }
     if (!(buttons&j) && (lastbuttons&j))
     {
       event.type=ev_keyup;
       event.data1=KEYD_MOUSE1+i;
       D_PostEvent(&event);
     }
     j<<=1;
   }
 }
 lastbuttons=buttons;
}

// -KM- 1998/09/01 Handles Joystick.  Hat support added
static void Joystick_Event(boolean enabled)
{
  static int oldbuttons=0;
  int buttons = 0;
  event_t event;

  if (enabled)
  {
    poll_joystick();
    event.type=ev_analogue;
  
    event.data1=joy_xaxis;
    event.data2=abs(joy_x) < 4 ? 0 : joy_x;
  
    event.data3=joy_yaxis;
    event.data4=abs(joy_y) < 4 ? 0 : joy_y;
  
    D_PostEvent(&event);
  
    //now, do buttons
    // -KM- 1998/12/16 Do all 8 buttons.
    if (joy_b1) buttons|=1;
    if (joy_b2) buttons|=2;
    if (joy_b3) buttons|=4;
    if (joy_b4) buttons|=8;
    if (joy_b5) buttons|=16;
    if (joy_b6) buttons|=32;
    if (joy_b7) buttons|=64;
    if (joy_b8) buttons|=128;
    // And hat switch
    if      (joy_hat == JOY_HAT_LEFT ) buttons|=256;
    else if (joy_hat == JOY_HAT_DOWN ) buttons|=512;
    else if (joy_hat == JOY_HAT_RIGHT) buttons|=1024;
    else if (joy_hat == JOY_HAT_UP   ) buttons|=2048;
  }
  if (buttons != oldbuttons)
  {
    int j=1, i;
    for (i=0;i<12;i++)
    {
      // Button press detected
      if ((buttons & j) && !(oldbuttons & j))
      {
        event.type=ev_keydown;
        event.data1=KEYD_JOY1+i;
        D_PostEvent(&event);
      }

      // Button released
      if ((oldbuttons & j) && !(buttons & j))
      {
        event.type=ev_keyup;
        event.data1=KEYD_JOY1+i;
        D_PostEvent(&event);
      }

      // Shift to next bit index
      j <<= 1;
    }
  }
  oldbuttons=buttons;
}

// -KM- 1998/09/01 Split joystick/mouse/keyboard into respective functions
void I_GetEvent()
{
  Keyboard_Event();

  Mouse_Event(usemouse);

  Joystick_Event(usejoystick);

}

//
// I_StartTic
//
void I_StartTic()
{
  I_GetEvent();
  //i dont think i have to do anything else here
}

//int  I_GetHeapSize (void)
//{
//    return mb_used*1024*1024;
//}

// -KM- 1998/07/31 Implemented memory sizing based on available;
//  instead of default value.
byte* I_ZoneBase (int*	size)
{
  static void *zonebase = NULL;
  int p, s = 1;
  if (zonebase) return zonebase;

  p = M_CheckParm ("-heapsize");
  *size = 0x600000;
  mb_used = 6;
  if (p && p < myargc-1)
  {
    mb_used=atoi(myargv[p+1]);
    *size = mb_used * 1024 * 1024;
  } else {
    s = get_config_int("system", "heapsize", 0);
    if (s) {
      mb_used = s;
      *size = mb_used * 1024; // Kilobytes :-) Kester
      s = 0;
    } else {
      *size = mb_used * 1024 * 1024;
      s = 1;
    }
  }

  I_Printf ("  Heapsize: %d %s\n\r",mb_used,s?"Megabytes":"Kilobytes");
  return (byte *)(zonebase = malloc (*size));
}

void I_timer(void)
{
  mselapsed++;
  cdcounter--;
}
END_OF_FUNCTION(I_timer);

// -KM- (-ACB- It changes from my base, must be new) Joystick Config
void I_CalibrateJoystick(int ch)
{
  const char *dirs[] = {"CENTRE", "LEFT", "DOWN", "RIGHT", "UP" };
  static int phase = 0;
  static int hat_phase = 0;
  static char* message = NULL;
  static char* joy_centre = NULL;
  static char* joy_tl = NULL;
  static char* joy_br = NULL;
  static char* joy_ht = NULL;   // use joy_ht instead of joy_hat -ACB-
  static char* joy_t_max = NULL;
  static char* joy_t_min = NULL;

  if (!joy_centre)
    joy_centre = DDF_LanguageLookup("JoystickCentre");

  if (!joy_tl)
    joy_tl = DDF_LanguageLookup("JoystickTL");

  if (!joy_br)
    joy_br = DDF_LanguageLookup("JoystickBR");

  if (!joy_ht)
    joy_ht = DDF_LanguageLookup("JoystickHAT");

  if (!joy_t_max)
    joy_t_max = DDF_LanguageLookup("JoyThrottleMAX");

  if (!joy_t_min)
    joy_t_min = DDF_LanguageLookup("JoyThrottleMIN");

  if (graphicsmode)
  {
    if (!message)
      message = Z_Malloc(64, PU_STATIC, &message);
    else
      Z_ChangeTag(message, PU_STATIC);

     // -KM- 1998/07/31 Added escape functionality.
    if (ch == KEYD_ESCAPE)
    {
      phase = hat_phase = 0;
      Z_ChangeTag(message, PU_CACHE);
      return;
    }


    switch (phase)
    {
      case 0:
           // -KM- 1998/09/01 Prevent joystick doing stuff while yer trying to calibrate it.
           usejoystick = false;
           load_joystick_data(NULL);
           strcpy(message, joy_centre);
           break;
      case 1:
           initialise_joystick();
           strcpy(message,  joy_tl);
           break;
      case 2:
           calibrate_joystick_tl();
           strcpy(message,  joy_br);
           break;
      case 3:
           calibrate_joystick_br();
/*           if (joy_type == JOY_TYPE_FSPRO)
           {
              strcpy(message,  joy_t_min);
              phase = 9;
              break;
           } else if (joy_type != JOY_TYPE_WINGEX)
           {*/
              strcpy(message, joy_centre);
              phase = 11;
              save_joystick_data(NULL);
              break;
//           }
           // -KM- 1998/07/31 Fixed bug.
           phase = 4;
      case 4:
           sprintf(message, joy_ht, dirs[hat_phase]);
           break;
      case 5 ... 8:
           calibrate_joystick_hat(hat_phase++);
           sprintf(message, joy_ht, dirs[hat_phase]);
           break;
      case 9:
           calibrate_joystick_hat(hat_phase);
           hat_phase = phase = 0;
           Z_ChangeTag(message, PU_CACHE);
           usejoystick = true;
           save_joystick_data(NULL);
           return;
      case 10:
           calibrate_joystick_throttle_min();
           strcpy(message,  joy_t_min);
           break;
      case 11:
           calibrate_joystick_throttle_max();
           save_joystick_data(NULL);
      case 12:
           usejoystick = true;
      default:
           phase = 0;
           Z_ChangeTag(message, PU_CACHE);
           return;
    }
    phase++;
    // -KM- 1998/07/31 Accept any key.
    M_StartMessage(message, I_CalibrateJoystick, false);
    return;
  } else {
    int key = 0;
    load_joystick_data(NULL);
    I_Printf(DDF_LanguageLookup("JoystickCentreT"));
    while (!keypressed() && !joy_b1 && !joy_b2) poll_joystick();
    if (keypressed()) key = readkey()>>8;
    I_Printf("\n");
    if (key == KEY_ESC || joy_b2) return;
    initialise_joystick();

    while (joy_b1) poll_joystick();
    I_Printf(DDF_LanguageLookup("JoystickTLT"));
    while (!keypressed() && !joy_b1) poll_joystick();
    if (keypressed()) readkey();
    I_Printf("\n");
    calibrate_joystick_tl();

    while (joy_b1) poll_joystick();
    I_Printf(DDF_LanguageLookup("JoystickBRT"));
    while (!keypressed() && !joy_b1) poll_joystick();
    if (keypressed()) readkey();
    I_Printf("\n");
    calibrate_joystick_br();
/*    if (joy_type == JOY_TYPE_FSPRO) {
      while (joy_b1) poll_joystick();
      I_Printf(DDF_LanguageLookup("JoyThrottleMIN"));
      while (!keypressed() && !joy_b1) poll_joystick();
      if (keypressed()) readkey();
      I_Printf("\n");
      calibrate_joystick_throttle_min();

      while (joy_b1) poll_joystick();
      I_Printf(DDF_LanguageLookup("JoyThrottleMAX"));
      while (!keypressed() && !joy_b1) poll_joystick();
      if (keypressed()) readkey();
      I_Printf("\n");
      calibrate_joystick_throttle_max();
    } else if (joy_type == JOY_TYPE_WINGEX) {
      int i;
      char* hat_string = DDF_LanguageLookup("JoystickHATT");
      for (i = 0; i < 5; i++) {
        while (joy_b1) poll_joystick();
        I_Printf(hat_string, dirs[i]);
        while (!keypressed() && !joy_b1) poll_joystick();
        if (keypressed()) readkey();
        I_Printf("\n");
        calibrate_joystick_hat(i);
      }
    }*/
    save_joystick_data(NULL);
  }
}

//
// I_Init
//
// -ACB- 1998/07/11 Reformatted the code.
//
void I_Init (void)
{
 //  allegro_init();

  //init keyboard
  memset(oldkeystate,0,128);

  install_keyboard();

  //init the joystick
  if (usejoystick)
    I_CalibrateJoystick(0);

  //init timer
  LOCK_VARIABLE(mselapsed);
  LOCK_VARIABLE(cdcounter);
  LOCK_FUNCTION(I_timer);

  // -ACB- 1998/07/18 Remove that bloody windows 95 hassle :)
  // -KM- 1998/07/21 Add it to your config file for permanancy
  // -ACB- 1998/07/21 I hate bill, disables win95 friendlyness....
  i_love_bill = get_config_int("system", "i_love_bill", FALSE);
  i_love_bill |= (!M_CheckParm("-ihatebill"));

  if (!M_CheckParm("-noirq"))
  {
    install_timer();
  
    install_int_ex(I_timer,BPS_TO_TIMER(TICRATE));
  }

  //init the mouse
  mousepresent=install_mouse();
  if (mousepresent!=-1)
    show_mouse(NULL);

  if (M_CheckParm("-novert"))
    novert=1;

  if (M_CheckParm("-nosound"))
    nosound = true;
  else
    nosound = I_InitSound();
}

//
// I_Quit
//
// -ACB- 1998/07/11 Tidying the code
//
void I_Quit (void)
{
  if (demorecording)
    G_CheckDemoStatus();

  D_QuitNetGame ();

  if (!nosound)
  {
      I_ShutdownSound();
      I_ShutdownMusic();
  }

  M_SaveDefaults();
  I_ShutdownGraphics();

  remove_keyboard();

  if (mousepresent!=-1)
    remove_mouse();

  remove_timer();

#ifdef DEVELOPERS
  if (debugfile)
    fclose(debugfile);
#endif

  // Throw the end text at the screen & exit - Kester
//  puttext(1, 1, 80, 25, W_CacheLumpName("ENDOOM", PU_CACHE));

  //
  // -ACB- 1998/09/06 line 25, or we end up on the top of Doom II's
  //                  ENDOOM Screen. Not good. 
  //
//  gotoxy(1, 25);
  exit(0);
}


void I_WaitVBL(int count)
{
  if (mselapsed>0)
    rest(count*1000/70);
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//
int I_GetTime (void)
{
  if (mselapsed>0)
  {
    return (mselapsed);
  }
  else
  {
    struct timeval        tp;
    struct timezone       tzp;
    int                   newtics;
    static int            basetime=0;

    gettimeofday(&tp, &tzp);

    if (!basetime)
      basetime = tp.tv_sec;

    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;
  }
}

byte*	I_AllocLow(int length)
{
    byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
static char         msgbuf[512];
void I_Error (char *error, ...)
{
    va_list	argptr;

    // Shutdown. Here might be other errors.
    if (demorecording)
      G_CheckDemoStatus();

    D_QuitNetGame ();

    // -KM- 1998/12/17 M_CheckParm("-nosound") changed to nosound
    if (!nosound)
    {
      I_ShutdownSound();
      I_ShutdownMusic();
    }

    if (graphicsmode)
      I_ShutdownGraphics();
    
#ifdef DEVELOPERS
  if (debugfile)
    fclose(debugfile);
#endif

    // Message last, so it actually prints on the screen
    va_start (argptr,error);

    // Print the message into a text string
    vsprintf(msgbuf, error, argptr);
    MessageBox (NULL, msgbuf, "Error", MB_ICONASTERISK | MB_OK);

    va_end (argptr);

    exit(-1);
}

static char textmode[25][81];
static BITMAP* loading = NULL;
void I_Printf (char *message, ...)
{
    va_list      argptr;
    char	*string = msgbuf;
    static int y = -1;
    static int x = 0;
    va_start (argptr, message);

    // Print the message into a text string
    vsprintf(msgbuf, message, argptr);
    
    // If debuging enabled, print to the debugfile
    Debug_Printf(msgbuf);
    
    // Clean up \n\r combinations
    while (*string) {
      if (*string == '\n') {
        memmove(string + 2, string + 1, strlen(string));
        string[1] = '\r';
        string++;
      }
    string++;
    }

    // If we are in graphics mode, we make the message
    // appear on the status line
    if (graphicsmode) players[consoleplayer].message = msgbuf;
    
    // Otherwise, print it to the screen
    else
    {
      int i;
      string = msgbuf;
      if (y == -1)
      {
        memset(textmode, 0, sizeof(textmode));
        x = y = 0;
        text_mode(-1);
      }
      while (*string)
      {
        if (*string == '\n')
        {
//          memset(&textmode[y][x], 0, 81 - x);
          y++; x = 0;
          if (y == 25)
          {
            memmove(&textmode[0][0], &textmode[1][0], 24*81);
            memset(textmode[24], 0, 81);
            y = 24;
          }
          string++;
          continue;
        }
        if (*string == '\x8')
          x--;
        else
          textmode[y][x++] = *string;
        if (x == 80)
        {
//          memset(&textmode[y][x], 0, 81 - x);
          y++; x = 0;
          if (y == 25)
          {
            memmove(&textmode[0][0], &textmode[1][0], 24*81);
            memset(textmode[24], 0, 81);
            y = 24;
          }
        }
        string++;
      }

      I_PutTitle("DOSDoom/Win");
      for (i = 0; i < 25; i++)
        textout(loading, font, textmode[i], 1, (i+1) * text_height(font)+3+i, makecol(127,127,127));
      acquire_screen();
      blit(loading, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      release_screen();
    }
    
    va_end (argptr);
}

// -KM- 1998/10/29 Use all of screen, not just first 25 rows.
void I_PutTitle(char *title)
{
      char string[81] = { [0 ... 79] = ' ', [80] = 0 };
      char dosdoom[81];
      int centre;
      int x;

      // Build the title
      // 23-6-98 KM Changed dosdoomver to hex to allow versions such as 0.65a
      sprintf(dosdoom, "DOSDoom v%x.%x", DOSDOOMVER / DOSDOOMVERFIX, DOSDOOMVER % DOSDOOMVERFIX);
      centre = (80 - strlen(title)) / 2;
      memcpy(&string[centre], title, strlen(title));
      memcpy(&string[1], dosdoom, strlen(dosdoom));
      sprintf(dosdoom, "DOOM v%d.%d", VERSION / VERSIONFIX, VERSION % VERSIONFIX);
      centre = 79 - strlen(dosdoom);
      memcpy(&string[centre], dosdoom, strlen(dosdoom));

      // Print the title
      if (!loading)
        loading = create_bitmap(SCREEN_W, SCREEN_H);
      clear(loading);
      for (x = 0; x < 640; x++)
         vline(loading, x, 0, text_height(font) + 2, makecol(x*255/640, 0, 0));
      textout(loading, font, string, 1, 2, makecol(255, 255, 255));
}

//
// I_CheckCPU
// Uses the allegro routine to gain cpu info
//
// -ACB- 1998/07/17 New CPU Checking Code
//
// -ES- 1998/08/05 Newer CPU Checking Code
// -ES- 1998/08/13 Some minor changes
// -ES- 1998/12/18 Added some new routines

col_func RDC8_C_KM   ={"KM",         R_DrawColumn8_KM,       false, NULL};
col_func RDC8_C      ={"C",          R_DrawColumn8_CVersion, false, &RDC8_C_KM};
col_func *RDC8_Head = &RDC8_C;

col_func RDC16_C_KM   ={"KM",        R_DrawColumn16_KM,      false, NULL};
col_func RDC16_C     ={"C",          R_DrawColumn16_CVersion,false, &RDC16_C_KM};
col_func *RDC16_Head = &RDC16_C;

span_func RDS8_KM     ={"KM",         R_DrawSpan8_KM,         false, NULL};
span_func RDS8_C      ={"C",          R_DrawSpan8_CVersion,   false, &RDS8_KM};
span_func *RDS8_Head = &RDS8_C;

span_func RDS16_BLF   ={"BLF",        R_DrawSpan16_BLF,       false, NULL};
span_func RDS16_KM    ={"KM",         R_DrawSpan16_KM,        false, &RDS16_BLF};
span_func RDS16_C     ={"C",          R_DrawSpan16_CVersion,  false, &RDS16_KM};
span_func *RDS16_Head = &RDS16_C;
// Initialised after deciding colour depth
col_func  *RDC_Head = NULL;
span_func *RDS_Head = NULL;

// CPU-specific optimisation
cpumodel_t cpu_c    = {"Default CPU",   &RDC8_C,       &RDC16_C,     &RDS8_C,     &RDS16_C};

void I_CheckCPU()
{
 cpu.model = &cpu_c; // Default: C versions of everything
 if (!M_CheckParm("-UseC"))
 {
   check_cpu();

   cpu.fpu   = cpu_fpu;
   // -KM- 1998/11/25 Allegro can handle mmx now, don't need to disable.
   cpu.mmx = cpu_mmx;

   I_Printf ("CPU Detected: %s ",cpu.model->name);
  
   if (cpu.mmx || cpu.fpu/* || cpu_3dnow*/)
   {
     I_Printf("with ");
  
     if (cpu.mmx)
       I_Printf("MMX,");
//     if (cpu_3dnow)
//       I_Printf("3dNow!,");
     if (cpu.fpu)
       I_Printf("FPU");
  
     I_Printf(" Present");
   }
  
   if (cpu_cpuid)
     I_Printf(" - CPUID: %s",cpu_vendor);
  
   I_Printf("\n");
 }
}



