//  
// DOSDoom Creature Action Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/09/27 Sounds.ddf stuff
//
#include <stdlib.h>
#include "dm_defs.h"
#include "dm_state.h"
#include "g_game.h"
#include "i_system.h"
#include "m_random.h"
#include "lu_sound.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

dirtype_t opposite[] =
{
  DI_WEST,
  DI_SOUTHWEST,
  DI_SOUTH,
  DI_SOUTHEAST,
  DI_EAST,
  DI_NORTHEAST,
  DI_NORTH,
  DI_NORTHWEST,
  DI_NODIR
};

dirtype_t diags[] =
{
  DI_NORTHWEST,
  DI_NORTHEAST,
  DI_SOUTHWEST,
  DI_SOUTHEAST
};

fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};


void A_Fall (mobj_t *actor);
void A_SpawnFly (mobj_t* mo);

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

mobj_t*		soundtarget;

void P_RecursiveSound(sector_t* sec, int soundblocks)
{
    int		i;
    line_t*	check;
    sector_t*	other;
	
    // has the sound flooded this sector
    if (sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
	return;
    
    // wake up all monsters in this sector
    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;
	
    for (i=0 ;i<sec->linecount ; i++)
    {
	check = sec->lines[i];

	if (!(check->flags & ML_TWOSIDED))
	    continue;
	
	P_LineOpening (check);

	if (openrange <= 0)
	    continue;	// closed door
	
	if (sides[check->sidenum[0]].sector == sec)
	    other=sides[check->sidenum[1]].sector;
	else
	    other=sides[check->sidenum[0]].sector;
	
	if (check->flags & ML_SOUNDBLOCK)
	{
	    if (!soundblocks)
	      P_RecursiveSound (other, 1);
	}
	else
        {
	    P_RecursiveSound (other, soundblocks);
        }
    }
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert (mobj_t* target, mobj_t* emmiter)
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}

//
// P_CheckMeleeRange
//
boolean P_CheckMeleeRange (mobj_t* actor)
{
    mobj_t*	pl;
    fixed_t	dist;
	
    if (!actor->target)
	return false;
		
    pl = actor->target;
    dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);

    if (dist >= MELEERANGE-20*FRACUNIT+pl->info->radius)
	return false;
	
    if (! P_CheckSight (actor, actor->target) )
	return false;
							
    return true;		
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
extern  int		maxspecialcross;
extern	line_t**	spechit;
extern	int		numspechit;

boolean P_Move (mobj_t*	actor)
{
  fixed_t tryx;
  fixed_t tryy;
    
  line_t* ld;

  //
  // warning: 'catch', 'throw', and 'try'
  // are all C++ reserved words
  //
  boolean try_ok;
  boolean good = false;
		
  if (actor->movedir == DI_NODIR)
    return false;
		
  if ((unsigned)actor->movedir >= 8)
    I_Error ("Weird actor->movedir!");
		
  tryx = actor->x + FixedMul(actor->speed,xspeed[actor->movedir]);
  tryy = actor->y + FixedMul(actor->speed,yspeed[actor->movedir]);

  try_ok = P_TryMove (actor, tryx, tryy);

  if (!try_ok)
  {
    // open any specials
    if (actor->flags & MF_FLOAT && floatok)
    {
      // must adjust height
      if (actor->z < tmfloorz)
 	actor->z += FLOATSPEED;
      else
		actor->z -= FLOATSPEED;

	    actor->flags |= MF_INFLOAT;
	    return true;
	}
		
	if (!numspechit)
	    return false;
			
	actor->movedir = DI_NODIR;
	good = false;
	while (numspechit--)
	{
	    ld = spechit[numspechit];
            if (P_UseSpecialLine(actor,ld,0))
	      good = true;
	}
	return good;
    }
    else
    {
	actor->flags &= ~MF_INFLOAT;
    }
		
    if (!(actor->flags & MF_FLOAT))
      actor->z = actor->floorz;

    return true; 
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
boolean P_TryWalk (mobj_t* actor)
{	
    if (!P_Move (actor))
    {
	return false;
    }

    actor->movecount = P_Random()&15;
    return true;
}

// -ACB- 1998/09/06 actor is now an object; different movement choices.
void P_NewChaseDir (mobj_t* object)
{

    fixed_t	deltax;
    fixed_t	deltay;
    int		tdir;
    
    dirtype_t	d[3];
    dirtype_t	olddir;
    dirtype_t	turnaround;

    olddir = object->movedir;
    turnaround = opposite[olddir];

    //
    // Movement choice: Previously this was calculation to find
    // the distance between object and target: if the object had
    // no target, a fatal error was returned. However it is now
    // possible to have movement without a target. if the object
    // has a target, go for that; else if it has a supporting
    // object aim to go within supporting distance of that; the
    // remaining option is to walk aimlessly: the target destination
    // is always 128*FRACUNIT in the old movement direction, think
    // of it like the donkey and the carrot sketch: the donkey will
    // move towards the carrot, but since the carrot is always a
    // set distance away from the donkey, the rather stupid mammal
    // will spend eternity trying to get the carrot and will walk
    // forever.
    //
    // -ACB- 1998/09/06
    //
    if (object->target)
    {
      deltax = object->target->x - object->x;
      deltay = object->target->y - object->y;
    }
    else if (object->supportobj)
    {
      // not too close
      deltax = (object->supportobj->x - object->x) - (object->supportobj->radius<<2);
      deltay = (object->supportobj->y - object->y) - (object->supportobj->radius<<2);
    }
    else
    {
      deltax = (128*FRACUNIT)*xspeed[olddir];
      deltay = (128*FRACUNIT)*yspeed[olddir];
    }

    if (deltax>10*FRACUNIT)
        d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
	d[1]= DI_WEST;
    else
	d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
	d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
	d[2]= DI_NORTH;
    else
	d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR && d[2] != DI_NODIR)
    {
	object->movedir = diags[((deltay<0)<<1)+(deltax>0)];
	if (object->movedir != turnaround && P_TryWalk(object))
	    return;
    }

    // try other directions
    if (P_Random() > 200 ||  abs(deltay)>abs(deltax))
    {
	tdir=d[1];
	d[1]=d[2];
	d[2]=tdir;
    }

    if (d[1]==turnaround)
      d[1]=DI_NODIR;

    if (d[2]==turnaround)
      d[2]=DI_NODIR;
	
    if (d[1]!=DI_NODIR)
    {
	object->movedir = d[1];
	if (P_TryWalk(object))
	{
	    // either moved forward or attacked
	    return;
	}
    }

    if (d[2]!=DI_NODIR)
    {
	object->movedir =d[2];

	if (P_TryWalk(object))
	    return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
	object->movedir =olddir;

	if (P_TryWalk(object))
	    return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
	for ( tdir=DI_EAST;
	      tdir<=DI_SOUTHEAST;
	      tdir++ )
	{
	    if (tdir!=turnaround)
	    {
		object->movedir =tdir;
		
		if ( P_TryWalk(object) )
		    return;
	    }
	}
    }
    else
    {
	for ( tdir=DI_SOUTHEAST;
	      tdir != (DI_EAST-1);
	      tdir-- )
	{
	    if (tdir!=turnaround)
	    {
		object->movedir =tdir;
		
		if ( P_TryWalk(object) )
		    return;
	    }
	}
    }

    if (turnaround !=  DI_NODIR)
    {
	object->movedir =turnaround;
	if ( P_TryWalk(object) )
	    return;
    }

    object->movedir = DI_NODIR; // can not move  */
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
boolean P_LookForPlayers (mobj_t* actor, boolean allaround)
{
    int		c;
    int		stop;
    player_t*	player;
    sector_t*	sector;
    angle_t	an;
    fixed_t	dist;
		
    sector = actor->subsector->sector;
	
    c = 0;
    stop = (actor->lastlook-1)&3;
	
    for ( ; ; actor->lastlook = (actor->lastlook+1)&3 )
    {
	if (!playeringame[actor->lastlook])
	    continue;
			
	if (c++ == 2
	    || actor->lastlook == stop)
	{
	    // done looking
	    return false;	
	}
	
	player = &players[actor->lastlook];

	if (player->health <= 0)
	    continue;		// dead

	if (!P_CheckSight (actor, player->mo))
	    continue;		// out of sight
			
	if (!allaround)
	{
	    an = R_PointToAngle2 (actor->x,
				  actor->y, 
				  player->mo->x,
				  player->mo->y)
		- actor->angle;
	    
	    if (an > ANG90 && an < ANG270)
	    {
		dist = P_AproxDistance (player->mo->x - actor->x,
					player->mo->y - actor->y);
		// if real close, react anyway
		if (dist > MELEERANGE)
		    continue;	// behind back
	    }
	}
		
	actor->target = player->mo;
	return true;
    }

    return false;
}

void A_Scream (mobj_t* actor)
{
    sfx_t*		sound;
	
    sound = actor->info->deathsound;

    // Check for bosses. Bosses make a lot of noise.
    // -ACB- 1998/06/14 - Extended flag check used
    if (actor->info->extendedflags & EF_BOSSMAN)
        S_StartSound (NULL, sound);
    else
	S_StartSound (actor, sound);

}


void A_XScream (mobj_t* actor)
{
    S_StartSound (actor, sfx_slop);
}

void A_Pain (mobj_t* actor)
{
    if (actor->info->painsound)
	S_StartSound (actor, actor->info->painsound);
}

void A_Fall (mobj_t *actor)
{
    actor->invisibility = VISIBLE; // dead and very visible

    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

    // So change this if corpse objects
    // are meant to be obstacles.
}


//
// A_Explode
//
void A_Explode (mobj_t* thingy)
{
    P_RadiusAttack ( thingy, thingy->target, 128 );
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
// This is big hack that will have to go for DDF
//
void A_BossDeath (mobj_t* deadboss)
{
    //thinker_t*	th;
    //mobj_t*	other;
    //line_t	fakeline;
    //int		i;

    switch (deadboss->info->doomednum)
    {
      case 7:    // Spider-demon
      case 16:   // Cyberdemon
      case 67:   // Mancubus
      case 68:   // Arachnotron
      case 3003: // Baron-of-hell
      {
        break;
      }
      default:
      {
        I_Error("Thing: %d\n",deadboss->info->doomednum);
        return;
      }
    }

    /*

    if (gamemission != doom)
    {
	if (gamemap != 7)
	    return;
		
	if ((mo->info->doomednum != 67) // Hack for Manucubus
	    && (mo->info->doomednum != 68)) // Hack for Arachnotron
	    return;
    }

    if (gamemission == doom)
    {
	switch(gameepisode)
	{
	  case 1:
	    if (gamemap != 8)
		return;

	    if (mo->info->doomednum != 3003) // Hack for Baron-of-Hell
		return;
	    break;
	    
	  case 2:
	    if (gamemap != 8)
		return;

	    if (mo->info->doomednum != 16) // Hack for Cyberdemon
		return;
	    break;
	    
	  case 3:
	    if (gamemap != 8)
		return;
	    
	    if (mo->info->doomednum != 7) // Hack for Spider
		return;
	    
	    break;
	    
	  case 4:
	    switch(gamemap)
	    {
	      case 6:
		if (mo->info->doomednum != 16) // Hack for Cyberdemon
		    return;
		break;
		
	      case 8: 
		if (mo->info->doomednum != 7) // Hack for Spider
		    return;
		break;
		
	      default:
		return;
		break;
	    }
	    break;
	    
	  default:
	    if (gamemap != 8)
		return;
	    break;
	}
		
    }

    
    // make sure there is a player alive for victory
    for (i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i] && players[i].health > 0)
	    break;
    
    if (i==MAXPLAYERS)
	return;	// no one left alive, so do not end game
    
    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
	if (th->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;
	
	mo2 = (mobj_t *)th;

	if (mo2 != mo && mo2->info->doomednum == mo->info->doomednum && mo2->health > 0)
	{
	    // other boss not dead
	    return;
	}
    }
	
    // victory!
    if (gamemission != doom)
    {
	if (gamemap == 7)
	{
	    if (mo->info->doomednum == 67) // Hack for Manucubus
	    {
		junk.tag = 666;
		EV_DoFloor(&junk,lowerFloorToLowest);
		return;
	    }
	    
	    if (mo->info->doomednum == 68) // Arachnotron
	    {
		junk.tag = 667;
		EV_DoFloor(&junk,raiseToTexture);
		return;
	    }
	}
    }
    else
    {
	switch(gameepisode)
	{
	  case 1:
	    junk.tag = 666;
	    EV_DoFloor (&junk, lowerFloorToLowest);
	    return;
	    break;
	    
	  case 4:
	    switch(gamemap)
	    {
	      case 6:
		junk.tag = 666;
		EV_DoDoor (&junk, blazeOpen);
		return;
		break;
		
	      case 8:
		junk.tag = 666;
		EV_DoFloor (&junk, lowerFloorToLowest);
		return;
		break;
	    }
	}
    }
	
    G_ExitLevel (); */
}

static int numbraintargets;
static int braintargeton;
static mobj_t **braintargets;


//
// P_EnemyBrainAwake
//
// Modified A_BrainAwake Routine, uses mobj_t linked list and no
// longer uses the limit of 32 boss targets.
//
// -ACB- 1998/08/30
//
void /*P_EnemyBrainAwake*/ A_BrainAwake(mobj_t* bossbrain)
{
  mobj_t* currmobj;
  int i = 0;

  // find all the target spots
  numbraintargets = 0;
  braintargeton = 0;

  // lets count the number of brain targets
  currmobj = mobjlisthead;
  while (currmobj != NULL)
  {
    if (currmobj->info == specials[MOBJ_SPAWNSPOT])
      numbraintargets++;

    currmobj = currmobj->next;
  }

  braintargets = Z_Malloc(sizeof(mobj_t*) * numbraintargets, PU_LEVEL, 0);

  currmobj = mobjlisthead;
  while (currmobj != NULL)
  {
    if (currmobj->info == specials[MOBJ_SPAWNSPOT])
    {
      braintargets[i] = currmobj;
      i++;
    }

    currmobj = currmobj->next;
  }

  S_StartSound (NULL,sfx_bossit);
}

//
// P_EnemyBrainPain: The brain and his pain...
//
void /*P_EnemyBrainPain*/A_BrainPain(mobj_t* bossbrain)
{
  S_StartSound (NULL,sfx_bospn);
}

//
// P_EnemyBrainExplode: The brain and his death...
//
void /*P_EnemyBrainExplode*/A_BrainScream(mobj_t* bossbrain)
{
  int x;
  int y;
  int z;
  mobj_t* th;
	
  for (x=(bossbrain->x)-(196*FRACUNIT); x<(bossbrain->x+320*FRACUNIT); x+=FRACUNIT*8)
  {
    y = bossbrain->y - 320*FRACUNIT;
    z = 128 + P_Random()*2*FRACUNIT;
    th = P_SpawnMobj (x,y,z, MT_ROCKET);
    th->momz = P_Random()*512;

    P_SetMobjState (th, S_BRAINEXPLODE1);

    th->tics -= P_Random()&7;

    if (th->tics < 1)
      th->tics = 1;
  }
	
  S_StartSound (NULL,sfx_bosdth);
}

void A_BrainExplode (mobj_t* mo)
{
    int		x;
    int		y;
    int		z;
    mobj_t*	th;
	
    x = mo->x + (P_Random()-P_Random())*2048;
    y = mo->y;
    z = 128 + P_Random()*2*FRACUNIT;
    th = P_SpawnMobj (x,y,z, MT_ROCKET);
    th->momz = P_Random()*512;

    P_SetMobjState (th, S_BRAINEXPLODE1);

    th->tics -= P_Random()&7;

    if (th->tics < 1)
      th->tics = 1;
}


void A_BrainDie (mobj_t*  mo)
{
    G_ExitLevel (35);
}

void A_BrainSpit (mobj_t* object)
{
  mobj_t* targetobj;
  mobj_t* newmobj;
    
  static int easy = 0;

  easy ^= 1;

  if (gameskill <= sk_easy && (!easy))
    return;
		
  // shoot a cube at current target
  targetobj = braintargets[braintargeton];
  braintargeton = (braintargeton+1)%numbraintargets;

  // spawn brain missile
  newmobj = P_SpawnMissile (object, targetobj, MT_SPAWNSHOT);
  newmobj->reactiontime = ((targetobj->y - object->y)/newmobj->momy)
                                / newmobj->state->tics;

  newmobj->target = targetobj;

  S_StartSound(NULL, sfx_bospit);
}

// -ACB- 1998/08/30 fixed quickly for boss to work....
void A_SpawnFly (mobj_t* mo)
{
    mobj_t*	newmobj;
    mobj_t*	fog;
    mobj_t*	targ;
    int		r;
    mobjinfo_t* type;
    mobjtype_t fire = MT_SPAWNFIRE;

    if (--mo->reactiontime)
	return;	// still flying
	
    targ = mo->target;

    // First spawn teleport fog.
    fog = P_SpawnMobj (targ->x, targ->y, targ->z, fire);
    S_StartSound (fog, sfx_telept);

    // Randomly select monster to spawn.
    r = P_Random ();

    // Probability distribution (kind of :)),
    // decreasing likelihood.
    if (r<50)
	type = DDF_MobjLookup("IMP");
    else if (r<90)
	type = DDF_MobjLookup("DEMON");
    else if (r<120)
	type = DDF_MobjLookup("SPECTRE");
    else if (r<130)
	type = DDF_MobjLookup("PAIN ELEMENTAL");
    else if (r<160)
	type = DDF_MobjLookup("CACODEMON");
    else if (r<162)
	type = DDF_MobjLookup("ARCHVILE");
    else if (r<172)
	type = DDF_MobjLookup("REVENANT");
    else if (r<192)
	type = DDF_MobjLookup("ARACHNOTRON");
    else if (r<222)
	type = DDF_MobjLookup("MANCUBUS");
    else if (r<246)
	type = DDF_MobjLookup("HELL KNIGHT");
    else
	type = DDF_MobjLookup("BARON OF HELL");

    newmobj = P_MobjCreateObject(targ->x, targ->y, targ->z, type);

    if (P_LookForPlayers (newmobj, true))
      P_SetMobjState (newmobj, newmobj->info->seestate);
	
    // telefrag anything in this spot
    P_TeleportMove (newmobj, newmobj->x, newmobj->y);

    // remove self (i.e., cube).
    P_RemoveMobj (mo);
}

// travelling cube sound
void A_SpawnSound (mobj_t* mo)	
{
    S_StartSound (mo,sfx_boscub);
    A_SpawnFly(mo);
}



/* void A_BrainAwake (mobj_t* mo)
{
    thinker_t*	thinker;
    mobj_t*	m;
	
    // find all the target spots
    numbraintargets = 0;
    braintargeton = 0;
	
    thinker = thinkercap.next;
    for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
    {
	if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;	// not a mobj

	m = (mobj_t *)thinker;

	if (m->type == MT_BOSSTARGET)
	{
	    braintargets[numbraintargets] = m;
	    numbraintargets++;
	}
    }
	
    S_StartSound (NULL,sfx_bossit);
}


void A_BrainPain (mobj_t*	mo)
{
    S_StartSound (NULL,sfx_bospn);
}


void A_BrainScream (mobj_t*	mo)
{
    int		x;
    int		y;
    int		z;
    mobj_t*	th;
	
    for (x=mo->x - 196*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
    {
	y = mo->y - 320*FRACUNIT;
	z = 128 + P_Random()*2*FRACUNIT;
	th = P_SpawnMobj (x,y,z, MT_ROCKET);
	th->momz = P_Random()*512;

	P_SetMobjState (th, S_BRAINEXPLODE1);

	th->tics -= P_Random()&7;
	if (th->tics < 1)
	    th->tics = 1;
    }
	
    S_StartSound (NULL,sfx_bosdth);
}



void A_BrainExplode (mobj_t* mo)
{
    int		x;
    int		y;
    int		z;
    mobj_t*	th;
	
    x = mo->x + (P_Random()-P_Random())*2048;
    y = mo->y;
    z = 128 + P_Random()*2*FRACUNIT;
    th = P_SpawnMobj (x,y,z, MT_ROCKET);
    th->momz = P_Random()*512;

    P_SetMobjState (th, S_BRAINEXPLODE1);

    th->tics -= P_Random()&7;
    if (th->tics < 1)
	th->tics = 1;
}


void A_BrainDie (mobj_t*  mo)
{
    G_ExitLevel ();
}

void A_BrainSpit (mobj_t* mo)
{
    mobj_t*	targ;
    mobj_t*	newmobj;
    
    static int	easy = 0;
	
    easy ^= 1;
    if (gameskill <= sk_easy && (!easy))
	return;
		
    // shoot a cube at current target
    targ = braintargets[braintargeton];
    braintargeton = (braintargeton+1)%numbraintargets;

    // spawn brain missile
    newmobj = P_SpawnMissile (mo, targ, MT_SPAWNSHOT);
    newmobj->target = targ;
    newmobj->reactiontime =
	((targ->y - mo->y)/newmobj->momy) / newmobj->state->tics;

    S_StartSound(NULL, sfx_bospit);
}

// travelling cube sound
void A_SpawnSound (mobj_t* mo)	
{
    S_StartSound (mo,sfx_boscub);
    A_SpawnFly(mo);
}

void A_SpawnFly (mobj_t* mo)
{
    mobj_t*	newmobj;
    mobj_t*	fog;
    mobj_t*	targ;
    int		r;
    mobjtype_t	type = MT_SPAWNFIRE;
	
    if (--mo->reactiontime)
	return;	// still flying
	
    targ = mo->target;

    // First spawn teleport fog.
    fog = P_SpawnMobj (targ->x, targ->y, targ->z, MT_SPAWNFIRE);
    S_StartSound (fog, sfx_telept);

    // Randomly select monster to spawn.
    r = P_Random ();

    // Probability distribution (kind of :)),
    // decreasing likelihood.
    if ( r<50 )
	type = MT_TROOP;
    else if (r<90)
	type = MT_SERGEANT;
    else if (r<120)
	type = MT_SHADOWS;
    else if (r<130)
	type = MT_PAIN;
    else if (r<160)
	type = MT_HEAD;
    else if (r<162)
	type = MT_VILE;
    else if (r<172)
	type = MT_UNDEAD;
    else if (r<192)
	type = MT_BABY;
    else if (r<222)
	type = MT_FATSO;
    else if (r<246)
	type = MT_KNIGHT;
    else
	type = MT_BRUISER;

    newmobj	= P_SpawnMobj (targ->x, targ->y, targ->z, type);
    if (P_LookForPlayers (newmobj, true) )
	P_SetMobjState (newmobj, newmobj->info->seestate);
	
    // telefrag anything in this spot
    P_TeleportMove (newmobj, newmobj->x, newmobj->y);

    // remove self (i.e., cube).
    P_RemoveMobj (mo);
} */

void A_PlayerScream (mobj_t* mo)
{
    // Default death sound.
    sfx_t*		sound = sfx_pldeth;
	
    if ( (mo->health < -50) && (W_CheckNumForName("DSPDIEHI") < 0))
    {
	// IF THE PLAYER DIES Note that mo->health can never go below zero ?? - Kester
	// LESS THAN -50% WITHOUT GIBBING
	sound = sfx_pdiehi;
    }
    
    S_StartSound (mo, sound);
}

