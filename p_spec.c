//  
// DOSDoom Specials Lines & Floor Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/09/01 Lines.ddf
//

#include <stdlib.h>
#include "dm_defs.h"
#include "dm_state.h"
#include "g_game.h"
#include "i_system.h"
#include "lu_sound.h"
#include "m_argv.h"
#include "m_random.h"
#include "p_local.h"
#include "r_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#include "ddf_main.h" // -KM- 98/07/31 Need animation definitions


//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
  boolean istexture;
  int picnum;
  int basepic;
  int numpics;
  int speed;    
}
anim_t;

#define MAXANIMS                32
//extern int	maxanims;
//extern int	lastanim;
//extern anim_t*	anims;

// -KM- 98/07/31 Moved init to ddf_anim.h
int		maxanims = MAXANIMS;
anim_t*		anims;
int		lastanim;


//
// Animating line specials
//
#define MAXLINEANIMS            64

extern  int		numlinespecials;
extern  line_t**	linespeciallist;

boolean P_DoSectorsFromTag(int tag, void* p1, void* p2, boolean (*func)(sector_t*, void*, void*));


void P_InitPicAnims (void)
{
  int i;
  anim_t* addanim;
    
  //	Init animation
  addanim = anims = Z_Malloc(sizeof(anim_t) * maxanims, PU_STATIC, NULL);
  lastanim = 0;
  for (i=0 ; animdefs[i].istexture != -1 ; i++)
  {
    addanim = &anims[lastanim];
    if (animdefs[i].istexture)
    {
      if (R_CheckTextureNumForName(animdefs[i].startname) == -1)
 	continue;

      addanim->picnum = R_TextureNumForName (animdefs[i].endname);
      addanim->basepic = R_TextureNumForName (animdefs[i].startname);
    }
    else
    {
      if (W_CheckNumForName(animdefs[i].startname) == -1)
  	continue;

      addanim->picnum = R_FlatNumForName (animdefs[i].endname);
      addanim->basepic = R_FlatNumForName (animdefs[i].startname);
    }

    addanim->istexture = animdefs[i].istexture;
    addanim->numpics = addanim->picnum - addanim->basepic + 1;

    if (addanim->numpics < 2)
    {
      I_Error ("P_InitPicAnims: bad cycle from %s to %s",
                             animdefs[i].startname, animdefs[i].endname);
    }

    addanim->speed = animdefs[i].speed;
    lastanim++;

    if (lastanim == maxanims)
      anims = Z_ReMalloc(anims, sizeof(anim_t) * ++maxanims);
  }
	
}

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  sfx_t*     sfx;
  boolean   sfxstarted;
  int       count;
} sectorsfx_t;

//
// T_SectorSFX
//
// -KM- 1998/09/27
//
// This function is called every so often to keep a sector's ambient
// sound going.  The sound should be looped.
//
void T_SectorSFX(sectorsfx_t *sec)
{
  if (--sec->count)
    return;

  sec->count = 7;

  if (!sec->sfxstarted)
  {
    if (P_AproxDistance(players[displayplayer].mo->x - sec->sector->soundorg.x,
                        players[displayplayer].mo->y - sec->sector->soundorg.y)
                        < (S_CLIPPING_DIST<<FRACBITS))
    {
      int channel = S_StartSound((mobj_t *)&sec->sector->soundorg,
                                 sec->sfx);
      if (channel >= 0)
          sec->sfxstarted = true;
    }
  }
  else
  {
    if (P_AproxDistance(players[displayplayer].mo->x - sec->sector->soundorg.x,
                        players[displayplayer].mo->y - sec->sector->soundorg.y)
                        > (S_CLIPPING_DIST<<FRACBITS))
    {
      S_StopSound((mobj_t *)&sec->sector->soundorg);
      sec->sfxstarted = false;
    }
  }
}


//
// UTILITIES
//

//
// getSide()
//
// Will return a side_t* given the number of the current sector,
// the line number, and the side (0/1) that you want.
//
side_t* getSide (int currentSector, int line, int side)
{
  return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}

//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t* getSector (int currentSector, int line, int side)
{
  return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}

//
// twoSided()
//
// Given the sector number and the line number, it will tell you whether the
// line is two-sided or not.
//
int twoSided (int sector, int line)
{
  return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

//
// getNextSector()
//
// Return sector_t * of sector next to current; NULL if not two-sided line
//
sector_t* getNextSector(line_t* line, sector_t* sec)
{
  if (!(line->flags & ML_TWOSIDED))
    return NULL;
		
  if (line->frontsector == sec)
    return line->backsector;
	
  return line->frontsector;
}

//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindLowestFloorSurrounding(sector_t* sec)
{
  int i;
  line_t* check;
  sector_t* other;
  fixed_t floor = sec->floorheight;
	
  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;
	
    if (other->floorheight < floor)
      floor = other->floorheight;
  }

  return floor;
}

