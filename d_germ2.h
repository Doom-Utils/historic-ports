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
//	German language support.
//
//-----------------------------------------------------------------------------
//
// Translation by Omega9 - eMail "Omega9@letterbox.com" or "Omega9@bigfoot.de"
// (Visit http://www.fortunecity.com/skyscraper/seagate/54/index.html)
//
//-----------------------------------------------------------------------------
//
// Translation modified and finished by TwoFace - eMail: "webwizard@gmx.net"
// URL: "http://privat.schlund.de/Danger-Zone"
//
//-----------------------------------------------------------------------------

#ifndef __D_GERMAN2__
#define __D_GERMAN2__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define GR2D_DEVSTR	"Development mode ON.\n"
#define GR2D_CDROM	"CD-ROM Version: default.cfg from c:\\doomdata\n"

//
// M_Menu.C
//
#define GR2PRESSKEY	"Taste druecken."
#define GR2PRESSYN	"Druecken Sie Y oder N."
#define GR2QUITMSG	"Wollen Sie dieses Spiel\nwirklich beenden?"
#define GR2LOADNET	"Laden im Netzwerk Modus nicht moeglich!\n\nTaste druecken."
#define GR2QLOADNET	"Quickload im Netzwerk Modus nicht moeglich!\n\nTaste druecken."
#define GR2QSAVESPOT	"Sie haben noch kein Quickload Feld ausgewaehlt!\n\nTaste druecken."
#define GR2SAVEDEAD	"Wenn Sie nicht spielen koennen Sie nicht speichern!\nTaste druecken."
#define GR2QSPROMPT	"Quicksave ueber vorhandenen Spielstand\n\n'%s'?\n\nDruecken Sie Y oder N."
#define GR2QLPROMPT	"Moechten Sie den Spielstand\n\n'%s' laden?\n\nDruecken Sie Y oder N."
					     
#define GR2NEWGAME	\
"Sie koennen kein neues Spiel\n"\
"im Netzwerk Modus starten.\n\nTaste druecken."

#define GR2NIGHTMARE	\
"Sind Sie sicher?\n"\
"Dieser Schwierigkeitsgrad ist nicht sehr fair.\n\nDruecken Sie Y oder N."

#define GR2SWSTRING	\
"Dies ist die Shareware Version von Doom.\n\n"\
"Sie muessen die Vollversion benutzen.\n\nTaste druecken."

#define GR2MSGOFF	"Nachrichten AUS"
#define GR2MSGON	"Nachrichten AN"
#define GR2NETEND	"Sie koennen kein Netzspiel beenden!\n\nTaste druecken."
#define GR2ENDGAME	"Wollen Sie das Spiel wirklich beenden?\n\nDruecken Sie Y oder N."

#define GR2DOSY		"(zum beenden Y druecken)"

#define GR2DETAILHI	"Hohe Detailstufe"
#define GR2DETAILLO	"Niedrige Detailstufe"
#define GR2GAMMALVL0	"Gamma Korrektur AUS"
#define GR2GAMMALVL1	"Gamma Korrektur Stufe 1"
#define GR2GAMMALVL2	"Gamma Korrektur Stufe 2"
#define GR2GAMMALVL3	"Gamma Korrektur Stufe 3"
#define GR2GAMMALVL4	"Gamma Korrektur Stufe 4"
#define GR2EMPTYSTRING	"Leerer Platz"

//
// P_inter.C
//
#define GR2GOTARMOR	"Armor aufgenommen."
#define GR2GOTMEGA	"MegaArmor aufgenommen!"
#define GR2GOTHTHBONUS	"Health Bonus aufgenommen."
#define GR2GOTARMBONUS	"Armor Bonus aufgenommen."
#define GR2GOTSTIM	"Stimpack aufgenommen."
#define GR2GOTMEDINEED	"Medikit aufgenommen, dass Sie WIRKLICH brauchen!"
#define GR2GOTMEDIKIT	"Medikit aufgenommen."
#define GR2GOTSUPER	"Soulsphere!"

#define GR2GOTBLUECARD	"Blaue keycard aufgenommen."
#define GR2GOTYELWCARD	"Gelbe keycard aufgenommen."
#define GR2GOTREDCARD	"Rote keycard aufgenommen."
#define GR2GOTBLUESKUL	"Blauen skullkey aufgenommen."
#define GR2GOTYELWSKUL	"Gelben skullkey aufgenommen."
#define GR2GOTREDSKULL	"Roten skullkey aufgenommen."

#define GR2GOTINVUL	"Unverwundbarkeit!"
#define GR2GOTBERSERK	"Berserk modus!"
#define GR2GOTINVIS	"Teilweise Unsichtbarkeit"
#define GR2GOTSUIT	"Strahlenschutzanzug"
#define GR2GOTMAP	"Computer Karte"
#define GR2GOTVISOR	"Nachtsichtgeraet"
#define GR2GOTMSPHERE	"Megasphere!"

