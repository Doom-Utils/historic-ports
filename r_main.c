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

#include "m_misc.h"

// -ES- 1999/03/14 Dynamic Field Of View
// Fineangles in the viewwidth wide window.
angle_t FIELDOFVIEW = 2048;
// The aspect ratio to use. A normal texel will be aspect_ratio times wider than high
fixed_t aspect_ratio = FRACUNIT*200/320;

// the extreme angles of the view
angle_t topangle    = +728;
angle_t bottomangle = -728;
angle_t rightangle  = -FINEANGLES/8;
angle_t leftangle   = FINEANGLES/8;

fixed_t leftslope   = FRACUNIT;
fixed_t rightslope  = -FRACUNIT;
fixed_t topslope    = FRACUNIT*200/320;
fixed_t bottomslope = -FRACUNIT*200/320;

fixed_t keylookspeed=1000/64;
int invertmouse=0;

angle_t viewangle = 0;

fixed_t normalfov,zoomedfov;
boolean viewiszoomed = false;

// increment every time a check is made
int validcount = 1;

// -KM- 1998/09/27 Dynamic colourmaps
lighttable_t* fixedcolormap;
extern int* walllights;

// -ES- 1999/03/19 rename from center to focus
int focusx;
int focusy;

fixed_t focusxfrac;
fixed_t focusyfrac;

// -ES- 1999/03/14 Added these. Unit Scale is used for things one distunit away.
fixed_t x_distunit;
fixed_t y_distunit;

// just for profiling purposes
int                     framecount;     

int                     sscount;
int                     linecount;

fixed_t viewx;
fixed_t viewy;
fixed_t viewz;

fixed_t viewcos;
fixed_t viewsin;

player_t* viewplayer;

//
// precalculated math tables
//

// -ES- 1999/03/20 Different right & left side clip angles, for asymmetric FOVs.
angle_t leftclipangle, rightclipangle;
angle_t clipscope;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X. 
int viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from leftangle to rightangle.
// -ES- 1998/08/20 Explicit init to NULL
angle_t *xtoviewangle = NULL;

fixed_t* finecosine = &finesine[FINEANGLES/4];

// -KM- 1998/09/27 Dynamic colourmaps
int scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
int scalelightfixed[MAXLIGHTSCALE];
int zlight[LIGHTLEVELS][MAXLIGHTZ];

// bumped light from gun blasts
int extralight;

angle_t viewangleoffset;

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
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     left;
    fixed_t     right;
        
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
    fixed_t     lx;
    fixed_t     ly;
    fixed_t     ldx;
    fixed_t     ldy;
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     left;
    fixed_t     right;
        
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
angle_t R_PointToAngle2 ( fixed_t x1, fixed_t y1, fixed_t x, fixed_t y )
{       
    x -= x1;
    y -= y1;
    
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

// -KM- 1999/01/31 R_PointToAngle always works.
angle_t R_PointToAngle ( fixed_t x, fixed_t y )
{       
    return R_PointToAngle2(viewx, viewy, x, y);
}

fixed_t R_PointToDist (fixed_t x, fixed_t y)
{
    int         angle;
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     temp;
    fixed_t     dist;
        
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
    fixed_t             scale;
    int                 anglea;
    int                 angleb;
    int                 sinea;
    int                 sineb;
    fixed_t             num;
    int                 den;

    anglea = ANG90 + (visangle-viewangle);
    angleb = ANG90 + (visangle-rw_normalangle);

    // both sines are always positive
    sinea = finesine[anglea>>ANGLETOFINESHIFT];
    sineb = finesine[angleb>>ANGLETOFINESHIFT];
    num = FixedMul(y_distunit,sineb)<<detailshift;
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
    int                 i;
    int                 x;
    int                 t;
    
    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.

    // -ES- 1999/03/19 removed focallength
    // -ES- 1999/03/20 support asymmetric fovs

    // -ES- 1999/03/13 Fixed i clipping to work with big FOVs
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        if (finetangent[i] > leftslope)
            t = -1;
        else if (finetangent[i] < rightslope)
            t = viewwidth+1;
        else
        {

            t = FixedMul (finetangent[i], x_distunit);
            t = (focusxfrac - t+FRACUNIT-1)>>FRACBITS;

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
        t = FixedMul (finetangent[i], x_distunit);
        t = focusx - t;
        
        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth+1)
            viewangletox[i]  = viewwidth;
    }
        
    leftclipangle = xtoviewangle[0];
    rightclipangle = xtoviewangle[viewwidth];
    clipscope = leftclipangle-rightclipangle;
}

//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP         2

