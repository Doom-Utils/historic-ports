# Top-level Makefile for Xdoom
# Originally created by Ron Record (rr@sco.com) 07-Mar-98
#
# Modified by various people and I lost track who did what, so no one
# else mentioned (and that includes me self) to be fair, sorry.

#
# Adjust this destination directories to your local environment.
# If you modify DESTDIR or DLIB you need to modify the path in
# contrib/xdoom.sh, sxdoom.sh and glxdoom.sh to the same directory!
# Also LLIB must be the same as LLIBDIR in the Makefile used to
# install xwadtools, else acc won't be able to find its include
# files!
#
# *** PID BEGIN ***
# Modified install portion of makefile to install ps-*doom files.
# Didn't explicitly denote where this happens in the file.
# *** PID END ***
#
DESTDIR=/usr
#DESTDIR=/u
DBIN=$(DESTDIR)/local/bin

# *** PID BEGIN ***
# Changed the DLIB to be /local/games/psdoom; changed it in the
# contrib/*.sh scripts as well.
DLIB=$(DESTDIR)/local/games/psdoom
# old code:
#DLIB=$(DESTDIR)/local/games/xdoom
# *** PID END ***

LLIB=$(DESTDIR)/local/lib
DMAN=$(DESTDIR)/local/man

# -----------------------------------------------------------------------
# It should not be necessary to modify anything below this line. If you
# have to do that I would like to know, because it probably is a problem
# which could be fixed.
# -----------------------------------------------------------------------

#
# Only used for FreeBSD yet, the install command is not portable between
# the various UNIX's, so I prefer to do that my self with mkdir, cp, chmod...
# which usually works on any UNIX.
#
INSTALL=/usr/bin/install

all:
	@echo "You must specify a target for one of the following"
	@echo "supported OS's:"
	@echo
	@echo "FreeBSD:                 make freebsd"
	@echo "FreeBSD + vgllib:        make freebsd-svga"
	@echo "Linux x86:               make linux-x86"
	@echo "Linux x86 + svgalib:     make linux-x86-svga"
	@echo "Linux x86 + Voodoo 3DFX: make linux-x86-3dfx"
	@echo "Linux Dec Alpha:         make linux-alpha"
	@echo "OpenServer 5:            make os5"
	@echo "Unixware 2:              make uw2"
	@echo "Unixware 7:              make uw7"
	@echo "Solaris SPARC:           make solsparc"
	@echo "IBM AIX:                 make aix"

os5: musserv-os5 sndserv-os5 xdoom-os5

uw7: musserv-uw7 sndserv-uw7 xdoom-uw7

uw2: musserv-uw2 sndserv-uw2 xdoom-uw2

linux-x86: musserv-linux sndserv-linux xdoom-linux-x86

linux-x86-svga: musserv-linux sndserv-linux xdoom-linux-x86-svga

linux-x86-3dfx: musserv-linux sndserv-linux xdoom-linux-x86-3dfx

linux-alpha: xdoom-alpha

freebsd: musserv-freebsd sndserv-freebsd xdoom-freebsd

freebsd-svga: musserv-freebsd sndserv-freebsd xdoom-freebsd-svga

solsparc: xdoom-solsparc

aix: xdoom-aix

sndserv-uw2:
	(cd sndserv; ${MAKE} -f Makefile.scouw2)

sndserv-uw7:
	(cd sndserv; ${MAKE} -f Makefile.scouw7)

sndserv-os5:
	(cd sndserv; ${MAKE} -f Makefile.scoos5)

sndserv-linux:
	(cd sndserv; ${MAKE} -f Makefile.linux)

sndserv-freebsd:
	(cd sndserv; ${MAKE} -f Makefile.freebsd)

xdoom-uw2:
	(cd xdoom; ${MAKE} -f Makefile.scouw2 all)

xdoom-uw7:
	(cd xdoom; ${MAKE} -f Makefile.scouw7 all)

