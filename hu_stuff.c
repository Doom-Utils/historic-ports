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
// DESCRIPTION:  Heads-up displays
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: hu_stuff.c,v 1.4 1997/02/03 16:47:52 b1 Exp $";

#include <ctype.h>

#include "doomdef.h"
#include "v_res.h"

#include "z_zone.h"

#include "m_swap.h"

#include "hu_stuff.h"
#include "hu_lib.h"
#include "w_wad.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "lu_sound.h"
#include "i_system.h"
#include "m_misc.h"

#include "rad_trig.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLE	(mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2	(mapnames2[gamemap-1])
#define HU_TITLEP	(mapnamesp[gamemap-1])
#define HU_TITLET	(mapnamest[gamemap-1])
#define HU_TITLEHEIGHT	1
#define HU_TITLEX	0
#define HU_TITLEY	(SCREENHEIGHT-(200-(167 - SHORT(hu_font[0]->height))))
#define HU_INPUTTOGGLE	key_talk
#define HU_INPUTX	HU_MSGX
#define HU_INPUTY	(HU_MSGY + HU_MSGHEIGHT*(SHORT(hu_font[0]->height) +1))
#define HU_INPUTWIDTH	64
#define HU_INPUTHEIGHT	1

#define HU_CROSSHAIRCOLOR (256-5*16)
#define SBARHEIGHT 32

char*	chat_macros[10];

char*	player_names[MAXPLAYERS];


char			chat_char; // remove later.
static player_t*	plr;
patch_t*		hu_font[HU_FONTSIZE];
static hu_textline_t	w_title;
boolean			chat_on;
static hu_itext_t	w_chat;
static boolean		always_off = false;
static char		chat_dest[MAXPLAYERS];
static hu_itext_t w_inputbuffer[MAXPLAYERS];

static boolean		message_on;
boolean			message_dontfuckwithme;
static boolean		message_nottobefuckedwith;

static hu_stext_t	w_message;
static int		message_counter;

extern int		showMessages;
extern boolean		automapactive;

static boolean		headsupactive = false;


static hu_textline_t	textlinefps;
static hu_textline_t	textlinepos;
static hu_textline_t	textlinestats;


//
// Builtin map names.
// The actual names can be found in DStrings.h.
//

char*	mapnames[40];
char*	mapnames2[40];
char*	mapnamesp[40];
char*   mapnamest[40];

const char*	shiftxform;

const char french_shiftxform[] =
{
    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31,
    ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '?', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    '0', // shift-0
    '1', // shift-1
    '2', // shift-2
    '3', // shift-3
    '4', // shift-4
    '5', // shift-5
    '6', // shift-6
    '7', // shift-7
    '8', // shift-8
    '9', // shift-9
    '/',
    '.', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127

};

const char english_shiftxform[] =
{

    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31,
    ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};

char frenchKeyMap[128]=
{
    0,
    1,2,3,4,5,6,7,8,9,10,
    11,12,13,14,15,16,17,18,19,20,
    21,22,23,24,25,26,27,28,29,30,
    31,
    ' ','!','"','#','$','%','&','%','(',')','*','+',';','-',':','!',
    '0','1','2','3','4','5','6','7','8','9',':','M','<','=','>','?',
    '@','Q','B','C','D','E','F','G','H','I','J','K','L',',','N','O',
    'P','A','R','S','T','U','V','Z','X','Y','W','^','\\','$','^','_',
    '@','Q','B','C','D','E','F','G','H','I','J','K','L',',','N','O',
    'P','A','R','S','T','U','V','Z','X','Y','W','^','\\','$','^',127
};

char ForeignTranslation(unsigned char ch)
{
    return ch < 128 ? frenchKeyMap[ch] : ch;
}

