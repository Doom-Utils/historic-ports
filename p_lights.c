//  
// DOSDoom Sector Lighting Code 
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/09/27 Lights generalised for ddf
//
#include "z_zone.h"
#include "m_random.h"

#include "dm_defs.h"
#include "dm_state.h"
#include "p_local.h"


// State.
#include "r_state.h"

//
// GENERALISED LIGHT
//
void T_Light(light_t* light)
{
  if (--light->count)
    return;

  // Flashing lights
  switch (light->type)
  {
    case lite_flash:
    {
      int i;
      i = P_Random();
      // Dark
      if (i <= light->probability)
      {
        light->sector->lightlevel = light->minlight;
        light->count = light->darktime;
      }
      else
      {
        light->sector->lightlevel = light->maxlight;
        light->count = light->brighttime;
      }
      break;
    }

    case lite_strobe:
      if (light->sector->lightlevel == light->maxlight)
      {
        // Go dark
        light->sector->lightlevel = light->minlight;
        light->count = light->darktime;
      }
      else
      {
        // Go Bright
        light->sector->lightlevel = light->maxlight;
        light->count = light->brighttime;
      }
      break;
  
    case lite_glow:
      if (light->direction == -1)
      {
        // Go dark
        light->sector->lightlevel -= GLOWSPEED;
        if (light->sector->lightlevel <= light->minlight)
        {
          light->sector->lightlevel = light->minlight;
          light->count = light->brighttime;
          light->direction = 1;
        }
        else
        {
          light->count = light->darktime;
        }
      }
      else
      {
        // Go Bright
        light->sector->lightlevel += GLOWSPEED;
        if (light->sector->lightlevel >= light->maxlight)
        {
          light->sector->lightlevel = light->maxlight;
          light->count = light->darktime;
          light->direction = -1;
        }
        else
        {
          light->count = light->brighttime;
        }
      }
      break;
  
    case lite_fireflicker:
    {
      // This unequal probability weights it in direction
      int i = ((P_Random()&63) - 18) * light->direction + light->sector->lightlevel;
      if (i <= light->minlight || i >= light->maxlight)
        light->direction = -light->direction;
  
      if (light->direction == -1)
        light->count = light->darktime;
      else
        light->count = light->brighttime;
    }
    default:
         break;

  }
}









//
// TURN LINE'S TAG LIGHTS OFF
//
/*
void EV_TurnTagLightsOff(line_t* line)
{
    int			i;
    int			j;
    int			min;
    sector_t*		sector;
    sector_t*		tsec;
    line_t*		templine;
	
    sector = sectors;
    
    for (j = 0;j < numsectors; j++, sector++)
    {
	if (sector->tag == line->tag)
	{
	    min = sector->lightlevel;
	    for (i = 0;i < sector->linecount; i++)
	    {
		templine = sector->lines[i];
		tsec = getNextSector(templine,sector);
		if (!tsec)
		    continue;
		if (tsec->lightlevel < min)
		    min = tsec->lightlevel;
	    }
	    sector->lightlevel = min;
	}
    }
}
*/

//
// TURN LINE'S TAG LIGHTS ON
//
void
EV_LightTurnOn
( line_t*	line,
  int		bright )
{
    int		i;
    int		j;
    sector_t*	sector;
    sector_t*	temp;
    line_t*	templine;
	
    sector = sectors;
	
    for (i=0;i<numsectors;i++, sector++)
    {
	if (sector->tag == line->tag)
	{
	    // bright = 0 means to search
	    // for highest light level
	    // surrounding sector
	    if (!bright)
	    {
		for (j = 0;j < sector->linecount; j++)
		{
		    templine = sector->lines[j];
		    temp = getNextSector(templine,sector);

		    if (!temp)
			continue;

		    if (temp->lightlevel > bright)
			bright = temp->lightlevel;
		}
	    }
	    sector-> lightlevel = bright;
	}
    }
}

    

