// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

//static const char
//rcsid[] = "$Id: i_main.c,v 1.4 1997/02/03 22:45:10 b1 Exp $";

#define _WINDOWS_
#include <winalleg.h>
#include <allegro.h>
#include "m_argv.h"
#include "d_main.h"
#include "dm_defs.h"

#include <stdio.h>

int VERSION       = 200;
int VERSIONFIX    = 100;
int DOSDOOMVER    = 0x651;
int DOSDOOMVERFIX = 0x1000;

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
  if (WinAllegro_Init (hInst) == 0)
  {
    int colours[] = {8, 15, 16, 24, 32};
    int i, e;
    // Init Arguments
    myargc = WinAllegro_GetArgc();
    myargv = WinAllegro_GetArgv();
    
    allegro_init();
    
    WinAllegro_SetWindowTitle("DOSDoom");
    
    for (i = 0; i < sizeof(colours)/sizeof(colours[0]); i++)
    {
      set_color_depth(colours[i]);
      e = set_gfx_mode(GFX_DIRECTXWIN, 640, 236, 0, 0);
      if (!e)
        break;
    }
    if (e < 0)
    {
    MessageBox (NULL, allegro_error, "Error", MB_ICONASTERISK | MB_OK);
        return -1;
    }
    
    // Run DOOM! -- Never returns...
    D_DoomMain ();
    
    allegro_exit();
    WinAllegro_Exit();
    return 0;
  } else {
    WinAllegro_Exit();
    MessageBox (NULL,
    "WinAllegro initialisation failed. For more information look in allegro.log",
    "WinAllegro Error",
    MB_ICONASTERISK | MB_OK );
    return -1;
  }
}



void _go32_dpmi_lock_data(void *p, int s)
{
}
