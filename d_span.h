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
//      Printed strings for translation.
//      Spanish language support.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Translation by Abel Hernandez Zanatta - eMail "azanatta@hotmail.com"
// or "azanatta@ags.ciateq.mx"
//-----------------------------------------------------------------------------


#ifndef __D_SPANSH__
#define __D_SPANSH__

//
//      Printed strings for translation
//

//
// D_Main.C
//
#define SPAD_DEVSTR     "Modo de Desarrollo ON.\n"
#define SPAD_CDROM      "Version en CD-ROM: default.cfg de c:\\doomdata\n"

//
//      M_Menu.C
//
#define SPAPRESSKEY     "Presiona una tecla."
#define SPAPRESSYN      "Presiona y o n."
#define SPAQUITMSG      "Estas seguro que quieres\nterminar este grandioso juego?"
#define SPALOADNET      "No puedes usar cargar durante un juego en red!\n\npresiona una tecla."
#define SPAQLOADNET     "No puedes usar cargado rapido durante un juego en red!\n\npresiona una tecla."
#define SPAQSAVESPOT    "No has elegido una casilla de salvado rapido todavia!\n\npresiona una tecla."
#define SPASAVEDEAD     "No puedes salvar si no estas jugando!\n\npresiona una tecla."
#define SPAQSPROMPT     "Salvado rapido sobre tu juego llamado\n\n'%s'?\n\npresiona y o n."
#define SPAQLPROMPT     "Quieres cargar rapido el juego llamado\n\n'%s'?\n\npresiona y o n."

#define SPANEWGAME      \
"No puedes empezar un juego nuevo\n"\
"mientras estes en un juego en red.\n\npresiona una tecla."

#define SPANIGHTMARE    \
"Estas seguro? este nivel de dificultad\n"\
"no es ni remotamente justo.\n\npresiona y o n."

#define SPASWSTRING     \
"Esta es la version shareware de Doom.\n\n"\
"necesitas ordenar la trilogia completa.\n\npresiona una tecla."

#define SPAMSGOFF       "Mensajes APAGADOS"
#define SPAMSGON        "Mensajes PRENDIDOS"
#define SPANETEND       "No puedes terminar un juego en red!\n\npresiona una tecla."
#define SPAENDGAME      "Estas seguro que quieres terminar el juego?\n\npresiona y o n."

#define SPADOSY         "(presiona 'y' para terminar)"

#define SPADETAILHI     "Detalle Alto"
#define SPADETAILLO     "Detalle Bajo"
#define SPAGAMMALVL0    "Correccion Gamma APAGADO"
#define SPAGAMMALVL1    "Correccion Gamma nivel 1"
#define SPAGAMMALVL2    "Correccion Gamma nivel 2"
#define SPAGAMMALVL3    "Correccion Gamma nivel 3"
#define SPAGAMMALVL4    "Correccion Gamma nivel 4"
#define SPAEMPTYSTRING  "casilla vacia"

//
//      P_inter.C
//
#define SPAGOTARMOR     "Recogiste la armadura."
#define SPAGOTMEGA      "Recogiste la MegaArmadura!"
#define SPAGOTHTHBONUS  "Recogiste bonos de Salud."
#define SPAGOTARMBONUS  "Recogiste bonos de Armadura."
#define SPAGOTSTIM      "Recogiste un botiquin."
#define SPAGOTMEDINEED  "Recogiste un Kit-medico que REALMENTE necesitabas!"
#define SPAGOTMEDIKIT   "Recogiste un Kit-medico."
#define SPAGOTSUPER     "Supercarga!"

#define SPAGOTBLUECARD  "Recogiste una tarjeta-llave azul."
#define SPAGOTYELWCARD  "Recogiste una tarjeta-llave amarilla."
#define SPAGOTREDCARD   "Recogiste una tarjeta-llave roja."
#define SPAGOTBLUESKUL  "Recogiste un craneo-llave azul."
#define SPAGOTYELWSKUL  "Recogiste un craneo-llave amarillo."
#define SPAGOTREDSKULL  "Recogiste un craneo-llave rojo."

#define SPAGOTINVUL     "Invulnerabilidad!"
#define SPAGOTBERSERK   "Berserk!"
#define SPAGOTINVIS     "Invisibilidad Parcial"
#define SPAGOTSUIT      "Traje Protector de Radiacion"
#define SPAGOTMAP       "Mapa del Area Computarizado"
#define SPAGOTVISOR     "Visor Amplificador de Luz"
#define SPAGOTMSPHERE   "MegaEsfera!"

#define SPAGOTCLIP      "Recogiste un cargador."
#define SPAGOTCLIPBOX   "Recogiste una caja de balas."
#define SPAGOTROCKET    "Recogiste un cohete."
#define SPAGOTROCKBOX   "Recogiste una caja de cohetes."
#define SPAGOTCELL      "Recogiste una celula de Energia."
#define SPAGOTCELLBOX   "Recogiste un paquetes de celulas de energia."
#define SPAGOTSHELLS    "Recogiste 4 cartuchos de escopeta."
#define SPAGOTSHELLBOX  "Recogiste una caja de cartuchos de escopeta."
#define SPAGOTBACKPACK  "Recogiste una mochila llena de municiones!"