void HU_Init(void)
{

    int		i;
    int		j;
    char	buffer[9];

    if (language==french)
	shiftxform = french_shiftxform;
    else
	shiftxform = english_shiftxform;

    // load the heads-up font
    j = HU_FONTSTART;
    for (i=0;i<HU_FONTSIZE;i++)
    {
	sprintf(buffer, "STCFN%.3d", j++);
	hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

}

void HU_Stop(void)
{
    headsupactive = false;
}

void HU_Start(void)
{

    int		i;
    char*	s = HU_TITLE;

    if (headsupactive)
	HU_Stop();

    plr = &players[consoleplayer];
    message_on = false;
    message_dontfuckwithme = false;
    message_nottobefuckedwith = false;
    chat_on = false;

    // create the message widget
    HUlib_initSText(&w_message,
		    HU_MSGX, HU_MSGY, HU_MSGHEIGHT,
		    hu_font,
		    HU_FONTSTART, &message_on);

    // create the map title widget
    HUlib_initTextLine(&w_title,
		       HU_TITLEX, HU_TITLEY,
		       hu_font,
		       HU_FONTSTART);


   //create stuff for showstats cheat
    HUlib_initTextLine(&textlinefps,
		       0, 1*(1+hu_font[0]->height),
		       hu_font,
		       HU_FONTSTART);
    HUlib_initTextLine(&textlinepos,
		       0, 3*(1+hu_font[0]->height),
		       hu_font,
		       HU_FONTSTART);
    HUlib_initTextLine(&textlinestats,
		       0, 2*(1+hu_font[0]->height),
		       hu_font,
		       HU_FONTSTART);
    
//Raven: Another patch to work with plutonia and tnt properly
//Raven: Helps display the map names properly in map view mode...
    switch(gamemission)
         {
            case pack_plut:
               s = HU_TITLEP;
               break;
            case pack_tnt:
               s = HU_TITLET;
               break;
            case doom2:
               s = HU_TITLE2;
               break;
            default:
            }
//Raven: End of patch
    
    while (*s)
	HUlib_addCharToTextLine(&w_title, *(s++));

    // create the chat widget
    HUlib_initIText(&w_chat,
		    HU_INPUTX, HU_INPUTY,
		    hu_font,
		    HU_FONTSTART, &chat_on);

    // create the inputbuffer widgets
    for (i=0 ; i<MAXPLAYERS ; i++)
	HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

    headsupactive = true;

}

void HU_PutPixel(int x,int y,int color)
  {
  if (BPP==1)
    screens[0][y*SCREENWIDTH+x]=color;
  else
    ((short *)(screens[0]))[y*SCREENWIDTH+x]=palette_color[color];
  }


int chcount=0, chdir=1, chtimer=0; 
extern int setblocks;              

void HU_Drawer(void)
{

    int sbarheight=SBARHEIGHT; 

    TIP_DisplayTips(((SCREENHEIGHT-sbarheight)/2)-2);
	
    HUlib_drawSText(&w_message);
    HUlib_drawIText(&w_chat);
    if (automapactive)
	HUlib_drawTextLine(&w_title, false);

    if (setblocks==11 && !automapactive)
       sbarheight=0;           //-JC- Make sure crosshair works full scr.

    // -jc- Pulsating
    if (chtimer++%5) {
        if (chcount==14)
           chdir=-1;
        else if (chcount==0)
                chdir=1;
        chcount+=chdir;
    }

    //do crosshairs
    if (crosshair==1)
      {
      HU_PutPixel(SCREENWIDTH/2-3,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2-2,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2+2,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2+3,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2-3,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2-2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2+2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2+3,HU_CROSSHAIRCOLOR+chcount);
      }
    else if (crosshair==2)
      {
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      }
    else if (crosshair==3)
      {
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2+1,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2+2,(SCREENHEIGHT-sbarheight)/2,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2+1,HU_CROSSHAIRCOLOR+chcount);
      HU_PutPixel(SCREENWIDTH/2,(SCREENHEIGHT-sbarheight)/2+2,HU_CROSSHAIRCOLOR+chcount);
      }

    //now, draw stats
    if (showstats)
      {
      char textbuf[100];
      char *s;      
      static int timelastframe=0,fps=0,numframes=0;
      int currtime,timediff;

      numframes++;
      currtime=I_GetTime();
      timediff=currtime-timelastframe;
      if (timediff>11)  //update every third of a second
        {
        if (timediff<350)
          fps=numframes*350/timediff;
        else
          fps=0;
        timelastframe=currtime;      
        numframes=0;
        }

      HUlib_clearTextLine(&textlinefps);
      sprintf(textbuf,"fps:%d.%d   time:%d:%d%d",
              fps/10,fps%10,(leveltime/TICRATE)/60,
              ((leveltime/TICRATE)%60)/10,
              ((leveltime/TICRATE)%60)%10);
      s=textbuf; while (*s) HUlib_addCharToTextLine(&textlinefps,*(s++));
      HUlib_drawTextLine(&textlinefps,0);

      if (!netgame)
        {
        HUlib_clearTextLine(&textlinepos);
        HUlib_clearTextLine(&textlinestats);


        // Convert angle & x,y co-ordinates so they are easier to read.
        sprintf(textbuf,"ang=0x%x; x,y=( %d, %d )",
              players[consoleplayer].mo->angle>>16,
              players[consoleplayer].mo->x>>16,
              players[consoleplayer].mo->y>>16);
        s=textbuf; 	 while (*s)HUlib_addCharToTextLine(&textlinepos,*(s++));
        sprintf(textbuf,"Kills:%d/%d   Items:%d/%d   Secrets:%d/%d",
              players[consoleplayer].killcount,totalkills,
              players[consoleplayer].itemcount,totalitems,
              players[consoleplayer].secretcount,totalsecret);
        s=textbuf; while (*s)HUlib_addCharToTextLine(&textlinestats,*(s++));
        HUlib_drawTextLine(&textlinepos,0);
        HUlib_drawTextLine(&textlinestats,0);
        }
      }


}

