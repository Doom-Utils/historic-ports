// W_wad.c

#ifdef NeXT
#include <libc.h>
#include <ctype.h>

// next doesn't need a binary flag in open call
#define	O_BINARY	0

#else

#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#include "DoomDef.h"

//===============
//   TYPES
//===============


typedef struct
{
	char		identification[4];		// should be IWAD
	int			numlumps;
	int			infotableofs;
} wadinfo_t;


typedef struct
{
	int			filepos;
	int			size;
	char		name[8];
} filelump_t;


//=============
// GLOBALS
//=============

lumpinfo_t	*lumpinfo;		// location of each lump on disk
int			numlumps;

void		**lumpcache;


//===================

//#ifdef NeXT

#define strcmpi strcasecmp

/*void strupr (char *s)
{
    while (*s)
	*s++ = toupper(*s);
}
*/

/*
================
=
= filelength
=
================
*/

/*int filelength (int handle)

{

    struct stat	fileinfo;
    
    if (fstat (handle,&fileinfo) == -1)
	I_Error ("Error fstating");

    return fileinfo.st_size;
}
*/

//#endif


void ExtractFileBase (char *path, char *dest)
{
    char*	src;
    int		length;

    src = path + strlen(path) - 1;
    
    // back up until a \ or the start
    while (src != path
	   && *(src-1) != '\\'
	   && *(src-1) != '/')
    {
	src--;
    }
    
    // copy up to eight characters
    memset (dest,0,8);
    length = 0;
    
    while (*src && *src != '.')
    {
	if (++length == 9)
	    I_Error ("Filename base of %s >8 chars",path);

	*dest++ = toupper((int)*src++);
    }
}

/*
============================================================================

						LUMP BASED ROUTINES

============================================================================
*/

int			reloadlump;
char*			reloadname;

/*
====================
=
= W_AddFile
=
= All files are optional, but at least one file must be found
= Files with a .wad extension are wadlink files with multiple lumps
= Other files are single lumps with the base filename for the lump name
=
====================
*/

void W_AddFile (char *filename)
{
    wadinfo_t		header;
    lumpinfo_t*		lump_p;
    unsigned		i;
    int			handle;
    int			length;
    int			startlump;
    filelump_t*		fileinfo;
    filelump_t		singleinfo;
    int			storehandle;
    
    // open the file and add to directory

    // handle reload indicator.
    if (filename[0] == '~')
    {
	filename++;
	reloadname = filename;
	reloadlump = numlumps;
    }
		
    if ( (handle = open (filename,O_RDONLY | O_BINARY)) == -1)
    {
	printf (" couldn't open %s\n",filename);
	return;
    }

    printf (" adding %s\n",filename);
    startlump = numlumps;
	
    if (strcmpi (filename+strlen(filename)-3 , "wad" ) )
    {
	// single lump file
	fileinfo = &singleinfo;
	singleinfo.filepos = 0;
	singleinfo.size = LONG(filelength(handle));
	ExtractFileBase (filename, singleinfo.name);
	numlumps++;
    }
    else 
    {
	// WAD file
	read (handle, &header, sizeof(header));
	if (strncmp(header.identification,"IWAD",4))
	{
	    // Homebrew levels?
	    if (strncmp(header.identification,"PWAD",4))
	    {
		I_Error ("Wad file %s doesn't have IWAD "
			 "or PWAD id\n", filename);
	    }
	    
	    // ???modifiedgame = true;		
	}
	header.numlumps = LONG(header.numlumps);
	header.infotableofs = LONG(header.infotableofs);
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = alloca (length);
	lseek (handle, header.infotableofs, SEEK_SET);
	read (handle, fileinfo, length);
	numlumps += header.numlumps;
    }

    
    // Fill in lumpinfo
    lumpinfo = realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));

    if (!lumpinfo)
	I_Error ("Couldn't realloc lumpinfo");

    lump_p = &lumpinfo[startlump];
	
    storehandle = reloadname ? -1 : handle;
	
    for (i=startlump ; i<numlumps ; i++,lump_p++, fileinfo++)
    {
	lump_p->handle = storehandle;
	lump_p->position = LONG(fileinfo->filepos);
	lump_p->size = LONG(fileinfo->size);
	strncpy (lump_p->name, fileinfo->name, 8);
    }
	
    if (reloadname)
	close (handle);
}


int W_IsS_START(lumpinfo_t* lump_p)
{
  return ( (lump_p->name[0]=='S') &&
           (lump_p->name[1]=='_') &&
           (lump_p->name[2]=='S') &&
           (lump_p->name[3]=='T') &&
           (lump_p->name[4]=='A') &&
           (lump_p->name[5]=='R') &&
           (lump_p->name[6]=='T') &&
           (lump_p->name[7]==0  ) );
}

int W_IsS_END(lumpinfo_t* lump_p)
{
  return ( (lump_p->name[0]=='S') &&
           (lump_p->name[1]=='_') &&
           (lump_p->name[2]=='E') &&
           (lump_p->name[3]=='N') &&
           (lump_p->name[4]=='D') &&
           (lump_p->name[5]==0  ) &&
           (lump_p->name[6]==0  ) &&
           (lump_p->name[7]==0  ) );
}

