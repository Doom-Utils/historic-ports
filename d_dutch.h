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
//	Dutch language support.
//
// ----------------------------------------------------------------------------
// Translation by Arno Slagboom - eMail "Arnos@freemail.nl"
// ----------------------------------------------------------------------------

#ifndef __D_DUTCH__
#define __D_DUTCH__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define DUTD_DEVSTR	"Ontwikkelingsmodus AAN.\n"
#define DUTD_CDROM	"CD-ROM Versie: default.cfg uit c:\\doomdata\n"

//
//	M_Menu.C
//
#define DUTPRESSKEY 	"druk op een toets."
#define DUTPRESSYN 	"toets y of n."
#define DUTQUITMSG	"weet je zeker dat je dit\ngeweldige spel wilt stoppen?"
#define DUTLOADNET 	"je kunt niet laden in netwerkmodus!\n\ndruk op een toets."
#define DUTQLOADNET	"je kunt niet versneld laden in netwerkmodus!\n\ndruk op een toets."
#define DUTQSAVESPOT	"je hebt nog geen quicksave veld gekozen!\n\ndruk op een toets."
#define DUTSAVEDEAD 	"je kunt niet opslaan als je niet speelt!\n\ndruk op een toets."
#define DUTQSPROMPT 	"versneld opslaan over je spel genaamd\n\n'%s'?\n\ntoets y of n."
#define DUTQLPROMPT	"wil je het spel\n\n'%s' versneld laden?\n\ntoets y of n."

#define DUTNEWGAME	\
"je kunt geen nieuw spel\n"\
"in netwerkmodus beginnen.\n\ndruk op een toets."

#define DUTNIGHTMARE	\
"weet je het zeker? dit skill level\n"\
"is zelfs niet een beetje fair.\n\npress y or n."

#define DUTSWSTRING	\
"dit is de shareware versie van doom.\n\n"\
"je moet de hele trilogie bestellen.\n\ndruk op een toets."

#define DUTMSGOFF	"Boodschappen UIT"
#define DUTMSGON		"Boodschappen AAN"
#define DUTNETEND	"je kunt geen netwerkspel beeindigen!\n\ndruk op een toets."
#define DUTENDGAME	"weet je zeker dat je het spel wilt beeindigen?\n\ntoets y of n."

#define DUTDOSY		"(toets y om te stoppen)"

#define DUTDETAILHI	"Veel detail"
#define DUTDETAILLO	"Weinig detail"
#define DUTGAMMALVL0	"Gamma correctie UIT"
#define DUTGAMMALVL1	"Gamma correctie niveau 1"
#define DUTGAMMALVL2	"Gamma correctie niveau 2"
#define DUTGAMMALVL3	"Gamma correctie niveau 3"
#define DUTGAMMALVL4	"Gamma correctie niveau 4"
#define DUTEMPTYSTRING	"Leeg veld"

//
//	P_inter.C
//
#define DUTGOTARMOR	"Armor opgeraapt."
#define DUTGOTMEGA	"MegaArmor opgeraapt!"
#define DUTGOTHTHBONUS	"Health bonus opgeraapt."
#define DUTGOTARMBONUS	"Armor bonus opgeraapt."
#define DUTGOTSTIM	"Stimpack opgeraapt."
#define DUTGOTMEDINEED	"Medikit opgeraapt die je hard nodig had!"
#define DUTGOTMEDIKIT	"Medikit opgeraapt."
#define DUTGOTSUPER	"Supercharge!"

#define DUTGOTBLUECARD	"Blauwe sleutelkaart opgeraapt."
#define DUTGOTYELWCARD	"Gele sleutelkaart opgeraapt."
#define DUTGOTREDCARD	"Rode sleutelkaart opgeraapt."
#define DUTGOTBLUESKUL	"Blauwe doodshoofdsleutel opgeraapt."
#define DUTGOTYELWSKUL	"Gele doodshoofdsleutel opgeraapt."
#define DUTGOTREDSKULL	"Rode doodshoofdsleutel opgeraapt."

#define DUTGOTINVUL	"Onkwetsbaarheid!"
#define DUTGOTBERSERK	"Razernij!"
#define DUTGOTINVIS	"Gedeeltelijke onzichtbaarheid"
#define DUTGOTSUIT	"Beschermingspak"
#define DUTGOTMAP	"Computerkaart"
#define DUTGOTVISOR	"Nachtkijker"
#define DUTGOTMSPHERE	"MegaSphere!"

#define DUTGOTCLIP	"Kogels opgeraapt."
#define DUTGOTCLIPBOX	"Een doos met kogels opgeraapt."
#define DUTGOTROCKET	"Een raket opgeraapt."
#define DUTGOTROCKBOX	"Een doos met raketten opgeraapt."
#define DUTGOTCELL	"Een energiecel opgeraapt."
#define DUTGOTCELLBOX	"Een pakket energiecellen opgeraapt."
#define DUTGOTSHELLS	"4 geweerpatronen opgeraapt."
#define DUTGOTSHELLBOX	"Een doos met geweerpatronen opgeraapt."
#define DUTGOTBACKPACK	"Een rugzak vol met munitie opgeraapt!"