#define GR2GOTCLIP	"Magazin aufgenommen."
#define GR2GOTCLIPBOX	"Kiste Patronen aufgenommen."
#define GR2GOTROCKET	"Rakete aufgenommen."
#define GR2GOTROCKBOX	"Kiste Raketen aufgenommen."
#define GR2GOTCELL	"Energiezelle aufgenommen."
#define GR2GOTCELLBOX	"Paket Energiezellen aufgenommen."
#define GR2GOTSHELLS	"4 Schrotpatronen aufgenommen."
#define GR2GOTSHELLBOX	"Schachtel Schrotpatronen aufgenommen."
#define GR2GOTBACKPACK	"Rucksack voller Munition aufgenommen!"

#define GR2GOTBFG9000	"Sie haben die BFG9000! Oh, ja!"
#define GR2GOTCHAINGUN	"Sie haben die Chaingun!"
#define GR2GOTCHAINSAW	"Eine Kettensaege! Her mit dem Fleisch!"
#define GR2GOTLAUNCHER	"Sie haben den Rocket Launcher!"
#define GR2GOTPLASMA	"Sie haben die Plasma Rifle!"
#define GR2GOTSHOTGUN	"Sie haben die Shotgun!"
#define GR2GOTSHOTGUN2	"Sie haben die Super Shotgun!"

//
// P_Doors.C
//
#define GR2PD_BLUEO	"Sie brauchen eine blaue Keycard um dieses Objekt zu aktivieren"
#define GR2PD_REDO	"Sie brauchen eine rote Keycard um dieses Objekt zu aktivieren"
#define GR2PD_YELLOWO	"Sie brauchen eine gelbe Keycard um dieses Objekt zu aktivieren"
#define GR2PD_BLUEK	"Diese Tuer benoetigt eine blaue Keycard"
#define GR2PD_REDK	"Diese Tuer benoetigt eine rote Keycard"
#define GR2PD_YELLOWK	"Diese Tuer benoetigt eine gelbe Keycard"

//
// G_game.C
//
#define GR2GGSAVED	"Spielstand gespeichert."

//
// HU_stuff.C
//
#define GR2HUSTR_MSGU	"[Nachricht nicht gesendet]"

#define GR2HUSTR_E1M1	"E1M1: hangar"
#define GR2HUSTR_E1M2	"E1M2: nuclear plant"
#define GR2HUSTR_E1M3	"E1M3: toxin refinery"
#define GR2HUSTR_E1M4	"E1M4: command control"
#define GR2HUSTR_E1M5	"E1M5: phobos lab"
#define GR2HUSTR_E1M6	"E1M6: central processing"
#define GR2HUSTR_E1M7	"E1M7: computer station"
#define GR2HUSTR_E1M8	"E1M8: phobos anomaly"
#define GR2HUSTR_E1M9	"E1M9: military base"

#define GR2HUSTR_E2M1	"E2M1: deimos anomaly"
#define GR2HUSTR_E2M2	"E2M2: containment area"
#define GR2HUSTR_E2M3	"E2M3: refinery"
#define GR2HUSTR_E2M4	"E2M4: deimos lab"
#define GR2HUSTR_E2M5	"E2M5: command center"
#define GR2HUSTR_E2M6	"E2M6: halls of the damned"
#define GR2HUSTR_E2M7	"E2M7: spawning vats"
#define GR2HUSTR_E2M8	"E2M8: tower of babel"
#define GR2HUSTR_E2M9	"E2M9: fortress of mystery"

#define GR2HUSTR_E3M1	"E3M1: hell keep"
#define GR2HUSTR_E3M2	"E3M2: slough of despair"
#define GR2HUSTR_E3M3	"E3M3: pandemonium"
#define GR2HUSTR_E3M4	"E3M4: house of pain"
#define GR2HUSTR_E3M5	"E3M5: unholy cathedral"
#define GR2HUSTR_E3M6	"E3M6: mt. erebus"
#define GR2HUSTR_E3M7	"E3M7: limbo"
#define GR2HUSTR_E3M8	"E3M8: dis"
#define GR2HUSTR_E3M9	"E3M9: warrens"

#define GR2HUSTR_E4M1	"E4M1: hell beneath"
#define GR2HUSTR_E4M2	"E4M2: perfect hatred"
#define GR2HUSTR_E4M3	"E4M3: sever the wicked"
#define GR2HUSTR_E4M4	"E4M4: unruly evil"
#define GR2HUSTR_E4M5	"E4M5: they will repent"
#define GR2HUSTR_E4M6	"E4M6: against thee wickedly"
#define GR2HUSTR_E4M7	"E4M7: and hell followed"
#define GR2HUSTR_E4M8	"E4M8: unto the cruel"
#define GR2HUSTR_E4M9	"E4M9: fear"

