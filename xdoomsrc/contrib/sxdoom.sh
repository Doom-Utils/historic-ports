#!/bin/sh

#
# If you modified LDIR in the top level makefile, you need to modify
# it here to the same directory too.
#

# *** PID BEGIN ***
# Changed this to use a psdoom directory rather than xdoom.
LDIR=/usr/local/games/psdoom
# old code:
#LDIR=/usr/local/games/xdoom
# *** PID END ***

[ "$DOOMWADDIR" ] || DOOMWADDIR=${LDIR}
PATH=${LDIR}:${PATH}
export DOOMWADDIR PATH

# *** PID BEGIN ***
exec ${LDIR}/ps-sxdoom $*
# old code:
#exec ${LDIR}/sxdoom $*
# *** PID END ***