//
// P_FindRaiseToTexture()
//
// FIND THE SHORTEST BOTTOM TEXTURE SURROUNDING sec
// AND RETURN IT'S TOP HEIGHT
//
// -KM- 1998/09/01 Lines.ddf; used to be inlined in p_floors
//
fixed_t P_FindRaiseToTexture(sector_t*  sec)
{
  int i;
  side_t* side;
  fixed_t minsize = MAXINT;
  int secnum = sec - sectors;

  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided (secnum, i) )
    {
      side = getSide(secnum,i,0);

      if (side->bottomtexture >= 0)
      {
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
      }

      side = getSide(secnum,i,1);

      if (side->bottomtexture >= 0)
      {
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
      }
    }
  }

  return sec->floorheight + minsize;
}


//
// P_FindHighestFloorSurrounding()
//
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec)
{
  int i;
  line_t* check;
  sector_t* other;
  fixed_t floor = -500*FRACUNIT;
	
  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);
	
    if (!other)
      continue;
	
    if (other->floorheight > floor)
      floor = other->floorheight;
  }

  return floor;
}



//
// P_FindNextHighestFloor
//
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
//
// 23-6-98 KM, done without a fixed array, based on
// the other find next thingys, fixed by ACB.
//
// -ACB- 1998/07/21 if no sectors of greater height found, set min to currentheight.
//
fixed_t P_FindNextHighestFloor ( sector_t* sec, fixed_t	height )
{
    int			i;
    int			min = MAXINT;
    sector_t*		other;

    for (i=0; i < sec->linecount ; i++)
    {
	other = getNextSector(sec->lines[i],sec);

	if (!other)
	    continue;

        if ((other->floorheight > height) && (other->floorheight < min))
          min = other->floorheight;
    }

    // -ACB- 1998/07/21 if No sectors of great height found, set min to currentheight.
    if (min == MAXINT)
      min = height;

    return min;
}

//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t
P_FindLowestCeilingSurrounding(sector_t* sec)
{
    int			i;
    line_t*		check;
    sector_t*		other;
    fixed_t		height = MAXINT;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;

	if (other->ceilingheight < height)
	    height = other->ceilingheight;
    }
    return height;
}


//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t	P_FindHighestCeilingSurrounding(sector_t* sec)
{
    int		i;
    line_t*	check;
    sector_t*	other;
    fixed_t	height = 0;
	
    for (i=0 ;i < sec->linecount ; i++)
    {
	check = sec->lines[i];
	other = getNextSector(check,sec);

	if (!other)
	    continue;

	if (other->ceilingheight > height)
	    height = other->ceilingheight;
    }
    return height;
}



//
// RETURN NEXT SECTOR # THAT TAG REFERS TO
//
// -KM- 1998/09/27 Doesn't need a line.
int P_FindSectorFromTag (int tag, int start)
{
    int	i;

    for (i=start+1;i<numsectors;i++)
    {
	if (sectors[i].tag == tag)
	    return i;
    }

    return -1;
}




//
// Find minimum light from an adjacent sector
//
int P_FindMinSurroundingLight (sector_t* sector, int max)
{
    int		i;
    int		min;
    line_t*	line;
    sector_t*	check;
	
    min = max;
    for (i=0 ; i < sector->linecount ; i++)
    {
	line = sector->lines[i];
	check = getNextSector(line,sector);

	if (!check)
	    continue;

	if (check->lightlevel < min)
	    min = check->lightlevel;
    }
    return min;
}



//
// EVENTS
//
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//


