//
// DOSDoom Finale Code on Game Completion
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT).
//
// -KM- 1998/07/21 Clear the background behind those end pics.
// -KM- 1998/09/27 sounds.ddf stuff: seesound -> DDF_LookupSound(seesound)
// -KM- 1998/11/25 Finale generalised.
//
#include <stdio.h>
#include <ctype.h>

// Functions.
#include "i_system.h"
#include "i_allegv.h"
#include "m_swap.h"
#include "z_zone.h"
#include "v_res.h"
#include "w_wad.h"
#include "s_sound.h"

// Data.
#include "dstrings.h"
#include "lu_sound.h"

#include "dm_state.h"
#include "m_random.h"
#include "r_state.h"
#include "f_finale.h"

#include "ddf_main.h"
#include "p_action.h"

typedef enum
{
  f_text,
  f_pic,
  f_bunny,
  f_cast,
  f_end
} finalestage_t;
// Stage of animation:
//  0 = text, 1 = art screen, 2 = character cast
static finalestage_t finalestage;

static int finalecount;
static int picnum;

#define	TEXTSPEED	3
#define	TEXTWAIT	250

#define	e1text E1TEXT
#define	e2text E2TEXT
#define	e3text E3TEXT
#define	e4text E4TEXT

#define	c1text C1TEXT
#define	c2text C2TEXT
#define	c3text C3TEXT
#define	c4text C4TEXT
#define	c5text C5TEXT
#define	c6text C6TEXT

#define	p1text P1TEXT
#define	p2text P2TEXT
#define	p3text P3TEXT
#define	p4text P4TEXT
#define	p5text P5TEXT
#define	p6text P6TEXT

#define	t1text T1TEXT
#define	t2text T2TEXT
#define	t3text T3TEXT
#define	t4text T4TEXT
#define	t5text T5TEXT
#define	t6text T6TEXT

static char*	finaletext;

static gameaction_t newgameaction;
static finale_t*    finale;
void	F_StartCast (void);
void	F_CastTicker (void);
boolean F_CastResponder (event_t *ev);
void	F_CastDrawer (void);

//
// F_StartFinale
//
void F_StartFinale (finale_t* f, gameaction_t newaction)
{
    int i;
    finalestage = f_text;
    finalecount = 0;
    gameaction = ga_nothing;
    viewactive = false;
    automapactive = false;
    finale = f;
    newgameaction = newaction;
    picnum = 0;
    for (i=0 ; i<maxplayers ; i++)
	players[i].cmd.buttons = 0;
    F_Ticker();
}



boolean F_Responder (event_t *event)
{
    if (finalestage == f_cast)
	return F_CastResponder (event);
	
    return false;
}


//
// F_Ticker
//
void F_Ticker (void)
{
    int		i;
    int         fstage = finalestage;
    
    // check for skipping
    // If a player presses a key, advance
    for (i=0 ; i<maxplayers ; i++)
    {
	if (players[i].cmd.buttons)
        {
          // -KM- 1998/12/16 Reset buttons to 0 so only accelerate
          //  one stage.
          players[i].cmd.buttons = 0;
	  break;
        }
    }
				
    // -KM- 1998/12/16 Don't accelerate final stage.
    if ((i < maxplayers) && (newgameaction != ga_nothing))
    {
        finalestage++;
        finalecount = 0;
    }

    switch (finalestage)
    {
      case f_text:
        if (finale->text)
        {
          gamestate = GS_FINALE;
          if (!finalecount)
          {
            finaletext = DDF_LanguageLookup(finale->text);
            S_ChangeMusic(finale->music, true);
            wipegamestate = -1;
            break;
          } else if (finalecount>strlen (finaletext)*TEXTSPEED + TEXTWAIT)
          {
            finalecount = 0;
          } else
              break;
        }
        finalestage++;
      case f_pic:
        if (finale->pics)
        {
          gamestate = GS_FINALE;
          if (finalecount > finale->picwait)
          {
            finalecount = 0;
            picnum++;
          }

          if (picnum >= finale->numpics)
          {
            finalecount = 0;
            picnum = 0;
          } else break;
        }
        finalestage++;
      case f_bunny:
           if (finale->dobunny)
           {
             gamestate = GS_FINALE;
             if (!finalecount)
             {
               int i;
               char buffer[9];

  	       wipegamestate = -1;		// force a wipe

               W_CacheLumpName("PFUB1", PU_STATIC);
               W_CacheLumpName("PFUB2", PU_STATIC);
               for (i = 0; i <= 6; i++) {
                 sprintf(buffer, "END%d", i);
                 W_CacheLumpName(buffer, PU_STATIC);
               }

             S_ChangeMusic ("d_bunny", false);
             }
             break;
           }
           finalestage++;
      case f_cast:
           if (finale->docast)
           {
             gamestate = GS_FINALE;
             if (!finalecount)
               F_StartCast();
             else
               F_CastTicker();
             break;
           }
           finalestage++;
      case f_end:
           if (newgameaction != ga_nothing)
             gameaction = newgameaction;
           else
             finalestage = fstage;
           break;
    }

    if (finalestage != fstage && finalestage != f_end)
      wipegamestate = -1;

    // advance animation
    finalecount++;
	
}



