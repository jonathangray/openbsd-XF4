/* $Header: /tmp/OpenBSD-XF4-repo/xc-mit/util/patch/Attic/INTERN.h,v 1.1.1.1 2001/10/24 13:13:01 todd Exp $
 *
 * $Log: INTERN.h,v $
 * Revision 1.1.1.1  2001/10/24 13:13:01  todd
 * This started out as X11R5, with mods from Jason Downs, and code from
 * 4.4BSD-lite.  This 'xc-mit' tree's purpose is to build XhpBSD for hp300,
 * anything it requires, and NOTHING else.
 *
 * Revision 2.0  86/09/17  15:35:58  lwall
 * Baseline for netwide release.
 * 
 */

#undef EXT
#define EXT

#undef INIT
#define INIT(x) = x

#define DOINIT
