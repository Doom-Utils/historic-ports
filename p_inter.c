//  
// DOSDoom Interactions (picking up items etc..) Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#include "dm_defs.h"
#include "dm_state.h"
#include "dstrings.h"
#include "i_system.h"
#include "lu_sound.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"
#include "z_zone.h"

#include "am_map.h"

#ifdef __GNUG__
#pragma implementation "p_inter.h"
#endif
#include "p_inter.h"

#define BONUSADD 6

// a weapon is found with two clip loads,
// a big item has five clip loads
//int maxammo[4] = {200, 50, 50, 300};
//int clipammo[NUMAMMO] = {10, 4, 1, 20};

//
// P_GiveAmmo
// Returns false if the ammo can't be picked up at all
//
// -ACB- 1998/06/19 DDF Change: Number passed is the exact amount of ammo given.
// -KM- 1998/11/25 Handles weapon change from priority.
//
boolean P_GiveAmmo (player_t* player, ammotype_t ammo, int num)
{
  int oldammo;
  int i;
  int priority = 0;
	
  if (ammo == am_noammo)
    return false;
		
  if (ammo < 0 || ammo > NUMAMMO)
    I_Error ("P_GiveAmmo: bad type %i", ammo);
		
  if (player->ammo[ammo] == player->maxammo[ammo])
    return false;

  // In Nightmare you need the extra ammo, in "baby" you are given double
  if ((gameskill == sk_baby) || (gameskill == sk_nightmare))
    num <<= 1;
    
  oldammo = player->ammo[ammo];
  player->ammo[ammo] += num;

  if (player->ammo[ammo] > player->maxammo[ammo])
    player->ammo[ammo] = player->maxammo[ammo];

  // if there is an oldammo value, we don't need to change weapons 
  if (oldammo)
    return true; 

  // We were down to zero, so select a new weapon.
  // Preferences are not user selectable.
  priority = weaponinfo[player->readyweapon].priority;
  for (i = 0; i < numweapons; i++)
  {
      if (weaponinfo[i].priority > priority &&
          player->weaponowned[i] &&
          (weaponinfo[i].ammo == am_noammo ||
          player->ammo[weaponinfo[i].ammo] >= weaponinfo[i].ammopershot) &&
          weaponinfo[i].ammo == ammo)
      {
          player->pendingweapon = i;
          priority = weaponinfo[i].priority;
      }
  }

  if (player->readyweapon == player->pendingweapon)
    player->pendingweapon = wp_nochange;
	
  return true;
}


//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
// -ACB- 1998/06/20 DDF Change: Item Info gives amount of amount given when
//                              this item is picked up.
// -KM- 1998/11/25 Handles based on Weapons.ddf
//
boolean P_GiveWeapon
( player_t* player,
  weapontype_t weapon,
  boolean dropped,
  int ammoamount,
  sfx_t* sound)
{
  boolean     gaveammo;
  boolean     gaveweapon;
  int i;

  for (i = 0; i < numweapons; i++)
  {
     if (player->weaponowned[i] && weapon == weaponinfo[i].replaces)
        return false;
  }

  // -ACB- 1998/06/27 Update Weapons Widget
  weaponupdate = true;

  if (netgame && (deathmatch <= 1) && !dropped )
  {
    // leave placed weapons forever on net games
    if (player->weaponowned[weapon])
      return false;

    player->bonuscount += BONUSADD;
    player->weaponowned[weapon] = true;
    if (weaponinfo[weapon].replaces != -1)
       player->weaponowned[weaponinfo[weapon].replaces] = false;

    //
    // Calculate assuming 'ammoamount' is the amount of ammo
    // given on a found weapon. if deathmatch multiply by 2.5 (?)
    // -ACB- 1998/06/20
    //
    if (deathmatch)
      P_GiveAmmo (player, weaponinfo[weapon].ammo, (ammoamount*5)/2 );
    else
      P_GiveAmmo (player, weaponinfo[weapon].ammo, ammoamount);

    player->pendingweapon = weapon;

    if (player == &players[consoleplayer])
      S_StartSound (NULL, sound);

    return false;
  }
	
  if (weaponinfo[weapon].ammo != am_noammo)
  {
    //
    // Calculate assuming 'ammoamount' is the amount of ammo
    // given on a found weapon. if dropped half this value.
    // -ACB- 1998/06/20
    //
    if (dropped)
      gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, ammoamount/2);
    else
      gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, ammoamount);
  }
  else
  {
    gaveammo = false;
  }
	
  if (player->weaponowned[weapon])
  {
    gaveweapon = false;
  }
  else
  {
    gaveweapon = true;
    player->weaponowned[weapon] = true;
    if (weaponinfo[weapon].replaces != -1)
       player->weaponowned[weaponinfo[weapon].replaces] = false;
    player->pendingweapon = weapon;
  }
	
  return (gaveweapon || gaveammo);
}

