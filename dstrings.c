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
//	Globally defined strings.
// 
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";


#ifdef __GNUG__
#pragma implementation "dstrings.h"
#endif
#include "dstrings.h"
#include "f_finale.h"
#include "m_misc.h"

char* endmsg[NUM_QUITMESSAGES+1]=
{
  // DOOM1
  NULL,
  "please don't leave, there's more\ndemons to toast!",
  "let's beat it -- this is turning\ninto a bloodbath!",
  "i wouldn't leave if i were you.\ndos is much worse.",
  "you're trying to say you like dos\nbetter than me, right?",
  "don't leave yet -- there's a\ndemon around that corner!",
  "ya know, next time you come in here\ni'm gonna toast ya.",
  "go ahead and leave. see if i care.",   //Raven: This ending comma was left out

  // QuitDOOM II messages
  "you want to quit?\nthen, thou hast lost an eighth!",
  "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!",
  "get outta here and go back\nto your boring programs.",
  "if i were your boss, i'd \n deathmatch ya in a minute!",
  "look, bud. you leave now\nand you forfeit your body count!",
  "just leave. when you come\nback, i'll be waiting with a bat.",
  "you're lucky i don't smack\nyou for thinking about leaving.", //Raven: This comma was also left out..

  // FinalDOOM?
  "fuck you, pussy!\nget the fuck out!",
  "you quit and i'll jizz\nin your cystholes!",
  "if you leave, i'll make\nthe lord drink my jizz.",
  "hey, ron! can we say\n'fuck' in the game?",
  "i'd leave: this is just\nmore monsters and levels.\nwhat a load.",
  "suck it down, asshole!\nyou're a fucking wimp!",
  "don't quit now! we're \nstill spending your money!",

  // Internal debug. Different style, too.
  "THIS IS NO MESSAGE!\nPage intentionally left blank."
};


extern castinfo_t castorder[];
extern char* chat_macros[];
extern char* player_names[];
extern char* mapnames[];
extern char* mapnames2[];
extern char* mapnamesp[];
extern char* mapnamest[];
extern char destination_keys[MAXPLAYERS];
extern char*  gammamsg[5];
extern default_t defaults[];

