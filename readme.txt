        NTDOOM release 1998-01-29
        -------------------------

        http://www.s2.org/ntdoom/

Petteri Kangaslampi, pekangas@sci.fi

This is the third release of my Win32 port of the Public DOOM 1.10 source
code. I'm aware of at least two other Win32 ports, one by Andy Bay and other
by Bruce Lewis, but neither of them currently work on my NT machine, and
neither have multiresolution support at the moment. At some point it is
probably useful to combine these ports, and possibly even the DOS port by Chi
Hoang, to a single source base - any volunteers?

New in this version:
	- Highcolor and other fixes from Chi Hoang's DOSDOOM 0.45 - highcolor
	  looks much better now
	- Other minor fixes and updates

Some highlights:
        - Uses windowed graphics using StretchDIBits, DirectDraw not required
        - Keyboard input works, apart from Alt
        - Mouse works, if you have problems with -grabmouse, try alt-tabbing
          to a different window and back again
        - Sound, using MIDAS Digital Audio System, supports both standard wave
          output and DirectSound
        - Music, grabbed from Andy Bay's Win32 port. Uses QMUS2MID code by
          Sebastien Bacquet and Hans Peter Verne. The QMUS2MID code is
          distributed under the GNU General Public License, see
          COPYING.
        - Support for resolutions over 320x200, and highcolor support, grabbed
          from Chi Hoang's DosDoom port. Highcolor support doesn't seem to
          make that big of a difference in the display, but doesn't slow the
          game much down either.
        - TCP/IP networking might work, but is still not tested
        - Source code compiles without warnings with Watcom C 10.6 and
          Visual C 4.2
        - I have already done some preliminary assembler optimizations, more
          is to come.
	- Run from a shell window if possible (Command Prompt or whatever),
          so that you can actually see the error messages if NTDOOM crashes
	- The game version is detected from the WAD file name, like the
          original code does. Use "doom1.wad" for DOOM1 shareware, "doom.wad"
          for DOOM1 registered, "doom2.wad" for DOOM2 and "doomu.wad" for
          Ultimate DOOM.	

What certainly doesn't work:
        - Joystick

The port was done under Windows NT, and works pretty much flawlessly under
it. Win95 was quickly tested, and mostly works too. This port is mainly
targetted at Windows NT though, as the name already suggests, and NT support
is always priority number 1. Using a 16-bit or better display mode is
recommended, but 8-bit modes should work reasonably well too.

Quick documentation on the command line options:
        -grabmouse      grabs the mouse to the DOOM window and hides it,
                        recommended if you use mouse
        -novertmouse    disables mouse vertical movement -- very
                        useful if you are used to Quake
        -2, -3, -4      multiplies the pixels to make the window
                        bigger. Unlike the X version, this port does not
                        stretch the pixels in software, but rather lets the
                        display adapter do that -- most modern display
                        adapters have hardware stretching
        -nosound        disables sound effects
        -wavonly        do not use DirectSound
        -primarysound   use DirectSound in primary buffer mode --
                        might help to lower latency, but can be problematic.
        -nomusic        disables MIDI music
        -width xx       sets rendering width to xx pixels
        -height yy      sets rendering height to yy pixels
        -hicolor        enables hicolor support, only for 16bpp and better
                        display modes

As a hint, the options I normally use are
        ntdoom -grabmouse -novertmouse -width 320 -height 240 -2 -nomusic

I have been planning to write a more user-friendly front-end, maybe some
day...

As Alt currently doesn't work, strafing needs to be done with separate strafe
keys, '.' and ',' by default. The file "sample.doomrc" includes an example
configuration for players used to Quake -- cursor keys are used for moving and
strafing and the mouse for turning. Copy it to ".doomrc" in either your home
directory or the current directory.

The source compiles with both Visual C and Watcom C. To compile, you'll need
MIDAS Digital Audio System 1.1.1, and a suitable compiler, plus GNU Make if
you want to use the default Makefile and NASM to use the assembler sources.
MIDAS is available at http://www.s2.org/midas/, and precompiled binaries for
GNU Make and NASM at http://www.s2.org/ntdoom/. The Makefile doesn't do
anything too magic, so you should be able to build a new Makefile or project
file for your favorite tools easily.

Anyway, have fun. Bug reports are welcome, concrete fix suggestions as well as
new feature ideas more than welcome, and actual source code for fixes and
updates extremely well appreciated. E-mail preferred, pekangas@sci.fi. The
latest version, as well as other NTDOOM information, can always be found at
http://www.s2.org/ntdoom/.


-- Petteri