//
// P_GiveHealth
// Returns false if not health is not needed,
//
// New Procedure: -ACB- 1998/06/21
//
boolean P_GiveHealth ( player_t* player, int addvalue, int limit )
{
  int newvalue;

  newvalue = player->health + addvalue;

  if (player->health >= limit)
    return false;

  if (newvalue > limit)
    newvalue = limit;

  player->mo->health = player->health = newvalue;

  return true;
}

//
// P_GiveArmour
// Returns false if the new armour would not benefit
//
// -ACB- 1998/06/21
//
boolean P_GiveArmour (player_t* player, int addvalue, int limit)
{
  int newvalue;

  newvalue = player->armorpoints + addvalue;

  if (player->armorpoints >= limit)
    return false;

  if (newvalue > limit)
    newvalue = limit;

  //
  // The newvalue check should not be necessary, but just in case someone
  // specifies a value higher than the standard, but set as not going over
  // the standard value (Make warning when reading DDF Files).
  //
  if ((addvalue > STANDARDARMOUR) && (newvalue > STANDARDARMOUR))
  {
    if (player->armortype != BLUEARMOUR)
      player->armortype = BLUEARMOUR;
  }
  else
  {
    if (!player->armortype)
      player->armortype = GREENARMOUR;
  }

  player->armorpoints = newvalue;

  return true;
}
 
//
// P_GiveCard
//
void P_GiveCard (player_t* player, card_t card)
{
  if (player->cards[card])
    return;
    
  player->bonuscount = BONUSADD;
  player->cards[card] = 1;

  // -ACB- 1998/06/10 Force redraw of status bar, to update keys.
  newhupd = true;
}


//
// P_GivePower
//
// DDF Change: duration is now passed as a parameter, for the berserker
//             the value is the health given, extendedflags also passed.
//
// The code was changes to a switch instead of a series of if's, also
// included is the use of limit, which gives a maxmium amount of protection
// for this item. -ACB- 1998/06/20
//
boolean P_GivePower ( player_t*	player, int power, int amount, int limit )
{
  // -ACB- 1998/06/20 - calculate duration in seconds
  int duration = amount * TICRATE;

  // -ACB- 1998/06/20 - There is nothing worse that a series of if's..
  switch (power)
  {
    case pw_invulnerability:
    case pw_infrared:
    case pw_ironfeet:
    case pw_jetpack:
    case pw_nightvision:
    {
      limit *= TICRATE;

      if (player->powers[power] >= limit)
      {
        return false;                      // no benefit to be got from item
      }
      else
      {
        player->powers[power] += duration;

        if (player->powers[power] >= limit)
          player->powers[power] = limit;
      }

      return true;
      break;
    }

    case pw_invisibility:
    {
      limit *= TICRATE;

      if (player->powers[power] >= limit)
      {
        return false; // no benefit to be got from item
      }
      else
      {
        player->powers[power] += duration;

        if (player->powers[power] > limit)
          player->powers[power] = limit;
      }

      player->mo->flags |= MF_SHADOW;
      return true;
      break;
    }

    case pw_strength:
    {
      if (player->powers[power] && player->health > amount)
        return false;

      player->powers[power] = 1;

      P_GiveHealth(player, amount, limit);
      return true;

      break;
    }

    default:
    {
      if (player->powers[power])
        return false;        // already got it

      break;
    }
  }

  player->powers[power] = 1;
  return true;
}


