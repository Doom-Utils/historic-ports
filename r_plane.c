//
// DOSDoom Floor and Ceiling Rendering Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -ACB- 1998/07/13 - Changed RANGECHECK for DEVELOPERS
// -KM- 1998/09/27 Dynamic colourmaps.

#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "dm_defs.h"
#include "dm_state.h"

#include "r_local.h"
#include "r_sky.h"

planefunction_t		floorfunc;
planefunction_t		ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
// 98-7-10 KM Init to NULL so we don't need a special case Z_Malloc
int 			maxvisplanes = 128;
visplane_t*		visplanes = NULL;
int			lastvisplane;
int			floorplane;
int			ceilingplane;

// ?
int			maxopenings;

// 98-7-10 KM Init to NULL so we don't need a special case Z_Malloc
short*			openings = NULL;
short*			lastopening;

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
// 98-7-10 KM Init to NULL so we don't need a special case Z_Malloc
short			*floorclip = NULL;
short			*ceilingclip = NULL;

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
// 98-7-10 KM Init to NULL so we don't need a special case Z_Malloc
int			*spanstart = NULL;
int			*spanstop = NULL;

//
// texture mapping
//
int*                    planezlight;
fixed_t			planeheight;

fixed_t			*yslope;
// 98-7-10 KM Init to NULL so we don't need a special case Z_Malloc
fixed_t			*origyslope = NULL;
fixed_t			*distscale = NULL;
fixed_t			basexscale;
fixed_t			baseyscale;

// 98-7-10 KM Init to NULL so we don't need a special case Z_Malloc
fixed_t			*cachedheight = NULL;
fixed_t			*cacheddistance = NULL;
fixed_t			*cachedxstep = NULL;
fixed_t			*cachedystep = NULL;


void resinit_r_plane_c(void)  //called before anything else
{

}

