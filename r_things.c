//
// DOSDoom Rendering things (objects as sprites) Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// 23-6-98 KM Replaced #ifdef RANGECHECK with #ifdef DEVELOPERS
// -KM- 1998/09/27 Dynamic colourmaps

#include <stdio.h>
#include <stdlib.h>

#include "dm_defs.h"
#include "m_swap.h"
#include "m_argv.h"

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

#include "dm_state.h"

// -ES- 1998/08/24
#include "r_things.h"

#include "d_debug.h"

#define MINZ	    (FRACUNIT*4)

typedef struct
{
    int		x1;
    int		x2;
	
    int		column;
    int		topclip;
    int		bottomclip;

} maskdraw_t;



//
// Sprite rotation 0 is facing the viewer,
//  rotation 1 is one angle turn CLOCKWISE around the axis.
// This is not the same as the angle,
//  which increases counter clockwise (protractor).
// There was a lot of stuff grabbed wrong, so I changed it...
//
fixed_t		pspritescale;
fixed_t		pspriteiscale;
fixed_t		pspritescale2;
fixed_t		pspriteiscale2;

int*            spritelights;

// constant arrays
//  used for psprite clipping and initializing clipping
// -ES- 1998/08/20 init to NULL
short		*negonearray = NULL;
short		*screenheightarray = NULL;


//
// INITIALIZATION FUNCTIONS
//

// variables used to look up
//  and range check thing_t sprites patches
spritedef_t*	sprites;
int		numsprites;

spriteframe_t	sprtemp[29];
int		maxframe;
char*		spritename;

//
// Define MHFX_LAXSPRITEROTATIONS for a temporary fix to the problem
// of PWADS that define sprite rotations in different orders. This
// should be regarded as a temporary fix only and might not work
// properly in all cases. See README.sprites for details and
// ESPECIALLY before using "-sprnolim" - you have been warned!
//
// Basically, this works by removing the checks on two WAD entries that
// map to the same sprite, having both a "no rotations" version of a
// sprite and various rotations of a sprite, and so forth. I cannot, so
// far, see any reason for Doom to check for this internally, as it seems
// to use the most recently loaded version of a sprite (which is, after all,
// what we want), so I assume that it is designed to protect against a
// corrupted IWAD or a buggy PWAD, or something like that.
//
#define MHFX_LAXSPRITEROTATIONS

#ifdef MHFX_LAXSPRITEROTATIONS
int  lax_sprite_rotations=0;
#endif



//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//
void
R_InstallSpriteLump
( int		lump,
  unsigned	frame,
  unsigned	rotation,
  boolean	flipped )
{
    int		r;
	
    if (frame >= 29 || rotation > 8)
	I_Error("R_InstallSpriteLump: "
		"Bad frame characters in lump %i", lump);
	
    if ((int)frame > maxframe)
	maxframe = frame;
		
    if (rotation == 0)
    {
	// the lump should be used for all rotations
        // false=0, true=1, but array initialised to -1
        // allows doom to have a "no value set yet" boolean value!

        if ( (sprtemp[frame].rotate == false)
#ifdef MHFX_LAXSPRITEROTATIONS
            && (!lax_sprite_rotations)
#endif
           )
            I_Error ("R_InitSprites: Sprite %s frame %c has "
                     "multip rot=0 lump", spritename, 'A'+frame);

        if ((sprtemp[frame].rotate == true)
#ifdef MHFX_LAXSPRITEROTATIONS
            && (!lax_sprite_rotations)
#endif
           )
            I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
		     "and a rot=0 lump", spritename, 'A'+frame);
			
	sprtemp[frame].rotate = false;
	for (r=0 ; r<8 ; r++)
	{
	    sprtemp[frame].lump[r] = lump;
	    sprtemp[frame].flip[r] = (byte)flipped;
	}
	return;
    }
	
    // the lump is only used for one rotation
    if ((sprtemp[frame].rotate == false)
#ifdef MHFX_LAXSPRITEROTATIONS
            && (!lax_sprite_rotations)
#endif
       )
	I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
		 "and a rot=0 lump", spritename, 'A'+frame);
		
    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;		
    if ((sprtemp[frame].lump[rotation] != -1)
#ifdef MHFX_LAXSPRITEROTATIONS
            && (!lax_sprite_rotations)
#endif
       )
	I_Error ("R_InitSprites: Sprite %s : %c : %c "
		 "has two lumps mapped to it",
		 spritename, 'A'+frame, '1'+rotation);
		
    sprtemp[frame].lump[rotation] = lump;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}




