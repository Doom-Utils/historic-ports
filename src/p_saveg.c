/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: p_saveg.c,v 1.11 1999/10/31 11:52:23 cphipps Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Archiving: SaveGame I/O.
 *
 *-----------------------------------------------------------------------------*/

static const char
rcsid[] = "$Id: p_saveg.c,v 1.11 1999/10/31 11:52:23 cphipps Exp $";

#include "doomstat.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_saveg.h"
#include "m_random.h"
#include "am_map.h"
#include "p_enemy.h"
#include "lprintf.h"

byte *save_p;

// Pads save_p to a 4-byte boundary
//  so that the load/save works on SGI&Gecko.
#define PADSAVEP()    do { save_p += (4 - ((int) save_p & 3)) & 3; } while (0)
//
// P_ArchivePlayers
//
void P_ArchivePlayers (void)
{
  int i;

  CheckSaveGame(sizeof(player_t) * MAXPLAYERS); // killough
  for (i=0 ; i<MAXPLAYERS ; i++)
    if (playeringame[i])
      {
        int      j;
        player_t *dest;

        PADSAVEP();
        dest = (player_t *) save_p;
        memcpy(dest, &players[i], sizeof(player_t));
        save_p += sizeof(player_t);
        for (j=0; j<NUMPSPRITES; j++)
          if (dest->psprites[j].state)
            dest->psprites[j].state =
              (state_t *)(dest->psprites[j].state-states);
      }
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers (void)
{
  int i;

  for (i=0 ; i<MAXPLAYERS ; i++)
    if (playeringame[i])
      {
        int j;

        PADSAVEP();

        memcpy(&players[i], save_p, sizeof(player_t));
        save_p += sizeof(player_t);

        // will be set when unarc thinker
        players[i].mo = NULL;
        players[i].message = NULL;
        players[i].attacker = NULL;

        for (j=0 ; j<NUMPSPRITES ; j++)
          if (players[i]. psprites[j].state)
            players[i]. psprites[j].state =
              &states[ (int)players[i].psprites[j].state ];
      }
}


//
// P_ArchiveWorld
//
void P_ArchiveWorld (void)
{
  int            i;
  const sector_t *sec;
  const line_t   *li;
  short          *put;
  const side_t   *si;

  // killough 3/22/98: fix bug caused by hoisting save_p too early
  {
    size_t size;
    
    // CPhipps - MBF changes, compatibility optioned
    if (compatibility_level < lxdoom_1_compatibility) {
      // CPhipps - fix size for Boom v2.02 soundtarget saving (one short extra per sector)
      size = (sizeof(short)*8 /*7*/)*numsectors + (sizeof(short)*3)*numlines + 4;
    } else {
      // killough 10/98: adjust size for changes below
      size = (sizeof(short)*5 + sizeof sec->floorheight + sizeof sec->ceilingheight) 
	* numsectors + sizeof(short)*3*numlines + 4;
    }
    
    for (i=0; i<numlines; i++)
      {
	// CPhipps - sidedef size
#define SIDEDEF_SIZE ((compatibility_level < lxdoom_1_compatibility) ? (sizeof(short)*5) : \
(sizeof(short)*3 + sizeof si->textureoffset + sizeof si->rowoffset))
        if (lines[i].sidenum[0] != -1)
          size += SIDEDEF_SIZE;
        if (lines[i].sidenum[1] != -1)
          size += SIDEDEF_SIZE;
      }

    CheckSaveGame(size); // killough
  }

  PADSAVEP();                // killough 3/22/98

  put = (short *)save_p;

  // do sectors
  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {
      // CPhipps - MBF more accurate savegame data, compatibility protected
      if (compatibility_level < lxdoom_1_compatibility) {
	*put++ = sec->floorheight >> FRACBITS;
	*put++ = sec->ceilingheight >> FRACBITS;
      } else {
	// killough 10/98: save full floor & ceiling heights, including fraction
        memcpy(put, &sec->floorheight, sizeof sec->floorheight);
        put = (void *)((char *) put + sizeof sec->floorheight);
        memcpy(put, &sec->ceilingheight, sizeof sec->ceilingheight);
        put = (void *)((char *) put + sizeof sec->ceilingheight);
      }
      *put++ = sec->floorpic;
      *put++ = sec->ceilingpic;
      *put++ = sec->lightlevel;
      *put++ = sec->special;            // needed?   yes -- transfer types
      *put++ = sec->tag;                // needed?   need them -- killough

      // phares 9/13/98: Save the index of the thinker, so that sound
      // traces can survive savegames. The prev pointer in the thinker
      // list has been mapped to an index in P_ThinkerToIndex().

      if (compatibility_level < lxdoom_1_compatibility) {
	if (sec->soundtarget)
	  *put++ = (long) sec->soundtarget->thinker.prev;
	else
	  *put++ = 0; // no soundtarget
      }
    }

  // do lines
  for (i=0, li = lines ; i<numlines ; i++,li++)
    {
      int j;

      *put++ = li->flags;
      *put++ = li->special;
      *put++ = li->tag;

      for (j=0; j<2; j++)
        if (li->sidenum[j] != -1)
          {
            si = &sides[li->sidenum[j]];
	    
	    // CPhipps - MBF more accurate savegame data, compatibility protected
	    if (compatibility_level < lxdoom_1_compatibility) {
	      *put++ = si->textureoffset >> FRACBITS;
	      *put++ = si->rowoffset >> FRACBITS;
	    } else {
              // killough 10/98: save full sidedef offsets,
              // preserving fractional scroll offsets 
              memcpy(put, &si->textureoffset, sizeof si->textureoffset);
              put = (void *)((char *) put + sizeof si->textureoffset);
              memcpy(put, &si->rowoffset, sizeof si->rowoffset);
              put = (void *)((char *) put + sizeof si->rowoffset);
	    }
            *put++ = si->toptexture;
            *put++ = si->bottomtexture;
            *put++ = si->midtexture;
          }
    }
  save_p = (byte *) put;
}



