//
// DOSDoom Moving, aiming, shooting & collision Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -MH- 1998/07/02 "shootupdown" --> "true3dgameplay"
//

#include <stdlib.h>

#include "d_debug.h"
#include "m_bbox.h"
#include "m_random.h"
#include "i_system.h"

#include "dm_defs.h"
#include "p_local.h"

#include "s_sound.h"

// State.
#include "dm_state.h"
#include "r_state.h"
// Data.
#include "lu_sound.h"

#include "z_zone.h"


fixed_t		tmbbox[4];
mobj_t*		tmthing;
int		tmflags;

fixed_t		tmx;
fixed_t		tmy;
fixed_t		tmz; // -ACB- 1998/06/15 Z Checking

line_t* ceilingline;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean		floatok;

fixed_t		tmfloorz;
fixed_t		tmceilingz;
fixed_t		tmdropoffz;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

int		maxspecialcross = 8;
line_t**	spechit = NULL;
int		numspechit;



//
// TELEPORT MOVE
// 

//
// PIT_StompThing
//
boolean PIT_StompThing (mobj_t* thing)
{
  fixed_t blockdist;

  if (!(thing->flags & MF_SHOOTABLE))
    return true;
		
  blockdist = thing->radius + tmthing->radius;

  // check to see we hit it
  if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
    return true; // no, we did not

  // check we aren't trying to stomp ourselves
  if (thing == tmthing)
    return true;

  // -ACB- 1998/08/19 Check that only player can STOMP
  if (!tmthing->player && (currentmap->flags & MPF_NOSTOMP))
    return false;

  P_DamageMobj (thing, tmthing, tmthing, 10000);

  return true;
}


//
// P_TeleportMove
//
boolean P_TeleportMove (mobj_t* thing, fixed_t x, fixed_t y)
{
  int xl;
  int xh;
  int yl;
  int yh;
  int bx;
  int by;
    
  subsector_t*	newsubsec;
    
  // kill anything occupying the position
  tmthing = thing;
  tmflags = thing->flags;
	
  tmx = x;
  tmy = y;
	
  tmbbox[BOXTOP] = y + tmthing->radius;
  tmbbox[BOXBOTTOM] = y - tmthing->radius;
  tmbbox[BOXRIGHT] = x + tmthing->radius;
  tmbbox[BOXLEFT] = x - tmthing->radius;

  newsubsec = R_PointInSubsector (x,y);
    
  // The base floor/ceiling is from the subsector that contains the point.
  // Any contacted lines the step closer together will adjust them.
  tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
  tmceilingz = newsubsec->sector->ceilingheight;
			
  validcount++;
  numspechit = 0;
    
  // stomp on any things contacted
  xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
 	return false;
    
  // the move is ok, so link the thing into its new position
  P_UnsetThingPosition (thing);

  thing->floorz = tmfloorz;
  thing->ceilingz = tmceilingz;
  thing->x = x;
  thing->y = y;

  P_SetThingPosition (thing);
	
  return true;
}


//
// MOVEMENT ITERATOR FUNCTIONS
//


//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
static boolean PIT_CheckLine (line_t* ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	|| tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
	|| tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
	|| tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
	return true;

    if (P_BoxOnLineSide (tmbbox, ld) != -1)
	return true;
		
    // A line has been hit
    
    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.
    
    if (!ld->backsector)
      return false;		// one sided line

    // -ACB- 1998/08/24 if the line is not one sided, we'll need the linedef
    ceilingline = ld;
		
    if (!(tmthing->flags & MF_MISSILE))
    {
      if (ld->flags & ML_BLOCKING)
	return false;   // explicitly blocking everything

      if (!tmthing->player && ld->flags & ML_BLOCKMONSTERS)
	return false;   // block monsters only
    }

    // set openrange, opentop, openbottom
    P_LineOpening (ld);	
	
    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
      tmceilingz = opentop;

    if (openbottom > tmfloorz)
       tmfloorz = openbottom;

    if (lowfloor < tmdropoffz)
	tmdropoffz = lowfloor;
		
    // if contacted a special line, add it to the list
    if (ld->special)
    {
        if (!spechit)
          spechit = Z_Malloc(sizeof(line_t *) * maxspecialcross, PU_STATIC, NULL);

        spechit[numspechit] = ld;

	numspechit++;

        if (numspechit == maxspecialcross)
          spechit = Z_ReMalloc(spechit, sizeof(line_t *) * ++maxspecialcross);
    }

    return true;
}