void R_InitLightTables (void)
{
    int         i;
    int         j;
    int         level;
    int         startmap;       
    int         scale;
    
    // Calculate the light levels to use
    //  for each level / distance combination.
    for (i=0 ; i< LIGHTLEVELS ; i++)
    {
        startmap = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
        for (j=0 ; j<MAXLIGHTZ ; j++)
        {
//          scale = FixedDiv ((SCREENWIDTH/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
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
boolean         setsizeneeded;
int             setblocks;
int             setdetail;
boolean         setresfailed = false;

void R_SetViewSize (int  blocks, int detail)
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
    fixed_t     cosadj;
    int         i;
    int         j;
    int         level;
    int         startmap;       

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

    leftslope  = finetangent[FINEANGLES/4+leftangle];
    rightslope = finetangent[FINEANGLES/4+rightangle];

    // -ES- 1999/03/19 Use focusx & focusy, for asymmetric fovs.
    focusxfrac = FixedDiv (FixedMul(leftslope,FRACUNIT*viewwidth),
                           leftslope-rightslope);
    focusx = (focusxfrac+FRACUNIT/2) >> FRACBITS;

    // Unit scale at distance distunit.
    x_distunit = FixedDiv(FRACUNIT*viewwidth,
                          leftslope-rightslope);

    colfunc = basecolfunc = R_DrawColumn;
    fuzzcolfunc = R_DrawFuzzColumn;
    transcolfunc = R_DrawTranslatedColumn;
    spanfunc = R_DrawSpan;

    R_InitBuffer (scaledviewwidth, viewheight);
        
    R_InitTextureMapping ();

    pspritescale = FRACUNIT*viewwidth/320;
    pspriteiscale = FRACUNIT*320/viewwidth;
    pspritescale2 = (SCREENHEIGHT*FRACUNIT/SCREENWIDTH)*viewwidth/200;
    pspriteiscale2 = (SCREENWIDTH*FRACUNIT/SCREENHEIGHT)*200/viewwidth;

    // thing clipping
    for (i=0 ; i<viewwidth ; i++)
        screenheightarray[i] = viewheight;
    
    // planes
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
// R_SetFOV
//
// Sets the specified field of view
// -ES- 1999/03/28 Added This
void R_SetFOV(angle_t fov)
{
  // can't change fov to angle below 5 or above 175 deg (approx). Round so
  // that 5 and 175 are allowed for sure.
  if (fov<ANG90/18 || fov>((ANG90+17)/18)*35)
    return;
  setsizeneeded=true;

  fov = fov>>ANGLETOFINESHIFT; // convert to fineangle format
  leftangle = fov/2;
  rightangle = -fov/2;
  FIELDOFVIEW = leftangle-rightangle;
}

void R_SetNormalFOV(angle_t newfov)
{
  extern int menunormalfov; // for the scroll bar in the menu

  menunormalfov = (newfov-ANG45/18)/(ANG45/9);
  cfgnormalfov = (newfov+ANG45/90)/(ANG180/180);
  normalfov = newfov;
  if (!viewiszoomed)
    R_SetFOV(normalfov);
}
void R_SetZoomedFOV(angle_t newfov)
{
  extern int menuzoomedfov; // for the scroll bar in the menu

  menuzoomedfov = (newfov-ANG45/18)/(ANG45/9);
  cfgzoomedfov = (newfov+ANG45/90)/(ANG180/180);
  zoomedfov = newfov;
  if (viewiszoomed)
    R_SetFOV(zoomedfov);
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
// -ES- 1999/04/05 Changed this to work with the viewbitmap system
boolean R_DoExecuteChangeResolution()
{
  changeresneeded = false;
  BPP = setbpp;
  SCREENWIDTH = setwidth;
  SCREENHEIGHT = setheight;

  if (enter_graphics_mode ())
    return true;

  colormaps = (BPP==1) ? colormaps8 : colormaps16;
  V_InitResolution ();
  R_InitPlanes ();
  R_InitLightTables ();
  R_InitSkyMap ();
  R_InitTranslationTables ();
  ST_ReInit ();

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
// Called once at startup, to initialise some rendering stuff.
extern int      detailLevel;
extern int      screenblocks;
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
    R_SetNormalFOV(cfgnormalfov*(ANG45/45));
    R_SetZoomedFOV(cfgzoomedfov*(ANG45/45));
    R_SetFOV (normalfov);
    I_Printf(".");
        
    framecount = 0;
}


//
// R_PointInSubsector
//
subsector_t* R_PointInSubsector (fixed_t x, fixed_t y)
{
    node_t*     node;
    int         side;
    int         nodenum;

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
// -ES- 1999/03/21 Moved the mlook code back to R_RenderPlayerView
//
void R_SetupFrame (player_t* player)
{
    int i;
    fixed_t     dy;
    
    viewplayer = player;

    viewx = player->mo->x;
    viewy = player->mo->y;
    viewangle = player->mo->angle + viewangleoffset;
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

    // -ES- 1999/03/20 Cleaned up more. All fractional angles are now allowed.
    // -ES- 1999/03/21 FIXME Use some sort of caching system
    y_distunit = FixedDiv(FRACUNIT*viewheight,
                          topslope-bottomslope);
    focusyfrac = FixedDiv(viewheight * topslope,
                          topslope-bottomslope);
    focusy = focusyfrac>>FRACBITS;
    for (i=0 ; i<viewheight; i++)
    {
      dy = i*FRACUNIT - focusyfrac + FRACUNIT/2;
      dy = abs(dy);
      // -ES- 1999/03/14 Fixed Aspect Ratio
      yslope[i] = FixedDiv (y_distunit, dy);
    }

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
    fixed_t slopeoffset;

    // -ES- 1999/03/21 Reworked & moved the mlook code back to this function...
    slopeoffset = FixedMul(finetangent[FINEANGLES/4+FIELDOFVIEW/2], aspect_ratio);
    slopeoffset = slopeoffset*viewheight/viewwidth;
    slopeoffset = slopeoffset*SCREENWIDTH/SCREENHEIGHT;
    topslope    = player->mo->vertangle+slopeoffset;
    bottomslope = player->mo->vertangle-slopeoffset;

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

void R_Render (void)
{
  R_RenderPlayerView (&players[displayplayer]);
}