//
// P_UnArchiveWorld
//
void P_UnArchiveWorld (void)
{
  int          i;
  sector_t     *sec;
  line_t       *li;
  const short  *get;

  PADSAVEP();                // killough 3/22/98

  get = (short *) save_p;

  // do sectors
  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {
      if (compatibility_level < lxdoom_1_compatibility) {
	sec->floorheight = *get++ << FRACBITS;
	sec->ceilingheight = *get++ << FRACBITS;
      } else {
        // killough 10/98: load full floor & ceiling heights, including fractions
  
        memcpy(&sec->floorheight, get, sizeof sec->floorheight);
        get = (void *)((char *) get + sizeof sec->floorheight);
        memcpy(&sec->ceilingheight, get, sizeof sec->ceilingheight);
        get = (void *)((char *) get + sizeof sec->ceilingheight);
      }
      sec->floorpic = *get++;
      sec->ceilingpic = *get++;
      sec->lightlevel = *get++;
      sec->special = *get++;
      sec->tag = *get++;
      sec->ceilingdata = 0; //jff 2/22/98 now three thinker fields, not two
      sec->floordata = 0;
      sec->lightingdata = 0;

      // phares 9/13/98: soundtarget has meaning, to save sound info across
      // savegames.
      // CPhipps - make MBF savegame compatible
      if (compatibility_level < lxdoom_1_compatibility)
	sec->soundtarget = (mobj_t *) ((long) *get++); // just get the index
                                                     // for now. convert it
                                                     // later, in
                                                     // P_UnArchiveThinkers.
    }

  // do lines
  for (i=0, li = lines ; i<numlines ; i++,li++)
    {
      int j;

      li->flags = *get++;
      li->special = *get++;
      li->tag = *get++;
      for (j=0 ; j<2 ; j++)
        if (li->sidenum[j] != -1)
          {
            side_t *si = &sides[li->sidenum[j]];
	    if (compatibility_level < lxdoom_1_compatibility) {
	      si->textureoffset = *get++ << FRACBITS;
	      si->rowoffset = *get++ << FRACBITS;
	    } else {
              // killough 10/98: load full sidedef offsets, including fractions
              memcpy(&si->textureoffset, get, sizeof si->textureoffset);
              get = (void *)((char *) get + sizeof si->textureoffset);
              memcpy(&si->rowoffset, get, sizeof si->rowoffset);
              get = (void *)((char *) get + sizeof si->rowoffset);
 	    }
            si->toptexture = *get++;
            si->bottomtexture = *get++;
            si->midtexture = *get++;
          }
    }
  save_p = (byte *) get;
}

//
// Thinkers
//

typedef enum {
  tc_end,
  tc_mobj
} thinkerclass_t;

// phares 9/13/98: Moved this code outside of P_ArchiveThinkers so the
// thinker indices could be used by the code that saves sector info.

static int number_of_thinkers;

void P_ThinkerToIndex(void)
  {
  thinker_t *th;

  // killough 2/14/98:
  // count the number of thinkers, and mark each one with its index, using
  // the prev field as a placeholder, since it can be restored later.

  number_of_thinkers = 0;
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function.acp1 == (actionf_p1) P_MobjThinker)
      th->prev = (thinker_t *) ++number_of_thinkers;
  }

// phares 9/13/98: Moved this code outside of P_ArchiveThinkers so the
// thinker indices could be used by the code that saves sector info.

