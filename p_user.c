//  
// DOSDoom Player User Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
#include "d_event.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "i_system.h"
#include "rad_trig.h"
#include "p_local.h"
#include "s_sound.h"
#include "w_wad.h"

#define INVERSECOLORMAP         32
#define NIGHTVISIONCOLORMAP     33 // -ACB- 1998/07/15 NightVision Code

// 16 pixels of bob
#define MAXBOB	      0x100000
#define JUMPHEIGHT    player->mo->info->jumpheight
#define JUMPBREATHER  1*TICRATE   // -ACB- 1998/08/09 Gap before next jump

boolean		onground;

//
// P_Thrust
// Moves the given origin along a given angle.
//
void P_Thrust (player_t* player, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
    
    player->mo->momx += FixedMul(move,finecosine[angle]); 
    player->mo->momy += FixedMul(move,finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t* player) 
{
    int		angle;
    fixed_t	bob;

    // Check for cheat.
    if ((player->cheats & CF_NOMOMENTUM) || !onground)
    {
	player->viewz = player->mo->z + VIEWHEIGHT;

	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
	    player->viewz = player->mo->ceilingz-4*FRACUNIT;

	player->viewz = player->mo->z + player->viewheight;
	return;
    }

    //
    // Sorted this, bob and viewheight are only calculated when
    // player is alive. -ACB- 1998/07/27
    //
    if (player->playerstate == PST_LIVE)
    {
        // ----CALCULATE BOB EFFECT----
        angle = (FINEANGLES/20*leveltime)&FINEMASK;

        // Regular movement bobbing (needs to be calculated for gun swing even
        // if not on ground) OPTIMIZE: tablify angle; Note: a LUT allows for
        // effects like a ramp with low health.
        player->bob =
	   FixedMul (player->mo->momx, player->mo->momx)
	    + FixedMul (player->mo->momy,player->mo->momy);
    
        player->bob >>= 2;

        if (player->bob>MAXBOB)
	  player->bob = MAXBOB;

        bob = FixedMul ( player->bob/2, finesine[angle]);

        // ----CALCULATE VIEWHEIGHT----
	player->viewheight += player->deltaviewheight;

	if (player->viewheight > VIEWHEIGHT)
	{
	    player->viewheight = VIEWHEIGHT;
	    player->deltaviewheight = 0;
	}

	if (player->viewheight < VIEWHEIGHT/2)
	{
	    player->viewheight = VIEWHEIGHT/2;
	    if (player->deltaviewheight <= 0)
		player->deltaviewheight = 1;
	}
	
	if (player->deltaviewheight)	
	{
	    player->deltaviewheight += FRACUNIT/4;
	    if (!player->deltaviewheight)
		player->deltaviewheight = 1;
	}

        player->viewz = player->mo->z + player->viewheight + bob;

    }
    else
    {
        // If one is dead, one looks from the floor with no effects.
        player->viewz = player->mo->z + player->viewheight;
    }

    // No heads above the ceiling
    if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
	  player->viewz = player->mo->ceilingz-4*FRACUNIT;

}



//
// P_MovePlayer
//
void P_MovePlayer (player_t* player)
{
    ticcmd_t*		cmd;
	
    cmd = &player->cmd;
	
    player->mo->angle += FixedMul(cmd->angleturn<<16, player->mo->speed);

    // Do not let the player control movement if not onground.
    // -MH- 1998/06/18  unless he has the JetPack!
    onground = (player->mo->z <= player->mo->floorz);
    if (player->powers[pw_jetpack])
      {
       if (player->powers[pw_jetpack]<=(5*TICRATE))
       {
         if (!(leveltime & 10))
           S_StartSound(player->mo,sfx_jpflow); 	// fuel low
       } else {
          if (cmd->upwardmove>0)
            S_StartSound(player->mo,sfx_jprise);
          else
            {
             if (cmd->upwardmove<0)
               S_StartSound(player->mo,sfx_jpdown);
             else
               {
                if (cmd->forwardmove || cmd->sidemove)
                   S_StartSound(player->mo,(onground?sfx_jpidle:sfx_jpmove));
                else
                   S_StartSound(player->mo,sfx_jpidle);
               }
            }
         }
      }

    // -MH- 1998/08/18  do vertical move
    if (cmd->upwardmove)
    {
       if (player->powers[pw_jetpack])
         player->mo->momz += FixedMul(cmd->upwardmove*2048,player->mo->speed);
       else
         player->mo->momz += FixedMul(player->mo->subsector->sector->viscosity,
                                      FixedMul(cmd->upwardmove*2048,player->mo->speed));
    }

    // -MH- 1998/06/18  ...or has the JetPack
    if (cmd->forwardmove)
    {
      if (onground || player->powers[pw_jetpack])
        P_Thrust (player, player->mo->angle, FixedMul(cmd->forwardmove*2048,player->mo->speed));
      else
        P_Thrust (player, player->mo->angle,
                  FixedMul(player->mo->subsector->sector->viscosity, FixedMul(cmd->forwardmove*2048,player->mo->speed)));
    }

    // -MH- 1998/06/18  ...or has the JetPack
    if (cmd->sidemove)
    {
      if (onground || player->powers[pw_jetpack])
	P_Thrust (player, player->mo->angle-ANG90, FixedMul(cmd->sidemove*2048, player->mo->speed));
      else
	P_Thrust (player, player->mo->angle-ANG90,
                  FixedMul(player->mo->subsector->sector->viscosity, FixedMul(cmd->sidemove*2048, player->mo->speed)));
    }

    if ( (cmd->forwardmove || cmd->sidemove) 
	 && player->mo->state == &states[player->mo->info->spawnstate] )
    {
	P_SetMobjState (player->mo, player->mo->info->seestate);
    }
}	

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5   	(ANG90/18)

void P_DeathThink (player_t* player)
{
    angle_t		angle;
    angle_t		delta;

    P_MovePsprites (player);
	
    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
	player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
	player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;
    onground = (player->mo->z <= player->mo->floorz);
    P_CalcHeight (player);
	
    if (player->attacker && player->attacker != player->mo)
    {
	angle = R_PointToAngle2 (player->mo->x,
				 player->mo->y,
				 player->attacker->x,
				 player->attacker->y);
	
	delta = angle - player->mo->angle;
	
	if (delta < ANG5 || delta > (unsigned)-ANG5)
	{
	    // Looking at killer,
	    //  so fade damage flash down.
	    player->mo->angle = angle;

	    if (player->damagecount)
		player->damagecount--;
	}
	else if (delta < ANG180)
	    player->mo->angle += ANG5;
	else
	    player->mo->angle -= ANG5;
    }
    else if (player->damagecount)
	player->damagecount--;
	
    if (deathmatch >= 3 && player->mo->movecount < player->mo->info->respawntime)
      return;

    if (player->cmd.buttons & BT_USE)
	player->playerstate = PST_REBORN;
}



//
// P_PlayerThink
//
void P_PlayerThink (player_t* player)
{
    ticcmd_t*		cmd;
    int	key;
	
    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
	player->mo->flags |= MF_NOCLIP;
    else
	player->mo->flags &= ~MF_NOCLIP;
    
    // chain saw run forward
    cmd = &player->cmd;
    if (player->mo->flags & MF_JUSTATTACKED)
    {
	cmd->angleturn = 0;
	cmd->forwardmove = 0xc800/512;
	cmd->sidemove = 0;
	player->mo->flags &= ~MF_JUSTATTACKED;
    }
				
    if (player->playerstate == PST_DEAD)
    {
	P_DeathThink (player);
	return;
    }
    
    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (player->mo->reactiontime)
	player->mo->reactiontime--;
    else
	P_MovePlayer (player);
    
    P_CalcHeight (player);

    if (player->mo->subsector->sector->special)
    	P_PlayerInSpecialSector (player);

    RAD_DoRadiTrigger(player);

    // Check for weapon change.

    // A special event has no other buttons.
    if (cmd->buttons & BT_SPECIAL)
	cmd->buttons = 0;			
		
    if (cmd->buttons & BT_CHANGE)
    {
        int newweapon = wp_none;
        int i, j;
        weaponkey_t* wk;
	// The actual changing of the weapon is done
	//  when the weapon psprite can do it
	//  (read: not in the middle of an attack).
	key = (cmd->buttons&BT_WEAPONMASK)>>BT_WEAPONSHIFT;
        wk = &weaponkey[key];

        // -KM- 1998/11/25 Make choice based on weapons.ddf
        for ( i = j = wk->choiceon; i < (j + wk->numchoices); i++)
        {
           if (wk->choice[i%wk->numchoices] == -1)
             continue;
           if (player->weaponowned[wk->choice[i%wk->numchoices]]
               && wk->choice[i%wk->numchoices] != player->readyweapon)
           {
             // -KM- 1998/12/16 Added check to make sure sprites exist.
             char wSprite[9];
             sprintf(wSprite, "%sA0", sprnames[states[weaponinfo[wk->choice[i%wk->numchoices]].upstate].sprite]);
             if (W_CheckNumForName(wSprite) != -1)
             {
               player->pendingweapon = wk->choice[i%wk->numchoices];
               wk->choiceon = i%wk->numchoices;
               break;
             } else
               player->weaponowned[i%wk->numchoices] = false;
           }
        }
        if (newweapon != player->readyweapon
            && newweapon != wp_none)
          player->pendingweapon = newweapon;
    }
    
    // check for use
    if (cmd->buttons & BT_USE)
    {
	if (!player->usedown)
	{
	    P_UseLines (player);
	    player->usedown = true;
	}
    }
    else
	player->usedown = false;

    // DOSDoom Feature: Jump Code
    //
    // -ACB- 1998/08/09 Check that jumping is allowed in the currentmap
    //                  Make player pause before jumping again
    //
    if (cmd->extbuttons & EBT_JUMP)
    {
      if ((!(currentmap->flags & MPF_NOJUMPING)) && (!player->jumpwait))
      {
        if (player->mo->z <= player->mo->floorz)
        {
          player->mo->momz += JUMPHEIGHT;
          player->jumpwait = JUMPBREATHER;
        }
      }
    }

    // DOSDoom Feature: Vertical Look (Mlook)
    //
    // -ACB- 1998/07/02 New Code used, rerouted via Ticcmd
    // -ACB- 1998/07/27 Used defines for look limits.
    //
    if (gameflags.freelook && (cmd->extbuttons & EBT_VERTLOOK))
    {
      player->deltaviewz += (cmd->vertangle*256);

      if (player->deltaviewz > LOOKUPLIMIT)
        player->deltaviewz = LOOKUPLIMIT;

      if (player->deltaviewz < LOOKDOWNLIMIT)
        player->deltaviewz = LOOKDOWNLIMIT;
    }

    // DOSDoom Feature: Vertical Look (Mlook)
    //
    // -ACB- 1998/07/02 Re-routed via Ticcmd
    //
    if (cmd->extbuttons & EBT_CENTER)
        player->deltaviewz = 0;

    player->mo->vertangle = (player->deltaviewz + 2*player->mo->vertangle) / 3;

    // decrement jumpwait counter
    if (player->jumpwait)
      player->jumpwait--;

    // cycle psprites
    P_MovePsprites (player);
    
    // Counters, time dependend power ups.

    // Strength counts up to diminish fade.
    if (player->powers[pw_strength])
      player->powers[pw_strength]++;
		
    // -MH- 1998/06/18  jetpack "fuel" counter
    if (player->powers[pw_jetpack])
      player->powers[pw_jetpack]--;

    // -ACB- 1998/07/16  nightvision counter decrementation
    if (player->powers[pw_nightvision])
      player->powers[pw_nightvision]--;

    if (player->powers[pw_invulnerability])
      player->powers[pw_invulnerability]--;

    if (player->powers[pw_invisibility])
	if (! --player->powers[pw_invisibility] )
	    player->mo->flags &= ~MF_SHADOW;
			
    if (player->powers[pw_infrared])
      player->powers[pw_infrared]--;
		
    if (player->powers[pw_ironfeet])
      player->powers[pw_ironfeet]--;
		
    if (player->damagecount)
      player->damagecount--;
		
    if (player->bonuscount)
      player->bonuscount--;

    
    // Handling colormaps.
    if (player->powers[pw_invulnerability])
    {
	if (player->powers[pw_invulnerability]>128 ||
           (player->powers[pw_invulnerability]&8) )
	    player->fixedcolormap = INVERSECOLORMAP;
	else
	    player->fixedcolormap = 0;
    }
    else if (player->powers[pw_infrared])	
    {
	if (player->powers[pw_infrared]>128 || (player->powers[pw_infrared]&8))
	  player->fixedcolormap = 1;
	else
	  player->fixedcolormap = 0;
    }
    else if (player->powers[pw_nightvision]) // -ACB- 1998/07/15 NightVision Code
    {
	if (player->powers[pw_nightvision]>128 || (player->powers[pw_nightvision]&8))
	  player->fixedcolormap = NIGHTVISIONCOLORMAP;
	else
	  player->fixedcolormap = 0;
    }
    else
    {
	player->fixedcolormap = 0;
    }
}


