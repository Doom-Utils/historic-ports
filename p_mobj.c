//
// DOSDoom Moving Object Handling Code  
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -MH- 1998/07/02  "shootupdown" --> "true3dgameplay"
//
// -ACB- 1998/07/30 Took an axe to the item respawn code: now uses a
//                  double-linked list to store to individual items;
//                  limit removed; P_MobjItemRespawn replaces P_RespawnSpecials
//                  as the procedure that handles respawning of items.
//
//                  P_NightmareRespawnOld -> P_TeleportRespawn
//                  P_NightmareRespawnNew -> P_ResurrectRespawn
//
// -ACB- 1998/07/31 Use new procedure to handle flying missiles that hammer
//                  into sky-hack walls & ceilings. Also don't explode the
//                  missile if it hits sky-hack ceiling or floor.
//
// -ACB- 1998/08/06 Implemented limitless mobjinfo list, altered/removed all
//                  mobjinfo[] references.
//

#include "d_debug.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "hu_stuff.h"
#include "i_system.h"
#include "lu_sound.h"
#include "m_argv.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "z_zone.h"

void G_PlayerReborn (int player);
void P_SpawnMapThing (mapthing_t* mthing);

// Holds the players real Z location, so the missile when teleported to
// a different height sector stay relative to it.
fixed_t realplayerz=0; //-jc-

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
int test;

boolean P_SetMobjState (mobj_t* mobj, statenum_t state)
{
  state_t* st;

  do
  {
    if (state == S_NULL)
    {
      mobj->state = (state_t *) S_NULL;
      P_RemoveMobj (mobj);
      return false;
    }

    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    if (st->action.acp1)
      st->action.acp1(mobj);
	
    state = st->nextstate;
  }
  while (!mobj->tics);
				
  return true;
}

//
// P_ExplodeMissile  
//
void P_ExplodeMissile (mobj_t* mo)
{
    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, mo->info->deathstate);//mobjinfo used -ACB- 1998/08/06

    mo->tics -= P_Random()&3;

    if (mo->tics < 1)
	mo->tics = 1;

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
	S_StartSound (mo, mo->info->deathsound);
}

//
// P_XYMovement  
//

void P_XYMovement (mobj_t* mo) 
{ 	
    fixed_t 	ptryx;
    fixed_t	ptryy;
    player_t*	player;
    fixed_t	xmove;
    fixed_t	ymove;
			
    if (!mo->momx && !mo->momy)
    {
	if (mo->flags & MF_SKULLFLY)
	{
	    // the skull slammed into something
	    mo->flags &= ~MF_SKULLFLY;
	    mo->momx = mo->momy = mo->momz = 0;

	    P_SetMobjState (mo, mo->info->spawnstate);
	}
	return;
    }
	
    player = mo->player;
		
    if (mo->momx > MAXMOVE) mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE) mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE) mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE) mo->momy = -MAXMOVE;
		
    xmove = mo->momx;
    ymove = mo->momy;

    // Keep attempting moves until object has lost all momentum.
    do
    {
        // if movement is more than half that of the maximum, attempt the move
        // in two halves or move.
	if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
	{
	    ptryx = mo->x + xmove/2;
	    ptryy = mo->y + ymove/2;
	    xmove >>= 1;
	    ymove >>= 1;
	}
	else
	{
	    ptryx = mo->x + xmove;
	    ptryy = mo->y + ymove;
	    xmove = ymove = 0;
	}
		
	if (!P_TryMove (mo, ptryx, ptryy)) // unable to complete desired move
	{
            /* if (mo->info->extendedflags & EF_SLIDEMOVE) DDF Here we come! */
	   if (mo->player) // try to slide along it
	   {
	     P_SlideMove (mo);
	   }
           else if (mo->flags & MF_MISSILE)
	   {
                //
                // This prevents missiles hitting a
                // wall that acts as sky. -ACB- 1998/07/31
                //
                // and Yes, one big if condition irritates me :).
                //
                if (!ceilingline)
                  P_ExplodeMissile(mo);
                else if (!ceilingline->backsector)
                  P_ExplodeMissile(mo);
                else if (ceilingline->backsector->ceilingpic != skyflatnum)
                  P_ExplodeMissile(mo);
                else if (mo->z <= ceilingline->backsector->ceilingheight)
                  P_ExplodeMissile(mo);
                else
                  P_MobjRemoveMissile(mo); // New Procedure -ACB- 1998/07/30
                return;
	    }
            else
              mo->momx = mo->momy = 0;
	}
    }
    while (xmove || ymove);
    
    // slow down
    if (player && player->cheats & CF_NOMOMENTUM)
    {
	// debug option for no sliding at all
	mo->momx = mo->momy = 0;
	return;
    }

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
	return; 	// no friction for missiles ever
		
    // -MH- 1998/08/18 - make mid-air movement normal when using the jetpack
    //                    When in mid-air there's no friction so you slide
    //                    about uncontrollably. This is realistic but makes
    //                    the game difficult to control to the extent that
    //                    for normal people, it's not worth playing - a bit
    //                    like having auto-aim permanently off (as most
    //                    real people are not crack-shots!)
    if ((mo->z > mo->floorz)
        && !(gameflags.true3dgameplay && mo->player && mo->player->powers[pw_jetpack]))
        return;      // no friction when airborne unless player has jetpack

    if (mo->flags & MF_CORPSE)
    {
	// do not stop sliding
	//  if halfway off a step with some momentum
	if (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4 ||
             mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4)
	{
	    if (mo->floorz != mo->subsector->sector->floorheight)
		return;
	}
    }

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED && mo->momy > -STOPSPEED
	&& mo->momy < STOPSPEED &&
        (!player || (player->cmd.forwardmove==0 && player->cmd.sidemove==0)))
    {
	// if in a walking frame, stop moving
        // -KM- 1998/12/16 Use info->spawnstate instead of S_PLAY_RUN1 etc
	if ( player&&(unsigned)((player->mo->state - states)- player->mo->info->seestate) < 4)
	    P_SetMobjState (player->mo, player->mo->info->spawnstate);
	
	mo->momx = 0;
	mo->momy = 0;
    }
    else
    {
        // -KM- 1998/12/16 Added viscosity so you can't get stuck in mid air in a
        //   no gravity sector.
	mo->momx = FixedMul (mo->momx, FixedMul(mo->subsector->sector->friction, FRACUNIT - mo->subsector->sector->viscosity));
	mo->momy = FixedMul (mo->momy, FixedMul(mo->subsector->sector->friction, FRACUNIT - mo->subsector->sector->viscosity));
    }
}

