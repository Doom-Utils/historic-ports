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
//        Printed strings for translation.
//        German language support.

// ----------------------------------------------------------------------------
// Translation by Omega9 - eMail "Omega9@letterbox.com" or "Omega9@bigfoot.de"
// (Visit http://www.fortunecity.com/skyscraper/seagate/54/index.html)
// ----------------------------------------------------------------------------

// Comments for German translation: Search for "/*@"

#ifndef __D_GERMAN1__
#define __D_GERMAN1__

//
//        Printed strings for translation
//

//
// D_Main.C
//
#define GR1D_DEVSTR    "Development mode ON.\n"
#define GR1D_CDROM     "CD-ROM Version: default.cfg from c:\\doomdata\n"

//
//        M_Menu.C
//
#define GR1PRESSKEY    "Tastendruck."
#define GR1PRESSYN     "Taste Y oder N."
#define GR1QUITMSG     "Wollen Sie dieses Spiel\nwirklich beenden?"
#define GR1LOADNET     "Laden im Netzwerk Mode nicht erlaubt!\n\nTastendruck."
#define GR1QLOADNET    "Quickload im Netzwerk Mode nicht erlaubt!\n\nTastendruck."
#define GR1QSAVESPOT   "Sie haben noch kein Quickload Feld gewaehlt!\n\nTastendruck."
#define GR1SAVEDEAD    "Spielen Sie nicht, speichern Sie nicht!\nTastendruck."
#define GR1QSPROMPT    "Quicksave entfernt vorhandenes Save\n\n'%s'?\n\nTaste Y oder N."
#define GR1QLPROMPT    "Save quickloaden?\n\n'%s'?\n\nTaste Y oder N."
					     
#define GR1NEWGAME     \
"Neues Spiel im Netzwerkmode\n"\
"nicht erlaubt.\n\nTastendruck."

#define GR1NIGHTMARE   \
"Sind Sie sicher? Dieser Skill\n"\
"Level ist nicht sehr fair.\n\nTaste Y oder N."

#define GR1SWSTRING    \
"Dies ist die Shareware Version von Doom.\n\n"\
"Ordern Sie die Entire Trilogy.\n\nTastendruck."

#define GR1MSGOFF      "nachrichten AUS"
#define GR1MSGON       "Nachrichten AN"
#define GR1NETEND      "Netzwerkspiel nicht beendbar!\n\nTastendruck."
#define GR1ENDGAME     "Wollen Sie das Spiel wirklich beenden?\n\nTaste Y oder N."

#define GR1DOSY                  "(\"y\" beendet)"

#define GR1DETAILHI    "Hohe Details"
#define GR1DETAILLO    "Niedrige Details"
#define GR1GAMMALVL0   "Helligkeitsregelung AUS"
#define GR1GAMMALVL1   "Helligkeitsregelung Stufe 1"
#define GR1GAMMALVL2   "Helligkeitsregelung Stufe 2"
#define GR1GAMMALVL3   "Helligkeitsregelung Stufe 3"
#define GR1GAMMALVL4   "Helligkeitsregelung Stufe 4"
#define GR1EMPTYSTRING "Leerer Platz"

//
//        P_inter.C
//
#define GR1GOTARMOR    "Armor aufgenommen."
#define GR1GOTMEGA     "MegaArmor aufgenommen!"
#define GR1GOTHTHBONUS "Health Bonus aufgenommen."
#define GR1GOTARMBONUS "Armor Bonus aufgenommen."
#define GR1GOTSTIM     "Stimpack aufgenommen."
#define GR1GOTMEDINEED "Medikit augenommen, das Sie WIRKLICH brauchen!"
#define GR1GOTMEDIKIT  "Medikit aufgenommen."
#define GR1GOTSUPER    "Supercharge!"

#define GR1GOTBLUECARD "Blaue Zugangskarte aufgenommen."
#define GR1GOTYELWCARD "Gelbe Zugangskarte aufgenommen."
#define GR1GOTREDCARD  "Rote Zugangskarte aufgenommen."
#define GR1GOTBLUESKUL "Blauen Kopf aufgenommen."
#define GR1GOTYELWSKUL "Gelben Kopf aufgenommen."
#define GR1GOTREDSKULL "Roten Kopf aufgenommen."

