/*      win32video.c
 *
 * Win32 DIB graphics routines for DOOM
 * 
 * Petteri Kangaslampi, pekangas@sci.fi
*/

/* Original file header: */
//-----------------------------------------------------------------------------
//
// $Id: win32video.c,v 1.8 1998/01/27 15:42:22 pekangas Exp $
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
// $Log: win32video.c,v $
// Revision 1.8  1998/01/27 15:42:22  pekangas
// Merged a bunch of highcolor and other fixes from Chi Hoang's
// DOSDOOm 0.45
//
// Revision 1.7  1998/01/07 18:44:21  pekangas
// Fixed a bunch of Visual C warnings, changed name to NTDOOM and
// made assembler optional
//
// Revision 1.6  1998/01/07 17:28:16  pekangas
// Improved mouse grab a little
//
// Revision 1.5  1998/01/05 14:46:35  pekangas
// Palette is now released when the window deactivates
//
// Revision 1.4  1998/01/05 13:40:58  pekangas
// Fixed I_ReadScreen for 16-bit modes
//
// Revision 1.3  1998/01/04 19:29:09  pekangas
// Added hicolor and multiresulution support from Chi Hoang's DOS port
//
// Revision 1.2  1997/12/29 21:17:17  pekangas
// Fixed to compile with Visual C
// Some Win95 fixes
//
// Revision 1.1  1997/12/29 19:49:10  pekangas
// Initial revision
//
// Revision 1.1.1.1  1997/12/28 12:59:03  pekangas
// Initial DOOM source release from id Software
//
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

const char
win32video_rcsid[] = "$Id: win32video.c,v 1.8 1998/01/27 15:42:22 pekangas Exp $";

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "doomstat.h"
#include "i_system.h"
#include "multires.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

/* Stuff for multiresolution support: */
int SCREENWIDTH;
int SCREENHEIGHT;
int BPP;
int weirdaspect;
byte *hicolortable;
short hicolortransmask1,hicolortransmask2;


#define POINTER_WARP_COUNTDOWN	1

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
boolean		grabMouse;
int		doPointerWarp = POINTER_WARP_COUNTDOWN;

int		X_width;
int		X_height;


/* Multiplication - we'll let windows stretch instead */
static int	multiply=1;

static int closed=0;

static int disableVerticalMouse = 0;

static int windowActive = 0;

HWND win;
static HINSTANCE inst;
static HDC dibDC;
static LOGPALETTE *palette;
static HPALETTE dibPal;
static BITMAPINFO *bminfo;
static unsigned char *dibData;
static int bits8;


/* [Petteri] no-allegro-hacks: */
int makecol(int r, int g, int b)
{
    assert(BPP==2);
    return (b >> 3) | ((g >> 3) << 5) | ((r >> 3) << 10);
}
int palette_color[256];


int TranslateKey(unsigned k)
{
/*wtf?    if ( (k >= VK_0) && (k <= VK_9) )*/
    if ( (k >= 0x30) && (k <= 0x39) )
        return (k - 0x30 + '0');
    if ( (k >= 0x41) && (k <= 0x5a) )
        return (k - 0x41 + 'a');

#define K(a,b) case a: return b;    
    switch ( k )
    {
        K(VK_LEFT, KEY_LEFTARROW);
        K(VK_RIGHT, KEY_RIGHTARROW);
        K(VK_UP, KEY_UPARROW);
        K(VK_DOWN, KEY_DOWNARROW);
        K(VK_BACK, KEY_BACKSPACE);
        K(VK_TAB, KEY_TAB);
        K(VK_RETURN, KEY_ENTER);
        K(VK_SHIFT, KEY_RSHIFT);
        K(VK_CONTROL, KEY_RCTRL);
        K(VK_MENU, KEY_RALT);
        K(VK_PAUSE, KEY_PAUSE);
        K(VK_ESCAPE, KEY_ESCAPE);
        K(VK_SPACE, ' ');
        K(VK_DELETE, KEY_BACKSPACE);
        K(VK_ADD, '+');
        K(VK_SUBTRACT, KEY_MINUS);
        K(0xBC, ',');
        K(0xBE, '.');
        K(VK_F1, KEY_F1);
        K(VK_F2, KEY_F2);
        K(VK_F3, KEY_F3);
        K(VK_F4, KEY_F4);
        K(VK_F5, KEY_F5);
        K(VK_F6, KEY_F6);
        K(VK_F7, KEY_F7);
        K(VK_F8, KEY_F8);
        K(VK_F9, KEY_F9);
        K(VK_F10, KEY_F10);
        K(VK_F11, KEY_F11);
        K(VK_F12, KEY_F12);
    }

    return 0;
}


