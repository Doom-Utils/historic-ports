//
// DOSDoom Main Rendering Organisation Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -KM- 1998/09/27 Dynamic Colourmaps
//
#include <math.h>

#include "dm_defs.h"
#include "dm_state.h"
#include "d_net.h"

#include "m_bbox.h"
#include "m_menu.h"

#include "r_local.h"
#include "r_sky.h"
#include "st_stuff.h"

#include "i_system.h"

// -ES- 1998/08/20 Need some stuff for ChangeResolution
#include "r_main.h"
#include "i_allegv.h"
#include "i_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW		2048	

fixed_t keylookspeed=1000/64;
int invertmouse=0;

//int viewangleoffset;

// increment every time a check is made
int validcount = 1;

// -KM- 1998/09/27 Dynamic colourmaps
lighttable_t* fixedcolormap;
extern int* walllights;

int centerx;
int centery;

fixed_t centerxfrac;
fixed_t	centeryfrac;
fixed_t	projection;

// just for profiling purposes
int			framecount;	

int			sscount;
int			linecount;

fixed_t	viewx;
fixed_t	viewy;
fixed_t	viewz;

angle_t viewangle;

fixed_t	viewcos;
fixed_t viewsin;

player_t* viewplayer;

//
// precalculated math tables
//
angle_t			clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X. 
int viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
// -ES- 1998/08/20 Explicit init to NULL
angle_t	*xtoviewangle = NULL;

fixed_t* finecosine = &finesine[FINEANGLES/4];

// -KM- 1998/09/27 Dynamic colourmaps
int scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
int scalelightfixed[MAXLIGHTSCALE];
int zlight[LIGHTLEVELS][MAXLIGHTZ];

// bumped light from gun blasts
int extralight;

void (*colfunc) (void);
void (*basecolfunc) (void);
void (*fuzzcolfunc) (void);
void (*transcolfunc) (void);
void (*spanfunc) (void);

//
// R_AddPointToBox
//
// Expand a given bbox so that it encloses a given point.
//
void R_AddPointToBox (int x, int y, fixed_t* box)
{
    if (x< box[BOXLEFT])
	box[BOXLEFT] = x;
    if (x> box[BOXRIGHT])
	box[BOXRIGHT] = x;
    if (y< box[BOXBOTTOM])
	box[BOXBOTTOM] = y;
    if (y> box[BOXTOP])
	box[BOXTOP] = y;
}


//
// R_PointOnSide
//
// Traverse BSP (sub) tree, check point against partition plane.
//
// Returns side 0 (front) or 1 (back).
//
int R_PointOnSide (fixed_t x, fixed_t y, node_t* node)
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	left;
    fixed_t	right;
	
    if (!node->dx)
    {
	if (x <= node->x)
	    return node->dy > 0;
	
	return node->dy < 0;
    }
    if (!node->dy)
    {
	if (y <= node->y)
	    return node->dx < 0;
	
	return node->dx > 0;
    }
	
    dx = (x - node->x);
    dy = (y - node->y);
	
    // Try to quickly decide by looking at sign bits.
    if ( (node->dy ^ node->dx ^ dx ^ dy)&0x80000000 )
    {
	if  ( (node->dy ^ dx) & 0x80000000 )
	{
	    // (left is negative)
	    return 1;
	}
	return 0;
    }

    left = FixedMul ( node->dy>>FRACBITS , dx );
    right = FixedMul ( dy , node->dx>>FRACBITS );
	
    if (right < left)
    {
	// front side
	return 0;
    }
    // back side
    return 1;			
}

int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t* line)
{
    fixed_t	lx;
    fixed_t	ly;
    fixed_t	ldx;
    fixed_t	ldy;
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	left;
    fixed_t	right;
	
    lx = line->v1->x;
    ly = line->v1->y;
	
    ldx = line->v2->x - lx;
    ldy = line->v2->y - ly;
	
    if (!ldx)
    {
	if (x <= lx)
	    return ldy > 0;
	
	return ldy < 0;
    }
    if (!ldy)
    {
	if (y <= ly)
	    return ldx < 0;
	
	return ldx > 0;
    }
	
    dx = (x - lx);
    dy = (y - ly);
	
    // Try to quickly decide by looking at sign bits.
    if ( (ldy ^ ldx ^ dx ^ dy)&0x80000000 )
    {
	if  ( (ldy ^ dx) & 0x80000000 )
	{
	    // (left is negative)
	    return 1;
	}
	return 0;
    }

    left = FixedMul ( ldy>>FRACBITS , dx );
    right = FixedMul ( dy , ldx>>FRACBITS );
	
    if (right < left)
    {
	// front side
	return 0;
    }
    // back side
    return 1;			
}


