#	$NetBSD: Makefile,v 1.3 1997/12/09 11:58:28 mrg Exp $
#	$OpenBSD: Makefile,v 1.54 2006/08/12 16:48:52 miod Exp $
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
.else
XMACH= ${MACHINE}
.endif
BINDIST=programs/Xserver/hw/xfree86/etc/bindist
HOSTDEF=xc/${BINDIST}/OpenBSD-${XMACH}/host.def
CONFHOSTDEF=xc/config/cf/host.def
HOSTDEFo=xc-old/${BINDIST}/OpenBSD-${XMACH}/host.def
CONFHOSTDEFo=xc-old/config/cf/host.def
XCONFIG=xc/${BINDIST}/OpenBSD-${XMACH}/xorg.conf

.if ${MACHINE} == i386
NEED_XC_OLD?=yes
REQUIRED_PACKAGES=	tcl-8.\* tk-8.\*
.else
NEED_XC_OLD?=no
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

FONTDIRS?= . 100dpi 75dpi CID TTF Type1 cyrillic local misc \
		encodings encodings/large util
.for _font in ${FONTDIRS}
FONTPATH+= ${DESTDIR}/usr/X11R6/lib/X11/fonts/${_font}
.endfor


all:	compile

compile: check-packages
	${RM} -f ${CONFHOSTDEF}
	${INSTALL} ${HOSTDEF} ${CONFHOSTDEF}
.ifdef NOFONTS
	@echo "#define BuildFonts NO" >> ${CONFHOSTDEF}
.endif
	cd xc && exec ${MAKE} World WORLDOPTS=
.if (${NEED_XC_OLD:L} == "yes")
	${INSTALL} ${HOSTDEFo} ${CONFHOSTDEFo}
	cd xc-old && exec ${MAKE} World WORLDOPTS=
.endif
	cd extras && ${MAKE} obj && ${MAKE} depend && exec ${MAKE}

check-packages:
	@for p in ${REQUIRED_PACKAGES}; do \
		pkg_info -q -e "$$p" || \
		(echo "Missing package $$p" ; exit 2); \
	done

build: 
	${MAKE} compile 
	${SUDO} ${MAKE} install 
	${SUDO} ${MAKE} fix-appd

release-rel:
	mkdir -p rel
	${MAKE} RELEASEDIR=`pwd`/rel DESTDIR=`pwd`/dest release

release: release-clean release-mkdir release-install dist

release-clean:
.if ! ( defined(DESTDIR) && defined(RELEASEDIR) )
	@echo You must set DESTDIR and RELEASEDIR for a release.; exit 255
.endif
	${RM} -rf ${DESTDIR}/usr/X11R6/* ${DESTDIR}/usr/X11R6/.[a-zA-Z0-9]*
	${RM} -rf ${DESTDIR}/var/X11/*
	${RM} -rf ${DESTDIR}/etc/X11/*
	${RM} -rf ${DESTDIR}/etc/fonts/*
	@if [ -d ${DESTDIR}/usr/X11R6 ] && [ "`cd ${DESTDIR}/usr/X11R6;ls`" ]; then \
		echo "Files found in ${DESTDIR}/usr/X11R6:"; \
		(cd ${DESTDIR}/usr/X11R6;/bin/pwd;ls -a); \
		echo "Cleanup before proceeding."; \
		exit 255; \
	fi

release-mkdir:
	@${INSTALL} -d -o ${DIROWN} -g ${DIRGRP} -m ${DIRMODE} \
		${DESTDIR}/usr/X11R6 ${DESTDIR}/etc/X11 \
		${DESTDIR}/usr/local/lib/X11 \
		${DESTDIR}/usr/X11R6/man \
		${DESTDIR}/usr/X11R6/man/cat1
	@${MAKE} perms

release-install:
	@${MAKE} install
.if ${MACHINE} == alpha || ${MACHINE} == hp300 || ${MACHINE} == mac68k || \
    ${MACHINE} == macppc || ${MACHINE} == sparc || ${MACHINE} == vax || \
    ${MACHINE} == zaurus
	@if [ -f $(DESTDIR)/etc/X11/xorg.conf ]; then \
	 echo "Not overwriting existing" $(DESTDIR)/etc/X11/xorg.conf; \
	else set -x; \
	 ${INSTALL} ${INSTALL_COPY} -o root -g wheel -m 644 \
		${XCONFIG} ${DESTDIR}/etc/X11 ; \
	fi
.endif
	@${MAKE} fix-appd

PERMDIRS = / /usr /usr/X11R6 /etc /etc/X11 /usr/local /usr/local/lib
PERMDIRS+= /usr/local/lib/X11
perms:
.for _dir in ${PERMDIRS}
	${CHOWN} ${DIROWN}:${DIRGRP} ${DESTDIR}${_dir}/.
.endfor
	find ${DESTDIR}/usr/X11R6/. ${DESTDIR}/etc/. ${DESTDIR}/usr/local/. \
		-type d \
		\( \! -user ${DIROWN} -o \! -group ${DIRGRP} \) \
		-ls
	find ${DESTDIR}/usr/X11R6/. ${DESTDIR}/etc/. \
		-type f \
		\( \! -user ${BINOWN} -o \! -group wheel \) \
		-ls

dist-rel:
	${MAKE} RELEASEDIR=`pwd`/rel DESTDIR=`pwd`/dest dist 2>&1 | tee distlog

dist:
	${MAKE} perms
	cd distrib/sets && \
		env MACHINE=${MACHINE} ksh ./maketars ${OSrev} ${OSREV} && \
		(env MACHINE=${MACHINE} ksh ./checkflist ${OSREV} || true)

install: install-xc install-xc-old install-extra install-distrib
.ifndef NOFONTS
	LD_LIBRARY_PATH=${DESTDIR}/usr/X11R6/lib ${DESTDIR}/usr/X11R6/bin/fc-cache -v ${FONTPATH}
.endif
	/usr/libexec/makewhatis ${DESTDIR}/usr/X11R6/man

install-xc:
	cd xc; ${MAKE} install && ${MAKE} install.man
	cd xc/programs/rstart; ${MAKE} install && ${MAKE} install.man

install-extra:
	cd extras && exec ${MAKE} install

install-xc-old:
.if (${NEED_XC_OLD:L} == "yes")
	cd xc-old; ${MAKE} install && ${MAKE} install.man
.endif

install-distrib:
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
	@echo sh ${.CURDIR}/build-release
	@exec sh ${.CURDIR}/build-release

.PHONY: all build release dist install install-xc install-xc-old \
    install-distrib clean distclean fix-appd b-r \
    release-clean release-mkdir release-install install-extra \
    check-packages

.include <bsd.own.mk>
