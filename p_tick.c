// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_tick.c,v 1.7 1998/05/15 00:37:56 killough Exp $
//
//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
// DESCRIPTION:
//      Thinker, Ticker.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: p_tick.c,v 1.7 1998/05/15 00:37:56 killough Exp $";

#include "doomstat.h"
#include "p_user.h"
#include "p_spec.h"
#include "p_tick.h"

int leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t thinkercap;

//
// P_InitThinkers
//

void P_InitThinkers(void)
{
  thinkercap.prev = thinkercap.next  = &thinkercap;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//

void P_AddThinker(thinker_t* thinker)
{
  thinkercap.prev->next = thinker;
  thinker->next = &thinkercap;
  thinker->prev = thinkercap.prev;
  thinkercap.prev = thinker;
}

//
// killough 4/25/98:
//
// The thinker's deletion has been delayed long enough, so promote its
// function to P_RemoveThinker() so that it will be deleted on the next tic.
//

void P_RemoveThinkerDelayed(thinker_t *thinker)
{
  thinker->function.acv = P_RemoveThinker;
}

//
// P_RemoveThinker
//
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
// killough 4/25/98:
//
// Instead of marking the function with -1 value cast to a function pointer,
// set the function to P_RemoveThinkerDelayed(), so that later, it will be
// promoted to P_RemoveThinker() automatically as part of the thinker process.
//

void P_RemoveThinker(thinker_t *thinker)
{
  thinker->function.acv = P_RemoveThinkerDelayed;
}

//
// P_RunThinkers
//
// killough 4/25/98:
//
// Fix deallocator to stop using "next" pointer after node has been freed
// (a Doom bug).
//
// Process each thinker. For thinkers which are marked deleted, we must
// load the "next" pointer prior to freeing the node. In Doom, the "next"
// pointer was loaded AFTER the thinker was freed, which could have caused
// crashes.
//
// But if we are not deleting the thinker, we should reload the "next"
// pointer after calling the function, in case additional thinkers are
// added at the end of the list.
//
// In P_MobjThinker(), if any mobj thinker refers indirectly to a deleted
// thinker as its target, the deleted thinker's function is changed to
// P_RemoveThinkerDelayed() so that its deletion is delayed another tic.
// This fixes some Doom crashes. killough
//

static void P_RunThinkers (void)
{
  register thinker_t *currentthinker = thinkercap.next;
  while (currentthinker != &thinkercap)
    if (currentthinker->function.acv == P_RemoveThinker)
      {
        register thinker_t *next = currentthinker->next;  // Load next pointer
        (next->prev = currentthinker->prev)->next = next; // Remove from list
        Z_Free(currentthinker);                           // Free the node
        currentthinker = next;                            // Go to next node
      }
    else
      {
        if (currentthinker->function.acp1)                // Call function
          currentthinker->function.acp1(currentthinker);  // (may insert nodes)
        currentthinker = currentthinker->next;            // Get next node
      }
}

//
// P_Ticker
//

void P_Ticker (void)
{
  int i;

  // pause if in menu and at least one tic has been run
  if (paused || (!netgame && menuactive && !demoplayback &&
                  players[consoleplayer].viewz != 1))
    return;

  for (i=0; i<MAXPLAYERS; i++)
    if (playeringame[i])
      P_PlayerThink(&players[i]);

  P_RunThinkers();
  P_UpdateSpecials();
  P_RespawnSpecials();
  leveltime++;                       // for par times
}

//----------------------------------------------------------------------------
//
// $Log: p_tick.c,v $
// Revision 1.7  1998/05/15  00:37:56  killough
// Remove unnecessary crash hack, fix demo sync
//
// Revision 1.6  1998/05/13  22:57:59  killough
// Restore Doom bug compatibility for demos
//
// Revision 1.5  1998/05/03  22:49:01  killough
// Get minimal includes at top
//
// Revision 1.4  1998/04/29  16:19:16  killough
// Fix typo causing game to not pause correctly
//
// Revision 1.3  1998/04/27  01:59:58  killough
// Fix crashes caused by thinkers being used after freed
//
// Revision 1.2  1998/01/26  19:24:32  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:01  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