void WinError(char *msg)
{
    printf("Windows Error: %s, GetLastError(): %u\n", msg, GetLastError());
    exit(EXIT_FAILURE);
}



static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
                                WPARAM wparam, LPARAM lparam)
{
    event_t event;
    RECT rect;
    
    switch ( message )
    {        
        case WM_DESTROY:
            if ( grabMouse )
            {
                ClipCursor(NULL);
                ShowCursor(TRUE);
            }
            if ( bits8 )
            {
                if ( !UnrealizeObject(dibPal) )
                    WinError("UnrealizeObject failed");
                if ( SetSystemPaletteUse(dibDC, SYSPAL_STATIC) == SYSPAL_ERROR )
                    WinError("SetSystemPaletteUse failed");
            }
            fprintf(stderr, "WM_DESTROY\n");
            PostQuitMessage(0);
            closed = 1;
            break;

        case WM_MOVE:
            GetWindowRect(win, &rect);
            fprintf(stderr, "%u,%u - %u, %u\n",
                    rect.left,rect.top,rect.right,rect.bottom);
            ClipCursor(&rect);
            break;

        case WM_ACTIVATE:
            fprintf(stderr, "WM_ACTIVATE %u\n", (unsigned) LOWORD(wparam));
            if ( LOWORD(wparam) )
            {
                if ( !windowActive )
                {
                    if ( grabMouse )
                    {
                        ClipCursor(NULL); /* helps with Win95? */
                        GetWindowRect(win, &rect);
                        fprintf(stderr, "%u,%u - %u, %u\n",
                                rect.left,rect.top,rect.right,rect.bottom);
                        ClipCursor(&rect);
                        ShowCursor(FALSE);
                    }                    
                }
                windowActive = 1;
                if ( bits8 )
                {
                    if ( SetSystemPaletteUse(dibDC, SYSPAL_NOSTATIC) == SYSPAL_ERROR )
                        WinError("SetSystemPaletteUse failed");
                    if ( SetPaletteEntries(dibPal, 0, 256, palette->palPalEntry) != 256 )
                        WinError("SetPaletteEntries failed");
                    if ( !UnrealizeObject(dibPal) )
                        WinError("UnrealizeObject failed");
                    if ( SelectPalette(dibDC, dibPal, FALSE) == NULL )
                        WinError("SelectPalette failed");
                    if ( RealizePalette(dibDC) == GDI_ERROR )
                        WinError("RealizePalette failed");
                }
            }
            else
            {
                if ( grabMouse )
                {
                    ClipCursor(NULL);
                    ShowCursor(TRUE);
                }
                windowActive = 0;
                if ( bits8 )
                {
                    if ( !UnrealizeObject(dibPal) )
                        WinError("UnrealizeObject failed");
                    if ( SetSystemPaletteUse(dibDC, SYSPAL_STATIC) == SYSPAL_ERROR )
                        WinError("SetSystemPaletteUse failed");
                }
            }
            return DefWindowProc(hwnd, message, wparam, lparam);

        case WM_KEYDOWN:
            event.type = ev_keydown;
            event.data1 = TranslateKey(wparam);
            if ( event.data1 != 0 )
                D_PostEvent(&event);
            break;

        case WM_KEYUP:
            event.type = ev_keyup;
            event.data1 = TranslateKey(wparam);
            if ( event.data1 != 0 )
                D_PostEvent(&event);
            break;            
            
        default:
            return(DefWindowProc(hwnd, message, wparam, lparam));
    }

    return 0;
}