#define GR1GOTINVUL    "Unverwundbarkeit!"
#define GR1GOTBERSERK  "Berserker!"
#define GR1GOTINVIS    "Teilweise Unsichtbarkeit"
#define GR1GOTSUIT     "Strahlungsanzug"
#define GR1GOTMAP      "Computer Levelkarte"
#define GR1GOTVISOR    "Restlichtverstaerker"
#define GR1GOTMSPHERE  "MegaSphere!"

#define GR1GOTCLIP     "Patrone aufgenommen"
#define GR1GOTCLIPBOX  "Schachtel Patronen aufgenommen"
#define GR1GOTROCKET   "Rakete aufgenommen."
#define GR1GOTROCKBOX  "Schachtel Raketen aufgenommen."
#define GR1GOTCELL     "Energie Zelle aufgenommen."
#define GR1GOTCELLBOX  "Energie Zellenpack aufgenommen."
#define GR1GOTSHELLS   "4 Gewehrpatronen aufgenommen."
#define GR1GOTSHELLBOX "Schactel Gewehrpatronen aufgenommen."
#define GR1GOTBACKPACK "Rucksack voller Munition aufgenommen!"

#define GR1GOTBFG9000  "Sie haben die BFG9000!  Oh, ja."
#define GR1GOTCHAINGUN "Schnellfeuerwaffe!"
#define GR1GOTCHAINSAW "Eine Kettensaege!  Her mit dem Fleisch!"
#define GR1GOTLAUNCHER "Raketenwerfer gefunden!"
#define GR1GOTPLASMA   "Plasmawerfer gefunden!"
#define GR1GOTSHOTGUN  "Schrotflinte gefunden!"
#define GR1GOTSHOTGUN2 "Sie haben die Super Schrotflinte gefunden!"

//
// P_Doors.C
//
#define GR1PD_BLUEO    "Um dieses Objekt zu aktivieren brauchen sie eine blaue Zugangskarte!"
#define GR1PD_REDO     "Um dieses Objekt zu aktivieren brauchen sie eine rote Zugangskarte!"
#define GR1PD_YELLOWO  "Um dieses Objekt zu aktivieren brauchen sie eine gelbe Zugangskarte!"

#define GR1PD_BLUEK    "Sie brauchen die blaue Zugangskarte"
#define GR1PD_REDK     "Sie brauchen die rote Zugangskarte"
#define GR1PD_YELLOWK  "Sie brauchen die glbe Zugangskarte"

//
//        G_game.C
//
#define GR1GGSAVED     "Spiel gespeichert."

//
//        HU_stuff.C
//
#define GR1HUSTR_MSGU  "[Nachricht nicht gesendet]"

#define GR1HUSTR_E1M1  "E1M1: Hangar"
#define GR1HUSTR_E1M2  "E1M2: Nuclear Plant"
#define GR1HUSTR_E1M3  "E1M3: Toxin Refinery"
#define GR1HUSTR_E1M4  "E1M4: Command Control"
#define GR1HUSTR_E1M5  "E1M5: Phobos Lab"
#define GR1HUSTR_E1M6  "E1M6: Central Processing"
#define GR1HUSTR_E1M7  "E1M7: Computer Station"
#define GR1HUSTR_E1M8  "E1M8: Phobos Anomaly"
#define GR1HUSTR_E1M9  "E1M9: Military Base"

#define GR1HUSTR_E2M1  "E2M1: Deimos Anomaly"
#define GR1HUSTR_E2M2  "E2M2: Containment Area"
#define GR1HUSTR_E2M3  "E2M3: Refinery"
#define GR1HUSTR_E2M4  "E2M4: Deimos Lab"
#define GR1HUSTR_E2M5  "E2M5: Command Center"
#define GR1HUSTR_E2M6  "E2M6: Halls of the Damned"
#define GR1HUSTR_E2M7  "E2M7: Spawning Vats"
#define GR1HUSTR_E2M8  "E2M8: Tower of Babel"
#define GR1HUSTR_E2M9  "E2M9: Fortress of Mystery"

#define GR1HUSTR_E3M1  "E3M1: Hell Keep"
#define GR1HUSTR_E3M2  "E3M2: Slough of Despair"
#define GR1HUSTR_E3M3  "E3M3: Pandemonium"
#define GR1HUSTR_E3M4  "E3M4: House of Pain"
#define GR1HUSTR_E3M5  "E3M5: Unholy Cathedral"
#define GR1HUSTR_E3M6  "E3M6: Mt. Erebus"
#define GR1HUSTR_E3M7  "E3M7: Limbo"
#define GR1HUSTR_E3M8  "E3M8: Dis"
#define GR1HUSTR_E3M9  "E3M9: Warrens"

