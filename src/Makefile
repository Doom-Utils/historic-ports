#* 	makefile
#*
#* Make script for DOOM, using MIDAS build option files
#*
#* $Id: Makefile,v 1.7 1998/01/07 18:44:17 pekangas Exp $
#*
#* Petteri Kangaslampi, pekangas@sci.fi

# Force target:
ifndef TARGET
  TARGET = win32
endif

# MIDAS directories:
MIDASDIR=../midas11
FIXMIDASDIR=..\\midas11

# Includes and libraries: - use retail MIDAS lib always
INCPATH=$(MIDASDIR)/src/midas
LIBPATH=$(MIDASDIR)/lib/$(_TARGET)/$(_COMP)retail
LIBS=midas
WIN32LIBS = user32.lib winmm.lib advapi32.lib gdi32.lib wsock32.lib

# Additional defines - use assembler:
DEFINES=X86_ASM

# Include build options:
BLDOPTSDIR=$(MIDASDIR)/src/bldopts
include $(BLDOPTSDIR)/bldopts.mak


# Ugly code assumes enums are ints:
ifdef _WC
  CCOPTS += -ei
endif


# Default target:
all :		ntdoom$(_EXE)

# Get build pattern rules:
include $(BLDOPTSDIR)/bldrules.mak

OBJS=				\
		doomdef.$(O)		\
		doomstat.$(O)		\
		dstrings.$(O)		\
		i_system.$(O)		\
		midassound.$(O)		\
		win32video.$(O)		\
		i_net.$(O)			\
		tables.$(O)			\
		f_finale.$(O)		\
		f_wipe.$(O) 		\
		d_main.$(O)			\
		d_net.$(O)			\
		d_items.$(O)		\
		g_game.$(O)			\
		m_menu.$(O)			\
		m_misc.$(O)			\
		m_argv.$(O)  		\
		m_bbox.$(O)			\
		m_fixed.$(O)		\
		m_swap.$(O)			\
		m_cheat.$(O)		\
		m_random.$(O)		\
		am_map.$(O)			\
		p_ceilng.$(O)		\
		p_doors.$(O)		\
		p_enemy.$(O)		\
		p_floor.$(O)		\
		p_inter.$(O)		\
		p_lights.$(O)		\
		p_map.$(O)			\
		p_maputl.$(O)		\
		p_plats.$(O)		\
		p_pspr.$(O)			\
		p_setup.$(O)		\
		p_sight.$(O)		\
		p_spec.$(O)			\
		p_switch.$(O)		\
		p_mobj.$(O)			\
		p_telept.$(O)		\
		p_tick.$(O)			\
		p_saveg.$(O)		\
		p_user.$(O)			\
		r_bsp.$(O)			\
		r_data.$(O)			\
		r_draw1.$(O)			\
		r_draw2.$(O)			\
		r_main.$(O)			\
		r_plane.$(O)		\
		r_segs.$(O)			\
		r_sky.$(O)			\
		r_things.$(O)		\
		w_wad.$(O)			\
		wi_stuff.$(O)		\
		v_video1.$(O)		\
		v_video2.$(O)		\
		st_lib.$(O)			\
		st_stuff.$(O)		\
		hu_stuff.$(O)		\
		hu_lib.$(O)			\
		s_sound.$(O)		\
		z_zone.$(O)			\
		info.$(O)				\
		sounds.$(O)	\
		multires.$(O) \
		funcpntr.$(O) \
		qmus2mid.$(O) \
		drawasm.$(O) \
		i_main.$(O)

ntdoom$(_EXE):	$(OBJS)
	$(BUILDEXE)


# Cleanup

clean distclean cvsclean :
	$(CLEANALL)

#############################################################
#
#############################################################