//
// P_ZMovement
//
void P_ZMovement (mobj_t* mo)
{
  fixed_t dist;
  fixed_t delta;
  // -KM- 1998/11/25 Gravity is now not precalculated so that
  //  menu changes affect instantly.
  fixed_t gravity = mo->subsector->sector->gravity*gameflags.grav;

  // check for smooth step up
  if (mo->player && mo->z < mo->floorz)
  {
    mo->player->viewheight -= mo->floorz-mo->z;
    mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight)>>3;
  }
    
  // adjust height
  mo->z += mo->momz;
	
  if (mo->flags & MF_FLOAT && mo->target)
  {
    // float down towards target if too close
    if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
    {
      dist = P_AproxDistance (mo->x - mo->target->x, mo->y - mo->target->y);
      delta = (mo->target->z + (mo->height>>1)) - mo->z;

      if (delta<0 && dist < -(delta*3))
        mo->z -= FLOATSPEED;
      else if (delta>0 && dist < (delta*3) )
        mo->z += FLOATSPEED;
    }
	
  }

  // clip movement
  if (mo->z <= mo->floorz)
  {
    // Note (id): somebody left this after the setting momz to 0, kinda useless there.
    if (mo->flags & MF_SKULLFLY)
      mo->momz = -mo->momz;
	
    if (mo->momz < 0)
    {
      if (mo->player && mo->momz < -gravity*8)
      {
 	// Squat down. Decrease viewheight for a moment after hitting the
        // ground (hard), and utter appropriate sound.
        mo->player->deltaviewheight = mo->momz>>3;
        S_StartSound (mo, sfx_oof);
      }
      // -KM- 1998/12/16 If bigger than max fall, take damage.
      if (mo->info->maxfall && mo->momz < FixedMul(-gravity, mo->info->maxfall))
          P_DamageMobj(mo, NULL, NULL, -mo->momz - FixedMul(gravity, mo->info->maxfall));

      mo->momz = 0;
    }

    mo->z = mo->floorz;

    if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
    {
      // if the floor is sky, don't explode missile -ACB- 1998/07/31
      if (mo->subsector->sector->floorpic != skyflatnum)
        P_ExplodeMissile(mo);
      else
        P_MobjRemoveMissile(mo);
      return;
    }
  }
  else if (!(mo->flags & MF_NOGRAVITY)
           && !(gameflags.true3dgameplay && mo->player && mo->player->powers[pw_jetpack]))
  {
    // -MH- 1998/08/18 - Disable gravity while player has jetpack
    //                   (nearly forgot this one:-)
    if (mo->momz == 0)
      mo->momz = FixedMul(-gravity*2, FRACUNIT-mo->subsector->sector->viscosity);
    else
      mo->momz -= FixedMul(gravity, FRACUNIT-mo->subsector->sector->viscosity);
  }

  if (mo->z + mo->height > mo->ceilingz)
  {
    // hit the ceiling
    if (mo->momz > 0)
      mo->momz = 0;

    mo->z = mo->ceilingz - mo->height;

    if (mo->flags & MF_SKULLFLY)
      mo->momz = -mo->momz; // the skull slammed into something
	
    if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
    {
      // if the ceiling is sky, don't explode missile -ACB- 1998/07/31
      if (mo->subsector->sector->ceilingpic != skyflatnum)
        P_ExplodeMissile(mo);
      else
        P_MobjRemoveMissile(mo);
      return;
    }
  }

  if (gameflags.true3dgameplay && mo->player)
  {
    if ((mo->momz > -STOPSPEED) && (mo->momz < STOPSPEED)
      && (mo->player->cmd.upwardmove==0))
    {
   	mo->momz = 0;
    }
    else
    {
        mo->momz = FixedMul (mo->momz, FixedMul(mo->subsector->sector->friction, FRACUNIT - mo->subsector->sector->viscosity));
    }
  }
}