//
// PIT_CheckThing
//
static boolean PIT_CheckThing (mobj_t* thing)
{
    fixed_t		blockdist;
    boolean		solid;

    if (thing == tmthing)
      return true;

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
      return true;

    blockdist = thing->radius + tmthing->radius;

    // Check that we didn't hit it
    if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
	return true; // no we missed this thing

    // -KM- 1998/9/19 True 3d gameplay checks.
    if (gameflags.true3dgameplay)
    {
     	// see if it went over / under
	if (tmthing->z >= thing->z + thing->height)
        {
          if (!(thing->flags & MF_SPECIAL))
          {
            if ((thing->z + thing->height) > tmfloorz)
              tmfloorz = thing->z + thing->height;
            return true;     // overhead
          }
        }
	if (tmthing->z+tmthing->height < thing->z)
        {
          if (!(thing->flags & MF_SPECIAL))
          {
            if (thing->z < tmceilingz)
              tmceilingz = thing->z;
            return true;   // underneath
          }
        }
    }

    // check for skulls slamming into things
    // -ACB- 1998/08/04 Use procedure
    // -KM- 1998/09/01 After I noticed Skulls slamming into boxes of rockets...
    if ((tmthing->flags & MF_SKULLFLY) && (thing->flags & MF_SOLID))
    {
        P_ActSlammedIntoObject(tmthing,thing);
	return false;		// stop moving
    }

    // check for missiles making contact
    // -ACB- 1998/08/04 Procedure for missile contact
    if (tmthing->flags & MF_MISSILE)
    {
	// see if it went over / under
	if (tmthing->z > thing->z + thing->height)
	  return true;		// overhead
	if (tmthing->z+tmthing->height < thing->z)
	  return true;		// underneath

        // thing isn't shootable, return depending on if the thing is solid.
	if (!(thing->flags & MF_SHOOTABLE))
	  return !(thing->flags & MF_SOLID);

        if (tmthing->source && tmthing->source == thing)
          return true;

        P_ActMissileContact(tmthing,thing);
        return false;
    }

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
	solid = thing->flags&MF_SOLID;
	if (tmflags&MF_PICKUP)
	{
	    // can remove thing
	    P_TouchSpecialThing (thing, tmthing);
	}
	return !solid;
    }
	
    return !(thing->flags & MF_SOLID);
}

//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
// 
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//
boolean P_CheckPosition ( mobj_t* thing, fixed_t x, fixed_t y )
{
    int			xl;
    int			xh;
    int			yl;
    int			yh;
    int			bx;
    int			by;
    subsector_t*	newsubsec;

    ceilingline = NULL;

    tmthing = thing;
    tmflags = thing->flags;
	
    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    
    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;
			
    validcount++;
    numspechit = 0;

    if (tmflags & MF_NOCLIP)
	return true;

    // -KM- 1998/11/25 Corpses aren't supposed to hang in the air...
    if (!(tmflags & MF_CORPSE))
    {
      // Check things first, possibly picking things up.
      // The bounding box is extended by MAXRADIUS
      // because mobj_ts are grouped into mapblocks
      // based on their origin point, and can overlap
      // into adjacent blocks by up to MAXRADIUS units.
      xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
      xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
      yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
      yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;
  
      for (bx=xl ; bx<=xh ; bx++)
  	for (by=yl ; by<=yh ; by++)
  	    if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
  		return false;
    }
    // check lines
    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
	for (by=yl ; by<=yh ; by++)
	    if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
		return false;

    return true;
}

//
// P_TryMove
//
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
boolean P_TryMove ( mobj_t* thing, fixed_t x, fixed_t y )
{
  fixed_t oldx;
  fixed_t oldy;
  int side;
  int oldside;
  line_t* ld;

  floatok = false;

  if (!P_CheckPosition (thing, x, y))
    return false;		// solid wall or thing
    
  if (!(thing->flags & MF_NOCLIP))
  {
    if (tmceilingz - tmfloorz < thing->height)
      return false;	// doesn't fit

    floatok = true;
	
    if (!(thing->flags&MF_TELEPORT)&&tmceilingz-thing->z < thing->height)
      return false;	// mobj must lower itself to fit

    if (!(thing->flags&MF_TELEPORT)&&tmfloorz-thing->z > 24*FRACUNIT)
      return false; // too big a step up

    if (!(thing->flags&(MF_DROPOFF|MF_FLOAT))&&tmfloorz-tmdropoffz > 24*FRACUNIT)
      return false; // don't stand over a dropoff
  }
    
  // the move is ok, so link the thing into its new position
  P_UnsetThingPosition (thing);

  oldx = thing->x;
  oldy = thing->y;
  thing->floorz = tmfloorz;
  thing->ceilingz = tmceilingz;
  thing->x = x;
  thing->y = y;

  P_SetThingPosition (thing);
    
  // if any special lines were hit, do the effect
  if (!(thing->flags&(MF_TELEPORT|MF_NOCLIP)))
  {
    while (numspechit--)
    {
      // see if the line was crossed
      ld = spechit[numspechit];
      side = P_PointOnLineSide (thing->x, thing->y, ld);
      oldside = P_PointOnLineSide (oldx, oldy, ld);

      if (side != oldside)
      {
 	if (ld->special)
          P_CrossSpecialLine (ld-lines, oldside, thing);
      }

    }
  }

  return true;
}


