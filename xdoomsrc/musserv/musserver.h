/*************************************************************************
 *  musserver.h
 *
 *  Copyright (C) 1995 Michael Heasley (mheasley@hmc.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *************************************************************************/


/**************************************************/
/* User-configurable parameters: program defaults */
/**************************************************/


/*************************************************************************
 * Change this to your preferred default playback device: external midi, *
 * FM synth, or AWE32 synth                                              *
 *************************************************************************/

/* #define DEFAULT_EXT_MIDI */
#define DEFAULT_FM_SYNTH
/* #define DEFAULT_AWE32_SYNTH */


/************************************************************************
 * To compile in support for AWE32 synth (requires AWE32 kernel driver, *
 * see README) regardless of the default playback device, define the    *
 * following                                                            *
 ************************************************************************/

#define AWE32_SYNTH_SUPPORT


/***************************************************************************
 * If you normally need the -u command-line switch to specify a particular *
 * device type, uncomment this line and change the type as needed          *
 ***************************************************************************/

/* #define DEFAULT_TYPE 8 */


/************************************/
/* End of user-configurable section */
/************************************/

#ifdef DEFAULT_AWE32_SYNTH
#  define AWE32_SYNTH_SUPPORT
#endif

#ifdef linux
#  include <sys/soundcard.h>
#  ifdef AWE32_SYNTH_SUPPORT
#    include <linux/awe_voice.h>
#  endif
#elif defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#  include <sys/soundcard.h>
#  ifdef AWE32_SYNTH_SUPPORT
#    include <sys/awe_voice.h>
#  endif
#elif defined(__FreeBSD__)
#  include <machine/soundcard.h>
#  ifdef AWE32_SYNTH_SUPPORT
#    include <awe_voice.h>
#  endif
#endif

#define MUS_VERSION "1.4"

struct mus_header {			/* header of music lump */
	char		id[4];
	unsigned short	music_size;
	unsigned short	header_size;
	unsigned short	channels;
	unsigned short	sec_channels;
	unsigned short	instrnum;
	unsigned short	dummy;
}; 

struct opl_instr {
	unsigned short   flags;
	unsigned char    finetune;
	unsigned char    note;
	sbi_instr_data   patchdata;
};

struct synth_voice {
	signed int    note;
	signed int    channel;
};

#define NO_SYNTH -1
#define FM_SYNTH 1
#define EXT_MIDI 2
#define AWE32_SYNTH 3
#define LIST_DEV -1
#define TERMINATED 4
#define MSG_WAIT 0

#ifdef DEFAULT_AWE32_SYNTH
#  define DEFAULT_DEV AWE32_SYNTH
#elif defined(DEFAULT_FM_SYNTH)
#  define DEFAULT_DEV FM_SYNTH
#else
#  define DEFAULT_DEV EXT_MIDI
#endif
