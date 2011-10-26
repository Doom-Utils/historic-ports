/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: l_main.c,v 1.14 2000/03/16 13:27:29 cph Exp $
 *
 *  Hybrid of the Boom i_main.c and original linuxdoom i_main.c
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
 *      Startup and quit functions. Handles signals, inits the 
 *      memory management, then calls D_DoomMain. Also contains 
 *      I_Init which does other system-related startup stuff.
 *
 *-----------------------------------------------------------------------------
 */

static const char
rcsid[] = "$Id: l_main.c,v 1.14 2000/03/16 13:27:29 cph Exp $";

#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "lprintf.h"
#include "m_random.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_sound.h"
#include "i_main.h"
#include "l_sdl.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int broken_pipe;

/* Most of the following has been rewritten by Lee Killough
 *
 * I_GetTime
 * killough 4/13/98: Make clock rate adjustable by scale factor
 * cphipps - much made static
 */

int realtic_clock_rate = 100;
static int_64_t I_GetTime_Scale = 1<<24;

static int I_GetTime_Scaled(void)
{
  return (int_64_t) I_GetTime_RealTime() * I_GetTime_Scale >> 24;
}

static int  I_GetTime_FastDemo(void)
{
  static int fasttic;
  return fasttic++;
}

static int I_GetTime_Error(void)
{
  I_Error("Error: GetTime() used before initialization");
  return 0;
}

int (*I_GetTime)(void) = I_GetTime_Error;

void I_Init(void)
{
  /* killough 4/14/98: Adjustable speedup based on realtic_clock_rate */
  if (fastdemo)
    I_GetTime = I_GetTime_FastDemo;
  else
    if (realtic_clock_rate != 100)
      {
        I_GetTime_Scale = ((int_64_t) realtic_clock_rate << 24) / 100;
        I_GetTime = I_GetTime_Scaled;
      }
    else
      I_GetTime = I_GetTime_RealTime;

  /* Jess 3/00: hack to allow lsdldoom to shutdown cleanly */ 
  I_InitSDL();

  { 
    /* killough 2/21/98: avoid sound initialization if no sound & no music */
    extern boolean nomusicparm, nosfxparm;
    if (!(nomusicparm && nosfxparm))
      I_InitSound();
  }
}

/* cleanup handling -- killough:
 */
static void I_SignalHandler(int s)
{
  char buf[2048];

#ifdef SIGPIPE
  /* CPhipps - report but don't crash on SIGPIPE */
  if (s == SIGPIPE) {
    fprintf(stderr, "Broken pipe\n");
    broken_pipe = 1;
    return;
  }
#endif
  signal(s,SIG_IGN);  /* Ignore future instances of this signal.*/

  strcpy(buf,"Exiting on signal: ");
  I_SigString(buf+strlen(buf),2000-strlen(buf),s);

  /* If corrupted memory could cause crash, dump memory
   * allocation history, which points out probable causes
   */
  if (s==SIGSEGV || s==SIGILL || s==SIGFPE)
    Z_DumpHistory(buf);

  I_Error("%s",buf);
}

/* killough 2/22/98: Add support for ENDBOOM, which is PC-specific
 *
 * this converts BIOS color codes to ANSI codes.  
 * Its not pretty, but it does the job - rain
 * CPhipps - made static
 */

static inline int convert(int color, int *bold)
{
  if (color > 7) {
    color -= 8;
    *bold = 1;
  }
  switch (color) {
  case 0:
    return 0;
  case 1:
    return 4;
  case 2:
    return 2;
  case 3:
    return 6;
  case 4:
    return 1;
  case 5:
    return 5;
  case 6:
    return 3;
  case 7:
    return 7;
  }
  return 0;
}

/* CPhipps - flags controlling ENDOOM behaviour */
enum {
  endoom_colours = 1, 
  endoom_nonasciichars = 2, 
  endoom_droplastline = 4
};

unsigned int endoom_mode;

static void PrintVer(void)
{
  printf("LsdlDoom v%s (http://jesshaas.com/lsdldoom/)\n",VERSION);
}

/* I_EndDoom
 * Prints out ENDOOM or ENDBOOM, using some common sense to decide which.
 * cphipps - moved to l_main.c, made static
 */