#define SPAGOTBFG9000   "Obtuviste el BFG9000!  Oh, si."
#define SPAGOTCHAINGUN  "Obtuviste la ametralladora!"
#define SPAGOTCHAINSAW  "Una motosierra!  Encontremos algo de carne!"
#define SPAGOTLAUNCHER  "Obtuviste el lanzacohetes!"
#define SPAGOTPLASMA    "Obtuviste el arma de plasma!"
#define SPAGOTSHOTGUN   "Obtuviste la escopeta!"
#define SPAGOTSHOTGUN2  "Obtuviste la super-escopeta!"

//
// P_Doors.C
//
#define SPAPD_BLUEO     "Necesitas una tarjeta-llave azul para activar este objeto"
#define SPAPD_REDO      "Necesitas una tarjeta-llave roja para activar este objeto"
#define SPAPD_YELLOWO   "Necesitas una tarjeta-llave amarilla para activar este objeto"
#define SPAPD_BLUEK     "Necesitas una llave azul para abrir esta puerta"
#define SPAPD_REDK      "Necesitas una llave roja para abrir esta puerta"
#define SPAPD_YELLOWK   "Necesitas una llave amarilla para abrir esta puerta"

//
//      G_game.C
//
#define SPAGGSAVED      "Juego salvado."

//
//      HU_stuff.C
//
#define SPAHUSTR_MSGU   "[Mensaje no enviado]"

#define SPAHUSTR_E1M1   "E1M1: Hangar"
#define SPAHUSTR_E1M2   "E1M2: Planta Nuclear"
#define SPAHUSTR_E1M3   "E1M3: Refineria de Toxinas"
#define SPAHUSTR_E1M4   "E1M4: Control de Comando"
#define SPAHUSTR_E1M5   "E1M5: Laboratorio de Phobos"
#define SPAHUSTR_E1M6   "E1M6: Procesadora Central"
#define SPAHUSTR_E1M7   "E1M7: Estacion Computarizada"
#define SPAHUSTR_E1M8   "E1M8: Anomalia de Phobos"
#define SPAHUSTR_E1M9   "E1M9: Base Militar"

#define SPAHUSTR_E2M1   "E2M1: Anomalia de Deimos"
#define SPAHUSTR_E2M2   "E2M2: Area de Contencion"
#define SPAHUSTR_E2M3   "E2M3: Refineria"
#define SPAHUSTR_E2M4   "E2M4: Laboratorio de Deimos"
#define SPAHUSTR_E2M5   "E2M5: Centro de Comando"
#define SPAHUSTR_E2M6   "E2M6: Pasillos de los Malditos"
#define SPAHUSTR_E2M7   "E2M7: Probetas de Reproduccion"
#define SPAHUSTR_E2M8   "E2M8: Torre de Babel"
#define SPAHUSTR_E2M9   "E2M9: Fortaleza del Misterio"

#define SPAHUSTR_E3M1   "E3M1: Infierno Mantenido"
#define SPAHUSTR_E3M2   "E3M2: Pantano de la Desesperacion"
#define SPAHUSTR_E3M3   "E3M3: Pandemonium"
#define SPAHUSTR_E3M4   "E3M4: Casa del Dolor"
#define SPAHUSTR_E3M5   "E3M5: Catedral Profana"
#define SPAHUSTR_E3M6   "E3M6: Monte Erebus"
#define SPAHUSTR_E3M7   "E3M7: Limbo"
#define SPAHUSTR_E3M8   "E3M8: Dis"
#define SPAHUSTR_E3M9   "E3M9: Consejeros"

#define SPAHUSTR_E4M1   "E4M1: Infierno Indigno"
#define SPAHUSTR_E4M2   "E4M2: Aborrecimiento Perfecto"
#define SPAHUSTR_E4M3   "E4M3: Separa lo Perverso"
#define SPAHUSTR_E4M4   "E4M4: Mal Indomito"
#define SPAHUSTR_E4M5   "E4M5: Ellos se Arrepentiran"
#define SPAHUSTR_E4M6   "E4M6: Contra la Perversion"
#define SPAHUSTR_E4M7   "E4M7: Y el infierno siguio"
#define SPAHUSTR_E4M8   "E4M8: Hasta Lo Cruel"
#define SPAHUSTR_E4M9   "E4M9: Miedo"

