# 
#  $Id: Makefile.am,v 1.3 2005/12/31 13:08:57 matthieu Exp $
# 
#  Copyright © 2003 Keith Packard
# 
#  Permission to use, copy, modify, distribute, and sell this software and its
#  documentation for any purpose is hereby granted without fee, provided that
#  the above copyright notice appear in all copies and that both that
#  copyright notice and this permission notice appear in supporting
#  documentation, and that the name of Keith Packard not be used in
#  advertising or publicity pertaining to distribution of the software without
#  specific, written prior permission.  Keith Packard makes no
#  representations about the suitability of this software for any purpose.  It
#  is provided "as is" without express or implied warranty.
# 
#  KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
#  DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.

DOC2MAN = docbook2man

FC_CACHE_SRC=${top_srcdir}/fc-cache

SGML = ${FC_CACHE_SRC}/fc-cache.sgml

INCLUDES=-I${top_srcdir} $(FREETYPE_CFLAGS)

bin_PROGRAMS=fc-cache

EXTRA_DIST=$(SGML)

fc_cache_LDADD = ${top_builddir}/src/libfontconfig.la

if USEDOCBOOK

man_MANS=fc-cache.1

${man_MANS}: ${SGML}	
	$(RM) $@
	$(DOC2MAN) ${SGML}
	$(RM) manpage.refs manpage.links

all-local: $(man_MANS)

clean-local:
	$(RM) $(man_MANS)

else
all-local:
clean-local:
endif