void HU_Erase(void)
{

    HUlib_eraseSText(&w_message);
    HUlib_eraseIText(&w_chat);
    HUlib_eraseTextLine(&w_title);

}

void HU_Ticker(void)
{

    int i, rc;
    char c;

    // tick down message counter if message is up
    if (message_counter && !--message_counter)
    {
	message_on = false;
	message_nottobefuckedwith = false;
    }

    if (showMessages || message_dontfuckwithme)
    {

	// display message if necessary
	if ((plr->message && !message_nottobefuckedwith)
	    || (plr->message && message_dontfuckwithme))
	{
	    HUlib_addMessageToSText(&w_message, 0, plr->message);
	    plr->message = 0;
	    message_on = true;
	    message_counter = HU_MSGTIMEOUT;
	    message_nottobefuckedwith = message_dontfuckwithme;
	    message_dontfuckwithme = 0;
	}

    } // else message_on = false;

    // check for incoming chat characters
    if (netgame)
    {
	for (i=0 ; i<MAXPLAYERS; i++)
	{
	    if (!playeringame[i])
		continue;
	    if (i != consoleplayer
		&& (c = players[i].cmd.chatchar))
	    {
		if (c <= HU_BROADCAST)
		    chat_dest[i] = c;
		else
		{
		    if (c >= 'a' && c <= 'z')
			c = (char) shiftxform[(unsigned char) c];
		    rc = HUlib_keyInIText(&w_inputbuffer[i], c);
		    if (rc && c == KEYD_ENTER)
		    {
			if (w_inputbuffer[i].l.len
			    && (chat_dest[i] == consoleplayer+1
				|| chat_dest[i] == HU_BROADCAST))
			{
			    HUlib_addMessageToSText(&w_message,
						    player_names[i],
						    w_inputbuffer[i].l.l);
			    
			    message_nottobefuckedwith = true;
			    message_on = true;
			    message_counter = HU_MSGTIMEOUT;
                            if (W_CheckNumForName("DSRADIO") < 0)
                              S_StartSound(NULL, sfx_tink);
                            else
  			      S_StartSound(NULL, sfx_radio);
			}
			HUlib_resetIText(&w_inputbuffer[i]);
		    }
		}
		players[i].cmd.chatchar = 0;
	    }
	}
    }

}

#define QUEUESIZE		128

static char	chatchars[QUEUESIZE];
static int	head = 0;
static int	tail = 0;


void HU_queueChatChar(char c)
{
    if (((head + 1) & (QUEUESIZE-1)) == tail)
    {
	plr->message = HUSTR_MSGU;
    }
    else
    {
	chatchars[head] = c;
	head = (head + 1) & (QUEUESIZE-1);
    }
}

