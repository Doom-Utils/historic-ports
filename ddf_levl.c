//
// DOSDoom Definition File Code (Level Defines)
//
// By the DOSDoom Team
//
// Level Setup and Parser Code
//
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_fixed.h"
#include "p_mobj.h"
#include "w_wad.h"
#include "z_zone.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation "ddf_main.h"
#endif
#include "ddf_main.h"

#include "ddf_locl.h"

static void DDF_LevelGetSpecials(char *info, int commandref);
static void DDF_LevelGetPic(char *info, int c);
static void DDF_LevelGetEpisode(char *info, int c);

static mapstuff_t* replace = NULL;

// -KM- 1998/11/25 Finales are all go.
commandlist_t levelcommands[] =
  {{"LUMPNAME"          , DDF_MainGetLumpName,     &buffermap.lump},
   {"DESCRIPTION"       , DDF_MainReferenceString, &buffermap.description},
   {"GRAPHIC NAME"      , DDF_MainGetLumpName,     &buffermap.graphicname},
   {"SKY TEXTURE"       , DDF_MainGetLumpName,     &buffermap.sky},
   {"MUSIC NAME"        , DDF_MainGetMusicName,    &buffermap.music},
   {"SURROUND FLAT"     , DDF_MainGetLumpName,     &buffermap.surround},
   {"SPECIAL"           , DDF_LevelGetSpecials,    NULL},
   {"NEXT MAP"          , DDF_MainGetString,       &buffermap.nextmapname},
   {"SECRET MAP"        , DDF_MainGetString,       &buffermap.secretmapname},
   {"AUTOTAG"           , DDF_MainGetNumeric,      &buffermap.autotag},
   {"PARTIME"           , DDF_MainGetTime,         &buffermap.partime},
   {"EPISODE"           , DDF_LevelGetEpisode,     &buffermap.interscr},
   {"ENDTEXT"           , DDF_MainGetString,       &buffermap.f[0].text},
   {"ENDTEXTBACKGROUND" , DDF_MainGetLumpName,     &buffermap.f[0].text_back},
   {"ENDTEXTFLAT"       , DDF_MainGetLumpName,     &buffermap.f[0].text_flat},
   {"ENDTEXTSPEED"      , DDF_MainGetNumeric,      &buffermap.f[0].text_speed},
   {"ENDTEXTWAIT"       , DDF_MainGetNumeric,      &buffermap.f[0].text_wait},
   {"ENDPIC"            , DDF_LevelGetPic,         &buffermap.f[0]},
   {"ENDPICWAIT"        , DDF_MainGetTime,         &buffermap.f[0].picwait},
   {"ENDCAST"           , DDF_MainGetBoolean,      &buffermap.f[0].docast},
   {"ENDBUNNY"          , DDF_MainGetBoolean,      &buffermap.f[0].dobunny},
   {"ENDMUSIC"          , DDF_MainGetMusicName,    &buffermap.f[0].music},
   {"PRETEXT"           , DDF_MainGetString,       &buffermap.f[1].text},
   {"PRETEXTBACKGROUND" , DDF_MainGetLumpName,     &buffermap.f[1].text_back},
   {"PRETEXTFLAT"       , DDF_MainGetLumpName,     &buffermap.f[1].text_flat},
   {"PRETEXTSPEED"      , DDF_MainGetNumeric,      &buffermap.f[1].text_speed},
   {"PRETEXTWAIT"       , DDF_MainGetNumeric,      &buffermap.f[1].text_wait},
   {"PREPIC"            , DDF_LevelGetPic,         &buffermap.f[1]},
   {"PREPICWAIT"        , DDF_MainGetTime,         &buffermap.f[1].picwait},
   {"PRECAST"           , DDF_MainGetBoolean,      &buffermap.f[1].docast},
   {"PREBUNNY"          , DDF_MainGetBoolean,      &buffermap.f[1].dobunny},
   {"PREMUSIC"          , DDF_MainGetMusicName,    &buffermap.f[1].music},
   {COMMAND_TERMINATOR  , NULL,                    NULL}};

specflags_t mapspecials[] =
  {{ "NO JUMPING",        MPF_NOJUMPING   },
   { "NO FREELOOK",       MPF_NOMLOOK     },
   { "NO TRANSLUCENCY",   MPF_NOTRANSLUC  },
   { "NO CHEATS",         MPF_NOCHEATS    },
   { "ITEM RESPAWN",      MPF_ITEMRESPAWN },
   { "NO ITEM RESPAWN",   MPF_NOITEMRESPN },
   { "FAST MONSTERS",     MPF_FAST        },
   { "RESURRECT RESPAWN", MPF_RESMONSTER  },
   { "TELEPORT RESPAWN",  MPF_TELMONSTER  },
   { "STRETCH SKY",       MPF_STRETCHSKY  },
   { "NORMAL SKY",        MPF_NORMALSKY   },
   { "DISABLE TRUE3D",    MPF_NOTRUE3D    },
   { "NO ENEMY STOMP",    MPF_NOSTOMP     },
   { "NORMAL BLOOD",      MPF_NORMBLOOD   },
   { "RESPAWN",           MPF_RESPAWN     },
   { "NO RESPAWN",        MPF_NORESPAWN   },
   { "NO AUTOAIM",        MPF_AUTOAIMOFF  },
   { "AUTOAIM",           MPF_AUTOAIMON   },
   { "AA MLOOK",          MPF_AUTOAIMMLOOK},
   { COMMAND_TERMINATOR,  0               }};

