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
//	turkish language support
//
//-----------------------------------------------------------------------------

#ifndef __D_TURK__
#define __D_TURK__

//
//	Printed strings for translation
//

//
// D_Main.C
//
#define TRKD_DEVSTR	"Development mode ON.\n"
#define TRKD_CDROM	"CD-ROM Version: default.cfg from c:\\doomdata\n"

//
//	M_Menu.C
//
#define TRKPRESSKEY 	"a tusuna bas."
#define TRKPRESSYN 	"y veya n 'ye bas."
#define TRKQUITMSG	"bu piskopat oyundan\ncýkmak istediginize eminmisin?"
#define TRKLOADNET 	"ag oyununda oyun yukleyemezsiniz!\n\na tusuna bas."
#define TRKQLOADNET	"ag oyununda hýzlý yukleme yapýlamaz!\n\na tusuna bas."
#define TRKQSAVESPOT	"kaydedilmis oyun slotunu yukleyemezsiniz!\n\na tusuna bas."
#define TRKSAVEDEAD 	"oynamadan kaydedemezsin hýyar!\n\na tusuna bas."
#define TRKQSPROMPT 	"sizin kaydedilmis oyun isminiz\n\n'%s'?\n\ny veya n 'ye bas."
#define TRKQLPROMPT	"oyunu hýzlý yuklemeye eminmisiniz\n'%s'?\n\ny veya n 'ye bas."

#define TRKNEWGAME	\
"yeni oyuna ag oyununda\n"\
"maalesef baslayamazsýn cicim.\n\na tusuna bas."

#define TRKNIGHTMARE	\
"eminmisin? Bu zorluk \n"\
"seviyesi o okadar zor degil.\n\ny veya n 'ye bas."

#define TRKSWSTRING	\
"doomun shareware versiyonu.\n\n"\
"sukru tunca'ya sicilletiriniz.\n\na tusuna bas."

#define TRKMSGOFF	"Mesaj KAPALI"
#define TRKMSGON		"Mesaj ACIK"
#define TRKNETEND	"ag oyununu bitiremezsiniz!\n\na tusuna bas."
#define TRKENDGAME	"bu oyunudan cýkmak istedigine eminmisin?\n\ny veya n 'ye bas."

#define TRKDOSY		"(y 'ye bas ve zektur git)"

#define TRKDETAILHI	"Yukses Cozunurluk"
#define TRKDETAILLO	"Alcak Cozunurluk"
#define TRKGAMMALVL0	"Gamma correction ACIK"
#define TRKGAMMALVL1	"Gamma correction seviye 1"
#define TRKGAMMALVL2	"Gamma correction seviye 2"
#define TRKGAMMALVL3	"Gamma correction seviye 3"
#define TRKGAMMALVL4	"Gamma correction seviye 4"
#define TRKEMPTYSTRING	"bom boooos"

//
//	P_inter.C
//
#define TRKGOTARMOR	"Zirh alindi."
#define TRKGOTMEGA	"MegarZirh alindi!"
#define TRKGOTHTHBONUS	"Saglik bonusu alindi."
#define TRKGOTARMBONUS	"Zirh bonusu alindi."
#define TRKGOTSTIM	"Stimpack alindi (O ne yaa??)."
#define TRKGOTMEDINEED	"HARBIDEN ihtiyacin oldugu saglik kiti alindi!"
#define TRKGOTMEDIKIT	"saglik kiti alindi."
#define TRKGOTSUPER	"Supersarj!"

#define TRKGOTBLUECARD	"Mavi anahtar karti alindi."
#define TRKGOTYELWCARD	"Sari anahtar karti alindi."
#define TRKGOTREDCARD	"Kirmizi anahtar kartý alindi."
#define TRKGOTBLUESKUL	"Mavi kelle anahtari alindi."
#define TRKGOTYELWSKUL	"Sari kelle anahtari alindi."
#define TRKGOTREDSKULL	"Kirmizi kelle anahtari alindi."

