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
//	DOOM graphics stuff for LINUX SvgaLib
// By Colin Phipps colin_phipps@hotmail.com
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <vga.h>
#include <vgagl.h>
//#include <vgakeyboard.h>

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

int mode;
GraphicsContext* physicalscreen;

int mousepresent;
int doublebufferflag=0;

byte *hicolortable;
short hicolortransmask1,hicolortransmask2;
short palette_color[256];

//extern int usejoystick;
//extern int usemouse;

void I_CalcTranslucTbl();
char *translucencytable25;
char *translucencytable50;
char *translucencytable75;
//end of newly added stuff

//void inithicolor();

void I_AutodetectBPP()
{
  BPP=1; // Nice and easy
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
}

//static int	lastmousex = 0;
//static int	lastmousey = 0;
boolean		mousemoved = false;
//boolean		shmFinished;

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
  // I don't know either, but it works like this it seems
}

void I_FinishUpdate(void)
{
  static int	lasttic;
  int		tics;
  int		i;
  // UNUSED static unsigned char *bigscreen=0;
  
  // draws little dots on the bottom of the screen
  if (devparm) {
    i = I_GetTime();
    tics = i - lasttic;
    lasttic = i;
    if (tics > 20) tics = 20;
    
    for (i=0 ; i<tics*2 ; i+=2) {
      if (BPP==1)
	screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)] = 0xff;
      else {
	screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0xff;
	screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0xff;
      }
    }
    for ( ; i<20*2 ; i+=2) {
      if (BPP==1)
	screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)] = 0x0;
      else {
	screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2] = 0x0;
	screens[0][ ((SCREENHEIGHT-1)*SCREENWIDTH + i)*2+1] = 0x0;
      }
    }    
  }
  //blast it to the screen
  // SVGALib out
  //   just some previous attempts that I kept
  //    vga_copytoplanar256(screens[0], SCREENWIDTH, 0, SCREENWIDTH, SCREENWIDTH, SCREENHEIGHT); // this doesn't work
  //    vga_imageblt(screens[0], 0, SCREENWIDTH, SCREENHEIGHT, SCREENWIDTH); // gives rubbish
  //    for (i=0; i<SCREENHEIGHT; i++) 
  //vga_drawscansegment(&(screens[0][i*SCREENWIDTH]), 0, i, SCREENWIDTH); // Works but looks slow in code
  gl_putbox(0, 0, SCREENWIDTH, SCREENHEIGHT, screens[0]); // Good
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT*BPP);
}

#if 0
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
#endif

#include "progbar.h"
void I_SetPalette (byte* palette, int redness)
{
  int i;
  //printf("I_SetPalette");
  //  ProgBar(16);
  for (i=0; i<256; i++) {
    //if (!(i&15)) printf(pbar);
    gl_setpalettecolor(i, gammatable[usegamma][palette[3*i]] >> 2, gammatable[usegamma][palette[3*i+1]] >> 2, gammatable[usegamma][palette[3*i+2]] >> 2);
  }
  printf("\n");
}

//static int oldkbmode;
//static struct termios  oldtermios, newtermios;

void I_InitGraphics(void) {        
  static int		firsttime=1;

  if (!firsttime) return;
  firsttime = 0;

  signal(SIGINT, (void (*)(int)) I_Quit);
  // Do it
  printf("I_InitGraphics\n");
  // Screen init
  mode=-1;

#define TESTMODE(w, h) if (SCREENWIDTH==w && SCREENHEIGHT==h && BPP==1) mode=G ## w ## x ## h ## x256

  TESTMODE(320,200);
  TESTMODE(320,240);
  TESTMODE(640,480);
  TESTMODE(800,600);
  TESTMODE(1024,768);
  TESTMODE(1280,1024);
#undef TESTMODE
#define TESTMODE(w, h) if (SCREENWIDTH==w && SCREENHEIGHT==h && BPP==2) mode=G ## w ## x ## h ## x32K;
  TESTMODE(320,200);
  TESTMODE(640,480);
  TESTMODE(1024,768);
  TESTMODE(1280,1024); // ?!? I wish :->

  if (mode==-1) {
    fprintf(stderr, "    Unsupported resolution: %dx%dx%ld\n", SCREENWIDTH, SCREENHEIGHT, (long)1 << (8*BPP)); 
    I_Error("Unsupported resolution\n");
  }
  //calc translucencytable if needed
  if ((BPP==1)&&(!M_CheckParm("-notrans")))
    I_CalcTranslucTbl();
  // Initialise screen
  vga_init(); // Note: must do before kb_install (i_input.c)
  vga_setmode(mode);
  gl_setcontextvga(mode);
  physicalscreen=gl_allocatecontext();
  gl_getcontext(physicalscreen);
#if 0
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
#endif
  }

