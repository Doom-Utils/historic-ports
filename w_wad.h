//  
// DOSDoom WAD Support Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//

#ifndef __W_WAD__
#define __W_WAD__


#ifdef __GNUG__
#pragma interface
#endif

#include "dm_defs.h"

//
// TYPES
//
typedef struct
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];		
    int			numlumps;
    int			infotableofs;
    
} wadinfo_t;


typedef struct
{
    int			filepos;
    int			size;
    char		name[8];
    
} filelump_t;

//
// WADFILE I/O related stuff.
//
typedef struct
{
    char	name[8];
    int		handle;
    int		position;
    int		size;
    int		wadfile;
} lumpinfo_t;

typedef struct wadtex_resource_s
{
        int mission; //useless, but here
	int pnames;
        int texture[2];
}
wadtex_resource_t;

extern	void**		lumpcache;
extern	lumpinfo_t*	lumpinfo;
extern	int		numlumps;

void    W_InitMultipleFiles (char** filenames);
void    W_Reload (void);

int	W_CheckNumForName2 (char* name) __attribute__((const));
int	W_GetNumForName2 (char* name) __attribute__((const));

int	W_LumpLength (int lump) __attribute__((const));
void    W_ReadLump (int lump, void *dest);

void*	W_CacheLumpNum2 (int lump, int tag);
void*	W_CacheLumpName2 (char* name, int tag);

wadtex_resource_t* W_GetTextureResources(void);
int*	W_GetList(char *name, int *num);

// Define this only in an emergency.  All these debug printfs quickly
// add up, and it takes only a few seconds to end up with a 40 meg debug file!
#ifdef WAD_CHECK
#include "d_debug.h"
static inline int W_CheckNumForName3(char *x, char * file, int line)
{
   Debug_Printf("Find '%s' @ %s:%d\n",x, file, line);
   return W_CheckNumForName2(x);
}

static inline int W_GetNumForName3(char *x, char *file, int line)
{
   Debug_Printf("Find '%s' @ %s:%d\n", x, file, line);
   return W_GetNumForName2(x);
}

static inline void*	W_CacheLumpNum3 (int lump, int tag, char * file, int line)
{
   Debug_Printf("Cache '%d' @ %s:%d\n",lump, file, line);
   return W_CacheLumpNum2(lump, tag);
}

static inline void*	W_CacheLumpName3(char* name, int tag, char * file, int line)
{
   Debug_Printf("Cache '%s' @ %s:%d\n",name, file, line);
   return W_CacheLumpName2(name, tag);
}

#define W_CheckNumForName(x) W_CheckNumForName3(x, __FILE__, __LINE__)
#define W_GetNumForName(x) W_GetNumForName3(x, __FILE__, __LINE__)
#define W_CacheLumpNum(x, y) W_CacheLumpNum3(x, y, __FILE__, __LINE__)
#define W_CacheLumpName(x, y) W_CacheLumpName3(x, y, __FILE__, __LINE__)

#else
#define W_CheckNumForName(x) W_CheckNumForName2(x)
#define W_GetNumForName(x) W_GetNumForName2(x)
#define W_CacheLumpNum(x, y) W_CacheLumpNum2(x, y)
#define W_CacheLumpName(x, y) W_CacheLumpName2(x, y)
#endif
#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
