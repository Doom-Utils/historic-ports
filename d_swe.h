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
//      Swedish language support.
//
//***************
// LAZY VERSION
//***************
//
//===============================================
//      TRANSLATIONS MADE BY FREDRIK EHNBOM
//          (fredde@mbox323.swipnet.se)
//===============================================
//
//====================
//      NOTES 
//====================
// The episode texts are only translated in doom and doom 2.
// If you want translation for the rest of the texts mail me.
//
// in the game:
//  [ will be †
//  ' will be „
//  ; will be ”
//
//-----------------------------------------------------------------------------

#ifndef __D_SWE__
#define __D_SWE__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define SWED_DEVSTR        "Development mode ON.\n"
#define SWED_CDROM "CD-ROM Version: default.cfg from c:\\doomdata\n"

//
//	M_Menu.C
//
#define SWEPRESSKEY     "tryck p[ en knapp."
#define SWEPRESSYN      "tryck y eller n."
#define SWEQUITMSG      "'r du s'ker p[ att du vill\ng[ ur detta spel?"
#define SWELOADNET      "du kan inte ladda n'r du 'r i ett n'tspel!\n\ntryck p[ en knapp"
#define SWEQLOADNET     "du kan inte snabbladda i ett n'tspel!\n\ntryck p[ en knapp"
#define SWEQSAVESPOT    "du har inte valt ett snabbspar f'lt 'nnu!\n\ntryck p[ en knapp"
#define SWESAVEDEAD     "du kan inte spara om du inte spelar!\n\ntryck p[ en knapp"
#define SWEQSPROMPT     "snabbspara ;ver ditt spel kallad\n\n'%s'?\n\ntryck y eller n"
#define SWEQLPROMPT     "vill du snabbladda spelet kallad\n\n'%s'?\n\ntryck y eller n"

#define SWENEWGAME \
"du kan inte starta ett nytt spel\n"\
"n'r du 'r inne i ett n'tv'rks spel.\n\ntryck p[ en knapp"

#define SWENIGHTMARE       \
"'r du s'ker? den h'r graden 'r\n"\
"inte ens r'ttvis p[ l[ngt avst[nd.\n\ntryck y eller n"

#define SWESWSTRING        \
"det h'r 'r shareware versionen av doom.\n\n"\
"du m[ste best'lla hela trilogin.\n\ntryck p[ en knapp"

#define SWEMSGOFF  "Meddelanden AV"
#define SWEMSGON           "Meddelanden P["
#define SWENETEND  "du kan inte avsluta ett n'tspel!\n\ntryck p[ en knapp"
#define SWEENDGAME "'r du s'ker att du vill avsluta spelet?\n\ntryck y eller n"

#define SWEDOSY            "(tryck y f;r att avsluta)"

#define SWEDETAILHI        "H;g detalj"
#define SWEDETAILLO        "L[g detalj"
#define SWEGAMMALVL0       "Gamma correction OFF"
#define SWEGAMMALVL1       "Gamma correction level 1"
#define SWEGAMMALVL2       "Gamma correction level 2"
#define SWEGAMMALVL3       "Gamma correction level 3"
#define SWEGAMMALVL4       "Gamma correction level 4"
#define SWEEMPTYSTRING     "tomt f'lt"
        
//
//	P_inter.C
//
#define SWEGOTARMOR        "Plockade upp armor."
#define SWEGOTMEGA         "Plockade upp MegaArmor!"
#define SWEGOTHTHBONUS     "Plockade upp en h'lso bonus."
#define SWEGOTARMBONUS     "Plockade upp ett armor bonus."
#define SWEGOTSTIM         "Plockade upp ett stimpack."
#define SWEGOTMEDINEED     "Plockade upp ett medikit som du VERKLIGEN beh;vde!"
#define SWEGOTMEDIKIT      "Plockade upp ett medikit."
#define SWEGOTSUPER        "Supercharge!"
        