#define GR2HUSTR_1	"Level 1: entryway"
#define GR2HUSTR_2	"Level 2: underhalls"
#define GR2HUSTR_3	"Level 3: the gantlet"
#define GR2HUSTR_4	"Level 4: the focus"
#define GR2HUSTR_5	"Level 5: the waste tunnels"
#define GR2HUSTR_6	"Level 6: the crusher"
#define GR2HUSTR_7	"Level 7: dead simple"
#define GR2HUSTR_8	"Level 8: tricks and traps"
#define GR2HUSTR_9	"Level 9: the pit"
#define GR2HUSTR_10	"Level 10: refueling base"
#define GR2HUSTR_11	"Level 11: 'o' of destruction!"
/* Isn't the real name "circle of death"? */

#define GR2HUSTR_12	"Level 12: the factory"
#define GR2HUSTR_13	"Level 13: downtown"
#define GR2HUSTR_14	"Level 14: the inmost dens"
#define GR2HUSTR_15	"Level 15: industrial zone"
#define GR2HUSTR_16	"Level 16: suburbs"
#define GR2HUSTR_17	"Level 17: tenements"
#define GR2HUSTR_18	"Level 18: the courtyard"
#define GR2HUSTR_19	"Level 19: the citadel"
#define GR2HUSTR_20	"Level 20: gotcha!"

#define GR2HUSTR_21	"Level 21: nirvana"
#define GR2HUSTR_22	"Level 22: the catacombs"
#define GR2HUSTR_23	"Level 23: barrels o' fun"
#define GR2HUSTR_24	"Level 24: the chasm"
#define GR2HUSTR_25	"Level 25: bloodfalls"
#define GR2HUSTR_26	"Level 26: the abandoned mines"
#define GR2HUSTR_27	"Level 27: monster condo"
#define GR2HUSTR_28	"Level 28: the spirit world"
#define GR2HUSTR_29	"Level 29: the living end"
#define GR2HUSTR_30	"Level 30: icon of sin"

#define GR2HUSTR_31	"Level 31: wolfenstein"
#define GR2HUSTR_32	"Level 32: grosse"

#define GR2PHUSTR_1	"Level 1: congo"
#define GR2PHUSTR_2	"Level 2: well of souls"
#define GR2PHUSTR_3	"Level 3: aztec"
#define GR2PHUSTR_4	"Level 4: caged"
#define GR2PHUSTR_5	"Level 5: ghost town"
#define GR2PHUSTR_6	"Level 6: baron's lair"
#define GR2PHUSTR_7	"Level 7: caughtyard"
#define GR2PHUSTR_8	"Level 8: realm"
#define GR2PHUSTR_9	"Level 9: abattoire"
#define GR2PHUSTR_10	"Level 10: onslaught"
#define GR2PHUSTR_11	"Level 11: hunted"

#define GR2PHUSTR_12	"Level 12: speed"
#define GR2PHUSTR_13	"Level 13: the crypt"
#define GR2PHUSTR_14	"Level 14: genesis"
#define GR2PHUSTR_15	"Level 15: the twilight"
#define GR2PHUSTR_16	"Level 16: the omen"
#define GR2PHUSTR_17	"Level 17: compound"
#define GR2PHUSTR_18	"Level 18: neurosphere"
#define GR2PHUSTR_19	"Level 19: nme"
#define GR2PHUSTR_20	"Level 20: the death domain"

#define GR2PHUSTR_21	"Level 21: slayer"
#define GR2PHUSTR_22	"Level 22: impossible mission"
#define GR2PHUSTR_23	"Level 23: tombstone"
#define GR2PHUSTR_24	"Level 24: the final frontier"
#define GR2PHUSTR_25	"Level 25: the temple of darkness"
#define GR2PHUSTR_26	"Level 26: bunker"
#define GR2PHUSTR_27	"Level 27: anti-christ"
#define GR2PHUSTR_28	"Level 28: the sewers"
#define GR2PHUSTR_29	"Level 29: odyssey of noises"
#define GR2PHUSTR_30	"Level 30: the gateway of hell"

#define GR2PHUSTR_31	"Level 31: cyberden"
#define GR2PHUSTR_32	"Level 32: go 2 it"

#define GR2THUSTR_1	"Level 1: system control"
#define GR2THUSTR_2	"Level 2: human bbq"
#define GR2THUSTR_3	"Level 3: power control"
#define GR2THUSTR_4	"Level 4: wormhole"
#define GR2THUSTR_5	"Level 5: hanger"
#define GR2THUSTR_6	"Level 6: open season"
#define GR2THUSTR_7	"Level 7: prison"
#define GR2THUSTR_8	"Level 8: metal"
#define GR2THUSTR_9	"Level 9: stronghold"
#define GR2THUSTR_10	"Level 10: redemption"
#define GR2THUSTR_11	"Level 11: storage facility"

#define GR2THUSTR_12	"Level 12: crater"
#define GR2THUSTR_13	"Level 13: nukage processing"
#define GR2THUSTR_14	"Level 14: steel works"
#define GR2THUSTR_15	"Level 15: dead zone"
#define GR2THUSTR_16	"Level 16: deepest reaches"
#define GR2THUSTR_17	"Level 17: processing area"
#define GR2THUSTR_18	"Level 18: mill"
#define GR2THUSTR_19	"Level 19: shipping/respawning"
#define GR2THUSTR_20	"Level 20: central processing"

