// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <X11/extensions/XShm.h>
// Had to dig up XShm.c for this one.
// It is in the libXext, but not in the XFree86 headers.
#ifdef LINUX
int XShmGetEventBase( Display* dpy ); // problems with g++?
#endif

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in.h>
//#include <errnos.h>
#include <signal.h>

#include "doomstat.h"
#include "i_system.h"
#include "multires.h"
#include "m_argv.h"
#include "d_main.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomdef.h"

//xwin stuff
#define POINTER_WARP_COUNTDOWN	1

Display*	X_display=0;
Window		X_mainWindow;
Colormap	X_cmap;
Visual*		X_visual;
GC		X_gc;
XEvent		X_event;
int		X_screen;
XVisualInfo	X_visualinfo;
XImage*		image;
int		X_width;
int		X_height;

// MIT SHared Memory extension.
boolean		doShm;

XShmSegmentInfo	X_shminfo;
int		X_shmeventtype;

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
boolean		grabMouse;
int		doPointerWarp = POINTER_WARP_COUNTDOWN;



//-newly added
int key_shifts=0;
int KB_CAPSLOCK_FLAG=1;

int SCREENWIDTH;
int SCREENHEIGHT;
int SCREENPITCH;
int BPP;
int weirdaspect;

int mousepresent;
int doublebufferflag=0;

byte *hicolortable;
short hicolortransmask1,hicolortransmask2;
short palette_color[256];

extern int usejoystick;
extern int usemouse;

void calctranslucencytable();
char *translucencytable25;
char *translucencytable50;
char *translucencytable75;
//end of newly added stuff

void inithicolor();

void I_AutodetectBPP()
  {
  int pnum;
  char *displayname;

  // check for command-line display name
  if ((pnum=M_CheckParm("-disp"))) // suggest parentheses around assignment
    displayname = myargv[pnum+1];
  else
    displayname = 0;

  X_display = XOpenDisplay(displayname);
  if (!X_display)
    {
	if (displayname)
	    I_Error("Could not open display [%s]", displayname);
	else
	    I_Error("Could not open display (DISPLAY=[%s])", getenv("DISPLAY"));
    }

  // use the default visual
  X_screen = DefaultScreen(X_display);
  if (XMatchVisualInfo(X_display, X_screen, 8, PseudoColor, &X_visualinfo))
    BPP=1;
  else if (XMatchVisualInfo(X_display, X_screen, 16, TrueColor, &X_visualinfo))
    BPP=2;
  else
    I_Error("xdosdoom currently only supports 256-color PseudoColor and 16-BPP TrueColor screens");

  XCloseDisplay(X_display);
  X_display=NULL;
  }

int I_ScanCode2DoomCode (int a)
  {
  return a;
  }
int I_DoomCode2ScanCode (int a)
  {
  return a;
  }

//
//  Translates the key currently in X_event
//

int xlatekey(void)
{

    int rc;

    switch(rc = XKeycodeToKeysym(X_display, X_event.xkey.keycode, 0))
    {
      case XK_Left:	rc = KEYD_LEFTARROW;	break;
      case XK_Right:	rc = KEYD_RIGHTARROW;	break;
      case XK_Down:	rc = KEYD_DOWNARROW;	break;
      case XK_Up:	rc = KEYD_UPARROW;	break;
      case XK_Escape:	rc = KEYD_ESCAPE;	break;
      case XK_Return:	rc = KEYD_ENTER;		break;
      case XK_Tab:	rc = KEYD_TAB;		break;
      case XK_F1:	rc = KEYD_F1;		break;
      case XK_F2:	rc = KEYD_F2;		break;
      case XK_F3:	rc = KEYD_F3;		break;
      case XK_F4:	rc = KEYD_F4;		break;
      case XK_F5:	rc = KEYD_F5;		break;
      case XK_F6:	rc = KEYD_F6;		break;
      case XK_F7:	rc = KEYD_F7;		break;
      case XK_F8:	rc = KEYD_F8;		break;
      case XK_F9:	rc = KEYD_F9;		break;
      case XK_F10:	rc = KEYD_F10;		break;
      case XK_F11:	rc = KEYD_F11;		break;
      case XK_F12:	rc = KEYD_F12;		break;
	
      case XK_BackSpace:
      case XK_Delete:	rc = KEYD_BACKSPACE;	break;

      case XK_Pause:	rc = KEYD_PAUSE;		break;

      case XK_KP_Equal:
      case XK_equal:	rc = KEYD_EQUALS;	break;

      case XK_KP_Subtract:
      case XK_minus:	rc = KEYD_MINUS;		break;

      case XK_Shift_L:
      case XK_Shift_R:
	rc = KEYD_RSHIFT;
	break;
	
      case XK_Control_L:
      case XK_Control_R:
	rc = KEYD_RCTRL;
	break;
	
      case XK_Alt_L:
      case XK_Meta_L:
      case XK_Alt_R:
      case XK_Meta_R:
	rc = KEYD_RALT;
	break;

      case XK_Page_Up:	rc = KEYD_PGUP;		break;
      case XK_Page_Down:	rc = KEYD_PGDN;		break;
      case XK_End:	rc = KEYD_END;		break;
      case XK_Home:	rc = KEYD_HOME;		break;

	
      default:
	if (rc >= XK_space && rc <= XK_asciitilde)
	    rc = rc - XK_space + ' ';
	if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';
	break;
    }

    return rc;

}


