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
// DESCRIPTION:
//	Printed strings for translation.
//	English language support (default).
//
//-----------------------------------------------------------------------------

#ifndef __D_ENGLSH__
#define __D_ENGLSH__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define ENGD_DEVSTR	"Development mode ON.\n"
#define ENGD_CDROM	"CD-ROM Version: default.cfg from c:\\doomdata\n"

//
//	M_Menu.C
//
#define ENGPRESSKEY 	"press a key."
#define ENGPRESSYN 	"press y or n."
#define ENGQUITMSG	"are you sure you want to\nquit this great game?"
#define ENGLOADNET 	"you can't do load while in a net game!\n\npress a key."
#define ENGQLOADNET	"you can't quickload during a netgame!\n\npress a key."
#define ENGQSAVESPOT	"you haven't picked a quicksave slot yet!\n\npress a key."
#define ENGSAVEDEAD 	"you can't save if you aren't playing!\n\npress a key."
#define ENGQSPROMPT 	"quicksave over your game named\n\n'%s'?\n\npress y or n."
#define ENGQLPROMPT	"do you want to quickload the game named\n\n'%s'?\n\npress y or n."

#define ENGNEWGAME	\
"you can't start a new game\n"\
"while in a network game.\n\npress a key."

#define ENGNIGHTMARE	\
"are you sure? this skill level\n"\
"isn't even remotely fair.\n\npress y or n."

#define ENGSWSTRING	\
"this is the shareware version of doom.\n\n"\
"you need to order the entire trilogy.\n\npress a key."

#define ENGMSGOFF	"Messages OFF"
#define ENGMSGON		"Messages ON"
#define ENGNETEND	"you can't end a netgame!\n\npress a key."
#define ENGENDGAME	"are you sure you want to end the game?\n\npress y or n."

#define ENGDOSY		"(press y to quit)"

#define ENGDETAILHI	"High detail"
#define ENGDETAILLO	"Low detail"
#define ENGGAMMALVL0	"Gamma correction OFF"
#define ENGGAMMALVL1	"Gamma correction level 1"
#define ENGGAMMALVL2	"Gamma correction level 2"
#define ENGGAMMALVL3	"Gamma correction level 3"
#define ENGGAMMALVL4	"Gamma correction level 4"
#define ENGEMPTYSTRING	"empty slot"

//
//	P_inter.C
//
#define ENGGOTARMOR	"Picked up the armor."
#define ENGGOTMEGA	"Picked up the MegaArmor!"
#define ENGGOTHTHBONUS	"Picked up a health bonus."
#define ENGGOTARMBONUS	"Picked up an armor bonus."
#define ENGGOTSTIM	"Picked up a stimpack."
#define ENGGOTMEDINEED	"Picked up a medikit that you REALLY need!"
#define ENGGOTMEDIKIT	"Picked up a medikit."
#define ENGGOTSUPER	"Supercharge!"

#define ENGGOTBLUECARD	"Picked up a blue keycard."
#define ENGGOTYELWCARD	"Picked up a yellow keycard."
#define ENGGOTREDCARD	"Picked up a red keycard."
#define ENGGOTBLUESKUL	"Picked up a blue skull key."
#define ENGGOTYELWSKUL	"Picked up a yellow skull key."
#define ENGGOTREDSKULL	"Picked up a red skull key."

#define ENGGOTINVUL	"Invulnerability!"
#define ENGGOTBERSERK	"Berserk!"
#define ENGGOTINVIS	"Partial Invisibility"
#define ENGGOTSUIT	"Radiation Shielding Suit"
#define ENGGOTMAP	"Computer Area Map"
#define ENGGOTVISOR	"Light Amplification Visor"
#define ENGGOTMSPHERE	"MegaSphere!"

#define ENGGOTCLIP	"Picked up a clip."
#define ENGGOTCLIPBOX	"Picked up a box of bullets."
#define ENGGOTROCKET	"Picked up a rocket."
#define ENGGOTROCKBOX	"Picked up a box of rockets."
#define ENGGOTCELL	"Picked up an energy cell."
#define ENGGOTCELLBOX	"Picked up an energy cell pack."
#define ENGGOTSHELLS	"Picked up 4 shotgun shells."
#define ENGGOTSHELLBOX	"Picked up a box of shotgun shells."
#define ENGGOTBACKPACK	"Picked up a backpack full of ammo!"