void applystrings() //call after initstrings and after dehacked
  {
  int i;

  //f_finale.h
  castorder[0].name=CC_ZOMBIE;
  castorder[1].name=CC_SHOTGUN;
  castorder[2].name=CC_HEAVY;
  castorder[3].name=CC_IMP;
  castorder[4].name=CC_DEMON;
  castorder[5].name=CC_LOST;
  castorder[6].name=CC_CACO;
  castorder[7].name=CC_HELL;
  castorder[8].name=CC_BARON;
  castorder[9].name=CC_ARACH;
  castorder[10].name=CC_PAIN;
  castorder[11].name=CC_REVEN;
  castorder[12].name=CC_MANCU;
  castorder[13].name=CC_ARCH;
  castorder[14].name=CC_SPIDER;
  castorder[15].name=CC_CYBER;
  castorder[16].name=CC_HERO;

  //hu_stuff.c
  chat_macros[0]=HUSTR_CHATMACRO0;
  chat_macros[1]=HUSTR_CHATMACRO1;
  chat_macros[2]=HUSTR_CHATMACRO2;
  chat_macros[3]=HUSTR_CHATMACRO3;
  chat_macros[4]=HUSTR_CHATMACRO4;
  chat_macros[5]=HUSTR_CHATMACRO5;
  chat_macros[6]=HUSTR_CHATMACRO6;
  chat_macros[7]=HUSTR_CHATMACRO7;
  chat_macros[8]=HUSTR_CHATMACRO8;
  chat_macros[9]=HUSTR_CHATMACRO9;

  player_names[0]=HUSTR_PLRGREEN;
  player_names[1]=HUSTR_PLRINDIGO;
  player_names[2]=HUSTR_PLRBROWN;
  player_names[3]=HUSTR_PLRRED;
  player_names[4]=HUSTR_PLRGOLD;
  player_names[5]=HUSTR_PLRBLUE;
  player_names[6]=HUSTR_PLRDKBLUE;
  player_names[7]=HUSTR_PLRPINK;

  mapnames[0]=HUSTR_E1M1;
  mapnames[1]=HUSTR_E1M2;
  mapnames[2]=HUSTR_E1M3;
  mapnames[3]=HUSTR_E1M4;
  mapnames[4]=HUSTR_E1M5;
  mapnames[5]=HUSTR_E1M6;
  mapnames[6]=HUSTR_E1M7;
  mapnames[7]=HUSTR_E1M8;
  mapnames[8]=HUSTR_E1M9;
  mapnames[9]=HUSTR_E2M1;
  mapnames[10]=HUSTR_E2M2;
  mapnames[11]=HUSTR_E2M3;
  mapnames[12]=HUSTR_E2M4;
  mapnames[13]=HUSTR_E2M5;
  mapnames[14]=HUSTR_E2M6;
  mapnames[15]=HUSTR_E2M7;
  mapnames[16]=HUSTR_E2M8;
  mapnames[17]=HUSTR_E2M9;
  mapnames[18]=HUSTR_E3M1;
  mapnames[19]=HUSTR_E3M2;
  mapnames[20]=HUSTR_E3M3;
  mapnames[21]=HUSTR_E3M4;
  mapnames[22]=HUSTR_E3M5;
  mapnames[23]=HUSTR_E3M6;
  mapnames[24]=HUSTR_E3M7;
  mapnames[25]=HUSTR_E3M8;
  mapnames[26]=HUSTR_E3M9;
  mapnames[27]=HUSTR_E4M1;
  mapnames[28]=HUSTR_E4M2;
  mapnames[29]=HUSTR_E4M3;
  mapnames[30]=HUSTR_E4M4;
  mapnames[31]=HUSTR_E4M5;
  mapnames[32]=HUSTR_E4M6;
  mapnames[33]=HUSTR_E4M7;
  mapnames[34]=HUSTR_E4M8;
  mapnames[35]=HUSTR_E4M9;

  mapnames2[0]=HUSTR_1;
  mapnames2[1]=HUSTR_2;
  mapnames2[2]=HUSTR_3;
  mapnames2[3]=HUSTR_4;
  mapnames2[4]=HUSTR_5;
  mapnames2[5]=HUSTR_6;
  mapnames2[6]=HUSTR_7;
  mapnames2[7]=HUSTR_8;
  mapnames2[8]=HUSTR_9;
  mapnames2[9]=HUSTR_10;
  mapnames2[10]=HUSTR_11;
  mapnames2[11]=HUSTR_12;
  mapnames2[12]=HUSTR_13;
  mapnames2[13]=HUSTR_14;
  mapnames2[14]=HUSTR_15;
  mapnames2[15]=HUSTR_16;
  mapnames2[16]=HUSTR_17;
  mapnames2[17]=HUSTR_18;
  mapnames2[18]=HUSTR_19;
  mapnames2[19]=HUSTR_20;
  mapnames2[20]=HUSTR_21;
  mapnames2[21]=HUSTR_22;
  mapnames2[22]=HUSTR_23;
  mapnames2[23]=HUSTR_24;
  mapnames2[24]=HUSTR_25;
  mapnames2[25]=HUSTR_26;
  mapnames2[26]=HUSTR_27;
  mapnames2[27]=HUSTR_28;
  mapnames2[28]=HUSTR_29;
  mapnames2[29]=HUSTR_30;
  mapnames2[30]=HUSTR_31;
  mapnames2[31]=HUSTR_32;

  mapnamesp[0]=PHUSTR_1;
  mapnamesp[1]=PHUSTR_2;
  mapnamesp[2]=PHUSTR_3;
  mapnamesp[3]=PHUSTR_4;
  mapnamesp[4]=PHUSTR_5;
  mapnamesp[5]=PHUSTR_6;
  mapnamesp[6]=PHUSTR_7;
  mapnamesp[7]=PHUSTR_8;
  mapnamesp[8]=PHUSTR_9;
  mapnamesp[9]=PHUSTR_10;
  mapnamesp[10]=PHUSTR_11;
  mapnamesp[11]=PHUSTR_12;
  mapnamesp[12]=PHUSTR_13;
  mapnamesp[13]=PHUSTR_14;
  mapnamesp[14]=PHUSTR_15;
  mapnamesp[15]=PHUSTR_16;
  mapnamesp[16]=PHUSTR_17;
  mapnamesp[17]=PHUSTR_18;
  mapnamesp[18]=PHUSTR_19;
  mapnamesp[19]=PHUSTR_20;
  mapnamesp[20]=PHUSTR_21;
  mapnamesp[21]=PHUSTR_22;
  mapnamesp[22]=PHUSTR_23;
  mapnamesp[23]=PHUSTR_24;
  mapnamesp[24]=PHUSTR_25;
  mapnamesp[25]=PHUSTR_26;
  mapnamesp[26]=PHUSTR_27;
  mapnamesp[27]=PHUSTR_28;
  mapnamesp[28]=PHUSTR_29;
  mapnamesp[29]=PHUSTR_30;
  mapnamesp[30]=PHUSTR_31;
  mapnamesp[31]=PHUSTR_32;

  mapnamest[0]=THUSTR_1;
  mapnamest[1]=THUSTR_2;
  mapnamest[2]=THUSTR_3;
  mapnamest[3]=THUSTR_4;
  mapnamest[4]=THUSTR_5;
  mapnamest[5]=THUSTR_6;
  mapnamest[6]=THUSTR_7;
  mapnamest[7]=THUSTR_8;
  mapnamest[8]=THUSTR_9;
  mapnamest[9]=THUSTR_10;
  mapnamest[10]=THUSTR_11;
  mapnamest[11]=THUSTR_12;
  mapnamest[12]=THUSTR_13;
  mapnamest[13]=THUSTR_14;
  mapnamest[14]=THUSTR_15;
  mapnamest[15]=THUSTR_16;
  mapnamest[16]=THUSTR_17;
  mapnamest[17]=THUSTR_18;
  mapnamest[18]=THUSTR_19;
  mapnamest[19]=THUSTR_20;
  mapnamest[20]=THUSTR_21;
  mapnamest[21]=THUSTR_22;
  mapnamest[22]=THUSTR_23;
  mapnamest[23]=THUSTR_24;
  mapnamest[24]=THUSTR_25;
  mapnamest[25]=THUSTR_26;
  mapnamest[26]=THUSTR_27;
  mapnamest[27]=THUSTR_28;
  mapnamest[28]=THUSTR_29;
  mapnamest[29]=THUSTR_30;
  mapnamest[30]=THUSTR_31;
  mapnamest[31]=THUSTR_32;

  destination_keys[0]=HUSTR_KEYGREEN[0];
  destination_keys[1]=HUSTR_KEYINDIGO[0];
  destination_keys[2]=HUSTR_KEYBROWN[0];
  destination_keys[3]=HUSTR_KEYRED[0];

  //m_menu.c
  gammamsg[0]=GAMMALVL0;
  gammamsg[1]=GAMMALVL1;
  gammamsg[2]=GAMMALVL2;
  gammamsg[3]=GAMMALVL3;
  gammamsg[4]=GAMMALVL4;

  //m_misc.c
  i=0; while (strcmp(defaults[i].name,"chatmacro0")!=0) i++;
  defaults[i].defaultvalue=(int)HUSTR_CHATMACRO0;
  defaults[i+1].defaultvalue=(int)HUSTR_CHATMACRO1;
  defaults[i+2].defaultvalue=(int)HUSTR_CHATMACRO2;
  defaults[i+3].defaultvalue=(int)HUSTR_CHATMACRO3;
  defaults[i+4].defaultvalue=(int)HUSTR_CHATMACRO4;
  defaults[i+5].defaultvalue=(int)HUSTR_CHATMACRO5;
  defaults[i+6].defaultvalue=(int)HUSTR_CHATMACRO6;
  defaults[i+7].defaultvalue=(int)HUSTR_CHATMACRO7;
  defaults[i+8].defaultvalue=(int)HUSTR_CHATMACRO8;
  defaults[i+9].defaultvalue=(int)HUSTR_CHATMACRO9;

  //dstrings.c
  endmsg[0]=QUITMSG;
  }



  


