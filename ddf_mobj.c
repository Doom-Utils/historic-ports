//
// DOSDoom Definition File Codes (MOBJ)
//
// By the DOSDoom Team
//
// Moving Object Setup and Parser Code
//
// -ACB- 1998/08/04 Written.
// -ACB- 1998/09/12 Use DDF_MainGetFixed for fixed number references.
// -ACB- 1998/09/13 Use DDF_MainGetTime for Time count references.
// -KM- 1998/11/25 Translucency is now a fixed_t. Fixed spelling of available.
// -KM- 1998/12/16 No limit on number of ammo types.
//

#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "z_zone.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

void DDF_MobjGetItemType(char *info, int commandref);
void DDF_MobjGetBpAmmo(char *info, int commandref);
void DDF_MobjGetBpAmmoLimit(char *info, int commandref);
void DDF_MobjGetBpArmour(char *info, int commandref);
void DDF_MobjGetBpKeys(char *info, int commandref);
void DDF_MobjGetBpWeapon(char *info, int commandref);

extern commandlist_t *thingcommands;

boolean backpackreq = false; // used for checking if we create backpackinfo

char *itemtype[NUMOFBENEFITS] =
{
  "KEY BLUECARD",  "KEY REDCARD",  "KEY YELLOWCARD",
  "KEY BLUESKULL", "KEY REDSKULL", "KEY YELLOWSKULL",

  "POWERUP ACIDSUIT",     "POWERUP ARMOUR",       "POWERUP AUTOMAP",
  "POWERUP BACKPACK",     "POWERUP BERSERK",      "POWERUP HEALTH",
  "POWERUP HEALTHARMOUR", "POWERUP INVULNERABLE", "POWERUP JETPACK",
  "POWERUP LIGHTGOGGLES", "POWERUP NIGHTVISION",  "POWERUP PARTINVIS",
};
//
// DDF_MobjLookup
//
// Looks an mobjinfo by name, returns a fatal error if it does not exist.
//
mobjinfo_t* DDF_MobjLookup(const char* refname)
{
  mobjinfo_t* currinfo = NULL;
  mobjinfo_t* refinfo = NULL;

  currinfo = mobjinfohead;

  while (currinfo != NULL && refinfo == NULL)
  {
    if (currinfo->name != NULL && !(stricmp(currinfo->name,refname)))
      refinfo = currinfo;

    currinfo = currinfo->next;
  }

  if (refinfo == NULL)
    I_Error("DDF_MobjLookup: '%s' does not exist",refname);

  return refinfo;
}

//
// DDF_MobjCheckStates
//
// Check through the states on an mobj and attempts to dereference any
// state destination subflags.
//
void DDF_MobjCheckStates()
{
  int i,j,k;
  state_t *currstate;

  i=j=k=0;

  // Sprite Derefencing

  //                                                                           |
  //                                                                           |
  // First Loop: cycle through the available state values in buffermobj        |
  //                                                                          \|/
  while (strcmp(framedestlist[i].redirector,COMMAND_TERMINATOR))
  {
    j = *framedestlist[i].state;

    // Can be ignored as the state value is not used.
    if (j == 0 || j == S_NULL)
    {
      i++;      // remember to increment
      continue;
    }

    // get the current state, get from state ref (j).
    currstate = &states[j];

    //
    // Second Loop: keep looping while the nextstate value is great than j,
    //              the reason for this is that when the nextstate value is
    //              less than j, we have come to the last in the order of     |
    //              states.                                                   |
    //                                                                       \|/
    while (currstate->nextstate > j)
    {
      k=0;

      //
      // Third Loop: Loop through the list of redirectors and compare the
      //             flag against the nextstate, if the return is true, we
      //             have found a director flag and it requires to be
      //             dereferenced.                                           |
      //                                                                    \|/
      while (strcmp(framedestlist[k].redirector,COMMAND_TERMINATOR) &&
            (!(currstate->nextstate & framedestlist[k].subflag)))
      {
        k++;
      }
      //  end of loop three                                              /* /|\ */
      //                                                                     |

      // if strcmp returns a value we didn't reach the end of a list,
      // therefore we found a director flag.
      if (strcmp(framedestlist[k].redirector,COMMAND_TERMINATOR))
      {
        // lose the director flag and store any offset value in j.
        j = currstate->nextstate - framedestlist[k].subflag;

        // confused? you will be... The nextstate value is now
        // got from the value in ref in frame destination list, if
        // the flag was FDF_SPAWN, then the value is from
        // buffermobj.spawnstate plus the offset value in j and so on.
        currstate->nextstate = (*framedestlist[k].state) + j;
        j = 0;
      }
      else
      {
        // No director flag found: Increment j to next frame, since only
        // increments of one are used if no director flag is present.
        j++;
        currstate = &states[j];
      }

    }
    //                                                                    /* /|\ */
    // End of second loop                                                     |
    //                                                                        |

    i++;  // increment count
  }
  //                                                                       /* /|\ */
  // End of third loop                                                         |
  //                                                                           |
  //                                                                           |

}

