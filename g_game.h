//  
// DOSDoom Game Handling Code
//
// Based on the Doom Source Code,
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT).
//
#ifndef __G_GAME__
#define __G_GAME__

#include "dm_defs.h"
#include "d_event.h"



//
// GAME
//
void G_DeathMatchSpawnPlayer (int playernum);

void G_InitNew (skill_t skill, mapstuff_t* map);

//
// Called by the Startup code & M_Responder; A normal game
// is started by calling the beginning map. The level jump
// cheat can get us anywhere.
//
// -ACB- 1998/08/10 New DDF Structure, Use map reference name.
//
boolean G_DeferedInitNew (skill_t skill, const char *mapname);

void G_DeferedPlayDemo (char* demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame (char* name);

void G_DoLoadGame (void);

// Called by M_Responder.
void G_SaveGame (int slot, char* description);

// Only called by startup code.
void G_RecordDemo (char* name);

void G_BeginRecording (void);

void G_PlayDemo (char* name);
void G_TimeDemo (char* name);
boolean G_CheckDemoStatus (void);

// -KM- 1998/11/25 Added Time param
void G_ExitLevel (int time);
void G_SecretExitLevel (int time);
void G_ExitToLevel(char* name, int time);

void G_WorldDone (void);

void G_Ticker (void);
boolean G_Responder (event_t*	ev);

void G_ScreenShot (void);


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