void P_IndexToThinker(void)
  {
  // killough 2/14/98: restore prev pointers
  thinker_t *th;
  thinker_t *prev = &thinkercap;

  for (th = thinkercap.next ; th != &thinkercap ; prev=th, th=th->next)
    th->prev = prev;
  }

// CPhipps - amount of mobj that we save
const size_t mobjsize = sizeof(mobj_t) - sizeof(int);

//
// P_ArchiveThinkers
//
// 2/14/98 killough: substantially modified to fix savegame bugs

void P_ArchiveThinkers (void)
{
  thinker_t *th;

  CheckSaveGame(sizeof brain);      // killough 3/26/98: Save boss brain state
  memcpy(save_p, &brain, sizeof brain);
  save_p += sizeof brain;

  // check that enough room is available in savegame buffer
  CheckSaveGame(number_of_thinkers*(mobjsize+4));       // killough 2/14/98

  // save off the current thinkers
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function.acp1 == (actionf_p1) P_MobjThinker)
      {
        mobj_t *mobj;

        *save_p++ = tc_mobj;
        PADSAVEP();
        mobj = (mobj_t *)save_p;
        memcpy (mobj, th, mobjsize);
        save_p += mobjsize;
        mobj->state = (state_t *)(mobj->state - states);

        // killough 2/14/98: convert pointers into indices.
        // Fixes many savegame problems, by properly saving
        // target and tracer fields. Note: we store NULL if
        // the thinker pointed to by these fields is not a
        // mobj thinker.

        if (mobj->target)
          mobj->target = mobj->target->thinker.function.acp1 ==
            (actionf_p1) P_MobjThinker ?
            (mobj_t *) mobj->target->thinker.prev : NULL;

        if (mobj->tracer)
          mobj->tracer = mobj->tracer->thinker.function.acp1 ==
            (actionf_p1) P_MobjThinker ?
            (mobj_t *) mobj->tracer->thinker.prev : NULL;

        // killough 2/14/98: new field: save last known enemy. Prevents
        // monsters from going to sleep after killing monsters and not
        // seeing player anymore.

        if (mobj->lastenemy)
          mobj->lastenemy = mobj->lastenemy->thinker.function.acp1 ==
            (actionf_p1) P_MobjThinker ?
            (mobj_t *) mobj->lastenemy->thinker.prev : NULL;

        // killough 2/14/98: end changes

        if (mobj->above_thing)                                      // phares
          mobj->above_thing = mobj->above_thing->thinker.function.acp1 ==
            (actionf_p1) P_MobjThinker ?
            (mobj_t *) mobj->above_thing->thinker.prev : NULL;

        if (mobj->below_thing)
          mobj->below_thing = mobj->below_thing->thinker.function.acp1 ==
            (actionf_p1) P_MobjThinker ?
            (mobj_t *) mobj->below_thing->thinker.prev : NULL;      // phares

        if (mobj->player)
          mobj->player = (player_t *)((mobj->player-players) + 1);
      }

  // add a terminating marker
  *save_p++ = tc_end;

  // killough 9/14/98: save soundtargets
  if (compatibility_level >= lxdoom_1_compatibility) {
    int i;
    CheckSaveGame(numsectors * mobjsize);       // killough 9/14/98
    for (i = 0; i < numsectors; i++)
      {
	mobj_t *target = sectors[i].soundtarget;
	if (target)
	  target = (mobj_t *) target->thinker.prev;
	memcpy(save_p, &target, sizeof target);
	save_p += sizeof target;
      }
  }
  
}

static void P_SetNewTarget(mobj_t**p, mobj_t* v)
{
  *p = NULL; P_SetTarget(p, v);
}

//
// P_UnArchiveThinkers
//
// 2/14/98 killough: substantially modified to fix savegame bugs
//

