//
// I_Input.c
//
// Input code for SVGALib & LibKB version of LinuxDoom

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include "vgamouse.h"
#include "kb.h"

#include "i_system.h"
#include "i_input.h"
#include "doomdef.h"
#include "d_main.h"
#include "doomstat.h"
#include "macros.h"

// Input devices info from m_misc.c
#ifdef LINUX
extern char* mousetype;
extern char* mousedev;
#endif
extern int usemouse;

#ifndef DOSDOOM
#define KEYD_PGUP KEYD_PAGEUP
#define KEYD_PGDN KEYD_PAGEDOWN
#endif

unsigned char* sctdc;

static const char keyb_num_row[]="1234567890";
static const char keyb_top_row[]="qwertyuiop";
static const char keyb_mid_row[]="asdfghjkl";
static const char keyb_bot_row[]="zxcvbnm";

static unsigned char kbdbuf[128];

void I_InitKeyTrans() 
{
  register int c;
  //printf("    Building key translation table\n");
  sctdc=(unsigned char*)calloc(256, sizeof(unsigned char));
  // Hopefully patch up the mouse, so no translation
  // Ouch, out of range, must handle as special case
  //sctdc[KEYD_MOUSE1]=KEYD_MOUSE1;
  //sctdc[KEYD_MOUSE2]=KEYD_MOUSE2;
  //sctdc[KEYD_MOUSE3]=KEYD_MOUSE3;
  // Some thrilling stuff here
  // ASCII translation for normal keys
  for (c=0; c<sizeof(keyb_num_row); c++) sctdc[KB_SCAN_1+c]=keyb_num_row[c];
  for (c=0; c<sizeof(keyb_top_row); c++) sctdc[KB_SCAN_Q+c]=keyb_top_row[c];
  for (c=0; c<sizeof(keyb_mid_row); c++) sctdc[KB_SCAN_A+c]=keyb_mid_row[c];
  for (c=0; c<sizeof(keyb_bot_row); c++) sctdc[KB_SCAN_Z+c]=keyb_bot_row[c];
  // Keys with special doom codes
  sctdc[KB_SCAN_RCONTROL]=KEYD_RCTRL;
  sctdc[KB_SCAN_RALT]=KEYD_RALT;
  sctdc[KB_SCAN_PGUP]=sctdc[KB_SCAN_9_PAD]=KEYD_PGUP;
  sctdc[KB_SCAN_PGDN]=sctdc[KB_SCAN_3_PAD]=KEYD_PGDN;
  sctdc[KB_SCAN_RSHIFT]=KEYD_RSHIFT;
  sctdc[KB_SCAN_UP]=sctdc[KB_SCAN_8_PAD]=KEYD_UPARROW;
  sctdc[KB_SCAN_LEFT]=sctdc[KB_SCAN_4_PAD]=KEYD_LEFTARROW;
  sctdc[KB_SCAN_RIGHT]=sctdc[KB_SCAN_6_PAD]=KEYD_RIGHTARROW;
  sctdc[KB_SCAN_DOWN]=sctdc[KB_SCAN_2_PAD]=KEYD_DOWNARROW;
  sctdc[KB_SCAN_BACKSPACE]=KEYD_BACKSPACE;
  sctdc[KB_SCAN_SPACE]=' ';
  sctdc[KB_SCAN_ESC]=KEYD_ESCAPE;
  sctdc[KB_SCAN_ENTER]=sctdc[KB_SCAN_ENTER_PAD]=KEYD_ENTER;
  sctdc[KB_SCAN_TAB]='\t';
  sctdc[KB_SCAN_LCONTROL]=KEYD_RCTRL; // No LCTRL defined in DOOMDEF ?
  sctdc[KB_SCAN_ALT]=KEYD_LALT;
  sctdc[KB_SCAN_LSHIFT]=KEYD_RSHIFT; // No RSHIFT ditto
  sctdc[KB_SCAN_F1]=KEYD_F1;
  sctdc[KB_SCAN_F2]=KEYD_F2;
  sctdc[KB_SCAN_F3]=KEYD_F3;
  sctdc[KB_SCAN_F4]=KEYD_F4;
  sctdc[KB_SCAN_F5]=KEYD_F5;
  sctdc[KB_SCAN_F6]=KEYD_F6;
  sctdc[KB_SCAN_F7]=KEYD_F7;
  sctdc[KB_SCAN_F8]=KEYD_F8;
  sctdc[KB_SCAN_F9]=KEYD_F9;
  sctdc[KB_SCAN_F10]=KEYD_F10;
  sctdc[KB_SCAN_F11]=KEYD_F11;
  sctdc[KB_SCAN_F12]=KEYD_F12;
  sctdc[KB_SCAN_HOME]=sctdc[KB_SCAN_7_PAD]=KEYD_HOME;
  sctdc[KB_SCAN_END]=sctdc[KB_SCAN_1_PAD]=KEYD_END;
  sctdc[KB_SCAN_MINUS]=sctdc[KB_SCAN_MINUS_PAD]='-';
  // Problems with +/=
  sctdc[KB_SCAN_EQUAL]='=';
  sctdc[KB_SCAN_PLUS_PAD]='+';
  sctdc[KB_SCAN_OPENBRACE]='[';
  sctdc[KB_SCAN_CLOSEBRACE]=']';
  sctdc[KB_SCAN_COMMA]=',';
  sctdc[KB_SCAN_MULTIPLY_PAD]='*';
  //sctdc[KB_SCAN_COLON]=':'; // This confuses the config screen badly
  sctdc[KB_SCAN_PERIOD]=sctdc[KB_SCAN_PERIOD_PAD]='.';
  sctdc[KB_SCAN_SLASH]=sctdc[KB_SCAN_DIVIDE_PAD]='/';
  sctdc[KB_SCAN_BACKSLASH]='\\';
  for (c=0; c<128; c++) kbdbuf[c]=0;
}