//
// P_TeleportRespawn
//
void P_TeleportRespawn (mobj_t* mobj)
{
  fixed_t x;
  fixed_t y;
  fixed_t z;
  mobj_t* mo;
  mapthing_t* mthing;
  subsector_t* ss;
		
  x = mobj->spawnpoint.x << FRACBITS;
  y = mobj->spawnpoint.y << FRACBITS;

  // something is occupying it's position?
  if (!P_CheckPosition (mobj, x, y))
    return;	// no respawn

  // spawn a teleport fog at old spot
  // because of removal of the body?

  // temp fix for teleport flash...
  mo = P_MobjCreateObject (mobj->x,mobj->y,
                            mobj->subsector->sector->floorheight,
                             DDF_MobjLookup("TELEPORT FLASH"));

  // spawn a teleport fog at the new spot
  ss = R_PointInSubsector (x,y);

  // temp fix for teleport flash...
  mo=P_MobjCreateObject(x,y,ss->sector->floorheight,DDF_MobjLookup("TELEPORT FLASH"));

  // spawn the new monster
  mthing = &mobj->spawnpoint;
	
  // spawn it
  if (mobj->info->flags & MF_SPAWNCEILING)
    z = ONCEILINGZ;
  else
    z = ONFLOORZ;

  // inherit attributes from deceased one
  // -ACB- 1998/08/06 Create Object
  mo = P_MobjCreateObject(x,y,z,mobj->info);

  mo->spawnpoint = mobj->spawnpoint;	
  mo->angle = ANG45 * (mthing->angle/45);

  if (mthing->options & MTF_AMBUSH)
    mo->flags |= MF_AMBUSH;

  mo->reactiontime = 18;
	
  // remove the old monster.
  P_RemoveMobj (mobj);
}

//
// P_ResurrectRespawn
//
// -ACB- 1998/07/29 Prevented respawning of ghosts
//                  Make monster deaf, if originally deaf
//                  Given a reaction time, delays monster starting up immediately.
//                  Given Invisibility level.
//                  Doesn't try to raise an object with no raisestate
//
void P_ResurrectRespawn (mobj_t* object)
{
  int oldradius;
  int oldheight;
  fixed_t x;
  fixed_t y;
  mobjinfo_t* info;
  mapthing_t* mthing;

  x = object->x;
  y = object->y;

  info = object->info;

  if (!object->info->raisestate)
    return; // cannot raise the unraisable

  // restore objects original height
  oldradius = object->radius;
  oldheight = object->height;
  object->radius = info->radius;
  object->height = info->height;

  // something is occupying it's position?
  // restore original height is position occupied
  if (!P_CheckPosition (object, x, y) )
  {
    object->radius = oldradius;
    object->height = oldheight;
    return;
  }

  if (object->state == &states[S_GIBS])
  {
    object->radius = oldradius;
    object->height = oldheight;
    return; // don't respawn gibs
  }

  // Resurrect monster
  S_StartSound (object, sfx_slop);

  P_SetMobjState (object,info->raisestate);

  object->flags = info->flags;
  object->extendedflags = object->extendedflags;
  object->health = info->spawnhealth;
  object->target = NULL;
  object->invisibility = info->invisibility;
  object->movecount = 0; // -ACB- 1998/08/03 Don't head off in any direction

  mthing = &object->spawnpoint;

  if (mthing->options & MTF_AMBUSH)
    object->flags |= MF_AMBUSH;

  object->reactiontime = 18;
}