//
// F_TextWrite
//

#include "hu_stuff.h"
extern	patch_t *hu_font[HU_FONTSIZE];


void F_TextWrite (void)
{
    int		w;
    int		count;
    char*	ch;
    int		c;
    int		cx;
    int		cy;
    
    // 98-7-10 KM erase the entire screen to a tiled background
    if (finale->text_flat[0])
      V_TextureBackScreen(finale->text_flat);
    else
    {
      V_ClearPageBackground(0);
      V_DrawPatchInDirect (0,0,0,
        W_CacheLumpName(finale->text_back,PU_CACHE));
    }
    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);
    
    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;
	
    count = (finalecount - 10)/finale->text_speed;
    if (count < 0)
	count = 0;
    for ( ; count ; count-- )
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = 10;
	    cy += 11;
	    continue;
	}
		
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (cx+w > SCREENWIDTH)
	    break;
	V_DrawPatchInDirect(cx, cy, 0, hu_font[c]);
	cx+=w;
    }
	
}

//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
extern mobjinfo_t oldmobjinfo[ORIG_NUMMOBJTYPES];

mobjinfo_t *castorder;

int		castnum;
int		casttics;
state_t*	caststate;
boolean		castdeath;
int		castframes;
int		castonmelee;
boolean		castattacking;

mobjinfo_t* F_GetNextActor(void)
{
    mobjinfo_t* ttype = mobjinfohead;
    // search for a matching cast
    while ( (ttype != NULL) && (castnum != ttype->castorder) )
            ttype = ttype->next;

    if (!ttype)
    {
      castnum = 1;
      return F_GetNextActor();
    }

    return ttype;
}

//
// F_StartCast
//
extern	gamestate_t     wipegamestate;


void F_StartCast (void)
{
    wipegamestate = -1;		// force a screen wipe
    castnum = 2;
    castorder = F_GetNextActor();
    caststate = &states[castorder->seestate];
    casttics = caststate->tics;
    castdeath = false;
    castframes = 0;
    castonmelee = 0;
    castattacking = false;
    S_ChangeMusic("d_evil", true);
}