#define ENGGOTBFG9000	"You got the BFG9000!  Oh, yes."
#define ENGGOTCHAINGUN	"You got the chaingun!"
#define ENGGOTCHAINSAW	"A chainsaw!  Find some meat!"
#define ENGGOTLAUNCHER	"You got the rocket launcher!"
#define ENGGOTPLASMA	"You got the plasma gun!"
#define ENGGOTSHOTGUN	"You got the shotgun!"
#define ENGGOTSHOTGUN2	"You got the super shotgun!"

//
// P_Doors.C
//
#define ENGPD_BLUEO	"You need a blue key to activate this object"
#define ENGPD_REDO	"You need a red key to activate this object"
#define ENGPD_YELLOWO	"You need a yellow key to activate this object"
#define ENGPD_BLUEK	"You need a blue key to open this door"
#define ENGPD_REDK	"You need a red key to open this door"
#define ENGPD_YELLOWK	"You need a yellow key to open this door"

//
//	G_game.C
//
#define ENGGGSAVED	"game saved."

//
//	HU_stuff.C
//
#define ENGHUSTR_MSGU	"[Message unsent]"

#define ENGHUSTR_E1M1	"E1M1: Hangar"
#define ENGHUSTR_E1M2	"E1M2: Nuclear Plant"
#define ENGHUSTR_E1M3	"E1M3: Toxin Refinery"
#define ENGHUSTR_E1M4	"E1M4: Command Control"
#define ENGHUSTR_E1M5	"E1M5: Phobos Lab"
#define ENGHUSTR_E1M6	"E1M6: Central Processing"
#define ENGHUSTR_E1M7	"E1M7: Computer Station"
#define ENGHUSTR_E1M8	"E1M8: Phobos Anomaly"
#define ENGHUSTR_E1M9	"E1M9: Military Base"

#define ENGHUSTR_E2M1	"E2M1: Deimos Anomaly"
#define ENGHUSTR_E2M2	"E2M2: Containment Area"
#define ENGHUSTR_E2M3	"E2M3: Refinery"
#define ENGHUSTR_E2M4	"E2M4: Deimos Lab"
#define ENGHUSTR_E2M5	"E2M5: Command Center"
#define ENGHUSTR_E2M6	"E2M6: Halls of the Damned"
#define ENGHUSTR_E2M7	"E2M7: Spawning Vats"
#define ENGHUSTR_E2M8	"E2M8: Tower of Babel"
#define ENGHUSTR_E2M9	"E2M9: Fortress of Mystery"

#define ENGHUSTR_E3M1	"E3M1: Hell Keep"
#define ENGHUSTR_E3M2	"E3M2: Slough of Despair"
#define ENGHUSTR_E3M3	"E3M3: Pandemonium"
#define ENGHUSTR_E3M4	"E3M4: House of Pain"
#define ENGHUSTR_E3M5	"E3M5: Unholy Cathedral"
#define ENGHUSTR_E3M6	"E3M6: Mt. Erebus"
#define ENGHUSTR_E3M7	"E3M7: Limbo"
#define ENGHUSTR_E3M8	"E3M8: Dis"
#define ENGHUSTR_E3M9	"E3M9: Warrens"

#define ENGHUSTR_E4M1	"E4M1: Hell Beneath"
#define ENGHUSTR_E4M2	"E4M2: Perfect Hatred"
#define ENGHUSTR_E4M3	"E4M3: Sever The Wicked"
#define ENGHUSTR_E4M4	"E4M4: Unruly Evil"
#define ENGHUSTR_E4M5	"E4M5: They Will Repent"
#define ENGHUSTR_E4M6	"E4M6: Against Thee Wickedly"
#define ENGHUSTR_E4M7	"E4M7: And Hell Followed"
#define ENGHUSTR_E4M8	"E4M8: Unto The Cruel"
#define ENGHUSTR_E4M9	"E4M9: Fear"

