# Location of your Doom][ IWAD file
DOOM2WAD=/usr/local/games/xdoom/doom2.wad

boomtst.wad: boomtst.lwad boomtst.gwad boomtst.rwad
	rm -f boomtst.wad
	wadcat -o boomtst.wad boomtst.lwad boomtst.gwad boomtst.rwad

boomtst.lwad: boomtst.map
	idbsp -q boomtst.map boomtst.lwad

boomtst.gwad: boomtst.wgc glass3f.ppm glass3b.ppm tlite.ppm
	wadext $(DOOM2WAD) PNAMES pnames.raw
	wadext $(DOOM2WAD) TEXTURE1 texture1.raw
	wadgc boomtst.wgc boomtst.gwad
	rm temp.* pnames.raw texture1.raw

boomtst.rwad: tran60.dat switches.lmp animated.lmp
	mkwad boomtst.rwad @boomtst.res

tran60.dat:
	mktran -v -f 60 -b 1.0 $(DOOM2WAD) tran60.dat

switches.lmp:
	swantbls swantbls.dat

animated.lmp:
	swantbls swantbls.dat

clean:
	rm -f *.bak temp.* *.raw tran*.dat \
	boomtst.lwad boomtst.gwad boomtst.rwad \
	switches.lmp animated.lmp
