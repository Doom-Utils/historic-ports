//
// DOSDoom Definition File Codes (Game settings)
//
// By the DOSDoom Team
//
// Overall Game Setup and Parser Code
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

static void DDF_GameGetPic(char* info, int c);
static void DDF_GameGetFrames(char *info, int c);
static void DDF_GameGetMap(char *info, int c);

wi_map_t* wi_maps = NULL;
int wi_nummaps = 0;

static wi_map_t buffer_wi;
static wi_anim_t buffer_anim;
static wi_frame_t buffer_frame;
static wi_map_t* replace = NULL;
commandlist_t wicommands[] =
 {{"BACKGROUND"           , DDF_MainGetLumpName,     &buffer_wi.background},
  {"SPLAT"                , DDF_MainGetLumpName,     &buffer_wi.splatpic},
  {"YAH1"                 , DDF_MainGetLumpName,     &buffer_wi.yah[0]},
  {"YAH2"                 , DDF_MainGetLumpName,     &buffer_wi.yah[1]},
  {"PERCENT SOUND"        , DDF_MainLookupSound,     &buffer_wi.percent},
  {"DONE SOUND"           , DDF_MainLookupSound,     &buffer_wi.done},
  {"ENDMAP SOUND"         , DDF_MainLookupSound,     &buffer_wi.endmap},
  {"NEXTMAP SOUND"        , DDF_MainLookupSound,     &buffer_wi.nextmap},
  {"ANIM"                 , DDF_GameGetFrames,       &buffer_frame},
  {"MUSIC"                , DDF_MainGetMusicName,    &buffer_wi.music},
  {"MAP"                  , DDF_GameGetMap,          NULL},
  {"FIRSTMAP"             , DDF_MainGetLumpName,     &buffer_wi.firstmap},
  {"GRAPHIC NAME"         , DDF_MainGetLumpName,     &buffer_wi.graphicname},
  {"TITLE MUSIC"          , DDF_MainGetMusicName,     &buffer_wi.titlemusic},
  {"TITLE PIC"            , DDF_GameGetPic,          &buffer_wi},
  {"TITLE TIME"           , DDF_MainGetTime,         &buffer_wi.titletics},
  {COMMAND_TERMINATOR     , NULL,                    NULL}};

static void DDF_GameAddFrame(void)
{
  buffer_anim.frames = realloc(buffer_anim.frames,
                               (buffer_anim.numframes + 1) * sizeof(wi_frame_t));
  memcpy(&buffer_anim.frames[buffer_anim.numframes++], &buffer_frame, sizeof(wi_frame_t));
  memset(&buffer_frame, 0, sizeof(wi_frame_t));
}

static void DDF_GameAddAnim(void)
{
  buffer_wi.anims = realloc(buffer_wi.anims,
                            (buffer_wi.numanims + 1) * sizeof(wi_anim_t));
  if (buffer_anim.level[0])
    buffer_anim.type = WI_LEVEL;
  else
    buffer_anim.type = WI_NORMAL;
  memcpy(&buffer_wi.anims[buffer_wi.numanims++], &buffer_anim, sizeof(wi_anim_t));
  memset(&buffer_anim, 0, sizeof(wi_anim_t));
}

static void DDF_GameGetFrames(char *info, int c)
{
  char* s = strdup(info);
  char* tok;
  wi_frame_t* f = (wi_frame_t*) wicommands[c].data;
  struct
  {
    int type;
    void* dest;
  } f_dest[] = {
    {1, f->pic},
    {0, &f->tics},
    {0, &f->pos.x},
    {2, &f->pos.y}
  };
  int i;
  tok = strtok(s, ":");

  if (tok[0] == '#')
  {
    if (!buffer_anim.numframes)
    {
      strncpy(buffer_anim.level, tok + 1, 8);
      tok = strtok(NULL, ":");
    } else if (!strncmp(tok, "#END", 4))
    {
      DDF_GameAddAnim();
      free(s);
      return;
    } else
      I_Error("\nInvalid # command '%s'\n", tok);
  }

  for (i = 0;
         tok && i < 4;
         tok = strtok(NULL, ":"), i++)
  {
    if (f_dest[i].type & 1)
      strncpy(f_dest[i].dest, tok, 8);
    else
      *(int *) f_dest[i].dest = atoi(tok);
    if (f_dest[i].type & 2)
    {
      DDF_GameAddFrame();
      free(s);
      return;
    }
  }
  I_Error("\nBad Frame command '%s'\n", info);
}

