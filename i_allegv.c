//
// DOSDoom Allegro Video Code
//
// By the DOSDoom Team.
//
// -KM- 1998/07/21 Added some debug info
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dpmi.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <sys/nearptr.h>
#include <allegro.h>

#include "d_debug.h"
#include "dm_state.h"
#include "m_argv.h"
#include "dm_defs.h"
#include "v_res.h"
#include "i_allegv.h"
#include "m_bbox.h"

#include "i_system.h"
#include "z_zone.h"

//vars
void (*flipscreens) (void);

boolean forcevga=false;
int bitspp = 8;
// -ES- 1998/08/20 Explicit init to NULL
BITMAP *doom_s = NULL;

//prototypes
static boolean select_blit_function();
static void linearblit(void);
static void lblblit(void);
static void bankedblit(void);
static void allegblit(void);
static void nearptrblit(void);
extern boolean graphicsmode;

// -KM- 1998/11/25 Should cut down code size by removing library support for 24+32bit
//   pixel modes.
DECLARE_COLOR_DEPTH_LIST(
  COLOR_DEPTH_8
  COLOR_DEPTH_15
  COLOR_DEPTH_16
);

#define BF_LINEAR 1
#define BF_NEARPTR 2

// ldb= low detail at the borders. No point yet, I'm just checking out what it looks like.
// in the future, this will get twice as fast as normal rendering.
void ldbblit(void)
{
  char *dest;
  char *src;
  unsigned long screen_base_addr;
  int x,y;
  int fw=444,fh=332;

  src=screens[0];
  __dpmi_get_segment_base_address(screen->seg, &screen_base_addr);
  dest = screen->line[0] + screen_base_addr - __djgpp_base_address;
  for (y=0; y<SCREENHEIGHT;y+=2)
  {
    for (x=0; x<SCREENWIDTH;x+=2)
    {
      dest[x]=dest[x+1]=src[x];
      dest[x+SCREENWIDTH]=dest[x+SCREENWIDTH+1]=src[x];
    }
    if (y>(SCREENHEIGHT-fh)/2 && y<(SCREENHEIGHT+fh)/2)
    {
      memcpy(dest+(SCREENWIDTH-fw)/2, src+(SCREENWIDTH-fw)/2, fw);
      memcpy(dest+SCREENWIDTH+(SCREENWIDTH-fw)/2, src+SCREENWIDTH+(SCREENWIDTH-fw)/2, fw);
    }

    dest += SCREENWIDTH*2;
    src+= SCREENWIDTH*2;
  }
}

static struct
{
  char* name;
  void (*func)();
  boolean flags;
} blitfuncs[] = {
  {"Allegro", allegblit, 0},
  {"Linear",  linearblit, BF_LINEAR},
  {"Nearptr", nearptrblit, BF_NEARPTR|BF_LINEAR},
  {"Lbl", lblblit, 0},
  {"banked", bankedblit, 0},
  {"ldb", ldbblit, BF_NEARPTR|BF_LINEAR},
};


