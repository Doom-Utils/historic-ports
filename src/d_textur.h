// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_textur.h,v 1.1.1.1 1997/12/28 12:59:02 pekangas Exp $
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
//	Typedefs related to to textures etc.,
//	 isolated here to make it easier separating modules.
//    
//-----------------------------------------------------------------------------


#ifndef __D_TEXTUR__
#define __D_TEXTUR__

#include "doomtype.h"




//
// Flats?
//
// a pic is an unmasked block of pixels
typedef struct
{
    byte		width;
    byte		height;
    byte		data;
} pic_t;




#endif
//-----------------------------------------------------------------------------
//
// $Log: d_textur.h,v $
// Revision 1.1.1.1  1997/12/28 12:59:02  pekangas
// Initial DOOM source release from id Software
//
//
//-----------------------------------------------------------------------------
