# Location of your Doom][ IWAD file
DOOM2WAD=/usr/local/games/xdoom/doom2.wad

plustst.wad: plustst.lwad plustst.gwad plustst.rwad
	rm -f plustst.wad
	wadcat -o plustst.wad plustst.lwad plustst.gwad plustst.rwad

plustst.lwad: plustst.map plustst.aco
	idbsp -q -b plustst.aco plustst.map plustst.lwad

plustst.gwad: plustst.wgc glass1.ppm
	wadext $(DOOM2WAD) PNAMES pnames.raw
	wadext $(DOOM2WAD) TEXTURE1 texture1.raw
	wadgc plustst.wgc plustst.gwad

plustst.rwad: switches.lmp animated.lmp
	mkwad -p plustst.rwad @plustst.res

switches.lmp:
	swantbls swantbls.dat

animated.lmp:
	swantbls swantbls.dat

plustst.aco: plustst.acs
	acc plustst.acs plustst.aco

clean:
	rm -f *.bak *.raw temp.* tmp.wad plustst.lwad plustst.gwad \
	plustst.rwad switches.lmp animated.lmp plustst.aco