#define SWEGOTBLUECARD     "Plockade upp ett bl[tt nyckelkort."
#define SWEGOTYELWCARD     "Plockade upp ett gult nyckelkort."
#define SWEGOTREDCARD      "Plockade upp ett r;tt nyckelkort."
#define SWEGOTBLUESKUL     "Plockade upp en bl[ skall nyckel."
#define SWEGOTYELWSKUL     "Plockade upp en gul skall nyckel."
#define SWEGOTREDSKULL     "Plockade upp en r;d skall nyckel."

#define SWEGOTINVUL        "Od;dlighet!"
#define SWEGOTBERSERK      "Berserk!"
#define SWEGOTINVIS        "Osynlighet!"
#define SWEGOTSUIT         "Radiation Shielding Suit"
#define SWEGOTMAP          "Computer Area Map"
#define SWEGOTVISOR        "Light Amplification Visor"
#define SWEGOTMSPHERE      "MegaSphere!"

#define SWEGOTCLIP         "Plockade upp ett magasin."
#define SWEGOTCLIPBOX      "Plockade upp en l[da med kulor."
#define SWEGOTROCKET       "Plockade upp en raket."
#define SWEGOTROCKBOX      "Plockade upp en l[da med raketer."
#define SWEGOTCELL         "Plockade upp ett batteri."
#define SWEGOTCELLBOX      "Plockade upp en packe med batterier."
#define SWEGOTSHELLS       "Plockade upp 4 hagelgev'rs patroner."
#define SWEGOTSHELLBOX     "Plockade upp en l[da med hagelgev'rs patroner."
#define SWEGOTBACKPACK     "Plockade upp en backpack full med ammo!"
        
#define SWEGOTBFG9000      "Du fick en BFG9000!  Oh, yes."
#define SWEGOTCHAINGUN     "Du fick en minigun!"
#define SWEGOTCHAINSAW     "En motors[g!  Hitta lite k;tt!"
#define SWEGOTLAUNCHER     "Du fick raket gev'ret!"
#define SWEGOTPLASMA       "Du fick plasma pistolen!"
#define SWEGOTSHOTGUN      "Du fick hagelgev'ret!"
#define SWEGOTSHOTGUN2     "Du fick det dubbelpipade hagelgev'ret!"

//
// P_Doors.C
//
#define SWEPD_BLUEO        "Du beh;ver en bl[ nyckel f;r att aktivera detta objekt"
#define SWEPD_REDO         "Du beh;ver en r;d nyckel f;r att aktivera detta objekt"
#define SWEPD_YELLOWO      "Du beh;ver en gul nyckel f;r att aktivera detta objekt"
#define SWEPD_BLUEK        "Du beh;ver en bl[ nyckel f;r att ;ppna den h'r d;rren"
#define SWEPD_REDK         "Du beh;ver en r;d nyckel f;r att ;ppna den h'r d;rren"
#define SWEPD_YELLOWK      "Du beh;ver en gul nyckel f;r att ;ppna den h'r d;rren"
        
//
//	G_game.C
//
#define SWEGGSAVED "spelet sparat."

//
//	HU_stuff.C
//
#define SWEHUSTR_MSGU      "[Message unsent]"

#define SWEHUSTR_E1M1      "E1M1: Hangar"
#define SWEHUSTR_E1M2      "E1M2: Nuclear Plant"
#define SWEHUSTR_E1M3      "E1M3: Toxin Refinery"
#define SWEHUSTR_E1M4      "E1M4: Command Control"
#define SWEHUSTR_E1M5      "E1M5: Phobos Lab"
#define SWEHUSTR_E1M6      "E1M6: Central Processing"
#define SWEHUSTR_E1M7      "E1M7: Computer Station"
#define SWEHUSTR_E1M8      "E1M8: Phobos Anomaly"
#define SWEHUSTR_E1M9      "E1M9: Military Base"

#define SWEHUSTR_E2M1      "E2M1: Deimos Anomaly"
#define SWEHUSTR_E2M2      "E2M2: Containment Area"
#define SWEHUSTR_E2M3      "E2M3: Refinery"
#define SWEHUSTR_E2M4      "E2M4: Deimos Lab"
#define SWEHUSTR_E2M5      "E2M5: Command Center"
#define SWEHUSTR_E2M6      "E2M6: Halls of the Damned"
#define SWEHUSTR_E2M7      "E2M7: Spawning Vats"
#define SWEHUSTR_E2M8      "E2M8: Tower of Babel"
#define SWEHUSTR_E2M9      "E2M9: Fortress of Mystery"

