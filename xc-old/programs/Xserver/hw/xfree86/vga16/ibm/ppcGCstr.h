/* $XFree86: xc/programs/Xserver/hw/xfree86/vga16/ibm/ppcGCstr.h,v 3.0 1996/11/18 13:13:36 dawes Exp $ */
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
/***********************************************************
		Copyright IBM Corporation 1988

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: ppcGCstr.h /main/3 1996/02/21 17:57:42 kaleb $ */

typedef struct {
    unsigned long	planemask ;
    unsigned long	fgPixel ;
    unsigned long	bgPixel ;
    int			alu ;
    int			fillStyle ;
    } ppcReducedRrop ;

/* ************************************************************************ */

/* private field of GC */
typedef struct {
/* The Next eleven (11) fields MUST CORRESPOND to
 * the fields of a "mfbPrivGC" struct
 * ----- BEGINNING OF "DO-NOT-CHANGE" REGION -----
 */
    unsigned char	rop ;		/* reduction of rasterop to 1 of 3 */
    unsigned char	ropOpStip ;	/* rop for opaque stipple */
    unsigned char	ropFillArea ;	/*  == alu, rop, or ropOpStip */
    unsigned	fExpose:1 ;		/* callexposure handling ? */
    unsigned	freeCompClip:1 ;
    PixmapPtr	pRotatedPixmap ;	/* tile/stipple  rotated to align */
    RegionPtr	pCompositeClip ;		/* free this based on freeCompClip
					   flag rather than NULLness */
    void 	(* FillArea)() ;		/* fills regions; look at the code */
/* ----- END OF "DO-NOT-CHANGE" REGION ----- */
    ppcReducedRrop	colorRrop ;
    short lastDrawableType ;	/* was last drawable a window or a pixmap? */
    short lastDrawableDepth ;	/* was last drawable 1 or 8 planes? */
    void (* cachedIGBlt)();	/* cached image glyph blit routine */
    void (* cachedPGBlt)();	/* cached poly glyph blit routine */
    pointer devPriv ;		/* Private area for device specific stuff */
    } ppcPrivGC ;
typedef ppcPrivGC *ppcPrivGCPtr ;
