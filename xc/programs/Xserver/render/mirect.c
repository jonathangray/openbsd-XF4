/*
 * $XFree86: xc/programs/Xserver/render/mirect.c,v 1.3 2000/12/08 07:52:05 keithp Exp $
 *
 * Copyright � 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "mi.h"
#include "picturestr.h"
#include "mipict.h"

static void
miColorRects (PicturePtr    pDst,
	      PicturePtr    pClipPict,
	      xRenderColor  *color,
	      int	    nRect,
	      xRectangle    *rects,
	      int	    xoff,
	      int	    yoff)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    CARD32		pixel;
    GCPtr		pGC;
    CARD32		tmpval[5];
    RegionPtr		pClip;
    unsigned long	mask;

    miRenderColorToPixel (pDst->pFormat, color, &pixel);

    pGC = GetScratchGC (pDst->pDrawable->depth, pScreen);
    if (!pGC)
	return;
    tmpval[0] = GXcopy;
    tmpval[1] = pixel;
    tmpval[2] = pDst->subWindowMode;
    mask = GCFunction | GCForeground | GCSubwindowMode;
    if (pClipPict->clientClipType == CT_REGION)
    {
	tmpval[3] = pDst->clipOrigin.x - xoff;
	tmpval[4] = pDst->clipOrigin.y - yoff;
	mask |= GCClipXOrigin|GCClipYOrigin;
	
	pClip = REGION_CREATE (pScreen, NULL, 1);
	REGION_COPY (pScreen, pClip,
		     (RegionPtr) pClipPict->clientClip);
	(*pGC->funcs->ChangeClip) (pGC, CT_REGION, pClip, 0);
    }

    ChangeGC (pGC, mask, tmpval);
    ValidateGC (pDst->pDrawable, pGC);
    if (xoff || yoff)
    {
	int	i;
	for (i = 0; i < nRect; i++)
	{
	    rects[i].x -= xoff;
	    rects[i].y -= yoff;
	}
    }
    (*pGC->ops->PolyFillRect) (pDst->pDrawable, pGC, nRect, rects);
    if (xoff || yoff)
    {
	int	i;
	for (i = 0; i < nRect; i++)
	{
	    rects[i].x += xoff;
	    rects[i].y += yoff;
	}
    }
    FreeScratchGC (pGC);
}

void
miCompositeRects (CARD8		op,
		  PicturePtr	pDst,
		  xRenderColor  *color,
		  int		nRect,
		  xRectangle    *rects)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    
    if (color->alpha == 0xffff)
    {
	if (op == PictOpOver)
	    op = PictOpSrc;
    }
    if (op == PictOpClear)
	color->red = color->green = color->blue = color->alpha = 0;
    
    if (op == PictOpSrc || op == PictOpClear)
    {
	miColorRects (pDst, pDst, color, nRect, rects, 0, 0);
	if (pDst->alphaMap)
	    miColorRects (pDst->alphaMap, pDst,
			  color, nRect, rects,
			  pDst->alphaOrigin.x,
			  pDst->alphaOrigin.y);
    }
    else
    {
	PictFormatPtr	rgbaFormat;
	PixmapPtr	pPixmap;
	PicturePtr	pSrc;
	xRectangle	one;
	int		error;
	Pixel		pixel;
	GCPtr		pGC;
	CARD32		tmpval[2];

	rgbaFormat = PictureMatchFormat (pScreen, 32, PICT_a8r8g8b8);
	if (!rgbaFormat)
	    goto bail1;
	
	pPixmap = (*pScreen->CreatePixmap) (pScreen, 1, 1,
					    rgbaFormat->depth);
	if (!pPixmap)
	    goto bail2;
	
	miRenderColorToPixel (rgbaFormat, color, &pixel);

	pGC = GetScratchGC (rgbaFormat->depth, pScreen);
	if (!pGC)
	    goto bail3;
	tmpval[0] = GXcopy;
	tmpval[1] = pixel;

	ChangeGC (pGC, GCFunction | GCForeground, tmpval);
	ValidateGC (&pPixmap->drawable, pGC);
	one.x = 0;
	one.y = 0;
	one.width = 1;
	one.height = 1;
	(*pGC->ops->PolyFillRect) (&pPixmap->drawable, pGC, 1, &one);
	
	tmpval[0] = xTrue;
	pSrc = CreatePicture (0, &pPixmap->drawable, rgbaFormat,
			      CPRepeat, tmpval, 0, &error);
			      
	if (!pSrc)
	    goto bail4;

	while (nRect--)
	{
	    CompositePicture (op, pSrc, 0, pDst, 0, 0, 0, 0, 
			      rects->x,
			      rects->y,
			      rects->width,
			      rects->height);
	    rects++;
	}

	FreePicture ((pointer) pSrc, 0);
bail4:
	FreeScratchGC (pGC);
bail3:
	(*pScreen->DestroyPixmap) (pPixmap);
bail2:
bail1:
	;
    }
}
