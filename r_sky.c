//
// DOSDoom Sky Handling Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
//  The DOOM sky is a texture map like any wall, wrapping around.
//  A 1024 columns equal 360 degrees. The default sky map is 256
//  columns and repeats 4 times on a 320 screen?
//  

// Needed for FRACUNIT.
#include "m_fixed.h"

// Needed for Flat retrieval.
#include "r_data.h"
#include "dm_state.h"

#ifdef __GNUG__
#pragma implementation "r_sky.h"
#endif
#include "r_sky.h"

//
// sky mapping
//
int			skyflatnum;
int			skytexture;
int			skytexturemid;

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap (void)
{
    skytexturemid=100*FRACUNIT;
}

//
// Old Procedure:
//
// void R_InitSkyMap (void)
//{
//  // skyflatnum = R_FlatNumForName ( SKYFLATNAME );
//
///*  if ((SCREENWIDTH==320)&&(SCREENHEIGHT==200))
//    skytexturemid = (SCREENHEIGHT/2)*FRACUNIT;
//  else
////    skytexturemid = (SCREENHEIGHT/2)*FRACUNIT;
//    skytexturemid = (SCREENWIDTH*3)*FRACUNIT/8;*/
//
//    skytexturemid=100*FRACUNIT;
//
//}  *

