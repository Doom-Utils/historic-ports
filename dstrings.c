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


#ifdef __GNUG__
#pragma implementation "dstrings.h"
#endif
#include "dstrings.h"
#include "f_finale.h"
#include "m_misc.h"

char *endmsg[]=
{
  // DOOM1
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

  // -KM- 1998/07/21 Censorship changed.  All these messages should go in a ddf file
  //                 anyways.
  "don't quit now! we're \nstill spending your money!",

  // Internal debug. Different style, too.
  "THIS IS NO MESSAGE!\nPage intentionally left blank.",
  NULL
};

extern char* chat_macros[];
extern char* player_names[MAXPLAYERS];
extern char destination_keys[MAXPLAYERS];
extern char*  gammamsg[5];
extern default_t defaults[];


void applystrings() //call after initstrings and after dehacked
{
  int i;

  //hu_stuff.c
  chat_macros[0]=DDF_LanguageLookup("DefaultCHATMACRO0");
  chat_macros[1]=DDF_LanguageLookup("DefaultCHATMACRO1");
  chat_macros[2]=DDF_LanguageLookup("DefaultCHATMACRO2");
  chat_macros[3]=DDF_LanguageLookup("DefaultCHATMACRO3");
  chat_macros[4]=DDF_LanguageLookup("DefaultCHATMACRO4");
  chat_macros[5]=DDF_LanguageLookup("DefaultCHATMACRO5");
  chat_macros[6]=DDF_LanguageLookup("DefaultCHATMACRO6");
  chat_macros[7]=DDF_LanguageLookup("DefaultCHATMACRO7");
  chat_macros[8]=DDF_LanguageLookup("DefaultCHATMACRO8");
  chat_macros[9]=DDF_LanguageLookup("DefaultCHATMACRO9");

  player_names[0]=DDF_LanguageLookup("PlayerNameOne");
  player_names[1]=DDF_LanguageLookup("PlayerNameTwo");
  player_names[2]=DDF_LanguageLookup("PlayerNameThree");
  player_names[3]=DDF_LanguageLookup("PlayerNameFour");
  player_names[4]=DDF_LanguageLookup("PlayerNameFive");
  player_names[5]=DDF_LanguageLookup("PlayerNameSix");
  player_names[6]=DDF_LanguageLookup("PlayerNameSeven");
  player_names[7]=DDF_LanguageLookup("PlayerNameEight");

  destination_keys[0]='g';
  destination_keys[1]='i';
  destination_keys[2]='b';
  destination_keys[3]='r';

  //m_menu.c
  gammamsg[0]=DDF_LanguageLookup("GammaOff");
  gammamsg[1]=DDF_LanguageLookup("GammaLevelOne");
  gammamsg[2]=DDF_LanguageLookup("GammaLevelTwo");
  gammamsg[3]=DDF_LanguageLookup("GammaLevelThree");
  gammamsg[4]=DDF_LanguageLookup("GammaLevelFour");

  //m_misc.c
  i = 0;

  while (strcmp(defaults[i].name,"chatmacro0"))
    i++;

  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO0");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO1");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO2");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO3");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO4");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO5");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO6");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO7");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO8");
  defaults[i++].defaultvalue=(int)DDF_LanguageLookup("DefaultCHATMACRO9");

}



  


