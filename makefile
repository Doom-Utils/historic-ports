#
# Makefile for DOSDoom 
#
OPTIMISATION_FLAGS=-mpentiumpro
#DEBUGFLAGS=-g
DEBUGFLAGS=
DEFINES=-DSMOOTHING

include dosdoom.mak

win32: force
	make -C win32 -f dosdoom.mak OPTIMISATION_FLAGS=$(OPTIMISATION_FLAGS) DEBUGFLAGS=$(DEBUGFLAGS) DEFINES=$(DEFINES)

clean: force
	rm -f obj/*.o
	make -C $(libamp.a.dir) -f libamp.mak clean
	make -C $(libjgmod.a.dir) -f libjgmod.mak clean
	make -C $(libbcd.a.dir) -f libbcd.mak clean
	make -C win32 -f dosdoom.mak clean

force::