// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_video.c,v 1.12 1998/05/03 22:40:35 killough Exp $
//
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
//
// DESCRIPTION:
//      DOOM graphics stuff
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_video.c,v 1.12 1998/05/03 22:40:35 killough Exp $";

#include "z_zone.h"  /* memory allocation wrappers -- killough */

#include <stdio.h>
#include <signal.h>
#include <allegro.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <dos.h>

#include "doomstat.h"
#include "v_video.h"
#include "d_main.h"
#include "m_bbox.h"
#include "st_stuff.h"
#include "m_argv.h"
#include "w_wad.h"
#include "r_draw.h"
#include "am_map.h"
#include "m_menu.h"
#include "wi_stuff.h"

/////////////////////////////////////////////////////////////////////////////
//
// JOYSTICK                                                  // phares 4/3/98
//
/////////////////////////////////////////////////////////////////////////////

extern int usejoystick;
extern int joystickpresent;
extern int joy_x,joy_y;
extern int joy_b1,joy_b2,joy_b3,joy_b4;

void poll_joystick(void);

// I_JoystickEvents() gathers joystick data and creates an event_t for
// later processing by G_Responder().

void I_JoystickEvents()
{
  event_t event;

  if (!joystickpresent || !usejoystick)
    return;
  poll_joystick(); // Reads the current joystick settings
  event.type = ev_joystick;
  event.data1 = 0;

  // read the button settings

  if (joy_b1)
    event.data1 |= 1;
  if (joy_b2)
    event.data1 |= 2;
  if (joy_b3)
    event.data1 |= 4;
  if (joy_b4)
    event.data1 |= 8;

  // Read the x,y settings. Convert to -1 or 0 or +1.

  if (joy_x < 0)
    event.data2 = -1;
  else if (joy_x > 0)
    event.data2 = 1;
  else
    event.data2 = 0;
  if (joy_y < 0)
    event.data3 = -1;
  else if (joy_y > 0)
    event.data3 = 1;
  else
    event.data3 = 0;

  // post what you found

  D_PostEvent(&event);
}


//
// I_StartFrame
//
void I_StartFrame (void)
{
  I_JoystickEvents(); // Obtain joystick data                 phares 4/3/98
}

/////////////////////////////////////////////////////////////////////////////
//
// END JOYSTICK                                              // phares 4/3/98
//
/////////////////////////////////////////////////////////////////////////////

//
// Keyboard routines
// By Lee Killough
// Based only a little bit on Chi's v0.2 code
//

int I_ScanCode2DoomCode (int a)
{
  switch (a)
    {
    default:   return key_ascii_table[a]>8 ? key_ascii_table[a] : a+0x80;
    case 0x7b: return KEYD_PAUSE;
    case 0x0e: return KEYD_BACKSPACE;
    case 0x48: return KEYD_UPARROW;
    case 0x4d: return KEYD_RIGHTARROW;
    case 0x50: return KEYD_DOWNARROW;
    case 0x4b: return KEYD_LEFTARROW;
    case 0x38: return KEYD_LALT;
    case 0x79: return KEYD_RALT;
    case 0x1d:
    case 0x78: return KEYD_RCTRL;
    case 0x36:
    case 0x2a: return KEYD_RSHIFT;
  }
}

// Automatic caching inverter, so you don't need to maintain two tables.
// By Lee Killough

int I_DoomCode2ScanCode (int a)
{
  static int inverse[256], cache;
  for (;cache<256;cache++)
    inverse[I_ScanCode2DoomCode(cache)]=cache;
  return inverse[a];
}

// killough 3/22/98: rewritten to use interrupt-driven keyboard queue

void I_GetEvent()
{
  extern int usemouse;   // killough 10/98
  event_t event;
  int tail;

  while ((tail=keyboard_queue.tail) != keyboard_queue.head)
    {
      int k = keyboard_queue.queue[tail];
      keyboard_queue.tail = (tail+1) & (KQSIZE-1);
      event.type = k & 0x80 ? ev_keyup : ev_keydown;
      event.data1 = I_ScanCode2DoomCode(k & 0x7f);
      D_PostEvent(&event);
    }

  if (mousepresent!=-1 && usemouse) // killough 10/98
    {
      static int lastbuttons;
      int xmickeys,ymickeys,buttons=mouse_b;
      get_mouse_mickeys(&xmickeys,&ymickeys);
      if (xmickeys || ymickeys || buttons!=lastbuttons)
        {
          lastbuttons=buttons;
          event.data1=buttons;
          event.data3=-ymickeys;
          event.data2=xmickeys;
          event.type=ev_mouse;
          D_PostEvent(&event);
        }
    }
}

