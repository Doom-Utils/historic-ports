//
// DOSDoom Rendering Data Handling Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -ACB- 1998/09/09 Reformatted File Layout.
// -KM- 1998/09/27 Colourmaps can be dynamically changed.
//

#include "d_debug.h"
#include "dm_state.h"
#include "dm_defs.h"
#include "m_argv.h"   // -ES- 1998/08/25 Needed for -fast_startup
#include "m_swap.h"
#include "i_system.h"
#include "p_local.h"
#include "r_data.h"
#include "r_local.h"
#include "r_sky.h"
#include "w_wad.h"
#include "z_zone.h"

//---------------------------------------------------------------------
// Graphics:
// ^^^^^^^^^
// DOOM graphics for walls and sprites is stored in vertical runs of
// opaque pixels (posts).
//
// A column is composed of zero or more posts, a patch or sprite is
// composed of zero or more columns.
//---------------------------------------------------------------------

//
// Texture definition.
//
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
//
// The lumps are referenced by number, and patched into the rectangular
// texture space using origin and possibly other attributes.
//
typedef struct
{
  short originx;
  short originy;
  short patch;
  short stepdir;
  short colormap;
}
mappatch_t;

//
// Texture definition.
//
// A DOOM wall texture is a list of patches which are to be combined in a
// predefined order.
//
// Removing the obsolete columndirectory fails because this defines how a
// texture is stored in the wad file.
//
typedef struct
{
  char name[8];
  boolean masked;
  short width;
  short height;
  void **columndirectory;
  short patchcount;
  mappatch_t patches[1];
}
maptexture_t;

//
// A single patch from a texture definition, basically a rectangular area
// within the texture rectangle.
//
// Note: Block origin (always UL), which has already accounted
// for the internal origin of the patch.
//
typedef struct
{
  int originx;
  int originy;
  int patch;
}
texpatch_t;

//
// A maptexturedef_t describes a rectangular texture, which is composed of
// one or more mappatch_t structures that arrange graphic patches.
//
typedef struct
{
  // Keep name for switch changing, etc.
  char name[8];
  short width;
  short height;
    
  // All the patches[patchcount] are drawn back to front into the cached texture.
  short patchcount;
  texpatch_t patches[1];
}
texture_t;

// flats list
int numflats;
int* flatlist = NULL;

// patches list
int firstpatch;
int lastpatch;
int numpatches;

// sprite lists
int numspritelumps;
int* spritelist = NULL;

// textures list
int numtextures = 0;
int addtexture = 0;
texture_t** textures = NULL;

int* texturewidthmask = NULL;
int* texturecompositesize = NULL;
byte** texturecomposite = NULL;
short** texturecolumnlump = NULL;
fixed_t* textureheight = NULL;
unsigned short** texturecolumnofs = NULL;

// for global animation
int* flattranslation;
int* texturetranslation = NULL;

// needed for pre rendering
fixed_t* spritewidth;
fixed_t* spriteoffset;
fixed_t* spritetopoffset;

lighttable_t *colormaps;

// -ES- 1998/08/20 Added these two. 8-bit and 16-bit version of colormaps
lighttable_t *colormaps8 = NULL, *colormaps16 = NULL;
int numcolourmaps = 0;

// -ES- 1998/08/25 Run-time R_GenerateLookup
boolean *lookuptextures; // true if R_GenerateLookup has been called with the texture

int lookupprogress;      // keeps the number of the last un-R_GenerateLookuped
                         // texture+1


//
// MAPTEXTURE_T CACHING
//
// When a texture is first needed, it counts the number of composite columns
// required in the texture and allocates space for a column directory and any
// new columns.
//
// The directory will simply point inside other patches if there is only one
// patch in a given column, but any columns with multiple patches will have
// new column_ts generated.
//