#define GR1HUSTR_E4M1  "E4M1: Hell Beneath"
#define GR1HUSTR_E4M2  "E4M2: Perfect Hatred"
#define GR1HUSTR_E4M3  "E4M3: Sever The Wicked"
#define GR1HUSTR_E4M4  "E4M4: Unruly Evil"
#define GR1HUSTR_E4M5  "E4M5: They Will Repent"
#define GR1HUSTR_E4M6  "E4M6: Against Thee Wickedly"
#define GR1HUSTR_E4M7  "E4M7: And Hell Followed"
#define GR1HUSTR_E4M8  "E4M8: Unto The Cruel"
#define GR1HUSTR_E4M9  "E4M9: Fear"

#define GR1HUSTR_1     "Level 1: entryway"
#define GR1HUSTR_2     "Level 2: underhalls"
#define GR1HUSTR_3     "Level 3: the gantlet"
#define GR1HUSTR_4     "Level 4: the focus"
#define GR1HUSTR_5     "Level 5: the waste tunnels"
#define GR1HUSTR_6     "Level 6: the crusher"
#define GR1HUSTR_7     "Level 7: dead simple"
#define GR1HUSTR_8     "Level 8: tricks and traps"
#define GR1HUSTR_9     "Level 9: the pit"
#define GR1HUSTR_10    "Level 10: refueling base"
#define GR1HUSTR_11    "Level 11: 'o' of destruction!"

#define GR1HUSTR_12    "Level 12: the factory"
#define GR1HUSTR_13    "Level 13: downtown"
#define GR1HUSTR_14    "Level 14: the inmost dens"
#define GR1HUSTR_15    "Level 15: industrial zone"
#define GR1HUSTR_16    "Level 16: suburbs"
#define GR1HUSTR_17    "Level 17: tenements"
#define GR1HUSTR_18    "Level 18: the courtyard"
#define GR1HUSTR_19    "Level 19: the citadel"
#define GR1HUSTR_20    "Level 20: gotcha!"

#define GR1HUSTR_21    "Level 21: nirvana"
#define GR1HUSTR_22    "Level 22: the catacombs"
#define GR1HUSTR_23    "Level 23: barrels o' fun"
#define GR1HUSTR_24    "Level 24: the chasm"
#define GR1HUSTR_25    "Level 25: bloodfalls"
#define GR1HUSTR_26    "Level 26: the abandoned mines"
#define GR1HUSTR_27    "Level 27: monster condo"
#define GR1HUSTR_28    "Level 28: the spirit world"
#define GR1HUSTR_29    "Level 29: the living end"
#define GR1HUSTR_30    "Level 30: icon of sin"

#define GR1HUSTR_31    "Level 31: wolfenstein"
#define GR1HUSTR_32    "Level 32: grosse"

#define GR1PHUSTR_1    "Level 1: congo"
#define GR1PHUSTR_2    "Level 2: well of souls"
#define GR1PHUSTR_3    "Level 3: aztec"
#define GR1PHUSTR_4    "Level 4: caged"
#define GR1PHUSTR_5    "Level 5: ghost town"
#define GR1PHUSTR_6    "Level 6: baron's lair"
#define GR1PHUSTR_7    "Level 7: caughtyard"
#define GR1PHUSTR_8    "Level 8: realm"
#define GR1PHUSTR_9    "Level 9: abattoire"
#define GR1PHUSTR_10   "Level 10: onslaught"
#define GR1PHUSTR_11   "Level 11: hunted"

#define GR1PHUSTR_12   "Level 12: speed"
#define GR1PHUSTR_13   "Level 13: the crypt"
#define GR1PHUSTR_14   "Level 14: genesis"
#define GR1PHUSTR_15   "Level 15: the twilight"
#define GR1PHUSTR_16   "Level 16: the omen"
#define GR1PHUSTR_17   "Level 17: compound"
#define GR1PHUSTR_18   "Level 18: neurosphere"
#define GR1PHUSTR_19   "Level 19: nme"
#define GR1PHUSTR_20   "Level 20: the death domain"

