//
// DOSDoom BSP Handling Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -KM- 1998/09/27 Special sector colourmap changing
//

#include "dm_defs.h"

#include "m_bbox.h"

#include "i_system.h"

#include "r_main.h"
#include "r_plane.h"
#include "r_things.h"

// State.
#include "dm_state.h"
#include "r_state.h"

// 23-6-98 KM Dynamic mem allocation
#include "z_zone.h"

seg_t*          curline;
side_t*         sidedef;
line_t*         linedef;
sector_t*       frontsector;
sector_t*       backsector;

int             maxdrawsegs = MAXDRAWSEGS;
drawseg_t*      drawsegs = NULL;
int             ds_p;


void
R_StoreWallRange
( int   start,
  int   stop );




//
// R_ClearDrawSegs
//
void R_ClearDrawSegs (void)
{
    ds_p = 0;
}



//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef struct
{
    int first;
    int last;
    
} cliprange_t;


// 98-6-21 KM Removing solidsegs limit
#define MAXSEGS 32
// newend is one past the last valid seg
int             newend;
int             maxsolidsegs = MAXSEGS;
cliprange_t*    solidsegs = NULL;




//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
// 
void
R_ClipSolidWallSegment
( int                   first,
  int                   last )
{
    cliprange_t*        next;
    cliprange_t*        start;
    cliprange_t*        end;

    // 98-6-21 KM  Removing solidsegs limit
    // -KM- 1998/07/31 Perhaps Solid segs go up somewhere else?
    if (newend >= maxsolidsegs)
    {
      maxsolidsegs += newend - maxsolidsegs;
      solidsegs = Z_ReMalloc(solidsegs, ++maxsolidsegs * sizeof(cliprange_t));
    }
    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    end   = solidsegs + newend;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start),
            //  so insert a new clippost.
            R_StoreWallRange (first, last);
            next = end;
            newend++;
            
            while (next != start)
            {
                *next = *(next-1);
                next--;
            }
            next->first = first;
            next->last = last;
            return;
        }
                
        // There is a fragment above *start.
        R_StoreWallRange (first, start->first - 1);
        // Now adjust the clip size.
        start->first = first;   
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;                 
                
    next = start;
    while (last >= (next+1)->first-1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange (next->last + 1, (next+1)->first - 1);
        next++;
        
        if (last <= next->last)
        {
            // Bottom is contained in next.
            // Adjust the clip size.
            start->last = next->last;   
            goto crunch;
        }
    }
        
    // There is a fragment after *next.
    R_StoreWallRange (next->last + 1, last);
    // Adjust the clip size.
    start->last = last;
        
    // Remove start+1 to next from the clip list,
    // because start now covers their area.
  crunch:
    if (next == start)
    {
        // Post just extended past the bottom of one post.
        return;
    }
    

    while (next++ != end)
    {
        // Remove a post.
        *++start = *next;
    }

    newend = start-solidsegs;
    newend++;
}



//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
void
R_ClipPassWallSegment
( int   first,
  int   last )
{
    cliprange_t*        start;



    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)

          {
            // Post is entirely visible (above start).
            R_StoreWallRange (first, last);
            return;
        }
                
        // There is a fragment above *start.
        R_StoreWallRange (first, start->first - 1);
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;                 
                
    while (last >= (start+1)->first-1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange (start->last + 1, (start+1)->first - 1);
        start++;
        
        if (last <= start->last)
            return;
    }
        
    // There is a fragment after *next.
    R_StoreWallRange (start->last + 1, last);
}



//
// R_ClearClipSegs
//
void R_ClearClipSegs (void)
{
    // 98-6-21 KM Removing solidsegs limit
    if (!solidsegs)
      solidsegs = Z_Malloc(sizeof(cliprange_t) * maxsolidsegs, PU_STATIC, &solidsegs);
    solidsegs[0].first = -0x7fffffff;
    solidsegs[0].last = -1;
    solidsegs[1].first = viewwidth;
    solidsegs[1].last = 0x7fffffff;
    newend = 2;
}

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
void R_AddLine (seg_t*  line)
{
    int                 x1;
    int                 x2;
    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;

    curline = line;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle (line->v1->x, line->v1->y);
    angle2 = R_PointToAngle (line->v2->x, line->v2->y);
    
    // Clip to view edges.

    // -ES- 1999/03/20 Replaced clipangle with clipscope/leftclipangle/rightclipangle

    span = angle1 - angle2;
    
    // Back side? I.e. backface culling?
    if (span >= ANG180)
        return;         

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;
        
    tspan = angle1 - rightclipangle;
    if (tspan > clipscope)
    {
        tspan -= clipscope;;

        // Totally off the left edge?
        if (tspan >= span)
            return;
        
        angle1 = leftclipangle;
    }
    tspan = leftclipangle - angle2;
    if (tspan > clipscope)
    {
        tspan -= clipscope;

        // Totally off the left edge?
        if (tspan >= span)
            return;     
        angle2 = rightclipangle;
    }
    
    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    // Does not cross a pixel?
    if (x1 == x2)
        return;                         
        
    backsector = line->backsector;

    // Single sided line?
    if ((!backsector)
        // Closed door.
        || (backsector->ceilingheight <= frontsector->floorheight
        || backsector->floorheight >= frontsector->ceilingheight))
      {
          R_ClipSolidWallSegment (x1, x2-1);
          return;
      }

    // Window.
    if ((backsector->ceilingheight != frontsector->ceilingheight
        || backsector->floorheight != frontsector->floorheight)
        // Reject empty lines used for triggers
        //  and special events.
        // Identical floor and ceiling on both sides,
        // identical light levels on both sides,
        // and no middle texture.
        || !(backsector->ceilingpic == frontsector->ceilingpic
        && backsector->floorpic == frontsector->floorpic
        && backsector->lightlevel == frontsector->lightlevel
        && curline->sidedef->midtexture == 0))
            R_ClipPassWallSegment (x1, x2-1);
}