static void I_EndDoom(void)
{
  int lump_eb, lump_ed, lump = -1;

  /* CPhipps - ENDOOM/ENDBOOM selection */
  lump_eb = W_CheckNumForName("ENDBOOM");/* jff 4/1/98 sign our work    */
  lump_ed = W_CheckNumForName("ENDOOM"); /* CPhipps - also maybe ENDOOM */

  if (lump_eb == -1) 
    lump = lump_ed;
  else if (lump_ed == -1) 
    lump = lump_eb;
  else { /* Both ENDOOM and ENDBOOM are present */
#define LUMP_IS_NEW(num) (!((lumpinfo[num].source == source_iwad) || (lumpinfo[num].source == source_auto_load)))
    switch ((LUMP_IS_NEW(lump_ed) ? 1 : 0 ) | 
	    (LUMP_IS_NEW(lump_eb) ? 2 : 0)) {
    case 1:
      lump = lump_ed;
      break;
    case 2:
      lump = lump_eb;
      break;
    default:
      /* Both lumps have equal priority, both present */
      lump = (P_Random(pr_misc) & 1) ? lump_ed : lump_eb;
      break;
    }
  }

  if (lump != -1)
    {
      const char (*endoom)[2] = (void*)W_CacheLumpNum(lump);
      int i, l = W_LumpLength(lump) / 2;

      /* cph - colour ENDOOM by rain */
      int oldbg = 0, oldcolor = 7, bold = 0, oldbold = 0, color = 0;
      if (endoom_mode & endoom_nonasciichars)
	/* switch to secondary charset, and set to cp437 (IBM charset) */
	printf("\e)K\016");

      /* cph - optionally drop the last line, so everything fits on one screen */
      if (endoom_mode & endoom_droplastline)
	l -= 80;
      putchar('\n');
      for (i=0; i<l; i++)
        {
#ifdef DJGPP
	  textattr(endoom[i][1]);
#else
	  if (endoom_mode & endoom_colours) {
	    if (!(i % 80)) {
	      /* reset everything when we start a new line */
	      oldbg = 0;
	      oldcolor = 7;
	      printf("\e[0m\n");
	    }
	    /* foreground color */
	    bold = 0;
	    color = endoom[i][1] % 16;
	    if (color != oldcolor) {
	      oldcolor = color;
	      color = convert(color, &bold);
	      if (oldbold != bold) {
		oldbold = bold;
		oldbg = 0;
	      }
	      /* we buffer everything or output is horrendously slow */
	      printf("\e[%d;%dm", bold, color + 30);
	      bold = 0;
	    }
	    /* background color */
	    color = endoom[i][1] / 16; 
	    if (color != oldbg) {
	      oldbg = color;
	      color = convert(color, &bold);
	      printf("\e[%dm", color + 40);
	    }
	  }
	  /* cph - portable ascii printout if requested */
	  if (isascii(endoom[i][0]) || (endoom_mode & endoom_nonasciichars))
	    putchar(endoom[i][0]);
	  else /* Probably a box character, so do #'s */
	    putchar('#');
#endif
        }
      putchar('\b');   /* hack workaround for extra newline at bottom of screen */
      putchar('\r');
      if (endoom_mode & endoom_nonasciichars)
	putchar('\017'); /* restore primary charset */
      W_UnlockLumpNum(lump);
    }
  if (endoom_mode & endoom_colours)
    puts("\e[0m"); /* cph - reset colours */
  PrintVer();
}

static int has_exited;

/* I_SafeExit
 * This function is called instead of exit() by functions that might be called 
 * during the exit process (i.e. after exit() has already been called)
 * Prevent infinitely recursive exits -- killough
 */

void I_SafeExit(int rc)
{
  if (!has_exited)    /* If it hasn't exited yet, exit now -- killough */
    {
      has_exited=rc ? 2 : 1;   
      exit(rc);
    }
}

void I_Quit (void)
{
  if (!has_exited)
    has_exited=1;   /* Prevent infinitely recursive exits -- killough */

  if (has_exited == 1) {
    I_EndDoom();
    if (demorecording)
      G_CheckDemoStatus();
    M_SaveDefaults ();
  }
}

#ifdef SECURE_UID
uid_t stored_euid = -1;
#endif

