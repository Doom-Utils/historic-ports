#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dpmi.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <allegro.h>

#include "doomstat.h"
#include "m_argv.h"
#include "doomdef.h"
#include "v_res.h"
#include "i_alleg.h"
#include "i_allegv.h"

#include "i_system.h"
#include "z_zone.h"

//vars
void (*flipscreens) (void);
boolean doublebufferflag=0;

boolean forcevga=0, nodblbuffer=1;
int bitspp = 8;
BITMAP *doom_s, *doom_s2;

//prototypes
void select_blit_function();
void doublebufferblit(void);
void linearblit(void);
void lblblit(void);
void bankedblit(void);
void allegblit(void);

void enter_graphics_mode()
  {
   if (M_CheckParm("-forcevga"))
    {
    if ((SCREENWIDTH==320)&&(SCREENHEIGHT<=200)&&(BPP==1))
      forcevga=true;
    }
  if (M_CheckParm("-dblbuffer")){
    nodblbuffer=false;
    retrace=0;
  }

  select_blit_function();
  if (M_CheckParm("-allegtest"))
    {
    if (flipscreens==doublebufferblit)
      I_Printf("Blitter: Vesa2 Double Buffering\n\r"); // - Kester
    else if (flipscreens==linearblit)
      I_Printf( "Blitter: Vesa2 Memcpy or Normal VGA\n\r"); // - Kester
    else if (flipscreens==lblblit)
      I_Printf( "Blitter: Line-by-line Blitter\n\r"); // - Kester
    else if (flipscreens==bankedblit)
      I_Printf( "Blitter: Vesa1 Banked Blit\n\r"); // - Kester
    else
      I_Printf( "Blitter: INVALID!\n\r"); // - Kester
    }
//  __dpmi_get_segment_base_address(screen->seg,&screenbaseaddr);
//  screenbaseaddr+=__djgpp_conventional_base;
//  clear(screen);
  if (doublebufferflag==1)
    {
/*    lfbscreen1=screenbaseaddr+screen->line[0];
    lfbscreen2=lfbscreen1+SCREENWIDTH*BPP*SCREENHEIGHT;
    screens[0]=lfbscreen1;
    screens[5]=lfbscreen1;
    screens[6]=lfbscreen2; */
    doom_s = create_video_bitmap(SCREENWIDTH, SCREENHEIGHT);
    doom_s2 = create_video_bitmap(SCREENWIDTH, SCREENHEIGHT);
    screens[0] = doom_s2->dat;
    }
  else
    {
     doom_s = create_bitmap(SCREENWIDTH, SCREENHEIGHT);
     screens[0]=doom_s->dat;
  //  memset(screens[0], 0, SCREENWIDTH*BPP*SCREENHEIGHT);

    }
/*   doom_s = malloc(sizeof(BITMAP));
   doom_s->dat = doom_s->line[0] = screens[0];
   doom_s->w = doom_s->cr = SCREENWIDTH;
   doom_s->h = doom_s->cb = SCREENHEIGHT;
   doom_s->clip = FALSE;
   doom_s->cl = doom_s->ct = 0;
   doom_s->vtable = _get_vtable(bitspp);
   doom_s->write_bank = doom_s->read_bank = _stub_bank_switch;
   doom_s->bitmap_id = 0;
   doom_s->extra = NULL;
   doom_s->x_ofs = 0;
   doom_s->y_ofs = 0;
   doom_s->seg = _my_ds();

   for (i=1; i<SCREENHEIGHT; i++)
      doom_s->line[i] = doom_s->line[i-1] + SCREENWIDTH * BPP;
  SCREENPITCH=SCREENHEIGHT*BPP;  //fixme
  */
  }
