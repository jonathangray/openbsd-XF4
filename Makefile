#	$NetBSD: Makefile,v 1.3 1997/12/09 11:58:28 mrg Exp $
#	$OpenBSD: Makefile,v 1.16 2001/10/24 13:42:25 todd Exp $
#
# The purpose of this file is to build and install X11,
# and create release tarfiles.
#
# To build a release, you can take two paths:
#   1. # mkdir -p dest rel && make b-r
#       .. you will see sparse output, this has been designed for low remote
#	   network bandwidth consumption
#   2. # env DESTDIR=/some/dest/dir RELEASEDIR=/some/release/dir
#      # mkdir -p ${DESTDIR} ${RELEASEDIR} && make build && make release
#
#  Option 2 is provided for similar functionality to /usr/src/Makefile and
#  /usr/src/etc/Makefile.  Option 1 is how the official X releases are built.
#  It uses the 'build' and 'release' targets, so there is no inconsistency
#  between the two.
#

.if ${MACHINE} == i386
XMACH= ix86
.elif ${MACHINE} == hp700
XMACH= hppa
.else
XMACH= ${MACHINE}
.endif
BINDIST=programs/Xserver/hw/xfree86/etc/bindist
HOSTDEF=xc/${BINDIST}/OpenBSD-${XMACH}/host.def
CONFHOSTDEF=xc/config/cf/host.def
HOSTDEFo=xc-old/${BINDIST}/OpenBSD-${XMACH}/host.def
CONFHOSTDEFo=xc-old/config/cf/host.def
XCONFIG=xc/${BINDIST}/OpenBSD-${XMACH}/XF86Config

.if ${MACHINE} == i386 || ${MACHINE} == amiga || ${MACHINE} == alpha \
	 || ${MACHINE} == mac68k
NEED_XC_OLD?=yes
.else
NEED_XC_OLD?=no
.endif

.if ${MACHINE} == hp300
NEED_XC_MIT?=yes
XHP?=${.CURDIR}/xc-mit/server/XhpBSD
.else
NEED_XC_MIT?=no
.endif

LN?= /bin/ln
CHOWN?=/usr/sbin/chown
CHMOD?=/bin/chmod
ECHO?=/bin/echo
RM?= /bin/rm
DIROWN?=root
DIRGRP?=wheel

MACHINE?=`uname -m`

LOCALAPPD=/usr/local/lib/X11/app-defaults
LOCALAPPX=/usr/local/lib/X11
REALAPPD=/etc/X11/app-defaults


all:	compile

compile:
.if (${NEED_XC_MIT:L} == "yes")
	cd xc-mit ; ${MAKE} -f Makefile.ini World BOOTSTRAPCFLAGS="-Dhp300 -Dhp9000"
.endif
	${RM} -f ${CONFHOSTDEF}
	${INSTALL} ${HOSTDEF} ${CONFHOSTDEF}
	cd xc ; ${MAKE} World WORLDOPTS=
.if (${NEED_XC_OLD:L} == "yes")
	${INSTALL} ${HOSTDEFo} ${CONFHOSTDEFo}
	cd xc-old ; ${MAKE} World WORLDOPTS=
.endif

build: compile install fix-appd

release-rel:
	${MAKE} RELEASEDIR=`pwd`/rel DESTDIR=`pwd`/dest release

release: release-clean release-mkdir release-install fix-appd dist

release-clean:
.if ! ( defined(DESTDIR) && defined(RELEASEDIR) )
	@echo You must set DESTDIR and RELEASEDIR for a release.; exit 255
.endif
	${RM} -rf ${DESTDIR}/usr/X11R6/* ${DESTDIR}/usr/X11R6/.[a-zA-Z0-9]*
	${RM} -rf ${DESTDIR}/var/X11/*
	${RM} -rf ${DESTDIR}/etc/X11/*
	@if [ "`cd ${DESTDIR}/usr/X11R6;ls`" ]; then \
		echo "Files found in ${DESTDIR}/usr/X11R6:"; \
		(cd ${DESTDIR}/usr/X11R6;/bin/pwd;ls -a); \
		echo "Cleanup before proceeding."; \
		exit 255; \
	fi

release-mkdir:
	@${INSTALL} -d -o ${DIROWN} -g ${DIRGRP} -m ${DIRMODE} \
		${DESTDIR}/usr/X11R6 ${DESTDIR}/etc/X11 \
		${DESTDIR}/usr/local/lib/X11
	@${MAKE} perms

release-install:
	@${MAKE} install
.if defined(MACHINE) && ${MACHINE} == hp300
	@${INSTALL} ${INSTALL_STRIP} -m 755 -o ${BINOWN} -g ${BINGRP} \
		${XHP} ${DESTDIR}/usr/X11R6/bin
	@${LN} -s XhpBSD ${DESTDIR}/usr/X11R6/bin/X
	@${ECHO} /dev/grf0 > ${DESTDIR}/usr/X11R6/lib/X11/X0screens
.endif
.if ${MACHINE} == macppc
	@if [ -f $(DESTDIR)/etc/X11/XF86Config ]; then \
	 echo "Not overwriting existing" $(DESTDIR)/etc/X11/XF86Config; \
	else set -x; \
	 ${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 444 \
		${XCONFIG} ${DESTDIR}/etc/X11 ; \
	fi
.endif
	@${MAKE} fix-appd

perms:
	@${CHOWN} ${BINOWN}.${BINGRP} ${DESTDIR}/.
	@${CHOWN} ${BINOWN}.${BINGRP} ${DESTDIR}/usr/.
	@${CHOWN} ${BINOWN}.${BINGRP} ${DESTDIR}/usr/X11R6/.
	@${CHOWN} ${BINOWN}.${BINGRP} ${DESTDIR}/etc/X11/.
	@find ${DESTDIR}/usr/X11R6/. \
		${DESTDIR}/etc/X11/. \! -user ${BINOWN} -ls

dist-rel:
	${MAKE} RELEASEDIR=`pwd`/rel DESTDIR=`pwd`/dest dist 2>&1 | tee distlog

dist:
	${MAKE} perms
	cd distrib/sets && env MACHINE=${MACHINE} csh ./maketars ${OSrev} && \
		env MACHINE=${MACHINE} csh ./checkflist

install: install-xc install-xc-old install-distrib
	/usr/libexec/makewhatis ${DESTDIR}/usr/X11R6/man

install-xc:
	cd xc; ${MAKE} install && ${MAKE} install.man
.if (${MACHINE} == "hp300")
	echo /dev/grf0 > ${DESTDIR}/usr/X11R6/lib/X11/X0screens
	chown root.wheel ${DESTDIR}/usr/X11R6/lib/X11/X0screens
.endif
	cd xc/programs/rstart; ${MAKE} install && ${MAKE} install.man

install-xc-old:
.if (${NEED_XC_OLD:L} == "yes")
	cd xc-old; ${MAKE} install && ${MAKE} install.man
.endif

install-distrib:
	${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 444 \
		distrib/ports.cf ${DESTDIR}/usr/X11R6/lib/X11/config
	cd distrib/notes; ${MAKE} install

fix-appd:
	# Make sure /usr/local/lib/X11/app-defaults is a link
	if [ ! -L $(DESTDIR)${LOCALAPPD} ]; then \
	    if [ -d $(DESTDIR)${LOCALAPPD} ]; then \
		mv $(DESTDIR)${LOCALAPPD}/* $(DESTDIR)${REALAPPD}; \
		rmdir $(DESTDIR)${LOCALAPPD}; \
	    fi; \
	    mkdir -p ${DESTDIR}${LOCALAPPX}; \
	    ln -s ${REALAPPD} ${DESTDIR}${LOCALAPPD}; \
	fi

clean:
	cd xc; ${MAKE} clean
.if (${NEED_XC_OLD:L} == "yes")
	cd xc-old; ${MAKE} clean
.endif

distclean:
	${MAKE} clean
	rm -f xc/xmakefile
.if (${NEED_XC_OLD:L} == "yes")
	rm -f xc-old/xmakefile
.endif

b-r:
	@echo ${.CURDIR}/build-release
	@exec ${.CURDIR}/build-release

.PHONY: all build release dist install install-xc install-xc-old \
    install-distrib clean distclean fix-appd b-r \
    release-clean release-mkdir release-install

.include <bsd.own.mk>