//
// P_ActivateSpecialLine
//
// Called when a special line is activated.
//
// line is the line to be activated, side is the side activated from,
// as lines can only be activated from the right thing is the thing
// activating, to check for player/monster only lines trig is how it
// was activated, ie shot/crossed/pushed.
//
// -KM- 1998/09/01 Procedure Written.
//
boolean P_ActivateSpecialLine (line_t* line, int side, mobj_t* thing, trigger_e trig)
{
  linedeftype_t* special;
  boolean texSwitch = false;
  boolean failedsecurity = true; // -ACB- 1998/09/11 Security pass/fail check

#ifdef DEVELOPERS
  if (!line->special)
    I_Error("P_ActivateSpecialLine: Line: %ld is not Special\n", line - lines);
#endif

  special = DDF_GetFromLineHashTable(line->special);

  if (special->type != trig)
    return false; // -ACB- 1998/09/11 Return Success or Failure

  // Check for use once.
  if (!line->count)
    return false; // -ACB- 1998/09/11 Return Success or Failure

  // Single sided line
  if (special->singlesided && side == 1)
    return false;

  if (special->lumpcheck)
  {
    if (special->lumpcheck[0] == '!')
    {
      if (W_CheckNumForName(special->lumpcheck + 1) != -1)
        return false;
    }
    else
    {
      if (W_CheckNumForName(special->lumpcheck) == -1)
        return false;
    }
  }

  if (thing)
  {
    // Check this type of thing can trigger
    if (!thing->player) // Missile / Monster
    {
      // Monsters/Missiles don't trigger secrets
      if (line->flags & ML_SECRET)
        return false; // -ACB- 1998/09/11 Return Success or Failure
  
      // -ACB- 1998/09/07 Remove missileteleport check, not needed.
      if (thing->flags & MF_MISSILE)
      {
        // Missiles can only trigger linedefs with the trig_projectile flag set
        if (!(special->obj & trig_projectile))
          return false; // -ACB- 1998/09/11 Return Success or Failure
      }
      else
      {
        // Monsters can only trigger if the trig_monster flag is set
        if (!(special->obj & trig_monster))
          return false; // -ACB- 1998/09/11 Return Success or Failure
      }
    }
    else
    {
      // Players can only trigger if the trig_player is set
      if (!(special->obj & trig_player))
        return false; // -ACB- 1998/09/11 Return Success or Failure
    }
  
    // Check for keys
    // -ACB- 1998/09/11 Key possibilites extended
    if (special->keys != KF_NONE)
    {
      // Monsters/Missiles have no keys
      if (!thing->player)
        return false; // -ACB- 1998/09/11 Return Success or Failure
  
      //
      // New Security Checks, allows for any combination of keys in
      // an AND or OR function. Therefore it extends the possibilities
      // of security above 3 possible combinations..
      //
      // Could be better, but cheap and effective.
      if ((special->keys & KF_BLUECARD) && thing->player->cards[it_bluecard])
        failedsecurity = false;
  
      if ((special->keys & KF_YELLOWCARD) && thing->player->cards[it_yellowcard])
        failedsecurity = false;
  
      if ((special->keys & KF_REDCARD) && thing->player->cards[it_redcard])
        failedsecurity = false;
  
      if ((special->keys & KF_BLUESKULL) && thing->player->cards[it_blueskull])
        failedsecurity = false;
  
      if ((special->keys & KF_YELLOWSKULL) && thing->player->cards[it_yellowskull])
        failedsecurity = false;
  
      if ((special->keys & KF_REDSKULL) && thing->player->cards[it_redskull])
        failedsecurity = false;
  
      if ((special->keys & (KF_BLUECARD<<8)) && !thing->player->cards[it_bluecard])
        failedsecurity = true;
  
      if ((special->keys & (KF_YELLOWCARD<<8)) && !thing->player->cards[it_yellowcard])
        failedsecurity = true;
  
      if ((special->keys & (KF_REDCARD<<8)) && !thing->player->cards[it_redcard])
        failedsecurity = true;
  
      if ((special->keys & (KF_BLUESKULL<<8)) && !thing->player->cards[it_blueskull])
        failedsecurity = true;
  
      if ((special->keys & (KF_YELLOWSKULL<<8)) && !thing->player->cards[it_yellowskull])
        failedsecurity = true;
  
      if ((special->keys & (KF_REDSKULL<<8)) && !thing->player->cards[it_redskull])
        failedsecurity = true;
  
      if (failedsecurity)
      {
        if (special->failedmessage)
          thing->player->message = DDF_LanguageLookup(special->failedmessage);
  
        return false; // -ACB- 1998/09/11 Return Success or Failure
      }
  
    }
  }
  // Do lights
  // -KM- 1998/09/27 Generalised light types.
  switch (special->l.type)
  {
    case lite_set:
      EV_LightTurnOn(line, special->l.light);
      texSwitch = true;
      break;

    case lite_none:
      break;

    default:
      texSwitch = P_DoSectorsFromTag(line->tag, &special->l, NULL,
                                         (boolean (*)(sector_t*, void*, void*))EV_Lights);
      break;
  }

  // -ACB- 1998/09/13 Use teleport define..
  if (special->t.teleport)
  {
    texSwitch = EV_Teleport(line, 1, thing, special->t.delay,
                              special->t.inspawnobj, special->t.outspawnobj);
  }

  if (special->e_exit == 1)
  {
    G_ExitLevel(5);
    texSwitch = true;
  }
  else if (special->e_exit == 2)
  {
    G_SecretExitLevel(5);
    texSwitch = true;
  }

  if (special->d.dodonut)
  {
    sfx_t* sfx[4] = {special->d.d_sfxout, special->d.d_sfxoutstop,
                  special->d.d_sfxin, special->d.d_sfxinstop};
    texSwitch = P_DoSectorsFromTag(line->tag, sfx, NULL, (boolean (*)(sector_t*, void*, void*))EV_DoDonut);
  }

  // - Plats/Floors -
  if (special->f.type != mov_undefined)
  {
    if (!line->tag)
    {
      texSwitch = EV_Manual(line, thing, &special->f);
    }
    else
    {
      texSwitch = P_DoSectorsFromTag(line->tag, &special->f, line->frontsector,
                                     (boolean (*)(sector_t*, void*, void*)) EV_DoSector);
    }
  }

  // - Doors/Ceilings -
  if (special->c.type != mov_undefined)
  {
    if (!line->tag)
    {
      texSwitch = EV_Manual(line, thing, &special->c);
    }
    else
    {
      texSwitch = P_DoSectorsFromTag(line->tag, &special->c, line->frontsector,
                                         (boolean (*)(sector_t*, void*, void*))EV_DoSector);
    }
  }

  if (special->colourmaplump[0])
  {
    int colourmaplump = W_GetNumForName(special->colourmaplump);
    int		secnum = -1;
  	
    while ((secnum = P_FindSectorFromTag(line->tag,secnum)) >= 0)
    {
      sectors[secnum].colourmaplump = colourmaplump;
      texSwitch = true;
    }
  }

  if (special->colourmap >= 0)
  {
    int		secnum = -1;
  	
    while ((secnum = P_FindSectorFromTag(line->tag,secnum)) >= 0)
    {
      sectors[secnum].colourmap = special->colourmap;
      texSwitch = true;
    }
  }

  if (special->gravity)
  {
    int		secnum = -1;
  	
    while ((secnum = P_FindSectorFromTag(line->tag,secnum)) >= 0)
    {
      sectors[secnum].gravity = special->gravity * 1024;
      texSwitch = true;
    }
  }

  if (special->friction)
  {
    int		secnum = -1;
  	
    while ((secnum = P_FindSectorFromTag(line->tag,secnum)) >= 0)
    {
      sectors[secnum].friction = special->friction;
      texSwitch = true;
    }
  }

  if (special->sfx)
  {
    int		secnum = -1;
  	
    while ((secnum = P_FindSectorFromTag(line->tag,secnum)) >= 0)
    {
      sectorsfx_t* sfx = Z_Malloc(sizeof(*sfx), PU_LEVSPEC, NULL);
 
      P_AddThinker(&sfx->thinker);
 
      sfx->count = 7;
      sfx->sector = &sectors[secnum];
      sfx->sfx = special->sfx;
      sfx->sfxstarted = false;
      sfx->thinker.function.acp1 = (actionf_p1)T_SectorSFX;

      texSwitch = true;
    }
  }

  if (special->music[0])
  {
    S_ChangeMusic(special->music, true);
    texSwitch = true;
  }


  // reduce count & clear special if necessary
  if (texSwitch)
  {
    if (line->count != -1)
    {
      line->count--;
      if (!line->count)
        line->special = 0;
    }
    // -KM- 1998/09/27 Reversable linedefs.
    if (line->special && special->newtrignum)
      line->special = special->newtrignum;

    P_ChangeSwitchTexture(line, line->special && !special->newtrignum ? true : false);
  }

  return true;
}

