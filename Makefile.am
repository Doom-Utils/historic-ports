#
# Top level automake Makefile.am for LsdlDoom
#
# $Id: Makefile.am,v 1.5 2000/03/19 16:28:37 cph Exp $
#

docdir=$(prefix)/share/doc/@PACKAGE@
SUBDIRS = doc data src
EXTRA_DIST = lsdldoom.spec.in
doc_DATA = README COPYING AUTHORS

rpm:	lsdldoom.spec dist
	mv @PACKAGE@-@VERSION@.tar.gz /usr/src/redhat/SOURCES
	rpm -bb lsdldoom.spec --target=i586-pc-linux-gnu

#
# $Log: Makefile.am,v $
# Revision 1.5  2000/03/19 16:28:37  cph
# Fix documentation directory
#
# Revision 1.4  2000/03/16 20:14:08  cph
# Add docs to make install, add RPM building
#
# Revision 1.3  1999/10/12 17:48:37  cphipps
# Remove CREDITS
#
# Revision 1.2  1999/10/01 20:38:00  cphipps
# Added ChangeLog to distributed files
#
# Revision 1.1  1999/09/10 20:08:16  cphipps
# Initial revision
#
#
