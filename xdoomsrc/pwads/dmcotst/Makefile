dmcotst.wad: dmcotst.lwad dmcotst.rwad
	rm -f dmcotst.wad
	wadcat -o dmcotst.wad dmcotst.lwad dmcotst.rwad

dmcotst.lwad: dmcotst.map
	idbsp -q dmcotst.map dmcotst.lwad

dmcotst.rwad: comtxt.txt
	mkwad -p dmcotst.rwad COMTXT comtxt.txt

clean:
	rm -f *.bak dmcotst.lwad dmcotst.rwad
