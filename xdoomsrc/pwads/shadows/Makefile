shadows.wad: shadows.lwad shadows.rwad
	rm -f shadows.wad
	wadcat -o shadows.wad shadows.lwad shadows.rwad

shadows.lwad: shadows.map
	idbsp -q shadows.map shadows.lwad
	#wadlc shadows.map tmp.wad
	#bsp -noprog tmp.wad -o shadows.lwad
	#rm tmp.wad

shadows.rwad: comtxt.txt
	mkwad -p shadows.rwad COMTXT comtxt.txt

clean:
	rm -f *.bak shadows.lwad shadows.rwad
