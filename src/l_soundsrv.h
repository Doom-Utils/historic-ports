/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: l_soundsrv.h,v 1.5 2000/03/19 20:14:32 cph Exp $
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

typedef struct {
  unsigned int sfxid;
  signed int link;
  unsigned int datalen;
} snd_pass_t;

/* I_GetLinkNum - returns linked sound number */
static inline signed int I_GetLinkNum(unsigned int i)
{
  return  ((S_sfx[i].link == NULL) ? -1 :
	    ((S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)));
}

/*
 * $Log: l_soundsrv.h,v $
 * Revision 1.5  2000/03/19 20:14:32  cph
 * Sound code cleaning: DosDoom, IPC and such unused code removed
 *
 * Revision 1.4  1999/10/12 13:00:57  cphipps
 * Changed header to GPL, converted C++ comments to C
 *
 */