#define TRKGOTINVUL	"Geberiksizlik!"
#define TRKGOTBERSERK	"Berserk (O ne yaa??)!"
#define TRKGOTINVIS	"Gorukmezlik"
#define TRKGOTSUIT	"Radyasyon Kiyafeti"
#define TRKGOTMAP	"Bilgisayar Haritasi"
#define TRKGOTVISOR	"Isik dalgasi alindi"
#define TRKGOTMSPHERE	"MegaSiper!"

#define TRKGOTCLIP	"Sarjor alindi."
#define TRKGOTCLIPBOX	"Kursun kutusu alindi."
#define TRKGOTROCKET	"Bi tane roket alindi."
#define TRKGOTROCKBOX	"Roket kutusu alindi."
#define TRKGOTCELL	"Cell alindi."
#define TRKGOTCELLBOX	"Cell kutusu alindi."
#define TRKGOTSHELLS	"4 fisek alindi."
#define TRKGOTSHELLBOX	"Fisek kutusu alindi."
#define TRKGOTBACKPACK	"Butun cephane doldu (Balli it)!"

#define TRKGOTBFG9000	"Iste bee. Yokedici GLX-Turbo Dizel Silahi."
#define TRKGOTCHAINGUN	"Taramali alindi!"
#define TRKGOTCHAINSAW	"Aha Testere! Bi iki agac kes!"
#define TRKGOTLAUNCHER	"Roket atar alindi!"
#define TRKGOTPLASMA	"Plazma silahi alindi!"
#define TRKGOTSHOTGUN	"Pompali alindi!"
#define TRKGOTSHOTGUN2	"Cifte Alindi (Vay bea Markasi SuperPoze)!"

//
// P_Doors.C
//
#define TRKPD_BLUEO	"Bu objeyi aktif etmek icin mavi anahtara ihtiyac var"
#define TRKPD_REDO	"Bu objeyi aktif etmek icin kirmizi anahtara ihtiyac var"
#define TRKPD_YELLOWO	"Bu objeyi aktif hale getirmek icin sari anahtara ihtiyac var"
#define TRKPD_BLUEK	"Kapinin acilmasi icin mavi anahtara ihtiyac var"
#define TRKPD_REDK	"Kapinin acilmasi icin kirmizi anahtara ihtiyac var"
#define TRKPD_YELLOWK	"Kapinin acilmasi icin sari anahtara ihtiyac var"

//
//	G_game.C
//
#define TRKGGSAVED	"oyun kaydedildi."

//
//	HU_stuff.C
//
#define TRKHUSTR_MSGU	"[Mesaj Giriniz]"

#define TRKHUSTR_E1M1	"E1M1: Sukru TUNCA"
#define TRKHUSTR_E1M2	"E1M2: Nukleer Santral"
#define TRKHUSTR_E1M3	"E1M3: Aliaga Rafineri"
#define TRKHUSTR_E1M4	"E1M4: Kontrol Odasi"
#define TRKHUSTR_E1M5	"E1M5: Kerhane"
#define TRKHUSTR_E1M6	"E1M6: Sukru TUNCA"
#define TRKHUSTR_E1M7	"E1M7: Pantera"
#define TRKHUSTR_E1M8	"E1M8: Hacker's Revenge"
#define TRKHUSTR_E1M9	"E1M9: Asker Ocagi"

#define TRKHUSTR_E2M1	"E2M1: Tepecik Sokaklari"
#define TRKHUSTR_E2M2	"E2M2: Izmir'in Kavaklari"
#define TRKHUSTR_E2M3	"E2M3: Rafineri"
#define TRKHUSTR_E2M4	"E2M4: Sepultura"
#define TRKHUSTR_E2M5	"E2M5: Metal Militia"
#define TRKHUSTR_E2M6	"E2M6: Brother's Of Metal"
#define TRKHUSTR_E2M7	"E2M7: K.S.K."
#define TRKHUSTR_E2M8	"E2M8: Alsancak"
#define TRKHUSTR_E2M9	"E2M9: LONG LIVE HEAVY METAL"

