# $OpenBSD: Makefile.inc,v 1.1.1.1 2001/11/09 15:50:08 espie Exp $

X11BASE=/usr/X11R6
BINDIR=${X11BASE}/bin
CFLAGS+=-I${X11BASE}/include
MANDIR=${X11BASE}/man/cat
X11LIBDIR=${X11BASE}/lib
