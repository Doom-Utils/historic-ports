//
// DOSDoom System Interface Video Code 
//
// Based on the DOOM Video Source Code
//
// Released by id Software (c) 1993-1996. (see DOOMLIC.TXT)
//
// -MH- 1998/07/02 Move I_DoomCode2ScanCode and I_ScanCode2DoomCode
//                 to i_system.c (and their relevant declarations to
//                 i_system.h. See i_system.c for details as to why.
//

#include <stdlib.h>
#include <allegro.h>

#include "d_debug.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "i_system.h"
#include "i_allegv.h"
#include "v_res.h"
#include "m_argv.h"
#include "w_wad.h"
#include "z_zone.h"

//dosstuff -newly added
int SCREENWIDTH;
int SCREENHEIGHT;
int SCREENPITCH;
int BPP;
int SCREENDEPTH;
fixed_t weirdaspect;
// -ES- 1998/08/20 explicit initialisation to false
boolean graphicsmode = false;

//byte *hicolortable;
//short hicolortransmask1,hicolortransmask2;
// -ES- 1998/11/29 Moved declarations here
long col2rgb16[65][256][2];
long col2rgb8[65][256];
unsigned char rgb_8k[16][32][16]; // 8K RGB table
unsigned long hicolortransmask3;
char hicolortransshift;


void calctranslucencytable();
char *translucencytable25;
char *translucencytable50;
char *translucencytable75;
//end of newly added stuff

// -KM- 1998/07/31 The disk (loading icon) is back!
static BITMAP*     the_disk = NULL;

void I_AutodetectBPP()
{
  int p;

  if (M_CheckParm("-hicolor"))
    BPP=2;

  // -ES- 1998/08/24 Added -bpp parameter: Required to override bpp=2 in config file
  p = M_CheckParm("-bpp");
  if (p && p < myargc-1)
    BPP=atoi(myargv[p+1]);
}

void I_GetResolution(void)
{

 int p;

  p=M_CheckParm("-width");
  if (p && p < myargc-1)
    SCREENWIDTH=atoi(myargv[p+1]);

  p=M_CheckParm("-height");
  if (p && p < myargc-1)
    SCREENHEIGHT=atoi(myargv[p+1]);

  // -ES- 1998/08/24 Added -res parameter: -res w h means -width w -height h
  p=M_CheckParm("-res");
  if (p && p < myargc-2)
  {
    SCREENWIDTH=atoi(myargv[p+1]);
    SCREENHEIGHT=atoi(myargv[p+2]);
  }
  // -ES- 1998/08/20 Moved away screendepth calculation to v_res.c
}

//
// I_StartFrame
//
void I_StartFrame (void)
  {
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
    static int  lasttic;
    int         tics;
    int         i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

        i = I_GetTime();
        tics = i - lasttic;
        lasttic = i;
        if (tics > 20) tics = 20;

       // -ACB- 1998/06/18 used SCREENDEPTH to replace j (SCREENWIDTH*BPP)
       for (i=0 ; i<tics*2 ; i+=2)
       {
            screens[0][ ((SCREENHEIGHT-1)*SCREENDEPTH + i*BPP)] = 0xff;
       if (BPP==2)
              screens[0][ ((SCREENHEIGHT-1)*SCREENDEPTH + i*BPP)+1] = 0xff;
       }
        for ( ; i<35*2 ; i+=2)
       {
            screens[0][ ((SCREENHEIGHT-1)*SCREENDEPTH + i*BPP)] = 0x0;
       if (BPP==2)
              screens[0][ ((SCREENHEIGHT-1)*SCREENDEPTH + i*BPP)+1] = 0x0;
       }
    }

  //Give the option of using vsync.
  if (retrace) vsync();

  //blast it to the screen
  flipscreens();
  }

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{ // -ES- 1998/08/24 SCREENDEPTH replaced SCREENWIDTH*BPP
    memcpy (scr, screens[0], SCREENDEPTH*SCREENHEIGHT);
}

static BITMAP*
GetDisk(void)
{
 patch_t*       patch;
 int            count;
 int            col;
 column_t*      column;
 byte*          source;
 int            w;
 int            y;
 patch = W_CacheLumpName("STDISK", PU_STATIC);
 the_disk = create_bitmap(patch->width, patch->height);
 clear(the_disk);

 w = patch->width;
 y = 0;

 for (col = 0; col<w ; col++)
 {
        column = (column_t *)((byte *)patch + patch->columnofs[col]);
 
        // step through the posts in a column 
        while   (column->topdelta       !=      0xff ) 
        { 
                 source = (byte *)column + 3;
                 y = column->topdelta;
                 count  = column->length;
                         
                 while (count--) 
                 {      
                  if (BPP == 2)
                    putpixel(the_disk, col, y, palette_color[*source++]);
                  else
                    putpixel(the_disk, col, y, *source++);
                  y++;
                 }      
                 column = (column_t *)((byte *)column + column->length + 4);
        } 
   }
  Z_ChangeTag(patch, PU_CACHE);
  return the_disk;
}