//
// P_ThingHeightClip
//
// Takes a valid thing and adjusts the thing->floorz, thing->ceilingz, and
// possibly thing->z.
//
// This is called for all nearby monsters whenever a sector changes height.
//
// If the thing doesn't fit, the z will be set to the lowest value
// and false will be returned.
//
boolean P_ThingHeightClip (mobj_t* thing)
{
    boolean		onfloor;
	
    onfloor = (thing->z == thing->floorz);
	
    P_CheckPosition (thing, thing->x, thing->y);

    // what about stranding a monster partially off an edge?
	
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
	
    if (onfloor)
    {
	// walking monsters rise and fall with the floor
	thing->z = thing->floorz;
    }
    else
    {
	// don't adjust a floating monster unless forced to
	if (thing->z+thing->height > thing->ceilingz)
	    thing->z = thing->ceilingz - thing->height;
    }
	
    if (thing->ceilingz - thing->floorz < thing->height)
	return false;
		
    return true;
}



//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t		bestslidefrac;
fixed_t		secondslidefrac;

line_t*		bestslideline;
line_t*		secondslideline;

mobj_t*		slidemo;

fixed_t		tmxmove;
fixed_t		tmymove;



//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine (line_t* ld)
{
    int			side;

    angle_t		lineangle;
    angle_t		moveangle;
    angle_t		deltaangle;
    
    fixed_t		movelen;
    fixed_t		newlen;
	
	
    if (ld->slopetype == ST_HORIZONTAL)
    {
	tmymove = 0;
	return;
    }
    
    if (ld->slopetype == ST_VERTICAL)
    {
	tmxmove = 0;
	return;
    }
	
    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);
	
    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);

    if (side == 1)
	lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);
    deltaangle = moveangle-lineangle;

    if (deltaangle > ANG180)
      deltaangle += ANG180;
    //	I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;
	
    movelen = P_AproxDistance (tmxmove, tmymove);
    newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (newlen, finecosine[lineangle]);
    tmymove = FixedMul (newlen, finesine[lineangle]);	
}


//
// PTR_SlideTraverse
//
boolean PTR_SlideTraverse (intercept_t* in)
{
    line_t*	li;
	
    if (!in->isaline)
	I_Error ("PTR_SlideTraverse: not a line?");
		
    li = in->d.line;
    
    /* if ( ! (li->flags & ML_TWOSIDED) )
    {
	if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
	{
	    // don't hit the back side
	    return true;		
	}
	goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_LineOpening (li);
    
    if (openrange < slidemo->height)
	goto isblocking;		// doesn't fit
		
    if (opentop - slidemo->z < slidemo->height)
	goto isblocking;		// mobj is too high

    if (openbottom - slidemo->z > 24*FRACUNIT )
	goto isblocking;		// too big a step up

    // this line doesn't block movement
    return true;*/

  if ( !(li->flags & ML_TWOSIDED) )
  {
      if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
        return true; // hit the back side
  }

  // set openrange, opentop, openbottom
  P_LineOpening (li);
    
  // check if it can fit in the space
  if (!openrange < slidemo->height)
  {
      // Check slide mobj is not to high
      if (!opentop - slidemo->z < slidemo->height)
      {
          // Check slide mobj can step over
          if (!openbottom - slidemo->z > 24*FRACUNIT )
            return true;

          /* if (!openbottom - slidemo->z > slidemo->stepsize)
            return true - DDF here we come! use mobjinfo table */
      }
  }
	
  // the line does block movement,
  // see if it is closer than best so far
  if (in->frac < bestslidefrac)
  {
	secondslidefrac = bestslidefrac;
	secondslideline = bestslideline;
	bestslidefrac = in->frac;
	bestslideline = li;
  }
	
  return false;	// stop
}



