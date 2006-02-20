/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6_dga.c,v 1.11 2002/09/18 18:14:58 martin Exp $ */
/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@xfree86.org>
 *
 * Credits:
 *
 *   Thanks to Ove Kåven <ovek@transgaming.com> for writing the Rage 128
 *   DGA support.  Portions of this file are based on the initialization
 *   code for that driver.
 *
 */

				/* Driver data structures */
#include "rm6.h"
#include "rm6_probe.h"

				/* X and server generic header files */
#include "xf86.h"

				/* DGA support */
#include "dgaproc.h"


static Bool RM6_OpenFramebuffer(ScrnInfoPtr, char **, unsigned char **,
				   int *, int *, int *);
static Bool RM6_SetMode(ScrnInfoPtr, DGAModePtr);
static int  RM6_GetViewport(ScrnInfoPtr);
static void RM6_SetViewport(ScrnInfoPtr, int, int, int);
static void RM6_FillRect(ScrnInfoPtr, int, int, int, int, unsigned long);
static void RM6_BlitRect(ScrnInfoPtr, int, int, int, int, int, int);
static void RM6_BlitTransRect(ScrnInfoPtr, int, int, int, int, int, int,
				 unsigned long);


static DGAModePtr RM6SetupDGAMode(ScrnInfoPtr pScrn,
				     DGAModePtr modes,
				     int *num,
				     int bitsPerPixel,
				     int depth,
				     Bool pixmap,
				     int secondPitch,
				     unsigned long red,
				     unsigned long green,
				     unsigned long blue,
				     short visualClass)
{
    RM6InfoPtr   info     = RM6PTR(pScrn);
    DGAModePtr      newmodes = NULL;
    DGAModePtr      currentMode;
    DisplayModePtr  pMode;
    DisplayModePtr  firstMode;
    unsigned int    size;
    int             pitch;
    int             Bpp      = bitsPerPixel >> 3;

SECOND_PASS:

    pMode = firstMode = pScrn->modes;

    while (1) {
	pitch = pScrn->displayWidth;
	size = pitch * Bpp * pMode->VDisplay;

	if ((!secondPitch || (pitch != secondPitch)) &&
	    (size <= info->FbMapSize)) {

	    if (secondPitch)
		pitch = secondPitch;

	    if (!(newmodes = xrealloc(modes, (*num + 1) * sizeof(DGAModeRec))))
		break;

	    modes       = newmodes;
	    currentMode = modes + *num;

	    currentMode->mode           = pMode;
	    currentMode->flags          = DGA_CONCURRENT_ACCESS;

	    if (pixmap)
		currentMode->flags     |= DGA_PIXMAP_AVAILABLE;

	    if (info->accel) {
	      if (info->accel->SetupForSolidFill &&
		  info->accel->SubsequentSolidFillRect)
		 currentMode->flags    |= DGA_FILL_RECT;
	      if (info->accel->SetupForScreenToScreenCopy &&
		  info->accel->SubsequentScreenToScreenCopy)
		 currentMode->flags    |= DGA_BLIT_RECT | DGA_BLIT_RECT_TRANS;
	      if (currentMode->flags &
		  (DGA_PIXMAP_AVAILABLE | DGA_FILL_RECT |
		   DGA_BLIT_RECT | DGA_BLIT_RECT_TRANS))
		  currentMode->flags   &= ~DGA_CONCURRENT_ACCESS;
	    }
	    if (pMode->Flags & V_DBLSCAN)
		currentMode->flags     |= DGA_DOUBLESCAN;
	    if (pMode->Flags & V_INTERLACE)
		currentMode->flags     |= DGA_INTERLACED;

	    currentMode->byteOrder      = pScrn->imageByteOrder;
	    currentMode->depth          = depth;
	    currentMode->bitsPerPixel   = bitsPerPixel;
	    currentMode->red_mask       = red;
	    currentMode->green_mask     = green;
	    currentMode->blue_mask      = blue;
	    currentMode->visualClass    = visualClass;
	    currentMode->viewportWidth  = pMode->HDisplay;
	    currentMode->viewportHeight = pMode->VDisplay;
	    currentMode->xViewportStep  = 8;
	    currentMode->yViewportStep  = 1;
	    currentMode->viewportFlags  = DGA_FLIP_RETRACE;
	    currentMode->offset         = 0;
	    currentMode->address        = (unsigned char*)info->LinearAddr;
	    currentMode->bytesPerScanline = pitch * Bpp;
	    currentMode->imageWidth     = pitch;
	    currentMode->imageHeight    = (info->FbMapSize
					   / currentMode->bytesPerScanline);
	    currentMode->pixmapWidth    = currentMode->imageWidth;
	    currentMode->pixmapHeight   = currentMode->imageHeight;
	    currentMode->maxViewportX   = (currentMode->imageWidth
					   - currentMode->viewportWidth);
	    /* this might need to get clamped to some maximum */
	    currentMode->maxViewportY   = (currentMode->imageHeight
					   - currentMode->viewportHeight);
	    (*num)++;
	}

	pMode = pMode->next;
	if (pMode == firstMode)
	    break;
    }

    if (secondPitch) {
	secondPitch = 0;
	goto SECOND_PASS;
    }

    return modes;
}

