/* kb.h -- interface of the 'libkb' keyboard library
  version 1.00, 23 Jul 1996.

  Copyright (C) 1995, 1996 Markus Franz Xaver Johannes Oberhumer

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Markus F.X.J. Oberhumer
  markus.oberhumer@jk.uni-linz.ac.at
 */


#ifndef __LIBKB_KB_H
#define __LIBKB_KB_H

#define KB_LIBKB            KB_VERSION_ID

#define KB_VERSION_ID       0x1000          /* version + patchlevel */
#define KB_VERSION_STRING   "1.00"
#define KB_VERSION_DATE     "23 Jul 1996"

#if defined(MSDOS) || defined(__MSDOS__) || defined(__EMX__)
#  define __KB_MSDOS
#  if defined(__32BIT__) || defined(__FLAT__) || defined(__GNUC__)
#    define __KB_MSDOS32
#  else
#    define __KB_MSDOS16
#  endif
#elif defined(__linux__) && defined(__GNUC__)
#  define __KB_LINUX
#  define __KB_UNIX
#else
#  error unsupported compiler
#endif


#if defined(__KB_MSDOS)
#  include <conio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* utility macros for bits accessing */
#define KB_ALL_MASK(x,mask)         (((x) & (mask)) == (mask))
#define KB_ANY_MASK(x,mask)         (((x) & (mask)) != 0)
#define KB_NO_MASK(x,mask)          (((x) & (mask)) == 0)


/***********************************************************************
// keyboard access
************************************************************************/

/* installation of keyboard handler */
int kb_install(unsigned long flags);    /* install keyboard handler */
void kb_remove(void);                   /* remove keyboard handler */
#define kb_flags()                  ((unsigned long) _kb_flags)
#define kb_mode()                   ((int) _kb_mode)

/* update the keyboard - needed under Linux */
void kb_update(void);

/* access all keys */
#define kb_key(scan)                ((unsigned char) _kb_key[scan])
#define kb_keys_pressed()           ((int) _kb_keys_pressed)
#define kb_last_key()               ((unsigned short) _kb_last_key)
#define kb_shift()                  ((unsigned short) _kb_shift)

#define kb_last_key_set(k)          ((void) (_kb_last_key = (k)))
#define kb_shift_off(s)             ((void) (_kb_shift &= ~(s)))
#define kb_shift_on(s)              ((void) (_kb_shift |= (s)))

/* read a single keypress */
unsigned kb_keypress(void);         /* basic access function */
unsigned long kb_inkey(void);       /* full info keypress */
unsigned kb_getkey(void);           /* compatible to getkey() */
int kb_kbhit(void);                 /* compatible to kbhit() */


/* you are allowed to modify the following variables */
extern volatile unsigned short _kb_shift;   /* current shift state */
extern volatile unsigned short _kb_last_key;/* last key pressed */

/* these are READ ONLY - use the access-macros above */
extern volatile unsigned char _kb_key[128]; /* the key flags */
extern volatile int _kb_keys_pressed;       /* number of keys pressed */
extern unsigned long _kb_flags;             /* flags after kb_install() */
extern int _kb_mode;                        /* do we have a keyboard handler? */


/***********************************************************************
// flags - kb_install() and kb_flags()
// note: the only flag you probably need is emergency exit
************************************************************************/

/* pass these to kb_install() */
#define KB_FLAG_NO_ATEXIT           0x0001   /* do not use atexit() */
#define KB_FLAG_NO_SIGNAL           0x0002   /* do not install signal handler */
#define KB_FLAG_NO_LOCK             0x0004   /* do not lock memory */
#define KB_FLAG_SIGINT              0x0010   /* raise SIGINT on Control-C */
#define KB_FLAG_EMERGENCY_EXIT      0x0020   /* enable emergency exit */
#define KB_FLAG_EMERGENCY_SIGALRM   0x0040   /* enable SIGALRM emergency */
#define KB_FLAG_LINUX_NO_VT         0x0100   /* no virtual term switching */
#define KB_FLAG_LINUX_VT_NO_KEY     0x0200   /* release all keys after VT */
#define KB_FLAG_DJGPP_NO_RM         0x0400   /* no real-mode handler */
#define KB_FLAG_REPEAT_OFF          0x1000   /* turn off key repeat */