void P_UnArchiveThinkers (void)
{
  thinker_t *th;
  mobj_t    **mobj_p;    // killough 2/14/98: Translation table
  size_t    size;        // killough 2/14/98: size of or index into table

  // killough 3/26/98: Load boss brain state
  memcpy(&brain, save_p, sizeof brain);
  save_p += sizeof brain;

  // remove all the current thinkers
  for (th = thinkercap.next; th != &thinkercap; )
    {
      thinker_t *next = th->next;
      if (th->function.acp1 == (actionf_p1) P_MobjThinker)
        P_RemoveMobj ((mobj_t *) th);
      else
        Z_Free (th);
      th = next;
    }
  P_InitThinkers ();

  // killough 2/14/98: count number of thinkers by skipping through them
  {
    byte *sp = save_p;     // save pointer and skip header
    for (size = 1; *save_p++ == tc_mobj; size++)  // killough 2/14/98
      {                     // skip all entries, adding up count
        PADSAVEP();
        save_p += mobjsize;
      }

    if (*--save_p != tc_end)
      I_Error ("Unknown tclass %i in savegame", *save_p);

    // first table entry special: 0 maps to NULL
    *(mobj_p = malloc(size * sizeof (*mobj_p))) = 0;   // table of pointers
    save_p = sp;           // restore save pointer
  }

  // read in saved thinkers
  for (size = 1; *save_p++ == tc_mobj; size++)    // killough 2/14/98
    {
      mobj_t *mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);

      // killough 2/14/98 -- insert pointers to thinkers into table, in order:
      mobj_p[size] = mobj;

      PADSAVEP();
      memcpy (mobj, save_p, mobjsize);
      save_p += mobjsize;
      mobj->references = 0;
      mobj->state = states + (int) mobj->state;

      if (mobj->player)
        (mobj->player = &players[(int) mobj->player - 1]) -> mo = mobj;

      P_SetThingPosition (mobj);
      mobj->info = &mobjinfo[mobj->type];

      // killough 2/28/98:
      // Fix for falling down into a wall after savegame loaded:
      //      mobj->floorz = mobj->subsector->sector->floorheight;
      //      mobj->ceilingz = mobj->subsector->sector->ceilingheight;

      mobj->thinker.function.acp1 = (actionf_p1) P_MobjThinker;
      P_AddThinker (&mobj->thinker);
    }

  // killough 2/14/98: adjust target and tracer fields, plus
  // lastenemy field, to correctly point to mobj thinkers.
  // NULL entries automatically handled by first table entry.

  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
      mobj_t* pm = (mobj_t*)th;
      P_SetNewTarget(&pm->target, mobj_p[(size_t)pm->target]);
      P_SetNewTarget(&pm->tracer, mobj_p[(size_t)pm->tracer]);
      P_SetNewTarget(&pm->lastenemy, mobj_p[(size_t)pm->lastenemy]);
      // phares: added two new fields for Sprite Height problem
      P_SetNewTarget(&pm->above_thing, mobj_p[(size_t)pm->above_thing]);
      P_SetNewTarget(&pm->below_thing, mobj_p[(size_t)pm->below_thing]);
    }

  if (compatibility_level < lxdoom_1_compatibility) {
    int       i;           // phares 9/13/98:   For sec->soundtarget restore
    sector_t* sec;         // phares 9/13/98:   For sec->soundtarget restore
    // phares 9/13/98: Restore sec->soundtarget pointers from indices.
    // NULL entries automatically handled by first table entry.
    for (i = 0, sec = sectors ; i < numsectors ; i++, sec++)
      P_SetNewTarget(&sec->soundtarget, mobj_p[(size_t) sec->soundtarget]);

  } else {  // killough 9/14/98: restore soundtargets
    int i;
    for (i = 0; i < numsectors; i++) {
      mobj_t *target;
      memcpy(&target, save_p, sizeof target);
      save_p += sizeof target;
      P_SetNewTarget(&sectors[i].soundtarget, mobj_p[(size_t) target]);
    }
  }

  free(mobj_p);    // free translation table

  // killough 3/26/98: Spawn icon landings:
  if (gamemode == commercial)
    P_SpawnBrainTargets();
}

//
// P_ArchiveSpecials
//
enum {
  tc_ceiling,
  tc_door,
  tc_floor,
  tc_plat,
  tc_flash,
  tc_strobe,
  tc_flicker,     //jff 8/8/98 add missing fire flicker entry
  tc_glow,
  tc_elevator,    //jff 2/22/98 new elevator type thinker                 
  tc_scroll,      // killough 3/7/98: new scroll effect thinker
  tc_friction,    // phares 3/18/98:  new friction effect thinker
  tc_pusher,      // phares 3/22/98:  new push/pull effect thinker
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
// T_MoveElevator, (plat_t: sector_t *), - active list      // jff 2/22/98
// T_Scroll                                                 // killough 3/7/98
// T_Friction                                               // phares 3/18/98
// T_Pusher                                                 // phares 3/22/98
//

void P_ArchiveSpecials (void)
{
  thinker_t *th;
  size_t    size = 0;          // killough

  // save off the current thinkers (memory size calculation -- killough)

  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function.acv == (actionf_v)NULL)
      {
        platlist_t *pl;
        ceilinglist_t *cl;     //jff 2/22/98 need this for ceilings too now
        for (pl=activeplats; pl; pl=pl->next)
          if (pl->plat == (plat_t *) th)   // killough 2/14/98
            {
              size += 4+sizeof(plat_t);
              goto end;
            }
        for (cl=activeceilings; cl; cl=cl->next) // search for activeceiling
          if (cl->ceiling == (ceiling_t *) th)   //jff 2/22/98
            {
              size += 4+sizeof(ceiling_t);
              goto end;
            }
      end:;
      }
    else
      size +=
        th->function.acp1==(actionf_p1)T_MoveCeiling  ? 4+sizeof(ceiling_t) :
        th->function.acp1==(actionf_p1)T_VerticalDoor ? 4+sizeof(vldoor_t)  :
        th->function.acp1==(actionf_p1)T_MoveFloor    ? 4+sizeof(floormove_t):
        th->function.acp1==(actionf_p1)T_PlatRaise    ? 4+sizeof(plat_t)    :
        th->function.acp1==(actionf_p1)T_LightFlash   ? 4+sizeof(lightflash_t):
        th->function.acp1==(actionf_p1)T_StrobeFlash  ? 4+sizeof(strobe_t)  :
        //jff 8/8/98 add missing fire flicker special
        th->function.acp1==(actionf_p1)T_FireFlicker  ? 4+sizeof(fireflicker_t) :
        th->function.acp1==(actionf_p1)T_Glow         ? 4+sizeof(glow_t)    :
        th->function.acp1==(actionf_p1)T_MoveElevator ? 4+sizeof(elevator_t):
        th->function.acp1==(actionf_p1)T_Scroll       ? 4+sizeof(scroll_t)  :
        th->function.acp1==(actionf_p1)T_Friction     ? 4+sizeof(friction_t):
        th->function.acp1==(actionf_p1)T_Pusher       ? 4+sizeof(pusher_t)  :
      0;

