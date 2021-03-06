/* $XFree86: xc/programs/Xserver/hw/xfree86/xf4bpp/ppcDepth.c,v 1.2 1998/07/25 16:59:33 dawes Exp $ */
/*
 * Copyright IBM Corporation 1987,1988,1989
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/* $XConsortium: ppcDepth.c /main/3 1996/02/21 17:57:31 kaleb $ */

/* Check to see if the alleged depth is acceptable for the Screen  
 *
 * T. Paquin 9/87
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "scrnintstr.h"

Bool
xf4bppDepthOK(pDraw,depth)
register DrawablePtr pDraw;
register int depth;
{
register ScreenPtr pScreen= pDraw->pScreen;
register int i = pScreen->numDepths;

    if ( ( pDraw->type == DRAWABLE_PIXMAP ) && ( depth == 1 ) )
	return TRUE ;

    while ( i-- )
	if ( depth == pScreen->allowedDepths[i].depth )
	    return TRUE ;

    return FALSE ;
}
