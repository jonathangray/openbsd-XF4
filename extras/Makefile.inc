# $OpenBSD: Makefile.inc,v 1.2 2002/01/22 20:10:46 matthieu Exp $

XEXPORTS=${.CURDIR}/../../xc/exports

X11BASE=/usr/X11R6
BINDIR=${X11BASE}/bin
CFLAGS+=-I${XEXPORTS}/include -I${X11BASE}/include
LDADD+=-L${XEXPORTS}/lib
MANDIR=${X11BASE}/man/cat
X11LIBDIR=${X11BASE}/lib
