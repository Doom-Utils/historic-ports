//
// DOSDoom Intermission Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -KM- 1998/12/16 Nuked half of this for DDF. DOOM 1 works now!
//
#include <stdio.h>

#include "d_debug.h"

#include "dm_state.h"
#include "g_game.h"
#include "i_system.h"
#include "m_random.h"
#include "m_menu.h"
#include "m_swap.h"
#include "lu_sound.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_res.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "z_zone.h"

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//

// GLOBAL LOCATIONS
#define WI_TITLEY		2
#define WI_SPACINGY    		33

// SINGPLE-PLAYER STUFF
#define SP_STATSX		50
#define SP_STATSY		50

#define SP_TIMEX		16
#define SP_TIMEY		(200-32)


// NET GAME STUFF
#define NG_STATSY		50
#define NG_STATSX		(32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX    		64


// DEATHMATCH STUFF
#define DM_MATRIXX		42
#define DM_MATRIXY		68
#define DM_SPACINGX		40
#define DM_TOTALSX		269
#define DM_KILLERSX		10
#define DM_KILLERSY		100
#define DM_VICTIMSX    		5
#define DM_VICTIMSY		50

//
// GENERAL DATA
//

//
// Locally used stuff.
//
#define FB 0


// States for single-player
#define SP_KILLS		0
#define SP_ITEMS		2
#define SP_SECRET		4
#define SP_FRAGS		6 
#define SP_TIME			8 
#define SP_PAR			ST_TIME

#define SP_PAUSE		1

// in seconds
#define SHOWNEXTLOCDELAY	4
#define SHOWLASTLOCDELAY	SHOWNEXTLOCDELAY

// used to accelerate or skip a stage
static boolean		acceleratestage;

// wbs->pnum
static int		me;

 // specifies current state
static stateenum_t	state;

// contains information passed into intermission
static wbstartstruct_t*	wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int 		cnt;  

// used for timing of background animation
static int 		bcnt;

// signals to refresh everything for one frame
static int 		firstrefresh; 

static int		cnt_kills[10];
static int		cnt_items[10];
static int		cnt_secret[10];
static int		cnt_time;
static int		cnt_par;
static int		cnt_pause;

// GRAPHICS

// background .
static patch_t*		bg;

// You Are Here graphic
static patch_t*		yah[2] = {NULL, NULL};

// splat
static patch_t*		splat[2] = {NULL, NULL};

// %, : graphics
static patch_t*		percent;
static patch_t*		colon;

// 0-9 graphic
static patch_t*		num[10];

// minus sign
static patch_t*		wiminus;

// "Finished!" graphics
static patch_t*		finished;

// "Entering" graphic
static patch_t*		entering; 

// "secret"
static patch_t*		sp_secret;

 // "Kills", "Scrt", "Items", "Frags"
static patch_t*		kills;
static patch_t*		secret;
static patch_t*		items;
static patch_t*		frags;

// Time sucks.
static patch_t*		time;
static patch_t*		par;
static patch_t*		sucks;

// "killers", "victims"
static patch_t*		killers;
static patch_t*		victims; 

// "Total", your face, your dead face
static patch_t*		total;
static patch_t*		star;
static patch_t*		bstar;

// "red P[1..MAXPLAYERS]"
//static patch_t*		p[MAXPLAYERS];

// "gray P[1..MAXPLAYERS]"
//static patch_t*		bp[MAXPLAYERS];

 // Name graphics of each level (centered)
static patch_t*	        lnames[2];

// -KM- 1998/12/17 Needs access from savegame code
wi_map_t        worldmap = { NULL };
//
// CODE
//

// slam background
// UNUSED static unsigned char *background=0;


void WI_slamBackground(void)
{
    // -ES- 1998/08/24 Replaced SCREENWIDTH*BPP with SCREENDEPTH
    memcpy(screens[0], screens[1], SCREENDEPTH * SCREENHEIGHT);
    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}

// Called everytime the map changes.
// -KM- 1998/12/17 Beefed up:  if worldmap.name == NULL we haven't
//   inited yet.  if map == NULL reload the intermission anyway
//   (we are starting a new game.)
void WI_MapInit(wi_map_t* map)
{
  if (!map)
    worldmap.name = NULL;
  else
  {
    if (!worldmap.name || strcmp(map->name, worldmap.name))
    {
      memcpy(&worldmap, map, sizeof(wi_map_t));
      memset(worldmap.mapdone, false, sizeof(boolean) * worldmap.nummaps);
    }
  }
}

// Draws "<Levelname> Finished!"
void WI_drawLF(void)
{
    int y = WI_TITLEY;

    // draw <LevelName> 
    V_DrawPatchInDirect((320 - SHORT(lnames[0]->width))/2, y, FB, lnames[0]);

    // draw "Finished!"
    y += (5*SHORT(lnames[0]->height))/4;
    
    V_DrawPatchInDirect((320 - SHORT(finished->width))/2, y, FB, finished);
}



// Draws "Entering <LevelName>"
void WI_drawEL(void)
{
    int y = WI_TITLEY;

    // -KM- 1998/11/25 If there is no level to enter, don't draw it.
    //  (Stop Map30 from crashing)
    if (lnames[1])
    {
      // draw "Entering"
      V_DrawPatchInDirect((320 - SHORT(entering->width))/2, y, FB, entering);
  
      // draw level
      y += (5*SHORT(lnames[1]->height))/4;
  
      V_DrawPatchInDirect((320 - SHORT(lnames[1]->width))/2, y, FB, lnames[1]);
    }
}

void WI_drawOnLnode (int n, patch_t* p[2] )
{
    int		i;
    int		left;
    int		top;
    int		right;
    int		bottom;
    boolean	fits = false;
    mappos_t*   mappos = NULL;

    mappos = &worldmap.mappos[n];

    i = 0;
    do
    {
	left = mappos->pos.x - SHORT(p[i]->leftoffset);
	top = mappos->pos.y - SHORT(p[i]->topoffset);
	right = left + SHORT(p[i]->width);
	bottom = top + SHORT(p[i]->height);

	if (left >= 0
	    && right < 320
	    && top >= 0
	    && bottom < 200)
	{
	    fits = true;
	}
	else
	{
	    i++;
	}
    } while (!fits && i!=2);

    if (fits && i<2)
    {
	V_DrawPatchInDirect(mappos->pos.x, mappos->pos.y,
		    FB, p[i]);
    }
    else
    {
	// DEBUG
	I_Printf("Could not place patch on level '%s'", mappos[n].name);
    }
}


//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

int
WI_drawNum
( int		x,
  int		y,
  int		n,
  int		digits )
{

    int		fontwidth = SHORT(num[0]->width);
    int		neg;
    int		temp;

    if (digits < 0)
    {
	if (!n)
	{
	    // make variable-length zeros 1 digit long
	    digits = 1;
	}
	else
	{
	    // figure out # of digits in #
	    digits = 0;
	    temp = n;

	    while (temp)
	    {
		temp /= 10;
		digits++;
	    }
	}
    }

    neg = n < 0;
    if (neg)
	n = -n;

    // if non-number, do not draw it
    if (n == 1994)
	return 0;

    // draw the new number
    while (digits--)
    {
	x -= fontwidth;
	V_DrawPatchInDirect(x, y, FB, num[ n % 10 ]);
	n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
	V_DrawPatchInDirect(x-=8, y, FB, wiminus);

    return x;

}

void
WI_drawPercent
( int		x,
  int		y,
  int		p )
{
    if (p < 0)
	return;

    V_DrawPatchInDirect(x, y, FB, percent);
    WI_drawNum(x, y, p, -1);
}



//
// Display level completion time and par,
//  or "sucks" message if overflow.
//
void
WI_drawTime
( int		x,
  int		y,
  int		t )
{

    int		div;
    int		n;

    if (t<0)
	return;

    if (t <= 61*59)
    {
	div = 1;

	do
	{
	    n = (t / div) % 60;
	    x = WI_drawNum(x, y, n, 2) - SHORT(colon->width);
	    div *= 60;

	    // draw
	    if (div==60 || t / div)
		V_DrawPatchInDirect(x, y, FB, colon);
	    
	} while (t / div);
    }
    else
    {
	// "sucks"
	V_DrawPatchInDirect(x - SHORT(sucks->width), y, FB, sucks);
    }
}


void WI_End(void)
{
    void WI_unloadData(void);
    WI_unloadData();
}

void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = false;
    cnt = 10;
}