#define DUTGOTBFG9000	"Je hebt de BFG9000!  Tjakka!"
#define DUTGOTCHAINGUN	"Je hebt het machinegeweer!"
#define DUTGOTCHAINSAW	"Je hebt de kettingzaag!  Ga vlees zoeken!"
#define DUTGOTLAUNCHER	"Je hebt de rakettenwerper!"
#define DUTGOTPLASMA	"Je hebt het plasma kanon!"
#define DUTGOTSHOTGUN	"Je hebt het geweer!"
#define DUTGOTSHOTGUN2	"Je hebt het supergeweer!"

//
// P_Doors.C
//
#define DUTPD_BLUEO	"Je hebt een blauwe sleutel nodig."
#define DUTPD_REDO	"Je hebt een rode sleutel nodig."
#define DUTPD_YELLOWO	"Je hebt een gele sleutel nodig."
#define DUTPD_BLUEK	"Je hebt een blauwe sleutel nodig."
#define DUTPD_REDK	"Je hebt een rode sleutel nodig."
#define DUTPD_YELLOWK	"Je hebt een gele sleutel nodig."

//
//	G_game.C
//
#define DUTGGSAVED	"spel opgeslagen."

//
//	HU_stuff.C
//
#define DUTHUSTR_MSGU	"[Boodschap niet verzonden]"

#define DUTHUSTR_E1M1	"E1M1: Hangar"
#define DUTHUSTR_E1M2	"E1M2: Nuclear Plant"
#define DUTHUSTR_E1M3	"E1M3: Toxin Refinery"
#define DUTHUSTR_E1M4	"E1M4: Command Control"
#define DUTHUSTR_E1M5	"E1M5: Phobos Lab"
#define DUTHUSTR_E1M6	"E1M6: Central Processing"
#define DUTHUSTR_E1M7	"E1M7: Computer Station"
#define DUTHUSTR_E1M8	"E1M8: Phobos Anomaly"
#define DUTHUSTR_E1M9	"E1M9: Military Base"

#define DUTHUSTR_E2M1	"E2M1: Deimos Anomaly"
#define DUTHUSTR_E2M2	"E2M2: Containment Area"
#define DUTHUSTR_E2M3	"E2M3: Refinery"
#define DUTHUSTR_E2M4	"E2M4: Deimos Lab"
#define DUTHUSTR_E2M5	"E2M5: Command Center"
#define DUTHUSTR_E2M6	"E2M6: Halls of the Damned"
#define DUTHUSTR_E2M7	"E2M7: Spawning Vats"
#define DUTHUSTR_E2M8	"E2M8: Tower of Babel"
#define DUTHUSTR_E2M9	"E2M9: Fortress of Mystery"

#define DUTHUSTR_E3M1	"E3M1: Hell Keep"
#define DUTHUSTR_E3M2	"E3M2: Slough of Despair"
#define DUTHUSTR_E3M3	"E3M3: Pandemonium"
#define DUTHUSTR_E3M4	"E3M4: House of Pain"
#define DUTHUSTR_E3M5	"E3M5: Unholy Cathedral"
#define DUTHUSTR_E3M6	"E3M6: Mt. Erebus"
#define DUTHUSTR_E3M7	"E3M7: Limbo"
#define DUTHUSTR_E3M8	"E3M8: Dis"
#define DUTHUSTR_E3M9	"E3M9: Warrens"

#define DUTHUSTR_E4M1	"E4M1: Hell Beneath"
#define DUTHUSTR_E4M2	"E4M2: Perfect Hatred"
#define DUTHUSTR_E4M3	"E4M3: Sever The Wicked"
#define DUTHUSTR_E4M4	"E4M4: Unruly Evil"
#define DUTHUSTR_E4M5	"E4M5: They Will Repent"
#define DUTHUSTR_E4M6	"E4M6: Against Thee Wickedly"
#define DUTHUSTR_E4M7	"E4M7: And Hell Followed"
#define DUTHUSTR_E4M8	"E4M8: Unto The Cruel"
#define DUTHUSTR_E4M9	"E4M9: Fear"

#define DUTHUSTR_1	"level 1: entryway"
#define DUTHUSTR_2	"level 2: underhalls"
#define DUTHUSTR_3	"level 3: the gantlet"
#define DUTHUSTR_4	"level 4: the focus"
#define DUTHUSTR_5	"level 5: the waste tunnels"
#define DUTHUSTR_6	"level 6: the crusher"
#define DUTHUSTR_7	"level 7: dead simple"
#define DUTHUSTR_8	"level 8: tricks and traps"
#define DUTHUSTR_9	"level 9: the pit"
#define DUTHUSTR_10	"level 10: refueling base"
#define DUTHUSTR_11	"level 11: 'o' of destruction!"

