//  
// DOSDoom Video Code  
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#include <stdlib.h>
#include "d_debug.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "d_event.h"

#include "am_map.h"
#include "i_video.h"
#include "i_allegv.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_fixed.h"
#include "r_draw1.h"
#include "r_draw2.h"
#include "r_plane.h"
#include "r_state.h"
#include "r_things.h"
#include "v_res.h"
#include "v_video1.h"
#include "v_video2.h"

//
//v_video.c stuff
//

// Each screen is [SCREENWIDTH*SCREENHEIGHT*BPP];
// 98-7-10 KM Init to 0
byte* screens[7] = {0, 0, 0, 0, 0, 0, 0};  // screens[5] and screens[6] are the
                                           // double-buffers
int usegamma;
//int transluc=1;

fixed_t DX, DY, DXI, DYI, DY2, DYI2, SCALEDWIDTH, SCALEDHEIGHT, X_OFFSET, Y_OFFSET;

fixed_t BASEYCENTER;


// Now where did these came from?
byte gammatable[5][256]=
{
    {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
     17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
     33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
     49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
     65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
     81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
     97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
     113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
     128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
     144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
     160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
     176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
     192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
     208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
     224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
     240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255},

    {2,4,5,7,8,10,11,12,14,15,16,18,19,20,21,23,24,25,26,27,29,30,31,
     32,33,34,36,37,38,39,40,41,42,44,45,46,47,48,49,50,51,52,54,55,
     56,57,58,59,60,61,62,63,64,65,66,67,69,70,71,72,73,74,75,76,77,
     78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,
     99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
     115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,129,
     130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,
     146,147,148,148,149,150,151,152,153,154,155,156,157,158,159,160,
     161,162,163,163,164,165,166,167,168,169,170,171,172,173,174,175,
     175,176,177,178,179,180,181,182,183,184,185,186,186,187,188,189,
     190,191,192,193,194,195,196,196,197,198,199,200,201,202,203,204,
     205,205,206,207,208,209,210,211,212,213,214,214,215,216,217,218,
     219,220,221,222,222,223,224,225,226,227,228,229,230,230,231,232,
     233,234,235,236,237,237,238,239,240,241,242,243,244,245,245,246,
     247,248,249,250,251,252,252,253,254,255},

    {4,7,9,11,13,15,17,19,21,22,24,26,27,29,30,32,33,35,36,38,39,40,42,
     43,45,46,47,48,50,51,52,54,55,56,57,59,60,61,62,63,65,66,67,68,69,
     70,72,73,74,75,76,77,78,79,80,82,83,84,85,86,87,88,89,90,91,92,93,
     94,95,96,97,98,100,101,102,103,104,105,106,107,108,109,110,111,112,
     113,114,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
     129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,144,
     144,145,146,147,148,149,150,151,152,153,153,154,155,156,157,158,159,
     160,160,161,162,163,164,165,166,166,167,168,169,170,171,172,172,173,
     174,175,176,177,178,178,179,180,181,182,183,183,184,185,186,187,188,
     188,189,190,191,192,193,193,194,195,196,197,197,198,199,200,201,201,
     202,203,204,205,206,206,207,208,209,210,210,211,212,213,213,214,215,
     216,217,217,218,219,220,221,221,222,223,224,224,225,226,227,228,228,
     229,230,231,231,232,233,234,235,235,236,237,238,238,239,240,241,241,
     242,243,244,244,245,246,247,247,248,249,250,251,251,252,253,254,254,
     255},

    {8,12,16,19,22,24,27,29,31,34,36,38,40,41,43,45,47,49,50,52,53,55,
     57,58,60,61,63,64,65,67,68,70,71,72,74,75,76,77,79,80,81,82,84,85,
     86,87,88,90,91,92,93,94,95,96,98,99,100,101,102,103,104,105,106,107,
     108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
     125,126,127,128,129,130,131,132,133,134,135,135,136,137,138,139,140,
     141,142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,
     155,156,157,158,159,160,160,161,162,163,164,165,165,166,167,168,169,
     169,170,171,172,173,173,174,175,176,176,177,178,179,180,180,181,182,
     183,183,184,185,186,186,187,188,189,189,190,191,192,192,193,194,195,
     195,196,197,197,198,199,200,200,201,202,202,203,204,205,205,206,207,
     207,208,209,210,210,211,212,212,213,214,214,215,216,216,217,218,219,
     219,220,221,221,222,223,223,224,225,225,226,227,227,228,229,229,230,
     231,231,232,233,233,234,235,235,236,237,237,238,238,239,240,240,241,
     242,242,243,244,244,245,246,246,247,247,248,249,249,250,251,251,252,
     253,253,254,254,255},

    {16,23,28,32,36,39,42,45,48,50,53,55,57,60,62,64,66,68,69,71,73,75,76,
     78,80,81,83,84,86,87,89,90,92,93,94,96,97,98,100,101,102,103,105,106,
     107,108,109,110,112,113,114,115,116,117,118,119,120,121,122,123,124,
     125,126,128,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
     142,143,143,144,145,146,147,148,149,150,150,151,152,153,154,155,155,
     156,157,158,159,159,160,161,162,163,163,164,165,166,166,167,168,169,
     169,170,171,172,172,173,174,175,175,176,177,177,178,179,180,180,181,
     182,182,183,184,184,185,186,187,187,188,189,189,190,191,191,192,193,
     193,194,195,195,196,196,197,198,198,199,200,200,201,202,202,203,203,
     204,205,205,206,207,207,208,208,209,210,210,211,211,212,213,213,214,
     214,215,216,216,217,217,218,219,219,220,220,221,221,222,223,223,224,
     224,225,225,226,227,227,228,228,229,229,230,230,231,232,232,233,233,
     234,234,235,235,236,236,237,237,238,239,239,240,240,241,241,242,242,
     243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,
     251,252,252,253,254,254,255,255}
};

