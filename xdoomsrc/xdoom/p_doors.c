// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-1999 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION: Door animation code (opening/closing)
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "z_zone.h"
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "i_system.h"
#include "doomstat.h"
#include "r_state.h"
#include "dstrings.h"
#include "sounds.h"
#include "w_wad.h"

//
// Sliding door frame information
//
slidename_t	slideFrameNames[MAXSLIDEDOORS] =
{
    // front
    {"GDOOR1F1","GDOOR1F2","GDOOR1F3","GDOOR1F4",
     "GDOOR1F5","GDOOR1F6","GDOOR1F7","GDOOR1F8",
    // back
     "GDOOR1B1","GDOOR1B2","GDOOR1B3","GDOOR1B4",
     "GDOOR1B5","GDOOR1B6","GDOOR1B7","GDOOR1B8"},

    // front
    {"GDOOR2F1","GDOOR2F2","GDOOR2F3","GDOOR2F4",
     "GDOOR2F5","GDOOR2F6","GDOOR2F7","GDOOR2F8",
    // back
     "GDOOR2B1","GDOOR2B2","GDOOR2B3","GDOOR2B4",
     "GDOOR2B5","GDOOR2B6","GDOOR2B7","GDOOR2B8"},

    // front
    {"GDOOR3F1","GDOOR3F2","GDOOR3F3","GDOOR3F4",
     "GDOOR3F5","GDOOR3F6","GDOOR3F7","GDOOR3F8",
    // back
     "GDOOR3B1","GDOOR3B2","GDOOR3B3","GDOOR3B4",
     "GDOOR3B5","GDOOR3B6","GDOOR3B7","GDOOR3B8"},

    // front
    {"GDOOR4F1","GDOOR4F2","GDOOR4F3","GDOOR4F4",
     "GDOOR4F5","GDOOR4F6","GDOOR4F7","GDOOR4F8",
    // back
     "GDOOR4B1","GDOOR4B2","GDOOR4B3","GDOOR4B4",
     "GDOOR4B5","GDOOR4B6","GDOOR4B7","GDOOR4B8"},

    {"\0","\0","\0","\0","\0","\0","\0","\0"}
};

//
// VERTICAL DOORS
//

//
// T_VerticalDoor
//
void T_VerticalDoor(vldoor_t *door)
{
    result_e	res;

    switch(door->direction)
    {
      case 0:
	// WAITING
	if (!--door->topcountdown)
	{
	    switch(door->type)
	    {
	      case blazeRaise:
		door->direction = -1; // time to go back down
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_bdcls);
		break;

	      case normal:
		door->direction = -1; // time to go back down
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_dorcls);
		break;

	      case close30ThenOpen:
		door->direction = 1;
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn);
		break;

	      default:
		break;
	    }
	}
	break;

      case 2:
	//  INITIAL WAIT
	if (!--door->topcountdown)
	{
	    switch(door->type)
	    {
	      case raiseIn5Mins:
		door->direction = 1;
		door->type = normal;
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn);
		break;

	      default:
		break;
	    }
	}
	break;

      case -1:
	// DOWN
	res = T_MovePlane(door->sector,
			  door->speed,
			  door->sector->floorheight,
			  false, 1, door->direction);
	if (res == pastdest)
	{
	    switch(door->type)
	    {
	      case blazeRaise:
	      case blazeClose:
		door->sector->ceilingdata = (void *)0;
		P_RemoveThinker(&door->thinker); // unlink and free
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_bdcls);
		break;

	      case normal:
	      case close:
		door->sector->ceilingdata = (void *)0;
		P_RemoveThinker(&door->thinker); // unlink and free
		break;

	      case close30ThenOpen:
		door->direction = 0;
		door->topcountdown = 35 * 30;
		break;

	      default:
		break;
	    }

	    // turn lighting off in tagged sectors of manual doors
	    // idea taken from Boom
	    if (door->line && door->line->tag)
	    {
	        switch (door->line->special)
	        {
		    case 1: case 31:
		    case 26:
		    case 27: case 28:
		    case 32: case 33:
		    case 34: case 117:
		    case 118:
			EV_TurnTagLightsOff(door->line);
			break;

		    default:
			break;
		}
	    }
	}
	else if (res == crushed)
	{
	    switch(door->type)
	    {
	      case blazeClose:
	      case close:		// DO NOT GO BACK UP!
		break;

	      default:
		door->direction = 1;
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn);
		break;
	    }
	}
	break;

      case 1:
	// UP
	res = T_MovePlane(door->sector,
			  door->speed,
			  door->topheight,
			  false, 1, door->direction);
	if (res == pastdest)
	{
	    switch(door->type)
	    {
	      case blazeRaise:
	      case normal:
		door->direction = 0; // wait at top
		door->topcountdown = door->topwait;
		break;

	      case close30ThenOpen:
	      case blazeOpen:
	      case open:
		door->sector->ceilingdata = (void *)0;
		P_RemoveThinker(&door->thinker); // unlink and free
		break;

	      default:
		break;
	    }

	    // turn lighting on in tagged sectors of manual doors
	    // idea taken from Boom
	    if (door->line && door->line->tag)
	    {
		switch (door->line->special)
		{
		    case 1: case 31:
		    case 26:
		    case 27: case 28:
		    case 32: case 33:
		    case 34: case 117:
		    case 118:
			EV_LightTurnOn(door->line, 0);
			break;

		    default:
			break;
		}
	    }
	}
	break;
    }
}