#define SWEHUSTR_E3M1      "E3M1: Hell Keep"
#define SWEHUSTR_E3M2      "E3M2: Slough of Despair"
#define SWEHUSTR_E3M3      "E3M3: Pandemonium"
#define SWEHUSTR_E3M4      "E3M4: House of Pain"
#define SWEHUSTR_E3M5      "E3M5: Unholy Cathedral"
#define SWEHUSTR_E3M6      "E3M6: Mt. Erebus"
#define SWEHUSTR_E3M7      "E3M7: Limbo"
#define SWEHUSTR_E3M8      "E3M8: Dis"
#define SWEHUSTR_E3M9      "E3M9: Warrens"
        
#define SWEHUSTR_E4M1      "E4M1: Hell Beneath"
#define SWEHUSTR_E4M2      "E4M2: Perfect Hatred"
#define SWEHUSTR_E4M3      "E4M3: Sever The Wicked"
#define SWEHUSTR_E4M4      "E4M4: Unruly Evil"
#define SWEHUSTR_E4M5      "E4M5: They Will Repent"
#define SWEHUSTR_E4M6      "E4M6: Against Thee Wickedly"
#define SWEHUSTR_E4M7      "E4M7: And Hell Followed"
#define SWEHUSTR_E4M8      "E4M8: Unto The Cruel"
#define SWEHUSTR_E4M9      "E4M9: Fear"

#define SWEHUSTR_1 "level 1: entryway"
#define SWEHUSTR_2 "level 2: underhalls"
#define SWEHUSTR_3 "level 3: the gantlet"
#define SWEHUSTR_4 "level 4: the focus"
#define SWEHUSTR_5 "level 5: the waste tunnels"
#define SWEHUSTR_6 "level 6: the crusher"
#define SWEHUSTR_7 "level 7: dead simple"
#define SWEHUSTR_8 "level 8: tricks and traps"
#define SWEHUSTR_9 "level 9: the pit"
#define SWEHUSTR_10        "level 10: refueling base"
#define SWEHUSTR_11        "level 11: 'o' of destruction!"
        
#define SWEHUSTR_12        "level 12: the factory"
#define SWEHUSTR_13        "level 13: downtown"
#define SWEHUSTR_14        "level 14: the inmost dens"
#define SWEHUSTR_15        "level 15: industrial zone"
#define SWEHUSTR_16        "level 16: suburbs"
#define SWEHUSTR_17        "level 17: tenements"
#define SWEHUSTR_18        "level 18: the courtyard"
#define SWEHUSTR_19        "level 19: the citadel"
#define SWEHUSTR_20        "level 20: gotcha!"

#define SWEHUSTR_21        "level 21: nirvana"
#define SWEHUSTR_22        "level 22: the catacombs"
#define SWEHUSTR_23        "level 23: barrels o' fun"
#define SWEHUSTR_24        "level 24: the chasm"
#define SWEHUSTR_25        "level 25: bloodfalls"
#define SWEHUSTR_26        "level 26: the abandoned mines"
#define SWEHUSTR_27        "level 27: monster condo"
#define SWEHUSTR_28        "level 28: the spirit world"
#define SWEHUSTR_29        "level 29: the living end"
#define SWEHUSTR_30        "level 30: icon of sin"

#define SWEHUSTR_31        "level 31: wolfenstein"
#define SWEHUSTR_32        "level 32: grosse"
        
#define SWEPHUSTR_1        "level 1: congo"
#define SWEPHUSTR_2        "level 2: well of souls"
#define SWEPHUSTR_3        "level 3: aztec"
#define SWEPHUSTR_4        "level 4: caged"
#define SWEPHUSTR_5        "level 5: ghost town"
#define SWEPHUSTR_6        "level 6: baron's lair"
#define SWEPHUSTR_7        "level 7: caughtyard"
#define SWEPHUSTR_8        "level 8: realm"
#define SWEPHUSTR_9        "level 9: abattoire"
#define SWEPHUSTR_10       "level 10: onslaught"
#define SWEPHUSTR_11       "level 11: hunted"