boolean enter_graphics_mode()
{
  int p;

  // -ES- 1998/08/20 Changed select_blit_function() error handling
  if (select_blit_function())
  {
    bitspp = _color_depth;
    if (graphicsmode)
      return true;
    else
      I_Error(DDF_LanguageLookup("ModeSelErrT"), SCREENWIDTH, SCREENHEIGHT, 1<<(BPP*8));
  }

  bitspp = _color_depth;
  graphicsmode = true;

  // -ES- 1998/08/20 Moved this here

  if (BPP==2)
  {
    int c,level;
    int r,g,b;

    // -ES- 1998/11/29 Added translucency table init

    if (bitspp == 16)
    {
      for (level = 0; level <= 64; level++)
      {
        for (c = 0; c<256; c++)
        {
          b = ((c&0x1F)*level)>>1;
          g = ((c&0xE0)*level)>>6;
          col2rgb16[level][c][0] = b | (g<<21);
          g = ((c&0x07)*level)<<2;
          r = ((c&0xF8)*level)>>3;
          col2rgb16[level][c][1] = (r<<10) | (g<<21);
        }
      }
      hicolortransmask3=0xFC1F03E0;
      hicolortransshift=5;
    }
    if (bitspp == 15)
    {
      for (level = 0; level <= 64; level++)
      {
        for (c = 0; c<256; c++)
        {
          b = ((c&0x1F)*level);
          g = ((c&0xE0)*level)>>5;
          col2rgb16[level][c][0] = b | (g<<21);
          g = ((c&0x03)*level)<<3;
          r = ((c&0x7C)*level)>>3;
          col2rgb16[level][c][1] = (r<<11) | (g<<21);
        }
      }
      hicolortransmask3=0xF81F07C0;
      hicolortransshift=6;
    }
  }

  p = M_CheckParm("-blitfunc");
  if (p && p < myargc-1)
  {
    int i;
    boolean available = true;
    for (i = 0; i < sizeof(blitfuncs)/sizeof(blitfuncs[0]); i++)
       if (!strcmp(myargv[p+1], blitfuncs[i].name))
         break;
    if (blitfuncs[i].flags & BF_LINEAR)
    {
      if ((screen->line[1] - screen->line[0]) != SCREENDEPTH)
        available = false;
    }

    if (blitfuncs[i].flags & BF_NEARPTR)
    {
      available = __djgpp_nearptr_enable() ? available : false;
    }
    if (available)
      flipscreens = blitfuncs[i].func;
  }

  // -KM- 1998/07/21 Changed all this to give debug info.
#ifdef DEVELOPERS
  Debug_Printf("%s\n", gfx_driver->desc);
  if (flipscreens==linearblit)
    Debug_Printf( "Blitter: Vesa2 Memcpy or Normal VGA\n\r"); // - Kester
  else if (flipscreens==lblblit)
    Debug_Printf( "Blitter: Line-by-line Blitter\n\r"); // - Kester
  else if (flipscreens==bankedblit)
    Debug_Printf( "Blitter: Vesa1 Banked Blit\n\r"); // - Kester
  else if (flipscreens==allegblit)
    Debug_Printf( "Blitter: Allegro\n");
  else
    Debug_Printf( "Blitter: INVALID!\n\r"); // - Kester
#endif
  // -ES- 1998/08/20 Destroy bitmaps
  if (doom_s)
  {
    destroy_bitmap(doom_s);
    doom_s = NULL;
  }

  doom_s = create_bitmap(SCREENWIDTH, SCREENHEIGHT);
  screens[0]=doom_s->dat;
  doom_s->vtable->mask_color = 247;

  // -ES- 1998/11/07 Remove garbage
  memset(screens[0], 0, SCREENDEPTH*SCREENHEIGHT);

  return false;
}

// -ES- 1998/08/20 Changed exit() and return to return -1 and return 0. Also re-indented the code.
boolean select_blit_function()
{
  int i;

  if (BPP==1)
  {
    set_color_depth(8);
    //try vesa2 double-buffer
    // VBEAF is written almost entirely in prot mode, so is probably
    // faster. Only supports 8 bit colour - Kester
    if (!forcevga)
    {
      //
      // -ACB- 1998/06/18
      // Changes GFX_VESA2L to GFX_AUTODETECT - this was not the problem :(.
      //
      // Minor lesson: don't fuck about with something you don't fully understand
      i=set_gfx_mode(GFX_AUTODETECT,SCREENWIDTH,SCREENHEIGHT,0,0);
      if (i==0)
      {
        if (((gfx_capabilities & GFX_HW_MEM_BLIT_MASKED) == 0) && ((screen->line[1] - screen->line[0]) == (SCREENWIDTH * BPP)))
          flipscreens = linearblit;
        else
          flipscreens = allegblit; // -ACB- 1998/06/18 was allegblit which is buggered up 23-6-98 KM Fixed this.
        return false;
      }
    }
    //vga is last resort
    if ((SCREENWIDTH==320)&&(SCREENHEIGHT<=200))
    {
      i=set_gfx_mode(GFX_VGA,SCREENWIDTH,SCREENHEIGHT,0,0);
      flipscreens=allegblit; // -ACB- 1998/06/18 was allegblit which is buggered up 23-6-98 KM Fixed this.
      return false;
    }
    // Failed.
    return true;
  }
  else
  {
#ifdef BLUR
SCREENWIDTH <<= 1;
SCREENHEIGHT <<= 1;
#endif
    //try raw vesa2 blit
    set_color_depth(16);
    i=set_gfx_mode(GFX_AUTODETECT, SCREENWIDTH,SCREENHEIGHT,0,0);
    if (i!=0)
    {
      set_color_depth(15);
      i=set_gfx_mode(GFX_AUTODETECT, SCREENWIDTH,SCREENHEIGHT,0,0);
    }
    if (i==0)
    {
      flipscreens = allegblit;
#ifdef BLUR
SCREENWIDTH >>= 1;
SCREENHEIGHT >>= 1;
#endif
      return false;
    }
    return true;
  }
}