//
// EV_DoLockedDoor
// Move a locked door up/down
//
int EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing)
{
    player_t	*p;

    p = thing->player;

    if (!p)
	return 0;

    switch(line->special)
    {
      case 99:	// Blue Lock
      case 133:
	if (!p)
	    return 0;
	if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
	{
	    p->message = PD_BLUEO;
	    S_StartSound(NULL, sfx_oof);
	    return 0;
	}
	break;

      case 134: // Red Lock
      case 135:
	if (!p)
	    return 0;
	if (!p->cards[it_redcard] && !p->cards[it_redskull])
	{
	    p->message = PD_REDO;
	    S_StartSound(NULL, sfx_oof);
	    return 0;
	}
	break;

      case 136:	// Yellow Lock
      case 137:
	if (!p)
	    return 0;
	if (!p->cards[it_yellowcard] &&
	    !p->cards[it_yellowskull])
	{
	    p->message = PD_YELLOWO;
	    S_StartSound(NULL, sfx_oof);
	    return 0;
	}
	break;
    }

    return EV_DoDoor(line, type);
}

int EV_DoDoor(line_t *line, vldoor_e type)
{
    int		secnum, rtn;
    sector_t	*sec;
    vldoor_t	*door;

    secnum = -1;
    rtn = 0;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
	sec = &sectors[secnum];
	if (sec->ceilingdata)
	    continue;

	// new door thinker
	rtn = 1;
	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);
	P_AddThinker(&door->thinker);
	sec->ceilingdata = door;

	door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
	door->sector = sec;
	door->type = type;
	door->topwait = VDOORWAIT;
	door->speed = VDOORSPEED;
	door->line = line;	// remember line that triggered us

	switch(type)
	{
	  case blazeClose:
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4 * FRACUNIT;
	    door->direction = -1;
	    door->speed = VDOORSPEED * 4;
	    S_StartSound((mobj_t *)&door->sector->soundorg,
			 sfx_bdcls);
	    break;

	  case close:
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4 * FRACUNIT;
	    door->direction = -1;
	    S_StartSound((mobj_t *)&door->sector->soundorg,
			 sfx_dorcls);
	    break;

	  case close30ThenOpen:
	    door->topheight = sec->ceilingheight;
	    door->direction = -1;
	    S_StartSound((mobj_t *)&door->sector->soundorg,
			 sfx_dorcls);
	    break;

	  case blazeRaise:
	  case blazeOpen:
	    door->direction = 1;
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4 * FRACUNIT;
	    door->speed = VDOORSPEED * 4;
	    if (door->topheight != sec->ceilingheight)
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_bdopn);
	    break;

	  case normal:
	  case open:
	    door->direction = 1;
	    door->topheight = P_FindLowestCeilingSurrounding(sec);
	    door->topheight -= 4 * FRACUNIT;
	    if (door->topheight != sec->ceilingheight)
		S_StartSound((mobj_t *)&door->sector->soundorg,
			     sfx_doropn);
	    break;

	  default:
	    break;
	}
    }
    return rtn;
}

