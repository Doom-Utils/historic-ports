// Emacs style mode select   -*- C++ -*-
// vi:set tabstop=8:
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1998, 1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION: Function to write the Doom end message text
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id:$";

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "doomstat.h"
#include "w_wad.h"
#include "z_zone.h"
#include "endtxt.h"

void ShowEndTxt()
{
	int i, j;
	int att = 0;
	int nlflag = 0;
	unsigned short *text;
	char *col;

	// if option -noendtxt is set, don't print the text
	if (noendtxt)
		return;

	// if the xterm has more than 80 columns we need to add nl's
	col = getenv("COLUMNS");
	if (col) {
		if (atoi(col) > 80)
			nlflag++;
	}

	// get the lump with the text
	text = W_CacheLumpNum(W_GetNumForName("ENDOOM"), PU_CACHE);

	// select first alternate font and hope it's a IBMPC font
	printf("\033[11m");

	// print 80x25 text and deal with the attributes too
	for (i = 1; i <= 80*25; i++) {
		// attribute first
		// attribute changed?
		if ((j = *text >> 8) != att) {
			// remember current attribute
			att = j;
			// set new attribute, forground color first
			printf("\033[");
			switch (j & 0x0f) {
			case 0:		// black
				printf("30");
				break;
			case 1:		// blue
				printf("34");
				break;
			case 2:		// green
				printf("32");
				break;
			case 3:		// cyan
				printf("36");
				break;
			case 4:		// red
				printf("31");
				break;
			case 5:		// magenta
				printf("35");
				break;
			case 6:		// brown
				printf("33");
				break;
			case 7:		// bright grey
				printf("37");
				break;
			case 8:		// dark grey
				printf("1;30");
				break;
			case 9:		// bright blue
				printf("1;34");
				break;
			case 10:	// bright green
				printf("1;32");
				break;
			case 11:	// bright cyan
				printf("1;36");
				break;
			case 12:	// bright red
				printf("1;31");
				break;
			case 13:	// bright magenta
				printf("1;35");
				break;
			case 14:	// yellow
				printf("1;33");
				break;
			case 15:	// white
				printf("1;37");
				break;
			}
			printf("m");
			// now background color
			printf("\033[");
			switch((j >> 4) & 0x0f) {
			case 0:		// black
				printf("40");
				break;
			case 1:		// blue
				printf("44");
				break;
			case 2:		// green
				printf("42");
				break;
			case 3:		// cyan
				printf("46");
				break;
			case 4:		// red
				printf("41");
				break;
			case 5:		// magenta
				printf("45");
				break;
			case 6:		// brown
				printf("43");
				break;
			case 7:		// bright grey
				printf("47");
				break;
			case 8:		// dark grey
				printf("1;40");
				break;
			case 9:		// bright blue
				printf("1;44");
				break;
			case 10:	// bright green
				printf("1;42");
				break;
			case 11:	// bright cyan
				printf("1;46");
				break;
			case 12:	// bright red
				printf("1;41");
				break;
			case 13:	// bright magenta
				printf("1;45");
				break;
			case 14:	// yellow
				printf("1;43");
				break;
			case 15:	// white
				printf("1;47");
				break;
			}
			printf("m");
		}

		// now the text
		putchar(*text++ & 0xff);

		// do we need a nl?
		if (nlflag) {
			if (!(i % 80)) {
				printf("\033[0m");
				att = 0;
				printf("\n");
			}
		}
	}

	// back to primary font
	printf("\033[10m");

	// all attributes off
	printf("\033[0m");

	if (nlflag)
		printf("\n");

	// cursor one line up, so that first line won't scroll out
	printf("\033[A");
	fflush(stdout);
}