//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant. 
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
void R_InitSpriteDefs (char** namelist) 
{ 
    char**	check;
    int		i;
    int		l;
    int		intname;
    int		frame;
    int		rotation;

    i = 1;

    // count the number of sprite names
    check = namelist;
    while (*check != NULL)
       check++;

    numsprites = check-namelist;
	
    if (!numsprites)
	return;
		
    sprites = Z_Malloc(numsprites *sizeof(*sprites), PU_STATIC, NULL);
	
    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i=0 ; i<numsprites ; i++)
    {
	spritename = namelist[i];
	memset (sprtemp,-1, sizeof(sprtemp));
		
	maxframe = -1;
	intname = *(int *)namelist[i];
	
	// scan the lumps,
	//  filling in the frames for whatever is found
	for (l=0 ; l<numspritelumps ; l++)
	{
	    if (*(int *)lumpinfo[spritelist[l]].name == intname)
	    {
		frame = lumpinfo[spritelist[l]].name[4] - 'A';
		rotation = lumpinfo[spritelist[l]].name[5] - '0';

		R_InstallSpriteLump (l, frame, rotation, false);

		if (lumpinfo[spritelist[l]].name[6])
		{
		    frame = lumpinfo[spritelist[l]].name[6] - 'A';
		    rotation = lumpinfo[spritelist[l]].name[7] - '0';
		    R_InstallSpriteLump (l, frame, rotation, true);
		}
	    }
	}
	
	// check the frames that were found for completeness
	if (maxframe == -1)
	{
	    sprites[i].numframes = 0;
	    continue;
	}
		
	maxframe++;
	
	for (frame = 0 ; frame < maxframe ; frame++)
	{
	    switch ((int)sprtemp[frame].rotate)
	    {
	      case -1:
		// no rotations were found for that frame at all
		I_Error ("R_InitSprites: No patches found "
			 "for %s frame %c", namelist[i], frame+'A');
		break;
		
	      case 0:
		// only the first rotation is needed
		break;
			
	      case 1:
		// must have all 8 frames
		for (rotation=0 ; rotation<8 ; rotation++)
		    if (sprtemp[frame].lump[rotation] == -1)
			I_Error ("R_InitSprites: Sprite %s frame %c "
				 "is missing rotations",
				 namelist[i], frame+'A');
		break;
	    }
	}
	
	// allocate space for the frames present and copy sprtemp to it
	sprites[i].numframes = maxframe;
	sprites[i].spriteframes = 
	    Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
	memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }

}




//
// GAME FUNCTIONS
//
int		maxvissprites = MAXVISSPRITES;
vissprite_t*	vissprites = NULL;
int*            visspriteq = NULL;
int		vissprite_p;
int		newvissprite;

//
// R_InitSprites
// Called at program start.
//
void R_InitSprites (char** namelist)
{

    // -ES- 1998/08/20 Moved negonearray init to multires_setres.

#ifdef MHFX_LAXSPRITEROTATIONS
    lax_sprite_rotations=( M_CheckParm("-gfixall")
                          ||
                           M_CheckParm("-gignrot") );
#endif
    vissprites = Z_Malloc(maxvissprites * sizeof(vissprite_t), PU_STATIC, NULL);
    visspriteq = Z_Malloc(maxvissprites * sizeof(*visspriteq), PU_STATIC, NULL);
    R_InitSpriteDefs (namelist);
}



//
// R_ClearSprites
// Called at frame start.
//
void R_ClearSprites (void)
{
    vissprite_p = 0;
}