//
//r_draw.c stuff
//

//
// For R_DrawColumn functions...
// Source is the top of the column to scale.
//
lighttable_t* dc_colormap;
int dc_x;
int dc_yl;
int dc_yh;
fixed_t dc_iscale;
fixed_t dc_texturemid;
// -KM- 1998/11/25 Added translucency parameter.
fixed_t dc_translucency;

// first pixel in a column
byte* dc_source;

byte* dc_translation;
byte* translationtables;


//r_drawspan:
int ds_y;
int ds_x1;
int ds_x2;

lighttable_t* ds_colormap;

fixed_t ds_xfrac;
fixed_t ds_yfrac;
fixed_t ds_xstep;
fixed_t ds_ystep;

// start of a 64*64 tile image 
byte* ds_source;

// just for profiling
int dscount;

// -ES- 1998/08/20 Explicit initialisation to NULL
byte** ylookup = NULL;
int* columnofs = NULL;

// -ES- 1998/08/20 Moved away FUZZOFF define, added fuzzpos declaration.
int FUZZTABLE;
int fuzzpos;
// Fuzztable of size 64 allows us to optimise to & 63 instead of % FUZZTABLE
int fuzzoffset[]=
{
     FUZZOFF,-FUZZOFF, FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF,-FUZZOFF,
     FUZZOFF, FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF, FUZZOFF,
     FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF, FUZZOFF,
     FUZZOFF,-FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF,
     FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF,
     FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF,
     FUZZOFF, FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF, FUZZOFF,-FUZZOFF,
    -FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF, FUZZOFF, FUZZOFF,-FUZZOFF,-FUZZOFF,
     NULL
}; 

//
// align_r_draw
//
// -ES- 1998/08/14 aligns some asm inner loops
// -ACB- 1998/08/15 Made the I_Printf into Debug_Printf 
// -ES- 1998/09/11 Made some of the Debug_Printfs into File_Printf
//
// align_loop: Helper macro for use in align_r_draw ONLY. Parameters:
// loop_lbl: Loop's label name.
// endfunc: Unused function occupying at least 31 bytes, placed immediately
// after the function containing the loop
// align_offs: Loop will start at align_offs mod 32
#define align_loop(loop_lbl,end_lbl,align_offs)\
asm("movl $" loop_lbl ",%0":"=g"(start)::"memory");\
Debug_Printf("start=%p ",start);\
new_start=(char*)((((long)start+31-align_offs) & ~31) + align_offs);\
File_Printf("new_start=%p ",new_start);\
asm("movl $" end_lbl ",%0":"=g"(end_func)::"memory");\
File_Printf("end_func=%p ",end_func);\
for (i=end_func-start-1; i >= 0; i--) new_start[i] = start[i];\
dist=new_start-start;\
File_Printf("dist=%ld",dist);\
if (dist==1)\
  *start=0x90;/*only one byte to insert, nop is enough*/\