#define DUTHUSTR_12	"level 12: the factory"
#define DUTHUSTR_13	"level 13: downtown"
#define DUTHUSTR_14	"level 14: the inmost dens"
#define DUTHUSTR_15	"level 15: industrial zone"
#define DUTHUSTR_16	"level 16: suburbs"
#define DUTHUSTR_17	"level 17: tenements"
#define DUTHUSTR_18	"level 18: the courtyard"
#define DUTHUSTR_19	"level 19: the citadel"
#define DUTHUSTR_20	"level 20: gotcha!"

#define DUTHUSTR_21	"level 21: nirvana"
#define DUTHUSTR_22	"level 22: the catacombs"
#define DUTHUSTR_23	"level 23: barrels o' fun"
#define DUTHUSTR_24	"level 24: the chasm"
#define DUTHUSTR_25	"level 25: bloodfalls"
#define DUTHUSTR_26	"level 26: the abandoned mines"
#define DUTHUSTR_27	"level 27: monster condo"
#define DUTHUSTR_28	"level 28: the spirit world"
#define DUTHUSTR_29	"level 29: the living end"
#define DUTHUSTR_30	"level 30: icon of sin"

#define DUTHUSTR_31	"level 31: wolfenstein"
#define DUTHUSTR_32	"level 32: grosse"

#define DUTPHUSTR_1	"level 1: congo"
#define DUTPHUSTR_2	"level 2: well of souls"
#define DUTPHUSTR_3	"level 3: aztec"
#define DUTPHUSTR_4	"level 4: caged"
#define DUTPHUSTR_5	"level 5: ghost town"
#define DUTPHUSTR_6	"level 6: baron's lair"
#define DUTPHUSTR_7	"level 7: caughtyard"
#define DUTPHUSTR_8	"level 8: realm"
#define DUTPHUSTR_9	"level 9: abattoire"
#define DUTPHUSTR_10	"level 10: onslaught"
#define DUTPHUSTR_11	"level 11: hunted"

#define DUTPHUSTR_12	"level 12: speed"
#define DUTPHUSTR_13	"level 13: the crypt"
#define DUTPHUSTR_14	"level 14: genesis"
#define DUTPHUSTR_15	"level 15: the twilight"
#define DUTPHUSTR_16	"level 16: the omen"
#define DUTPHUSTR_17	"level 17: compound"
#define DUTPHUSTR_18	"level 18: neurosphere"
#define DUTPHUSTR_19	"level 19: nme"
#define DUTPHUSTR_20	"level 20: the death domain"

#define DUTPHUSTR_21	"level 21: slayer"
#define DUTPHUSTR_22	"level 22: impossible mission"
#define DUTPHUSTR_23	"level 23: tombstone"
#define DUTPHUSTR_24	"level 24: the final frontier"
#define DUTPHUSTR_25	"level 25: the temple of darkness"
#define DUTPHUSTR_26	"level 26: bunker"
#define DUTPHUSTR_27	"level 27: anti-christ"
#define DUTPHUSTR_28	"level 28: the sewers"
#define DUTPHUSTR_29	"level 29: odyssey of noises"
#define DUTPHUSTR_30	"level 30: the gateway of hell"

#define DUTPHUSTR_31	"level 31: cyberden"
#define DUTPHUSTR_32	"level 32: go 2 it"

#define DUTTHUSTR_1	"level 1: system control"
#define DUTTHUSTR_2	"level 2: human bbq"
#define DUTTHUSTR_3	"level 3: power control"
#define DUTTHUSTR_4	"level 4: wormhole"
#define DUTTHUSTR_5	"level 5: hanger"
#define DUTTHUSTR_6	"level 6: open season"
#define DUTTHUSTR_7	"level 7: prison"
#define DUTTHUSTR_8	"level 8: metal"
#define DUTTHUSTR_9	"level 9: stronghold"
#define DUTTHUSTR_10	"level 10: redemption"
#define DUTTHUSTR_11	"level 11: storage facility"

#define DUTTHUSTR_12	"level 12: crater"
#define DUTTHUSTR_13	"level 13: nukage processing"
#define DUTTHUSTR_14	"level 14: steel works"
#define DUTTHUSTR_15	"level 15: dead zone"
#define DUTTHUSTR_16	"level 16: deepest reaches"
#define DUTTHUSTR_17	"level 17: processing area"
#define DUTTHUSTR_18	"level 18: mill"
#define DUTTHUSTR_19	"level 19: shipping/respawning"
#define DUTTHUSTR_20	"level 20: central processing"

