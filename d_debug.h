//
// DOSDoom Debug Functions.
//
// -KM- 1998-07-21 Added Z_CheckPointer
//
// -ACB- 1998/07/26 Added File_Printf
//

#ifndef __D_DEBUG_H__
#define __D_DEBUG_H__

#ifdef DEVELOPERS
#include <stdio.h>
// -KM- 98/07/31 Fixes Z_CheckPointer.
#include "z_zone.h"

extern FILE*	debugfile;

// This macro prints a string to the debug file.
#define Debug_Printf(format, args...) \
  if (debugfile) fprintf(debugfile, "%s: ", __FUNCTION__);\
  if (debugfile) fprintf(debugfile, format, ## args)

// This macro does the same as above, without the function name.
#define File_Printf(format, args...)  \
  if (debugfile) fprintf(debugfile, format, ## args)

// This macro checks a pointer to see if zoneid has
// been overwritten (bounds check/null pointer assignment)
// -KM- 98/07/31 Fixed this.
#define Z_CheckPointer(p) \
  { \
  memblock_t *chk = ((memblock_t *) (p) - 1); \
  if (chk->id != 0x1d4a11) \
    I_Error(__FILE__":%d: Pointer failed ZONEID!\n", __LINE__); \
  }

#else

// These macros do nothing, because we are not in DEVELOPER mode.
#define Debug_Printf(format, args...)
#define File_Printf(format, args...)
#define Z_CheckPointer(p)


#endif // DEVELOPERS

#endif //__D_DEBUG_H
