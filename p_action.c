//
// DOSDoom Play Simulation Action routines
//
// By The DOSDoom Team
//
// Notes:
//  All Procedures here are never called directly, except possibly
//  by another P_Act* Routine. Otherwise the procedure is called
//  by referencing an code pointer from the states[] table. The only
//  exception to these rules are P_ActMissileContact and
//  P_ActSlammedIntoObject that requiring "acting" on the part
//  of an obj.
//
// This file was created for all action code by DDF.
//
// -KM- 1998/09/27 Added sounds.ddf capability
// -KM- 1998/11/25 Visibility is now a fixed_t.
// -KM- 1998/12/21 New smooth visibility.
//

#include <stdlib.h>
#include <math.h>

#include "d_debug.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "g_game.h"
#include "i_system.h"
#include "m_random.h"
#include "lu_sound.h"
#include "p_local.h"
#include "p_pspr.h"
#include "r_state.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#define TRACEANGLE 0xc000000

void P_ActAttack(mobj_t *object);
void P_ActRangeAttack(mobj_t *object);
void P_ActMeleeAttack(mobj_t *object);

//-----------------------------------------
//--------------MISCELLANOUS---------------
//-----------------------------------------

//
// P_ActLookForTargets
//
// Looks for targets: used in the same way as enemy things look
// for players
//
// TODO: Write a decent procedure.
//
boolean P_ActLookForTargets(mobj_t* object)
{
  mobj_t* currmobj;

  currmobj = mobjlisthead;

  while (currmobj != NULL)
  {
    if ((currmobj != object) &&
          (currmobj != object->supportobj) &&
            (object->info != currmobj->info) &&
              (object->supportobj != currmobj->supportobj))
    {
      if ((currmobj->flags & MF_SHOOTABLE) && P_CheckSight(object, currmobj))
      {
        object->target = currmobj;
        P_SetMobjState(object, object->info->seestate);
        return true;
      }
    }
    currmobj = currmobj->next;
  }

  return false;
}

//
// P_ActDecideMeleeAttack
//
// This is based on P_CheckMeleeRange, except that it relys upon
// info from the objects close combat attack, the original code
// used a set value for all objects which was MELEERANGE+(20*FRACUNIT),
// this code allows different melee ranges for different objects.
//
// -ACB- 1998/08/15
// -KM- 1998/11/25 Added attack parameter.
//
boolean P_ActDecideMeleeAttack(mobj_t* object, attacktype_t *attack)
{
  mobj_t* target;
  fixed_t distance;
  fixed_t meleedist;

  target = object->target;

  if (!target)
    return false;

  if (!P_CheckSight (object, target))
    return false;

  if (!attack)
    return false; // cannot evaluate range with no attack range

  distance = P_AproxDistance (target->x - object->x, target->y - object->y);
  if (gameflags.true3dgameplay)
    distance = P_AproxDistance (target->z - object->z, distance);
  // could be *FRACUNIT as opposed to << 16
  meleedist = (attack->range)-20*FRACUNIT+target->info->radius;

  if (distance >= meleedist)
    return false;
        
  return true;
}


//
// P_ActDecideRangeAttack
//
// This is based on P_CheckMissileRange, contrary the name it does more
// than check the missile range, it makes a decision of whether or not an
// attack should be made or not depending on the object with the option
// to attack. A return of false is mandatory if the object cannot see its
// target (makes sense, doesn't it?), after this the distance is calculated,
// it will eventually be check to see if it is greater than a number from
// the Random Number Generator; if so the procedure returns true. Essentially
// the closer the object is to its target, the more chance an attack will
// be made (another logical decision).
//
// -ACB- 1998/08/15
//
boolean P_ActDecideRangeAttack (mobj_t* object)
{
  int chance;
  fixed_t distance;
  attacktype_t* attack;
        
  if (!P_CheckSight(object, object->target))
    return false;

  if (object->info->rangeattack)
    attack = object->info->rangeattack;
  else
    return false; // cannot evaluate range with no attack range

  // Just been hit (and have felt pain), so in true tit-for-tat
  // style, the object - without regard to anything else - hits back.
  if (object->flags & MF_JUSTHIT)
  {
    object->flags &= ~MF_JUSTHIT;
    return true;
  }

  // Bit slow on the up-take: the object hasn't had time to
  // react his target.
  if (object->reactiontime)
    return false;

  // Get the distance, a basis for our decision making from now on
  distance = P_AproxDistance(object->x - object->target->x,
                               object->y - object->target->y);

  // No need to multiply all our checks by FRACUNIT
  distance >>= FRACBITS;

  // If no close-combat attack, increase the chance of a missile attack
  if (!object->info->meleestate)
    distance -= 192;
  else
    distance -= 64;

  // Object is too far away to attack?
  if (attack->range && distance >= (attack->range>>FRACBITS))
    return false;

  // Object is too close to target
  if (attack->tooclose && attack->tooclose >= distance)
    return false;

  // Object likes to fire? if so, double the chance of it happening
  if (object->extendedflags & EF_TRIGGERHAPPY)
    distance >>= 1;

  // The chance in the object is one given that the attack will happen, so
  // we inverse the result (since its one in 255) to get the chance that
  // the attack will not happen.
  chance = 255 - object->info->minatkchance;

  // if chance is 255, the minimum attack chance is zero, so we do not have to
  // modify the distance count.
  if (chance != 255)
  {
    if (distance > chance)
      distance = chance;
  }

  // now after modifing distance where applicable, we get the random number and
  // check if it is less than distance, if so no attack is made.
  if (P_Random () < distance)
    return false;

  return true;
}

//
// P_ActFaceTarget
//
// Look at the prey......
//
void P_ActFaceTarget(mobj_t *object)
{
  mobj_t* target;

  target = object->target;

  if (!target)
    return;

  if (object->flags & MF_STEALTH)
      object->deltainvis = VISIBLE;

  object->flags &= ~MF_AMBUSH;
        
  object->angle = R_PointToAngle2 (object->x, object->y, target->x, target->y);
    
  if (target->flags & MF_SHADOW)
    object->angle+=(P_Random()-P_Random())<<21;

  if (target->invisibility < VISIBLE)
    object->angle += ((P_Random()-P_Random())*((VISIBLE-target->invisibility)>>10));
}

//
// P_ActMakeIntoCorpse
//
// Gives the effect of the object being a corpse....
//
void P_ActMakeIntoCorpse(mobj_t* object)
{
  if (object->flags & MF_STEALTH)
    object->deltainvis = VISIBLE; // dead and very visible

  // object is on ground, it can be walked over
  object->flags &= ~MF_SOLID;
}

//
// P_ActResetSpreadCount
//
// Resets the spreader count for fixed-order spreaders, normally used at the
// beginning of a set of missile states to ensure that an object fires in
// the same object each time.
//
void P_ActResetSpreadCount(mobj_t* object)
{
  object->spreadcount = 0;
}

//
// P_ActSelectTarget
//
// Search the things list for a target
//
mobj_t* P_ActSelectTarget()
{
  mobj_t* currmobj = NULL;
  mobj_t* startmobj = NULL;
  mobj_t* target = NULL;
  static int count = 0;
  int i;

  currmobj = mobjlisthead;
  count = count + P_Random();

  if (count > 500)
    count = 0;

  for (i=0; i<=count; i++)
  {
    currmobj = currmobj->next;

    // found the end, start from the beginning
    if (currmobj->next == NULL)
      currmobj = mobjlisthead;
  }

  // store the starting point, if we encounter it again, we
  // have been through the list.
  startmobj = currmobj;

  while (target != NULL && startmobj == NULL)
  {
    if ((currmobj->flags & MF_COUNTKILL) && (currmobj->health > 0))
    {
      target = currmobj;
    }
    else
    {
      currmobj = currmobj->next;

      if (currmobj==startmobj)
        startmobj = NULL;

      // found the end, start from the beginning
      if (currmobj->next==NULL)
        currmobj = mobjlisthead;
    }
  }

  // cycled through the entire list with no possible targets
  if (startmobj == NULL)
    return NULL;

  return target;
}