void I_ShutdownGraphics(void)
{
  gl_freecontext(physicalscreen);
  vga_setmode(TEXT);
}

struct twolongints {
  unsigned long int a;
  unsigned long int b;
};

// Blimey, this is complicated and _slow_
#ifndef FAST_BUT_BAD
void I_CalcTranslucTbl() {
  int i,j,k;
  int destr,destg,destb;
  int diffr,diffg,diffb;
  int dist,closestdist;
  unsigned char closestcolor;
  int rdifftable[512],gdifftable[512],bdifftable[512];
  unsigned char *thepalette;
  char *tempmem;
  FILE* fp;
  struct twolongints header;

  tempmem=(char *)malloc(65536*3); // Wow
  translucencytable25=tempmem;
  translucencytable50=tempmem+65536;
  translucencytable75=tempmem+65536*2;

  printf ("I_CalcTranslucTbl ");
  i=W_CheckNumForName("TRANSTBL");
  if (i!=-1) {
    if (W_LumpLength(i)==3*65536) {
      W_ReadLump(i, translucencytable25);
      printf("found TRANSTBL\n");
      return;
    }
  }
  
  thepalette=W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
  
  for (i=0;i<512;i++) {
    rdifftable[i]=((i-256)*(i-256))*54;
    gdifftable[i]=((i-256)*(i-256))*183;
    bdifftable[i]=((i-256)*(i-256))*19;
  }
  ProgBar(32);
  //first, do 50% translucency
  for (i=0;i<256;i++) { //1st color
    for (j=0;j<=i;j++) { //2nd color
      destr=256-(((int)thepalette[i*3+0])+((int)thepalette[j*3+0]))/2;
      destg=256-(((int)thepalette[i*3+1])+((int)thepalette[j*3+1]))/2;
      destb=256-(((int)thepalette[i*3+2])+((int)thepalette[j*3+2]))/2;
      closestdist=2000000000; closestcolor=i;
      for (k=0;k<256;k++) { //check with each other color
	diffr=(((int)thepalette[k*3])+destr);
	diffg=(((int)thepalette[k*3+1])+destg);
	diffb=(((int)thepalette[k*3+2])+destb);
	dist=rdifftable[diffr]+gdifftable[diffg]+bdifftable[diffb];
	if (dist<closestdist) {
	  closestdist=dist; closestcolor=k;
	}
      }
      translucencytable50[(j<<8)+i]=closestcolor;
      translucencytable50[(i<<8)+j]=closestcolor;
    }
    if ((i%16)==0) printf (pbar);
  }

  //now, do 25%/75% translucency
  for (i=0;i<256;i++) { //1st color
    for (j=0;j<256;j++) {  //2nd color
      destr=256-(((int)thepalette[i*3+0])+((int)thepalette[j*3+0])*3)/4;
      destg=256-(((int)thepalette[i*3+1])+((int)thepalette[j*3+1])*3)/4;
      destb=256-(((int)thepalette[i*3+2])+((int)thepalette[j*3+2])*3)/4;
      closestdist=2000000000; closestcolor=i;
      for (k=0;k<256;k++) { //check with each other color
	diffr=(((int)thepalette[k*3])+destr);
	diffg=(((int)thepalette[k*3+1])+destg);
	diffb=(((int)thepalette[k*3+2])+destb);
	dist=rdifftable[diffr]+gdifftable[diffg]+bdifftable[diffb];
	if (dist<closestdist) {
	  closestdist=dist; closestcolor=k;
	}
      }
      translucencytable25[(j<<8)+i]=closestcolor;
      translucencytable75[(i<<8)+j]=closestcolor;
    }
    if ((i%16)==0) printf (pbar);
  }

  for (i=0;i<256;i++) {
    translucencytable25[(i<<8)+i]=i;
    translucencytable50[(i<<8)+i]=i;
    translucencytable75[(i<<8)+i]=i;
  }
  printf ("\n");
  fp=fopen("transtbl.wad", "wb");
  if (fp==NULL) {
    printf("   failed to write translucency table to TRANSTBL.WAD\n");
    return;
  }
  fwrite("PWAD", 4, 1, fp);
  header.a=1; header.b=12+3*65536;
  fwrite(&header, 8, 1, fp);
  for (i=0; i<3*256; i++) fwrite(&(translucencytable25[i*256]), 1, 256, fp);
  header.a=12; header.b=3*65536;
  fwrite(&header, 8, 1, fp);
  fwrite("TRANSTBL", 8, 1, fp);
  fclose(fp);
  printf("    translucency info saved to TRANSTBL.WAD\n");
}
#else