//
// P_CrossSpecialLine - TRIGGER
//
// Called every time a thing origin is about
// to cross a line with a non 0 special.
//
// -KM- 1998/09/01 Now much simpler
// -ACB- 1998/09/12 Return success/failure
//
boolean P_CrossSpecialLine (int linenum, int side, mobj_t* thing)
{
  return P_ActivateSpecialLine (&lines[linenum], side, thing, line_walkable);
}


//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void P_ShootSpecialLine (mobj_t* thing, line_t*	line)
{
  P_ActivateSpecialLine(line, 0, thing, line_shootable);
}

//
// P_PlayerInSpecialSector
//
// Called every tic frame that the player origin is in a special sector
//
// -KM- 1998/09/27 Generalised for sectors.ddf
void P_PlayerInSpecialSector (player_t* player)
{
    sector_t*	sector;
    specialsector_t* special;
	
    sector = player->mo->subsector->sector;

    // Falling, not all the way down yet?
    if (player->mo->z != sector->floorheight)
	return;	

    special = DDF_GetFromSectHashTable(sector->special);

    if (special->damage)
      if (!player->powers[pw_ironfeet])
        if (!(leveltime % special->damagetime))
          P_DamageMobj (player->mo, NULL, NULL, special->damage);

    if (special->secret)
    {
      player->secretcount++;
      sector->special = 0;
    }

    if (special->exit == 1)
    {
      player->cheats &= ~CF_GODMODE;
      if (player->health <= special->damage)
      {
        S_StartSound(player->mo, player->mo->info->deathsound);
        // -KM- 1998/12/16 We don't want to alter the special type,
        //   modify the sector's attributes instead.
        sector->special = 0;
        G_ExitLevel(1);
      }
    } else if (special->exit == 2)
    {
      player->cheats &= ~CF_GODMODE;
      if (player->health <= special->damage)
      {
        S_StartSound(player->mo, player->mo->info->deathsound);
        sector->special = 0;
        G_SecretExitLevel(1);
      }
    }
}




//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//
boolean		levelTimer;
int		levelTimeCount;

