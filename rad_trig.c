// (John Cole) Radius Trigger / Tip Code
//
// Uses a hash table to store the triggers to make it quicker for
// processing large numbers of triggers.
//
// ---

#include <stdio.h>
#include <stdlib.h>
#include "i_system.h"
#include "z_zone.h"
#include "doomdef.h"
#include "doomstat.h"
#include "m_swap.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "p_spec.h"
#include "v_res.h"
#include "s_sound.h"
#include "lu_sound.h"
#include "ctype.h"
#include "p_local.h"
#include "m_argv.h"
#include "g_game.h"
#include "r_defs.h"

#define PARSERV       2                    // Radius Trigger Parser Version
#define NUMTICS       35
#define MAXSTRLEN     160
#define MAXTIPLEN     56
#define MINMAP        1
#define MINEPS        1
#define MAXMAP        32
#define MAXEPS        4

#define H_MULTIPLY    33
#define H_MAXSIZE     H_MULTIPLY*MAXEPS    // MaxSize for hash table
#define H_CALC        (((gameepisode-1) * H_MULTIPLY) + (gamemap-1))


//Action Definitions
//Tip Function <"TEXT"> <DISPLAYTIME> <SOUND ON/OFF>
typedef struct {
                 char tip_text[MAXTIPLEN];
                 int  display_time;
                 boolean playsound;
} s_tip_t;

//SpawnThing Function <THINGTYPE> {<x> <y>}
typedef struct s_thing_s {
                 fixed_t          x;    // If the object is spawned somewhere
                 fixed_t          y;    // else on the map.
                 fixed_t          angle;
                 mobjtype_t       thingid;
                 struct s_thing_s *next;
        } s_thing_t;

// Radius Damage Trigger
typedef struct {
                 int damageamount;
        } s_damagep_t;

// Radius Heal Player Trigger
typedef struct {
		 int limit;
                 int healamount;
        } s_healp_t;

// Radius GiveArmour Player Trigger
typedef struct {
		 int limit;
                 int armoramount;
        } s_armor_t;

// Set Skill
typedef struct {
                 skill_t skill;
                 boolean Respawn;
                 boolean FastMonsters;
        } s_skill_t;

// Warp to map Trigger
typedef struct {
                 int gamemap;
        } s_warpto_t;

//PlaySound Function <SOUNDNO> {<x> <y>}
typedef struct s_sound_s {
                 mobj_t           mo;                      // Sound Location
                 int              soundid;
                 struct s_sound_s *next;
        } s_sound_t;

//Sector Vertical movement
typedef struct s_sectorv_s {
                 int         secnum;
                 int         inc;
//               int         limit;
                 boolean     corf; // Ceiling or Floor
                 struct s_sectorv_s *next;
        } s_sectorv_t;

//Sector Vertical movement
typedef struct s_sectorl_s {
                 int         secnum;
                 int         inc;
                 struct s_sectorl_s *next;
        } s_sectorl_t;

//TAGGED_REPEATABLE <NO. OF ITERATIONS> <DELAY> <DIVISOR>
//0 Iterations is Infinite.
typedef struct {
                 int     repetitions_done;           // Is it completed?
                 int     repetitions;                // Tagged_Repeatable
                 int     delay;
                 int     lasttic;
                 int     ticamount;                  // Time
        } s_repeat_t;

// Trigger Definition (Made up of actions)
// Start_Map & Radius_Trigger Declaration
typedef struct radscr_s {
                 int episodeid;                      // For Doom I
                 int mapid;                          // Doom I & II
                 fixed_t radius;                     // Trigger Radius
                 fixed_t x;                          // Map Coordinates
                 fixed_t y;
                 boolean already_done;               // Is it finished?
                 boolean usebutton;                  // Check for use.
                 boolean active;                     // Running?
                 boolean independant;                // Continue Working
                 boolean immediate;                  // Do NOW!

// Not Implemented yet.
//               boolean reverse;
//               int     sleeptime;

                 s_tip_t       *tip;                 // Pointers to Action
                 s_thing_t     *th;                  // data. List
                 s_damagep_t   *damage;
                 s_healp_t     *heal;
                 s_warpto_t    *warpto;
                 s_sound_t     *psound;              // List
                 s_armor_t     *armor;
                 s_skill_t     *skill;
                 s_sectorv_t   *sectorv;             // List
                 s_sectorl_t   *sectorl;             // List

                 s_repeat_t    *rpt;

       struct    radscr_s      *next;                // Next Action
       } radscr_t;