//
// R_DrawColumnInCache
//
// Clip and draw a column from a patch into a cached post.
//
void R_DrawColumnInCache(column_t* patch, byte* cache, int originy, int cacheheight)
{
  int count;
  int position;
  byte* source;
  byte* dest;
	
  dest = (byte *)cache + 3;
	
  while (patch->topdelta != 0xff)
  {
    source = (byte *)patch + 3;
    count = patch->length;
    position = originy + patch->topdelta;

    if (position < 0)
    {
      count += position;
      position = 0;
    }

    if (position + count > cacheheight)
      count = cacheheight - position;

    if (count > 0)
      memcpy (cache + position, source, count);
		
    patch = (column_t *)(  (byte *)patch + patch->length + 4);
  }
}

//
// R_GenerateComposite
//
// Using the texture definition, the composite texture is created from the patches,
// and each column is cached.
//
void R_GenerateComposite (int texnum)
{
  int x;
  int x1;
  int x2;
  int i;

  byte* block;
  short* collump;
  patch_t* realpatch;
  column_t* patchcol;
  texture_t* texture;
  texpatch_t* patch;
  unsigned short* colofs;
	
  texture = textures[texnum];

  block=Z_Malloc(texturecompositesize[texnum],PU_STATIC,&texturecomposite[texnum]);

  collump = texturecolumnlump[texnum];
  colofs = texturecolumnofs[texnum];
    
  // Composite the columns together.
  patch = texture->patches;
		
  for (i=0, patch = texture->patches; i<texture->patchcount; i++, patch++)
  {
    realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
    x1 = patch->originx;
    x2 = x1 + SHORT(realpatch->width);

    if (x1<0)
      x = 0;
    else
      x = x1;
	
    if (x2 > texture->width)
      x2 = texture->width;

    for ( ; x<x2 ; x++)
    {
      // Column does not have multiple patches?
      if (collump[x] >= 0)
	continue;
	    
      patchcol = (column_t *)((byte *)realpatch + LONG(realpatch->columnofs[x-x1]));

      R_DrawColumnInCache(patchcol, block+colofs[x], patch->originy, texture->height);
    }
  }

  // Now that the texture has been built in column cache, it is purgable from
  // zone memory.
  Z_ChangeTag (block, PU_CACHE);
}

//
// R_GenerateLookup
//
void R_GenerateLookup (int texnum)
{
  int x;
  int x1;
  int x2;
  int i;

  byte* patchcount;     // patchcount[texture->width]
  patch_t* realpatch;
  texture_t* texture;
  texpatch_t* patch;

  short* collump;
  unsigned short* colofs;
	
  // -ES- 1998/08/18 We don't have to init this texnum again
  lookuptextures[texnum] = true;

  texture = textures[texnum];

  // Composited texture not created yet.
  texturecomposite[texnum] = 0;
    
  texturecompositesize[texnum] = 0;
  collump = texturecolumnlump[texnum];
  colofs = texturecolumnofs[texnum];
    
  // Now count the number of columns that are covered by more than one patch.
  // Fill in the lump / offset, so columns with only a single patch are all done.
  patchcount = (byte *)alloca (texture->width);
  memset (patchcount, 0, texture->width);
  patch = texture->patches;
		
  for (i=0, patch = texture->patches; i<texture->patchcount; i++, patch++)
  {
    realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
    x1 = patch->originx;
    x2 = x1 + SHORT(realpatch->width);
	
    if (x1 < 0)
      x = 0;
    else
      x = x1;

    if (x2 > texture->width)
      x2 = texture->width;

    for ( ; x<x2 ; x++)
    {
      patchcount[x]++;
      collump[x] = patch->patch;
      colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
    }
  }
	
  for (x=0 ; x<texture->width ; x++)
  {
    if (!patchcount[x])
    {
      I_Printf ("R_GenerateLookup: column without a patch \'%.8s\'\n", texture->name);
      return;
    }
	
    if (patchcount[x] > 1)
    {
      // Use the cached block.
      collump[x] = -1;
      colofs[x] = texturecompositesize[texnum];
	    
      if (texturecompositesize[texnum] > 0x10000-texture->height)
        I_Error ("R_GenerateLookup: texture %i is >64k", texnum);
	    
      texturecompositesize[texnum] += texture->height;
    }

  }

}