#define SPAHUSTR_1      "Nivel 1: la entrada"
#define SPAHUSTR_2      "Nivel 2: masmorras"
#define SPAHUSTR_3      "Nivel 3: la manopla"
#define SPAHUSTR_4      "Nivel 4: el foco"
#define SPAHUSTR_5      "Nivel 5: los tuneles de desperdicio"
#define SPAHUSTR_6      "Nivel 6: la trituradora"
#define SPAHUSTR_7      "Nivel 7: muerte simple"
#define SPAHUSTR_8      "Nivel 8: trucos y trampas"
#define SPAHUSTR_9      "Nivel 9: el foso"
#define SPAHUSTR_10     "Nivel 10: base de reabastecimiento"
#define SPAHUSTR_11     "Nivel 11: 'o' de destruccion!"

#define SPAHUSTR_12     "Nivel 12: la fabrica"
#define SPAHUSTR_13     "Nivel 13: el centro"
#define SPAHUSTR_14     "Nivel 14: lo mas denso"
#define SPAHUSTR_15     "Nivel 15: zona industrial"
#define SPAHUSTR_16     "Nivel 16: suburbios"
#define SPAHUSTR_17     "Nivel 17: vecindades"
#define SPAHUSTR_18     "Nivel 18: el patio"
#define SPAHUSTR_19     "Nivel 19: la ciudadela"
#define SPAHUSTR_20     "Nivel 20: gotcha!"

#define SPAHUSTR_21     "Nivel 21: nirvana"
#define SPAHUSTR_22     "Nivel 22: las catacumbas"
#define SPAHUSTR_23     "Nivel 23: barriles de diversion"
#define SPAHUSTR_24     "Nivel 24: el precipicio"
#define SPAHUSTR_25     "Nivel 25: Cascadas de Sangre"
#define SPAHUSTR_26     "Nivel 26: las minas abandonadas"
#define SPAHUSTR_27     "Nivel 27: condominio de monstruos"
#define SPAHUSTR_28     "Nivel 28: el mundo de espiritus"
#define SPAHUSTR_29     "Nivel 29: el fin viviente"
#define SPAHUSTR_30     "Nivel 30: icono del pecado"

#define SPAHUSTR_31     "Nivel 31: wolfenstein"
#define SPAHUSTR_32     "Nivel 32: grosse"

#define SPAPHUSTR_1     "Nivel 1: congo"
#define SPAPHUSTR_2     "Nivel 2: manantial de almas"
#define SPAPHUSTR_3     "Nivel 3: azteca"
#define SPAPHUSTR_4     "Nivel 4: enjaulado"
#define SPAPHUSTR_5     "Nivel 5: pueblo fantasma"
#define SPAPHUSTR_6     "Nivel 6: guarida del baron"
#define SPAPHUSTR_7     "Nivel 7: patio de los atrapados"
#define SPAPHUSTR_8     "Nivel 8: reino"
#define SPAPHUSTR_9     "Nivel 9: abattoire"
#define SPAPHUSTR_10    "Nivel 10: matanza"
#define SPAPHUSTR_11    "Nivel 11: cazado"

#define SPAPHUSTR_12    "Nivel 12: velocidad"
#define SPAPHUSTR_13    "Nivel 13: la cripta"
#define SPAPHUSTR_14    "Nivel 14: genesis"
#define SPAPHUSTR_15    "Nivel 15: el destello"
#define SPAPHUSTR_16    "Nivel 16: el presagio"
#define SPAPHUSTR_17    "Nivel 17: compuesto"
#define SPAPHUSTR_18    "Nivel 18: neuroesfera"
#define SPAPHUSTR_19    "Nivel 19: nme"
#define SPAPHUSTR_20    "Nivel 20: el dominio de la muerte"

#define SPAPHUSTR_21    "Nivel 21: matadero"
#define SPAPHUSTR_22    "Nivel 22: mision imposible"
#define SPAPHUSTR_23    "Nivel 23: la tumba"
#define SPAPHUSTR_24    "Nivel 24: la frontera final"
#define SPAPHUSTR_25    "Nivel 25: el templo de la obscuridad"
#define SPAPHUSTR_26    "Nivel 26: bunker"
#define SPAPHUSTR_27    "Nivel 27: anti-cristo"
#define SPAPHUSTR_28    "Nivel 28: las cloacas"
#define SPAPHUSTR_29    "Nivel 29: odisea de ruidos"
#define SPAPHUSTR_30    "Nivel 30: la antesala del infierno"

#define SPAPHUSTR_31    "Nivel 31: cyberden"
#define SPAPHUSTR_32    "Nivel 32: ve hacia el"

#define SPATHUSTR_1     "Nivel 1: Control del Sistema"
#define SPATHUSTR_2     "Nivel 2: barbacoa humana"
#define SPATHUSTR_3     "Nivel 3: Control de Poder"
#define SPATHUSTR_4     "Nivel 4: hoyo de gusano"
#define SPATHUSTR_5     "Nivel 5: colgadero"
#define SPATHUSTR_6     "Nivel 6: temporada abierta"
#define SPATHUSTR_7     "Nivel 7: prision"
#define SPATHUSTR_8     "Nivel 8: metal"
#define SPATHUSTR_9     "Nivel 9: plaza fuerte"
#define SPATHUSTR_10    "Nivel 10: redencion"
#define SPATHUSTR_11    "Nivel 11: Almacen"