int main(int argc, char **argv)
{
#ifdef SECURE_UID
  /* First thing, revoke setuid status (if any) */
  stored_euid = geteuid();
  if (getuid() != stored_euid)
  {
    if (seteuid(getuid()) < 0) 
      fprintf(stderr, "Failed to revoke setuid\n");
    else
      fprintf(stderr, "Revoked uid %d\n",stored_euid);
  }
#endif
  /* Version info */
  putchar('\n');
  PrintVer();

  myargc = argc;
  myargv = (const char* const *)argv;

  /*
     killough 1/98:

     This fixes some problems with exit handling
     during abnormal situations.

     The old code called I_Quit() to end program,
     while now I_Quit() is installed as an exit
     handler and exit() is called to exit, either
     normally or abnormally. Seg faults are caught
     and the error handler is used, to prevent
     being left in graphics mode or having very
     loud SFX noise because the sound card is
     left in an unstable state.
  */

  Z_Init();                  /* 1/18/98 killough: start up memory stuff first */

  atexit(I_Quit);
  signal(SIGSEGV, I_SignalHandler);
#ifdef SIGPIPE
  signal(SIGPIPE, I_SignalHandler); /* CPhipps - add SIGPIPE, as this is fatal */
#endif
  signal(SIGTERM, I_SignalHandler);
  signal(SIGILL,  I_SignalHandler);
  signal(SIGFPE,  I_SignalHandler);
  signal(SIGILL,  I_SignalHandler);
  signal(SIGINT,  I_SignalHandler);  /* killough 3/6/98: allow CTRL-BRK during init */
  signal(SIGABRT, I_SignalHandler);

  /* cphipps - call to video specific startup code */
  I_PreInitGraphics();

  /* 2/2/98 Stan
   * Must call this here.  It's required by both netgames and i_video.c.
   */

  D_DoomMain ();
  return 0;
}


/*----------------------------------------------------------------------------
 *
 * $Log: l_main.c,v $
 * Revision 1.14  2000/03/16 13:27:29  cph
 * Clean up uid stuff
 *
 * Revision 1.13  2000/01/25 21:33:22  cphipps
 * Fix security in case of being setuid
 *
 * Revision 1.12  1999/11/01 00:20:11  cphipps
 * Change int64_t's to int_64_t's
 * (the latter being LxDoom's wrapper for 64 bit int type)
 *
 * Revision 1.11  1999/10/31 16:07:38  cphipps
 * Moved most system functions that are specifically for the doom game from
 * l_system.c to here.
 * Cahnged all C++ comments to C.
 * Use new functions in l_system.c to get the LxDoom ver and signal names.
 * New function I_SafeExit is a wrapper for exit() that checks has_exited.
 *
 * Revision 1.10  1999/10/12 13:01:11  cphipps
 * Changed header to GPL
 *
 * Revision 1.9  1999/07/03 13:15:07  cphipps
 * Add broken_pipe variable to allow for broken pipe checking
 *
 * Revision 1.8  1999/06/20 14:04:13  cphipps
 * Code cleaning
 *
 * Revision 1.7  1999/01/11 16:03:37  cphipps
 * Fix version string printout
 *
 * Revision 1.6  1998/11/17 16:40:06  cphipps
 * Modified to work for DosDoom and LxDoom
 *
 * Revision 1.5  1998/10/16 22:20:50  cphipps
 * Match argv to myargv in type const char* const *
 * Disable dodgy BOOMPATH hack to fix D_DoomExeDir remotely, since it writes argv[0]
 *
 * Revision 1.4  1998/10/15 20:13:02  cphipps
 * Made SIGPIPE non-fatal
 *
 * Revision 1.3  1998/10/13 11:52:29  cphipps
 * Added i_video.h and I_PreInitGraphics call
 *
 * Revision 1.2  1998/09/23 09:34:53  cphipps
 * Added identifying string at startup.
 * Add code to patch up myargv[0].
 * Cleaned up exit handling
 * Removed allegro_init call
 *
 * Revision 1.1  1998/09/13 16:49:50  cphipps
 * Initial revision
 *
 * Revision 1.8  1998/05/15  00:34:03  killough
 * Remove unnecessary crash hack
 *
 * Revision 1.7  1998/05/13  22:58:04  killough
 * Restore Doom bug compatibility for demos
 *
 * Revision 1.6  1998/05/03  22:38:36  killough
 * beautification
 *
 * Revision 1.5  1998/04/27  02:03:11  killough
 * Improve signal handling, to use Z_DumpHistory()
 *
 * Revision 1.4  1998/03/09  07:10:47  killough
 * Allow CTRL-BRK during game init
 *
 * Revision 1.3  1998/02/03  01:32:58  stan
 * Moved __djgpp_nearptr_enable() call from I_video.c to i_main.c
 *
 * Revision 1.2  1998/01/26  19:23:24  phares
 * First rev with no ^Ms
 *
 * Revision 1.1.1.1  1998/01/19  14:02:57  rand
 * Lee's Jan 19 sources
 *
 *----------------------------------------------------------------------------*/