#define TRKHUSTR_E3M1	"E3M1: Sukru TUNCA"
#define TRKHUSTR_E3M2	"E3M2: Muradiye"
#define TRKHUSTR_E3M3	"E3M3: Horozkoy"
#define TRKHUSTR_E3M4	"E3M4: Laleli"
#define TRKHUSTR_E3M5	"E3M5: AptesHane"
#define TRKHUSTR_E3M6	"E3M6: Manowar"
#define TRKHUSTR_E3M7	"E3M7: Lingo Lingo Siseler"
#define TRKHUSTR_E3M8	"E3M8: Kaf-Sin-Kaf"
#define TRKHUSTR_E3M9	"E3M9: Metallica"

#define TRKHUSTR_E4M1	"E4M1: Fasulya"
#define TRKHUSTR_E4M2	"E4M2: Kuru Fasulya"
#define TRKHUSTR_E4M3	"E4M3: Yas Fasulya"
#define TRKHUSTR_E4M4	"E4M4: Fasulyanin Faydalari"
#define TRKHUSTR_E4M5	"E4M5: Fasulyanin Zararlari"
#define TRKHUSTR_E4M6	"E4M6: Cok Fasulye yiyene Noolur?"
#define TRKHUSTR_E4M7	"E4M7: Az Fasulya yiyene Noolur?"
#define TRKHUSTR_E4M8	"E4M8: Hay Senin Fasulyana"
#define TRKHUSTR_E4M9	"E4M9: Fear Of The Dark"

#define TRKHUSTR_1	"level 1: Manisa"
#define TRKHUSTR_2	"level 2: Kiro Fighters"
#define TRKHUSTR_3	"level 3: Tepecik"
#define TRKHUSTR_4	"level 4: Karsiyaka"
#define TRKHUSTR_5	"level 5: Hacker Sukru"
#define TRKHUSTR_6	"level 6: DubDub Dubaa"
#define TRKHUSTR_7	"level 7: L-Manyak"
#define TRKHUSTR_8	"level 8: Mast-Der"
#define TRKHUSTR_9	"level 9: Cagdas Tunca"
#define TRKHUSTR_10	"level 10: Fik Fik"
#define TRKHUSTR_11	"level 11: K. S. K.!"

#define TRKHUSTR_12	"level 12: Fabrikada tutun sarar.."
#define TRKHUSTR_13	"level 13: Izmir"
#define TRKHUSTR_14	"level 14: Tavernada Dugun var"
#define TRKHUSTR_15	"level 15: Dobi"
#define TRKHUSTR_16	"level 16: Ciko"
#define TRKHUSTR_17	"level 17: Kerhane"
#define TRKHUSTR_18	"level 18: Manisa"
#define TRKHUSTR_19	"level 19: Topik"
#define TRKHUSTR_20	"level 20: Sukru TUNCA!"

#define TRKHUSTR_21	"level 21: AC/DC"

#define TRKHUSTR_22	"level 22: Fuckin' Time"
#define TRKHUSTR_23	"level 23: Variller"
#define TRKHUSTR_24	"level 24: Ucurum"
#define TRKHUSTR_25	"level 25: Roots Bloody Roots"
#define TRKHUSTR_26	"level 26: Mama Said"
#define TRKHUSTR_27	"level 27: Manisa Streets"
#define TRKHUSTR_28	"level 28: Buyu Dunyasi"
#define TRKHUSTR_29	"level 29: Yasayan Son"
#define TRKHUSTR_30	"level 30: SON BOLUM"

#define TRKHUSTR_31	"level 31: Sukrustein"
#define TRKHUSTR_32	"level 32: Babaanne"