#if 0
void select_blit_function()
{
  int i;

  if (BPP==1)
  {
    set_color_depth(8);
    //try vesa2 double-buffer
    // VBEAF is written almost entirely in prot mode, so is probably
    // faster. Only supports 8 bit colour - Kester
    if (forcevga == 0) {

      if (nodblbuffer == 0) {
        i=set_gfx_mode(GFX_VBEAF, SCREENWIDTH, SCREENHEIGHT,
                                  SCREENWIDTH, SCREENHEIGHT*2);
        if (i != 0)
          i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,
                                    SCREENWIDTH,SCREENHEIGHT*2);
        if (i != 0)
          i = set_gfx_mode(GFX_AUTODETECT, SCREENWIDTH, SCREENHEIGHT, SCREENWIDTH, SCREENHEIGHT * 2);
        if (i==0) {
          if (((screen->line[1])-(screen->line[0]))==(SCREENWIDTH*BPP))
          {
            flipscreens=doublebufferblit;
            doublebufferflag=1;
            return;
          }
        }
      }

      //try raw vesa2 blit
      i=set_gfx_mode(GFX_VBEAF, SCREENWIDTH, SCREENHEIGHT, 0, 0);
      if (i != 0)
        i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,0,0);
      if (i==0)
      {
        if (((int)screen->line[1]-(int)screen->line[0])==(SCREENWIDTH*BPP))
          flipscreens=linearblit;
        else
          flipscreens=bankedblit;
        flipscreens = allegblit;
        return;
      }

      //i guess were stuck with vesa1
      i=set_gfx_mode(GFX_VESA1,SCREENWIDTH,SCREENHEIGHT,0,0);
      if (i==0)
      {
        flipscreens=bankedblit;
        return;
      }
    }
    //vga is last resort
    if ((SCREENWIDTH==320)&&(SCREENHEIGHT<=200))
      {
      i=set_gfx_mode(GFX_VGA,SCREENWIDTH,SCREENHEIGHT,0,0);
      flipscreens=linearblit;
      return;
      }
    I_Printf( "Unable to Initialize %d x %d x %d Graphics Mode\n\r",SCREENWIDTH,SCREENHEIGHT,BPP);
    allegro_exit();
    exit(0);
    }
  else
    {
    //try vesa2 double-buffer
    set_color_depth(16); bitspp = 16;
    i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);
    if (i!=0) {
     set_color_depth(15);
     bitspp = 15;
     i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);
    }
    if ((i==0)&&(forcevga==0)&&(nodblbuffer==0))
      {
      if (((screen->line[1])-(screen->line[0]))==(SCREENWIDTH*BPP))
        {
        flipscreens=doublebufferblit;
        doublebufferflag=1;
        return;
        }
      }
    //try raw vesa2 blit
    set_color_depth(16); bitspp = 16;
    i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,0,0);
    if (i!=0) {
      set_color_depth(15);
      bitspp = 15;
      i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,0,0);
    }
    if ((i==0)&&(forcevga==0))
      {
      if (((screen->line[1])-(screen->line[0]))==(SCREENWIDTH*BPP))
        flipscreens=linearblit;
      else
        flipscreens=lblblit;
      flipscreens = allegblit;
      return;
      }
    //i guess were stuck with vesa1
    set_color_depth(16); bitspp = 16;
    i=set_gfx_mode(GFX_VESA1,SCREENWIDTH,SCREENHEIGHT,0,0);
    if (i!=0) {
      set_color_depth(15);
      bitspp = 15;
      i=set_gfx_mode(GFX_VESA1,SCREENWIDTH,SCREENHEIGHT,0,0);
    }
    if ((i==0)&&(forcevga==0))
      {
      flipscreens=bankedblit;
      return;
      }
    I_Printf( "Unable to Initialize %d x %d x %d Graphics Mode\n",SCREENWIDTH,SCREENHEIGHT,BPP);
    allegro_exit();
    exit(0);
    }
  }