//
// R_GetColumn
//
byte* R_GetColumn (int tex, int col)
{
  int lump;
  int ofs;
	
  // -ES- 1998/08/25 Check if we first have to call R_GenerateLookup
  if (lookuptextures[tex] == false)
    R_GenerateLookup(tex);

  col &= texturewidthmask[tex];
  lump = texturecolumnlump[tex][col];
  ofs = texturecolumnofs[tex][col];

  if (lump > 0)
    return (byte *)W_CacheLumpNum(lump,PU_CACHE)+ofs;

  if (!texturecomposite[tex])
    R_GenerateComposite (tex);

  return texturecomposite[tex] + ofs;
}

//
// R_InitTextureLumps
//
// -ACB- 1998/09/09 Removed the Doom II SkyName change: unnecessary and not DDF.
//                  Reformatted and cleaned up.
//
void R_InitTextureLumps (int pnames, int texture1, int texture2)
{
  maptexture_t* mtexture;
  texture_t* texture;
  mappatch_t* mpatch;
  texpatch_t* patch;

  int i;
  int j;
  int totalwidth;
  int nummappatches = 0;
  int offset;
  int maxoff;
  int maxoff2;
  int numtextures1;
  int numtextures2;
  int counttextures;

  int* maptex;
  int* maptex2;
  int* maptex1;
  int* patchlookup;
  int* directory;

  char name[9];
  char* names;
  char* name_p;
    
  // Load the patch names from pnames.lmp.
  name[8] = 0;
  names = W_CacheLumpNum (pnames, PU_STATIC);
  nummappatches = LONG ( *((int *)names) );
  name_p = names+4;
  patchlookup = alloca (nummappatches*sizeof(*patchlookup));
    
  for (i=0 ; i<nummappatches ; i++)
  {
    strncpy (name,name_p+i*8, 8);
    patchlookup[i] = W_CheckNumForName (name);
  }

  Z_Free (names);

  //
  // Load the map texture definitions from textures.lmp.
  //
  // The data is contained in one or two lumps:
  //   TEXTURE1 for shareware
  //   TEXTURE2 for commercial.
  //
  maptex = maptex1 = W_CacheLumpNum (texture1, PU_STATIC);
  numtextures1 = LONG(*maptex);
  maxoff = W_LumpLength (texture1);
  directory = maptex+1;

  if (texture2 != -1)
  {
    maptex2 = W_CacheLumpNum (texture2, PU_STATIC);
    numtextures2 = LONG(*maptex2);
    maxoff2 = W_LumpLength (texture2);
  }
  else
  {
    maptex2 = NULL;
    numtextures2 = 0;
    maxoff2 = 0;
  }

  counttextures = numtextures;
  numtextures += numtextures1 + numtextures2;

  textures = Z_ReMalloc (textures, numtextures*sizeof(int));
  texturecolumnlump = Z_ReMalloc (texturecolumnlump, numtextures*sizeof(int));
  texturecolumnofs = Z_ReMalloc (texturecolumnofs, numtextures*sizeof(int));
  texturecomposite = Z_ReMalloc (texturecomposite, numtextures*sizeof(int));
  texturecompositesize = Z_ReMalloc (texturecompositesize, numtextures*sizeof(int));
  texturewidthmask = Z_ReMalloc (texturewidthmask, numtextures*sizeof(int));
  textureheight = Z_ReMalloc (textureheight, numtextures*sizeof(int));

  totalwidth = 0;
    	
  for (i=counttextures ; i<numtextures ; i++, directory++)
  {
    if (!(i&63))
      I_Printf (".");

    if (i == (numtextures1 + counttextures))
    {
      // Start looking in second texture file.
      maptex = maptex2;
      maxoff = maxoff2;
      directory = maptex+1;
    }
		
    offset = LONG(*directory);

    if (offset > maxoff)
      I_Error ("R_InitTextures: bad texture directory");
	
    mtexture = (maptexture_t *) ( (byte *)maptex + offset);

    texture = textures[addtexture] = Z_Malloc (sizeof(texture_t) +
                                                sizeof(texpatch_t) *
                                                 (SHORT(mtexture->patchcount)-1),
		                                   PU_STATIC, 0);
	
    texture->width = SHORT(mtexture->width);
    texture->height = SHORT(mtexture->height);
    texture->patchcount = SHORT(mtexture->patchcount);

    memcpy (texture->name, mtexture->name, sizeof(texture->name));

    mpatch = &mtexture->patches[0];
    patch = &texture->patches[0];

    for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
    {
      patch->originx = SHORT(mpatch->originx);
      patch->originy = SHORT(mpatch->originy);
      patch->patch = patchlookup[SHORT(mpatch->patch)];

      if (patch->patch == -1)
	I_Error ("R_InitTextures: Missing patch in texture \'%.8s\'", texture->name);
    }

    texturecolumnlump[addtexture] = Z_Malloc (texture->width*2, PU_STATIC,0);
    texturecolumnofs[addtexture] = Z_Malloc (texture->width*2, PU_STATIC,0);

    j = 1;

    while (j*2 <= texture->width)
      j<<=1;

    texturewidthmask[addtexture] = j-1;
    textureheight[addtexture] = texture->height<<FRACBITS;
		
    totalwidth += texture->width;
    addtexture++;
  }

  Z_Free (maptex1);

  if (maptex2)
    Z_Free (maptex2);

  // -ES- 1998/08/25 Init lookuptextures
  lookuptextures = Z_Malloc (numtextures * sizeof(boolean), PU_STATIC, 0);

  for (i=0 ; i<numtextures ; i++)
    lookuptextures[i] = false;

  lookupprogress = numtextures;

  // -ES- 1998/08/25 check for parameter
  if (!M_CheckParm("-fast_startup"))
  {
    // Precalculate whatever possible.
    for (i=counttextures ; i<numtextures ; i++)
      R_GenerateLookup (i);
  }

  // Create translation table for global animation.
  texturetranslation = Z_ReMalloc (texturetranslation, (numtextures+1)*sizeof(int));
    
  for (i=counttextures ; i<numtextures ; i++)
    texturetranslation[i] = i;
}

