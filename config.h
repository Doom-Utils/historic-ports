/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */
/* acconfig.h for LxDoom
 *
 * $Id: acconfig.h,v 1.3 2000/01/25 21:33:22 cphipps Exp $
 * Parts Copyright (C) 1993-1996 by id Software, Inc.
 *
 * Process this file with autoheader to produce config.h.in,
 * which config.status then translates to config.h.  */

/* Define to the name of the distribution.  */
#define PACKAGE "lsdldoom"

/* Define to the version of the distribution.  */
#define VERSION "1.5"

/* Define to strcasecmp, if we have it */
#define stricmp strcasecmp

/* Define to strncasecmp, if we have it */
#define strnicmp strncasecmp

/* Define for gnu-linux target */
#define LINUX 1

/* Define for freebsd target */
/* #undef FREEBSD */

/* Define on I386 target */
#define I386 1

/* Define on big endian target */
/* #undef __BIG_ENDIAN__ */

/* Define for high resolution support */
#define HIGHRES 1

/* Define to enable internal range checking */
/* #undef RANGECHECK */

/* Define this to see real-time memory allocation
 * statistics, and enable extra debugging features 
 */
/* #undef INSTRUMENTED */

/* Uncomment this to exhaustively run memory checks
 * while the game is running (this is EXTREMELY slow).
 * Only useful if INSTRUMENTED is also defined.
 */
/* #undef CHECKHEAP */

/* Uncomment this to perform id checks on zone blocks,
 * to detect corrupted and illegally freed blocks
 */
#define ZONEIDCHECK 1

/* CPhipps - some debugging macros for the new wad lump handling code */
/* Defining this causes quick checks which only impose an overhead if a 
 *  posible error is detected. */
#define SIMPLECHECKS 1

/* Defining this causes time stamps to be created each time a lump is locked, and 
 *  lumps locked for long periods of time are reported */
/* #undef TIMEDIAG */

/* Define this to revoke any setuid status at startup (except to be reenabled 
 * when SVGALib needs it). Ony relevant to the SVGALib/Linux version */
#define SECURE_UID 1

/* Define to be the path where Doom WADs are stored */
#define DOOMWADDIR "${prefix}/share/games/doom"

/* Define to be the path to the sound server */
/* #undef SNDSERV_PATH */

/* Define to be the path to the lxmusserver */
/* #undef MUSSERV_PATH */

/* Define if you have library -lXext */
/* #undef HAVE_LIBXEXT */

/* Define if you have the DGA library -lXxf86dga */
/* #undef HAVE_LIBXXF86DGA */

/* 
 * $Log: acconfig.h,v $
 * Revision 1.3  2000/01/25 21:33:22  cphipps
 * Fix security in case of being setuid
 *
 * Revision 1.2  1999/10/02 11:43:37  cphipps
 * Added autoconf options to control diagnostics
 *
 * Revision 1.1  1999/09/10 20:09:11  cphipps
 * Initial revision
 *
 */

/* Define to 1 if you have the declaration of `sys_siglist', and to 0 if you
   don't. */
#define HAVE_DECL_SYS_SIGLIST 1

/* Define to 1 if you have the `inet_aton' function. */
#define HAVE_INET_ATON 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <linux/joystick.h> header file. */
#define HAVE_LINUX_JOYSTICK_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/filio.h> header file. */
/* #undef HAVE_SYS_FILIO_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Name of package */
#define PACKAGE "lsdldoom"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.5"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */
