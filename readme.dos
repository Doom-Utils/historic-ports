v0.1 of a DOS port of LinuxDOOM

-DOOM is copyright ID software (www.idsoftware.com)
-This particular port was written by Chi Hoang (cyhoang@acs.ucalgary.ca)
-I am not responsible for any damage that may be done by this program
-------------------------------------------------------------------------

        Well, today, id software released the doom source code, but only for
the linux version.  Since i don't happen to have linux installed on my
machine, i decided to port it to dos.  This little exe is the result.  You
might need cwsdpmi.exe, so copy it from your quake dir if you dont have it.
This has only been tested on my computer, with DOOM 2.

Whats implemented:
  -mouse
  -keyboard
  -graphics

Whats not implemented:
  -sound
  -networking
 
Programming Notes:
  -since this is a port of the unix version, it probably still looks for config
   data in .doomrc, not in default.cfg.  Actually, i havent tried, so i
   dunno for sure.
  -it took me 4-5 hours to port, half that time was spent tracking an
   incredibly stupid little bug (on my part).  Simply put, the program was
   printing to stdout (i.e.: printf) while in graphics mode and i couldnt
   figure out why the screen was scrolling
  -timer resolution is still only to 1/18.2 of a second, that may or may not
   make a difference.
  -almost all changes were made to i_*.c, but minor tweaks had to be made
   to other files.  Most of the new code was just ripped from other programs
   i wrote, except the scancode-to-ascii table which was from wolfenstein :)

Installation of Modified Source Code:
  - Do this in a COPY of the the doom source, cuz it'll overwrite files
  - u need djgpp 2.01
  - Note that you need long filenames to be enabled, so set lfn=y if needed,
    and make sure u're under win95.
  - U probably should use the patch.exe that comes with djgpp 2, cuz other
    patch.exe's might not do long filenames
  - Also note that the directory MUST be called linuxdoom-1.10, just like it
    originally was in the tarball
  - if the doom source is in c:\sources\linuxdoom-1.10, then go to c:\sources
    and type patch -p0 < dosdoom.patch.
  - Now make a directory under c:\sources\linuxdoom-1.10 called dos, or else
    it wont compile.
  - Type make to compile


