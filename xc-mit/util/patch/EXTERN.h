/* $Header: /tmp/OpenBSD-XF4-repo/xc-mit/util/patch/Attic/EXTERN.h,v 1.1.1.1 2001/10/24 13:13:01 todd Exp $
 *
 * $Log: EXTERN.h,v $
 * Revision 1.1.1.1  2001/10/24 13:13:01  todd
 * This started out as X11R5, with mods from Jason Downs, and code from
 * 4.4BSD-lite.  This 'xc-mit' tree's purpose is to build XhpBSD for hp300,
 * anything it requires, and NOTHING else.
 *
 * Revision 2.0  86/09/17  15:35:37  lwall
 * Baseline for netwide release.
 * 
 */

#undef EXT
#define EXT extern

#undef INIT
#define INIT(x)

#undef DOINIT