///
// DOSDoom SaveGame Handling Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -ACB- 1998/09/15 DEVELOPERS support added.
// -KM- 1998/11/25 Invisibility is fixed_t.
// -KM- 1998/12/16 Fixed savegames.
//

#include "d_debug.h"
#include "dm_state.h"
#include "i_system.h"
#include "p_local.h"
#include "p_spec.h"
#include "r_state.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

int savegame_size;
extern byte* savebuffer;
byte* save_p;

//
// Save Game Map Object definition.
//
// If anything new is added to the mobj_t and is information that
// would need to be recorded here also, anything that is a pointer
// (that is hopefully in a linked list..) should be record as an int
// with the _id suffix. All _id types represent the number in the
// linked list that the pointer pointed to in the game so that it
// can restored when loaded, you must also add (de)referencing code
// in P_ArchiveThinkers, P_UnArchiveSingleThinker & P_UnArchiveThinker.
//
// -ACB- 1998/09/14 Written for DDF Items.
//
typedef struct savegmobj_s
{
  fixed_t x;
  fixed_t y;
  fixed_t z;

  angle_t angle;
  char sprite[4];
  int frame;

  fixed_t floorz;
  fixed_t ceilingz;

  fixed_t radius;
  fixed_t height;

  fixed_t momx;
  fixed_t momy;
  fixed_t momz;

  fixed_t speed;

  int tics;
  int state;
  int flags;
  int health;

  int movedir;
  int movecount;

  int reactiontime;
  int threshold;

  int lastlook;

  mapthing_t spawnpoint;

  int source;
  int target;
  int tracer;
  int supportobj;

  byte playxtra;
  fixed_t origheight;

  fixed_t invisibility;
  fixed_t deltainvis;

  int extendedflags;

  int backpackinfo;

  fixed_t vertangle;
  int spreadcount;

  int player;

  char* type;
  char* currentattack;
}
savegmobj_t;

// Pads save_p to a 4-byte boundary so that the load/save works on SGI&Gecko.
#define PADSAVEP() save_p += (4 - ((int) save_p & 3)) & 3

static inline void P_GrowSGBuffer(int numBytes)
{
  int length;
  if ((savegame_size - (save_p - savebuffer)) < numBytes)
  {
    savegame_size += numBytes;
    length = save_p - savebuffer;
    savebuffer = Z_ReMalloc(savebuffer, savegame_size);
    save_p = savebuffer + length;
  }
}

