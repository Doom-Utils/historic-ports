// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1997-2000 by Udo Munk
// Copyright (C) 1998 by Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
// Copyright (C) 2000 by David Koppenhofer
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
// DESCRIPTION:
//	Archiving: SaveGame I/O.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include "i_system.h"
#include "z_zone.h"
#include "p_local.h"
#include "doomstat.h"
#include "r_state.h"
// *** PID BEGIN ***
// Need to include initialization functions.
#include "pr_process.h"
// *** PID END ***

byte	*save_p;

// Pads save_p to a 4-byte boundary
//  so that the load/save works on SGI&Gecko.
#define PADSAVEP()	(save_p += (4 - ((long)save_p & 3)) & 3)

//
// P_ArchivePlayers
//
void P_ArchivePlayers(void)
{
    int		i;
    int		j;
    player_t	*dest;

    for (i = 0; i < MAXPLAYERS; i++)
    {
	if (!playeringame[i])
	    continue;

	PADSAVEP();

	dest = (player_t *)save_p;
	memcpy(dest, &players[i], sizeof(player_t));
	save_p += sizeof(player_t);
	for (j = 0; j < NUMPSPRITES; j++)
	{
	    if (dest->psprites[j].state)
	    {
		dest->psprites[j].state
		    = (state_t *)(dest->psprites[j].state-states);
	    }
	}
    }
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers(void)
{
    int		i;
    int		j;

    for (i = 0; i < MAXPLAYERS; i++)
    {
	if (!playeringame[i])
	    continue;

	PADSAVEP();

	memcpy(&players[i],save_p, sizeof(player_t));
	save_p += sizeof(player_t);

	// will be set when unarc thinker
	players[i].mo = NULL;
	players[i].message = NULL;
	players[i].attacker = NULL;

	for (j = 0; j < NUMPSPRITES; j++)
	{
	    if (players[i].psprites[j].state)
	    {
		players[i].psprites[j].state
		    = &states[(int)players[i].psprites[j].state];
	    }
	}
    }
}

//
// P_ArchiveWorld
//
void P_ArchiveWorld(void)
{
    int			i;
    int			j;
    sector_t		*sec;
    line_t		*li;
    side_t		*si;
    short		*put;

    put = (short *)save_p;

    // do sectors
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
	*put++ = sec->floorheight >> FRACBITS;
	*put++ = sec->ceilingheight >> FRACBITS;
	*put++ = sec->floorpic;
	*put++ = sec->ceilingpic;
	*put++ = sec->lightlevel;
	*put++ = sec->special;
	*put++ = sec->tag;
    }

    // do lines
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
	*put++ = li->flags;
	*put++ = li->special;
	*put++ = li->tag;
	for (j = 0; j < 2; j++)
	{
	    if (li->sidenum[j] == -1)
		continue;

	    si = &sides[li->sidenum[j]];

	    *put++ = si->textureoffset >> FRACBITS;
	    *put++ = si->rowoffset >> FRACBITS;
	    *put++ = si->toptexture;
	    *put++ = si->bottomtexture;
	    *put++ = si->midtexture;
	}
    }

    save_p = (byte *)put;
}

//
// P_UnArchiveWorld
//
void P_UnArchiveWorld(void)
{
    int			i;
    int			j;
    sector_t		*sec;
    line_t		*li;
    side_t		*si;
    short		*get;

    get = (short *)save_p;

    // do sectors
    for (i = 0, sec = sectors; i < numsectors; i++, sec++)
    {
	sec->floorheight = *get++ << FRACBITS;
	sec->ceilingheight = *get++ << FRACBITS;
	sec->floorpic = *get++;
	sec->ceilingpic = *get++;
	sec->lightlevel = *get++;
	sec->special = *get++;
	sec->tag = *get++;
	sec->floordata = 0;
	sec->ceilingdata = 0;
	sec->lightingdata = 0;
	sec->soundtarget = 0;
    }

    // do lines
    for (i = 0, li = lines; i < numlines; i++, li++)
    {
	li->flags = *get++;
	li->special = *get++;
	li->tag = *get++;
	for (j = 0; j < 2; j++)
	{
	    if (li->sidenum[j] == -1)
		continue;
	    si = &sides[li->sidenum[j]];
	    si->textureoffset = *get++ << FRACBITS;
	    si->rowoffset = *get++ << FRACBITS;
	    si->toptexture = *get++;
	    si->bottomtexture = *get++;
	    si->midtexture = *get++;
	}
    }
    save_p = (byte *)get;
}