if (dist>1)\
{/*more than one byte to insert, use unconditional short jump insead*/\
  *(start)=0xEB;/*jmp rel8*/\
  *(start+1)=(char)dist-2;/*distance to jump*/\
}/*dist<1 means dist==0, nothing to insert*/\
File_Printf(", done\n");

//
// Jumps that cross the loop label won't work after the alignment. This macro
// makes such jumps work. from_lbl should be an asm label pointing to the
// instruction RIGHT AFTER the jump, and to_lbl should be the jump destination.
// The jump must be relative, to 0xDEAD0FF5 (write this "jmp 0xDEAD0FF5").
// It may be conditional or unconditional.
//
// This macro only works with the latest inner loop aligned with align_loop
// or unaligned with unalign_loop.
//
#define set_jmpdest(from_lbl,to_lbl)\
asm("movl $" to_lbl ",%0":"=g"(to)::"memory");\
Debug_Printf("  to %p ",to);\
asm("movl $" from_lbl ",%0":"=g"(from)::"memory");\
Debug_Printf("from %p",from);\
*(long*)(from - 4) = (long)(dist+to-from);\
Debug_Printf(", done\n");

//
// Like set_jmpdest, but to_lbl is an absolute, rather than relative, address
//
#define set_address(from_lbl,to_lbl)\
asm("movl $" to_lbl ",%0":"=g"(to)::"memory");\
Debug_Printf("  to %p ",to);\
asm("movl $" from_lbl ",%0":"=g"(from)::"memory");\
File_Printf("from %p",from);\
*(long*)(from - 4) = (long)(dist+to);\
File_Printf(", done\n");

void align_r_draw(void)
{
#ifdef DJGPP
  char *start;
  long dist;
  char *new_start;
  char *end_func;
  char *to;
  char *from;
  static boolean firsttime = 1;
  int loops_to_align=0;
  int i;

  // -ES- 1999/03/07 This is getting time-critical, so we only align what we need to align
  if (firsttime) // The first time we have to align all the routines
  {
    loops_to_align=-1;
    firsttime=0;
  } else // Otherwise we just align the ones we use
  {
    if (cpu.RDC->func == R_DrawColumn8_K6_MMX)
      loops_to_align |= 0x1;
    if (cpu.RDC->func == R_DrawColumn8_id_Erik)
      loops_to_align |= 0x2;
    if (cpu.RDC->func == R_DrawColumn8_id)
      loops_to_align |= 0x4;
    if (cpu.RDS->func == R_DrawSpan8_MMX)
      loops_to_align |= 0x8;
    if (cpu.RDS->func == R_DrawSpan8_id_Erik)
      loops_to_align |= 0x10;
    if (cpu.RDS->func == R_DrawSpan8_id)
      loops_to_align |= 0x20;
  }

  if (loops_to_align & 0x1)
  {
    align_loop("rdc8mloop","R_DrawColumn8_K6_MMX_end",0);
    set_jmpdest("rdc8moffs1","rdc8mdone");
    set_jmpdest("rdc8moffs2","rdc8mdone");
  }

  if (loops_to_align & 0x2)
  {
    align_loop("rdc8eloop","R_DrawColumn8_id_Erik_end",0);
    set_jmpdest("rdc8eoffs1","rdc8edone");
    set_jmpdest("rdc8eoffs2","rdc8eloop");
    set_jmpdest("rdc8eoffs3","rdc8echecklast");
    set_address("rdc8epatcher1","rdc8epatch1-4");
    set_address("rdc8epatcher2","rdc8epatch2-4");
    set_address("rdc8epatcher3","rdc8epatch3-4");
    set_address("rdc8epatcher4","rdc8epatch4-4");
  }

  if (loops_to_align & 0x4)
  {
    // -ES- 1998/08/20 Fixed id alignment
    align_loop("rdc8iloop","R_DrawColumn8_id_end",0);
    set_jmpdest("rdc8ioffs1","rdc8idone");
    set_jmpdest("rdc8ioffs2","rdc8iloop");
    set_jmpdest("rdc8ioffs3","rdc8ichecklast");
    set_address("rdc8ipatcher1","rdc8ipatch1-4");
    set_address("rdc8ipatcher2","rdc8ipatch2-4");
  }

  if (loops_to_align & 0x8)
  {
    align_loop("rds8mloop","R_DrawSpan8MMX_end",16);
    set_jmpdest("rds8moffs1","rds8mdone");
  }

  if (loops_to_align & 0x10)
  {
    align_loop("rds8eloop","R_DrawSpan8_id_Erik_end",0);
    set_jmpdest("rds8eoffs1","rds8edone");
    set_jmpdest("rds8eoffs2","rds8eloop");
    set_jmpdest("rds8eoffs3","rds8echecklast");
    set_address("rds8epatcher1","rds8epatch1-4");
    set_address("rds8epatcher2","rds8epatch2-4");
    set_address("rds8epatcher3","rds8epatch3-4");
    set_address("rds8epatcher4","rds8epatch4-4");
  }

  if (loops_to_align & 0x20)
  {
    align_loop("rds8iloop","R_DrawSpan8_id_end",0);
    set_jmpdest("rds8ioffs1","rds8idone");
    set_jmpdest("rds8ioffs2","rds8iloop");
    set_jmpdest("rds8ioffs3","rds8ichecklast");
    set_address("rds8ipatcher1","rds8ipatch1-4");
    set_address("rds8ipatcher2","rds8ipatch2-4");
  }
#endif
}

