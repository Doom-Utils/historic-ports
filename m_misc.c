// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
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
//
// $Log:$
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------
// 07-Apr-98  Eduardo Casino <eduardo@medusa.es)
//       Added "video" and "vid_path" options. Removed SNDSERV stuff
// 09-Apr-98  Eduardo Casino <eduardo@medusa.es)
//       Added "darken_screen" option

static const char
rcsid[] = "$Id: m_misc.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef DJGPP
#include <allegro.h>
#endif

#include <ctype.h>

#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "v_res.h"

#include "hu_stuff.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*		hu_font[HU_FONTSIZE];

int
M_DrawText
( int		x,
  int		y,
  boolean	direct,
  char*		string )
{
    int 	c;
    int		w;

    while (*string)
    {
	c = toupper(*string) - HU_FONTSTART;
	string++;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    x += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (x+w > SCREENWIDTH)
	    break;
	if (direct)
	    V_DrawPatchDirect(x, y, 0, hu_font[c]);
	else
	    V_DrawPatch(x, y, 0, hu_font[c]);
	x+=w;
    }

    return x;
}




//
// M_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

boolean
M_WriteFile
( char const*	name,
  void*		source,
  int		length )
{
    int		handle;
    int		count;
	
    handle = open ( name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);

    if (handle == -1)
	return false;

    count = write (handle, source, length);
    close (handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//
int
M_ReadFile
( char const*	name,
  byte**	buffer )
{
    int	handle, count, length;
    struct stat	fileinfo;
    byte		*buf;
	
    handle = open (name, O_RDONLY | O_BINARY, 0666);
    if (handle == -1)
	I_Error ("Couldn't read file %s", name);
    if (fstat (handle,&fileinfo) == -1)
	I_Error ("Couldn't read file %s", name);
    length = fileinfo.st_size;
    buf = Z_Malloc (length, PU_STATIC, NULL);
    count = read (handle, buf, length);
    close (handle);
	
    if (count < length)
	I_Error ("Couldn't read file %s", name);
		
    *buffer = buf;
    return length;
}


//
// DEFAULTS
//
int		usemouse;
int		usejoystick;

extern int	viewwidth;
extern int	viewheight;

extern int	mouseSensitivity;

extern int	detailLevel;

extern int	screenblocks;

extern int	darken_screen;

extern int	showMessages;

// machine-independent sound params
int	numChannels;

//machie-dependant
int snd_musicdevice;
int snd_sfxdevice;
int snd_sbport;
int snd_sbirq;
int snd_sbdma;
int snd_mport;
int showmessages;
int comport;

#ifdef LINUX
char*		mousetype;
char*		mousedev;
char*		videoInterface;
char*		vid_path;
#endif

extern char*	chat_macros[];




default_t	defaults[] =
{
    {"mouse_sensitivity",&mouseSensitivity, 5},
    {"sfx_volume",&snd_SfxVolume, 8},
    {"music_volume",&snd_MusicVolume, 8},
    {"cdmusic_volume",&snd_CDMusicVolume, 8},
    {"show_messages",&showMessages, 1},
    

    {"key_right",&key_right, KEYD_RIGHTARROW},
    {"key_left",&key_left, KEYD_LEFTARROW},
    {"key_up",&key_up, KEYD_UPARROW},
    {"key_down",&key_down, KEYD_DOWNARROW},
    {"key_lookup",&key_lookup, KEYD_PGUP},
    {"key_lookdown",&key_lookdown, KEYD_PGDN},
    {"key_lookcenter",&key_lookcenter,KEYD_HOME},
    {"key_strafeleft",&key_strafeleft, ','},
    {"key_straferight",&key_straferight, '.'},

    {"key_fire",&key_fire, KEYD_RCTRL},
    {"key_use",&key_use, ' '},
    {"key_strafe",&key_strafe, KEYD_RALT},
    {"key_speed",&key_speed, KEYD_RSHIFT},
    {"key_nextweapon",&key_nextweapon, '/'},
    {"key_jump",&key_jump, 0},
    {"key_180",&key_180, 0},
    {"key_map",&key_map, KEYD_TAB},
    {"key_talk",&key_talk, 't'},

    {"swapstereo",(int *) &swapstereo,0},
    {"mlookon",&mlookon,0},
    {"invertmouse",&invertmouse,0},
    {"mlookspeed",&keylookspeed,1000/64},
    {"translucency",&transluc,1},
    {"spectreability",&spectreability,1},
    {"lostsoulability",&lostsoulability,0},
    {"crosshair",&crosshair,0},
    {"stretchsky",&stretchsky,1},
    {"rotatemap",&rotatemap,0},
    {"newhud",&newhud,0},
    {"newnmrespawn",&newnmrespawn,0},
    {"itemrespawn",&itemrespawn,0},
    {"infight",&infight,0},
    {"lessaccuratemon",&lessaccuratemon,0},
    {"lessaccuratezom",&lessaccuratezom,1},
    {"grav",&grav,8},
    {"shootupdown",&shootupdown,0},
    {"missileteleport",&missileteleport,0}, 
    {"teleportdelay",&teleportdelay,0},     
    {"menuoptionshade",&menuoptionshade,0},
    {"menunameshade",&menunameshade,0},



#ifdef LINUX
    {"mousedev", (int*)&mousedev, (int)"/dev/ttyS0"},
    {"mousetype", (int*)&mousetype, (int)"microsoft"},

    {"video", (int*)&videoInterface, (int)"x"},
    {"vid_path", (int*)&vid_path, (int)"/usr/lib/games/doom"},
#endif
    {"novert",&novert, 0},
    {"use_mouse",&usemouse, 1},
    {"mouseb_fire",&mousebfire,0},
    {"mouseb_strafe",&mousebstrafe,1},
    {"mouseb_forward",&mousebforward,2},

    {"use_joystick",&usejoystick, 0},
    {"joyb_fire",&joybfire,0},
    {"joyb_strafe",&joybstrafe,1},
    {"joyb_use",&joybuse,3},
    {"joyb_speed",&joybspeed,2},

    {"screenblocks",&screenblocks, 9},
    {"detaillevel",&detailLevel, 0},
    {"vsync",&retrace, 0},

    {"darken_screen",&darken_screen, 1},

    {"snd_channels",&numChannels, 3},

    //this is just to preserve dos's settings
    {"snd_musicdevice",&snd_musicdevice, 0},
    {"snd_sfxdevice",&snd_sfxdevice, 0},
    {"snd_sbport",&snd_sbport, 0},
    {"snd_sbirq",&snd_sbirq, 0},
    {"snd_sbdma",&snd_sbdma, 0},
    {"snd_mport",&snd_mport, 0},
    {"showmessages",&showmessages, 1},
    {"comport",&comport, 1},
    //end of new stuff

    {"usegamma",&usegamma, 0},

    {"chatmacro0", (int *) &chat_macros[0], 0},//(int) HUSTR_CHATMACRO0 },
    {"chatmacro1", (int *) &chat_macros[1], 0},//(int) HUSTR_CHATMACRO1 },
    {"chatmacro2", (int *) &chat_macros[2], 0},//(int) HUSTR_CHATMACRO2 },
    {"chatmacro3", (int *) &chat_macros[3], 0},//(int) HUSTR_CHATMACRO3 },
    {"chatmacro4", (int *) &chat_macros[4], 0},//(int) HUSTR_CHATMACRO4 },
    {"chatmacro5", (int *) &chat_macros[5], 0},//(int) HUSTR_CHATMACRO5 },
    {"chatmacro6", (int *) &chat_macros[6], 0},//(int) HUSTR_CHATMACRO6 },
    {"chatmacro7", (int *) &chat_macros[7], 0},//(int) HUSTR_CHATMACRO7 },
    {"chatmacro8", (int *) &chat_macros[8], 0},//(int) HUSTR_CHATMACRO8 },
    {"chatmacro9", (int *) &chat_macros[9], 0} //(int) HUSTR_CHATMACRO9 }

};

int	numdefaults;
char*	defaultfile;


//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
  int		i;
  int		v;
  FILE*	f;
	
  f = fopen (defaultfile, "w");
  if (!f)
    return; // can't write the file, but don't complain
		
  for (i=0 ; i<numdefaults ; i++)
    {
    if ((defaults[i].defaultvalue > -0xfff
        && defaults[i].defaultvalue < 0xfff)||(memcmp(defaults[i].name,"key_",4)==0))
      {
      v = *defaults[i].location;
      if (memcmp(defaults[i].name,"key_",4)==0)
        fprintf (f,"%s\t\t%i\no%s\t\t%i\n",defaults[i].name,I_DoomCode2ScanCode(v&0xffff),defaults[i].name,I_DoomCode2ScanCode(v>>16));
      else
        fprintf(f,"%s\t\t%i\n",defaults[i].name,v);
      }
    else
      {
      fprintf (f,"%s\t\t\"%s\"\n",defaults[i].name,
               * (char **) (defaults[i].location));
      }
    }
	
  fclose (f);
}


//
// M_LoadDefaults
//
extern byte	scantokey[128];

void M_LoadDefaults (void)
{
    int		i;
    int		len;
    FILE*	f;
    char	def[80];
    char	strparm[100];
    char*	newstring = 0;
    int		parm;
    boolean	isstring;
    
    // set everything to base values
    numdefaults = sizeof(defaults)/sizeof(defaults[0]);
    for (i=0 ; i<numdefaults ; i++)
	*defaults[i].location = defaults[i].defaultvalue;
    
    // check for a custom default file
    i = M_CheckParm ("-config");
    if (i && i<myargc-1)
    {
	defaultfile = myargv[i+1];
	I_Printf ("	default file: %s\n",defaultfile);
    }
    else
	defaultfile = basedefault;
    
    // read the file in, overriding any set defaults
    f = fopen (defaultfile, "r");
    if (f)
    {
	while (!feof(f))
	{
	    isstring = false;
	    if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
	    {
		if (strparm[0] == '"')
		{
		    // get a string default
		    isstring = true;
		    len = strlen(strparm);
		    newstring = (char *) malloc(len);
		    strparm[len-1] = 0;
		    strcpy(newstring, strparm+1);
		}
		else if (strparm[0] == '0' && strparm[1] == 'x')
		    sscanf(strparm+2, "%x", &parm);
		else
		    sscanf(strparm, "%i", &parm);
		for (i=0 ; i<numdefaults ; i++)
                    {
		    if (!strcmp(def, defaults[i].name))
		      {
			if (!isstring)
                          {
                          if (memcmp(defaults[i].name,"key_",4)!=0)
                            {
                            *defaults[i].location = parm;
                            }
                          else
                            {
                            if (parm!=0)
    			      *defaults[i].location = I_ScanCode2DoomCode(parm);
                            }
                          }
			else
			    *defaults[i].location =
				(int) newstring;
			break;
		      }
                    if (def[0]=='o')
                      if (!strcmp(def+1,defaults[i].name))
                        if (!isstring)
                          if (memcmp(defaults[i].name,"key_",4)==0)
                            if (parm!=0)
                              *defaults[i].location|= I_ScanCode2DoomCode(parm)<<16;
                    }
	    }
	}
		
	fclose (f);
   detailLevel=0;
    }
}


//
// SCREEN SHOTS
//


typedef struct
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
} pcx_t;


