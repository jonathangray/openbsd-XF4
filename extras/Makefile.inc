# $OpenBSD: Makefile.inc,v 1.3 2004/06/07 20:37:21 matthieu Exp $

XEXPORTS=${.CURDIR}/../../xc/exports

X11BASE=/usr/X11R6
BINDIR=${X11BASE}/bin
CFLAGS+=-I${XEXPORTS}/include -I${X11BASE}/include
LDADD:=-L${XEXPORTS}/lib ${LDADD}
MANDIR=${X11BASE}/man/cat
X11LIBDIR=${X11BASE}/lib