//
// Unalign
//
// Needed when changing resolution run-time: The resinit_r_draw_c function
// uses some labels to patch screenwidth, so the code must be restored.
//
// unalign_loop: Restores a loop aligned with align_loop. Pass exactly the
// same parameters you passed to align_loop.
// If you want the unaligned code to be runnable without first re-aligning it,
// you could use the same set_jmpdests as you used to align.
#define unalign_loop(loop_lbl,end_lbl,align_offs)\
asm("movl $" loop_lbl ",%0":"=g"(start)::"memory");\
Debug_Printf("start=%p ",start);\
new_start=(char*)((((long)start+31-align_offs) & ~31) + align_offs);\
File_Printf("new_start=%p ",new_start);\
asm("movl $" end_lbl ",%0":"=g"(end_func)::"memory");\
File_Printf("end_func=%p ",end_func);\
for (i=0; i<end_func-start; i++) start[i] = new_start[i];\
dist=0;\
File_Printf(", done\n");
void unalign_r_draw(void)
{
#ifdef DJGPP
  char *start;
  long dist;
  char *new_start;
  char *end_func;
  char *to;
  char *from;
  int loops_to_unalign=0;
  int i;

  // -ES- 1999/03/07 This is getting a bit time-critical, so we only align the routines we need to align
  if (cpu.RDC->func == R_DrawColumn8_K6_MMX)
    loops_to_unalign |= 0x1;
  if (cpu.RDC->func == R_DrawColumn8_id_Erik)
    loops_to_unalign |= 0x2;
  if (cpu.RDC->func == R_DrawColumn8_id)
    loops_to_unalign |= 0x4;
  if (cpu.RDS->func == R_DrawSpan8_MMX)
    loops_to_unalign |= 0x8;
  if (cpu.RDS->func == R_DrawSpan8_id_Erik)
    loops_to_unalign |= 0x10;
  if (cpu.RDS->func == R_DrawSpan8_id)
    loops_to_unalign |= 0x20;

  if (loops_to_unalign & 0x1)
  {
    unalign_loop("rdc8mloop","R_DrawColumn8_K6_MMX_end",0);
    set_jmpdest("rdc8moffs1","rdc8mdone");
    set_jmpdest("rdc8moffs2","rdc8mdone");
  }

  if (loops_to_unalign & 0x2)
  {
    unalign_loop("rdc8eloop","R_DrawColumn8_id_Erik_end",0);
    set_jmpdest("rdc8eoffs1","rdc8edone");
    set_jmpdest("rdc8eoffs2","rdc8eloop");
    set_jmpdest("rdc8eoffs3","rdc8echecklast");
    set_address("rdc8epatcher1","rdc8epatch1-4");
    set_address("rdc8epatcher2","rdc8epatch2-4");
    set_address("rdc8epatcher3","rdc8epatch3-4");
    set_address("rdc8epatcher4","rdc8epatch4-4");
  }

  if (loops_to_unalign & 0x4)
  {
    // -ES- 1998/08/20 Fixed alignment of id routines
    unalign_loop("rdc8iloop","R_DrawColumn8_id_end",0);
    set_jmpdest("rdc8ioffs1","rdc8idone");
    set_jmpdest("rdc8ioffs2","rdc8iloop");
    set_jmpdest("rdc8ioffs3","rdc8ichecklast");
    set_address("rdc8ipatcher1","rdc8ipatch1-4");
    set_address("rdc8ipatcher2","rdc8ipatch2-4");
  }

  if (loops_to_unalign & 0x8)
  {
    unalign_loop("rds8mloop","R_DrawSpan8MMX_end",16);
    set_jmpdest("rds8moffs1","rds8mdone");
  }

  if (loops_to_unalign & 0x10)
  {
    unalign_loop("rds8eloop","R_DrawSpan8_id_Erik_end",0);
    set_jmpdest("rds8eoffs1","rds8edone");
    set_jmpdest("rds8eoffs2","rds8eloop");
    set_jmpdest("rds8eoffs3","rds8echecklast");
    set_address("rds8epatcher1","rds8epatch1-4");
    set_address("rds8epatcher2","rds8epatch2-4");
    set_address("rds8epatcher3","rds8epatch3-4");
    set_address("rds8epatcher4","rds8epatch4-4");
  }

  if (loops_to_unalign & 0x20)
  {
    unalign_loop("rds8iloop","R_DrawSpan8_id_end",0);
    set_jmpdest("rds8ioffs1","rds8idone");
    set_jmpdest("rds8ioffs2","rds8iloop");
    set_jmpdest("rds8ioffs3","rds8ichecklast");
    set_address("rds8ipatcher1","rds8ipatch1-4");
    set_address("rds8ipatcher2","rds8ipatch2-4");
  }
#endif
}