//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
int     checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},
    {2,0,2,1},
    {0,0,0,0},
    {3,1,3,0},
    {0},
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};


boolean R_CheckBBox (fixed_t*   bspcoord)
{
    int                 boxx;
    int                 boxy;
    int                 boxpos;

    fixed_t             x1;
    fixed_t             y1;
    fixed_t             x2;
    fixed_t             y2;
    
    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;

    cliprange_t*        start;

    int                 sx1;
    int                 sx2;
    
    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (viewx <= bspcoord[BOXLEFT])
        boxx = 0;
    else if (viewx < bspcoord[BOXRIGHT])
        boxx = 1;
    else
        boxx = 2;
                
    if (viewy >= bspcoord[BOXTOP])
        boxy = 0;
    else if (viewy > bspcoord[BOXBOTTOM])
        boxy = 1;
    else
        boxy = 2;
                
    boxpos = (boxy<<2)+boxx;
    if (boxpos == 5)
        return true;
        
    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];
    
    // check clip list for an open space
    angle1 = R_PointToAngle (x1, y1) - viewangle;
    angle2 = R_PointToAngle (x2, y2) - viewangle;
        
    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
        return true;
    
    // -ES- 1999/03/20 Replaced clipangle with clipscope/leftclipangle/rightclipangle

    tspan = angle1 - rightclipangle;

    if (tspan > clipscope)
    {
        tspan -= clipscope;

        // Totally off the left edge?
        if (tspan >= span)
            return false;       

        angle1 = leftclipangle;
    }
    tspan = leftclipangle - angle2;
    if (tspan > clipscope)
    {
        tspan -= clipscope;

        // Totally off the left edge?
        if (tspan >= span)
            return false;
        
        angle2 = rightclipangle;
    }


    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    sx1 = viewangletox[angle1];
    sx2 = viewangletox[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;                   
    sx2--;
        
    start = solidsegs;
    while (start->last < sx2)
        start++;
    
    if (sx1 >= start->first
        && sx2 <= start->last)
    {
        // The clippost contains the new span.
        return false;
    }

    return true;
}



//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
void R_Subsector (int num)
{
    int                 count;
    seg_t*              line;
    subsector_t*        sub;
        
#ifdef DEVELOPERS
    if (num>=numsubsectors)
        I_Error ("R_Subsector: ss %i with numss = %i",
                 num,
                 numsubsectors);
#endif

    sscount++;
    sub = &subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &segs[sub->firstline];

    if (frontsector->floorheight < viewz)
    {
        floorplane = R_FindPlane (frontsector->floorheight,
                                  frontsector->floorpic,
                                  frontsector->lightlevel,
                                  frontsector->colourmaplump,
                                  frontsector->colourmap);
    }
    else
        // -KM- 1998/07/31 Try to fix those wads...
        floorplane = -1;
    
    if (frontsector->ceilingheight > viewz 
        || frontsector->ceilingpic == skyflatnum)
    {
        ceilingplane = R_FindPlane (frontsector->ceilingheight,
                                    frontsector->ceilingpic,
                                    frontsector->lightlevel,
                                    frontsector->colourmaplump,
                                    frontsector->colourmap);
    }
    else
        ceilingplane = -1;
                
    R_AddSprites (frontsector); 

    while (count--)
    {
        R_AddLine (line);
        line++;
    }
}




//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
void R_RenderBSPNode (int bspnum)
{
    node_t*     bsp;
    int         side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)                       
            R_Subsector (0);
        else
            R_Subsector (bspnum&(~NF_SUBSECTOR));
        return;
    }
                
    bsp = &nodes[bspnum];
    
    // Decide which side the view point is on.
    side = R_PointOnSide (viewx, viewy, bsp);

    // Recursively divide front space.
    R_RenderBSPNode (bsp->children[side]); 

    // Possibly divide back space.
    if (R_CheckBBox (bsp->bbox[side^1]))        
        R_RenderBSPNode (bsp->children[side^1]);
}


