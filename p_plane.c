//  
// DOSDoom Floor/Teleport Action Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/09/01 Changed for DDF.
// -ACB- 1998/09/13 Moved the teleport procedure here
//
#include "z_zone.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "i_system.h"
#include "lu_sound.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"


int maxsecs = 0;
secMove_t** activesecs = NULL;

extern int DDF_GetSecHeightReference(heightref_e ref, sector_t *sec);
static void P_RemoveActiveSector(secMove_t* sec);
static boolean P_StasifySector(sector_t* sec);
static boolean P_ActivateInStasis(int tag);

static inline int HEIGHT(sector_t* sec, boolean floorOrCeiling)
{
  if (floorOrCeiling)
    return sec->ceilingheight;

  return sec->floorheight;
}

static inline int SECPIC(sector_t* sec, boolean floorOrCeiling, int newpic)
{
  if (floorOrCeiling)
  {
    if (newpic != -1)
      sec->ceilingpic = newpic;
    return sec->ceilingpic;
  }

  if (newpic != -1)
    sec->floorpic = newpic;

  return sec->floorpic;
}



//
// FLOORS
//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void T_MoveSector (secMove_t* sec)
{
    result_e	res;
	
    switch(sec->direction)
    {
      // In Stasis
      case -2:
           sec->sfxstarted = false;
           break;

      // DOWN
      case -1:
	res = T_MovePlane(sec->sector,
			  sec->speed,
			  sec->startheight < sec->destheight ?
			  sec->startheight : sec->destheight,
			  sec->floorOrCeiling && sec->crush,
                          sec->floorOrCeiling,sec->direction);
	
	if (!sec->sfxstarted)
	{
            S_StartSound((mobj_t *)&sec->sector->soundorg, sec->type->sfxdown);
            sec->sfxstarted = true;
	}
	
	if (res == pastdest)
	{
            S_StartSound((mobj_t *)&sec->sector->soundorg, sec->type->sfxstop);
            sec->speed = sec->type->speed_up;
            sec->sector->special = sec->newspecial;
            SECPIC(sec->sector, sec->floorOrCeiling, sec->texture);

            switch(sec->type->type)
	    {
	      case mov_Continuous:
		sec->direction = 0;
                sec->waited = sec->type->wait;
                sec->speed = sec->type->speed_up;
		break;

              case mov_MoveWaitReturn:
                if (HEIGHT(sec->sector, sec->floorOrCeiling) == sec->startheight && sec->completed)
                  P_RemoveActiveSector(sec);

                else if (HEIGHT(sec->sector, sec->floorOrCeiling) == sec->destheight)
                {
                  sec->direction = 0;
                  sec->waited = sec->type->wait;
                  sec->speed = sec->type->speed_up;
                  sec->completed = true;
                }
                break;

              default:
              case mov_Stairs:
	      case mov_Once:
		P_RemoveActiveSector(sec);
		break;

	    }
	}
        else if (res == crushed)
        {
          if (sec->crush)
            sec->speed = sec->type->speed_down / 8;
          else if (sec->type->type == mov_MoveWaitReturn) // Go back up
          {
            sec->direction = 1;
            sec->sfxstarted = false;
            sec->waited = 0;
            sec->speed = sec->type->speed_up;
          }
        }

	break;

      // WAITING
      case 0:
	if (--sec->waited <= 0)
	{
            int dir = 0;
            int dest;

            if (HEIGHT(sec->sector, sec->floorOrCeiling) == sec->destheight)
              dest = sec->startheight;
            else
              dest = sec->destheight;

            if (HEIGHT(sec->sector, sec->floorOrCeiling) > dest)
            {
              dir = -1;
              sec->speed = sec->type->speed_down;
            }
            else
            {
              dir = 1;
              sec->speed = sec->type->speed_up;
            }

            sec->direction = dir; // time to go back

            if (dir)
                S_StartSound((mobj_t *)&sec->sector->soundorg,
                                     sec->type->sfxstart);
            sec->sfxstarted = false;
/*            if (dir == -1)
                S_StartSound((mobj_t *)&sec->sector->soundorg,
                                     sec->type->f.sfxdown);
            else if (dir == 1)
                S_StartSound((mobj_t *)&sec->sector->soundorg,
                                     sec->type->f.sfxup); */
	}
	break;

      // UP
      case 1:
	res = T_MovePlane(sec->sector,
			  sec->speed,
			  sec->destheight > sec->startheight ?
                          sec->destheight : sec->startheight,
			  sec->crush && !sec->floorOrCeiling,
                          sec->floorOrCeiling,sec->direction);
	
	if (!sec->sfxstarted)
	{
            S_StartSound((mobj_t *)&sec->sector->soundorg, sec->type->sfxup);
            sec->sfxstarted = true;
	}
	
	if (res == pastdest)
	{
            S_StartSound((mobj_t *)&sec->sector->soundorg,
                         sec->type->sfxstop);

            sec->sector->special = sec->newspecial;
            SECPIC(sec->sector, sec->floorOrCeiling, sec->texture);

            switch(sec->type->type)
	    {
              case mov_Continuous:
		sec->direction = 0;
                sec->waited = sec->type->wait;
                sec->speed = sec->type->speed_down;
		break;

              case mov_MoveWaitReturn:
                if (sec->completed &&
                    HEIGHT(sec->sector, sec->floorOrCeiling) == sec->startheight)
                  P_RemoveActiveSector(sec);
                else if (HEIGHT(sec->sector, sec->floorOrCeiling) == sec->destheight)
                {
                  sec->direction = 0;
                  sec->speed = sec->type->speed_down;
                  sec->waited = sec->type->wait;
                  sec->completed = true;
                }
                break;
		
	      default:
	      case mov_Once:
              case mov_Stairs:
		P_RemoveActiveSector(sec);
		break;
	    }
	    
	}
	else // ( res != pastdest )
	{
	    if (res == crushed)
	    {
                    if (sec->crush)
		      sec->speed = sec->type->speed_up / 8;
                    else // Go back down
                    {
                      sec->direction = -1;
                      sec->sfxstarted = false;
                      sec->waited = 0;
                      sec->speed = sec->type->speed_down;
                    }
	    }
	}
	break;
	
      // INITIAL WAIT
      case 2:
	if (--sec->waited <= 0)
	{
           if (sec->startheight < sec->destheight)
           {
  	     sec->direction = 1;
  	     S_StartSound((mobj_t *)&sec->sector->soundorg, sec->type->sfxup);
             sec->speed = sec->type->speed_up;
           }
           else
           {
             sec->direction = -1;
             S_StartSound((mobj_t *)&sec->sector->soundorg, sec->type->sfxdown);
             sec->speed = sec->type->speed_down;
           }

           sec->type->type = mov_MoveWaitReturn;
           sec->waited = sec->type->wait;
	}
	break;

    }

}