#define GR2THUSTR_21	"Level 21: administration center"
#define GR2THUSTR_22	"Level 22: habitat"
#define GR2THUSTR_23	"Level 23: lunar mining project"
#define GR2THUSTR_24	"Level 24: quarry"
#define GR2THUSTR_25	"Level 25: baron's den"
#define GR2THUSTR_26	"Level 26: ballistyx"
#define GR2THUSTR_27	"Level 27: mount pain"
#define GR2THUSTR_28	"Level 28: heck"
#define GR2THUSTR_29	"Level 29: river styx"
#define GR2THUSTR_30	"Level 30: last call"

#define GR2THUSTR_31	"Level 31: pharaoh"
#define GR2THUSTR_32	"Level 32: caribbean"

#define GR2HUSTR_CHATMACRO1	"Ich bin bereit!"
#define GR2HUSTR_CHATMACRO2	"Ich bin OK."
#define GR2HUSTR_CHATMACRO3	"Ich sehe nicht gerade gut aus!"
#define GR2HUSTR_CHATMACRO4	"Hilfe!"
#define GR2HUSTR_CHATMACRO5	"Du bist Scheisse!"
#define GR2HUSTR_CHATMACRO6	"Naechstes mal, kleiner..."
#define GR2HUSTR_CHATMACRO7	"Komm her!"
#define GR2HUSTR_CHATMACRO8	"Ich werd mich dessen annehmen."
#define GR2HUSTR_CHATMACRO9	"Ja"
#define GR2HUSTR_CHATMACRO0	"Nein"

#define GR2HUSTR_TALKTOSELF1	"Du fuehrst Selbstgespraeche"
#define GR2HUSTR_TALKTOSELF2	"Wer ist dort?"
#define GR2HUSTR_TALKTOSELF3	"Du erschreckst dich selbst"
#define GR2HUSTR_TALKTOSELF4	"Du beginnst auszuflippen"
#define GR2HUSTR_TALKTOSELF5	"Du hast verloren..."

#define GR2HUSTR_MESSAGESENT	"[Nachricht gesendet]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define GR2HUSTR_PLRGREEN	"Green: "
#define GR2HUSTR_PLRINDIGO	"Indigo: "
#define GR2HUSTR_PLRBROWN	"Brown: "
#define GR2HUSTR_PLRRED		"Red: "
#define GR2HUSTR_PLRGOLD	"Gold: "
#define GR2HUSTR_PLRBLUE	"Blue: "
#define GR2HUSTR_PLRDKBLUE	"DkBlue: "
#define GR2HUSTR_PLRPINK	"Pink: "

#define GR2HUSTR_KEYGREEN	"g"
#define GR2HUSTR_KEYINDIGO	"i"
#define GR2HUSTR_KEYBROWN	"b"
#define GR2HUSTR_KEYRED		"r"

//
// AM_map.C
//

#define GR2AMSTR_FOLLOWON	"Verfolgungs Modus AN"
#define GR2AMSTR_FOLLOWOFF	"Verfolgungs Modus AUS"

#define GR2AMSTR_GRIDON		"Gitter AN"
#define GR2AMSTR_GRIDOFF	"Gitter AUS"

#define GR2AMSTR_MARKEDSPOT	"Stelle markiert"
#define GR2AMSTR_MARKSCLEARED	"Alle Markierungen aufgehoben"

//
// ST_stuff.C
//

#define GR2STSTR_MUS		"Musikwechsel"
#define GR2STSTR_NOMUS		"Unmoegliche Auswahl"
#define GR2STSTR_DQDON		"Unverwundbarkeit AN"
#define GR2STSTR_DQDOFF		"Unverwundbarkeit AUS"

#define GR2STSTR_KFAADDED	"Volle Munition erhalten"
#define GR2STSTR_FAADDED	"Munition (keine Schluessel) erhalten"

#define GR2STSTR_NCON		"Anti-Kollisions Modus AN"
#define GR2STSTR_NCOFF		"Anti-Kollisions Modus AUS"

/* kept V,S,I,R,A,L - inVuln, Str, Inviso, Rad, Allmap, Lite-amp */
#define GR2STSTR_BEHOLD		"unVerwundbar, berSerk, unsIchtbar, stRahlenschutz, kArte oder Licht"
#define GR2STSTR_BEHOLDX	"Power-Up erhalten"

/* "Doesn't suck - GM" translated to: "Doesn't matter - GM" */
#define GR2STSTR_CHOPPERS        "... Macht nix - GM"
#define GR2STSTR_CLEV            "Wechsle Level..."