//
// P_ActCreateAggression
//
// Sets an object up to target a previously stored object.
//
boolean P_ActCreateAggression (mobj_t *object)
{
  static mapstuff_t* mapcheck = NULL;
  static mobj_t* target = NULL;
  static int count = 0;
  mobjinfo_t* targinfo;
  mobjinfo_t* objinfo;

  count++;

  if (mapcheck == NULL || mapcheck != currentmap)
  {
    mapcheck=currentmap;
    target = P_ActSelectTarget();
    count=0;
    return false;
  }

  if (target==NULL || target->health <= 0)
  {
    if (!(object->extendedflags & EF_NEVERTARGET))
      target = object;
    else
      target = P_ActSelectTarget();

    count=0;
    return false;
  }

  if (object == target)
    return false;

  // This object has been checked too many times, try a another one.
  if (count > 127)
  {
    target = P_ActSelectTarget();
    count=0;
    return false;
  }

  objinfo = object->info;
  targinfo = target->info;

  if (!P_CheckSight(object, target))
    return false;

  if (objinfo->rangeattack->attackstyle != ATK_SHOT)
  {
    if ((targinfo == objinfo) && (!(objinfo->extendedflags & EF_DISLOYALTYPE)))
      return false;

    if (!(objinfo->extendedflags & EF_OWNATTACKHURTS))
    {
      if (targinfo == objinfo)
        return false;

      if ((objinfo->rangeattack == targinfo->rangeattack) &&
             (objinfo->closecombat == targinfo->closecombat))
      {
        return false;
      }

      if ((objinfo->rangeattack == targinfo->closecombat) &&
              (objinfo->closecombat == targinfo->rangeattack))
      {
        return false;
      }
    }
  }

  object->target = target;
  P_SetMobjState (object, object->info->seestate);
  return true;
}


//-------------------------------------------------------------------
//-------------------VISIBILITY HANDLING ROUTINES--------------------
//-------------------------------------------------------------------

//
// P_ActAlterTransluc
//
// Alters the translucency of an item, EF_LESSVIS is used
// internally to tell the object if it should be getting
// more visible or less visible; EF_LESSVIS is set when an
// object is to get less visible (because it has become
// to a level of lowest translucency) and the flag is unset
// if the object has become as highly translucent as possible.
//
void P_ActAlterTransluc (mobj_t *object)
{
  state_t* st;
  st = object->state;

  if (object->extendedflags & EF_LESSVIS)
  {
    object->deltainvis -= st->misc1;
    if (object->deltainvis <= INVISIBLE)
    {
      object->deltainvis = INVISIBLE;
      object->extendedflags &= ~EF_LESSVIS;
    }
  }
  else
  {
    object->deltainvis += st->misc1;
    if (object->deltainvis >= VISIBLE)
    {
      object->deltainvis = VISIBLE;
      object->extendedflags |= EF_LESSVIS;
    }
  }
}

//
// P_ActAlterVisibility
//
// Alters the visibilility of an item, EF_LESSVIS is used
// internally to tell the object if it should be getting
// more visible or less visible; EF_LESSVIS is set when an
// object is to get less visible (because it has become
// fully visible) and the flag is unset if the object has
// become invisible.
//
void P_ActAlterVisibility (mobj_t *object)
{
  state_t* st;
  st = object->state;
  object->deltainvis = st->misc1;
}

//
// P_ActBecomeLessVisible
//
void P_ActBecomeLessVisible (mobj_t *object)
{
  state_t* st;
  st = object->state;
  object->deltainvis -= st->misc1;
  if (object->deltainvis < INVISIBLE)
    object->deltainvis = INVISIBLE;
}

//
// P_ActBecomeMoreVisible
//
void P_ActBecomeMoreVisible (mobj_t *object)
{
  state_t* st;
  st = object->state;
  object->deltainvis += st->misc1;
  if (object->deltainvis > VISIBLE)
    object->deltainvis = VISIBLE;
}

//-------------------------------------------------------------------
//-------------------SOUND CAUSING ROUTINES--------------------------
//-------------------------------------------------------------------

//
// P_ActMakeAmbientSound
//
// Just a sound generating procedure that cause the sound ref
// in seesound to be generated.
//
void P_ActMakeAmbientSound(mobj_t *object)
{
  if (object->info->seesound)
    S_StartSound(object,object->info->seesound);

#ifdef DEVELOPERS
  else
    Debug_Printf("%s has no ambient sound\n",object->info->name);
#endif
}

//
// P_ActMakeAmbientSoundRandom
//
// Give a small "random" chance that this object will make its
// ambient sound. Currently this is a set value of 50, however
// the code that drives this, should allow for the user to set
// the value, note for further DDF Development.
//
void P_ActMakeAmbientSoundRandom(mobj_t *object)
{
  if (object->info->seesound)
  {
    if (J_Random() < 50)
        S_StartSound(object,object->info->seesound);
  }
#ifdef DEVELOPERS
  else
  {
    Debug_Printf("%s has no ambient sound\n",object->info->name);
    return;
  }
#endif

}

//
// P_ActMakeDyingSound
//
// This procedure is like everyother sound generating
// procedure with the exception that if the object is
// a boss (EF_BOSSMAN extended flag) then the sound is
// generated at full volume (source = NULL).
//
void P_ActMakeDyingSound (mobj_t* object)
{
  sfx_t* sound;

  sound = object->info->deathsound;

  if (sound)
  {
    if (object->info->extendedflags & EF_BOSSMAN)
      S_StartSound (NULL, sound);
    else
      S_StartSound (object, sound);
  }
#ifdef DEVELOPERS
  else
  {
    Debug_Printf("%s has no death sound\n",object->info->name);
    return;
  }
#endif
}

//
// P_ActMakePainSound (Ow!! it hurts!)
//
void P_ActMakePainSound (mobj_t* object)
{
  if (object->info->painsound)
  {
    S_StartSound (object, object->info->painsound);
  }
#ifdef DEVELOPERS
  else
  {
    Debug_Printf("%s has no pain sound\n",object->info->name);
  }
#endif
}

//
// P_ActMakeOverKillSound
//
// Slop, needs to be made designer-selectable?
//
void P_ActMakeOverKillSound(mobj_t *object)
{
  S_StartSound (object, sfx_slop);
}

//
// P_ActMakeCloseAttemptSound
//
// Attempting close combat sound
//
void P_ActMakeCloseAttemptSound(mobj_t *object)
{
  sfx_t* sound;

  sound = object->info->closecombat->initsound;

  if (sound)
  {
    S_StartSound (object, sound);
  }
#ifdef DEVELOPERS
  else
  {
    Debug_Printf("%s has no close combat attempt sound\n",object->info->name);
  }
#endif

}

//
// P_ActMakeRangeAttemptSound
//
// Attempting attack at range sound
//
void P_ActMakeRangeAttemptSound(mobj_t *object)
{
  sfx_t* sound;

  sound = object->info->rangeattack->initsound;

  if (sound)
    S_StartSound (object, sound);
#ifdef DEVELOPERS
  else
    Debug_Printf("%s has no range attack attempt sound\n",object->info->name);
#endif
}

//-------------------------------------------------------------------
//-------------------EXPLOSION DAMAGE ROUTINES-----------------------
//-------------------------------------------------------------------

//
// P_ActSetDamageExplosion
//
// Radius Attack damage set by info->damage. Used for the original Barrels
//
void P_ActSetDamageExplosion(mobj_t* object)
{
#ifdef DEVELOPERS
  if (!object->info->damage)
    Debug_Printf("%s will always cause no damage\n",object->info->name);
#endif

  P_RadiusAttack (object, object->source, object->info->damage);
}

//
// P_ActVaryingDamageExplosion
//
// Same as above, but the damage is calculated by:
//         "Random" Number between 1 and damagerange, then
//         Multiply by damagemulti.
//
// For Example: if damagerange = 16 & damagemulti was 8
//     would give a damage of between 8 and 128.
//
// -ACB- 1998/08/04
//
void P_ActVaryingDamageExplosion (mobj_t* object)
{
  int damage;
  damage = (P_Random() % object->info->damagerange) * object->info->damagemulti;

#ifdef DEVELOPERS
  if (!damage)
    Debug_Printf("%s caused a varied explosion with no damage\n",object->info->name);
#endif

  P_RadiusAttack (object, object->source, damage);
}

//-------------------------------------------------------------------
//-------------------MISSILE HANDLING ROUTINES-----------------------
//-------------------------------------------------------------------

//
// P_ActExplodeMissile
//
// Explode the projectile - kill it.
//
void P_ActExplodeMissile (mobj_t* projectile)
{
  projectile->momx = projectile->momy = projectile->momz = 0;

  P_SetMobjState (projectile, projectile->info->deathstate);

  projectile->tics -= P_Random()&3;

  if (projectile->tics < 1)
    projectile->tics = 1;

  projectile->flags &= ~MF_MISSILE;

  if (projectile->info->deathsound)
    S_StartSound (projectile, projectile->info->deathsound);

}