//
// DDF_MobjGetItemType
// Reference the benefit type, check it exists and then store.
//
void DDF_MobjGetItemType(char *info, int commandref)
{
  int i,j;

  j = -1;

  for (i = 0; i < NUMOFBENEFITS; i++)
  {
   if (!strcmp(info,itemtype[i]))
     j = i;
  }

  if (j == -1)
  {
    // -KM- 1998/11/25 Check for weapon type.
    j = DDF_WeaponGetType(info);
    if (j == -1)
    {
      // -KM- 1998/12/16 Eliminate ammo types.
      if (!strncmp(info, "AMMO TYPE", 9))
      {
        j = atoi(info + 9);
        buffermobj.benefittype = AMMO_TYPE;
        buffermobj.benefitammo = j - 1;
        return;
      } else
        I_Error("\nDDF_ItemGetType: Unknown Type: '%s'\n", info);
    }
    buffermobj.benefittype = WEAPON;
    buffermobj.benefitweapon = j;
    return;
  }

  buffermobj.benefittype = j;

  return;
}

//
// DDF_MobjGetBpAmmo
//
// Looks for the first ammo type with ammo value of -1 and then gives
// the value from the string to it, if there is no ammo type with a value of -1
// returns an error
//
void DDF_MobjGetBpAmmo(char *info, int commandref)
{
  int i,j;

  i = atoi(info); // straight conversion, no messin'

  j=0;

  if (!buffpack.ammo)
  {
    buffpack.ammo = malloc(sizeof(int) * NUMAMMO);
    memset(buffpack.ammo, -1, sizeof(int) * NUMAMMO);
  }

  // search for first type with -1 or the end of the types (NUMAMMO)
  while (buffpack.ammo[j] != -1 && j != NUMAMMO) j++;

  if (j == NUMAMMO)
    I_Error("\n\tDDF_ItemGetBpAmmo: No more ammo types to give ammo to\n");
  else
    buffpack.ammo[j] = i;

  backpackreq = true; // we need to create backpackinfo when we make a mobjinfo entry
}

//
// DDF_MobjGetBpAmmoLimit
//
// Looks for the first ammo type with ammolimit value of -1 and then gives
// the value from the string to it, if there is no ammo type with a value of -1
// returns an error
//
void DDF_MobjGetBpAmmoLimit(char *info, int commandref)
{
  int i,j;

  i = atoi(info); // straight conversion, no messin'

  j=0;

  if (!buffpack.ammolimit)
  {
    buffpack.ammolimit = malloc(sizeof(int) * NUMAMMO);
    memset(buffpack.ammolimit, -1, sizeof(int) * NUMAMMO);
  }

  // search for first type with -1 or the end of the types (NUMAMMO)
  while (buffpack.ammolimit[j] != -1 && j != NUMAMMO) j++;

  if (j == NUMAMMO)
    I_Error("\n\tDDF_ItemGetBpAmmoLimit: No more ammo types to give ammolimit to\n");
  else
    buffpack.ammolimit[j] = i;

  backpackreq = true; // we need to create backpackinfo when we make a mobjinfo entry
}

//
// DDF_ItemGetBpArmour
//
// Get a numeric value for the armour, if above 100, class = Blue; else it is Green
//
void DDF_MobjGetBpArmour(char *info, int commandref)
{
  int i;

  i = atoi(info); // straight conversion, no messin'

  if (i > 100)
    buffpack.armourclass = BLUEARMOUR;
  else
    buffpack.armourclass = GREENARMOUR;

  buffpack.armour = i;

  backpackreq = true; // we need to create backpackinfo when we make a mobjinfo entry
}

//
// DDF_MobjGetBpKeys
//
// Use a local lookup table to compare with the string to put the keys into backpack
//
void DDF_MobjGetBpKeys(char *info, int commandref)
{
  int i;

  // Local LUT, I see no reason for this to be global
  char *keynames[NUMCARDS+1] = {"BLUECARD","YELLOWCARD","REDCARD",
                                "BLUESKULL","YELLOWSKULL","REDSKULL",
                                COMMAND_TERMINATOR };
  i = 0;

  while (strcmp(info,keynames[i]) && strcmp(keynames[i],COMMAND_TERMINATOR)) i++;

  if (!strcmp(keynames[i],COMMAND_TERMINATOR))
    I_Error("\n\tDDF_ItemGetBpKeys: No such key name\n");
  else
    buffpack.cards[i] = true;

  backpackreq = true; // we need to create backpackinfo when we make a mobjinfo entry
}

