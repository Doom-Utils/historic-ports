// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-2000 by Udo Munk
// Copyright (C) 1999 by Achim Oppelt
// Copyright (C) 1999 by Oliver Kraus
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <X11/extensions/XShm.h>
// Had to dig up XShm.c for this one.
// It is in the libXext, but not in the X headers.
//#if defined(LINUX)
int XShmGetEventBase(Display *dpy);
//#endif

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include "strcmp.h"
#endif

#include "doom_icon.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "d_main.h"
#include "s_sound.h"
#include "i_sound.h"

#ifdef USE_JOYSTICK
#include "i_joy.h"
#endif

extern boolean	showkey;
extern int	use_mmap;
extern int	sndupd_flag;
static int	snd_updated;

Display		*X_display = 0;
Window		X_mainWindow;
Colormap	X_cmap;
Pixmap		X_icon;
Visual		*X_visual;
GC		X_gc;
XEvent		X_event;
int		X_screen;
XVisualInfo	X_visualinfo;
XSizeHints	X_size;
XWMHints	X_wm;
XClassHint	X_class;
Atom		X_wm_delwin;
int		X_width;
int		X_height;
XImage		*image;

static Window	dummy;
static int	dont_care;

// MIT SHared Memory extension
boolean		doShm;

XShmSegmentInfo	X_shminfo;
int		X_shmeventtype;

// Mouse handling
static boolean	grabMouse;
static boolean	Mousegrabbed = false;

// Joystick handling
extern int	usejoystick;

static event_t	event;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply = 1;

// X visual mode
static int		x_depth = 1;
static int		x_bpp = 1;
static int		x_pseudo = 1;
static unsigned short	*x_colormap2 = 0;
static unsigned char	*x_colormap3 = 0;
static unsigned long	*x_colormap4 = 0;
static unsigned long	x_red_mask = 0;
static unsigned long	x_green_mask = 0;
static unsigned long	x_blue_mask = 0;
static unsigned char	x_red_offset = 0;
static unsigned char	x_green_offset = 0;
static unsigned char	x_blue_offset = 0;

//
//  Translates the key currently in X_event
//
static int xlatekey(void)
{
    int rc;

    switch (rc = XKeycodeToKeysym(X_display, X_event.xkey.keycode, 0))
    {
      case XK_Left:
      case XK_KP_Left:	rc = KEY_LEFTARROW;	break;

      case XK_Right:
      case XK_KP_Right:	rc = KEY_RIGHTARROW;	break;

      case XK_Down:
      case XK_KP_Down:	rc = KEY_DOWNARROW;	break;

      case XK_Up:
      case XK_KP_Up:	rc = KEY_UPARROW;	break;

      case XK_Escape:	rc = KEY_ESCAPE;	break;
      case XK_Return:	rc = KEY_ENTER;		break;
      case XK_Tab:	rc = KEY_TAB;		break;
      case XK_F1:	rc = KEY_F1;		break;
      case XK_F2:	rc = KEY_F2;		break;
      case XK_F3:	rc = KEY_F3;		break;
      case XK_F4:	rc = KEY_F4;		break;
      case XK_F5:	rc = KEY_F5;		break;
      case XK_F6:	rc = KEY_F6;		break;
      case XK_F7:	rc = KEY_F7;		break;
      case XK_F8:	rc = KEY_F8;		break;
      case XK_F9:	rc = KEY_F9;		break;
      case XK_F10:	rc = KEY_F10;		break;
      case XK_F11:	rc = KEY_F11;		break;
      case XK_F12:	rc = KEY_F12;		break;

      case XK_BackSpace:
      case XK_Delete:	rc = KEY_BACKSPACE;	break;

      case XK_Pause:	rc = KEY_PAUSE;		break;

      case XK_KP_Equal:
      case XK_KP_Add:
      case XK_equal:	rc = KEY_EQUALS;	break;

      case XK_KP_Subtract:
      case XK_minus:	rc = KEY_MINUS;		break;

      case XK_Shift_L:
      case XK_Shift_R:
	rc = KEY_RSHIFT;
	break;

      case XK_Caps_Lock:
	rc = KEY_CAPSLOCK;
	break;

      case XK_Control_L:
      case XK_Control_R:
	rc = KEY_RCTRL;
	break;

      case XK_Alt_L:
      case XK_Meta_L:
      case XK_Alt_R:
      case XK_Meta_R:
	rc = KEY_RALT;
	break;

      default:
	if (rc >= XK_space && rc <= XK_asciitilde)
	    rc = rc - XK_space + ' ';
	if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';
	break;
    }

    if (showkey)
      printf("Key: %d\n", rc);

    return rc;
}

void I_ShutdownGraphics(void)
{
  // was graphics initialized anyway?
  if (!X_display)
	return;

  // Uh oh, don't do that if SHM not used!!
  if (doShm)
  {
    // Detach from X server
    if (!XShmDetach(X_display, &X_shminfo))
	    I_Error("XShmDetach() failed in I_ShutdownGraphics()");

    // Release shared memory.
    shmdt(X_shminfo.shmaddr);
    shmctl(X_shminfo.shmid, IPC_RMID, 0);
  }
}

static int	lastmousex;
static int	lastmousey;
#ifdef POLL_POINTER
static int	newmousex;
static int	newmousey;
#endif
volatile boolean shmFinished;