//
// P_SlideMove
// The momx / momy move is bad, so try to slide along a wall.
//
// Find the first line hit, move flush to it, and slide along it
//
// -ACB- 1998/07/28 This is NO LONGER a kludgy mess; removed goto rubbish.
//
void P_SlideMove (mobj_t* mo)
{
    fixed_t		leadx;
    fixed_t		leady;
    fixed_t		trailx;
    fixed_t		traily;
    fixed_t		newx;
    fixed_t		newy;
    int			hitcount;
    boolean             retry;
		
    slidemo = mo;
    hitcount = 0;
    retry = true;
    
    while (retry)
    {
     if (++hitcount == 3)
     {
     	if (!P_TryMove (mo, mo->x, mo->y + mo->momy))
	  P_TryMove (mo, mo->x + mo->momx, mo->y);
	return;
     }
    
     // trace along the three leading corners
     if (mo->momx > 0)
     {
	leadx = mo->x + mo->radius;
	trailx = mo->x - mo->radius;
     }
     else
     {
	leadx = mo->x - mo->radius;
	trailx = mo->x + mo->radius;
     }
	
     if (mo->momy > 0)
     {
	leady = mo->y + mo->radius;
	traily = mo->y - mo->radius;
     }
     else
     {
	leady = mo->y - mo->radius;
	traily = mo->y + mo->radius;
     }
		
     bestslidefrac = FRACUNIT+1;
	
     P_PathTraverse ( leadx, leady, leadx+mo->momx, leady+mo->momy,
		     PT_ADDLINES, PTR_SlideTraverse );
     P_PathTraverse ( trailx, leady, trailx+mo->momx, leady+mo->momy,
		     PT_ADDLINES, PTR_SlideTraverse );
     P_PathTraverse ( leadx, traily, leadx+mo->momx, traily+mo->momy,
		     PT_ADDLINES, PTR_SlideTraverse );
    
     // move up to the wall
     if (bestslidefrac == FRACUNIT+1)
     {
	// the move most have hit the middle, so stairstep
	if (!P_TryMove (mo, mo->x, mo->y + mo->momy))
	    P_TryMove (mo, mo->x + mo->momx, mo->y);
	return;
     }

     // fudge a bit to make sure it doesn't hit
     bestslidefrac -= 0x800;
     if (bestslidefrac > 0)
     {
	newx = FixedMul (mo->momx, bestslidefrac);
	newy = FixedMul (mo->momy, bestslidefrac);
	
	if (!P_TryMove (mo, mo->x+newx, mo->y+newy))
        {
     	  if (!P_TryMove (mo, mo->x, mo->y + mo->momy))
	    P_TryMove (mo, mo->x + mo->momx, mo->y);
	  return;
        }
     }
    
     // Now continue along the wall.
     // First calculate remainder.
     bestslidefrac = FRACUNIT-(bestslidefrac+0x800);
    
     if (bestslidefrac > FRACUNIT)
	bestslidefrac = FRACUNIT;
    
     if (bestslidefrac <= 0)
	return;
    
     tmxmove = FixedMul (mo->momx, bestslidefrac);
     tmymove = FixedMul (mo->momy, bestslidefrac);

     P_HitSlideLine (bestslideline);	// clip the moves

     mo->momx = tmxmove;
     mo->momy = tmymove;
		
     if (P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove))
      retry = false;
  }
}


//
// P_LineAttack
//
mobj_t*	linetarget;	// who got hit (or NULL)
mobj_t* shootthing;     // who started the shoot

fixed_t shootz;         // startz and stopz on target
fixed_t attackrange;
fixed_t aimslope;
fixed_t shootangle;     // trace angle -ACB- 1998/09/01

int la_damage;

// slopes to top and bottom of target
extern fixed_t	topslope;
extern fixed_t	bottomslope;	

//
// PTR_AimTraverse
// Sets linetarget and aimslope when a target is aimed at.
//
static boolean PTR_AimTraverse (intercept_t* in)
{
    line_t*		li;
    mobj_t*		th;
    fixed_t		slope;
    fixed_t		thingtopslope;
    fixed_t		thingbottomslope;
    fixed_t		dist;
		
    if (in->isaline)
    {
	li = in->d.line;
	
	if ( !(li->flags & ML_TWOSIDED) )
	    return false;		// stop
	
	// Crosses a two sided line.
	// A two sided line will restrict
	// the possible target ranges.
	P_LineOpening (li);
	
	if (openbottom >= opentop)
	    return false;		// stop
	
	dist = FixedMul (attackrange, in->frac);

	if (li->frontsector->floorheight != li->backsector->floorheight)
	{
	    slope = FixedDiv (openbottom - shootz , dist);
	    if (slope > bottomslope)
	      bottomslope = slope;
	}
		
	if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
	{
	    slope = FixedDiv (opentop - shootz , dist);
	    if (slope < topslope)
	      topslope = slope;
	}
		
	if (topslope <= bottomslope)
	    return false;		// stop
			
	return true;			// shot continues
    }
    
    // shoot a thing
    th = in->d.thing;

    if (th == shootthing)
	return true;			// can't shoot self
    
    if (!(th->flags&MF_SHOOTABLE))
	return true;			// has to be able to be shot

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);

    thingtopslope = FixedDiv (th->z+th->height - shootz, dist);

    if (thingtopslope < bottomslope)
	return true;			// shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > topslope)
	return true;			// shot under the thing
    
    // this thing can be hit!
    if (thingtopslope > topslope)
      thingtopslope = topslope;
    
    if (thingbottomslope < bottomslope)
      thingbottomslope = bottomslope;

    aimslope = (thingtopslope+thingbottomslope)/2;
    linetarget = th;

    return false;			// don't go any farther
}