// Tip Displayer info.
typedef struct {
                 char tip_text[MAXTIPLEN];           // Tip Text DOH!
                 int  tip_time;                      // Level Time tip Arrived
                 int  tip_delay;                     // Display Time
                 int  tip_do_once;                   // Sound Related
                 int  tip_width;                     // Width(Pixels) of Tip
                 int  tip_override;                  // Can be overidden?
        } tip_t;

extern int           gameepisode;
extern int           gamemap;
extern patch_t*	     hu_font[HU_FONTSIZE];
       hu_textline_t tips;                           // Player Tips
       tip_t         ptips = {{0},0,0,false,0,false};// Set up the tip struct.
       radscr_t      *rscript[H_MAXSIZE];            // Script Hash Table
       int           itemsread=0;                    // # Triggers
       int           lastmap=0;                      // Used by parser.
       int           lastepisode=1;


// TIP Functions
// Remember a tip which emits a sound has a higher priority than one without,
// so if one in use emits no sound it is terminated abruptly.

void TIP_SendTip(char *tiptext, int showtime, boolean dosound)
{
   int j;

   if (ptips.tip_override && !dosound) return;

   ptips.tip_delay   =showtime;
   ptips.tip_time    =leveltime;
   ptips.tip_override=ptips.tip_do_once=true & dosound;
   ptips.tip_width   =0;

   // Calculate the text width so it can be centered.
   for (j=0;j<sprintf(ptips.tip_text,"%s",tiptext);j++)
       ptips.tip_width+=(ptips.tip_text[j]=' ') ? 7 : SHORT(hu_font[(toupper(ptips.tip_text[j])-HU_FONTSTART)]->width);

}

void TIP_DisplayTips(int y)
{
    char *tipptr=&ptips.tip_text[0];
    int x=((SCREENWIDTH/2)-(ptips.tip_width/2));

    // Is there actually a tip to display
    if (ptips.tip_time==0) return;

    // If the display time is up reset the tip and erase it.
    if (!(leveltime < (ptips.tip_time+(NUMTICS*ptips.tip_delay)))) {
       ptips.tip_time    =0;
       ptips.tip_override=false;
       HUlib_eraseTextLine(&tips);
       return;
    }

    // Return if the width is 0 (Shouldn't happen but it's better to be
    // safe than sorry.
    if (ptips.tip_width<0 || ptips.tip_width>SCREENWIDTH) return;

    // Make a noise when the tip is first displayed.
    // Note: This happens only once. (do_once)
    if (ptips.tip_do_once) {
       ptips.tip_do_once=false;
       S_StartSound(0, sfx_tink);
    }

    // Dump it to the screen
    HUlib_initTextLine(&tips, x, y, hu_font, HU_FONTSTART);

    while (*tipptr)  HUlib_addCharToTextLine(&tips, *(tipptr++));

//  HU_NewColor(36);
    HUlib_drawTextLine(&tips, false);
//  HU_LastColor();
}


void ResetTipStructure(void)
{
    ptips.tip_text[0] =0;
    ptips.tip_time    =0;
    ptips.tip_delay   =0;
    ptips.tip_time    =0;
    ptips.tip_override=false;
    ptips.tip_width   =0;
}

// Radius Trigger Code & Script Parser --------------------------------------

// Resets the triggers so that they can be used again, eg. when restarting
// a single player game.
void ResetRadiTriggers(void)
{
    int i;

    // Can't happen in a netgame.
    if (netgame) return;

    for (i=0;i<H_MAXSIZE;i++) {
         radscr_t *radscr=rscript[i];

         for (;radscr;radscr=radscr->next) {
             radscr->already_done=false;
             if (radscr->rpt!=NULL) {
                radscr->rpt->lasttic=0;
                radscr->rpt->repetitions_done=0;
             }
             radscr->active=false;
         }
    }
    ResetTipStructure();
}

// Clear timers so if warp was used to go back to a previous level the
// triggers activate immediately.
void ClearRadiTriggersTimers(void)
{
    int i;

    for (i=0;i<H_MAXSIZE;i++) {
        radscr_t *radscr=rscript[i];

        for (;radscr;radscr=radscr->next) {
            if (radscr->rpt!=NULL) {
               radscr->rpt->lasttic=0;
            }
            radscr->active=false;
        }
    }
}

// If SectorV is used update the map objects within that sector
// accordingly.
extern boolean P_ThingHeightClip (mobj_t* thing);

void UpdateThingsinSector(sector_t *sec)
{
 mobj_t *t=sec->thinglist;

 for (;t;t=t->snext) {
     if (!P_ThingHeightClip(t)) {
        P_DamageMobj(t, NULL, NULL,MAXHEALTH);
     }
 }
}

