//
// DOSDoom Definition File Codes (Animated textures)
//
// By the DOSDoom Team
//
// Animated Texture/Flat Setup and Parser Code
//

#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "r_local.h"
#include "p_spec.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

animdef_t bufferanim;

// -ACB- 1998/08/10 Use DDF_MainGetLumpName for getting the..lump name.
// -KM- 1998/09/27 Use DDF_MainGetTime for getting tics
commandlist_t animcommands[] =
 {{"FIRST"                , DDF_MainGetLumpName,     &bufferanim.startname},
  {"LAST"                 , DDF_MainGetLumpName,     &bufferanim.endname},
  {"TICS"                 , DDF_MainGetTime,         &bufferanim.speed},
  {"ISTEXTURE"            , DDF_MainGetNumeric,      &bufferanim.istexture},
  {COMMAND_TERMINATOR     , NULL,                    NULL}};

//
// Floor/ceiling animation sequences, defined by first and last frame,
//  i.e. the flat (64x64 tile) name or texture name to
//  be used.
//
//  The full animation sequence is given using all the flats between the start
//  and end entry, in the order found in the WAD file.
//
static int            numAnims = 0;
static int            maxAnims = 32;
animdef_t             *animdefs = NULL;
//
// DDF_AnimCreateAnim
//
static void DDF_AnimCreateAnim(void)
{
  // Resize the switch list on overflow
  if (numAnims == (maxAnims - 1))
    animdefs = realloc(animdefs, sizeof(*animdefs) * ++maxAnims);

  animdefs[numAnims++]=bufferanim;

  memset (&animdefs[numAnims], -1, sizeof(*animdefs));
  memset (&bufferanim,0,sizeof(bufferanim)); // clear the buffer
}

void DDF_ReadAnims(void *data, int size)
{
  readinfo_t anims;
  // Allocate original space
  if (!data)
  {
    anims.message               = "DDF_InitAnimations";
    anims.filename              = "anims.ddf";
    anims.memfile = NULL;
  } else {
    anims.memfile = data;
    anims.memsize = size;
    anims.message = NULL;
    anims.filename = NULL;
  }
  anims.DDF_MainCheckName     = DDF_DummyFunction;
  anims.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  anims.DDF_MainCreateEntry   = DDF_AnimCreateAnim;
  anims.DDF_MainFinishingCode = DDF_AnimCreateAnim;
  anims.cmdlist               = animcommands;

  DDF_MainReadFile(&anims);
}

void DDF_AnimInit()
{
  // Allocate original space
  animdefs = malloc(sizeof(*animdefs) * maxAnims);
  memset(animdefs, -1, sizeof(*animdefs));

  DDF_ReadAnims(NULL, 0);
}


