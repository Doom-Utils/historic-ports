/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: l_joy.c,v 1.12 1999/10/31 09:12:17 cphipps Exp $
 *
 *  New module for LxDoom, a Doom port for Linux/Unix
 *  Copyright (C) 1999 by Colin Phipps
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
 *   Joystick handling for Linux
 *
 *-----------------------------------------------------------------------------
 */

#ifndef lint
static const char rcsid[] = "$Id: l_joy.c,v 1.12 1999/10/31 09:12:17 cphipps Exp $";
#endif /* lint */

#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joy.h"
#include "lprintf.h"

#undef NO_JOY_CODE

#ifndef HAVE_LINUX_JOYSTICK_H
#define NO_JOY_CODE
#else

#include <linux/joystick.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

static int joy_fd = -1;

#endif

int joyleft;
int joyright;
int joyup;
int joydown;

int usejoystick;

void I_EndJoystick(void)
{
#ifndef NO_JOY_CODE
  lprintf(LO_DEBUG, "I_EndJoystick : closing joystick\n");
  close(joy_fd);
  joy_fd = -1;
#endif
}

#ifndef NO_JOY_CODE
/* On read errors, try reopening. Patch from Benjamin McGee */
static int I_ReopenJoystick(void)
{
  char jdev[10];
  const char *fname = "I_ReopenJoystick : ";
  if (joy_fd != -1) I_EndJoystick();
  sprintf(jdev, "/dev/js%d", usejoystick-1);
  joy_fd = open(jdev, O_RDONLY);
  if (joy_fd == -1) {
    lprintf(LO_ERROR, "%serror re-opening %s\n", fname, jdev);
    return 0;
  }
  lprintf(LO_WARN, "%s%s successfully re-opened\n", fname, jdev);
  return 1;
}
#endif

#ifndef NO_JOY_CODE
static struct JS_DATA_TYPE jdata;

static int I_ReadJoystick(void)
{
  if (read(joy_fd, &jdata, JS_RETURN) != JS_RETURN){
    lprintf(LO_WARN, "I_PollJoystick : read failed\n");
    I_EndJoystick();
    return 0;
  }
  return 1;
}

static int I_WaitButton(void)
{
  do {
    usleep(10);
	if (!I_ReadJoystick() && !I_ReopenJoystick() && !I_ReadJoystick())
      return 0;
  } while (!jdata.buttons);
  while (jdata.buttons) {
    usleep(10);
    if (!I_ReadJoystick())
      return 0;
  }
  return 1;
}
#endif

void I_PollJoystick(void)
{
#ifndef NO_JOY_CODE
  if (!usejoystick || (joy_fd == -1)) return;
  if (!I_ReadJoystick()) I_ReopenJoystick();
	if (I_ReadJoystick())	 {
    event_t ev;

#ifndef DOSDOOM
    ev.type = ev_joystick;
    ev.data1 = jdata.buttons;
#else
    ev.type = ev_analogue;
    ev.data1 = ev.data3 = 0;
#endif

    ev.data2=(jdata.x < joyleft) ? -1 : ((jdata.x > joyright) ? 1 : 0);

#ifndef DOSDOOM
    ev.data3 = 
#else
    ev.data4 = 
#endif
      (jdata.y < joyup) ? -1 : ((jdata.y > joydown ) ? 1 : 0);
    D_PostEvent(&ev);

#ifdef DOSDOOM
    /* Buttons handled as keypress events */
    {
      static unsigned int old_buttons = 0;
      int button_num;

      for (button_num = 0; button_num<3; button_num++) {
	unsigned int mask = 1 << button_num;

	if ((old_buttons & mask) != (jdata.buttons & mask)) {
	  ev.type = (jdata.buttons & mask) ? ev_keydown : ev_keyup;
	  ev.data1 = KEYD_JOY1 + button_num;
	}
      }
    }  
#endif
  }
#endif
}

void I_InitJoystick(void)
{
#ifndef NO_JOY_CODE
  char jdev[10];
  const char* fname = "I_InitJoystick : ";

  if (!usejoystick) return;
  if (M_CheckParm("-nojoy") || (usejoystick>9) || (usejoystick<0)) {
    if ((usejoystick > 9) || (usejoystick < 0))
      lprintf(LO_WARN, "%sinvalid joystick %d\n", fname, usejoystick);
    else
      lprintf(LO_INFO, "%suser disabled\n", fname);
    joy_fd = -1; return;
  }
  sprintf(jdev, "/dev/js%d", usejoystick-1);
  joy_fd = open(jdev, O_RDONLY);
  if (joy_fd == -1)
    lprintf(LO_ERROR, "%serror opening %s\n", fname, jdev);
  else {
    atexit(I_EndJoystick);
    lprintf(LO_INFO, "%sopened %s\n", fname, jdev);
    if ((joyup == joydown) || (joyleft == joyright)) {
      int joyt, joyl, joyb, joyr, joycx, joycy;
      printf("Invalid joystick calibration. Calbrating now...\n");
      
      printf("Move joystick to top-left and press a button\n");
      fflush(stdin);
      if (!I_WaitButton()) return;
      joyt = jdata.y; joyl = jdata.x;
      
      printf("Move joystick to bottom right and press a button\n");
      fflush(stdin);
      if (!I_WaitButton()) return;
      joyb = jdata.y; joyr = jdata.x;
      
      printf("Move joystick to centre and press a button\n");
      fflush(stdin);
      if (!I_WaitButton()) return;
      joycy = jdata.y; joycx = jdata.x;

      joyleft = (joyl + joycx)/2; joyright = (joyr + joycx)/2;
      joyup   = (joyt + joycy)/2; joydown  = (joyb + joycy)/2;
    }
  }
#endif
}

/*
 * $Log: l_joy.c,v $
 * Revision 1.12  1999/10/31 09:12:17  cphipps
 * Fix comment typo
 *
 * Revision 1.11  1999/10/31 09:10:10  cphipps
 * Applied path from Benjamin McGee to reopen the joystick device on read
 * errors.
 * Same patch fixes a typo in the joystick calibration stuff.
 * Changed a lot of fprintf's to lprintf's.
 * Fixed last C++ comment.
 *
 * Revision 1.10  1999/10/12 13:01:11  cphipps
 * Changed header to GPL
 *
 * Revision 1.9  1999/09/10 18:08:11  cphipps
 * Modified to use autoconf generated define to decide whether joystick header is
 * available. Thanks to Josh Parsons.
 *
 * Revision 1.8  1999/01/04 19:30:00  cphipps
 * Remove duplicate joystick variable instances
 *
 * Revision 1.7  1998/12/16 22:27:12  cphipps
 * Fix warnings when NO_JOY_CODE is defined by making the static unused functions not even be defined in that case
 *
 */