#define SWEPHUSTR_12       "level 12: speed"
#define SWEPHUSTR_13       "level 13: the crypt"
#define SWEPHUSTR_14       "level 14: genesis"
#define SWEPHUSTR_15       "level 15: the twilight"
#define SWEPHUSTR_16       "level 16: the omen"
#define SWEPHUSTR_17       "level 17: compound"
#define SWEPHUSTR_18       "level 18: neurosphere"
#define SWEPHUSTR_19       "level 19: nme"
#define SWEPHUSTR_20       "level 20: the death domain"

#define SWEPHUSTR_21       "level 21: slayer"
#define SWEPHUSTR_22       "level 22: impossible mission"
#define SWEPHUSTR_23       "level 23: tombstone"
#define SWEPHUSTR_24       "level 24: the final frontier"
#define SWEPHUSTR_25       "level 25: the temple of darkness"
#define SWEPHUSTR_26       "level 26: bunker"
#define SWEPHUSTR_27       "level 27: anti-christ"
#define SWEPHUSTR_28       "level 28: the sewers"
#define SWEPHUSTR_29       "level 29: odyssey of noises"
#define SWEPHUSTR_30       "level 30: the gateway of hell"

#define SWEPHUSTR_31       "level 31: cyberden"
#define SWEPHUSTR_32       "level 32: go 2 it"

#define SWETHUSTR_1        "level 1: system control"
#define SWETHUSTR_2        "level 2: human bbq"
#define SWETHUSTR_3        "level 3: power control"
#define SWETHUSTR_4        "level 4: wormhole"
#define SWETHUSTR_5        "level 5: hanger"
#define SWETHUSTR_6        "level 6: open season"
#define SWETHUSTR_7        "level 7: prison"
#define SWETHUSTR_8        "level 8: metal"
#define SWETHUSTR_9        "level 9: stronghold"
#define SWETHUSTR_10       "level 10: redemption"
#define SWETHUSTR_11       "level 11: storage facility"
        
#define SWETHUSTR_12       "level 12: crater"
#define SWETHUSTR_13       "level 13: nukage processing"
#define SWETHUSTR_14       "level 14: steel works"
#define SWETHUSTR_15       "level 15: dead zone"
#define SWETHUSTR_16       "level 16: deepest reaches"
#define SWETHUSTR_17       "level 17: processing area"
#define SWETHUSTR_18       "level 18: mill"
#define SWETHUSTR_19       "level 19: shipping/respawning"
#define SWETHUSTR_20       "level 20: central processing"

#define SWETHUSTR_21       "level 21: administration center"
#define SWETHUSTR_22       "level 22: habitat"
#define SWETHUSTR_23       "level 23: lunar mining project"
#define SWETHUSTR_24       "level 24: quarry"
#define SWETHUSTR_25       "level 25: baron's den"
#define SWETHUSTR_26       "level 26: ballistyx"
#define SWETHUSTR_27       "level 27: mount pain"
#define SWETHUSTR_28       "level 28: heck"
#define SWETHUSTR_29       "level 29: river styx"
#define SWETHUSTR_30       "level 30: last call"

#define SWETHUSTR_31       "level 31: pharaoh"
#define SWETHUSTR_32       "level 32: caribbean"

#define SWEHUSTR_CHATMACRO1        "Jag 'r redo att sparka r;v!"
#define SWEHUSTR_CHATMACRO2        "Jag 'r OK."
#define SWEHUSTR_CHATMACRO3        "Jag ser inte s[ bra ut!"
#define SWEHUSTR_CHATMACRO4        "Hj'lp!"
#define SWEHUSTR_CHATMACRO5        "Du suger!"
#define SWEHUSTR_CHATMACRO6        "N'sta g[ng, snorunge..."
#define SWEHUSTR_CHATMACRO7        "Kom hit!"
#define SWEHUSTR_CHATMACRO8        "Jag tar hand om det."
#define SWEHUSTR_CHATMACRO9        "Ja"
#define SWEHUSTR_CHATMACRO0        "Nej"