//
// F_CastTicker
//
// -KM- 1998/10/29 Use sfx_t.  Known bug: Chaingun/Spiderdemon's sounds aren't
//                  stopped.
static int shotsfxchannel = -1;
void F_CastTicker (void)
{
    int	st;
    sfx_t* sfx = NULL;
    boolean wantchannel = false;
	
    if (--casttics > 0)
	return;			// not time to change state yet
		
    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {
	// switch from deathstate to next monster
	castnum++;
	castdeath = false;
	castorder = F_GetNextActor();

	if (castorder->seesound)
	    S_StartSound (NULL, castorder->seesound);

	caststate = &states[castorder->seestate];
	castframes = 0;
    }
    else
    {
	// just advance to next state in animation
	st = caststate->nextstate;
	caststate = &states[st];
	castframes++;
        casttics = caststate->tics;
	
        // Yuk, handles sounds
        if (caststate->action.acp1 == P_ActMakeCloseAttemptSound)
           sfx = castorder->closecombat->initsound;
        else if (caststate->action.acp1 == P_ActMakeRangeAttemptSound)
           sfx = castorder->rangeattack->initsound;
        else if (caststate->action.acp1 == P_ActRangeAttack)
        {
           if (castorder->rangeattack->attackstyle == ATK_SHOT)
           {
             sfx = castorder->rangeattack->sound;
             wantchannel = true;
             if (sfx && shotsfxchannel >= 0)
               S_StopChannel(shotsfxchannel);
           } else if (castorder->rangeattack->attackstyle == ATK_SKULLFLY)
             sfx = castorder->rangeattack->initsound;
           else if (castorder->rangeattack->attackstyle == ATK_SPAWNER)
             sfx = castorder->rangeattack->projectile->rangeattack->initsound;
           else if (castorder->rangeattack->attackstyle == ATK_TRACKER)
             sfx = castorder->rangeattack->initsound;
           else
             sfx = castorder->rangeattack->projectile->seesound;
        } else if (caststate->action.acp1 == P_ActMeleeAttack)
           sfx = castorder->closecombat->sound;
        else if (castorder->activesound && !P_Random () && !castdeath)
           sfx = castorder->activesound;
        else if (caststate->action.acp1 == P_ActWalkSoundChase)
           sfx = castorder->walksound;
        else if (caststate->action.acp1 == P_ActComboAttack)
        {
          if (castonmelee)
          {
            sfx = castorder->closecombat->sound;
          } else {
           if (castorder->rangeattack->attackstyle == ATK_SHOT)
           {
             sfx = castorder->rangeattack->sound;
             wantchannel = true;
             if (sfx && shotsfxchannel >= 0)
               S_StopChannel(shotsfxchannel);
           } else if (castorder->rangeattack->attackstyle == ATK_SKULLFLY)
             sfx = castorder->rangeattack->initsound;
           else if (castorder->rangeattack->attackstyle == ATK_SPAWNER)
             sfx = castorder->rangeattack->projectile->rangeattack->initsound;
           else if (castorder->rangeattack->attackstyle == ATK_TRACKER)
             sfx = castorder->rangeattack->initsound;
           else
             sfx = castorder->rangeattack->projectile->seesound;
          }
        }
    }
	
    if (castframes == 24)
    {
	// go into attack frame
	castattacking = true;
	castonmelee ^= 1;
	if (castonmelee)
	    caststate=&states[castorder->meleestate];
	else
	    caststate=&states[castorder->missilestate];
	if (caststate == &states[S_NULL])
	{
            castonmelee ^= 1;
	    if (castonmelee)
		caststate=
		    &states[castorder->meleestate];
	    else
		caststate=
		    &states[castorder->missilestate];
	}
        sfx = castorder->attacksound;
    }
	
    if (castattacking)
    {
	if (castframes == 36
	    ||	caststate == &states[castorder->seestate] )
	{
	    castattacking = false;
	    castframes = 0;
	    caststate = &states[castorder->seestate];
	}
    }
	
    casttics = caststate->tics;
    if (casttics == -1)
	casttics = 15;

    if (sfx)
    {
      if (wantchannel)
        shotsfxchannel = S_StartSound(NULL, sfx);
      else
        S_StartSound(NULL, sfx);
    }
}


//
// F_CastResponder
//

boolean F_CastResponder (event_t* ev)
{
    if (ev->type != ev_keydown)
	return false;
		
    if (castdeath)
	return true;			// already in dying frames
		
    // go into death frame
    castdeath = true;
    caststate = &states[castorder->deathstate];
    casttics = caststate->tics;
    castframes = 0;
    castattacking = false;
    if (castorder->deathsound)
	S_StartSound (NULL, castorder->deathsound);
	
    return true;
}


void F_CastPrint (char* text)
{
    char*	ch;
    int		c;
    int		cx;
    int		w;
    int		width;
    
    // find width
    ch = text;
    width = 0;
	
    while (ch)
    {
	c = *ch++;
	if (!c)
	    break;
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    width += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	width += w;
    }
    
    // draw it
    cx = 160-width/2;
    ch = text;
    while (ch)
    {
	c = *ch++;
	if (!c)
	    break;
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	V_DrawPatchInDirect(cx, 180, 0, hu_font[c]);
	cx+=w;
    }
	
}


//
// F_CastDrawer
//
void F_CastDrawer (void)
{
    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    boolean		flip;
    patch_t*		patch;
    
    // erase the entire screen to a background
    // -KM- 1998/07/21 Clear around the pic too.
    V_ClearPageBackground(0);
    V_DrawPatchInDirect (0,0,0, W_CacheLumpName ("BOSSBACK", PU_CACHE));

    F_CastPrint (castorder->name);
    
    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[ caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];
			
    patch = W_CacheLumpNum (spritelist[lump], PU_CACHE);
    if (flip)
	V_DrawPatchInDirectFlipped (160,170,0,patch);
    else
	V_DrawPatchInDirect (160,170,0,patch);
}