static sector_t* P_GetSectorSurrounding(sector_t* sec, int dest, boolean forc)
{
  int i;
  int secnum = sec - sectors;
  sector_t*  sector;

  for (i=sec->linecount; i--;)
  {
    if (twoSided(secnum, i))
    {
      if (getSide(secnum,i,0)->sector-sectors == secnum)
      {
        sector = getSector(secnum,i,1);

        if ((SECPIC(sector, forc, -1) != SECPIC(sec, forc, -1))
                    && (HEIGHT(sector, forc) == dest))
        {
          return sector;
        }

      }
      else
      {
        sector = getSector(secnum,i,0);

        if ((SECPIC(sector, forc, -1) != SECPIC(sec, forc, -1))
                    && (HEIGHT(sector, forc) == dest))
        {
          return sector;
        }
      }
    }
  }
  return NULL;
}

//
// P_SetupFloorAction
//
// Setup the Floor Action, depending on the linedeftype trigger and the
// sector info.
//
// -ACB- 1998/09/09 Renamed function from DoFloor.
//                  Instant movement floor added.
//                  Reformatted and cleaned function.
//
static secMove_t* P_SetupSectorAction(sector_t* sector, movinPlane_t* type, sector_t* model)
{
  secMove_t* sec;
  int start, dest;

  // new door thinker
  sec = Z_Malloc (sizeof(*sec), PU_LEVSPEC, 0);

  sector->specialdata[type->floorOrCeiling] = sec;
  sec->thinker.function.acp1 = (actionf_p1)T_MoveSector;
  sec->sector = sector;
  sec->crush = type->crush;
  sec->sfxstarted = sec->completed = false;
  start = HEIGHT(sector, type->floorOrCeiling);

  dest = DDF_GetSecHeightReference(type->destref, sector);
  dest += type->dest;

  //--------------------------------------------------------------------------
  // Floor Speed Notes:
  //
  //   Floor speed setup; -1 is the default speed, This is used to simulate
  //   the use of an instant movement. i.e. a floor that raises or falls to
  //   its destination height in one tic: This is also implemented for WAD's
  //   that use odd linedef requests to achieve the instant effect.
  //
  //   Therefore a speed of -1, is translated to the distance between the
  //   start and destination: instant movement; otherwise the speed is taken
  //   from the linedef type.
  //
  //--------------------------------------------------------------------------
  if (type->prewait)
  {
    sec->direction = 0;
    sec->waited = type->prewait;
  } else if (dest > start)
  {
    sec->direction = 1;

    // -ACB- 1998/09/09 See floor speed notes...
    if (type->speed_up != -1)
      sec->speed = type->speed_up;
    else
      sec->speed = dest - start;
  }
  else if (start > dest)
  {
    sec->direction = -1;

    // -ACB- 1998/09/09 See floor speed notes...
    if (type->speed_down != -1)
      sec->speed = type->speed_down;
    else
      sec->speed = start - dest;
  }
  else
  {
    sector->specialdata[type->floorOrCeiling] = NULL;
    Z_Free(sec);
    return NULL;
  }

  sec->destheight = dest;
  sec->startheight = start;
  sec->tag = sector->tag;
  sec->type = type;
  sec->texture = SECPIC(sector, type->floorOrCeiling, -1);
  sec->newspecial = sector->special;
  sec->wait = type->wait;
  sec->floorOrCeiling = type->floorOrCeiling;

  // change to surrounding
  if (type->tex[0] == '-')
  {
    model = P_GetSectorSurrounding(sector, sec->destheight, type->floorOrCeiling);
    if (model)
    {
      sec->texture = SECPIC(model, type->floorOrCeiling, -1);
      sec->newspecial = model->special;
    }
    if (sec->direction == (type->floorOrCeiling ? -1 : 1))
    {
      SECPIC(sector, type->floorOrCeiling, sec->texture);
    }
  }
  else if (type->tex[0] == '+')
  {
    sec->texture = SECPIC(model, type->floorOrCeiling, -1);
    sec->newspecial = model->special;
    if (sec->direction == (type->floorOrCeiling ? -1 : 1))
    {
      SECPIC(sector, type->floorOrCeiling, sec->texture);
    }
  }
  else if (type->tex[0] != NULL)
  {
    sec->texture = R_FlatNumForName(type->tex);
  }

  P_AddActiveSector(sec);
  return sec;
}

//
// EV_Teleport
//
// Teleportation is an effect which is simulated by searching for the first
// special[MOBJ_TELEPOS] in a sector with the same tag as the activation line,
// moving an object from one sector to another upon the MOBJ_TELEPOS found, and
// possibly spawning an effect object (i.e teleport flash) at either the entry &
// exit points or both.
//
// -KM- 1998/09/01 Added stuff for lines.ddf (mostly sounds)
//
// -ACB- 1998/09/11 Reformatted and cleaned up.
//
// -ACB- 1998/09/12 Teleport delay setting from linedef.
//
// -ACB- 1998/09/13 used effect objects: the objects themselves make any sound and
//                  the in effect object can be different to the out object.
//
// -ACB- 1998/09/13 Removed the missile checks: no need since this would have been
//                  Checked at the linedef stage.
// -KM- 1998/11/25 Changed Erik's code a bit, Teleport flash still appears.
//  if def faded_teleportation == 1, doesn't if faded_teleportation == 2
//
// -ES- 1998/11/28 Changed Kester's code a bit :-) Teleport method can now be
//  toggled in the menu. (That is the way it should be. -KM)
//
void R_StartFading(int,int);
int faded_teleportation;
boolean EV_Teleport (line_t* line, int side, mobj_t* thing, int delay,
                       mobjinfo_t* ineffectobj, mobjinfo_t *outeffectobj)
{
  int i;
  int tag;
  angle_t an;
  mobj_t* currmobj;
  sector_t* sector;
  fixed_t oldx;
  fixed_t oldy;
  fixed_t oldz;
  mobj_t *fog;

  // Don't teleport if hit back of line, so you can get out of teleporter.
  if (!thing)
    return false;
    
  tag = line->tag;

  for (i = 0; i < numsectors; i++)
  {
    if (sectors[i].tag == tag)
    {
      currmobj = mobjlisthead;

      while (currmobj != NULL)
      {
        // not a teleportman
        if (currmobj->info != specials[MOBJ_TELEPOS])
        {
          currmobj = currmobj->next;
          continue;
        }

        sector = currmobj->subsector->sector;

	// wrong sector
        if (sector-sectors != i)
        {
          currmobj = currmobj->next;
          continue;
        }

        oldx = thing->x;
        oldy = thing->y;
        oldz = thing->z;
				
        if (!P_TeleportMove (thing, currmobj->x, currmobj->y))
          return false;

        // -KM- 1998/09/01 Allows a spawn on ceiling teleport ... :)
        thing->z = (currmobj->flags & MF_SPAWNCEILING) ?
                     thing->ceilingz - thing->height : thing->floorz;

        if (thing->player)
          thing->player->viewz = thing->z+thing->player->viewheight;
				
	// spawn teleport fog at source and destination
        P_MobjCreateObject(oldx, oldy, oldz, ineffectobj);

        an=currmobj->angle >> ANGLETOFINESHIFT;

        //
        // -ACB- 1998/09/06 Switched 40 to 20. This by my records is
        //                  the original setting.
        //
        // -ES- 1998/10/29 When fading, we don't want to see the fog.
        fog = P_MobjCreateObject (currmobj->x+20*finecosine[an],
                                  currmobj->y+20*finesine[an],
                                     currmobj->z, outeffectobj);
        if (thing->player && faded_teleportation == 2)
          fog->deltainvis = fog->invisibility = INVISIBLE;

        // don't move for a bit
        if (thing->player)
        {
          thing->reactiontime = delay;
          // -ES- 1998/10/29 Start the fading
          if (faded_teleportation)
            R_StartFading(0,(delay*5)/2);
          thing->momx = thing->momy = thing->momz = 0;
        }
        else if (thing->flags & MF_MISSILE)
        {
          thing->z = thing->floorz + thing->origheight;

          thing->momx=FixedMul(thing->speed,
                                 finecosine[currmobj->angle>>ANGLETOFINESHIFT]);

          thing->momy=FixedMul(thing->speed,
                                 finesine[currmobj->angle>>ANGLETOFINESHIFT]);
        }

        thing->angle = currmobj->angle;
        return true;

      } // while (currmobj)

    } // if (sector)

  } // for (sector) loop

  return false;
}