void BlitDIB(void)
{
    RECT rect;

    GetClientRect(win, &rect);
    if ( bits8 )
    {
        if ( StretchDIBits(dibDC, rect.left, rect.top, rect.right-rect.left,
                           rect.bottom-rect.top, 0, 0, SCREENWIDTH,
                           SCREENHEIGHT, dibData, bminfo, DIB_PAL_COLORS,
                           SRCCOPY)
             == GDI_ERROR )
            WinError("StrecthDIBits failed");
    }
    else
    {
        if ( StretchDIBits(dibDC, rect.left, rect.top, rect.right-rect.left,
                           rect.bottom-rect.top, 0, 0, SCREENWIDTH,
                           SCREENHEIGHT, dibData, bminfo, DIB_RGB_COLORS,
                           SRCCOPY)
             == GDI_ERROR )
            WinError("StrecthDIBits failed");
    }
        
    GdiFlush();    
}


void I_ShutdownGraphics(void)
{
    if ( grabMouse )
    {
        ClipCursor(NULL);
        ShowCursor(TRUE);
    }
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

/*static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;*/


void I_GetEvent(void)
{
    MSG msg;
    POINT point;
    static LONG prevX, prevY;
    static int hadMouse = 0;
    event_t event;
    RECT rect;
    int lb, rb;
    static int prevlb = 0, prevrb = 0;
    
    /* Dispatch all messages: */
    while ( PeekMessage(&msg, NULL, 0, 0xFFFFFFFF, PM_REMOVE) )
    {
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }

    /* Check mouse and generate events if necessary: */
    if ( !GetCursorPos(&point) )
        WinError("GetCursorPos() failed");
    if ( hadMouse && windowActive)
    {
        lb = (GetAsyncKeyState(VK_LBUTTON) < 0);
        rb = (GetAsyncKeyState(VK_RBUTTON) < 0);
            
        if ( (prevX != point.x) || (prevY != point.y) ||
             (prevlb != lb) || (prevrb != rb) )
        {
            event.type = ev_mouse;
            event.data1 = lb | (rb << 1);
            event.data2 = (point.x - prevX) << 2;
            if ( disableVerticalMouse )
                event.data3 = 0;
            else
                event.data3 = (prevY - point.y) << 2;
            prevX = point.x;
            prevY = point.y;
            prevlb = lb;
            prevrb = rb;
            D_PostEvent(&event);
        }

        if ( grabMouse )
        {
            GetWindowRect(win, &rect);
            if ( !SetCursorPos((rect.left + rect.right) / 2,
                               (rect.top + rect.bottom) / 2) )
                WinError("SetCursorPos() failed");
            prevX = (rect.left + rect.right) / 2;
            prevY = (rect.top + rect.bottom) / 2;
        }
    }
    else
    {
        prevX = point.x;
        prevY = point.y;
        hadMouse = 1;
    }        

#if 0    
    event_t event;

    // put event-grabbing stuff in here
    XNextEvent(X_display, &X_event);
    switch (X_event.type)
    {
      case KeyPress:
	event.type = ev_keydown;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// fprintf(stderr, "k");
	break;
      case KeyRelease:
	event.type = ev_keyup;
	event.data1 = xlatekey();
	D_PostEvent(&event);
	// fprintf(stderr, "ku");
	break;
      case ButtonPress:
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xbutton.state & Button1Mask)
	    | (X_event.xbutton.state & Button2Mask ? 2 : 0)
	    | (X_event.xbutton.state & Button3Mask ? 4 : 0)
	    | (X_event.xbutton.button == Button1)
	    | (X_event.xbutton.button == Button2 ? 2 : 0)
	    | (X_event.xbutton.button == Button3 ? 4 : 0);
	event.data2 = event.data3 = 0;
	D_PostEvent(&event);
	// fprintf(stderr, "b");
	break;
      case ButtonRelease:
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xbutton.state & Button1Mask)
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
	// fprintf(stderr, "bu");
	break;
      case MotionNotify:
	event.type = ev_mouse;
	event.data1 =
	    (X_event.xmotion.state & Button1Mask)
	    | (X_event.xmotion.state & Button2Mask ? 2 : 0)
	    | (X_event.xmotion.state & Button3Mask ? 4 : 0);
	event.data2 = (X_event.xmotion.x - lastmousex) << 2;
	event.data3 = (lastmousey - X_event.xmotion.y) << 2;

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
	break;
	
      case Expose:
      case ConfigureNotify:
	break;
	
      default:
	if (doShm && X_event.type == X_shmeventtype) shmFinished = true;
	break;
    }