#define SPATHUSTR_12    "Nivel 12: crater"
#define SPATHUSTR_13    "Nivel 13: procesamiento nuclear"
#define SPATHUSTR_14    "Nivel 14: trabajos de acero"
#define SPATHUSTR_15    "Nivel 15: zona muerta"
#define SPATHUSTR_16    "Nivel 16: rachas profundas"
#define SPATHUSTR_17    "Nivel 17: area de procesamiento"
#define SPATHUSTR_18    "Nivel 18: molino"
#define SPATHUSTR_19    "Nivel 19: envio/reproduccion"
#define SPATHUSTR_20    "Nivel 20: Central Procesadora"

#define SPATHUSTR_21    "Nivel 21: centro de administracion"
#define SPATHUSTR_22    "Nivel 22: habitat"
#define SPATHUSTR_23    "Nivel 23: proyecto minero lunar"
#define SPATHUSTR_24    "Nivel 24: presa"
#define SPATHUSTR_25    "Nivel 25: den del baron"
#define SPATHUSTR_26    "Nivel 26: ballistyx"
#define SPATHUSTR_27    "Nivel 27: Monte del Dolor"
#define SPATHUSTR_28    "Nivel 28: heck"
#define SPATHUSTR_29    "Nivel 29: river styx"
#define SPATHUSTR_30    "Nivel 30: ultima llamada"

#define SPATHUSTR_31    "Nivel 31: faraon"
#define SPATHUSTR_32    "Nivel 32: caribe"

#define SPAHUSTR_CHATMACRO1     "Estoy listo para patear culos!"
#define SPAHUSTR_CHATMACRO2     "Estoy Bien."
#define SPAHUSTR_CHATMACRO3     "No me veo muy bien!"
#define SPAHUSTR_CHATMACRO4     "Ayuda!"
#define SPAHUSTR_CHATMACRO5     "Apestas!"
#define SPAHUSTR_CHATMACRO6     "La proxima vez, escoria..."
#define SPAHUSTR_CHATMACRO7     "Ven Aqui!"
#define SPAHUSTR_CHATMACRO8     "Tendre cuidado de eso."
#define SPAHUSTR_CHATMACRO9     "Si"
#define SPAHUSTR_CHATMACRO0     "No"

#define SPAHUSTR_TALKTOSELF1    "Te recriminas tu mismo"
#define SPAHUSTR_TALKTOSELF2    "Quien esta ahi?"
#define SPAHUSTR_TALKTOSELF3    "Te asustas tu mismo"
#define SPAHUSTR_TALKTOSELF4    "Empiezas a delirar"
#define SPAHUSTR_TALKTOSELF5    "Lo perdiste..."

#define SPAHUSTR_MESSAGESENT    "[Mensaje Enviado]"

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define SPAHUSTR_PLRGREEN       "Green: "
#define SPAHUSTR_PLRINDIGO      "Indigo: "
#define SPAHUSTR_PLRBROWN       "Brown: "
#define SPAHUSTR_PLRRED         "Red: "
#define SPAHUSTR_PLRGOLD        "Gold: "
#define SPAHUSTR_PLRBLUE        "Blue: "
#define SPAHUSTR_PLRDKBLUE      "DkBlue: "
#define SPAHUSTR_PLRPINK        "Pink: "

#define SPAHUSTR_KEYGREEN       "g"
#define SPAHUSTR_KEYINDIGO      "i"
#define SPAHUSTR_KEYBROWN       "b"
#define SPAHUSTR_KEYRED         "r"

//
//      AM_map.C
//

#define SPAAMSTR_FOLLOWON       "Modo de Seguimiento PRENDIDO"
#define SPAAMSTR_FOLLOWOFF      "Mode de Segumiento APAGADO"

#define SPAAMSTR_GRIDON         "Rejilla PRENDIDA"
#define SPAAMSTR_GRIDOFF        "Rejilla APAGADA"

#define SPAAMSTR_MARKEDSPOT     "Lugar Marcado"
#define SPAAMSTR_MARKSCLEARED   "Todas las marcas eliminadas"

//
//      ST_stuff.C
//

#define SPASTSTR_MUS            "Cambio de Musica"
#define SPASTSTR_NOMUS          "SELECCION IMPOSIBLE"
#define SPASTSTR_DQDON          "Modo Degreelessness Prendido"
#define SPASTSTR_DQDOFF         "Modo Degreelessness Apagado"

#define SPASTSTR_KFAADDED       "Muy feliz Armado Agrgado"
#define SPASTSTR_FAADDED        "Armado (sin llaves) Agregado"

#define SPASTSTR_NCON           "Modo sin Recorte PRENDIDO"
#define SPASTSTR_NCOFF          "Modo sin Recorte APAGADO"

