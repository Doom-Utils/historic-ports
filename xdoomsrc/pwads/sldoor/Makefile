# Location of your Doom][ IWAD file
DOOM2WAD=/usr/local/games/xdoom/doom2.wad
#DOOM2WAD=c:\games\doom2\doom2.wad

sldoor.wad: sldoor.lwad sldoor.gwad sldoor.rwad
	rm -f sldoor.wad
	wadcat -o sldoor.wad sldoor.lwad sldoor.gwad sldoor.rwad

sldoor.lwad: sldoor.map
	idbsp -q sldoor.map sldoor.lwad

sldoor.gwad: sldoor.wgc gdoor1f1.ppm gdoor1f2.ppm gdoor1f3.ppm gdoor1f4.ppm \
	gdoor1f5.ppm gdoor1f6.ppm gdoor1f7.ppm gdoor1f8.ppm
	wadext $(DOOM2WAD) PNAMES pnames.raw
	wadext $(DOOM2WAD) TEXTURE1 texture1.raw
	wadgc sldoor.wgc sldoor.gwad
	rm temp.* pnames.raw texture1.raw

sldoor.rwad: comtxt.txt
	mkwad -p sldoor.rwad COMTXT comtxt.txt

clean:
	rm -f *.bak sldoor.gwad sldoor.lwad sldoor.rwad