//
// P_ActCheckMissileSpawn
//
// This procedure handles a newly spawned missile, it moved
// by half the amount of momentum and then checked to see
// if the move is possible, if not the projectile is
// exploded. Also the number of initial tics on its
// current state is taken away from by a random number
// between 0 and 3, although the number of tics will never
// go below 1.
//
// -ACB- 1998/08/04
//
void P_ActCheckMissileSpawn (mobj_t* projectile)
{
  projectile->tics -= P_Random()&3;

  if (projectile->tics < 1)
    projectile->tics = 1;

  projectile->x += (projectile->momx>>1);
  projectile->y += (projectile->momy>>1);
  projectile->z += (projectile->momz>>1);
    
  if (!P_TryMove (projectile, projectile->x, projectile->y))
    P_ActExplodeMissile (projectile);
}

//
// P_ActLaunchProjectile
//
// This procedure launches a project the direction of the target mobj.
// * source - the source of the projectile
// * target - the target of the projectile
// * type   - the mobj type of the projectile
//
// For all sense and purposes it is possible for the target to be a dummy
// mobj, just to act as a carrier for a set of target co-ordinates.
//
// Missiles can be spawned at different locations on and around
// the mobj. Traditionally an mobj would fire a projectile
// at a height of 32*FRACUNIT from the centerpoint of that
// mobj, this was true for all creatures from the Cyberdemon to
// the Imp. The currentattack holds the height and x & y
// offsets that dictates the spawning location of a projectile.
//
// Traditionally: Height   = 4*8*FRACUNIT
//                x-offset = 0
//                y-offset = 0
//
// The exception to this rule is the revenant, which did increase
// its z value by 16*FRACUNIT before firing: This was a hack
// to launch a missile at a height of 48*FRACUNIT. The revenants
// height was reduced to normal after firing, this new code
// makes that an unnecesary procedure.
//
// projx, projy & projz are the projectiles spawn location
//
// -ACB- 1998/08/04
// -KM- 1998/11/25 Accuracy is now a fixed_t
//
mobj_t* P_ActLaunchProjectile(mobj_t* source, mobj_t* target, mobjinfo_t* type)
{
  attacktype_t* attack;
  angle_t angle;
  fixed_t projx;
  fixed_t projy;
  fixed_t projz;
  mobj_t* projectile = NULL;
  int distance;

  projx = projy = projz = 0;

  attack = source->currentattack;

  projx = source->x;
  projy = source->y;

  // do not calculate offsets relative to the angle, if
  // xoffset & yoffset don't hold any value.
  if (attack->xoffset || attack->yoffset)
  {
    angle = source->angle+ANG90;
    angle >>= ANGLETOFINESHIFT;
    projx += (FixedMul(attack->xoffset, finecosine[angle]));
    projy += (FixedMul(attack->xoffset, finesine[angle]));

    angle = source->angle;
    angle >>= ANGLETOFINESHIFT;
    projx += (FixedMul(attack->yoffset, finecosine[angle]));
    projy += (FixedMul(attack->yoffset, finesine[angle]));

    projz = source->z + attack->height;
    projectile = P_MobjCreateObject(projx, projy, projz, type);

    // check for blocking lines between source and projectile
    if (P_MapCheckBlockingLine(source, projectile))
    {
      P_RemoveMobj(projectile);
      return NULL;
    }
  }
  else
  {
    projz = source->z + attack->height;
    projectile = P_MobjCreateObject(projx, projy, projz, type);
  }

  if (projectile->info->seesound)
    S_StartSound (projectile, projectile->info->seesound);

  //
  // currentattack is held so that when a collision takes place
  // with another object, we know whether or not the object hit
  // can shake off the attack or is damaged by it.
  //
  projectile->currentattack = attack;
  projectile->source = source;

  angle = R_PointToAngle2 (projx, projy, target->x, target->y);

  // is the attack not accurate?
  if (attack->accuracy != FRACUNIT)
    angle += (P_Random()-P_Random())*(FRACUNIT-attack->accuracy)*20;


  //
  // Now add the fact that the target may be difficult to spot and
  // make the projectile's target the same as the sources. Only
  // do these if the object is not a dummy object, otherwise just
  // flag the missile not to trace: you cannot track a target that
  // does not exist...
  //
  if (target->extendedflags & EF_DUMMYMOBJ)
  {
    projectile->extendedflags |= EF_NOTRACE;
  }
  else
  {
    projectile->target = target;
    projectile->extendedflags |= EF_FIRSTCHECK;

    if (target->flags & MF_SHADOW)
      angle += (P_Random()-P_Random()) << 20;

    if (target->invisibility != VISIBLE)
      angle += (P_Random()-P_Random()) * ((VISIBLE-target->invisibility)>>10);
  }

  projectile->angle = angle;
  angle >>= ANGLETOFINESHIFT;
  projectile->momx = FixedMul(finecosine[angle], projectile->speed);
  projectile->momy = FixedMul( finesine[angle],  projectile->speed);

  // Calculate Z momentum
  distance = P_AproxDistance (target->x - projx, target->y - projy);

  distance = FixedDiv(distance, projectile->speed);

  if (distance < 1)
    distance = 1;

  projectile->momz = FixedDiv(target->z + target->height / 2 - projz, distance);

  P_ActCheckMissileSpawn (projectile);

  return projectile;
}

// -KM- 1998/10/29
// P_ActLaunchSmartProjectile, by Kester Maddock.  This procedure has the same
// effect as P_ActLaunchProjectile, but it calculates a point where the target
// and missile will intersect.  This comes from the fact that to shoot something,
// you have to aim slightly ahead of it.  It will also put an end to circle-strafing.
// :-)
// -KM- 1998/12/16 Fixed it up.  Works quite well :-)
mobj_t* P_ActLaunchSmartProjectile(mobj_t* source, mobj_t* target, mobjinfo_t* type)
{
  double a, b, c;
  double t1 = -1, t2 = -1, t = -1;
  double dx, dy;
  double mx, my;
  double s;

  mx = ((double) target->momx) / 65536.0;
  my = ((double) target->momy) / 65536.0;

  dx = ((double) (source->x - target->x)) / 65536.0;
  dy = ((double) (source->y - target->y)) / 65536.0;

  s = (((double) type->speed) / 65536.0);
  if (gameflags.fastparm)
    s *= (((double) type->fast)/65536.0);

  a = mx*mx + my*my - s*s;
  b = 2 * (dx*mx + dy*my);
  c = dx*dx + dy*dy;

  if (a && ((b*b - 4*a*c) >= 0))
  {
    t1 = -b + sqrt(b*b - 4*a*c);
    t1 /= 2*a;

    t2 = -b - sqrt(b*b - 4*a*c);
    t2 /= 2*a;
  }

  if (t1 < 0)
    t1 = t2;

  if (t2 < 0)
    t2 = t1;

  t = t1<t2? t1:t2;

  if (t > 0)
  {
    static mobj_t spot;
    mobj_t* projectile;

    spot.x = target->x + ((fixed_t) (mx * t * 65536.0));
    spot.y = target->y + ((fixed_t) (my * t * 65536.0));
    spot.z = target->z + target->height/2;
    spot.height = 0;
    spot.extendedflags = EF_DUMMYMOBJ;

//    P_ActFaceTarget(source);

    projectile = P_ActLaunchProjectile(source, &spot, type);
    source->angle = projectile->angle;

    return projectile;
  }
  return P_ActLaunchProjectile(source, target, type);
}
//
// P_ActMissileContact
//
// Called by PIT_Checkthing when a missile comes into
// contact with another object. Placed here with
// the other missile code for cleaner code.
//
// -ACB- 1998/08/10
//
void P_ActMissileContact(mobj_t *object, mobj_t* objecthit)
{
  mobj_t* source;
  int damage, dmrange, dmmulti;

  source = object->source;

  if (source)
  {
    if (source->info == objecthit->info)
    {
      if (!(objecthit->extendedflags & EF_DISLOYALTYPE))
        return;
    }


    if (object->currentattack != NULL &&
          !(objecthit->extendedflags & EF_OWNATTACKHURTS))
    {
      if (object->currentattack == objecthit->info->rangeattack)
        return;
      if (object->currentattack == objecthit->info->closecombat)
        return;
    }

  }

  // transitional hack
  if (object->currentattack == NULL)
  {
    damage = ((P_Random()%8)+1)*object->info->damage;
  }
  else
  {
    dmrange = object->currentattack->damagerange;
    dmmulti = object->currentattack->damagemulti;
    damage = ((P_Random()%dmrange)+1)*dmmulti;
  }

#ifdef DEVELOPERS
  if (!damage)
    Debug_Printf("%s has caused damage of zero.\n",object->info->name);
#endif

  P_DamageMobj (objecthit, object, object->source, damage);
  return;
}