#else
void select_blit_function()
{
  int i;

  if (BPP==1)
  {
    set_color_depth(8);
    //try vesa2 double-buffer
    // VBEAF is written almost entirely in prot mode, so is probably
    // faster. Only supports 8 bit colour - Kester
    if (forcevga == 0) {

      if (nodblbuffer == 0) {
        i = set_gfx_mode(GFX_AUTODETECT, SCREENWIDTH, SCREENHEIGHT, SCREENWIDTH, SCREENHEIGHT * 2);
        if (i==0)
          if ((gfx_capabilities & GFX_CAN_TRIPLE_BUFFER) && ((screen->line[1] - screen->line[0]) == (SCREENWIDTH * BPP)))
          {
            flipscreens=doublebufferblit;
            doublebufferflag=1;
            return;
          }
      }

    //try raw vesa2 blit
      i=set_gfx_mode(GFX_AUTODETECT,SCREENWIDTH,SCREENHEIGHT,0,0);
      if (i==0)
      {
        if (((gfx_capabilities & GFX_HW_MEM_BLIT_MASKED) == 0) && ((screen->line[1] - screen->line[0]) == (SCREENWIDTH * BPP)))
          flipscreens = linearblit;
        else
          flipscreens = allegblit;
        return;
      }
    }
    //vga is last resort
    if ((SCREENWIDTH==320)&&(SCREENHEIGHT<=200))
      {
      i=set_gfx_mode(GFX_VGA,SCREENWIDTH,SCREENHEIGHT,0,0);
      flipscreens=allegblit;
      return;
      }
    I_Printf( "Unable to Initialize %d x %d x %d Graphics Mode\n\r",SCREENWIDTH,SCREENHEIGHT,BPP);
    allegro_exit();
    exit(0);
    }
  else
    {
    //try vesa2 double-buffer
    set_color_depth(16); bitspp = 16;
    i=set_gfx_mode(GFX_AUTODETECT,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);
    if (i!=0) {
     set_color_depth(15);
     bitspp = 15;
     i=set_gfx_mode(GFX_AUTODETECT,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);
    }
    if ((i==0)&&(forcevga==0)&&(nodblbuffer==0))
      {
      if (((screen->line[1])-(screen->line[0]))==(SCREENWIDTH*BPP))
        {
        flipscreens=doublebufferblit;
        doublebufferflag=1;
        return;
        }
      }
    //try raw vesa2 blit
    set_color_depth(16); bitspp = 16;
    i=set_gfx_mode(GFX_AUTODETECT, SCREENWIDTH,SCREENHEIGHT,0,0);
    if (i!=0) {
      set_color_depth(15);
      bitspp = 15;
      i=set_gfx_mode(GFX_AUTODETECT, SCREENWIDTH,SCREENHEIGHT,0,0);
    }
    if (i==0)
      {
      flipscreens = allegblit;
      return;
      }
    I_Printf( "Unable to Initialize %d x %d x %d Graphics Mode\n",SCREENWIDTH,SCREENHEIGHT,BPP);
    allegro_exit();
    exit(0);
    }
  }
#endif
void doublebufferblit(void)
  {
#if 0
   __dpmi_regs r;

  if (screens[0]==lfbscreen2)
    {
    screens[0]=lfbscreen1;
    r.x.ax=0x4f07; /*set display start*/
    r.x.bx=0x0000;
    r.x.cx=0;
    r.x.dx=SCREENHEIGHT;
    __dpmi_int(0x10,&r);
    }
  else
    {
    screens[0]=lfbscreen2;
    r.x.ax=0x4f07; /*set display start*/
    r.x.bx=0x0000;
    r.x.cx=0;
    r.x.dx=0;
    __dpmi_int(0x10,&r);
    }
#else
  if (screens[0] == doom_s2->dat) {
    request_video_bitmap(doom_s2->dat);
    screens[0] = doom_s->dat;
    }
  else
    {
     request_video_bitmap(doom_s->dat);
     screens[0] = doom_s->dat;
     }
#endif
  }

void linearblit(void)
  {
  //memcpy(screenbaseaddr+screen->line[0],screens[0],SCREENWIDTH*SCREENHEIGHT*BPP);
  movedata(_my_ds(), (int) screens[0],
           screen->seg, (int) screen->line[0],
           SCREENWIDTH*SCREENHEIGHT*BPP);
  }

void lblblit(void)
  {
  int i,j;
  unsigned int temppointer;

  temppointer=(unsigned int) screens[0]; j=SCREENWIDTH*BPP;
  for (i=0;i<SCREENHEIGHT;i++,temppointer+=SCREENWIDTH*BPP)
    {
     movedata(_my_ds(), temppointer, screen->seg, (unsigned int) screen->line[i], j);
//     memcpy(screenbaseaddr+screen->line[i],temppointer,j);
    }
  }

void bankedblit(void)
  {
  int i,j;
  unsigned long temppointer,destpointer;

  temppointer=(unsigned long) screens[0]; j=SCREENWIDTH*BPP;
  for (i=0;i<SCREENHEIGHT;i++)
    {
    destpointer = bmp_write_line(screen,i);
    movedata(_my_ds(), temppointer, screen->seg, destpointer, j);
//    memcpy(destpointer,temppointer,j);
    temppointer+=SCREENWIDTH*BPP;
    }
  }


void allegblit(void)
{
 if (viewheight == SCREENHEIGHT)
   blit(doom_s, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
 else {
   masked_blit(doom_s, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   clear_to_color(doom_s, (BPP == 1)? 0 : (bitspp == 15)? MASK_COLOR_15 : MASK_COLOR_16);
 }
}
