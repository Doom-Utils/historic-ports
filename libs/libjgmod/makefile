####################################################
#                                                  #
#     JGMOD makefile by Guan Foo Wah               #
#                                                  #
####################################################

LIBDEST = $(DJDIR)/lib
INCDEST = $(DJDIR)/include
CPLIBDEST = $(subst /,\,$(LIBDEST))
CPINCDEST = $(subst /,\,$(INCDEST))

PARAM = -O2  -W -Wno-unused -Wall -m486 -ffast-math -fomit-frame-pointer

OBJS = obj/load_mod.o obj/load_s3m.o obj/mod.o obj/player.o


all : $(OBJS) libjgmod.a jgmod.exe
	copy libjgmod.a $(CPLIBDEST)
	copy jgmod.h $(CPINCDEST)
	@echo
	@echo Done. Please read readme.txt



obj/mod.o : mod.c jgmod.h
	gcc -c mod.c -o obj/mod.o $(PARAM)

obj/load_mod.o : load_mod.c jgmod.h
	gcc -c load_mod.c -o obj/load_mod.o $(PARAM)

obj/load_s3m.o : load_s3m.c jgmod.h
	gcc -c load_s3m.c -o obj/load_s3m.o $(PARAM)

obj/player.o : player.c jgmod.h
	gcc -c player.c -o obj/player.o $(PARAM)




libjgmod.a : load_mod.c player.c mod.c load_s3m.c
	ar rs libjgmod.a $(OBJS)
	

jgmod.exe : jgmod.c jgmod.h libjgmod.a
	gcc jgmod.c -o jgmod.exe -s  libjgmod.a -lalleg $(PARAM)



clean :
	del *.o 
	del *.a 
	del *.exe 
	del $(CPLIBDEST)\libjgmod.a 
	del $(CPINCDEST)\jgmod.h
