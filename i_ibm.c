
// I_IBM.C

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/nearptr.h>

//#include <graph.h>
#include "DoomDef.h"
#include "R_local.h"
#include "sounds.h"
#include "i_sound.h"
//#include "dmx.h"   <-- Ugh...the old sound libary

// Macros

#define DPMI_INT 0x31
//#define NOKBD
//#define NOTIMER

// Public Data

int DisplayTicker = 0;
int mb_used       = 8;

long col2rgb16[65][256][2];
unsigned long hicolortransmask3;
char hicolortransshift;

byte *hicolortable;
short hicolortransmask1,hicolortransmask2;


// Code

void main(int argc, char **argv)
{

        myargc = argc;
	myargv = argv;

	allegro_init();

        if (__djgpp_nearptr_enable())
           { D_DoomMain(); }
        else
           { printf("Failed try to allocate DOS near pointers!\n\nProgram aborts.\n"); }
}

void calctranslucencytable();
char *translucencytable25;
char *translucencytable50;
char *translucencytable75;



void I_StartupNet (void);
void I_ShutdownNet (void);
void I_ReadExternDriver(void);

typedef struct
{
	unsigned        edi, esi, ebp, reserved, ebx, edx, ecx, eax;
	unsigned short  flags, es, ds, fs, gs, ip, cs, sp, ss;
} dpmiregs_t;

extern  dpmiregs_t      dpmiregs;

void I_ReadMouse (void);
void I_InitDiskFlash (void);

extern  int     usemouse, usejoystick;

extern void **lumpcache;


/*
=============================================================================

							CONSTANTS

=============================================================================
*/

#define SC_INDEX                0x3C4
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_H_TOTAL    0
#define CRTC_H_DISPEND  1
#define CRTC_H_BLANK    2
#define CRTC_H_ENDBLANK 3
#define CRTC_H_RETRACE  4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL    6
#define CRTC_OVERFLOW   7
#define CRTC_ROWSCAN    8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND  11
#define CRTC_STARTHIGH  12
#define CRTC_STARTLOW   13
#define CRTC_CURSORHIGH 14
#define CRTC_CURSORLOW  15
#define CRTC_V_RETRACE  16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND  18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE  20
#define CRTC_V_BLANK    21
#define CRTC_V_ENDBLANK 22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE 24


#define GC_INDEX                0x3CE
#define GC_SETRESET             0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE 2
#define GC_DATAROTATE   3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN    17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN              19
#define ATR_COLORSELECT 20

#define STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA                0x3c9
#define PEL_MASK                0x3c6

boolean grmode;

//==================================================
//
// joystick vars
//
//==================================================

boolean         joystickpresent;
extern  unsigned        joystickx, joysticky;
boolean I_ReadJoystick (void);          // returns false if not connected


//==================================================

#define VBLCOUNTER              34000           // hardware tics to a frame


#define TIMERINT 8
#define KEYBOARDINT 9

#define CRTCOFF (_inbyte(STATUS_REGISTER_1)&1)
#define CLI     _disable()
#define STI     _enable()

#define _outbyte(x,y) (outp(x,y))
#define _outhword(x,y) (outpw(x,y))

#define _inbyte(x) (inp(x))
#define _inhword(x) (inpw(x))

#define MOUSEB1 1
#define MOUSEB2 2
#define MOUSEB3 4

boolean mousepresent;
//static  int tsm_ID = -1; // tsm init flag

//===============================

int             ticcount;

// REGS stuff used for int calls
//union REGS regs;
//struct SREGS segregs;

boolean novideo; // if true, stay in text mode for debugging

#define KBDQUESIZE 32
byte keyboardque[KBDQUESIZE];
int kbdtail, kbdhead;

char    oldkeystate[128];

//#define KEY_LSHIFT      0xfe

//#define KEY_INS         (0x80+0x52)
//#define KEY_DEL         (0x80+0x53)
//#define KEY_PGUP        (0x80+0x49)
//#define KEY_PGDN        (0x80+0x51)
//#define KEY_HOME        (0x80+0x47)
//#define KEY_END         (0x80+0x4f)

#define SC_RSHIFT       0x36
#define SC_LSHIFT       0x2a