static void multires_setres(void)
{
  int i;
  fixed_t s1, s2;

  // -ES- 1998/08/20 Moved away resolution autodetection to V_MultiResInit
  weirdaspect=(SCREENHEIGHT<<FRACBITS)/SCREENWIDTH;

  // -ES- 1999/03/04 Removed weird aspect ratio warning - bad ratios don't look awful anymore :-)

  s1 = SCREENWIDTH*FRACUNIT/320;
  s2 = SCREENHEIGHT*FRACUNIT/200;

  if (s2 < s1)
    s1 = s2;

  SCALEDWIDTH = (SCREENWIDTH-(SCREENWIDTH%320));
  SCALEDHEIGHT = 200*(SCALEDWIDTH/320);

  // -KM- 1999/01/31 Add specific check for this: resolutions such as 640x350
  //  used to fail.
  if (SCALEDHEIGHT > SCREENHEIGHT)
  {
    SCALEDHEIGHT = (SCREENHEIGHT-(SCREENHEIGHT%200));
    SCALEDWIDTH = 320*(SCALEDHEIGHT/200);
  }

  // -ES- 1999/03/29 Allow very low resolutions
  if (SCALEDWIDTH<320 || SCALEDHEIGHT<200)
  {
    SCALEDWIDTH=SCREENWIDTH;
    SCALEDHEIGHT=SCREENHEIGHT;
    X_OFFSET=Y_OFFSET=0;
  } else
  {
    X_OFFSET = (SCREENWIDTH - SCALEDWIDTH) / (2 * (SCALEDWIDTH / 320) );
    Y_OFFSET = (SCREENHEIGHT - SCALEDHEIGHT) / (2 * (SCALEDHEIGHT / 200) );
  }

  //
  // Weapon Centering
  // Calculates the weapon height, relative to the aspect ratio.
  //
  // Moved here from a #define in r_things.c  -ACB- 1998/08/04
  //
  // -ES- 1999/03/04 Better psprite scaling
  BASEYCENTER = 100*FRACUNIT;

  // -KM- 1998/07/31 Cosmetic indenting
  I_Printf("  Scaled Resolution: %d x %d\n",SCALEDWIDTH,SCALEDHEIGHT);

  DX  = (SCALEDWIDTH << 16) / 320;
  DXI = (320 << 16) / SCALEDWIDTH;
  DY  = (SCALEDHEIGHT << 16) / 200;
  DYI = (200 << 16) / SCALEDHEIGHT;
  DY2 = DY / 2;
  DYI2 = DYI * 2;

  // -ES- 1998/08/20 realloc instead of malloc
  columnofs = realloc(columnofs,SCREENWIDTH * sizeof(int));
  ylookup   = realloc(ylookup,SCREENHEIGHT * sizeof(byte *));

  //allocate all the res-dependant vars
  unalign_r_draw();
  align_r_draw();
  unalign_r_draw();
  resinit_r_plane_c();
  resinit_am_map_c();
  resinit_r_draw_c();
  align_r_draw();

  // -ES- 1998/08/20 realloc instead of calloc
  negonearray=(short *)realloc(negonearray,SCREENWIDTH*sizeof(short));
  for (i=0 ; i<SCREENWIDTH ; i++)
    negonearray[i] = -1;

  xtoviewangle=(angle_t *)realloc(xtoviewangle,(SCREENWIDTH+1)*sizeof(angle_t));
  for (i=0 ; i<SCREENWIDTH+1 ; i++)
    xtoviewangle[i] = 0;

  screenheightarray=(short *)realloc(screenheightarray,SCREENWIDTH*sizeof(short));
  for (i=0 ; i<SCREENWIDTH ; i++)
    screenheightarray[i] = 0;
}

