//
// DOSDoom Heads-Up-Display Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT)
//

#include <ctype.h>

#include "dm_defs.h"
#include "v_res.h"

#include "z_zone.h"

#include "m_swap.h"

#include "hu_stuff.h"
#include "hu_lib.h"
#include "w_wad.h"

#include "s_sound.h"

#include "dm_state.h"

// Data.
#include "dstrings.h"
#include "lu_sound.h"
#include "i_system.h"
#include "i_allegv.h"
#include "m_misc.h"

#include "rad_trig.h"

//
// Locally used constants, shortcuts.
//
// -ACB- 1998/08/09 Removed the HU_TITLE stuff; Use currentmap->description.
//
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

//char*	player_names[MAXPLAYERS];
char**	player_names;


char			chat_char; // remove later.
patch_t*		hu_font[HU_FONTSIZE];
boolean			chat_on;
static player_t*	plr;
static hu_textline_t	w_title;
static hu_itext_t	w_chat;
static boolean		always_off = false;
//static char		chat_dest[MAXPLAYERS];
//static hu_itext_t       w_inputbuffer[MAXPLAYERS];
static char*		chat_dest;
static hu_itext_t*      w_inputbuffer;

boolean			message_dontfuckwithme;
static boolean		message_on;
static boolean		message_nottobefuckedwith;

static hu_stext_t	w_message;
static int		message_counter;

extern int		showMessages;
extern boolean		automapactive;

static boolean		headsupactive = false;

// 23-6-98 KM Added a line showing the current limits in the
// render code.  Note that these are not really limits,
// just show how many items we have enough memory for.  These
// numbers will increase as needed.  vp = visplanes, ds = drawsegs,
// vs = vissprites, cs = clipsegs
static hu_textline_t	textlinefps;
static hu_textline_t	textlinepos;
static hu_textline_t	textlinestats;
static hu_textline_t	textlinelimits;


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
    '{', '|', '}', '~', 127,
// -KM- 1998/10/29 Added support for foreign chars.  Not finished, some
// capital chars need to be mapped to lowercase
 128 , 129 , 130 , 131 , 132 , 133 , 134 , 135 , 136 , 137 , 138 , 139 , 140 ,
 141 , 132 , 143 , 144 , 145 , 146 , 147 , 148 , 149 , 150 , 151 , 152 , 148 ,
 154 , 155 , 156 , 157 , 158 , 159 , 160 , 161 , 162 , 163 , 164 , 165 , 166 ,
 167 , 168 , 169 , 170 , 171 , 172 , 173 , 174 , 175 , 176 , 177 , 178 , 179 ,
 180 , 181 , 182 , 183 , 184 , 185 , 186 , 187 , 188 , 189 , 190 , 191 , 192 ,
 193 , 194 , 195 , 196 , 197 , 198 , 199 , 200 , 201 , 202 , 203 , 204 , 205 ,
 206 , 207 , 208 , 209 , 210 , 211 , 212 , 213 , 214 , 215 , 216 , 217 , 218 ,
 219 , 220 , 221 , 222 , 223 , 224 , 225 , 226 , 227 , 228 , 229 , 230 , 231 ,
 232 , 233 , 234 , 235 , 236 , 237 , 238 , 239 , 240 , 241 , 242 , 243 , 244 ,
 245 , 246 , 247 , 248 , 249 , 250 , 251 , 252 , 253 , 254 , 255
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
    '{', '|', '}', '~', 127,
 128 , 129 , 130 , 131 , 132 , 133 , 134 , 135 , 136 , 137 , 138 , 139 , 140 ,
 141 , 132 , 143 , 144 , 145 , 146 , 147 , 148 , 149 , 150 , 151 , 152 , 148 ,
 154 , 155 , 156 , 157 , 158 , 159 , 160 , 161 , 162 , 163 , 164 , 165 , 166 ,
 167 , 168 , 169 , 170 , 171 , 172 , 173 , 174 , 175 , 176 , 177 , 178 , 179 ,
 180 , 181 , 182 , 183 , 184 , 185 , 186 , 187 , 188 , 189 , 190 , 191 , 192 ,
 193 , 194 , 195 , 196 , 197 , 198 , 199 , 200 , 201 , 202 , 203 , 204 , 205 ,
 206 , 207 , 208 , 209 , 210 , 211 , 212 , 213 , 214 , 215 , 216 , 217 , 218 ,
 219 , 220 , 221 , 222 , 223 , 224 , 225 , 226 , 227 , 228 , 229 , 230 , 231 ,
 232 , 233 , 234 , 235 , 236 , 237 , 238 , 239 , 240 , 241 , 242 , 243 , 244 ,
 245 , 246 , 247 , 248 , 249 , 250 , 251 , 252 , 253 , 254 , 255
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
    int         lump;
    char	buffer[9];

    chat_dest = Z_Malloc(maxplayers*sizeof(*chat_dest), PU_STATIC, NULL);
    w_inputbuffer = Z_Malloc(maxplayers*sizeof(*w_inputbuffer), PU_STATIC, NULL);
    shiftxform = english_shiftxform;

    // load the heads-up font
    j = HU_FONTSTART;
    for (i=0;i<HU_FONTSIZE;i++)
    {
        // -KM- 1998/10/29 Chars not found will be replaced by a default.
	sprintf(buffer, "STCFN%.3d", j++);
        lump = W_CheckNumForName(buffer);
        if (lump == -1)
          hu_font[i] = (patch_t *) W_CacheLumpName("STCFN000", PU_STATIC);
        else
	  hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }

}