#define TRKPHUSTR_1	"level 1: Macera Zamani"
#define TRKPHUSTR_2	"level 2: Karnim Ac"
#define TRKPHUSTR_3	"level 3: I'm a Hacker"
#define TRKPHUSTR_4	"level 4: Hababam Sinifi"
#define TRKPHUSTR_5	"level 5: Kel Mahmut"
#define TRKPHUSTR_6	"level 6: Guduk"
#define TRKPHUSTR_7	"level 7: Sabaaan"
#define TRKPHUSTR_8	"level 8: TURKIYE!"
#define TRKPHUSTR_9	"level 9: Mast-Der"
#define TRKPHUSTR_10	"level 10: SUKRU TUNCA"
#define TRKPHUSTR_11	"level 11: Hack some Servers"

#define TRKPHUSTR_12	"level 12: Barrak"
#define TRKPHUSTR_13	"level 13: Tavuk"
#define TRKPHUSTR_14	"level 14: Lavuk"
#define TRKPHUSTR_15	"level 15: Turk Power"
#define TRKPHUSTR_16	"level 16: Hack Some Ass"
#define TRKPHUSTR_17	"level 17: Crack Down it"
#define TRKPHUSTR_18	"level 18: Atmosfer"
#define TRKPHUSTR_19	"level 19: F.U.C.K."
#define TRKPHUSTR_20	"level 20: Geberikler"

#define TRKPHUSTR_21	"level 21: Slayer"
#define TRKPHUSTR_22	"level 22: Bu bolumu gecene helalinden minti sakiz"
#define TRKPHUSTR_23	"level 23: Mezarlik"
#define TRKPHUSTR_24	"level 24: Komurluk"
#define TRKPHUSTR_25	"level 25: Mahzen"
#define TRKPHUSTR_26	"level 26: Kafirler"
#define TRKPHUSTR_27	"level 27: Brothers Of Metal"
#define TRKPHUSTR_28	"level 28: Long Live Heavy Metal again"
#define TRKPHUSTR_29	"level 29: E = m.c^2"
#define TRKPHUSTR_30	"level 30: Son Parti"

#define TRKPHUSTR_31	"level 31: Otopsi"
#define TRKPHUSTR_32	"level 32: Git Getir Bobi"

#define TRKTHUSTR_1	"level 1: system control"
#define TRKTHUSTR_2	"level 2: human bbq"
#define TRKTHUSTR_3	"level 3: power control"
#define TRKTHUSTR_4	"level 4: wormhole"
#define TRKTHUSTR_5	"level 5: hanger"
#define TRKTHUSTR_6	"level 6: open season"
#define TRKTHUSTR_7	"level 7: prison"
#define TRKTHUSTR_8	"level 8: metal"
#define TRKTHUSTR_9	"level 9: stronghold"
#define TRKTHUSTR_10	"level 10: redemption"
#define TRKTHUSTR_11	"level 11: storage facility"

#define TRKTHUSTR_12	"level 12: crater"
#define TRKTHUSTR_13	"level 13: nukage processing"
#define TRKTHUSTR_14	"level 14: steel works"
#define TRKTHUSTR_15	"level 15: dead zone"
#define TRKTHUSTR_16	"level 16: deepest reaches"
#define TRKTHUSTR_17	"level 17: processing area"
#define TRKTHUSTR_18	"level 18: mill"
#define TRKTHUSTR_19	"level 19: shipping/respawning"
#define TRKTHUSTR_20	"level 20: central processing"

#define TRKTHUSTR_21	"level 21: administration center"
#define TRKTHUSTR_22	"level 22: habitat"
#define TRKTHUSTR_23	"level 23: lunar mining project"
#define TRKTHUSTR_24	"level 24: quarry"
#define TRKTHUSTR_25	"level 25: baron's den"
#define TRKTHUSTR_26	"level 26: ballistyx"
#define TRKTHUSTR_27	"level 27: mount pain"
#define TRKTHUSTR_28	"level 28: heck"
#define TRKTHUSTR_29	"level 29: river styx"
#define TRKTHUSTR_30	"level 30: GINA GELDI LAAAN"

#define TRKTHUSTR_31	"level 31: pharaoh"
#define TRKTHUSTR_32	"level 32: caribbean"

#define TRKHUSTR_CHATMACRO1	"Kicini tekmelemeye hazirim!"
#define TRKHUSTR_CHATMACRO2	"Ýyiyim."
#define TRKHUSTR_CHATMACRO3	"Sapina Kadar Kaf-Sin-Kafliyiz!"
#define TRKHUSTR_CHATMACRO4	"Abi.. Birak simdi oyunu. televizyonda Tutti Frutti basladý!"
#define TRKHUSTR_CHATMACRO5	"5 manyetikli, Super-tremo, Kilitli elektro-gitar 230$!"
#define TRKHUSTR_CHATMACRO6	"K. S. K."
#define TRKHUSTR_CHATMACRO7	"Gel Lan Gavat!"
#define TRKHUSTR_CHATMACRO8	"By Sukru TUNCA (Hacker)."
#define TRKHUSTR_CHATMACRO9	"Nevet"
#define TRKHUSTR_CHATMACRO0	"Hayir"

#define TRKHUSTR_TALKTOSELF1	"Hoca..2 kisi bul 2 el okey cevirelim"
#define TRKHUSTR_TALKTOSELF2	"Nirdesin??"
#define TRKHUSTR_TALKTOSELF3	"Hoca gel iki el okey atalým"
#define TRKHUSTR_TALKTOSELF4	"Bi pispirik cevirelimmi?"
#define TRKHUSTR_TALKTOSELF5	"Fik Fik Time"

#define TRKHUSTR_MESSAGESENT	"[Mesaj Yollamak]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define TRKHUSTR_PLRGREEN	"Yesil: "
#define TRKHUSTR_PLRINDIGO	"Indigo: "
#define TRKHUSTR_PLRBROWN	"Koyu: "
#define TRKHUSTR_PLRRED		"Kirmizi: "
#define TRKHUSTR_PLRGOLD	"Gold: "
#define TRKHUSTR_PLRBLUE	"Blue: "
#define TRKHUSTR_PLRDKBLUE	"DkBlue: "
#define TRKHUSTR_PLRPINK	"Pink: "

#define TRKHUSTR_KEYGREEN	"g"
#define TRKHUSTR_KEYINDIGO	"i"
#define TRKHUSTR_KEYBROWN	"b"
#define TRKHUSTR_KEYRED	"r"

//
//	AM_map.C
//

#define TRKAMSTR_FOLLOWON	"Takip Modu ACIK"
#define TRKAMSTR_FOLLOWOFF	"Takip Modu KAPALI"

#define TRKAMSTR_GRIDON	"Sebeke ACIK"
#define TRKAMSTR_GRIDOFF	"Sebeke KAPALI"

#define TRKAMSTR_MARKEDSPOT	"Isaret konuldu"
#define TRKAMSTR_MARKSCLEARED	"Tum Isaretler silindi"

//
//	ST_stuff.C
//

#define TRKSTSTR_MUS		"Muzik Degisimi"
#define TRKSTSTR_NOMUS		" IMKANSIZ SECIM"
#define TRKSTSTR_DQDON		"Degreelessness Modu ACIK (O ne yaa??)"
#define TRKSTSTR_DQDOFF	"Degreelessness Modu KAPALI"

#define TRKSTSTR_KFAADDED	"Cephane Dolduruldu"
#define TRKSTSTR_FAADDED	"Cephane (anahtarsiz) eklendi"

#define TRKSTSTR_NCON		"Duvardan Gecme modu ACIK"
#define TRKSTSTR_NCOFF		"Duvardan Gecme modu KAPALI"

#define TRKSTSTR_BEHOLD	"OlmSzlk, Str, grnmzlik, Rad, TumHrt, or isik"
#define TRKSTSTR_BEHOLDX	"Guc Alindi"

#define TRKSTSTR_CHOPPERS	"... By Sukru TUNCA"
#define TRKSTSTR_CLEV		"Seviye Degistiriliyor..."