struct VideoFunc_s
{
  void (*V_Init) (void);
  
  void (*V_CopyRect)
  ( int         srcx,
    int         srcy,
    int         srcscrn,
    int         width,
    int         height,
    int         destx,
    int         desty,
    int         destscrn );
  
  
  void (*V_DrawPatch) ( int x, int y, int       scrn, patch_t* patch);
  void (*V_DrawPatchDirect) ( int       x, int y, int scrn, patch_t* patch );
  void (*V_DrawPatchFlipped) ( int x, int       y, int scrn, patch_t* patch );
  void (*V_DrawPatchShrink) ( int x, int y, int scrn, patch_t* patch );
  void (*V_DrawPatchTrans) ( int x, int y, int index ,int       scrn, patch_t* patch );
  
  void (*V_DrawPatchInDirect) ( int x, int y, int scrn, patch_t*        patch );
  void (*V_DrawPatchInDirectFlipped) ( int x, int y, int scrn, patch_t* patch );
  void (*V_DrawPatchInDirectTrans) ( int x, int y, int index, int       scrn, patch_t* patch );
  
  // Draw a linear block of pixels into the view buffer.
  void (*V_DrawBlock) ( int x, int y, int       scrn, int width, int height, byte* src );
  
  // Reads a linear block of pixels into the view buffer.
  void (*V_GetBlock) ( int x, int       y, int scrn, int width, int height, byte* dest );
  
  void (*V_MarkRect) ( int x, int y, int width, int height );
  
  void (*V_DarkenScreen)(int scrn);
  
  // 98-7-10 KM Reduce code reduncany
  void (*V_TextureBackScreen)(char *flatname);
  
  void (*resinit_r_draw_c) (void);
  
  void (*R_DrawColumn) (void);
  void (*R_DrawFuzzColumn) (void);
  void (*R_DrawTranslatedColumn) (void);
  void (*R_DrawTranslucentTranslatedColumn) (void);
  
  void (*R_VideoErase) ( unsigned       ofs, int count );
  
  void (*R_DrawSpan) (void);
  
  void (*R_InitBuffer) ( int width, int height );
  void (*R_InitTranslationTables) (void);
  void (*R_FillBackScreen) (void);
  void (*R_DrawViewBorder) (void);
  
  void (*R_DrawTranslucentColumn) (void);
  
}
VideoFunc[] = {
  {
    V_Init8,
    V_CopyRect8,
  
    V_DrawPatch8,
    V_DrawPatchDirect8,
    V_DrawPatchFlipped8,
    V_DrawPatchShrink8,
    V_DrawPatchTrans8,

    V_DrawPatchInDirect8,
    V_DrawPatchInDirectFlipped8,
    V_DrawPatchInDirectTrans8,

    V_DrawBlock8,
    V_GetBlock8,
    V_MarkRect8,
    V_DarkenScreen8,

    // 98-7-10 KM Reduce code redundancy
    V_TextureBackScreen8,

    resinit_r_draw_c8,
    R_DrawColumn8_Rasem,
    R_DrawFuzzColumn8,
    R_DrawTranslatedColumn8,
    R_DrawTranslucentTranslatedColumn8,
    R_VideoErase8,
    R_DrawSpan8_Rasem,
    R_InitBuffer8,
    R_InitTranslationTables8,
    R_FillBackScreen8,
    R_DrawViewBorder8,
    R_DrawTranslucentColumn8,
  },
  {
    V_Init16,
    V_CopyRect16,

    V_DrawPatch16,
    V_DrawPatchDirect16,
    V_DrawPatchFlipped16,
    V_DrawPatchShrink16,
    V_DrawPatchTrans16,
    
    V_DrawPatchInDirect16,
    V_DrawPatchInDirectFlipped16,
    V_DrawPatchInDirectTrans16,

    V_DrawBlock16,
    V_GetBlock16,
    V_MarkRect16,
    V_DarkenScreen16,

    // 98-7-10 KM Reduce code redundancy
    V_TextureBackScreen16,
    resinit_r_draw_c16,
    R_DrawColumn16_Rasem,
    R_DrawFuzzColumn16,
    R_DrawTranslatedColumn16,
    R_DrawTranslucentTranslatedColumn16,
    R_VideoErase16,
    R_DrawSpan16_Rasem,
    R_InitBuffer16,
    R_InitTranslationTables16,
    R_FillBackScreen16,
    R_DrawViewBorder16,
    R_DrawTranslucentColumn16,
  }
};