void I_GetEvent(void)
{
    // put event-grabbing stuff in here
    XNextEvent(X_display, &X_event);
    switch (X_event.type)
    {
      case KeyPress:
	event.type = ev_keydown;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// printf("k");
	break;

      case KeyRelease:
	event.type = ev_keyup;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// printf("ku");
	break;

#ifndef POLL_POINTER
      case ButtonPress:
	if (grabMouse)
	{
	  event.type = ev_mouse;
	  event.data1 =
	    (X_event.xbutton.state & Button1Mask ? 1 : 0)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0)
	    | (X_event.xbutton.button == Button1 ? 1 : 0)
	    | (X_event.xbutton.button == Button2 ? 2 : 0)
	    | (X_event.xbutton.button == Button3 ? 4 : 0);
	  event.data2 = event.data3 = 0;
	  D_PostEvent(&event);
	  // printf("b");
	}
	break;

      case ButtonRelease:
	if (grabMouse)
	{
	  event.type = ev_mouse;
	  event.data1 =
	    (X_event.xbutton.state & Button1Mask ? 1 : 0)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0);
	  // suggest parentheses around arithmetic in operand of |
	  event.data1 =
	    event.data1
	    ^ (X_event.xbutton.button == Button1 ? 1 : 0)
	    ^ (X_event.xbutton.button == Button2 ? 2 : 0)
	    ^ (X_event.xbutton.button == Button3 ? 4 : 0);
	  event.data2 = event.data3 = 0;
	  D_PostEvent(&event);
	  // printf("bu");
	}
	break;

      case MotionNotify:
	if (grabMouse)
	{
	  // If the event is from warping the pointer back to middle
	  // of the screen then ignore it.
	  if ((X_event.xmotion.x == X_width / 2) &&
	    (X_event.xmotion.y == X_height / 2))
	  {
	    lastmousex = X_event.xmotion.x;
	    lastmousey = X_event.xmotion.y;
	    break;
	  }
	  else
	  {
	    event.data2 = (X_event.xmotion.x - lastmousex) << 2;
	    lastmousex = X_event.xmotion.x;
	    event.data3 = (lastmousey - X_event.xmotion.y) << 2;
	    lastmousey = X_event.xmotion.y;
	  }
	  event.type = ev_mouse;
	  event.data1 =
	    (X_event.xmotion.state & Button1Mask ? 1 : 0)
	    | (X_event.xmotion.state & Button2Mask ? 2 : 0)
	    | (X_event.xmotion.state & Button3Mask ? 4 : 0);
	  D_PostEvent(&event);
	  // printf("m");
	  // Warp the pointer back to the middle of the window
	  //  or we cannot move any further if it's at a border.
	  if ((X_event.xmotion.x < 1) || (X_event.xmotion.y < 1)
	    || (X_event.xmotion.x > X_width - 2)
	    || (X_event.xmotion.y > X_height - 2))
	  {
		XWarpPointer(X_display,
			  None,
			  X_mainWindow,
			  0, 0,
			  0, 0,
			  X_width / 2, X_height / 2);
	  }
	}
	break;
#endif

      case UnmapNotify:
	S_PauseSound();
	while (1)
	{
	  XNextEvent(X_display, &X_event);
	  if (doShm && X_event.type == X_shmeventtype)
	    shmFinished = true;
	  if ((X_event.type == MapNotify))
	    break;
	}
	S_ResumeSound();
	break;

      case ClientMessage:
	if (X_event.xclient.data.l[0] == X_wm_delwin)
	    M_QuitResponse('y');
	break;

      case Expose:
      case ConfigureNotify:
	break;

      default:
	if (doShm && X_event.type == X_shmeventtype)
	    shmFinished = true;
	break;
    }

#ifdef USE_JOYSTICK
    if (usejoystick)
	joystick_events();
#endif

}

static Cursor createnullcursor(Display* display, Window root)
{
    Pixmap	cursormask;
    XGCValues	xgc;
    GC		gc;
    XColor	dummycolour;
    Cursor	cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
				 &dummycolour, &dummycolour, 0, 0);
    XFreePixmap(display, cursormask);
    XFreeGC(display, gc);
    return cursor;
}

//
// I_StartTic
//
void I_StartTic(void)
{
    unsigned int mask;

    if (!X_display)
	return;

    if (grabMouse)
    {
	if ((paused || demoplayback) && Mousegrabbed)
	{
	    XUngrabPointer(X_display, CurrentTime);
	    Mousegrabbed = false;
	}
	else if (!(paused || demoplayback) && !Mousegrabbed)
	{
	    XGrabPointer(X_display, X_mainWindow, True,
#ifndef POLL_POINTER
		     ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
#else
		     0,
#endif
		     GrabModeAsync, GrabModeAsync,
		     X_mainWindow, None, CurrentTime);
	    XQueryPointer(X_display, X_mainWindow, &dummy, &dummy,
		  &dont_care, &dont_care,
		  &lastmousex, &lastmousey,
		  &mask);
	    Mousegrabbed = true;
	}
#ifdef POLL_POINTER
	if (Mousegrabbed)
	{
	  XQueryPointer(X_display, X_mainWindow, &dummy, &dummy,
		  &dont_care, &dont_care,
		  &newmousex, &newmousey,
		  &mask);
	  event.type = ev_mouse;
	  event.data1 = ((mask & Button1Mask) ? 1 : 0) |
		      ((mask & Button2Mask) ? 2 : 0) |
		      ((mask & Button3Mask) ? 4 : 0);
	  event.data2 = (newmousex - lastmousex) << 2;
	  event.data3 = -((newmousey - lastmousey) << 2);
	  D_PostEvent(&event);
	}
	if ((newmousex < 1) || (newmousey < 1)
	    || (newmousex > X_width - 2)
	    || (newmousey > X_height - 2))
	{
		XWarpPointer(X_display,
			  None,
			  X_mainWindow,
			  0, 0,
			  0, 0,
			  X_width / 2, X_height / 2);
	    lastmousex = X_width / 2;
	    lastmousey = X_height / 2;
	}
	else
	{
	    lastmousex = newmousex;
	    lastmousey = newmousey;
	}
#endif
    }

    while (XPending(X_display))
	I_GetEvent();
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
	// empty
}

