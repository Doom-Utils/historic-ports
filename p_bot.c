//
// DOSDoom Play Simulation Action routines: 'DeathBots'
//
// By The DOSDoom Team
//
#include "d_debug.h"
#include "dm_defs.h"
#include "dm_state.h"
#include "g_game.h"
#include "i_system.h"
#include "m_random.h"
#include "lu_sound.h"
#include "p_local.h"
#include "p_pspr.h"
#include "p_bot.h"
#include "r_state.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "p_action.h"
#include "rad_trig.h"

void BOT_Reborn(mobj_t* bot)
{
    player_t*	p;
    boolean* w;
    int		i; 
    int		frags;
    int		killcount;
    int		itemcount;
    int		secretcount;
    int*        ammo, *ammolimit;

    weaponupdate = true;
	 
    p = bot->player;

    frags = p->frags;
    killcount = p->killcount;
    itemcount = p->itemcount;
    secretcount = p->secretcount;
	 
    w = p->weaponowned;
    ammo = p->ammo;
    ammolimit = p->maxammo;
    memset (bot->player, 0, sizeof(bot_t));
    for (i = 0; i < numweapons; i++)
       w[i] = weaponinfo[i].autogive;
    p->weaponowned = w;
    p->ammo = ammo;
    p->maxammo = ammolimit;

    memset(ammo, 0, NUMAMMO*sizeof(int));
 
    p->frags = frags;
    p->killcount = killcount;
    p->itemcount = itemcount;
    p->secretcount = secretcount;
 
    p->playerstate = PST_LIVE;       
    p->health = NORMHEALTH;
    p->readyweapon = p->pendingweapon = DDF_WeaponGetType("PISTOL");
    p->ammo[am_clip] = NORMAMMO;
	 
    for (i=0 ; i<4 ; i++)
	p->maxammo[i] = maxammo[i];
    for (; i<NUMAMMO; i++)
        p->maxammo[i] = 0;
    p->mo = bot;
    p->mo->flags |= MF_PICKUP;
}

void BOT_NewChaseDir(mobj_t* bot)
{
  P_NewChaseDir(bot);
  bot->angle = bot->movedir << 29;
  bot->movecount <<= 2;
}

int BOT_EvaluateWeapon(mobj_t* mo, int w)
{
  weaponinfo_t* weapon = &weaponinfo[w];
  attacktype_t* attack = weapon->attack;
  int value = 0;

  // Don't have this weapon
  if (!mo->player->weaponowned[w])
    return INT_MIN;

  // Don't have enough ammo
  if (weapon->ammo != am_noammo)
  {
    if (mo->player->ammo[weapon->ammo] < weapon->ammopershot)
      return INT_MIN;
  }

  value = 64*(attack->damagerange * attack->damagemulti + attack->damage);

  switch (attack->attackstyle)
  {
     case  ATK_CLOSECOMBAT:
       if (mo->player->powers[pw_strength])
         value += 1024;
       else
         value /= 2;
       break;
     case ATK_SMARTPROJECTILE:
       value *= 2;
     case ATK_PROJECTILE:
       value += 256*(attack->projectile->damagerange * attack->projectile->damagemulti + attack->projectile->damage);
       value *= attack->projectile->speed;
       value /= 20;
       break;
     case ATK_SPAWNER:
       value *= 2;
       break;
     case ATK_TRIPLESPAWNER:
       value *= 6;
       break;
     case ATK_SHOT:
       value *= attack->count;
       break;

  }
  value = FixedMul(attack->accuracy+32768, value<<FRACBITS)>>FRACBITS;
  value -= weapon->ammopershot*8;
  if (w == mo->player->readyweapon)
    value += 1024;
  value += (P_Random()-128)*16;
  return value;
}

void P_BotSpawn(mobj_t* mo)
{
  if (!mo->player)
  {
    mo->player = Z_Malloc(sizeof(bot_t), PU_LEVEL, &mo->player);
    memset(mo->player, 0, sizeof(bot_t));
    mo->player->weaponowned =
       Z_Malloc(numweapons * sizeof(boolean), PU_LEVEL,&mo->player->weaponowned);
    mo->player->ammo =
       Z_Malloc(NUMAMMO * sizeof(int), PU_LEVEL, &mo->player->ammo);
    mo->player->maxammo = Z_Malloc(NUMAMMO * sizeof(int), PU_LEVEL, &mo->player->maxammo);
    BOT_Reborn(mo);
    P_SetupPsprites (mo->player);
  }
}