#define GR1PHUSTR_21   "Level 21: slayer"
#define GR1PHUSTR_22   "Level 22: impossible mission"
#define GR1PHUSTR_23   "Level 23: tombstone"
#define GR1PHUSTR_24   "Level 24: the final frontier"
#define GR1PHUSTR_25   "Level 25: the temple of darkness"
#define GR1PHUSTR_26   "Level 26: bunker"
#define GR1PHUSTR_27   "Level 27: anti-christ"
#define GR1PHUSTR_28   "Level 28: the sewers"
#define GR1PHUSTR_29   "Level 29: odyssey of noises"
#define GR1PHUSTR_30   "Level 30: the gateway of hell"

#define GR1PHUSTR_31   "Level 31: cyberden"
#define GR1PHUSTR_32   "Level 32: go 2 it"

#define GR1THUSTR_1    "Level 1: system control"
#define GR1THUSTR_2    "Level 2: human bbq"
#define GR1THUSTR_3    "Level 3: power control"
#define GR1THUSTR_4    "Level 4: wormhole"
#define GR1THUSTR_5    "Level 5: hanger"
#define GR1THUSTR_6    "Level 6: open season"
#define GR1THUSTR_7    "Level 7: prison"
#define GR1THUSTR_8    "Level 8: metal"
#define GR1THUSTR_9    "Level 9: stronghold"
#define GR1THUSTR_10   "Level 10: redemption"
#define GR1THUSTR_11   "Level 11: storage facility"

#define GR1THUSTR_12   "Level 12: crater"
#define GR1THUSTR_13   "Level 13: nukage processing"
#define GR1THUSTR_14   "Level 14: steel works"
#define GR1THUSTR_15   "Level 15: dead zone"
#define GR1THUSTR_16   "Level 16: deepest reaches"
#define GR1THUSTR_17   "Level 17: processing area"
#define GR1THUSTR_18   "Level 18: mill"
#define GR1THUSTR_19   "Level 19: shipping/respawning"
#define GR1THUSTR_20   "Level 20: central processing"

#define GR1THUSTR_21   "Level 21: administration center"
#define GR1THUSTR_22   "Level 22: habitat"
#define GR1THUSTR_23   "Level 23: lunar mining project"
#define GR1THUSTR_24   "Level 24: quarry"
#define GR1THUSTR_25   "Level 25: baron's den"
#define GR1THUSTR_26   "Level 26: ballistyx"
#define GR1THUSTR_27   "Level 27: mount pain"
#define GR1THUSTR_28   "Level 28: heck"
#define GR1THUSTR_29   "Level 29: river styx"
#define GR1THUSTR_30   "Level 30: last call"

#define GR1THUSTR_31   "Level 31: pharaoh"
#define GR1THUSTR_32   "Level 32: caribbean"

#define GR1HUSTR_CHATMACRO1      "Ich bin fertig Alter!"
#define GR1HUSTR_CHATMACRO2      "Ich bin OK."
#define GR1HUSTR_CHATMACRO3      "Ich seh net grad gut aus!"
#define GR1HUSTR_CHATMACRO4      "Hilfe!"
#define GR1HUSTR_CHATMACRO5      "Du Hirsch!"
#define GR1HUSTR_CHATMACRO6      "Naechstes mal, Kleiner..."
#define GR1HUSTR_CHATMACRO7      "Komm her!"
#define GR1HUSTR_CHATMACRO8      "Werd's mir ansehen."
#define GR1HUSTR_CHATMACRO9      "Ja"
#define GR1HUSTR_CHATMACRO0      "Nein"

#define GR1HUSTR_TALKTOSELF1     "Du murmelst zu dirselbst"
#define GR1HUSTR_TALKTOSELF2     "Wer ist da?"
#define GR1HUSTR_TALKTOSELF3     "Du erschrickst dich"
#define GR1HUSTR_TALKTOSELF4     "Du beginnst zu toben"
#define GR1HUSTR_TALKTOSELF5     "Du hast es verloren..."

#define GR1HUSTR_MESSAGESENT     "[Nachricht gesendet]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define GR1HUSTR_PLRGREEN        "Green: "
#define GR1HUSTR_PLRINDIGO       "Indigo: "
#define GR1HUSTR_PLRBROWN        "Brown: "
#define GR1HUSTR_PLRRED          "Red: "
#define GR1HUSTR_PLRGOLD	"Gold: "
#define GR1HUSTR_PLRBLUE	"Blue: "
#define GR1HUSTR_PLRDKBLUE	"DkBlue: "
#define GR1HUSTR_PLRPINK	"Pink: "