//
// I_StartFrame
//
void I_StartFrame(void)
{
	// update sound if necessary
#if !defined(SNDINTR) && !defined(SNDSERV)
	if (sndupd_flag && !use_mmap)
	{
		I_SubmitSound();
		snd_updated = 1;
	}
	else
		snd_updated = 0;
#endif
}

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    static int	lasttic;
    int		tics;
    int		i;

    // draws little dots on the bottom of the screen, a simple fps meter
    if (devparm)
    {
	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i = 0; i < tics * 2; i += 2)
	    screens[0][(SCREENHEIGHT - 2) * SCREENWIDTH + i + 3] = 0xff;
	for (; i < 20 * 2; i += 2)
	    screens[0][(SCREENHEIGHT - 2) * SCREENWIDTH + i + 3] = 0x0;
    }

    // Special optimization for 16bpp and screen size * 2, because this
    // probably is the most used one. This code does the scaling
    // and colormap transformation in one single loop instead of two,
    // which halves the time needed for the transformations.
    if ((multiply == 2) && (x_bpp == 2))
    {
	unsigned int		*olineptrs[2];
	unsigned char		*ilineptr;
	int			x, y, i;
	unsigned short		pixel;
	unsigned int		twopixel;
	const int		step = X_width/2;

	ilineptr = (unsigned char *) screens[0];
	for (i = 0; i < 2; i++)
	    olineptrs[i] = (unsigned int *) &(image->data[i * X_width * 2]);

	y = SCREENHEIGHT - 1;
	do
	{
	    x = SCREENWIDTH - 1;
	    do
	    {
		pixel = x_colormap2[*ilineptr++];
		twopixel = (pixel << 16) | pixel;
		*olineptrs[0]++ = twopixel;
		*olineptrs[1]++ = twopixel;
	    } while (x--);
	    olineptrs[0] += step;
	    olineptrs[1] += step;
	} while (y--);
	goto blit_it;	// indenting the whole stuff with one more
			// elseif won't make it better
    }

    // Special optimization for 16bpp and screen size * 3, because I
    // just was working on it anyway. This code does the scaling
    // and colormap transformation in one single loop instead of two,
    // which halves the time needed for the transformations.
    if ((multiply == 3) && (x_bpp == 2))
    {
	unsigned short		*olineptrs[3];
	unsigned char		*ilineptr;
	int			x, y, i;
	unsigned short		pixel;
	const int		step = 2 * X_width;

	ilineptr = (unsigned char *) screens[0];
	for (i = 0; i < 3; i++)
	    olineptrs[i] = (unsigned short *) &(image->data[i * X_width * 2]);

	y = SCREENHEIGHT - 1;
	do
	{
	    x = SCREENWIDTH - 1;
	    do
	    {
		pixel = x_colormap2[*ilineptr++];
		*olineptrs[0]++ = pixel;
		*olineptrs[0]++ = pixel;
		*olineptrs[0]++ = pixel;
		*olineptrs[1]++ = pixel;
		*olineptrs[1]++ = pixel;
		*olineptrs[1]++ = pixel;
		*olineptrs[2]++ = pixel;
		*olineptrs[2]++ = pixel;
		*olineptrs[2]++ = pixel;
	    } while (x--);
	    olineptrs[0] += step;
	    olineptrs[1] += step;
	    olineptrs[2] += step;
	} while (y--);
	goto blit_it;	// indenting the whole stuff with one more
			// elseif won't make it better
    }

    // Special optimization for 32bpp and screen size * 2 contributed
    // by Oliver Kraus. ~ 20% faster screen refresh was measured on
    // a Sun SPARC with usage of MIT shared memory.
#if defined(BIGEND)
    if ((multiply == 2) && (x_bpp == 4))
    {
	unsigned int		*olineptr0;
	unsigned int		*olineptr1;
	const unsigned int	*ilineptr;
	int			x, y, i;
	unsigned int		twoopixels;
	unsigned int		twomoreopixels;
	unsigned int		fouripixels;
	const unsigned int	step = x_bpp * SCREENWIDTH * 2 / 4;
	unsigned int		p0, p1, p2, p3;

	ilineptr = (unsigned int *)(screens[0]);
	olineptr0 = (unsigned int *)image->data;
	olineptr1 = olineptr0 + step;

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		p0 = x_colormap4[fouripixels & 0x0ff];
		p1 = x_colormap4[(fouripixels >> 8) & 0x0ff];
		p2 = x_colormap4[(fouripixels >> 16) & 0x0ff];
		p3 = x_colormap4[(fouripixels >> 24) & 0x0ff];

		*olineptr0++ = p3;
		*olineptr1++ = p3;
		*olineptr0++ = p3;
		*olineptr1++ = p3;
		*olineptr0++ = p2;
		*olineptr1++ = p2;
		*olineptr0++ = p2;
		*olineptr1++ = p2;
		*olineptr0++ = p1;
		*olineptr1++ = p1;
		*olineptr0++ = p1;
		*olineptr1++ = p1;
		*olineptr0++ = p0;
		*olineptr1++ = p0;
		*olineptr0++ = p0;
		*olineptr1++ = p0;

	    } while (x -= 4);
	    olineptr0 += step;
	    olineptr1 += step;
	}

	goto blit_it;	// indenting the whole stuff with one more
			// elseif won't make it better
    }