void lblblit(void)
{
  int i;
  unsigned int temppointer;

  temppointer=(unsigned int) screens[0]; 
  // -ACB- 1998/06/18 used SCREENDEPTH to replace j (SCREENWIDTH*BPP)
  for (i=0;i<SCREENHEIGHT;i++,temppointer+=SCREENDEPTH)
  {
     movedata(_my_ds(), temppointer, screen->seg, (unsigned int) screen->line[i], SCREENDEPTH);
  }
}

void bankedblit(void)
{
  int i;
  unsigned long temppointer,destpointer;

  temppointer=(unsigned long) screens[0];
   for (i=0;i<SCREENHEIGHT;i++)
   {
     destpointer = bmp_write_line(screen,i);
     // -ACB- 1998/06/18 used SCREENDEPTH to replace j (SCREENWIDTH*BPP)
     movedata(_my_ds(), temppointer, screen->seg, destpointer, SCREENDEPTH);
     temppointer+=SCREENDEPTH;
   }
}

// -ES- 1998/10/29 Added fading effect
// -ES- 1998/11/08 Added 16-bit version
// -ES- 1998/02/12 Changed col2rgb format
unsigned char *fade_screen = NULL;
void (*oldblitter)(void);
int fade_start,fade_progress,fade_goal;
int fade_w, fade_h, fade_bpp; // SCREENWIDTH and SCREENHEIGHT

void do_fadeblit8(fixed_t fglevel)
{
  int x,y;
  unsigned char *old_scr,*new_scr;
  unsigned long fg;
  long bglevel;
  unsigned long *fg2rgb,*bg2rgb;

  bglevel = FRACUNIT-fglevel;
  fg2rgb = col2rgb8[fglevel>>10];
  bg2rgb = col2rgb8[bglevel>>10];
  new_scr=screens[0];
  old_scr=fade_screen;
  for (y=0;y<SCREENHEIGHT;y++)
  {
    for (x=SCREENWIDTH;x>0;x--)
    {
      fg = (fg2rgb[*new_scr]+bg2rgb[*old_scr++]) & 0xF80F0780;
      *new_scr++ = rgb_8k[0][0][(0x1FFF & (fg>>7)) | (fg>>23)];
    }
  }
}
void do_fadeblit16(fixed_t fglevel)
{
  int x,y;
  unsigned short *old_scr,*new_scr;
  unsigned long fg,bg,c;
  long bglevel;

  bglevel = FRACUNIT - fglevel;
  fglevel >>= 10;
  bglevel >>= 10;
  new_scr=(short*)screens[0];
  old_scr=(short*)fade_screen;

  for (y=0;y<SCREENHEIGHT;y++)
  {
    for (x=SCREENWIDTH;x>0;x--)
    {
      // -ES- 1998/11/29 Use the new algorithm
      fg = *new_scr;
      bg = *old_scr;
      c = col2rgb16[fglevel][fg&0xff][0] +
          col2rgb16[fglevel][fg>>8][1]   +
          col2rgb16[bglevel][bg&0xff][0] +
          col2rgb16[bglevel][bg>>8][1];
      c &= hicolortransmask3;
      c |= c>>16;
      c >>= hicolortransshift;
      *new_scr = (short)c;
      new_scr++;
      old_scr++;
    }
  }
}

