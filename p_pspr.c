
//  
// DOSDoom Weapon (player sprites) Action Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/11/25 Added/Changed stuff for weapons.ddf
//
#include "d_debug.h"
#include "d_event.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "lu_sound.h"
#include "m_random.h"
#include "p_local.h"
#include "p_pspr.h"
#include "s_sound.h"

//
// P_SetPsprite
//
void P_SetPsprite (player_t* player, int position, int stnum)
{
    pspdef_t*	psp;
    state_t*	state;
	
    psp = &player->psprites[position];
	
    do
    {
	if (!stnum)
	{
	    // object removed itself
	    psp->state = NULL;
	    break;
	}
	
	state = &states[stnum];
	psp->state = state;
	psp->tics = state->tics;	// could be 0

	if (state->misc1)
	{
	    // coordinate set
	    psp->sx = state->misc1 << FRACBITS;
	    psp->sy = state->misc2 << FRACBITS;
	}
	
	// Call action routine.
	// Modified handling.
	if (state->action.acp2)
	{
	    state->action.acp2(player, psp);
	    if (!psp->state)
		break;
	}
	
	stnum = psp->state->nextstate;
	
    } while (!psp->tics);
    // an initial state of 0 could cycle through
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon (player_t* player)
{
    statenum_t	newstate;
	
    if (player->pendingweapon == wp_nochange)
      player->pendingweapon = player->readyweapon;
		
    if (weaponinfo[player->pendingweapon].start)
      S_StartSound (player->mo, weaponinfo[player->pendingweapon].start);
		
    newstate = weaponinfo[player->pendingweapon].upstate;

    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;

    P_SetPsprite (player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
boolean P_CheckAmmo (player_t* player)
{
    ammotype_t		ammo;
    int			count;
    int                 priority;
    int                 i;

    ammo = weaponinfo[player->readyweapon].ammo;

    // Minimal amount for one shot varies.
    count = weaponinfo[player->readyweapon].ammopershot;

    // Some do not need ammunition anyway.
    // Return if current ammunition sufficient.
    if (ammo == am_noammo || player->ammo[ammo] >= count)
	return true;
		
    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.
    priority = 0;
    for (i = 0; i < numweapons; i++)
    {
        if (weaponinfo[i].priority >= priority &&
            player->weaponowned[i] &&
            (weaponinfo[i].ammo == am_noammo ||
            player->ammo[weaponinfo[i].ammo] >= weaponinfo[i].ammopershot))
        {
            player->pendingweapon = i;
            priority = weaponinfo[i].priority;
        }
    }

    // Now set appropriate weapon overlay.
    P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].downstate);
    return false;	
}


//
// P_FireWeapon.
//
void P_FireWeapon (player_t* player)
{
    statenum_t	newstate;
	
    if (!P_CheckAmmo (player))
	return;

    player->flash = false;
    P_SetMobjState (player->mo, player->mo->info->missilestate);
    newstate = weaponinfo[player->readyweapon].atkstate;
    P_SetPsprite (player, ps_weapon, newstate);
    P_NoiseAlert (player->mo, player->mo);
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon (player_t* player)
{
    P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void A_WeaponReady (player_t* player, pspdef_t* psp)
{	
    statenum_t	newstate;
    int		angle;
    weaponinfo_t* w = &weaponinfo[player->readyweapon];
    
    // get out of attack state
    if (player->mo->state == &states[player->mo->info->missilestate]
         || player->mo->state == &states[player->mo->info->missilestate+1] )
    {
	P_SetMobjState (player->mo, player->mo->info->spawnstate);
    }

    if (w->idle && psp->state == &states[w->readystate])
         S_StartSound (player->mo, w->idle);

    // check for change if player is dead, put the weapon away
    if (player->pendingweapon != wp_nochange || !player->health)
    {
	// change weapon (pending weapon should already be validated)
	newstate = weaponinfo[player->readyweapon].downstate;
	P_SetPsprite (player, ps_weapon, newstate);
	return;	
    }
    
    // check for fire: the missile launcher and bfg do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
        if(w->autofire || (!player->attackdown && !w->autofire))
        {
	    player->attackdown = true;
	    P_FireWeapon (player);		
	    return;
	}
    }
    else
    {
	player->attackdown = false;
    }

    // bob the weapon based on movement speed
    angle = (128*leveltime)&FINEMASK;
    psp->sx = FRACUNIT + FixedMul (player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul (player->bob, finesine[angle]);
}



//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire (player_t* player, pspdef_t* psp)
{
    weaponinfo_t* w = &weaponinfo[player->readyweapon];
    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if (player->cmd.buttons & BT_ATTACK)
    {
      // -KM- 1999/01/31 Check for semiautomatic weapons.
      if ( player->pendingweapon == wp_nochange
  	 && player->health && (w->autofire || (!player->attackdown && !w->autofire)))
      {
  	player->refire++;
  	P_FireWeapon (player);
      }
      else
      {
  	player->refire = 0;
  	P_CheckAmmo (player);
      }
    } else {
      player->refire = 0;
      P_CheckAmmo (player);
    }
}


// -KM- 1999/01/31 Check clip size.
void A_CheckReload (player_t* player, pspdef_t* psp)
{
    P_CheckAmmo (player);
    if (weaponinfo[player->readyweapon].reloadstate && player->pendingweapon == wp_nochange)
    {
      weaponinfo_t* w = &weaponinfo[player->readyweapon];

      if (!((player->ammo[w->ammo] / w->ammopershot) % w->clip))
        P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].reloadstate);
    }
}

//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//
void A_Lower (player_t* player, pspdef_t* psp)
{	
    psp->sy += LOWERSPEED;

    // Is already down.
    if (psp->sy < WEAPONBOTTOM)
	return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
	psp->sy = WEAPONBOTTOM;

	// don't bring weapon back up
	return;		
    }
    
    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->health)
    {
	// Player is dead, so keep the weapon off screen.
	P_SetPsprite (player,  ps_weapon, S_NULL);
	return;	
    }
	
    player->readyweapon = player->pendingweapon; 

    P_BringUpWeapon (player);
}


