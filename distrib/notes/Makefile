# $OpenBSD: Makefile,v 1.2 2002/10/07 21:52:19 todd Exp $

FILE= README.${MACHINE}

${FILE}.generated: ${FILE} m4.common
	m4 m4.common ${FILE} > ${FILE}.generated

install: ${FILE}.generated
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 444 ${FILE}.generated \
	${DESTDIR}/usr/X11R6/README

clean:
	rm -f *.generated