#define ENGHUSTR_1	"level 1: entryway"
#define ENGHUSTR_2	"level 2: underhalls"
#define ENGHUSTR_3	"level 3: the gantlet"
#define ENGHUSTR_4	"level 4: the focus"
#define ENGHUSTR_5	"level 5: the waste tunnels"
#define ENGHUSTR_6	"level 6: the crusher"
#define ENGHUSTR_7	"level 7: dead simple"
#define ENGHUSTR_8	"level 8: tricks and traps"
#define ENGHUSTR_9	"level 9: the pit"
#define ENGHUSTR_10	"level 10: refueling base"
#define ENGHUSTR_11	"level 11: 'o' of destruction!"

#define ENGHUSTR_12	"level 12: the factory"
#define ENGHUSTR_13	"level 13: downtown"
#define ENGHUSTR_14	"level 14: the inmost dens"
#define ENGHUSTR_15	"level 15: industrial zone"
#define ENGHUSTR_16	"level 16: suburbs"
#define ENGHUSTR_17	"level 17: tenements"
#define ENGHUSTR_18	"level 18: the courtyard"
#define ENGHUSTR_19	"level 19: the citadel"
#define ENGHUSTR_20	"level 20: gotcha!"

#define ENGHUSTR_21	"level 21: nirvana"
#define ENGHUSTR_22	"level 22: the catacombs"
#define ENGHUSTR_23	"level 23: barrels o' fun"
#define ENGHUSTR_24	"level 24: the chasm"
#define ENGHUSTR_25	"level 25: bloodfalls"
#define ENGHUSTR_26	"level 26: the abandoned mines"
#define ENGHUSTR_27	"level 27: monster condo"
#define ENGHUSTR_28	"level 28: the spirit world"
#define ENGHUSTR_29	"level 29: the living end"
#define ENGHUSTR_30	"level 30: icon of sin"

#define ENGHUSTR_31	"level 31: wolfenstein"
#define ENGHUSTR_32	"level 32: grosse"

#define ENGPHUSTR_1	"level 1: congo"
#define ENGPHUSTR_2	"level 2: well of souls"
#define ENGPHUSTR_3	"level 3: aztec"
#define ENGPHUSTR_4	"level 4: caged"
#define ENGPHUSTR_5	"level 5: ghost town"
#define ENGPHUSTR_6	"level 6: baron's lair"
#define ENGPHUSTR_7	"level 7: caughtyard"
#define ENGPHUSTR_8	"level 8: realm"
#define ENGPHUSTR_9	"level 9: abattoire"
#define ENGPHUSTR_10	"level 10: onslaught"
#define ENGPHUSTR_11	"level 11: hunted"

#define ENGPHUSTR_12	"level 12: speed"
#define ENGPHUSTR_13	"level 13: the crypt"
#define ENGPHUSTR_14	"level 14: genesis"
#define ENGPHUSTR_15	"level 15: the twilight"
#define ENGPHUSTR_16	"level 16: the omen"
#define ENGPHUSTR_17	"level 17: compound"
#define ENGPHUSTR_18	"level 18: neurosphere"
#define ENGPHUSTR_19	"level 19: nme"
#define ENGPHUSTR_20	"level 20: the death domain"

#define ENGPHUSTR_21	"level 21: slayer"
#define ENGPHUSTR_22	"level 22: impossible mission"
#define ENGPHUSTR_23	"level 23: tombstone"
#define ENGPHUSTR_24	"level 24: the final frontier"
#define ENGPHUSTR_25	"level 25: the temple of darkness"
#define ENGPHUSTR_26	"level 26: bunker"
#define ENGPHUSTR_27	"level 27: anti-christ"
#define ENGPHUSTR_28	"level 28: the sewers"
#define ENGPHUSTR_29	"level 29: odyssey of noises"
#define ENGPHUSTR_30	"level 30: the gateway of hell"

#define ENGPHUSTR_31	"level 31: cyberden"
#define ENGPHUSTR_32	"level 32: go 2 it"