xdoom-os5:
	(cd xdoom; ${MAKE} -f Makefile.scoos5 all)

xdoom-linux-x86:
	(cd xdoom; ${MAKE} -f Makefile.linux-x86)

xdoom-linux-x86-svga:
	(cd xdoom; ${MAKE} -f Makefile.linux-x86 linux-x86/sxdoom)

xdoom-linux-x86-3dfx:
	(cd xdoom; ${MAKE} -f Makefile.linux-x86 linux-x86/glxdoom)

xdoom-alpha:
	(cd xdoom; ${MAKE} -f Makefile.linux-alpha)

xdoom-freebsd:
	(cd xdoom; ${MAKE} -f Makefile.freebsd)

xdoom-freebsd-svga:
	(cd xdoom; ${MAKE} -f Makefile.freebsd freebsd/sxdoom)

xdoom-solsparc:
	(cd xdoom; ${MAKE} -f Makefile.solsparc)

xdoom-aix:
	(cd xdoom; ${MAKE} -f Makefile.aix)

musserv-uw2:
	(cd musserv; ${MAKE} -f Makefile.scouw2)

musserv-uw7:
	(cd musserv; ${MAKE} -f Makefile.scouw7)

musserv-os5:
	(cd musserv; ${MAKE} -f Makefile.scoos5)

musserv-linux:
	(cd musserv; ${MAKE} -f Makefile.linux)

musserv-freebsd:
	(cd musserv; ${MAKE} -f Makefile.freebsd)

install:
	@echo "You must specify a target for one of the following"
	@echo "supported OS's:"
	@echo
	@echo "FreeBSD:                 make install-freebsd"
	@echo "FreeBSD + vgllib:        make install-freebsd-svga"
	@echo "Linux x86:               make install-linux-x86"
	@echo "Linux x86 + svgalib      make install-linux-x86-svga"
	@echo "Linux x86 + Voodoo 3DFX: make install-linux-x86-3dfx"
	@echo "Linux Dec Alpha:         make install-linux-alpha"
	@echo "OpenServer 5:            make install-os5"
	@echo "Unixware 2:              make install-uw2"
	@echo "Unixware 7:              make install-uw7"
	@echo "Solaris SPARC:           make install-solsparc"
	@echo "IBM AIX:                 make install-aix"