//
// P_TouchSpecialThing
//
// -KM- 1999/01/31 Things that give you item bonus are always
//  picked up.  Picked up object is set to death frame instead
//  of removed so that effects can happen.
void P_TouchSpecialThing ( mobj_t* special, mobj_t* toucher )
{
  player_t* player;
  fixed_t delta;
  int addvalue;
  int limit;
  sfx_t* sound;
  int i;
  boolean dropped;
  boolean pickup = false;
  backpack_t* backpack;

  delta = special->z - toucher->z;

  // out of reach
  if (delta > toucher->height || delta < -special->height)
    return;
    	
  player = toucher->player;

  // Dead thing touching. Can happen with a sliding player corpse.
  if (toucher->health <= 0)
    return;

  addvalue = special->info->benefitamount;
  dropped = (special->flags & MF_DROPPED);
  limit = special->info->limit;
  backpack = special->backpackinfo;
  // -KM- 1998/09/27 Sounds.ddf
  sound = special->info->activesound;
  toucher->flags |= MF_JUSTPICKEDUP;

  //
  // -ACB- 1998/06/19, DDF Change: Give benefit in reference to info
  //
  switch (special->info->benefittype)
  {
    // -KM- 1998/12/16 Ammo types are generalised.
    case AMMO_TYPE:
       if (dropped)
         pickup = P_GiveAmmo (player, special->info->benefitammo, special->info->benefitamount/2);
       else
         pickup = P_GiveAmmo (player, special->info->benefitammo, special->info->benefitamount);
       break;
    case KEY_BLUECARD:
    {
      if (!player->cards[it_bluecard])
        player->message = special->info->message;

      P_GiveCard (player, it_bluecard);

      if (!netgame)
         pickup = true;
      else
      {
        player->message = special->info->message;
        S_StartSound (player->mo, sound);
      }

      break;
    }

    case KEY_REDCARD:
    {
      if (!player->cards[it_redcard])
        player->message = special->info->message;

      P_GiveCard (player, it_redcard);

      if (!netgame)
        pickup = true;
      else
      {
        player->message = special->info->message;
        S_StartSound (player->mo, sound);
      }

      break;
    }

    case KEY_YELLOWCARD:
    {
      if (!player->cards[it_yellowcard])
        player->message = special->info->message;

      P_GiveCard (player, it_yellowcard);

      if (!netgame)
        pickup = true;
      else
      {
        player->message = special->info->message;
        S_StartSound (player->mo, sound);
      }

      break;
    }

    case KEY_BLUESKULL:
    {
      if (!player->cards[it_blueskull])
        player->message = special->info->message;

      P_GiveCard (player, it_blueskull);

      if (!netgame)
        pickup = true;
      else
      {
        player->message = special->info->message;
        S_StartSound (player->mo, sound);
      }

      break;
    }

    case KEY_REDSKULL:
    {
      if (!player->cards[it_redskull])
        player->message = special->info->message;

      P_GiveCard (player, it_redskull);

      if (!netgame)
        pickup = true;
      else
      {
        player->message = special->info->message;
        S_StartSound (player->mo, sound);
      }

      break;
    }

    case KEY_YELLOWSKULL:
    {
      if (!player->cards[it_yellowskull])
        player->message = special->info->message;

      P_GiveCard (player, it_yellowskull);

      if (!netgame)
        pickup = true;
      else
      {
        player->message = special->info->message;
        S_StartSound (player->mo, sound);
      }

      break;
    }

    case POWERUP_ACIDSUIT:
    {
      pickup = P_GivePower (player, pw_ironfeet, addvalue, limit);
      break;
    }

    case POWERUP_ARMOUR:
    {
      pickup = P_GiveArmour(player, addvalue, limit);
      break;
    }

    case POWERUP_AUTOMAP:
    {
      pickup = P_GivePower (player, pw_allmap, addvalue, limit);
      break;
    }

    case POWERUP_BACKPACK:
    {
      if (backpack == NULL)
        I_Error("P_TouchSpecialThing: No backpack info!");

      // -ACB- 1998/06/27 Update Weapons Widget
      weaponupdate = true;

      for (i = 0; i < NUMAMMO; i++ )
      {
        if (backpack->ammolimit)
          if (backpack->ammolimit[i] > player->maxammo[i])
            player->maxammo[i] = backpack->ammolimit[i];
        if (backpack->ammo)
          P_GiveAmmo(player, i, backpack->ammo[i]);
      }

      if (backpack->armour > player->armorpoints)
        player->armorpoints = backpack->armour;

      if (backpack->armourclass > player->armortype)
        player->armortype = backpack->armourclass;

      if (backpack->weapons)
        for (i = 0; i < numweapons; i++ )
        {
          if (backpack->weapons[i])
            player->weaponowned[i] = true;
        }

      for (i = 0; i < NUMCARDS; i++ )
      {
         if (backpack->cards[i] == true)
           player->cards[i] = true;
      }
      pickup = true;
      break;
    }

    case POWERUP_BERSERK:
    {
      if (P_GivePower (player, pw_strength, addvalue, limit))
      {
        pickup = true;

        if (player->readyweapon != DDF_WeaponGetType("FIST"))
          player->pendingweapon = DDF_WeaponGetType("FIST");
      }
      break;
    }

    case POWERUP_HEALTH:
    {
      pickup = P_GiveHealth( player, addvalue, limit );
      break;
    }

    case POWERUP_HEALTHARMOUR:
    {
      P_GiveArmour(player, addvalue, limit);
      P_GiveHealth(player, addvalue, limit);
      pickup = true;
      break;
    }

    case POWERUP_INVULNERABLE:
    {
      pickup = P_GivePower (player, pw_invulnerability, addvalue, limit);
      break;
    }

    case POWERUP_JETPACK:
    {
      pickup = P_GivePower (player, pw_jetpack, addvalue, limit);
      break;
    }

    case POWERUP_LIGHTGOGGLES:
    {
      pickup = P_GivePower (player, pw_infrared, addvalue, limit);
      break;
    }

    case POWERUP_NIGHTVISION:
    {
      pickup = P_GivePower (player, pw_nightvision, addvalue, limit);
      break;
    }

    case POWERUP_PARTINVIS:
    {
      pickup = P_GivePower (player, pw_invisibility, addvalue, limit);
      break;
    }

    // -KM- 1998/11/25 New weapon handling
    case WEAPON:           // WEAPON
    {
      i = special->info->benefitweapon;

      pickup = P_GiveWeapon (player, i, dropped, addvalue, sound);
      break;
    }

    default:
        I_Error ("P_SpecialThing: Unknown Benefit Type Specified");
  }
	
  if (pickup || special->flags & MF_COUNTITEM)
  {
    if (special->flags & MF_COUNTITEM)
      player->itemcount++;
  
    special->health = 0;
    P_KillMobj (player->mo, special);
  
    player->bonuscount += BONUSADD;
  
    player->message = special->info->message;
    S_StartSound (player->mo, sound);
  }
}