  CheckSaveGame(size);          // killough

  // save off the current thinkers
  for (th=thinkercap.next; th!=&thinkercap; th=th->next)
    {
      if (th->function.acv == (actionf_v)NULL)
        {
          platlist_t *pl;
          ceilinglist_t *cl;    //jff 2/22/98 add iter variable for ceilings

          // killough 2/8/98: fix plat original height bug.
          // Since acv==NULL, this could be a plat in stasis.
          // so check the active plats list, and save this
          // plat (jff: or ceiling) even if it is in stasis.

          for (pl=activeplats; pl; pl=pl->next)
            if (pl->plat == (plat_t *) th)      // killough 2/14/98
              goto plat;

          for (cl=activeceilings; cl; cl=cl->next)
            if (cl->ceiling == (ceiling_t *) th)      //jff 2/22/98
              goto ceiling;

          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_MoveCeiling)
        {
          ceiling_t *ceiling;
        ceiling:                               // killough 2/14/98
          *save_p++ = tc_ceiling;
          PADSAVEP();
          ceiling = (ceiling_t *)save_p;
          memcpy (ceiling, th, sizeof(*ceiling));
          save_p += sizeof(*ceiling);
          ceiling->sector = (sector_t *)(ceiling->sector - sectors);
          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_VerticalDoor)
        {
          vldoor_t *door;
          *save_p++ = tc_door;
          PADSAVEP();
          door = (vldoor_t *) save_p;
          memcpy (door, th, sizeof *door);
          save_p += sizeof(*door);
          door->sector = (sector_t *)(door->sector - sectors);
          //jff 1/31/98 archive line remembered by door as well
          door->line = (line_t *) (door->line ? door->line-lines : -1);
          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_MoveFloor)
        {
          floormove_t *floor;
          *save_p++ = tc_floor;
          PADSAVEP();
          floor = (floormove_t *)save_p;
          memcpy (floor, th, sizeof(*floor));
          save_p += sizeof(*floor);
          floor->sector = (sector_t *)(floor->sector - sectors);
          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_PlatRaise)
        {
          plat_t *plat;
        plat:   // killough 2/14/98: added fix for original plat height above
          *save_p++ = tc_plat;
          PADSAVEP();
          plat = (plat_t *)save_p;
          memcpy (plat, th, sizeof(*plat));
          save_p += sizeof(*plat);
          plat->sector = (sector_t *)(plat->sector - sectors);
          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_LightFlash)
        {
          lightflash_t *flash;
          *save_p++ = tc_flash;
          PADSAVEP();
          flash = (lightflash_t *)save_p;
          memcpy (flash, th, sizeof(*flash));
          save_p += sizeof(*flash);
          flash->sector = (sector_t *)(flash->sector - sectors);
          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_StrobeFlash)
        {
          strobe_t *strobe;
          *save_p++ = tc_strobe;
          PADSAVEP();
          strobe = (strobe_t *)save_p;
          memcpy (strobe, th, sizeof(*strobe));
          save_p += sizeof(*strobe);
          strobe->sector = (sector_t *)(strobe->sector - sectors);
          continue;
        }

      //jff 8/8/98 add missing fire flicker special
      if (th->function.acp1 == (actionf_p1) T_FireFlicker)
        {
          fireflicker_t *flick;
          *save_p++ = tc_flicker;
          PADSAVEP();
          flick = (fireflicker_t *)save_p;
          memcpy (flick, th, sizeof(*flick));
          save_p += sizeof(*flick);
          flick->sector = (sector_t *)(flick->sector - sectors);
          continue;
        }

      if (th->function.acp1 == (actionf_p1) T_Glow)
        {
          glow_t *glow;
          *save_p++ = tc_glow;
          PADSAVEP();
          glow = (glow_t *)save_p;
          memcpy (glow, th, sizeof(*glow));
          save_p += sizeof(*glow);
          glow->sector = (sector_t *)(glow->sector - sectors);
          continue;
        }

      //jff 2/22/98 new case for elevators
      if (th->function.acp1 == (actionf_p1) T_MoveElevator)
        {
          elevator_t *elevator;         //jff 2/22/98
          *save_p++ = tc_elevator;
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
          continue;
        }

      // killough 3/7/98: Scroll effect thinkers
      if (th->function.acp1 == (actionf_p1) T_Scroll)
        {
          *save_p++ = tc_scroll;
          memcpy (save_p, th, sizeof(scroll_t));
          save_p += sizeof(scroll_t);
          continue;
        }

      // phares 3/18/98: Friction effect thinkers

      if (th->function.acp1 == (actionf_p1) T_Friction)
        {
          *save_p++ = tc_friction;
          memcpy (save_p, th, sizeof(friction_t));
          save_p += sizeof(friction_t);
          continue;
        }

      // phares 3/22/98: Push/Pull effect thinkers

      if (th->function.acp1 == (actionf_p1) T_Pusher)
        {
          *save_p++ = tc_pusher;
          memcpy (save_p, th, sizeof(pusher_t));
          save_p += sizeof(pusher_t);
          continue;
        }
    }