//
// I_StartTic
//

void I_StartTic()
{
  I_GetEvent();
}

//
// I_UpdateNoBlit
//

void I_UpdateNoBlit (void)
{
}

// 1/25/98 killough: faster blit for Pentium, PPro and PII CPUs:
extern void ppro_blit(void *, size_t);
extern void pent_blit(void *, size_t);

extern void blast(void *destin, void *src);  // blits to VGA planar memory
extern void ppro_blast(void *destin, void *src);  // same but for PPro CPU

int use_vsync;     // killough 2/8/98: controls whether vsync is called
int page_flip;     // killough 8/15/98: enables page flipping
int hires;
BITMAP *screens0_bitmap;
boolean noblit;

static int in_graphics_mode;
static int in_page_flip, in_hires, linear;
static int scroll_offset;
static unsigned long screen_base_addr;
static unsigned destscreen;

void I_FinishUpdate(void)
{
  if (noblit || !in_graphics_mode)
    return;

  // draws little dots on the bottom of the screen
  if (devparm)
    {
      static int lasttic;
      byte *s = screens[0];

      int i = I_GetTime();
      int tics = i - lasttic;
      lasttic = i;
      if (tics > 20)
        tics = 20;
      if (in_hires)    // killough 11/98: hires support
        {
          for (i=0 ; i<tics*2 ; i+=2)
            s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i] =
              s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i+1] =
              s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i+SCREENWIDTH*2] =
              s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i+SCREENWIDTH*2+1] =
              0xff;
          for ( ; i<20*2 ; i+=2)
            s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i] =
              s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i+1] =
              s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i+SCREENWIDTH*2] =
              s[(SCREENHEIGHT-1)*SCREENWIDTH*4+i+SCREENWIDTH*2+1] =
              0x0;
        }
      else
        {
          for (i=0 ; i<tics*2 ; i+=2)
            s[(SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
          for ( ; i<20*2 ; i+=2)
            s[(SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
        }
    }

  if (in_page_flip)
    if (!in_hires)
      {
	//
	// killough 8/15/98:
	//
	// 320x200 Wait-free page-flipping for flicker-free display
	//
	// blast it to the screen

	destscreen += 0x4000;             // Move address up one page
	destscreen &= 0xffff;             // Reduce address mod 4 pages

	// Pentium Pros and above need special consideration in the
	// planar multiplexing code, to avoid partial stalls. killough

	if (cpu_family >= 6)
	  ppro_blast((byte *) __djgpp_conventional_base
		     + 0xa0000 + destscreen, *screens);
	else
	  blast((byte *) __djgpp_conventional_base  // Other CPUs, e.g. 486
		+ 0xa0000 + destscreen, *screens);

	// page flip 
	outportw(0x3d4, destscreen | 0x0c); 

        return;
      }
    else       // hires hardware page-flipping (VBE 2.0)
      scroll_offset = scroll_offset ? 0 : 400;
  else        // killough 8/15/98: no page flipping; use other methods
    if (use_vsync && !timingdemo)
      vsync(); // killough 2/7/98: use vsync() to prevent screen breaks.

  if (!linear)  // Banked mode is slower. But just in case it's needed...
    blit(screens0_bitmap, screen, 0, 0, 0, scroll_offset,
         SCREENWIDTH << hires, SCREENHEIGHT << hires);
  else
    {   // 1/16/98 killough: optimization based on CPU type
      int size =
        in_hires ? SCREENWIDTH*SCREENHEIGHT*4 : SCREENWIDTH*SCREENHEIGHT;
      byte *dascreen = screen_base_addr + screen->line[scroll_offset];
      if (cpu_family >= 6)     // PPro, PII
        ppro_blit(dascreen,size);
      else
        if (cpu_family >= 5)   // Pentium
          pent_blit(dascreen,size);
        else                   // Others
          memcpy(dascreen,*screens,size);
    }

  if (in_page_flip)  // hires hardware page-flipping (VBE 2.0)
    scroll_screen(0, scroll_offset);
}

//
// I_ReadScreen
//

void I_ReadScreen(byte *scr)
{
  int size = hires ? SCREENWIDTH*SCREENHEIGHT*4 : SCREENWIDTH*SCREENHEIGHT;

  // 1/18/98 killough: optimized based on CPU type:
  if (cpu_family >= 6)     // PPro or PII
    ppro_blit(scr,size);
  else
    if (cpu_family >= 5)   // Pentium
      pent_blit(scr,size);
    else                     // Others
      memcpy(scr,*screens,size);
}

//
// killough 10/98: init disk icon
//

int disk_icon;

static BITMAP *diskflash, *old_data;

static void I_InitDiskFlash(void)
{
  byte temp[32*32];

  if (diskflash)
    {
      destroy_bitmap(diskflash);
      destroy_bitmap(old_data);
    }

  diskflash = create_bitmap_ex(8, 16<<hires, 16<<hires);
  old_data = create_bitmap_ex(8, 16<<hires, 16<<hires);

  V_GetBlock(0, 0, 0, 16, 16, temp);
  V_DrawPatchDirect(0, 0, 0, W_CacheLumpName(M_CheckParm("-cdrom") ?
                                             "STCDROM" : "STDISK", PU_CACHE));
  V_GetBlock(0, 0, 0, 16, 16, diskflash->line[0]);
  V_DrawBlock(0, 0, 0, 16, 16, temp);
}

//
// killough 10/98: draw disk icon
//

void I_BeginRead(void)
{
  if (!disk_icon || !in_graphics_mode)
    return;

  blit(screen, old_data,
       (SCREENWIDTH-16) << hires,
       scroll_offset + ((SCREENHEIGHT-16)<<hires),
       0, 0, 16 << hires, 16 << hires);

  blit(diskflash, screen, 0, 0, (SCREENWIDTH-16) << hires,
       scroll_offset + ((SCREENHEIGHT-16)<<hires), 16 << hires, 16 << hires);
}

//
// killough 10/98: erase disk icon
//

void I_EndRead(void)
{
  if (!disk_icon || !in_graphics_mode)
    return;

  blit(old_data, screen, 0, 0, (SCREENWIDTH-16) << hires,
       scroll_offset + ((SCREENHEIGHT-16)<<hires), 16 << hires, 16 << hires);
}

void I_SetPalette(byte *palette)
{
  int i;

  if (!in_graphics_mode)             // killough 8/11/98
    return;

  if (!timingdemo)
    while (!(inportb(0x3da) & 8));

  outportb(0x3c8,0);
  for (i=0;i<256;i++)
    {
      outportb(0x3c9,gammatable[usegamma][*palette++]>>2);
      outportb(0x3c9,gammatable[usegamma][*palette++]>>2);
      outportb(0x3c9,gammatable[usegamma][*palette++]>>2);
    }
}

void I_ShutdownGraphics(void)
{
  if (in_graphics_mode)  // killough 10/98
    {
      clear(screen);

      // Turn off graphics mode
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

      in_graphics_mode = 0;
    }
}

extern boolean setsizeneeded;

//
// killough 11/98: New routine, for setting hires and page flipping
//

static void I_InitGraphicsMode(void)
{
  scroll_offset = 0;

  if (hires || !page_flip)
    {
      set_color_depth(8);     // killough 2/7/98: use allegro set_gfx_mode

      if (hires)
        {
          if (page_flip)
            if (set_gfx_mode(GFX_AUTODETECT, 640, 400, 640, 800))
              {
                warn_about_changes(S_BADVID);      // Revert to no pageflipping
                page_flip = 0;
              }
            else
              set_clip(screen, 0, 0, 640, 800);    // Allow full access

          if (!page_flip && set_gfx_mode(GFX_AUTODETECT, 640, 400, 0, 0))
            {
              hires = 0;                           // Revert to lowres
              page_flip = in_page_flip;            // Restore orig pageflipping
              warn_about_changes(S_BADVID);
              I_InitGraphicsMode();                // Start all over
              return;
            }
        }

      if (!hires)
        set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);

      linear = is_linear_bitmap(screen);

      __dpmi_get_segment_base_address(screen->seg, &screen_base_addr);
      screen_base_addr -= __djgpp_base_address;

      V_Init();
    }
  else
    {
      // killough 8/15/98:
      //
      // Page flipping code for wait-free, flicker-free display.
      //
      // This is the method Doom originally used, and was removed in the
      // Linux source. It only works for 320x200. The VGA hardware is
      // switched to planar mode, which allows 256K of VGA RAM to be
      // addressable, and allows page flipping, split screen, and hardware
      // panning.

      V_Init();

      destscreen = 0;

      //
      // VGA mode 13h
      //
      
      set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);

      // 
      // turn off chain 4 and odd/even 
      // 
      outportb(0x3c4, 4); 
      outportb(0x3c5, (inportb(0x3c5) & ~8) | 4); 
 
      // 
      // turn off odd/even and set write mode 0 
      // 
      outportb(0x3ce, 5); 
      outportb(0x3cf, inportb(0x3cf) & ~0x13);
      
      // 
      // turn off chain 4 
      // 
      outportb(0x3ce, 6); 
      outportb(0x3cf, inportb(0x3cf) & ~2); 
      
      // 
      // clear the entire buffer space, because int 10h only did 16 k / plane 
      // 
      outportw(0x3c4, 0xf02);
      blast(0xa0000 + (byte *) __djgpp_conventional_base, *screens);

      // Now we do most of this stuff again for Allegro's benefit :)
      // All that work above was just to clear the screen first.
      set_gfx_mode(GFX_MODEX, 320, 200, 320, 800);
    }

  in_graphics_mode = 1;
  in_page_flip = page_flip;
  in_hires = hires;

  setsizeneeded = true;

  I_InitDiskFlash();        // Initialize disk icon

  I_SetPalette(W_CacheLumpName("PLAYPAL",PU_CACHE));
}