// True means the trigger is free to proceed.
// False means the trigger must wait.
boolean TriggerCheckGameTime(radscr_t *trig)
{
  int TICS=trig->rpt->ticamount; // Each trigger can have its own repeat rate.

  // Is this the first time?
  if (trig->rpt->lasttic==0) {
      trig->rpt->lasttic=leveltime+(TICS*trig->rpt->delay);
      ++trig->rpt->repetitions_done;
  }
  else {
       if (leveltime>trig->rpt->lasttic+(TICS*trig->rpt->delay)) {
           trig->rpt->lasttic=leveltime+(TICS*trig->rpt->delay);
           ++trig->rpt->repetitions_done;
       }
       else
           return false;
  }

  // If the TAGGED_REPEATABLE instruction was used make sure it stops
  // after n iterations.
  if (trig->rpt->repetitions_done==trig->rpt->repetitions) {
     trig->already_done=true;
  }

  return true;
}


boolean WithinRadius(player_t *p, radscr_t *r)
{
    fixed_t dist;
    fixed_t dx, dy;

    // Calculate the distance of player from it's centerpoint.
    dx=abs(p->mo->x - r->x);
    dy=abs(p->mo->y - r->y);
    dist=(dx<dy)?dx+dy-(dx>>1):dx+dy-(dy>>1);

    return (dist>=r->radius*FRACUNIT+p->mo->radius) ? false : true;
}


// Called by P_PlayerThink
// Radius Trigger Event handler.
void DoRadiTrigger(player_t *p)
{
    radscr_t *radscr=rscript[H_CALC] ;

    // Start looking through the trigger list.
    for (;radscr;radscr=radscr->next) {

        // Don't process, if its already done.
        if (radscr->already_done) continue;

        // Immediate triggers are just that. Immediate.
        if (!radscr->immediate) {
           // Not within range so skip it and it's not a use trigger which
           // is active.
           if (!WithinRadius(p,radscr) && !radscr->active) continue;

           // Check for use key trigger.
           if (!p->usedown && radscr->usebutton && !radscr->active)
              continue;
           else
              if (radscr->usebutton) radscr->active=true;

           // Independant, means you don't have to stay within the trigger
           // radius for it to operate, It will operate on it's own.
           if (radscr->independant && !radscr->active) radscr->active=true;
        }

        // Okay, the player must be within the required radius.
        // Is it repeatable? (Tagged_Repeatable)
        if (radscr->rpt!=NULL) {
           if (!TriggerCheckGameTime(radscr))
              continue;
        }
        else {
           // Non Repeatable Radius trigger.
           radscr->already_done=true;
        }

        // Only Display the tip to the player that stepped into the radius
        // trigger.
        if (radscr->tip!=NULL && p==&players[consoleplayer]) {
           TIP_SendTip(radscr->tip->tip_text,radscr->tip->display_time,radscr->tip->playsound);
        }

        // These *MUST* happen to everyone to keep netgames consistent.
        // Spawn a new map object.
        if (radscr->th!=NULL) {
           s_thing_t *t=radscr->th;

           // Step through the list
           for (;t;t=t->next) {
             mobj_t  *mo;

             mo = P_SpawnMobj (t->x,t->y, ONFLOORZ, MT_IFOG);
             S_StartSound (mo, sfx_itmbk);
             mo=P_SpawnMobj(t->x,t->y, ONFLOORZ, t->thingid);
             mo->angle = t->angle;

             mo->spawnpoint.x=SHORT(t->x >> FRACBITS);
             mo->spawnpoint.y=SHORT(t->y >> FRACBITS);
             mo->spawnpoint.angle=SHORT(t->angle >> FRACBITS);
             mo->spawnpoint.type=SHORT(mobjinfo[t->thingid].doomednum);
           }
        }

        // Make sure these can happen to everyone within the radius.
        // Damage the player(s)
        if (radscr->damage!=NULL) {
           int i;
           for (i=0;i<MAXPLAYERS;i++) {
               if (!playeringame[i]) continue;
               if (WithinRadius(&players[i],radscr)) {
                  P_DamageMobj(players[i].mo, NULL, NULL,radscr->damage->damageamount);
               }
           }
        }

        // Heal the player(s)
        if (radscr->heal!=NULL) {
           int i;
           for (i=0;i<MAXPLAYERS;i++) {
               if (!playeringame[i]) continue;
               if (WithinRadius(&players[i],radscr)) {
                  if (players[i].health>radscr->heal->limit)
                     continue;

                  if (players[i].health+radscr->heal->healamount >
                      radscr->heal->limit)
                     players[i].health=radscr->heal->limit;
                  else
                     players[i].health+=radscr->heal->healamount;

                  players[i].mo->health=players[i].health;
               }
           }
        }

        // Armor for player(s)
        if (radscr->armor!=NULL) {
           int i;
           for (i=0;i<MAXPLAYERS;i++) {
               if (!playeringame[i]) continue;
               if (WithinRadius(&players[i],radscr)) {
                  if (players[i].armorpoints>radscr->armor->limit)
                     continue;

                  if (players[i].armorpoints+radscr->armor->armoramount >
                      radscr->armor->limit)
                     players[i].armorpoints=radscr->armor->limit;
                  else
                     players[i].armorpoints+=radscr->armor->armoramount;

                  // I hope this is right.
                  if (players[i].armorpoints>0 && players[i].armorpoints<100)
                     players[i].armortype=GREENARMOUR;
                  else
                     players[i].armortype=(players[i].armorpoints>100) ?
                                                      BLUEARMOUR : NORMARMOUR;
               }
           }
        }

        // Warp to level n
        if (radscr->warpto!=NULL) {
           gamemap    =radscr->warpto->gamemap;
           G_ExitLevel();
        }

        // Ambient sound
        if (radscr->psound!=NULL) {
           s_sound_t *t=radscr->psound;

           // Step through the list
           for (;t;t=t->next)
             S_StartSound (&t->mo,t->soundid);
        }

        // Skill selection trigger function
        if (radscr->skill!=NULL) {
           gameskill      =radscr->skill->skill;
           fastparm       =radscr->skill->FastMonsters;
           respawnmonsters=radscr->skill->Respawn;

           // This may have to change with the advent of the DosDoom
           // Definition file.
           if (fastparm) {
              int j;
              for (j=S_SARG_RUN1 ; j<=S_SARG_PAIN2 ; j++) states[j].tics>>=1;
              mobjinfo[MT_BRUISERSHOT].speed =
              mobjinfo[MT_HEADSHOT].speed    =
              mobjinfo[MT_TROOPSHOT].speed   = 20*FRACUNIT;
           }
           else {
              int j;
              for (j=S_SARG_RUN1 ; j<=S_SARG_PAIN2 ; j++) states[j].tics<<=1;
              mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
              mobjinfo[MT_HEADSHOT].speed    =
              mobjinfo[MT_TROOPSHOT].speed   = 10*FRACUNIT;
           }

        }

        // SectorV
        if (radscr->sectorv!=NULL) {
           s_sectorv_t *t=radscr->sectorv;
           // Step through the list
           for (;t;t=t->next) {
               if (t->corf) // Floor Height
                  sectors[t->secnum].floorheight += t->inc;
               else         // Ceiling Height
                  sectors[t->secnum].ceilingheight -= t->inc;
               UpdateThingsinSector(&sectors[t->secnum]);
           }
        }

        // SectorL (Lighting)
        if (radscr->sectorl!=NULL) {
           s_sectorl_t *t=radscr->sectorl;
           // Step through the list
           for (;t;t=t->next) sectors[t->secnum].lightlevel += t->inc;
        }

   }
}

