//
// DOSDoom SEGS Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -KM- 1998/09/27 Dynamic colourmaps.
//
#include <stdlib.h>

#include "i_system.h"

#include "dm_defs.h"
#include "dm_state.h"

#include "r_local.h"
// -ES- 1998/08/05
#include "r_segs.h"
#include "r_sky.h"

#include "w_wad.h"

#include "z_zone.h"

// OPTIMIZE: closed two sided lines as single sided

// True if any of the segs textures might be visible.
boolean         segtextured;    

// False if the back side is the same plane.
boolean         markfloor;      
boolean         markceiling;

boolean         maskedtexture;
int             toptexture;
int             bottomtexture;
int             midtexture;


angle_t         rw_normalangle;

// angle to line origin
int             rw_angle1;      

//
// regular wall
//
int             rw_x;
int             rw_stopx;
angle_t         rw_centerangle;
fixed_t         rw_offset;
fixed_t         rw_distance;
fixed_t         rw_scale;
fixed_t         rw_scalestep;
fixed_t         rw_midtexturemid;
fixed_t         rw_toptexturemid;
fixed_t         rw_bottomtexturemid;

// -ES- 1999/03/24 Added These. They show the light level.
fixed_t         rw_light;
fixed_t         rw_lightstep;

int             worldtop;
int             worldbottom;
int             worldhigh;
int             worldlow;

fixed_t         pixhigh;
fixed_t         pixlow;
fixed_t         pixhighstep;
fixed_t         pixlowstep;

fixed_t         topfrac;
fixed_t         topstep;

fixed_t         bottomfrac;
fixed_t         bottomstep;


int*            walllights;

short*          maskedtexturecol;



//
// R_RenderMaskedSegRange
//
void
R_RenderMaskedSegRange
( drawseg_t*    ds,
  int           x1,
  int           x2 )
{
    unsigned    index;
    column_t*   col;
    int         lightnum;
    int         texnum;
    
    // Calculate light table.
    // Use different light tables
    //   for horizontal / vertical / diagonal. Diagonal?
    // OPTIMIZE: get rid of LIGHTSEGSHIFT globally
    curline = ds->curline;
    frontsector = curline->frontsector;
    backsector = curline->backsector;
    texnum = texturetranslation[curline->sidedef->midtexture];
        
    lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT)+extralight;

    if (curline->v1->y == curline->v2->y)
        lightnum--;
    else if (curline->v1->x == curline->v2->x)
        lightnum++;

    if (lightnum < 0)           
        walllights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
        walllights = scalelight[LIGHTLEVELS-1];
    else
        walllights = scalelight[lightnum];

    maskedtexturecol = ds->maskedtexturecol;

    rw_scalestep = ds->scalestep;               
    spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
    mfloorclip = ds->sprbottomclip;
    mceilingclip = ds->sprtopclip;

    // -ES- 1999/03/24 Added lighting vars to fix bug
    rw_lightstep = ds->lightstep;
    rw_light = ds->light1 + (x1-ds->x1)*rw_lightstep;
    
    // find positioning
    if (curline->linedef->flags & ML_DONTPEGBOTTOM)
    {
        dc_texturemid = frontsector->floorheight > backsector->floorheight
            ? frontsector->floorheight : backsector->floorheight;
        dc_texturemid = dc_texturemid + textureheight[texnum] - viewz;
    }
    else
    {
        dc_texturemid =frontsector->ceilingheight<backsector->ceilingheight
            ? frontsector->ceilingheight : backsector->ceilingheight;
        dc_texturemid = dc_texturemid - viewz;
    }
    dc_texturemid += curline->sidedef->rowoffset;
                        
    if (fixedcolormap)
        dc_colormap = fixedcolormap;
    
    // draw the columns
    for (dc_x = x1 ; dc_x <= x2 ; dc_x++)
    {
        // calculate lighting
        if (maskedtexturecol[dc_x] != SHRT_MAX)
        {
            if (!fixedcolormap)
            {
                index = rw_light>>LIGHTSCALESHIFT;

                if (index >=  MAXLIGHTSCALE )
                    index = MAXLIGHTSCALE-1;

                dc_colormap = colormaps + walllights[index];
            }
                        
            sprtopscreen = focusyfrac - FixedMul(dc_texturemid, spryscale);
            dc_iscale = 0xffffffffu / (unsigned)spryscale;
            
            // draw the texture
            col = (column_t *)( 
                (byte *)R_GetColumn(texnum,maskedtexturecol[dc_x]) -3);
                        
            R_DrawMaskedColumn (col);
            maskedtexturecol[dc_x] = SHRT_MAX;
        }
        spryscale += rw_scalestep;
        rw_light += rw_lightstep;
    }
        
}