//
// EV_VerticalDoor : open a door manually, no tag value
//
void EV_VerticalDoor(line_t *line, mobj_t *thing)
{
    player_t	*player;
    int		secnum;
    sector_t	*sec;
    vldoor_t	*door;
    int		side;

    side = 0;	// only front sides can be used

    //	Check for locks
    player = thing->player;

    switch(line->special)
    {
      case 26: // Blue Lock
      case 32:
	if (!player)
	    return;

	if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
	{
	    player->message = PD_BLUEK;
	    S_StartSound(NULL, sfx_oof);
	    return;
	}
	break;

      case 27: // Yellow Lock
      case 34:
	if (!player)
	    return;

	if (!player->cards[it_yellowcard] &&
	    !player->cards[it_yellowskull])
	{
	    player->message = PD_YELLOWK;
	    S_StartSound(NULL, sfx_oof);
	    return;
	}
	break;

      case 28: // Red Lock
      case 33:
	if (!player)
	    return;

	if (!player->cards[it_redcard] && !player->cards[it_redskull])
	{
	    player->message = PD_REDK;
	    S_StartSound(NULL, sfx_oof);
	    return;
	}
	break;
    }

    // if the sector has an active thinker, use it
    sec = sides[line->sidenum[side ^ 1]].sector;
    secnum = sec-sectors;

    if (sec->ceilingdata)
    {
	door = sec->ceilingdata;
	switch(line->special)
	{
	  case	1: // ONLY FOR "RAISE" DOORS, NOT "OPEN"s
	  case	26:
	  case	27:
	  case	28:
	  case	117:
	    if (door->direction == -1)
		door->direction = 1;	// go back up
	    else
	    {
		if (!thing->player)
		    return;		// JDC: bad guys never close doors

		door->direction = -1;	// start going down immediately
	    }
	    return;
	}
    }

    // for proper sound
    switch(line->special)
    {
      case 117:	// BLAZING DOOR RAISE
      case 118:	// BLAZING DOOR OPEN
	S_StartSound((mobj_t *)&sec->soundorg, sfx_bdopn);
	break;

      case 1:	// NORMAL DOOR SOUND
      case 31:
	S_StartSound((mobj_t *)&sec->soundorg, sfx_doropn);
	break;

      default:	// LOCKED DOOR SOUND
	S_StartSound((mobj_t *)&sec->soundorg, sfx_doropn);
	break;
    }

    // new door thinker
    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);
    P_AddThinker(&door->thinker);
    sec->ceilingdata = door;
    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;
    door->line = line;	// remember line that triggered us

    switch(line->special)
    {
      case 1:
      case 26:
      case 27:
      case 28:
	door->type = normal;
	break;

      case 31:
      case 32:
      case 33:
      case 34:
	door->type = open;
	line->special = 0;
	break;

      case 117:	// blazing door raise
	door->type = blazeRaise;
	door->speed = VDOORSPEED * 4;
	break;

      case 118:	// blazing door open
	door->type = blazeOpen;
	line->special = 0;
	door->speed = VDOORSPEED * 4;
	break;
    }

    // find the top and bottom of the movement range
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
}