//
// I_StartFrame
//
void I_StartFrame (void)
  {
  }

static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;

void I_GetEvent()
  {

    event_t event;

    // put event-grabbing stuff in here
    XNextEvent(X_display, &X_event);
    switch (X_event.type)
    {
      case KeyPress:
        if (XKeycodeToKeysym(X_display, X_event.xkey.keycode, 0)!=XK_Caps_Lock)
          {
	  event.type = ev_keydown;
	  event.data1 = xlatekey();
	  D_PostEvent(&event);
          }
        else
          key_shifts=1-key_shifts;
	fprintf(stderr, "k");
	break;
      case KeyRelease:
	event.type = ev_keyup;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// fprintf(stderr, "ku");
	break;
      case ButtonPress:
       if ((mousepresent!=-1)&&(usemouse))
        {
        int i,j,btndown;

	btndown =
	    (X_event.xbutton.state & Button1Mask)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0)
	    | (X_event.xbutton.button == Button1)
	    | (X_event.xbutton.button == Button2 ? 2 : 0)
	    | (X_event.xbutton.button == Button3 ? 4 : 0);
        j=1; for (i=0;i<3;i++)
          {
          if ((btndown&j)==j)
            {
  	    event.type = ev_keydown;
            event.data1=KEYD_MOUSE1+i;
            D_PostEvent(&event);
            }
          j*=2;
          }
        }
       break;
      case ButtonRelease:
       if ((mousepresent!=-1)&&(usemouse))
        {
        int i,j,btnup;

	btnup =
	    (X_event.xbutton.state & Button1Mask)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0);
	// suggest parentheses around arithmetic in operand of |
	btnup =
	    btnup
	    ^ (X_event.xbutton.button == Button1 ? 1 : 0)
	    ^ (X_event.xbutton.button == Button2 ? 2 : 0)
	    ^ (X_event.xbutton.button == Button3 ? 4 : 0);
        j=1; for (i=0;i<3;i++)
          {
          if ((btnup&j)==j)
            {
  	    event.type = ev_keyup;
            event.data1=KEYD_MOUSE1+i;
            D_PostEvent(&event);
            }
          j*=2;
          }
        }
       break;
      case MotionNotify:
       if ((mousepresent!=-1)&&(usemouse))
        {
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xmotion.state & Button1Mask)
	    | (X_event.xmotion.state & Button2Mask ? 2 : 0)
	    | (X_event.xmotion.state & Button3Mask ? 4 : 0);
	event.data2 = (X_event.xmotion.x - lastmousex) << 2;
        if (novert==0)
          event.data3 = (lastmousey - X_event.xmotion.y) << 2;
        else
          event.data3=0;

	if (event.data2 || event.data3)
	  {
	    lastmousex = X_event.xmotion.x;
	    lastmousey = X_event.xmotion.y;
	    if (X_event.xmotion.x != X_width/2 &&
		X_event.xmotion.y != X_height/2)
	    {
		D_PostEvent(&event);
		// fprintf(stderr, "m");
		mousemoved = false;
	    } else
	    {
		mousemoved = true;
	    }
	  }
        }
       break;
	
      case Expose:
      case ConfigureNotify:
	break;
	
      default:
	if (doShm && X_event.type == X_shmeventtype) shmFinished = true;
	break;
    }


  }


Cursor
createnullcursor
( Display*	display,
  Window	root )
{
    Pixmap cursormask;
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
				 &dummycolour,&dummycolour, 0,0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    return cursor;
}