//
// WritePCXfile
//
void
WritePCXfile
( char*		filename,
  byte*		data,
  int		width,
  int		height,
  byte*		palette )
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));


    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}


//
// M_ScreenShot
//
void M_ScreenShot (void)
{
    int		i;
    char	lbmname[12];

#ifdef DJGPP
    BITMAP *tempbitmap;
    PALETTE temppal;
#else
    byte*	linear;
#endif
    
    
    // find a file name to save it to
    strcpy(lbmname,"DOOM00.pcx");
		
    for (i=0 ; i<=99 ; i++)
    {
	lbmname[4] = i/10 + '0';
	lbmname[5] = i%10 + '0';
	if (access(lbmname,0) == -1)
	    break;	// file doesn't exist
    }
    if (i==100)
	I_Error ("M_ScreenShot: Couldn't create a PCX");
    
#ifdef DJGPP
    tempbitmap=create_sub_bitmap(screen,0,0,SCREENWIDTH,SCREENHEIGHT);
    get_palette(temppal);
    save_pcx(lbmname,tempbitmap,temppal);
    destroy_bitmap(tempbitmap);
#else
    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen (linear);

    // save the pcx file
    WritePCXfile (lbmname, linear,
		  SCREENWIDTH, SCREENHEIGHT,
		  W_CacheLumpName ("PLAYPAL",PU_CACHE));
#endif
		  
    players[consoleplayer].message = "screen shot";
}