void I_SetPalette (byte* palette, int redness)
  {
  int c,i,j;
  PALETTE pal;
  short *tempcolormap;

  if (BPP==1)
  {
    for (i=0;i<256;i++)
    {
      c=gammatable[usegamma][*palette++];
      pal[i].r=c>>2;
      c=gammatable[usegamma][*palette++];
      pal[i].g=c>>2;
      c=gammatable[usegamma][*palette++];
      pal[i].b=c>>2;
    }
    set_palette(pal);
  } else {  //replace this with some sorta caching system?
    int tempr[256],tempg[256],tempb[256];
    int c;

    for (i=0;i<256;i++)
    {
      tempr[i]=gammatable[usegamma][*palette++];
      tempg[i]=gammatable[usegamma][*palette++];
      tempb[i]=gammatable[usegamma][*palette++];
      pal[i].r=tempr[i]>>2; pal[i].g=tempg[i]>>2; pal[i].b=tempb[i]>>2;
    }
    tempcolormap = (short *) colormaps16;
    for (i = 0; i < numcolourmaps; i++)
    {
      for (j = 0; j < 256; j++)
      {
        c = colormaps8[i * 256 + j];
        tempcolormap[i * 256 + j] = makecol(tempr[c],tempg[c], tempb[c]);
      }
    }
    set_palette(pal);
    newhupd = true;
/*
    tempcolormap=(short *)colormaps;
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
    } else {
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
      pal[i].r=tempr[i]>>2; pal[i].g=tempg[i]>>2; pal[i].b=tempb[i]>>2;
    }
    set_palette(pal);
*/
  }

  { // The disk icon can't be done in hi-color until
    // the palette is set up.
    boolean update_disk = false;
    if (!the_disk)
      update_disk = true;
    // This is in case the colour depth changes dynamically... :-)
    else if (the_disk)
      if (((bitmap_color_depth(the_disk)+1)/8) != BPP)
        update_disk = true;

    if (update_disk)
      GetDisk();
  }
}

//
// I_InitGraphics
// Inits some tables.
// -ES- 1998/09/11 Removed enter_graphics_mode call and first time stuff
void I_InitGraphics(void)
{
  //calc translucencytable if needed
  //-ES- 1998/08/20 Removed BPP==1 check, for dynamic res changing
  calctranslucencytable();
//  if (!M_CheckParm("-notrans"))
//    settingflags.trans = false;

  //do the hicolorpal table if necessary
  //-ES- 1998/08/20 Always necessary, resolution can change to hicolor
/*
  tempptr=hicolortable=(byte *)Z_Malloc(256*32*9, PU_STATIC, NULL);
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
  }*/
  // -ES- 1998/08/20 Moved away hicolortransmask calculations to enter_graphics_mode
}

void I_ShutdownGraphics(void)
{
  set_gfx_mode(GFX_TEXT,80,25,0,0);
  graphicsmode = false;
}

/* progress indicator for the translucency table calculations */
void callback_func()
  {
  static int i = 0;

  if (!(15&i++))
    I_Printf(".");
  }


/* dosdoom_create_rgb_table:
 *
 * -ES- 1998/10/29: Added this. This is a modified version of Allegro's
 * create_rgb_table, by Jan Hubicka. The only difference is that this version
 * will insert colour 0 into the table, and Allegro's version won't (since
 * that's Allegro's transparent colour).
 *
 *  Fills an RGB_MAP lookup table with conversion data for the specified
 *  palette. This is the faster version by Jan Hubicka.
 *
 *  Uses alg. similiar to foodfill - it adds one seed per every color in 
 *  palette to its best possition. Then areas around seed are filled by 
 *  same color because it is best aproximation for them, and then areas 
 *  about them etc...
 *
 *  It does just about 80000 tests for distances and this is about 100
 *  times better than normal 256*32000 tests so the caluclation time
 *  is now less than one second at all computers I tested.
 */
/* 1.5k lookup table for color matching */
static unsigned col_diff[3*128]; 
/* bestfit_init:
 *  Color matching is done with weighted squares, which are much faster
 *  if we pregenerate a little lookup table...
 */
