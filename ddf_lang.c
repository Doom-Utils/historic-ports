//
// DOSDoom Definition File Codes (Language handling settings)
//
// By the DOSDoom Team
//
// Language handling Setup and Parser Code
//
// 1998/10/29 -KM- Allow cmd line selection of language.
//
// This is somewhat different to most DDF reading files. In order to read the
// language specific strings, it uses the format:
//
// <RefName>=<String>;
//
// as opposed to the normal entry, which should be:
//
// [<Refname>]
// STRING=<string>;
//
// also the file suffix is LDF (Language Def File), this is to avoid confusion with
// the oridnary DDF files. The default file is DEFAULT.LDF, which can be subbed by
// using -lang <NameOfLangFile>.
//
// In order to use the new format, we use a programming technique called "Hacking".
// * The [<Name>] check is a dummy function (as with some of the DDF files).
// * The <command>= check actually sets up the reference for the string and
//   the return commandref is always the same and refs the DDF_LanguageStrToRef
//   function to read the data.
//
#include "dm_state.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_argv.h"
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

void DDF_LanguageStrToRef(char *info, int commandref);

commandlist_t langcommands[] =
 {{COMMAND_TERMINATOR, DDF_LanguageStrToRef, NULL}};

langref_t* langrefhead = NULL;

// 1998/10/29 -KM- This is true if the currently-being-read language is
//  the user selected language.
static boolean valid_language = false;

//
// DDF_LanguageLookup
//
// Globally Visibile to all files that directly or indirectly include ddf_main.h;
// This compares the ref name given with the refnames in the language lookup
// table. If one compares with the other, a pointer to the string is returned. If
// one is not found than an error is generated.
//
char* DDF_LanguageLookup(const char *refname)
{
    langref_t* entry;
    entry = langrefhead;

    while (entry != NULL && stricmp(refname,entry->refname))
      entry = entry->next;

    if (entry == NULL)
      I_Error("\n\tDDF_LanguageLookup: Unknown String Ref - '%s'\n", refname);

    return entry->string;
}

//
// DDF_LanguageNewRef
//
// Clears the buffer and add the reference to it.
//
int DDF_LanguageNewRef(char *info)
{
  if (!valid_language)
    return 0;

  memset(&bufflangref,0,sizeof(langref_t));

  bufflangref.refname = strdup(info);

  if (bufflangref.refname == NULL)
    I_Error("DDF_LanguageNewRef: Malloc Failed)");

  return 0;
}

//
// DDF_LanguageStrToRef
//
// Puts the string into the buffer entry and adds it to the linked list.
//
void DDF_LanguageStrToRef(char *info, int commandref)
{
  langref_t* entry;
  langref_t* newentry;
  
  if (!valid_language)
    return;

  bufflangref.string = strdup(info);

  if (bufflangref.string == NULL)
    I_Error("\n\tDDF_LanguageStrToRef: Malloc Failed\n");

  newentry = malloc(sizeof(langref_t));

  if (newentry == NULL)
    I_Error("\n\tDDF_LanguageStrToRef: Malloc Failed\n");

  memcpy(newentry,&bufflangref,sizeof(langref_t));

  if (langrefhead == NULL)
  {
    langrefhead = newentry;
    newentry->next = NULL;
  }
  else
  {
    entry = langrefhead;

    do
    {
      // Replaces a string.  This is so unknown (new)
      // strings fall back to the default (english) string
      if (!stricmp(entry->refname, newentry->refname))
      {
        free(entry->refname);
        free(entry->string);
        entry->refname = newentry->refname;
        entry->string = newentry->string;
        free(newentry);
        return;
      }

      if (!entry->next)
      {
        entry->next = newentry;
        newentry->next = NULL;
        return;
      }

      entry = entry->next;
    } while(1);
  }
}

void DDF_LanguageCheckName(char *info)
{
  if (stricmp(info, "DEFAULT"))
  {
    int p;

    valid_language = false;

    p = M_CheckParm("-lang");
    if (p && (p < myargc-1))
    {
      if (!stricmp(info, myargv[p+1]))
      {
        valid_language = true;
      }
    }
    return;
  }

  valid_language = true;

}

void DDF_ReadLangs(void *data, int size)
{
  readinfo_t language;

  if (!data)
  {
    language.message               = "DDF_InitLanguage";
    language.filename              = "default.ldf";
    language.memfile = NULL;
  } else {
    language.memfile = data;
    language.memsize = size;
    language.message = NULL;
    language.filename = NULL;
  }
  language.DDF_MainCheckName     = DDF_LanguageCheckName;
  language.DDF_MainCreateEntry   = DDF_DummyFunction;
  language.DDF_MainCheckCmd      = DDF_LanguageNewRef;
  language.DDF_MainFinishingCode = DDF_DummyFunction;
  language.cmdlist               = langcommands;
 
  DDF_MainReadFile(&language);
}
void DDF_LanguageInit()
{
  memset(&bufflangref,0,sizeof(langref_t));
  DDF_ReadLangs(NULL, 0);
}