//
// F_DrawPatchCol
//
// -KM- 1998/07/31 Made our bunny friend take up more of the screen.
void
F_DrawPatchCol
( int		x,
  patch_t*	patch,
  int		col,
  fixed_t       fracstep)
  {
  column_t*	column;
  byte*	source;
  int		count;
  fixed_t       frac;

  column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

  if (BPP==1)
    {
    byte*	dest;
    byte*	desttop;

    desttop = screens[0]+x+(SCREENWIDTH-SCALEDWIDTH)/2+
                            SCREENWIDTH*((SCREENHEIGHT-SCALEDHEIGHT)/2);
    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
      source = (byte *)column + 3;
      dest = desttop + column->topdelta*SCREENWIDTH;
      count = (column->length << FRACBITS) / fracstep;
      frac = 0;
      while (count--)
      {
         *dest = source[frac >> FRACBITS];
         dest += SCREENWIDTH;
         frac += fracstep;
      }
      column = (column_t *)(  (byte *)column + column->length + 4 );
      }
    }
  else
    {
    short*	dest;
    short*	desttop;
    desttop = (short *)(screens[0]+2*(x+(SCREENWIDTH-SCALEDWIDTH)/2+
                     SCREENWIDTH*((SCREENHEIGHT-SCALEDHEIGHT)/2)));
    // step through the posts in a column
    while (column->topdelta != 0xff )
      {
      source = (byte *)column + 3;
      dest = desttop + column->topdelta*SCREENWIDTH;
      count = (column->length << FRACBITS) / fracstep;
      frac = 0;
      while (count--)
        {
         *dest = palette_color[source[frac >> FRACBITS]];
         dest += SCREENWIDTH;
         frac += fracstep;
        }
      column = (column_t *)(  (byte *)column + column->length + 4 );
      }
    }
  }


//
// F_BunnyScroll
//
// -KM- 1998/07/31 Made our bunny friend take up more screen space.
// -KM- 1998/12/16 Removed fading routine.
void F_BunnyScroll (void)
{
    int		scrolled;
    int		x;
    patch_t*	p1;
    patch_t*	p2;
    char	name[10];
    int		stage;
    static int	laststage;
    const fixed_t     yscale = (200 << FRACBITS) / SCALEDHEIGHT;
    const fixed_t     xscale = (320 << FRACBITS) / SCALEDWIDTH;
    fixed_t     frac = 0;
		
    p1 = W_CacheLumpName ("PFUB2", PU_LEVEL);
    p2 = W_CacheLumpName ("PFUB1", PU_LEVEL);

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);
	
    scrolled = 320 - (finalecount-230)/2;
    if (scrolled > 320)
	scrolled = 320;
    if (scrolled < 0)
	scrolled = 0;
   // 23-6-1998 KM Changed the background colour to black not real dark grey
   // -KM- 1998/07/21  Replaced SCREENWIDTH*BPP with SCREENDEPTH (two places)
   V_ClearPageBackground(0);
   for ( x=0 ; x<320 ; x++)
    {
        do
        {
  	  if (x+scrolled < 320)
	    F_DrawPatchCol ( frac / xscale, p1, x+scrolled, yscale);
	  else
	    F_DrawPatchCol ( frac / xscale, p2, x+scrolled - 320, yscale);
          frac += xscale;
        }
        while ((frac >> FRACBITS) <= x);
    }
	
    if (finalecount < 1130)
	return;
    if (finalecount < 1180)
    {
	V_DrawPatchInDirect ((320-13*8)/2,
		     (200-8*8)/2,0, W_CacheLumpName ("END0",PU_CACHE));
	laststage = 0;
	return;
    }
	
    stage = (finalecount-1180) / 5;
    if (stage > 6)
	stage = 6;
    if (stage > laststage)
    {
	S_StartSound (NULL, sfx_pistol);
	laststage = stage;
    }
	
    sprintf (name,"END%i",stage);
    V_DrawPatchInDirect ((320-13*8)/2, (200-8*8)/2,0, W_CacheLumpName (name,PU_CACHE));
}


//
// F_Drawer
//
void F_Drawer (void)
{
    switch (finalestage)
    {
      case f_text:
           F_TextWrite();
           break;
      case f_pic:
           // -KM- 1998/07/21 Clear around the pic too
           V_ClearPageBackground(0);
           V_DrawPatchInDirect (0,0,0,
 		 W_CacheLumpName(&finale->pics[picnum*9],PU_CACHE));
           break;
      case f_bunny:
           F_BunnyScroll();
           break;
      case f_cast:
           F_CastDrawer();
           break;
      case f_end:
           break;
    }
}