//
// Thinkers
//
typedef enum
{
    tc_end,
    tc_mobj
} thinkerclass_t;

//
// P_ArchiveThinkers
//
void P_ArchiveThinkers(void)
{
    thinker_t		*th;
    mobj_t		*mobj;

// *** PID BEGIN ***
// Print status message.
    fprintf(stderr, "***** save thinkers: *****\n");
// *** PID END ***

    // save off the current thinkers
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
	if (th->function.acp1 == (actionf_p1)P_MobjThinker)
	{
// *** PID BEGIN ***
// Don't save pid monsters, as the process list will be different next
// time someone restores the game.
            mobj_t	*temp_mobj = NULL;
            temp_mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, (void*)0);
            memcpy(temp_mobj, th, sizeof(*mobj));
            // if this is a pid mobj, print status msg and don't save
            if ( temp_mobj->m_pid != 0 ){
               fprintf(stderr, "   not saving pid %d\n", temp_mobj->m_pid);
               Z_Free(temp_mobj);
               continue;  // get next mobj, this one's a pid monster.
            } 
            Z_Free(temp_mobj);
// *** PID END ***

	    *save_p++ = tc_mobj;
	    PADSAVEP();
	    mobj = (mobj_t *)save_p;
	    memcpy(mobj, th, sizeof(*mobj));
	    save_p += sizeof(*mobj);
	    mobj->state = (state_t *)(mobj->state - states);

	    if (mobj->player)
		mobj->player = (player_t *)((mobj->player-players) + 1);
	    continue;
	}

	// I_Error("P_ArchiveThinkers: Unknown thinker function");
    }

    // add a terminating marker
    *save_p++ = tc_end;
}

//
// P_UnArchiveThinkers
//
void P_UnArchiveThinkers(void)
{
    byte		tclass;
    thinker_t		*currentthinker;
    thinker_t		*next;
    mobj_t		*mobj;

// *** PID BEGIN ***
// Print status message.
    fprintf(stderr, "***** restore thinkers: *****\n");

// Remove all pid monsters from the pid linked list before unallocating them.
// Do this by calling cleanup_pid_list().
    cleanup_pid_list(NULL);
// *** PID END ***

    // remove all the current thinkers
    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
	next = currentthinker->next;

	if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
	    P_RemoveMobj((mobj_t *)currentthinker);
	else
	    Z_Free(currentthinker);

	currentthinker = next;
    }
    P_InitThinkers();

    // read in saved thinkers
    while (1)
    {
	tclass = *save_p++;
	switch (tclass)
	{
	  case tc_end:
// *** PID BEGIN ***
// We want to get to the statement after the while loop, so we have
// to get to the end of the switch first.
	    break;      // out of switch statement
// old code:
//	    return; 	// end of list
// *** PID END ***

	  case tc_mobj:
	    PADSAVEP();
	    mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, (void *)0);
	    memcpy(mobj, save_p, sizeof(*mobj));
	    save_p += sizeof(*mobj);
	    mobj->state = &states[(int)mobj->state];
	    mobj->target = NULL;
	    if (mobj->player)
	    {
		mobj->player = &players[(int)mobj->player - 1];
		mobj->player->mo = mobj;
	    }
	    P_SetThingPosition(mobj);
	    mobj->info = &mobjinfo[mobj->type];
	    mobj->floorz = mobj->subsector->sector->floorheight;
	    mobj->ceilingz = mobj->subsector->sector->ceilingheight;
	    mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
	    P_AddThinker(&mobj->thinker);
	    P_AddMobjToList(mobj);
	    break;

	  default:
	    I_Error("Unknown tclass %i in savegame", tclass);
	}