void WI_updateNoState(void)
{
    if (!--cnt)
    {
	WI_End();
	G_WorldDone();
    }

}

static boolean		snl_pointeron = false;


void WI_initShowNextLoc(void)
{
    int i;
    state = ShowNextLoc;
    acceleratestage = false;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    for (i = 0; i < worldmap.nummaps; i++)
       if (!strcmp(worldmap.mappos[i].name, wbs->last->name))
         worldmap.mapdone[i] = true;
}

void WI_updateShowNextLoc(void)
{
    if (!--cnt || acceleratestage)
	WI_initNoState();
    else
	snl_pointeron = (cnt & 31) < 20;
}

void WI_drawShowNextLoc(void)
{
    int		i;

    for (i = 0; i < worldmap.nummaps; i++)
    {
      if (worldmap.mapdone[i])
        WI_drawOnLnode(i, splat);
      if (wbs->next)
        if (snl_pointeron && !strcmp(wbs->next->name, worldmap.mappos[i].name))
          WI_drawOnLnode(i, yah);
    }

    WI_drawEL();
}

void WI_drawNoState(void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}


static int		dm_state;
static int		dm_frags[10];
static int		dm_totals[10];
static int              dm_rank[10];

static inline int WI_DeathmatchScore(int pl)
{
  if (pl >= 0)
  {
    return plrs[pl].totalfrags*2 + plrs[pl].frags;
  }
  return INT_MIN;
}

void WI_initDeathmatchStats(void)
{

    int		i;
    int		j;
    boolean     done = false;
    int         rank[maxplayers];

    state = StatCount;
    acceleratestage = false;
    dm_state = 1;

    cnt_pause = TICRATE;

    for (i=0; i<maxplayers; i++)
       rank[i] = plrs[i].in?i:-1;
    for (i=0; i < 10; i++)
       dm_frags[i] = dm_totals[i] = 0;

    while (!done)
    {
      for (i = 0; i < maxplayers-1; i++)
      {
        if (WI_DeathmatchScore(rank[i]) < WI_DeathmatchScore(rank[i+1]))
        {
          j = rank[i];
          rank[i] = rank[i+1];
          rank[i+1] = j;
        }
      }
      done = true;
      for (i = 0; i < maxplayers-1; i++)
      {
        if (WI_DeathmatchScore(rank[i]) < WI_DeathmatchScore(rank[i+1]))
        {
          done = false;
          break;
        }
      }
    }
    for (i = 0; i < (10>maxplayers?maxplayers:10); i++)
       dm_rank[i] = rank[i];
    for (; i < 10; i++)
       dm_rank[i] = -1;
}



void WI_updateDeathmatchStats(void)
{
    int		i;
    int         p;
    
    boolean	stillticking;

    if (acceleratestage && dm_state != 4)
    {
	acceleratestage = false;

	for (i=0 ; i<10 ; i++)
	{
          p = dm_rank[i];
          if (p >= 0)
          {
	    if (playeringame[p])
	    {
              dm_frags[i] = plrs[p].frags;
              dm_totals[i] = plrs[p].totalfrags;
	    }
          }
	}
	

	S_StartSound(0, worldmap.done);
	dm_state = 4;
    }

    switch (dm_state)
    {
       case 2:
	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);
	
	stillticking = false;
        for (i = 0; i < 10; i++)
        {
          p = dm_rank[i];
          if (p >= 0)
          {
            if (playeringame[p])
            {
              if (dm_frags[i] < plrs[p].frags)
              {
                dm_frags[i]++;
                stillticking = true;
              }
              if (dm_totals[i] < plrs[p].totalfrags)
              {
                dm_totals[i]++;
                stillticking = true;
              }
            }
          }
	}
	if (!stillticking)
	{
	    S_StartSound(0, worldmap.done);
	    dm_state++;
	}
        break;
      case 4:
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_slop);

            if (!worldmap.nummaps)
  	      WI_initNoState();
            else
	      WI_initShowNextLoc();
	}
        break;
      default:
	if (!--cnt_pause)
	{
	    dm_state++;
	    cnt_pause = TICRATE;
	}
        break;
    }
}