//
// P_ActCreateSmokeTrail
//
// Just spawns smoke behind an mobj: the smoke is
// risen by giving it z momentum, in order to
// prevent the smoke appearing uniform (which obviously
// does not happen), the number of tics that the smoke
// mobj has is "randomly" reduced, although the number
// of tics never gets to zero or below.
//
// -ACB- 1998/08/10
//
void P_ActCreateSmokeTrail (mobj_t* projectile)
{
  mobj_t* smoke;
  fixed_t z;

  z = projectile->z + 4*FRACUNIT;

  // spawn a puff of smoke behind the rocket
  P_SpawnPuff (projectile->x, projectile->y, z);
        
  smoke = P_MobjCreateObject (projectile->x-projectile->momx,
                                projectile->y-projectile->momy,
                                  z, specials[MOBJ_SMOKE]);

  smoke->momz = FRACUNIT;
  smoke->tics -= P_Random()&3;

  if (smoke->tics < 1)
    smoke->tics = 1;
}

//
// P_ActRandomHomingProjectile
//
// This projectile will alter its course to intercept its
// target, if is possible for this procedure to be called
// and nothing results because of a chance that the
// projectile will not chase its target.
//
// As this code is based on the revenant tracer, it did use
// a bit check on the current gametic - which was why every so
// often a revenant fires a missile straight and not one that
// homes in on its target: If the gametic has bits 1+2 on
// (which boils down to 1 in every 4 tics), the trick in this
// is that - in conjuntion with the tic count for the
// tracing object's states - the tracing will always fail or
// pass the check: if it passes first time, it will always
// pass and vice versa. The problem with this was two fold:
// demos will go out of sync if the starting gametic if different
// from when the demo was recorded (which admittly is easily
// fixable), the second is that for someone designing a new
// tracing projectile it would be more than a bit confusing to
// joe "dooming" public.
//
// The new system that affects the original gameplay slightly is
// to get a random chance of the projectile not homing in on its
// target and working this out first time round, the test result
// is recorded (in the form of the presence or lack of the
// extended flag: EF_NOTRACE) and everytime this procedure is
// called, it will check for the flag and act accordingly.
//
// Chance calculated is in percentage terms. The procedure below
// this one gives the original gameplay.
//
// -ACB- 1998/08/10
//
void P_ActRandomHomingProjectile (mobj_t* projectile)
{
  int i;
  angle_t exact;
  fixed_t distance;
  fixed_t slope;
  mobj_t* destination;

  if (projectile->extendedflags & EF_NOTRACE)
    return;

  if (projectile->extendedflags & EF_FIRSTCHECK)
  {
    i = P_Random();

    // if either value is zero, the projectile will trace
    if (i && projectile->currentattack->notracechance)
    {
      // notracechance is in percentage terms
      i = (i * 100) / 255;

      if (i < projectile->currentattack->notracechance)
      {
        projectile->extendedflags |= EF_NOTRACE;
        return;
      }

    }

    projectile->extendedflags &= ~EF_FIRSTCHECK;
  }

  destination = projectile->target;

  if (projectile->currentattack->flags & AF_TRACESMOKE)
    P_ActCreateSmokeTrail(projectile);

  if (!destination || destination->health <= 0)
    return;
    
  // change angle
  exact = R_PointToAngle2 (projectile->x, projectile->y,
                            destination->x, destination->y);

  if (exact != projectile->angle)
  {
    if (exact - projectile->angle > 0x80000000)
    {
      projectile->angle -= TRACEANGLE;

      if (exact - projectile->angle < 0x80000000)
        projectile->angle = exact;
    }
    else
    {
       projectile->angle += TRACEANGLE;

       if (exact - projectile->angle > 0x80000000)
         projectile->angle = exact;
    }
  }
        
  exact = projectile->angle>>ANGLETOFINESHIFT;
  projectile->momx = FixedMul (projectile->speed, finecosine[exact]);
  projectile->momy = FixedMul (projectile->speed, finesine[exact]);
    
  // change slope
  distance = P_AproxDistance (destination->x - projectile->x,
                              destination->y - projectile->y);
    
  distance = distance / projectile->speed;

  if (distance < 1)
    distance = 1;

  slope = (destination->z+40*FRACUNIT - projectile->z) / distance;

  if (slope < projectile->momz)
    projectile->momz -= FRACUNIT/8;
  else
    projectile->momz += FRACUNIT/8;
}

//
// P_ActFixedHomingProjectile
//
// This projectile will alter its course to intercept its
// target, if is possible for this procedure to be called
// and nothing results because of a chance that the
// projectile will not chase its target.
//
// Same as above, but more this is for purists; the above
// procedure gives a random chance; the one here is based on
// a modulas result from gametic (subtracting the levelstarttic
// to make sure the results are the same when playing back the
// demos: it is possible for gametic to be different when playing
// a demo), the test result is recorded (in the form of the
// presence or lack of the extended flag: EF_NOTRACE) and
// everytime this procedure is called, it will check for the
// flag and act accordingly. Although a Purist function, it does
// allow a dual firing object thats launchs two tracer-types
// in the same tic to generate either tracers or normal missiles.
//
// -ACB- 1998/08/20
//
void P_ActFixedHomingProjectile (mobj_t* projectile)
{
  int i;
  angle_t exact;
  fixed_t distance;
  fixed_t slope;
  mobj_t* destination;

  if (projectile->extendedflags & EF_NOTRACE)
    return;

  if (projectile->extendedflags & EF_FIRSTCHECK)
  {
    i = (gametic-levelstarttic);
    projectile->extendedflags &= ~EF_FIRSTCHECK;

    // if value is zero, the projectile will trace
    if (projectile->currentattack->notracechance)
    {
      i = i%projectile->currentattack->notracechance;

      if (!i)
      {
        projectile->extendedflags |= EF_NOTRACE;
        return;
      }
    }

  }

  destination = projectile->target;

  if (projectile->currentattack->flags & AF_TRACESMOKE)
    P_ActCreateSmokeTrail(projectile);

  if (!destination || destination->health <= 0)
    return;
    
  // change angle
  exact = R_PointToAngle2 (projectile->x, projectile->y,
                           destination->x, destination->y);

  if (exact != projectile->angle)
  {
    if (exact - projectile->angle > 0x80000000)
    {
      projectile->angle -= TRACEANGLE;

      if (exact - projectile->angle < 0x80000000)
        projectile->angle = exact;
    }
    else
    {
       projectile->angle += TRACEANGLE;

       if (exact - projectile->angle > 0x80000000)
         projectile->angle = exact;
    }
  }
        
  exact = projectile->angle>>ANGLETOFINESHIFT;
  projectile->momx = FixedMul (projectile->speed, finecosine[exact]);
  projectile->momy = FixedMul (projectile->speed, finesine[exact]);
    
  // change slope
  distance = P_AproxDistance (destination->x - projectile->x,
                              destination->y - projectile->y);
    
  distance = distance / projectile->speed;

  if (distance < 1)
    distance = 1;

  slope = (destination->z+40*FRACUNIT - projectile->z) / distance;

  if (slope < projectile->momz)
    projectile->momz -= FRACUNIT/8;
  else
    projectile->momz += FRACUNIT/8;
}

//
// P_ActLaunchOrderedSpread
//
// Due to the unique way of handling that the mancubus fires, it is necessary
// to write a single procedure to handle the firing. In real terms it amounts
// to a glorified hack; The table holds the angle modifier and the choice of
// whether the firing object or the projectile is affected. This procedure
// should NOT be used for players as it will alter the player's mobj, bypassing
// the normal player controls; The only reason for its existance is to maintain
// the original mancubus behaviour. Although it is possible to make this generic,
// the benefits of doing so are minimal. Purist function....
//
// -ACB- 1998/08/15
//
void P_ActLaunchOrderedSpread(mobj_t *object)
{
  // left side = angle modifier
  // right side = object or projectile (true for object).
  static int spreadorder[] =
  {
    (ANG90/8),  true,
    (ANG90/8),  false,
   -(ANG90/8),  true,
   -(ANG90/4),  false,
   -(ANG90/16), false,
    (ANG90/16), false
  };

  mobj_t* projectile;
  angle_t angle;
  int count;

  count = object->spreadcount;

  if (count < 0 || count > 12)
    object->spreadcount = 0;

  // object or projectile? - if true is the object, else it is the projectile
  if (spreadorder[count+1])
  {
    object->angle += spreadorder[count];
    P_ActLaunchProjectile(object, object->target, object->currentattack->projectile);
  }
  else
  {
    projectile = P_ActLaunchProjectile(object, object->target,
                                         object->currentattack->projectile);

    if (projectile==NULL)
      return;

    projectile->angle += spreadorder[count];
    angle = projectile->angle >> ANGLETOFINESHIFT;

    projectile->momx = FixedMul (projectile->speed, finecosine[angle]);
    projectile->momy = FixedMul (projectile->speed, finesine[angle]);
  }

  object->spreadcount+=2;
}