static void DDF_GameGetMap(char *info, int c)
{
  char* s = strdup(info);
  char* tok;
  struct
  {
    int type;
    void* dest;
  } dest[3];
  int i;

  buffer_wi.mappos = realloc(buffer_wi.mappos, sizeof(mappos_t) * (buffer_wi.nummaps + 1));
  buffer_wi.mapdone = realloc(buffer_wi.mapdone, sizeof(boolean) * (buffer_wi.nummaps + 1));

  dest[0].type = 1; dest[0].dest = buffer_wi.mappos[buffer_wi.nummaps].name;
  dest[1].type = 0; dest[1].dest = &buffer_wi.mappos[buffer_wi.nummaps].pos.x;
  dest[2].type = 2; dest[2].dest = &buffer_wi.mappos[buffer_wi.nummaps++].pos.y;
  for (tok = strtok(s, ":"), i = 0;
       tok && i < 3;
       tok = strtok(NULL, ":"), i++)
  {
    if (dest[i].type & 1)
      strncpy(dest[i].dest, tok, 8);
    else
      *(int *) dest[i].dest = atoi(tok);
    if (dest[i].type & 2)
    {
      free(s);
      return;
    }
  }
  I_Error("\nBad Map command '%s'\n", info);
}

static void DDF_GameGetPic(char* info, int c)
{
  buffer_wi.titlepics = realloc(buffer_wi.titlepics, sizeof(char *) * (buffer_wi.numtitlepics + 1));
  buffer_wi.titlepics[buffer_wi.numtitlepics] = strdup(info);
  buffer_wi.numtitlepics++;
}

static void DDF_GameCheckName(char* info)
{
  int i;
  replace = NULL;
  for (i = 0 ; i < wi_nummaps; i++)
  {
     if (!strcmp(info, wi_maps[i].name))
     {
       if (ddf_replace)
         replace = &wi_maps[i];
       else
         I_Error("\nMission '%s' already declared\n", info);
     }
  }
  buffer_wi.name = strdup(info);
}

wi_map_t* DDF_GameGetName(char* name)
{
  int i;
  for (i = 0 ; i < wi_nummaps; i++)
  {
     if (!strcmp(name, wi_maps[i].name))
       return &wi_maps[i];
  }
  I_Error("DDF_GameGetMission: '%s' not declared\n", name);
  return NULL;
}

void DDF_GameCreate_WI(void)
{
  if (!replace)
  {
    wi_maps = realloc(wi_maps, sizeof(wi_map_t) * (wi_nummaps + 1));
    memcpy(&wi_maps[wi_nummaps++], &buffer_wi, sizeof(wi_map_t));
  } else
    memcpy(replace, &buffer_wi, sizeof(wi_map_t));

  memset(&buffer_wi, 0, sizeof(wi_map_t));
  memset(&buffer_anim, 0, sizeof(wi_anim_t));
  memset(&buffer_frame, 0, sizeof(wi_frame_t));
}

void DDF_ReadGames(void *data, int size)
{
  readinfo_t games;

  if (!data)
  {
    games.message               = "DDF_InitIntermissions";
    games.filename              = "mission.ddf";
    games.memfile = NULL;
  } else {
    games.message = NULL;
    games.memfile = data;
    games.memsize = size;
    games.filename = NULL;
  }
  games.DDF_MainCheckName     = DDF_GameCheckName;
  games.DDF_MainCheckCmd      = DDF_MainCheckCommand;
  games.DDF_MainCreateEntry   = DDF_GameCreate_WI;
  games.DDF_MainFinishingCode = DDF_GameCreate_WI;
  games.cmdlist               = wicommands;

  memset(&buffer_wi, 0, sizeof(wi_map_t));
  memset(&buffer_anim, 0, sizeof(wi_anim_t));
  memset(&buffer_frame, 0, sizeof(wi_frame_t));

  DDF_MainReadFile(&games);
}

void DDF_GameInit()
{
  DDF_ReadGames(NULL, 0);
}