//
// R_InitTextures
//
// Initializes the texture list with the textures from the world map.
//
// -ACB- 1998/09/09 Fixed the Display routine from display rubbish.
//
void R_InitTextures (void)
{
  wadtex_resource_t* tex;
  int maxtex;
  int maxtextures = 0;
  int i, printlen;
  int* texlump;
  char* printstatus;

  tex = W_GetTextureResources();

  for (maxtex = 0; tex[maxtex].mission != -1; maxtex++)
  {
    for (i = 0; i < 2; i++)
    {
      if (tex[maxtex].texture[i] != -1)
      {
        texlump = W_CacheLumpNum(tex[maxtex].texture[i], PU_STATIC);
        maxtextures += LONG(*texlump);
      }
    }
  }

  spritelist = W_GetList("S", &numspritelumps);
   
  printlen = ((numspritelumps + 63)/64) + ((maxtextures+63)/64);
  printstatus = alloca((printlen + 11)*sizeof(char));

  memset(printstatus, ' ', (printlen + 9)*sizeof(char));
  printstatus[0] = '[';
  printstatus[printlen + 7] = ']';
  printstatus[printlen + 8] = '\0';

  I_Printf("%s", printstatus);

  for (i=strlen(printstatus); --i;)
    I_Printf("\x8");

  // -ACB- 1998/09/09 Removed the Doom II SkyName change: unnecessary and not DDF.
  for (i = 0; i < maxtex; i++)
  {
    if (tex[i].texture[0] != -1)
      R_InitTextureLumps(tex[i].pnames, tex[i].texture[0], tex[i].texture[1]);
  }

  Z_Free(tex);
}

//
// R_InitFlats
//
void R_InitFlats (void)
{
  int i;
	
  if (!flatlist)
    flatlist = W_GetList("F", &numflats);
	
  // Create translation table for global animation.
  flattranslation = Z_Malloc ((numflats+1)*sizeof(int), PU_STATIC, 0);
    
  for (i=0 ; i<numflats ; i++)
    flattranslation[i] = i;
}