void P_BotRespawn(mobj_t* mo)
{
  int             i,j;
  int                         selections;
  mobj_t*                     bot;

  selections = deathmatch_p - deathmatchstarts;
  if (selections < 4)
      I_Error ("Only %i deathmatch spots, 4 required", selections);

  for (j=0 ; j<selections ; j++)
  {
      i = P_Random() % selections;
      bot = P_MobjCreateObject(deathmatchstarts[i].x<<FRACBITS,
                               deathmatchstarts[i].y<<FRACBITS,
                               ONFLOORZ,
                               mo->info);
      if (P_CheckPosition(bot, bot->x, bot->y))
      {
        P_MobjCreateObject(bot->x, bot->y, bot->z, mo->info->respawneffect);
        bot->player = mo->player;
        bot->spawnpoint = mo->spawnpoint;
        bot->playxtra = mo->playxtra;
        bot->side = mo->side;
        mo->player = NULL;
        BOT_Reborn(bot);
        P_SetupPsprites(bot->player);
        P_MobjCreateObject (bot->x,bot->y,
                                  bot->subsector->sector->floorheight,
                                   bot->info->respawneffect);
        P_MobjCreateObject (mo->x, mo->y, mo->subsector->sector->floorheight,
                            bot->info->respawneffect);
        P_RemoveMobj(mo);
        return;
      }
      P_RemoveMobj(bot);

  }
}

extern mobj_t* shootthing;
extern int attackrange;
static boolean PTR_BotLook (intercept_t* in)
{
    line_t*		li;
    mobj_t*		th;
    fixed_t		dist;
    int                 ammotype;
		
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
            if (abs(li->frontsector->floorheight - li->backsector->floorheight) > 24*FRACUNIT)
              return false;
	}
		
	return true;			// shot continues
    }
    
    // shoot a thing
    th = in->d.thing;

    if (th == shootthing)
	return true;			// can't shoot self
    
    if (!(th->flags&MF_SPECIAL))
	return true;			// has to be able to be shot

    ammotype = th->info->benefitammo;
    switch (th->info->benefittype)
    {
      // -KM- 1998/11/25 New weapon handling
      case WEAPON:           // WEAPON
        if (!shootthing->player->weaponowned[th->info->benefitweapon])
          break;
        ammotype = weaponinfo[th->info->benefitweapon].ammo;
      case AMMO_TYPE:
        if (shootthing->player->ammo[ammotype] ==
            shootthing->player->maxammo[ammotype])
          return true;
        break;
      case KEY_BLUECARD:
        if (shootthing->player->cards[it_bluecard])
          return true;
        break;
      case KEY_REDCARD:
        if (shootthing->player->cards[it_redcard])
          return true;
        break;
      case KEY_YELLOWCARD:
        if (shootthing->player->cards[it_yellowcard])
          return true;
        break;
      case KEY_BLUESKULL:
        if (shootthing->player->cards[it_blueskull])
          return true;
        break;
      case KEY_REDSKULL:
        if (shootthing->player->cards[it_redskull])
          return true;
        break;
      case KEY_YELLOWSKULL:
        if (shootthing->player->cards[it_yellowskull])
          return true;
        break;
      case POWERUP_ACIDSUIT:
        if (shootthing->player->powers[pw_ironfeet] >= th->info->benefitamount*TICRATE/3)
          return true;
        break;
      case POWERUP_ARMOUR:
        if (shootthing->player->armorpoints >= th->info->limit)
          return true;
        break;
      case POWERUP_AUTOMAP:
        return true;
      case POWERUP_BACKPACK:
        break;
      case POWERUP_BERSERK:
        if (shootthing->player->powers[pw_strength])
          return true;
        break;
  
      case POWERUP_HEALTH:
        if (shootthing->health >= th->info->limit)
          return true;
        break;
  
      case POWERUP_HEALTHARMOUR:
        if ((shootthing->health >= th->info->limit) &&
            (shootthing->player->armorpoints >= th->info->limit))
          return true;
        break;
  
      case POWERUP_INVULNERABLE:
        break;
  
      case POWERUP_JETPACK:
        if (shootthing->player->powers[pw_jetpack] >= th->info->benefitamount*TICRATE/3)
          return true;
        break;
  
      case POWERUP_LIGHTGOGGLES:
        if (shootthing->player->powers[pw_infrared] >= th->info->benefitamount*TICRATE/3)
          return true;
        break;
  
      case POWERUP_NIGHTVISION:
        if (shootthing->player->powers[pw_nightvision] >= th->info->benefitamount*TICRATE/3)
          return true;
        break;
  
      case POWERUP_PARTINVIS:
        if (shootthing->player->powers[pw_invisibility] >= th->info->benefitamount*TICRATE/3)
          return true;
        break;

    }

    linetarget = th;

    return false;			// don't go any farther
}