#define SPASTSTR_BEHOLD         "inVuln, Str, Inviso, Rad, Allmap, or Lite-amp"
#define SPASTSTR_BEHOLDX        "Poder Activado"

#define SPASTSTR_CHOPPERS       "... no apesta - GM"
#define SPASTSTR_CLEV           "Cambiando nivel..."

//
//      F_Finale.C
//
#define SPAE1TEXT \
"Una vez que has vencido a los malos y\n"\
"limpiado la base lunar supones que\n"\
"ganaste, o no? Oh no? Donde esta\n"\
"tu gorda recompensa y tu boleto a casa? Que\n"\
"rayos es esto? No se suponia que\n"\
"debia terminar de esta forma!\n"\
"\n" \
"Esto apesta como carne podrida, pero parece\n"\
"como la base perdida de Deimos.  Parece que\n"\
"estas atrapado en Las Playas del Infierno.\n"\
"La unica forma de salir es a traves de ellas.\n"\
"\n"\
"Para continuar la experiencia de DOOM, juega\n"\
"Las Playas del Infierno y su sorprendente\n"\
"sequela, Inferno!\n"


#define SPAE2TEXT \
"Lo has hecho! El horrible se¤or\n"\
"cyber-demonio que goberno la perdida base\n"\
"lunar de Deimos ha sido eliminado y tu eres\n"\
"el triunfador! Pero ... Donde estas?\n"\
"Escalas al la orilla de la\n"\
"luna y ves hacia abajo para ver la espantosa\n"\
"realidad.\n" \
"\n"\
"Deimos flota encima del infierno!\n"\
"Nunca has oido de alguien que haya esacapado\n"\
"del infierno, pero haras que los bastardos\n"\
"sientan haber oido de ti! Rapidamente,\n"\
"bajas a rapel hasta la superficie del\n"\
"Infierno.\n"\
"\n" \
"Ahora, estas en el capitulo final de\n"\
"DOOM! -- Inferno."


#define SPAE3TEXT \
"La repugnate ara¤a-demonio que\n"\
"comando la invasion de la bases\n"\
"lunares y causo tanta muerte ha recibido\n"\
"una patada en su culo para siempre.\n"\
"\n"\
"Un pasaje secreto se abre y entras.\n"\
"Has probado ser demasiado rudo para el\n"\
"infierno, y ahora el infierno al fin juega\n"\
"justo -- por fin emerges de la puerta\n"\
"para ver los verdes campos de la tierra!\n"\
"Hogar al fin.\n" \
"\n"\
"Piensas que habra estado pasando en\n"\
"la tierra mientras peleabas con las\n"\
"fuerzas del mal. Es bueno que ninguna\n"\
"reencarnacion infernal haya atravezado\n"\
"la puerta contigo ..."


#define SPAE4TEXT \
"la ara¤a mente maestra debe ser enviada delante\n"\
"de sus legiones infernales antes de tu\n"\
"confrontacion final con esa terrible\n"\
"bestia del infierno.  Pero tu das un paso\n"\
"y traes delante una maldicion eterna y\n"\
"sufriendo ante la horda como un verdadero heroe\n"\
"en la cara de algo tan siniestro.\n"\
"\n"\
"Ademas, alguien va a pagar por lo que le\n"\
"paso a daisy, tu conejita.\n"\
"\n"\
"pero ahora, ves una propagacion de\n"\
"potencial dolor y una nacion de\n"\
"demonios corriendo en nuestras ciudades.\n"\
"\n"\
"siguiente parada, infierno en la tierra!"


// after level 6, put this:

#define SPAC1TEXT \
"HAS ENTRADO EN LAS PROFUNDIDAES INFESTADAS DEL\n" \
"PUERTO ESTELAR. PERO ALGO ESTA MAL. LOS\n" \
"MONSTRUOS HAN TRAIDO SU PROPIA REALIDAD\n" \
"CON ELLOS, Y LA TECNOLOGIA DEL PUERTO ESTELAR\n" \
"ESTA SIENDO INVERTIDA POR SU PRESENCIA.\n" \
"\n"\
"ADELANTE, VES UN ADELANTO DEL INFIERNO, A\n" \
"UNA ZONA FORTIFICADA. SI PUEDES PASARLA,\n" \
"PENTRARAS EL CORAZON EMBRUJADO\n" \
"DE LA BASE ESTELAR Y ENCONTRARAS EL INTERRUPTOR\n" \
"QUE APRISIONA LA PROBLACION DE LA TIERRA\n" \
"COMO REHEN."

// After level 11, put this:

#define SPAC2TEXT \
"HAS GANADO! TU VICTORIA HA PERMITIDO A LA\n" \
"HUMANIDAD EVACUAR LA TIERRA Y ESCAPAR\n"\
"DE LA PESADILLA.  AHORA ERES EL UNICO\n"\
"HUMANO EN LA FAZ DEL PLANETA.\n"\
"MUTACIONES CANIBALES, ALIENS CARNIVOROS,\n"\
"Y ESPIRITUS MALIGNOS SON TUS UNICOS VECINOS.\n"\
"TE SIENTAS Y ESPERAS LA MUERTE, CONTENTO\n"\
"DE QUE HAS SALVADO TU ESPECIE.\n"\
"\n"\
"PERO ENTONCES, EL CONTROL TERRESTRE ENVIA UN\n"\
"MENSAJE DEL ESPACIO: \"LOS SENSORES HAN LOCALIZADO\n"\
"LA FUENTE DE LA INVASION ALIENIGENA. SI VAS\n"\
"AHI, PODRIAS BLOQUEAR SU ENTRADA.\n"\
"LA BASE ALIENIGENA ESTA EN EL CORAZON DE\n"\
"SU PROPIA CIUDAD, NO MUY LEJOS DE EL PUERTO\n"\
"ESTELAR.\" LENTA Y DOLOROSAMENTE TE LEVANTAS\n"\
"Y REGRESAS A LA BATALLA."


// After level 20, put this:

#define SPAC3TEXT \
"ESTAS EN EL CORRUPTO CORAZON DE LA CIUDAD,\n"\
"RODEADO POR LOS CUERPOS DE TUS ENEMIGOS.\n"\
"NO VES FORMA ALGUNA DE DESTRUIR A LAS CRIATURAS'\n"\
"DE ESTE LADO, ASI QUE APRIETAS TUS\n"\
"DIENTES Y PASAS A TRAVES DE EL.\n"\
"\n"\
"DEBE DE HABER ALGUNA FORMA DE CERRARLO EN EL\n"\
"OTRO LADO. QUE IMPORTA SI TIENES QUE\n"\
"IR A TRAVES DEL INFIERNO PARA HACERLO?"


// After level 29, put this:

#define SPAC4TEXT \
"LA HORRENDA VISION DEL DEMONIO MAS GRANDE\n"\
"QUE HAS VISTO SE DESMORONA ANTE TI,\n"\
"DESPUES DE DISPARAR TUS COHETES DENTRO SU\n"\
"CEREBRO EXPUESTO. EL MONSTRUO SE MARCHITA\n"\
"Y MUERE, SUS MIEMBROS DEVASTADORES\n"\
"DA¥ARON INCONTABLES MILLAS DE LA SUPERFICIE\n"\
"DEL INFIERNO.\n"\
"\n"\
"LO HAS HECHO. LA INVASION HA TERMINADO.\n"\
"LA TIERRA ESTA A SALVO. EL INFIERNO ES UNA RUINA.\n"\
"PIENSAS ADONDE IRAN LOS MALOS CUANDO\n"\
"MUERAN, AHORA. LIMPIANDOTE EL SUDOR DE TU\n"\
"FRENTE EMPIEZAS EL LARGO CAMINO DE REGESO A\n"\
"CASA. RECONSTRUIR LA TIERRA DEBE DE SER MUCHO\n"\
"MAS DIVERTIDO QUE LO FUE DESTRUIRLA.\n"



// Before level 31, put this:

#define SPAC5TEXT \
"FELICIDADES , ENCONTRASTE EL NIVEL\n"\
"SECRETO! PARECE COMO SI HUBIERA SIDO CONSTRUIDO POR\n"\
"HUMANOS, EN LUGAR DE DEMONIOS. PIENSAS QUIENES\n"\
"SERAN LOS RESIDENTES DE ESTA ESQUINA DEL\n"\
"INFIERNO."


// Before level 32, put this:

#define SPAC6TEXT \
"FELICIDADES, HAS ENCONTRADO EL\n"\
"NIVEL SUPER SECRETO!  MEJOR TE APURAS\n"\
"A PASAR A TRAVES DE ESTE!\n"


// after map 06 

#define SPAP1TEXT  \
"Te regocijas con malicia sobre la carcasa humeante del\n"\
"Guardian.  Con su muerte, has arrancado\n"\
"el acelerador  de las apestosas garras del\n"\
"Infierno.  Te relajas  y  hechas un vistazo alrrededor\n"\
"del cuarto.  Maldicion!  Se supone que deberia de\n"\
"haber al menos un prototipo funcionando, pero no lo\n"\
"puedes ver. Los demonios deben de haberlo tomado.\n"\
"\n"\
"Debes de encontrar el prototipo, o todos tus\n"\
"esfuerzos habran sido un desperdicio. Mantenerse\n"\
"en movimiento, peleando, matando.\n"\
"Oh si, mantenerse vivo, tambien."


// after map 11

#define SPAP2TEXT \
"Aun el mortal laberinto del Arch-Vile no\n"\
"pudo detenerte, has obtenido el prototipo\n"\
"acelerador el cual pronto sera desactivado\n"\
"permanente y eficientemente.\n"\
"\n"\
"Eres bueno para ese tipo de cosas."


