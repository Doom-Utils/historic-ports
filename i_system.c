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
#include <conio.h>
#include <string.h>

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>

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

#define BGCOLOR         4
#define FGCOLOR         0xf

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
// -ES- 1999/03/28 Added Enter on numeric keypad
#define USABLE_EXTKEY(i)      \
        (  (i==0x48)          \
         ||(i==0x50)          \
         ||(i==0x4b)          \
         ||(i==0x4d)          \
         ||(i==0x0e)          \
         ||(i==0x2a)          \
         ||(i==KEY_ENTER)     \
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

static int CountAxis(int joynum)
{
  int sticks;
  int axis = 0;
  for (sticks = 0; sticks < joy[joynum].num_sticks; sticks++)
     axis += joy[joynum].stick[sticks].num_axis;

  return axis;
}

// -KM- 1998/09/01 Handles Joystick.  Hat support added
static void Joystick_Event(boolean enabled)
{
  static boolean* oldbuttons = NULL;
  static int numButtons = -1;

  static int*     axis = NULL;
  static int numAxis = -1;
  int i, a, s;
  event_t event;

  if (enabled)
  {
    if (numButtons != joy[0].num_buttons)
    {
      oldbuttons = Z_ReMalloc(oldbuttons, sizeof(boolean) * joy[0].num_buttons);
      numButtons = joy[0].num_buttons;
      memset (oldbuttons, false, sizeof(boolean) * numButtons);
    }

    i = CountAxis(0);
    if (numAxis != i)
    {
      char name[32];
      numAxis = i;
      axis = Z_ReMalloc(axis, sizeof(int) * numAxis);
      for (i = 0; i < numAxis-2; i++)
      {
         sprintf(name, "MiscAxis%d", i+1);
         axis[i] = get_config_int("Joystick", name, AXIS_DISABLE);
      }
    }

    poll_joystick();
    event.type=ev_analogue;
  
    event.data1=joy_xaxis;
    event.data2=abs(joy_x) <= 8 ? 0 : joy_x;
  
    event.data3=joy_yaxis;
    event.data4=abs(joy_y) <= 8 ? 0 : joy_y;
  
    D_PostEvent(&event);

    a = 0;
    for (s = 1; s < joy[0].num_sticks; s++)
    {
      for (i = 0; i < joy[0].stick[s].num_axis; i++)
      {
         event.type = ev_analogue;
         event.data1 = axis[a++];
         event.data2 =
           abs(joy[0].stick[s].axis[i].pos) <= 8 ? 0 : joy[0].stick[s].axis[i].pos;
         event.data3 = AXIS_DISABLE;
         event.data4 = 0;
         if (joy[0].stick[s].flags & JOYFLAG_UNSIGNED)
           event.data2 -= 128;

         D_PostEvent(&event);
      }
    }

    //now, do buttons
    // -KM- 1998/12/16 Do all 8 buttons.
    for (i = 0; i < numButtons; i++)
    {
       if (joy[0].button[i].b && !oldbuttons[i])
       {
         event.type = ev_keydown;
         event.data1 = KEYD_JOYBASE + i;
         D_PostEvent(&event);
       }

       if (!joy[0].button[i].b && oldbuttons[i])
       {
         event.type = ev_keyup;
         event.data1 = KEYD_JOYBASE + i;
         D_PostEvent(&event);
       }

       oldbuttons[i] = joy[0].button[i].b;
    }
  }
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
}

// -KM- 1998/07/31 Implemented memory sizing based on available;
//  instead of default value.
// -KM- 1999/01/31 Always use available memory.  Give a warning on low memory.
byte* I_ZoneBase (int*  size)
{
  static void *zonebase = NULL;
  int p, s = 1;
  int phys, virt, recom, tphys;
  _go32_dpmi_meminfo mem;
  if (zonebase) return zonebase;

  p = M_CheckParm ("-heapsize");
  _go32_dpmi_get_free_memory_information(&mem);
  virt = mem.available_memory;
  tphys = mem.total_physical_pages * 4096;
  phys = mem.available_physical_pages * 4096;
  recom = phys - (8 * 1024 * 1024);
  // Set minimum 4 mb
  if (recom < 0x400000)
  {
    I_Printf("  Warning: You are very low on memory. (%d/%d)\n", phys/0x100000, tphys/0x100000);
    I_Printf("           We'll give it a shot anyway though and see how far we get.\n");
    recom = 0x400000;
  }
#ifdef DEVELOPERS
  I_Printf ("  Memory: Physical %d/%d, Virtual %d, Recommended: %d\n", phys,tphys, virt, recom);
#endif
  *size = recom;
  mb_used = recom / (1024 * 1024);
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
// -KM- 1999/01/31 Use Allegro keyboard routines as install_keyboard has
//  been called.
void I_CalibrateJoystick(int ch)
{
  static int phase = 0;
  static char* message = NULL;
  static char* joy_centre = NULL;

  if (!joy_centre)
    joy_centre = DDF_LanguageLookup("JoystickCentre");

  if (graphicsmode)
  {
    if (!message)
      message = Z_Malloc(64, PU_STATIC, &message);
    else
      Z_ChangeTag(message, PU_STATIC);

     // -KM- 1998/07/31 Added escape functionality.
    if (ch == KEYD_ESCAPE)
    {
      phase = 0;
      usejoystick = false;
      Z_ChangeTag(message, PU_CACHE);
      return;
    }

    switch (phase)
    {
      case 0:
        usejoystick = false;
        remove_joystick();
        strcpy(message, joy_centre);
        phase = 1;
        break;
      case 1:
        install_joystick(JOY_TYPE_AUTODETECT);
        if (joy[0].flags & JOYFLAG_CALIBRATE)
        {
          sprintf(message, "%s\n\npress a key", calibrate_joystick_name(0));
          phase = 2;
          break;
        }
      case -1:
        usejoystick = true;
        Z_ChangeTag(message, PU_CACHE);
        phase = 0;
        return;
      case 2:
        if (!calibrate_joystick(0))
        {
          if (joy[0].flags & JOYFLAG_CALIBRATE)
            sprintf(message, "%s\n\npress a key", calibrate_joystick_name(0));
          else
          {
            strcpy(message, joy_centre);
            phase = -1;
          }
        } else
          phase = -2;
        break;
    }

    if (phase == -2)
    {
      phase = 0;
      Z_ChangeTag(message, PU_CACHE);
      return;
    }

    M_StartMessage(message, I_CalibrateJoystick, false);
    return;
  } else {
    if (load_joystick_data(NULL))
    {
      clear_keybuf();
      I_Printf(DDF_LanguageLookup("JoystickCentreT"));
      while (!keypressed() && !joy_b1 && !joy_b2) poll_joystick();
      install_joystick(JOY_TYPE_AUTODETECT);
      clear_keybuf();
      while (joy_b1 || joy_b2) poll_joystick();
      I_Printf("\n");
  
      while (joy[0].flags & JOYFLAG_CALIBRATE)
      {
        I_Printf("%s and press a key", calibrate_joystick_name(0));
        while (!keypressed() && !joy_b1 && !joy_b2) poll_joystick();
        clear_keybuf();
        while (joy_b1 || joy_b2) poll_joystick();
        I_Printf("\n");
        if (calibrate_joystick(0))
        {
          I_Printf("Error calibrating joystick.\n");
          usejoystick = false;
          return;
        }
      }
    }
  }
}

//
// I_Init
//
// -ACB- 1998/07/11 Reformatted the code.
//
void I_Init (void)
{
  allegro_init();

  //init keyboard
  // -KM- 1999/01/31 Init keyboard first
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

  // -KM- 1999/01/31 Added -noirq param.  Uses Chi's original
  //  clock reader.  Means that DOSDoom will run in background
  //  windows.
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

  if (usejoystick)
    save_joystick_data(NULL);

  if (mousepresent!=-1)
    remove_mouse();

  remove_timer();

  // -KM- 1999/01/31 Close the debugfile
#ifdef DEVELOPERS
  if (debugfile)
    fclose(debugfile);
#endif

  // Throw the end text at the screen & exit - Kester
  puttext(1, 1, 80, 25, W_CacheLumpName("ENDOOM", PU_CACHE));

  //
  // -ACB- 1998/09/06 line 25, or we end up on the top of Doom II's
  //                  ENDOOM Screen. Not good. 
  //
  gotoxy(1, 25);
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

byte*   I_AllocLow(int length)
{
    byte*       mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
void I_Error (char *error, ...)
{
    va_list     argptr;

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
    
    // Message last, so it actually prints on the screen
    va_start (argptr,error);
    fprintf (stdout, "\n");
    vfprintf (stdout, error, argptr);
    fprintf (stdout, "\n");
    // -KM- 1999/01/31 Print the error to the debugfile
#ifdef DEVELOPERS
    if (debugfile)
    {
      fprintf(debugfile, "\n");
      vfprintf(debugfile, error, argptr);
      fprintf(debugfile, "\n");
    }
#endif
    va_end (argptr);
    fflush( stdout );

    // -KM- 1999/01/31 Close the debugfile
#ifdef DEVELOPERS
    if (debugfile)
      fclose(debugfile);
#endif

    exit(-1);
}

static char         msgbuf[512];
void I_Printf (char *message, ...)
{
    va_list      argptr;
    char        *string = msgbuf;
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
    else cprintf(msgbuf);
    
    va_end (argptr);
}

// -KM- 1998/10/29 Use all of screen, not just first 25 rows.
void I_PutTitle(char *title)
{
      char string[81] = { [0 ... 79] = ' ', [80] = 0 };
      char dosdoom[81];
      int centre;

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
      textattr(0x07);
      clrscr();
      textattr((BGCOLOR << 4) + FGCOLOR);
      I_Printf ("%s\n",string);
      textattr(0x07);
      window(1, 2, 80, ScreenRows());
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

col_func RDC8_C_Lo   ={"LoRes",      R_DrawColumn8_LowRes,   false, NULL};
col_func RDC8_C_KM   ={"KM",         R_DrawColumn8_KM,       false, &RDC8_C_Lo};
col_func RDC8_C_BLF  ={"BLF",        R_DrawColumn8_BLF,      false, &RDC8_C_KM};
col_func RDC8_C_MIP  ={"MIP",        R_DrawColumn8_MIP,      false, &RDC8_C_BLF};
col_func RDC8_id2    ={"id2",        R_DrawColumn8_id_Erik,  false, &RDC8_C_MIP};
col_func RDC8_id     ={"id",         R_DrawColumn8_id,       false, &RDC8_id2};
col_func RDC8_Pentium={"Pentium",    R_DrawColumn8_Pentium,  false, &RDC8_id};
col_func RDC8_K6     ={"K6",         R_DrawColumn8_K6_MMX,   true,  &RDC8_Pentium};
col_func RDC8_Old    ={"Old",        R_DrawColumn8_NOMMX,    false, &RDC8_K6};
col_func RDC8_Rasem  ={"Rasem",      R_DrawColumn8_Rasem,    false, &RDC8_Old};
col_func RDC8_C      ={"C",          R_DrawColumn8_CVersion, false, &RDC8_Rasem};
col_func *RDC8_Head  = &RDC8_C;

col_func RDC16_C_KM  ={"KM",         R_DrawColumn16_KM,      false, NULL};
col_func RDC16_C_BLF ={"BLF",        R_DrawColumn16_BLF,     false, &RDC16_C_KM};
col_func RDC16_Old   ={"Old",        R_DrawColumn16_Old,     false, &RDC16_C_BLF};
col_func RDC16_Rasem ={"Rasem",      R_DrawColumn16_Rasem,   false, &RDC16_Old};
col_func RDC16_C     ={"C",          R_DrawColumn16_CVersion,false, &RDC16_Rasem};
col_func *RDC16_Head = &RDC16_C;

span_func RDS8_C_Lo   ={"LoRes",      R_DrawSpan8_LowRes,     false, NULL};
span_func RDS8_C_KM   ={"KM",         R_DrawSpan8_KM,         false, &RDS8_C_Lo};
span_func RDS8_C_BLF  ={"BLF",        R_DrawSpan8_BLF,        false, &RDS8_C_KM};
span_func RDS8_C_MIP  ={"MIP",        R_DrawSpan8_MIP,        false, &RDS8_C_BLF};
span_func RDS8_id2    ={"id2",        R_DrawSpan8_id_Erik,    false, &RDS8_C_MIP};
span_func RDS8_id     ={"id",         R_DrawSpan8_id,         false, &RDS8_id2};
span_func RDS8_MMX    ={"MMX",        R_DrawSpan8_MMX,        true,  &RDS8_id};
span_func RDS8_Rasem  ={"Rasem",      R_DrawSpan8_Rasem,      false, &RDS8_MMX};
span_func RDS8_C      ={"C",          R_DrawSpan8_CVersion,   false, &RDS8_Rasem};
span_func *RDS8_Head = &RDS8_C;

span_func RDS16_C_KM  ={"KM",         R_DrawSpan16_KM,        false, NULL};
span_func RDS16_C_BLF ={"BLF",        R_DrawSpan16_BLF,       false, &RDS16_C_KM};
span_func RDS16_Rasem ={"Rasem",      R_DrawSpan16_Rasem,     false, &RDS16_C_BLF};
span_func RDS16_C     ={"C",          R_DrawSpan16_CVersion,  false, &RDS16_Rasem};
span_func *RDS16_Head = &RDS16_C;

// Initialised after deciding colour depth
col_func  *RDC_Head = NULL;
span_func *RDS_Head = NULL;

// CPU-specific optimisation
cpumodel_t cpu_c    = {"Default CPU",   &RDC8_C,       &RDC16_C,     &RDS8_C,     &RDS16_C};
cpumodel_t cpu_386  = {"386",           &RDC8_id,      &RDC16_Rasem, &RDS8_id,    &RDS16_Rasem};
cpumodel_t cpu_486  = {"486",           &RDC8_id2,     &RDC16_Rasem, &RDS8_id2,   &RDS16_Rasem};
cpumodel_t cpu_Pentium = {"Pentium",    &RDC8_Pentium, &RDC16_Rasem, &RDS8_Rasem, &RDS16_Rasem};
cpumodel_t cpu_6x86 = {"6x86",          &RDC8_Rasem,   &RDC16_Rasem, &RDS8_Rasem, &RDS16_Rasem};
cpumodel_t cpu_PMMX = {"Pentium MMX",   &RDC8_Pentium, &RDC16_Rasem, &RDS8_MMX,   &RDS16_Rasem};
cpumodel_t cpu_6x86MX = {"6x86MX",      &RDC8_Rasem,   &RDC16_Rasem, &RDS8_MMX,   &RDS16_Rasem};
cpumodel_t cpu_K6   = {"K6",            &RDC8_K6,      &RDC16_Rasem, &RDS8_MMX,   &RDS16_Rasem};
cpumodel_t cpu_K6_2 = {"K6-2 or K6-3",  &RDC8_K6,      &RDC16_Rasem, &RDS8_MMX,   &RDS16_Rasem};
cpumodel_t cpu_PPro = {"Pentium Pro",   &RDC8_Rasem,   &RDC16_Rasem, &RDS8_Rasem, &RDS16_Rasem};
cpumodel_t cpu_PII  = {"Pentium II",    &RDC8_K6,      &RDC16_Rasem, &RDS8_MMX,   &RDS16_Rasem};
cpumodel_t cpu_newmmx={"Unknown MMX CPU",&RDC8_K6,     &RDC16_Rasem, &RDS8_MMX,   &RDS16_Rasem};
cpumodel_t cpu_new  = {"Unknown new CPU",&RDC8_Rasem,  &RDC16_Rasem, &RDS8_Rasem, &RDS16_Rasem};

void I_CheckCPU()
{
 cpu.model = &cpu_c; // Default: C versions of everything
 if (!M_CheckParm("-UseC"))
 {
   check_cpu();

   cpu.fpu   = cpu_fpu;
   // -KM- 1998/11/25 Allegro can handle mmx now, don't need to disable.
   cpu.mmx = cpu_mmx;

   switch (cpu_family)
   {
    case 3: // 386 detected
       cpu.model = &cpu_386;
       break;
    case 4: // 486 or 5x86 detected
       cpu.model = &cpu_486;
       break;
    case 5: // Pentium-compatible detected
       switch (cpu_model)
       {
        case 4: // Pentium MMX detected
          cpu.model = &cpu_PMMX;
          break;
        case 6 ... 7: // AMD-K6 detected
          cpu.model = &cpu_K6;
          break;
        case 8 ... 9: // AMD-K6-2 or K6-3 detected.
          cpu.model = &cpu_K6_2;
          break;
        case 14: // Cyrix detected
          if (cpu_mmx) // Check for MMX
            cpu.model = &cpu_6x86MX;
          else
            cpu.model = &cpu_6x86;
          break;
        default:
          cpu.model = &cpu_Pentium; // Pentium as default
       }
       break;
    case 6:
       if (cpu_mmx)
          cpu.model = &cpu_PII;
       else
          cpu.model = &cpu_PPro;
       break;
    default:
       if (cpu_family>6)
       {
         if (cpu_mmx)
           cpu.model = &cpu_newmmx;
         else
           cpu.model = &cpu_new;
       }

   }

   I_Printf ("CPU Detected: %s ",cpu.model->name);
  
   if (cpu.mmx || cpu.fpu || cpu_3dnow)
   {
     I_Printf("with ");
  
     if (cpu.mmx)
       I_Printf("MMX,");
     if (cpu_3dnow)
       I_Printf("3dNow!,");
     if (cpu.fpu)
       I_Printf("FPU");
  
     I_Printf(" Present");
   }
  
   if (cpu_cpuid)
     I_Printf(" - CPUID: %s",cpu_vendor);
  
   I_Printf("\n");
 }
}