void fadeblit(void)
{
  long fglevel;

  fade_progress = leveltime;
  if ((fade_progress >= fade_goal) || (fade_w != SCREENWIDTH) ||
      (fade_h != SCREENHEIGHT) || (fade_bpp != BPP))
  {
    free(fade_screen);
    fade_screen = NULL;
    flipscreens = oldblitter;
    flipscreens();
    return;
  }
  fglevel = (fade_progress-fade_start)*FRACUNIT/(fade_goal-fade_start);
  if (BPP == 1)
    do_fadeblit8(fglevel);
  else
    do_fadeblit16(fglevel);
  oldblitter();
}

//
// R_StartFading
//
// Saves the current screen, and hooks in fadeblit as blitter. During the
// following (range-start) tics, the screen in its current state will
// gradually fade out from ((range-start)/range) to invisible, while the new
// screens will fade in from (start/range) to solid.
//

void R_StartFading(int start,int range)
{
  fade_w = SCREENWIDTH;
  fade_h = SCREENHEIGHT;
  fade_bpp = BPP;
  if(!fade_screen)
    fade_screen=malloc(SCREENDEPTH*SCREENHEIGHT);

  memcpy(fade_screen, screens[0], SCREENDEPTH*SCREENHEIGHT);
  if (flipscreens!=fadeblit)
  {
    oldblitter = flipscreens;
    flipscreens = fadeblit;
  }
  fade_start = leveltime-start;
  fade_progress = leveltime;
  fade_goal = range+leveltime-start;
}

#ifdef BLUR

void nearptrblit()
{}

// -ES- 1998/08/11 16-bit fast blurred blit: Dirty hack, pass half width and
// height as parameters to DOSDoom. Only 25% of the pixels are computed, and
// this function fills the rest of the pixels by interpolating between them.
// Unoptimised, but much faster than drawing all pixels without interpolating
void allegblit(void)
{
asm(
"  pushl %%ebp          ; "
"  pushl %%es           ; "
"  pushl %%esi          ; "
"  pushl %%ebx          ; "
"  pushl %%edx          ; "

"  movl %%ax, %%es      ; "
"  cld                  ; "

"  movl _SCREENHEIGHT,%%ebp        ; "
"  decl %%ebp ; "

"blur16: ; "
"  movl %%ebp,%%eax     ; "
"  addl %%eax,%%eax     ; "
"  movl (%%esp),%%edx   ; "
"  movl 4(%%esp),%%ebx   ; "
"  call %%ebx           ; "
"  movl %%eax,%%edi     ; "

"  movl %%ebp,%%esi ; "
"  shll $2,%%esi ; "
"  addl 8(%%esp),%%esi  ; "
"  movl 0(%%esi),%%esi ; "

"  movl _SCREENWIDTH,%%ecx ; "
"inner_loop: "
"movl (%%esi), %%eax    ; "
"movl %%eax, %%edx      ; "
"andl $0xf7dff7df, %%edx ; " // mask away low RGB bits
"movl %%edx,%%ebx ; "
"shrl $16, %%ebx ; "
"andl $0xffff,%%edx ; "
"addl %%edx,%%ebx ; "
"shll $15,%%ebx ; "
"andl $0xffff0000,%%ebx ; "
"orl %%edx,%%ebx ; "
"movl %%ebx, %%es:(%%edi) ; "
"addl $4, %%edi ; "
"addl $2, %%esi ; "
"decl %%ecx ; "
"jnz inner_loop ; "

"  movl %%ebp,%%eax     ; "
"  addl %%eax,%%eax     ; "
"  incl %%eax ; "
"  movl (%%esp),%%edx   ; "
"  movl 4(%%esp),%%ebx   ; "
"  call %%ebx           ; "
"  movl %%eax,%%edi     ; "

"  movl %%ebp,%%esi ; "
"  shll $2,%%esi ; "
"  addl 8(%%esp),%%esi  ; "
"  movl 0(%%esi),%%esi ; "

"  movl _SCREENWIDTH,%%ecx ; "

"pushl %%ebp ; "
"movl %%ecx,%%ebp ; "
"addl %%ebp,%%ebp ; "
"inner_loop2: "
"movl 0(%%esi), %%eax    ; "
"movl (%%esi,%%ebp), %%ebx ; "
"andl $0xf7def7df, %%eax ; " // mask away low RGB bits
"andl $0xf7def7df, %%ebx ; " // mask away low RGB bits
"addl %%ebx,%%eax ; "
"rcrl $1, %%eax ; "
"movl %%eax, %%edx      ; "
"andl $0xf7dff7df, %%edx ; " // mask away low RGB bits
"movl %%edx,%%ebx ; "
"shrl $16, %%ebx ; "
"andl $0xffff,%%edx ; "
"addl %%edx,%%ebx ; "
"shll $15,%%ebx ; "
"andl $0xffff0000,%%ebx ; "
"orl %%edx,%%ebx ; "
"movl %%ebx, %%es:(%%edi) ; "
"addl $4, %%edi ; "
"addl $2, %%esi ; "
"decl %%ecx ; "
"jnz inner_loop2 ; "
"popl %%ebp ; "

"  decl %%ebp           ; "
"  jge blur16           ; "

"  popl %%edx           ; "
"  popl %%ebx           ; "
"  popl %%esi           ; "
"  popl %%es            ; "
"  popl %%ebp           ; "

::"a" (screen->seg), "b" (screen->write_bank), "d" (screen), "S" (doom_s->line)
:"memory");
}