#define GR1HUSTR_KEYGREEN        "g"
#define GR1HUSTR_KEYINDIGO       "i"
#define GR1HUSTR_KEYBROWN        "b"
#define GR1HUSTR_KEYRED          "r"

//
//        AM_map.C
//

#define GR1AMSTR_FOLLOWON        "Verfolger Mode AN"
#define GR1AMSTR_FOLLOWOFF       "Verfolger Mode AUS"

#define GR1AMSTR_GRIDON          "Gitter AN"
#define GR1AMSTR_GRIDOFF         "Gitter AUS"

#define GR1AMSTR_MARKEDSPOT      "Stelle markiert"
#define GR1AMSTR_MARKSCLEARED    "Alle Marker entfernt"

//
//        ST_stuff.C
//

#define GR1STSTR_MUS             "Musikwechsel"
#define GR1STSTR_NOMUS           "FLASCHE AUSWAHL"
#define GR1STSTR_DQDON           "Unverwundbarkeit An"
#define GR1STSTR_DQDOFF          "Unverwundbarkeit Aus"

#define GR1STSTR_KFAADDED        "Volle Munition erhalten"
#define GR1STSTR_FAADDED         "Munition (keine Karten) erhalten"

#define GR1STSTR_NCON            "No Clipping Mode ON"
#define GR1STSTR_NCOFF           "No Clipping Mode OFF"

   
/*@ kept V,S,I,R,A,L           inVuln        Str     Inviso     Rad             Allmap    Lite-amp */
#define GR1STSTR_BEHOLD          "unVerwundbar, Str, unsIchtbar, stRahlenschutz , kArte oder Licht"

#define GR1STSTR_BEHOLDX         "Power-Up genommen"

#define GR1STSTR_CHOPPERS        "... Macht nix - GM"
#define GR1STSTR_CLEV            "Wechsle Level..."

//
//        F_Finale.C
//
#define GR1E1TEXT \
"Nun, da Sie die ganzen Schweine\n"\
"erledigt und die MondBasis gereinigt\n"\
"haben, wo ist der verdammte Gewinn? Wo\n"\
"ist die Belohnung, und das Ticket nach\n"\
"Hause? Was zum Teufel ist das? So soll\n"\
"es nicht enden!\n"\
"\n" \
"Es stinkt wie verrottetes Fleisch,\n"\
"sieht aber aus wie die Lost Deimons\n"\
"Base. Sie stecken auf den Shores of Hell\n"\
"fest. Es gibt nur einen Ausweg - Durch!\n"\
"\n"\
"Um das DOOM Erlebnis fortzusetzen,\n"\
"spielen Sie The Shores of Hell und die\n"\
"groáartige Folge, Inferno!\n"


#define GR1E2TEXT \
"Sie haben es vollbracht! Der riesige\n"\
"Cyber Demon Lord, der die Lost Deimos\n"\
"MondBasis regiert hat wurde vernichtet!\n"\
"Sie triumphieren! Aber... wo sind\n"\
"Sie? Sie erklimmen den Rand des Mondes\n"\
"und blicken herunter, um die schreck-\n"\
"liche Wahrheit zu sehen.\n"\
"\n"\
"Monster nehmen alles ein!\n"\
"Noch niemals hat jemand die Hoelle\n"\
"wieder verlassen, aber Sie werden es\n"\
"den Bastarden zeigen! So schnell wie\n"\
"es geht klettern Sie auf die Oberflaeche"\
"der Hoelle.\n"\
"\n" \
"Nun liegt es am letzten Kapitel von\n"\
"DOOM -- Inferno."


#define GR1E3TEXT \
"Der verhaáte Spinnen Daemon,\n"\
"der die ganze Invasion auf die Erde\n"\
"angefuehrt, und so viel Tod und Zerstoerung"\
"zu verantworten hat, ist endlich tot.\n"\
"\n"\
"Sie betreten eine versteckte Tuer, die\n"\
"sich vor Ihnen oeffnet. Sie haben\n"\
"gesiegt. In der Ferne koennen Sie\n"\
"die gruenen Felder der Erde erkennen.\n"\
"Sie treten den Heimweg an...\n"\
"Endlich zuhause.\n" \
"\n"\
"Sie fragen sich, was sich auf der\n"\
"Erde wohl alles veraendert hat,\n"\
"waehrend Sie das Uebel bekaempften.\n"\
"Es ist gut, das nichts mit ihnen\n"\
"durch die Tuer kommen konnte ..."