//
// P_ArchivePlayers
//
void P_ArchivePlayers (void)
{
  int i;
  int j;
  player_t* dest;
  int* s;
		
  P_GrowSGBuffer(MAXPLAYERS * sizeof(player_t) +
                 2 * sizeof(int) +
                 numweapons * sizeof(boolean) +
                 2 * NUMAMMO * sizeof(int));

  for (i=0 ; i<MAXPLAYERS ; i++)
  {
    if (!playeringame[i])
      continue;
	
#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

    PADSAVEP();

    s = (int *) save_p;
    s[0] = numweapons;
    s[1] = NUMAMMO;
    save_p += 2 * sizeof(int);
    dest = (player_t *)save_p;
    memcpy (dest,&players[i],sizeof(player_t));
    save_p += sizeof(player_t);
    memcpy (save_p, players[i].weaponowned, numweapons * sizeof(boolean));
    save_p += numweapons * sizeof(boolean);
    memcpy (save_p, players[i].ammo, NUMAMMO * sizeof(int));
    save_p += NUMAMMO * sizeof(int);
    memcpy (save_p, players[i].maxammo, NUMAMMO * sizeof(int));
    save_p += NUMAMMO * sizeof(int);

    for (j=0 ; j<NUMPSPRITES ; j++)
    {
      if (dest->psprites[j].state)
        dest->psprites[j].state = (state_t *)(dest->psprites[j].state-states);
    }
  }

#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers (void)
{
  int i;
  int j;
  int p_numammo;
  int p_numweapons;
  int *maxammo, *ammo, *s;
  boolean *weaponowned;
	
#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

  for (i=0 ; i<MAXPLAYERS ; i++)
  {
#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

    if (!playeringame[i])
      continue;
	
    PADSAVEP();

    s = (int *) save_p;
    p_numweapons = s[0];
    p_numammo = s[1];
    save_p += sizeof(int) * 2;
    maxammo = players[i].maxammo;
    ammo = players[i].ammo;
    weaponowned = players[i].weaponowned;
    memcpy (&players[i],save_p, sizeof(player_t));
    save_p += sizeof(player_t);
    players[i].maxammo = maxammo;
    players[i].ammo = ammo;
    players[i].weaponowned = weaponowned;
    if (p_numweapons >= numweapons)
    {
      memcpy(weaponowned, save_p, numweapons * sizeof(boolean));
      save_p += p_numweapons * sizeof(boolean);
#ifdef DEVELOPERS
      if (p_numweapons != numweapons)
        Debug_Printf("Savegame numweapons > current numweapons!");
#endif
    } else {
      memcpy(weaponowned, save_p, p_numweapons * sizeof(boolean));
      memset(&weaponowned[p_numweapons], false, sizeof(boolean) * (numweapons - p_numweapons));
      save_p += p_numweapons * sizeof(boolean);
      Debug_Printf("Savegame numweapons < current numweapons!");
    }
    if (p_numammo >= NUMAMMO)
    {
      memcpy(ammo, save_p, NUMAMMO * sizeof(int));
      save_p += p_numammo * sizeof(int);
      memcpy(maxammo, save_p, NUMAMMO * sizeof(int));
      save_p += p_numammo * sizeof(int);
#ifdef DEVELOPERS
      if (p_numammo != NUMAMMO)
        Debug_Printf("Savegame numammo > current numammo!");
#endif
    } else {
      memcpy(ammo, save_p, p_numammo * sizeof(int));
      memset(&ammo[p_numammo], 0, sizeof(int) * (NUMAMMO - p_numammo));
      save_p += p_numammo * sizeof(int);
      memcpy(maxammo, save_p, p_numammo * sizeof(int));
      memset(&maxammo[p_numammo], 0, sizeof(int) * (NUMAMMO - p_numammo));
      save_p += p_numammo * sizeof(int);
      Debug_Printf("Savegame numweapons < current numweapons!");
    }

    // will be set when we unarc thinker
    players[i].mo = NULL;
    players[i].message = NULL;
    players[i].attacker = NULL;

    for (j=0 ; j<NUMPSPRITES ; j++)
    {
      if (players[i].psprites[j].state)
        players[i].psprites[j].state = &states[(int)players[i].psprites[j].state];
    }
  }

#ifdef DEVELOPERS
    Debug_Printf("Current position: %ld\n",save_p - savebuffer);
#endif

}

typedef struct
{
  short textureoffset;
  short rowoffset;
  short toptexture;
  short bottomtexture;
  short midtexture;
}
saveside_t;

typedef struct
{
  short flags;
  short special;
  short tag;
  saveside_t front;
  saveside_t back;
}
saveline_t;

typedef struct
{
  short floorheight;
  short ceilingheight;
  char floorpic[8];
  char ceilingpic[8];
  short lightlevel;
  short special;
  short tag;
  char colourmaplump[8];
  int colourmap;
  fixed_t gravity;
  fixed_t friction;
  fixed_t viscosity;
}
savesector_t;

//
// P_ArchiveWorld
//
// Archives sector and linedef states.
//
// -ACB- 1998/09/15 Saving of sectors and lines now use set structures, Originally
//                  written as an attempted bugfix from a savegame de-sync, but it
//                  has been kept as it is much more stable then the old method.
//
void P_ArchiveWorld (void)
{
  int i;
  line_t* li;
  side_t* si;
  sector_t* sec;
  savesector_t buffsec;
  saveline_t buffline;
	    
  // do sectors
  // -ACB- 1998/09/14 Altered to store a defined savesector type....
  for (i=0, sec = sectors ; i<numsectors ; i++, sec++)
  {
    buffsec.floorheight = sec->floorheight >> FRACBITS;
    buffsec.ceilingheight = sec->ceilingheight >> FRACBITS;
    memcpy(buffsec.floorpic, lumpinfo[flatlist[sec->floorpic]].name, 8);
    memcpy(buffsec.ceilingpic, lumpinfo[flatlist[sec->ceilingpic]].name, 8);
    if (sec->colourmaplump != -1)
      memcpy(buffsec.colourmaplump, lumpinfo[sec->colourmaplump].name, 8);
    else
      memset(buffsec.colourmaplump, 0, 8);
    buffsec.lightlevel = sec->lightlevel;
    buffsec.special = sec->special;
    buffsec.tag = sec->tag;
    buffsec.colourmap = sec->colourmap;
    buffsec.gravity = sec->gravity;
    buffsec.friction = sec->friction;
    buffsec.viscosity = sec->viscosity;

    P_GrowSGBuffer(sizeof(savesector_t));
    PADSAVEP();
    memcpy ((savesector_t *)save_p, &buffsec, sizeof(savesector_t));
    save_p += sizeof(savesector_t);
  }

  // do lines
  // -ACB- 1998/09/14 Altered to store a defined saveline type....
  for (i=0, li = lines ; i<numlines ; i++, li++)
  {
#ifdef DEVELOPERS
    Debug_Printf("Line: %d Tag: %d Special: %d\n", i, li->tag , li->special);
#endif

    memset(&buffline,0,sizeof(saveline_t));

    buffline.flags = li->flags;
    buffline.special = li->special;
    buffline.tag = li->tag;

    if (li->sidenum[0] != -1)
    {
      si = &sides[li->sidenum[0]];

      buffline.front.textureoffset = si->textureoffset >> FRACBITS;
      buffline.front.rowoffset = si->rowoffset >> FRACBITS;
      buffline.front.toptexture = si->toptexture;
      buffline.front.bottomtexture = si->bottomtexture;
      buffline.front.midtexture = si->midtexture;
    }

    if (li->sidenum[1] != -1)
    {
      si = &sides[li->sidenum[1]];

      buffline.back.textureoffset = si->textureoffset >> FRACBITS;
      buffline.back.rowoffset = si->rowoffset >> FRACBITS;
      buffline.back.toptexture = si->toptexture;
      buffline.back.bottomtexture = si->bottomtexture;
      buffline.back.midtexture = si->midtexture;
    }

    P_GrowSGBuffer(sizeof(saveline_t));
    PADSAVEP();
    memcpy ((saveline_t *)save_p, &buffline, sizeof(saveline_t));
    save_p += sizeof(saveline_t);
  }

  // -KM- 1998/12/17 Save the intermission
  if (worldmap.name)
  {
     P_GrowSGBuffer(strlen(worldmap.name) + 1);
     sprintf(save_p, "%s", worldmap.name);
     save_p += strlen(worldmap.name) + 1;

     P_GrowSGBuffer(sizeof(boolean) * worldmap.nummaps + sizeof(int));
     *(int *) save_p = worldmap.nummaps;
     save_p += sizeof(int);

     memcpy(save_p, worldmap.mapdone, sizeof(boolean) * worldmap.nummaps);
     save_p += sizeof(boolean) * worldmap.nummaps;
  }

#ifdef DEVELOPERS
    Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif
}

//
// P_UnArchiveWorld
//
// Un-Archives sector and linedef states.
//
// -ACB- 1998/09/15 Saving of sectors and lines now use set structures, Originally
//                  written as an attempted bugfix from a savegame de-sync, but it
//                  has been kept as it is much more stable then the old method.
//
void P_UnArchiveWorld (void)
{
  int i;
  line_t* li;
  side_t* si;
  sector_t* sec;
  saveline_t buffline;
  savesector_t buffsec;
	
  // do sectors
  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
  {
    PADSAVEP();
    memcpy(&buffsec, (savesector_t *)save_p, sizeof(savesector_t));

    sec->floorheight = buffsec.floorheight << FRACBITS;
    sec->ceilingheight = buffsec.ceilingheight << FRACBITS;
    sec->floorpic = R_FlatNumForName(buffsec.floorpic);
    sec->ceilingpic = R_FlatNumForName(buffsec.ceilingpic);
    if (buffsec.colourmaplump[0])
      sec->colourmaplump = W_GetNumForName(buffsec.colourmaplump);
    else
      sec->colourmaplump = -1;
    sec->colourmap = buffsec.colourmap;
    sec->friction = buffsec.friction;
    sec->viscosity = buffsec.viscosity;
    sec->lightlevel = buffsec.lightlevel;
    sec->special = buffsec.special;
    sec->tag = buffsec.tag;
    sec->specialdata[0] = 0;
    sec->specialdata[1] = 0;
    sec->specialdata[2] = 0;
    sec->soundtarget = 0;

    save_p += sizeof(savesector_t);
  }
    
  // do lines
  for (i=0, li = lines ; i<numlines ; i++,li++)
  {
    PADSAVEP();
    memcpy(&buffline,(saveline_t*)save_p,sizeof(saveline_t));

    li->flags   = buffline.flags;
    li->special = buffline.special;
    li->tag     = buffline.tag;

    // we have a frontside?
    if (li->sidenum[0] != -1)
    {
      si = &sides[li->sidenum[0]];

      si->textureoffset = buffline.front.textureoffset << FRACBITS;
      si->rowoffset     = buffline.front.rowoffset << FRACBITS;
      si->toptexture    = buffline.front.toptexture;
      si->bottomtexture = buffline.front.bottomtexture;
      si->midtexture    = buffline.front.midtexture;
    }

    // we have a backside?
    if (li->sidenum[1] != -1)
    {
      si = &sides[li->sidenum[1]];

      si->textureoffset = buffline.back.textureoffset << FRACBITS;
      si->rowoffset     = buffline.back.rowoffset << FRACBITS;
      si->toptexture    = buffline.back.toptexture;
      si->bottomtexture = buffline.back.bottomtexture;
      si->midtexture    = buffline.back.midtexture;
    }

#ifdef DEVELOPERS
    Debug_Printf("Line: %d Tag: %d Special: %d\n", i, li->tag , li->special);
#endif

    save_p += sizeof(saveline_t);
  }

  // -KM- 1998/12/17 Reload the intermission.
  if (*save_p)
  {
     int nummaps;
     WI_MapInit(DDF_GameGetName(save_p));
     save_p += strlen(save_p) + 1;

     nummaps = *(int *) save_p;
     save_p += sizeof(int);
     if (nummaps >= worldmap.nummaps)
     {
       memcpy(worldmap.mapdone, save_p, sizeof(boolean) * worldmap.nummaps);
       save_p += sizeof(boolean) * nummaps;
     } else {
       memcpy(worldmap.mapdone, save_p, sizeof(boolean) * nummaps);
       memset(&worldmap.mapdone[nummaps], false, sizeof(boolean) * (worldmap.nummaps - nummaps));
       save_p += sizeof(boolean) * nummaps;
     }
  }

}

//
// Thinkers
//
typedef enum
{
  tc_end,
  tc_mobj
}
thinkerclass_t;

//
// P_ArchiveThinkers
//
// Records the list of objects in the current world. The list
// is taken from the top to the end. The copy if not direct, since
// the mobj contains pointers that must be translated into id numbers;
// The id numbers are the position in the linked list of that type,
// these numbers will always be the same if the same DDF procedures
// and files are used; the mobj list is loaded the same because it
// will be stored in the order is currently exists.
//
// The Save-game object structure (savegmobj_t) is used to store the
// objects: it is the mobj_t with all the pointers replaced with id
// refs.
//
// -ACB- 1998/09/14 Procedure rewritten for DDF Objects...
//
void P_ArchiveThinkers (void)
{
  attacktype_t* searchattack;
  backpack_t* searchbackpack;
  mobj_t* currmobj;
  mobj_t* searchmobj;

  savegmobj_t savemobj;
  int count;

#ifdef DEVELOPERS
    Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

  for (currmobj = mobjlisthead; currmobj != NULL; currmobj = currmobj->next)
  {
#ifdef DEVELOPERS
    Debug_Printf("Thing: %s %d %d %d\n",currmobj->info->name,
                     currmobj->x,currmobj->y,currmobj->z);
#endif
    // Lets go through the straight number storage bit...
    savemobj.type = currmobj->info->name;

    savemobj.x = currmobj->x;
    savemobj.y = currmobj->y;
    savemobj.z = currmobj->z;

    savemobj.angle = currmobj->angle;
    memcpy(savemobj.sprite, sprnames[currmobj->sprite], 4);
    savemobj.frame = currmobj->frame;

    savemobj.floorz = currmobj->floorz;
    savemobj.ceilingz = currmobj->ceilingz;

    savemobj.radius = currmobj->radius;
    savemobj.height = currmobj->height;

    savemobj.momx = currmobj->momx;
    savemobj.momy = currmobj->momy;
    savemobj.momz = currmobj->momz;

    savemobj.speed = currmobj->speed;

    savemobj.tics = currmobj->tics;
    savemobj.state = currmobj->state - states;
    savemobj.flags = currmobj->flags;
    savemobj.health = currmobj->health;

    savemobj.movedir = currmobj->movedir;
    savemobj.movecount = currmobj->movecount;

    savemobj.reactiontime = currmobj->reactiontime;
    savemobj.threshold = currmobj->threshold;

    savemobj.lastlook = currmobj->lastlook;

    savemobj.spawnpoint = currmobj->spawnpoint;

    savemobj.playxtra = currmobj->playxtra;
    savemobj.origheight = currmobj->origheight;

    savemobj.invisibility = currmobj->invisibility;
    savemobj.deltainvis = currmobj->deltainvis;

    savemobj.extendedflags = currmobj->extendedflags;

    savemobj.spreadcount = currmobj->spreadcount;
    savemobj.vertangle = currmobj->vertangle;

    // Now store the current attack type id number
    searchattack = attackhead;

    while (searchattack != currmobj->currentattack && searchattack)
      searchattack = searchattack->next;

    if (!searchattack)
      savemobj.currentattack = "";
    else
      savemobj.currentattack = searchattack->name;

    // Moving Object References....
    //
    // Searches through the list of objects to see if any match the
    // following objects: if so the object is stored.
    //
    savemobj.source = -1;
    savemobj.target = -1;
    savemobj.tracer = -1;
    savemobj.supportobj = -1;

    count=0;

    for (searchmobj=mobjlisthead; searchmobj!=NULL; searchmobj=searchmobj->next)
    {
      if (currmobj->source && currmobj->source == searchmobj)
        savemobj.source = count;

      if (currmobj->supportobj && currmobj->supportobj == searchmobj)
        savemobj.supportobj = count;

      if (currmobj->target && currmobj->target == searchmobj)
        savemobj.target = count;

      if (currmobj->tracer && currmobj->tracer == searchmobj)
        savemobj.tracer = count;

      count++;
    }

    // lets store the ref for what the object is (mobjinfo)
    count = 0;
    searchbackpack = backpackhead;

    while (searchbackpack != currmobj->backpackinfo && searchbackpack)
    {
      searchbackpack = searchbackpack->next;
      count++;
    }

    if (!searchbackpack)
      savemobj.backpackinfo = -1;
    else
      savemobj.backpackinfo = count;

    if (currmobj->player)
      savemobj.player = (currmobj->player - players);
    else
      savemobj.player = -1;

    P_GrowSGBuffer(sizeof(savegmobj_t) + 3 - sizeof(char *) + strlen(savemobj.currentattack) +
                   strlen(savemobj.type));
    *save_p++ = tc_mobj;
    PADSAVEP();
    memcpy ((savegmobj_t *)save_p, &savemobj, sizeof(savegmobj_t)-sizeof(char*));
    save_p += sizeof(savegmobj_t) - (2*sizeof(char *));
    save_p += sprintf(save_p, "%s", savemobj.type) + 1;
    save_p += sprintf(save_p, "%s", savemobj.currentattack) + 1;
  }

  // add a terminating marker
  P_GrowSGBuffer(1);
  *save_p++ = tc_end;

#ifdef DEVELOPERS
    Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

}

//
// P_UnArchiveSingleThinker
//
// Converts the saved save-game object into the world object. Its
// de-references the id numbers to pointers in the object, this
// does NOT happen with pointers to other objects here. Its is
// attempted when the objects list has been fully reconstructed (for
// obvious reasons).
//
// -ACB- 1998/09/14 Procedure Written.
//
void P_UnArchiveSingleThinker(savegmobj_t* savedmobj)
{
  attacktype_t* searchattack;
  backpack_t* searchbackp;
  mobj_t* mobj;
  mobj_t* listcurrmobj;
  mobjinfo_t *objecttype;
  int i;

  mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, 0);

  mobj->x = savedmobj->x;
  mobj->y = savedmobj->y;
  mobj->z = savedmobj->z;

  mobj->angle = savedmobj->angle;
  for (i = 0; i < NUMSPRITES; i++)
     if (!memcmp(savedmobj->sprite, sprnames[i], 4))
     {
       mobj->sprite = i;
       break;
     }

  if (i == NUMSPRITES)
    I_Error("P_UnArchiveSingleThinker: Unknown sprite name '%.4s'\n",savedmobj->sprite);

  mobj->frame = savedmobj->frame;

  mobj->floorz = savedmobj->floorz;
  mobj->ceilingz = savedmobj->ceilingz;

  mobj->radius = savedmobj->radius;
  mobj->height = savedmobj->height;

  mobj->momx = savedmobj->momx;
  mobj->momy = savedmobj->momy;
  mobj->momz = savedmobj->momz;

  mobj->speed = savedmobj->speed;

  savedmobj->type = save_p;
  save_p += strlen(savedmobj->type) + 1;
  objecttype = mobjinfohead;
  while (objecttype && strcmp(objecttype->name, savedmobj->type)) objecttype = objecttype->next;

  if (!objecttype)
      I_Error("P_UnArchiveSingleThinker: Unknown Mobjinfo type encountered\n");

  mobj->info = objecttype;
  mobj->type = objecttype->doomednum;

  mobj->tics = savedmobj->tics;
  mobj->state = &states[savedmobj->state];
  mobj->flags = savedmobj->flags;
  mobj->health = savedmobj->health;

  mobj->movedir = savedmobj->movedir;
  mobj->movecount = savedmobj->movecount;

  mobj->reactiontime = savedmobj->reactiontime;
  mobj->threshold = savedmobj->threshold;

  mobj->lastlook = savedmobj->lastlook;

  mobj->spawnpoint = savedmobj->spawnpoint;

  mobj->playxtra = savedmobj->playxtra;
  mobj->origheight = savedmobj->origheight;

  mobj->invisibility = savedmobj->invisibility;
  mobj->deltainvis = savedmobj->deltainvis;

  mobj->extendedflags = savedmobj->extendedflags;

  mobj->vertangle = savedmobj->vertangle;
  mobj->spreadcount = savedmobj->spreadcount;

  mobj->backpackinfo = NULL;
  mobj->currentattack = NULL;

  if (savedmobj->player != -1)
  {
    mobj->player = &players[savedmobj->player];
    mobj->player->mo = mobj;
  }
  else
  {
    mobj->player = NULL;
  }

  if (savedmobj->backpackinfo != -1)
  {
    searchbackp = backpackhead;

    for (i=0; i<(savedmobj->backpackinfo); i++, searchbackp = searchbackp->next)
    {
      if (!searchbackp)
        I_Error("P_UnArchiveSingleThinker: Unknown Backpack Type id found\n");
    }

    mobj->backpackinfo = searchbackp;
  }
  else
  {
    mobj->backpackinfo = NULL;
  }

  savedmobj->currentattack = save_p;
  save_p += strlen(savedmobj->currentattack) + 1;

  if (savedmobj->currentattack[0])
  {
    searchattack = attackhead;
    while (searchattack && !strcmp(searchattack->name, savedmobj->currentattack))
      searchattack = searchattack->next;

    if (!searchattack)
        I_Error("P_UnArchiveSingleThinker: Unknown Attack Type id found\n");

    mobj->currentattack = searchattack;
  }
  else
  {
    mobj->currentattack = NULL;
  }

  P_SetThingPosition (mobj);
  mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
  P_AddThinker (&mobj->thinker);

  mobj->target = (mobj_t*) savedmobj->target;
  mobj->source = (mobj_t*) savedmobj->source;
  mobj->supportobj = (mobj_t*) savedmobj->supportobj;
  mobj->tracer = (mobj_t*) savedmobj->tracer;

  //
  // -ACB- 1998/08/27 Mobj Linked-List Addition
  //
  // A useful way of cycling through the current things without
  // having to deref everything using thinkers.
  //
  if (mobjlisthead == NULL)
  {
    mobjlisthead = mobj;
    mobj->prev = NULL;
    mobj->next = NULL;
  }
  else
  {
    listcurrmobj = mobjlisthead;

    while (listcurrmobj->next != NULL)
     listcurrmobj = listcurrmobj->next;

    listcurrmobj->next = mobj;
    mobj->prev = listcurrmobj;
    mobj->next = NULL;
  }

}

//
// P_UnArchiveThinkers
//
// Loaded the objects from the same games, the conversion from
// save-game object to world-object (mobj_t) takes place. Once
// all of the objects have been loaded from the file, the final
// action of this procedure is to translate the id numbers of a
// save-game object into pointers for the world-objects to other
// world-objects. Confused? You will be...
//
// -ACB- 1998/09/14 Rewritten for DDF Items.
//
void P_UnArchiveThinkers (void)
{
  boolean endfound;
  byte tclass;
  mobj_t* mobj;
  savegmobj_t* savedmobj;
  thinker_t* next;
  thinker_t* currentthinker;

  int nummobjs = 0;
  mobj_t** mobj_list;

  int i;

  // remove all the current thinkers, not just the objects...
  currentthinker = thinkercap.next;

  while (currentthinker != &thinkercap)
  {
    next = currentthinker->next;

    if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
      P_RemoveMobj ((mobj_t *)currentthinker);
    else
      Z_Free (currentthinker);

    currentthinker = next;
  }

  // Put the thinkers list in the init state...
  P_InitThinkers ();

  endfound = false;

#ifdef DEVELOPERS
  Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

  while (!endfound)
  {
    tclass = *save_p++;

    // so what we reading exactly? tclass should be either tc_end or tc_mobj.
    switch (tclass)
    {
      case tc_end:
      {
        endfound = true; // end of list
	break;
      }

      case tc_mobj:
      {
        PADSAVEP();

	savedmobj = Z_Malloc (sizeof(savegmobj_t), PU_LEVEL, NULL);
	memcpy (savedmobj, (savegmobj_t*)save_p, sizeof(savegmobj_t) - sizeof(char*));
	save_p += sizeof(savegmobj_t)-(2*sizeof(char *));

        P_UnArchiveSingleThinker(savedmobj);
        nummobjs++;

        break;
      }
      default:
      {
        I_Error ("Unknown tclass %i in savegame",tclass);
        break; // lets be pedantic :)
      }	
    }
  }

  //
  // The list of objects has now been reconstructed in the order from the
  // savegame file (and in the same order that it existed when the game was
  // saved), we will now reference the id numbers to pointers to the
  // correct object, any failure indicates a corrupt savegame file.
  //
  mobj = mobjlisthead;
  mobj_list = Z_Malloc(sizeof(mobj_t*) * nummobjs, PU_STATIC, NULL);

  for (mobj = mobjlisthead, i = 0; mobj; i++, mobj = mobj->next)
     mobj_list[i] = mobj;

  for (mobj = mobjlisthead; mobj; mobj = mobj->next)
  {
     if ((int) mobj->target != -1)
       mobj->target = mobj_list[(int) mobj->target];
     else
       mobj->target = NULL;
     if ((int) mobj->source != -1)
       mobj->source = mobj_list[(int) mobj->source];
     else
       mobj->source = NULL;
     if ((int) mobj->tracer != -1)
       mobj->tracer = mobj_list[(int) mobj->tracer];
     else
       mobj->tracer = NULL;
     if ((int) mobj->supportobj != -1)
       mobj->supportobj = mobj_list[(int) mobj->supportobj];
     else
       mobj->supportobj = NULL;
  }

  Z_Free(mobj_list);

#ifdef DEVELOPERS
  Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

}

