# $OpenBSD: Makefile,v 1.1 2001/04/04 16:07:58 todd Exp $

all: sort

sort:
	@for f in */m*; do echo "===> sorting $$f"; \
		sort $$f | uniq > $$f.tmp; \
		diff -u $$f $$f.tmp || true; \
		mv $$f.tmp $$f; \
	done;
		