Bool RM6DGAInit(ScreenPtr pScreen)
{
    ScrnInfoPtr    pScrn = xf86Screens[pScreen->myNum];
    RM6InfoPtr  info  = RM6PTR(pScrn);
    DGAModePtr     modes = NULL;
    int            num   = 0;

    /* 8 */
    modes = RM6SetupDGAMode(pScrn, modes, &num, 8, 8,
			       (pScrn->bitsPerPixel == 8),
				((pScrn->bitsPerPixel != 8)
				 ? 0 : pScrn->displayWidth),
			       0, 0, 0, PseudoColor);

    /* 15 */
    modes = RM6SetupDGAMode(pScrn, modes, &num, 16, 15,
			       (pScrn->bitsPerPixel == 16),
			       ((pScrn->depth != 15)
				? 0 : pScrn->displayWidth),
			       0x7c00, 0x03e0, 0x001f, TrueColor);

    modes = RM6SetupDGAMode(pScrn, modes, &num, 16, 15,
			       (pScrn->bitsPerPixel == 16),
			       ((pScrn->depth != 15)
				? 0 : pScrn->displayWidth),
			       0x7c00, 0x03e0, 0x001f, DirectColor);

    /* 16 */
    modes = RM6SetupDGAMode(pScrn, modes, &num, 16, 16,
			       (pScrn->bitsPerPixel == 16),
			       ((pScrn->depth != 16)
				? 0 : pScrn->displayWidth),
			       0xf800, 0x07e0, 0x001f, TrueColor);

    modes = RM6SetupDGAMode(pScrn, modes, &num, 16, 16,
			       (pScrn->bitsPerPixel == 16),
			       ((pScrn->depth != 16)
				? 0 : pScrn->displayWidth),
			       0xf800, 0x07e0, 0x001f, DirectColor);

    /* 32 */
    modes = RM6SetupDGAMode(pScrn, modes, &num, 32, 24,
			       (pScrn->bitsPerPixel == 32),
			       ((pScrn->bitsPerPixel != 32)
				? 0 : pScrn->displayWidth),
			       0xff0000, 0x00ff00, 0x0000ff, TrueColor);

    modes = RM6SetupDGAMode(pScrn, modes, &num, 32, 24,
			       (pScrn->bitsPerPixel == 32),
			       ((pScrn->bitsPerPixel != 32)
				? 0 : pScrn->displayWidth),
			       0xff0000, 0x00ff00, 0x0000ff, DirectColor);

    info->numDGAModes = num;
    info->DGAModes    = modes;

    info->DGAFuncs.OpenFramebuffer       = RM6_OpenFramebuffer;
    info->DGAFuncs.CloseFramebuffer      = NULL;
    info->DGAFuncs.SetMode               = RM6_SetMode;
    info->DGAFuncs.SetViewport           = RM6_SetViewport;
    info->DGAFuncs.GetViewport           = RM6_GetViewport;

    info->DGAFuncs.Sync                  = NULL;
    info->DGAFuncs.FillRect              = NULL;
    info->DGAFuncs.BlitRect              = NULL;
    info->DGAFuncs.BlitTransRect         = NULL;

    if (info->accel) {
	info->DGAFuncs.Sync              = info->accel->Sync;
	if (info->accel->SetupForSolidFill &&
	    info->accel->SubsequentSolidFillRect)
	    info->DGAFuncs.FillRect      = RM6_FillRect;
	if (info->accel->SetupForScreenToScreenCopy &&
	    info->accel->SubsequentScreenToScreenCopy) {
	    info->DGAFuncs.BlitRect      = RM6_BlitRect;
	    info->DGAFuncs.BlitTransRect = RM6_BlitTransRect;
	}
    }

    return DGAInit(pScreen, &info->DGAFuncs, modes, num);
}

