/* $Header: /tmp/OpenBSD-XF4-repo/xc-mit/util/patch/Attic/inp.h,v 1.1.1.1 2001/10/24 13:13:01 todd Exp $
 *
 * $Log: inp.h,v $
 * Revision 1.1.1.1  2001/10/24 13:13:01  todd
 * This started out as X11R5, with mods from Jason Downs, and code from
 * 4.4BSD-lite.  This 'xc-mit' tree's purpose is to build XhpBSD for hp300,
 * anything it requires, and NOTHING else.
 *
 * Revision 2.0  86/09/17  15:37:25  lwall
 * Baseline for netwide release.
 * 
 */

EXT LINENUM input_lines INIT(0);	/* how long is input file in lines */
EXT LINENUM last_frozen_line INIT(0);	/* how many input lines have been */
					/* irretractibly output */

bool rev_in_string();
void scan_input();
bool plan_a();			/* returns false if insufficient memory */
void plan_b();
char *ifetch();