#define GR1E4TEXT \
"Der Spinnen Daemon muá seine Legionen\n"\
"der Hoelle vor ihrer letzten Konfron-"\
"tation ausgesendet haben.\n"\
"Aber Sie liefen weiter, und brachten\n"\
"ewige Verdammnis und Leiden ueber die\n"\
"Menschen, im Gesicht des Boesen...\n"\
"\n"\
"Nebenbei, jemand wird fuer das, was\n"\
"ihrem Hasen Daisy passiert ist bueáen!\n"\
"\n"\
"Aber nun beeilen Sie sich lieber, und\n"\
"baendigen die Horde Daemonen, die in\n"\
"unseren Staedten Amok laufen...\n"\
"\n"\
"Naechster Stop, Hell on Earth!"


// after level 6, put this:

#define GR1C1TEXT \
"SIE SIND TIEF IN DEN BEFALLENEN FLUG\n" \
"HAFEN EINGEDRUNGEN. ABER ETWAS IST FALSCH.\n" \
"DIE MONSTER HABEN IHRE EIGENE REALITAET\n" \
"MITGEBRACHT, UND DIE TECHNOLOGIE DES\n" \
"FLUGHAFENS WIRD VON IHNEN UNTERBUNDEN.\n" \
"\n"\
"IN DER FERNE SEHEN SIE EINEN VORPOSTEN\n" \
"DER HOELLE. WENN SIE ZU IHM GELANGEN,\n" \
"KOENNEN SIE IN DAS HERZ DES FLUGHAFENS\n" \
"EINDRINGEN, UND DEN KONTROLLHEBEL,\n" \
"DER DIE ERDBEVOELKERUNG ALS GEISELN\n" \
"FESTHAELLT, DEAKTIVIEREN..."

// After level 11, put this:

#define GR1C2TEXT \
"SIE HABEN GEWONNEN! IHR SIEG ERMOEGLICHTE\n" \
"ES DEN MENSCHEN, DEM ALPTRAUM ZU ENT-\n"\
"KOMMEN. NUN SIND SIE DER EINZIGE MENSCH\n"\
"AUF DEM PLANTETEN...\n"\
"SCHRECKLICHE MUTATIONEN, ALIENS,\n"\
"UND UNTOTE WESEN SIND IHRE EINZIGEN\n"\
"NACHBARN. SIE LEHNEN SICH ZURUECK, UND\n"\
"WARTEN AUF IHREN TOD. ALLE GERETTET...\n"\
"\n"\
"DOCH DIE ERDKONTROLLE BEAMT EINE\n"\
"NACHRICHT VOM ALL: \"SENSOREN HABEN DIE\n"\
"QUELLE DER ALIEN INVASION ENTDECKT!\n"\
"WENN SIE DORT HINGEHEN, SIND SIE\n"\
"VIELLEICHT IN DER LAGE, DIESEN ALPTRAUM\n"\
"ZU STOPPEN. DAS HERZ DES GANZEN IST\n"\
"IM FLUGHAFEN.\" LANGSAM ERHEBEN SIE SICH,\n"\
"UND MACHEN SICH WIEDER AUF DEN WEG."


// After level 20, put this:

#define GR1C3TEXT \
"SIE SIND IM HERZEN DER STADT, UMZINGELT\n"\
"VON DEN LEICHNAMEN IHRER FEINDE.\n"\
"SIE SEHEN KEINEN WEG DEN EINGANG AUF\n"\
"DIESER SEITE ZU BLOCKIEREN, ALSO BETRETEN\n"\
"SIE DAS TOR.\n"\
"\n"\
"ES MUSS EINEN WEG GEBEN, DAS TOR VON\n"\
"DER ANDEREN SEITE ZU VERSCHLIESSEN.\n"\
"SIE WISSEN: DER FEIND WARTET SCHON..."


// After level 29, put this:

#define GR1C4TEXT \
"DIE SCHRECKLICHE VISAGE DES GROESSTEN\n"\
"MONSTERS DAS SIE JE GESEHEN HABEN\n"\
"KRIECHT VOR IHNEN, NACHDEM SIE SEIN\n"\
"GEHIRN MIT RAKETEN VOLLGEPUMPT HABEN.\n"\
"NOCH EINMAL ZUCKT ES ZUSAMMEN, DANN\n"\
"VERSCHWINDET ES, MIT ALL DEM BOESEN,\n"\
"DAS ES EINMAL KONTROLLIERT HAT...\n"\
"\n"\
"SIE HABEN ES GESCHAFFT. DIE ERDE IST\n"\
"GERETTET. DIE HOELLE IST ZERSTOERT.\n"\
"SIE FRAGEN SICH, WO DIE GANZEN VERBRECHER\n"\
"HINKOMMEN,  WENN SIE STERBEN...\n"\
"SIE WISCHEN SICH DEN SCHWEISS VON DER\n"\
"STIRN. EIN LANGER RUECKWEG WARTET AUF\n"\
"SIE, ABER DAS WAR ES WERT...\n"



// Before level 31, put this:

#define GR1C5TEXT \
"GRATULATION! SIE HABEN DEN SECRET LEVEL\n"\
"GEFUNDEN! SIEHT SO AUS, ALS WAERE ER\n"\
"VON MENSCHEN ERBAUT WORDEN. SIE FRAGEN\n"\
"SICH, WELCHE AUSGEBURTEN DER HOELLE SIE\n"\
"HIER ERWARTEN."


// Before level 32, put this:

#define GR1C6TEXT \
"GRATULATION! SIE HABEN DEN SUPER\n"\
"SECRET LEVEL GEFUNDEN! SIE HAETTEN\n"\
"IHN BESSER AUSLASSEN SOLLEN!\n"



// after map 06     

#define GR1P1TEXT  \
"Sie triumphieren ueber dem dampfenden\n"\
"Kadaver des Waechters. Mit seinem Tod haben\n"\
"Sie den Beschleuniger der stinkenden Klaue\n"\
"der Hoelle! Sie atmen auf und schauen sich\n"\
"etwas in dem Raum um. Verdammt! Wenigstens\n"\
"ein funktionierender Prototyp... aber Sie\n"\
"koennen keinen Sehen. Die haben ihn...\n"\
"\n"\
"Sie muessen den Prototyp finden, oder alles\n"\
"ware umsonst. Laufen Sie weiter, kaempfen\n"\
"Sie weiter, toeten Sie weiter...\n"\
"Oh ja, ... und leben Sie weiter."


// after map 11

#define GR1P2TEXT \
"Nicht einmal das Arch-Vile Labyrinth\n"\
"konnte Sie stoppen. Oh, Sie haben den\n"\
"Prototyp Beschleuniger vergessen, der\n"\
"bald fuer immer deaktiviert sein wird.\n"\
"\n"\
"Sie sind gut in diesem Job."


// after map 20

#define GR1P3TEXT \
"Sie haben sich den Weg freigeschlagen,\n"\
"direkt ins Nest, in das Herz des Boesen.\n"\
"Zeit fuer eine Such-und-Toete-den-Torwaechter\n"\
"Mission, dessen ekelhafter Abkoemmling\n"\
"zur Erde stuerzt. Yep, er ist gemein.\n"\
"Aber Sie wissen wer schlimmer ist!\n"\
"\n"\
"Sie grinsen feindseelig, und machen sich\n"\
"bereit, dem Hoellenbastard ihre eigene\n"\
"Hoelle zu zeigen!"

// after map 30

#define GR1P4TEXT \
"Des Torwaechters Gesicht ist ueber den\n"\
"ganzen Platz verteilt. Als sein Leichnahm\n"\
"zerbricht, formt sich ein umgekehrtes\n"\
"Tor ueber dem letzten Prototyp\n"\
"Beschleuniger, ganz zu schweigen von den\n"\
"paar uebriggebliebenen Daemonen. Sie sind\n"\
"am Ende. Die Hoelle ist zu den boesen\n"\
"Menschen zurueckgekehrt. Denken Sie\n"\
"daran, ihren Enkelkindern zu sagen, dass\n"\
"sie einen Raketenwerfer in ihren Kaffe tun\n"\
"Wenn Sie zur Hoelle fahren werden Sie ihn\n"\
"brauchen, um endgueltig aufzuraeumen..." 

// before map 31

#define GR1P5TEXT \
"Sie haben den zweitschwersten Level den\n"\
"es gibt gefunden. Wenn Sie kein gesichertes\n"\
"Spiel haben, machen Sie sich bereit zu\n"\
"sterben. Nur fuer die Elite!"

// before map 32

#define GR1P6TEXT \
"Sie haben sich schon gewundert, welcher\n"\
"wohl der schwerste Level ist. Jetzt wissen\n"\
"Sie's. Niemand kommt hier lebend raus!"