//
// Spawn a door that closes after 30 seconds
//
void P_SpawnDoorCloseIn30(sector_t *sec)
{
    vldoor_t	*door;

    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);

    P_AddThinker(&door->thinker);

    sec->ceilingdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * 35;
    door->line = (line_t *)0;	// no line triggered us
}

//
// Spawn a door that opens after 5 minutes
//
void P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum)
{
    vldoor_t	*door;

    door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);

    P_AddThinker(&door->thinker);

    sec->ceilingdata = door;
    sec->special = 0;

    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * 35;
    door->line = (line_t *)0;	// no line triggered us
}

//
// EV_SlidingDoor : slide a door horizontally
// (animate midtexture, then set noblocking line)
//


slideframe_t slideFrames[MAXSLIDEDOORS];

void P_InitSlidingDoorFrames(void)
{
    int		i;
    int		f1, f2, f3, f4, f5, f6, f7, f8;
    char	tname[9];

    // DOOM II ONLY...
    if (gamemode != commercial)
	return;

    for (i = 0; i < MAXSLIDEDOORS; i++)
    {
	if (!slideFrameNames[i].frontFrame1[0])
	    break;

	// no textures for sliding doors in any IWAD...
	sprintf(&tname[0], "GDOOR%1dF1", i + 1);
	if (W_CheckNumForName(&tname[0]) == -1)
	    continue;

	f1 = R_TextureNumForName(slideFrameNames[i].frontFrame1);
	f2 = R_TextureNumForName(slideFrameNames[i].frontFrame2);
	f3 = R_TextureNumForName(slideFrameNames[i].frontFrame3);
	f4 = R_TextureNumForName(slideFrameNames[i].frontFrame4);
	f5 = R_TextureNumForName(slideFrameNames[i].frontFrame5);
	f6 = R_TextureNumForName(slideFrameNames[i].frontFrame6);
	f7 = R_TextureNumForName(slideFrameNames[i].frontFrame7);
	f8 = R_TextureNumForName(slideFrameNames[i].frontFrame8);

	slideFrames[i].frontFrames[0] = f1;
	slideFrames[i].frontFrames[1] = f2;
	slideFrames[i].frontFrames[2] = f3;
	slideFrames[i].frontFrames[3] = f4;
	slideFrames[i].frontFrames[4] = f5;
	slideFrames[i].frontFrames[5] = f6;
	slideFrames[i].frontFrames[6] = f7;
	slideFrames[i].frontFrames[7] = f8;

	f1 = R_TextureNumForName(slideFrameNames[i].backFrame1);
	f2 = R_TextureNumForName(slideFrameNames[i].backFrame2);
	f3 = R_TextureNumForName(slideFrameNames[i].backFrame3);
	f4 = R_TextureNumForName(slideFrameNames[i].backFrame4);
	f5 = R_TextureNumForName(slideFrameNames[i].backFrame5);
	f6 = R_TextureNumForName(slideFrameNames[i].backFrame6);
	f7 = R_TextureNumForName(slideFrameNames[i].backFrame7);
	f8 = R_TextureNumForName(slideFrameNames[i].backFrame8);

	slideFrames[i].backFrames[0] = f1;
	slideFrames[i].backFrames[1] = f2;
	slideFrames[i].backFrames[2] = f3;
	slideFrames[i].backFrames[3] = f4;
	slideFrames[i].backFrames[4] = f5;
	slideFrames[i].backFrames[5] = f6;
	slideFrames[i].backFrames[6] = f7;
	slideFrames[i].backFrames[7] = f8;
    }
}

//
// Return index into "slideFrames" array
// for which door type to use
//
int P_FindSlidingDoorType(line_t *line)
{
    int		i;
    int		val;

    for (i = 0; i < MAXSLIDEDOORS; i++)
    {
	val = sides[line->sidenum[0]].midtexture;
	if (val == slideFrames[i].frontFrames[0])
	    return i;
    }

    return -1;
}