//
// R_PointToAngle
//
// To get a global angle from cartesian coordinates,
// the coordinates are flipped until they are in
// the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a
// tangent (slope) value which is looked up in the
// tantoangle[] table.
//
angle_t R_PointToAngle ( fixed_t x, fixed_t y )
{	
    x -= viewx;
    y -= viewy;
    
    if ( (!x) && (!y) )
	return 0;

    if (x>= 0)
    {
	// x >=0
	if (y>= 0)
	{
	    // y>= 0

	    if (x>y)
	    {
		// octant 0
		return tantoangle[ SlopeDiv(y,x)];
	    }
	    else
	    {
		// octant 1
		return ANG90-1-tantoangle[ SlopeDiv(x,y)];
	    }
	}
	else
	{
	    // y<0
	    y = -y;

	    if (x>y)
	    {
		// octant 8
		return -tantoangle[SlopeDiv(y,x)];
	    }
	    else
	    {
		// octant 7
		return ANG270+tantoangle[ SlopeDiv(x,y)];
	    }
	}
    }
    else
    {
	// x<0
	x = -x;

	if (y>= 0)
	{
	    // y>= 0
	    if (x>y)
	    {
		// octant 3
		return ANG180-1-tantoangle[ SlopeDiv(y,x)];
	    }
	    else
	    {
		// octant 2
		return ANG90+ tantoangle[ SlopeDiv(x,y)];
	    }
	}
	else
	{
	    // y<0
	    y = -y;

	    if (x>y)
	    {
		// octant 4
		return ANG180+tantoangle[ SlopeDiv(y,x)];
	    }
	    else
	    {
		 // octant 5
		return ANG270-1-tantoangle[ SlopeDiv(x,y)];
	    }
	}
    }
    return 0;
}

angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2 )
{	
    viewx = x1;
    viewy = y1;
    
    return R_PointToAngle (x2, y2);
}

fixed_t R_PointToDist (fixed_t x, fixed_t y)
{
    int		angle;
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	temp;
    fixed_t	dist;
	
    dx = abs(x - viewx);
    dy = abs(y - viewy);
	
    if (dy>dx)
    {
	temp = dx;
	dx = dy;
	dy = temp;
    }
	
    angle = (tantoangle[ FixedDiv(dy,dx)>>DBITS ]+ANG90) >> ANGLETOFINESHIFT;

    // use as cosine
    dist = FixedDiv (dx, finesine[angle] );
	
    return dist;
}

//
// R_ScaleFromGlobalAngle
//
// Returns the texture mapping scale
// for the current line (horizontal span)
// at the given angle.
//
// rw_distance must be calculated first.
//
fixed_t R_ScaleFromGlobalAngle (angle_t visangle)
{
    fixed_t		scale;
    int			anglea;
    int			angleb;
    int			sinea;
    int			sineb;
    fixed_t		num;
    int			den;

    anglea = ANG90 + (visangle-viewangle);
    angleb = ANG90 + (visangle-rw_normalangle);

    // both sines are always positive
    sinea = finesine[anglea>>ANGLETOFINESHIFT];
    sineb = finesine[angleb>>ANGLETOFINESHIFT];
    num = FixedMul(projection,sineb)<<detailshift;
    den = FixedMul(rw_distance,sinea);

    if (den > num>>16)
    {
	scale = FixedDiv (num, den);

	if (scale > 64*FRACUNIT)
	    scale = 64*FRACUNIT;
	else if (scale < 256)
	    scale = 256;
    }
    else
	scale = 64*FRACUNIT;
	
    return scale;
}

//
// R_InitTextureMapping
//
void R_InitTextureMapping (void)
{
    int			i;
    int			x;
    int			t;
    fixed_t		focallength;
    
    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    //
    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.
    focallength = FixedDiv (centerxfrac,
			    finetangent[FINEANGLES/4+FIELDOFVIEW/2] );
	
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
	if (finetangent[i] > FRACUNIT*2)
	    t = -1;
	else if (finetangent[i] < -FRACUNIT*2)
	    t = viewwidth+1;
	else
	{
	    t = FixedMul (finetangent[i], focallength);
	    t = (centerxfrac - t+FRACUNIT-1)>>FRACBITS;

	    if (t < -1)
		t = -1;
	    else if (t>viewwidth+1)
		t = viewwidth+1;
	}
	viewangletox[i] = t;
    }
    
    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.	
    for (x=0;x<=viewwidth;x++)
    {
	i = 0;
	while (viewangletox[i]>x)
	    i++;
	xtoviewangle[x] = (i<<ANGLETOFINESHIFT)-ANG90;
    }
    
    // Take out the fencepost cases from viewangletox.
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
	t = FixedMul (finetangent[i], focallength);
	t = centerx - t;
	
	if (viewangletox[i] == -1)
	    viewangletox[i] = 0;
	else if (viewangletox[i] == viewwidth+1)
	    viewangletox[i]  = viewwidth;
    }
	
    clipangle = xtoviewangle[0];
}