// after map 20

#define SPAP3TEXT \
"Has peleado y demolido tu camino hacia\n"\
"el corazon de la colmena del diablo. Es tiempo\n"\
"de una mision Busca-y-Destruye, apuntando al\n"\
"portero, cuya descendencia esta cayendo\n"\
"a la Tierra.  Si, el es malo. Pero tu\n"\
"sabes quien es peor!\n"\
"\n"\
"Sonriendo malvadamente, checas tu equipo, y\n"\
"estas listo para darle al bastardo un peque¤o\n"\
"infierno hecho por ti mismo!"

// after map 30

#define SPAP4TEXT \
"La diabolica cara del Portero ha quedado embarrada\n"\
"por todo el lugar.  Conforme su cadaver harapiento\n"\
"se colapsa, una puerta invertida se forma y\n"\
"succciona las partes del ultimo prototipo\n"\
"acelerador, sin mencionar algunos demonios\n"\
"que habia por ahi.  Lo hiciste. El Infierno\n"\
"ha regresado a los malos y muertos en \n"\
"lugar de los buenos y vivos.  Recuerda contarles\n"\
"a tus nietos como pusiste un lanzacohetes\n"\
"en su ataud. Si vas al Infierno cuando\n"\
"mueras, lo necesitaras para hacer la\n"\
"limpieza final ..."

// before map 31

#define SPAP5TEXT \
"Has encontrado el segundo nivel mas dificil que\n"\
"tenemos. Espero que hayas salvado el juego un nivel\n"\
"o dos previos.  Si no, debes de estar preparado para\n"\
"morir a plenitud. Solo para master marines."

// before map 32

#define SPAP6TEXT \
"Apuesto a que pensabas cual FUE el nivel mas dificil\n"\
"que teniamos ahora?  Ahora ya lo sabes.\n"\
"Nadie sale vivo de aqui."


#define SPAT1TEXT \
"Has pelado todo el camino hacia los infestados\n"\
"laboratorios experimentales.  Parece que la UAC\n"\
"una vez mas se la ha tragado.  Con su alta\n"\
"perdida, debe ser muy dificil para la pobre\n"\
"y vieja UAC comprar un seguro de vida corporativo\n"\
"estos dias..\n"\
"\n"\
"Adelante se encuentra el complejo militar, ahora\n"\
"con un enjambre de horrores listos para clavarte\n"\
"sus dientes. Con suerte, en el complejo todavia\n"\
"existen algunas municiones y armas tiradas por\n"\
"ahi."


#define SPAT2TEXT \
"Escuchas el sonido de maquinaria pesada\n"\
"adelante.  Tienes la esperanza de que no hay\n"\
"un nuevo infierno, pero estas listo para\n"\
"enfrentar cualquier cosa si tienes que hacerlo.\n"\
"Ellos tal vez estan planenando un festin de sangre, pero\n"\
"te sientes como si hubieran dosmil maniaticos\n"\
"empacados en un asesino loco.\n"\
"\n"\
"No planeas que sea algo facil."


#define SPAT3TEXT \
"El panorama que se ve adelante luce como una maldicion\n"\
"familiar. Huele familiar, tambien -- parece\n"\
"excremento frito. Este lugar no te gustaba\n"\
"antes, y estas seguro que no planeas\n"\
"que te guste ahora. Entre mas piensas en\n"\
"ello, mas loco te pones.\n"\
"Cargando tu arma, una sonrisa malvada se\n"\
"dibuja en tu cara. Tiempo de tomar algunos nombres."

#define SPAT4TEXT \
"De repente, todo esta silencioso, desde un horizonte\n"\
"hasta el otro. El agonizante eco del Infierno se\n"\
"desvanece a lo lejos, el cielo de pesadilla se vuelve\n"\
"azul, los montones de cadaveres de monstruos empienzan\n"\
"a evaporarse junto con la pestilencia maldita \n"\
"que llenaba el aire. Uups, tal vez lo has\n"\
"hecho. Realmente has ganado?\n"\
"\n"\
"Algo suena en la distancia.\n"\
"Una luz azul empieza a brillar dentro de un\n"\
"craneo arruinado de un alma demoniaca."


#define SPAT5TEXT \
"Ahora que? Luce totalmente diferente. Es una especie\n"\
"de condominio del Rey Tut. Bueno, lo\n"\
"que sea no puede ser peor que lo\n"\
"usual. O puede serlo?  O tal vez lo mejor\n"\
"es dejar durmiendo a los dioses.."


#define SPAT6TEXT \
"Tiempo de vacaciones. Has reventado las\n"\
"tripas del infierno y por fin estas listo\n"\
"para un descanso. Te callas a ti mismo,\n"\
"Tal vez alguien mas puede patearle el culo\n"\
"al infierno la proxima vez. Adelante esta un pueblo,\n"\
"con un pacifico nacimiento acuifero, singulares\n"\
"edificios, y presumiblemente nada de infierno.\n"\
"\n"\
"Conforme bajas del transporte, escuchas la\n"\
"pisada de un zapato de hierro de un cyberdemonio."