#define ENGTHUSTR_1	"level 1: system control"
#define ENGTHUSTR_2	"level 2: human bbq"
#define ENGTHUSTR_3	"level 3: power control"
#define ENGTHUSTR_4	"level 4: wormhole"
#define ENGTHUSTR_5	"level 5: hanger"
#define ENGTHUSTR_6	"level 6: open season"
#define ENGTHUSTR_7	"level 7: prison"
#define ENGTHUSTR_8	"level 8: metal"
#define ENGTHUSTR_9	"level 9: stronghold"
#define ENGTHUSTR_10	"level 10: redemption"
#define ENGTHUSTR_11	"level 11: storage facility"

#define ENGTHUSTR_12	"level 12: crater"
#define ENGTHUSTR_13	"level 13: nukage processing"
#define ENGTHUSTR_14	"level 14: steel works"
#define ENGTHUSTR_15	"level 15: dead zone"
#define ENGTHUSTR_16	"level 16: deepest reaches"
#define ENGTHUSTR_17	"level 17: processing area"
#define ENGTHUSTR_18	"level 18: mill"
#define ENGTHUSTR_19	"level 19: shipping/respawning"
#define ENGTHUSTR_20	"level 20: central processing"

#define ENGTHUSTR_21	"level 21: administration center"
#define ENGTHUSTR_22	"level 22: habitat"
#define ENGTHUSTR_23	"level 23: lunar mining project"
#define ENGTHUSTR_24	"level 24: quarry"
#define ENGTHUSTR_25	"level 25: baron's den"
#define ENGTHUSTR_26	"level 26: ballistyx"
#define ENGTHUSTR_27	"level 27: mount pain"
#define ENGTHUSTR_28	"level 28: heck"
#define ENGTHUSTR_29	"level 29: river styx"
#define ENGTHUSTR_30	"level 30: last call"

#define ENGTHUSTR_31	"level 31: pharaoh"
#define ENGTHUSTR_32	"level 32: caribbean"

#define ENGHUSTR_CHATMACRO1	"I'm ready to kick butt!"
#define ENGHUSTR_CHATMACRO2	"I'm OK."
#define ENGHUSTR_CHATMACRO3	"I'm not looking too good!"
#define ENGHUSTR_CHATMACRO4	"Help!"
#define ENGHUSTR_CHATMACRO5	"You suck!"
#define ENGHUSTR_CHATMACRO6	"Next time, scumbag..."
#define ENGHUSTR_CHATMACRO7	"Come here!"
#define ENGHUSTR_CHATMACRO8	"I'll take care of it."
#define ENGHUSTR_CHATMACRO9	"Yes"
#define ENGHUSTR_CHATMACRO0	"No"

#define ENGHUSTR_TALKTOSELF1	"You mumble to yourself"
#define ENGHUSTR_TALKTOSELF2	"Who's there?"
#define ENGHUSTR_TALKTOSELF3	"You scare yourself"
#define ENGHUSTR_TALKTOSELF4	"You start to rave"
#define ENGHUSTR_TALKTOSELF5	"You've lost it..."

#define ENGHUSTR_MESSAGESENT	"[Message Sent]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define ENGHUSTR_PLRGREEN	"Green: "
#define ENGHUSTR_PLRINDIGO	"Indigo: "
#define ENGHUSTR_PLRBROWN	"Brown: "
#define ENGHUSTR_PLRRED		"Red: "
#define ENGHUSTR_PLRGOLD	"Gold: "
#define ENGHUSTR_PLRBLUE	"Blue: "
#define ENGHUSTR_PLRDKBLUE	"DkBlue: "
#define ENGHUSTR_PLRPINK	"Pink: "

#define ENGHUSTR_KEYGREEN	"g"
#define ENGHUSTR_KEYINDIGO	"i"
#define ENGHUSTR_KEYBROWN "b"
#define ENGHUSTR_KEYRED	"r"

//
//	AM_map.C
//

#define ENGAMSTR_FOLLOWON	"Follow Mode ON"
#define ENGAMSTR_FOLLOWOFF	"Follow Mode OFF"