#define DUTTHUSTR_21	"level 21: administration center"
#define DUTTHUSTR_22	"level 22: habitat"
#define DUTTHUSTR_23	"level 23: lunar mining project"
#define DUTTHUSTR_24	"level 24: quarry"
#define DUTTHUSTR_25	"level 25: baron's den"
#define DUTTHUSTR_26	"level 26: ballistyx"
#define DUTTHUSTR_27	"level 27: mount pain"
#define DUTTHUSTR_28	"level 28: heck"
#define DUTTHUSTR_29	"level 29: river styx"
#define DUTTHUSTR_30	"level 30: last call"

#define DUTTHUSTR_31	"level 31: pharaoh"
#define DUTTHUSTR_32	"level 32: caribbean"

#define DUTHUSTR_CHATMACRO1	"Ik ben er helemaal klaar voor!"
#define DUTHUSTR_CHATMACRO2	"Ik ben OK."
#define DUTHUSTR_CHATMACRO3	"Ik zie er niet zo goed uit!"
#define DUTHUSTR_CHATMACRO4	"Help!"
#define DUTHUSTR_CHATMACRO5	"Jij deugt niet!"
#define DUTHUSTR_CHATMACRO6	"Volgende keer, zak stront..."
#define DUTHUSTR_CHATMACRO7	"Kom hier!"
#define DUTHUSTR_CHATMACRO8	"Ik zorg er wel voor."
#define DUTHUSTR_CHATMACRO9	"Ja"
#define DUTHUSTR_CHATMACRO0	"Nee"

#define DUTHUSTR_TALKTOSELF1	"Je praat tegen jezelf"
#define DUTHUSTR_TALKTOSELF2	"Wie is daar?"
#define DUTHUSTR_TALKTOSELF3	"Je maakt jezelf bang"
#define DUTHUSTR_TALKTOSELF4	"Je begint te flippen"
#define DUTHUSTR_TALKTOSELF5	"Je hebt het verloren..."

#define DUTHUSTR_MESSAGESENT	"[Boodschap verzonden]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define DUTHUSTR_PLRGREEN	"Groen: "
#define DUTHUSTR_PLRINDIGO	"Indigo: "
#define DUTHUSTR_PLRBROWN	"Bruin: "
#define DUTHUSTR_PLRRED		"Rood: "
#define DUTHUSTR_PLRGOLD	"Goud: "
#define DUTHUSTR_PLRBLUE	"Blauw: "
#define DUTHUSTR_PLRDKBLUE	"DkBluw: "
#define DUTHUSTR_PLRPINK	"Roze: "

#define DUTHUSTR_KEYGREEN	"g"
#define DUTHUSTR_KEYINDIGO	"i"
#define DUTHUSTR_KEYBROWN "b"
#define DUTHUSTR_KEYRED	"r"

//
//	AM_map.C
//

#define DUTAMSTR_FOLLOWON	"Volgmodus AAN"
#define DUTAMSTR_FOLLOWOFF	"Volgmodus UIT"

#define DUTAMSTR_GRIDON	"Raster AAN"
#define DUTAMSTR_GRIDOFF	"Raster UIT"

#define DUTAMSTR_MARKEDSPOT	"Gemarkeerd punt"
#define DUTAMSTR_MARKSCLEARED	"Alle markeringen gewist"

//
//	ST_stuff.C
//

#define DUTSTSTR_MUS		"Andere muziek"
#define DUTSTSTR_NOMUS		"ONMOGELIJKE SELECTIE"
#define DUTSTSTR_DQDON		"Onkwetsbaarheidsmodus Aan"
#define DUTSTSTR_DQDOFF	"Onkwetsbaarheidsmodus Uit"

#define DUTSTSTR_KFAADDED	"Volledige munitie toegevoegd"
#define DUTSTSTR_FAADDED	"Munitie (geen sleutels) toegevoegd"

#define DUTSTSTR_NCON		"Botsingvrije modus AAN"
#define DUTSTSTR_NCOFF		"Botsingvrije modus UIT"

#define DUTSTSTR_BEHOLD	"leVen, berSerk, onzIchtbaar, bescheRmingspak, kAart, of Licht"
#define DUTSTSTR_BEHOLDX	"Power-up gekregen"

#define DUTSTSTR_CHOPPERS	"...geeft niet - GM"
#define DUTSTSTR_CLEV		"Level veranderen..."

//
//	F_Finale.C
//
#define DUTE1TEXT \
"Nu je de grote slechterikken verslagen\n"\
"en de maanbasis gereinigd hebt, zou je\n"\
"gewonnen moeten hebben, toch? Waar is\n"\
"dan je vette beloning en je ticket naar\n"\
"huis? Wat is dit verdorie? Zo mag het\n"\
"toch niet aflopen!\n"\
"\n" \
"Het stinkt naar rottend vlees, maar\n"\
"lijkt op de verloren Deimos basis. Het\n"\
"lijkt wel of je vastzit op de Shores of\n"\
"Hell. De enige uitweg is er doorheen!\n"\
"\n"\
"Om de Doom ervaring voort te zetten,\n"\
"speel The Shores of Hell en zijn\n"\
"geweldige vervolg Inferno!\n"