//
// BUILD A STAIRCASE!
//
static boolean EV_BuildStairs (sector_t* sector, movinPlane_t* type)
{
    int			secnum;
    int			height;
    int			i;
    int			newsecnum;
    int			texture;
    boolean		ok;
    boolean		rtn = false;

    secMove_t*           stairs;
    
    sector_t*		sec;
    sector_t*		tsec;

    fixed_t		stairsize = type->dest;

    secnum = -1;
    while ((secnum = P_FindSectorFromTag(sector->tag,secnum)) >= 0)
    {
	sec = &sectors[secnum];
		
	// ALREADY MOVING?  IF SO, KEEP GOING...
	if (sec->specialdata[type->floorOrCeiling])
	    continue;
	
	// new floor thinker
        stairs = P_SetupSectorAction(sec, type, sec);
	rtn = stairs ? true : rtn;
        height = stairsize;
	texture = sec->floorpic;
	
	// Find next sector to raise
	// 1.	Find 2-sided line with same sector side[0]
	// 2.	Other side is the next sector to raise
	do
	{
	    ok = false;
	    for (i = 0;i < sec->linecount;i++)
	    {
		if ( !((sec->lines[i])->flags & ML_TWOSIDED) )
		    continue;
					
		tsec = (sec->lines[i])->frontsector;
		newsecnum = tsec-sectors;
		
		if (secnum != newsecnum)
		    continue;

		tsec = (sec->lines[i])->backsector;
		newsecnum = tsec - sectors;

		if (tsec->floorpic != texture)
		    continue;
					
		if (tsec->specialdata[type->floorOrCeiling])
		    continue;
					
		sec = tsec;
		secnum = newsecnum;

		stairs = P_SetupSectorAction(sec, type, sec);
                if (stairs)
                {
                  stairs->destheight += height;
                  ok = true;
                }
		height += stairsize;

		break;
	    }
	} while(ok);
    }
    return rtn;
}