//-- Parsing Stuff -----------------------------------------------------------

typedef enum {int_t, char_t, formula_t} type_t;

typedef struct define_s {
        char     name[80];
        int      valuei;
        char     *valuec;
        type_t   dtype;
        struct   define_s *next;
} define_t;

// Error messages
char StrERR[12][50]={"Unknown Error",
                     "START_MAP, block not terminated",
                     "END_MAP with no START_MAP",
                     "RADIUS_TRIGGER, block not terminated",
                     "END_RADIUSTRIGGER with no RADIUS_TRIGGER",
                     "Unknown Function",
                     "Parameter is not of integer type",
                     "Wrong number of parameters",
                     "Invalid number of quotes",
                     "Variable Unknown",
                     "Function cannot be redefined",
                     "Integer not within range specified"
};

define_t *defines;   // Define List

boolean AlreadyShutdown;

int ParseStatusFail=false; // True when a parse error has occured
int ErrorCode=0;           // Error code used with strERR
int ENDMAPcheck=0;
int ENDRADcheck=0;         // Determine whether the code blocks are started
                           // and terminated.
int hashloc=0;             // Location within hash table

void SetParseError(int errno)
{
     ErrorCode=errno;
     ParseStatusFail=true;
}


//Searches through the #defines namespace for a match and returns it's value
//if it exists.
boolean CheckForDefine(char *s, int *val)
{
  define_t *tempnode=defines;

  for (;tempnode;tempnode=tempnode->next) {
     if (!strcmp(s,tempnode->name)) {
        *val=tempnode->valuei;
        return true;
     }
  }
  return false;
}