//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP		2

void R_InitLightTables (void)
{
    int		i;
    int		j;
    int		level;
    int		startmap; 	
    int		scale;
    
    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
	startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
	for (j=0 ; j<MAXLIGHTZ ; j++)
	{
//	    scale = FixedDiv ((SCREENWIDTH/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
	    scale = FixedDiv ((320/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
	    scale >>= LIGHTSCALESHIFT;
	    level = startmap - scale/DISTMAP;
	    
	    if (level < 0)
		level = 0;

	    if (level >= NUMCOLORMAPS)
		level = NUMCOLORMAPS-1;

	    zlight[i][j] = level*256*BPP;
	}
    }
}



//
// R_SetViewSize
//
// Do not really change anything here,
// because it might be in the middle of a refresh.
//
// The change will take effect next refresh.
//
boolean		setsizeneeded;
int		setblocks;
int		setdetail;
boolean         setresfailed = false;

void R_SetViewSize (int	 blocks, int detail)
{
    setsizeneeded = true;
    setblocks = blocks;
    setdetail = detail;
}

//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize (void)
{
    fixed_t	cosadj;
    fixed_t	dy;
    int		i;
    int		j;
    int		level;
    int		startmap; 	

    setsizeneeded = false;

    if (setblocks == 11)
    {
	scaledviewwidth = SCREENWIDTH;
	viewheight = SCREENHEIGHT;
    }
    else
    {
	scaledviewwidth = (setblocks*SCREENWIDTH/10)&~7;
	// Fixes 400x300 etc.
	viewheight=(setblocks<10)?(setblocks*(SCREENHEIGHT-ST_HEIGHT)/10)&~7:
                                  SCREENHEIGHT-ST_HEIGHT;
    }
    
    viewwidth = scaledviewwidth>>detailshift;
	
    centery = viewheight/2;
    centerx = viewwidth/2;
    centerxfrac = centerx<<FRACBITS;
    centeryfrac = centery<<FRACBITS;
    projection = centerxfrac;

    colfunc = basecolfunc = R_DrawColumn;
    fuzzcolfunc = R_DrawFuzzColumn;
    transcolfunc = R_DrawTranslatedColumn;
    spanfunc = R_DrawSpan;

    R_InitBuffer (scaledviewwidth, viewheight);
	
    R_InitTextureMapping ();
    
    pspritescale = FRACUNIT*viewwidth/320;
    pspriteiscale = FRACUNIT*320/viewwidth;
    pspritescale2 = FRACUNIT*viewheight/200;
    pspriteiscale2 = FRACUNIT*200/viewheight;

    // thing clipping
    for (i=0 ; i<viewwidth ; i++)
	screenheightarray[i] = viewheight;
    
    // planes
    for (i=0 ; i<(viewheight*2) ; i++)
    {
	dy = ((i-viewheight)<<FRACBITS)+FRACUNIT/2;
	dy = abs(dy);
	origyslope[i] = FixedDiv ( (viewwidth<<detailshift)/2*FRACUNIT, dy);
        yslope=origyslope+(viewheight/2);
    }
	
    for (i=0 ; i<viewwidth ; i++)
    {
	cosadj = abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
	distscale[i] = FixedDiv (FRACUNIT,cosadj);
    }
    
    // Calculate the light levels to use
    //  for each level / scale combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
	startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
	for (j=0 ; j<MAXLIGHTSCALE ; j++)
	{
	    level = startmap - j*SCREENWIDTH/(viewwidth<<detailshift)/DISTMAP;
	    
	    if (level < 0)
		level = 0;

	    if (level >= NUMCOLORMAPS)
		level = NUMCOLORMAPS-1;

	    scalelight[i][j] = level*256*BPP;
	}
    }
}

//
// R_ChangeResolution
// Makes R_ExecuteChangeResolution execute at start of next refresh
//
// -ES- 1998/08/20 Added this
//
boolean changeresneeded = false;
int setwidth,setheight,setbpp;

void R_ChangeResolution (int width, int height, int bpp)
{
  changeresneeded = true;
  setsizeneeded = true; // need to re-init some stuff

  setwidth = width;
  setheight = height;
  setbpp = bpp;
}