#define DUTE2TEXT \
"Het is je gelukt! De vreselijke cyber-\n"\
"demon lord die over de verloren Deimos\n"\
"maanbasis regeerde is geslacht en jij\n"\
"bent de overwinnaar! Maar ... waar ben\n"\
"je? Je klautert naar de rand van de\n"\
"maan en kijkt naar beneden om de ver-\n"\
"schrikkelijke waarheid te aanschouwen.\n" \
"\n"\
"Deimos drijft boven de hel!\n"\
"Nog nooit is het iemand gelukt uit de\n"\
"hel te ontsnappen, maar jij zal ervoor\n"\
"zorgen dat men het zal berouwen ooit van\n"\
"jou gehoord te hebben! Snel daal je af\n"\
"naar het oppervlak van de hel\n"\
"\n" \
"Nu is het tijd voor het laatste hoofd-\n"\
"stuk van DOOM! -- Inferno."


#define DUTE3TEXT \
"De walgelijke spiderdemon die de\n"\
"invasie van de maanbases geleid had\n"\
"en zoveel dood en verderf zaaide\n"\
"zal voor eeuwig die trap onder zijn\n"\
"kont blijven voelen.\n"\
"Een verborgen deur opent zich en\n"\
"jij gaat naar binnen. Je bent te\n"\
"taai voor de hel gebleken en nu is\n"\
"de hel uiteindelijk eerlijk -- achter\n"\
"de deur zul je de groene velden van\n"\
"de aarde weer kunnen zien\n"\
"Eindelijk thuis.\n" \
"Je vraagt je af wat er is gebeurd\n"\
"op aarde terwijl jij het kwaad aan\n"\
"het bestrijden was. Het is maar goed\n"\
"dat geen hellegebroed met je door die\n"\
"deur heeft kunnen komen ..."


#define DUTE4TEXT \
"de spider mastermind moet zijn legioenen\n"\
"van hellegebroed voor zich uit hebben\n"\
"gestuurd voordat je dat verschrikkelijke\n"\
"beest uit de hel uiteindelijk ontmoette.\n"\
"maar jij ging gewoon door en zaaide voor\n"\
"eeuwig dood en verderf onder de horden,\n"\
"zoals het een held betaamt bij iets dat zo\n"\
"slecht is.\n"\
"\n"\
"daarnaast, iemand moest toch boeten voor\n"\
"wat er gebeurd is met daisy, je konijn.\n"\
"\n"\
"maar nu zie je nog meer pijn en ellende\n"\
"opdoemen, want een natie van demonen is\n"\
"onze steden onveilig aan het maken.\n"\
"\n"\
"volgende halte, hell on earth!"


// after level 6, put this:

#define DUTC1TEXT \
"JE BENT DIEP DOORGEDRONGEN IN DE BEZETTE\n" \
"RUIMTEBASIS. MAAR ER KLOPT IETS NIET. DE\n" \
"MONSTERS HEBBEN HUN EIGEN VORM VAN REALI-\n" \
"TEIT MEEGEBRACHT EN DE TECHNOLOGIE VAN DE\n" \
"RUIMTEBASIS IS VAN SLAG GERAAKT DOOR HUN\n" \
"AANWEZIGHEID.\n" \
"\n"\
"VOOR JE ZIE JE EEN VOORPOST VAN DE HEL,\n" \
"EEN ZWAAR BELEGERDE ZONE. ALS JE ER LANGS\n" \
"KAN KOMEN KUN JE DOORDRINGEN TOT HET\n" \
"BEHEKSTE HART VAN DE RUIMTEBASIS EN KUN\n" \
"JE DE SCHAKELAAR VINDEN DIE DE BEVOLKING\n" \
"VAN DE AARDE GEVANGEN HOUDT."

// After level 11, put this:

#define DUTC2TEXT \
"JE HEBT GEWONNEN! JOUW OVERWINNING ZORGT\n" \
"ERVOOR DAT DE MENSHEID KAN EVACUEREN VAN\n"\
"DE AARDE OM DE NACHTMERRIE TE ONTVLUCHTEN.\n"\
"NU BEN JE DE ENIGE OVERGEBLEVEN MENS OP DE\n"\
"PLANEET. MENSETENDE MUTATIES, VLEESETENDE\n"\
"WEZENS EN BOZE GEESTEN ZIJN JE ENIGE BUREN.\n"\
"JE LEUNT ACHTEROVER EN WACHT OP DE DOOD,\n"\
"BLIJ ZIJNDE DAT JE JE SOORTGENOTEN HEBT\n"\
"KUNNEN REDDEN.\n"\
"\n"\
"MAAR DAN ZENDT DE LEIDING VAN DE MENSHEID\n"\
"JE EEN BOODSCHAP VANUIT DE RUIMTE.\n"\
"SENSOREN HEBBEN DE BRON VAN EEN BUITEN-\n"\
"AARDSE INVASIE ONTDEKT. ALS JE DAAR NAAR\n"\
"TOE GAAT KUN JE HEN MISSCHIEN TEGENHOUDEN.\n"\
"HUN BASIS IS IN HET HART VAN JE EIGEN\n"\
"WOONPLAATS, NIET VER VAN DE RUIMTEBASIS.\n"\
"LANGZAAM EN MET PIJN STA JE OP EN KEERT\n"\
"TERUG NAAR HET STRIJDPERK."