#define SWEHUSTR_TALKTOSELF1       "du mumlar f;r dig sj'lv"
#define SWEHUSTR_TALKTOSELF2       "Vem d'r?"
#define SWEHUSTR_TALKTOSELF3       "Du skr'mmer dig sj'lv"
#define SWEHUSTR_TALKTOSELF4         "Du b;rjar att gorma"
#define SWEHUSTR_TALKTOSELF5       "Du har f;rlorat det..."

#define SWEHUSTR_MESSAGESENT       "[Message Sent]"
        
// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define SWEHUSTR_PLRGREEN  "Gr;n: "
#define SWEHUSTR_PLRINDIGO "Indigo: "
#define SWEHUSTR_PLRBROWN  "Brun: "
#define SWEHUSTR_PLRRED            "R;d: "
#define SWEHUSTR_PLRGOLD	"Gold: "
#define SWEHUSTR_PLRBLUE	"Blue: "
#define SWEHUSTR_PLRDKBLUE	"DkBlue: "
#define SWEHUSTR_PLRPINK	"Pink: "
        
#define SWEHUSTR_KEYGREEN  "g"
#define SWEHUSTR_KEYINDIGO "i"
#define SWEHUSTR_KEYBROWN  "b"
#define SWEHUSTR_KEYRED    "r"
        
//
//	AM_map.C
//

#define SWEAMSTR_FOLLOWON  "Follow Mode ON"
#define SWEAMSTR_FOLLOWOFF "Follow Mode OFF"
        
#define SWEAMSTR_GRIDON    "Grid ON"
#define SWEAMSTR_GRIDOFF   "Grid OFF"

#define SWEAMSTR_MARKEDSPOT        "Marked Spot"
#define SWEAMSTR_MARKSCLEARED      "All Marks Cleared"

//
//	ST_stuff.C
//

#define SWESTSTR_MUS               "Music Change"
#define SWESTSTR_NOMUS             "IMPOSSIBLE SELECTION"
#define SWESTSTR_DQDON             "Degreelessness Mode On"
#define SWESTSTR_DQDOFF    "Degreelessness Mode Off"

#define SWESTSTR_KFAADDED  "Alla Prylar Fuskades Till"
#define SWESTSTR_FAADDED   "Ammo (inga nycklar) Fuskades Till"
        
#define SWESTSTR_NCON              "No Clipping Mode ON"
#define SWESTSTR_NCOFF             "No Clipping Mode OFF"
        
#define SWESTSTR_BEHOLD    "inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define SWESTSTR_BEHOLDX   "Power-up Toggled"

#define SWESTSTR_CHOPPERS  "... Suger Inte - GM"
#define SWESTSTR_CLEV              "Byter Bana..."

//
//	F_Finale.C
//
#define SWEE1TEXT \
"N'r du v'l har d;dat alla dom stora bovarna\n"\
"och rensat rent p[ m[nbasen 'r du f;rmodad\n"\
"att vinna, 'r du inte det? 'r du inte det? Var\n"\
"'r din feta bel;ning och biljett hem? Vad\n"\
"i helvete 'r det h'r? Det 'r inte menat att\n"\
"sluta p[ det h'r s'ttet!\n"\
"\n" \
"Det stinker som ruttet k;tt, men ser ut\n"\
"som den f;rlorade Deimos basen.  Ser ut som\n"\
"du 'r fast p[ helvettets strand.\n"\
"Den enda v'gen ut 'r rakt igenom.\n"\
"\n"\
"F;r att forts'tta DOOM upplevelsen, spela\n"\
"The Shores of Hell och dess f;rbluffande\n"\
"forts'ttning, Inferno!\n"