//
// Do Platforms/Floors/Stairs/Ceilings/Doors
//
boolean EV_DoSector (sector_t* sec, movinPlane_t* type, sector_t* model)
{
  // Activate all <type> plats that are in_stasis
  switch(type->type)
  {
    case mov_Continuous:
      if (P_ActivateInStasis(sec->tag))
        return true;
      break;

    case mov_Stairs:
      return EV_BuildStairs(sec, type);

    case mov_Stop:
      return P_StasifySector(sec);

    default:
      break;
  }
	
  if (sec->specialdata[type->floorOrCeiling])
      return false;
	
  // Do Floor action
  return P_SetupSectorAction(sec, type, model) ? true : false;
}

boolean
EV_Manual
( line_t*	        line,
  mobj_t*	        thing,
  movinPlane_t*         type)
{
    int		secnum;
    sector_t*	sec;
    secMove_t*	msec;
    int		side;
    int         dir = 1;
    int         olddir = 1;
	
    side = 0;	// only front sides can be used

    // if the sector has an active thinker, use it
    sec = sides[line->sidenum[side^1]].sector;
    secnum = sec-sectors;

    if (sec->specialdata[type->floorOrCeiling] && thing)
    {
	msec = sec->specialdata[type->floorOrCeiling];
	switch(type->type)
	{
          case  mov_MoveWaitReturn:
            olddir = msec->direction;
            // Only players close doors
            if ((msec->direction != -1) && thing->player)
              dir = msec->direction = -1;
            else
              dir = msec->direction = 1;
            break;
          default:
            break;
        }
        if (dir != olddir)
        {
            S_StartSound((mobj_t *)&sec->soundorg, type->sfxstart);
            msec->sfxstarted = false;
            return true;
        }
        return false;
    }

    return EV_DoSector(sec, type, sec);
}


static boolean P_ActivateInStasis(int tag)
{
    int		i;
    boolean     rtn = false;
	
    for (i = 0;i < maxsecs;i++)
	if (activesecs[i]
	    && (activesecs[i])->tag == tag
	    && (activesecs[i])->direction == -2)
	{
	    (activesecs[i])->direction = (activesecs[i])->olddirection;
	    (activesecs[i])->thinker.function.acp1
	      = (actionf_p1) T_MoveSector;
            rtn = true;
	}

    return rtn;
}

static boolean P_StasifySector(sector_t* sec)
{
    int		j;
    boolean     rtn = false;
	
    for (j = 0;j < maxsecs;j++)
	if (activesecs[j]
	    && ((activesecs[j])->direction != -2)
	    && ((activesecs[j])->tag == sec->tag))
	{
	    (activesecs[j])->olddirection = (activesecs[j])->direction;
	    (activesecs[j])->direction = -2;
	    (activesecs[j])->thinker.function.acv = (actionf_v)NULL;
            rtn = true;
	}

    return rtn;
}

void P_AddActiveSector(secMove_t* sec)
{
  int i;
    
  for (i = 0;i < maxsecs;i++)
  {
    if (activesecs[i] == NULL)
      break;
  }

  if (i == maxsecs)
    activesecs = Z_ReMalloc(activesecs, sizeof(secMove_t**) * ++maxsecs);

  activesecs[i] = sec;
  P_AddThinker(&activesecs[i]->thinker);
}

static void P_RemoveActiveSector(secMove_t* sec)
{
    int i;

    for (i = 0;i < maxsecs;i++)
    {
	if (sec == activesecs[i])
	{
	    sec->sector->specialdata[sec->floorOrCeiling] = NULL;
	    P_RemoveThinker(&(activesecs[i])->thinker);
	    activesecs[i] = NULL;
	    return;
	}
    }

    I_Error ("P_RemoveActiveSector: can't find sector!");
}