//
// PTR_ShootTraverse
//
// -ACB- 1998/07/28 Cleaned up.
//
static boolean PTR_ShootTraverse (intercept_t* in)
{
  fixed_t x, y, z;
  fixed_t frac;
  fixed_t slope;
  fixed_t dist;
  int     side;
  fixed_t thingtopslope;
  fixed_t thingbottomslope;

  boolean hitfloor;
  boolean hitceiling;
        
  line_t* li;
  mobj_t* th;

  // Intercept is a line?
  if (in->isaline)
  {
    li = in->d.line;

    // Line is a special, Cause action....
    if (li->special)
      P_ShootSpecialLine (shootthing, li);

    // shoot doesn't go through a one-sided line, since one sided lines
    // do not have a sector the other side.
    if (li->flags & ML_TWOSIDED)
    {
      P_LineOpening (li);
		
      dist = FixedMul (attackrange, in->frac);

      if (li->frontsector->floorheight == li->backsector->floorheight)
      {
        hitfloor = false;
      }
      else
      {
        // ceiling heights are different, check trace path
        slope = FixedDiv (openbottom - shootz, dist);

        if (slope >= aimslope)
          hitfloor = true;
        else
          hitfloor = false;
      }

      if (li->frontsector->ceilingheight == li->backsector->ceilingheight)
      {
        hitceiling = false;
      }
      else
      {
        slope = FixedDiv (opentop - shootz, dist);

        if (slope <= aimslope)
          hitceiling = true;
        else
          hitceiling = false;
      }

      // didn't hit either floor or ceiling, shot may continue...
      if (!hitceiling && !hitfloor)
        return true;
    }

    // position a bit closer
    frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);
    z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

    if (li->frontsector->ceilingpic == skyflatnum)
    {
      // don't shoot the sky!
      if (z > li->frontsector->ceilingheight)
        return false;

      if (li->backsector)
      {
        // lets check we haven't just hit the sky hack line...
        // -ACB- 1998/09/02
        if (li->backsector->ceilingpic==skyflatnum && li->backsector->floorheight<z)
          return false;

      }

    }

    // -KM- 1998/09/18 Bullet puffs can hit ceilings/floors
    side = P_PointOnLineSide(trace.x, trace.y, li);
    if (z < sides[li->sidenum[side]].sector->floorheight)
    {
       z = sides[li->sidenum[side]].sector->floorheight;
       frac = FixedDiv(z - shootz, FixedMul(aimslope, attackrange));
       x = trace.x + FixedMul (trace.dx, frac);
       y = trace.y + FixedMul (trace.dy, frac);
    }
    if (z > sides[li->sidenum[side]].sector->ceilingheight)
    {
       z = sides[li->sidenum[side]].sector->ceilingheight;
       frac = FixedDiv(z - shootz, FixedMul(aimslope, attackrange));
       x = trace.x + FixedMul (trace.dx, frac);
       y = trace.y + FixedMul (trace.dy, frac);
    }
    //dist = FixedDiv((shootz-frontceiling), aimslope);
    //x = shootthing->x + FixedMul(dist, finecosine[shootangle]);
    //y = shootthing->y + FixedMul(dist, finesine[shootangle]);

    // Spawn bullet puffs.
    P_SpawnPuff (x, y, z);
	
    // don't go any farther
    return false;
  }
    
  // shoot a thing
  th = in->d.thing;

  if (th == shootthing)
    return true;		// don't shoot self
    
  if (!(th->flags&MF_SHOOTABLE))
    return true;		// got to able to shoot it
		
  // check angles to see if the thing can be aimed at
  dist = FixedMul (attackrange, in->frac);
  thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

  if (thingtopslope < aimslope)
    return true;		// shot over the thing

  thingbottomslope = FixedDiv (th->z - shootz, dist);

  if (thingbottomslope > aimslope)
    return true;		// shot under the thing

    
  // hit thing
  // position a bit closer
  frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);

  x = trace.x + FixedMul (trace.dx, frac);
  y = trace.y + FixedMul (trace.dy, frac);
  z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

  // Spawn bullet puffs or blood spots,
  // depending on target type.
  if (in->d.thing->flags & MF_NOBLOOD)
    P_SpawnPuff (x,y,z);
  else
    P_SpawnBlood (x,y,z, la_damage, shootangle);

  if (la_damage)
    P_DamageMobj (th, shootthing, shootthing, la_damage);

  // don't go any farther
  return false;
	
}

//
// P_AimLineAttack
//
fixed_t P_AimLineAttack ( mobj_t* t1, angle_t angle, fixed_t distance )
{
  fixed_t x2;
  fixed_t y2;
	
  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;
    
  x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
  y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

  if (t1->player)
  {
    topslope = (100+(t1->vertangle>>8))*FRACUNIT/160;
    bottomslope = (-100+(t1->vertangle>>8))*FRACUNIT/160;
  }
  else
  {
    topslope = 100*FRACUNIT/160;
    bottomslope = -100*FRACUNIT/160;
  }
    
  attackrange = distance;
  linetarget = NULL;
	
  P_PathTraverse (t1->x, t1->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_AimTraverse);
		
  if (linetarget)
    return aimslope;

  return 0;
}

//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//
void
P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int damage)
{
  fixed_t x2;
  fixed_t y2;
	
  shootangle = angle;
  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;
  la_damage = damage;
  x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
  y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
  attackrange = distance;
  aimslope = slope;
		
  P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_ShootTraverse);
}

//
// P_MapTargetTheory
//
// Returns a dummy moving object for a target, used for mobjs
// that don't have a specific target; but need to launch some sort
// of projectile in the direction based upon the angle & vertical
// angle of the mobj.
//
// -ACB- 1998/09/01
//
mobj_t* P_MapTargetTheory(mobj_t* source)
{
  angle_t angle;
  fixed_t slope;
  fixed_t distance;
  static mobj_t theorytarget;

  angle = source->angle>>ANGLETOFINESHIFT;
  distance = MISSILERANGE;

  // not slope = source->vertangle; must allow for player weapon
  slope = (source->vertangle*5)/4;

  memset(&theorytarget,0,sizeof(mobj_t));

  theorytarget.x = FixedMul(distance, finecosine[angle]) + source->x;
  theorytarget.y = FixedMul(distance, finesine[angle]) + source->y;
  theorytarget.z = FixedMul(distance, slope) + source->z;

  theorytarget.extendedflags |= EF_DUMMYMOBJ;

  return &theorytarget;
}