#define SWEE2TEXT \
"Du har gjort det! Den ot'cka cyber-\n"\
"demon lorden som styrde p[ den f;rlorade\n"\
"Deimos m[nbasen har blivit dr'pt och du\n"\
"firar! Men ... var 'r du?\n"\
"Du kl'ttrar till kanten av m[nen\n"\
"och ser ner f;r att se den hemska\n"\
"sanningen.\n" \
"\n"\
"Deimos sv'var ;ver sj'lvaste Helvetet!\n"\
"Du har aldrig h;rt om n[gon som lyckats fly\n"\
"fr[n Helvetet, men du ska f[ dom j'vlarna\n"\
"att [ngra att dom n[gonsin fick h;ra om dig!\n"\
"Snabbt, kutar du ner till ytan av\n"\
"Helvetet.\n"\
"\n" \
"Nu, 'r det det sista kapitlet av\n"\
"DOOM! -- Inferno."


#define SWEE3TEXT \
"Den vidriga spindeldemonen som\n"\
"ledde invasionen till m[nbaserna\n"\
"och orsakade s[ mycket d;d har f[tt\n"\
"sitt arsle sparkat en sista g[ng.\n"\
"\n"\
"En g;md port ;ppnas och du kliver in.\n"\
"Du har bevisat att du 'r f;r tuff f;r\n"\
"helvetet att beh[lla, och nu spelar\n"\
"Helvetet 'ntligen r'ttvist -- du dyker\n"\
"upp ur d;rren f;r att se Jordens gr;na\n"\
"f'lt! 'ntligen hemma.\n" \
"\n"\
"Du undrar vad som har h'nt p[ Jorden\n"\
"undertiden du har slagits mot de onda.\n"\
"Det 'r bra att inget Helvettesyngel\n"\
"skulle ha kunnat komma igenom d;rren\n"\
"tillsammans med dig..."


#define SWEE4TEXT \
"Ledar spindeln m[ste ha sent sina\n"\
"trupper av Helvetesavkommor innan din\n"\
"sista konfrontation med den f;rskr'ckliga\n"\
"besten fr[n Helvetet.  Men du steg fram[t\n"\
"och tog med dig evig f;rd;melse och\n"\
"lidande [t horden som en sann hj'lte\n"\
"skulle ha gjort i ansiktet p[ n[got s[\n"\
"elakt.\n"\
"\n"\
"'nd[, n[gon skulle f[ betala f;r vad\n"\
"som h'nde daisy, din favorit kanin.\n"\
"\n"\
"men nu, ser du brett framf;r din mer\n"\
"m;jliga sm'rta och gibbitude som en nation\n"\
"av demoner som l;per amok i v[ra st'der.\n"\
"\n"\
"n'sta h[llplats, hell on earth!"


// after level 6, put this:

#define SWEC1TEXT \
"DU HAR STIGIT DJUPT IN I DEN HEMS;KTA\n" \
"STJ'RNPORTEN. MEN N[GOT 'R FEL.\n" \
"MONSTERNA HAR TAGIT MED SIG SIN EGEN\n" \
"VERKLIGHET, OCH STJ'RNPORTENS TEKNOLOGI\n" \
"BLIR OANV'NDBAR AV DERAS N'RVARO.\n" \
"\n" \
"FRAMF;R, SER DU EN UTPOST AV HELVETET, ETT\n" \
"F;RST'RKT OMR[DE. OM DU KAN KOMMA F;RBI\n" \
"DEN, KAN DU TR'NGA IN I DET HEMS;KTA\n" \
"HJ'RTAT AV M[NBASEN OCH HITTA KONTROLL-\n" \
"SPAKEN SOM H[LLER JORDENS BEFOLKNING\n" \
"SOM GISSLAN."

// After level 11, put this:

#define SWEC2TEXT \
"DU HAR VUNNIT! DIN SEGER HAR GJORT DET\n" \
"M;JLIGT F;R M'NNISKOSL'KTET ATT UTYMMA\n"\
"JORDEN OCH RYMMA FR[N MARDR;MMEN. NU 'R\n"\
"DU DEN ENDA M'NNISKAN KVAR P[ PLANETENS YTA.\n"\
"KANNIBAL MUTATIONER, K;TT'TANDE UTOMJORDINGAR,\n"\
"OCH ONDA ANDAR 'R DINA GRANNAR.\n"\
"DU LUTAR DIG BAK[T OCH V'NTAR P[ D'DEN,\n"\
"N;JD MED ATT HA R'DDAT DIN RAS.\n"\
"\n"\
"MEN D[, TAR JORD KONTROLLEN EMOT ETT\n"\
"MEDDELANDE FR[N RYMNDEN: \"SENSORER HAR\n"\
"SP[RAT K'LLAN AV INVASIONEN. OM DU\n"\
"[KER DIT, KAN DU KANSKE BLOCKERA DERAS\n"\
"ING[NG. UTOMJORDINGS BASEN LIGGET I HJ'RTAT\n"\
"I DIN EGEN HEMSTAD, INTE L[NGT IFR[N\n"\
"STJ'RNPORTEN.\" L[NGSAMT OCH SM'RTANDE STIGER\n"\
"DU UPP OCH [TERV'NDER TILL STRIDEN."


// After level 20, put this:

#define SWEC3TEXT \
"DU 'R I DEN F;RD'RVADE STADENS HJ'RTA,\n"\
"OMRINGAD AV DINA FIENDERS LIK.  DU SER\n"\
"INGET S'TT ATT F;RST;RA VARELSERNAS\n"\
"ING[NG FR[N DEN H'R SIDAN, S[ DU BITER\n"\
"SAMMAN DINA T'NDER OCH HOPPAR IGENOM DEN.\n"\
"\n"\
"DET M[STE FINNAS ETT S'TT ATT ST'NGA DEN\n"\
"FR[N ANDRA SIDAN. VAD BRYR DU DIG OM DU\n"\
"M[STE KORSA HELVETET F;R ATT KOMMA TILL\n"\
"DEN?"


// After level 29, put this:

#define SWEC4TEXT \
"DET F;RSKR'CKLIGA ANSIKTET AV DEN\n"\
"ST;RSTA DEMONEN DU N[GONSIN SETT SMULAS\n"\
"S;NDER FRAMF;R DIG, EFTER ATT DU PUMPAT\n"\
"DINA RAKETER IN I HANS S[RBARA HJ'RNA.\n"\
"MONSTREN SKRUMPNAR IHOP OCH D;R, DESS\n"\
"LEMMAR ;DEL'GGER HELVETETS O'NDLIGA YTA.\n"\
"\n"\
"\n"\
"DU HAR GJORT DET. INVASIONEN 'R ;VER.\n"\
"JORDEN 'R R'DDAD. HELVETET 'R ;DELAGT.\n"\
"DU UNDRAR VAR ELAKA M'NNISKOR KOMMER\n"\
"N'R DOM D;R, NU. MEDAN DU TORKAR SVETT\n"\
"FR[N DIN PANNA B;RJAR DU DEN L[NGA\n"\
"VANDRINGEN HEM. ATT [TERUPPBYGGA JORDEN\n"\
"B;R VARA ROLIGARE 'N ATT F;RD'RVA DEN\n" \
"VAR.\n"



// Before level 31, put this:

#define SWEC5TEXT \
"GRATULERAR, DU HAR HITTAT DEN HEMLIGA\n"\
"BANAN! SER UT SOM OM DEN SNARARE 'R\n"\
"BYGGD AV M'NNISKOR 'N AV DEMONER. DU\n"\
"UNDRAR TILL MINNE AV VEM DEN H'R H;RNAN\n"\
"AV HELVETET 'R BYGGD."


// Before level 32, put this:

#define SWEC6TEXT \
"GRATULERAR, DU HAR HITTAT DEN SUPER\n"\
"HEMLIGA BANAN!  B'ST ATT DU\n"\
"KUTAT IGENOM DEN H'R!\n"


// after map 06	

#define SWEP1TEXT  \
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

#define SWEP2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."


// after map 20

#define SWEP3TEXT \
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

#define SWEP4TEXT \
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

#define SWEP5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."

// before map 32

#define SWEP6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."


#define SWET1TEXT \
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


#define SWET2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."


#define SWET3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define SWET4TEXT \
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


#define SWET5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."