char HU_dequeueChatChar(void)
{
    char c;

    if (head != tail)
    {
	c = chatchars[tail];
	tail = (tail + 1) & (QUEUESIZE-1);
    }
    else
    {
	c = 0;
    }

    return c;
}

    char		destination_keys[MAXPLAYERS];/* =
    {
	HUSTR_KEYGREEN,
	HUSTR_KEYINDIGO,
	HUSTR_KEYBROWN,
	HUSTR_KEYRED
    };*/

boolean HU_Responder(event_t *ev)
{

    static char		lastmessage[HU_MAXLINELENGTH+1];
    char*		macromessage;
    boolean		eatkey = false;
    static boolean	shiftdown = false;
    static boolean	altdown = false;
    unsigned char 	c;
    int			i;
    int			numplayers;
    
    
    static int		num_nobrainers = 0;

    numplayers = 0;
    for (i=0 ; i<MAXPLAYERS ; i++)
	numplayers += playeringame[i];

    if (ev->data1 == KEYD_RSHIFT)
    {
	shiftdown = ev->type == ev_keydown;
	return false;
    }
    else if (ev->data1 == KEYD_RALT || ev->data1 == KEYD_LALT)
    {
	altdown = ev->type == ev_keydown;
	return false;
    }

    if (ev->type != ev_keydown)
	return false;

    if (!chat_on)
    {
	if (ev->data1 == HU_MSGREFRESH)
	{
	    message_on = true;
	    message_counter = HU_MSGTIMEOUT;
	    eatkey = true;
	}
	else if (netgame && ((ev->data1==(HU_INPUTTOGGLE>>16))||(ev->data1==(HU_INPUTTOGGLE&0xffff))))
	{
	    eatkey = chat_on = true;
	    HUlib_resetIText(&w_chat);
	    HU_queueChatChar(HU_BROADCAST);
	}
	else if (netgame && numplayers > 2)
	{
	    for (i=0; i<MAXPLAYERS ; i++)
	    {
		if (ev->data1 == destination_keys[i])
		{
		    if (playeringame[i] && i!=consoleplayer)
		    {
			eatkey = chat_on = true;
			HUlib_resetIText(&w_chat);
			HU_queueChatChar(i+1);
			break;
		    }
		    else if (i == consoleplayer)
		    {
			num_nobrainers++;
			if (num_nobrainers < 3)
			    plr->message = HUSTR_TALKTOSELF1;
			else if (num_nobrainers < 6)
			    plr->message = HUSTR_TALKTOSELF2;
			else if (num_nobrainers < 9)
			    plr->message = HUSTR_TALKTOSELF3;
			else if (num_nobrainers < 32)
			    plr->message = HUSTR_TALKTOSELF4;
			else
			    plr->message = HUSTR_TALKTOSELF5;
		    }
		}
	    }
	}
    }
    else
    {
	c = ev->data1;
	// send a macro
	if (altdown)
	{
	    c = c - '0';
	    if (c > 9)
		return false;
	    // fprintf(stderr, "got here\n");
	    macromessage = chat_macros[c];
	    
	    // kill last message with a '\n'
	    HU_queueChatChar(KEYD_ENTER); // DEBUG!!!
	    
	    // send the macro message
	    while (*macromessage)
		HU_queueChatChar(*macromessage++);
	    HU_queueChatChar(KEYD_ENTER);
	    
	    // leave chat mode and notify that it was sent
	    chat_on = false;
	    strcpy(lastmessage, chat_macros[c]);
	    plr->message = lastmessage;
	    eatkey = true;
	}
	else
	{
	    if (language==french)
		c = ForeignTranslation(c);
	    if (shiftdown || (c >= 'a' && c <= 'z'))
		c = shiftxform[c];
	    eatkey = HUlib_keyInIText(&w_chat, c);
	    if (eatkey)
	    {
		// static unsigned char buf[20]; // DEBUG
		HU_queueChatChar(c);
		
		// sprintf(buf, "KEY: %d => %d", ev->data1, c);
		//      plr->message = buf;
	    }
	    if (c == KEYD_ENTER)
	    {
		chat_on = false;
		if (w_chat.l.len)
		{
		    strcpy(lastmessage, w_chat.l.l);
		    plr->message = lastmessage;
		}
	    }
	    else if (c == KEYD_ESCAPE)
		chat_on = false;
	}
    }

    return eatkey;

}