install-uw7:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp musserv/unixware7/musserver $(DLIB)/musserver.bin
	cp musserv/musserver.sh $(DLIB)/musserver
	cp sndserv/unixware7/sndserver $(DLIB)
	cp xdoom/unixware7/ps-xdoom $(DLIB)
	cp xdoom/unixware7/ps-sxdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	cp contrib/sxdoom.sh $(DBIN)/ps-sxdoom
	chmod 755 $(DLIB)/musserver.bin
	chmod 755 $(DLIB)/musserver
	chmod 755 $(DLIB)/sndserver
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DLIB)/ps-sxdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	chmod 755 $(DBIN)/ps-sxdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-uw2:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp musserv/unixware2/musserver $(DLIB)/musserver.bin
	cp musserv/musserver.sh $(DLIB)/musserver
	cp sndserv/unixware2/sndserver $(DLIB)
	cp xdoom/unixware2/ps-xdoom $(DLIB)
	cp xdoom/unixware2/ps-sxdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	cp contrib/sxdoom.sh $(DBIN)/ps-sxdoom
	chmod 755 $(DLIB)/musserver.bin
	chmod 755 $(DLIB)/musserver
	chmod 755 $(DLIB)/sndserver
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DLIB)/ps-sxdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	chmod 755 $(DBIN)/ps-sxdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-os5:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp musserv/openserver5/musserver $(DLIB)/musserver.bin
	cp musserv/musserver.sh $(DLIB)/musserver
	cp sndserv/openserver5/sndserver $(DLIB)
	cp xdoom/openserver5/ps-xdoom $(DLIB)
	cp xdoom/openserver5/ps-sxdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	cp contrib/sxdoom.sh $(DBIN)/ps-sxdoom
	chmod 755 $(DLIB)/musserver.bin
	chmod 755 $(DLIB)/musserver
	chmod 755 $(DLIB)/sndserver
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DLIB)/ps-sxdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	chmod 755 $(DBIN)/ps-sxdoom
	cp doc/xdoom.6 $(DMAN)/man.6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-linux-x86:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp musserv/linux/musserver $(DLIB)/musserver.bin
	cp musserv/musserver.sh $(DLIB)/musserver
	cp sndserv/linux/sndserver $(DLIB)
	cp xdoom/linux-x86/ps-xdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	chmod 755 $(DLIB)/musserver.bin
	chmod 755 $(DLIB)/musserver
	chmod 755 $(DLIB)/sndserver
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-linux-x86-svga:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp musserv/linux/musserver $(DLIB)/musserver.bin
	cp musserv/musserver.sh $(DLIB)/musserver
	cp sndserv/linux/sndserver $(DLIB)
	cp xdoom/linux-x86/ps-sxdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/sxdoom.sh $(DBIN)/ps-sxdoom
	chmod 755 $(DLIB)/musserver.bin
	chmod 755 $(DLIB)/musserver
	chmod 755 $(DLIB)/sndserver
	chmod 755 $(DLIB)/ps-sxdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-sxdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-linux-x86-3dfx:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp musserv/linux/musserver $(DLIB)/musserver.bin
	cp musserv/musserver.sh $(DLIB)/musserver
	cp sndserv/linux/sndserver $(DLIB)
	cp xdoom/linux-x86/ps-glxdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/glxdoom.sh $(DBIN)/ps-glxdoom
	chmod 755 $(DLIB)/musserver.bin
	chmod 755 $(DLIB)/musserver
	chmod 755 $(DLIB)/sndserver
	chmod 755 $(DLIB)/ps-glxdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-glxdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-linux-alpha:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp xdoom/linux-alpha/ps-xdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-freebsd: freebsd
	${INSTALL} -d $(DBIN)
	${INSTALL} -d $(DLIB)
	${INSTALL} -d $(LLIB)/acc/xdoom
	${INSTALL} -d $(DMAN)/man6
	${INSTALL} -C -s -m 755 musserv/freebsd/musserver $(DLIB)/musserver.bin
	${INSTALL} -C -s -m 755 musserv/musserver.sh $(DLIB)/musserver
	${INSTALL} -C -s -m 755 sndserv/freebsd/sndserver $(DLIB)/sndserver
	${INSTALL} -C -s -m 755 xdoom/freebsd/ps-xdoom $(DLIB)/ps-xdoom
	${INSTALL} -C -m 755 ../xdlaunch/xdlaunch $(DBIN)/xdlaunch
	${INSTALL} -C -m 755 contrib/xdoom.sh $(DBIN)/ps-xdoom
	${INSTALL} -C -m 644 doc/xdoom.6 $(DMAN)/man6/xdoom.6
	${INSTALL} -C -m 644 data/xdoom.wad $(DLIB)/xdoom.wad
	${INSTALL} -C -m 644 data/common.acs $(LLIB)/acc/xdoom
	${INSTALL} -C -m 644 data/specials.acs $(LLIB)/acc/xdoom
	${INSTALL} -C -m 644 data/defs.acs $(LLIB)/acc/xdoom