/* these information bits are set by kb_install() but do not cause an error */
#define KB_FLAG_ATEXIT_FAILED   0x00010000L  /* error atexit() */
#define KB_FLAG_SIGNAL_FAILED   0x00020000L  /* error installing signals */
#define KB_FLAG_LOCK_FAILED     0x00040000L  /* error while locking memory */

/* these information bits are set by kb_install() */
#define KB_FLAG_ATEXIT_DONE     0x00100000L  /* atexit() was called */
#define KB_FLAG_SIGNAL_DONE     0x00200000L  /* signal handler installed */
#define KB_FLAG_LOCK_DONE       0x00400000L  /* memory was locked */


/***********************************************************************
// keyboard scan codes - kb_key()
// 1-88,96-111 are fairly standard and should probably not be changed
************************************************************************/

/* get the name of a key */
const char *kb_keyname(int scan);

/* raw keys 1-88, 87 keys */
#define KB_SCAN_ESC             1
#define KB_SCAN_1               2 
#define KB_SCAN_2               3 
#define KB_SCAN_3               4
#define KB_SCAN_4               5
#define KB_SCAN_5               6
#define KB_SCAN_6               7
#define KB_SCAN_7               8
#define KB_SCAN_8               9
#define KB_SCAN_9               10
#define KB_SCAN_0               11
#define KB_SCAN_MINUS           12
#define KB_SCAN_EQUAL           13
#define KB_SCAN_BACKSPACE       14
#define KB_SCAN_TAB             15 
#define KB_SCAN_Q               16
#define KB_SCAN_W               17
#define KB_SCAN_E               18
#define KB_SCAN_R               19
#define KB_SCAN_T               20
#define KB_SCAN_Y               21
#define KB_SCAN_U               22
#define KB_SCAN_I               23
#define KB_SCAN_O               24
#define KB_SCAN_P               25
#define KB_SCAN_OPENBRACE       26
#define KB_SCAN_CLOSEBRACE      27
#define KB_SCAN_ENTER           28
#define KB_SCAN_LCONTROL        29    /* Left Control */
#define KB_SCAN_A               30
#define KB_SCAN_S               31
#define KB_SCAN_D               32
#define KB_SCAN_F               33
#define KB_SCAN_G               34
#define KB_SCAN_H               35
#define KB_SCAN_J               36
#define KB_SCAN_K               37
#define KB_SCAN_L               38
#define KB_SCAN_COLON           39
#define KB_SCAN_QUOTE           40
#define KB_SCAN_BACKQUOTE       41
#define KB_SCAN_LSHIFT          42    /* Left Shift */
#define KB_SCAN_BACKSLASH       43
#define KB_SCAN_Z               44
#define KB_SCAN_X               45
#define KB_SCAN_C               46
#define KB_SCAN_V               47
#define KB_SCAN_B               48
#define KB_SCAN_N               49
#define KB_SCAN_M               50
#define KB_SCAN_COMMA           51
#define KB_SCAN_PERIOD          52
#define KB_SCAN_SLASH           53
#define KB_SCAN_RSHIFT          54    /* Right Shift */
#define KB_SCAN_MULTIPLY_PAD    55
#define KB_SCAN_ALT             56    /* Left Alt */
#define KB_SCAN_LALT            KB_SCAN_ALT
#define KB_SCAN_SPACE           57
#define KB_SCAN_CAPSLOCK        58
#define KB_SCAN_F1              59
#define KB_SCAN_F2              60
#define KB_SCAN_F3              61
#define KB_SCAN_F4              62
#define KB_SCAN_F5              63
#define KB_SCAN_F6              64
#define KB_SCAN_F7              65
#define KB_SCAN_F8              66
#define KB_SCAN_F9              67
#define KB_SCAN_F10             68
#define KB_SCAN_NUMLOCK         69
#define KB_SCAN_SCRLOCK         70
#define KB_SCAN_7_PAD           71
#define KB_SCAN_8_PAD           72
#define KB_SCAN_9_PAD           73
#define KB_SCAN_MINUS_PAD       74
#define KB_SCAN_4_PAD           75
#define KB_SCAN_5_PAD           76
#define KB_SCAN_6_PAD           77
#define KB_SCAN_PLUS_PAD        78
#define KB_SCAN_1_PAD           79
#define KB_SCAN_2_PAD           80
#define KB_SCAN_3_PAD           81
#define KB_SCAN_0_PAD           82
#define KB_SCAN_PERIOD_PAD      83
#define KB_SCAN_SYS_REQ         84    /* Alt+Print */
#define KB_SCAN_ALT_PRINT       KB_SCAN_SYS_REQ
#define KB_SCAN_LAST_CONSOLE    KB_SCAN_SYS_REQ
                             /* 85 is not used */