//
// F_Finale.C
//
#define GR2E1TEXT \
"nun, da sie die ganzen daemonen besiegt\n"\
"und die mondbasis gereinigt haben, sind\n"\
"sie zum gewinnen bestimmt, nicht wahr?\n"\
"nicht wahr? wo ist die fette belohnung,\n"\
"und das ticket nach hause? was zum\n"\
"teufel ist das? So soll es nicht enden!\n"\
"\n"\
"es stinkt wie verrottetes fleisch,\n"\
"aber sieht aus wie die verlorene deimos\n"\
"basis. scheint so als ob sie auf den\n"\
"Shores of Hell festsitzen.\n"\
"es gibt nur einen ausweg - dadurch!\n"\
"\n"\
"um das DOOM erlebnis fortzusetzen,\n"\
"spielen sie The Shores of Hell und die\n"\
"grossartige fortsetzung, Inferno!"


#define GR2E2TEXT \
"sie haben es vollbracht! der riesige\n"\
"cyberdemon lord, der die verlorene deimos\n"\
"mondbasis regiert hat wurde vernichtet,\n"\
"und sie triumphieren! aber ... wo sind\n"\
"sie? sie erklimmen den rand des mondes\n"\
"und blicken hinunter, um die\n"\
"schreckliche wahrheit zu sehen.\n"\
"\n"\
"daemonen ueberfluten selbst die hoelle!\n"\
"noch nie hat es jemand geschafft aus der\n"\
"hoelle zu entkommen, aber man wird es\n"\
"bereuen von ihnen gehoert zu haben!\n"\
"schnell klettern sie herab auf die\n"\
"oberflaeche der hoelle.\n"\
"\n"\
"nun liegt es am letzten kapitel von\n"\
"DOOM -- Inferno."

#define GR2E3TEXT \
"dem verhassten spiderdemon, der die\n"\
"invasion auf die erde angefuehrt,\n"\
"und so viel tod verursacht hat, wurde\n"\
"endgueltig in den arsch getreten.\n"\
"\n"\
"eine versteckte tuer oeffnet sich und\n"\
"sie treten ein. sie haben bewiesen dass\n"\
"sie zu gut fuer die hoelle sind; am\n"\
"ende spielt die hoelle fair - endlichy\n"\
"sehen sie die gruenen felder der erde.\n"\
"endlich zuhause.\n"\
"\n"\
"sie fragen sich, was wohl auf der erde\n"\
"geschehen ist, waehrend sie das boehse\n"\
"bekaempft haben. aber es ist gut,\n"\
"das kein wesen der hoelle mit ihnen\n"\
"durch die Tuer kommen konnte ..."


#define GR2E4TEXT \
"der spiderdemon muss seine legionen\n"\
"der hoellenwesen vor ihrer letzten\n"\
"begegnung mit dem biest aus der hoelle\n"\
"ausgesendet haben. aber sie machten weiter\n"\
"und brachten ewige verdammnis und leiden\n"\
"ueber die horde, wie es ein wahrer held\n"\
"im angesicht des boesen tun wuerde.\n"\
"\n"\
"nebenbei, jemand wird fuer das, was\n"\
"ihrem hasen daisy passiert ist bezahlen!\n"\
"\n"\
"aber nun sehen sie den sich als eine horde\n"\
"daemonen ausbreitenden schmerz, der ueber\n"\
"unsere staedte amok laeuft...\n"\
"\n"\
"naechster halt, Hell on Earth!"


// after level 6, put this:

#define GR2C1TEXT \
"SIE SIND TIEF IN DEN VERSEUCHTEN RAUM-\n"\
"HAFEN VORGEDRUNGEN. ABER ETWAS IST FALSCH.\n"\
"DIE MONSTER HABEN IHRE EIGENE REALITAET\n"\
"MITGEBRACHT, UND DIE TECHNOLOGIE DES\n"\
"RAUMHAFENS WIRD VON IHNEN UNTERBUNDEN.\n"\
"\n"\
"VORAUS SEHEN SIE EINEN VORPOSTEN DER\n"\
"HOELLE, EIN BEFESTIGTES GEBIET. WENN\n"\
"SIE ES HINTER SICH LASSEN, KOENNEN SIE\n"\
"IN DAS HERZ DES RAUMHAFENS EINDRINGEN,\n"\
"UND DEN KONTROLLHEBEL FINDEN, DER DIE\n"\
"ERDBEVOELKERUNG ALS GEISELN FESTHAELLT."


// After level 11, put this:

#define GR2C2TEXT \
"SIE HABEN GEWONNEN! IHR SIEG ERMOEGLICHTE\n"\
"DIE EVAKUIERUNG DER MENSCHENHEIT, UND DEM\n"\
"ALPTRAUM ZU ENTKOMMEN. NUN SIND SIE DER\n"\
"EINZIGE MENSCH AUF DEM PLANTETEN...\n"\
"KANNIBALISCHE MUTATIONEN, ALIENS,\n"\
"UND UNTOTE GEISTER SIND IHRE EINZIGEN\n"\
"NACHBARN. SIE LEHNEN SICH ZURUECK, UND\n"\
"WARTEN AUF IHREN TOD, WISSEND DAS SIE\n"\
"IHRE SPEZIES GERETTET HABEN ...\n"\
"ABER DANN BEAMT DIE ERDKONTROLLE EINE\n"\
"NACHRICHT AUS DEM ALL: \"SENSOREN HABEN\n"\
"DIE QUELLE DER ALIEN INVASION ENTDECKT!\n"\
"VIELLEICHT SIND SIE DORT IN DER LAGE\n"\
"IHREN ZUGANG ZU BLOCKIEREN. DIE BASIS IST\n"\
"IM HERZEN IHRER STADT, GANZ IN DER NAEHE\n"\
"DES RAUMHAFENS.\" LANGSAM UND SCHMERZVOLL\n"\
"STEHEN SIE AUF UND MACHEN SICH AUF DEN WEG."


// After level 20, put this:

#define GR2C3TEXT \
"SIE SIND IM HERZEN DER STADT, UMGEBEN\n"\
"VON DEN LEICHEN IHRER FEINDE. SIE SEHEN\n"\
"KEINEN WEG DEN ZUGANG DER KREATUREN\n"\
"AUF DIESER SEITE ZU BLOCKIEREN, ALSO\n"\
"BEISSEN SIE IHRE ZAEHNE ZUSAMMEN UND\n"\
"BETRETEN DAS TOR.\n"\
"\n"\
"ES MUSS EINEN WEG GEBEN, DAS TOR VON\n"\
"DER ANDEREN SEITE ZU VERSCHLIESSEN.\n"\
"WAS KUEMMERT ES SIE SCHON DAS SIE\n"\
"DURCH DIE HOELLE GEHEN MUESSEN UM\n"\
"DORTHIN ZU GELANGEN."


// After level 29, put this:

#define GR2C4TEXT \
"DIE SCHRECKLICHE VISAGE DES GROESSTEN\n"\
"MONSTERS DAS SIE JE GESEHEN HABEN\n"\
"KRIECHT VOR IHNEN, NACHDEM SIE SEIN\n"\
"GEHIRN MIT RAKETEN VOLLGEPUMPT HABEN.\n"\
"DAS MONSTER ZUCKT NOCH MAL ZUSAMMEN,\n"\
"UND STIRBT; SEINE GLIEDER VERWUESTEN\n"\
"UNZAEHLIGE MEILEN DER HOELLE.\n"\
"\n"\
"SIE HABEN ES GESCHAFFT. DIE INVASION\n"\
"IST VORBEI. DIE HOELLE IST EIN WRACK.\n"\
"SIE FRAGEN SICH, WO DIE GANZEN MONSTER\n"\
"HINGEHEN, WENN SIE STERBEN...\n"\
"DEN SCHWEISS VON DER STIRN WISCHEND,\n"\
"TRETEN SIE DEN LANGEN HEIMWEG AN.\n"\
"DER NEUAUFBAUF DER ERDE SOLLTE MEHR\n"\
"SPASS MACHEN, ALS DEREN ZERSTOERUNG."


// Before level 31, put this:

#define GR2C5TEXT \
"GRATULATION! SIE HABEN DEN GEHEIM LEVEL\n"\
"GEFUNDEN! SIEHT SO AUS, ALS WAERE ER\n"\
"VON MENSCHEN ERBAUT WORDEN. SIE FRAGEN\n"\
"SICH, WER DIE EINWOHNER WOHL IN DIESER\n"\
"ECKE DER HOELLE SEIEN WERDEN."


// Before level 32, put this:

#define GR2C6TEXT \
"GRATULATION! SIE HABEN DEN SUPER\n"\
"GEHEIM GEFUNDEN! SIE HAETTEN\n"\
"IHN BESSER AUSLASSEN SOLLEN!"


// after map 06     

#define GR2P1TEXT  \
"sie freuen sich ueber den dampfenden\n"\
"kadaver des hueters. mit seinem tod haben\n"\
"sie den beschleuniger den stinkenden\n"\
"klauen der hoelle entzogen. sie relaxen\n"\
"und schauen im zimmer herum. verdammt!\n"\
"da sollte doch noch ein funktionierender\n"\
"prototyp sein, aber sie koennen ihn nicht\n"\
"sehen. die daemonen muessen ihn\n"\
"mitgenommen haben.\n"\
"\n"\
"sie muessen den prototypen finden, oder\n"\
"all Ihre kaempfe werden verschwendung\n"\
"gewesen sein. bleiben sie in bewegung,\n"\
"am kaempfen, am toeten, oh ja, und auch\n"\
"am leben."


// after map 11

#define GR2P2TEXT \
"auch das toetliche arch-vile labyrinth\n"\
"konnte sie nicht stoppen, und sie sind\n"\
"zum beschleuniger prototyp gelangt, der\n"\
"bald effizient und permanent deaktiviert\n"\
"werden wird.\n"\
"\n"\
"sie sind einfach gut in solchen sachen."


// after map 20