//
// R_NewVisSprite
//
int R_NewVisSprite (fixed_t scale)
{
    if (vissprite_p >= maxvissprites)
    {
      maxvissprites += 128;
      vissprites = Z_ReMalloc(vissprites, sizeof(vissprite_t) * maxvissprites);
      visspriteq = Z_ReMalloc(visspriteq, sizeof(*visspriteq) * maxvissprites);
    }
    {
      int leap, target, max = vissprite_p, min = -1;
      leap = (max + 1) /2;
      target = max / 2;
      visspriteq[max] = 0;
      while (leap)
      {
        // Optimise: if they are equal scale it doesn't matter
        // which is first.
        if (vissprites[visspriteq[target]].scale == scale)
          break;
        else if (vissprites[visspriteq[target]].scale < scale)
        {
          leap = (max - target+1)/2;
          min = target;
          target += leap;
        }
        else
        {
          leap = (target - min)/2;
          max = target;
          target -= leap;
        }
      }
  
      memmove(visspriteq + target + 1, visspriteq + target,
              (vissprite_p - target) * sizeof(int));
  
      visspriteq[target] = vissprite_p;
    }
    return vissprite_p++;
}



//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//
short*		mfloorclip;
short*		mceilingclip;

fixed_t		spryscale;
fixed_t		sprtopscreen;

void R_DrawMaskedColumn (column_t* column)
{
    int		topscreen;
    int 	bottomscreen;
    fixed_t	basetexturemid;
	
    basetexturemid = dc_texturemid;
	
    for ( ; column->topdelta != 0xff ; ) 
    {
	// calculate unclipped screen coordinates
	//  for post
	topscreen = sprtopscreen + spryscale*column->topdelta;
	bottomscreen = topscreen + spryscale*column->length;

	dc_yl = (topscreen+FRACUNIT-1)>>FRACBITS;
	dc_yh = (bottomscreen-1)>>FRACBITS;
		
	if (dc_yh >= mfloorclip[dc_x])
	    dc_yh = mfloorclip[dc_x]-1;
	if (dc_yl <= mceilingclip[dc_x])
	    dc_yl = mceilingclip[dc_x]+1;

	if (dc_yl <= dc_yh)
	{
#ifdef SMOOTHING
            extern fixed_t dc_xfrac;
            extern byte* dc_source2;
            dc_xfrac = 0;
            dc_source2 =
#endif
	    dc_source = (byte *)column + 3;
	    dc_texturemid = basetexturemid - (column->topdelta<<FRACBITS);
	    // dc_source = (byte *)column + 3 - column->topdelta;

	    // Drawn by either R_DrawColumn
	    //  or (SHADOW) R_DrawFuzzColumn.
	    colfunc ();
	}
	column = (column_t *)(  (byte *)column + column->length + 4);
    }
	
    dc_texturemid = basetexturemid;
}