//
// R_InitSpriteLumps
//
// Finds the width and hoffset of all sprites in the wad, so the sprite does
// not need to be cached completely just for having the header info ready
// during rendering.
//
void R_InitSpriteLumps (void)
{
  int i;
  patch_t *patch;
	
  if (!spritelist)
    spritelist = W_GetList("S", &numspritelumps);
    
  spritewidth = Z_Malloc (numspritelumps*sizeof(int), PU_STATIC, 0);
  spriteoffset = Z_Malloc (numspritelumps*sizeof(int), PU_STATIC, 0);
  spritetopoffset = Z_Malloc (numspritelumps*sizeof(int), PU_STATIC, 0);
	
  for (i=0 ; i< numspritelumps ; i++)
  {
    if (!(i&63))
      I_Printf (".");

    patch = W_CacheLumpNum (spritelist[i], PU_CACHE);
    spritewidth[i] = SHORT(patch->width)<<FRACBITS;
    spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
    spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;
  }
}

//
// R_SetColourMap
//
// -KM- 1998/09/25 This is for changing colourmaps on the fly.  I don't know if
// 16 bit works too well.  Also, freeing the colourmap here is not as bad as it
// seems.  The old colourmap will be cached in memory.
//
// -ES- 1998/09/27 Fixed alignment of colormaps8.
// Also increased alignment of colormaps16 to 512 (enables an optimisation trick)
//
// -KM- 1998/09/28 And I thought alignment of colourmaps wasn't important!
//                 Applied fix above, and fixed it.  Problem is that Colourmap
//                 is read from disk *every* time it is needed.  Used the lumpcache.
void R_SetColourMap(int lump)
{
  static int colourmaplump = 0;
  if (colormaps8)
    Z_ChangeTag(lumpcache[colourmaplump], PU_CACHE);

  if (!lumpcache[lump])  // Cache Miss :-(
  {
    // read the lump in
    Z_Malloc (W_LumpLength (lump) + 255, PU_STATIC, &lumpcache[lump]);
    colormaps8 = (byte *)( ((int)lumpcache[lump] + 255)&~0xff);
    W_ReadLump (lump, colormaps8);
  }
  else // Cache Hit :-)
  {
    Z_ChangeTag (lumpcache[lump],PU_STATIC);
    colormaps8 = (byte *)( ((int)lumpcache[lump] + 255)&~0xff);
  }

  colourmaplump = lump;
  numcolourmaps = W_LumpLength(lump)/256;

  if (!colormaps16)
  {
    colormaps16 = Z_Malloc (numcolourmaps*256*2+511, PU_STATIC, 0);
    colormaps16 = (byte *)( ((int)colormaps16 + 511)&~0x1ff);
  }

  //colormap data is set up in set_palette
  colormaps = (BPP==1) ? colormaps8 : colormaps16;
}

//
// R_InitColormaps
//
// -ES- 1998/08/20 Splitted up colormaps into one 8-bit and one 16-bit version
//
void R_InitColormaps (void)
{
  R_SetColourMap(W_GetNumForName("COLORMAP"));
}
//
// R_InitData
//
// Locates all the lumps that will be used by all views
//
// Must be called after W_Init.
//
void R_InitData (void)
{
  R_InitTextures ();
  Debug_Printf ("\nInitTextures");

  R_InitFlats ();
  Debug_Printf ("\nInitFlats");

  R_InitSpriteLumps ();
  Debug_Printf ("\nInitSprites");

  R_InitColormaps ();
  Debug_Printf ("\nInitColormaps");
}

//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName (char* name)
{
  int i;

  for (i = numflats; i--;)
  {
    if (!strncasecmp(name, lumpinfo[flatlist[i]].name, 8))
      break;
  }

  if (i == -1)
  {
    // 25-6-98 KM We have a backup plan...
    i = W_CheckNumForName(name);

    if (i == -1)
      I_Error ("R_FlatNumForName: \'%.8s\' not found",name);

    flatlist = Z_ReMalloc(flatlist, (numflats + 1) * sizeof(int));
    flattranslation = Z_ReMalloc(flattranslation, (numflats + 2) * sizeof(int));
    flatlist[numflats] = i;
    flattranslation[numflats] = numflats;
    i = numflats++;
  }

  return i;
}