//
//	F_Finale.C
//
#define TRKE1TEXT \
"Efferin evladim bu harukulade oyunu\n"\
"bitiremedin.Geberdin gittin......\n"\
"Peki ya simdi noolacak??'Gencligine yazýk\n"\
"Ama bi dakka.. Olayýmýz burada bitmiyor..Daha\n"\
"bu bir baslangic....Seni daha ne bolumler\n"\
"bekliyor! Yapman gereken tek sey Sukru TUNCA'dan\n"\
"yeni bolumleri istemek\n" \
"\n"\
"Bunu yapmak icin pantera@mail.org'a mail\n"\
"at.Yeni Dehset bolumler seni bekliyor..\n"\
"Sunuda unutmaki butun oyunlar TURKCE.\n"\
"Firmamizin bazi oyunlari: Wolfenstein\n"\
"Zork I,Kiro Fighters,Doom II,Descent\n"\
"BY SUKRU TUNCA (HacX-Zoft©)\n"\
"                 HacX-Zoftware!\n"


#define TRKE2TEXT \
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


#define TRKE3TEXT \
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


#define TRKE4TEXT \
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

#define TRKC1TEXT \
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

#define TRKC2TEXT \
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

#define TRKC3TEXT \
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

#define TRKC4TEXT \
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

#define TRKC5TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE SECRET\n"\
"LEVEL! LOOKS LIKE IT'S BEEN BUILT BY\n"\
"HUMANS, RATHER THAN DEMONS. YOU WONDER\n"\
"WHO THE INMATES OF THIS CORNER OF HELL\n"\
"WILL BE."


// Before level 32, put this:

#define TRKC6TEXT \
"CONGRATULATIONS, YOU'VE FOUND THE\n"\
"SUPER SECRET LEVEL!  YOU'D BETTER\n"\
"BLAZE THROUGH THIS ONE!\n"


// after map 06	

#define TRKP1TEXT  \
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

#define TRKP2TEXT \
"Even the deadly Arch-Vile labyrinth could\n"\
"not stop you, and you've gotten to the\n"\
"prototype Accelerator which is soon\n"\
"efficiently and permanently deactivated.\n"\
"\n"\
"You're good at that kind of thing."


// after map 20

#define TRKP3TEXT \
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

#define TRKP4TEXT \
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

#define TRKP5TEXT \
"You've found the second-hardest level we\n"\
"got. Hope you have a saved game a level or\n"\
"two previous.  If not, be prepared to die\n"\
"aplenty. For master marines only."

// before map 32

#define TRKP6TEXT \
"Betcha wondered just what WAS the hardest\n"\
"level we had ready for ya?  Now you know.\n"\
"No one gets out alive."


#define TRKT1TEXT \
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


#define TRKT2TEXT \
"You hear the grinding of heavy machinery\n"\
"ahead.  You sure hope they're not stamping\n"\
"out new hellspawn, but you're ready to\n"\
"ream out a whole herd if you have to.\n"\
"They might be planning a blood feast, but\n"\
"you feel about as mean as two thousand\n"\
"maniacs packed into one mad killer.\n"\
"\n"\
"You don't plan to go down easy."


#define TRKT3TEXT \
"The vista opening ahead looks real damn\n"\
"familiar. Smells familiar, too -- like\n"\
"fried excrement. You didn't like this\n"\
"place before, and you sure as hell ain't\n"\
"planning to like it now. The more you\n"\
"brood on it, the madder you get.\n"\
"Hefting your gun, an evil grin trickles\n"\
"onto your face. Time to take some names."

#define TRKT4TEXT \
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


#define TRKT5TEXT \
"What now? Looks totally different. Kind\n"\
"of like King Tut's condo. Well,\n"\
"whatever's here can't be any worse\n"\
"than usual. Can it?  Or maybe it's best\n"\
"to let sleeping gods lie.."