// Finds items for the bot to get.
mobj_t* BOT_LookForStuff( mobj_t* bot, angle_t angle )
{
  fixed_t x2;
  fixed_t y2;
	
  angle >>= ANGLETOFINESHIFT;
  shootthing = bot;
    
  x2 = bot->x + 1024*finecosine[angle];
  y2 = bot->y + 1024*finesine[angle];

  attackrange = 1024<<16;
  linetarget = NULL;
	
  P_PathTraverse (bot->x, bot->y, x2, y2, PT_ADDLINES|PT_ADDTHINGS, PTR_BotLook);
		
  return linetarget;
}

void BOT_Move (mobj_t*	actor, angle_t angle)
{
    actor->momx = FixedMul(actor->speed, finecosine[angle>>ANGLETOFINESHIFT]);
    actor->momy = FixedMul(actor->speed, finesine[angle>>ANGLETOFINESHIFT]);
}

void P_BotThink( mobj_t* bot)
{
  bot_t* botthink = (bot_t*) bot->player;
  boolean move_ok = P_AproxDistance(bot->momx, bot->momy) > STOPSPEED;
  boolean seetarget = false;
  int best = bot->player->readyweapon;
  int best_val = INT_MIN;
  int i, j;

  if (!bot->player)
    return;

  // Update PSprites
  P_MovePsprites(bot->player);

  // Check we can see the target
  if (bot->target)
    seetarget = P_CheckSight(bot, bot->target);

  bot->threshold--;
  bot->movecount--;

  // Select a suitable weapon
  if ((bot->threshold < 0 || bot->movecount < 0) && !(bot->player->cmd.buttons & BT_ATTACK))
  {
    for (i = 0; i < numweapons; i++)
    {
      j = BOT_EvaluateWeapon(bot, i);
      if (j > best_val)
      {
        best_val = j;
        best = i;
      }
    }
  
    if (best != bot->player->readyweapon)
      bot->player->pendingweapon = best;
  }
  // Look for enemies
  if (!seetarget)
  {
    if (bot->threshold < 0)
    {
      P_ActLookForTargets(bot);
      if (bot->target)
        bot->threshold = P_Random()&31;
    }
  }

  // Can't see a target || don't have a suitable weapon to take it out with?
  if (!seetarget)
  {
    mobj_t* newtarget = NULL;

    if (bot->threshold < 0)
    {
      // Find some stuff!
      for (i = (ANG180/(5*TICRATE))*(leveltime%TICRATE); i < ANG180; i += ANG180/5)
      {
         newtarget = BOT_LookForStuff(bot, bot->angle + i);
         if (!newtarget)
           newtarget = BOT_LookForStuff(bot, bot->angle - i);
         if (newtarget)
         {
           bot->target = newtarget;
           bot->threshold = P_Random()&31;
           seetarget = true;
           break;
         }
      }
    }
  }

  // Look up/down
  if (bot->target)
    bot->vertangle = FixedDiv(bot->target->z - bot->z,
        P_AproxDistance(bot->x - bot->target->x, bot->y - bot->target->y));
  else
    bot->vertangle = 0;

  // Special sector fx
  if (bot->subsector->sector->special)
    P_PlayerInSpecialSector (bot->player);

  // BOTS act radius triggers
  RAD_DoRadiTrigger(bot->player);

  // Strength counts up to diminish fade.
  // Countdown powers
  if (bot->player->powers[pw_strength])
    bot->player->powers[pw_strength]++;

  if (bot->player->powers[pw_jetpack])
    bot->player->powers[pw_jetpack]--;

  if (bot->player->powers[pw_nightvision])
    bot->player->powers[pw_nightvision]--;

  if (bot->player->powers[pw_invulnerability])
    bot->player->powers[pw_invulnerability]--;

  if (bot->player->powers[pw_invisibility])
    if (! --bot->player->powers[pw_invisibility] )
       bot->flags &= ~MF_SHADOW;
	
  if (bot->player->powers[pw_infrared])
    bot->player->powers[pw_infrared]--;

  if (bot->player->powers[pw_ironfeet])
    bot->player->powers[pw_ironfeet]--;

  if (bot->player->damagecount)
    bot->player->damagecount--;

  if (bot->player->bonuscount)
    bot->player->bonuscount--;

  if (bot->reactiontime>0)
  {
    bot->reactiontime--;
    return;
  }

  // Remove attack flag.
  bot->player->cmd.buttons &= ~BT_ATTACK;
  // Got a special (item) target?
  if (bot->target && (bot->target->flags & MF_SPECIAL))
  {
    if ((bot->flags & MF_JUSTPICKEDUP))
    {
      // Just got the item.
      bot->flags &= ~MF_JUSTPICKEDUP;
      bot->target = NULL;
    } else if (bot->movecount < 0) {
      // Move in the direction of the item.
      bot->angle = R_PointToAngle2(bot->x, bot->y, bot->target->x, bot->target->y);
      bot->movecount = (P_Random()&15)*16;
#ifdef DEVELOPERS
      // Debug: Item seekers are GREEN
      bot->playxtra = 0;
#endif
    }
    // If there is a wall or something in the way, pick a new direction.
    if (!move_ok)
      BOT_NewChaseDir(bot);
    // Move the bot.
    BOT_Move (bot, bot->angle);
    return;
  }

  // Target can be killed.
  if (bot->target && (bot->target->flags & MF_SHOOTABLE))
  {
#ifdef DEVELOPERS
    // DEBUG: Show what weapon bot is using
    bot->playxtra = bot->player->readyweapon;
#endif
    // Can see a target,
    if (seetarget)
    {
      // So face it,
      P_ActFaceTarget(bot);
      // Shoot it,
      bot->player->cmd.buttons |= BT_ATTACK;
      // strafe it.
      BOT_Move (bot, bot->angle + (botthink->strafedir?ANG90:-ANG90));
      if (bot->movecount < 0)
      {
        bot->movecount = P_Random()&31;
        botthink->strafedir = P_Random()&1;
      }
    } else
      BOT_Move (bot, bot->angle);

    // chase towards player
    if (bot->movecount<0 || !move_ok)
    {
      BOT_NewChaseDir (bot);
      if (!move_ok)
        botthink->strafedir = P_Random()&1;
    }
    return;
  }

  // Wander around.
  BOT_Move(bot, bot->angle);
  if (!move_ok || (bot->target && bot->target->health <= 0)
      || bot->movecount<0)
  {
    bot->target = NULL;
    BOT_NewChaseDir(bot);
    botthink->strafedir = P_Random()&1;
    bot->movecount <<= 2;
#ifdef DEVELOPERS
    bot->playxtra = 15;
#endif
  }

}


void BOT_DMSpawn(void)
{
    int             i,j;
    int                         selections;
    mobj_t*                     bot;
    mobjinfo_t*                 info;

    selections = deathmatch_p - deathmatchstarts;
    if (selections < 4)
        I_Error ("Only %i deathmatch spots, 4 required", selections);

    info = mobjinfohead;

    // -ACB- 1998/08/06 use linked list table
    while (info && (info->playernum != -2))
      info = info->next;

    // MOBJTYPE not found, don't crash out: JDS Compliance.
    // -ACB- 1998/07/21
    if (!info)
    {
	Debug_Printf ("No DEATHBOTS defined\n");
        return;
    }

    for (j=0 ; j<selections ; j++)
    {
        i = P_Random() % selections;
        bot = P_MobjCreateObject(deathmatchstarts[i].x<<FRACBITS,
                                 deathmatchstarts[i].y<<FRACBITS,
                                 ONFLOORZ,
                                 info);
        if (P_CheckPosition(bot, bot->x, bot->y))
        {
          P_MobjCreateObject(bot->x, bot->y, bot->z, info->respawneffect);
          bot->playxtra = P_Random() % 15;
          bot->side = 1<<(P_Random()&31);
          return;
        }
        P_RemoveMobj(bot);

    }
}
