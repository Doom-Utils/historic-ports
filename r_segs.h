// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_segs.h,v 1.5 1998/05/03 23:02:40 killough Exp $
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
//      Refresh module, drawing LineSegs from BSP.
//
//-----------------------------------------------------------------------------

#ifndef __R_SEGS__
#define __R_SEGS__

#ifdef __GNUG__
#pragma interface
#endif

void R_RenderMaskedSegRange(drawseg_t *ds, int x1, int x2);
void R_StoreWallRange(int start, int stop);

#endif

//----------------------------------------------------------------------------
//
// $Log: r_segs.h,v $
// Revision 1.5  1998/05/03  23:02:40  killough
// beautification, add R_StoreWallRange() decl
//
// Revision 1.4  1998/04/27  02:01:28  killough
// Program beautification
//
// Revision 1.3  1998/03/02  11:53:29  killough
// add scrolling walls
//
// Revision 1.2  1998/01/26  19:27:44  phares
// First rev with no ^Ms
//
// Revision 1.1.1.1  1998/01/19  14:03:09  rand
// Lee's Jan 19 sources
//
//----------------------------------------------------------------------------