void WI_drawDeathmatchStats(void)
{
    int		i;
    int		y;
    int         p;

    char        temp[16];

    WI_drawLF();

    M_WriteText(20, 40, "Player");
    M_WriteText(100, 40, "Frags");
    M_WriteText(200, 40, "Total Frags");
    y = 40;

    for (i=0 ; i<10 ; i++)
    {
        p = dm_rank[i];
        if (p >= 0)
        {
              y += 12;
              if (p == me && ((bcnt & 31) < 16))
                continue;
  
              M_WriteTextTrans(20, y, 15, player_names[p]);
              sprintf(temp, "%5d", dm_frags[i]);
              M_WriteTextTrans(100, y, 15, temp);
              sprintf(temp, "%11d", dm_totals[i]);
              M_WriteTextTrans(200, y, 15, temp);
        }
    }
}

static int	cnt_frags[10];
static int      cnt_tfrags[10];
static int	dofrags;
static int	ng_state;

// Calculates value of this player for ranking
static inline int WI_NetgameScore (pl)
{
  if (pl >= 0)
  {
    int kills = plrs[pl].skills*400/wbs->maxkills;
    int items = plrs[pl].sitems*100/wbs->maxitems;
    int secret = plrs[pl].ssecret*200/wbs->maxsecret;
    int frags = (plrs[pl].frags + plrs[pl].totalfrags)*25;

    return kills + items + secret - frags;
  }
  return INT_MIN;
}


void WI_initNetgameStats(void)
{
    int rank[maxplayers];
    int i, j;
    boolean done = false;

    state = StatCount;
    acceleratestage = false;
    ng_state = 1;

    cnt_pause = TICRATE;

    for (i=0; i<(10>maxplayers?maxplayers:10); i++)
       rank[i] = plrs[i].in?i:-1;

    while (!done)
    {
      for (i = 0; i < maxplayers-1; i++)
      {
        if (WI_NetgameScore(rank[i]) < WI_NetgameScore(rank[i+1]))
        {
          j = rank[i];
          rank[i] = rank[i+1];
          rank[i+1] = j;
        }
      }
      done = true;
      for (i = 0; i < maxplayers-1; i++)
      {
        if (WI_NetgameScore(rank[i]) < WI_NetgameScore(rank[i+1]))
        {
          done = false;
          break;
        }
      }
    }
    for (i = 0; i < (10>maxplayers?maxplayers:10); i++)
       dm_rank[i] = rank[i];
    for (; i < 10; i++)
       dm_rank[i] = -1;


    for (i=0 ; i<10 ; i++)
    {
        if (dm_rank[i] < 0)
          continue;
	if (!playeringame[dm_rank[i]])
	    continue;

	cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = cnt_tfrags[i] = 0;

	dofrags += plrs[dm_rank[i]].frags + plrs[dm_rank[i]].totalfrags;
    }
}