//
// P_ActLaunchRandomSpread
//
// This is a the generic function that should be used for a spreader like
// mancubus, although its random nature would certainly be a change to the
// ordered method used now. The random number is bit shifted to the right
// and then the ANG90 is divided by it, the first bit of the RN is checked
// to detemine if the angle is change is negative or not (approx 50% chance).
// The result is the modifier for the projectile's angle.
//
// -ACB- 1998/08/15
//
void P_ActLaunchRandomSpread(mobj_t *object)
{
  mobj_t *projectile;
  angle_t spreadangle;
  angle_t angle;
  int i;

  projectile = P_ActLaunchProjectile(object, object->target,
                                         object->currentattack->projectile);

  if (projectile==NULL)
    return;

  i = P_Random()%128;

  if (i>>1)
  {
    spreadangle = (ANG90/(i>>1));

    if (i & 1)
      spreadangle -= spreadangle << 1;

    projectile->angle += spreadangle;
  }

  angle = projectile->angle >> ANGLETOFINESHIFT;

  projectile->momx = FixedMul (projectile->speed, finecosine[angle]);
  projectile->momy = FixedMul (projectile->speed, finesine[angle]);

}

//-------------------------------------------------------------------
//-------------------LINEATTACK ATTACK ROUTINES-----------------------
//-------------------------------------------------------------------
// -KM- 1998/11/25 Added uncertainty to the z component of the line.
void P_ActShotAttack(mobj_t *object)
{
  angle_t angle = 0;
  int count;
  int damage;
  int horzaccshift;
  angle_t objangle;
  fixed_t slope, objslope;
  int i;
  attacktype_t* attack;
        
  attack = object->currentattack;
  horzaccshift = attack->accuracy;
  count = attack->count;

  // -ACB- 1998/09/05 Remember to use the object angle, fool!
  objangle = object->angle;
  if (object->player && (object->target->extendedflags & EF_DUMMYMOBJ))
    objslope = object->vertangle;
  else
    objslope = P_AimLineAttack(object, objangle, MISSILERANGE);

  if (attack->sound)
    S_StartSound (object, attack->sound);

  // -ACB- 1998/09/04 Fixed minor bug; was horzaccshift (which is very hard to play).
  if (horzaccshift == FRACUNIT)
  {
    for (i=0; i<count; i++)
    {
      damage = ((P_Random()%attack->damagerange)+1)*attack->damagemulti;
      P_LineAttack (object, objangle, MISSILERANGE, objslope, damage);
    }
  }
  else
  {
    for (i=0; i<count; i++)
    {
      angle = objangle + ((P_Random()-P_Random())*(FRACUNIT-horzaccshift)*20);
      slope = objslope + ((P_Random()-P_Random())*(FRACUNIT-horzaccshift))/1024;
      damage = ((P_Random()%attack->damagerange)+1)*attack->damagemulti;
      P_LineAttack (object, angle, MISSILERANGE, slope, damage);
    }
  }

}

// -KM- 1998/11/25 BFG Spray attack.  Must be used from missiles.
//   Will do a BFG spray on every monster in sight.
void P_ActSprayAttack(mobj_t* mo)
{
    int			i;
    int			damage;
    angle_t		an;
    attacktype_t*       attack;
    mobj_t*             m;

    attack = mo->currentattack;
	
    // offset angles from its attack angle
    for (i=0 ; i<40 ; i++)
    {
	an = mo->angle - ANG90/2 + (ANG90/40)*i;

	// mo->source is the originator (player)
	//  of the missile
	P_AimLineAttack (mo->source?mo->source:mo, an, attack->range);

	if (!linetarget)
	    continue;

        m = P_MobjCreateObject(linetarget->x,
                           linetarget->y,
                           linetarget->z + (linetarget->height>>2),
                           attack->projectile);
	
	// -KM- 1998/12/21 oops was
        // damage = ((P_Random()*attack->damagerange) + 1)*attack->damagemulti;
        // giving about 3000 damage per shot...
	damage = ((P_Random()%attack->damagerange) + 1)*attack->damagemulti;

	P_DamageMobj (linetarget, NULL, mo->source, damage);
    }
}

//-------------------------------------------------------------------
//--------------------TRACKER HANDLING ROUTINES----------------------
//-------------------------------------------------------------------

//
// A Tracker is an object that follows its target, by being on top of
// it. This is the attack style used by an Arch-Vile. The first routines
// handle the tracker itself, the last two are called by the source of
// the tracker.
//

//
// P_ActTrackerFollow
//
// Called by the tracker to follow its target.
//
// -ACB- 1998/08/22
//
void P_ActTrackerFollow(mobj_t* tracker)
{
  mobj_t* destination;
  fixed_t angle;
                
  destination = tracker->target;

  if (!destination)
    return;

  // check for dummy object
  if (destination->extendedflags & EF_DUMMYMOBJ)
    return;

  // Can the source of the tracker, see the destination target?
  if (!P_CheckSight (tracker->source, destination))
    return;

  angle = destination->angle >> ANGLETOFINESHIFT;

  P_UnsetThingPosition (tracker);
  tracker->x = destination->x + FixedMul (24*FRACUNIT, finecosine[angle]);
  tracker->y = destination->y + FixedMul (24*FRACUNIT, finesine[angle]);
  tracker->z = destination->z;
  P_SetThingPosition (tracker);

}

//
// P_ActTrackerActive
//
// Called by the tracker to make its active sound: also tracks
//
// -ACB- 1998/08/22
//
void P_ActTrackerActive(mobj_t* tracker)
{
  if (tracker->info->activesound)
    S_StartSound(tracker, tracker->info->activesound);

  P_ActTrackerFollow(tracker);
}

//
// P_ActTrackerStart
//
// Called by the tracker to make its launch (see) sound: also tracks
//
// -ACB- 1998/08/22
//
void P_ActTrackerStart(mobj_t* tracker)
{
  if (tracker->info->seesound)
    S_StartSound(tracker, tracker->info->seesound);

  P_ActTrackerFollow(tracker);
}

//
// P_ActLaunchTracker
//
// This procedure starts a tracking object off and links
// the tracker and the object together.
//
// -ACB- 1998/08/22
//
void P_ActLaunchTracker(mobj_t* object)
{
  mobj_t* tracker;
  mobj_t* target;
  attacktype_t* attack;

  attack = object->currentattack;
  target = object->target;
        
  if (!object->target)
    return;

  tracker = P_MobjCreateObject(target->x, target->y, target->z, attack->projectile);
  
  object->tracer = tracker;    // link the tracker to the object
  tracker->source = object;    // tracker source is the object
  tracker->target = target;    // tracker's target is the object's target
  P_ActTrackerFollow(tracker);
}

//
// P_ActEffectTracker
//
// Called by the object that launched the tracker to
// cause damage to its target and a radius attack
// (explosion) at the location of the tracker.
//
// -ACB- 1998/08/22
//
void P_ActEffectTracker(mobj_t* object)
{
  mobj_t* tracker;
  mobj_t* target;
  attacktype_t* attack;
  angle_t angle;
  int damage;
        
  if (!object->target)
    return;

  attack = object->currentattack;
  target = object->target;

  if (attack->flags & AF_FACETARGET)
    P_ActFaceTarget(object);

  if (attack->flags & AF_NEEDSIGHT)
  {
    if (!P_CheckSight (object, target))
      return;
  }

  if (attack->sound)
    S_StartSound (object, attack->sound);

  if (!attack->damagerange)
    damage = attack->damage; // if no damagerange, it must be a set damage attack
  else
    damage = ((P_Random()%attack->damagerange)+1)*attack->damagemulti;

  P_DamageMobj(target, object, object, damage);
  target->momz = 1000*FRACUNIT/target->info->mass;      
  angle = object->angle >> ANGLETOFINESHIFT;
  tracker = object->tracer;

  if (!tracker)
    return;

#ifdef DEVELOPERS
  if (!damage)
    Debug_Printf("%s + %s attack has done zero damage\n",
                  object->info->name,
                   tracker->info->name);
#endif

  // move the tracker between the object and the object's target
  tracker->x = target->x - FixedMul (24*FRACUNIT, finecosine[angle]);
  tracker->y = target->y - FixedMul (24*FRACUNIT, finesine[angle]);

  if (!tracker->info->damagerange)
    damage = tracker->info->damage;
  else
    damage = ((P_Random()%tracker->info->damagerange)+1)*tracker->info->damagemulti;

#ifdef DEVELOPERS
  if (!damage)
    Debug_Printf("%s + %s explosive attack has done zero damage\n",
                   object->info->name,
                    tracker->info->name);
#endif

  P_RadiusAttack (tracker, object, damage);

}

