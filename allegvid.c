#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <allegro.h>

//#include "doomstat.h"
//#include "m_argv.h"
#include "doomdef.h"
//#include "multires.h"
//#include "allegvid.h"

//vars
void (*flipscreens) (void);
int doublebufferflag=0;

byte *lfbscreen1,*lfbscreen2;
unsigned long screenbaseaddr;
int forcevga=0, nodblbuffer=1;

//prototypes
void select_blit_function();
void doublebufferblit(void);
void linearblit(void);
void lblblit(void);
void bankedblit(void);

int SCREENWIDTH;
int SCREENHEIGHT;
int SCREENPITCH;
int BPP;

void enter_graphics_mode()
  {
   if (M_CheckParm("-forcevga"))
    {
    if ((SCREENWIDTH==320)&&(SCREENHEIGHT<=200)&&(BPP==1))
      forcevga=1;
    }
  if (M_CheckParm("-dblbuffer"))
    nodblbuffer=0;

  select_blit_function();

  if (M_CheckParm("-allegtest"))
    {
    if (flipscreens==doublebufferblit)
      printf("Blitter: Vesa2 Double Buffering\n");
    else if (flipscreens==linearblit)
      printf("Blitter: Vesa2 Memcpy or Normal VGA\n");
    else if (flipscreens==lblblit)
      printf("Blitter: Line-by-line Blitter\n");
    else if (flipscreens==bankedblit)
      printf("Blitter: Vesa1 Banked Blit\n");
    else
      printf("Blitter: INVALID!\n");
    }
  __dpmi_get_segment_base_address(screen->seg,&screenbaseaddr);
  screenbaseaddr+=__djgpp_conventional_base;

  if (doublebufferflag==1)
    {
    lfbscreen1=screenbaseaddr+screen->line[0];
    lfbscreen2=lfbscreen1+SCREENWIDTH*BPP*SCREENHEIGHT;
    screens[0]=lfbscreen1;
    screens[5]=lfbscreen1;
    screens[6]=lfbscreen2;
    }
  else
    {
    screens[0]=(byte *)malloc(SCREENWIDTH*BPP*SCREENHEIGHT);
    }
  SCREENPITCH=SCREENHEIGHT*BPP;  //fixme
  }

void select_blit_function()
  {
  int i;

  if (BPP==1)
    {
    set_color_depth(8);
    //try vesa2 double-buffer
    i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);
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
    i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,0,0);
    if ((i==0)&&(forcevga==0))
      {
      if (((screen->line[1])-(screen->line[0]))==(SCREENWIDTH*BPP))
        flipscreens=linearblit;
      else
        flipscreens=lblblit;
      return;
      }
    //i guess were stuck with vesa1
    i=set_gfx_mode(GFX_VESA1,SCREENWIDTH,SCREENHEIGHT,0,0);
    if ((i==0)&&(forcevga==0))
      {
      flipscreens=bankedblit;
      return;
      }
    //vga is last resort
    if ((SCREENWIDTH==320)&&(SCREENHEIGHT<=200))
      {
      i=set_gfx_mode(GFX_VGA,SCREENWIDTH,SCREENHEIGHT,0,0);
      flipscreens=linearblit;
      return;
      }
    printf("Unable to Initialize %d x %d x %d Graphics Mode\n",SCREENWIDTH,SCREENHEIGHT,BPP);
    exit(0);
    }
  else
    {
    //try vesa2 double-buffer
    set_color_depth(16);
    i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);
    if (i!=0) {set_color_depth(15); i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,SCREENWIDTH,SCREENHEIGHT*2);}
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
    set_color_depth(16);
    i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,0,0);
    if (i!=0) {set_color_depth(15); i=set_gfx_mode(GFX_VESA2L,SCREENWIDTH,SCREENHEIGHT,0,0);}
    if ((i==0)&&(forcevga==0))
      {
      if (((screen->line[1])-(screen->line[0]))==(SCREENWIDTH*BPP))
        flipscreens=linearblit;
      else
        flipscreens=lblblit;
      return;
      }
    //i guess were stuck with vesa1
    set_color_depth(16);
    i=set_gfx_mode(GFX_VESA1,SCREENWIDTH,SCREENHEIGHT,0,0);
    if (i!=0) {set_color_depth(15); i=set_gfx_mode(GFX_VESA1,SCREENWIDTH,SCREENHEIGHT,0,0);}
    if ((i==0)&&(forcevga==0))
      {
      flipscreens=bankedblit;
      return;
      }
    printf("Unable to Initialize %d x %d x %d Graphics Mode\n",SCREENWIDTH,SCREENHEIGHT,BPP);
    exit(0);
    }
  }

void doublebufferblit(void)
  {
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
  }

void linearblit(void)
  {
  memcpy(screenbaseaddr+screen->line[0],screens[0],SCREENWIDTH*SCREENHEIGHT*BPP);
  }

void lblblit(void)
  {
  int i,j;
  byte *temppointer;

  temppointer=screens[0]; j=SCREENWIDTH*BPP;
  for (i=0;i<SCREENHEIGHT;i++)
    {
    memcpy(screenbaseaddr+screen->line[i],temppointer,j);
    temppointer+=SCREENWIDTH*BPP;
    }
  }

void bankedblit(void)
  {
  int i,j;
  byte *temppointer,*destpointer;

  temppointer=screens[0]; j=SCREENWIDTH*BPP;
  for (i=0;i<SCREENHEIGHT;i++)
    {
    destpointer=(byte *)(screenbaseaddr+bmp_write_line(screen,i));
    memcpy(destpointer,temppointer,j);
    temppointer+=SCREENWIDTH*BPP;
    }
  }