//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked
//  texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling
//  textures.
// CALLED: CORE LOOPING ROUTINE.
//
#define HEIGHTBITS              12
#define HEIGHTUNIT              (1<<HEIGHTBITS)

#ifdef SMOOTHING
byte* dc_source2;
fixed_t dc_xfrac;
#endif
void R_RenderSegLoop (void)
{
    angle_t             angle;
    unsigned            index;
    int                 yl;
    int                 yh;
    int                 mid;
    fixed_t             texturecolumn = 0;
    int                 top;
    int                 bottom;
    fixed_t             texture_yoffset; // column's y offset due to screen position

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
        // mark floor / ceiling areas
        yl = (topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;

        // no space above wall?
        if (yl < ceilingclip[rw_x]+1)
            yl = ceilingclip[rw_x]+1;
        
        // -KM- 1998/07/31 Check for invalidness
        if (markceiling)
        {
            top = ceilingclip[rw_x]+1;
            bottom = yl-1;

            if (bottom >= floorclip[rw_x])
                bottom = floorclip[rw_x]-1;

            if (top <= bottom && ceilingplane >= 0)
            {
                visplanes[ceilingplane].top[rw_x] = top;
                visplanes[ceilingplane].bottom[rw_x] = bottom;
            }
        }
                
        yh = bottomfrac>>HEIGHTBITS;

        if (yh >= floorclip[rw_x])
            yh = floorclip[rw_x]-1;

        // -KM- 1998/07/31 Again, check for invalidness
        if (markfloor)
        {
            top = yh+1;
            bottom = floorclip[rw_x]-1;
            if (top <= ceilingclip[rw_x])
                top = ceilingclip[rw_x]+1;
            if (top <= bottom && floorplane >= 0)
            {
                visplanes[floorplane].top[rw_x] = top;
                visplanes[floorplane].bottom[rw_x] = bottom;
            }
        }
        
        // texturecolumn and lighting are independent of wall tiers
        // -ES- 1999/03/23 moved code into this if block, to shut up warning
        if (segtextured)
        {
            // calculate texture offset
            angle = (rw_centerangle + xtoviewangle[rw_x])>>ANGLETOFINESHIFT;
            texturecolumn = rw_offset-FixedMul(finetangent[angle],rw_distance);
#ifdef SMOOTHING
            dc_xfrac = texturecolumn & 0xffff;
#endif
            texturecolumn >>= FRACBITS;
            // calculate lighting
            index = rw_light>>LIGHTSCALESHIFT;

            if (index >=  MAXLIGHTSCALE )
                index = MAXLIGHTSCALE-1;

            if (fixedcolormap)
              dc_colormap = fixedcolormap;
            else
              dc_colormap = colormaps + walllights[index];
            dc_x = rw_x;
            dc_iscale = 0xffffffffu / (unsigned)rw_scale;
            // -ES- 1999/03/20 Added this, for asymmetric y fovs
            texture_yoffset = FixedMul(dc_iscale,focusyfrac);

            // draw the wall tiers

            if (midtexture)
            {
                // single sided line
                dc_yl = yl;
                dc_yh = yh;
                dc_texturemid = rw_midtexturemid - texture_yoffset;
                dc_source = R_GetColumn(midtexture,texturecolumn);
    #ifdef SMOOTHING
                dc_source2 = R_GetColumn(midtexture, texturecolumn+1);
    #endif
                colfunc ();
                ceilingclip[rw_x] = viewheight;
                floorclip[rw_x] = -1;
            }
            else
            {
                // two sided line
                if (toptexture)
                {
                    // top wall
                    mid = pixhigh>>HEIGHTBITS;
                    pixhigh += pixhighstep;
    
                    if (mid >= floorclip[rw_x])
                        mid = floorclip[rw_x]-1;
    
                    if (mid >= yl)
                    {
                        dc_yl = yl;
                        dc_yh = mid;
                        dc_texturemid = rw_toptexturemid - texture_yoffset;
                        dc_source = R_GetColumn(toptexture,texturecolumn);
#ifdef SMOOTHING
                        dc_source2 = R_GetColumn(toptexture, texturecolumn+1);
#endif
                        colfunc ();
                        ceilingclip[rw_x] = mid;
                    }
                    else
                        ceilingclip[rw_x] = yl-1;
                }
                            
                if (bottomtexture)
                {
                    // bottom wall
                    mid = (pixlow+HEIGHTUNIT-1)>>HEIGHTBITS;
                    pixlow += pixlowstep;
    
                    // no space above wall?
                    if (mid <= ceilingclip[rw_x])
                        mid = ceilingclip[rw_x]+1;
                    
                    if (mid <= yh)
                    {
                        dc_yl = mid;
                        dc_yh = yh;
                        dc_texturemid = rw_bottomtexturemid - texture_yoffset;
                        dc_source = R_GetColumn(bottomtexture, texturecolumn);
    #ifdef SMOOTHING
                        dc_source2 = R_GetColumn(bottomtexture, texturecolumn+1);
    #endif
                        colfunc ();
                        floorclip[rw_x] = mid;
                    }
                    else
                        floorclip[rw_x] = yh+1;
                }
                                    
                if (maskedtexture)
                {
                    // save texturecol
                    //  for backdrawing of masked mid texture
                    maskedtexturecol[rw_x] = texturecolumn;
                }
            }
        }
        if (!toptexture && markceiling)
            // no top wall
            ceilingclip[rw_x] = yl-1;
        if (!bottomtexture && markfloor)
            // no bottom wall
            floorclip[rw_x] = yh+1;

        rw_light += rw_lightstep;
        rw_scale += rw_scalestep;
        topfrac += topstep;
        bottomfrac += bottomstep;
    }
}