//-----------------------------------------------------------------
//--------------------BOSS HANDLING PROCEDURES---------------------
//-----------------------------------------------------------------

// soon to be here................
void P_ActShootToSpot(mobj_t* object)
{
  static mobj_t** targetspots = NULL;
  static int targetnum = 0;
  static int currtarget = 0;
  mobj_t* currmobj;

  if (!targetnum)
  {
    currmobj = mobjlisthead;
    while (currmobj != NULL)
    {
      if (currmobj->info == specials[MOBJ_SPAWNSPOT])
        targetnum++;

      currmobj = currmobj->next;
    }

    if (!targetnum)
      I_Error("P_ActShootSpawner: No spawn spots on this map");

    targetspots = Z_Malloc(sizeof(mobj_t*)*targetnum, PU_LEVEL, 0);

    currmobj = mobjlisthead;
    currtarget=0;
    while (currmobj != NULL)
    {
      if (currmobj->info == specials[MOBJ_SPAWNSPOT])
      {
        targetspots[currtarget] = currmobj;
        currtarget++;
      }
      currmobj = currmobj->next;
    }

    currtarget=0;
  }

  currtarget = (currtarget+1)%targetnum;
  P_ActLaunchProjectile(object, targetspots[currtarget],
                          object->currentattack->projectile);

}

//-------------------------------------------------------------------
//-------------------OBJECT-SPAWN-OBJECT HANDLING--------------------
//-------------------------------------------------------------------

//
// P_ActObjectSpawning
//
// An Object spawns another object and is spawned in the state specificed
// by attack->objinitstate. The procedure is based on the A_PainShootSkull
// which is the routine for shooting skulls from a pain elemental. In
// this the object being created is decided in the attack. This
// procedure also used the new blocking line check to see if
// the object is spawned across a blocking line, if so the procedure
// terminates.
//
// -ACB- 1998/08/23
//
void P_ActObjectSpawning(mobj_t* object, angle_t angle)
{
  fixed_t spawnx;
  fixed_t spawny;
  fixed_t spawnz;

  attacktype_t* attack;
  mobjinfo_t* shoottype;
  mobj_t* newobject;
  int prestep;

  attack = object->currentattack;
  shoottype = attack->projectile;

  angle >>= ANGLETOFINESHIFT;

  if (attack->flags & AF_SPAWNPRESTEP)
  {
    prestep = 4*FRACUNIT+3*(object->info->radius+shoottype->radius)/2;
    spawnx = object->x + FixedMul (prestep, finecosine[angle]);
    spawny = object->y + FixedMul (prestep, finesine[angle]);
  }
  else
  {
    spawnx = object->x;
    spawny = object->y;
  }

  spawnz = object->z + attack->height;

  newobject = P_MobjCreateObject(spawnx, spawny, spawnz, shoottype);

  // Blocking line detected between object and spawnpoint?
  if (P_MapCheckBlockingLine(object, newobject))
  {
    P_RemoveMobj(newobject);
    return;
  }

  if (attack->sound)
    S_StartSound(object, attack->sound);

  // If the object cannot move from its position, remove it or kill it.
  if (!P_TryMove (newobject, newobject->x, newobject->y))
  {
    if (attack->flags & AF_SPAWNREMOVEFAIL)
      P_RemoveMobj (newobject);                        // remove
    else
      P_DamageMobj (newobject, object, object, 10000); // kill

    return;
  }
                
  newobject->target = object->target;
  newobject->playxtra = object->playxtra;
  P_SetMobjState (newobject, attack->objinitstate);
}

void P_ActObjectTripleSpawn(mobj_t* object)
{
  P_ActObjectSpawning(object, object->angle+ANG90);
  P_ActObjectSpawning(object, object->angle+ANG180);
  P_ActObjectSpawning(object, object->angle+ANG270);
}

//-------------------------------------------------------------------
//-------------------SKULLFLY HANDLING ROUTINES----------------------
//-------------------------------------------------------------------

//
// P_ActSkullFlyAssault
//
// This is the attack procedure for objects that launch themselves
// at their target like a missile.
//
// -ACB- 1998/08/16
//
void P_ActSkullFlyAttack (mobj_t* object)
{
  mobj_t* destination;
  angle_t angle;
  int distance;
  fixed_t speed;
  sfx_t* sound;

  if (!object->target)
    return;

  speed = object->currentattack->speed*FRACUNIT;
  if (gameflags.fastparm)
    speed = FixedMul(speed, object->currentattack->projectile->fast);
  destination = object->target;
  object->flags |= MF_SKULLFLY;
  sound = object->currentattack->initsound;

  if (sound)
    S_StartSound (object, sound);

  angle = object->angle >> ANGLETOFINESHIFT;

  object->momx = FixedMul (speed, finecosine[angle]);
  object->momy = FixedMul (speed, finesine[angle]);

  distance = P_AproxDistance (destination->x - object->x,
                               destination->y - object->y);

  distance /= speed;
    
  if (distance < 1) distance = 1;

  object->momz = (destination->z + (destination->height>>1) - object->z) / distance;
}

//
// P_ActSlammedIntoObject
//
// Used when a flying object hammers into another object when on the
// attack. Replaces the code in PIT_Checkthing.
//
// -ACB- 1998/07/29 
//
void P_ActSlammedIntoObject(mobj_t *object, mobj_t* objecthit)
{
  int damage, dmrange, dmmulti;
  sfx_t* sound = object->currentattack->sound;

  if (objecthit != NULL)
  {
    dmrange = object->currentattack->damagerange;
    dmmulti = object->currentattack->damagemulti;

    damage = ((P_Random()%dmrange+1)*dmmulti);

    P_DamageMobj (objecthit, object, object, damage);
  }

  if (sound)
    S_StartSound(object, sound);

  object->flags &= ~MF_SKULLFLY;
  object->momx = object->momy = object->momz = 0;

  // -ACB- 1998/09/06 if we have no spawnstate, use meanderstate.
  if (object->info->spawnstate)
    P_SetMobjState (object, object->info->spawnstate);
  else
    P_SetMobjState (object, object->info->meanderstate);

  return;
}

//-------------------------------------------------------------------
//--------------------ATTACK HANDLING PROCEDURES---------------------
//-------------------------------------------------------------------

//
// P_ActAttack
//
// When an object goes on the attack, it current attack is handled here;
// the attack type is discerned and the assault is launched.
//
// -ACB- 1998/08/07
//
void P_ActAttack(mobj_t *object)
{
  attacktype_t* attack;
  mobj_t* target;
  int damage;

  attack = object->currentattack;
  target = object->target;

  switch(attack->attackstyle)
  {

    case ATK_CLOSECOMBAT:
    {
      // -KM- 1998/12/21 Use Line attack so bullet puffs are spawned.
      fixed_t slope;
      if (!P_ActDecideMeleeAttack(object, attack))
      {
        P_LineAttack (object, object->angle, attack->range, object->vertangle, 1);
        return;
      }

      if (attack->sound)
        S_StartSound(object,attack->sound);

      // -KM- 1998/11/25 Berserk ability
      if (object->player && object->player->powers[pw_strength])
        damage = ((P_Random()%attack->damagerange)+1)*attack->damagemulti*10;
      else
        damage = ((P_Random()%attack->damagerange)+1)*attack->damagemulti;

      slope = P_AimLineAttack(object, object->angle, attack->range);
      P_LineAttack (object, object->angle, attack->range, slope, damage);
//      P_DamageMobj (target, object, object, damage);
      // -KM- 1998/11/25 Close combat spawns blood.
//      P_SpawnBlood (target->x,target->y,target->z+attack->height, damage,
//                    object->angle >= ANG180 ? object->angle - ANG180 : object->angle + ANG180);
      break;
    }

    case ATK_PROJECTILE:
    {
      P_ActLaunchProjectile(object, target, attack->projectile);
      break;
    }

    case ATK_SMARTPROJECTILE:
    {
      P_ActLaunchSmartProjectile(object, target, attack->projectile);
      break;
    }

    case ATK_RANDOMSPREAD:
    {
      P_ActLaunchRandomSpread(object);
      break;
    }

    case ATK_SHOOTTOSPOT:
    {
      P_ActShootToSpot(object);
      break;
    }

    case ATK_SHOT:
    {
      P_ActShotAttack(object);
      break;
    }

    case ATK_SKULLFLY:
    {
      P_ActSkullFlyAttack(object);
      break;
    }

    case ATK_SPAWNER:
    {
      P_ActObjectSpawning(object, object->angle);
      break;
    }

    case ATK_SPREADER:
    {
      P_ActLaunchOrderedSpread(object);
      break;
    }

    case ATK_TRACKER:
    {
      P_ActLaunchTracker(object);
      break;
    }

    case ATK_TRIPLESPAWNER:
    {
      P_ActObjectTripleSpawn(object);
      break;
    }

    // -KM- 1998/11/25 Added spray attack
    case ATK_SPRAY:
    {
      P_ActSprayAttack(object);
    }

    default: // THIS SHOULD NOT HAPPEN
    {
#ifdef DEVELOPERS
      Debug_Printf("P_ActAttack: %s has an unknown attack type.\n",object->info->name);
#endif
      break;
    }
  }

}