  // add a terminating marker
  *save_p++ = tc_endspecials;
}


//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials (void)
{
  byte tclass;

  // read in saved thinkers
  while ((tclass = *save_p++) != tc_endspecials)  // killough 2/14/98
    switch (tclass)
      {
      case tc_ceiling:
        PADSAVEP();
        {
          ceiling_t *ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
          memcpy (ceiling, save_p, sizeof(*ceiling));
          save_p += sizeof(*ceiling);
          ceiling->sector = &sectors[(int)ceiling->sector];
          ceiling->sector->ceilingdata = ceiling; //jff 2/22/98

          if (ceiling->thinker.function.acp1)
            ceiling->thinker.function.acp1 = (actionf_p1) T_MoveCeiling;

          P_AddThinker (&ceiling->thinker);
          P_AddActiveCeiling(ceiling);
          break;
        }

      case tc_door:
        PADSAVEP();
        {
          vldoor_t *door = Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
          memcpy (door, save_p, sizeof(*door));
          save_p += sizeof(*door);
          door->sector = &sectors[(int)door->sector];

          //jff 1/31/98 unarchive line remembered by door as well
          door->line = (int)door->line!=-1? &lines[(int)door->line] : NULL;

          door->sector->ceilingdata = door;       //jff 2/22/98
          door->thinker.function.acp1 = (actionf_p1) T_VerticalDoor;
          P_AddThinker (&door->thinker);
          break;
        }

      case tc_floor:
        PADSAVEP();
        {
          floormove_t *floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);
          memcpy (floor, save_p, sizeof(*floor));
          save_p += sizeof(*floor);
          floor->sector = &sectors[(int)floor->sector];
          floor->sector->floordata = floor; //jff 2/22/98
          floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
          P_AddThinker (&floor->thinker);
          break;
        }

      case tc_plat:
        PADSAVEP();
        {
          plat_t *plat = Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
          memcpy (plat, save_p, sizeof(*plat));
          save_p += sizeof(*plat);
          plat->sector = &sectors[(int)plat->sector];
          plat->sector->floordata = plat; //jff 2/22/98

          if (plat->thinker.function.acp1)
            plat->thinker.function.acp1 = (actionf_p1) T_PlatRaise;

          P_AddThinker (&plat->thinker);
          P_AddActivePlat(plat);
          break;
        }

      case tc_flash:
        PADSAVEP();
        {
          lightflash_t *flash = Z_Malloc (sizeof(*flash), PU_LEVEL, NULL);
          memcpy (flash, save_p, sizeof(*flash));
          save_p += sizeof(*flash);
          flash->sector = &sectors[(int)flash->sector];
          flash->thinker.function.acp1 = (actionf_p1) T_LightFlash;
          P_AddThinker (&flash->thinker);
          break;
        }

      case tc_strobe:
        PADSAVEP();
        {
          strobe_t *strobe = Z_Malloc (sizeof(*strobe), PU_LEVEL, NULL);
          memcpy (strobe, save_p, sizeof(*strobe));
          save_p += sizeof(*strobe);
          strobe->sector = &sectors[(int)strobe->sector];
          strobe->thinker.function.acp1 = (actionf_p1) T_StrobeFlash;
          P_AddThinker (&strobe->thinker);
          break;
        }

      //jff 8/8/98 add missing flicker special
      case tc_flicker:
        PADSAVEP();
        {
          fireflicker_t *flick = Z_Malloc (sizeof(*flick), PU_LEVEL, NULL);
          memcpy (flick, save_p, sizeof(*flick));
          save_p += sizeof(*flick);
          flick->sector = &sectors[(int)flick->sector];
          flick->thinker.function.acp1 = (actionf_p1) T_FireFlicker;
          P_AddThinker (&flick->thinker);
          break;
        }

      case tc_glow:
        PADSAVEP();
        {
          glow_t *glow = Z_Malloc (sizeof(*glow), PU_LEVEL, NULL);
          memcpy (glow, save_p, sizeof(*glow));
          save_p += sizeof(*glow);
          glow->sector = &sectors[(int)glow->sector];
          glow->thinker.function.acp1 = (actionf_p1) T_Glow;
          P_AddThinker (&glow->thinker);
          break;
        }

        //jff 2/22/98 new case for elevators
      case tc_elevator:
        PADSAVEP();
        {
          elevator_t *elevator = Z_Malloc (sizeof(*elevator), PU_LEVEL, NULL);
          memcpy (elevator, save_p, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = &sectors[(int)elevator->sector];
          elevator->sector->floordata = elevator; //jff 2/22/98
          elevator->sector->ceilingdata = elevator; //jff 2/22/98
          elevator->thinker.function.acp1 = (actionf_p1) T_MoveElevator;
          P_AddThinker (&elevator->thinker);
          break;
        }

      case tc_scroll:       // killough 3/7/98: scroll effect thinkers
        {
          scroll_t *scroll = Z_Malloc (sizeof(scroll_t), PU_LEVEL, NULL);
          memcpy (scroll, save_p, sizeof(scroll_t));
          save_p += sizeof(scroll_t);
          scroll->thinker.function.acp1 = (actionf_p1) T_Scroll;
          P_AddThinker(&scroll->thinker);
          break;
        }

      case tc_friction:   // phares 3/18/98: new friction effect thinkers
        {
          friction_t *friction = Z_Malloc (sizeof(friction_t), PU_LEVEL, NULL);
          memcpy (friction, save_p, sizeof(friction_t));
          save_p += sizeof(friction_t);
          friction->thinker.function.acp1 = (actionf_p1) T_Friction;
          P_AddThinker(&friction->thinker);
          break;
        }

      case tc_pusher:   // phares 3/22/98: new Push/Pull effect thinkers
        {
          pusher_t *pusher = Z_Malloc (sizeof(pusher_t), PU_LEVEL, NULL);
          memcpy (pusher, save_p, sizeof(pusher_t));
          save_p += sizeof(pusher_t);
          pusher->thinker.function.acp1 = (actionf_p1) T_Pusher;
          pusher->source = P_GetPushThing(pusher->affectee);
          P_AddThinker(&pusher->thinker);
          break;
        }

      default:
        I_Error ("P_UnarchiveSpecials:Unknown tclass %i "
                 "in savegame",tclass);
      }
}