//
// P_KillMobj
//
// Altered to reflect the fact that the dropped item
// is a pointer to mobjinfo_t, uses new procedure: P_MobjCreateObject.
//
// -ACB- 1998/08/01
//
void P_KillMobj (mobj_t* source, mobj_t* target)
{
  mobjinfo_t* item;
  mobj_t* mo;

  item = NULL;
  mo = NULL;
	
  target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

  if (!(target->extendedflags & EF_NOGRAVKILL))
    target->flags &= ~MF_NOGRAVITY;

  target->flags |= MF_CORPSE|MF_DROPOFF;
  target->height >>= 2;

  if (source && source->player)
  {
    // count for intermission
    if (target->flags & MF_COUNTKILL)
      source->player->killcount++; 

    if (target->player)
    {
      // Killed a team mate?
      if ((deathmatch >= 3) && (target->side & source->side))
      {
        source->player->frags--;
        source->player->totalfrags--;
      }
      else
      {
        source->player->frags++;
        source->player->totalfrags++;
      }
    }
  }
  else if (!netgame && (target->flags & MF_COUNTKILL) )
  {
    // count all monster deaths,
    // even those caused by other monsters
    players[0].killcount++;
  }
    
  if (target->player)
  {
    // count environment kills against you
    if (!source)
    {
      target->player->frags--;
      target->player->totalfrags--;
    }
			
    target->flags &= ~MF_SOLID;
    target->player->playerstate = PST_DEAD;
    P_DropWeapon (target->player);

    // don't die in auto map, switch view prior to dying
    if (target->player == &players[consoleplayer] && automapactive)
      AM_Stop (); 
  }

  if (target->health < -target->info->spawnhealth && target->info->xdeathstate)
    P_SetMobjState (target, target->info->xdeathstate);
  else
    P_SetMobjState (target, target->info->deathstate);

  target->tics -= P_Random()&3;

  if (target->tics < 1)
    target->tics = 1;
		
  // Drop stuff. This determines the kind of object spawned
  // during the death frame of a thing.
  item = target->info->dropitem;

  if (item)
  {
    mo = P_MobjCreateObject(target->x,target->y, ONFLOORZ, item);

    if (mo) // -ES- 1998/07/18 NULL check to prevent crashing
      mo->flags |= MF_DROPPED;
  }

}


