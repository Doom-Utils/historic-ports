CPU=i486
CFLAGS = -g -O3 -m486 -march=$(CPU) -mcpu=$(CPU) -Wall
CC = gcc
OBJS = bcd.o

all: bcd libbcd.a

bcd: bcd.c
	$(CC) $(CFLAGS) -DSTANDALONE -o $@ $<

libbcd.a: $(OBJS)
	ar sru $@ $(OBJS)

clean:
	del *.o
	del bcd
	del bcd.exe

DIST = bcd_102.zip
DISTOBJ = file_id.diz bcd.doc bcd.c bcd.h makefile
dist: $(DIST)
$(DIST): $(DISTOBJ)
	del $(DIST)
	pkzip $(DIST) $(DISTOBJ)