//
// R_DrawVisSprite
//
// -KM- 1998/11/25 Rewritten to take advantage of Erik's 65535 level trans routines.
//
void R_DrawVisSprite (vissprite_t* vis, int x1, int x2)
{
    column_t*		column;
    int			texturecolumn;
    fixed_t		frac;
    patch_t*		patch;
    int                 trans = 0;
	
    patch = W_CacheLumpNum (vis->patch, PU_CACHE);

    if (vis->colourmaplump >= 0)
      R_SetColourMap(vis->colourmaplump);
    dc_colormap = vis->colormap + colormaps;

    if (vis->colormap < 0)
    {
      // NULL colormap = shadow draw
      trans |= 1;
    }

    if (vis->invisibility < VISIBLE && gameflags.trans)
    {
      trans |= 2;
      dc_translucency = vis->invisibility;
    }

    if (vis->playxtra)
    {
      dc_translation = translationtables - 256 + (vis->playxtra<<8);
      trans |= 4;
    }

    colfunc = R_DrawColumn;
    if (trans&1)
      colfunc = R_DrawFuzzColumn;
    else switch(trans)
    {
        default:
             break;
        case 2:
             colfunc = R_DrawTranslucentColumn;
             break;
        case 4:
             colfunc = R_DrawTranslatedColumn;
             break;
        case 6:
             colfunc = R_DrawTranslucentTranslatedColumn;
             break;
    }
    dc_iscale = abs(vis->xiscale)>>detailshift;
    dc_texturemid = vis->texturemid;
    frac = vis->startfrac;
    spryscale = vis->scale;
    sprtopscreen = centeryfrac - FixedMul(dc_texturemid,spryscale);
	
    for (dc_x=vis->x1 ; dc_x<=vis->x2 ; dc_x++, frac += vis->xiscale)
    {
	texturecolumn = frac>>FRACBITS;

#ifdef DEVELOPERS
	if (texturecolumn < 0 || texturecolumn >= SHORT(patch->width))
	    I_Error ("R_DrawSpriteRange: bad texturecolumn");
#endif

	column = (column_t *) ((byte *)patch +
			       LONG(patch->columnofs[texturecolumn]));
	R_DrawMaskedColumn (column);
    }

    if (vis->colourmaplump >= 0)
      R_SetColourMap(W_GetNumForName("COLORMAP"));

    colfunc = basecolfunc;
}



//
// R_ProjectSprite
//
// Generates a vissprite for a thing if it might be visible.
//
void R_ProjectSprite (mobj_t* thing, int colourmaplump, int colourmap)
{
    fixed_t		tr_x;
    fixed_t		tr_y;
    
    fixed_t		gxt;
    fixed_t		gyt;
    
    fixed_t		tx;
    fixed_t		tz;

    fixed_t		xscale;
    
    int			x1;
    int			x2;

    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    
    unsigned		rot;
    boolean		flip;
    
    int			index;

    vissprite_t*	vis;
    
    angle_t		ang;
    fixed_t		iscale;

    if (!thing->invisibility)
      return; // not visible

    // transform the origin point
    tr_x = thing->x - viewx;
    tr_y = thing->y - viewy;
	
    gxt = FixedMul(tr_x,viewcos); 
    gyt = -FixedMul(tr_y,viewsin);
    
    tz = gxt-gyt; 

    // thing is behind view plane?
    if (tz < MINZ)
	return;
    
    xscale = FixedDiv(projection, tz);
	
    gxt = -FixedMul(tr_x,viewsin); 
    gyt = FixedMul(tr_y,viewcos); 
    tx = -(gyt+gxt); 

    // too far off the side?
    if (abs(tx)>(tz<<2))
	return;
    
    // decide which patch to use for sprite relative to player
#ifdef DEVELOPERS
    if ((unsigned)thing->sprite >= numsprites)
	I_Error ("R_ProjectSprite: invalid sprite number %i ",
		 thing->sprite);
#endif
    sprdef = &sprites[thing->sprite];

// -ACB- 1998/06/29 More Informative Error Message
#ifdef DEVELOPERS
    if ((thing->frame&FF_FRAMEMASK) >= sprdef->numframes)
    {
        if (thing->frame & FF_FULLBRIGHT)
           thing->frame -= FF_FULLBRIGHT;

	I_Error ("R_ProjectSprite: Invalid sprite frame %s:%c",
		 sprnames[thing->sprite], (thing->frame+'A') );
    }
#endif

    sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate)
    {
	// choose a different rotation based on player view
	ang = R_PointToAngle (thing->x, thing->y);
	rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
	lump = sprframe->lump[rot];
	flip = (boolean)sprframe->flip[rot];
    }
    else
    {
	// use single rotation for all views
	lump = sprframe->lump[0];
	flip = (boolean)sprframe->flip[0];
    }
    
    // calculate edges of the shape
    tx -= spriteoffset[lump];	
    x1 = (centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS;

    // off the right side?
    if (x1 > viewwidth)
	return;
    
    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >>FRACBITS) - 1;

    // off the left side
    if (x2 < 0)
	return;
    
    // store information in a vissprite
    vis = &vissprites[R_NewVisSprite (xscale<<detailshift)];

    vis->mobjflags = thing->flags;
    vis->invisibility = thing->invisibility;
    vis->playxtra  = thing->playxtra;
    vis->scale = xscale<<detailshift;
    vis->gx = thing->x;
    vis->gy = thing->y;
    vis->gz = thing->z;
    vis->gzt = thing->z + spritetopoffset[lump];
    vis->texturemid = vis->gzt - viewz;
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    iscale = FixedDiv (FRACUNIT, xscale);

    if (flip)
    {
	vis->startfrac = spritewidth[lump]-1;
	vis->xiscale = -iscale;
    }
    else
    {
	vis->startfrac = 0;
	vis->xiscale = iscale;
    }

    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale*(vis->x1-x1);
    vis->patch = spritelist[lump];
    
    vis->colourmaplump = -1;
    // get light level
    if (thing->flags & MF_SHADOW)
    {
	// shadow draw
	vis->colormap = -1;
    }
    else if (fixedcolormap)
    {
	// fixed map
	vis->colormap = fixedcolormap - colormaps;
    }
    else
    {
      vis->colourmaplump = colourmaplump;

      if (colourmap >= 0)
      {
        vis->colormap = colourmap*256*sizeof(lighttable_t)*BPP;
      }
      else if (thing->frame & FF_FULLBRIGHT)
      {
	// full bright
	vis->colormap = 0;
      }
      else
      {
	// diminished light
	index = xscale>>(LIGHTSCALESHIFT-detailshift);

	if (index >= MAXLIGHTSCALE) 
	    index = MAXLIGHTSCALE-1;

	vis->colormap = spritelights[index];
      }
    }
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
void R_AddSprites (sector_t* sec)
{
    mobj_t*		thing;
    int			lightnum;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
	return;		

    // Well, now it will be done.
    sec->validcount = validcount;
	
    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT)+extralight;

    if (lightnum < 0)		
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS-1];
    else
	spritelights = scalelight[lightnum];

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
	R_ProjectSprite (thing, sec->colourmaplump, sec->colourmap);
}