void P_ResetActiveSecs(void)
{
  int i;
  for (i = 0;i < maxsecs;i++)
    activesecs[i] = NULL;
}

//
// Move a plane (floor or ceiling) and check for crushing
//
result_e
T_MovePlane
( sector_t*	sector,
  fixed_t	speed,
  fixed_t	dest,
  boolean	crush,
  int		floorOrCeiling,
  int		direction )
{
    boolean	flag;
    fixed_t	lastpos;
	
    switch(floorOrCeiling)
    {
      case 0:
	// FLOOR
	switch(direction)
	{
	  case -1:
	    // DOWN
	    if (sector->floorheight - speed < dest)
	    {
		lastpos = sector->floorheight;
		sector->floorheight = dest;
		flag = P_ChangeSector(sector,crush);
		if (flag == true)
		{
		    sector->floorheight =lastpos;
		    P_ChangeSector(sector,crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		lastpos = sector->floorheight;
		sector->floorheight -= speed;
		flag = P_ChangeSector(sector,crush);
		if (flag == true)
		{
		    sector->floorheight = lastpos;
		    P_ChangeSector(sector,crush);
		    return crushed;
		}
	    }
	    break;
						
	  case 1:
	    // UP
	    if (sector->floorheight + speed > dest)
	    {
		lastpos = sector->floorheight;
		sector->floorheight = dest;
		flag = P_ChangeSector(sector,crush);
		if (flag == true)
		{
		    sector->floorheight = lastpos;
		    P_ChangeSector(sector,crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		// COULD GET CRUSHED
		lastpos = sector->floorheight;
		sector->floorheight += speed;
		flag = P_ChangeSector(sector,crush);
		if (flag == true)
		{
		    if (crush == true)
			return crushed;
		    sector->floorheight = lastpos;
		    P_ChangeSector(sector,crush);
		    return crushed;
		}
	    }
	    break;
	}
	break;
									
      case 1:
	// CEILING
	switch(direction)
	{
	  case -1:
	    // DOWN
	    if (sector->ceilingheight - speed < dest)
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight = dest;
		flag = P_ChangeSector(sector,crush);

		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_ChangeSector(sector,crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		// COULD GET CRUSHED
		lastpos = sector->ceilingheight;
		sector->ceilingheight -= speed;
		flag = P_ChangeSector(sector,crush);

		if (flag == true)
		{
		    if (crush == true)
			return crushed;
		    sector->ceilingheight = lastpos;
		    P_ChangeSector(sector,crush);
		    return crushed;
		}
	    }
	    break;
						
	  case 1:
	    // UP
	    if (sector->ceilingheight + speed > dest)
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight = dest;
		flag = P_ChangeSector(sector,crush);
		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_ChangeSector(sector,crush);
		    //return crushed;
		}
		return pastdest;
	    }
	    else
	    {
		lastpos = sector->ceilingheight;
		sector->ceilingheight += speed;
		flag = P_ChangeSector(sector, crush);
// UNUSED:  Can't crush if the ceiling is on the way up...
#if 0
		if (flag == true)
		{
		    sector->ceilingheight = lastpos;
		    P_ChangeSector(sector,crush);
		    return crushed;
		}
#endif
	    }
	    break;
	}
	break;
		
    }
    return ok;
}

static linedeftype_t donut[2] =
{
  {
    0,                                      // Trigger num
    0,
    0,                                      // How triggered (walk/switch/shoot)
    0,                                      // OBJs that can trigger it
    KF_NONE,                               // No key needed
    NULL,
    1,                                      // Can be activated repeatedly
    0,                                      // Special type: CANCEL ACID
    false,                                  // Crushing

    {                                            // Floor:
      mov_Once,                                  //  Type
      false, false,
      FLOORSPEED/2, FLOORSPEED/2,                //  speed up/down
      ref_absolute,                              //  dest ref
      32767*FRACUNIT,                            //  dest
      "-",                                       //  texture
      0, 0,                                      //  wait, prewait
      0, 0, 0, 0                                 //  SFX start/up/down/stop
    },
    {                                            // Ceiling:
      mov_undefined,                             //  Type
      true, false,
      CEILSPEED, CEILSPEED,                      //  speed up/down
      ref_absolute,                              //  dest ref
      0,                                         //  dest
      "",                                        //  texture
      0, 0,                                      //  wait, prewait
      0, 0, 0, 0                                 //  SFX start/up/down/stop
    },
    {                                            // Donut:
      false,                                     //  dodonut
      0, 0, 0, 0
    },
    {                                            // Teleport:
      false,                                     //  is a teleporter
      NULL,                                      //  effect object -> in
      NULL,                                      //  effect object -> out
      0                                          //  delay
    },
    {                                            // Lights:
      lite_none                                 //  lights Action
    },
    0,                                           // Not an exit
    0, FRACUNIT,                                 // Not scrolling
    NULL,                                        // Failed Security Message
    "\0",                                        // Colourmaplump
    -1,                                          // Colourmap
    0,                                           // Gravity
    0,
    0,                                          // SFX
    "",                                         // Music
    false,                                      // Autoline
    false,                                      // Two sided
    false                                        // Hash table
  },
  {
    0,                                      // Trigger num
    0,
    0,                                      // How triggered (walk/switch/shoot)
    0,                                      // OBJs that can trigger it
    KF_NONE,                                // No key needed
    NULL,
    1,                                      // Can be activated repeatedly
    -1,                                     // Special type: don't change.
    false,                                  // Crushing
    {                                            // Floor:
      mov_Once,                                  //  Type
      false, false,
      FLOORSPEED/2, FLOORSPEED/2,                //  speed up/down
      ref_absolute,                              //  dest ref
      -32767*FRACUNIT,                           //  dest
      "",                                        //  texture
      0, 0,                                      //  wait, prewait
      0, 0, 0, 0
    },
    {                                            // Ceiling:
      mov_undefined,                             //  Type
      true, false,
      CEILSPEED, CEILSPEED,                      //  speed up/down
      ref_absolute,                              //  dest ref
      0,                                         //  dest
      "",                                        //  texture
      0, 0,                                      //  wait, prewait
      0, 0, 0, 0
    },
    {                                            // Donut:
      false                                     //  dodonut
    },
    {                                            // Teleport:
      false,                                     //  is a teleporter
      NULL,                                      //  effect object -> in
      NULL,                                      //  effect object -> out
      0                                          //  delay
    },
    {                                            // Lights:
      lite_none,                                 //  light action
      64                                         //  Set light to this level
    },
    0,                                           // Not an exit
    0, FRACUNIT,                                 // Not scrolling
    NULL,                                        // Failed Security String
    "\0",                                        // Colourmaplump
    -1,                                          // Colourmap
    0,                                           // Gravity
    0,                                           // Friction
    0,                                          // SFX
    "",                                         // Music
    false,                                      // Autoline
    false,                                      // Two sided
    false                                        // Hash table
  }
};

//
// Special Stuff that can not be categorized
// Mmmmmmm....  Donuts....
//
boolean EV_DoDonut (sector_t* s1, sfx_t* sfx[4])
{
    sector_t*		s2;
    sector_t*		s3;
    boolean		rtn = false;
    int			i;
    secMove_t*          sec;
	
    // ALREADY MOVING?  IF SO, KEEP GOING...
    if (s1->specialdata[FLOOR])
      return false;
    
    s2 = getNextSector(s1->lines[0],s1);
    for (i = 0;i < s2->linecount;i++)
    {
      if ((!s2->lines[i]->flags & ML_TWOSIDED) ||
        (s2->lines[i]->backsector == s1))
          continue;

      s3 = s2->lines[i]->backsector;
      
      rtn = true;
      
      //	Spawn rising slime
      donut[0].f.sfxup = sfx[0];
      donut[0].f.sfxstop = sfx[1];
      sec = P_SetupSectorAction(s2, &donut[0].f, s3);
      if (sec)
      {
        sec->destheight = s3->floorheight;
        s2->floorpic = sec->texture = s3->floorpic;
      }
      
      //	Spawn lowering donut-hole
      donut[1].f.sfxup = sfx[2];
      donut[1].f.sfxstop = sfx[3];
      sec = P_SetupSectorAction(s1, &donut[1].f, s1);
      if (sec)
        sec->destheight = s3->floorheight;
      break;
    }
    return rtn;
}