#define GR1T1TEXT \
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


#define GR1T2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."


#define GR1T3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define GR1T4TEXT \
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


#define GR1T5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."


#define GR1T6TEXT \
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
#define GR1CC_ZOMBIE   "ZOMBIEMAN"
#define GR1CC_SHOTGUN  "SHOTGUN GUY"
#define GR1CC_HEAVY    "HEAVY WEAPON DUDE"
#define GR1CC_IMP      "IMP"
#define GR1CC_DEMON    "DEMON"
#define GR1CC_LOST     "LOST SOUL"
#define GR1CC_CACO     "CACODEMON"
#define GR1CC_HELL     "HELL KNIGHT"
#define GR1CC_BARON    "BARON OF HELL"
#define GR1CC_ARACH    "ARACHNOTRON"
#define GR1CC_PAIN     "PAIN ELEMENTAL"
#define GR1CC_REVEN    "REVENANT"
#define GR1CC_MANCU    "MANCUBUS"
#define GR1CC_ARCH     "ARCH-VILE"
#define GR1CC_SPIDER   "THE SPIDER MASTERMIND"
#define GR1CC_CYBER    "THE CYBERDEMON"
#define GR1CC_HERO     "UNSER HELD"


//new strings
#define GR1V_INITSTR    "V_Init: allocate screens.\n"
#define GR1M_LDEFSTR    "M_LoadDefaults: Load system defaults.\n"
#define GR1Z_INITSTR    "Z_Init: Init zone memory allocation daemon. \n"
#define GR1W_INITSTR    "W_Init: Init WADfiles.\n"
#define GR1M_INITSTR    "M_Init: Init miscellaneous info.\n"
#define GR1R_INITSTR    "R_Init: Init DOOM refresh daemon - "
#define GR1P_INITSTR    "\nP_Init: Init Playloop state.\n"
#define GR1I_INITSTR    "I_Init: Setting up machine state.\n"
#define GR1D_CHKNETSTR  "D_CheckNetGame: Checking network game status.\n"
#define GR1S_INITSTR    "S_Init: Setting up sound.\n"
#define GR1HU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define GR1ST_INITSTR   "ST_Init: Init status bar.\n"
#define GR1NETLISTEN    "listening for network start info...\n"
#define GR1NETSEND      "sending network start info...\n"
#define GR1TURBOSCLSTR  "turbo scale: %i%%\n"
#define GR1ISTURBOSTR   "%s is turbo!"

#define GR1MODMSG\
	    "===========================================================================\n"\
	    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"\
	    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"\
	    "        You will not receive technical support for modified games.\n"\
	    "                      press enter to continue\n"\
	    "===========================================================================\n"
#define GR1NOSWMSG\
	    "===========================================================================\n"\
	    "             This version is NOT SHAREWARE, do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define GR1NOSWMSG2\
	    "===========================================================================\n"\
	    "                            Do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define GR1SWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define GR1UDOOMSTART\
 "The Ultimate DOOM"
#define GR1SHAREDOOMSTART\
 "DOOM (Shareware)"
#define GR1REGDOOMSTART\
 "DOOM (Registered)"
#define GR1DOOM2START\
 "DOOM 2: Hell on Earth"
#define GR1PLUTSTART\
 "DOOM 2: The Plutonia Experiment"
#define GR1TNTSTART\
 "DOOM 2: TNT - Evilution"
#define GR1PUBDOOMSTART\
 "DOOM"
#define GR1DOSDOOMSTART\
 "DOSDoom"



//these are unused.  They're just here for Deh compatibility
#define GR1I_SDPMI   "I_StartupDPMI\n"
#define GR1I_SMOUSE  "I_StartupMouse\n"
#define GR1I_SJOY    "I_StartupJoystick\n"
#define GR1I_SKEY    "I_StartupKeyboard\n"
#define GR1I_SSOUND  "I_StartupSound\n"
#define GR1I_STIMER  "I_StartupTimer()\n"
#define GR1MOUSEOFF  "Mouse: not present\n"
#define GR1MOUSEON   "Mouse: detected\n"
#define GR1DPMI1     "DPMI memory: 0x%x"
#define GR1DPMI2     ", 0x%x allocated for zone\n"
#define GR1COMMVER   "\tcommercial version.\n"
#define GR1SHAREVER  "\tshareware version.\n"
#define GR1REGVER    "\tregistered version.\n"


#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
