# Make this point to the directory with your Doom IWAD file, you are
# going to use. The first line is for UNIX systems, the second line
# if you use the Cygwin tools for Windows.

DOOMWADDIR=/usr/local/games/xdoom
#DOOMWADDIR=h:/doom/doom2

# Maybe you need to change this line, if you don't own Doom][.
# It doesn't matter from which IWAD file you get the color tables,
# to calculate the translucency table from, even the Doom 1
# shareware IWAD is fine.
IWAD=doom2.wad

all: xdoom.wad

xdoom.wad: xdoom.rwad xdoom.gwad
	rm -f xdoom.wad
	wadcat -o xdoom.wad xdoom.gwad xdoom.rwad
	rm xdoom.rwad xdoom.gwad

xdoom.gwad: xdoom.wgc m_cdvol.ppm comdev.ppm
	wadgc xdoom.wgc xdoom.gwad
	rm temp.*

xdoom.rwad: tranmap.dat switches.lmp animated.lmp
	mkwad -v -p xdoom.rwad @xdoom.res

tranmap.dat:
	mktran -v -f 50 -b 1.2 $(DOOMWADDIR)/$(IWAD) tranmap.dat

switches.lmp:
	swantbls swantbls.dat

animated.lmp:
	swantbls swantbls.dat

clean:
	rm -f tranmap.dat switches.lmp animated.lmp

allclean:
	make clean
	rm -f xdoom.wad
