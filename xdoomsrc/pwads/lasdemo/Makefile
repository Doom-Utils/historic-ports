# Location of your Doom][ IWAD file
DOOM2WAD=/usr/local/games/xdoom/doom2.wad

lasdemo.wad: lasdemo.lwad lasdemo.gwad lasdemo.rwad
	rm -f lasdemo.wad
	wadcat -o lasdemo.wad lasdemo.lwad lasdemo.gwad lasdemo.rwad

lasdemo.gwad: lasdemo.wgc um_lasl.ppm um_lasr.ppm um_lassl.ppm um_lassr.ppm \
	um_lsig.ppm um_vlas.ppm sky1.ppm \
	umcomp1.ppm umcomp2.ppm umcomp3.ppm \
	laser01.ppm laser02.ppm laser03.ppm laser04.ppm laser05.ppm laser06.ppm
	wadext $(DOOM2WAD) PNAMES pnames.raw
	wadext $(DOOM2WAD) TEXTURE1 texture1.raw
	wadgc lasdemo.wgc lasdemo.gwad
	rm temp.* pnames.raw texture1.raw

lasdemo.rwad: comtxt.txt switches.lmp animated.lmp
	mkwad -p lasdemo.rwad @lasdemo.res

switches.lmp:
	swantbls swantbls.dat

animated.lmp:
	swantbls swantbls.dat

clean:
	rm -f *.bak lasdemo.lwad lasdemo.gwad lasdemo.rwad \
	switches.lmp animated.lmp

%.lwad: %.map
	idbsp -q $< $@