// After level 20, put this:

#define DUTC3TEXT \
"JE BENT IN HET CORRUPTE HART VAN DE STAD,\n"\
"OMRINGD DOOR DE LIJKEN VAN JE VIJANDEN.\n"\
"JE WEET GEEN MANIER OM DE INGANG VAN DE\n"\
"WEZENS AAN DEZE KANT TE VERNIETIGEN, DUS\n"\
"JE BALT JE VUISTEN EN STORMT NAAR BINNEN.\n"\
"\n"\
"ER MOET EEN MANIER ZIJN OM HET AAN DE\n"\
"ANDERE KANT TE SLUITEN. WAT KAN JOU HET\n"\
"SCHELEN ALS JE DOOR DE HEL MOET OM ER\n"\
"TE KOMEN?"


// After level 29, put this:

#define DUTC4TEXT \
"DE ANGSTAANJAGENDE AANBLIK VAN HET\n"\
"GROOTSTE MONSTER DAT JE OOIT HEBT GEZIEN\n"\
"BROKKELT LANGZAAM AF, NADAT JIJ JE\n"\
"RAKETTEN IN ZIJN BLOOTGELEGDE BREIN HEBT\n"\
"GEPOMPT. HET MONSTER SCHROMPELT INEEN EN\n"\
"STERFT, TERWIJL ZIJN AFVALLENDE LEDEMATEN\n"\
"KILOMETERS AARDOPPERVLAK VERWOESTEN.\n"\
"\n"\
"HET IS JE GELUKT. DE INVASIE IS VOORBIJ.\n"\
"DE AARDE IS GERED. DE HEL IS EEN RUINE.\n"\
"JE VRAAGT JE AF WAAR SLECHTERIKKEN NU\n"\
"NAAR TOE ZULLEN GAAN NA HUN DOOD.\n"\
"HET ZWEET VAN JE VOORHOOFD WISSEND BEGIN\n"\
"JE AAN DE LANGE REIS NAAR HUIS. HET\n"\
"WEER OPBOUWEN VAN DE AARDE MOET VEEL\n"\
"LEUKER ZIJN DAN HET AFBREKEN WAS."


// Before level 31, put this:

#define DUTC5TEXT \
"GEFELICITEERD, JE HEBT HET GEHEIME\n"\
"LEVEL GEVONDEN! HET LIJKT WEL OF HET\n"\
"GEBOUWD IS DOOR MENSEN IN PLAATS VAN\n"\
"DEMONEN. WIE ZULLEN DE BEWONERS VAN\n"\
"DEZE UITHOEK VAN DE HEL ZIJN?"


// Before level 32, put this:

#define DUTC6TEXT \
"GEFELICITEERD, JE HEBT HET SUPER\n"\
"GEHEIME LEVEL GEVONDEN!  HIER KAN\n"\
"JE MAAR BETER DOORHEEN RAZEN!\n"


// after map 06	

#define DUTP1TEXT  \
"Met plezier kijk je naar het nastomende\n"\
"lijk van de poortwachter. Met zijn dood\n"\
"heb je de kwantumversneller uit de\n"\
"stinkende klauwen van de hel gered. Je\n"\
"ontspant je en kijkt de kamer eens rond.\n"\
"Verdorie, er zou toch minstens een werkend\n"\
"prototype moeten zijn, maar je ziet hem\n"\
"niet. De demonen zullen hem meegenomen\n"\
"hebben.\n"\
"Je moet het prototype vinden, anders\n"\
"zullen al je worstelingen voor niets zijn\n"\
"geweest. Je moet doorgaan, blijven vechten,\n"\
"blijven doden. Oh ja, en blijven leven."


// after map 11

#define DUTP2TEXT \
"Zelfs de dodelijke Archvile kon jou niet\n"\
"tegenhouden, en je hebt het prototype van\n"\
"de kwantumversneller gevonden, welke snel\n"\
"efficient en permanent gedeactiveerd zal\n"\
"worden.\n"\
"\n"\
"Hier ben je goed in..."


// after map 20

#define DUTP3TEXT \
"Je hebt je een weg geslagen naar het hart\n"\
"van de bijenkorf van demonen.  Tijd voor\n"\
"een zoek-en-vernietig missie, gericht tegen\n"\
"de portier, wiens verdorven kroost zich over\n"\
"de aarde verspreidt.  O wat is hij slecht.\n"\
"Maar jij weet wie er erger is!\n"\
"\n"\
"Kwaadaardig grijnzend controleer je je\n"\
"wapens en maakt je op om die rotzak een\n"\
"beetje hel van jezelf toe te dienen!"

