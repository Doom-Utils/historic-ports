
#DEBUG = -DDEBUG

# ---- MS-DOS/pc ----

AR = ar
CC = gcc
DEFINES = -DNO_BYTE_SWAPPING -DOS_MSDOS -DARCH_i586
CFLAGS = -g -O3 -ffast-math -m486 \
	 ${DEFINES} ${DEBUG}
LDFLAGS = -pg

OBJ = dump.o formats.o getbits.o huffman.o layer3.o getdata.o misc2.o transform.o \
      audiolib.o audioalg.o position.o layer2.o rtbuf.o guicontrol.o 

DEMOOBJ = libdemo.o

LIBS = -lalleg -lm

all: libamp.a libdemo.exe

.PHONY: clean dep
clean:
	rm -f *.o libamp.a libdemo.exe

libamp.a: $(OBJ)
	$(AR) rs libamp.a $(OBJ)

libdemo.exe: $(DEMOOBJ) libamp.a
	$(CC) -o libdemo.exe $(DEMOOBJ) libamp.a $(LDFLAGS) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $(GGDB) $(PG) $< 

.c.s:
	$(CC) -S $(CFLAGS) $(GGDB) $(PG) $< 

dep:
	gcc -MM ${DEFINES} ${DEBUG} *.c > .depend

#-include .depend