#endif

    // From here on the old code, first scale screen, then in a second step
    // do the colormap transformation. This works for all combinations.

    // scales the screen size before blitting it
    if (multiply == 2)
    {

	// This is what id originaly wrote, should be fairly portable.

#if !defined(__GNUC__) || defined(BIGEND)

	unsigned int		*olineptrs[2];
	const unsigned int	*ilineptr;
	int			x, y, i;
	unsigned int		twoopixels;
	unsigned int		twomoreopixels;
	unsigned int		fouripixels;
	const unsigned int	step = X_width / 4;

	ilineptr = (unsigned int *) (screens[0]);
	for (i = 0; i < 2; i++)
	    olineptrs[i] = (unsigned int *) &image->data[i * X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		twoopixels =	(fouripixels & 0xff000000)
		    |	((fouripixels >> 8) & 0xffff00)
		    |	((fouripixels >> 16) & 0xff);
		twomoreopixels =	((fouripixels << 16) & 0xff000000)
		    |	((fouripixels << 8) & 0xffff00)
		    |	(fouripixels & 0xff);

#ifdef BIGEND
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
#else
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
#endif
	    } while (x -= 4);
	    olineptrs[0] += step;
	    olineptrs[1] += step;
	}

#elif defined(__GNUC__) && defined(i386)

	// With GNU C on x86 we can optimize that some by using assembler.

	unsigned int		*olineptrs[2];
	const unsigned int	*ilineptr;
	int			x, y;
	register unsigned int	twoopixels;
	register unsigned int	twomoreopixels;
	const unsigned int	step = X_width / 4;

	ilineptr = (unsigned int *) (screens[0]);
	olineptrs[0] = (unsigned int *) &image->data[0];
	olineptrs[1] = (unsigned int *) &image->data[X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		asm(	"movl %%eax,%%edx;"
			"shrl $16,%%eax;"
			"movb %%dh,%%cl;"
			"movb %%dh,%%ch;"
			"movb %%ah,%%bh;"
			"shll $16,%%ecx;"
			"movb %%ah,%%bl;"
			"shll $16,%%ebx;"
			"movb %%dl,%%ch;"
			"movb %%al,%%bh;"
			"movb %%dl,%%cl;"
			"movb %%al,%%bl;"
			: "=b" (twoopixels), "=c" (twomoreopixels)
			: "a" (*ilineptr++) : "%cc", "%edx"
		);

		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;

	    } while (x -= 4);
	    olineptrs[0] += step;
	    olineptrs[1] += step;
	}

#else	/* __GNUC__ */

	// With a GNU C compiler we can use long long reliable
	// and write 64bit integers, which halves the inner loop.
	// Only works on little endian.

	unsigned long long	*olineptrs[2];
	const unsigned		int *ilineptr;
	int			x, y, i;
	unsigned int		fouripixels;
	unsigned long long	fouropixels;
	const unsigned int	step = X_width / 8;

	ilineptr = (unsigned int *) (screens[0]);
	for (i = 0; i < 2; i++)
	    olineptrs[i] = (unsigned long long *)&image->data[i * X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels =
			(((long long)fouripixels & 0xff000000) << 32)
		    |	((((long long)fouripixels >> 8) & 0xffff00) << 32)
		    |	((((long long)fouripixels >> 16) & 0xff) << 32)
		    |	((fouripixels << 16) & 0xff000000)
		    |	((fouripixels << 8) & 0xffff00)
		    |	(fouripixels & 0xff);
		*olineptrs[0]++ = fouropixels;
		*olineptrs[1]++ = fouropixels;
	    } while (x -= 4);
	    olineptrs[0] += step;
	    olineptrs[1] += step;
	}

#endif /* __GNUC__ */

    }
    else if (multiply == 3)
    {
	unsigned int		*olineptrs[3];
	const unsigned int	*ilineptr;
	int			x, y, i;
	unsigned int		fouropixels[3];
	unsigned int		fouripixels;
	const unsigned int	step = 2 * X_width / 4;

	ilineptr = (unsigned int *) (screens[0]);
	for (i = 0; i < 3; i++)
	    olineptrs[i] = (unsigned int *) &image->data[i * X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels[0] = (fouripixels & 0xff000000)
		    |	((fouripixels >> 8) & 0xff0000)
		    |	((fouripixels >> 16) & 0xffff);
		fouropixels[1] = ((fouripixels << 8) & 0xff000000)
		    |	(fouripixels & 0xffff00)
		    |	((fouripixels >> 8) & 0xff);
		fouropixels[2] = ((fouripixels << 16) & 0xffff0000)
		    |	((fouripixels << 8) & 0xff00)
		    |	(fouripixels & 0xff);
#ifdef BIGEND
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
#else
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
#endif
	    } while (x -= 4);
	    olineptrs[0] += step;
	    olineptrs[1] += step;
	    olineptrs[2] += step;
	}

    }
    else if (multiply == 4)
    {
	// Broken. Gotta fix this some day.
	static void Expand4(unsigned *, double *);
  	Expand4((unsigned *)(screens[0]), (double *) (image->data));
    }

    // colormap transformation dependend on X server color depth
    if (x_bpp == 2)
    {
      int			x, y;
      const int			xstart = SCREENWIDTH * multiply - 1;
      const unsigned char	*ilineptr;
      unsigned short		*olineptr;

      y = SCREENHEIGHT * multiply;
      while (y--)
      {
	olineptr =  (unsigned short *) &(image->data[y * X_width * x_bpp]);
	if (multiply == 1)
	  ilineptr = (unsigned char *)(screens[0] + y * X_width);
	else
	  ilineptr =  (unsigned char *)&(image->data[y * X_width]);
	x = xstart;
	do
	{
	  olineptr[x] = x_colormap2[ilineptr[x]];
	} while (x--);
      }
    }
    else if (x_bpp == 3)
    {
      int			x, y;
      const int			xstart = SCREENWIDTH * multiply - 1;
      const unsigned char	*ilineptr;
      unsigned char		*olineptr;

      y = SCREENHEIGHT*multiply;
      while (y--)
      {
	olineptr =  (unsigned char *)&image->data[y * X_width * x_bpp];
	if (multiply == 1)
	  ilineptr = (unsigned char *)(screens[0] + y * X_width);
	else
	  ilineptr =  (unsigned char *)&image->data[y * X_width];
	x = xstart;
	do
	{
	  memcpy(olineptr + 3 * x, x_colormap3 + 3 * ilineptr[x], 3);
	} while (x--);
      }
    }
    else if (x_bpp == 4)
    {
      int			x, y;
      const int			xstart = SCREENWIDTH * multiply - 1;
      const unsigned char	*ilineptr;
      unsigned int		*olineptr;

      y = SCREENHEIGHT * multiply;
      while (y--)
      {
	olineptr =  (unsigned int *) &(image->data[y * X_width * x_bpp]);
	if (multiply == 1)
	  ilineptr = (unsigned char *)(screens[0] + y * X_width);
	else
	  ilineptr =  (unsigned char *)&(image->data[y * X_width]);
	x = xstart;
	do
	{
	  olineptr[x] = x_colormap4[ilineptr[x]];
	} while (x--);
      }
    }