static void bestfit_init()
{
   int i;

   for (i=1; i<64; i++) {
      int k = i * i;
      col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
      col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
      col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
   }
}
void dosdoom_create_rgb_table(RGB_MAP *table, PALLETE pal, void (*callback)(int pos))
{
   #define UNUSED 65535
   #define LAST 65532

   /* macro add adds to single linked list */
   #define add(i)    (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                     (first != LAST ? (next[last] = (i)) : (first = (i))), \
                     (last = (i))) : 0)

   /* same but w/o checking for first element */
   #define add1(i)   (next[(i)] == UNUSED ? (next[(i)] = LAST, \
                     next[last] = (i), \
                     (last = (i))) : 0)

   /* calculates distance between two colors */
   #define dist(a1, a2, a3, b1, b2, b3) \
                     (col_diff[ ((a2) - (b2)) & 0x7F] + \
                     (col_diff + 128)[((a1) - (b1)) & 0x7F] + \
                     (col_diff + 256)[((a3) - (b3)) & 0x7F])

   /* converts r,g,b to position in array and back */
   #define pos(r, g, b) \
                     (((r) / 2) * 32 * 32 + ((g) / 2) * 32 + ((b) / 2))

   #define depos(pal, r, g, b) \
                     ((b) = ((pal) & 31) * 2, \
                      (g) = (((pal) >> 5) & 31) * 2, \
                      (r) = (((pal) >> 10) & 31) * 2)

   /* is current color better than pal1? */
   #define better(r1, g1, b1, pal1) \
                     (((int)dist((r1), (g1), (b1), \
                                 (pal1).r, (pal1).g, (pal1).b)) > (int)dist2)

   /* checking of possition */
   #define dopos(rp, gp, bp, ts) \
      if ((rp > -1 || r > 0) && (rp < 1 || r < 61) && \
          (gp > -1 || g > 0) && (gp < 1 || g < 61) && \
          (bp > -1 || b > 0) && (bp < 1 || b < 61)) { \
         i = first + rp * 32 * 32 + gp * 32 + bp; \
         if (ts ? data[i] != val : !data[i]) { \
            dist2 = (rp ? (col_diff+128)[(r+2*rp-pal[val].r) & 0x7F] : r2) + \
                    (gp ? (col_diff)[(g+2*gp-pal[val].g) & 0x7F] : g2) + \
                    (bp ? (col_diff+256)[(b+2*bp-pal[val].b) & 0x7F] : b2); \
            if (better((r+2*rp), (g+2*gp), (b+2*bp), pal[data[i]])) { \
               data[i] = val; \
               add1 (i); \
            } \
         } \
      }

   int i, curr, r, g, b, val, r2, g2, b2, dist2;
   unsigned short next[32*32*32];
   unsigned char *data;
   int first = LAST;
   int last = LAST;
   int count = 0;
   int cbcount = 0;

   #define AVERAGE_COUNT   18000

   if (col_diff[1] == 0)
      bestfit_init();

   memset(next, 255, sizeof(next));
   memset(table->data, 0, sizeof(char)*32*32*32);

   data = (unsigned char *)table->data;

   /* add starting seeds for foodfill */
   // -ES- Here's the only difference between this version and Allegro's:
   // The 0 on the next line is 1 in Allegro's version.
   for (i=0; i<256; i++) {
      curr = pos(pal[i].r, pal[i].g, pal[i].b);
      if (next[curr] == UNUSED) {
         data[curr] = i;
         add(curr);
      }
   }

   /* main foodfill: two versions of loop for faster growing in blue axis */
   while (first != LAST) { 
      depos(first, r, g, b);

      /* calculate distance of current color */
      val = data[first];
      r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
      g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
      b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

      /* try to grow to all directions */
      dopos( 0, 0, 1, 1); 
      dopos( 0, 0,-1, 1);
      dopos( 1, 0, 0, 1);
      dopos(-1, 0, 0, 1);
      dopos( 0, 1, 0, 1);
      dopos( 0,-1, 0, 1);

      /* faster growing of blue direction */
      if ((b > 0) && (data[first-1] == val)) { 
         b -= 2;
         first--;
         b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

         dopos(-1, 0, 0, 0);
         dopos( 1, 0, 0, 0);
         dopos( 0,-1, 0, 0);
         dopos( 0, 1, 0, 0);

         first++;
      }

      /* get next from list */
      i = first; 
      first = next[first];
      next[i] = UNUSED;

      /* second version of loop */
      if (first != LAST) { 
         depos(first, r, g, b);

         val = data[first];
         r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
         g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
         b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

         dopos( 0, 0, 1, 1);
         dopos( 0, 0,-1, 1);
         dopos( 1, 0, 0, 1);
         dopos(-1, 0, 0, 1);
         dopos( 0, 1, 0, 1);
         dopos( 0,-1, 0, 1);

         if ((b < 61) && (data[first + 1] == val)) {
            b += 2;
            first++;
            b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7f];

            dopos(-1, 0, 0, 0);
            dopos( 1, 0, 0, 0);
            dopos( 0,-1, 0, 0);
            dopos( 0, 1, 0, 0);

            first--;
         }

         i = first;
         first = next[first];
         next[i] = UNUSED;
      }

      count++;
      if (count == (cbcount+1)*AVERAGE_COUNT/256) {
         if (cbcount < 256) {
            if (callback)
               callback(cbcount);
            cbcount++;
         }
      }
   }

   if (callback)
      while (cbcount < 256)
         callback(cbcount++);
}