static void multires_setbpp(void)
{
  int i,p;
  col_func *RDC;
  span_func *RDS;

  // -ES- 1998/08/20 Moved away BPP autodetect to V_MultiResInit
  i = BPP - 1;

  //okay, set all the function pointers
  V_Init=VideoFunc[i].V_Init;
  V_CopyRect=VideoFunc[i].V_CopyRect;
  V_DrawPatch=VideoFunc[i].V_DrawPatch;
  V_DrawPatchFlipped=VideoFunc[i].V_DrawPatchFlipped;
  V_DrawPatchDirect=VideoFunc[i].V_DrawPatchDirect;
  V_DrawPatchInDirect=VideoFunc[i].V_DrawPatchInDirect;
  V_DrawPatchInDirectFlipped=VideoFunc[i].V_DrawPatchInDirectFlipped;
  V_DrawPatchTrans=VideoFunc[i].V_DrawPatchTrans;
  V_DrawPatchInDirectTrans=VideoFunc[i].V_DrawPatchInDirectTrans;
  V_DrawPatchShrink=VideoFunc[i].V_DrawPatchShrink;
  V_DrawBlock=VideoFunc[i].V_DrawBlock;
  V_GetBlock=VideoFunc[i].V_GetBlock;
  V_MarkRect=VideoFunc[i].V_MarkRect;
  V_DarkenScreen=VideoFunc[i].V_DarkenScreen;

  // 98-7-10 KM Reduce code redundancy
  V_TextureBackScreen=VideoFunc[i].V_TextureBackScreen;

  resinit_r_draw_c=VideoFunc[i].resinit_r_draw_c;

  R_DrawFuzzColumn=VideoFunc[i].R_DrawFuzzColumn;
  R_DrawTranslatedColumn=VideoFunc[i].R_DrawTranslatedColumn;
  R_DrawTranslucentTranslatedColumn=VideoFunc[i].R_DrawTranslucentTranslatedColumn;
  R_VideoErase=VideoFunc[i].R_VideoErase;
  R_DrawSpan=VideoFunc[i].R_DrawSpan;
  R_InitBuffer=VideoFunc[i].R_InitBuffer;
  R_InitTranslationTables=VideoFunc[i].R_InitTranslationTables;
  R_FillBackScreen=VideoFunc[i].R_FillBackScreen;
  R_DrawViewBorder=VideoFunc[i].R_DrawViewBorder;
  R_DrawTranslucentColumn=VideoFunc[i].R_DrawTranslucentColumn;

  // -ES- 1998/08/05 Routine choice depending on CPU type
  // -ES- 1998/08/13 Added -list
  if (M_CheckParm("-list"))
  {
    I_Printf("Available 8-bit column routines:\n");
    for (RDC = RDC8_Head; RDC; RDC = RDC->next)
      I_Printf(" %s %s\n",RDC->name, (RDC->mmx) ? "(requires MMX)":"");
    I_Printf("Available 16-bit column routines:\n");
    for (RDC = RDC16_Head; RDC; RDC = RDC->next)
      I_Printf(" %s %s\n",RDC->name, (RDC->mmx) ? "(requires MMX)":"");
    I_Printf("Available 8-bit span routines:\n");
    for (RDS = RDS8_Head; RDS; RDS = RDS->next)
      I_Printf(" %s %s\n",RDS->name, (RDS->mmx) ? "(requires MMX)":"");
    I_Printf("Available 16-bit span routines:\n");
    for (RDS = RDS16_Head; RDS; RDS = RDS->next)
      I_Printf(" %s %s\n",RDS->name, (RDS->mmx) ? "(requires MMX)":"");
    exit(0);
  }

  if (BPP==1)
  {
    RDC_Head = RDC8_Head;
    cpu.RDC=cpu.model->RDC8;
    RDS_Head = RDS8_Head;
    cpu.RDS=cpu.model->RDS8;
  } else
  {
    RDC_Head = RDC16_Head;
    cpu.RDC=cpu.model->RDC16;
    RDS_Head = RDS16_Head;
    cpu.RDS=cpu.model->RDS16;
  }

  p = M_CheckParm ("-UseCol");
  if (p && p < myargc-1)
  {
    RDC = RDC_Head;
    while (RDC)
    {
      if ( !strcasecmp(myargv[p+1], RDC->name) )
         break;
      RDC = RDC->next;
    }
    if (!RDC) // Routine doesn't exist, exit
      I_Error("multires_setbpp: No %d-bit column drawing routine called \"%s\"",4<<BPP,myargv[p+1]);
    if (RDC->mmx && !cpu.mmx)
      I_Error("multires_setbpp: CPU doesn't support MMX, used by column routine \"%s\"",myargv[p+1]);
    cpu.RDC = RDC;
  }
  R_DrawColumn = cpu.RDC->func;
  I_Printf("Using %d-bit %s column drawing routine\n",4<<BPP,cpu.RDC->name);

  p = M_CheckParm ("-UseSpan");
  if (p && p < myargc-1)
  {
    RDS = RDS_Head;
    while (RDS)
    {
      if ( !strcasecmp(myargv[p+1], RDS->name) )
         break;
      RDS = RDS->next;
    }
    if (!RDS) // Routine doesn't exist, exit
      I_Error("multires_setbpp: No %d-bit span drawing routine called \"%s\"",4<<BPP,myargv[p+1]);
    if (RDS->mmx && !cpu.mmx)
      I_Error("multires_setbpp: CPU doesn't support MMX, used by span routine \"%s\"",myargv[p+1]);
    cpu.RDS = RDS;
  }
  R_DrawSpan = cpu.RDS->func;
  I_Printf("Using %d-bit %s span drawing routine\n",4<<BPP,cpu.RDS->name);

  // -ES- 1998/08/20 Moved this here
  // -ACB- 1998/06/18 used SCREENDEPTH to be a global variable for SCREENWIDTH*BPP
  SCREENDEPTH = SCREENWIDTH*BPP;
}