int W_IsNULLNAME(lumpinfo_t* lump_p)
{
  return ( (lump_p->name[0]==0) &&
           (lump_p->name[1]==0) &&
           (lump_p->name[2]==0) &&
           (lump_p->name[3]==0) &&
           (lump_p->name[4]==0) &&
           (lump_p->name[5]==0) &&
           (lump_p->name[6]==0) &&
           (lump_p->name[7]==0) );
}

void W_FindAndDeleteLump(
   lumpinfo_t* first,    /* first lump in list - stop when get to it */
   lumpinfo_t* lump_p,   /* lump just after one to start at          */
   char *name)           /* name of lump to remove if found          */
//
// Find lump by name, starting before specifed pointer.
// Overwrite name with nulls if found. This is used to remove
// the originals of sprite entries duplicated in a PWAD, the
// sprite code doesn't like two sprite lumps of the same name
// existing in the sprites list.
//
{
    union {
	char	s[9];
	int	x[2];
    } name8;
    
    int		v1;
    int		v2;

    // make the name into two integers for easy compares
    strncpy (name8.s,name,8);

    // in case the name was a full 8 chars
    name8.s[8] = 0;

    // case insensitive
    strupr (name8.s);		

    v1 = name8.x[0];
    v2 = name8.x[1];

    do {
        lump_p--;
    } while ((lump_p != first)
             && ((*(int *)lump_p->name != v1)
                 || (*(int *)&lump_p->name[4] != v2)));

    if (   (*(int *)lump_p->name == v1)
        && (*(int *)&lump_p->name[4] == v2))
          memset(lump_p,0,sizeof(lumpinfo_t));
}

void W_GroupSprites(void)
{
    int newnumlumps=0;
    lumpinfo_t* lumpinfo_copy;
    lumpinfo_t* lump_s;
    lumpinfo_t* lump_d; /* dest, source */
    lumpinfo_t* lump_t; /* temp */

    lumpinfo_copy=malloc(numlumps*sizeof(lumpinfo_t));
    lump_s = lumpinfo;
    lump_d = lumpinfo_copy;

    /* skip to first s_start flag */
    while ( (lump_s < lumpinfo+numlumps)
           &&
            (!W_IsS_START(lump_s)) ) lump_s++;
    /* copy the first s_start */
    memcpy(lump_d++,lump_s,sizeof(lumpinfo_t));
    /* prepare for loop below to skip over imaginary s_end */
    lump_d++;
    do
    { /* skip through entries */
      if (!W_IsS_START(lump_s))
      {
         lump_s++;
      }
      else
      {
        lump_d--; /* skip back to overwrite previous s_end */
        memset(lump_s++,0,sizeof(lumpinfo_t)); /* zap S_START, go to next */
        /* copy rest of this sprite group, including the s_end */
        do {                                                        
          /* for each sprite, remove the original if it exists */
          W_FindAndDeleteLump(lumpinfo_copy,lump_d,lump_s->name);
          /* copy it */
          memcpy(lump_d,lump_s,sizeof(lumpinfo_t));
          /* zap the lump in the original list */
          memset(lump_s++,0,sizeof(lumpinfo_t));
        } while (!W_IsS_END(lump_d++));
      }
    } while (lump_s < lumpinfo+numlumps);

    /* now copy other, non-sprite entries */
    lump_s = lumpinfo;
/*  lump_d = at next free slot in lumpinfo_copy */
    do
    { /* skip through entries */
      if (!W_IsNULLNAME(lump_s))
      {
        memcpy(lump_d++,lump_s,sizeof(lumpinfo_t));
      }
    } while (lump_s++ < lumpinfo+numlumps);

    /* now replace original lumpinfo, squeezing out blanked sprites */
/*  lump_d = at next "free slot" in lumpinfo_copy */
    lump_t=lumpinfo;
    lump_s=lumpinfo_copy;
    while (lump_s != lump_d)
    { 
      if (!W_IsNULLNAME(lump_s))
      {
        newnumlumps++;
        memcpy(lump_t++,lump_s,sizeof(lumpinfo_t));
      }
      lump_s++;
    }
    printf("ONL=%d NNL=%d\n",numlumps,newnumlumps);
//    getchar();
    numlumps=newnumlumps;
    free(lumpinfo_copy);
    realloc(lumpinfo,numlumps*sizeof(lumpinfo_t));
    if (!lumpinfo)
	I_Error ("Couldn't realloc lumpinfo");
}



/*
====================
=
= W_InitMultipleFiles
=
= Pass a null terminated list of files to use.
=
= All files are optional, but at least one file must be found
=
= Files with a .wad extension are idlink files with multiple lumps
=
= Other files are single lumps with the base filename for the lump name
=
= Lump names can appear multiple times. The name searcher looks backwards,
= so a later file can override an earlier one.
=
====================
*/