//
// R_ExecuteChangeResolution
//
// Do the resolution change
//
// -ES- 1998/08/20 Added this
//
boolean R_DoExecuteChangeResolution()
{
  changeresneeded = false;
  BPP = setbpp;
  SCREENWIDTH = setwidth;
  SCREENHEIGHT = setheight;

  colormaps = (BPP==1) ? colormaps8 : colormaps16;
  V_InitResolution ();
  R_InitPlanes ();
  R_InitLightTables ();
  R_InitSkyMap ();
  R_InitTranslationTables ();
  ST_ReInit ();
  if (enter_graphics_mode ())
    return true;

  I_SetPalette(W_CacheLumpName("PLAYPAL",PU_CACHE),0);
  return false;
}

void R_ExecuteChangeResolution ()
{
  int oldBPP = BPP, oldSCREENWIDTH = SCREENWIDTH, oldSCREENHEIGHT = SCREENHEIGHT;
  if (R_DoExecuteChangeResolution())
  {
    setbpp = oldBPP;
    setwidth = oldSCREENWIDTH;
    setheight = oldSCREENHEIGHT;
    setsizeneeded = true;
    if (R_DoExecuteChangeResolution())
      I_Error(DDF_LanguageLookup("ModeSelErrT"), SCREENWIDTH, SCREENHEIGHT, 1<<(BPP*8));
    setresfailed = true;
    return;
  }
  setresfailed = false;
}

//
// R_Init
//
extern int	detailLevel;
extern int	screenblocks;

void R_Init (void)
{
    R_InitData ();
    I_Printf(".");
    // viewwidth / viewheight / detailLevel are set by the defaults
    I_Printf(".");
    R_SetViewSize (screenblocks, detailLevel);
    R_InitPlanes ();
    I_Printf(".");
    R_InitLightTables ();
    I_Printf(".");
    R_InitSkyMap ();
    I_Printf(".");
    // -ES- 1998/09/11 Removed InitTranslationTables call.
//    R_InitTranslationTables ();
    I_Printf(".");
	
    framecount = 0;
}


//
// R_PointInSubsector
//
subsector_t* R_PointInSubsector (fixed_t x, fixed_t y)
{
    node_t*	node;
    int		side;
    int		nodenum;

    // single subsector is a special case
    if (!numnodes)				
	return subsectors;
		
    nodenum = numnodes-1;

    while (!(nodenum & NF_SUBSECTOR))
    {
	node = &nodes[nodenum];
	side = R_PointOnSide (x, y, node);
	nodenum = node->children[side];
    }
	
    return &subsectors[nodenum & ~NF_SUBSECTOR];
}



//
// R_SetupFrame
//
// -ACB- 1998/07/03 Added the mlook code and reworked it from R_RenderPlayerView
//
void R_SetupFrame (player_t* player)
{
    int i;
    int vertoffset;
    
    viewplayer = player;

    viewx = player->mo->x;
    viewy = player->mo->y;
    viewangle = player->mo->angle + gameflags.viewangleoffset;
    viewz = player->viewz;
    extralight = player->extralight;

    i =  viewangle>>ANGLETOFINESHIFT;
    viewsin = finesine[i];
    viewcos = finecosine[i];
	
    sscount = 0;
	
    if (player->fixedcolormap)
    {
     fixedcolormap = colormaps + player->fixedcolormap*256*sizeof(lighttable_t)*BPP;
	
     walllights = scalelightfixed;

     for (i=0 ; i<MAXLIGHTSCALE ; i++)
	    scalelightfixed[i] = player->fixedcolormap*256*sizeof(lighttable_t)*BPP;
    }
    else
    {
      fixedcolormap = 0;
    }

    // -ACB- 1998/09/01 Cleaned up a bit....
    vertoffset  = (player->mo->vertangle*viewheight)>>16;
    centery     = (viewheight/2)+vertoffset;
    centeryfrac = centery<<FRACBITS;
    yslope      = origyslope+(viewheight/2)-vertoffset;

    framecount++;
    validcount++;
}


//
// R_RenderView
//
// -ACB- 1998/07/03 Remove the hacked-in mlook and reworked it in R_SetupFrame
//
void R_RenderPlayerView (player_t* player)
{
    R_SetupFrame (player);

    // Clear buffers.
    R_ClearClipSegs ();
    R_ClearDrawSegs ();
    R_ClearPlanes ();
    R_ClearSprites ();
    
    // check for new console commands.
    NetUpdate ();

    // The head node is the last node output.
    R_RenderBSPNode (numnodes-1);
    
    // Check for new console commands.
    NetUpdate ();
    
    R_DrawPlanes ();
    
    // Check for new console commands.
    NetUpdate ();
    
    R_DrawMasked ();

    // Check for new console commands.
    NetUpdate ();
}
