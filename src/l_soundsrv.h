/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: l_soundsrv.h,v 1.4 1999/10/12 13:00:57 cphipps Exp $
 *
 *  Sound server for LxDoom, based on the sound server released with the 
 *   original linuxdoom sources.
 *  Copyright (C) 1993-1996 by id Software
 *  Copyright (C) 1999 by Colin Phipps
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Common header for soundserver data passing
 *-----------------------------------------------------------------------------*/

/* Define message ID's for IPC between soundserver and doom */
typedef enum {
  SNDSERV_SFX = 1,
  SNDSERV_MUSIC = 2,
} sound_ipc_id_e;

/* IPC key to use for IPC between sndserver and main program */

static const int snd_ipc_key = ('D' << 24) | ('S' << 16) | ('N' << 8) | 'D';

#define SND_IPC_BUF_SIZE (1<<16)
typedef struct {
  int srv_num;
  int req_num;
  int datalen;
  unsigned char* data[SND_IPC_BUF_SIZE];
} snd_ipc_t;

typedef struct {
  unsigned int sfxid;
  signed int link;
  unsigned int datalen;
} snd_pass_t;

/* I know, I know, vars in header files... */
static enum { SP_IPC, SP_PIPE } pass_by;

#define IPC_OPT_STR "--ipc"

/* I_GetLinkNum - returns linked sound number */
static inline signed int I_GetLinkNum(unsigned int i)
{
  return  ((S_sfx[i].link == NULL) ? -1 :
	    ((S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)));
}

/*
 * $Log: l_soundsrv.h,v $
 * Revision 1.4  1999/10/12 13:00:57  cphipps
 * Changed header to GPL, converted C++ comments to C
 *
 */
