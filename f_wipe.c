//  
// DOSDoom Wipe/Melt Screen Effect Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//

#include "z_zone.h"
#include "i_video.h"
#include "v_res.h"
#include "m_random.h"

#include "dm_defs.h"
#include "dm_state.h"

#include "f_wipe.h"
#include <allegro.h>

//
// SCREEN WIPE PACKAGE
//

// when zero, stop the wipe
static boolean  go = 0;

static byte*    wipe_scr_start;
static byte*    wipe_scr_end;
static byte*    wipe_scr=NULL;


void
wipe_shittyColMajorXform
( short*        array,
  int           width,
  int           height )
{
    int         x;
    int         y;
    short*      dest;

    dest = (short*) Z_Malloc(width*height*2, PU_STATIC, 0);

    for(y=0;y<height;y++)
        for(x=0;x<width;x++)
            dest[x*height+y] = array[y*width+x];

    memcpy(array, dest, width*height*2);

    Z_Free(dest);

}

int
wipe_initColorXForm
( int   width,
  int   height,
  int   ticks )
{
    memcpy(wipe_scr, wipe_scr_start, width*height*BPP);
    return 0;
}

// -ES- 1998/11/08 Added proper xform routines with new translucency
#define WIPE_XFORM_TICS 30
int wipe_ColorXForm_tick = 0;

int wipe_doColorXForm16 (int width, int height, int ticks)
{
  unsigned short *w;
  unsigned short *e;
  unsigned short *s;

  unsigned long elevel,slevel;
  unsigned long c1,c2;

  wipe_ColorXForm_tick += ticks;
  elevel = (wipe_ColorXForm_tick*FRACUNIT)/WIPE_XFORM_TICS;
  if (elevel > FRACUNIT)
  {
    memcpy(wipe_scr_start, wipe_scr_end, width*height*BPP);
    wipe_ColorXForm_tick = 0;
    return 1;
  }

  slevel = FRACUNIT - elevel;

  slevel >>= 10;
  elevel >>= 10;

  w = (unsigned short *)wipe_scr;
  e = (unsigned short *)wipe_scr_end;
  s = (unsigned short *)wipe_scr_start;

  while (w!=(unsigned short*)wipe_scr+width*height)
  {
    // -ES- 1998/11/29 Use the new algorithm
    c1 = *s++;
    c2 = *e++;
    c1 = col2rgb16[slevel][c1&0xff][0] +
         col2rgb16[slevel][c1>>8][1]   +
         col2rgb16[elevel][c2&0xff][0] +
         col2rgb16[elevel][c2>>8][1];
    c1 &= hicolortransmask3;
    c1 |= c1>>16;
    c1 >>= hicolortransshift;

    *w++ = (short)c1;
  }

  return 0;
}

// -ES- 1998/02/12 Changed col2rgb format
int wipe_doColorXForm8 (int width, int height, int ticks)
{
  byte          *w;
  byte          *e;
  byte          *s;

  unsigned long wlevel;
  unsigned long *w2rgb,*e2rgb;
  unsigned long c;

  wipe_ColorXForm_tick += ticks;
  wlevel = (wipe_ColorXForm_tick*64)/WIPE_XFORM_TICS;
  if (wlevel > 64)
  {
    memcpy(wipe_scr_start, wipe_scr_end, width*height*BPP);
    wipe_ColorXForm_tick = 0;
    return 1;
  }

  w2rgb = col2rgb8[64-wlevel];
  e2rgb = col2rgb8[wlevel];

  w = wipe_scr;
  e = wipe_scr_end;
  s = wipe_scr_start;

  while (w!=wipe_scr+width*height)
  {
    c = (w2rgb[*s]+e2rgb[*e]) & 0xF80F0780;
    *w = rgb_8k[0][0][(0x1FFF&(c>>7)) | (c>>23)];
    e++;
    w++;
    s++;
  }

  return 0;
}

int wipe_doColorXForm (int width, int height, int ticks)
{
  return BPP == 1 ? wipe_doColorXForm8(width,height,ticks) :
                    wipe_doColorXForm16(width,height,ticks);
}

int
wipe_exitColorXForm
( int   width,
  int   height,
  int   ticks )
{
    return 0;
}


static int*     y;

int
wipe_initMelt
( int   width,
  int   height,
  int   ticks )
{
    int i, r;
    
    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width*height*BPP);
    
    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((short*)wipe_scr_start, width*BPP/2, height);
    wipe_shittyColMajorXform((short*)wipe_scr_end, width*BPP/2, height);
    
    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) Z_Malloc(width*sizeof(int), PU_STATIC, 0);
    y[0] = -(M_Random()%16);
    for (i=1;i<width;i++)
    {
        r = (M_Random()%3) - 1;
        y[i] = y[i-1] + r;
        if (y[i] > 0) y[i] = 0;
        else if (y[i] == -16) y[i] = -15;
    }

    return 0;
}

int
wipe_doMelt
( int   width,
  int   height,
  int   ticks )
{
    int         i;
    int         j;
    int         dy;
    int         idx;
    
    short*      s;
    short*      d;
    boolean     done = true;

    width=width*BPP/2;

    while (ticks--)
    {
        for (i=0;i<width;i++)
        {
            if (y[i]<0)
            {
                y[i]++; done = false;
            }
            else if (y[i] < height)
            {
                dy = (y[i] < 16) ? y[i]+1 : 8;
                if (y[i]+dy >= height) dy = height - y[i];
                s = &((short *)wipe_scr_end)[i*height+y[i]];
                d = &((short *)wipe_scr)[y[i]*width+i];
                idx = 0;
                for (j=dy;j;j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }
                y[i] += dy;
                s = &((short *)wipe_scr_start)[i*height];
                d = &((short *)wipe_scr)[y[i]*width+i];
                idx = 0;
                for (j=height-y[i];j;j--)
                {
                    d[idx] = *(s++);
                    idx += width;
                }
                done = false;
            }
        }
    }

    return done;

}

int
wipe_exitMelt
( int   width,
  int   height,
  int   ticks )
{
    Z_Free(y);
    return 0;
}

int
wipe_StartScreen
( int   x,
  int   y,
  int   width,
  int   height )
{
    wipe_scr_start = screens[2];
    I_ReadScreen(wipe_scr_start);
    return 0;
}

int
wipe_EndScreen
( int   x,
  int   y,
  int   width,
  int   height )
{
    wipe_scr_end = screens[3];
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y, 0, width, height, wipe_scr_start); // restore start scr.
    return 0;
}

int wipe_ScreenWipe 
( int   wipeno,
  int   x,
  int   y,
  int   width,
  int   height,
  int   ticks )
{
    int rc;
    static int (*wipes[])(int, int, int) =
    {
        wipe_initColorXForm, wipe_doColorXForm, wipe_exitColorXForm,
        wipe_initMelt, wipe_doMelt, wipe_exitMelt
    };

    // initial stuff
    if (!go)
    {
        go = 1;
   
     // -ES- 1998/08/20 Used Z_ReMalloc instead of Z_Malloc
        wipe_scr = (byte *) Z_ReMalloc(wipe_scr, width*height*BPP); // DEBUG
        
        (*wipes[wipeno*3])(width, height, ticks);
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, width, height);
    rc = (*wipes[wipeno*3+1])(width, height, ticks);
    V_DrawBlock(x, y, 0, width, height, wipe_scr); // DEBUG

    // final stuff
    if (rc)
    {
        go = 0;
        (*wipes[wipeno*3+2])(width, height, ticks);
    }

    return !go;

}