void T_SlidingDoor(slidedoor_t *door)
{
    switch(door->status)
    {
      case sd_opening:
	if (!door->timer--)
	{
	    if (++door->frame == SNUMFRAMES)
	    {
		// IF DOOR IS DONE OPENING...
		// doesn't look good if the door vanishes into the concrete
		//sides[door->line->sidenum[0]].midtexture = 0;
		//sides[door->line->sidenum[1]].midtexture = 0;

		// also feels better if one can pass a half open door already
		//door->line->flags &= ML_BLOCKING ^ 0xff;

		if (door->type == sdt_openOnly)
		{
		    door->frontsector->ceilingdata = (void *)0;
		    P_RemoveThinker(&door->thinker);
		    break;
		}

		door->timer = SDOORWAIT;
		door->status = sd_waiting;
	    }
	    else
	    {
		// make door passable if half open
		if (door->frame == SNUMFRAMES / 2)
		    door->line->flags &= ML_BLOCKING ^ 0xff;

		// IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
		door->timer = SWAITTICS;

		sides[door->line->sidenum[0]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    frontFrames[door->frame];
		sides[door->line->sidenum[1]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    backFrames[door->frame];
	    }
	}
	break;

      case sd_waiting:
	// IF DOOR IS DONE WAITING...
	if (!door->timer--)
	{
	    // CAN DOOR CLOSE?
	    if (door->frontsector->thinglist != NULL ||
		door->backsector->thinglist != NULL)
	    {
		door->timer = SDOORWAIT;
		break;
	    }

	    //door->frame = SNUMFRAMES - 1;
	    door->status = sd_closing;
	    door->timer = SWAITTICS;
	    S_StartSound((mobj_t *)&door->frontsector->soundorg, sfx_dorcls);
	}
	break;

      case sd_closing:
	if (!door->timer--)
	{
	    if (--door->frame < 0)
	    {
		// IF DOOR IS DONE CLOSING...
		// sneaking through an almost closed door doesn't feel right
		// door->line->flags |= ML_BLOCKING;
		door->frontsector->ceilingdata = (void *)0;
		P_RemoveThinker(&door->thinker);
		break;
	    }
	    else
	    {
		// if door is 3/4 closed make it unpassable
		if (door->frame == 2)
		    door->line->flags |= ML_BLOCKING;

		// IF DOOR NEEDS TO ANIMATE TO NEXT FRAME...
		door->timer = SWAITTICS;

		sides[door->line->sidenum[0]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    frontFrames[door->frame];
		sides[door->line->sidenum[1]].midtexture =
		    slideFrames[door->whichDoorIndex].
		    backFrames[door->frame];
	    }
	}
	break;
    }
}

void EV_SlidingDoor(line_t *line, mobj_t *thing)
{
    sector_t		*sec;
    slidedoor_t		*door;

    // DOOM II ONLY...
    if (gamemode != commercial)
	return;

    // Make sure door isn't already being animated
    sec = line->frontsector;
    door = NULL;
    if (sec->ceilingdata)
    {
	if (!thing->player)
	    return;

	door = sec->ceilingdata;
	if (door->type == sdt_openAndClose)
	{
	    if (door->status == sd_waiting)
	    {
		door->status = sd_closing;
	    }
	}
	else
	    return;
    }

    // Init sliding door vars
    if (!door)
    {
	door = Z_Malloc(sizeof(*door), PU_LEVSPEC, (void *)0);
	P_AddThinker(&door->thinker);
	sec->ceilingdata = door;

	door->type = sdt_openAndClose;
	door->status = sd_opening;
	door->whichDoorIndex = P_FindSlidingDoorType(line);

	if (door->whichDoorIndex < 0)
	    I_Error("EV_SlidingDoor: Can't use texture for sliding door!");

	door->frontsector = sec;
	door->backsector = line->backsector;
	door->thinker.function.acv = T_SlidingDoor;
	door->timer = SWAITTICS;
	door->frame = 0;
	door->line = line;

	S_StartSound((mobj_t *)&sec->soundorg, sfx_doropn);
    }
}

//
// EV_ForceField : switch a force field off and on
//
void T_ForceField (forcefield_t *field)
{
    switch(field->status)
    {
      // open it and let it close again after some time
      case ff_open:
	field->s0_texture = sides[field->line->sidenum[0]].midtexture;
	field->s1_texture = sides[field->line->sidenum[1]].midtexture;
	sides[field->line->sidenum[0]].midtexture = 0;
	sides[field->line->sidenum[1]].midtexture = 0;
	field->s0_lightlevel = field->frontsector->lightlevel;
	field->s1_lightlevel = field->backsector->lightlevel;
	field->backsector->lightlevel = P_FindMinSurroundingLight(
		field->backsector, field->backsector->lightlevel);
	field->frontsector->lightlevel = P_FindMinSurroundingLight(
		field->frontsector, field->frontsector->lightlevel);
	field->line->flags &= ~(ML_BLOCKING | ML_SHOOTBLOCK);
	field->timer = FFWAITTICS;
	field->status = ff_waiting;
	S_StartSound((mobj_t *)&field->frontsector->soundorg, sfx_metal);
	break;

      // switch it off permanentely
      case ff_damaged:
	sides[field->line->sidenum[0]].midtexture = 0;
	sides[field->line->sidenum[1]].midtexture = 0;
	field->backsector->lightlevel = P_FindMinSurroundingLight(
		field->backsector, field->backsector->lightlevel);
	field->frontsector->lightlevel = P_FindMinSurroundingLight(
		field->frontsector, field->frontsector->lightlevel);
	field->line->flags &= ~(ML_BLOCKING | ML_SHOOTBLOCK);
	field->line->special = 0;
	S_StartSound((mobj_t *)&field->frontsector->soundorg, sfx_metal);
	P_RemoveThinker(&field->thinker);
	break;

      // wait and after timer is used up close it
      case ff_waiting:
	if (!field->timer--)
	{
	    // make sure no things get stuck in the force field
	    if ((field->frontsector->thinglist != NULL) ||
		(field->backsector->thinglist != NULL))
	    {
		field->timer = FFWAITTICS;
		break;
	    }
	    sides[field->line->sidenum[0]].midtexture = field->s0_texture;
	    sides[field->line->sidenum[1]].midtexture = field->s1_texture;
	    field->frontsector->lightlevel = field->s0_lightlevel;
	    field->backsector->lightlevel = field->s1_lightlevel;
	    field->line->flags = field->oldflags;
	    S_StartSound((mobj_t *)&field->frontsector->soundorg, sfx_metal);
	    P_RemoveThinker(&field->thinker);
	}
	break;
    }
}

void EV_ForceField (line_t *line, int flag)
{
    register int	i;
    forcefield_t	*field;

    // find all 2s lines with same tag but force field special
    for (i = -1; (i = P_FindLineFromLineTag(line, i)) >= 0;)
    {
	// force field must be 2s line
	if (!lines[i].flags & ML_TWOSIDED)
	    continue;

	// make sure force field isn't open already
	if (sides[lines[i].sidenum[0]].midtexture == 0)
	    continue;

	// force field special?
	if (lines[i].special == 320)
	{
	    field = Z_Malloc(sizeof(*field), PU_LEVSPEC, (void *)0);
	    field->frontsector = lines[i].frontsector;
	    field->backsector = lines[i].backsector;
	    field->line = &lines[i];
	    field->oldflags = lines[i].flags;
	    // used a switch?
	    if (!flag)
	    {
		field->status = ff_open;
		field->timer = FFWAITTICS;
	    }
	    // no, damaged with gun shot or deactivated with comm gadget
	    else
	    {
		field->status = ff_damaged;
	    }
	    P_AddThinker(&field->thinker);
	    field->thinker.function.acv = T_ForceField;
	}
    }
}
