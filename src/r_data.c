/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 * $Id: r_data.c,v 1.13 2000/01/25 22:40:45 cphipps Exp $
 *
 *  LxDoom, a Doom port for Linux/Unix
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *   and Colin Phipps
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
 *      Preparation of data for rendering,
 *      generation of lookups, caching, retrieval by name.
 *
 *-----------------------------------------------------------------------------*/

static const char
rcsid[] = "$Id: r_data.c,v 1.13 2000/01/25 22:40:45 cphipps Exp $";

#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_sky.h"
#include "lprintf.h"  // jff 08/03/98 - declaration of lprintf

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
//

//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//

typedef struct
{
  short originx;
  short originy;
  short patch;
  short stepdir;         // unused in Doom but might be used in Phase 2 Boom
  short colormap;        // unused in Doom but might be used in Phase 2 Boom
} __attribute__((packed)) mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
  char       name[8];
  boolean    masked;
  short      width;
  short      height;
  char       pad[4];       // unused in Doom but might be used in Boom Phase 2
  short      patchcount;
  mappatch_t patches[1];
} __attribute__((packed)) maptexture_t;


// A single patch from a texture definition, basically
// a rectangular area within the texture rectangle.
typedef struct
{
  int originx, originy;  // Block origin, which has already accounted
  int patch;             // for the internal origin of the patch.
} __attribute__((packed)) texpatch_t;


// A maptexturedef_t describes a rectangular texture, which is composed
// of one or more mappatch_t structures that arrange graphic patches.

typedef struct
{
  char  name[8];         // Keep name for switch changing, etc.
  int   next, index;     // killough 1/31/98: used in hashing algorithm
  // CPhipps - moved arrays with per-texture entries to elements here
  unsigned  widthmask;
  size_t    compositesize;
  byte     *composite;
  short    *columnlump;
  unsigned *columnofs;
  // CPhipps - end of additions
  short width, height;
  short patchcount;      // All the patches[patchcount] are drawn
  texpatch_t patches[1]; // back-to-front into the cached texture.
} texture_t;


// killough 4/17/98: make firstcolormaplump,lastcolormaplump external
int firstcolormaplump, lastcolormaplump;      // killough 4/17/98

int       firstflat, lastflat, numflats;
int       firstspritelump, lastspritelump, numspritelumps;
int       numtextures;
static texture_t **textures;
fixed_t   *textureheight; //needed for texture pegging (and TFE fix - killough)
int       *flattranslation;             // for global animation
int       *texturetranslation;

// needed for pre-rendering
fixed_t   *spritewidth, *spriteoffset, *spritetopoffset;

//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//

//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//

void R_DrawColumnInCache(const column_t *patch, byte *cache,
                         int originy, int cacheheight, byte *marks)
{
  while (patch->topdelta != 0xff)
    {
      int count = patch->length;
      int position = originy + patch->topdelta;

      if (position < 0)
        {
          count += position;
          position = 0;
        }

      if (position + count > cacheheight)
        count = cacheheight - position;

      if (count > 0)
        {
          memcpy (cache + position, (byte *)patch + 3, count);

          // killough 4/9/98: remember which cells in column have been drawn,
          // so that column can later be converted into a series of posts, to
          // fix the Medusa bug.

          memset (marks + position, 0xff, count);
        }

      patch = (column_t *)((byte *) patch + patch->length + 4);
    }
}

//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
// Rewritten by Lee Killough for performance and to fix Medusa bug