//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//  23-6-98 KM Rearranged some stuff regarding ds limit.
void R_StoreWallRange (int start, int stop)
{
    fixed_t             hyp;
    fixed_t             sineval;
    angle_t             distangle, offsetangle;
    fixed_t             vtop;
    int                 lightnum;
    lighttable_t*       tempcolourmap = fixedcolormap;
    drawseg_t*          ds;


    // don't overflow and crash
    if (ds_p == maxdrawsegs)
      drawsegs = Z_ReMalloc(drawsegs, ++maxdrawsegs * sizeof(drawseg_t));
                
#ifdef DEVELOPERS
    if (start >=viewwidth || start > stop)
        I_Error ("Bad R_RenderWallRange: %i to %i", start , stop);
#endif
    
    sidedef = curline->sidedef;
    linedef = curline->linedef;

    // mark the segment as visible for auto map
    linedef->flags |= ML_MAPPED;
    
    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle + ANG90;
    offsetangle = abs(rw_normalangle-rw_angle1);
    
    if (offsetangle > ANG90)
        offsetangle = ANG90;

    distangle = ANG90 - offsetangle;
    hyp = R_PointToDist (curline->v1->x, curline->v1->y);

    sineval = finesine[distangle>>ANGLETOFINESHIFT];
    rw_distance = FixedMul (hyp, sineval);
                
        
    ds = &drawsegs[ds_p++];
    ds->x1 = rw_x = start;
    ds->x2 = stop;
    ds->curline = curline;
    rw_stopx = stop+1;
    
    // calculate scale at both ends and step
    ds->scale1 = rw_scale =
        R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);
    ds->light1 = rw_light = FixedDiv(rw_scale * 160, y_distunit);

    if (stop > start )
    {
        ds->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
        ds->scalestep = rw_scalestep =
            (ds->scale2 - rw_scale) / (stop-start);
        ds->lightstep = rw_lightstep = FixedDiv(rw_scalestep * 160, y_distunit);
    }
    else
    {
        // UNUSED: try to fix the stretched line bug
#if 0
        if (rw_distance < FRACUNIT/2)
        {
            fixed_t             trx,try;
            fixed_t             gxt,gyt;

            trx = curline->v1->x - viewx;
            try = curline->v1->y - viewy;
                        
            gxt = FixedMul(trx,viewcos); 
            gyt = -FixedMul(try,viewsin); 
            ds->scale1 = FixedDiv(focusxfrac, gxt-gyt)<<detailshift;
        }
#endif
        ds->scale2 = ds->scale1;
    }
    
    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed
    worldtop = frontsector->ceilingheight - viewz;
    worldbottom = frontsector->floorheight - viewz;
        
    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds->maskedtexturecol = NULL;
        
    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];
        // a single sided line is terminal, so it must mark ends
        markfloor = markceiling = true;
        if (linedef->flags & ML_DONTPEGBOTTOM)
        {
            vtop = frontsector->floorheight +
                textureheight[sidedef->midtexture];
            // bottom of texture at bottom
            rw_midtexturemid = vtop - viewz;    
        }
        else
        {
            // top of texture at top
            rw_midtexturemid = worldtop;
        }
        rw_midtexturemid += sidedef->rowoffset;

        ds->silhouette = SIL_BOTH;
        ds->sprtopclip = screenheightarray;
        ds->sprbottomclip = negonearray;
        ds->bsilheight = INT_MAX;
        ds->tsilheight = INT_MIN;
    }
    else
    {
        // two sided line
        ds->sprtopclip = ds->sprbottomclip = NULL;
        ds->silhouette = 0;
        
        if (frontsector->floorheight > backsector->floorheight)
        {
            ds->silhouette = SIL_BOTTOM;
            ds->bsilheight = frontsector->floorheight;
        }
        else if (backsector->floorheight > viewz)
        {
            ds->silhouette = SIL_BOTTOM;
            ds->bsilheight = INT_MAX;
            // ds->sprbottomclip = negonearray;
        }
        
        if (frontsector->ceilingheight < backsector->ceilingheight)
        {
            ds->silhouette |= SIL_TOP;
            ds->tsilheight = frontsector->ceilingheight;
        }
        else if (backsector->ceilingheight < viewz)
        {
            ds->silhouette |= SIL_TOP;
            ds->tsilheight = INT_MIN;
            // ds->sprtopclip = screenheightarray;
        }
                
        if (backsector->ceilingheight <= frontsector->floorheight)
        {
            ds->sprbottomclip = negonearray;
            ds->bsilheight = INT_MAX;
            ds->silhouette |= SIL_BOTTOM;
        }
        
        if (backsector->floorheight >= frontsector->ceilingheight)
        {
            ds->sprtopclip = screenheightarray;
            ds->tsilheight = INT_MIN;
            ds->silhouette |= SIL_TOP;
        }
        
        worldhigh = backsector->ceilingheight - viewz;
        worldlow = backsector->floorheight - viewz;
                
        // hack to allow height changes in outdoor areas
        if (frontsector->ceilingpic == skyflatnum 
            && backsector->ceilingpic == skyflatnum)
        {
            worldtop = worldhigh;
        }
        
                        
        if (worldlow != worldbottom 
            || backsector->floorpic != frontsector->floorpic
            || backsector->lightlevel != frontsector->lightlevel)
        {
            markfloor = true;
        }
        else
        {
            // same plane on both sides
            markfloor = false;
        }
        
                        
        if (worldhigh != worldtop 
            || backsector->ceilingpic != frontsector->ceilingpic
            || backsector->lightlevel != frontsector->lightlevel)
        {
            markceiling = true;
        }
        else
        {
            // same plane on both sides
            markceiling = false;
        }
        
        if (backsector->ceilingheight <= frontsector->floorheight
            || backsector->floorheight >= frontsector->ceilingheight)
        {
            // closed door
            markceiling = markfloor = true;
        }
        

        if (worldhigh < worldtop)
        {
            // top texture
            toptexture = texturetranslation[sidedef->toptexture];
            if (linedef->flags & ML_DONTPEGTOP)
            {
                // top of texture at top
                rw_toptexturemid = worldtop;
            }
            else
            {
                vtop =
                    backsector->ceilingheight
                    + textureheight[sidedef->toptexture];
                
                // bottom of texture
                rw_toptexturemid = vtop - viewz;        
            }
        }
        if (worldlow > worldbottom)
        {
            // bottom texture
            bottomtexture = texturetranslation[sidedef->bottomtexture];

            if (linedef->flags & ML_DONTPEGBOTTOM )
            {
                // bottom of texture at bottom
                // top of texture at top
                rw_bottomtexturemid = worldtop;
            }
            else        // top of texture at top
                rw_bottomtexturemid = worldlow;
        }
        rw_toptexturemid += sidedef->rowoffset;
        rw_bottomtexturemid += sidedef->rowoffset;
        
        // allocate space for masked texture tables
        if (sidedef->midtexture)
        {
            // masked midtexture
            maskedtexture = true;
            ds->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
            lastopening += rw_stopx - rw_x;

            // -ES- 1998/08/19 Prevents middle texture from being clipped
            // to the wrong sector
            markceiling = markfloor = true;
        }
    }
    
    // calculate rw_offset (only needed for textured lines)
    segtextured = midtexture | toptexture | bottomtexture | maskedtexture;

    if (segtextured)
    {
        offsetangle = rw_normalangle-rw_angle1;
        
        if (offsetangle > ANG180)
            offsetangle = -offsetangle;

        if (offsetangle > ANG90)
            offsetangle = ANG90;

        sineval = finesine[(offsetangle>>ANGLETOFINESHIFT)];
        rw_offset = FixedMul (hyp, sineval);

        if (rw_normalangle-rw_angle1 < ANG180)
            rw_offset = -rw_offset;

        rw_offset += sidedef->textureoffset + curline->offset;
        rw_centerangle = ANG90 + viewangle - rw_normalangle;
        
        // calculate light table
        //  use different light tables
        //  for horizontal / vertical / diagonal
        // OPTIMIZE: get rid of LIGHTSEGSHIFT globally
        if (!fixedcolormap)
        {
            if (frontsector->colourmaplump >= 0)
              R_SetColourMap(frontsector->colourmaplump);
      
            if (frontsector->colourmap >= 0)
              fixedcolormap = colormaps + frontsector->colourmap*256*sizeof(lighttable_t)*BPP;

            lightnum = (frontsector->lightlevel >> LIGHTSEGSHIFT)+extralight;

            if (curline->v1->y == curline->v2->y)
                lightnum--;
            else if (curline->v1->x == curline->v2->x)
                lightnum++;

            if (lightnum < 0)           
                walllights = scalelight[0];
            else if (lightnum >= LIGHTLEVELS)
                walllights = scalelight[LIGHTLEVELS-1];
            else
                walllights = scalelight[lightnum];
        }
    }
    
    // if a floor / ceiling plane is on the wrong side
    //  of the view plane, it is definitely invisible
    //  and doesn't need to be marked.
    
  
    if (frontsector->floorheight >= viewz)
    {
        // above view plane
        markfloor = false;
    }
    
    if (frontsector->ceilingheight <= viewz 
        && frontsector->ceilingpic != skyflatnum)
    {
        // below view plane
        markceiling = false;
    }

    
    // calculate incremental stepping values for texture edges
    worldtop >>= 4;
    worldbottom >>= 4;
        
    topstep = -FixedMul (rw_scalestep, worldtop);
    topfrac = (focusyfrac>>4) - FixedMul (worldtop, rw_scale);

    bottomstep = -FixedMul (rw_scalestep,worldbottom);
    bottomfrac = (focusyfrac>>4) - FixedMul (worldbottom, rw_scale);
        
    if (backsector)
    {   
        worldhigh >>= 4;
        worldlow >>= 4;

        if (worldhigh < worldtop)
        {
            pixhigh = (focusyfrac>>4) - FixedMul (worldhigh, rw_scale);
            pixhighstep = -FixedMul (rw_scalestep,worldhigh);
        }
        
        if (worldlow > worldbottom)
        {
            pixlow = (focusyfrac>>4) - FixedMul (worldlow, rw_scale);
            pixlowstep = -FixedMul (rw_scalestep,worldlow);
        }
    }
    
    // render it
    // -KM- 1998/07/31 Check for invalidness
    if (markceiling && ceilingplane >= 0)
        ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
    
    if (markfloor && floorplane >= 0)
        floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);

    R_RenderSegLoop ();

    fixedcolormap = tempcolourmap;
    if ((frontsector->colourmaplump >= 0) && !fixedcolormap)
      R_SetColourMap(W_GetNumForName("COLORMAP"));
    
    // save sprite clipping info
    if ( ((ds->silhouette & SIL_TOP) || maskedtexture)
         && !ds->sprtopclip)
    {
        memcpy (lastopening, ceilingclip+start, 2*(rw_stopx-start));
        ds->sprtopclip = lastopening - start;
        lastopening += rw_stopx - start;
    }
    
    if ( ((ds->silhouette & SIL_BOTTOM) || maskedtexture)
         && !ds->sprbottomclip)
    {
        memcpy (lastopening, floorclip+start, 2*(rw_stopx-start));
        ds->sprbottomclip = lastopening - start;
        lastopening += rw_stopx - start;        
    }

    if (maskedtexture && !(ds->silhouette&SIL_TOP))
    {
        ds->silhouette |= SIL_TOP;
        ds->tsilheight = INT_MIN;
    }
    if (maskedtexture && !(ds->silhouette&SIL_BOTTOM))
    {
        ds->silhouette |= SIL_BOTTOM;
        ds->bsilheight = INT_MAX;
    }
}

