/* $Header: /tmp/OpenBSD-XF4-repo/xc-mit/util/patch/Attic/version.c,v 1.1.1.1 2001/10/24 13:13:01 todd Exp $
 *
 * $Log: version.c,v $
 * Revision 1.1.1.1  2001/10/24 13:13:01  todd
 * This started out as X11R5, with mods from Jason Downs, and code from
 * 4.4BSD-lite.  This 'xc-mit' tree's purpose is to build XhpBSD for hp300,
 * anything it requires, and NOTHING else.
 *
 * Revision 2.0  86/09/17  15:40:11  lwall
 * Baseline for netwide release.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "INTERN.h"
#include "patchlevel.h"
#include "version.h"

/* Print out the version number and die. */

void
version()
{
    extern char rcsid[];

#ifdef lint
    rcsid[0] = rcsid[0];
#else
    fatal3("%s\nPatch level: %d\n", rcsid, PATCHLEVEL);
#endif
}