#define KB_SCAN_LESS            86    /* '<' on my German keyboard */
#define KB_SCAN_F11             87
#define KB_SCAN_F12             88

/* prefixed keys mapped to unique scancodes, 5 keys */
#define KB_SCAN_ENTER_PAD       96    /* on keypad */
#define KB_SCAN_RCONTROL        97    /* Right Control */
#define KB_SCAN_DIVIDE_PAD      98    /* on keypad */
#define KB_SCAN_PRINT           99 
#define KB_SCAN_ALTGR           100   /* Alt Graphics = Right Alt */
#define KB_SCAN_RALT            KB_SCAN_ALTGR
                             /* 101 is used internally */

/* prefixed cursor keys mapped to unique scancodes, 10 keys */
#define KB_SCAN_HOME            102   /* KB_SCAN_7_PAD */
#define KB_SCAN_UP              103   /* KB_SCAN_8_PAD */
#define KB_SCAN_PGUP            104   /* KB_SCAN_9_PAD */
#define KB_SCAN_LEFT            105   /* KB_SCAN_4_PAD */
#define KB_SCAN_RIGHT           106   /* KB_SCAN_6_PAD */
#define KB_SCAN_END             107   /* KB_SCAN_1_PAD */
#define KB_SCAN_DOWN            108   /* KB_SCAN_2_PAD */
#define KB_SCAN_PGDN            109   /* KB_SCAN_3_PAD */
#define KB_SCAN_INSERT          110   /* KB_SCAN_0_PAD */
#define KB_SCAN_DELETE          111   /* KB_SCAN_PERIOD_PAD */

/* prefixed keys of new Microsoft keyboard (untested), 3 keys */
#define KB_SCAN_LBANNER         112   /* left window button */
#define KB_SCAN_RBANNER         113   /* right window button */
#define KB_SCAN_RMENU           114   /* menu button */


/***********************************************************************
// keyboard shift states - kb_shift()
************************************************************************/

/* shift flags */
#define KB_SHIFT_LSHIFT         0x0001
#define KB_SHIFT_RSHIFT         0x0002
#define KB_SHIFT_LCONTROL       0x0004
#define KB_SHIFT_RCONTROL       0x0008
#define KB_SHIFT_ALT            0x0010  
#define KB_SHIFT_ALTGR          0x0020 
#define KB_SHIFT_LALT           KB_SHIFT_ALT
#define KB_SHIFT_RALT           KB_SHIFT_ALTGR