#define SWET6TEXT \
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
#define SWECC_ZOMBIE       "ZOMBIEMAN"
#define SWECC_SHOTGUN      "SHOTGUN GUY"
#define SWECC_HEAVY        "HEAVY WEAPON DUDE"
#define SWECC_IMP          "IMP"
#define SWECC_DEMON        "DEMON"
#define SWECC_LOST         "LOST SOUL"
#define SWECC_CACO         "CACODEMON"
#define SWECC_HELL         "HELL KNIGHT"
#define SWECC_BARON        "BARON OF HELL"
#define SWECC_ARACH        "ARACHNOTRON"
#define SWECC_PAIN         "PAIN ELEMENTAL"
#define SWECC_REVEN        "REVENANT"
#define SWECC_MANCU        "MANCUBUS"
#define SWECC_ARCH         "ARCH-VILE"
#define SWECC_SPIDER       "LEDAR SPINDELN"
#define SWECC_CYBER        "CYBERDEMONEN"
#define SWECC_HERO         "V[R HJ'LTE"
        
//new strings
#define SWEV_INITSTR    "V_Init: allocate screens.\n"
#define SWEM_LDEFSTR    "M_LoadDefaults: Load system defaults.\n"
#define SWEZ_INITSTR    "Z_Init: Init zone memory allocation daemon. \n"
#define SWEW_INITSTR    "W_Init: Init WADfiles.\n"
#define SWEM_INITSTR    "M_Init: Init miscellaneous info.\n"
#define SWER_INITSTR    "R_Init: Init DOOM refresh daemon - "
#define SWEP_INITSTR    "\nP_Init: Init Playloop state.\n"
#define SWEI_INITSTR    "I_Init: Setting up machine state.\n"
#define SWED_CHKNETSTR  "D_CheckNetGame: Checking network game status.\n"
#define SWES_INITSTR    "S_Init: Setting up sound.\n"
#define SWEHU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define SWEST_INITSTR   "ST_Init: Init status bar.\n"
#define SWENETLISTEN    "listening for network start info...\n"
#define SWENETSEND      "sending network start info...\n"
#define SWETURBOSCLSTR  "turbo scale: %i%%\n"
#define SWEISTURBOSTR   "%s is turbo!"

#define SWEMODMSG\
	    "===========================================================================\n"\
	    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"\
	    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"\
	    "        You will not receive technical support for modified games.\n"\
	    "                      press enter to continue\n"\
	    "===========================================================================\n"
#define SWENOSWMSG\
	    "===========================================================================\n"\
	    "             This version is NOT SHAREWARE, do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define SWENOSWMSG2\
	    "===========================================================================\n"\
	    "                            Do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define SWESWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define SWEUDOOMSTART\
 "                         The Ultimate DOOM Startup v%i.%i                        "
#define SWESHAREDOOMSTART\
 "                            DOOM Shareware Startup v%i.%i                        "
#define SWEREGDOOMSTART\
 "                            DOOM Registered Startup v%i.%i                       "
#define SWEDOOM2START\
 "                         DOOM 2: Hell on Earth v%i.%i                            "
#define SWEPLUTSTART\
 "                   DOOM 2: Plutonia Experiment v%i.%i                            "
#define SWETNTSTART\
 "                     DOOM 2: TNT - Evilution v%i.%i                              "
#define SWEPUBDOOMSTART\
 "                     Public DOOM - v%i.%i                                        "

//these are unused.  They're just here for Deh compatibility
#define SWEI_SDPMI   "I_StartupDPMI\n"
#define SWEI_SMOUSE  "I_StartupMouse\n"
#define SWEI_SJOY    "I_StartupJoystick\n"
#define SWEI_SKEY    "I_StartupKeyboard\n"
#define SWEI_SSOUND  "I_StartupSound\n"
#define SWEI_STIMER  "I_StartupTimer()\n"
#define SWEMOUSEOFF  "Mouse: not present\n"
#define SWEMOUSEON   "Mouse: detected\n"
#define SWEDPMI1     "DPMI memory: 0x%x"
#define SWEDPMI2     ", 0x%x allocated for zone\n"
#define SWECOMMVER   "\tcommercial version.\n"
#define SWESHAREVER  "\tshareware version.\n"
#define SWEREGVER    "\tregistered version.\n"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