//
// DDF_LevelGetNewMap
//
// Changes the current map: this is globally visible to all those who ref
// ddf_main.h. It will check there is an map with that entry name exists
// and the map lump name also exists. Returns map if it exists.
//
mapstuff_t* DDF_LevelGetNewMap(const char *newmapname)
{
  mapstuff_t *checkmap;

  if (!newmapname)
    return NULL;

  checkmap = maphead;

  while (checkmap != NULL && stricmp(newmapname, checkmap->name))
    checkmap = checkmap->next;

  if (checkmap == NULL)
    return NULL;

  if (W_CheckNumForName(checkmap->lump) == -1)
    return NULL;

  return checkmap;
}

//
// DDF_LevelGetPic
//
// Addes finale pictures to the level's list.
//
void DDF_LevelGetPic(char *info, int c)
{
  finale_t* f = (finale_t*) levelcommands[c].data;
  f->pics = realloc(f->pics, 9 * (f->numpics+1));
  strncpy(f->pics + f->numpics * 9, info, 8);
  (f->numpics)++;
}

static void DDF_LevelGetEpisode(char *info, int c)
{
   *(wi_map_t**) levelcommands[c].data = DDF_GameGetName(info);
}

void DDF_LevelGetSpecials(char *info, int commandref)
{
  int i = 0;

  while (strcmp(info,mapspecials[i].name) &&
           strcmp(mapspecials[i].name,COMMAND_TERMINATOR))
  {
    i++;
  }

  if (!strcmp(mapspecials[i].name,COMMAND_TERMINATOR))
    I_Error("\n\tDDF_LevelGetSpecials: Unknown Special on Level: %s",info);

  buffermap.flags += mapspecials[i].flag;
}

void DDF_LevelCreateMap()
{
  mapstuff_t* newmap;
  mapstuff_t* currmap;

  if (replace)
  {
    newmap = replace;
    buffermap.next = replace->next;
  }
  else
  {
    newmap = malloc(sizeof(mapstuff_t));
  
    if (newmap == NULL)
      I_Error("DDF_LevelCreateMap: Malloc error");

    if (maphead == NULL)
    {
      maphead = newmap;
    }
    else
    {
      currmap = maphead;
  
      while (currmap->next != NULL)
        currmap = currmap->next;
  
      currmap->next = newmap;
    }
  }
  memcpy(newmap, &buffermap, sizeof(mapstuff_t));

  memset(&buffermap,0,sizeof(mapstuff_t));
  buffermap.f[0].text_speed = buffermap.f[1].text_speed = 3;
  buffermap.f[0].text_wait = buffermap.f[1].text_wait = 150;
}

void DDF_LevelFinishingCode()
{
  DDF_LevelCreateMap();
  currentmap = maphead; // Initial Hack
}

void DDF_LevelCheckName(char *info)
{
  mapstuff_t* entry;
#ifdef DEVELOPERS
  if (info == NULL)
    I_Error("Info has no info\n");
#endif

  entry = maphead;
  replace = NULL;

  while (entry != NULL)
  {
    if (entry->name != NULL)
    {
      if (!strcmp(info,entry->name))
      {
        if (ddf_replace)
          replace = entry;
        else
          I_Error("DDF_LevelCheckName: '%s' already declared\n",info);
      }
    }

   entry = entry->next;
  }


  if ((buffermap.name = strdup(info))==NULL)
    I_Error("DDF_LevelCheckName: Unable to allocate memory\n");

  return;
};

void DDF_ReadLevels(void *data, int size)
{
  readinfo_t levels;
  if (!data)
  {
    levels.message               = "DDF_InitLevels";
    levels.filename              = "levels.ddf";
    levels.memfile = NULL;
  } else {
    levels.message = NULL;
    levels.memfile = data;
    levels.memsize = size;
    levels.filename = NULL;
  }
  levels.DDF_MainCheckName     = DDF_LevelCheckName;
  levels.DDF_MainCreateEntry   = DDF_LevelCreateMap;
  levels.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  levels.DDF_MainFinishingCode = DDF_LevelFinishingCode;
  levels.cmdlist               = levelcommands;
 
  DDF_MainReadFile(&levels);
}

void DDF_LevelInit()
{
  memset(&buffermap,0,sizeof(mapstuff_t));

  DDF_ReadLevels(NULL, 0);
}