//
// P_MobjThinker
//
void P_MobjThinker (mobj_t* mobj)
{
  int tics = 1;
  // -KM- 1998/12/16 Allow for fastparm.
  if (gameflags.fastparm)
  {
    tics = mobj->fasttics;
    mobj->fasttics += mobj->info->fast;
    tics = (mobj->fasttics - tics)>>FRACBITS;
  }
  while (tics--)
  {
    mobj->invisibility = (34*mobj->invisibility + mobj->deltainvis)/35;


    // momentum movement
    if (mobj->momx || mobj->momy || (mobj->flags&MF_SKULLFLY) )
    {
      P_XYMovement (mobj);
  
      // FIXME: decent NOP/NULL/Nil function pointer please.
      if (mobj->thinker.function.acv == (actionf_v) (-1))
        return;		// mobj was removed
    }
  
    if ((mobj->z != mobj->floorz) || mobj->momz)
    {
      P_ZMovement (mobj);
  	
      // FIXME: decent NOP/NULL/Nil function pointer please.
      if (mobj->thinker.function.acv == (actionf_v) (-1))
        return;		// mobj was removed
    }
  
    // cycle through states, calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;
    		
        // you can cycle through multiple states in a tic
        if (!mobj->tics)
        {
          if (!P_SetMobjState (mobj, mobj->state->nextstate) )
            return;         // freed itself
        }
    }
    else
    {
      // check for nightmare respawn
      if (!(mobj->flags & MF_COUNTKILL))
        return;
  
      // replaced respawnmonsters & newnmrespawn with respawnsetting
      // -ACB- 1998/07/30
      if (!gameflags.respawn)
        return;
  
      mobj->movecount++;
  
      //
      // Uses movecount as a timer, when movecount hits 12*TICRATE the
      // object will try to respawn. So after 12 seconds the object will
      // try to respawn.
      //
      if (mobj->movecount < 12*TICRATE)
        return;
  
      // if the first 5 bits of leveltime are on, don't respawn now...ok?
      if (leveltime&31)
        return;
  
      // give a limited "random" chance that respawn don't respawn now
      if (P_Random()>4)
        return;
  
      // replaced respawnmonsters & newnmrespawn with respawnsetting
      // -ACB- 1998/07/30
      if (gameflags.respawnsetting == RS_RESURRECT)
        P_ResurrectRespawn(mobj);
      else if (gameflags.respawnsetting == RS_TELEPORT)
        P_TeleportRespawn(mobj);
    }
  }
}

//
// P_SpawnMobj
//
// TO BE REMOVED - THIS PROCEDURE SHOULD BE UNUSED AND REMOVED WHEN DDF COMPLETE
//
mobj_t* P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
  mobj_t* listcurrmobj;
  mobj_t* mobj;
  state_t* st;
  mobjinfo_t* info;
  extern mobjinfo_t oldmobjinfo[ORIG_NUMMOBJTYPES];
	
  mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
  memset (mobj, 0, sizeof (*mobj));
  info = &oldmobjinfo[type];
	
  mobj->type = info->doomednum;
  mobj->info = info;
  mobj->x = x;
  mobj->y = y;
  mobj->radius = info->radius;
  mobj->height = info->height;
  mobj->flags = info->flags;
  mobj->health = info->spawnhealth;
  mobj->speed = info->speed;
  if (gameflags.fastparm)
    mobj->speed = FixedMul(mobj->speed,info->fast);

  // -ACB- 1998/06/25 new mobj Stuff (1998/07/11 - invisibility added)
  mobj->backpackinfo  = info->backpackinfo;
  mobj->extendedflags = info->extendedflags;
  mobj->deltainvis = mobj->invisibility  = info->invisibility;

  if (gameskill != sk_nightmare)
    mobj->reactiontime = info->reactiontime;
    
  mobj->lastlook = P_Random () % MAXPLAYERS;

  //
  // do not set the state with P_SetMobjState,
  // because action routines can not be called yet
  //
  st = &states[info->spawnstate];

  mobj->state = st;
  mobj->tics = st->tics;
  mobj->fasttics = 0;
  mobj->sprite = st->sprite;
  mobj->frame = st->frame;

  // set subsector and/or block links
  P_SetThingPosition (mobj);
	
  mobj->floorz = mobj->subsector->sector->floorheight;
  mobj->ceilingz = mobj->subsector->sector->ceilingheight;

  if (z == ONFLOORZ)
    mobj->z = mobj->floorz;
  else if (z == ONCEILINGZ)
    mobj->z = mobj->ceilingz - mobj->info->height;
  else
    mobj->z = z;

  // Find the real players height (TELEPORT WEAPONS).
  mobj->origheight=z-realplayerz; //-jc-

  mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
	
  P_AddThinker (&mobj->thinker);

  //
  // -ACB- 1998/08/27 Mobj Linked-List Addition
  //
  // A useful way of cycling through the current things without
  // having to deref everything using thinkers.
  //
  if (mobjlisthead == NULL)
  {
    mobjlisthead = mobj;
    mobj->prev = NULL;
    mobj->next = NULL;
  }
  else
  {
    listcurrmobj = mobjlisthead;

    while (listcurrmobj->next != NULL)
     listcurrmobj = listcurrmobj->next;

    listcurrmobj->next = mobj;
    mobj->prev = listcurrmobj;
    mobj->next = NULL;
  }

  return mobj;
}