void I_ResetScreen(void)
{
  if (!in_graphics_mode)
    {
      setsizeneeded = true;
      V_Init();
      return;
    }

  I_ShutdownGraphics();     // Switch out of old graphics mode

  I_InitGraphicsMode();     // Switch to new graphics mode

  if (automapactive)
    AM_Start();             // Reset automap dimensions

  ST_Start();               // Reset palette

  if (gamestate == GS_INTERMISSION)
    {
      WI_DrawBackground();
      V_CopyRect(0, 0, 1, SCREENWIDTH, SCREENHEIGHT, 0, 0, 0);
    }

  Z_CheckHeap();
}

void I_InitGraphics(void)
{
  static int firsttime=1;

  if (!firsttime)
    return;

  firsttime=0;

  check_cpu();    // 1/16/98 killough -- sets cpu_family based on CPU

#ifndef RANGECHECK
  asm("fninit");  // 1/16/98 killough -- prevents FPU exceptions
#endif

  timer_simulate_retrace(0);

  if (nodrawers) // killough 3/2/98: possibly avoid gfx mode
    return;

  //
  // enter graphics mode
  //

  atexit(I_ShutdownGraphics);

  signal(SIGINT, SIG_IGN);  // ignore CTRL-C in graphics mode

  in_page_flip = page_flip;

  I_InitGraphicsMode();    // killough 10/98

  Z_CheckHeap();
}