#define ENGAMSTR_GRIDON	"Grid ON"
#define ENGAMSTR_GRIDOFF	"Grid OFF"

#define ENGAMSTR_MARKEDSPOT	"Marked Spot"
#define ENGAMSTR_MARKSCLEARED	"All Marks Cleared"

//
//	ST_stuff.C
//

#define ENGSTSTR_MUS		"Music Change"
#define ENGSTSTR_NOMUS		"IMPOSSIBLE SELECTION"
#define ENGSTSTR_DQDON		"Degreelessness Mode On"
#define ENGSTSTR_DQDOFF	"Degreelessness Mode Off"

#define ENGSTSTR_KFAADDED	"Very Happy Ammo Added"
#define ENGSTSTR_FAADDED	"Ammo (no keys) Added"

#define ENGSTSTR_NCON		"No Clipping Mode ON"
#define ENGSTSTR_NCOFF		"No Clipping Mode OFF"

#define ENGSTSTR_BEHOLD	"inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define ENGSTSTR_BEHOLDX	"Power-up Toggled"

#define ENGSTSTR_CHOPPERS	"... doesn't suck - GM"
#define ENGSTSTR_CLEV		"Changing Level..."

//
//	F_Finale.C
//
#define ENGE1TEXT \
"Once you beat the big badasses and\n"\
"clean out the moon base you're supposed\n"\
"to win, aren't you? Aren't you? Where's\n"\
"your fat reward and ticket home? What\n"\
"the hell is this? It's not supposed to\n"\
"end this way!\n"\
"\n" \
"It stinks like rotten meat, but looks\n"\
"like the lost Deimos base.  Looks like\n"\
"you're stuck on The Shores of Hell.\n"\
"The only way out is through.\n"\
"\n"\
"To continue the DOOM experience, play\n"\
"The Shores of Hell and its amazing\n"\
"sequel, Inferno!\n"


#define ENGE2TEXT \
"You've done it! The hideous cyber-\n"\
"demon lord that ruled the lost Deimos\n"\
"moon base has been slain and you\n"\
"are triumphant! But ... where are\n"\
"you? You clamber to the edge of the\n"\
"moon and look down to see the awful\n"\
"truth.\n" \
"\n"\
"Deimos floats above Hell itself!\n"\
"You've never heard of anyone escaping\n"\
"from Hell, but you'll make the bastards\n"\
"sorry they ever heard of you! Quickly,\n"\
"you rappel down to  the surface of\n"\
"Hell.\n"\
"\n" \
"Now, it's on to the final chapter of\n"\
"DOOM! -- Inferno."


#define ENGE3TEXT \
"The loathsome spiderdemon that\n"\
"masterminded the invasion of the moon\n"\
"bases and caused so much death has had\n"\
"its ass kicked for all time.\n"\
"\n"\
"A hidden doorway opens and you enter.\n"\
"You've proven too tough for Hell to\n"\
"contain, and now Hell at last plays\n"\
"fair -- for you emerge from the door\n"\
"to see the green fields of Earth!\n"\
"Home at last.\n" \
"\n"\
"You wonder what's been happening on\n"\
"Earth while you were battling evil\n"\
"unleashed. It's good that no Hell-\n"\
"spawn could have come through that\n"\
"door with you ..."


#define ENGE4TEXT \
"the spider mastermind must have sent forth\n"\
"its legions of hellspawn before your\n"\
"final confrontation with that terrible\n"\
"beast from hell.  but you stepped forward\n"\
"and brought forth eternal damnation and\n"\
"suffering upon the horde as a true hero\n"\
"would in the face of something so evil.\n"\
"\n"\
"besides, someone was gonna pay for what\n"\
"happened to daisy, your pet rabbit.\n"\
"\n"\
"but now, you see spread before you more\n"\
"potential pain and gibbitude as a nation\n"\
"of demons run amok among our cities.\n"\
"\n"\
"next stop, hell on earth!"


// after level 6, put this:

#define ENGC1TEXT \
"YOU HAVE ENTERED DEEPLY INTO THE INFESTED\n" \
"STARPORT. BUT SOMETHING IS WRONG. THE\n" \
"MONSTERS HAVE BROUGHT THEIR OWN REALITY\n" \
"WITH THEM, AND THE STARPORT'S TECHNOLOGY\n" \
"IS BEING SUBVERTED BY THEIR PRESENCE.\n" \
"\n"\
"AHEAD, YOU SEE AN OUTPOST OF HELL, A\n" \
"FORTIFIED ZONE. IF YOU CAN GET PAST IT,\n" \
"YOU CAN PENETRATE INTO THE HAUNTED HEART\n" \
"OF THE STARBASE AND FIND THE CONTROLLING\n" \
"SWITCH WHICH HOLDS EARTH'S POPULATION\n" \
"HOSTAGE."

// After level 11, put this:

#define ENGC2TEXT \
"YOU HAVE WON! YOUR VICTORY HAS ENABLED\n" \
"HUMANKIND TO EVACUATE EARTH AND ESCAPE\n"\
"THE NIGHTMARE.  NOW YOU ARE THE ONLY\n"\
"HUMAN LEFT ON THE FACE OF THE PLANET.\n"\
"CANNIBAL MUTATIONS, CARNIVOROUS ALIENS,\n"\
"AND EVIL SPIRITS ARE YOUR ONLY NEIGHBORS.\n"\
"YOU SIT BACK AND WAIT FOR DEATH, CONTENT\n"\
"THAT YOU HAVE SAVED YOUR SPECIES.\n"\
"\n"\
"BUT THEN, EARTH CONTROL BEAMS DOWN A\n"\
"MESSAGE FROM SPACE: \"SENSORS HAVE LOCATED\n"\
"THE SOURCE OF THE ALIEN INVASION. IF YOU\n"\
"GO THERE, YOU MAY BE ABLE TO BLOCK THEIR\n"\
"ENTRY.  THE ALIEN BASE IS IN THE HEART OF\n"\
"YOUR OWN HOME CITY, NOT FAR FROM THE\n"\
"STARPORT.\" SLOWLY AND PAINFULLY YOU GET\n"\
"UP AND RETURN TO THE FRAY."


// After level 20, put this:

#define ENGC3TEXT \
"YOU ARE AT THE CORRUPT HEART OF THE CITY,\n"\
"SURROUNDED BY THE CORPSES OF YOUR ENEMIES.\n"\
"YOU SEE NO WAY TO DESTROY THE CREATURES'\n"\
"ENTRYWAY ON THIS SIDE, SO YOU CLENCH YOUR\n"\
"TEETH AND PLUNGE THROUGH IT.\n"\
"\n"\
"THERE MUST BE A WAY TO CLOSE IT ON THE\n"\
"OTHER SIDE. WHAT DO YOU CARE IF YOU'VE\n"\
"GOT TO GO THROUGH HELL TO GET TO IT?"


// After level 29, put this:

#define ENGC4TEXT \
"THE HORRENDOUS VISAGE OF THE BIGGEST\n"\
"DEMON YOU'VE EVER SEEN CRUMBLES BEFORE\n"\
"YOU, AFTER YOU PUMP YOUR ROCKETS INTO\n"\
"HIS EXPOSED BRAIN. THE MONSTER SHRIVELS\n"\
"UP AND DIES, ITS THRASHING LIMBS\n"\
"DEVASTATING UNTOLD MILES OF HELL'S\n"\
"SURFACE.\n"\
"\n"\
"YOU'VE DONE IT. THE INVASION IS OVER.\n"\
"EARTH IS SAVED. HELL IS A WRECK. YOU\n"\
"WONDER WHERE BAD FOLKS WILL GO WHEN THEY\n"\
"DIE, NOW. WIPING THE SWEAT FROM YOUR\n"\
"FOREHEAD YOU BEGIN THE LONG TREK BACK\n"\
"HOME. REBUILDING EARTH OUGHT TO BE A\n"\
"LOT MORE FUN THAN RUINING IT WAS.\n"



// Before level 31, put this:

#define ENGC5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"\
"WHO THE INMATES OF THIS CORNER OF HELL\n"\
"WILL BE."


// Before level 32, put this:

#define ENGC6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\n"\
"SUPER SECRET LEVEL!  YOU'D BETTER\n"\
"BLAZE THROUGH THIS ONE!\n"


// after map 06	

#define ENGP1TEXT  \
"You gloat over the steaming carcass of the\n"\
"Guardian.  With its death, you've wrested\n"\
"the Accelerator from the stinking claws\n"\
"of Hell.  You relax and glance around the\n"\
"room.  Damn!  There was supposed to be at\n"\
"least one working prototype, but you can't\n"\
"see it. The demons must have taken it.\n"\
"\n"\
"You must find the prototype, or all your\n"\
"struggles will have been wasted. Keep\n"\
"moving, keep fighting, keep killing.\n"\
"Oh yes, keep living, too."


// after map 11

#define ENGP2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."


// after map 20

#define ENGP3TEXT \
"You've bashed and battered your way into\n"\
"the heart of the devil-hive.  Time for a\n"\
"Search-and-Destroy mission, aimed at the\n"\
"Gatekeeper, whose foul offspring is\n"\
"cascading to Earth.  Yeah, he's bad. But\n"\
"you know who's worse!\n"\
"\n"\
"Grinning evilly, you check your gear, and\n"\
"get ready to give the bastard a little Hell\n"\
"of your own making!"

// after map 30

#define ENGP4TEXT \
"The Gatekeeper's evil face is splattered\n"\
"all over the place.  As its tattered corpse\n"\
"collapses, an inverted Gate forms and\n"\
"sucks down the shards of the last\n"\
"prototype Accelerator, not to mention the\n"\
"few remaining demons.  You're done. Hell\n"\
"has gone back to pounding bad dead folks \n"\
"instead of good live ones.  Remember to\n"\
"tell your grandkids to put a rocket\n"\
"launcher in your coffin. If you go to Hell\n"\
"when you die, you'll need it for some\n"\
"final cleaning-up ..."

// before map 31

#define ENGP5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."

// before map 32

#define ENGP6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."


#define ENGT1TEXT \
"You've fought your way out of the infested\n"\
"experimental labs.   It seems that UAC has\n"\
"once again gulped it down.  With their\n"\
"high turnover, it must be hard for poor\n"\
"old UAC to buy corporate health insurance\n"\
"nowadays..\n"\
"\n"\
"Ahead lies the military complex, now\n"\
"swarming with diseased horrors hot to get\n"\
"their teeth into you. With luck, the\n"\
"complex still has some warlike ordnance\n"\
"laying around."


#define ENGT2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."


#define ENGT3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define ENGT4TEXT \
"Suddenly, all is silent, from one horizon\n"\
"to the other. The agonizing echo of Hell\n"\
"fades away, the nightmare sky turns to\n"\
"blue, the heaps of monster corpses start \n"\
"to evaporate along with the evil stench \n"\
"that filled the air. Jeeze, maybe you've\n"\
"done it. Have you really won?\n"\
"\n"\
"Something rumbles in the distance.\n"\
"A blue light begins to glow inside the\n"\
"ruined skull of the demon-spitter."


#define ENGT5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."


#define ENGT6TEXT \
"Time for a vacation. You've burst the\n"\
"bowels of hell and by golly you're ready\n"\
"for a break. You mutter to yourself,\n"\
"Maybe someone else can kick Hell's ass\n"\
"next time around. Ahead lies a quiet town,\n"\
"with peaceful flowing water, quaint\n"\
"buildings, and presumably no Hellspawn.\n"\
"\n"\
"As you step off the transport, you hear\n"\
"the stomp of a cyberdemon's iron shoe."



