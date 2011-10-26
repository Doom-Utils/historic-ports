#
# automake Makefile.am for the LxDoom source directory
#
# $Id: Makefile.am,v 1.7.2.1 2000/03/28 17:13:43 cph Exp $
#
# Process this file with automake to produce Makefile.in
#
#
gamesdir=$(prefix)/games
EXTRA_PROGRAMS = lsdldoom lsdldoom-game-server
games_PROGRAMS = @BUILD_SDLDOOM@ @BUILD_SERVER@

if I386_ASM
ASMS = drawspan.S drawcol.S drawasm.h
else
ASMS = 
endif

AM_CFLAGS = @SDL_CFLAGS@ @MIXER_CFLAGS@ @NET_CFLAGS@ @CFLAGS@ 

lsdldoom_game_server_SOURCES = d_server.c l_udp.c protocol.h l_system.c
lsdldoom_game_server_LDADD = @NET_LIBS@ @SDL_LIBS@

COMMON_SRC = \
 am_map.c       g_game.c           p_maputl.h       r_plane.h   \
 am_map.h       g_game.h           p_mobj.c         r_segs.c    \
 hu_lib.c       lprintf.c          p_mobj.h         r_segs.h    \
 hu_lib.h       lprintf.h          p_plats.c        r_sky.c	\
 d_deh.c        hu_stuff.c         m_argv.c         p_pspr.c    r_sky.h	    \
 d_deh.h        hu_stuff.h         m_argv.h         p_pspr.h    r_state.h   \
 d_englsh.h     i_joy.h            m_bbox.c         p_saveg.c   r_things.c  \
 d_event.h      i_net.h            m_bbox.h         p_saveg.h   r_things.h  \
 d_items.c      i_network.h        m_cheat.c        p_setup.c   s_sound.c   \
 d_items.h      i_sound.h          m_cheat.h        p_setup.h   s_sound.h   \
 d_main.c       i_system.h         m_fixed.h        p_sight.c   sounds.c    \
 d_main.h       i_video.h          m_menu.c         p_spec.c    sounds.h    \
                info.c             m_menu.h         p_spec.h    st_lib.c    \
 d_net.h        info.h             m_misc.c         p_switch.c  st_lib.h    \
 d_player.h     l_joy.c            m_misc.h         p_telept.c  st_stuff.c  \
 m_random.c     p_tick.c           st_stuff.h       l_main.c    i_main.h    \
 d_think.h      m_random.h         p_tick.h         tables.c    \
 d_ticcmd.h     m_swap.h           p_user.c         tables.h    l_system.c  \
 doomdata.h     p_ceilng.c         p_user.h         v_video.c   \
 doomdef.c      p_doors.c          protocol.h       v_video.h   \
 doomdef.h      p_enemy.c          r_bsp.c          version.c   \
 doomstat.c     p_enemy.h          r_bsp.h          version.h   \
 doomstat.h     p_floor.c          r_data.c         w_wad.c	\
 doomtype.h     p_genlin.c         r_data.h         w_wad.h	\
 dstrings.c     p_inter.c          r_defs.h         wi_stuff.c  \
 dstrings.h     p_inter.h          r_draw.c         wi_stuff.h  \
 f_finale.c     p_lights.c         r_draw.h         z_bmalloc.c \
 f_finale.h     p_map.c            r_main.c         z_bmalloc.h \
 f_wipe.c       p_map.h            r_main.h         z_zone.c    \
 f_wipe.h       p_maputl.c         r_plane.c        z_zone.h    $(ASMS)

if HAVE_NET
NET_CLIENT_SRC = d_client.c l_udp.c
else
NET_CLIENT_SRC = d_client.c
endif

lsdldoom_SOURCES = l_sdl.c l_video_trans.h l_video_trans.c l_video_sdl.c l_sound_sdl.c qmus2mid.c qmus2mid.h $(COMMON_SRC) $(NET_CLIENT_SRC)

lsdldoom_LDADD = @MIXER_LIBS@ @NET_LIBS@ @SDL_LIBS@ -lm

#
# $Log: Makefile.am,v $
# Revision 1.7.2.1  2000/03/28 17:13:43  cph
# Remove old makefile
#
# Revision 1.7  2000/03/19 20:14:32  cph
# Sound code cleaning: DosDoom, IPC and such unused code removed
#
# Revision 1.6  1999/12/13 17:24:08  cphipps
# Add i_main.h to lxdoom/lsdoom
#
# Revision 1.5  1999/10/31 16:32:21  cphipps
# Added l_system.c to lxdoom-game-server
#
# Revision 1.4  1999/10/27 07:41:53  cphipps
# Remove overriding of CFLAGS, so configure can set them according to
# architecture
#
# Revision 1.3  1999/10/02 15:00:48  cphipps
# Added X_CFLAGS to the compile flags, so autoconf can pass the path to
# the X11 include files for systems that don't put them in the main include
# tree (fixes compile problem on FreeBSD)
#
# Revision 1.2  1999/09/16 22:19:22  cphipps
# Add missing header files for sound and netgame servers
#
# Revision 1.1  1999/09/10 20:10:47  cphipps
# Initial revision
#
#