unsigned short int I_ScanCode2DoomCode (unsigned short int a)
{
#ifdef RANGECHECK
  if (sctdc==NULL) I_Error("ScanCode2DoomCode: No translation\n");
#endif
 // fprintf(stderr, "%d ", (int)sctdc[a]);
 if (a>=256) return a; // Probably (hopefully) a mouse code
 return sctdc[a];
}

unsigned short int I_DoomCode2ScanCode (unsigned short int a)
{
  unsigned int b;
  if (a>=256) return a; // Probably (hopefully) mouse code
  for (b=0; b<255; b++) if (sctdc[b]==a) return b;
  return 0; // Ideas?
       	 // What the hell uses this anyway?
       	 // OK, so m_misc uses it. Great.
}

#define NEW_KB_HANDLER
//define TIMEOUT
#ifdef TIMEOUT
static unsigned int counter=0;
#endif

#define mousecentre 100
static int /*mousex, mousey,*/ mouseb;

#ifdef GPROF
static unsigned long int counter=0;
#endif

void I_StartTic()
{
  int x,y;
  char sc, pr;
  event_t ev;
  register int c;
#ifdef GPROF
  // Was for : For when I'm really paranoid about keyboard failure
  if (counter++==65000) {
    I_Error("Safety cutout");
  }
  // Now its for profiling // Well it would be if it worked
#endif
#ifndef GPROF
  kb_update();
  while ((c=kb_keypress())!=0) {
    sc=c & 0xff;
    pr=kb_key(sc);
    ev.data1=I_ScanCode2DoomCode(sc);
    ev.type=(pr) ? ev_keydown : ev_keyup;
    D_PostEvent(&ev);
    kbdbuf[c]=pr;
  }
  for (c=0; c<128; c++) {
    if (kbdbuf[c]!=kb_key(c)) {
      ev.data1=I_ScanCode2DoomCode(c); 
      ev.type=(kb_key(c)) ? ev_keydown : ev_keyup;
      D_PostEvent(&ev);
      kbdbuf[c]=kb_key(c);
    }
  }
  if (!usemouse) return;
  mouse_update();
  x=mouse_getx(); y=mouse_gety(); c=mouse_getbutton();
  mouse_setposition(mousecentre, mousecentre);
  if ((x!=mousecentre) || (y!=mousecentre)
#ifndef DOSDOOM
      || (c^mouseb!=0)
#endif
          ) {
    ev.type=ev_mouse;
    // Why does it need this stuff?
    ev.data1=(c & MOUSE_LEFTBUTTON) ? 1 : 0;
    ev.data1+=(c & MOUSE_MIDDLEBUTTON) ? 2 : 0;
    ev.data1+=(c & MOUSE_RIGHTBUTTON) ? 4 : 0;
    // Amount moved
    ev.data2=x-mousecentre;
#ifdef DOSDOOM 
    ev.data3=-((novert) ? 0 : y-mousecentre); // Minus sign: because it works
#else
    ev.data3=mousecentre-y;
#endif
    ev.data2<<=2; ev.data3<<=2; // Seems OK
    D_PostEvent(&ev);
  }
#ifdef DOSDOOM
// It seems BOOM doesn't need this
  if (c!=mouseb) {
    if ((c & MOUSE_LEFTBUTTON) != (mouseb & MOUSE_LEFTBUTTON)) {
      ev.data1=KEYD_MOUSE1;
      ev.type=(c & MOUSE_LEFTBUTTON) ? ev_keydown : ev_keyup;
      D_PostEvent(&ev);
    }
    if ((c & MOUSE_MIDDLEBUTTON) != (mouseb & MOUSE_MIDDLEBUTTON)) {
      ev.data1=KEYD_MOUSE2; 
      ev.type=(c & MOUSE_MIDDLEBUTTON) ? ev_keydown : ev_keyup;
      D_PostEvent(&ev);
    }
    if ((c & MOUSE_RIGHTBUTTON) != (mouseb & MOUSE_RIGHTBUTTON)) {
      ev.data1=KEYD_MOUSE3; 
      ev.type=(c & MOUSE_RIGHTBUTTON) ? ev_keydown : ev_keyup;
      D_PostEvent(&ev);
    }
//    mouseb=c;
  }
#endif
#endif
  mouseb=c; // Must do this
}

void I_InitInputs()
{
  int mtype=-1;
// Note: call this after I_InitGraphics
  printf("I_InitInputs\n");
#ifndef GPROF
  kb_install(KB_FLAG_SIGINT);
  if (usemouse) {
    if (!strcasecmp(mousetype, "microsoft")) mtype=MOUSE_MICROSOFT;
    if (!strcasecmp(mousetype, "ps2")) mtype=MOUSE_PS2;
    if (!strcasecmp(mousetype, "logitech")) mtype=MOUSE_LOGITECH;
// Too lazy to add other cases
    if (mtype==-1) {
      fprintf(stderr, "    unsupported mouse type %s\n", mousetype);
      usemouse=false;
    } else {
      if (mouse_init(mousedev, mtype, MOUSE_DEFAULTSAMPLERATE)==-1) {
        fprintf(stderr, "    unable to open mouse on %s\n", mousedev);
        usemouse=false;
      } else {
        mouse_setposition(mousecentre, mousecentre);
        mouseb=0;
        printf("    opened mouse %s as %s\n", mousedev, mousetype);
      }
    }
  }
#endif
}

void I_DestroyInputs()
{
#ifdef GPROF
  kb_remove(); // Would be done automagically by signals, but I must override this :-(
  if (usemouse) mouse_close();
#endif
  free(sctdc); sctdc=NULL;
}