static Bool RM6_SetMode(ScrnInfoPtr pScrn, DGAModePtr pMode)
{
    static RM6FBLayout  SavedLayouts[MAXSCREENS];
    int                    indx = pScrn->pScreen->myNum;
    RM6InfoPtr          info = RM6PTR(pScrn);

    if (!pMode) { /* restore the original mode */
	/* put the ScreenParameters back */
	if (info->DGAactive)
	    memcpy(&info->CurrentLayout, &SavedLayouts[indx],
		   sizeof(RM6FBLayout));

	pScrn->currentMode = info->CurrentLayout.mode;

	RM6SwitchMode(indx, pScrn->currentMode, 0);
	if (info->accelOn)
	    RM6EngineInit(pScrn);
	RM6AdjustFrame(indx, 0, 0, 0);
	info->DGAactive = FALSE;
    } else {
	if (!info->DGAactive) {  /* save the old parameters */
	    memcpy(&SavedLayouts[indx], &info->CurrentLayout,
		   sizeof(RM6FBLayout));
	    info->DGAactive = TRUE;
	}

	info->CurrentLayout.bitsPerPixel = pMode->bitsPerPixel;
	info->CurrentLayout.depth        = pMode->depth;
	info->CurrentLayout.displayWidth = (pMode->bytesPerScanline /
					    (pMode->bitsPerPixel >> 3));
	info->CurrentLayout.pixel_bytes  = pMode->bitsPerPixel / 8;
	info->CurrentLayout.pixel_code   = (pMode->bitsPerPixel != 16
					    ? pMode->bitsPerPixel
					    : pMode->depth);
	/* RM6ModeInit() will set the mode field */

	RM6SwitchMode(indx, pMode->mode, 0);

	if (info->accelOn)
	    RM6EngineInit(pScrn);
    }

    return TRUE;
}

static int RM6_GetViewport(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    return info->DGAViewportStatus;
}

static void RM6_SetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    RM6AdjustFrame(pScrn->pScreen->myNum, x, y, flags);
    info->DGAViewportStatus = 0;  /* FIXME */
}

static void RM6_FillRect(ScrnInfoPtr pScrn,
			    int x, int y, int w, int h,
			    unsigned long color)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    (*info->accel->SetupForSolidFill)(pScrn, color, GXcopy, (CARD32)(~0));
    (*info->accel->SubsequentSolidFillRect)(pScrn, x, y, w, h);

    if (pScrn->bitsPerPixel == info->CurrentLayout.bitsPerPixel)
	SET_SYNC_FLAG(info->accel);
}

static void RM6_BlitRect(ScrnInfoPtr pScrn,
			    int srcx, int srcy, int w, int h,
			    int dstx, int dsty)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    int            xdir = ((srcx < dstx) && (srcy == dsty)) ? -1 : 1;
    int            ydir = (srcy < dsty) ? -1 : 1;

    (*info->accel->SetupForScreenToScreenCopy)(pScrn, xdir, ydir,
					       GXcopy, (CARD32)(~0), -1);
    (*info->accel->SubsequentScreenToScreenCopy)(pScrn, srcx, srcy,
						 dstx, dsty, w, h);

    if (pScrn->bitsPerPixel == info->CurrentLayout.bitsPerPixel)
	SET_SYNC_FLAG(info->accel);
}

static void RM6_BlitTransRect(ScrnInfoPtr pScrn,
				 int srcx, int srcy, int w, int h,
				 int dstx, int dsty, unsigned long color)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    int            xdir = ((srcx < dstx) && (srcy == dsty)) ? -1 : 1;
    int            ydir = (srcy < dsty) ? -1 : 1;

    info->XAAForceTransBlit = TRUE;

    (*info->accel->SetupForScreenToScreenCopy)(pScrn, xdir, ydir,
					       GXcopy, (CARD32)(~0), color);

    info->XAAForceTransBlit = FALSE;

    (*info->accel->SubsequentScreenToScreenCopy)(pScrn, srcx, srcy,
						 dstx, dsty, w, h);

    if (pScrn->bitsPerPixel == info->CurrentLayout.bitsPerPixel)
	SET_SYNC_FLAG(info->accel);
}

static Bool RM6_OpenFramebuffer(ScrnInfoPtr pScrn,
				   char **name,
				   unsigned char **mem,
				   int *size, int *offset, int *flags)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    *name   = NULL;             /* no special device */
    *mem    = (unsigned char*)info->LinearAddr;
    *size   = info->FbMapSize;
    *offset = 0;
    *flags  = 0; /* DGA_NEED_ROOT; -- don't need root, just /dev/mem access */

    return TRUE;
}