// Returns true if the string can be converted to an integer.
boolean CheckForInt(char *value, int *retvalue)
{
  char *temp=value;
  int count=0;
  int val;
  int length=strlen(value);

  // Accomodate for "-" as you could have -5 or something like that.
  if (*value=='-') {
      count++;
      value++;
  }
  while (isdigit(*(value++))) count++;

  // Is the value an integer?
  if (length==count) {
     *retvalue=atoi(temp);
     return true;
  }

  // Maybe the value passed is a #define, better check.
  if (CheckForDefine(temp,&val)) {
     *retvalue=val;
     return true;
  }

  SetParseError(6);
  return false;
}


boolean CheckForComments(char c)
{
   if (c==';' || c==0) return true;

   return false;
}

// Makes sure there is part of the string enclosed in quotes and returns it
// back through "sout". If it was successful a pointer to the rest of the
// string is returned for further parsing, otherwise NULL is returned as
// there must have been an error.
char *CheckForQuotes(char *s, char *sout)
{
 int check=0;
 char *c=s;

 while (*(++s)!='\0') {
   if (*s=='"') ++check;
 }

 if (check<2 || check > 2) {
    SetParseError(8);
    return NULL;
 }

 while (*(++c)!='"');
 while (*(++c)!='"') *(sout++)=*c;
 *sout='\0';

 return ++c;

}


boolean CheckForBoolean(char *s)
{
   int val;

   if (!strcmp("TRUE",s)) return true;
   if (!strcmp("FALSE",s)) return false;

   // This could actually be a define, better go check it.
   if (CheckForDefine(s,&val)) {
      switch (val) {
        case 0:return false;
        case 1:return true;
      }
   }

   // Nope, it's an error.
   SetParseError(9);
   return false;
}

boolean CheckBetween(int i1, int i2, int j1, int j2)
{
  if (i1>=i2 && j1<=j2) return true;
  SetParseError(11);
  return false;
}


boolean CheckItems(int noitems, int wantedno)
{
    if (noitems!=wantedno) {
       SetParseError(7);
       return false;
    }

    return true;
}