//
// DDF_MobjGetBpWeapon
//
// Gets the number from info and checks you are that many types.
//
void DDF_MobjGetBpWeapon(char *info, int commandref)
{
  int i;

  i = DDF_WeaponGetType(info);
  if (i == -1)
    i = atoi(info);

  if (!buffpack.weapons)
  {
    buffpack.weapons = malloc(sizeof(int)*numweapons);
    memset(buffpack.weapons, 0, sizeof(int) * numweapons);
  }

  if (i < 0 || i >= numweapons)
    I_Error("DDF_ItemGetBpKeys: No such weapon type '%d'\n", i);

  buffpack.weapons[i] = true;

  backpackreq = true; // we need to create backpackinfo when we make a mobjinfo entry
}

void DDF_MobjCreateThing(void)
{
  int i;
  backpack_t* backpack;
  backpack_t* searchbackpack;
  mobjinfo_t* entry;
  mobjinfo_t* newentry;
  mobjinfo_t* next;

  DDF_MobjCheckStates();

  entry = mobjinfohead;

  while (entry->next != bufferreplacemobj)
    entry = entry->next;

  if (bufferreplacemobj)
  {
    newentry = bufferreplacemobj;
    next = bufferreplacemobj->next;
  } else {
    newentry = malloc(sizeof(mobjinfo_t));

    if (newentry == NULL)
      I_Error("\n\tDDF_MobjCreateItem: Malloc failed\n");

    entry->next = newentry;
    next = NULL;
  }

  memcpy(newentry,&buffermobj,sizeof(mobjinfo_t));
  newentry->next = next;

  if (!newentry->mass)
    newentry->mass = 100;

  if (!newentry->spawnhealth)
    newentry->spawnhealth = 1000;

  if (!newentry->respawntime)
    newentry->respawntime = 30*TICRATE;

  // Backpack info given, create space for the info and give the pointer to mobjinfo
  if (backpackreq)
  {
    newentry->backpackinfo = malloc ( sizeof(backpack_t) );
    backpack = newentry->backpackinfo;

    if (backpack == NULL)
      I_Error("\n\tDDF_MobjCreateItem: Unable to alocate memory for backpack");

    backpack->ammo = buffpack.ammo;
    backpack->ammolimit = buffpack.ammolimit;

    backpack->armour = buffpack.armour;
    backpack->armourclass = buffpack.armourclass;

    // -KM- 1998/11/25 Dynamic weapon allocation.
    backpack->weapons = buffpack.weapons;

    for (i=0; i<NUMCARDS; i++)
    {
      if (buffpack.cards[i])
        backpack->cards[i] = true;
    }

    // -ACB- 1998/09/14 Add the backpack to the list..
    backpack->next = NULL;

    if (!backpackhead)
    {
      backpackhead = backpack;
    }
    else
    {
      searchbackpack = backpackhead;

      while (searchbackpack->next != NULL)
        searchbackpack = searchbackpack->next;

      searchbackpack->next = backpack;
    }

  }

  memset (&buffermobj,0,sizeof(mobjinfo_t)); // clear the buffer mobj
  memset (&buffpack,0,sizeof(backpack_t));   // clear the buffer backpack

  // -KM- 1998/11/25 Transluc changed.
  buffermobj.invisibility = VISIBLE;
  buffermobj.respawntime = 12*TICRATE;
  buffermobj.fast = FRACUNIT;

  backpackreq = false;
}

void DDF_ReadThings(void* data, int size)
{
  readinfo_t things;

  memset (&buffermobj,0,sizeof(mobjinfo_t)); // clear the buffer mobj
  memset (&buffpack,0,sizeof(backpack_t));   // clear the buffer backpack

  // -KM- 1998/11/25 Transluc changed.
  buffermobj.invisibility = VISIBLE;
  buffermobj.respawntime = 12*TICRATE;
  buffermobj.fast = FRACUNIT;

  backpackreq = false;

  if (!data)
  {
    things.message               = "DDF_InitThings";
    things.filename              = "things.ddf";
    things.memfile = NULL;
  } else {
    things.message = NULL;
    things.memfile = data;
    things.memsize = size;
    things.filename = NULL;
  }
  things.DDF_MainCheckName     = DDF_MainMobjCheckName;
  things.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  things.DDF_MainCreateEntry   = DDF_MobjCreateThing;
  things.DDF_MainFinishingCode = DDF_MobjCreateThing;
  things.cmdlist               = thingcommands;
 
  DDF_MainReadFile(&things);
}

void DDF_MobjInit()
{
  DDF_ReadThings(NULL, 0);
}