// killough 2/16/98: save/restore random number generator state information
// CPhipps - fixed compatibility with Boom v2.02

void P_ArchiveRNG(void)
{
  CheckSaveGame(sizeof rng);
  if (compatibility_level >= lxdoom_1_compatibility) {
    memcpy(save_p, &rng, sizeof rng);
    save_p += sizeof rng;
  } else {
    // Copy all the Boom v2.02 RNG classes
    memcpy(save_p, &rng, (pr_all_in_one+1)*sizeof(rng.seed[0]));
    save_p += (pr_all_in_one+1)*sizeof(rng.seed[0]);
    // Copy the rest of the struct
    memcpy(save_p, &(rng.rndindex), sizeof(rng) - sizeof(rng.seed));
    save_p += sizeof(rng) - sizeof(rng.seed);
  }
}

void P_UnArchiveRNG(void)
{
  if (compatibility_level >= lxdoom_1_compatibility) {
    memcpy(&rng, save_p, sizeof rng);
    save_p += sizeof rng;
  } else {
    memset(&rng, 0, sizeof(rng));
    memcpy(&rng, save_p, (pr_all_in_one+1)*sizeof(rng.seed[0]));
    save_p += (pr_all_in_one+1)*sizeof(rng.seed[0]);
    memcpy(&(rng.rndindex), save_p, sizeof(rng) - sizeof(rng.seed));
    save_p += sizeof(rng) - sizeof(rng.seed);    
  }
}