void HU_Stop(void)
{
    headsupactive = false;
}

// -ACB- 1998/08/09 Used Currentmap to set the map name in string
void HU_Start(void)
{
    int i;
    char* string;

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
   // 23-6-98 KM Limits info added.
    HUlib_initTextLine(&textlinefps,
		       0, 1*(1+hu_font[0]->height),
		       hu_font,
		       HU_FONTSTART);
    HUlib_initTextLine(&textlinestats,
		       0, 2*(1+hu_font[0]->height),
		       hu_font,
		       HU_FONTSTART);
    HUlib_initTextLine(&textlinepos,
		       0, 3*(1+hu_font[0]->height),
		       hu_font,
		       HU_FONTSTART);
    HUlib_initTextLine(&textlinelimits,
 		       0, 4*(1+hu_font[0]->height),
  		       hu_font,
  		       HU_FONTSTART);

    // -ACB- 1998/08/09 Use Currentmap settings
    string = currentmap->description;
    while (*string)
	HUlib_addCharToTextLine(&w_title, *(string++));

    // create the chat widget
    HUlib_initIText(&w_chat,
		    HU_INPUTX, HU_INPUTY,
		    hu_font,
		    HU_FONTSTART, &chat_on);

    // create the inputbuffer widgets
    for (i=0 ; i<maxplayers ; i++)
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

#define TIPSHEIGHT 100

void HU_Drawer(void)
{

    int sbarheight=SBARHEIGHT; 

    // -ACB- 1998/06/14 work out height properly with new scaled patch indirect.
    TIP_DisplayTips(TIPSHEIGHT);
	
    HUlib_drawSText(&w_message);
    HUlib_drawIText(&w_chat);
    if (automapactive)
	HUlib_drawTextLine(&w_title, false);

    if (setblocks==11 && !automapactive)
       sbarheight=0;           //-JC- Make sure crosshair works full scr.

    // -jc- Pulsating
    if (chtimer++%5)
    {
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
    // -ACB- 1998/09/11 Used White Colour Scaling.
    if (showstats)
    {
      char textbuf[100];
      char *s;      
      static int timelastframe=0,fps=0,numframes=0;
      int currtime,timediff;
      extern int maxdrawsegs;
      extern int maxvissprites;
      extern int maxvisplanes;
      extern int maxsolidsegs;

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

      HUlib_clearTextLine(&textlinelimits);
      sprintf(textbuf,"vp:%d vs:%d ds:%d ss:%d",
              maxvisplanes,
              maxvissprites,
              maxdrawsegs,
              maxsolidsegs);
      s=textbuf; while (*s) HUlib_addCharToTextLine(&textlinelimits,*(s++));
      HUlib_drawTextLine(&textlinelimits,0);

      if (!netgame)
      {
        HUlib_clearTextLine(&textlinepos);
        HUlib_clearTextLine(&textlinestats);

        // Convert angle & x,y co-ordinates so they are easier to read.
        // -KM- 1998/11/25 Added z co-ordinate
        sprintf(textbuf,"LookDir=%d; x,y,z=( %d, %d, %d )",
              players[consoleplayer].mo->angle>>FRACBITS,
              players[consoleplayer].mo->x>>FRACBITS,
              players[consoleplayer].mo->y>>FRACBITS,
              players[consoleplayer].mo->z>>FRACBITS);
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
	for (i=0 ; i<maxplayers; i++)
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
	plr->message = DDF_LanguageLookup("UnsentMsg");
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

char*		destination_keys;
/*char		destination_keys[maxplayers+1] =
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
    for (i=0 ; i<maxplayers ; i++)
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
	    for (i=0; i<maxplayers ; i++)
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
			    plr->message = DDF_LanguageLookup("TALKTOSELF1");
			else if (num_nobrainers < 6)
			    plr->message = DDF_LanguageLookup("TALKTOSELF2");
			else if (num_nobrainers < 9)
			    plr->message = DDF_LanguageLookup("TALKTOSELF3");
			else if (num_nobrainers < 32)
			    plr->message = DDF_LanguageLookup("TALKTOSELF4");
			else
			    plr->message = DDF_LanguageLookup("TALKTOSELF5");
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
	    while (*macromessage) HU_queueChatChar(*macromessage++);

	    HU_queueChatChar(KEYD_ENTER);
	    
	    // leave chat mode and notify that it was sent
	    chat_on = false;
	    strcpy(lastmessage, chat_macros[c]);
	    plr->message = lastmessage;
	    eatkey = true;
	}
	else
	{
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
