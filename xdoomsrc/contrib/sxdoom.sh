#!/bin/sh

#
# If you modified LDIR in the top level makefile, you need to modify
# it here to the same directory too.
#
LDIR=/usr/local/games/xdoom
#LDIR=/u/local/games/xdoom

[ "$DOOMWADDIR" ] || DOOMWADDIR=${LDIR}
PATH=${LDIR}:${PATH}
export DOOMWADDIR PATH

exec ${LDIR}/sxdoom $*