void WI_updateNetgameStats(void)
{

    int		i;
    int         p;
    
    boolean	stillticking;

    if (acceleratestage && ng_state != 10)
    {
	acceleratestage = false;

	for (i=0; i<10; i++)
	{
            p = dm_rank[i];
            if (p < 0)
              continue;
	    if (!playeringame[p])
	      continue;

	    cnt_kills[i] = (plrs[p].skills * 100) / wbs->maxkills;
	    cnt_items[i] = (plrs[p].sitems * 100) / wbs->maxitems;
	    cnt_secret[i] = (plrs[p].ssecret * 100) / wbs->maxsecret;

	    if (dofrags)
            {
		cnt_frags[i] = plrs[p].frags;
                cnt_tfrags[i] = plrs[p].totalfrags;
            }
	}
	S_StartSound(0, worldmap.done);
	ng_state = 10;
    }

    switch (ng_state)
    {
      case 2:
	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	stillticking = false;

	for (i=0 ; i<10 ; i++)
	{
            p = dm_rank[i];
            if (p < 0)
                break;
	    if (!playeringame[p])
		continue;


	    cnt_kills[i] += 2;

	    if (cnt_kills[i] >= (plrs[p].skills * 100) / wbs->maxkills)
		cnt_kills[i] = (plrs[p].skills * 100) / wbs->maxkills;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, worldmap.done);
	    ng_state++;
	}
        break;
      case 4:
	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	stillticking = false;

	for (i=0 ; i<10 ; i++)
	{
            p = dm_rank[i];
            if (p < 0)
                break;
	    if (!playeringame[p])
		continue;

	    cnt_items[i] += 2;
	    if (cnt_items[i] >= (plrs[p].sitems * 100) / wbs->maxitems)
		cnt_items[i] = (plrs[p].sitems * 100) / wbs->maxitems;
	    else
		stillticking = true;
	}
	if (!stillticking)
	{
	    S_StartSound(0,worldmap.done);
	    ng_state++;
	}
        break;
      case 6:
	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	stillticking = false;

	for (i=0 ; i<10 ; i++)
	{
            p = dm_rank[i];
            if (p < 0)
              break;
	    if (!playeringame[p])
	      continue;

	    cnt_secret[i] += 2;

	    if (cnt_secret[i] >= (plrs[p].ssecret * 100) / wbs->maxsecret)
		cnt_secret[i] = (plrs[p].ssecret * 100) / wbs->maxsecret;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, worldmap.done);
	    ng_state += 1 + 2*!dofrags;
	}
        break;
    case 8:
	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	stillticking = false;

	for (i=0 ; i<10 ; i++)
	{
            p = dm_rank[i];
            if (p < 0)
              break;

	    if (!playeringame[p])
		continue;

	    cnt_frags[i] ++;
            cnt_tfrags[i] ++;

	    if (cnt_frags[i] >= plrs[p].frags)
		cnt_frags[i] = plrs[p].frags;
	    else if (cnt_tfrags[i] >= plrs[p].totalfrags)
                cnt_tfrags[i] = plrs[p].totalfrags;
            else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_pldeth);
	    ng_state++;
	}
        break;
    case 10:
	if (acceleratestage)
	{
	    S_StartSound(0, worldmap.nextmap);
            if (!worldmap.nummaps)
  	      WI_initNoState();
            else
	      WI_initShowNextLoc();
	}
    default:
	if (!--cnt_pause)
	{
	    ng_state++;
	    cnt_pause = TICRATE;
	}
  }
}



void WI_drawNetgameStats(void)
{
    int		i;
    int		y;
    int         p;
    char        temp[16];

    WI_drawLF();

    M_WriteText(6, 40, "Player");
    M_WriteText(56, 40, "Kills");
    M_WriteText(98, 40, "Items");
    M_WriteText(142, 40, "Secret");
    if (dofrags)
    {
      M_WriteText(190, 40, "Frags");
      M_WriteText(232, 40, "Total Frags");
    }

    y = 40;
    for (i = 0; i < 10; i++)
    {
       p = dm_rank[i];
       if (p < 0)
         break;
       y += 12;
       if (p == me && ((bcnt & 31) < 16))
         continue;
       sprintf(temp, "%s", player_names[p]);
       M_WriteTextTrans(6, y, 15, temp);
       sprintf(temp, "%%%3d", cnt_kills[i]);
       M_WriteTextTrans(64, y, 15, temp);
       sprintf(temp, "%%%3d", cnt_items[i]);
       M_WriteTextTrans(106, y, 15, temp);
       sprintf(temp, "%%%3d", cnt_secret[i]);
       M_WriteTextTrans(158, y, 15, temp);

       if (dofrags)
       {
         sprintf(temp, "%5d", cnt_frags[i]);
         M_WriteTextTrans(190, y, 15, temp);
         sprintf(temp, "%11d", cnt_tfrags[i]);
         M_WriteTextTrans(232, y, 15, temp);
       }
    }
}

typedef enum
{
  sp_paused = 1,
  sp_kills = 2,
  sp_items = 4,
  sp_scrt = 6,
  sp_time = 8,
  sp_end = 10
} sp_state_t;


static sp_state_t	sp_state;

void WI_initStats(void)
{
    state = StatCount;
    acceleratestage = false;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;

    //WI_initAnimatedBack()
}