typedef enum
{
  tq_end,
  tq_item
}
queclass_t;

//
// P_ArchiveItemRespawnQue
//
// Archives the Item-Respawn-Que.
//
// -ACB- 1998/09/15
//
void P_ArchiveItemRespawnQue(void)
{
  iteminque_t* curritem;

  for (curritem=itemquehead; curritem != NULL; curritem=curritem->next)
  {
    P_GrowSGBuffer(sizeof(mapthing_t)+1+sizeof(int));
    *save_p++ = tq_item;
    PADSAVEP();
    memcpy ((mapthing_t *)save_p, &curritem->info, sizeof(mapthing_t));
    save_p += sizeof(mapthing_t);
    memcpy ((int *)save_p, &curritem->time, sizeof(int));
    save_p += sizeof(int);
  }

  P_GrowSGBuffer(1);
  *save_p++ = tq_end;
}

//
// P_ArchiveItemRespawnQue
//
// Archives the Item-Respawn-Que.
//
// -ACB- 1998/09/15
//
void P_UnArchiveItemRespawnQue(void)
{
  iteminque_t* newitem;
  iteminque_t* searchitem;

  newitem = Z_Malloc(sizeof(iteminque_t), PU_LEVEL, 0);

  while (*save_p++ != tq_end)
  {
    PADSAVEP();
    memcpy (&newitem->info,(mapthing_t *)save_p, sizeof(mapthing_t));
    save_p += sizeof(mapthing_t);
    memcpy (&newitem->time, (int *)save_p, sizeof(int));
    save_p += sizeof(int);

    searchitem = itemquehead;

    if (itemquehead)
    {
      while (searchitem->next != NULL)
        searchitem = searchitem->next;

      searchitem->next = newitem;
      newitem->prev = searchitem;
    }
    else
    {
      itemquehead = newitem;
      newitem->prev = NULL;
    }

    newitem->next = NULL;
  }

}