#define GR2P3TEXT \
"sie haben sich ihren weg in das herz\n"\
"des deamonen-nestes freigeschlagen.\n"\
"zeit fuer eine suchen-und-zerstoeren\n"\
"mission nach dem torwaechter, dessen\n"\
"faule nachkommen auf der erde einfallen.\n"\
"ja, er ist boese. aber sie wissen wer\n"\
"schlimmer ist!\n"\
"\n"\
"teuflisch grinsend pruefen sie ihre\n"\
"waffen und machen sich bereit um dem\n"\
"bastard ein bisschen ihrer eigenen\n"\
"hoelle zu zeigen!"


// after map 30

#define GR2P4TEXT \
"das teuflische gesicht des torwaechters\n"\
"ist ueber die gesamte umgebung verteilt.\n"\
"als sein zerissener koerper zusammenfaellt,\n"\
"formt sich ein umgekehrtes tor und saugt\n"\
"die scherben des letzten beschleuniger\n"\
"prototypen aus; nicht zu vergessen die\n"\
"paar uebrigen deamonen. sie haben es\n"\
"geschafft. die hoelle ist zurueckgegangen\n"\
"um die toten daemonen zu zerschlagen\n"\
"anstatt die leben der menschen. vergessen\n"\
"sie nicht ihre enkel daran zu erinnern\n"\
"einen raketenwerfer in ihren sarg mit\n"\
"einzupacken. wenn sie sterben und zur\n"\
"hoelle gehen, werden sie ihn brauchen\n"\
"zu letzten aufraeumarbeiten ..."


// before map 31

#define GR2P5TEXT \
"sie haben das zweitschwierigste level\n"\
"gefunden, dass wir haben. hoffentlich\n"\
"haben sie einen gespeicherten spielstand\n"\
"ein oder zwei level zuvor. Wenn nicht\n"\
"seien sie bereit zu sterben.\n"\
"\n"\
"nur fuer harte typen."


// before map 32

#define GR2P6TEXT \
"haben sie sich schon gefragt welches das\n"\
"schwierigste level war welches wir fuer\n"\
"sie haben? nun wissen sie es.\n"\
"niemand kommt hier lebend heraus."


#define GR2T1TEXT \
"sie haben sich ihren weg durch die\n"\
"verseuchten experimentier labore frei\n"\
"gekaempft. es scheint als ob UAC sie\n"\
"wieder geschluckt haette. mit ihrem\n"\
"grossen sturz muss es fuer die	gute\n"\
"alte UAC schwer sein heutzutage eine\n"\
"firmengesundheitsversicherung\n"\
"abzuschliessen...\n"\
"\n"\
"voraus liegt der militaer komplex, nun\n"\
"ueberfuellt mit kranken gestalten, die\n"\
"heiss darauf sind ihre zaehne in sie zu\n"\
"bekommen. mit glueck liegen im komplex\n"\
"aber noch ein paar waffen herum."


#define GR2T2TEXT \
"sie hoeren das knirschen der schweren\n"\
"maschienen voraus. sie hoffen sicher\n"\
"das sie nicht neue daemonen erschaffen,\n"\
"aber sie sind bereit alle zu vernichten,\n"\
"wenn sie muessen. sie planen vielleicht\n"\
"ein blutmahl, aber sie fuehlen sich als\n"\
"ob man zweitausend wahnsinnige in einen\n"\
"verrueckten killer gesteckt haette.\n"\
"\n"\
"sie werden nicht so einfach aufgeben."


#define GR2T3TEXT \
"der offene ausblick voraus sieht verdammt\n"\
"vertraut aus. es riecht auch vertraut --\n"\
"wie exkremente. sie mochten den ort nicht\n"\
"zuvor, und sicherlich planen sie es jetzt\n"\
"auch nicht zu tun. je laenger sie hier\n"\
"bleiben, desto wahnsinniger werden sie.\n"\
"ein teuflisches grinsen tropft auf ihr\n"\
"gesicht. zeit sich einen namen zu machen."


#define GR2T4TEXT \
"endlich ist alles still, von einem\n"\
"horizont zum anderen. das qualvolle echo\n"\
"der hoelle verschwindet, der alptraum\n"\
"himmel verwandelt sich in blau, der haufen\n"\
"monsterleichen beginnt zu verdampfen,\n"\
"zusammen mit dem teuflischen gestank in\n"\
"der luft. jawohl, vielleicht haben sie es\n"\
"geschafft. haben sie wirklich gewonnen?\n"\
"\n"\
"etwas droehnt in der entfernung.\n"\
"im verfallenen schaedel des daemonen\n"\
"beginnt ein blaues licht zu gluehen."


#define GR2T5TEXT \
"was nun? es sieht total anders aus.\n"\
"ein bisschen wie koenig tuts condo.\n"\
"was kann hier schon schlimmer sein\n"\
"als sonst, nicht wahr? vielleicht\n"\
"ist es aber besser schlafende\n"\
"goetter schlafen zu lassen ..."