byte        scantokey[128] =
					{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    KEYH_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13 ,    KEYH_RCTRL,'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	39 ,    '`',    KEY_LSHIFT,92,  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '/',    KEYH_RSHIFT,'*',
	KEYH_RALT,' ',   0  ,    KEYH_F1, KEYH_F2, KEYH_F3, KEYH_F4, KEYH_F5,   // 3
	KEYH_F6, KEYH_F7, KEYH_F8, KEYH_F9, KEYH_F10,0  ,    0  , KEY_HOME,
	KEYH_UPARROW,KEY_PGUP,'-',KEYH_LEFTARROW,'5',KEYH_RIGHTARROW,'+',KEY_END, //4
	KEYH_DOWNARROW,KEY_PGDN,KEY_INSERT,KEY_DEL,0,0,             0,              KEY_F11,
	KEYH_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7
					};

//==========================================================================


int I_ScanCode2DoomCode (int a)
  {
  switch (a)
    {
    case 0x48: return KEYH_UPARROW;
    case 0x4d: return KEYH_RIGHTARROW;
    case 0x50: return KEYH_DOWNARROW;
    case 0x4b: return KEYH_LEFTARROW;
    case 0x0e: return KEYH_BACKSPACE;
    case 0x2a: return KEYH_RSHIFT;
    case KEY_ALTGR: return KEYH_RALT;
    case KEY_RCONTROL: return KEYH_RCTRL;
    }

  if (key_ascii_table[a]>8)
    return key_ascii_table[a];
  else
    return a+0x80;
  }

int I_DoomCode2ScanCode (int a)
  {
  int i;

  switch (a)
    {
    case KEYH_UPARROW: return 0x48;
    case KEYH_RIGHTARROW: return 0x4d;
    case KEYH_DOWNARROW: return 0x50;
    case KEYH_LEFTARROW: return 0x4b;
    case KEYH_BACKSPACE: return 0x0e;
    }
  if (a>=0x80)
    return (a-0x80);
  else
    {
    for (i=0;i<128;i++)
      if (key_ascii_table[i]==a)
        return i;
    return 0;
    }
  }

void I_GetEvent()
  {
  event_t event;
  int i;

  char keystate[128];
  int xmickeys,ymickeys,buttons;
  static int lastbuttons=0;

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
      if ((i==0x48)||(i==0x4d)||(i==0x50)||(i==0x4b)||(i==KEY_ALTGR)||(i==KEY_RCONTROL)||(i=KEY_PRTSCR))
        {
        event.type=ev_keydown;
        event.data1=I_ScanCode2DoomCode(i);
        D_PostEvent(&event);
        }
      else if (i==0x7b)
        {
        event.type=ev_keydown;
        event.data1=KEY_PAUSE;
        D_PostEvent(&event);
        key[0x7b]=0;break;
        event.type=ev_keyup;
        event.data1=KEY_PAUSE;
        D_PostEvent(&event);
        }
      }
    if ((extkey==0)&&(oldextkey!=0))
      {
      if ((i==0x48)||(i==0x4d)||(i==0x50)||(i==0x4b)||(i==KEY_ALTGR)||(i==KEY_RCONTROL)||(i=KEY_PRTSCR))
        {
        event.type=ev_keyup;
        event.data1=I_ScanCode2DoomCode(i);
        D_PostEvent(&event);
        }
      }
    }
  memcpy(oldkeystate,keystate,128);


  //mouse movement
  if ((mousepresent!=-1)&&(usemouse))
    {
    get_mouse_mickeys(&xmickeys,&ymickeys);
    buttons=mouse_b;

    event.type=ev_mouse;
    event.data1=buttons;
    event.data2=xmickeys;
//    if (novert==0)
      event.data3= -ymickeys;
//    else
//      event.data3=0;
    if ((xmickeys!=0)||(ymickeys!=0)||(buttons!=lastbuttons))
       { D_PostEvent(&event); }
    lastbuttons=buttons;
    }



/*  //joystick
  if (usejoystick)
    {
    event.type=ev_joystick;
    event.data1=0;
    if (joy_b1) event.data1|=1;
    if (joy_b2) event.data1|=2;
    if (joy_b3) event.data1|=4;
    if (joy_b4) event.data1|=8;
    event.data2=joy_x;
    event.data3=joy_y;
    event.data2=(abs(event.data2)<4)?0:event.data2;
    event.data3=(abs(event.data3)<4)?0:event.data3;
    D_PostEvent(&event);
    }
*/


  }









//--------------------------------------------------------------------------
//
// FUNC I_GetTime
//
// Returns time in 1/35th second tics.
//
//--------------------------------------------------------------------------

volatile int mselapsed=0;

int I_GetTime (void)
{

if (mselapsed > 0)
   { return (mselapsed * TICRATE / 1000); }
else
   {
   struct timeval       tp;
   struct timezone      tzp;
   int                  newtics;
   static int           basetime=0;

   gettimeofday(&tp, &tzp);

   if (!basetime)
      { basetime = tp.tv_sec; }

   newtics = (tp.tv_sec-basetime) *TICRATE + tp.tv_usec*TICRATE/ 1000000;
   return newtics;
   }
}

//--------------------------------------------------------------------------
//
// PROC I_ColorBorder
//
//--------------------------------------------------------------------------

void I_ColorBorder(void)
{
	int i;

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 3; i++)
	{
		_outbyte(PEL_DATA, 63);
	}
}

//--------------------------------------------------------------------------
//
// PROC I_UnColorBorder
//
//--------------------------------------------------------------------------

void I_UnColorBorder(void)
{
	int i;

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 3; i++)
	{
		_outbyte(PEL_DATA, 0);
	}
}

/*
============================================================================

								USER INPUT

============================================================================
*/

//--------------------------------------------------------------------------
//
// PROC I_WaitVBL
//
//--------------------------------------------------------------------------

void I_WaitVBL(int count)
{
if (mselapsed > 0)
   { rest(count*1000/70); }
}

//--------------------------------------------------------------------------
//
// PROC I_SetPalette
//
// Palette source must use 8 bit RGB elements.
//
//--------------------------------------------------------------------------

void I_SetPalette(byte *palette)
{
  int c,i,j;
  PALETTE pal;

  for (i=0;i<256;i++)
      {
       c=gammatable[usegamma][*palette++];
       pal[i].r=c>>2;
       c=gammatable[usegamma][*palette++];
       pal[i].g=c>>2;
       c=gammatable[usegamma][*palette++];
       pal[i].b=c>>2;
      }
  set_palette(pal);

}

/*
============================================================================

							GRAPHICS MODE

============================================================================
*/


/*
==============
=
= I_Update
=
==============
*/

int UpdateState;
extern int screenblocks;

void I_Update (void)
{
	int i;
	byte *dest;
	int tics;
	static int lasttic;

//
// blit screen to video
//
	if(DisplayTicker)
	{
		if(screenblocks > 9 || UpdateState&(I_FULLSCRN|I_MESSAGES))
		{
                        dest = (byte *)screens[1];
		}
		else
		{
                        dest = (byte *)screens[0];
		}
		tics = ticcount-lasttic;
		lasttic = ticcount;
		if(tics > 20)
		{
			tics = 20;
		}
		for(i = 0; i < tics; i++)
		{
			*dest = 0xff;
			dest += 2;
		}
		for(i = tics; i < 20; i++)
		{
			*dest = 0x00;
			dest += 2;
		}
	}
	if(UpdateState == I_NOUPDATE)
	{
		return;
	}
	if(UpdateState&I_FULLSCRN)
	{
                memcpy(screens[0], screen, SCREENWIDTH*SCREENHEIGHT);
		UpdateState = I_NOUPDATE; // clear out all draw types
	}
	if(UpdateState&I_FULLVIEW)
	{
		if(UpdateState&I_MESSAGES && screenblocks > 7)
		{
			for(i = 0; i <
				(viewwindowy+viewheight)*SCREENWIDTH; i += SCREENWIDTH)
			{
                                memcpy(screens[0]+i, screen+i, SCREENWIDTH);
			}
			UpdateState &= ~(I_FULLVIEW|I_MESSAGES);
		}
		else
		{
			for(i = viewwindowy*SCREENWIDTH+viewwindowx; i <
				(viewwindowy+viewheight)*SCREENWIDTH; i += SCREENWIDTH)
			{
                                memcpy(screens[0]+i, screen+i, viewwidth);
			}
			UpdateState &= ~I_FULLVIEW;
		}
	}
	if(UpdateState&I_STATBAR)
	{
                memcpy(screens[0]+SCREENWIDTH*(SCREENHEIGHT-SBARHEIGHT),
			screen+SCREENWIDTH*(SCREENHEIGHT-SBARHEIGHT),
			SCREENWIDTH*SBARHEIGHT);
		UpdateState &= ~I_STATBAR;
	}
	if(UpdateState&I_MESSAGES)
	{
                memcpy(screens[0], screen, SCREENWIDTH*28);
		UpdateState &= ~I_MESSAGES;
	}

  memcpy(screens[0], screen, SCREENHEIGHT*SCREENWIDTH);
}

//--------------------------------------------------------------------------
//
// PROC I_InitGraphics
//
//--------------------------------------------------------------------------

/* progress indicator for the translucency table calculations */
void callback_func()
  {
  static int i = 0;

  if (!(15&i++))
     { //fprintf(stderr, ".");
     }
  }


void calctranslucencytable()
  { /* This function uses Allegro routines to create a translucency table very quickly */
  int x, y;
  RGB_MAP rgb_table;
  unsigned char *thepalette;
  PALETTE pal;
  RGB c;
  int i;

  thepalette = W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
  translucencytable25=(char *)malloc(65536*3);
  translucencytable50=translucencytable25+65536;
  translucencytable75=translucencytable25+65536*2;

  for (i = 0; i < 256; i++) // The palette must first be converted to Allegro's format (RGB values 0-63)
    {
    pal[i].r = thepalette[i*3+0] >> 2;
    pal[i].g = thepalette[i*3+1] >> 2;
    pal[i].b = thepalette[i*3+2] >> 2;
    }
  printf("Calculating translucency table\n");

  /* this isn't needed, but it speeds up the color table calculations */
   create_rgb_table(&rgb_table, pal, callback_func);
   rgb_map = &rgb_table;

   /* Create the table */
   for (x=0; x<PAL_SIZE; x++)
     {
     for (y=0; y<=x/* < PAL_SIZE*/; y++)
       {
       c.r = ((int)pal[x].r + (int)pal[y].r) >> 2;
       c.g = ((int)pal[x].g + (int)pal[y].g) >> 2;
       c.b = ((int)pal[x].b + (int)pal[y].b) >> 2;
       translucencytable50[(x<<8)+y] = rgb_map->data[c.r][c.g][c.b];
       translucencytable50[(y<<8)+x] = rgb_map->data[c.r][c.g][c.b];
       }
     callback_func(x);
     }
   for (x=0; x<PAL_SIZE; x++)
     {
     for (y=0; y<PAL_SIZE; y++)
       {
       c.r = ((int)pal[x].r + ((int)pal[y].r)*3) >> 3;
       c.g = ((int)pal[x].g + ((int)pal[y].g)*3) >> 3;
       c.b = ((int)pal[x].b + ((int)pal[y].b)*3) >> 3;
       translucencytable75[(x<<8)+y]=rgb_map->data[c.r][c.g][c.b];
       translucencytable25[(y<<8)+x]=rgb_map->data[c.r][c.g][c.b];
       }
     callback_func(x);
     }

   rgb_map = NULL;
}




void I_timer(void)
{
  mselapsed+=5;
  //cdcounter+=5;
}
END_OF_FUNCTION(I_timer);


void I_InitGraphics(void)
{
	grmode = true;

        calctranslucencytable();

        enter_graphics_mode();
        I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));

        //init timer
        LOCK_VARIABLE(mselapsed);
        LOCK_FUNCTION(I_timer);
        install_timer();
        install_int(I_timer,5);

        //init keyboard
        memset(oldkeystate,0,128);
        install_keyboard();
        install_mouse();


}

//--------------------------------------------------------------------------
//
// PROC I_ShutdownGraphics
//
//--------------------------------------------------------------------------

void I_ShutdownGraphics(void)
{
// switch back to text mode here
}

//--------------------------------------------------------------------------
//
// PROC I_ReadScreen
//
// Reads the screen currently displayed into a linear buffer.
//
//--------------------------------------------------------------------------

void I_ReadScreen(byte *scr)
{
	memcpy(scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//===========================================================================

/*
===================
=
= I_StartTic
=
// called by D_DoomLoop
// called before processing each tic in a frame
// can call D_PostEvent
// asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events
===================
*/

#define SC_UPARROW              0x48
#define SC_DOWNARROW    0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW   0x4d

void   I_StartTic (void)
{
I_GetEvent();
}


void   I_ReadKeys (void)
{
	int             k;
	event_t ev;


	while (1)
	{
	   while (kbdtail < kbdhead)
	   {
		   k = keyboardque[kbdtail&(KBDQUESIZE-1)];
		   kbdtail++;
		   printf ("0x%x\n",k);
		   if (k == 1)
			   I_Quit ();
	   }
	}
}

/*
===============
=
= I_StartFrame
=
===============
*/

void I_StartFrame (void)
{
//	I_JoystickEvents ();
	I_ReadExternDriver();
}

/*
============================================================================

					TIMER INTERRUPT

============================================================================
*/

void I_ColorBlack (int r, int g, int b)
{
_outbyte (PEL_WRITE_ADR,0);
_outbyte(PEL_DATA,r);
_outbyte(PEL_DATA,g);
_outbyte(PEL_DATA,b);
}


/*
================
=
= I_TimerISR
=
================
*/

int I_TimerISR (void)
{
	ticcount++;
	return 0;
}

/*
============================================================================

						KEYBOARD

============================================================================
*/

//void (__interrupt __far *oldkeyboardisr) () = NULL;

int lastpress;

/*
================
=
= I_KeyboardISR
=
================
*/

void I_KeyboardISR (void)
{
/*// Get the scan code

//        keyboardque[kbdhead&(KBDQUESIZE-1)] = lastpress = _inbyte(0x60);
//        kbdhead++;

// acknowledge the interrupt

//        _outbyte(0x20,0x20);
*/
}



/*
===============
=
= I_StartupKeyboard
=
===============
*/

void I_StartupKeyboard (void)
{
//#ifndef NOKBD
//	oldkeyboardisr = _dos_getvect(KEYBOARDINT);
//	_dos_setvect (0x8000 | KEYBOARDINT, I_KeyboardISR);
//#endif

//install_keyboard();

//I_ReadKeys ();
}


void I_ShutdownKeyboard (void)
{
//	if (oldkeyboardisr)
//		_dos_setvect (KEYBOARDINT, oldkeyboardisr);
//	*(short *)0x41c = *(short *)0x41a;      // clear bios key buffer

remove_keyboard();
}



/*
============================================================================

							MOUSE

============================================================================
*/


int I_ResetMouse (void)
{
//	regs.w.ax = 0;                  // reset
//	int386 (0x33, &regs, &regs);
//	return regs.w.ax;
}



/*
================
=
= StartupMouse
=
================
*/

void I_StartupCyberMan(void);

void I_StartupMouse (void)
{
	mousepresent = 0;
	if ( M_CheckParm ("-nomouse") || !usemouse )
		return;

        install_mouse();

        printf ("Mouse: detected ");

	mousepresent = 1;

}


/*
================
=
= ShutdownMouse
=
================
*/

void I_ShutdownMouse (void)
{
        remove_mouse();
}


/*
================
=
= I_ReadMouse
=
================
*/

void I_ReadMouse (void)
{
  event_t event;
  int i;
  int xmickeys,ymickeys,buttons;
  static int lastbuttons=0;


  //mouse movement
  if ((mousepresent!=-1)&&(usemouse))
    {
    get_mouse_mickeys(&xmickeys,&ymickeys);
    buttons=mouse_b;

    event.type=ev_mouse;
    event.data1=buttons;
    event.data2=xmickeys;
//    if (novert==0)
      event.data3=ymickeys;
//    else
//      event.data3=0;
    if ((xmickeys!=0)||(ymickeys!=0)||(buttons!=lastbuttons))
       { D_PostEvent(&event); }
    lastbuttons=buttons;
    }
}

/*
============================================================================

					JOYSTICK

============================================================================
*/

int     joyxl, joyxh, joyyl, joyyh;

boolean WaitJoyButton (void)
{
	int             oldbuttons, buttons;

	oldbuttons = 0;
	do
	{
		I_WaitVBL (1);
		buttons =  ((inp(0x201) >> 4)&1)^1;
		if (buttons != oldbuttons)
		{
			oldbuttons = buttons;
			continue;
		}

		if ( (lastpress& 0x7f) == 1 )
		{
			joystickpresent = false;
			return false;
		}
	} while ( !buttons);

	do
	{
		I_WaitVBL (1);
		buttons =  ((inp(0x201) >> 4)&1)^1;
		if (buttons != oldbuttons)
		{
			oldbuttons = buttons;
			continue;
		}

		if ( (lastpress& 0x7f) == 1 )
		{
			joystickpresent = false;
			return false;
		}
	} while ( buttons);

	return true;
}



/*
===============
=
= I_StartupJoystick
=
===============
*/

int             basejoyx, basejoyy;

void I_StartupJoystick (void)
{
/*	int     buttons;
	int     count;
	int     centerx, centery;

	joystickpresent = 0;
	if ( M_CheckParm ("-nojoy") || !usejoystick )
		return;

	if (!I_ReadJoystick ())
	{
		joystickpresent = false;
		tprintf ("joystick not found ",0);
		return;
	}
	printf("joystick found\n");
	joystickpresent = true;

	printf("CENTER the joystick and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	centerx = joystickx;
	centery = joysticky;

	printf("\nPush the joystick to the UPPER LEFT corner and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	joyxl = (centerx + joystickx)/2;
	joyyl = (centerx + joysticky)/2;

	printf("\nPush the joystick to the LOWER RIGHT corner and press button 1:");
	if (!WaitJoyButton ())
		return;
	I_ReadJoystick ();
	joyxh = (centerx + joystickx)/2;
	joyyh = (centery + joysticky)/2;
	printf("\n");

*/
}

/*
===============
=
= I_JoystickEvents
=
===============
*/

void I_JoystickEvents (void)
{
	event_t ev;

//
// joystick events
//
/*	if (!joystickpresent)
		return;

	I_ReadJoystick ();
	ev.type = ev_joystick;
	ev.data1 =  ((inp(0x201) >> 4)&15)^15;

	if (joystickx < joyxl)
		ev.data2 = -1;
	else if (joystickx > joyxh)
		ev.data2 = 1;
	else
		ev.data2 = 0;
	if (joysticky < joyyl)
		ev.data3 = -1;
	else if (joysticky > joyyh)
		ev.data3 = 1;
	else
		ev.data3 = 0;

	D_PostEvent (&ev);
*/
}



/*
============================================================================

					DPMI STUFF

============================================================================
*/

#define REALSTACKSIZE   1024

dpmiregs_t      dpmiregs;

unsigned                realstackseg;

void I_DivException (void);
int I_SetDivException (void);

void DPMIFarCall (void)
{
/*	segread (&segregs);
	regs.w.ax = 0x301;
	regs.w.bx = 0;
	regs.w.cx = 0;
//	regs.x.edi = (unsigned)&dpmiregs;
	segregs.es = segregs.ds;
	int386x( DPMI_INT, &regs, &regs, &segregs );
*/
}


//void DPMIInt (int i)
//{
/*	dpmiregs.ss = realstackseg;
	dpmiregs.sp = REALSTACKSIZE-4;

	segread (&segregs);
	regs.w.ax = 0x300;
	regs.w.bx = i;
	regs.w.cx = 0;
	regs.x.edi = (unsigned)&dpmiregs;
	segregs.es = segregs.ds;
	int386x( DPMI_INT, &regs, &regs, &segregs );
*/
//}


/*
==============
=
= I_StartupDPMI
=
==============
*/

void I_StartupDPMI (void)
{
/*	extern char __begtext;
	extern char ___argc;
	int     n,d;

//
// allocate a decent stack for real mode ISRs
//
	realstackseg = (int)I_AllocLow (1024) >> 4;

//
// lock the entire program down
//

//      _dpmi_lockregion (&__begtext, &___argc - &__begtext);


//
// catch divide by 0 exception
//
#if 0
	segread(&segregs);
	regs.w.ax = 0x0203;             // DPMI set processor exception handler vector
	regs.w.bx = 0;                  // int 0
	regs.w.cx = segregs.cs;
	regs.x.edx = (int)&I_DivException;
 printf ("%x : %x\n",regs.w.cx, regs.x.edx);
	int386( DPMI_INT, &regs, &regs);
#endif

#if 0
	n = I_SetDivException ();
	printf ("return: %i\n",n);
	n = 100;
	d = 0;
   printf ("100 / 0 = %i\n",n/d);

exit (1);
#endif

*/
}



/*
============================================================================

					TIMER INTERRUPT

============================================================================
*/

//void (__interrupt __far *oldtimerisr) ();


void IO_ColorBlack (int r, int g, int b)
{
_outbyte (PEL_WRITE_ADR,0);
_outbyte(PEL_DATA,r);
_outbyte(PEL_DATA,g);
_outbyte(PEL_DATA,b);
}


/*
================
=
= IO_TimerISR
=
================
*/

//void __interrupt IO_TimerISR (void)

void IO_TimerISR (void)
{
	ticcount++;
	_outbyte(0x20,0x20);                            // Ack the interrupt
}

/*
=====================
=
= IO_SetTimer0
=
= Sets system timer 0 to the specified speed
=
=====================
*/

void IO_SetTimer0(int speed)
{
//        if (speed > 0 && speed < 150)
//                I_Error ("INT_SetTimer0: %i is a bad value",speed);
//
//        _outbyte(0x43,0x36);                            // Change timer 0
//        _outbyte(0x40,speed);
//        _outbyte(0x40,speed >> 8);
}



/*
===============
=
= IO_StartupTimer
=
===============
*/

void IO_StartupTimer (void)
{
//	oldtimerisr = _dos_getvect(TIMERINT);
//
//	_dos_setvect (0x8000 | TIMERINT, IO_TimerISR);
//        IO_SetTimer0 (VBLCOUNTER);
}

void IO_ShutdownTimer (void)
{
//	if (oldtimerisr)
//	{
//                IO_SetTimer0 (0);              // back to 18.4 ips
//                _dos_setvect (TIMERINT, oldtimerisr);
//	}
}

//===========================================================================


/*
===============
=
= I_Init
=
= hook interrupts and set graphics mode
=
===============
*/

void I_Init (void)
{
	extern void I_StartupTimer(void);

        I_InitSound();

        novideo = M_CheckParm("-novideo");
        printf("S_Init... \n");
        S_Init(snd_SfxVolume, snd_MusicVolume);
        //S_Start();
}


/*
===============
=
= I_Shutdown
=
= return to default system state
=
===============
*/

void I_Shutdown (void)
{
	I_ShutdownGraphics ();
	//S_ShutDown ();
        remove_mouse();
        remove_keyboard();
        remove_timer();
}



/*
================
=
= I_Error
=
================
*/

void I_Error (char *error, ...)
{
    va_list	argptr;

    // Shutdown. Here might be other errors.
//    if (demorecording)
//        G_CheckDemoStatus();

//    D_QuitNetGame ();
//    I_ShutdownSound();
//    I_ShutdownMusic();

    remove_keyboard();
    if (mousepresent!=-1)
    remove_mouse();
    remove_timer();
    set_gfx_mode(GFX_TEXT,80,25,0,0);
    
    // Message last, so i actually prints on the screen
    va_start (argptr,error);
    vfprintf (stdout,error,argptr);
    fprintf (stdout, "\n");
    va_end (argptr);
    fflush( stdout );


    exit(-1);

}

//--------------------------------------------------------------------------
//
// I_Quit
//
// Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//--------------------------------------------------------------------------

void I_Quit(void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
//    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    printf("\nHeXetic exit: clean exit\n\n");
    exit(0);
}

/*
===============
=
= I_ZoneBase
=
===============
*/

byte *I_ZoneBase (int *size)
{
  int p;

  p = M_CheckParm ("-heapsize");
  if (p && p < myargc-1)
    {
    mb_used=atoi(myargv[p+1]);
    }

  printf ("Heapsize: %d Megabytes\n",mb_used);
  *size = mb_used*1024*1024;
  return (byte *) malloc (*size);
}

/*
=============================================================================

					DISK ICON FLASHING

=============================================================================
*/

void I_InitDiskFlash (void)
{
#if 0
	void    *pic;
	byte    *temp;

	pic = W_CacheLumpName ("STDISK",PU_CACHE);
	temp = destscreen;
	destscreen = (byte *)0xac000;
	V_DrawPatchDirect (SCREENWIDTH-16,SCREENHEIGHT-16,0,pic);
	destscreen = temp;
#endif
}

// draw disk icon
void I_BeginRead (void)
{
#if 0
	byte    *src,*dest;
	int             y;

	if (!grmode)
		return;

// write through all planes
	outp (SC_INDEX,SC_MAPMASK);
	outp (SC_INDEX+1,15);
// set write mode 1
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)|1);

// copy to backup
	src = currentscreen + 184*80 + 304/4;
	dest = (byte *)0xac000 + 184*80 + 288/4;
	for (y=0 ; y<16 ; y++)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		src += 80;
		dest += 80;
	}

// copy disk over
	dest = currentscreen + 184*80 + 304/4;
	src = (byte *)0xac000 + 184*80 + 304/4;
	for (y=0 ; y<16 ; y++)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		src += 80;
		dest += 80;
	}


// set write mode 0
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)&~1);
#endif
}

// erase disk icon
void I_EndRead (void)
{
#if 0
	byte    *src,*dest;
	int             y;

	if (!grmode)
		return;

// write through all planes
	outp (SC_INDEX,SC_MAPMASK);
	outp (SC_INDEX+1,15);
// set write mode 1
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)|1);


// copy disk over
	dest = currentscreen + 184*80 + 304/4;
	src = (byte *)0xac000 + 184*80 + 288/4;
	for (y=0 ; y<16 ; y++)
	{
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		src += 80;
		dest += 80;
	}

// set write mode 0
	outp (GC_INDEX,GC_MODE);
	outp (GC_INDEX+1,inp(GC_INDEX+1)&~1);
#endif
}



/*
=============
=
= I_AllocLow
=
=============
*/

byte *I_AllocLow (int length)
{
    byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}

/*
============================================================================

						NETWORKING

============================================================================
*/

/* // FUCKED LINES
typedef struct
{
	char    priv[508];
} doomdata_t;
*/ // FUCKED LINES

#define DOOMCOM_ID              0x12345678l

/* // FUCKED LINES
typedef struct
{
	long    id;
	short   intnum;                 // DOOM executes an int to execute commands

// communication between DOOM and the driver
	short   command;                // CMD_SEND or CMD_GET
	short   remotenode;             // dest for send, set by get (-1 = no packet)
	short   datalength;             // bytes in doomdata to be sent

// info common to all nodes
	short   numnodes;               // console is allways node 0
	short   ticdup;                 // 1 = no duplication, 2-5 = dup for slow nets
	short   extratics;              // 1 = send a backup tic in every packet
	short   deathmatch;             // 1 = deathmatch
	short   savegame;               // -1 = new game, 0-5 = load savegame
	short   episode;                // 1-3
	short   map;                    // 1-9
	short   skill;                  // 1-5

// info specific to this node
	short   consoleplayer;
	short   numplayers;
	short   angleoffset;    // 1 = left, 0 = center, -1 = right
	short   drone;                  // 1 = drone

// packet data to be sent
	doomdata_t      data;
} doomcom_t;
*/ // FUCKED LINES

extern  doomcom_t               *doomcom;

/*
====================
=
= I_InitNetwork
=
====================
*/

void I_InitNetwork (void)
{
	int             i;

	i = M_CheckParm ("-net");
	if (!i)
	{
	//
	// single player game
	//
		doomcom = malloc (sizeof (*doomcom) );
		memset (doomcom, 0, sizeof(*doomcom) );
		netgame = false;
		doomcom->id = DOOMCOM_ID;
		doomcom->numplayers = doomcom->numnodes = 1;
		doomcom->deathmatch = false;
		doomcom->consoleplayer = 0;
		doomcom->ticdup = 1;
		doomcom->extratics = 0;
		return;
	}

	netgame = true;
	doomcom = (doomcom_t *)atoi(myargv[i+1]);
//DEBUG
doomcom->skill = startskill;
doomcom->episode = startepisode;
doomcom->map = startmap;
doomcom->deathmatch = deathmatch;

}

void I_NetCmd (void)
{
	if (!netgame)
		I_Error ("I_NetCmd when not in netgame");
//	DPMIInt (doomcom->intnum);
}

int i_Vector;
//externdata_t *i_ExternData;
boolean useexterndriver;

//=========================================================================
//
// I_CheckExternDriver
//
//		Checks to see if a vector, and an address for an external driver
//			have been passed.
//=========================================================================

void I_CheckExternDriver(void)
{
/*	int i;

	if(!(i = M_CheckParm("-externdriver")))
	{
		return;
	}
	i_ExternData = (externdata_t *)atoi(myargv[i+1]);
	i_Vector = i_ExternData->vector;

	useexterndriver = true;
*/
}

//=========================================================================
//
// I_ReadExternDriver
//
//		calls the external interrupt, which should then update i_ExternDriver
//=========================================================================

void I_ReadExternDriver(void)
{
	event_t ev;

	if(useexterndriver)
	{
//		DPMIInt(i_Vector);
	}
}