#endif
}


//
// I_StartTic
//
void I_StartTic (void)
{
    I_GetEvent();
#if 0
    if (!X_display)
	return;

    while (XPending(X_display))
	I_GetEvent();

    // Warp the pointer back to the middle of the window
    //  or it will wander off - that is, the game will
    //  loose input focus within X11.
    if (grabMouse)
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
#endif
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
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
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0xff;
            if (BPP==2)
                screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0xff;
        }
	for ( ; i<20*2 ; i+=2)
        {
	    screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0x0;
            if (BPP==2)
                screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0x0;
        }    
    }

    BlitDIB();
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT*BPP);
}


//
// I_SetPalette
//
void I_SetPalette (byte* pal, int redness)
{
    unsigned i, j;
    RGBQUAD *rgb;

    if ( !bits8 )
    {
        if ( BPP == 1 )
        {
            /* Just set the bitmap palette: */
            rgb = bminfo->bmiColors;

            for ( i = 0; i < 256; i++ )
            {
                rgb->rgbRed = gammatable[usegamma][*pal++];
                rgb->rgbGreen = gammatable[usegamma][*pal++];
                rgb->rgbBlue = gammatable[usegamma][*pal++];
                rgb++;
            }
        }
        else
        {
            /* Highcolor magic from Chi Choang's <cyhoang@acs.ucalgary.ca>
               DOS version: */
            int tempr[256],tempg[256],tempb[256];
            short *tempcolormap;
            byte* tempptr;

            tempcolormap=(short *)colormaps;
            for (i=0;i<256;i++)
            {
                tempr[i]=gammatable[usegamma][*pal++];
                tempg[i]=gammatable[usegamma][*pal++];
                tempb[i]=gammatable[usegamma][*pal++];
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

                /* No allegro -hack: */
                palette_color[i] = makecol(tempr[i], tempg[i], tempb[i]);
            }
        }
        
        return;
    }

    fprintf(stderr, "I_SetPalette\n");

    /* 8-bit display */
    assert(BPP == 1);
    
    for ( i = 0; i < 256; i++ )
    {
        palette->palPalEntry[i].peRed = gammatable[usegamma][*pal++];
        palette->palPalEntry[i].peGreen = gammatable[usegamma][*pal++];
        palette->palPalEntry[i].peBlue = gammatable[usegamma][*pal++];
        palette->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
    }

    /* Make sure we'll get an identity palette */
    palette->palPalEntry[255].peRed = 255;
    palette->palPalEntry[255].peGreen = 255;
    palette->palPalEntry[255].peBlue = 255;
    palette->palPalEntry[255].peFlags = 0;

    palette->palPalEntry[0].peRed = 0;
    palette->palPalEntry[0].peGreen = 0;
    palette->palPalEntry[0].peBlue = 0;
    palette->palPalEntry[0].peFlags = 0;

    if ( !windowActive )
        return;

    if ( SetPaletteEntries(dibPal, 0, 256, palette->palPalEntry) != 256 )
        WinError("SetPaletteEntries failed");
    if ( !UnrealizeObject(dibPal) )
        WinError("UnrealizeObject failed");
    if ( SelectPalette(dibDC, dibPal, FALSE) == NULL )
        WinError("SelectPalette failed");
    if ( RealizePalette(dibDC) == GDI_ERROR )
        WinError("RealizePalette failed");
}