#define GR2T6TEXT \
"zeit fuer urlaub. sie haben die eier der\n"\
"hoelle zerschmettert und sie sind bereit\n"\
"fuer eine pause. sie murmeln zu sich:\n"\
"vielleicht kann das naechste mal jemand\n"\
"anderes der hoelle in der arsch treten.\n"\
"voraus liegt eine stille stadt mit\n"\
"friedlich fliessendem wasser, alten\n"\
"gebaeuden und sicher ohne daemonen.\n"\
"\n"\
"als sie aus dem transporter aussteigen,\n"\
"hoeren sie das stampfen eines cyberdemons."


//
// Character cast strings F_FINALE.C
//
#define GR2CC_ZOMBIE   "ZOMBIEMAN"
#define GR2CC_SHOTGUN  "SHOTGUN GUY"
#define GR2CC_HEAVY    "HEAVY WEAPON DUDE"
#define GR2CC_IMP      "IMP"
#define GR2CC_DEMON    "DEMON"
#define GR2CC_LOST     "LOST SOUL"
#define GR2CC_CACO     "CACODEMON"
#define GR2CC_HELL     "HELL KNIGHT"
#define GR2CC_BARON    "BARON OF HELL"
#define GR2CC_ARACH    "ARACHNOTRON"
#define GR2CC_PAIN     "PAIN ELEMENTAL"
#define GR2CC_REVEN    "REVENANT"
#define GR2CC_MANCU    "MANCUBUS"
#define GR2CC_ARCH     "ARCH-VILE"
#define GR2CC_SPIDER   "THE SPIDER MASTERMIND"
#define GR2CC_CYBER    "THE CYBERDEMON"
#define GR2CC_HERO     "UNSER HELD"

//new strings
#define GR2V_INITSTR    "V_Init: allocate screens.\n"
#define GR2M_LDEFSTR    "M_LoadDefaults: Load system defaults.\n"
#define GR2Z_INITSTR    "Z_Init: Init zone memory allocation daemon. \n"
#define GR2W_INITSTR    "W_Init: Init WADfiles.\n"
#define GR2M_INITSTR    "M_Init: Init miscellaneous info.\n"
#define GR2R_INITSTR    "R_Init: Init DOOM refresh daemon - "
#define GR2P_INITSTR    "\nP_Init: Init Playloop state.\n"
#define GR2I_INITSTR    "I_Init: Setting up machine state.\n"
#define GR2D_CHKNETSTR  "D_CheckNetGame: Checking network game status.\n"
#define GR2S_INITSTR    "S_Init: Setting up sound.\n"
#define GR2HU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define GR2ST_INITSTR   "ST_Init: Init status bar.\n"
#define GR2NETLISTEN    "listening for network start info...\n"
#define GR2NETSEND      "sending network start info...\n"
#define GR2TURBOSCLSTR  "turbo scale: %i%%\n"
#define GR2ISTURBOSTR   "%s is turbo!"

#define GR2MODMSG\
	    "===========================================================================\n"\
	    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"\
	    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"\
	    "        You will not receive technical support for modified games.\n"\
	    "                      press enter to continue\n"\
	    "===========================================================================\n"
#define GR2NOSWMSG\
	    "===========================================================================\n"\
	    "             This version is NOT SHAREWARE, do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define GR2NOSWMSG2\
	    "===========================================================================\n"\
	    "                            Do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define GR2SWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define GR2UDOOMSTART\
 "                         The Ultimate DOOM Startup v%i.%i                        "
#define GR2SHAREDOOMSTART\
 "                            DOOM Shareware Startup v%i.%i                        "
#define GR2REGDOOMSTART\
 "                            DOOM Registered Startup v%i.%i                       "
#define GR2DOOM2START\
 "                         DOOM 2: Hell on Earth v%i.%i                            "
#define GR2PLUTSTART\
 "                   DOOM 2: Plutonia Experiment v%i.%i                            "
#define GR2TNTSTART\
 "                     DOOM 2: TNT - Evilution v%i.%i                              "
#define GR2PUBDOOMSTART\
 "                     Public DOOM - v%i.%i                                        "


//these are unused.  They're just here for Deh compatibility
#define GR2I_SDPMI   "I_StartupDPMI\n"
#define GR2I_SMOUSE  "I_StartupMouse\n"
#define GR2I_SJOY    "I_StartupJoystick\n"
#define GR2I_SKEY    "I_StartupKeyboard\n"
#define GR2I_SSOUND  "I_StartupSound\n"
#define GR2I_STIMER  "I_StartupTimer()\n"
#define GR2MOUSEOFF  "Mouse: not present\n"
#define GR2MOUSEON   "Mouse: detected\n"
#define GR2DPMI1     "DPMI memory: 0x%x"
#define GR2DPMI2     ", 0x%x allocated for zone\n"
#define GR2COMMVER   "\tcommercial version.\n"
#define GR2SHAREVER  "\tshareware version.\n"
#define GR2REGVER    "\tregistered version.\n"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