//
// I_StartTic
//
void I_StartTic()
  {

    if (!X_display)
	return;

    while (XPending(X_display))
	I_GetEvent();

    // Warp the pointer back to the middle of the window
    //  or it will wander off - that is, the game will
    //  loose input focus within X11.
    if ((grabMouse)&&(mousepresent!=-1)&&(usemouse))
    {
	if (!--doPointerWarp)
	{
	    XWarpPointer( X_display,
			  None,
			  X_mainWindow,
			  0, 0,
			  0, 0,
			  X_width/2, X_height/2);

	    doPointerWarp = POINTER_WARP_COUNTDOWN;
	}
    }

    mousemoved = false;

  }


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

void I_FinishUpdate(void)
  {
    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
          {
          if (BPP==1)
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)] = 0xff;
          else
            {
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0xff;
            screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0xff;
            }
          }
	for ( ; i<20*2 ; i+=2)
          {
          if (BPP==1)
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)] = 0x0;
          else
            {
            screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0x0;
            screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0x0;
            }
          }    
    }

  //blast it to the screen
    if (doShm)
    {

	if (!XShmPutImage(	X_display,
				X_mainWindow,
				X_gc,
				image,
				0, 0,
				0, 0,
				X_width, X_height,
				True ))
	    I_Error("XShmPutImage() failed\n");

	// wait for it to finish and processes all input events
	shmFinished = false;
	do
	{
	    I_GetEvent();
	} while (!shmFinished);

    }
    else
    {

	// draw the image
	XPutImage(	X_display,
			X_mainWindow,
			X_gc,
			image,
			0, 0,
			0, 0,
			X_width, X_height );

	// sync up with server
	XSync(X_display, False);

    }
  }

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT*BPP);
}


static XColor	colors[256];
int rshl,bshl,gshl,rshr,bshr,gshr;

void inithicolor()
  {
  int rmask,gmask,bmask;
  int i,j;

  rmask=X_visual->red_mask;
  gmask=X_visual->green_mask;
  bmask=X_visual->blue_mask;

  rshr=gshr=bshr=8;
  j=1;
  for (i=0;i<16;i++)
    {
    if ((rmask&j)==j) rshr--;
    if ((gmask&j)==j) gshr--;
    if ((bmask&j)==j) bshr--;
    j*=2;
    }
  
  rshl=bshl=gshl=0;
  while ((rmask&1)==0)
    {
    rmask>>=1;
    rshl++;
    }
  while ((gmask&1)==0)
    {
    gmask>>=1;
    gshl++;
    }
  while ((bmask&1)==0)
    {
    bmask>>=1;
    bshl++;
    }  
  }

int makecol(int r,int g,int b)
  {
  r=((r>>rshr)<<rshl);
  g=((g>>gshr)<<gshl);
  b=((b>>bshr)<<bshl);
  return (r+g+b);
  }

void UploadNewPalette(Colormap cmap, byte *palette)
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
		for (i=0 ; i<256 ; i++)
		{
		    colors[i].pixel = i;
		    colors[i].flags = DoRed|DoGreen|DoBlue;
		}
	    }

	    // set the X colormap entries
	    for (i=0 ; i<256 ; i++)
	    {
		c = gammatable[usegamma][*palette++];
		colors[i].red = (c<<8) + c;
		c = gammatable[usegamma][*palette++];
		colors[i].green = (c<<8) + c;
		c = gammatable[usegamma][*palette++];
		colors[i].blue = (c<<8) + c;
	    }

	    // store the colors to the current colormap
	    XStoreColors(X_display, cmap, colors, 256);

	}
}