// killough 2/22/98: Save/restore automap state
void P_ArchiveMap(void)
{
  int zero = 0, one = 1;
  CheckSaveGame(2 * sizeof zero + sizeof markpointnum +
                markpointnum * sizeof *markpoints +
                sizeof automapmode + sizeof one);

  memcpy(save_p, &automapmode, sizeof automapmode);
  save_p += sizeof automapmode;
  memcpy(save_p, &one, sizeof one);   // CPhipps - used to be viewactive, now
  save_p += sizeof one;               // that's worked out locally by D_Display
  memcpy(save_p, &zero, sizeof zero); // CPhipps - used to be followplayer
  save_p += sizeof zero;              //  that is now part of automapmode
  memcpy(save_p, &zero, sizeof zero); // CPhipps - used to be automap_grid, ditto
  save_p += sizeof zero;
  memcpy(save_p, &markpointnum, sizeof markpointnum);
  save_p += sizeof markpointnum;

  if (markpointnum)
    {
      memcpy(save_p, markpoints, sizeof *markpoints * markpointnum);
      save_p += markpointnum * sizeof *markpoints;
    }
}

void P_UnArchiveMap(void)
{
  int unused;
  memcpy(&automapmode, save_p, sizeof automapmode);
  save_p += sizeof automapmode;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;

  if (automapmode & am_active)
    AM_Start();

  memcpy(&markpointnum, save_p, sizeof markpointnum);
  save_p += sizeof markpointnum;

  if (markpointnum)
    {
      while (markpointnum >= markpointnum_max)
        markpoints = realloc(markpoints, sizeof *markpoints *
         (markpointnum_max = markpointnum_max ? markpointnum_max*2 : 16));
      memcpy(markpoints, save_p, markpointnum * sizeof *markpoints);
      save_p += markpointnum * sizeof *markpoints;
    }
}

//----------------------------------------------------------------------------
//
// $Log: p_saveg.c,v $
// Revision 1.11  1999/10/31 11:52:23  cphipps
// Include lprintf.h for I_Error
//
// Revision 1.10  1999/10/17 09:35:14  cphipps
// Fixed hanging else(s)
//
// Revision 1.9  1999/10/12 13:01:13  cphipps
// Changed header to GPL
//
// Revision 1.8  1999/08/31 19:48:09  cphipps
// Removed old viewactive variable, replaced with 'true' in savegames
//
// Revision 1.7  1999/03/26 11:10:25  cphipps
// Removed a couple of depreciated automap mode variables from game saving, replaced by 0's
//
// Revision 1.6  1999/03/07 22:16:36  cphipps
// Change automap state variable name
//
// Revision 1.5  1999/02/04 15:31:53  cphipps
// Changes for mobj pointer reference counting like MBF.
// Also changed mobj saving to remain compatible with old savegames.
//
// Revision 1.4  1999/02/04 10:31:06  cphipps
// Use new target-setting macros to enable pointer reference counting
//
// Revision 1.3  1998/12/28 15:32:23  cphipps
// Import MBF savegame improvements for lxdoom_1_compatibility
// Fix potential buffer overrun in saving boom v2.02 savegames
//
// Revision 1.2  1998/10/27 18:33:26  cphipps
// Import Boom v2.02 source
//
// Revision 1.19  1998/09/16  06:59:39  phares
// Save soundtarget across savegames
//
// Revision 1.18  1998/08/08  15:18:53  jim
// flicker special restora
//
// Revision 1.17  1998/05/03  23:10:22  killough
// beautification
//
// Revision 1.16  1998/04/19  01:16:06  killough
// Fix boss brain spawn crashes after loadgames
//
// Revision 1.15  1998/03/28  18:02:17  killough
// Fix boss spawner savegame crash bug
//
// Revision 1.14  1998/03/23  15:24:36  phares
// Changed pushers to linedef control
//
// Revision 1.13  1998/03/23  03:29:54  killough
// Fix savegame crash caused in P_ArchiveWorld
//
// Revision 1.12  1998/03/20  00:30:12  phares
// Changed friction to linedef control
//
// Revision 1.11  1998/03/09  07:20:23  killough
// Add generalized scrollers
//
// Revision 1.10  1998/03/02  12:07:18  killough
// fix stuck-in wall loadgame bug, automap status
//
// Revision 1.9  1998/02/24  08:46:31  phares
// Pushers, recoil, new friction, and over/under work
//
// Revision 1.8  1998/02/23  04:49:42  killough
// Add automap marks and properties to saved state
//
// Revision 1.7  1998/02/23  01:02:13  jim
// fixed elevator size, comments
//
// Revision 1.4  1998/02/17  05:43:33  killough
// Fix savegame crashes and monster sleepiness
// Save new RNG info
// Fix original plats height bug
//
// Revision 1.3  1998/02/02  22:17:55  jim
// Extended linedef types
//
// Revision 1.2  1998/01/26  19:24:21  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:07  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
