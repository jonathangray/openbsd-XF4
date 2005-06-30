# $OpenBSD: Makefile,v 1.2 2005/06/30 06:09:56 fgsch Exp $

PROG=	xidle
CFLAGS+=-DPATH_XLOCK=\"$(X11BASE)/bin/xlock\" -Wall
LDADD+=	-L$(X11LIBDIR) -lX11 -lXss

.include <bsd.prog.mk>