//
// R_DrawPSprite
//
// -ACB- 1998/09/02 Fixed Error Messages to display correct procedure.
//
void R_DrawPSprite (pspdef_t* psp, int colourmaplump, int colourmap)
{
    fixed_t		tx;
    int			x1;
    int			x2;
    spritedef_t*	sprdef;
    spriteframe_t*	sprframe;
    int			lump;
    boolean		flip;
    vissprite_t*        vis;
    static vissprite_t  avis; // -ACB- 1998/09/02 Made Static
    
    // decide which patch to use

#ifdef DEVELOPERS
    if ( (unsigned)psp->state->sprite >= numsprites)
	I_Error ("R_DrawPSprite: invalid sprite number %i ",
		 psp->state->sprite);
#endif

    sprdef = &sprites[psp->state->sprite];

#ifdef DEVELOPERS
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
	I_Error ("R_DrawPSprite: invalid sprite frame %d : %ld ",
		 psp->state->sprite, psp->state->frame);
#endif

    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];
    
    // calculate edges of the shape
    tx = psp->sx-160*FRACUNIT;
	
    tx -= spriteoffset[lump];	
    x1 = (centerxfrac + FixedMul (tx,pspritescale) ) >>FRACBITS;

    // off the right side
    if (x1 > viewwidth)
	return;		

    tx +=  spritewidth[lump];
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) -1;

    // off the left side
    if (x2 < 0)
	return;
    
    // store information in a vissprite
    vis = &avis;

    // -ACB- 1998/09/02 Clear the vissprite mem before setup. Safety thing..
    memset(vis,0,sizeof(vissprite_t));

    vis->texturemid = BASEYCENTER-(psp->sy-spritetopoffset[lump]);
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;	
    vis->scale = pspritescale<<detailshift;
    
    if (flip)
    {
	vis->xiscale = -pspriteiscale;
	vis->startfrac = spritewidth[lump]-1;
    }
    else
    {
	vis->xiscale = pspriteiscale;
	vis->startfrac = 0;
    }
    
    if (vis->x1 > x1)
	vis->startfrac += vis->xiscale*(vis->x1-x1);

    vis->patch = spritelist[lump];
    vis->colourmaplump = -1;
    vis->invisibility = viewplayer->mo->invisibility;

    if (viewplayer->powers[pw_invisibility] > 4*32
	|| viewplayer->powers[pw_invisibility] & 8)
    {
	// shadow draw
	vis->colormap = -1;
    }
    else if (fixedcolormap)
    {
	// fixed color
	vis->colormap = fixedcolormap - colormaps;
    }
    else
    {
      vis->colourmaplump = colourmaplump;

      if (colourmap >= 0)
      {
        vis->colormap = colourmap*256*sizeof(lighttable_t)*BPP;
      }
      else if (psp->state->frame & FF_FULLBRIGHT)
      {
  	// full bright
  	vis->colormap = 0;
      }
      else
      {
  	// local light
  	vis->colormap = spritelights[MAXLIGHTSCALE-1];
      }
    }

    R_DrawVisSprite (vis, vis->x1, vis->x2);

}