RGB_MAP rgb_table;

void calctranslucencytable()
  { /* This function uses Allegro routines to create a translucency table very quickly */
  int x, y;
  unsigned char *thepalette;
  PALETTE pal;
  int i;
  int r,g,b;

  thepalette = W_CacheLumpNum (W_GetNumForName("PLAYPAL"), PU_CACHE);
//  translucencytable25=(char *)Z_Malloc(65536*3, PU_STATIC, NULL);
//  translucencytable50=translucencytable25+65536;
//  translucencytable75=translucencytable25+65536*2;

  for (i = 0; i < 256; i++) // The palette must first be converted to Allegro's format (RGB values 0-63)
  {
    pal[i].r = thepalette[i*3+0] >> 2;
    pal[i].g = thepalette[i*3+1] >> 2;
    pal[i].b = thepalette[i*3+2] >> 2;
  }
  I_Printf("Calculating translucency table\n");

   /* this isn't needed, but it speeds up the color table calculations */
   dosdoom_create_rgb_table(&rgb_table, pal, callback_func);
   rgb_map = &rgb_table;

   /* create the small RGB table */
   for (r=0; r<16; r++)
     for (g=0; g<32; g++)
       for (b=0; b<16; b++)
       {
         rgb_8k[r][g][b] = rgb_table.data[r*2][g][b*2];
       }

   /* Create the table */
#if 0
   for (x=0; x<PAL_SIZE; x++)
     {
     for (y=0; y<=x/* < PAL_SIZE*/; y++)
       {
       c.r = ((int)pal[x].r + (int)pal[y].r) >> 2;
       c.g = ((int)pal[x].g + (int)pal[y].g) >> 2;
       c.b = ((int)pal[x].b + (int)pal[y].b) >> 2;
       translucencytable50[(x<<8)+y] = rgb_map->data[c.r][c.g][c.b];
       translucencytable50[(y<<8)+x] = rgb_map->data[c.r][c.g][c.b];
       }
     callback_func(x);
     }
   for (x=0; x<PAL_SIZE; x++)
     {
     for (y=0; y<PAL_SIZE; y++)
       {
       c.r = ((int)pal[x].r + ((int)pal[y].r)*3) >> 3;
       c.g = ((int)pal[x].g + ((int)pal[y].g)*3) >> 3;
       c.b = ((int)pal[x].b + ((int)pal[y].b)*3) >> 3;
       translucencytable75[(x<<8)+y]=rgb_map->data[c.r][c.g][c.b];
       translucencytable25[(y<<8)+x]=rgb_map->data[c.r][c.g][c.b];
       }
     callback_func(x);
     }
#endif
   // -ES- 1999/02/12 Changed col2rgb format
   for (x=0; x<65; x++)
   {
     for (y=0; y<256; y++)
     {
       col2rgb8[x][y] = ((thepalette[y*3+0]*x>>5)<<11)  |
                       ((thepalette[y*3+1]*x>>2)<<20) |
                        (thepalette[y*3+2]*x>>3);
     }
   }

  rgb_map = NULL;
  I_Printf("\n");
}

// -KM- 1998/07/31 The disk is back!
void I_BeginRead(void)
{
  if (graphicsmode && the_disk)
  {
    int w, h;
    // Scale by the same amount
    w = (the_disk->w * SCREEN_W) / 320;
    h = (the_disk->h * SCREEN_H) / 200;
    stretch_blit(/* source */ the_disk, /* dest */ screen,
      /* source: x,y, w,h */  0, 0, the_disk->w, the_disk->h,
      /*   dest: x,y, w,h */ SCREEN_W - w, SCREEN_H - h, w, h);
  }
}

void I_EndRead(void)
{
}