void I_CalcTranslucTbl() {
  unsigned char* colorgrid;
  unsigned char* palette;
  int cl,clx,cly;
  int badcl;
  int entries;
  register unsigned char red, green, blue;
  signed int r, rx, ry, rz;
  printf("I_CalcTranslucencyTable: ");
#define TTL25 translucencytable25
#define TTL50 translucencytable50
#define TTL75 translucencytable75
  TTL25=(char*)malloc(3*256*256);
  TTL50=TTL25+256*256; TTL75=TTL50+256*256;
  colorgrid=(char*)calloc(24*12*12, sizeof(unsigned char));
  if (colorgrid==NULL) I_Error("I_InitTranslucencyTable : malloc failure");
  // actually 16*8*8 (that's it, 3D), padded to avoid clipping
  // Note : we ideally would have 256*256*256, but that is way too sparse
  // Also most VGA boards only do 64*64*64 anyway
  // Stretched in red direction for more accuracy of imp fireballs
#define PCLGRID(r,g,b) colorgrid[(12*12)*(r+4) + 12*(g+2) + b+2]
#define CLGRID(r,g,b) PCLGRID((r>>4),(g>>5),(b>>5))
  palette=W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
#define PALR(color) palette[3*color]
#define PALG(color) palette[3*color+1]
#define PALB(color) palette[3*color+2]
  // OK, so we build a 3D table of the colours
  for (entries=1, cl=0; cl<256; cl++) {
    if (CLGRID(PALR(cl), PALG(cl), PALB(cl))==0) entries++;
    CLGRID(PALR(cl), PALG(cl), PALB(cl))=cl;
  }
  printf("Density %d%% ", (100*entries)/(8*8*8));
  // Now do the scans
  ProgBar(16);
  for (badcl=0,clx=0; clx<256; clx++) {
    if (!(clx & 15)) printf(pbar);
    for (cly=0; cly<256; cly++) {
      // 25% / 75%
      red=  ((int)3*PALR(clx)+PALR(cly)) >> 6;
      green=((int)3*PALG(clx)+PALG(cly)) >> 7;
      blue= ((int)3*PALB(clx)+PALB(cly)) >> 7;
      // Now for the final killer macro
      // Messy but fast :->
#define FINDCL(red,green,blue) \
for (cl=0, r=0; r<=4 && cl==0; r++) \
for (rx=-r; rx<=r; rx++) \
for (ry=-(r>>1); ry<=(r>>1); ry++) \
for (rz=-(r>>1); rz<=(r>>1); rz++) \
if ((cl=PCLGRID(red+rx, green+ry, blue+rz))!=0) r=-10; \
if (cl==0) { cl=clx; badcl++;};

#if 0
if ((cl=PCLGRID(red,green,blue))==0) \
if ((cl=PCLGRID(red-1,green,blue))==0) \
if ((cl=PCLGRID(red+1,green,blue))==0) \
if ((cl=PCLGRID(red,green,blue-1))==0) \
if ((cl=PCLGRID(red,green,blue+1))==0) \
if ((cl=PCLGRID(red,green-1,blue))==0) \
if ((cl=PCLGRID(red,green+1,blue))==0) \
if ((cl=PCLGRID(red+1,green+1,blue))==0) \
if ((cl=PCLGRID(red-1,green+1,blue))==0) \
if ((cl=PCLGRID(red,green+1,blue+1))==0) \
if ((cl=PCLGRID(red,green+1,blue-1))==0) \
if ((cl=PCLGRID(red-1,green-1,blue))==0) \
if ((cl=PCLGRID(red+1,green-1,blue))==0) \
if ((cl=PCLGRID(red,green-1,blue-1))==0) \
if ((cl=PCLGRID(red-1,green-1,blue+1))==0) \
if ((cl=PCLGRID(red+1,green-1,blue+1))==0) \
if ((cl=PCLGRID(red-1,green+1,blue+1))==0) \
if ((cl=PCLGRID(red+1,green+1,blue+1))==0) \
if ((cl=PCLGRID(red-1,green-1,blue-1))==0) \
if ((cl=PCLGRID(red+1,green-1,blue-1))==0) \
if ((cl=PCLGRID(red-1,green+1,blue-1))==0) \
if ((cl=PCLGRID(red+1,green+1,blue-1))==0) \
{ cl=clx; badcl++; }
#endif
      FINDCL(red,green,blue);
      TTL75[(cly<<8) + clx]=cl; TTL25[(clx<<8) + cly]=cl;
      // 50%
      if (clx>cly) continue;
      red  =((int)PALR(clx)+PALR(cly)) >> 5;
      green=((int)PALG(clx)+PALG(cly)) >> 6;
      blue =((int)PALB(clx)+PALB(cly)) >> 6;
      FINDCL(red,green,blue);
      TTL50[(clx<<8) + cly]=cl;
    }
  }
  free(colorgrid);
  printf(" %d bad.\n", badcl);
}
#endif