//
// R_DrawPlayerSprites
//
void R_DrawPlayerSprites (void)
{
    int		i;
    int		lightnum;
    pspdef_t*	psp;
    
    // get light level
    lightnum =
	(viewplayer->mo->subsector->sector->lightlevel >> LIGHTSEGSHIFT) 
	+extralight;

    if (lightnum < 0)		
	spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
	spritelights = scalelight[LIGHTLEVELS-1];
    else
	spritelights = scalelight[lightnum];
    
    // clip to screen bounds
    mfloorclip = screenheightarray;
    mceilingclip = negonearray;

    // add all active psprites
    for (i=0, psp=viewplayer->psprites;
	 i<NUMPSPRITES;
	 i++,psp++)
    {
	if (psp->state)
	    R_DrawPSprite (psp, viewplayer->mo->subsector->sector->colourmaplump,
                           viewplayer->mo->subsector->sector->colourmap);
    }

    
}



/*
//
// R_SortVisSprites
//
vissprite_t	vsprsortedhead;


void R_SortVisSprites (void)
{
    int			i;
    int			count;
    vissprite_t*	ds;
    vissprite_t*	best = NULL;
    vissprite_t		unsorted;
    fixed_t		bestscale;

    count = vissprite_p;
	
    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
	return;
		
    for (ds=vissprites ; ds < &vissprites[vissprite_p] ; ds++)
    {
	ds->next = ds+1;
	ds->prev = ds-1;
    }
    
    vissprites[0].prev = &unsorted;
    unsorted.next = &vissprites[0];
    vissprites[vissprite_p-1].next = &unsorted;
    unsorted.prev = &vissprites[vissprite_p-1];
    
    // pull the vissprites out by scale
    //best = 0;		// shut up the compiler warning
    vsprsortedhead.next = vsprsortedhead.prev = &vsprsortedhead;
    for (i=0 ; i<count ; i++)
    {
	bestscale = MAXINT;
	for (ds=unsorted.next ; ds!= &unsorted ; ds=ds->next)
	{
	    if (ds->scale < bestscale)
	    {
		bestscale = ds->scale;
		best = ds;
	    }
	}
	best->next->prev = best->prev;
	best->prev->next = best->next;
	best->next = &vsprsortedhead;
	best->prev = vsprsortedhead.prev;
	vsprsortedhead.prev->next = best;
	vsprsortedhead.prev = best;
    }
}

*/

