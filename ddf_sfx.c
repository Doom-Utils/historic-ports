//
// DOSDoom Definition Files Code (sounds)
//
// By the DOSDoom Team
//
// -KM- 1998/09/27 Finished :-)
//
#include <ctype.h>
#include <string.h>
#include "dm_defs.h"
#include "dm_type.h"
#include "ddf_locl.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_random.h"

static void DDF_SFXLookupLink(char *info, int commandref);

sfxinfo_t buffersfx=
  { "none", false,  0, 0, -1, -1, false, 0, false, 0 };
commandlist_t sfxcommands[] =
 {{"NAME"               , DDF_MainGetLumpName,     &buffersfx.name},
  {"SINGULAR"           , DDF_MainGetNumeric,      &buffersfx.singularity},
  {"PRIORITY"           , DDF_MainGetNumeric,      &buffersfx.priority},
  {"LINK"               , DDF_SFXLookupLink,       &buffersfx.link},
  {"PITCH"              , DDF_MainGetNumeric,      &buffersfx.pitch},
  {"VOLUME"             , DDF_MainGetNumeric,      &buffersfx.volume},
  {"LOOP"               , DDF_MainGetBoolean,      &buffersfx.looping},
  {"BITS"               , DDF_MainGetNumeric,      &buffersfx.bits},
  {"STEREO"             , DDF_MainGetBoolean,      &buffersfx.stereo},
  {COMMAND_TERMINATOR   , NULL,                    NULL}};

int             numsfx = 0;
static int             maxsfx = 128;
sfxinfo_t*             S_sfx;

// -KM- 1998/10/29 Done sfx_t first so structure is aligned.
bastard_sfx_t bastard_sfx[] =
 {
  { 0, "swtchn" },
  { 0, "tink" },
  { 0, "radio" },
  { 0, "oof" },
  { 0, "slop" },
  { 0, "itmbk" },
  { 0, "pstop" },
  { 0, "stnmov" },
  { 0, "pistol" },
  { 0, "swtchx" },
  { 0, "noway" },
  { 0, "bossit" },
  { 0, "bospn" },
  { 0, "bosdth" },
  { 0, "bospit" },
  { 0, "telept" },
  { 0, "boscub" },
  { 0, "pldeth" },
  { 0, "pdiehi" },
//  { 0, "sawup" },
//  { 0, "sawidl" },
//  { 0, "punch" },
//  { 0, "sawful" },
//  { 0, "sawhit" },
  { 0, "shotgn" },
//  { 0, "bfg" },
//  { 0, "dshtgn" },
//  { 0, "dbopn" },
//  { 0, "dbload" },
//  { 0, "dbcls" },
  { 0, "barexp" },
  { 0, "sgcock" },
//  { 0, "chgun" },
  { 0, "jpmove" },
  { 0, "jpidle" },
  { 0, "jprise" },
  { 0, "jpdown" },
  { 0, "jpflow" }
};

void DDF_SFXCreateEntry(void)
{
 // Resize the switch list on overflow
 if (numsfx == (maxsfx - 1))
   S_sfx = realloc(S_sfx, sizeof(*S_sfx) * ++maxsfx);

 S_sfx[numsfx++]=buffersfx;

 memset (&buffersfx,0,sizeof(buffersfx)); // clear the buffer
 buffersfx.volume = -1;
 buffersfx.pitch = -1;
}

static void DDF_SFXLookupLink(char *info, int commandref)
{
  int i;
  for (i = numsfx; i--;)
  {
    if (!strncasecmp(S_sfx[i].name, info, 8))
    {
      buffersfx.link = &S_sfx[i];
      return;
    }
  }
  I_Error("\nLink to unknown sound %s\n", info);
}

void DDF_ReadSFX(void* data, int size)
{
 readinfo_t sfx;

 if (!data)
 {
   sfx.message               = "DDF_InitSounds";
   sfx.filename              = "sounds.ddf";
   sfx.memfile = NULL;
 } else {
   sfx.message = NULL;
   sfx.memfile = data;
   sfx.memsize = size;
 }

 sfx.DDF_MainCheckName     = DDF_DummyFunction;
 sfx.DDF_MainCheckCmd      = DDF_MainCheckCommand;
 sfx.DDF_MainCreateEntry   = DDF_SFXCreateEntry;
 sfx.DDF_MainFinishingCode = DDF_SFXCreateEntry;
 sfx.cmdlist               = sfxcommands;

 DDF_MainReadFile(&sfx);
}
void DDF_SFXInit(void)
{
 int i;

 // Allocate original space
 S_sfx = malloc(sizeof(*S_sfx) * maxsfx);

 memset(S_sfx, 0, sizeof(*S_sfx));

 // Create the null sfx
 DDF_SFXCreateEntry();

 DDF_ReadSFX(NULL, 0);

 for (i = sizeof(bastard_sfx) / sizeof(bastard_sfx[0]); i--;)
   bastard_sfx[i].s = DDF_LookupSound(bastard_sfx[i].name);
}

// Takes two strings, and up to n characters from each, ignoring case
// '?' is taken as a wild card, so posit? will match posit, posit1, positx etc...
int strncasecmpwild(const char* s1, const char* s2, int n)
{
  int i = 0;
  for (i = 0; s1[i] && s2[i] && i < n; i++)
  {
    if ((toupper(s1[i]) != toupper(s2[i]))
     &&(s1[i] != '?')
     &&(s2[i] != '?'))
      break;
  }
  if (s1[i] == '?' || s2[i] == '?')
    return 0;
  return s1[i] - s2[i];
}

sfx_t* DDF_LookupSound(char *name)
{
  int i;
  int*  match = alloca(sizeof(int) * numsfx);
  sfx_t* r;
  int   num = 0;

  // NULL Sound
  if (!name)
    return NULL;

  if (!name[0])
    return NULL;

  for (i = numsfx; i--;)
  {
     if (!strncasecmpwild(name, S_sfx[i].name, 8))
     {
       match[num++] = i;
     }
  }
  if (!num)
    I_Error("Unknown SFX: '%.8s'\n", name);

  r = malloc(sizeof(sfx_t) + num * sizeof(int));

#ifdef DEVELOPERS
  if (!r)
    I_Error("malloc failed in DDF_LookupSound!\n");
#endif

  r->num = num;
  memcpy(r->sounds, match, num * sizeof(int));

  return r;
}

