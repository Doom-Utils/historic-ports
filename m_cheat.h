//
//
//

#ifndef __M_CHEAT__
#define __M_CHEAT__

// -MH- 1998/06/17 for cheat to give jetpack
// -KM- 1998-07-21 Added some extra headers in here
#include "dm_defs.h"
#include "dm_type.h"
#include "dm_state.h"
#include "d_event.h"

//
// CHEAT SEQUENCE PACKAGE
//
// -KM- 1998/07/21 Needed in am_map.c (iddt cheat)
typedef struct
{
    unsigned char*	sequence;
    unsigned char*	p;
    
} cheatseq_t;

static inline char* M_ConvertCheat(char* cheat)
{
  cheat[strlen(cheat)] = 0xff;
  return cheat;
}

int M_CheckCheat(cheatseq_t* cht, char key);
void M_GetCheatParam(cheatseq_t* cht, char* buffer);
boolean M_CheatResponder (event_t* ev);
void M_CheatInit(void);

#endif