//
// R_InitPlanes
// Only at game startup.
//
// 98-7-10 KM Prepare for dynamic screen sizing
void R_InitPlanes (void)
{
  int i;
  unsigned short *memory;

  if (!visplanes)
  {
    visplanes = Z_Malloc(maxvisplanes * sizeof(visplane_t), PU_STATIC, NULL);
    memset(visplanes, 0, maxvisplanes * sizeof(visplane_t));
  }

  if (!drawsegs)
    drawsegs  = Z_Malloc(maxdrawsegs  * sizeof(drawseg_t), PU_STATIC, NULL);

  //use calloc instead of malloc in case doom depends on global vars being
  //initialized to 0
  for (i=0;i<maxvisplanes;i++)
  {
    if (visplanes[i].top)
      memory = visplanes[i].top - 1;
    else
      memory = (unsigned short *) 0;
    memory = Z_ReMalloc(memory, (SCREENWIDTH*2 + 4) * sizeof(unsigned short));
    memset(memory, 0, (SCREENWIDTH*2+4) * sizeof(unsigned short));
    visplanes[i].top=memory+1;
    visplanes[i].bottom=memory+SCREENWIDTH+3;
  }

  maxopenings = SCREENWIDTH * 128;
  openings=(short *)Z_ReMalloc(openings, maxopenings * sizeof(short));
  memset(openings, 0, maxopenings * sizeof(short));

  floorclip=(short *)Z_ReMalloc(floorclip, SCREENWIDTH * sizeof(short));
  memset(floorclip, 0, SCREENWIDTH * sizeof(short));

  ceilingclip=(short *)Z_ReMalloc(ceilingclip, SCREENWIDTH * sizeof(short));
  memset(ceilingclip, 0, SCREENWIDTH * sizeof(short));

  spanstart=(int *)Z_ReMalloc(spanstart, SCREENHEIGHT * sizeof(int));
  memset(spanstart, 0, SCREENHEIGHT * sizeof(int));

  spanstop=(int *)Z_ReMalloc(spanstop, SCREENHEIGHT * sizeof(int));
  // -ES- 1998/08/30 This memset now clears the number of allocated bytes :-)
  memset(spanstop, 0, SCREENHEIGHT * sizeof(int));

  origyslope=(fixed_t *)Z_ReMalloc(origyslope, SCREENHEIGHT*2*sizeof(fixed_t));
  memset(origyslope, 0, SCREENHEIGHT*2*sizeof(fixed_t));
  yslope=origyslope+(SCREENHEIGHT/2);

  distscale=(fixed_t *)Z_ReMalloc(distscale, SCREENWIDTH*sizeof(fixed_t));
// -ES- 1998/09/08 Oops, this one too :-)
  memset(distscale, 0, SCREENWIDTH*sizeof(fixed_t));

  cachedheight=(fixed_t *)Z_ReMalloc(cachedheight, SCREENHEIGHT*sizeof(fixed_t));
  memset(cachedheight, 0, SCREENHEIGHT*sizeof(fixed_t));

  cacheddistance=(fixed_t *)Z_ReMalloc(cacheddistance, SCREENHEIGHT*sizeof(fixed_t));
  memset(cacheddistance, 0, SCREENHEIGHT*sizeof(fixed_t));

  cachedxstep=(fixed_t *)Z_ReMalloc(cachedxstep, SCREENHEIGHT*sizeof(fixed_t));
  memset(cachedxstep, 0, SCREENHEIGHT*sizeof(fixed_t));

  cachedystep=(fixed_t *)Z_ReMalloc(cachedystep, SCREENHEIGHT*sizeof(fixed_t));
  memset(cachedystep, 0, SCREENHEIGHT*sizeof(fixed_t));

}


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
void
R_MapPlane
( int		y,
  int		x1,
  int		x2 )
{
    angle_t	angle;
    fixed_t	distance;
    fixed_t	length;
    unsigned	index;
	
#ifdef DEVELOPERS
    if (x2 < x1 || x1<0 || x2>=viewwidth || (unsigned)y>viewheight)
    {
	I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
    }
#endif

    if (planeheight != cachedheight[y])
    {
	cachedheight[y] = planeheight;
	distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
	ds_xstep = cachedxstep[y] = FixedMul (distance,basexscale);
	ds_ystep = cachedystep[y] = FixedMul (distance,baseyscale);
    }
    else
    {
	distance = cacheddistance[y];
	ds_xstep = cachedxstep[y];
	ds_ystep = cachedystep[y];
    }
	
    length = FixedMul (distance,distscale[x1]);
    angle = (viewangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;
    ds_xfrac = viewx + FixedMul(finecosine[angle], length);
    ds_yfrac = -viewy - FixedMul(finesine[angle], length);

    if (fixedcolormap)
	ds_colormap = fixedcolormap;
    else
    {
	index = distance >> LIGHTZSHIFT;
	
	if (index >= MAXLIGHTZ )
	    index = MAXLIGHTZ-1;

	ds_colormap = colormaps + planezlight[index];
    }
	
    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc ();	
}


//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes (void)
{
    int		i;
    angle_t	angle;
    
    // opening / clipping determination
    for (i=0 ; i<viewwidth ; i++)
    {
	floorclip[i] = viewheight;
	ceilingclip[i] = -1;
    }

    lastvisplane = 0;
    lastopening = openings;
    
    // texture calculation
    memset (cachedheight, 0, SCREENHEIGHT*sizeof(fixed_t));

    // left to right mapping
    angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;
	
    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}




//
// R_FindPlane
// 23-6-98 KM Visplanes fixed
int
R_FindPlane
( fixed_t	height,
  int		picnum,
  int		lightlevel,
  int           colourmaplump,
  int           colourmap)
{
    int		check;
    visplane_t*	pl;
	
    if (picnum == skyflatnum)
    {
//	height = 0;			// all skys map together
	lightlevel = 0;
        colourmaplump = -1;
        colourmap = -1;
    }
	
    for (check = lastvisplane, pl = visplanes; check--; pl++)
    {
        // -KM- 1998/07/31 Little Optimisation here...
	if ((height == pl->height)
	    && (picnum == pl->picnum)
	    && (lightlevel == pl->lightlevel)
            && (colourmaplump == pl->colourmaplump)
            && (colourmap == pl->colourmaplump))
	      return lastvisplane - check - 1;
    }


    if (lastvisplane == maxvisplanes)
    {
        unsigned short*		memory;
        visplanes = Z_ReMalloc(visplanes, sizeof(visplane_t) * ++maxvisplanes);
        pl = &visplanes[lastvisplane];
        memory = (unsigned short *)Z_Malloc((SCREENWIDTH*2+4)*sizeof(unsigned short), PU_STATIC, NULL);
        pl->top = memory + 1;
        pl->bottom = memory + SCREENWIDTH + 3;
        memset(pl->bottom, 0, SCREENWIDTH*sizeof(unsigned short));
    }

    pl = &visplanes[lastvisplane];
    pl->height = height;
    pl->picnum = picnum;
    pl->lightlevel = lightlevel;
    pl->colourmaplump = colourmaplump;
    pl->colourmap = colourmap;
    pl->minx = SCREENWIDTH;
    pl->maxx = -1;

    // -KM- 1998/07/31 Fix here... 0xff -> -1
    memset (pl->top,-1,SCREENWIDTH*sizeof(unsigned short));

    return lastvisplane++;
}


//
// R_CheckPlane
// 23-6-98 KM Visplanes fixed
int
R_CheckPlane
( int		pl_i,
  int		start,
  int		stop )
{
    int		intrl;
    int		intrh;
    int		unionl;
    int		unionh;
    int		x;
    visplane_t* pl = &visplanes[pl_i];
	
    if (start < pl->minx)
    {
	intrl = pl->minx;
	unionl = start;
    }
    else
    {
	unionl = pl->minx;
	intrl = start;
    }
	
    if (stop > pl->maxx)
    {
	intrh = pl->maxx;
	unionh = stop;
    }
    else
    {
	unionh = pl->maxx;
	intrh = stop;
    }

    for (x=intrl ; x<= intrh ; x++)
	if (pl->top[x] != 0xffff)
	    break;

    if (x > intrh)
    {
	pl->minx = unionl;
	pl->maxx = unionh;

	// use the same one
	return pl_i;
    }
	
    if (lastvisplane == maxvisplanes)
    {
        unsigned short*		memory;
        visplanes = Z_ReMalloc(visplanes, sizeof(visplane_t) * ++maxvisplanes);
        pl = &visplanes[lastvisplane];
        memory=(unsigned short *)Z_Malloc((SCREENWIDTH*2+4)*sizeof(unsigned short), PU_STATIC, NULL);
        pl->top = memory + 1;
        pl->bottom = memory + SCREENWIDTH + 3;
        memset(pl->bottom, 0, SCREENWIDTH * sizeof(unsigned short));
        pl = &visplanes[pl_i];
    }

    // make a new visplane
    visplanes[lastvisplane].height = pl->height;
    visplanes[lastvisplane].picnum = pl->picnum;
    visplanes[lastvisplane].lightlevel = pl->lightlevel;
    visplanes[lastvisplane].colourmaplump = pl->colourmaplump;
    visplanes[lastvisplane].colourmap = pl->colourmap;
    
    pl = &visplanes[lastvisplane];
    pl->minx = start;
    pl->maxx = stop;

    memset (pl->top,-1,SCREENWIDTH*sizeof(unsigned short));

    return lastvisplane++;
}


//
// R_MakeSpans
//
void
R_MakeSpans
( int		x,
  int		t1,
  int           b1,
  int		t2,
  int		b2 )
{

#ifdef DEVELOPERS
    // This is will crash when passed to R_MapPlane when using big Heapsizes
    if (b1 == 65535 && x == 0)
       I_Error("R_MakeSpans: Andy's MapPlane Error \n");
#endif

    while (t1 < t2 && t1<=b1)
    {
	R_MapPlane (t1,spanstart[t1],x-1);
	t1++;
    }
    while (b1 > b2 && b1>=t1)
    {
        R_MapPlane (b1,spanstart[b1],x-1);
	b1--;
    }
	
    while (t2 < t1 && t2<=b2)
    {
	spanstart[t2] = x;
	t2++;
    }
    while (b2 > b1 && b2>=t2)
    {
	spanstart[b2] = x;
	b2--;
    } 
}



//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes (void)
{
    visplane_t*		pl;
    lighttable_t* tempcolourmap = fixedcolormap;
    int			light;
    int			x;
    int			i;
    int			stop;
    int			angle;
				
#ifdef DEVELOPERS
    // 23-6-98 KM Fixed this detection routine.
    // Could possibly happen
    if (lastopening - openings > maxopenings)
	I_Error ("R_DrawPlanes: opening overflow (%ld)",
		 (lastopening - openings) - maxopenings);
#endif

    for (pl = visplanes, i = lastvisplane ; i-- ; pl++)
    {
	if (pl->minx > pl->maxx)
	    continue;

	
	// sky flat
	if (pl->picnum == skyflatnum)
	{
	    dc_iscale = pspriteiscale2>>gameflags.stretchsky;
	    
	    // Sky is allways drawn full bright,
	    //  i.e. colormaps[0] is used.
	    // Because of this hack, sky is not affected
	    //  by INVUL inverse mapping.
            if (fixedcolormap == (colormaps + 33*256*sizeof(lighttable_t)))
              dc_colormap = fixedcolormap;
            else
	      dc_colormap = colormaps;
	    dc_texturemid = skytexturemid;
	    for (x=pl->minx ; x <= pl->maxx ; x++)
	    {
		dc_yl = pl->top[x];
		dc_yh = pl->bottom[x];

		if (dc_yl <= dc_yh)
		{
#ifdef SMOOTHING
                    extern byte* dc_source2;
                    extern fixed_t dc_xfrac;
                    dc_xfrac = ((viewangle+xtoviewangle[x])>>6)&0xffff;
#endif
		    angle = (viewangle + xtoviewangle[x])>>ANGLETOSKYSHIFT;
		    dc_x = x;
                    // -KM- 1998/07/31 Give skytextures the possibility of being
                    //  animated...
		    dc_source = R_GetColumn(texturetranslation[skytexture], angle);
#ifdef SMOOTHING
                    dc_source2 = R_GetColumn(texturetranslation[skytexture], angle+1);
#endif
		    colfunc ();
		}
	    }
	}
        else
        {
          // regular flat
          ds_source = W_CacheLumpNum(flatlist[flattranslation[pl->picnum]], PU_STATIC);
    
          planeheight = abs(pl->height-viewz);
          light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;
    
          if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS-1;
    
          if (light < 0)
            light = 0;
    
          if ((pl->colourmaplump >= 0) && !fixedcolormap)
            R_SetColourMap(pl->colourmaplump);
    
          if ((pl->colourmap >= 0) && !fixedcolormap)
            fixedcolormap = colormaps + pl->colourmap*256*sizeof(lighttable_t)*BPP;

          planezlight = zlight[light];
    
          pl->top[pl->maxx+1] = 0xffff;
          pl->top[pl->minx-1] = 0xffff;
    		
          stop = pl->maxx + 1;
    
          for (x=pl->minx ; x<= stop ; x++)
          {
            R_MakeSpans(x,pl->top[x-1],
              pl->bottom[x-1],
              pl->top[x],
              pl->bottom[x]);
          }

          fixedcolormap = tempcolourmap;

          if ((pl->colourmaplump >= 0) && !fixedcolormap)
            R_SetColourMap(W_GetNumForName("COLORMAP"));

          Z_ChangeTag (ds_source, PU_CACHE);
        }
    }
}