//
// V_InitResolution
// Inits everything resolution-dependent to SCREENWIDTH x SCREENHEIGHT x BPP
//
// -ES- 1998/08/20 Added this
//
void V_InitResolution(void)
{
  multires_setbpp();
  multires_setres();
  V_Init();
}

//
// V_MultiResInit
// Called once at startup to initialise first V_InitResolution
//
void V_MultiResInit(void)
{
  // -ES- 1998/08/20 Moved some autodetect stuff here
  I_AutodetectBPP();
  I_GetResolution();

  // -KM- 1999/01/31 Forcevga actually forces 320x200x256c VGA.
  forcevga = M_CheckParm("-forcevga");
  if (forcevga)
  {
    SCREENWIDTH = 320;
    SCREENHEIGHT = 200;
    BPP = 1;
  }

  // -KM- 1998/07/31 Nice cosmetic change...
  I_Printf("  Resolution: %d x %d x %dc\n",SCREENWIDTH,SCREENHEIGHT,1<<(BPP*8));

  // multires_setres unaligns first of all, so the routines must first be aligned
  align_r_draw();
}

// -KM- 1998/07/21 This func clears around the edge of a scaled pic
void V_ClearPageBackground(int scrn)
{
  int y;
  int leftoffset = BPP * (SCREENWIDTH - SCALEDWIDTH) / 2;
  int topoffset = (SCREENHEIGHT - SCALEDHEIGHT) / 2;

  if (SCALEDHEIGHT < SCREENHEIGHT)
  {
    // Clear top of screen & bottom of screen
    memset(screens[scrn],
           0,
           topoffset * SCREENDEPTH);
    memset(screens[scrn] + SCREENDEPTH * (SCALEDHEIGHT + topoffset),
           0,
           SCREENDEPTH * topoffset);
  }

  if (SCALEDWIDTH < SCREENWIDTH)
  {
    // Clear the first black bit on the left
    memset(screens[scrn] + SCREENDEPTH * topoffset,
           0,
           leftoffset);
    // Because the right edge of a screen line runs into the left of the next
    // screenline, we can clear them all with one fell swoop.
    for (y = (topoffset + 1) * SCREENDEPTH - leftoffset;
         y < (SCREENHEIGHT - topoffset - 1) * SCREENDEPTH;
         y+= SCREENDEPTH)
             memset(screens[scrn] + y, 0, leftoffset * 2);

    // Clear the last end bit at the right
    memset(screens[scrn] + (SCREENHEIGHT - topoffset) * SCREENDEPTH - leftoffset,
    0,
    leftoffset);
  }
}

