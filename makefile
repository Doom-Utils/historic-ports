#
# Makefile for DOSDoom 
#
CC=gcc

#DEBUG=1
#ERIKDOESNOTWANTYOUTOUSETHISBUTITISCOOL=1
#PROFILE=1
RELEASE=1

ifdef CPU
CPUFLAGS=-m$(CPU) -march=$(CPU) -mcpu=$(CPU)
else
CPUFLAGS=-m486
endif


ifdef DEBUG
LIBS=-lemu -lalleg -lbcd 
LDFLAGS= 
CFLAGS=-g $(CPUFLAGS) -DDEVELOPERS -Wall
endif

ifdef PROFILE
LIBS=-lemu -lalleg -lbcd
LDFLAGS= 
CFLAGS=-pg -O3 -ffast-math $(CPUFLAGS)
endif

ifdef ERIKDOESNOTWANTYOUTOUSETHISBUTITISCOOL
LIBS=-lemu -lalleg -lbcd 
LDFLAGS= -Xlinker -s
CFLAGS=-O3 -ffast-math -fomit-frame-pointer $(CPUFLAGS) -DBLUR
endif

ifdef RELEASE
LIBS=-lemu -lalleg -lbcd 
LDFLAGS= -Xlinker -s
CFLAGS=-O3 -ffast-math -fomit-frame-pointer $(CPUFLAGS)
endif

L=libs
O=obj

$(O)/%.o: %.c; $(CC) $(CFLAGS) -c $< -o $@
$(O)/%.o: %.S; $(CC) $(CFLAGS) -c $< -o $@
$(O)/%.o: %.s; $(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

OBJS   =$(O)/am_map.o $(O)/d_main.o $(O)/d_net.o\
        $(O)/ddf_anim.o\
        $(O)/ddf_atk.o\
        $(O)/ddf_game.o\
        $(O)/ddf_lang.o\
        $(O)/ddf_levl.o\
        $(O)/ddf_line.o\
        $(O)/ddf_main.o\
        $(O)/ddf_mobj.o\
        $(O)/ddf_sect.o\
        $(O)/ddf_sfx.o\
        $(O)/ddf_swth.o\
        $(O)/ddf_weap.o\
        $(O)/dm_defs.o\
        $(O)/dm_state.o $(O)/dstrings.o $(O)/f_finale.o $(O)/f_wipe.o\
        $(O)/g_game.o $(O)/hu_lib.o $(O)/hu_stuff.o $(O)/i_allegv.o\
        $(O)/i_music.o $(O)/i_net.o $(O)/i_sound.o $(O)/i_system.o\
        $(O)/i_video.o $(O)/lu_math.o\
        $(O)/m_argv.o $(O)/m_bbox.o $(O)/m_cheat.o\
        $(O)/m_fixed.o $(O)/m_menu.o\
        $(O)/m_misc.o $(O)/m_option.o $(O)/m_random.o $(O)/m_swap.o\
        $(O)/mus_midi.o $(O)/mus_mod.o $(O)/mus_mp3.o $(O)/mus_mus.o\
        $(O)/p_action.o\
        $(O)/p_enemy.o $(O)/p_plane.o\
        $(O)/p_inter.o $(O)/p_lights.o $(O)/p_map.o $(O)/p_maputl.o\
        $(O)/p_mobj.o\
        $(O)/p_pspr.o $(O)/p_saveg.o\
        $(O)/p_setup.o\
        $(O)/p_sight.o\
        $(O)/p_spec.o\
        $(O)/p_switch.o\
        $(O)/p_tick.o $(O)/p_user.o $(O)/r_bsp.o\
        $(O)/r_column.o\
        $(O)/r_data.o $(O)/r_draw1.o\
        $(O)/r_draw2.o $(O)/r_main.o $(O)/r_plane.o $(O)/r_segs.o\
        $(O)/r_sky.o\
        $(O)/r_span.o\
        $(O)/r_things.o $(O)/rad_trig.o $(O)/s_sound.o $(O)/st_lib.o\
        $(O)/st_stuff.o $(O)/v_func.o $(O)/v_res.o $(O)/v_video1.o\
        $(O)/v_video2.o $(O)/w_wad.o $(O)/wi_stuff.o $(O)/z_zone.o\
        $(L)/libamp.a $(L)/libjgmod.a 

all:    $(O)/dosdoom.exe

clean:
	rm -f *.o *~ *.flc
	rm -f dos/*

$(O)/dosdoom.exe:   $(OBJS) $(O)/i_main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_main.o \
        -o $(O)/dosdoom.exe $(LIBS)