void WI_updateStats(void)
{

    //WI_updateAnimatedBack();

    if (acceleratestage && sp_state != sp_end)
    {
	acceleratestage = false;
	cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
	cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
	cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
	cnt_time = plrs[me].stime / TICRATE;
	cnt_par = wbs->partime / TICRATE;
	S_StartSound(NULL, worldmap.done);
	sp_state = sp_end;
    }

    if (sp_state == sp_kills)
    {
	cnt_kills[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
	{
	    cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
	    S_StartSound(0, worldmap.done);
	    sp_state++;
	}
    }
    else if (sp_state == sp_items)
    {
	cnt_items[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
	{
	    cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
	    S_StartSound(0, worldmap.done);
	    sp_state++;
	}
    }
    else if (sp_state == sp_scrt)
    {
	cnt_secret[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
	{
	    cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
	    S_StartSound(0, worldmap.done);
	    sp_state++;
	}
    }

    else if (sp_state == sp_time)
    {
	if (!(bcnt&3))
	    S_StartSound(0, worldmap.percent);

	cnt_time += 3;

	if (cnt_time >= plrs[me].stime / TICRATE)
	    cnt_time = plrs[me].stime / TICRATE;

	cnt_par += 3;

	if (cnt_par >= wbs->partime / TICRATE)
	{
	    cnt_par = wbs->partime / TICRATE;

	    if (cnt_time >= plrs[me].stime / TICRATE)
	    {
		S_StartSound(0,worldmap.done);
		sp_state++;
	    }
	}
    }
    else if (sp_state == sp_end)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, worldmap.nextmap);

            if (!worldmap.nummaps)
  	      WI_initNoState();
            else
	      WI_initShowNextLoc();
	}
    }
    else if (sp_state & sp_paused)
    {
	if (!--cnt_pause)
	{
	    sp_state++;
	    cnt_pause = TICRATE;
	}
    }

}

void WI_drawStats(void)
{
    // line height
    int lh;	

    lh = (3*SHORT(num[0]->height))/2;

//    WI_slamBackground();

    // draw animated background
    //WI_drawAnimatedBack();
    
    WI_drawLF();

    V_DrawPatchInDirect(SP_STATSX, SP_STATSY, FB, kills);
    WI_drawPercent(320 - SP_STATSX, SP_STATSY, cnt_kills[0]);

    V_DrawPatchInDirect(SP_STATSX, SP_STATSY+lh, FB, items);
    WI_drawPercent(320 - SP_STATSX, SP_STATSY+lh, cnt_items[0]);

    V_DrawPatchInDirect(SP_STATSX, SP_STATSY+2*lh, FB, sp_secret);
    WI_drawPercent(320 - SP_STATSX, SP_STATSY+2*lh, cnt_secret[0]);

    V_DrawPatchInDirect(SP_TIMEX, SP_TIMEY, FB, time);
    WI_drawTime(320/2 - SP_TIMEX, SP_TIMEY, cnt_time);

    // -KM- 1998/11/25 Removed episode check. Replaced with partime check
    if (wbs->partime)
    {
      V_DrawPatchInDirect(320/2 + SP_TIMEX, SP_TIMEY, FB, par);
      WI_drawTime(320 - SP_TIMEX, SP_TIMEY, cnt_par);
    }
}

void WI_checkForAccelerate(void)
{
    int   i;
    player_t  *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<maxplayers ; i++, player++)
    {
	if (playeringame[i])
	{
	    if (player->cmd.buttons & BT_ATTACK)
	    {
		if (!player->attackdown)
		    acceleratestage = true;
		player->attackdown = true;
	    }
	    else
		player->attackdown = false;
	    if (player->cmd.buttons & BT_USE)
	    {
		if (!player->usedown)
		    acceleratestage = true;
		player->usedown = true;
	    }
	    else
		player->usedown = false;
	}
    }
}