#define TRKT6TEXT \
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
#define TRKCC_ZOMBIE	"SULO"
#define TRKCC_SHOTGUN	"NECO"
#define TRKCC_HEAVY	"VE KARDESI"
#define TRKCC_IMP	"MAYMUN"
#define TRKCC_DEMON	"SEVKIPON"
#define TRKCC_LOST	"KELLE PACA"
#define TRKCC_CACO	"YUVARLAK"
#define TRKCC_HELL	"KIRO ALI"
#define TRKCC_BARON	"NECMETTIN"
#define TRKCC_ARACH	"BITIRIM AHMET"
#define TRKCC_PAIN	"KEMIK"
#define TRKCC_REVEN	"BABAANNE"
#define TRKCC_MANCU	"DUBDUBDUBA"
#define TRKCC_ARCH	"ARCHIL-SOTA"
#define TRKCC_SPIDER	"ORUMCEK BEYIN"
#define TRKCC_CYBER	"MUSTAFA ER"
#define TRKCC_HERO	"SUKRU TUNCA (HACKER)"

//new strings
#define TRKV_INITSTR    "V_Init: allocate screens.\n"
#define TRKM_LDEFSTR    "M_LoadDefaults: Load system defaults.\n"
#define TRKZ_INITSTR    "Z_Init: Init zone memory allocation daemon. \n"
#define TRKW_INITSTR    "W_Init: Init WADfiles.\n"
#define TRKM_INITSTR    "M_Init: Init miscellaneous info.\n"
#define TRKR_INITSTR    "R_Init: Init DOOM refresh daemon - "
#define TRKP_INITSTR    "\nP_Init: Init Playloop state.\n"
#define TRKI_INITSTR    "I_Init: Setting up machine state.\n"
#define TRKD_CHKNETSTR  "D_CheckNetGame: Checking network game status.\n"
#define TRKS_INITSTR    "S_Init: Setting up sound.\n"
#define TRKHU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define TRKST_INITSTR   "ST_Init: Init status bar.\n"
#define TRKNETLISTEN    "listening for network start info...\n"
#define TRKNETSEND      "sending network start info...\n"
#define TRKTURBOSCLSTR  "turbo scale: %i%%\n"
#define TRKISTURBOSTR   "%s is turbo!"

#define TRKMODMSG\
	    "===========================================================================\n"\
	    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"\
	    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"\
	    "        You will not receive technical support for modified games.\n"\
	    "                      press enter to continue\n"\
	    "===========================================================================\n"
#define TRKNOSWMSG\
	    "===========================================================================\n"\
	    "             This version is NOT SHAREWARE, do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define TRKNOSWMSG2\
	    "===========================================================================\n"\
	    "                            Do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define TRKSWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define TRKUDOOMSTART\
 "The Ultimate DOOM"
#define TRKSHAREDOOMSTART\
 "DOOM (Shareware)"
#define TRKREGDOOMSTART\
 "DOOM (Registered)"
#define TRKDOOM2START\
 "DOOM 2: Hell on Earth"
#define TRKPLUTSTART\
 "DOOM 2: The Plutonia Experiment"
#define TRKTNTSTART\
 "DOOM 2: TNT - Evilution"
#define TRKPUBDOOMSTART\
 "DOOM"
#define TRKDOSDOOMSTART\
 "DOSDoom"



//these are unused.  They're just here for Deh compatibility
#define TRKI_SDPMI   "I_StartupDPMI\n"
#define TRKI_SMOUSE  "I_StartupMouse\n"
#define TRKI_SJOY    "I_StartupJoystick\n"
#define TRKI_SKEY    "I_StartupKeyboard\n"
#define TRKI_SSOUND  "I_StartupSound\n"
#define TRKI_STIMER  "I_StartupTimer()\n"
#define TRKMOUSEOFF  "Mouse: not present\n"
#define TRKMOUSEON   "Mouse: detected\n"
#define TRKDPMI1     "DPMI memory: 0x%x"
#define TRKDPMI2     ", 0x%x allocated for zone\n"
#define TRKCOMMVER   "\tcommercial version.\n"
#define TRKSHAREVER  "\tshareware version.\n"
#define TRKREGVER    "\tregistered version.\n"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