//
// P_DamageMobj
//
// Damages both enemies and players, decreases the amount of health
// an mobj has and "kills" an mobj in the event of health being 0 or
// less, the parameters are:
// * Target    - mobj to be damaged
// * Inflictor - mobj which is causing the damage.
// * Source    - mobj who is responsible for doing the damage. Can be NULL
// * Damage    - amount of damage done
//
// Both source and inflictor can be NULL, slime damage and barrel explosions
// etc....
//
void P_DamageMobj (mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage )
{
  angle_t angle;
  int saved;
  player_t* player;
  fixed_t thrust;
  int temp;

#ifdef DEVELOPERS
  if (!(target->flags & MF_SHOOTABLE))
    return; // shouldn't happen...
#endif

  // -ACB- 1998/07/12 Use Visibility Enum
  // A Damaged Stealth Creature becomes more visible
  if (target->flags & MF_STEALTH)
    target->deltainvis = VISIBLE;

  if (target->health <= 0)
    return;

  if (target->flags & MF_SKULLFLY)
  {
    target->momx = target->momy = target->momz = 0;
    target->flags &= ~MF_SKULLFLY;
  }
	
  player = target->player;

  if (player && gameskill == sk_baby)
    damage >>= 1;  // take half damage in trainer mode
		

  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.
  if (inflictor && !(target->flags & MF_NOCLIP)
	&& (!source || !source->player || !weaponinfo[source->player->readyweapon].nothrust))
  {
    angle = R_PointToAngle2 (inflictor->x, inflictor->y, target->x, target->y);
    thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

    // make fall forwards sometimes
    if (damage < 40 && damage > target->health
         && target->z - inflictor->z > 64*FRACUNIT
          && (P_Random ()&1) )
    {
      angle += ANG180;
      thrust *= 4;
    }
		
    angle >>= ANGLETOFINESHIFT;

    target->momx += FixedMul (thrust, finecosine[angle]);
    target->momy += FixedMul (thrust, finesine[angle]);
  }
    
  // player specific
  if (player)
  {
     // Below certain threshold, ignore damage in GOD mode, or with INVUL power.
    if (damage < 1000 &&
        ((player->cheats&CF_GODMODE) || player->powers[pw_invulnerability]))
    {
      return;
    }
	
    if (player->armortype)
    {
      if (player->armortype == GREENARMOUR)
        saved = damage/3;
      else
        saved = damage/2;
	    
      if (player->armorpoints <= saved)
      {
 	// armor is used up
	saved = player->armorpoints;
        player->armortype = 0;
      }

      player->armorpoints -= saved;
      damage -= saved;
    }

    player->health -= damage; 	// mirror mobj health here for Dave

    if (player->health < 0)
      player->health = 0;
	
    player->attacker = source;

    if (damage>0)
      player->damagecount += damage;	// add damage after armor / invuln

    if (player->damagecount > 100)
      player->damagecount = 100;	// teleport stomp does 10k points...
	
    temp = damage < 100 ? damage : 100;

    if (player == &players[consoleplayer])
      I_Tactile (40,10,40+temp*2);
  }
    
  // do the damage
  target->health -= damage;

  if (target->health <= 0)
  {
    P_KillMobj (source, target);
    return;
  }

  if ((P_Random() < target->info->painchance) && !(target->flags&MF_SKULLFLY))
  {
    target->flags |= MF_JUSTHIT; // setup to hit back
    P_SetMobjState (target, target->info->painstate);
  }
			
  target->reactiontime = 0;		// we're awake now...

  if ((!target->threshold || target->extendedflags & EF_NOGRUDGE) &&
       source && source != target && (!(source->extendedflags & EF_NEVERTARGET)))
  {
    // if not intent on another player,
    // chase after this one
    target->target = source;
    target->threshold = BASETHRESHOLD;

    if (target->state == &states[target->info->spawnstate]
         && target->info->seestate != S_NULL)
    {
      P_SetMobjState (target, target->info->seestate);
    }
  }
			
}

