//
// DOSDoom Thinker & Ticker Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -ACB- 1998/09/14 Removed P_AllocateThinker: UNUSED
//

#include "dm_state.h"
#include "p_local.h"
#include "z_zone.h"

//
// THINKERS
//
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
//
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t thinkercap;

int leveltime;

//
// P_InitThinkers
//
void P_InitThinkers (void)
{
  thinkercap.prev = thinkercap.next = &thinkercap;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker (thinker_t* thinker)
{
  thinkercap.prev->next = thinker;
  thinker->next = &thinkercap;
  thinker->prev = thinkercap.prev;
  thinkercap.prev = thinker;
}

//
// P_RemoveThinker
//
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker (thinker_t* thinker)
{
  // FIXME: NOP.
  thinker->function.acv = (actionf_v)(-1);
}

//
// P_RunThinkers
//
void P_RunThinkers (void)
{
  thinker_t* currentthinker;

  currentthinker = thinkercap.next;

  while (currentthinker != &thinkercap)
  {
    if (currentthinker->function.acv == (actionf_v)(-1))
    {
      // time to remove it
      currentthinker->next->prev = currentthinker->prev;
      currentthinker->prev->next = currentthinker->next;
      Z_Free (currentthinker);
    }
    else
    {
      if (currentthinker->function.acp1)
        currentthinker->function.acp1 (currentthinker);
    }

    currentthinker = currentthinker->next;
  }
}

//
// P_Ticker
//
void P_Ticker (void)
{
  int i;

  if (paused)
    return;

  // pause if in menu and at least one tic has been run
  if (!netgame && menuactive && !demoplayback && players[consoleplayer].viewz != 1)
    return;

  for (i=0 ; i<maxplayers ; i++)
  {
    if (playeringame[i])
      P_PlayerThink (&players[i]);
  }

  P_RunThinkers ();
  P_UpdateSpecials ();
  P_MobjItemRespawn ();

  // for par times
  leveltime++;
}