//
// P_ActComboAttack
//
// This is called at end of a set of states that can result in
// either a closecombat or ranged attack. The procedure checks
// to see if the target is within melee range and picks the
// approiate attack.
//
// -ACB- 1998/08/07
//
void P_ActComboAttack(mobj_t *object)
{
  attacktype_t* attack;

  if (!object->target)
    return;

  if (P_ActDecideMeleeAttack(object, object->info->closecombat))
    attack = object->info->closecombat;
  else
    attack = object->info->rangeattack;

  if (attack)
  {
    if (attack->flags & AF_FACETARGET)
      P_ActFaceTarget(object);

    if (attack->flags & AF_NEEDSIGHT)
    {
      if (!P_CheckSight (object, object->target))
        return;
    }

    object->currentattack = attack;
    P_ActAttack(object);
  }
#ifdef DEVELOPERS
  else
  {
    if (!object->info->closecombat)
      Debug_Printf("%s hasn't got a close combat attack\n",object->info->name);
    else
      Debug_Printf("%s hasn't got a range attack\n",object->info->name);
  }
#endif

}

//
// P_ActMeleeAttack
// Setup a close combat assault
//
// -ACB- 1998/08/07
//
void P_ActMeleeAttack(mobj_t *object)
{
  attacktype_t* attack;

  if (!object->target)
    return;

  attack = object->info->closecombat;

#ifdef DEVELOPERS
  if (!attack)
    I_Error("P_ActMeleeAttack: %s has no close combat attack",object->info->name);
#endif

  if (attack->flags & AF_FACETARGET)
    P_ActFaceTarget(object);

  if (attack->flags & AF_NEEDSIGHT)
  {
    if (!P_CheckSight (object, object->target))
      return;
  }

  object->currentattack = attack;
  P_ActAttack(object);
}

//
// P_ActRangeAttack
// Setup an attack at range
//
// -ACB- 1998/08/07
//
void P_ActRangeAttack(mobj_t *object)
{
  attacktype_t* attack;

  if (!object->target)
    return;

  attack = object->info->rangeattack;

#ifdef DEVELOPERS
  if (!attack)
    I_Error("P_ActRangeAttack: %s hasn't got a range attack",object->info->name);
#endif

  if (attack->flags & AF_FACETARGET)
    P_ActFaceTarget(object);

  if (attack->flags & AF_NEEDSIGHT)
  {
    if (!P_CheckSight (object, object->target))
      return;
  }

  object->currentattack = attack;
  P_ActAttack(object);
}

//
// P_ActSpareAttack
//
// Setup an attack that is not defined as close or range. can be
// used to act as a follow attack for close or range, if you want one to
// add to the others.
//
// -ACB- 1998/08/24
//
void P_ActSpareAttack(mobj_t* object)
{
  attacktype_t* attack;

// -KM- 1998/11/25 Doesn't need a target: missiles sometimes don't.
//  if (!object->target)
//    return;

  attack = object->info->spareattack;

  if (attack)
  {
    if (attack->flags & AF_FACETARGET)
      P_ActFaceTarget(object);

    if (attack->flags & AF_NEEDSIGHT)
    {
      if (!P_CheckSight (object, object->target))
        return;
    }

    object->currentattack = attack;
    P_ActAttack(object);
  }
#ifdef DEVELOPERS
  else
  {
    Debug_Printf("P_ActSpareAttack: %s hasn't got a spare attack\n",object->info->name);
    return;
  }
#endif

}

//
// P_ActRefireCheck
//
// This procedure will be called inbetween firing on an object
// that will fire repeatly (Chaingunner/Arachontron etc...), the
// purpose of this is to see if the object should refire and
// performs checks to that effect, first there is a check to see
// if the object will keep firing regardless and the others
// check if the the target exists, is alive and within view. The
// only other code here is a stealth check: a object with stealth
// capabilitys will lose the ability while firing.
//
// -ACB- 1998/08/10
//
void P_ActRefireCheck(mobj_t *object)
{
  mobj_t* target;
  attacktype_t* attack;
  boolean seetarget;

  attack = object->currentattack;

  if (attack->flags & AF_FACETARGET)
    P_ActFaceTarget(object);

  // Random chance that object will keep firing regardless
  if (P_Random () < attack->keepfirechance)
    return;

  target = object->target;

  seetarget = P_CheckSight(object, target);

  if (!target || (target->health <= 0) || !seetarget)
    P_SetMobjState (object, object->info->seestate);
  else if (object->flags & MF_STEALTH)
      object->deltainvis = VISIBLE;
}

//---------------------------------------------
//-----------LOOKING AND CHASING---------------
//---------------------------------------------

//
// P_ActStandardLook
//
// Standard Lookout procedure
//
// -ACB- 1998/08/22
//
void P_ActStandardLook (mobj_t* object)
{
  mobj_t* targ;

  object->threshold = 0;        // any shot will wake up
  targ = object->subsector->sector->soundtarget;

  if (object->flags & MF_STEALTH)
      object->deltainvis = VISIBLE;

  if (infight)
  {
    if (P_ActCreateAggression(object))
      return;
  }

  if (targ && (targ->flags & MF_SHOOTABLE))
  {
    object->target = targ;

    if (object->flags & MF_AMBUSH)
    {
      if (!P_CheckSight(object, object->target) && !P_LookForPlayers (object, false))
        return;
    }
  }
  else
  {
    if (!P_LookForPlayers (object, false))
      return;
  }

  if (object->info->seesound)
  {
    if (object->info->extendedflags & EF_BOSSMAN)
      S_StartSound (NULL, object->info->seesound);
    else
      S_StartSound (object, object->info->seesound);
  }

  P_SetMobjState (object, object->info->seestate);
}

//
// P_ActPlayerSupportLook
//
// Player Support Lookout procedure
//
// -ACB- 1998/09/05
//
void P_ActPlayerSupportLook (mobj_t* object)
{
  object->threshold = 0;        // any shot will wake up

  if (object->flags & MF_STEALTH)
      object->deltainvis = VISIBLE;

  if (!object->supportobj)
  {
    if (P_LookForPlayers(object,false))
    {
      object->supportobj = object->target;
      object->target = NULL;

      if (object->info->seesound)
      {
        if (object->info->extendedflags & EF_BOSSMAN)
          S_StartSound (NULL, object->info->seesound);
        else
          S_StartSound (object, object->info->seesound);
      }
    }
    else
    {
      return;
    }
  }

  P_SetMobjState(object, object->info->meanderstate);

}

//
// P_ActStandardMeander
//
void P_ActStandardMeander(mobj_t *object)
{
  int delta;
  object->threshold = 0;        // any shot will wake up

  // move within supporting distance of player
  if (--object->movecount<0 || !P_Move (object))
    P_NewChaseDir (object);
    
  // turn towards movement direction if not there yet
  if (object->movedir < 8)
  {
    object->angle &= (7<<29);
    delta = object->angle - (object->movedir << 29);
        
    if (delta > 0)
      object->angle -= ANG90/2;
    else if (delta < 0)
      object->angle += ANG90/2;
  }

  if (!P_LookForPlayers(object, false))
    return;

  P_SetMobjState (object, object->info->seestate);
}