boolean EV_Lights(sector_t* sec, lighttype_t* type, void *null)
{
    light_t*    light;
	
    if (sec->specialdata[LIGHTS])
     return false;

    light = Z_Malloc(sizeof(*light), PU_LEVSPEC, NULL);

    P_AddThinker(&light->thinker);

    light->thinker.function.acp1 = (actionf_p1) T_Light;
    light->sector = sec;

    light->type = type->type;

    light->minlight = P_FindMinSurroundingLight(sec,sec->lightlevel);
    light->maxlight = sec->lightlevel;
    light->direction = -1;
    light->darktime = type->darktime;
    light->brighttime = type->brighttime;
    light->probability = type->light;
    light->count = type->sync ? (leveltime % type->sync) + 1: light->darktime;

    return true;
}
//
// Spawn glowing light
//
/*
void T_Glow(glow_t*	g)
{
    switch(g->direction)
    {
      case -1:
	// DOWN
	g->sector->lightlevel -= GLOWSPEED;
	if (g->sector->lightlevel <= g->minlight)
	{
	    g->sector->lightlevel += GLOWSPEED;
	    g->direction = 1;
	}
	break;
	
      case 1:
	// UP
	g->sector->lightlevel += GLOWSPEED;
	if (g->sector->lightlevel >= g->maxlight)
	{
	    g->sector->lightlevel -= GLOWSPEED;
	    g->direction = -1;
	}
	break;
    }
}


void P_SpawnGlowingLight(sector_t*	sector)
{
    light_t*	g;
	
    g = Z_Malloc( sizeof(*g), PU_LEVSPEC, 0);

    P_AddThinker(&g->thinker);

    g->sector = sector;
    g->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    g->maxlight = sector->lightlevel;
    g->thinker.function.acp1 = (actionf_p1) T_Light;
    g->direction = -1;
    g->type = lite_glow;
    g->count = g->brighttime = g->darktime = 1;

    sector->special = 0;
}

//
// FIRELIGHT FLICKER
//

//
// T_FireFlicker
//

void T_FireFlicker (fireflicker_t* flick)
{
    int	amount;
	
    if (--flick->count)
	return;
	
    amount = (P_Random()&3)*16;
    
    if (flick->sector->lightlevel - amount < flick->minlight)
	flick->sector->lightlevel = flick->minlight;
    else
	flick->sector->lightlevel = flick->maxlight - amount;

    flick->count = 4;
}



//
// P_SpawnFireFlicker
//
void P_SpawnFireFlicker (sector_t*	sector)
{
    light_t*  flick;
	
    // Note that we are resetting sector attributes.
    // Nothing special about it during gameplay.
    sector->special = 0; 
	
    flick = Z_Malloc ( sizeof(*flick), PU_LEVSPEC, 0);

    P_AddThinker (&flick->thinker);

    flick->thinker.function.acp1 = (actionf_p1) T_Light;
    flick->sector = sector;
    flick->maxlight = sector->lightlevel;
    flick->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel)+16;
    flick->count = flick->brighttime = flick->darktime = 4;
    flick->direction = -1;
    flick->type = lite_fireflicker;
}



//
// BROKEN LIGHT FLASHING
//


//
// T_LightFlash
// Do flashing lights.
//

void T_LightFlash (lightflash_t* flash)
{
    if (--flash->count)
	return;
	
    if (flash->sector->lightlevel == flash->maxlight)
    {
	flash-> sector->lightlevel = flash->minlight;
	flash->count = (P_Random()&flash->mintime)+1;
    }
    else
    {
	flash-> sector->lightlevel = flash->maxlight;
	flash->count = (P_Random()&flash->maxtime)+1;
    }

}




//
// P_SpawnLightFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void P_SpawnLightFlash (sector_t*	sector)
{
    light_t*	flash;

    // nothing special about it during gameplay
    sector->special = 0;	
	
    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);

    P_AddThinker (&flash->thinker);

    flash->thinker.function.acp1 = (actionf_p1) T_Light;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;

    flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    flash->probability = 25;
    flash->count = flash->darktime = flash->brighttime = 8;
    flash->type = lite_flash;
}



//
// STROBE LIGHT FLASHING
//


//
// T_StrobeFlash
//

void T_StrobeFlash (strobe_t*		flash)
{
    if (--flash->count)
	return;
	
    if (flash->sector->lightlevel == flash->minlight)
    {
	flash-> sector->lightlevel = flash->maxlight;
	flash->count = flash->brighttime;
    }
    else
    {
	flash-> sector->lightlevel = flash->minlight;
	flash->count =flash->darktime;
    }

}



//
// P_SpawnStrobeFlash
// After the map has been loaded, scan each sector
// for specials that spawn thinkers
//
void
P_SpawnStrobeFlash
( sector_t*	sector,
  int		fastOrSlow,
  int		inSync )
{
    light_t*	flash;
	
    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);

    P_AddThinker (&flash->thinker);

    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->thinker.function.acp1 = (actionf_p1) T_Light;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);
    flash->type = lite_strobe;
		
    if (flash->minlight == flash->maxlight)
	flash->minlight = 0;

    // nothing special about it during gameplay
    sector->special = 0;	

    if (!inSync)
	flash->count = (P_Random()&7)+1;
    else
	flash->count = 1;
}


//
// Start strobing lights (usually from a trigger)
//

void EV_StartLightStrobing(line_t*	line)
{
    int		secnum;
    sector_t*	sec;
	
    secnum = -1;
    while ((secnum = P_FindSectorFromTag(line->tag,secnum)) >= 0)
    {
	sec = &sectors[secnum];
	if (sec->specialdata[LIGHTS])
	    continue;
	
	P_SpawnStrobeFlash (sec,SLOWDARK, 0);
    }
}
*/

