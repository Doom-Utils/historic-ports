//
// DOSDoom Misc: PCX Screenshots, Menu and defaults Code
//
// Based on the Doom Source Code
//
// Released by id Software, (c) 1993-1996 (see DOOMLIC.TXT) 
//
// -MH- 1998/07/02  Added key_flyup and key_flydown
// -MH- 1998/07/02 "shootupdown" --> "true3dgameplay"
//

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef DJGPP
// For screen shots.
#include <allegro.h>
#endif

#include <ctype.h>

#include "dm_defs.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

// 98-7-10 KM For non critical errors
#include "m_menu.h"

#include "f_wipe.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "v_res.h"

#include "hu_stuff.h"

// State.
#include "dm_state.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

#include "r_main.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*         hu_font[HU_FONTSIZE];

int
M_DrawText
( int           x,
  int           y,
  boolean       direct,
  char*         string )
{
    int         c = 0;
    int         w;

    while (*string)
    {
        if (c < 128)
          c = toupper(*string);
        else
          c = *string;

        c -= HU_FONTSTART;
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

boolean M_WriteFile (char const* name, void* source, int length)
{
    int         handle;
    int         count;
        
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
int M_ReadFile (char const* name, byte** buffer)
{
    int handle, count, length;
    struct stat fileinfo;
    byte                *buf;
        
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
int             usemouse;
int             usejoystick;

int             cfgnormalfov;
int             cfgzoomedfov;

extern int      viewwidth;
extern int      viewheight;

extern int      mouseSensitivity;

extern int      detailLevel;

extern int      screenblocks;

extern int      darken_screen;

extern int      showMessages;

// -KM- 1998/07/21 Save the blood setting
extern boolean blood;

// machine-independent sound params
extern int      numChannels;

int showmessages;

#ifdef LINUX
char*           mousetype;
char*           mousedev;
char*           videoInterface;
char*           vid_path;
#endif

extern char*    chat_macros[];

default_t       defaults[] =
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
    // -ES- 1999/03/28 Zoom Key
    {"key_zoom",&key_zoom,KEYD_TILDE},
    {"key_strafeleft",&key_strafeleft, ','},
    {"key_straferight",&key_straferight, '.'},

    // -ACB- for -MH- 1998/07/02 Flying Keys
    {"key_flyup",&key_flyup, KEYD_INSERT},
    {"key_flydown",&key_flydown, KEYD_DELETE},

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
    {"invertmouse",&invertmouse,0},
    {"mlookspeed",&keylookspeed,1000/64},
    {"translucency",(int *) &settingflags.trans,1},
    // -ES- 1998/11/28 Save fade settings
    {"faded_teleportation",&faded_teleportation,0},
    {"wipe_method",&wipe_method,wipe_Melt},
    {"crosshair",&crosshair,0},
    {"stretchsky",(int *) &settingflags.stretchsky,1},
    {"rotatemap",&rotatemap,0},
    {"newhud",&newhud,0},
    {"respawnsetting",(int *) &settingflags.respawnsetting,0},
    {"itemrespawn",(int *) &settingflags.itemrespawn,0},
    {"respawn",(int *) &settingflags.respawn, 0},
    {"fastparm", (int *) &settingflags.fastparm, 0},
//    {"infight",&infight,0},
//    {"lessaccuratemon",&lessaccuratemon,0},
//    {"lessaccuratezom",&lessaccuratezom,1},
    {"grav",&settingflags.grav,8},
    {"true3dgameplay",(int *) &settingflags.true3dgameplay,0},
    {"autoaim",(int *) &settingflags.autoaim, 1},
    {"missileteleport",&missileteleport,0}, 
    {"teleportdelay",&teleportdelay,0},     
    // -KM- 1998/07/21 Save the blood setting
    {"blood",(int *) &settingflags.blood,0},


#ifdef LINUX
    {"mousedev", (int*)&mousedev, (int)"/dev/ttyS0"},
    {"mousetype", (int*)&mousetype, (int)"microsoft"},

    {"video", (int*)&videoInterface, (int)"x"},
    {"vid_path", (int*)&vid_path, (int)"/usr/lib/games/doom"},
#endif

    {"novert",&novert, 0},

    // -KM- 1998/09/01 Useless mouse/joy stuff removed,
    //                 analogue binding added
    {"use_mouse",&usemouse, 1},
    {"mouse_xaxis",&mouse_xaxis, AXIS_TURN},
    {"mouse_yaxis",&mouse_yaxis, AXIS_FORWARD},

    // -ACB- 1998/09/06 Two-stage turning & Speed controls added
    {"twostage_turning",(int *)&stageturn, 0},
    {"forwardmove_speed",&forwardmovespeed, 0},
    {"angleturn_speed",&angleturnspeed, 0},
    {"sidemove_speed",&sidemovespeed, 0 },

    {"use_joystick",&usejoystick, 0},
    {"joy_xaxis",&joy_xaxis, AXIS_TURN},
    {"joy_yaxis",&joy_yaxis, AXIS_FORWARD},

    {"screenblocks",&screenblocks, 9},
    {"detaillevel",&detailLevel, 0},
    // -ES- 1999/03/30 Added fov stuff.
    {"fieldofview",&cfgnormalfov, 90},
    {"zoomedfieldofview",&cfgzoomedfov, 10},
    {"vsync",&retrace, 0},
    {"screenwidth", &SCREENWIDTH, 320},
    {"screenheight",&SCREENHEIGHT, 200},
    {"bpp",&BPP, 1},

    {"darken_screen",&darken_screen, 1},

    {"snd_channels",&numChannels, 3},
    {"cdaudio",(int*)&cdaudio, 0},

    {"showmessages",&showmessages, 1},

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

int     numdefaults;
char*   defaultfile;


//
// M_SaveDefaults
//
void M_SaveDefaults (void)
{
  int           i;
  int           v;
  FILE* f;
        
  // Don't want to save settings in a network game: might not
  // be ours.
  if (netgame)
    return;

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
extern byte     scantokey[128];

void M_LoadDefaults (void)
{
    int         i;
    int         len;
    FILE*       f;
    char        def[80];
    char        strparm[100];
    char*       newstring = 0;
    int         parm;
    boolean     isstring;
    
    // set everything to base values
    numdefaults = sizeof(defaults)/sizeof(defaults[0]);
    for (i=0 ; i<numdefaults ; i++)
        *defaults[i].location = defaults[i].defaultvalue;
    
    // check for a custom default file
    i = M_CheckParm ("-config");
    if (i && i<myargc-1)
    {
        defaultfile = myargv[i+1];
        I_Printf ("     default file: %s\n",defaultfile);
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
    char                manufacturer;
    char                version;
    char                encoding;
    char                bits_per_pixel;

    unsigned short      xmin;
    unsigned short      ymin;
    unsigned short      xmax;
    unsigned short      ymax;
    
    unsigned short      hres;
    unsigned short      vres;

    unsigned char       palette[48];
    
    char                reserved;
    char                color_planes;
    unsigned short      bytes_per_line;
    unsigned short      palette_type;
    
    char                filler[58];
    unsigned char       data;           // unbounded
} pcx_t;


//
// WritePCXfile
//
void
WritePCXfile
( char*         filename,
  byte*         data,
  int           width,
  int           height,
  byte*         palette )
{
    int         i;
    int         length;
    pcx_t*      pcx;
    byte*       pack;
        
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;           // PCX id
    pcx->version = 5;                   // 256 color
    pcx->encoding = 1;                  // uncompressed
    pcx->bits_per_pixel = 8;            // 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;              // chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);       // not a grey scale
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
    *pack++ = 0x0c;     // palette ID byte
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
    static int          i = 0;

    // 25-6-98 KM Allow for 10000 screen shots.
    char        lbmname[13];
    static char *nopcx = NULL;

#ifdef DJGPP
    BITMAP *tempbitmap;
    PALETTE temppal;
#else
    byte*       linear;
#endif
    
    for (; i<10000 ; i++)
    {
    // find a file name to save it to
        sprintf(lbmname, "DOOM%04d.pcx", i);
        if (access(lbmname,0) == -1)
            break;      // file doesn't exist
    }
    if (i==10000)
    {
        // Errors suck.  Lets print a warning instead. 25-6-98 KM
        if (!nopcx)
        {
          // -ES- 10/08/19 Proper size of allocated block, for translation
          nopcx = Z_Malloc(24+strlen(DDF_LanguageLookup("PressAKey")),
                            PU_STATIC, &nopcx); // -ACB- LanguageLookup Used
          sprintf(nopcx,
                   "Couldn't create a PCX\n\n%s",
                     DDF_LanguageLookup("PressAKey")); // -ACB- LanguageLookup Used
        }
        M_StartMessage (nopcx, NULL, false);
        return;
    }
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