void W_InitMultipleFiles (char **filenames)
{	
    int		size;
    
    // open all the files, load headers, and count lumps
    numlumps = 0;

    // will be realloced as lumps are added
    lumpinfo = malloc(1);	

    for ( ; *filenames ; filenames++)
	W_AddFile (*filenames);

    if (!numlumps)
	I_Error ("W_InitFiles: no files found");
    
    // set up caching
    size = numlumps * sizeof(*lumpcache);
    lumpcache = malloc (size);
    
    if (!lumpcache)
	I_Error ("Couldn't allocate lumpcache");

    memset (lumpcache,0, size);
    W_GroupSprites();
}



/*
====================
=
= W_InitFile
=
= Just initialize from a single file
=
====================
*/

void W_InitFile (char *filename)
{
    char*	names[2];

    names[0] = filename;
    names[1] = NULL;
    W_InitMultipleFiles (names);
}



/*
====================
=
= W_NumLumps
=
====================
*/

int	W_NumLumps (void)
{
	return numlumps;
}



/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/

int	W_CheckNumForName (char *name)
{
    union {
	char	s[9];
	int	x[2];
	
    } name8;
    
    int		v1;
    int		v2;
    lumpinfo_t*	lump_p;

    // make the name into two integers for easy compares
    strncpy (name8.s,name,8);

    // in case the name was a fill 8 chars
    name8.s[8] = 0;

    // case insensitive
    strupr (name8.s);		

    v1 = name8.x[0];
    v2 = name8.x[1];


    // scan backwards so patch lump files take precedence
    lump_p = lumpinfo + numlumps;

    while (lump_p-- != lumpinfo)
    {
	if ( *(int *)lump_p->name == v1
	     && *(int *)&lump_p->name[4] == v2)
	{
	    return lump_p - lumpinfo;
	}
    }

    // TFB. Not found.
    return -1;
}


/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int	W_GetNumForName (char *name)
{
    int	i;

    i = W_CheckNumForName (name);
    
    if (i == -1)
      I_Error ("W_GetNumForName: %s not found!", name);
      
    return i;
}


/*
====================
=
= W_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_LumpLength (int lump)
{
    if (lump >= numlumps)
	I_Error ("W_LumpLength: %i >= numlumps",lump);

    return lumpinfo[lump].size;
}


/*
====================
=
= W_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/

void W_ReadLump (int lump, void *dest)
{
    int		c;
    lumpinfo_t*	l;
    int		handle;
	
    if (lump >= numlumps)
	I_Error ("W_ReadLump: %i >= numlumps",lump);

    l = lumpinfo+lump;
	
    // ??? I_BeginRead ();
	
    if (l->handle == -1)
    {
	// reloadable file, so use open / read / close
	if ( (handle = open (reloadname,O_RDONLY | O_BINARY)) == -1)
	    I_Error ("W_ReadLump: couldn't open %s",reloadname);
    }
    else
	handle = l->handle;
		
    lseek (handle, l->position, SEEK_SET);
    c = read (handle, dest, l->size);

    if (c < l->size)
	I_Error ("W_ReadLump: only read %i of %i on lump %i",
		 c,l->size,lump);	

    if (l->handle == -1)
	close (handle);
		
    // ??? I_EndRead ();
}



/*
====================
=
= W_CacheLumpNum
=
====================
*/

void	*W_CacheLumpNum (int lump, int tag)
{
    byte*	ptr;

    if ((unsigned)lump >= numlumps)
	I_Error ("W_CacheLumpNum: %i >= numlumps",lump);
		
    if (!lumpcache[lump])
    {
	// read the lump in
	
	//printf ("cache miss on lump %i\n",lump);
	ptr = Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
	W_ReadLump (lump, lumpcache[lump]);
    }
    else
    {
	//printf ("cache hit on lump %i\n",lump);
	Z_ChangeTag (lumpcache[lump],tag);
    }
	
    return lumpcache[lump];
}


/*
====================
=
= W_CacheLumpName
=
====================
*/

void	*W_CacheLumpName (char *name, int tag)
{
    return W_CacheLumpNum (W_GetNumForName(name), tag);
}


/*
====================
=
= W_Profile
=
====================
*/

// Ripped out for Heretic
int	info[2500][10];
int	profilecount;

void W_Profile (void)
{
    int		i;
    memblock_t*	block;
    void*	ptr;
    char	ch;
    FILE*	f;
    int		j;
    char	name[9];
	
	
    for (i=0 ; i<numlumps ; i++)
    {	
	ptr = lumpcache[i];
	if (!ptr)
	{
	    ch = ' ';
	    continue;
	}
	else
	{
	    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	    if (block->tag < PU_PURGELEVEL)
		ch = 'S';
	    else
		ch = 'P';
	}
	info[i][profilecount] = ch;
    }
    profilecount++;
	
    f = fopen ("waddump.txt","w");
    name[8] = 0;

    for (i=0 ; i<numlumps ; i++)
    {
	memcpy (name,lumpinfo[i].name,8);

	for (j=0 ; j<8 ; j++)
	    if (!name[j])
		break;

	for ( ; j<8 ; j++)
	    name[j] = ' ';

	fprintf (f,"%s ",name);

	for (j=0 ; j<profilecount ; j++)
	    fprintf (f,"    %c",info[i][j]);

	fprintf (f,"\n");
    }
    fclose (f);
}