//
// P_ActPlayerSupportMeander
//
void P_ActPlayerSupportMeander(mobj_t *object)
{
  int delta;
  object->threshold = 0;        // any shot will wake up

  // move within supporting distance of player
  if (--object->movecount<0 || !P_Move (object))
    P_NewChaseDir (object);
    
  // turn towards movement direction if not there yet
  if (object->movedir < 8)
  {
    object->angle &= (7<<29);
    delta = object->angle - (object->movedir << 29);
        
    if (delta > 0)
      object->angle -= ANG90/2;
    else if (delta < 0)
      object->angle += ANG90/2;
  }

  // we have now meandered, now check for a support object, if we don't
  // look for one and return; else look for targets to take out, if we
  // find one, go for the chase.
  if (!object->supportobj)
  {
    P_ActPlayerSupportLook(object);
    return;
  }

  if (P_ActLookForTargets(object))
    P_SetMobjState (object, object->info->seestate);
}

//
// P_ActStandardChase
//
// Standard AI Chase Procedure
//
// -ACB- 1998/08/22 Procedure Written
// -ACB- 1998/09/05 Added Support Object Check
//
void P_ActStandardChase(mobj_t* object)
{
  int delta;
  sfx_t* sound;

  if (object->reactiontime)
    object->reactiontime--;

  // object has a pain threshold, while this is true, reduce it. while
  // the threshold is true, the object will remain intent on its target.
  if (object->threshold)
  {
    if (!object->target || object->target->health <= 0)
      object->threshold = 0;
    else
      object->threshold--;
  }

  // A Chasing Stealth Creature becomes less visible
  if (object->flags & MF_STEALTH)
      object->deltainvis = INVISIBLE;

  // turn towards movement direction if not there yet
  if (object->movedir < 8)
  {
    object->angle &= (7<<29);
    delta = object->angle - (object->movedir << 29);
        
    if (delta > 0)
      object->angle -= ANG90/2;
    else if (delta < 0)
      object->angle += ANG90/2;
  }

  if (!object->target || !(object->target->flags&MF_SHOOTABLE))
  {
    // -ACB- 1998/09/05 Support Object setup covered
    // Original Note: look for a new target
    if (object->supportobj)
    {
      if (P_ActLookForTargets(object))
        return;
    }
    else
    {
      if (P_LookForPlayers(object,true))
        return;   // got a new target
    }

    // -ACB- 1998/09/06 Target is not relevant: NULLify.
    object->target = NULL;

    // -ACB- 1998/09/06 Use meanderstate, if no spawnstate is given
    if (object->info->spawnstate)
      P_SetMobjState(object, object->info->spawnstate);
    else
      P_SetMobjState(object, object->info->meanderstate);

    return;
  }

  // do not attack twice in a row
  if (object->flags & MF_JUSTATTACKED)
  {
    object->flags &= ~MF_JUSTATTACKED;

    // -KM- 1998/12/16 Nightmare mode set the fast parm.
    if (!gameflags.fastparm)
      P_NewChaseDir (object);

    return;
  }

  sound = object->info->attacksound;
    
  // check for melee attack
  if (object->info->meleestate && P_ActDecideMeleeAttack(object, object->info->closecombat))
  {
    if (sound)
      S_StartSound (object, sound);

    P_SetMobjState (object, object->info->meleestate);
    return;
  }
    
  // check for missile attack
  if (object->info->missilestate)
  {
    // -KM- 1998/12/16 Nightmare set the fastparm.
    if ( !(!gameflags.fastparm && object->movecount) )
    {
      if (P_ActDecideRangeAttack(object))
      {
        P_SetMobjState (object, object->info->missilestate);
        object->flags |= MF_JUSTATTACKED;
        return;
      }
    }
  }

  // possibly choose another target
  // -ACB- 1998/09/05 Object->support->object check, go for new targets
  if (!P_CheckSight(object, object->target) && !object->threshold)
  {
    if (object->supportobj)
    {
      if (P_ActLookForTargets(object))
        return;
    }
    else
    {
      if (netgame && P_LookForPlayers(object,true))
        return;   // got a new target
    }
  }
    
  // chase towards player
  if (--object->movecount<0 || !P_Move (object))
    P_NewChaseDir (object);
    
  // make active sound
  if (object->info->activesound && P_Random () < 3)
    S_StartSound (object, object->info->activesound);

}

//
// P_ActResurrectChase
//
// Before undertaking the standard chase procedure, the object
// will check for a nearby corpse and raises one if it exists.
//
// -ACB- 1998/08/22 Procedure written
// -ACB- 1998/09/05 Support Check: Raised object supports raiser's supportobj
//
void P_ActResurrectChase(mobj_t* object)
{
  mobj_t* corpse;
  mobjinfo_t* info;

  corpse = P_MapFindCorpse(object);

  if (corpse)
  {
    object->angle = R_PointToAngle2 (object->x, object->y, corpse->x, corpse->y);
    P_SetMobjState (object, object->info->resstate);
    S_StartSound (corpse, sfx_slop);

    info = corpse->info;
    P_SetMobjState (corpse, info->raisestate);
    corpse->flags = info->flags;
    corpse->health = info->spawnhealth;
    corpse->radius = info->radius;
    corpse->height = info->height;
    corpse->extendedflags = info->extendedflags;
    corpse->deltainvis = info->invisibility;

    // -ACB- 1998/09/05 Support Check: Res creatures to support that object
    if (object->supportobj)
    {
      corpse->supportobj = object->supportobj;
      corpse->target = object->target;
    }
    else
    {
      corpse->supportobj = NULL;
      corpse->target = NULL;
    }

  }

  P_ActStandardChase(object);
}

//
// P_ActWalkSoundChase
//
// Make a sound and then chase...
//
void P_ActWalkSoundChase(mobj_t* object)
{
#ifdef DEVELOPERS
  if (!object->info->walksound)
    I_Error("%s hasn't got a walksound",object->info->name);
#endif

  S_StartSound(object, object->info->walksound);
  P_ActStandardChase(object);
}

// ----------------------------------------------------------
// -----------------PLAYER WEAPON ROUTINES-------------------
// ----------------------------------------------------------

//
// P_ActWeaponLower
//
void P_ActWeaponLower (player_t* player, pspdef_t* weaponsprite)
{       
  weaponsprite->sy += WPNLOWERSPEED;

  // Is already down.
  if (weaponsprite->sy < WEAPONBOTTOM)
    return;

  // Player is dead. Don't bring weapon back up
  if (player->playerstate == PST_DEAD)
  {
    weaponsprite->sy = WEAPONBOTTOM;
    return;
  }
    
  // Player has no health? if so keep the weapon off screen.
  if (!player->health)
  {
    P_SetPsprite (player,  ps_weapon, S_NULL);
    return;
  }
        
  player->readyweapon = player->pendingweapon;

  P_BringUpWeapon (player);
}


//
// P_ActWeaponRaise
//
void P_ActWeaponRaise(player_t* player, pspdef_t* weaponsprite)
{
  int newstate;
        
  weaponsprite->sy -= WPNRAISESPEED;

  if (weaponsprite->sy > WEAPONTOP)
    return;
    
  weaponsprite->sy = WEAPONTOP;
    
  // The weapon has been raised all the way, so change to the ready state.
  newstate = weaponinfo[player->readyweapon].readystate;

  P_SetPsprite(player, ps_weapon, newstate);
}

//
// P_ActPlayerAttack
//
#define AIMDISTANCE 16*64*FRACUNIT

void P_ActPlayerAttack (mobj_t* playerobj, attacktype_t* attack)
{
  int angle = 0;

  playerobj->currentattack = attack;

  // see which target is to be aimed at
  angle = playerobj->angle;
  playerobj->target = P_MapTargetAutoAim(playerobj, angle, attack->range);

  // -KM- 1998/12/16 If that is a miss, aim slightly right or slightly left of the
  //  target.
  if (playerobj->target->extendedflags & EF_DUMMYMOBJ)
  {
    angle += 1<<26;
    playerobj->target = P_MapTargetAutoAim(playerobj, angle, attack->range);
  }

  if (playerobj->target->extendedflags & EF_DUMMYMOBJ)
  {
    angle -= 2<<26;
    playerobj->target = P_MapTargetAutoAim(playerobj, angle, attack->range);
  }

  if (attack->flags & AF_FACETARGET)
    P_ActFaceTarget(playerobj);

  P_ActAttack(playerobj);
}



