/* these are toggles - they only change when the key is pressed */
#define KB_SHIFT_CAPSLOCK       0x0040
#define KB_SHIFT_NUMLOCK        0x0080
#define KB_SHIFT_SCRLOCK        0x0100
#define KB_SHIFT_INSERT         0x0200
#define KB_SHIFT_PAUSE          0x0400   /* this key has no scan code */
#define KB_SHIFT_CONTROL_BREAK  0x0800   /* this key has no scan code */

/* flags for masking multiple shift keys */
#define KB_SHIFT_ANY            0x0fff
#define KB_SHIFT_ANY_SHIFT      (KB_SHIFT_LSHIFT | KB_SHIFT_RSHIFT)
#define KB_SHIFT_ANY_CONTROL    (KB_SHIFT_LCONTROL | KB_SHIFT_RCONTROL)
#define KB_SHIFT_ANY_ALT        (KB_SHIFT_ALT | KB_SHIFT_ALTGR)

/* internal information, not very useful */
#define KB_SHIFT_OVERFLOW       0x1000   /* overflow occurred */
#define KB_SHIFT_UNKNOWN        0x2000   /* an unknown key was pressed */


/***********************************************************************
// advanced features: signals, keycodes and misc
************************************************************************/

typedef void (*kb_sighandler_t)(int);       /* signal handler function type */

extern const kb_sighandler_t kb_signal_handler;     /* libkb signal handler */

/* use this to install the libkb handler for additional signals */
kb_sighandler_t kb_signal(int signum, kb_sighandler_t handler);

/* look at source code for what these are useful */
kb_sighandler_t kb_get_chained_signal(int signum);
kb_sighandler_t kb_set_chained_signal(int signum, kb_sighandler_t handler);
extern const int _kb_nsig, _kb_sigmin, _kb_sigmax;

/* when using KB_FLAG_EMERGENCY_SIGALRM */
int _kb_signal_alarm_pause(void);
int _kb_signal_alarm_resume(void);

/* callback function that can be called from within keyboard handler */
typedef void (*kb_handler_callback_t)(int);
extern kb_handler_callback_t kb_handler_callback;

/* keycode conversion */
unsigned kb_keycode(unsigned k);
unsigned kb_os_keycode(unsigned k1, unsigned k2);
unsigned kb_bios_keycode(unsigned k);

/* single keypress access */
unsigned kb_keypeek(void);      /* peek for a keypress */
unsigned kb_waitkey(void);      /* wait for a keypress */
void kb_clearkeys(void);        /* clear keypress buffer */

/* single keypress access without interrupt disabling */
unsigned kb_keypress_i(void);   
unsigned long kb_inkey_i(void);
unsigned kb_getkey_i(void);   
int kb_kbhit_i(void);        
unsigned kb_keypeek_i(void); 
unsigned kb_waitkey_i(void);
void kb_clearkeys_i(void); 

/* set additional cleanup functions - look at source code */
void kb_set_cleanup( void (*before)(void), void (*after)(void) );
void kb_set_emergency_cleanup( void (*before)(void), void (*after)(void) );

/* misc. utility */
int kb_init(void);                 /* init keycode tables and internal vars */
int  _kb_iswin(void);       /* check if we are running in a Windows DOS box */
void _kb_usleep (unsigned long usec);        /* sleep for a very short time */

/* debugging */
void _kb_port_debug(void);


/***********************************************************************
// Portability wrappers for standard system functions.
// Operating system and (if applicable) BIOS keyboard access
//
// Don't get confused, these have absolutely nothing to do with the
// keyboard handler and are only here for your convenience.
************************************************************************/

int kb_os_kbhit(void);
unsigned kb_os_getkey(void);
unsigned kb_os_waitkey(void);      /* wait for a keypress */

int kb_bios_kbhit(void);
unsigned kb_bios_getkey(void);

/* a compatible kbhit()/getch() implementation for emx+gcc */
#if defined(__EMX__)
#undef kbhit
#undef getch
int kbhit(void);
int getch(void);
#endif



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4
*/
