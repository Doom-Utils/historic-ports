///
// DOSDoom SaveGame Handling Header
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#ifndef __P_SAVEG__
#define __P_SAVEG__

#ifdef __GNUG__
#pragma interface
#endif

// Persistent storage/archiving. These are the load/save game routines.
void P_ArchivePlayers (void);
void P_UnArchivePlayers (void);
void P_ArchiveWorld (void);
void P_UnArchiveWorld (void);
void P_ArchiveThinkers (void);
void P_UnArchiveThinkers (void);
void P_ArchiveItemRespawnQue (void);
void P_UnArchiveItemRespawnQue (void);
void P_ArchiveSpecials (void);
void P_UnArchiveSpecials (void);

extern int savegame_size;
extern byte* save_p;

#endif