// *** PID BEGIN ***
// Check tclass again here (at the end of the switch stmt) to break
// the while loop if we're at the end of the thinkers.
       if ( tclass == tc_end ){
          break;   // out of the while loop
       }
// *** PID END ***

    }

// *** PID BEGIN ***
// Now that all the thinkers are reloaded from the savegame, add the
// pid monsters.  Also mark them for deletion next time cleanup_pid_list()
// is called, unless they validate themselves through pr_check() in the
// meantime.
   pr_check();
   cleanup_pid_list(NULL);
// *** PID END ***

}

//
// P_ArchiveSpecials
//
enum
{
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_flicker,
    tc_field,
    tc_sldoor,
    tc_scroll,
    tc_endspecials
} specials_e;

//
// Things to handle:
//
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
// T_Flicker
// T_ForceField
// T_SlidingDoor
// T_Scroll
//
void P_ArchiveSpecials(void)
{
    thinker_t		*th;
    ceiling_t		*ceiling;
    vldoor_t		*door;
    floormove_t		*floor;
    plat_t		*plat;
    lightflash_t	*flash;
    strobe_t		*strobe;
    glow_t		*glow;
    fireflicker_t	*fire;
    forcefield_t	*field;
    slidedoor_t		*sldoor;
    scroll_t		*scroll;

    // save off the current thinkers
    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
	if (th->function.acv == (actionf_v)NULL)
	{
	    platlist_t		*pl;
	    ceilinglist_t	*cl;

	    for (pl = activeplats; pl; pl = pl->next)
		if (pl->plat == (plat_t *) th)
		    goto plat;

	    for (cl = activeceilings; cl; cl = cl->next)
		if (cl->ceiling == (ceiling_t *) th)
		    goto ceiling;

	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_MoveCeiling)
	{
ceiling:
	    *save_p++ = tc_ceiling;
	    PADSAVEP();
	    ceiling = (ceiling_t *)save_p;
	    memcpy(ceiling, th, sizeof(*ceiling));
	    save_p += sizeof(*ceiling);
	    ceiling->sector = (sector_t *)(ceiling->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_VerticalDoor)
	{
	    *save_p++ = tc_door;
	    PADSAVEP();
	    door = (vldoor_t *)save_p;
	    memcpy(door, th, sizeof(*door));
	    save_p += sizeof(*door);
	    door->sector = (sector_t *)(door->sector - sectors);
	    door->line = (line_t *)(door->line ? door->line - lines : -1);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_MoveFloor)
	{
	    *save_p++ = tc_floor;
	    PADSAVEP();
	    floor = (floormove_t *)save_p;
	    memcpy(floor, th, sizeof(*floor));
	    save_p += sizeof(*floor);
	    floor->sector = (sector_t *)(floor->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_PlatRaise)
	{
plat:
	    *save_p++ = tc_plat;
	    PADSAVEP();
	    plat = (plat_t *)save_p;
	    memcpy(plat, th, sizeof(*plat));
	    save_p += sizeof(*plat);
	    plat->sector = (sector_t *)(plat->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_LightFlash)
	{
	    *save_p++ = tc_flash;
	    PADSAVEP();
	    flash = (lightflash_t *)save_p;
	    memcpy(flash, th, sizeof(*flash));
	    save_p += sizeof(*flash);
	    flash->sector = (sector_t *)(flash->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
	{
	    *save_p++ = tc_strobe;
	    PADSAVEP();
	    strobe = (strobe_t *)save_p;
	    memcpy(strobe, th, sizeof(*strobe));
	    save_p += sizeof(*strobe);
	    strobe->sector = (sector_t *)(strobe->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_Glow)
	{
	    *save_p++ = tc_glow;
	    PADSAVEP();
	    glow = (glow_t *)save_p;
	    memcpy(glow, th, sizeof(*glow));
	    save_p += sizeof(*glow);
	    glow->sector = (sector_t *)(glow->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_FireFlicker)
	{
	    *save_p++ = tc_flicker;
	    PADSAVEP();
	    fire = (fireflicker_t *)save_p;
	    memcpy(fire, th, sizeof(*fire));
	    save_p += sizeof(*fire);
	    fire->sector = (sector_t *)(fire->sector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_ForceField)
	{
	    *save_p++ = tc_field;
	    PADSAVEP();
	    field = (forcefield_t *)save_p;
	    memcpy(field, th, sizeof(*field));
	    save_p += sizeof(*field);
	    field->frontsector = (sector_t *)(field->frontsector - sectors);
	    field->backsector = (sector_t *)(field->backsector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_SlidingDoor)
	{
	    *save_p++ = tc_sldoor;
	    PADSAVEP();
	    sldoor = (slidedoor_t*)save_p;
	    memcpy(sldoor, th, sizeof(*sldoor));
	    save_p += sizeof(*sldoor);
	    sldoor->frontsector = (sector_t *)(sldoor->frontsector - sectors);
	    sldoor->backsector = (sector_t *)(sldoor->backsector - sectors);
	    continue;
	}

	if (th->function.acp1 == (actionf_p1)T_Scroll)
	{
	    *save_p++ = tc_scroll;
	    PADSAVEP();
	    scroll = (scroll_t *)save_p;
	    memcpy(scroll, th, sizeof(*scroll));
	    save_p += sizeof(*scroll);
	    continue;
	}
    }

    // add a terminating marker
    *save_p++ = tc_endspecials;
}

//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials(void)
{
    byte		tclass;
    ceiling_t		*ceiling;
    vldoor_t		*door;
    floormove_t		*floor;
    plat_t		*plat;
    lightflash_t	*flash;
    strobe_t		*strobe;
    glow_t		*glow;
    fireflicker_t	*fire;
    forcefield_t	*field;
    slidedoor_t		*sldoor;
    scroll_t		*scroll;

    // read in saved thinkers
    while (1)
    {
	tclass = *save_p++;
	switch (tclass)
	{
	  case tc_endspecials:
	    return;	// end of list

	  case tc_ceiling:
	    PADSAVEP();
	    ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, (void *)0);
	    memcpy(ceiling, save_p, sizeof(*ceiling));
	    save_p += sizeof(*ceiling);
	    ceiling->sector = &sectors[(int)ceiling->sector];
	    ceiling->sector->ceilingdata = ceiling;

	    if (ceiling->thinker.function.acp1)
		ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

	    P_AddThinker(&ceiling->thinker);
	    P_AddActiveCeiling(ceiling);
	    break;

	  case tc_door:
	    PADSAVEP();
	    door = Z_Malloc(sizeof(*door), PU_LEVEL, (void *)0);
	    memcpy(door, save_p, sizeof(*door));
	    save_p += sizeof(*door);
	    door->sector = &sectors[(int)door->sector];
	    door->line = (int)door->line != -1 ? &lines[(int)door->line] :
			 (line_t *)0;
	    door->sector->ceilingdata = door;
	    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
	    P_AddThinker(&door->thinker);
	    break;

	  case tc_floor:
	    PADSAVEP();
	    floor = Z_Malloc(sizeof(*floor), PU_LEVEL, (void *)0);
	    memcpy(floor, save_p, sizeof(*floor));
	    save_p += sizeof(*floor);
	    floor->sector = &sectors[(int)floor->sector];
	    floor->sector->floordata = floor;
	    floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
	    P_AddThinker(&floor->thinker);
	    break;

	  case tc_plat:
	    PADSAVEP();
	    plat = Z_Malloc(sizeof(*plat), PU_LEVEL, (void *)0);
	    memcpy(plat, save_p, sizeof(*plat));
	    save_p += sizeof(*plat);
	    plat->sector = &sectors[(int)plat->sector];
	    plat->sector->floordata = plat;

	    if (plat->thinker.function.acp1)
		plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

	    P_AddThinker(&plat->thinker);
	    P_AddActivePlat(plat);
	    break;

	  case tc_flash:
	    PADSAVEP();
	    flash = Z_Malloc(sizeof(*flash), PU_LEVEL, (void *)0);
	    memcpy(flash, save_p, sizeof(*flash));
	    save_p += sizeof(*flash);
	    flash->sector = &sectors[(int)flash->sector];
	    flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
	    P_AddThinker(&flash->thinker);
	    break;

	  case tc_strobe:
	    PADSAVEP();
	    strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, (void *)0);
	    memcpy(strobe, save_p, sizeof(*strobe));
	    save_p += sizeof(*strobe);
	    strobe->sector = &sectors[(int)strobe->sector];
	    strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
	    P_AddThinker(&strobe->thinker);
	    break;

	  case tc_glow:
	    PADSAVEP();
	    glow = Z_Malloc(sizeof(*glow), PU_LEVEL, (void *)0);
	    memcpy(glow, save_p, sizeof(*glow));
	    save_p += sizeof(*glow);
	    glow->sector = &sectors[(int)glow->sector];
	    glow->thinker.function.acp1 = (actionf_p1)T_Glow;
	    P_AddThinker(&glow->thinker);
	    break;

	  case tc_flicker:
	    PADSAVEP();
	    fire = Z_Malloc(sizeof(*fire), PU_LEVEL, (void *)0);
	    memcpy(fire, save_p, sizeof(*fire));
	    save_p += sizeof(*fire);
	    fire->sector = &sectors[(int)fire->sector];
	    fire->thinker.function.acp1 = (actionf_p1)T_FireFlicker;
	    P_AddThinker(&fire->thinker);
	    break;

	  case tc_field:
	    PADSAVEP();
	    field = Z_Malloc(sizeof(*field), PU_LEVEL, (void *)0);
	    memcpy(field, save_p, sizeof(*field));
	    save_p += sizeof(*field);
	    field->frontsector = &sectors[(int)field->frontsector];
	    field->backsector = &sectors[(int)field->backsector];
	    field->thinker.function.acp1 = (actionf_p1)T_ForceField;
	    P_AddThinker(&field->thinker);
	    break;

	  case tc_sldoor:
	    PADSAVEP();
	    sldoor = Z_Malloc(sizeof(*sldoor), PU_LEVEL, (void *)0);
	    memcpy(sldoor, save_p, sizeof(*sldoor));
	    save_p += sizeof(*sldoor);
	    sldoor->frontsector = &sectors[(int)sldoor->frontsector];
	    sldoor->backsector = &sectors[(int)sldoor->backsector];
	    sldoor->thinker.function.acp1 = (actionf_p1)T_SlidingDoor;
	    P_AddThinker(&sldoor->thinker);
	    break;

	  case tc_scroll:
	    PADSAVEP();
	    scroll = Z_Malloc(sizeof(*scroll), PU_LEVEL, (void *)0);
	    memcpy(scroll, save_p, sizeof(*scroll));
	    save_p += sizeof(*scroll);
	    scroll->thinker.function.acp1 = (actionf_p1)T_Scroll;
	    P_AddThinker(&scroll->thinker);
	    break;

	  default:
	    I_Error("P_UnarchiveSpecials:Unknown tclass %i "
		    "in savegame", tclass);
	}
    }
}