//
// R_DrawSprite
// 23-6-98 KM Reworked ds into neater code
void R_DrawSprite (vissprite_t* spr)
{
    drawseg_t*		ds;
    int			ds_index;
    short		clipbot[SCREENWIDTH];
    short		cliptop[SCREENWIDTH];
    int			x;
    int			r1;
    int			r2;
    fixed_t		scale;
    fixed_t		lowscale;
    int			silhouette;
		
    for (x = spr->x1 ; x<=spr->x2 ; x++)
	clipbot[x] = cliptop[x] = -2;
    
    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale
    //  is the clip seg.
    for (ds_index = ds_p; ds_index--;)
    {
        ds = &drawsegs[ds_index];
	// determine if the drawseg obscures the sprite
	if (ds->x1 > spr->x2
	    || ds->x2 < spr->x1
	    || (!ds->silhouette
		&& !ds->maskedtexturecol) )
	{
	    // does not cover sprite
	    continue;
	}
			
	r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
	r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

	if (ds->scale1 > ds->scale2)
	{
	    lowscale = ds->scale2;
	    scale = ds->scale1;
	}
	else
	{
	    lowscale = ds->scale1;
	    scale = ds->scale2;
	}
		
	if (scale < spr->scale
	    || ( lowscale < spr->scale
		 && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline) ) )
	{
	    // masked mid texture?
	    if (ds->maskedtexturecol)	
		R_RenderMaskedSegRange (ds, r1, r2);
	    // seg is behind sprite
	    continue;			
	}

	
	// clip this piece of the sprite
	silhouette = ds->silhouette;
	
	if (spr->gz >= ds->bsilheight)
	    silhouette &= ~SIL_BOTTOM;

	if (spr->gzt <= ds->tsilheight)
	    silhouette &= ~SIL_TOP;
			
	if (silhouette == 1)
	{
	    // bottom sil
	    for (x=r1 ; x<=r2 ; x++)
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
	}
	else if (silhouette == 2)
	{
	    // top sil
	    for (x=r1 ; x<=r2 ; x++)
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	}
	else if (silhouette == 3)
	{
	    // both
	    for (x=r1 ; x<=r2 ; x++)
	    {
		if (clipbot[x] == -2)
		    clipbot[x] = ds->sprbottomclip[x];
		if (cliptop[x] == -2)
		    cliptop[x] = ds->sprtopclip[x];
	    }
	}
		
    }
    
    // all clipping has been performed, so draw the sprite

    // check for unclipped columns
    for (x = spr->x1 ; x<=spr->x2 ; x++)
    {
	if (clipbot[x] == -2)		
	    clipbot[x] = viewheight;

	if (cliptop[x] == -2)
	    cliptop[x] = -1;
    }
		
    mfloorclip = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr, spr->x1, spr->x2);
}




//
// R_DrawMasked
// 23-6-98 KM Reworked ds into neater code
//
void R_DrawMasked (void)
{
    drawseg_t*		ds;
    int			ds_index;

    fixed_t             oldcentery;
    fixed_t             oldcenteryfrac;
	
//    R_SortVisSprites ();

    if (vissprite_p)
    {
	// draw all vissprites back to front
/*	for (spr = vsprsortedhead.next ;
	     spr != &vsprsortedhead ;
	     spr=spr->next)
	{
            R_DrawSprite (spr);
	}*/
        int i;
        for (i = 0; i < vissprite_p; i++)
           R_DrawSprite (&vissprites[visspriteq[i]]);
    }
    
    // render any remaining masked mid textures
    for (ds_index = ds_p; ds_index--;)
    {
        ds = &drawsegs[ds_index];
	if (ds->maskedtexturecol)
	    R_RenderMaskedSegRange (ds, ds->x1, ds->x2);
    }

    //
    // draw the psprites on top of everything
    //  but does not draw on side views
    //
    // -ACB- 1998/07/03 When using Vertical Look, player sprites are unaffected
    //                  restore to normalized values
    //
    // -ACB- 1998/07/19 Fixed problem with flicking weapons: values restored
    //                  to mlook adjusted values
    //
    if (!gameflags.viewangleoffset)
    {
        oldcentery=centery;
        oldcenteryfrac=centeryfrac;

        centery=(viewheight/2);
        centeryfrac=centery<<FRACBITS;

        R_DrawPlayerSprites ();

        centery=oldcentery;
        centeryfrac=oldcenteryfrac;

    }
}