// after map 30

#define DUTP4TEXT \
"Het boosaardige gezicht van de portier is\n"\
"over de hele kamer uiteengespat. Wanneer\n"\
"zijn aan flarden gescheurde lichaam\n"\
"uiteenvalt, vormt zich een omgekeerde\n"\
"poort, welke het laatste prototype en niet\n"\
"te vergeten de laatste overgebleven demonen\n"\
"opslokt. Je bent klaar. De hel is verder-\n"\
"gegaan met het martelen van slechte dode\n"\
"in plaats van goede levende mensen. Vergeet\n"\
"niet tegen je kleinkinderen te zeggen dat\n"\
"ze een rakettenwerper in je grafkist moeten\n"\
"leggen als je doodgaat. Je zult het nodig\n"\
"hebben voor een laatste grote schoonmaak."

// before map 31

#define DUTP5TEXT \
"Je hebt het op een na moeilijkste level dat\n"\
"we hebben gevonden. Hopelijk heb je kort\n"\
"geleden nog opgeslagen.  Zo niet, wees dan\n"\
"voorbereid om te sterven. Dit is alleen voor\n"\
"meester mariniers."

// before map 32

#define DUTP6TEXT \
"Je zult je wel afgevraagd hebben wat dan\n"\
"het moeilijkste level was dat we voor je in\n"\
"petto hadden. Je zult er snel achterkomen\n"\
"Niemand komt hier levend uit."


#define DUTT1TEXT \
"Je hebt je een weg gebaand uit de belegerde\n"\
"experimentele laboratoria. Het lijkt wel\n"\
"of de UAC weer eens gestikt is. Met hun hoge\n"\
"omzet moet het moeilijk zijn voor de oude\n"\
"UAC een bedrijfslevensverzekering te kopen.\n"\
"\n"\
"Voor je ligt het militaire complex, vol\n"\
"met ziekelijke gruwelen die niet kunnen\n"\
"wachten om hun tanden in jou te zetten.\n"\
"Als je geluk hebt ligt er nog wat munitie\n"\
"in het complex verspreid."


#define DUTT2TEXT \
"Je hoort in de verte zware machines draaien.\n"\
"Je hoopt dat ze geen nieuw hellegebroed\n"\
"voortbrengen, maar jij staat klaar om een\n"\
"hele horde uit te roeien als het moet.\n"\
"Misschien bereiden ze wel een bloedbad voor,\n"\
"maar jij voelt je zo sterk als tweeduizend\n"\
"maniakken, verpakt in een woeste moorde-\n"\
"naar.\n"\
"\n"\
"Je bent niet van plan om snel neer te gaan."


#define DUTT3TEXT \
"De grote opening voor je ziet er verdraaid\n"\
"bekend uit. Het ruikt ook bekend, als\n"\
"gefrituurde uitwerpselen. Je hebt deze\n"\
"plek nooit fijn gevonden en je weet zeker\n"\
"dat je het nu ook niet fijn vindt.\n"\
"Hoe langer je er over nadenkt, des te\n"\
"bozer je wordt. Terwijl je je geweer optilt,\n"\
"verschijnt er een boosaardige grijns op je\n"\
"gezicht. Het wordt tijd dat men bang voor\n"\
"je wordt."
                                            
#define DUTT4TEXT \
"Plotseling is alles stil, van de ene horizon\n"\
"tot de andere. De kwellende echo van de hel\n"\
"ebt weg, de lucht uit de nachtmerrie wordt\n"\
"blauw, de stapels monsterlijken beginnen\n"\
"te verdampen, samen met de kwaadaardige\n"\
"stank die in de lucht hing. Jeetje, het is\n"\
"je misschien gelukt. Zou je echt gewonnen\n"\
"hebben?\n"\
"Er rommelt iets in de verte.\n"\
"Er begint een blauw licht te gloeien\n"\
"in de verbouwde kop van de monsteruitspuger."


#define DUTT5TEXT \
"Wat nu? Dit ziet er totaal anders uit.\n"\
"Een beetje als Koning Toet's appartement.\n"\
"Och, wat hier kan erger zijn dan wat je\n"\
"gewend bent? Toch? Of misschien zou het\n"\
"beter zijn om slapende goden niet wakker\n"\
"te maken..."

#define DUTT6TEXT \
"Tijd voor vakantie. Je hebt de ingewanden\n"\
"van de hel eruit laten komen en nu ben je\n"\
"wel toe aan een pauze. Je mompelt in jezelf,\n"\
"misschien kan iemand anders de volgende\n"\
"keer de hel eens op zijn kloten geven. Voor\n"\
"je ligt een rustig stadje, met vreedzaam\n"\
"stromend water, ouderwetse gebouwtjes en\n"\
"waarschijnlijk geen hellegebroed.\n"\
"\n"\
"Terwijl je uit je vervoermiddel stapt, hoor\n"\
"je het gekletter van de hoefijzers van een\n"\
"cyberdemon."



