//  
// DOSDoom Misc System Interface Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -MH-  1998/07/02 Move I_DoomCode2ScanCode and I_ScanCode2DoomCode
//                  to i_system.c (and their relevant declarations to
//                  i_system.h. See i_system.c for details as to why.
//
// -ACB- 1998/07/17 New CPU Checking Code
//
// -ES- 1998/08/05 Newer CPU Checking Code
//

#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include "d_event.h"
#include "d_ticcmd.h"

#ifdef __GNUG__
#pragma interface
#endif

// Called by DoomMain.
void I_Init (void);

// -MH- 1998/07/02 Moved from i_video.h
int I_ScanCode2DoomCode (int a);
int I_DoomCode2ScanCode (int a);


// Called by startup code
// to get the ammount of memory to malloc
// for the zone management.
byte*	I_ZoneBase (int *size);


// Called by D_DoomLoop,
// returns current time in tics.
int I_GetTime (void);


//
// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
void I_StartFrame (void);


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic (void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void I_Quit (void) __attribute__((noreturn));

void I_CalibrateJoystick(int ch);

// Allocates from low memory under dos,
// just mallocs under unix
byte* I_AllocLow (int length);

void I_Tactile (int on, int off, int total);

void I_Error (char *error, ...) __attribute__((noreturn, format(printf, 1, 2)));
void I_Printf (char *message, ...) __attribute__((format(printf, 1, 2)));

// Done this way so that other systems can use non dos ways.
void I_Window(int left, int top, int right, int bottom);
      
void I_TextAttr(int attr);
      
void I_ClrScr(void);

void I_PutTitle(char *title);

// -ACB- 1998/07/17 New Code for checking cpu family and fpu & mmx.
void I_CheckCPU(void);

// -ES- 1998/08/05 Added cpumodel struct
// Current names:
// C is C version
// Asm1 is Rasem's asm
// Asm2 is original ASM
// K6 is K6 version
// MMX is MMX version
// id is id's original version
// id2 is id's original version, but more optimised

// col_func:
typedef struct col_func
{
  char *name; // for cmdline overrides
  void (*func)(void);
  boolean mmx;
  struct col_func *next;
} col_func;

typedef struct span_func
{
  char *name; // for cmdline overrides
  void (*func)(void);
  boolean mmx;
  struct span_func *next;
} span_func;

typedef struct
{
 char           *name;
 col_func       *RDC8;
 col_func       *RDC16;
 span_func      *RDS8;
 span_func      *RDS16;
} cpumodel_t;

typedef struct
{
 cpumodel_t     *model; // The CPU model: contains default routines, and name
 col_func       *RDC; // The actual routines used. Default=model->RDC8/16. Can however be overridden.
 span_func      *RDS;
 boolean        fpu;
 boolean        mmx;
} cputype_t;

extern col_func  *RDC8_Head, *RDC16_Head, *RDC_Head;
extern span_func *RDS8_Head, *RDS16_Head, *RDS_Head;

extern cputype_t cpu;

#endif