// Updates stuff each tick
void WI_Ticker(void)
{
    int i;
    // counter for general background animation
    bcnt++;  

    if (bcnt == 1)
    {
	// intermission music
	  S_ChangeMusic(worldmap.music, true);
    }

    WI_checkForAccelerate();

    for (i = 0; i < worldmap.numanims; i++)
    {
      if (worldmap.anims[i].count >= 0)
      {
        if (!worldmap.anims[i].count)
        {
          worldmap.anims[i].frameon
            = (worldmap.anims[i].frameon + 1) % worldmap.anims[i].numframes;
          worldmap.anims[i].count
            = worldmap.anims[i].frames[worldmap.anims[i].frameon].tics;
        }
        worldmap.anims[i].count--;
      }
    }

    switch (state)
    {
      case StatCount:
	if (deathmatch) WI_updateDeathmatchStats();
	else if (netgame) WI_updateNetgameStats();
	else WI_updateStats();
	break;
	
      case ShowNextLoc:
	WI_updateShowNextLoc();
	break;
	
      case NoState:
	WI_updateNoState();
	break;
    }

}

void WI_loadData(void)
{
    int		i, j;
    char	name[9];

    // background
    bg = W_CacheLumpName(worldmap.background, PU_CACHE);
    
    // -KM- 1998/07/21 Clear around the edge of the background
    V_ClearPageBackground(1);
    V_DrawPatchInDirect(0, 0, 1, bg);

    lnames[0] = W_CacheLumpName(wbs->last->graphicname, PU_STATIC);

    if (wbs->next)
      lnames[1] = W_CacheLumpName(wbs->next->graphicname, PU_STATIC);
    else
      lnames[1] = NULL;

    //yah[0] = W_CacheLumpName("WIURH0", PU_STATIC); // you are here
    //yah[1] = W_CacheLumpName("WIURH1", PU_STATIC); // you are here (alt.)
    //splat = W_CacheLumpName("WISPLAT", PU_STATIC);
    if (worldmap.yah[0][0])
      yah[0] = W_CacheLumpName(worldmap.yah[0], PU_STATIC);
    if (worldmap.yah[1][0])
      yah[1] = W_CacheLumpName(worldmap.yah[1], PU_STATIC);
    if (worldmap.splatpic[0])
      splat[0] = W_CacheLumpName(worldmap.splatpic, PU_STATIC);

    wiminus = W_CacheLumpName("WIMINUS", PU_STATIC);
    percent = W_CacheLumpName("WIPCNT", PU_STATIC);
    finished = W_CacheLumpName("WIF", PU_STATIC);
    entering = W_CacheLumpName("WIENTER", PU_STATIC);
    kills = W_CacheLumpName("WIOSTK", PU_STATIC);
    secret = W_CacheLumpName("WIOSTS", PU_STATIC);      // "scrt"
    sp_secret = W_CacheLumpName("WISCRT2", PU_STATIC);  // "secret"
    items = W_CacheLumpName("WIOSTI", PU_STATIC);
    frags = W_CacheLumpName("WIFRGS", PU_STATIC);    
    colon = W_CacheLumpName("WICOLON", PU_STATIC); 
    time = W_CacheLumpName("WITIME", PU_STATIC);   
    sucks = W_CacheLumpName("WISUCKS", PU_STATIC);  
    par = W_CacheLumpName("WIPAR", PU_STATIC);   
    killers = W_CacheLumpName("WIKILRS", PU_STATIC); // "killers" (vertical)
    victims = W_CacheLumpName("WIVCTMS", PU_STATIC); // "victims" (horiz)
    total = W_CacheLumpName("WIMSTT", PU_STATIC);
    star = W_CacheLumpName("STFST01", PU_STATIC); // your face
    bstar = W_CacheLumpName("STFDEAD0", PU_STATIC); // dead face
/*
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	sprintf(name, "STPB%d", i);      
	p[i] = W_CacheLumpName(name, PU_STATIC);

	if (i<=3) // "1,2,3,4"
        {
  	  sprintf(name, "WIBP%d", i+1);
	  bp[i] = W_CacheLumpName(name, PU_STATIC);
        }
    }
  */
    for (i=0;i<10;i++)
    {
	// numbers 0-9
	sprintf(name, "WINUM%d", i);     
	num[i] = W_CacheLumpName(name, PU_STATIC);
    }

    for (i = 0; i < worldmap.numanims; i++)
      for (j = 0; j < worldmap.anims[i].numframes; j++)
      {
        Debug_Printf("'%s'\n", worldmap.anims[i].frames[j].pic);
        worldmap.anims[i].frames[j].p = W_CacheLumpName(worldmap.anims[i].frames[j].pic, PU_STATIC);
      }
}