//
// Character cast strings F_FINALE.C
//
#define DUTCC_ZOMBIE	"ZOMBIEMAN"
#define DUTCC_SHOTGUN	"SHOTGUN GUY"
#define DUTCC_HEAVY	"HEAVY WEAPON DUDE"
#define DUTCC_IMP	"IMP"
#define DUTCC_DEMON	"DEMON"
#define DUTCC_LOST	"LOST SOUL"
#define DUTCC_CACO	"CACODEMON"
#define DUTCC_HELL	"HELL KNIGHT"
#define DUTCC_BARON	"BARON OF HELL"
#define DUTCC_ARACH	"ARACHNOTRON"
#define DUTCC_PAIN	"PAIN ELEMENTAL"
#define DUTCC_REVEN	"REVENANT"
#define DUTCC_MANCU	"MANCUBUS"
#define DUTCC_ARCH	"ARCH-VILE"
#define DUTCC_SPIDER	"DE SPIDER MASTERMIND"
#define DUTCC_CYBER	"DE CYBERDEMON"
#define DUTCC_HERO	"ONZE HELD"

//new strings
#define DUTV_INITSTR    "V_Init: schermen alloceren.\n"
#define DUTM_LDEFSTR    "M_LoadDefaults: Systeem defaults laden.\n"
#define DUTZ_INITSTR    "Z_Init: Init zone geheugen allocatie daemon. \n"
#define DUTW_INITSTR    "W_Init: Init WADbestanden.\n"
#define DUTM_INITSTR    "M_Init: Init verschillende info.\n"
#define DUTR_INITSTR    "R_Init: Init DOOM ververs daemon - "
#define DUTP_INITSTR    "\nP_Init: Init Playloop status.\n"
#define DUTI_INITSTR    "I_Init: Setting up machine status.\n"
#define DUTD_CHKNETSTR  "D_CheckNetGame: Checken netwerk spel status.\n"
#define DUTS_INITSTR    "S_Init: Setting up geluid.\n"
#define DUTHU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define DUTST_INITSTR   "ST_Init: Init status balk.\n"
#define DUTNETLISTEN    "luisteren naar netwerk start info...\n"
#define DUTNETSEND      "verzenden netwerk start info...\n"
#define DUTTURBOSCLSTR  "turbo schaal: %i%%\n"
#define DUTISTURBOSTR   "%s is turbo!"

#define DUTMODMSG\
	    "===========================================================================\n"\
	    "ATTENTIE:  Deze versie van DOOM is gewijzigd.  Als je een exemplaar\n"\
	    "van het volledige spel wilt hebben, bel 1-800-IDGAMES of lees readme.txt.\n"\
	    "  Je zult geen technische ondersteuning krijgen voor gewijzigde spellen.\n"\
	    "                      toets enter om verder te gaan\n"\
	    "===========================================================================\n"
#define DUTNOSWMSG\
	    "===========================================================================\n"\
	    "             Deze versie is GEEN SHAREWARE, niet verspreiden!\n"\
	    "         Rapporteer software piraterij aan de SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define DUTNOSWMSG2\
	    "===========================================================================\n"\
	    "                            Niet verspreiden!\n"\
	    "         Rapporteer software piraterij aan de SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define DUTSWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define DUTUDOOMSTART\
 "The Ultimate DOOM"
#define DUTSHAREDOOMSTART\
 "DOOM (Shareware)"
#define DUTREGDOOMSTART\
 "DOOM (Geregistered)"
#define DUTDOOM2START\
 "DOOM 2: Hell on Earth"
#define DUTPLUTSTART\
 "DOOM 2: The Plutonia Experiment"
#define DUTTNTSTART\
 "DOOM 2: TNT - Evilution"
#define DUTPUBDOOMSTART\
 "DOOM"
#define DUTDOSDOOMSTART\
 "DOSDoom"

//these are unused.  They're just here for Deh compatibility
#define DUTI_SDPMI   "I_StartupDPMI\n"
#define DUTI_SMOUSE  "I_StartupMouse\n"
#define DUTI_SJOY    "I_StartupJoystick\n"
#define DUTI_SKEY    "I_StartupKeyboard\n"
#define DUTI_SSOUND  "I_StartupSound\n"
#define DUTI_STIMER  "I_StartupTimer()\n"
#define DUTMOUSEOFF  "Mouse: not present\n"
#define DUTMOUSEON   "Mouse: detected\n"
#define DUTDPMI1     "DPMI memory: 0x%x"
#define DUTDPMI2     ", 0x%x allocated for zone\n"
#define DUTCOMMVER   "\tcommercial version.\n"
#define DUTSHAREVER  "\tshareware version.\n"
#define DUTREGVER    "\tregistered version.\n"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