//
// Character cast strings F_FINALE.C
//
#define ENGCC_ZOMBIE	"ZOMBIEMAN"
#define ENGCC_SHOTGUN	"SHOTGUN GUY"
#define ENGCC_HEAVY	"HEAVY WEAPON DUDE"
#define ENGCC_IMP	"IMP"
#define ENGCC_DEMON	"DEMON"
#define ENGCC_LOST	"LOST SOUL"
#define ENGCC_CACO	"CACODEMON"
#define ENGCC_HELL	"HELL KNIGHT"
#define ENGCC_BARON	"BARON OF HELL"
#define ENGCC_ARACH	"ARACHNOTRON"
#define ENGCC_PAIN	"PAIN ELEMENTAL"
#define ENGCC_REVEN	"REVENANT"
#define ENGCC_MANCU	"MANCUBUS"
#define ENGCC_ARCH	"ARCH-VILE"
#define ENGCC_SPIDER	"THE SPIDER MASTERMIND"
#define ENGCC_CYBER	"THE CYBERDEMON"
#define ENGCC_HERO	"OUR HERO"

//new strings
#define ENGV_INITSTR    "V_Init: allocate screens.\n"
#define ENGM_LDEFSTR    "M_LoadDefaults: Load system defaults.\n"
#define ENGZ_INITSTR    "Z_Init: Init zone memory allocation daemon. \n"
#define ENGW_INITSTR    "W_Init: Init WADfiles.\n"
#define ENGM_INITSTR    "M_Init: Init miscellaneous info.\n"
#define ENGR_INITSTR    "R_Init: Init DOOM refresh daemon - "
#define ENGP_INITSTR    "\nP_Init: Init Playloop state.\n"
#define ENGI_INITSTR    "I_Init: Setting up machine state.\n"
#define ENGD_CHKNETSTR  "D_CheckNetGame: Checking network game status.\n"
#define ENGS_INITSTR    "S_Init: Setting up sound.\n"
#define ENGHU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define ENGST_INITSTR   "ST_Init: Init status bar.\n"
#define ENGNETLISTEN    "listening for network start info...\n"
#define ENGNETSEND      "sending network start info...\n"
#define ENGTURBOSCLSTR  "turbo scale: %i%%\n"
#define ENGISTURBOSTR   "%s is turbo!"

#define ENGMODMSG\
	    "===========================================================================\n"\
	    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"\
	    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"\
	    "        You will not receive technical support for modified games.\n"\
	    "                      press enter to continue\n"\
	    "===========================================================================\n"
#define ENGNOSWMSG\
	    "===========================================================================\n"\
	    "             This version is NOT SHAREWARE, do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define ENGNOSWMSG2\
	    "===========================================================================\n"\
	    "                            Do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define ENGSWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define ENGUDOOMSTART\
 "                         The Ultimate DOOM Startup v%i.%i                        "
#define ENGSHAREDOOMSTART\
 "                            DOOM Shareware Startup v%i.%i                        "
#define ENGREGDOOMSTART\
 "                            DOOM Registered Startup v%i.%i                       "
#define ENGDOOM2START\
 "                         DOOM 2: Hell on Earth v%i.%i                            "
#define ENGPLUTSTART\
 "                   DOOM 2: Plutonia Experiment v%i.%i                            "
#define ENGTNTSTART\
 "                     DOOM 2: TNT - Evilution v%i.%i                              "
#define ENGPUBDOOMSTART\
 "                     Public DOOM - v%i.%i                                        "

//these are unused.  They're just here for Deh compatibility
#define ENGI_SDPMI   "I_StartupDPMI\n"
#define ENGI_SMOUSE  "I_StartupMouse\n"
#define ENGI_SJOY    "I_StartupJoystick\n"
#define ENGI_SKEY    "I_StartupKeyboard\n"
#define ENGI_SSOUND  "I_StartupSound\n"
#define ENGI_STIMER  "I_StartupTimer()\n"
#define ENGMOUSEOFF  "Mouse: not present\n"
#define ENGMOUSEON   "Mouse: detected\n"
#define ENGDPMI1     "DPMI memory: 0x%x"
#define ENGDPMI2     ", 0x%x allocated for zone\n"
#define ENGCOMMVER   "\tcommercial version.\n"
#define ENGSHAREVER  "\tshareware version.\n"
#define ENGREGVER    "\tregistered version.\n"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
