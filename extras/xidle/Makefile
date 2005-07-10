# $OpenBSD: Makefile,v 1.4 2005/07/10 21:12:10 matthieu Exp $

PROG=	xidle
CFLAGS+= -Wall
LDADD+=	-L$(X11LIBDIR) -lXss -lXext -lX11

.include <bsd.prog.mk>