void P_UpdateSpecials (void)
{
    anim_t*	anim;
    int		pic;
    int		i;
    line_t*	line;
    linedeftype_t*   special;

    
    //	LEVEL TIMER
    if (levelTimer == true)
    {
	levelTimeCount--;
	if (!levelTimeCount)
	    G_ExitLevel(1);
    }
    
    //	ANIMATE FLATS AND TEXTURES GLOBALLY
    for (anim = &anims[lastanim] ; anim-- != &anims[0];)
    {
	for (i=anim->basepic ; i<anim->basepic+anim->numpics ; i++)
	{
	    pic = anim->basepic + ( (leveltime/anim->speed + i)%anim->numpics );
	    if (anim->istexture)
		texturetranslation[i] = pic;
	    else
		flattranslation[i] = pic;
	}
    }

    
    //	ANIMATE LINE SPECIALS
    // -KM- 1998/09/01 Lines.ddf
    for (i = 0; i < numlinespecials; i++)
    {
      line = linespeciallist[i];
      special = DDF_GetFromLineHashTable(line->special);
      if (special->scroller)
      {
        if (special->scroller & dir_horiz)
          sides[line->sidenum[0]].textureoffset +=
            special->scroller & dir_left ? special->s_speed : -special->s_speed;
        if (special->scroller & dir_vert)
          sides[line->sidenum[0]].rowoffset +=
            special->scroller & dir_up   ? special->s_speed : -special->s_speed;
      }
    }

    
    //	DO BUTTONS
    for (i = 0; i < maxbuttons; i++)
	if (buttonlist[i].btimer)
	{
	    buttonlist[i].btimer--;
	    if (!buttonlist[i].btimer)
	    {
		switch(buttonlist[i].where)
		{
		  case top:
		    sides[buttonlist[i].line->sidenum[0]].toptexture =
			buttonlist[i].btexture;
		    break;
		    
		  case middle:
		    sides[buttonlist[i].line->sidenum[0]].midtexture =
			buttonlist[i].btexture;
		    break;
		    
		  case bottom:
		    sides[buttonlist[i].line->sidenum[0]].bottomtexture =
			buttonlist[i].btexture;
		    break;
		}
		S_StartSound((mobj_t *)&buttonlist[i].soundorg,sfx_swtchn);
		memset(&buttonlist[i],0,sizeof(button_t));
	    }
	}
	
}

//
// SPECIAL SPAWNING
//

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//
int		maxlinespecials = MAXLINEANIMS;
int		numlinespecials;
line_t**	linespeciallist = NULL;


//
// P_SpawnSpecials
//
// This function is called at the start of every level.  It parses command line
// parameters for level timer, spawns passive special sectors, (ie sectors that
// act even when a player is not in them, and counts total secrets) spawns
// passive lines, (ie scrollers) and resets floor/ceiling movement.
//
// (-ACB- 1998/09/06 Its parses command line parameters? really? cool. I going to
//                   to get my C book out and see how this works [:)]. )
// -KM- 1998/09/27 Generalised for sectors.ddf
// -KM- 1998/11/25 Lines with auto tag are automatically triggered.
void P_SpawnSpecials (int autotag)
{
     sector_t*	sector;
     specialsector_t* secSpecial;
     int i;
     int episode;
     line_t* line;
     linedeftype_t* special;

     episode = 1;
     if (W_CheckNumForName("texture2") >= 0)
       episode = 2;

    
     // See if -TIMER needs to be used.
     levelTimer = false;
	
     i = M_CheckParm("-avg");
     if (i && deathmatch)
     {
	levelTimer = true;
	levelTimeCount = 20 * 60 * 35;
     }
	
     i = M_CheckParm("-timer");

     if (i && deathmatch)
     {
	int	time;
	time = atoi(myargv[i+1]) * 60 * 35;
	levelTimer = true;
	levelTimeCount = time;
     }
    
     //	Init other misc stuff
     P_ResetActiveSecs();

     for (i = 0;i < maxbuttons;i++)
       memset(&buttonlist[i],0,sizeof(button_t));

     //	Init special SECTORs.
     sector = sectors;
     for (i=0 ; i<numsectors ; i++, sector++)
     {
        sector->colourmaplump = -1;
        sector->colourmap = -1;
        sector->gravity = 8192;
        sector->friction = FRICTION;
        sector->viscosity = 0;

	if (!sector->special)
          continue;

        secSpecial = DDF_GetFromSectHashTable(sector->special);

        if (secSpecial->lumpcheck)
        {
          if (secSpecial->lumpcheck[0] == '!')
          {
            if (W_CheckNumForName(secSpecial->lumpcheck + 1) != -1)
              continue;
          }
          else
          {
            if (W_CheckNumForName(secSpecial->lumpcheck) == -1)
              continue;
          }
        }

        if (secSpecial->l.type != lite_none)
          EV_Lights(sector, &secSpecial->l, NULL);

        if (secSpecial->secret)
          totalsecret++;

        if (secSpecial->colourmaplump[0])
          sector->colourmaplump = W_GetNumForName(secSpecial->colourmaplump);

        sector->colourmap = secSpecial->colourmap;

        if (secSpecial->sfx)
        {
          sectorsfx_t* sfx = Z_Malloc(sizeof(*sfx), PU_LEVSPEC, NULL);

          P_AddThinker(&sfx->thinker);

          sfx->count = 7;
          sfx->sector = sector;
          sfx->sfx = secSpecial->sfx;
          sfx->sfxstarted = false;
          sfx->thinker.function.acp1 = (actionf_p1)T_SectorSFX;
        }

        // - Plats/Floors -
        if (secSpecial->f.type != mov_undefined)
          EV_DoSector(sector, &secSpecial->f, sector);
      
        // - Doors/Ceilings -
        if (secSpecial->c.type != mov_undefined)
          EV_DoSector(sector, &secSpecial->c, sector);

        sector->gravity = secSpecial->gravity * 1024;

        sector->friction = secSpecial->friction;
     }

     // Init line EFFECTs
     //
     // -ACB- & -JC- 1998/06/10 Implemented additional scroll effects
     //
     // -ACB- Added the code
     // -JC-  Designed and contributed code
     // -KM-  Removed Limit
     // -KM- 1998/09/01 Added lines.ddf support
     //
     numlinespecials = 0;
     if (!linespeciallist)
       linespeciallist = Z_Malloc(sizeof(line_t *) * maxlinespecials, PU_STATIC, &linespeciallist);
     for (i = 0;i < numlines; i++)
     {
       if (lines[i].special)
       {
         special = DDF_GetFromLineHashTable(lines[i].special);

         lines[i].count = special->count;
         if (special->scroller)
         {
            linespeciallist[numlinespecials] = &lines[i];
            if (++numlinespecials == maxlinespecials)
              linespeciallist = Z_ReMalloc(linespeciallist, sizeof(line_t *) * ++maxlinespecials);
         }
         if (special->autoline)
           P_ActivateSpecialLine(&lines[i], 0, NULL, line_pushable);

         // -KM- 1998/11/25 This line should be pushed automatically (MAP07)
         if (autotag && lines[i].tag == autotag)
           P_ActivateSpecialLine(&lines[i], 0, NULL, line_pushable);
       }
       else
       {
         lines[i].count = 0;
       }
     }

     // ANIMATE LINE SPECIALS
     //
     // -ACB- & -JC- 1998/06/10 Implemented additional scroll effects
     //
     // -ACB- Attached the code
     // -JC-  Designed and contributed code originally.
     // -KM- 1998/09/01 Reimplemented with DDF
     //
     for (i = 0; i < numlinespecials; i++)
     {
       line = linespeciallist[i];
       special = DDF_GetFromLineHashTable(line->special);

       if (!special) continue;

       if (special->scroller)
       {
         if (special->scroller & dir_horiz)
           sides[line->sidenum[0]].textureoffset +=
              special->scroller & dir_left ? special->s_speed : -special->s_speed;

         if (special->scroller & dir_vert)
           sides[line->sidenum[0]].rowoffset +=
              special->scroller & dir_up   ? special->s_speed : -special->s_speed;
       }

     }
    
     // UNUSED: no horizonal sliders.
     //	P_InitSlidingDoorFrames();
}