//
// P_RemoveMobj
//
// Removes the object from the play simulation: no longer thinks, if
// the mobj is MF_SPECIAL: i.e. item can be picked up, it is added to
// the item-respawn-que, so it gets respawned if needed; The respawning
// only happens if itemrespawn is set or the deathmatch mode is
// version 2.0: altdeath.
//
void P_RemoveMobj (mobj_t* mobj)
{
    iteminque_t *currentitem = NULL;
    iteminque_t *newiteminque = NULL;

    if ((mobj->flags & MF_SPECIAL) &&
         !(mobj->extendedflags & EF_NORESPAWN) && !(mobj->flags & MF_DROPPED))
    {
        // itemquehead is NULL, allocate space for new item-in-respawn-que
        if (itemquehead == NULL)
        {
          itemquehead = Z_Malloc(sizeof(iteminque_t),PU_LEVEL,NULL);
          newiteminque = itemquehead;
          newiteminque->prev = NULL;
          newiteminque->next = NULL;
        }
        else // item-respawn-que exists, add new item to que
        {
          currentitem = itemquehead;

          while (currentitem->next != NULL)
            currentitem = currentitem->next;

          newiteminque = Z_Malloc(sizeof(iteminque_t),PU_LEVEL,NULL);
          currentitem->next = newiteminque;
          newiteminque->prev = currentitem;
          newiteminque->next = NULL;
        }

        newiteminque->info = mobj->spawnpoint;
        newiteminque->time = mobj->info->respawntime;
    }
	
    // unlink from sector and block lists
    P_UnsetThingPosition (mobj);
    
    // stop any playing sound
    S_StopSound (mobj);

    // -------------------------------------------------------------------
    // -ACB- 1998/08/27 Mobj Linked-List Removal
    //
    // A useful way of cycling through the current things without
    // having to deref and search everything using thinkers.
    //
    if (mobj->prev == NULL) // no previous, must be first item
    {
      mobjlisthead = mobj->next;

      if (mobjlisthead != NULL)
        mobjlisthead->prev = NULL;
    }
    else
    {
      mobj->prev->next = mobj->next;

      if (mobj->next != NULL)
        mobj->next->prev = mobj->prev;
    }
    //
    // -------------------------------------------------------------------

    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// -KM- 1998/12/21 Cleaned this up a bit.
void P_SpawnPlayer (mapthing_t* mthing)
{
    player_t*           p;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    mobj_t*             mobj;
    mobjinfo_t*         objtype;

    int                 i;
    int                 consolep; 

    consolep=(mthing->type>4000 && mthing->type<4005) ? mthing->type-3996:mthing->type-1;

    // not playing?
    //-jc-
    if (!playeringame[consolep])
      return;

    p =&players[consolep];

    if (p->playerstate == PST_REBORN)
    {
          G_PlayerReborn (consolep);
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ONFLOORZ;

    // -KM- 1998/11/25 This is in preperation for skins.  The creatures.ddf
    //   will hold player start objects, sprite will be taken for skin.
    objtype = mobjinfohead;

    // -ACB- 1998/08/06 use linked list table
    while ((objtype != NULL) && (mthing->type != objtype->doomednum))
      objtype = objtype->next;

    mobj = P_MobjCreateObject(x,y,z, objtype/*specials[MOBJ_PLAYER]*/);

    // set color translations for player sprites
//    if (mthing->type > 1)
//      mobj->playxtra = consolep;

    mobj->angle = ANG45 * (mthing->angle/45);
    mobj->player = p;
    mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    p->jumpwait = 0;

    // setup gun psprite
    P_SetupPsprites (p);

    // give all cards in death match mode
    if (deathmatch)
        for (i=0 ; i<NUMCARDS ; i++)
            p->cards[i] = true;
 
    if (consolep == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();
    }

    // Heh, make a drone player invisible and no clip
    if (doomcom->drone & (1<<consolep))
    {
      mobj->deltainvis = mobj->invisibility = INVISIBLE;
      mobj->flags |= MF_NOCLIP;
      mobj->flags &= ~(MF_SHOOTABLE|MF_SOLID);
    }
}


//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//
void P_SpawnMapThing (mapthing_t* mthing)
{
    int			bit;
    mobj_t*		mobj;
    mobjinfo_t*         objtype;
    fixed_t		x;
    fixed_t		y;
    fixed_t		z;
		
    // count deathmatch start positions
    if (mthing->type == 11)
    {
	if (deathmatch_p == &deathmatchstarts[max_deathmatch_starts])
        {
          int	length = deathmatch_p - deathmatchstarts;

          deathmatchstarts =
            Z_ReMalloc(deathmatchstarts, sizeof(mapthing_t) * ++max_deathmatch_starts);

          deathmatch_p = deathmatchstarts + length;
        }
	memcpy (deathmatch_p, mthing, sizeof(*mthing));
	deathmatch_p++;
	return;
    }
	
    // check for players specially
    //-jc-
    if ((mthing->type >4000 && mthing->type <4005) ||
        (mthing->type >0 && mthing->type <5))
    {
        // save spots for respawning in network games
       if (mthing->type>4000 && mthing->type<4005)
         playerstarts[(mthing->type-4001)+4] = *mthing;
       else
         playerstarts[mthing->type-1] = *mthing;

       if (!deathmatch)
         P_SpawnPlayer (mthing);

       return;
    }

    // check for apropriate skill level
    if (!netgame && (mthing->options & 16) )
	return;
		
    if (gameskill == sk_baby)
	bit = 1;
    else if (gameskill == sk_nightmare)
	bit = 4;
    else
	bit = 1<<(gameskill-1);

    if (!(mthing->options & bit) )
	return;

    objtype = mobjinfohead;

    // -ACB- 1998/08/06 use linked list table
    while ((objtype != NULL) && (mthing->type != objtype->doomednum))
      objtype = objtype->next;

    // MOBJTYPE not found, don't crash out: JDS Compliance.
    // -ACB- 1998/07/21
    if (objtype==NULL)
    {
#ifdef DEVELOPERS
	Debug_Printf ("Unknown type %i at (%i, %i)\n",
		                      mthing->type, mthing->x, mthing->y);
#endif
        return;
    }
		
    // don't spawn keycards and players in deathmatch
    if (deathmatch && objtype->flags & MF_NOTDMATCH)
      return;

    // don't spawn any monsters if -nomonsters
    // PRE-DDF Hack: 3006 is map number for lost soul
    if (gameflags.nomonsters && (objtype->doomednum==3006 || (objtype->flags & MF_COUNTKILL)))
	return;

    
    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (objtype->flags & MF_SPAWNCEILING)
	z = ONCEILINGZ;
    else
    	z = ONFLOORZ;
    
    // Use MobjCreateObject -ACB- 1998/08/06
    mobj = P_MobjCreateObject(x,y,z,objtype);

    mobj->spawnpoint = *mthing;

    if (mobj->tics > 0)
      mobj->tics = 1 + (P_Random () % mobj->tics);
    if (mobj->flags & MF_COUNTKILL) totalkills++;
    if (mobj->flags & MF_COUNTITEM) totalitems++;
		
    mobj->angle = ANG45 * (mthing->angle/45);

    if (mthing->options & MTF_AMBUSH) mobj->flags |= MF_AMBUSH;
}



//
// GAME SPAWN FUNCTIONS
//


//
// P_SpawnPuff
//
extern fixed_t attackrange;

void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t*	th;
	
    z += ((P_Random()-P_Random())<<10);

    // -ACB- 1998/08/06 Specials table for non-negotiables....
    th = P_MobjCreateObject(x,y,z,specials[MOBJ_PUFF]);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;

    if (th->tics < 1) th->tics = 1;
	
    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
      P_SetMobjState (th, S_PUFF3);
}



//
// P_SpawnBlood
//
// -KM- 1998/11/25 Made more violent. :-)
void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage, angle_t angle)
{
    mobj_t*	th;
    int i;

    angle += ANG180;

    for (i = gameflags.blood ? (P_Random() % 7)+(damage>>2)<7?(damage>>2):7 : 1; i; i--)
    {
      z += ((P_Random()-P_Random())<<10);
  
      th = P_MobjCreateObject(x,y,z, specials[MOBJ_BLOOD]);
  
      th->momz = FRACUNIT*2+FRACUNIT*i/6;
      th->momx = FixedMul(i * 2*FRACUNIT/6, finecosine[angle>>ANGLETOFINESHIFT]);
      th->momy = FixedMul(i * 2*FRACUNIT/6, finesine[angle>>ANGLETOFINESHIFT]);

      th->tics -= P_Random()&3;
  
      if (th->tics < 1) th->tics = 1;
  		
      if (damage <= 12 && damage >= 9)
        P_SetMobjState (th,S_BLOOD2);
      else if (damage < 9)
        P_SetMobjState (th,S_BLOOD3);

      angle += ((P_Random()-P_Random())*0xB60B60);
    }
}



