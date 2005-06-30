# $OpenBSD: Makefile,v 1.1 2005/06/30 01:48:45 fgsch Exp $

PROG=	xidle
CFLAGS+=-DPATH_XLOCK=\"$(X11BASEDIR)/bin/xlock\" -Wall
LDADD+=	-L$(X11LIBDIR) -lX11 -lXss

.include <bsd.prog.mk>