//
// R_CheckTextureNumForName
//
// Check whether texture is available. Filter out NoTexture indicator.
//
int R_CheckTextureNumForName (char *name)
{
  int i;

  // "NoTexture" marker.
  if (!name)
    return 0;
  if (!name[0] || name[0] == '-')
    return 0;
		
  for (i=numtextures ; i-- ; )
  {
    if (!strncasecmp (textures[i]->name, name, 8) )
      return i;
  }

  return -1;
}

//
// R_TextureNumForName
//
// Calls R_CheckTextureNumForName, aborts with error message.
//
int R_TextureNumForName (char* name)
{
  int i;
	
  i = R_CheckTextureNumForName (name);

  if (i==-1)
    I_Error ("R_TextureNumForName: \'%.8s\' not found", name);

  return i;
}

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int flatmemory;
int texturememory;
int spritememory;

void R_PrecacheLevel (void)
{
  char* flatpresent;
  char* texturepresent;
  char* spritepresent;

  int i;
  int j;
  int k;
  int lump;
    
  texture_t* texture;

#ifdef ORIGINAL
  thinker_t* th;
#else
  mobj_t* currmobj;
#endif

  spriteframe_t* sf;

  if (demoplayback)
    return;
    
  // Precache flats.
  flatpresent = alloca(numflats);
  memset (flatpresent,0,numflats);

  for (i=0 ; i<numsectors ; i++)
  {
    flatpresent[sectors[i].floorpic] = 1;
    flatpresent[sectors[i].ceilingpic] = 1;
  }
	
  flatmemory = 0;

  for (i=0 ; i<numflats ; i++)
  {
    if (flatpresent[i])
    {
      lump = flatlist[i];
      flatmemory += lumpinfo[lump].size;
      W_CacheLumpNum(lump, PU_CACHE);
    }
  }
    
  // Precache textures.
  texturepresent = alloca(numtextures);
  memset (texturepresent,0, numtextures);

  for (i=0 ; i<numsides ; i++)
  {
    texturepresent[sides[i].toptexture] = 1;
    texturepresent[sides[i].midtexture] = 1;
    texturepresent[sides[i].bottomtexture] = 1;
  }

  //
  // Sky texture is always present.
  //
  // Note that F_SKY1 is the name used to indicate a sky floor/ceiling as a flat,
  // while the sky texture is stored like a wall texture, with a level depended name;
  // dictated to by the currentmap structure.
  //
  texturepresent[skytexture] = 1;
	
  texturememory = 0;

  for (i=0 ; i<numtextures ; i++)
  {
    if (!texturepresent[i])
      continue;

    texture = textures[i];
	
    for (j=0 ; j<texture->patchcount ; j++)
    {
      lump = texture->patches[j].patch;
      texturememory += lumpinfo[lump].size;
      W_CacheLumpNum(lump , PU_CACHE);
    }
  }

  // Precache sprites.
  spritepresent = alloca(numsprites);
  memset (spritepresent,0, numsprites);

#ifdef ORIGINAL
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
  {
    if (th->function.acp1 == (actionf_p1)P_MobjThinker)
    spritepresent[((mobj_t *)th)->sprite] = 1;
  }
#else
  for (currmobj=mobjlisthead; currmobj==NULL; currmobj=currmobj->next)
    spritepresent[currmobj->sprite] = 1;
#endif
	
  spritememory = 0;

  for (i=0 ; i<numsprites ; i++)
  {
    if (!spritepresent[i])
      continue;

    for (j=0 ; j<sprites[i].numframes ; j++)
    {
      sf = &sprites[i].spriteframes[j];

      for (k=0 ; k<8 ; k++)
      {
 	lump = spritelist[sf->lump[k]];
 	spritememory += lumpinfo[lump].size;
	W_CacheLumpNum(lump , PU_CACHE);
      }

    }

  }

}




