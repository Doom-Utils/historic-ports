//
// DOSDoom Definition File Codes (Switch textures)
//
// By the DOSDoom Team
//
// Switch Texture Setup and Parser Code
//

#include "d_debug.h"
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "p_local.h"
#include "p_spec.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

switchlist_t bufferswitch;

// -ACB- 1998/08/10 Use DDF_MainGetLumpName for getting the..lump name.
commandlist_t switchcommands[] =
 {{"SWITCHON"           , DDF_MainGetLumpName,     &bufferswitch.name1},
  {"SWITCHOFF"          , DDF_MainGetLumpName,     &bufferswitch.name2},
  {"SWITCHSOUND"        , DDF_MainLookupSound,     &bufferswitch.sfx},
  {COMMAND_TERMINATOR   , NULL,                    NULL}};

switchlist_t*  alphSwitchList = NULL;

static int            numSwitches = 0;
static int            maxSwitches = 48;

//
// DDF_SWCreateSwitch
// Added a switch into the list.
//
static void DDF_SWCreateSwitch(void)
{
 // Resize the switch list on overflow
 if (numSwitches == (maxSwitches - 1))
   alphSwitchList = realloc(alphSwitchList, sizeof(*alphSwitchList) * ++maxSwitches);

 memcpy(&alphSwitchList[numSwitches++], &bufferswitch, sizeof(*alphSwitchList));
 memset (&alphSwitchList[numSwitches], 0 , sizeof(*alphSwitchList));
 memset (&bufferswitch,0,sizeof(bufferswitch)); // clear the buffer
}

void DDF_ReadSW(void *data, int size)
{
#ifdef DEVELOPERS
  int i;
#endif
  readinfo_t switches;

  if (!data)
  {
    switches.message               = "DDF_InitSwitches";
    switches.filename              = "switch.ddf";
    switches.memfile = NULL;
  } else {
    switches.message = NULL;
    switches.memfile = data;
    switches.memsize = size;
    switches.filename = NULL;
  }
  switches.DDF_MainCheckName     = DDF_DummyFunction; // no need to check name
  switches.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  switches.DDF_MainCreateEntry   = DDF_SWCreateSwitch;
  switches.DDF_MainFinishingCode = DDF_SWCreateSwitch;
  switches.cmdlist               = switchcommands;

  DDF_MainReadFile(&switches);

#ifdef DEVELOPERS // -ACB- 1998/09/07 Debugging aid
   Debug_Printf("Switch List:\n");
  
   for (i=0; i<numSwitches; i++)
   {
     Debug_Printf("No: %d  ON: '%s'  OFF: '%s'\n",
                   i, alphSwitchList[i].name1, alphSwitchList[i].name2);
   }
#endif
}
void DDF_SWInit()
{
  // Allocate original space
  alphSwitchList = malloc(sizeof(*alphSwitchList) * maxSwitches);
  memset(alphSwitchList, 0, sizeof(*alphSwitchList));

  DDF_ReadSW(NULL, 0);
}