install-freebsd-svga: freebsd
	${INSTALL} -d $(DBIN)
	${INSTALL} -d $(DLIB)
	${INSTALL} -d $(LLIB)/acc/xdoom
	${INSTALL} -d $(DMAN)/man6
	${INSTALL} -C -s -m 755 musserv/freebsd/musserver $(DLIB)/musserver.bin
	${INSTALL} -C -s -m 755 musserv/musserver.sh $(DLIB)/musserver
	${INSTALL} -C -s -m 755 sndserv/freebsd/sndserver $(DLIB)/sndserver
	${INSTALL} -C -s -m 755 xdoom/freebsd/ps-sxdoom $(DLIB)/ps-sxdoom
	${INSTALL} -C -m 755 ../xdlaunch/xdlaunch $(DBIN)/xdlaunch
	${INSTALL} -C -m 755 contrib/sxdoom.sh $(DBIN)/ps-sxdoom
	${INSTALL} -C -m 644 doc/xdoom.6 $(DMAN)/man6/xdoom.6
	${INSTALL} -C -m 644 data/xdoom.wad $(DLIB)/xdoom.wad
	${INSTALL} -C -m 644 data/common.acs $(LLIB)/acc/xdoom
	${INSTALL} -C -m 644 data/specials.acs $(LLIB)/acc/xdoom
	${INSTALL} -C -m 644 data/defs.acs $(LLIB)/acc/xdoom

install-solsparc:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp xdoom/solarissparc/ps-xdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

install-aix:
	-mkdir -p $(DBIN)
	-mkdir -p $(DLIB)
	-mkdir -p $(LLIB)/acc/xdoom
	-mkdir -p $(DMAN)
	cp xdoom/aix/ps-xdoom $(DLIB)
	cp ../xdlaunch/xdlaunch $(DBIN)
	cp contrib/xdoom.sh $(DBIN)/ps-xdoom
	chmod 755 $(DLIB)/ps-xdoom
	chmod 755 $(DBIN)/xdlaunch
	chmod 755 $(DBIN)/ps-xdoom
	cp doc/xdoom.6 $(DMAN)/man6
	cp data/xdoom.wad $(DLIB)
	#cp doom1.wad $(DLIB)
	cp data/common.acs $(LLIB)/acc/xdoom
	cp data/specials.acs $(LLIB)/acc/xdoom
	cp data/defs.acs $(LLIB)/acc/xdoom

clean:
	@echo "You must specify a target for one of the following"
	@echo "supported OS's:"
	@echo
	@echo "FreeBSD:                 make clean-freebsd"
	@echo "Linux x86:               make clean-linux-x86"
	@echo "Linux Dec Alpha:         make clean-linux-alpha"
	@echo "OpenServer 5:            make clean-os5"
	@echo "Unixware 2:              make clean-uw2"
	@echo "Unixware 7:              make clean-uw7"
	@echo "Solaris SPARC:           make clean-solsparc"
	@echo "IBM AIX:                 make clean-aix"

clean-uw7:
	(cd sndserv; ${MAKE} -f Makefile.scouw7 clean)
	(cd musserv; ${MAKE} -f Makefile.scouw7 clean)
	(cd xdoom; ${MAKE} -f Makefile.scouw7 clean)

clean-uw2:
	(cd sndserv; ${MAKE} -f Makefile.scouw2 clean)
	(cd musserv; ${MAKE} -f Makefile.scouw2 clean)
	(cd xdoom; ${MAKE} -f Makefile.scouw2 clean)

clean-os5:
	(cd sndserv; ${MAKE} -f Makefile.scoos5 clean)
	(cd musserv; ${MAKE} -f Makefile.scoos5 clean)
	(cd xdoom; ${MAKE} -f Makefile.scoos5 clean)

clean-linux-x86:
	(cd sndserv; ${MAKE} -f Makefile.linux clean)
	(cd musserv; ${MAKE} -f Makefile.linux clean)
	(cd xdoom; ${MAKE} -f Makefile.linux-x86 clean)

clean-linux-alpha:
	(cd xdoom; ${MAKE} -f Makefile.linux-alpha clean)

clean-freebsd:
	(cd sndserv; ${MAKE} -f Makefile.freebsd clean)
	(cd musserv; ${MAKE} -f Makefile.freebsd clean)
	(cd xdoom; ${MAKE} -f Makefile.freebsd clean)

clean-solsparc:
	(cd xdoom; ${MAKE} -f Makefile.solsparc clean)

clean-aix:
	(cd xdoom; ${MAKE} -f Makefile.aix clean)