//
// P_MapTargetAutoAim
//
// Returns a moving object for a target; will search for a mobj to
// lock onto, however a dummy target returned if no object cannot be
// locked onto.
//
// -ACB- 1998/09/01
//
mobj_t* P_MapTargetAutoAim(mobj_t* source, angle_t angle, fixed_t distance)
{
  fixed_t x2;
  fixed_t y2;
	
  angle >>= ANGLETOFINESHIFT;
  shootthing = source;
    
  x2 = source->x + (distance>>FRACBITS)*finecosine[angle];
  y2 = source->y + (distance>>FRACBITS)*finesine[angle];
  shootz = source->z + (source->height>>1) + 8*FRACUNIT;

  if (source->player)
  {
    topslope = (100+(source->vertangle>>8))*FRACUNIT/160;
    bottomslope = (-100+(source->vertangle>>8))*FRACUNIT/160;
  }
  else
  {
    topslope = 100*FRACUNIT/160;
    bottomslope = -100*FRACUNIT/160;
  }
    
  attackrange = distance;
  linetarget = NULL;
	
  P_PathTraverse (source->x, source->y, x2, y2,
                    PT_ADDLINES|PT_ADDTHINGS, PTR_AimTraverse);		
  if (linetarget)
    return linetarget;
  else
    return P_MapTargetTheory(source);
}


//
// USE LINES
//
mobj_t*		usething;

boolean	PTR_UseTraverse (intercept_t* in)
{
  int		side;
	
  if (!in->d.line->special)
  {
    P_LineOpening (in->d.line);

    if (openrange <= 0)
    {
      S_StartSound (usething, sfx_noway);
	    
      // can't use through a wall
      return false;
    }

    // not a special line, but keep checking
    return true ;
  }
	
  side = 0;

  if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
    side = 1;
    
  P_UseSpecialLine (usething, in->d.line, side);

  // can't use for than one special line in a row
  return false;
}


//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines (player_t*	player) 
{
    int		angle;
    fixed_t	x1;
    fixed_t	y1;
    fixed_t	x2;
    fixed_t	y2;
	
    usething = player->mo;
		
    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
    y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];
	
    P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
}


//
// RADIUS ATTACK
//
mobj_t*		bombsource;
mobj_t*		bombspot;
int		bombdamage;


//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
boolean PIT_RadiusAttack (mobj_t* thing)
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	dist;
	
    if (!(thing->flags & MF_SHOOTABLE) )
	return true;

    // Boss types take no damage from concussion.
    // -ACB- 1998/06/14 Changed enum reference to extended flag check.
    if (thing->info->extendedflags & EF_BOSSMAN)
	return true;	
		
    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);
    
    dist = dx>dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;

    if (dist < 0)
	dist = 0;

    if (dist >= bombdamage)
	return true;	// out of range

    if ( P_CheckSight (thing, bombspot) )
    {
	// must be in direct path
	P_DamageMobj (thing, bombspot, bombsource, bombdamage - dist);
    }
    
    return true;
}

//
// PIT_SphereAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
// -ACB- 1998/07/15 New procedure that differs for RadiusAttack - it checks
//                  Height, therefore it is a sphere attack.
//
// -ACB- 1998/08/22
// -KM- 1998/11/25 Fixed.  Added z movement for rocket jumping.
//
boolean PIT_SphereAttack (mobj_t* thing)
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	dz;
    fixed_t	dist;
    fixed_t     hdist;
	
    if (!(thing->flags & MF_SHOOTABLE) )
	return true;

    // Boss types take no damage from concussion.
    // -ACB- 1998/06/14 Changed enum reference to extended flag check.
    if (thing->info->extendedflags & EF_BOSSMAN)
	return true;	
		
    dx = abs(thing->x - bombspot->x) - thing->radius;
    dy = abs(thing->y - bombspot->y) - thing->radius;
    dz = abs(thing->z + thing->height/2 - bombspot->z);

    hdist = P_AproxDistance(dx, dy);
    dist = P_AproxDistance(hdist, dz);

    dist >>= FRACBITS;

    if (dist < 0)
      dist = 0;

    if (dist >= bombdamage)
       return true;	// out of range

    if ( P_CheckSight (thing, bombspot) )
    {
        fixed_t momz = (thing->z + thing->height/2 - bombspot->z);
        int updown = momz >= 0 ? 1 : -1;
	P_DamageMobj (thing, bombspot, bombsource, bombdamage - dist);

        if (abs(momz) < bombdamage*FRACUNIT)
        {
          momz = (bombdamage*FRACUNIT - abs(momz))/bombdamage;
          momz = FixedMul((bombdamage - dist)*(FRACUNIT>>3)*100/thing->info->mass, momz);

          thing->momz += updown * momz;
        }
    }
    return true;
}