// Script Parser
// This will be broken down for the next release.
void ParseLine(char *s)
{
     char istr1[MAXSTRLEN]={0},
          istr2[MAXSTRLEN]={0},
          istr3[MAXSTRLEN]={0},
          istr4[MAXSTRLEN]={0},
          istr5[MAXSTRLEN]={0};

     int  val=0;
     int  items=0;

     // Ignore Comments / Blank Lines
     sscanf(s,"%s",istr1);
     if (CheckForComments(istr1[0])) return;


     // #define
     // #define <identifier> <n>
     if (!strcmp("#DEFINE",istr1)) {
        define_t *newnode;
        if (!CheckItems(sscanf(s,"%s %s %s",istr1, istr2, istr3),3)) return;
        if (!CheckForInt(istr3,&val)) return;
        newnode=(define_t *) calloc(1,sizeof(define_t));
        strcpy(newnode->name,istr2);
        newnode->valuei=val;
        newnode->next=defines;
        defines=newnode;
        return;
     }


     // Start Checking
     // Start_Map <map> || <episode> <map>
     if (!strcmp("START_MAP",istr1)) {
        if (ENDMAPcheck>0) {
           SetParseError(1);
           return;
        }

        ++ENDMAPcheck;
        items=sscanf(s,"%s %s %s",istr1, istr2, istr3);

        if (CheckForComments(istr3[0])) items=2;

        switch (items) {
          case 2 :                                        // Must be Doom2
               if (!CheckForInt(istr2,&lastmap)) return;
               lastepisode=1;
               break;
          case 3 :
               if (!(CheckForInt(istr3,&lastmap)       &&
                     CheckForInt(istr2,&lastepisode))) return;
               break;
        }

        if (!(CheckBetween(lastmap    ,MINMAP,lastmap    ,MAXMAP)   &&
              CheckBetween(lastepisode,MINEPS,lastepisode,MAXEPS))) return;

        hashloc=((lastepisode-1) * H_MULTIPLY) + (lastmap-1);
        return;
     }

     // Radius Trigger (converts to fixed point)
     // Radius_Trigger <x> <y> <radius>
     if (!strcmp("RADIUSTRIGGER",istr1)) {
        radscr_t *newnode;

        if (ENDRADcheck>0) {
           SetParseError(3);
           return;
        }

        ++ENDRADcheck;

        // Set the node up, from now on we can use rscript as it points
        // to the new node.
        newnode=(radscr_t *) calloc(1,sizeof(radscr_t));
        newnode->next=rscript[hashloc];
        rscript[hashloc]=newnode;

        if (!CheckItems((sscanf(s,"%s %s %s %s", istr1, istr2, istr3, istr4)),4)) return;
        if (!(CheckForInt(istr2,&rscript[hashloc]->x)        &&
              CheckForInt(istr3,&rscript[hashloc]->y)        &&
              CheckForInt(istr4,&rscript[hashloc]->radius))) return;

        rscript[hashloc]->x<<=FRACBITS;
        rscript[hashloc]->y<<=FRACBITS;

        return;
     }

     // Tagged Repeatable
     // Tagged_Repeatable <no. of repetitions> <time in seconds> <divisor>
     if (!strcmp("TAGGED_REPEATABLE",istr1)) {
        if (!CheckItems(sscanf(s,"%s %s %s %s", istr1, istr2, istr3, istr4),4)) return;
        if (rscript[hashloc]->rpt) {
           SetParseError(10);
           return;
        }
        rscript[hashloc]->rpt=(s_repeat_t *) calloc(1,sizeof(s_repeat_t));

        if (!(CheckForInt(istr2,&rscript[hashloc]->rpt->repetitions) &&
              CheckForInt(istr3,&rscript[hashloc]->rpt->delay)       &&
              CheckForInt(istr4,&rscript[hashloc]->rpt->ticamount))) return;

        rscript[hashloc]->rpt->lasttic         =0;
        rscript[hashloc]->rpt->repetitions_done=0;
        return;
     }

     // Use
     if (!strcmp("TAGGED_USE",istr1)) {
        rscript[hashloc]->usebutton=true;
        return;
     }


     // Independant
     if (!strcmp("TAGGED_INDEPENDANT",istr1)) {
        rscript[hashloc]->independant=true;
        return;
     }

     // Immediate
     if (!strcmp("TAGGED_IMMEDIATE",istr1)) {
        rscript[hashloc]->immediate=true;
        return;
     }

     // Tip
     // Tip "<Text>" <Display Time> <Sound(True)/NoSound(False)>
     if (!strcmp("TIP",istr1)) {
        char *tc;

        if (rscript[hashloc]->tip) {
           SetParseError(10);
           return;
        }
        rscript[hashloc]->tip=(s_tip_t *) calloc(1,sizeof(s_tip_t));

        if ((tc=CheckForQuotes(s,rscript[hashloc]->tip->tip_text))==NULL) return;
        if (!CheckItems(sscanf(tc,"%s %s",istr1,istr2),2)) return;
        if (!CheckForInt(istr1,&rscript[hashloc]->tip->display_time)) return;

        rscript[hashloc]->tip->playsound=CheckForBoolean(istr2) ? true : false;
        return;
     }

    // Spawnthing
    // Spawnthing <thingid>
    // Spawnthing <thingid> {<x> <y>}
    if (!strcmp("SPAWNTHING",istr1)) {
       s_thing_t *newnode;
       items=sscanf(s,"%s %s %s %s %s", istr1, istr2, istr3, istr4, istr5);

       if (CheckForComments(istr4[0])) items=3;

       switch (items) {
       case 3 :
          newnode=(s_thing_t *) calloc(1,sizeof(s_thing_t));

          if (!(CheckForInt(istr2,&val)              &&
                CheckForInt(istr3,&newnode->angle))) return;

          newnode->thingid = val;
          newnode->x       = rscript[hashloc]->x;
          newnode->y       = rscript[hashloc]->y;
          newnode->angle <<= FRACBITS;

          newnode->next=rscript[hashloc]->th;
          rscript[hashloc]->th=newnode;
          break;
       case 5:
          newnode=(s_thing_t *) calloc(1,sizeof(s_thing_t));

          if (!(CheckForInt(istr2,&val)              &&
                CheckForInt(istr3,&newnode->x)       &&
                CheckForInt(istr4,&newnode->y)       &&
                CheckForInt(istr5,&newnode->angle))) return;

          newnode->thingid = val;
          newnode->x     <<= FRACBITS;
          newnode->y     <<= FRACBITS;
          newnode->angle <<= FRACBITS;

          newnode->next=rscript[hashloc]->th;
          rscript[hashloc]->th=newnode;
          break;

        default:
          SetParseError(7);
          break;
       }

       return;
    }

    // PlaySound
    // PlaySound <soundid>
    // PlaySound <soundid> {<x> <y>}
    if (!strcmp("PLAYSOUND",istr1)) {
       s_sound_t *newnode;
       items=sscanf(s,"%s %s %s %s", istr1, istr2, istr3, istr4);
       if (CheckForComments(istr3[0])) items=2;

       switch (items) {
       case 2 :
          newnode=(s_sound_t *) calloc(1,sizeof(s_sound_t));
          if (!CheckForInt(istr2,&val)) return;
          newnode->soundid = val;
          newnode->mo.x    = rscript[hashloc]->x;
          newnode->mo.y    = rscript[hashloc]->y;
          newnode->mo.z    = ONFLOORZ;
          newnode->next=rscript[hashloc]->psound;
          rscript[hashloc]->psound=newnode;
          break;
       case 4:
          newnode=(s_sound_t *) calloc(1,sizeof(s_sound_t));
          if (!(CheckForInt(istr2,&val) &&
                CheckForInt(istr3,&newnode->mo.x)   &&
                CheckForInt(istr4,&newnode->mo.y))) return;
          newnode->soundid = val;
          newnode->mo.x  <<= FRACBITS;
          newnode->mo.y  <<= FRACBITS;
          newnode->mo.z    = ONFLOORZ;
          newnode->next=rscript[hashloc]->psound;
          rscript[hashloc]->psound=newnode;
          break;
        default:
          SetParseError(7);
          break;
       }

       return;
    }

    // DamagePlayer
    // DamagePlayer <n>
    if (!strcmp("DAMAGEPLAYER",istr1)) {
        if (!CheckItems(sscanf(s,"%s %s",istr1 , istr2),2)) return;

        if (rscript[hashloc]->damage) {
           SetParseError(10);
           return;
        }

       rscript[hashloc]->damage=(s_damagep_t *) calloc(1,sizeof(s_damagep_t));

       if (!CheckForInt(istr2,&rscript[hashloc]->damage->damageamount))
          return;

       return;
    }

    // HealPlayer
    // HealPlayer <n> <limit>
    if (!strcmp("HEALPLAYER",istr1)) {
        if (!CheckItems(sscanf(s,"%s %s %s",istr1 , istr2, istr3),3)) return;

        if (rscript[hashloc]->heal) {
           SetParseError(10);
           return;
        }

       rscript[hashloc]->heal=(s_healp_t *) calloc(1,sizeof(s_healp_t));

       if (!(CheckForInt(istr2,&rscript[hashloc]->heal->healamount) &&
             CheckForInt(istr3,&rscript[hashloc]->heal->limit)))    return;

       if (!CheckBetween(rscript[hashloc]->heal->limit,0,rscript[hashloc]->heal->limit,MAXHEALTH))
          rscript[hashloc]->heal->limit=MAXHEALTH;

       return;
    }


    // GiveArmour
    // GiveArmour <n> <limit>
    if (!strcmp("GIVEARMOR",istr1)) {
       if (!CheckItems(sscanf(s,"%s %s %s",istr1 , istr2, istr3),3)) return;
       if (rscript[hashloc]->armor) {
          SetParseError(10);
          return;
       }
       rscript[hashloc]->armor=(s_armor_t *) calloc(1,sizeof(s_armor_t));
       if (!(CheckForInt(istr2,&rscript[hashloc]->armor->armoramount)  &&
             CheckForInt(istr3,&rscript[hashloc]->armor->limit)))      return;

       if (!CheckBetween(rscript[hashloc]->armor->limit,0,rscript[hashloc]->armor->limit,MAXARMOUR))
          rscript[hashloc]->armor->limit=MAXARMOUR;

       return;
    }

    // Change Skill
    // Skill <n> {<n>} {<n>}
    if (!strcmp("SKILL",istr1)) {
       if (!CheckItems(sscanf(s,"%s %s %s %s",istr1 , istr2, istr3, istr4),4)) return;
       if (rscript[hashloc]->skill) {
          SetParseError(10);
          return;
       }
       rscript[hashloc]->skill=(s_skill_t *) calloc(1,sizeof(s_skill_t));
       if (!CheckForInt(istr2,&val)) return;

       rscript[hashloc]->skill->skill       =val;
       rscript[hashloc]->skill->Respawn     =CheckForBoolean(istr3) ? true : false;
       rscript[hashloc]->skill->FastMonsters=CheckForBoolean(istr4) ? true : false;
       return;
    }

    // GotoMap
    // GotoMap <map>                            Doesn't support Doom1 anymore.
    if (!strcmp("GOTOMAP",istr1)) {
       if (!CheckItems(items=sscanf(s,"%s %s",istr1 , istr2),2)) return;
       if (rscript[hashloc]->warpto) {
          SetParseError(10);
          return;
       }
       rscript[hashloc]->warpto=(s_warpto_t *) calloc(1,sizeof(s_warpto_t));
       if (!CheckForInt(istr2,&rscript[hashloc]->warpto->gamemap)) return;
       rscript[hashloc]->warpto->gamemap-=1;
       return;
    }

    // Vertical Sector Movement
    // SectorV <n>
    if (!strcmp("SECTORV",istr1)) {
       s_sectorv_t *newnode;

       if (!CheckItems(sscanf(s,"%s %s %s %s",istr1 , istr2, istr3, istr4),4)) return;

       newnode=(s_sectorv_t *) calloc(1,sizeof(s_sectorv_t));

       if (!(CheckForInt(istr2,&newnode->secnum) &&
             CheckForInt(istr3,&newnode->inc)))  return;

       newnode->inc <<=FRACBITS;
       newnode->corf  =CheckForBoolean(istr4) ? true : false;
       newnode->next=rscript[hashloc]->sectorv;
       rscript[hashloc]->sectorv=newnode;

       return;
    }

    // Sector Lights
    // SectorL <n> <light>
    if (!strcmp("SECTORL",istr1)) {
       s_sectorl_t *newnode;

       if (!CheckItems(sscanf(s,"%s %s %s",istr1 , istr2, istr3),3)) return;
       newnode=(s_sectorl_t *) calloc(1,sizeof(s_sectorl_t));
       if (!(CheckForInt(istr2,&newnode->secnum) &&
             CheckForInt(istr3,&newnode->inc)))  return;

       newnode->next=rscript[hashloc]->sectorl;
       rscript[hashloc]->sectorl=newnode;
       return;
    }


    if (!strcmp("END_RADIUSTRIGGER",istr1)) {
        if (ENDRADcheck<=0) {
           SetParseError(4);
           return;
        }
       rscript[hashloc]->already_done=false;
       rscript[hashloc]->mapid       =lastmap;
       rscript[hashloc]->episodeid   =lastepisode;
       ++itemsread;
       --ENDRADcheck;
       return;
    }

    if (!strcmp("END_MAP",istr1)) {
        if (ENDMAPcheck<=0) {
           SetParseError(2);
           return;
        }
       --ENDMAPcheck;
       return;
    }

    // Really if it get's to here an error should be produced,
    SetParseError(5);
}