void I_InitGraphics(void)
{
    static int firsttime=1;
    
    WNDCLASS wc;
    unsigned i, x, y, j;
    WORD *d;
    unsigned char *b;
    int bits;
    int frameX, frameY, capY;
    RECT rect;
    int width, height;
    RGBQUAD *rgb;
    

    if (!firsttime)
	return;
    firsttime = 0;

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-4"))
	multiply = 4;

    X_width = SCREENWIDTH * multiply;
    X_height = SCREENHEIGHT * multiply;

    // check if the user wants to grab the mouse (quite unnice)
    grabMouse = !!M_CheckParm("-grabmouse");

    /* [Petteri] New: Option to disable mouse vertical movement - useful
       for players used to Quake: */
    disableVerticalMouse = !!M_CheckParm("-novertmouse");

    /* Build and initialize the window: */
    
    inst = (HINSTANCE) GetModuleHandle(NULL);

    frameX = GetSystemMetrics(SM_CXFRAME);
    frameY = GetSystemMetrics(SM_CYFRAME);
    capY = GetSystemMetrics(SM_CYCAPTION);

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = inst;
    wc.hIcon = NULL;
    if ( grabMouse )
        wc.hCursor = LoadCursor( 0, IDC_ARROW );
    else
        wc.hCursor = LoadCursor( 0, IDC_ARROW );
    /*wc.hbrBackground = GetStockObject( WHITE_BRUSH );*/
    wc.hbrBackground = NULL;    
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "DoomWindowClass";

    RegisterClass(&wc);

    width = X_width + 2*frameX;
    height = X_height + 2*frameY + capY;

    win = CreateWindow("DoomWindowClass", "NTDOOM",
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width, height,
                       NULL, NULL, inst, NULL);

    /* Display the window: */
    ShowWindow(win, SW_SHOW);
    UpdateWindow(win);

    GetClientRect(win, &rect);
    fprintf(stderr, "I_InitGraphics: Client area: %ux%u\n",
            rect.right-rect.left, rect.bottom-rect.top);

    if ( (rect.right-rect.left) != X_width )
    {
        fprintf(stderr, "I_InitGraphics: Fixing width\n");
        width += X_width - (rect.right-rect.left);
        MoveWindow(win, 0, 0, width, height, TRUE);
    }
    if ( (rect.bottom-rect.top) != X_height )
    {
        fprintf(stderr, "I_InitGraphics: Fixing height\n");
        height += X_height - (rect.bottom-rect.top);
        MoveWindow(win, 0, 0, width, height, TRUE);
    }

    GetClientRect(win, &rect);
    fprintf(stderr, "I_InitGraphics: Client area: %ux%u\n",
            rect.right-rect.left, rect.bottom-rect.top);    
        
    dibDC = GetDC(win);

    bits = GetDeviceCaps(dibDC, BITSPIXEL);
    fprintf(stderr, "I_InitGraphics: %i bpp screen\n", bits);
    if ( bits == 8 )
    {
        bits8 = 1;
        if ( BPP != 1 )
        {
            fprintf(stderr, "I_InitGraphics: Forcing 8-bit display for "
                    "8-bit screen\n");
            BPP = 1;
        }
    }
    else
    {
        bits8 = 0;
    }

    if ( bits8 )
        if ( SetSystemPaletteUse(dibDC, SYSPAL_NOSTATIC) == SYSPAL_ERROR )
            WinError("SetSystemPaletteUse failed");

    if ( bits8 )
    {
        palette = malloc(sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
        palette->palVersion = 0x300;
        palette->palNumEntries = 256;
        
        for ( i = 0; i < 256; i++ )
        {
            palette->palPalEntry[i].peRed = i;
            palette->palPalEntry[i].peGreen = i;
            palette->palPalEntry[i].peBlue = i;
            palette->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
        }


        if ( bits8 )
        {
            /* Make sure we'll get an identity palette */
            palette->palPalEntry[255].peRed = 255;
            palette->palPalEntry[255].peGreen = 255;
            palette->palPalEntry[255].peBlue = 255;
            palette->palPalEntry[255].peFlags = 0;
            
            palette->palPalEntry[0].peRed = 0;
            palette->palPalEntry[0].peGreen = 0;
            palette->palPalEntry[0].peBlue = 0;
            palette->palPalEntry[0].peFlags = 0;
        }

        if ( (dibPal = CreatePalette(palette)) == NULL )
            WinError("CreatePalette failed");
        if ( SelectPalette(dibDC, dibPal, FALSE) == NULL )
            WinError("SelectPalette failed");
        if ( RealizePalette(dibDC) == GDI_ERROR )
            WinError("RealizePalette failed");
    }

    if ( BPP == 1 )
        bminfo = malloc(sizeof(BITMAPINFOHEADER) + 4*256);
    else
        bminfo = malloc(sizeof(BITMAPINFOHEADER));
        

    if ( bits8 )
    {
        d = (WORD*) bminfo->bmiColors;
        for ( i = 0; i < 256; i++ )
            *(d++) = i;
    }
    else
    {
        if ( BPP == 1 )
        {
            rgb = bminfo->bmiColors;
            for ( i = 0; i < 256; i++ )
            {
                rgb->rgbRed = i;
                rgb->rgbGreen = i;
                rgb->rgbBlue = i;
                rgb->rgbReserved = 0;
                rgb++;
            }
        }
    }
    
    bminfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bminfo->bmiHeader.biWidth = SCREENWIDTH;
    bminfo->bmiHeader.biHeight = -SCREENHEIGHT;
    bminfo->bmiHeader.biPlanes = 1;
    if ( BPP == 1 )
        bminfo->bmiHeader.biBitCount = 8;
    else
        bminfo->bmiHeader.biBitCount = 16;
    bminfo->bmiHeader.biCompression = BI_RGB;
    bminfo->bmiHeader.biSizeImage = 0;
    bminfo->bmiHeader.biXPelsPerMeter = 0;
    bminfo->bmiHeader.biYPelsPerMeter = 0;
    bminfo->bmiHeader.biClrUsed = 0;
    bminfo->bmiHeader.biClrImportant = 0;
    
    dibData = malloc(SCREENWIDTH*SCREENHEIGHT*BPP);
    if ( BPP == 1 )
    {
        b = dibData;
        for ( y = 0; y < (unsigned)SCREENHEIGHT; y++ )
            for ( x = 0; x < (unsigned)SCREENWIDTH; x++ )
                *(b++) = y;
    }
    else
    {
        d = (WORD*) dibData;
        for ( y = 0; y < (unsigned)SCREENHEIGHT; y++ )
            for ( x = 0; x < (unsigned)SCREENWIDTH; x++ )                
                *(d++) = (WORD) makecol(x, 0, y);
    }

    BlitDIB();

    screens[0] = (unsigned char *) (dibData);

    /* Build magic highcolor table: */
    if (BPP==2)
    {
        byte *tempptr, *tempptr2;

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



/*
 * $Log: win32video.c,v $
 * Revision 1.8  1998/01/27 15:42:22  pekangas
 * Merged a bunch of highcolor and other fixes from Chi Hoang's
 * DOSDOOm 0.45
 *
 * Revision 1.7  1998/01/07 18:44:21  pekangas
 * Fixed a bunch of Visual C warnings, changed name to NTDOOM and
 * made assembler optional
 *
 * Revision 1.6  1998/01/07 17:28:16  pekangas
 * Improved mouse grab a little
 *
 * Revision 1.5  1998/01/05 14:46:35  pekangas
 * Palette is now released when the window deactivates
 *
 * Revision 1.4  1998/01/05 13:40:58  pekangas
 * Fixed I_ReadScreen for 16-bit modes
 *
 * Revision 1.3  1998/01/04 19:29:09  pekangas
 * Added hicolor and multiresulution support from Chi Hoang's DOS port
 *
 * Revision 1.2  1997/12/29 21:17:17  pekangas
 * Fixed to compile with Visual C
 * Some Win95 fixes
 *
 * Revision 1.1  1997/12/29 19:49:10  pekangas
 * Initial revision
 *
*/