//
// Character cast strings F_FINALE.C
//
#define SPACC_ZOMBIE    "HOMBRE ZOMBIE"
#define SPACC_SHOTGUN   "TIPO CON ESCOPETA"
#define SPACC_HEAVY     "TIPO CON ARMA PESADA"
#define SPACC_IMP       "IMP"
#define SPACC_DEMON     "DEMONIO"
#define SPACC_LOST      "ALMA PERDIDA"
#define SPACC_CACO      "CACODEMONIO"
#define SPACC_HELL      "CABALLERO DEL INFIERNO"
#define SPACC_BARON     "BARON DEL INFIERNO"
#define SPACC_ARACH     "ARACHNOTRON"
#define SPACC_PAIN      "ELEMENTO DEL DOLOR"
#define SPACC_REVEN     "REVENANT"
#define SPACC_MANCU     "MANCUBUS"
#define SPACC_ARCH      "ARCH-VILE"
#define SPACC_SPIDER    "LA ARAÑA MAESTRA"
#define SPACC_CYBER     "EL CYBERDEMONIO"
#define SPACC_HERO      "NUESTRO HEROE"

//new strings
#define SPAV_INITSTR    "V_Init: allocate screens.\n"
#define SPAM_LDEFSTR    "M_LoadDefaults: Load system defaults.\n"
#define SPAZ_INITSTR    "Z_Init: Init zone memory allocation daemon. \n"
#define SPAW_INITSTR    "W_Init: Init WADfiles.\n"
#define SPAM_INITSTR    "M_Init: Init miscellaneous info.\n"
#define SPAR_INITSTR    "R_Init: Init DOOM refresh daemon - "
#define SPAP_INITSTR    "\nP_Init: Init Playloop state.\n"
#define SPAI_INITSTR    "I_Init: Setting up machine state.\n"
#define SPAD_CHKNETSTR  "D_CheckNetGame: Checking network game status.\n"
#define SPAS_INITSTR    "S_Init: Setting up sound.\n"
#define SPAHU_INITSTR   "HU_Init: Setting up heads up display.\n"
#define SPAST_INITSTR   "ST_Init: Init status bar.\n"
#define SPANETLISTEN    "listening for network start info...\n"
#define SPANETSEND      "sending network start info...\n"
#define SPATURBOSCLSTR  "turbo scale: %i%%\n"
#define SPAISTURBOSTR   "%s is turbo!"

#define SPAMODMSG\
	    "===========================================================================\n"\
	    "ATTENTION:  This version of DOOM has been modified.  If you would like to\n"\
	    "get a copy of the original game, call 1-800-IDGAMES or see the readme file.\n"\
	    "        You will not receive technical support for modified games.\n"\
	    "                      press enter to continue\n"\
	    "===========================================================================\n"
#define SPANOSWMSG\
	    "===========================================================================\n"\
	    "             This version is NOT SHAREWARE, do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define SPANOSWMSG2\
	    "===========================================================================\n"\
	    "                            Do not distribute!\n"\
	    "         Please report software piracy to the SPA: 1-800-388-PIR8\n"\
	    "===========================================================================\n"
#define SPASWMSG\
	    "===========================================================================\n"\
	    "                                Shareware!\n"\
	    "===========================================================================\n"
#define SPAUDOOMSTART\
 "The Ultimate DOOM"
#define SPASHAREDOOMSTART\
 "DOOM (Shareware)"
#define SPAREGDOOMSTART\
 "DOOM (Registered)"
#define SPADOOM2START\
 "DOOM 2: Hell on Earth"
#define SPAPLUTSTART\
 "DOOM 2: The Plutonia Experiment"
#define SPATNTSTART\
 "DOOM 2: TNT - Evilution"
#define SPAPUBDOOMSTART\
 "DOOM"
#define SPADOSDOOMSTART\
 "DOSDoom"

//these are unused.  They're just here for Deh compatibility
#define SPAI_SDPMI   "I_StartupDPMI\n"
#define SPAI_SMOUSE  "I_StartupMouse\n"
#define SPAI_SJOY    "I_StartupJoystick\n"
#define SPAI_SKEY    "I_StartupKeyboard\n"
#define SPAI_SSOUND  "I_StartupSound\n"
#define SPAI_STIMER  "I_StartupTimer()\n"
#define SPAMOUSEOFF  "Mouse: not present\n"
#define SPAMOUSEON   "Mouse: detected\n"
#define SPADPMI1     "DPMI memory: 0x%x"
#define SPADPMI2     ", 0x%x allocated for zone\n"
#define SPACOMMVER   "\tcommercial version.\n"
#define SPASHAREVER  "\tshareware version.\n"
#define SPAREGVER    "\tregistered version.\n"

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