void I_SetPalette (byte* palette, int redness)
  {
  int i,j;
  short *tempcolormap;
  byte* tempptr;

  if (BPP==1)
    {
    UploadNewPalette(X_cmap, palette);
    }
  else
    {  //replace this with some sorta caching system?
    int tempr[256],tempg[256],tempb[256];

    tempcolormap=(short *)colormaps;
    for (i=0;i<256;i++)
      {
      tempr[i]=gammatable[usegamma][*palette++];
      tempg[i]=gammatable[usegamma][*palette++];
      tempb[i]=gammatable[usegamma][*palette++];
      }
    if ((redness<1)||(redness>8))
      {
      for (i=32;i>0;i--)
        {
        tempptr=hicolortable+256*(i-1);
        for (j=0;j<256;j++)
          {
          *tempcolormap=makecol(tempptr[tempr[j]],tempptr[tempg[j]],tempptr[tempb[j]]);
          tempcolormap++;
          }
        }
      }
    else
      {
      int redconst=redness*31;
      for (i=32;i>0;i--)
        {
        tempptr=hicolortable+256*(i-1)+(256*32*redness);
        for (j=0;j<256;j++)
          {
          *tempcolormap=makecol(redconst+tempptr[tempr[j]],tempptr[tempg[j]],tempptr[tempb[j]]);
          tempcolormap++;
          }
        }
      }
    tempptr=hicolortable+256*32;
    for (i=0;i<256;i++)
      {
      j=tempr[i]*54+tempg[i]*183+tempb[i]*19;  //from the colorspace faq
      *tempcolormap=makecol(255-(j/256),255-(j/256),255-(j/256));  //lookup table?
      tempcolormap[256]=0;
      tempcolormap++;
      palette_color[i]=makecol(tempr[i],tempg[i],tempb[i]);
      }      
    }
  }