void WI_unloadData(void)
{
    int		i, j;

    for (i = 0; i < worldmap.numanims; i++)
      for (j = 0; j < worldmap.anims[i].numframes; j++)
        Z_ChangeTag(worldmap.anims[i].frames[j].p, PU_CACHE);

    for (i=0 ; i<10 ; i++)
       Z_ChangeTag(num[i], PU_CACHE);
    
    for (i=0 ; i<2 ; i++)
    {
      if (lnames[i])
       Z_ChangeTag(lnames[i], PU_CACHE);
      if (yah[i])
        Z_ChangeTag(yah[i], PU_CACHE);
      if (splat[i])
        Z_ChangeTag(splat[i], PU_CACHE);
      lnames[i] = NULL;
      yah[i] = NULL;
      splat[i] = NULL;
    }
    Z_ChangeTag(wiminus, PU_CACHE);
    Z_ChangeTag(percent, PU_CACHE);
    Z_ChangeTag(colon, PU_CACHE);
    Z_ChangeTag(finished, PU_CACHE);
    Z_ChangeTag(entering, PU_CACHE);
    Z_ChangeTag(kills, PU_CACHE);
    Z_ChangeTag(secret, PU_CACHE);
    Z_ChangeTag(sp_secret, PU_CACHE);
    Z_ChangeTag(items, PU_CACHE);
    Z_ChangeTag(frags, PU_CACHE);
    Z_ChangeTag(time, PU_CACHE);
    Z_ChangeTag(sucks, PU_CACHE);
    Z_ChangeTag(par, PU_CACHE);
    Z_ChangeTag(victims, PU_CACHE);
    Z_ChangeTag(killers, PU_CACHE);
    Z_ChangeTag(total, PU_CACHE);
    Z_ChangeTag(star, PU_CACHE);
    Z_ChangeTag(bstar, PU_CACHE);
/*
    for (i=0 ; i<MAXPLAYERS ; i++)
	Z_ChangeTag(p[i], PU_CACHE);

    for (i=0 ; i<4 ; i++)
	Z_ChangeTag(bp[i], PU_CACHE);*/
}

void WI_Drawer (void)
{
    int i;
    wi_anim_t* a;
    wi_frame_t* f = NULL;

    V_ClearPageBackground(FB);
    V_DrawPatchInDirect(0, 0, FB, bg);

    for (i = 0; i < worldmap.numanims; i++)
    {
       a = &worldmap.anims[i];
       if (a->frameon == -1)
         continue;
       if (a->type == WI_LEVEL)
       {
         if (!wbs->next)
           f = NULL;
         else if (!strcmp(wbs->next->name, a->level))
           f = &a->frames[a->frameon];
         else
           f = NULL;
       }
       else
         f = &a->frames[a->frameon];
       if (f)
         V_DrawPatchInDirect(f->pos.x, f->pos.y, FB, f->p);
    }

    switch (state)
    {
      case StatCount:
	if (deathmatch)
	    WI_drawDeathmatchStats();
	else if (netgame)
	    WI_drawNetgameStats();
	else
	    WI_drawStats();
	break;
	
      case ShowNextLoc:
	WI_drawShowNextLoc();
	break;
	
      case NoState:
	WI_drawNoState();
	break;
    }
}


void WI_InitVariables(wbstartstruct_t* wbstartstruct)
{
    int i;

    wbs = wbstartstruct;

    acceleratestage = false;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
	wbs->maxkills = 1;

    if (!wbs->maxitems)
	wbs->maxitems = 1;

    if (!wbs->maxsecret)
	wbs->maxsecret = 1;

    WI_MapInit(wbs->last->interscr);
    for (i = 0; i < worldmap.numanims; i++)
    {
       worldmap.anims[i].count = 0;
       worldmap.anims[i].frameon = -1;
    }
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{
    WI_InitVariables(wbstartstruct);
    WI_loadData();

    if (deathmatch)
	WI_initDeathmatchStats();
    else if (netgame)
	WI_initNetgameStats();
    else
	WI_initStats();
}