//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
void P_CheckMissileSpawn (mobj_t* th)
{
    th->tics -= P_Random()&3;

    if (th->tics < 1) th->tics = 1;
    
    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y))
	P_ExplodeMissile (th);
}


//
// P_SpawnMissile
//
mobj_t* P_SpawnMissile (mobj_t* source, mobj_t* dest, mobjtype_t type)
{
    mobj_t*	th;
    angle_t	an;
    int		dist;

    th = P_SpawnMobj (source->x, source->y, source->z+4*8*FRACUNIT, type);
    
    if (th->info->seesound)
	S_StartSound (th, th->info->seesound);

    th->source = source;	// where it came from
    an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);	

    // fuzzy player
    if (dest->flags & MF_SHADOW)
	an += (P_Random()-P_Random())<<20;
    else if (lessaccuratemon)
	an += (P_Random()-P_Random())<<19;

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->speed, finecosine[an]);
    th->momy = FixedMul (th->speed, finesine[an]);
	
    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    dist = dist / th->speed;

    if (dist < 1)
	dist = 1;

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn (th);
	
    return th;
}


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
void P_SpawnPlayerMissile (mobj_t* source, mobjtype_t type)
{
    mobj_t*	th;
    angle_t	an;

    fixed_t	x;
    fixed_t	y;
    fixed_t	z;
    fixed_t	slope;

    // see which target is to be aimed at
    an = source->angle;

    slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
      an += 1<<26;
      slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
    }

    if (!linetarget)
    {
      an -= 2<<26;
      slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
    }

    // -ACB- 1998/07/03 If no linetarget, calculate slope by the updown angle
    if (!linetarget)
    {
      an = source->angle;
      slope = ((source->vertangle*5)/4);
    }

    x = source->x;
    y = source->y;
    z = source->z + 4*8*FRACUNIT;

    realplayerz=source->floorz; //-JC-
	
    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
	S_StartSound (th, th->info->seesound);

    th->source = source;
    th->angle = an;
    th->momx = FixedMul(th->speed, finecosine[an>>ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->speed, finesine[an>>ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->speed, slope);

    P_CheckMissileSpawn (th);
}

//
// P_MobjItemRespawn
//
// Replacement procedure for P_RespawnSpecials, uses a linked list to go through
// the item-respawn-que. The time until respawn (in tics) is decremented every tic,
// when the item-in-the-que has a time of zero is it respawned.
//
// -ACB- 1998/07/30 Procedure written.
//
void P_MobjItemRespawn(void)
{
  fixed_t x;
  fixed_t y;
  fixed_t z;
  mobj_t* mo;
  mobjinfo_t* objtype;
  subsector_t* ss;

  iteminque_t *currentitem = NULL;
  iteminque_t *respawneditem = NULL;

  // No item-respawn-que exists, so nothing to process.
  if (itemquehead == NULL)
    return;

  // only respawn items in deathmatch or if itemrespawn
  if ((deathmatch != 2)&&(!gameflags.itemrespawn))
    return;

  currentitem = itemquehead; // lets start from the beginning....

  while(currentitem != NULL)
  {
    currentitem->time--;

    if (!currentitem->time) // no time left? if so respawn object
    {
      x = currentitem->info.x << FRACBITS;
      y = currentitem->info.y << FRACBITS;

      // spawn a teleport fog at the new spot
      ss=R_PointInSubsector(x, y);

      // -ACB- 1998/08/06 Specials table for non-negotiables....
      mo=P_MobjCreateObject(x, y, ss->sector->floorheight, specials[MOBJ_RESPAWNFOG]);

      S_StartSound (mo, sfx_itmbk);

      // -ACB- 1998/08/06 Cycle through the linked mobjinfo list....
      objtype = mobjinfohead;

      while (objtype != NULL && currentitem->info.type != objtype->doomednum)
        objtype = objtype->next;

      if (objtype == NULL)
      {
        I_Error("P_MobjItemRespawn: No such item type!");
        return; // shouldn't happen.
      }

      if (objtype->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
      else
        z = ONFLOORZ;

      // -ACB- 1998/08/06 Use MobjCreateObject
      mo = P_MobjCreateObject(x,y,z,objtype);

      mo->spawnpoint = currentitem->info;
      mo->angle = ANG45*(currentitem->info.angle/45);

      // Taking this item-in-que out of the que, remove
      // any references by the previous and next items to
      // the current one.....
      if (currentitem->prev == NULL) // no previous, must be first item
      {
        itemquehead = currentitem->next;

        if (itemquehead != NULL)
          itemquehead->prev = NULL;
      }
      else
      {
        currentitem->prev->next = currentitem->next;

        if (currentitem->next != NULL)
          currentitem->next->prev = currentitem->prev;
      }

      respawneditem = currentitem;     // ....retain pointer for removal....
      currentitem = currentitem->next; // ....move currentitem on to the next....
      Z_Free(respawneditem);           // ....free memory used by old item-in-que
    }
    else
    {
      currentitem = currentitem->next;
    }
  }
}

//
// P_MobjRemoveMissile
//
// This procedure only is used when a flying missile is removed because
// it "hit" a wall or ceiling that in the simulation acts as a sky. The
// only major differences with P_RemoveMobj are that now item respawn check
// is not done (not needed) and any sound will continue playing despite
// the fact the missile has been removed: This is only done due to the
// fact that a missile in reality would continue flying through a sky and
// you should still be able to hear it.
//
// -ACB- 1998/07/31 Procedure written.
//
void P_MobjRemoveMissile (mobj_t* missile)
{
    // unlink from sector and block lists
    P_UnsetThingPosition (missile);

    // -------------------------------------------------------------------
    // -ACB- 1998/08/27 Mobj Linked-List Removal
    //
    // A useful way of cycling through the current things without
    // having to deref and search everything using thinkers.
    //
    if (missile->prev == NULL) // no previous, must be first item
    {
      mobjlisthead = missile->next;

      if (mobjlisthead != NULL)
        mobjlisthead->prev = NULL;
    }
    else
    {
      missile->prev->next = missile->next;

      if (missile->next != NULL)
        missile->next->prev = missile->prev;
    }
    //
    // -------------------------------------------------------------------

    // free block
    P_RemoveThinker ((thinker_t*)missile);
}

//
// P_MobjCreateObject
//
// Idenitical to P_SpawnMobj, with the exception that the type
// is given by a pointer to an mobjinfo_t, not a type. Type has to
// be removed as the engine should spawn generic types, dictated to
// by the info given. mobj->type should not be specific to a individual
// thing, but a class of things.
//
// -ACB- 1998/08/02 Procedure written.
//
mobj_t* P_MobjCreateObject (fixed_t x, fixed_t y, fixed_t z, mobjinfo_t *type)
{
  mobj_t* listcurrmobj;
  mobj_t* mobj;
  state_t* st;
	
  mobj = Z_Malloc (sizeof(mobj_t), PU_LEVEL, NULL);
  memset (mobj, 0, sizeof(mobj_t));
	
  mobj->info = type;
  mobj->type = type->doomednum;
  mobj->x = x;
  mobj->y = y;
  mobj->radius = type->radius;
  mobj->height = type->height;
  mobj->flags = type->flags;
  mobj->health = type->spawnhealth;
  mobj->speed = type->speed;
  if (gameflags.fastparm)
    mobj->speed = FixedMul(mobj->speed,type->fast);

  // -ACB- 1998/06/25 new mobj Stuff (1998/07/11 - invisibility added)
  mobj->backpackinfo  = type->backpackinfo;
  mobj->extendedflags = type->extendedflags;
  mobj->deltainvis = mobj->invisibility  = type->invisibility;
  mobj->currentattack = NULL;

  if (gameskill != sk_nightmare)
    mobj->reactiontime = type->reactiontime;
    
  mobj->lastlook = P_Random () % MAXPLAYERS;

  //
  // Do not set the state with P_SetMobjState,
  // because action routines can not be called yet
  //
  // if we have a spawnstate use that; else try the meanderstate
  // -ACB- 1998/09/06
  //
  if (type->spawnstate)
    st = &states[type->spawnstate];
  else
    st = &states[type->meanderstate];

  mobj->state = st;
  mobj->tics = st->tics;
  mobj->fasttics = 0;
  mobj->sprite = st->sprite;
  mobj->frame = st->frame;

  // set subsector and/or block links
  P_SetThingPosition (mobj);

  mobj->floorz = mobj->subsector->sector->floorheight;
  mobj->ceilingz = mobj->subsector->sector->ceilingheight;

  if (z == ONFLOORZ)
    mobj->z = mobj->floorz;
  else if (z == ONCEILINGZ)
    mobj->z = mobj->ceilingz - type->height;
  else
    mobj->z = z;

  if (M_CheckParm("-andymagic"))
  {
    mobj->playxtra = P_Random()%6;

    if (mobj->playxtra != 0)
      mobj->playxtra += 16;
  }
  else
  {
    mobj->playxtra = mobj->info->palremap;
  }

  // Find the real players height (TELEPORT WEAPONS).
  mobj->origheight=z-realplayerz; //-jc-

  mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
	
  P_AddThinker (&mobj->thinker);

  //
  // -ACB- 1998/08/27 Mobj Linked-List Addition
  //
  // A useful way of cycling through the current things without
  // having to deref everything using thinkers.
  //
  if (mobjlisthead == NULL)
  {
    mobjlisthead = mobj;
    mobj->prev = NULL;
    mobj->next = NULL;
  }
  else
  {
    listcurrmobj = mobjlisthead;

    while (listcurrmobj->next != NULL)
      listcurrmobj = listcurrmobj->next;

    listcurrmobj->next = mobj;
    mobj->prev = listcurrmobj;
    mobj->next = NULL;
  }

  return mobj;
}