//
// P_ArchiveSpecials
//
typedef enum
{
  tc_sector,
  tc_light,
  tc_endspecials
}
specials_e;	



//
// P_ArchiveSpecials
//
// Things to handle:
//
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
//
// -KM- 1998/09/01 Remove doors/plats (DDF)
//
void P_ArchiveSpecials (void)
{
    thinker_t*		th;
    secMove_t*		sec;
    light_t*            light;
    int			i;
	
#ifdef DEVELOPERS
     Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

    // save off the current thinkers
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
	if (th->function.acv == (actionf_v)NULL)
	{
	    for (i = 0; i < maxsecs;i++)
		if (activesecs[i] == (secMove_t *)th)
		    break;
	    
	    if (i<maxsecs)
	    {
              P_GrowSGBuffer(sizeof(*sec) + 2);
              *save_p++ = tc_sector;
              PADSAVEP();
              sec = (secMove_t *)save_p;
              memcpy (sec, th, sizeof(*sec));
              save_p += sizeof(*sec);
              sec->sector = (sector_t *)(sec->sector - sectors);
              memcpy (save_p, sec->type, sizeof(movinPlane_t));
              save_p += sizeof(movinPlane_t);
            }
	    continue;
	}
			
	if (th->function.acp1 == (actionf_p1)T_MoveSector)
	{
            P_GrowSGBuffer(sizeof(*sec) + sizeof(movinPlane_t) + 2);
	    *save_p++ = tc_sector;
	    PADSAVEP();
	    sec = (secMove_t *)save_p;
	    memcpy (sec, th, sizeof(*sec));
	    save_p += sizeof(*sec);
	    sec->sector = (sector_t *)(sec->sector - sectors);
            memcpy (save_p, sec->type, sizeof(movinPlane_t));
            save_p += sizeof(movinPlane_t);
	    continue;
	}
/*
	if (th->function.acp1 == (actionf_p1)T_VerticalDoor)
	{
            P_GrowSGBuffer(sizeof(*door) + 1);
	    *save_p++ = tc_door;
	    PADSAVEP();
	    door = (ceiling_t *)save_p;
	    memcpy (door, th, sizeof(*door));
	    save_p += sizeof(*door);
	    door->sector = (sector_t *)(door->sector - sectors);
	    continue;
	}
*/
/*	if (th->function.acp1 == (actionf_p1)T_MoveFloor)
	{
            P_GrowSGBuffer(sizeof(*floor) + 1);
	    *save_p++ = tc_floor;
	    PADSAVEP();
	    floor = (floormove_t *)save_p;
	    memcpy (floor, th, sizeof(*floor));
	    save_p += sizeof(*floor);
	    floor->sector = (sector_t *)(floor->sector - sectors);
            floor->type = (linedeftype_t *)(floor->type->trignum);
	    continue;
	}*/
			
/*	if (th->function.acp1 == (actionf_p1)T_PlatRaise)
	{
            P_GrowSGBuffer(sizeof(*plat) + 1);
	    *save_p++ = tc_plat;
	    PADSAVEP();
	    plat = (plat_t *)save_p;
	    memcpy (plat, th, sizeof(*plat));
	    save_p += sizeof(*plat);
	    plat->sector = (sector_t *)(plat->sector - sectors);
	    continue;
	}
*/
	if (th->function.acp1 == (actionf_p1)T_Light)
	{
            P_GrowSGBuffer(sizeof(*light) + 1);
	    *save_p++ = tc_light;
	    PADSAVEP();
	    light = (light_t *)save_p;
	    memcpy (light, th, sizeof(*light));
	    save_p += sizeof(*light);
	    light->sector = (sector_t *)(light->sector - sectors);
	    continue;
	}
/*	if (th->function.acp1 == (actionf_p1)T_LightFlash)
	{
            P_GrowSGBuffer(sizeof(*flash) + 1);
	    *save_p++ = tc_flash;
	    PADSAVEP();
	    flash = (lightflash_t *)save_p;
	    memcpy (flash, th, sizeof(*flash));
	    save_p += sizeof(*flash);
	    flash->sector = (sector_t *)(flash->sector - sectors);
	    continue;
	}
			
	if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
	{
            P_GrowSGBuffer(sizeof(*strobe) + 1);
	    *save_p++ = tc_strobe;
	    PADSAVEP();
	    strobe = (strobe_t *)save_p;
	    memcpy (strobe, th, sizeof(*strobe));
	    save_p += sizeof(*strobe);
	    strobe->sector = (sector_t *)(strobe->sector - sectors);
	    continue;
	}
			
	if (th->function.acp1 == (actionf_p1)T_Glow)
	{
            P_GrowSGBuffer(sizeof(*glow) + 1);
	    *save_p++ = tc_glow;
	    PADSAVEP();
	    glow = (glow_t *)save_p;
	    memcpy (glow, th, sizeof(*glow));
	    save_p += sizeof(*glow);
	    glow->sector = (sector_t *)(glow->sector - sectors);
	    continue;
	}*/
    }
	
    // add a terminating marker
    P_GrowSGBuffer(1);
    *save_p++ = tc_endspecials;