// -KM- 1998/09/27 This helper function is used to do stuff to all the sectors
// with the specified line's tag.
boolean P_DoSectorsFromTag(int tag, void* p1, void* p2, boolean (*func)(sector_t*, void*, void*))
{
  int		secnum;
  sector_t*	sec;
  boolean     rtn = false, rtn2 = false;
	
  secnum = -1;
  while ((secnum = P_FindSectorFromTag(tag,secnum)) >= 0)
  {
	sec = &sectors[secnum];
        rtn2 = func(sec, p1, p2);
        rtn = rtn2 ? rtn2 : rtn;
  }
  return rtn;
}
#if 0
// -KM- 1998/09/01 Old code
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void P_CrossSpecialLine ( int linenum, int side, mobj_t* thing )
{
    line_t*	line;
    int		ok;

    line = &lines[linenum];

    if (!line->special)
      I_Error("DOH!");
    
    //	Triggers that other things can activate
    if (!thing->player)
    {
	// Things that NOW CAN trigger specials...
/*      if (!missileteleport)
        {                                          //-JC-
	   switch(thing->type)
	   {
	      case MT_ROCKET:
	      case MT_PLASMA:
	      case MT_BFG:
                 return;
                 break;

	      default: break;
           }
        }    */

	// Things that should NOT trigger specials...
/*	switch(thing->type)
	{
	  case MT_TROOPSHOT:
	  case MT_HEADSHOT:
	  case MT_BRUISERSHOT:
	    return;
	    break;
	    
	  default: break;
	}            */
		
	ok = 0;
	switch(line->special)
	{
	  case 39:	// TELEPORT TRIGGER
	  case 97:	// TELEPORT RETRIGGER
	  case 125:	// TELEPORT MONSTERONLY TRIGGER
	  case 126:	// TELEPORT MONSTERONLY RETRIGGER
	  case 4:	// RAISE DOOR
	  case 10:	// PLAT DOWN-WAIT-UP-STAY TRIGGER
	  case 88:	// PLAT DOWN-WAIT-UP-STAY RETRIGGER
	    ok = 1;
	    break;
	}
	if (!ok)
	    return;
    }

    
    // Note: could use some const's here.
    switch (line->special)
    {
	// TRIGGERS.
	// All from here to RETRIGGERS.
      case 2:
	// Open Door
	EV_DoDoor(line,open);
	line->special = 0;
	break;

      case 3:
	// Close Door
	EV_DoDoor(line,close);
	line->special = 0;
	break;

      case 4:
	// Raise Door
	EV_DoDoor(line,normal);
	line->special = 0;
	break;
	
      case 5:
	// Raise Floor
	EV_DoFloor(line,raiseFloor);
	line->special = 0;
	break;
	
      case 6:
	// Fast Ceiling Crush & Raise
	EV_DoCeiling(line,fastCrushAndRaise);
	line->special = 0;
	break;
	
      case 8:
	// Build Stairs
	EV_BuildStairs(line,build8);
	line->special = 0;
	break;
	
      case 10:
	// PlatDownWaitUp
	EV_DoPlat(line,downWaitUpStay,0);
	line->special = 0;
	break;
	
      case 12:
	// Light Turn On - brightest near
	EV_LightTurnOn(line,0);
	line->special = 0;
	break;
	
      case 13:
	// Light Turn On 255
	EV_LightTurnOn(line,255);
	line->special = 0;
	break;
	
      case 16:
	// Close Door 30
	EV_DoDoor(line,close30ThenOpen);
	line->special = 0;
	break;
	
      case 17:
	// Start Light Strobing
	EV_StartLightStrobing(line);
	line->special = 0;
	break;
	
      case 19:
	// Lower Floor
	EV_DoFloor(line,lowerFloor);
	line->special = 0;
	break;
	
      case 22:
	// Raise floor to nearest height and change texture
	EV_DoPlat(line,raiseToNearestAndChange,0);
	line->special = 0;
	break;
	
      case 25:
	// Ceiling Crush and Raise
	EV_DoCeiling(line,crushAndRaise);
	line->special = 0;
	break;
	
      case 30:
	// Raise floor to shortest texture height
	//  on either side of lines.
	EV_DoFloor(line,raiseToTexture);
	line->special = 0;
	break;
	
      case 35:
	// Lights Very Dark
	EV_LightTurnOn(line,35);
	line->special = 0;
	break;
	
      case 36:
	// Lower Floor (TURBO)
	EV_DoFloor(line,turboLower);
	line->special = 0;
	break;
	
      case 37:
	// LowerAndChange
	EV_DoFloor(line,lowerAndChange);
	line->special = 0;
	break;
	
      case 38:
	// Lower Floor To Lowest
	EV_DoFloor( line, lowerFloorToLowest );
	line->special = 0;
	break;
	
      case 39:
	// TELEPORT!
	EV_Teleport( line, side, thing );
	line->special = 0;
	break;

      case 40:
	// RaiseCeilingLowerFloor
	EV_DoCeiling( line, raiseToHighest );
	EV_DoFloor( line, lowerFloorToLowest );
	line->special = 0;
	break;
	
      case 44:
	// Ceiling Crush
	EV_DoCeiling( line, lowerAndCrush );
	line->special = 0;
	break;
	
      case 52:
	// EXIT!
	G_ExitLevel ();
	break;
	
      case 53:
	// Perpetual Platform Raise
	EV_DoPlat(line,perpetualRaise,0);
	line->special = 0;
	break;
	
      case 54:
	// Platform Stop
	EV_StopPlat(line);
	line->special = 0;
	break;

      case 56:
	// Raise Floor Crush
	EV_DoFloor(line,raiseFloorCrush);
	line->special = 0;
	break;

      case 57:
	// Ceiling Crush Stop
	EV_CeilingCrushStop(line);
	line->special = 0;
	break;
	
      case 58:
	// Raise Floor 24
	EV_DoFloor(line,raiseFloor24);
	line->special = 0;
	break;

      case 59:
	// Raise Floor 24 And Change
	EV_DoFloor(line,raiseFloor24AndChange);
	line->special = 0;
	break;
	
      case 104:
	// Turn lights off in sector(tag)
	EV_TurnTagLightsOff(line);
	line->special = 0;
	break;
	
      case 108:
	// Blazing Door Raise (faster than TURBO!)
	EV_DoDoor (line,blazeRaise);
	line->special = 0;
	break;
	
      case 109:
	// Blazing Door Open (faster than TURBO!)
	EV_DoDoor (line,blazeOpen);
	line->special = 0;
	break;
	
      case 100:
	// Build Stairs Turbo 16
	EV_BuildStairs(line,turbo16);
	line->special = 0;
	break;
	
      case 110:
	// Blazing Door Close (faster than TURBO!)
	EV_DoDoor (line,blazeClose);
	line->special = 0;
	break;

      case 119:
	// Raise floor to nearest surr. floor
	EV_DoFloor(line,raiseFloorToNearest);
	line->special = 0;
	break;
	
      case 121:
	// Blazing PlatDownWaitUpStay
	EV_DoPlat(line,blazeDWUS,0);
	line->special = 0;
	break;
	
      case 124:
	// Secret EXIT
	G_SecretExitLevel ();
	break;
		
      case 125:
	// TELEPORT MonsterONLY
	if (!thing->player)
	{
	    EV_Teleport( line, side, thing );
	    line->special = 0;
	}
	break;
	
      case 130:
	// Raise Floor Turbo
	EV_DoFloor(line,raiseFloorTurbo);
	line->special = 0;
	break;
	
      case 141:
	// Silent Ceiling Crush & Raise
	EV_DoCeiling(line,silentCrushAndRaise);
	line->special = 0;
	break;
	
	// RETRIGGERS.  All from here till end.
      case 72:
	// Ceiling Crush
	EV_DoCeiling( line, lowerAndCrush );
	break;

      case 73:
	// Ceiling Crush and Raise
	EV_DoCeiling(line,crushAndRaise);
	break;

      case 74:
	// Ceiling Crush Stop
	EV_CeilingCrushStop(line);
	break;
	
      case 75:
	// Close Door
	EV_DoDoor(line,close);
	break;
	
      case 76:
	// Close Door 30
	EV_DoDoor(line,close30ThenOpen);
	break;
	
      case 77:
	// Fast Ceiling Crush & Raise
	EV_DoCeiling(line,fastCrushAndRaise);
	break;
	
      case 79:
	// Lights Very Dark
	EV_LightTurnOn(line,35);
	break;
	
      case 80:
	// Light Turn On - brightest near
	EV_LightTurnOn(line,0);
	break;
	
      case 81:
	// Light Turn On 255
	EV_LightTurnOn(line,255);
	break;
	
      case 82:
	// Lower Floor To Lowest
	EV_DoFloor( line, lowerFloorToLowest );
	break;
	
      case 83:
	// Lower Floor
	EV_DoFloor(line,lowerFloor);
	break;

      case 84:
	// LowerAndChange
	EV_DoFloor(line,lowerAndChange);
	break;

      case 86:
	// Open Door
	EV_DoDoor(line,open);
	break;
	
      case 87:
	// Perpetual Platform Raise
	EV_DoPlat(line,perpetualRaise,0);
	break;
	
      case 88:
	// PlatDownWaitUp
	EV_DoPlat(line,downWaitUpStay,0);
	break;
	
      case 89:
	// Platform Stop
	EV_StopPlat(line);
	break;
	
      case 90:
	// Raise Door
	EV_DoDoor(line,normal);
	break;
	
      case 91:
	// Raise Floor
	EV_DoFloor(line,raiseFloor);
	break;
	
      case 92:
	// Raise Floor 24
	EV_DoFloor(line,raiseFloor24);
	break;
	
      case 93:
	// Raise Floor 24 And Change
	EV_DoFloor(line,raiseFloor24AndChange);
	break;
	
      case 94:
	// Raise Floor Crush
	EV_DoFloor(line,raiseFloorCrush);
	break;
	
      case 95:
	// Raise floor to nearest height
	// and change texture.
	EV_DoPlat(line,raiseToNearestAndChange,0);
	break;
	
      case 96:
	// Raise floor to shortest texture height
	// on either side of lines.
	EV_DoFloor(line,raiseToTexture);
	break;
	
      case 97:
	// TELEPORT!
	EV_Teleport( line, side, thing );
	break;
	
      case 98:
	// Lower Floor (TURBO)
	EV_DoFloor(line,turboLower);
	break;

      case 105:
	// Blazing Door Raise (faster than TURBO!)
	EV_DoDoor (line,blazeRaise);
	break;
	
      case 106:
	// Blazing Door Open (faster than TURBO!)
	EV_DoDoor (line,blazeOpen);
	break;

      case 107:
	// Blazing Door Close (faster than TURBO!)
	EV_DoDoor (line,blazeClose);
	break;

      case 120:
	// Blazing PlatDownWaitUpStay.
	EV_DoPlat(line,blazeDWUS,0);
	break;
	
      case 126:
	// TELEPORT MonsterONLY.
	if (!thing->player)
	    EV_Teleport( line, side, thing );
	break;
	
      case 128:
	// Raise To Nearest Floor
	EV_DoFloor(line,raiseFloorToNearest);
	break;
	
      case 129:
	// Raise Floor Turbo
	EV_DoFloor(line,raiseFloorTurbo);
	break;
    }
}



//
// P_ShootSpecialLine - IMPACT SPECIALS
// Called when a thing shoots a special line.
//
void
P_ShootSpecialLine
( mobj_t*	thing,
  line_t*	line )
{
    int		ok;
    
    //	Impacts that other things can activate.
    if (!thing->player)
    {
	ok = 0;
	switch(line->special)
	{
	  case 46:
	    // OPEN DOOR IMPACT
	    ok = 1;
	    break;
	}
	if (!ok)
	    return;
    }

    switch(line->special)
    {
      case 24:
	// RAISE FLOOR
	EV_DoFloor(line,raiseFloor);
	P_ChangeSwitchTexture(line,0);
	break;
	
      case 46:
	// OPEN DOOR
	EV_DoDoor(line,open);
	P_ChangeSwitchTexture(line,1);
	break;
	
      case 47:
	// RAISE FLOOR NEAR AND CHANGE
	EV_DoPlat(line,raiseToNearestAndChange,0);
	P_ChangeSwitchTexture(line,0);
	break;
    }
}

#endif