blit_it:
    if (doShm)
    {
	if (!XShmPutImage(X_display,
			  X_mainWindow,
			  X_gc,
			  image,
			  0, 0,
			  0, 0,
			  X_width, X_height,
			  True))
	    I_Error("XShmPutImage() failed\n");

	// wait for it to finish and processes all input events
	shmFinished = false;
	do
	{
	    if (XPending(X_display))
		I_GetEvent();

	    // sleep a bit if there was no sound update
	    if (!snd_updated)
	    {
		I_WaitVBL(1);
	    }
	} while (!shmFinished);

    }
    else
    {
	// draw the image
	XPutImage(X_display,
		  X_mainWindow,
		  X_gc,
		  image,
		  0, 0,
		  0, 0,
		  X_width, X_height);
    }
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr)
{
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// Palette stuff.
//
static XColor colors[256];

static void UploadNewPalette(Colormap cmap, byte *palette)
{

    register int	i;
    register int	c;
    static boolean	firstcall = true;

#ifdef __cplusplus
    if (X_visualinfo.c_class == PseudoColor && X_visualinfo.depth == 8)
#else
    if (X_visualinfo.class == PseudoColor && X_visualinfo.depth == 8)
#endif
    {
	// initialize the colormap
	if (firstcall)
	{
	    firstcall = false;
	    for (i = 0; i < 256; i++)
	    {
		colors[i].pixel = i;
		colors[i].flags = DoRed | DoGreen | DoBlue;
	    }
	}

	// set the X colormap entries
	for (i = 0; i < 256; i++)
	{
	    c = gammatable[usegamma][*palette++];
	    colors[i].red = (c << 8) + c;
	    c = gammatable[usegamma][*palette++];
	    colors[i].green = (c << 8) + c;
	    c = gammatable[usegamma][*palette++];
	    colors[i].blue = (c << 8) + c;
	}

	// store the colors to the current colormap
	XStoreColors(X_display, cmap, colors, 256);
    }
}

static void EmulateNewPalette(byte *palette)
{
    register int	i;

    for (i = 0; i < 256; i++)
    {
      if (x_bpp == 2)
      {
	x_colormap2[i] =
	  ((gammatable[usegamma][*palette] >> x_red_mask) << x_red_offset) |
	  ((gammatable[usegamma][palette[1]] >> x_green_mask) << x_green_offset) |
	  ((gammatable[usegamma][palette[2]] >> x_blue_mask) << x_blue_offset);
	palette += 3;
      }
      else if (x_bpp == 3)
      {
	x_colormap3[3 * i + x_red_offset] = gammatable[usegamma][*palette++];
	x_colormap3[3 * i + x_green_offset] = gammatable[usegamma][*palette++];
	x_colormap3[3 * i + x_blue_offset] = gammatable[usegamma][*palette++];
      }
      else if (x_bpp == 4)
      {
	x_colormap4[i] = 0;
	((unsigned char*)(x_colormap4 + i))[x_red_offset] =
	  gammatable[usegamma][*palette++];
	((unsigned char*)(x_colormap4 + i))[x_green_offset] =
	  gammatable[usegamma][*palette++];
	((unsigned char*)(x_colormap4 + i))[x_blue_offset] =
	  gammatable[usegamma][*palette++];
      }
    }
}

//
// I_SetPalette
//
void I_SetPalette(byte *palette)
{
  if (x_pseudo)
    UploadNewPalette(X_cmap, palette);
  else
    EmulateNewPalette(palette);
}

//
// This function is probably redundant,
//  if XShmDetach works properly.
// ddt never detached the XShm memory,
//  thus there might have been stale
//  handles accumulating.
//
static void grabsharedmemory(int size)
{
  int			key = ('d' << 24) | ('o' << 16) | ('o' << 8) | 'm';
  struct shmid_ds	shminfo;
  int			minsize = 320 * 200;
  int			id;
  int			rc;
  int			pollution = 5;

  // try to use what was here before
  do
  {
    id = shmget((key_t) key, minsize, 0777); // just get the id
    if (id != -1)
    {
      rc = shmctl(id, IPC_STAT, &shminfo); // get stats on it
      if (!rc)
      {
	if (shminfo.shm_nattch)
	{
	  printf("User %d appears to be running "
		  "DOOM.  Is that wise?\n", shminfo.shm_cpid);
	  key++;
	}
	else
	{
	  if (getuid() == shminfo.shm_perm.cuid)
	  {
	    rc = shmctl(id, IPC_RMID, 0);
	    if (!rc)
	      printf("Was able to kill my old shared memory\n");
	    else
	      I_Error("Was NOT able to kill my old shared memory");

	    id = shmget((key_t)key, size, IPC_CREAT | 0777);
	    if (id == -1)
	      I_Error("Could not get shared memory");

	    rc=shmctl(id, IPC_STAT, &shminfo);

	    break;
	  }
	  if (size >= shminfo.shm_segsz)
	  {
	    printf( "will use %d's stale shared memory\n",
		    shminfo.shm_cpid);
	    break;
	  }
	  else
	  {
	    printf("warning: can't use stale "
		    "shared memory belonging to id %d, "
		    "key=0x%x\n",
		    shminfo.shm_cpid, key);
	    key++;
	  }
	}
      }
      else
      {
	I_Error("could not get stats on key=%d", key);
      }
    }
    else
    {
      id = shmget((key_t)key, size, IPC_CREAT | 0777);
      if (id == -1)
      {
	extern int errno;
	printf("errno=%d\n", errno);
	I_Error("Could not get any shared memory");
      }
      break;
    }
  } while (--pollution);

  if (!pollution)
  {
    I_Error("Sorry, system too polluted with stale "
	    "shared memory segments.\n");
  }

  X_shminfo.shmid = id;

  // attach to the shared memory segment
  image->data = X_shminfo.shmaddr = shmat(id, 0, 0);

  printf("shared memory id=%d, addr=0x%lx\n", id,
	  (long)(image->data));
}

void I_InitGraphics(void)
{
    char		*displayname;
    char		*window_name = "XDoom";
    char		*icon_name = window_name;
    char		*d;
    int			n;
    int			pnum;
    int			x = 0;
    int			y = 0;
    char		xsign = ' ';
    char		ysign = ' ';

    int			oktodraw;
    unsigned long	attribmask;
    XSetWindowAttributes attribs;
    XGCValues		xgcvalues;
    XTextProperty	windowName, iconName;
    int			valuemask;
    static int		firsttime = 1;

    if (!firsttime)
	return;
    firsttime = 0;

    signal(SIGINT, (void (*)(int))I_Quit);
    signal(SIGQUIT, (void (*)(int))I_Quit);
    signal(SIGHUP, (void (*)(int))I_Quit);
    signal(SIGTERM, (void (*)(int))I_Quit);

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-4"))
	multiply = 4;

    X_width = SCREENWIDTH * multiply;
    X_height = SCREENHEIGHT * multiply;

    // check for command-line display name
    if ((pnum = M_CheckParm("-display")))
	displayname = myargv[pnum + 1];
    else
	displayname = 0;

    // check if the user wants to grab the mouse
    grabMouse = !!M_CheckParm("-grabmouse");

    // check for command-line geometry
    if ( (pnum = M_CheckParm("-geometry")) )
    {
	n = sscanf(myargv[pnum + 1], "%c%d%c%d", &xsign, &x, &ysign, &y);

	if (n == 2)
	    x = y = 0;
	else if (n == 6)
	{
	    if (xsign == '-')
		x = -x;
	    if (ysign == '-')
		y = -y;
	}
    }

    // open the display
    X_display = XOpenDisplay(displayname);
    if (!X_display)
    {
	if (displayname)
	    I_Error("Could not open display [%s]", displayname);
	else
	{
	    if (getenv("DISPLAY") == (char *)0)
		I_Error("Could not open display, $DISPLAY not set\n");
	    else
		I_Error("Could not open display (DISPLAY=[%s])", getenv("DISPLAY"));
	}
    }

    // find visual
    X_screen = DefaultScreen(X_display);
    if (XMatchVisualInfo(X_display, X_screen, 8, PseudoColor, &X_visualinfo))
      { x_depth = 1; x_pseudo = 1; printf("Using visual 8bpp Pseudo Color\n"); }
    else if
      (XMatchVisualInfo(X_display, X_screen, 16, TrueColor, &X_visualinfo))
      { x_depth = 2; x_pseudo = 0; printf("Using visual 16bpp True Color\n"); }
    else if
      (XMatchVisualInfo(X_display, X_screen, 24, TrueColor, &X_visualinfo))
      { x_depth = 3; x_pseudo = 0; printf("Using visual 24bpp True Color\n"); }
    else if
      (XMatchVisualInfo(X_display, X_screen, 32, TrueColor, &X_visualinfo))
      { x_depth = 4; x_pseudo = 0; printf("Using visual 32bpp True Color\n"); }
    else
	I_Error("no supported visual found");
    X_visual = X_visualinfo.visual;
    x_red_mask = X_visual->red_mask;
    x_green_mask = X_visual->green_mask;
    x_blue_mask = X_visual->blue_mask;

    if (x_depth == 3 || x_depth == 4)
    {
      if (ImageByteOrder(X_display) == MSBFirst) // big endian X server
      {
	switch (x_red_mask)
	{
	  case 0x000000ff: x_red_offset = 3; break;
	  case 0x0000ff00: x_red_offset = 2; break;
	  case 0x00ff0000: x_red_offset = 1; break;
	  case 0xff000000: x_red_offset = 0; break;
	}
	switch (x_green_mask)
	{
	  case 0x000000ff: x_green_offset = 3; break;
	  case 0x0000ff00: x_green_offset = 2; break;
	  case 0x00ff0000: x_green_offset = 1; break;
	  case 0xff000000: x_green_offset = 0; break;
	}
	switch (x_blue_mask)
	{
	  case 0x000000ff: x_blue_offset = 3; break;
	  case 0x0000ff00: x_blue_offset = 2; break;
	  case 0x00ff0000: x_blue_offset = 1; break;
	  case 0xff000000: x_blue_offset = 0; break;
	}
      }
      else				// little endian X server
      {
	switch (x_red_mask)
	{
	  case 0x000000ff: x_red_offset = 0; break;
	  case 0x0000ff00: x_red_offset = 1; break;
	  case 0x00ff0000: x_red_offset = 2; break;
	  case 0xff000000: x_red_offset = 3; break;
	}
	switch (x_green_mask)
	{
	  case 0x000000ff: x_green_offset = 0; break;
	  case 0x0000ff00: x_green_offset = 1; break;
	  case 0x00ff0000: x_green_offset = 2; break;
	  case 0xff000000: x_green_offset = 3; break;
	}
	switch (x_blue_mask)
	{
	  case 0x000000ff: x_blue_offset = 0; break;
	  case 0x0000ff00: x_blue_offset = 1; break;
	  case 0x00ff0000: x_blue_offset = 2; break;
	  case 0xff000000: x_blue_offset = 3; break;
	}
      }
    }
    if (x_depth == 2)
    {
      // for 16bpp, x_*_offset specifies the number of bits to shift
      unsigned long mask;

      mask = x_red_mask;
      x_red_offset = 0;
      while (!(mask & 1))
      {
	x_red_offset++;
	mask >>= 1;
      }
      x_red_mask = 8;
      while (mask & 1)
      {
	x_red_mask--;
	mask >>= 1;
      }

      mask = x_green_mask;
      x_green_offset = 0;
      while (!(mask & 1))
      {
	x_green_offset++;
	mask >>= 1;
      }
      x_green_mask = 8;
      while (mask & 1)
      {
	x_green_mask--;
	mask >>= 1;
      }

      mask = x_blue_mask;
      x_blue_offset = 0;
      while (!(mask & 1))
      {
	x_blue_offset++;
	mask >>= 1;
      }
      x_blue_mask = 8;
      while (mask & 1)
      {
	x_blue_mask--;
	mask >>= 1;
      }
    }

    {
      int count;
      XPixmapFormatValues* X_pixmapformats =
	XListPixmapFormats(X_display, &count);
      if (X_pixmapformats)
      {
	int i;
	x_bpp = 0;
	for (i = 0; i < count; i++)
	{
	  if (X_pixmapformats[i].depth == x_depth * 8)
	  {
	    x_bpp = X_pixmapformats[i].bits_per_pixel / 8; break;
	  }
	}
	if (x_bpp == 0)
	  I_Error("Could not determine bits_per_pixel");
	XFree(X_pixmapformats);
      }
      else
	I_Error("Could not get list of pixmap formats");
    }

    // check for the MITSHM extension
    doShm = XShmQueryExtension(X_display);

    // even if it's available, make sure it's a local connection
    if (doShm)
    {
	if (!displayname) displayname = (char *)getenv("DISPLAY");
	if (displayname)
	{
	    d = displayname;
	    while (*d && (*d != ':')) d++;
	    if (*d) *d = 0;
	    if (!(!strcasecmp(displayname, "unix") || !*displayname))
		doShm = false;
	}
    }

    if (doShm)
	printf("Using MITSHM extension\n");

    // create the colormap
    if (x_pseudo)
      X_cmap = XCreateColormap(X_display, RootWindow(X_display, X_screen),
			       X_visual, AllocAll);
    else if (x_bpp == 2)
      x_colormap2 = malloc(2 * 256);
    else if (x_bpp == 3)
      x_colormap3 = malloc(3 * 256);
    else if (x_bpp == 4)
      x_colormap4 = malloc(4 * 256);

    // setup attributes for main window
    attribmask = CWEventMask | CWColormap | CWBorderPixel;
    attribs.event_mask =
	KeyPressMask
	| KeyReleaseMask
#ifndef POLL_POINTER
	| PointerMotionMask | ButtonPressMask | ButtonReleaseMask
#endif
	| ExposureMask | StructureNotifyMask;

    attribs.colormap = X_cmap;
    attribs.border_pixel = 0;

    // create the main window
    X_mainWindow = XCreateWindow(X_display,
				 RootWindow(X_display, X_screen),
				 x, y,
				 X_width, X_height,
				 0, // borderwidth
				 8 * x_depth, // depth
				 InputOutput,
				 X_visual,
				 attribmask,
				 &attribs);

    // make cursor invisible when over this window
    XDefineCursor(X_display, X_mainWindow,
		  createnullcursor(X_display, X_mainWindow));

    // create the GC
    valuemask = GCGraphicsExposures;
    xgcvalues.graphics_exposures = False;
    X_gc = XCreateGC(X_display,
  		     X_mainWindow,
  		     valuemask,
  		     &xgcvalues);

    // create icon pixmap
    X_icon = XCreateBitmapFromData(X_display, X_mainWindow,
		(char *)doom_icon_bits, doom_icon_width, doom_icon_height);

    // set size hints for window manager, so that resizing isn't possible
    X_size.flags = PPosition | PSize | PMinSize | PMaxSize;
    X_size.min_width = X_size.max_width = X_size.base_width = X_width;
    X_size.min_height = X_size.max_height = X_size.base_height = X_height;

    // window and icon name for the window manager
    XStringListToTextProperty(&window_name, 1, &windowName);
    XStringListToTextProperty(&icon_name, 1, &iconName);

    // window manager hints
    X_wm.initial_state = NormalState;
    X_wm.input = True;
    X_wm.icon_pixmap = X_icon;
    X_wm.flags = StateHint | InputHint | IconPixmapHint;

    // resource class, in case we get a resource configuration file sometime
    X_class.res_name = "xdoom";
    X_class.res_class = "Xdoom";

    // set the properties
    XSetWMProperties(X_display, X_mainWindow, &windowName, &iconName,
		0 /*argv*/, 0 /*argc*/, &X_size, &X_wm, &X_class);

    // set window manager protocol
    X_wm_delwin = XInternAtom(X_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(X_display, X_mainWindow, &X_wm_delwin, 1);

    // map the window
    XMapWindow(X_display, X_mainWindow);

    // wait until it is OK to draw
    oktodraw = 0;
    while (!oktodraw)
    {
	XNextEvent(X_display, &X_event);
	if (X_event.type == Expose
	    && !X_event.xexpose.count)
	{
	    oktodraw = 1;
	}
    }

    // grabs the pointer so it is restricted to this window
    if (grabMouse)
    {
	XGrabPointer(X_display, X_mainWindow, True,
#ifndef POLL_POINTER
		     ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
#else
		     0,
#endif
		     GrabModeAsync, GrabModeAsync,
		     X_mainWindow, None, CurrentTime);
	Mousegrabbed = true;
    }

    // set the focus to the window, some window managers don't and
    // we're in deep shit if option -grabmouse was used
    XSetInputFocus(X_display, X_mainWindow, RevertToPointerRoot, CurrentTime);

    // get the pointer coordinates so that the first pointer move won't
    // be completely wrong
    XQueryPointer(X_display, X_mainWindow, &dummy, &dummy,
		  &dont_care, &dont_care,
		  &lastmousex, &lastmousey,
		  (unsigned int *)&dont_care);

    if (doShm)
    {
	X_shmeventtype = XShmGetEventBase(X_display) + ShmCompletion;

	// create the image
	image = XShmCreateImage(X_display,
				X_visual,
				8 * x_depth,
				ZPixmap,
				0,
				&X_shminfo,
				X_width,
				X_height);

	grabsharedmemory(image->bytes_per_line * image->height);

	if (!image->data)
	{
	    perror("");
	    I_Error("shmat() failed in InitGraphics()");
	}

	// get the X server to attach to it
	if (!XShmAttach(X_display, &X_shminfo))
	    I_Error("XShmAttach() failed in InitGraphics()");
    }
    else
    {
	image = XCreateImage(X_display,
    			     X_visual,
    			     8 * x_depth,
    			     ZPixmap,
    			     0,
    			     (char *)malloc(X_width * X_height * x_bpp),
    			     X_width, X_height,
    			     8 * x_bpp,
    			     X_width * x_bpp);
    }

    if (multiply == 1 && x_depth == 1)
	screens[0] = (unsigned char *)(image->data);
    else
	screens[0] = (unsigned char *)malloc(SCREENWIDTH * SCREENHEIGHT);

#ifdef USE_JOYSTICK
    if (usejoystick)
	I_InitJoystick();
#endif
}

static unsigned	exptable[256];

static void InitExpand(void)
{
    int		i;

    for (i = 0; i < 256; i++)
	exptable[i] = i | (i << 8) | (i << 16) | (i << 24);
}

static double exptable2[256 * 256];

static void InitExpand2(void)
{
    int		i;
    int		j;
    double	*exp;
    union
    {
	double 		d;
	unsigned	u[2];
    } pixel;

    //printf("building exptable2...\n");
    exp = exptable2;
    for (i = 0; i < 256; i++)
    {
	pixel.u[0] = i | (i << 8) | (i << 16) | (i << 24);
	for (j = 0; j < 256; j++)
	{
	    pixel.u[1] = j | (j << 8) | (j << 16) | (j << 24);
	    *exp++ = pixel.d;
	}
    }
    //printf("done.\n");
}

static int inited;

static void Expand4(unsigned *lineptr, double *xline)
{
    double	dpixel;
    unsigned	x;
    unsigned 	y;
    unsigned	fourpixels;
    unsigned	step;
    double	*exp;

    exp = exptable2;
    if (!inited)
    {
	inited = 1;
	InitExpand2();
    }

    step = 3 * SCREENWIDTH / 2;

    y = SCREENHEIGHT - 1;
    do
    {
	x = SCREENWIDTH;
	do
	{
	    fourpixels = lineptr[0];

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff0000) >> 13));
	    xline[0] = dpixel;
	    xline[160] = dpixel;
	    xline[320] = dpixel;
	    xline[480] = dpixel;

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff) << 3));
	    xline[1] = dpixel;
	    xline[161] = dpixel;
	    xline[321] = dpixel;
	    xline[481] = dpixel;

	    fourpixels = lineptr[1];

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff0000) >> 13));
	    xline[2] = dpixel;
	    xline[162] = dpixel;
	    xline[322] = dpixel;
	    xline[482] = dpixel;

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff) << 3));
	    xline[3] = dpixel;
	    xline[163] = dpixel;
	    xline[323] = dpixel;
	    xline[483] = dpixel;

	    fourpixels = lineptr[2];

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff0000) >> 13));
	    xline[4] = dpixel;
	    xline[164] = dpixel;
	    xline[324] = dpixel;
	    xline[484] = dpixel;

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff) << 3));
	    xline[5] = dpixel;
	    xline[165] = dpixel;
	    xline[325] = dpixel;
	    xline[485] = dpixel;

	    fourpixels = lineptr[3];

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff0000) >> 13));
	    xline[6] = dpixel;
	    xline[166] = dpixel;
	    xline[326] = dpixel;
	    xline[486] = dpixel;

	    dpixel = *(double *)((long)exp + ((fourpixels & 0xffff) << 3));
	    xline[7] = dpixel;
	    xline[167] = dpixel;
	    xline[327] = dpixel;
	    xline[487] = dpixel;

	    lineptr += 4;
	    xline += 8;
	} while (x -= 16);
	xline += step;
    } while (y--);
}