#ifdef DEVELOPERS
     Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

}


//
// P_UnArchiveSpecials
//
// -KM- 1998/09/01 Remove doors/plats (DDF)
//
void P_UnArchiveSpecials (void)
{
    byte tclass;
    secMove_t* sec;
    light_t* light;
		
#ifdef DEVELOPERS
     Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif

    // read in saved thinkers
    while (1)
    {
	tclass = *save_p++;
	switch (tclass)
	{
	  case tc_endspecials:
#ifdef DEVELOPERS
            Debug_Printf("Current Position: %ld\n",save_p - savebuffer);
#endif
	    return;	// end of list
			
	  case tc_sector:
	    PADSAVEP();
	    sec = Z_Malloc (sizeof(*sec), PU_LEVEL, NULL);
	    memcpy (sec, save_p, sizeof(*sec));
	    save_p += sizeof(*sec);
	    sec->sector = &sectors[(int)sec->sector];
	    sec->sector->specialdata[sec->floorOrCeiling] = sec;

	    if (sec->thinker.function.acp1)
	      sec->thinker.function.acp1 = (actionf_p1)T_MoveSector;

            sec->type = Z_Malloc(sizeof(movinPlane_t), PU_LEVEL, NULL);
            memcpy(sec->type, save_p, sizeof(movinPlane_t));
            save_p += sizeof(movinPlane_t);

//	    P_AddThinker (&ceiling->thinker);
	    P_AddActiveSector(sec);
	    break;
/*
	  case tc_door:
	    PADSAVEP();
	    door = Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
	    memcpy (door, save_p, sizeof(*door));
	    save_p += sizeof(*door);
	    door->sector = &sectors[(int)door->sector];
	    door->sector->specialceiling = door;

	    door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
	    P_AddThinker (&door->thinker);
	    break;
*/
/*	  case tc_floor:
	    PADSAVEP();
	    floor = Z_Malloc (sizeof(floormove_t), PU_LEVEL, NULL);
	    memcpy (floor, save_p, sizeof(floormove_t));
	    save_p += sizeof(floormove_t);
	    floor->sector = &sectors[(int)floor->sector];
	    floor->sector->specialdata = floor;
	    floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
            floor->type = DDF_GetFromLineHashTable((int) floor->type);
            P_AddActiveFloor (floor);
	    break; */
				
/*	  case tc_plat:
	    PADSAVEP();
	    plat = Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
	    memcpy (plat, save_p, sizeof(*plat));
	    save_p += sizeof(*plat);
	    plat->sector = &sectors[(int)plat->sector];
	    plat->sector->specialdata = plat;

	    if (plat->thinker.function.acp1)
		plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

	    P_AddThinker (&plat->thinker);
	    P_AddActivePlat(plat);
	    break;
*/
          case tc_light:
            PADSAVEP();
            light = Z_Malloc(sizeof(*light), PU_LEVSPEC, NULL);
            memcpy (light, save_p, sizeof(*light));
            save_p += sizeof(*light);
            light->sector = &sectors[(int)light->sector];
            light->thinker.function.acp1 = (actionf_p1)T_Light;
            P_AddThinker(&light->thinker);
            break;

/*	  case tc_flash:
	    PADSAVEP();
	    flash = Z_Malloc (sizeof(lightflash_t), PU_LEVEL, NULL);
	    memcpy (flash, save_p, sizeof(lightflash_t));
	    save_p += sizeof(lightflash_t);
	    flash->sector = &sectors[(int)flash->sector];
	    flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
	    P_AddThinker (&flash->thinker);
	    break;
				
	  case tc_strobe:
	    PADSAVEP();
	    strobe = Z_Malloc (sizeof(strobe_t), PU_LEVEL, NULL);
	    memcpy (strobe, save_p, sizeof(strobe_t));
	    save_p += sizeof(strobe_t);
	    strobe->sector = &sectors[(int)strobe->sector];
	    strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
	    P_AddThinker (&strobe->thinker);
	    break;
				
	  case tc_glow:
	    PADSAVEP();
	    glow = Z_Malloc (sizeof(glow_t), PU_LEVEL, NULL);
	    memcpy (glow, save_p, sizeof(glow_t));
	    save_p += sizeof(glow_t);
	    glow->sector = &sectors[(int)glow->sector];
	    glow->thinker.function.acp1 = (actionf_p1)T_Glow;
	    P_AddThinker (&glow->thinker);
	    break;
*/
	  default:
	    I_Error ("P_UnarchiveSpecials:Unknown tclass %i in savegame",tclass);
	}
	
    }

}

