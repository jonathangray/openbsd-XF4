/*
 * Copyright � 2004 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL RED HAT
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  S�ren Sandmann (sandmann@redhat.com)
 * 
 * Based on work by Owen Taylor
 */
#ifdef USE_GCC34_MMX
Bool fbHaveMMX(void);
#else
#define fbHaveMMX FALSE
#endif

#ifdef USE_GCC34_MMX

void fbCompositeSolidMask_nx8888x0565Cmmx (CARD8      op,
					   PicturePtr pSrc,
					   PicturePtr pMask,
					   PicturePtr pDst,
					   INT16      xSrc,
					   INT16      ySrc,
					   INT16      xMask,
					   INT16      yMask,
					   INT16      xDst,
					   INT16      yDst,
					   CARD16     width,
					   CARD16     height);
void fbCompositeSrcAdd_8888x8888mmx (CARD8	op,
				     PicturePtr	pSrc,
				     PicturePtr	pMask,
				     PicturePtr	pDst,
				     INT16	xSrc,
				     INT16      ySrc,
				     INT16      xMask,
				     INT16      yMask,
				     INT16      xDst,
				     INT16      yDst,
				     CARD16     width,
				     CARD16     height);
void fbCompositeSolidMask_nx8888x8888Cmmx (CARD8	op,
					   PicturePtr	pSrc,
					   PicturePtr	pMask,
					   PicturePtr	pDst,
					   INT16	xSrc,
					   INT16	ySrc,
					   INT16	xMask,
					   INT16	yMask,
					   INT16	xDst,
					   INT16	yDst,
					   CARD16	width,
					   CARD16	height);
void fbCompositeSolidMask_nx8x8888mmx (CARD8      op,
				       PicturePtr pSrc,
				       PicturePtr pMask,
				       PicturePtr pDst,
				       INT16      xSrc,
				       INT16      ySrc,
				       INT16      xMask,
				       INT16      yMask,
				       INT16      xDst,
				       INT16      yDst,
				       CARD16     width,
				       CARD16     height);
void fbCompositeSrcAdd_8000x8000mmx (CARD8	op,
				     PicturePtr pSrc,
				     PicturePtr pMask,
				     PicturePtr pDst,
				     INT16      xSrc,
				     INT16      ySrc,
				     INT16      xMask,
				     INT16      yMask,
				     INT16      xDst,
				     INT16      yDst,
				     CARD16     width,
				     CARD16     height);
void fbCompositeSrc_8888RevNPx8888mmx (CARD8      op,
				       PicturePtr pSrc,
				       PicturePtr pMask,
				       PicturePtr pDst,
				       INT16      xSrc,
				       INT16      ySrc,
				       INT16      xMask,
				       INT16      yMask,
				       INT16      xDst,
				       INT16      yDst,
				       CARD16     width,
				       CARD16     height);
void fbCompositeSrc_8888RevNPx0565mmx (CARD8      op,
				       PicturePtr pSrc,
				       PicturePtr pMask,
				       PicturePtr pDst,
				       INT16      xSrc,
				       INT16      ySrc,
				       INT16      xMask,
				       INT16      yMask,
				       INT16      xDst,
				       INT16      yDst,
				       CARD16     width,
				       CARD16     height);
void fbCompositeSolid_nx8888mmx (CARD8		op,
				 PicturePtr	pSrc,
				 PicturePtr	pMask,
				 PicturePtr	pDst,
				 INT16		xSrc,
				 INT16		ySrc,
				 INT16		xMask,
				 INT16		yMask,
				 INT16		xDst,
				 INT16		yDst,
				 CARD16		width,
				 CARD16		height);
void fbCompositeSolid_nx0565mmx (CARD8		op,
				 PicturePtr	pSrc,
				 PicturePtr	pMask,
				 PicturePtr	pDst,
				 INT16		xSrc,
				 INT16		ySrc,
				 INT16		xMask,
				 INT16		yMask,
				 INT16		xDst,
				 INT16		yDst,
				 CARD16		width,
				 CARD16		height);
void fbCompositeSolidMask_nx8x0565mmx (CARD8      op,
				       PicturePtr pSrc,
				       PicturePtr pMask,
				       PicturePtr pDst,
				       INT16      xSrc,
				       INT16      ySrc,
				       INT16      xMask,
				       INT16      yMask,
				       INT16      xDst,
				       INT16      yDst,
				       CARD16     width,
				       CARD16     height);
Bool fbSolidFillmmx (DrawablePtr	pDraw,
		     int		x,
		     int		y,
		     int		width,
		     int		height,
		     FbBits		xor);

#endif /* USE_GCC34_MMX */