//----------------------------------------------------------------------------
//
// $Log: i_video.c,v $
// Revision 1.12  1998/05/03  22:40:35  killough
// beautification
//
// Revision 1.11  1998/04/05  00:50:53  phares
// Joystick support, Main Menu re-ordering
//
// Revision 1.10  1998/03/23  03:16:10  killough
// Change to use interrupt-driver keyboard IO
//
// Revision 1.9  1998/03/09  07:13:35  killough
// Allow CTRL-BRK during game init
//
// Revision 1.8  1998/03/02  11:32:22  killough
// Add pentium blit case, make -nodraw work totally
//
// Revision 1.7  1998/02/23  04:29:09  killough
// BLIT tuning
//
// Revision 1.6  1998/02/09  03:01:20  killough
// Add vsync for flicker-free blits
//
// Revision 1.5  1998/02/03  01:33:01  stan
// Moved __djgpp_nearptr_enable() call from I_video.c to i_main.c
//
// Revision 1.4  1998/02/02  13:33:30  killough
// Add support for -noblit
//
// Revision 1.3  1998/01/26  19:23:31  phares
// First rev with no ^Ms
//
// Revision 1.2  1998/01/26  05:59:14  killough
// New PPro blit routine
//
// Revision 1.1.1.1  1998/01/19  14:02:50  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