//
// A_Raise
//
void A_Raise (player_t* player, pspdef_t* psp)
{
    statenum_t	newstate;
	
    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP)
	return;
    
    psp->sy = WEAPONTOP;
    
    // The weapon has been raised all the way,
    //  so change to the ready state.
    newstate = weaponinfo[player->readyweapon].readystate;

    P_SetPsprite (player, ps_weapon, newstate);
}



//
// A_GunFlash
//
void A_GunFlash (player_t* player, pspdef_t* psp)
{
    if (!player->flash)
    {
      P_SetMobjState (player->mo, player->mo->info->missilestate + 1);
      P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);
      player->flash = true;
    }
}



//
// WEAPON ATTACKS
//

void A_FireWeapon(player_t* p, pspdef_t* psp)
{
  weaponinfo_t* w = &weaponinfo[p->readyweapon];
  attacktype_t* attack = w->attack;
  ammotype_t ammo;
  int count;

  ammo = weaponinfo[p->readyweapon].ammo;

  // Minimal amount for one shot varies.
  count = weaponinfo[p->readyweapon].ammopershot;

  // Some do not need ammunition anyway.
  // Return if current ammunition sufficient.
  if (!(ammo == am_noammo || p->ammo[ammo] >= count))
    return;

  P_ActPlayerAttack(p->mo, attack);
  p->deltaviewheight -= w->kick;
  p->mo->vertangle += w->kick;
  if (p->mo->target && !(p->mo->target->extendedflags & EF_DUMMYMOBJ))
  {
    if (w->hit)
      S_StartSound (p->mo, w->hit);
    if (w->feedback)
      p->mo->flags |= MF_JUSTATTACKED;
  }
  if (w->engaged)
  {
    if (!p->mo->target)
    {
      S_StartSound (p->mo, w->engaged);
      return;
    }
  
    if (p->mo->target->extendedflags & EF_DUMMYMOBJ)
    {
      S_StartSound (p->mo, w->engaged);
      return;
    }
  }

  P_SetMobjState (p->mo, p->mo->info->missilestate + 1);
  if (w->ammo != am_noammo)
    p->ammo[w->ammo]-=w->ammopershot;
  if (w->flashstate && !p->flash)
  {
    P_SetPsprite (p,ps_flash, w->flashstate);
    p->flash = true;
  }
}
void A_SFXWeapon1 (player_t* player, pspdef_t* psp)
{
    S_StartSound (player->mo, weaponinfo[player->readyweapon].sound1);
}

void A_SFXWeapon2 (player_t* player, pspdef_t* psp)
{
    S_StartSound (player->mo, weaponinfo[player->readyweapon].sound2);
}

void A_SFXWeapon3 (player_t* player, pspdef_t* psp)
{
    S_StartSound (player->mo, weaponinfo[player->readyweapon].sound3);
}

//
// ?
//
void A_Light0 (player_t *player, pspdef_t *psp)
{
    player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
    player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
    player->extralight = 2;
}

void A_RandomJump(player_t *player, pspdef_t *psp)
{
  if (P_Random() < psp->state->misc1)
  {
    P_SetPsprite(player, player->psprites - psp, psp->state - states + psp->state->misc2);
  }
}


//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites (player_t* player) 
{
    int	i;
	
    // remove all psprites
    for (i=0 ; i<NUMPSPRITES ; i++)
	player->psprites[i].state = NULL;
		
    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon (player);
}




//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites (player_t* player) 
{
    int		i;
    pspdef_t*	psp;
	
    psp = &player->psprites[0];
    for (i=0 ; i<NUMPSPRITES ; i++, psp++)
    {
	// a null state means not active
	if (psp->state)
	{
	    // drop tic count and possibly change state

	    // a -1 tic count never changes
	    if (psp->tics != -1)	
	    {
		psp->tics--;
		if (!psp->tics)
		    P_SetPsprite (player, i, psp->state->nextstate);
	    }				
	}
    }
    
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}