void R_GenerateComposite(int texnum)
{
  texture_t *texture = textures[texnum];
  byte *block = Z_Malloc(texture->compositesize, PU_STATIC,
                         (void **)&texture->composite);
  // Composite the columns together.
  texpatch_t *patch = texture->patches;
  short *collump = texture->columnlump;
  unsigned *colofs = texture->columnofs; // killough 4/9/98: make 32-bit
  int i = texture->patchcount;
  // killough 4/9/98: marks to identify transparent regions in merged textures
  byte *marks = calloc(texture->width, texture->height), *source;

  for (; --i >=0; patch++)
    {
      const patch_t *realpatch = W_CacheLumpNum(patch->patch); // cph
      int x1 = patch->originx, x2 = x1 + SHORT(realpatch->width);
      const int *cofs = realpatch->columnofs-x1;
      if (x1<0)
        x1 = 0;
      if (x2 > texture->width)
        x2 = texture->width;
      for (; x1<x2 ; x1++)
        if (collump[x1] == -1)      // Column has multiple patches?
          // killough 1/25/98, 4/9/98: Fix medusa bug.
          R_DrawColumnInCache((column_t*)((byte*)realpatch+LONG(cofs[x1])),
                              block+colofs[x1],patch->originy,texture->height,
                              marks + x1 * texture->height);

      W_UnlockLumpNum(patch->patch); // cph - unlock the patch lump
    }

  // killough 4/9/98: Next, convert multipatched columns into true columns,
  // to fix Medusa bug while still allowing for transparent regions.

  source = malloc(texture->height);       // temporary column
  for (i=0; i < texture->width; i++)
    if (collump[i] == -1)                 // process only multipatched columns
      {
        column_t *col = (column_t *)(block + colofs[i] - 3);  // cached column
        const byte *mark = marks + i * texture->height;
        int j = 0;

        // save column in temporary so we can shuffle it around
        memcpy(source, (byte *) col + 3, texture->height);

        for (;;)  // reconstruct the column by scanning transparency marks
          {
            while (j < texture->height && !mark[j]) // skip transparent cells
              j++;
            if (j >= texture->height)           // if at end of column
              {
                col->topdelta = -1;             // end-of-column marker
                break;
              }
            col->topdelta = j;                  // starting offset of post
            for (col->length=0; j < texture->height && mark[j]; j++)
              col->length++;                    // count opaque cells
            // copy opaque cells from the temporary back into the column
            memcpy((byte *) col + 3, source + col->topdelta, col->length);
            col = (column_t *)((byte *) col + col->length + 4); // next post
          }
      }
  free(source);         // free temporary column
  free(marks);          // free transparency marks

  // Now that the texture has been built in column cache,
  // it is purgable from zone memory.

  Z_ChangeTag(block, PU_CACHE);
}

//
// R_GenerateLookup
//
// Rewritten by Lee Killough for performance and to fix Medusa bug
//

static void R_GenerateLookup(int texnum, int *const errors)
{
  texture_t *texture = textures[texnum];

  // killough 4/9/98: make column offsets 32-bit;
  // clean up malloc-ing to use sizeof
  // CPhipps - moved allocing here
  short *collump = texture->columnlump = 
    Z_Malloc(texture->width*sizeof(*texture->columnlump), PU_STATIC,0);
  unsigned *colofs = texture->columnofs = 
    Z_Malloc(texture->width*sizeof(*texture->columnofs), PU_STATIC,0); 

  // killough 4/9/98: keep count of posts in addition to patches.
  // Part of fix for medusa bug for multipatched 2s normals.

  struct {
    unsigned short patches, posts;
  } *count = calloc(sizeof *count, texture->width);

  {
    int i = texture->patchcount;
    const texpatch_t *patch = texture->patches;

    while (--i >= 0)
      {
        int pat = patch->patch;
        const patch_t *realpatch = W_CacheLumpNum(pat);
        int x1 = patch++->originx, x2 = x1 + SHORT(realpatch->width), x = x1;
        const int *cofs = realpatch->columnofs-x1;

        if (x2 > texture->width)
          x2 = texture->width;
        if (x1 < 0)
          x = 0;
        for ( ; x<x2 ; x++)
          {
            // killough 4/9/98: keep a count of the number of posts in column,
            // to fix Medusa bug while allowing for transparent multipatches.

            const column_t *col = (column_t*)((byte*)realpatch+LONG(cofs[x]));
            for (;col->topdelta != 0xff; count[x].posts++)
              col = (column_t *)((byte *) col + col->length + 4);
            count[x].patches++;
            collump[x] = pat;
            colofs[x] = LONG(cofs[x])+3;
          }

	W_UnlockLumpNum(pat);
      }
  }

  // Composited texture not created yet.
  texture->composite = NULL;

  // Now count the number of columns
  //  that are covered by more than one patch.
  // Fill in the lump / offset, so columns
  //  with only a single patch are all done.

  {
    int x = texture->width;
    int height = texture->height;
    int csize = 0;

    while (--x >= 0)
      {
        if (!count[x].patches)          // killough 4/9/98
          {
            //jff 8/3/98 use logical output routine
            lprintf(LO_WARN,
                    "\nR_GenerateLookup: Column %d is without a patch in texture %.8s",
                    x, texture->name);
            if (errors) ++*errors;
	    else I_Error("R_GenerateLookup failed");
          }
        if (count[x].patches > 1)       // killough 4/9/98
          {
            // killough 1/25/98, 4/9/98:
            //
            // Fix Medusa bug, by adding room for column header
            // and trailer bytes for each post in merged column.
            // For now, just allocate conservatively 4 bytes
            // per post per patch per column, since we don't
            // yet know how many posts the merged column will
            // require, and it's bounded above by this limit.

            collump[x] = -1;              // mark lump as multipatched
            colofs[x] = csize + 3;        // three header bytes in a column
            csize += 4*count[x].posts+1;  // 1 stop byte plus 4 bytes per post
          }
        csize += height;                  // height bytes of texture data
      }
    texture->compositesize = csize;
  }
  free(count);                    // killough 4/9/98
}