void linearblit(void)
{
asm(
"pushl %%es ; "
"pushl %%ds ; "
"pushl %%ebp ; "

"movl %%ax, %%ds ; "
"movl %%bx, %%es ; "
"movl _translucencytable50, %%edx ; "
"movl _translucencytable75, %%ebp ; "
"movl $0, %%eax ; "
"movl $0, %%ebx ; "

"blurtop:"
"movb -1(%%esi), %%bh; "
"movb 1(%%esi), %%bl ; "
"movb (%%edx,%%ebx), %%al ; "
"movb -640(%%esi), %%bh; "
"movb 640(%%esi), %%bl ; "
"movb (%%edx,%%ebx), %%ah ; "
"movb (%%edx,%%eax), %%al ; "
"movb 0(%%esi), %%ah; "
"movb (%%ebp,%%eax), %%al; "
"movb %%al, %%es:(%%edi) ; "
"incl %%esi ; "
"incl %%edi ; "
"decl %%ecx ; "
"jg blurtop ; "

"popl %%ebp ; "
"popl %%ds ; "
"popl %%es ; "
::
"a" (_my_ds()),
"b" (screen->seg),
"S" (screens[0]),
"D" (screen->line[1]),
"c" (SCREENWIDTH*(SCREENHEIGHT-2))
:"%eax","%ebx","%ecx","%edx","%esi","%edi");
}

#else

// 23-6-98 KM Fixed this.
void allegblit(void)
{
   blit(doom_s, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void linearblit(void)
{
  // -ES- 1998/08/24 Replaced SCREENWIDTH*BPP with SCREENDEPTH
  movedata(_my_ds(), (int) screens[0],
           screen->seg, (int) screen->line[0],
           SCREENDEPTH*SCREENHEIGHT);
}

void nearptrblit(void)
{
  char* dest;
  unsigned long screen_base_addr;
  __dpmi_get_segment_base_address(screen->seg, &screen_base_addr);
  dest = screen->line[0] + screen_base_addr - __djgpp_base_address;
  memcpy(dest, screens[0], SCREENDEPTH*SCREENHEIGHT);
}


#endif