// Free allocated memory.
void RadiShutdown(void)
{
   int i;
   define_t *deft=defines;

   // Don't try to release memory if it's been shutdown before due to a
   // parser error.
   if (AlreadyShutdown) return;

   //Release #define
   for (;deft;deft=deft->next) free(deft);


   for (i=0;i<H_MAXSIZE;i++) {
       radscr_t *tempnode=rscript[i];

       // Free Action Lists and Trigger Lists
       for (;tempnode;tempnode=tempnode->next) {
           if (tempnode->th !=NULL){
              s_thing_t *t=tempnode->th;
              for (;t;t=t->next) free(t);
           }
           if (tempnode->tip!=NULL)     free(tempnode->tip);
           if (tempnode->rpt!=NULL)     free(tempnode->rpt);
           if (tempnode->damage!=NULL)  free(tempnode->damage);
           if (tempnode->heal!=NULL)    free(tempnode->heal);
           if (tempnode->warpto!=NULL)  free(tempnode->warpto);
           if (tempnode->psound!=NULL) {
              s_sound_t *t=tempnode->psound;
              for (;t;t=t->next) free(t);
           }
           if (tempnode->armor!=NULL)   free(tempnode->armor);
           if (tempnode->skill!=NULL)   free(tempnode->skill);
           if (tempnode->sectorv!=NULL) {
              s_sectorv_t *t=tempnode->sectorv;
              for (;t;t=t->next) free(t);
           }
           if (tempnode->sectorl!=NULL) {
              s_sectorl_t *t=tempnode->sectorl;
              for (;t;t=t->next) free(t);
           }
           free(tempnode);
       }
   }

   // Already Shutdown is use for parser errors.
   AlreadyShutdown=true;
}


void LoadScript(void)
{
     FILE *fp;
     char s[MAXSTRLEN];
     int  n=0;
     int  lineno=0;
     int  cparm;

     if (!(cparm=M_CheckParm("-script"))) return;

     printf ("Radius Trigger Script Parser v%i.%i\n",PARSERV/100,PARSERV%100);

     if ((fp=fopen(myargv[cparm+1],"r"))==NULL) {
       printf("Error : File %s not found.\n",myargv[cparm+1]);
       getchar();
       return;
     }

     printf ("Script Found, Parsing");

     while (fgets(s,MAXSTRLEN-1,fp)) {
           strupr(s);
           ParseLine(s);
           if (n!=itemsread) printf(".");
           n=itemsread;

           if (ParseStatusFail) {
              printf("\nError: %s, at line %d.\n%s\n",
                                                 StrERR[ErrorCode],lineno,s);
              fclose(fp);
              RadiShutdown();
              itemsread=0;
              getchar();
              return;
           }
           lineno++;
     }

     fclose(fp);

     printf ("\n%d blocks parsed.\n",itemsread);

}

//-- End of Parsing Stuff ----------------------------------------------------