//
// R_GetColumn
//

const byte *R_GetColumn(int tex, int col)
{
  const texture_t *texture = textures[tex];
  if (!texture->columnlump) R_GenerateLookup(tex, NULL);
  {
  int lump = texture->columnlump[col &= texture->widthmask];
  int ofs  = texture->columnofs[col]; // cph - WARNING: must be after the above line
  // cph - remember the last lump, so we can unlock it if no longer needed, 
  //  or reuse it if possible to reduce lump locking/unlocking
  static int lastlump = -1;
  static const byte* lastlumpdata;

  if ((lump<=0) && (lastlump<=0))
    lump = lastlump; // cph - force equal

  if (lump != lastlump) {
    // cph - must change the cached lump
    if (lastlump>0)
      W_UnlockLumpNum(lastlump);

    if ((lastlump = lump) > 0)
      lastlumpdata = W_CacheLumpNum(lump);
#ifdef RANGECHECK
    else
      lastlumpdata = NULL;
#endif
  }

  if (lump > 0)
    return lastlumpdata + ofs;

  if (!texture->composite)
    R_GenerateComposite(tex);

  return texture->composite + ofs;
  }
}

//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//

void R_InitTextures (void)
{
  maptexture_t *mtexture;
  texture_t    *texture;
  mappatch_t   *mpatch;
  texpatch_t   *patch;
  int  i, j;
  int         maptex_lump[2] = {-1, -1};
  const int  *maptex;
  const int  *maptex1, *maptex2;
  char name[9];
  int names_lump; // cph - new wad lump handling
  const char *names; // cph - 
  const char *name_p;// const*'s
  int  *patchlookup;
  int  totalwidth;
  int  nummappatches;
  int  offset;
  int  maxoff, maxoff2;
  int  numtextures1, numtextures2;
  const int *directory;
  int  errors = 0;

  // Load the patch names from pnames.lmp.
  name[8] = 0;
  names = W_CacheLumpNum(names_lump = W_GetNumForName("PNAMES"));
  nummappatches = LONG(*((const int *)names));
  name_p = names+4;
  patchlookup = malloc(nummappatches*sizeof(*patchlookup));  // killough

  for (i=0 ; i<nummappatches ; i++)
    {
      strncpy (name,name_p+i*8, 8);
      patchlookup[i] = W_CheckNumForName(name);
      if (patchlookup[i] == -1)
        {
          // killough 4/17/98:
          // Some wads use sprites as wall patches, so repeat check and
          // look for sprites this time, but only if there were no wall
          // patches found. This is the same as allowing for both, except
          // that wall patches always win over sprites, even when they
          // appear first in a wad. This is a kludgy solution to the wad
          // lump namespace problem.

          patchlookup[i] = (W_CheckNumForName)(name, ns_sprites);

          if (patchlookup[i] == -1 && devparm)
            //jff 8/3/98 use logical output routine
            lprintf(LO_WARN,"\nWarning: patch %.8s, index %d does not exist",name,i);
        }
    }
  W_UnlockLumpNum(names_lump); // cph - release the lump

  // Load the map texture definitions from textures.lmp.
  // The data is contained in one or two lumps,
  //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.

  maptex = maptex1 = W_CacheLumpNum(maptex_lump[0] = W_GetNumForName("TEXTURE1"));
  numtextures1 = LONG(*maptex);
  maxoff = W_LumpLength(maptex_lump[0]);
  directory = maptex+1;

  if (W_CheckNumForName("TEXTURE2") != -1)
    {
      maptex2 = W_CacheLumpNum(maptex_lump[1] = W_GetNumForName("TEXTURE2"));
      numtextures2 = LONG(*maptex2);
      maxoff2 = W_LumpLength(maptex_lump[1]);
    }
  else
    {
      maptex2 = NULL;
      numtextures2 = 0;
      maxoff2 = 0;
    }
  numtextures = numtextures1 + numtextures2;

  // killough 4/9/98: make column offsets 32-bit;
  // clean up malloc-ing to use sizeof

  textures = Z_Malloc(numtextures*sizeof*textures, PU_STATIC, 0);
  textureheight = Z_Malloc(numtextures*sizeof*textureheight, PU_STATIC, 0);

  totalwidth = 0;

  for (i=0 ; i<numtextures ; i++, directory++)
    {
      if (i == numtextures1)
        {
          // Start looking in second texture file.
          maptex = maptex2;
          maxoff = maxoff2;
          directory = maptex+1;
        }

      offset = LONG(*directory);

      if (offset > maxoff)
        I_Error("R_InitTextures: bad texture directory");

      mtexture = (maptexture_t *) ( (byte *)maptex + offset);

      texture = textures[i] =
        Z_Malloc(sizeof(texture_t) +
                 sizeof(texpatch_t)*(SHORT(mtexture->patchcount)-1),
                 PU_STATIC, 0);

      texture->width = SHORT(mtexture->width);
      texture->height = SHORT(mtexture->height);
      texture->patchcount = SHORT(mtexture->patchcount);

        /* Mattias Engdeg�rd emailed me of the following explenation of
         * why memcpy doesnt work on some systems:
         * "I suppose it is the mad unaligned allocation
         * going on (and which gcc in some way manages to cope with
         * through the __attribute__ ((packed))), and which it forgets
         * when optimizing memcpy (to a single word move) since it appears
         * to be aligned. Technically a gcc bug, but I can't blame it when
         * it's stressed with that amount of
         * non-standard nonsense."
	 * So in short the unaligned struct confuses gcc's optimizer so
	 * i took the memcpy out alltogether to avoid future problems-Jess
         */
      { 
	 int j; 
	 for(j=0;j<sizeof(texture->name);j++) 
	    texture->name[j]=mtexture->name[j]; 
      }

      mpatch = mtexture->patches;
      patch = texture->patches;

      for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
        {
          patch->originx = SHORT(mpatch->originx);
          patch->originy = SHORT(mpatch->originy);
          patch->patch = patchlookup[SHORT(mpatch->patch)];
          if (patch->patch == -1)
            {
              //jff 8/3/98 use logical output routine
              lprintf(LO_ERROR,"\nR_InitTextures: Missing patch %d in texture %.8s",
                     SHORT(mpatch->patch), texture->name); // killough 4/17/98
              ++errors;
            }
        }

      texture->columnofs = NULL; texture->columnlump = NULL;

      for (j=1; j*2 <= texture->width; j<<=1)
        ;
      texture->widthmask = j-1;
      textureheight[i] = texture->height<<FRACBITS;

      totalwidth += texture->width;
    }
 
  free(patchlookup);         // killough

  for (i=0; i<2; i++) // cph - release the TEXTUREx lumps
    if (maptex_lump[i] != -1)
      W_UnlockLumpNum(maptex_lump[i]);

  if (errors)
    I_Error("\n\n%d errors.", errors);
    
  // Precalculate whatever possible.
  if (devparm) // cph - If in development mode, generate now so all errors are found at once
    for (i=0 ; i<numtextures ; i++)
      R_GenerateLookup(i, &errors);

  if (errors)
    I_Error("\n\n%d errors.", errors);

  // Create translation table for global animation.
  // killough 4/9/98: make column offsets 32-bit;
  // clean up malloc-ing to use sizeof

  texturetranslation =
    Z_Malloc((numtextures+1)*sizeof*texturetranslation, PU_STATIC, 0);

  for (i=0 ; i<numtextures ; i++)
    texturetranslation[i] = i;

  // killough 1/31/98: Initialize texture hash table
  for (i = 0; i<numtextures; i++)
    textures[i]->index = -1;
  while (--i >= 0)
    {
      int j = W_LumpNameHash(textures[i]->name) % (unsigned) numtextures;
      textures[i]->next = textures[j]->index;   // Prepend to chain
      textures[j]->index = i;
    }
}