//
// This function is probably redundant,
//  if XShmDetach works properly.
// ddt never detached the XShm memory,
//  thus there might have been stale
//  handles accumulating.
//
void grabsharedmemory(int size)
{

  int			key = ('d'<<24) | ('o'<<16) | ('o'<<8) | 'm';
  struct shmid_ds	shminfo;
  int			minsize = 320*200;
  int			id;
  int			rc;
  // UNUSED int done=0;
  int			pollution=5;
  
  // try to use what was here before
  do
  {
    id = shmget((key_t) key, minsize, 0777); // just get the id
    if (id != -1)
    {
      rc=shmctl(id, IPC_STAT, &shminfo); // get stats on it
      if (!rc) 
      {
	if (shminfo.shm_nattch)
	{
	  fprintf(stderr, "User %d appears to be running "
		  "DOOM.  Is that wise?\n", shminfo.shm_cpid);
	  key++;
	}
	else
	{
	  if (getuid() == shminfo.shm_perm.cuid)
	  {
	    rc = shmctl(id, IPC_RMID, 0);
	    if (!rc)
	      fprintf(stderr,
		      "Was able to kill my old shared memory\n");
	    else
	      I_Error("Was NOT able to kill my old shared memory");
	    
	    id = shmget((key_t)key, size, IPC_CREAT|0777);
	    if (id==-1)
	      I_Error("Could not get shared memory");
	    
	    rc=shmctl(id, IPC_STAT, &shminfo);
	    
	    break;
	    
	  }
	  if (size >= shminfo.shm_segsz)
	  {
	    fprintf(stderr,
		    "will use %d's stale shared memory\n",
		    shminfo.shm_cpid);
	    break;
	  }
	  else
	  {
	    fprintf(stderr,
		    "warning: can't use stale "
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
      id = shmget((key_t)key, size, IPC_CREAT|0777);
      if (id==-1)
      {
	extern int errno;
	fprintf(stderr, "errno=%d\n", errno);
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
  
  fprintf(stderr, "shared memory id=%d, addr=0x%x\n", id,
	  (int) (image->data));
}



void I_InitGraphics(void)
  {

    char*		displayname;
    char*		d;
    int			n;
    int			pnum;
    int			x=0;
    int			y=0;
    
    // warning: char format, different type arg
    char		xsign=' ';
    char		ysign=' ';
    
    int			oktodraw;
    unsigned long	attribmask;
    XSetWindowAttributes attribs;
    XGCValues		xgcvalues;
    int			valuemask;
    static int		firsttime=1;
    int i,j;

    if (!firsttime)
	return;
    firsttime = 0;

    signal(SIGINT, (void (*)(int)) I_Quit);

    X_width = SCREENWIDTH;
    X_height = SCREENHEIGHT;

    // check for command-line display name
    if ( (pnum=M_CheckParm("-disp")) ) // suggest parentheses around assignment
	displayname = myargv[pnum+1];
    else
	displayname = 0;

    // check if the user wants to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");

    // check for command-line geometry
    if ( (pnum=M_CheckParm("-geom")) ) // suggest parentheses around assignment
    {
	// warning: char format, different type arg 3,5
	n = sscanf(myargv[pnum+1], "%c%d%c%d", &xsign, &x, &ysign, &y);
	
	if (n==2)
	    x = y = 0;
	else if (n==6)
	{
	    if (xsign == '-')
		x = -x;
	    if (ysign == '-')
		y = -y;
	}
	else
	    I_Error("bad -geom parameter");
    }

    // open the display
    X_display = XOpenDisplay(displayname);
    if (!X_display)
    {
	if (displayname)
	    I_Error("Could not open display [%s]", displayname);
	else
	    I_Error("Could not open display (DISPLAY=[%s])", getenv("DISPLAY"));
    }

    // use the default visual 
    X_screen = DefaultScreen(X_display);
    if (BPP==1)
      XMatchVisualInfo(X_display, X_screen, 8, PseudoColor, &X_visualinfo);
    else
      XMatchVisualInfo(X_display, X_screen, 16, TrueColor, &X_visualinfo);
    X_visual = X_visualinfo.visual;
    if (BPP==2)
      inithicolor();


    // check for the MITSHM extension
    doShm = XShmQueryExtension(X_display);

    // even if it's available, make sure it's a local connection
    if (doShm)
    {
	if (!displayname) displayname = (char *) getenv("DISPLAY");
	if (displayname)
	{
	    d = displayname;
	    while (*d && (*d != ':')) d++;
	    if (*d) *d = 0;
	    if (!(!strcasecmp(displayname, "unix") || !*displayname)) doShm = false;
	}
    }

    fprintf(stderr, "Using MITSHM extension\n");

    // create the colormap
    if (BPP==1)
      X_cmap = XCreateColormap(X_display, RootWindow(X_display,
						   X_screen), X_visual, AllocAll);
    else
      X_cmap = XCreateColormap(X_display, RootWindow(X_display,
						   X_screen), X_visual, AllocNone);

    // setup attributes for main window
    attribmask = CWEventMask | CWColormap | CWBorderPixel;
    attribs.event_mask =
	KeyPressMask
	| KeyReleaseMask
	// | PointerMotionMask | ButtonPressMask | ButtonReleaseMask
	| ExposureMask;

    attribs.colormap = X_cmap;
    attribs.border_pixel = 0;

    // create the main window
    X_mainWindow = XCreateWindow(	X_display,
					RootWindow(X_display, X_screen),
					x, y,
					X_width, X_height,
					0, // borderwidth
					8*BPP, // depth
					InputOutput,
					X_visual,
					attribmask,
					&attribs );

    XDefineCursor(X_display, X_mainWindow,
		  createnullcursor( X_display, X_mainWindow ) );

    // create the GC
    valuemask = GCGraphicsExposures;
    xgcvalues.graphics_exposures = False;
    X_gc = XCreateGC(	X_display,
  			X_mainWindow,
  			valuemask,
  			&xgcvalues );

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
	XGrabPointer(X_display, X_mainWindow, True,
		     ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
		     GrabModeAsync, GrabModeAsync,
		     X_mainWindow, None, CurrentTime);

    if (doShm)
    {

	X_shmeventtype = XShmGetEventBase(X_display) + ShmCompletion;

	// create the image
	image = XShmCreateImage(	X_display,
					X_visual,
					BPP*8,
					ZPixmap,
					0,
					&X_shminfo,
					X_width,
					X_height );

	grabsharedmemory(image->bytes_per_line * image->height);


	// UNUSED
	// create the shared memory segment
	// X_shminfo.shmid = shmget (IPC_PRIVATE,
	// image->bytes_per_line * image->height, IPC_CREAT | 0777);
	// if (X_shminfo.shmid < 0)
	// {
	// perror("");
	// I_Error("shmget() failed in InitGraphics()");
	// }
	// fprintf(stderr, "shared memory id=%d\n", X_shminfo.shmid);
	// attach to the shared memory segment
	// image->data = X_shminfo.shmaddr = shmat(X_shminfo.shmid, 0, 0);
	

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
	image = XCreateImage(	X_display,
    				X_visual,
    				BPP*8,
    				ZPixmap,
    				0,
    				(char*)malloc(X_width * X_height),
    				X_width, X_height,
    				8,
    				X_width *BPP  );

    }

  screens[0] = (unsigned char *) (image->data);

  XStoreName(X_display,X_mainWindow,"XDosDoom");
  XSetIconName(X_display,X_mainWindow,"XDosDoom");

//  if (usemouse)
    {
    mousepresent=2;
    if (M_CheckParm("-novert"))
      novert=1;
    }

  //calc translucencytable if needed
  if ((BPP==1)&&(!M_CheckParm("-notrans")))
    calctranslucencytable();

  //do the hicolorpal table if necessary
  if (BPP==2)
    {
    byte *tempptr,*tempptr2;

    tempptr=hicolortable=(byte *)malloc(256*32*9);
    for (i=0;i<32;i++)
      {
      for (j=0;j<256;j++)
        {
        *tempptr=j*gammatable[3][i*(256/32)]/256;
        tempptr++;
        }
      }
    for (i=1;i<=8;i++)
      {
      tempptr2=hicolortable;
      for (j=0;j<(256*32);j++)
        {
        *tempptr=(byte)(((int)(*tempptr2))*(8-i)/8);
        tempptr++; tempptr2++;
        }
      }
    hicolortransmask1=makecol(127,127,127);
    hicolortransmask2=makecol(63,63,63);
    }

  }

void I_ShutdownGraphics(void)
{
  // Detach from X server
  if (!XShmDetach(X_display, &X_shminfo))
	    I_Error("XShmDetach() failed in I_ShutdownGraphics()");

  // Release shared memory.
  shmdt(X_shminfo.shmaddr);
  shmctl(X_shminfo.shmid, IPC_RMID, 0);

  // Paranoia.
  image->data = NULL;
}

void calctranslucencytable()
{
int i,j,k;
int destr,destg,destb;
int diffr,diffg,diffb;
int dist,closestdist;
unsigned char closestcolor;
int rdifftable[512],gdifftable[512],bdifftable[512];
unsigned char *thepalette;
char *tempmem;

printf ("Calculating translucency table\n");
thepalette=W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
tempmem=(char *)malloc(65536*3);
translucencytable25=tempmem;
translucencytable50=tempmem+65536;
translucencytable75=tempmem+65536*2;

for (i=0;i<512;i++)
  {
  rdifftable[i]=((i-256)*(i-256))*54;
  gdifftable[i]=((i-256)*(i-256))*183;
  bdifftable[i]=((i-256)*(i-256))*19;
  }

//first, do 50% translucency
for (i=0;i<256;i++)  //1st color
  {
  for (j=0;j<=i;j++)   //2nd color
    {
    destr=256-(((int)thepalette[i*3+0])+((int)thepalette[j*3+0]))/2;
    destg=256-(((int)thepalette[i*3+1])+((int)thepalette[j*3+1]))/2;
    destb=256-(((int)thepalette[i*3+2])+((int)thepalette[j*3+2]))/2;
    closestdist=2000000000; closestcolor=i;
    for (k=0;k<256;k++)  //check with each other color
      {
      diffr=(((int)thepalette[k*3])+destr);
      diffg=(((int)thepalette[k*3+1])+destg);
      diffb=(((int)thepalette[k*3+2])+destb);
      dist=rdifftable[diffr]+gdifftable[diffg]+bdifftable[diffb];
      if (dist<closestdist)
        {closestdist=dist; closestcolor=k;}
      }
    translucencytable50[(j<<8)+i]=closestcolor;
    translucencytable50[(i<<8)+j]=closestcolor;
    }
  if ((i%8)==0)
    fprintf (stderr,".");
  }

//now, do 25%/75% translucency
for (i=0;i<256;i++)  //1st color
  {
  for (j=0;j<256;j++)   //2nd color
    {
    destr=256-(((int)thepalette[i*3+0])+((int)thepalette[j*3+0])*3)/4;
    destg=256-(((int)thepalette[i*3+1])+((int)thepalette[j*3+1])*3)/4;
    destb=256-(((int)thepalette[i*3+2])+((int)thepalette[j*3+2])*3)/4;
    closestdist=2000000000; closestcolor=i;
    for (k=0;k<256;k++)  //check with each other color
      {
      diffr=(((int)thepalette[k*3])+destr);
      diffg=(((int)thepalette[k*3+1])+destg);
      diffb=(((int)thepalette[k*3+2])+destb);
      dist=rdifftable[diffr]+gdifftable[diffg]+bdifftable[diffb];
      if (dist<closestdist)
        {closestdist=dist; closestcolor=k;}
      }
    translucencytable25[(j<<8)+i]=closestcolor;
    translucencytable75[(i<<8)+j]=closestcolor;
    }
  if ((i%8)==0)
    fprintf (stderr,".");
  }


for (i=0;i<256;i++)
  {
  translucencytable25[(i<<8)+i]=i;
  translucencytable50[(i<<8)+i]=i;
  translucencytable75[(i<<8)+i]=i;
  }
printf ("\n");
}