//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack (mobj_t* spot, mobj_t* source, int damage)
{
    int		x;
    int		y;
    
    int		xl;
    int		xh;
    int		yl;
    int		yh;
    
    fixed_t	dist;
	
    dist = (damage+MAXRADIUS)<<FRACBITS;
    yh = (spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = spot;
    bombsource = source;
    bombdamage = damage;

    // -ACB- 1998/07/15 This normally does damage to everything within
    //                  a radius regards of height, however true 3D uses
    //                  a sphere attack, which checks height.
    if (gameflags.true3dgameplay)
    {
      for (y=yl ; y<=yh ; y++)
	for (x=xl ; x<=xh ; x++)
	    P_BlockThingsIterator (x, y, PIT_SphereAttack );
    }
    else
    {
      for (y=yl ; y<=yh ; y++)
	for (x=xl ; x<=xh ; x++)
	    P_BlockThingsIterator (x, y, PIT_RadiusAttack );
    }

}



//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
//
// If crunch is true, they will take damage
//  as they are being crushed.
//
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//
//  to undo the changes.
//
boolean		crushchange;
boolean		nofit;


//
// PIT_ChangeSector
//
boolean PIT_ChangeSector (mobj_t*	thing)
{
    mobj_t*	mo;
	
    if (P_ThingHeightClip (thing))
    {
	// keep checking
	return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        /* P_SetMobjState(thing,thing->info->gibbedstate) - DDF Here we come */
	P_SetMobjState (thing, S_GIBS);
	thing->flags &= ~MF_SOLID; // just been crushed, isn't solid.
	thing->height = 0;
	thing->radius = 0;
	return true;		
    }

    // dropped items get removed by a falling ceiling
    if (thing->flags & MF_DROPPED)
    {
	P_RemoveMobj (thing);
	return true;		
    }

    // if thing is not shootable, can't be crushed
    if (!(thing->flags & MF_SHOOTABLE))
      return true;

    nofit = true;

    if (crushchange && !(leveltime&3) )
    {
	P_DamageMobj(thing,NULL,NULL,10);

	// spray blood in a random direction
	mo = P_MobjCreateObject (thing->x, thing->y,
			             thing->z + thing->height/2, 
                               specials[MOBJ_BLOOD]);
	
	mo->momx = (P_Random() - P_Random ())<<12;
	mo->momy = (P_Random() - P_Random ())<<12;
    }

    // keep checking (crush other things)	
    return true;	
}



//
// P_ChangeSector
//
boolean P_ChangeSector (sector_t* sector, boolean crunch)
{
    int		x;
    int		y;
	
    nofit = false;
    crushchange = crunch;
	
    // re-check heights for all things near the moving sector
    for (x=sector->blockbox[BOXLEFT] ; x<= sector->blockbox[BOXRIGHT] ; x++)
	for (y=sector->blockbox[BOXBOTTOM];y<= sector->blockbox[BOXTOP] ; y++)
	    P_BlockThingsIterator (x, y, PIT_ChangeSector);
	
	
    return nofit;
}

//
// PIT_CorpseCheck
// Detect a corpse that could be raised.
//
// Based upon PIT_VileCheck: checks for any corpse within thing's radius.
//
// -ACB- 1998/08/22
//
static mobj_t* corpsehit;
static mobj_t* raiserobj;
static fixed_t raisertryx;
static fixed_t raisertryy;

static boolean PIT_CorpseCheck (mobj_t* thing)
{
  int maxdist;
  int oldradius;
  int oldheight;
  boolean check;
	
  if (!(thing->flags & MF_CORPSE))
    return true; // not a corpse
    
  if (thing->tics != -1)
    return true; // not lying still yet
    
  if (thing->info->raisestate == S_NULL)
    return true; // monster doesn't have a raise state

  // -KM- 1998/12/21 Monster can't be resurrected.
  if (thing->info->extendedflags & EF_NORESURRECT)
    return true;

  // -ACB- 1998/08/06 Use raiserobj for radius info
  maxdist = thing->info->radius + raiserobj->info->radius;
	
  if (abs(thing->x - raisertryx) > maxdist || abs(thing->y - raisertryy) > maxdist)
    return true; // not actually touching
		
  corpsehit = thing;
  corpsehit->momx = corpsehit->momy = 0;

  oldradius = corpsehit->radius;
  oldheight = corpsehit->height;

  // -ACB- 1998/08/22 Check making sure with have the correct radius & height.
  corpsehit->radius = corpsehit->info->radius;
  corpsehit->height = corpsehit->info->height;

  check = P_CheckPosition (corpsehit, corpsehit->x, corpsehit->y);

  // -ACB- 1998/08/22 Restore radius & height: we are only checking.
  corpsehit->radius = oldradius;
  corpsehit->height = oldheight;

  if (!check)
    return true; // doesn't fit here
		
  return false;	// got one, so stop checking
}

//
// P_MapFindCorpse
//
// Used to detect corpses that have a raise state and therefore can be
// raised. Arch-Viles (Raisers in-general) use this procedure to pick
// their corpse. NULL is returned if no corpse is found, if one is
// found it is returned.
//
// -ACB- 1998/08/22
//
mobj_t* P_MapFindCorpse(mobj_t* thing)
{
  int xlow, xhigh, xcount, ylow, yhigh, ycount;

  if (thing->movedir != DI_NODIR)
  {
    raiserobj = thing;

    // check for corpses to raise
    raisertryx = thing->x + FixedMul(thing->speed,xspeed[thing->movedir]);
    raisertryy = thing->y + FixedMul(thing->speed,yspeed[thing->movedir]);

    xlow  = (raisertryx - bmaporgx - MAXRADIUS*2)>>MAPBLOCKSHIFT;
    xhigh = (raisertryx - bmaporgx + MAXRADIUS*2)>>MAPBLOCKSHIFT;
    ylow  = (raisertryy - bmaporgy - MAXRADIUS*2)>>MAPBLOCKSHIFT;
    yhigh = (raisertryy - bmaporgy + MAXRADIUS*2)>>MAPBLOCKSHIFT;

    for (xcount=xlow; xcount<=xhigh ; xcount++)
      for (ycount=ylow; ycount<=yhigh ; ycount++)
 	if (!P_BlockThingsIterator(xcount, ycount, PIT_CorpseCheck))
          return corpsehit; // got one - return it
  }

  return NULL;
}

//
// PIT_CheckBlockingLine
//
// Used for checking that any movement between one set of coordinates does not cross
// blocking lines. If the line is twosided and has no restrictions, the move is
// allowed; the next check is to check the respective bounding boxes, see if any
// contact is made and the check is made to see if the objects are on different
// sides of the line.
//
// -ACB- 1998/08/23
//
static boolean missile;

static int mx1; //
static int my1; // Moving Object x,y cordinates
static int mx2; // for object one and object two.
static int my2; //

static int mb2; // spawn object base
static int mt2; // spawn object top

static boolean PIT_CheckBlockingLine (line_t *line)
{
  if (tmbbox[BOXRIGHT] <= line->bbox[BOXLEFT] &&
        tmbbox[BOXLEFT] >= line->bbox[BOXRIGHT] &&
         tmbbox[BOXTOP] <= line->bbox[BOXBOTTOM] &&
          tmbbox[BOXBOTTOM] >= line->bbox[BOXTOP])
  {
    return true; // no possible contact made between the respective bounding boxes.
  }

  // if the result is the same, we haven't crossed the line.
  if (P_PointOnLineSide(mx1,my1,line) == P_PointOnLineSide(mx2,my2,line))
    return true;

  if (!missile && (line->flags & (ML_BLOCKING|ML_BLOCKMONSTERS)))
    return false;

  if (line->flags & ML_TWOSIDED)
  {
    if (line->backsector->floorheight <= mb2 &&
         line->frontsector->floorheight <= mb2 &&
           line->backsector->ceilingheight >= mt2 &&
            line->frontsector->ceilingheight >= mt2)
    {
      return true; // two sided line with no restriction
    }
  }

  return false; // stop checking, objects are on different sides of a blocking line
}

//
// P_MapCheckBlockingLine
//
// Checks for a blocking line between thing and the spawnthing cooridnates
// given. Return true if there is a line; blockable indicates whether or not
// whether the ML_BLOCKING & ML_BLOCKMONSTERS should be ignored or not.
//
// -ACB- 1998/08/23
//
boolean P_MapCheckBlockingLine(mobj_t* thing, mobj_t* spawnthing)
{
  int xlow, xhigh, xcount, ylow, yhigh, ycount;

  mx1 = thing->x;
  my1 = thing->y;
  mx2 = spawnthing->x;
  my2 = spawnthing->y;
  mb2 = spawnthing->z;
  mt2 = spawnthing->z + spawnthing->info->height;
  missile = (spawnthing->flags & MF_MISSILE);

  tmbbox[BOXLEFT]   = mx1<mx2 ? mx1 : mx2;
  tmbbox[BOXRIGHT]  = mx1>mx2 ? mx1 : mx2;
  tmbbox[BOXBOTTOM] = my1<my2 ? my1 : my2;
  tmbbox[BOXTOP]    = my1>my2 ? my1 : my2;

  xlow  = (tmbbox[BOXLEFT]   - bmaporgx)>>MAPBLOCKSHIFT;
  xhigh = (tmbbox[BOXRIGHT]  - bmaporgx)>>MAPBLOCKSHIFT;
  ylow  = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
  yhigh = (tmbbox[BOXTOP]    - bmaporgy)>>MAPBLOCKSHIFT;

  validcount++;

  for (xcount=xlow; xcount<=xhigh; xcount++)
    for (ycount=ylow; ycount<=yhigh; ycount++)
      if (!P_BlockLinesIterator(xcount, ycount, PIT_CheckBlockingLine))
        return true;

  return false;
}