//
// R_InitFlats
//
void R_InitFlats(void)
{
  int i;

  firstflat = W_GetNumForName("F_START") + 1;
  lastflat  = W_GetNumForName("F_END") - 1;
  numflats  = lastflat - firstflat + 1;

  // Create translation table for global animation.
  // killough 4/9/98: make column offsets 32-bit;
  // clean up malloc-ing to use sizeof

  flattranslation =
    Z_Malloc((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0);

  for (i=0 ; i<numflats ; i++)
    flattranslation[i] = i;
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
// so the sprite does not need to be cached completely
// just for having the header info ready during rendering.
//
void R_InitSpriteLumps(void)
{
  int i;
  const patch_t *patch;

  firstspritelump = W_GetNumForName("S_START") + 1;
  lastspritelump = W_GetNumForName("S_END") - 1;
  numspritelumps = lastspritelump - firstspritelump + 1;

  // killough 4/9/98: make columnd offsets 32-bit;
  // clean up malloc-ing to use sizeof

  spritewidth = Z_Malloc(numspritelumps*sizeof*spritewidth, PU_STATIC, 0);
  spriteoffset = Z_Malloc(numspritelumps*sizeof*spriteoffset, PU_STATIC, 0);
  spritetopoffset =
    Z_Malloc(numspritelumps*sizeof*spritetopoffset, PU_STATIC, 0);

  for (i=0 ; i< numspritelumps ; i++)
    {
      patch = W_CacheLumpNum(firstspritelump+i);
      spritewidth[i] = SHORT(patch->width)<<FRACBITS;
      spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
      spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
      W_UnlockLumpNum(firstspritelump+i);
    }
}

//
// R_InitColormaps
//
// killough 3/20/98: rewritten to allow dynamic colormaps
// and to remove unnecessary 256-byte alignment
//
// killough 4/4/98: Add support for C_START/C_END markers
//

// CPhipps - reinstate 256-byte alignment of colourmaps for I386 targets
static void* R_GetColourmaps(int lump)
{
#ifdef I386
  // Load in the light tables, 
  //  256 byte align tables.
  void  *colormaps;
  size_t length = W_LumpLength (lump) + 255; 
  colormaps = Z_Malloc (length, PU_STATIC, 0); 
  colormaps = (byte *)( ((int)colormaps + 255)&~0xff); 
  W_ReadLump (lump,colormaps); 
  return colormaps;
#else
  return W_CacheLumpNum(lump);
#endif
}
void R_InitColormaps(void)
{
  int i;
  firstcolormaplump = W_GetNumForName("C_START");
  lastcolormaplump  = W_GetNumForName("C_END");
  numcolormaps = lastcolormaplump - firstcolormaplump;
  colormaps = Z_Malloc(sizeof(*colormaps) * numcolormaps, PU_STATIC, 0);
  colormaps[0] = R_GetColourmaps(W_GetNumForName("COLORMAP"));
  for (i=1; i<numcolormaps; i++)
    colormaps[i] = R_GetColourmaps(i+firstcolormaplump); 
  // cph - always lock
}

// killough 4/4/98: get colormap number from name
// killough 4/11/98: changed to return -1 for illegal names
// killough 4/17/98: changed to use ns_colormaps tag

int R_ColormapNumForName(const char *name)
{
  register int i = 0;
  if (strncasecmp(name,"COLORMAP",8))     // COLORMAP predefined to return 0
    if ((i = (W_CheckNumForName)(name, ns_colormaps)) != -1)
      i -= firstcolormaplump;
  return i;
}

//
// R_InitTranMap
//
// Initialize translucency filter map
//
// By Lee Killough 2/21/98
//

int tran_filter_pct = 66;       // filter percent

#define TSC 12        /* number of fixed point digits in filter percent */

void R_InitTranMap(int progress)
{
  int lump = W_CheckNumForName("TRANMAP");

  // If a tranlucency filter map lump is present, use it

  if (lump != -1)  // Set a pointer to the translucency filter maps.
    main_tranmap = W_CacheLumpNum(lump);   // killough 4/11/98
  else
    {   // Compose a default transparent filter map based on PLAYPAL.
      const byte *playpal = W_CacheLumpName("PLAYPAL");
      byte       *my_tranmap;

      char fname[PATH_MAX+1], *D_DoomExeDir(void);
      struct {
        unsigned char pct;
        unsigned char playpal[256];
      } cache;
      FILE *cachefp = fopen(strcat(strcpy(fname, D_DoomExeDir()),
                                   "/tranmap.dat"),"r+b");

      main_tranmap = my_tranmap = Z_Malloc(256*256, PU_STATIC, 0);  // killough 4/11/98

      // Use cached translucency filter if it's available

      if (!cachefp ? cachefp = fopen(fname,"wb") , 1 :
          fread(&cache, 1, sizeof cache, cachefp) != sizeof cache ||
          cache.pct != tran_filter_pct ||
          memcmp(cache.playpal, playpal, sizeof cache.playpal) ||
          fread(my_tranmap, 256, 256, cachefp) != 256 ) // killough 4/11/98
        {
          long pal[3][256], tot[256], pal_w1[3][256];
          long w1 = ((unsigned long) tran_filter_pct<<TSC)/100;
          long w2 = (1l<<TSC)-w1;

	  if (progress)
	    lprintf(LO_INFO, "Tranmap build [        ]\x08\x08\x08\x08\x08\x08\x08\x08\x08");

          // First, convert playpal into long int type, and transpose array,
          // for fast inner-loop calculations. Precompute tot array.

          {
            register int i = 255;
            register const unsigned char *p = playpal+255*3;
            do
              {
                register long t,d;
                pal_w1[0][i] = (pal[0][i] = t = p[0]) * w1;
                d = t*t;
                pal_w1[1][i] = (pal[1][i] = t = p[1]) * w1;
                d += t*t;
                pal_w1[2][i] = (pal[2][i] = t = p[2]) * w1;
                d += t*t;
                p -= 3;
                tot[i] = d << (TSC-1);
              }
            while (--i>=0);
          }

          // Next, compute all entries using minimum arithmetic.

          {
            int i,j;
            byte *tp = my_tranmap;
            for (i=0;i<256;i++)
              {
                long r1 = pal[0][i] * w2;
                long g1 = pal[1][i] * w2;
                long b1 = pal[2][i] * w2;
                if (!(i & 31) && progress)
                  //jff 8/3/98 use logical output routine
                  lprintf(LO_INFO,".");
                for (j=0;j<256;j++,tp++)
                  {
                    register int color = 255;
                    register long err;
                    long r = pal_w1[0][j] + r1;
                    long g = pal_w1[1][j] + g1;
                    long b = pal_w1[2][j] + b1;
                    long best = LONG_MAX;
                    do
                      if ((err = tot[color] - pal[0][color]*r
                          - pal[1][color]*g - pal[2][color]*b) < best)
                        best = err, *tp = color;
                    while (--color >= 0);
                  }
              }
          }
          if (cachefp)        // write out the cached translucency map
            {
              cache.pct = tran_filter_pct;
              memcpy(cache.playpal, playpal, 256);
              fseek(cachefp, 0, SEEK_SET);
              fwrite(&cache, 1, sizeof cache, cachefp);
              fwrite(main_tranmap, 256, 256, cachefp);
	      // CPhipps - leave close for a few lines...
            }
        }

      if (cachefp)              // killough 11/98: fix filehandle leak
        fclose(cachefp);
 
      W_UnlockLumpName("PLAYPAL");
    }
}

//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//

void R_InitData(void)
{
  lprintf(LO_INFO, "Textures ");
  R_InitTextures();
  lprintf(LO_INFO, "Flats ");
  R_InitFlats();
  lprintf(LO_INFO, "Sprites ");
  R_InitSpriteLumps();
  if (default_translucency)             // killough 3/1/98
    R_InitTranMap(1);                   // killough 2/21/98, 3/6/98
  R_InitColormaps();                    // killough 3/20/98
}

//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
// killough 4/17/98: changed to use ns_flats namespace
//

int R_FlatNumForName(const char *name)    // killough -- const added
{
  int i = (W_CheckNumForName)(name, ns_flats);
  if (i == -1)
    I_Error("R_FlatNumForName: %.8s not found", name);
  return i - firstflat;
}

//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
// Rewritten by Lee Killough to use hash table for fast lookup. Considerably
// reduces the time needed to start new levels. See w_wad.c for comments on
// the hashing algorithm, which is also used for lump searches.
//
// killough 1/21/98, 1/31/98
//

int R_CheckTextureNumForName(const char *name)
{
  int i = 0;
  if (*name != '-')     // "NoTexture" marker.
    {
      i = textures[W_LumpNameHash(name) % (unsigned) numtextures]->index;
      while (i >= 0 && strncasecmp(textures[i]->name,name,8))
        i = textures[i]->next;
    }
  return i;
}

//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//

int R_TextureNumForName(const char *name)  // const added -- killough
{
  int i = R_CheckTextureNumForName(name);
  if (i == -1)
    I_Error("R_TextureNumForName: %.8s not found", name);
  return i;
}

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
// Totally rewritten by Lee Killough to use less memory,
// to avoid using alloca(), and to improve performance.
// cph - new wad lump handling, calls cache functions but acquires no locks

void R_PrecacheLevel(void)
{
  register int i;
  register byte *hitlist;

  if (demoplayback)
    return;

  {
    size_t size = numflats > numsprites  ? numflats : numsprites;
    hitlist = malloc(numtextures > size ? numtextures : size);
  }

  // Precache flats.

  memset(hitlist, 0, numflats);

  for (i = numsectors; --i >= 0; )
    hitlist[sectors[i].floorpic] = hitlist[sectors[i].ceilingpic] = 1;

  for (i = numflats; --i >= 0; )
    if (hitlist[i])
      (W_CacheLumpNum)(firstflat + i, 0);

  // Precache textures.

  memset(hitlist, 0, numtextures);

  for (i = numsides; --i >= 0;)
    hitlist[sides[i].bottomtexture] =
      hitlist[sides[i].toptexture] =
      hitlist[sides[i].midtexture] = 1;

  // Sky texture is always present.
  // Note that F_SKY1 is the name used to
  //  indicate a sky floor/ceiling as a flat,
  //  while the sky texture is stored like
  //  a wall texture, with an episode dependend
  //  name.

  hitlist[skytexture] = 1;

  for (i = numtextures; --i >= 0; )
    if (hitlist[i])
      {
        texture_t *texture = textures[i];
        int j = texture->patchcount;
        while (--j >= 0)
          (W_CacheLumpNum)(texture->patches[j].patch, 0);
      }

  // Precache sprites.
  memset(hitlist, 0, numsprites);

  {
    thinker_t *th;
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
      if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        hitlist[((mobj_t *)th)->sprite] = 1;
  }

  for (i=numsprites; --i >= 0;)
    if (hitlist[i])
      {
        int j = sprites[i].numframes;
        while (--j >= 0)
          {
            short *sflump = sprites[i].spriteframes[j].lump;
            int k = 7;
            do
              (W_CacheLumpNum)(firstspritelump + sflump[k], 0);
            while (--k >= 0);
          }
      }
  free(hitlist);
}

//-----------------------------------------------------------------------------
//
// $Log: r_data.c,v $
// Revision 1.13  2000/01/25 22:40:45  cphipps
// Revert SPARC memcpy patch
//
// Revision 1.12  1999/10/27 18:38:14  cphipps
// Made W_CacheLump* return a const pointer
//
// Revision 1.11  1999/10/12 13:01:14  cphipps
// Changed header to GPL
//
// Revision 1.10  1999/09/06 19:38:03  cphipps
// Struct packing and other portability stuff, by Josh Parsons <josh@schlick.anu.edu.au>
//
// Revision 1.9  1999/09/05 13:34:24  cphipps
// In development mode, texture lookups are all calculated initially,
// so all errors are shown at once
//
// Revision 1.8  1999/03/23 10:37:12  cphipps
// Lazy creation of the composite texture information for textures; instead of
// doing them all at startup, do only when needed.
// Changed startup progress indicators, since only tranmap building is slow now
//
// Revision 1.7  1999/03/23 09:54:11  cphipps
// Removed a lot of arrays with 1 entry per texture, merging their data into the
// texture_t for that texture.
// Made texture_t *textures; static
//
// Revision 1.6  1999/01/25 20:22:36  cphipps
// Reinstate 256-byte alignment for colourmaps
//
// Revision 1.5  1999/01/23 07:36:57  cphipps
// Fix crash generating translucancy map
//
// Revision 1.4  1999/01/01 13:34:17  cphipps
// Made R_GetColumn return a const*
// Modified for new wad lump handling
// R_GetColumn remembers last lump used, only locks/unlocks if a different lump is needed.
// Modified so translucency tables can be const
//
// Revision 1.3  1998/12/22 21:18:18  cphipps
// Fix file handle leak, using MBF fix
//
// Revision 1.2  1998/10/27 18:47:56  cphipps
// Boom v2.02 update imported
//
// Revision 1.24  1998/09/07  20:11:45  jim
// Logical output routine added
//
// Revision 1.23  1998/05/23  08:05:57  killough
// Reformatting
//
// Revision 1.21  1998/05/07  00:52:03  killough
// beautification
//
// Revision 1.20  1998/05/03  22:55:15  killough
// fix #includes at top
//
// Revision 1.19  1998/05/01  18:23:06  killough
// Make error messages look neater
//
// Revision 1.18  1998/04/28  22:56:07  killough
// Improve error handling of bad textures
//
// Revision 1.17  1998/04/27  01:58:08  killough
// Program beautification
//
// Revision 1.16  1998/04/17  10:38:58  killough
// Tag lumps with namespace tags to resolve collisions
//
// Revision 1.15  1998/04/16  10:47:40  killough
// Improve missing flats error message
//
// Revision 1.14  1998/04/14  08:12:31  killough
// Fix seg fault
//
// Revision 1.13  1998/04/12  09:52:51  killough
// Fix ?bad merge? causing seg fault
//
// Revision 1.12  1998/04/12  02:03:51  killough
// rename tranmap main_tranmap, better colormap support
//
// Revision 1.11  1998/04/09  13:19:35  killough
// Fix Medusa for transparent middles, and remove 64K composite texture size limit
//
// Revision 1.10  1998/04/06  04:39:58  killough
// Support multiple colormaps and C_START/C_END
//
// Revision 1.9  1998/03/23  03:33:29  killough
// Add support for an arbitrary number of colormaps, e.g. WATERMAP
//
// Revision 1.8  1998/03/09  07:26:03  killough
// Add translucency map caching
//
// Revision 1.7  1998/03/02  11:54:26  killough
// Don't initialize tranmap until needed
//
// Revision 1.6  1998/02/23  04:54:03  killough
// Add automatic translucency filter generator
//
// Revision 1.5  1998/02/02  13:35:36  killough
// Improve hashing algorithm
//
// Revision 1.4  1998/01/26  19:24:38  phares
// First rev with no ^Ms
//
// Revision 1.3  1998/01/26  06:11:42  killough
// Fix Medusa bug, tune hash function
//
// Revision 1.2  1998/01/22  05:55:56  killough
// Improve hashing algorithm
//
// Revision 1.3  1997/01/29 20:10
// ???
//
//-----------------------------------------------------------------------------
