/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nsc/nsc_gx2_dga.c,v 1.1 2002/12/10 15:12:24 alanh Exp $ */
/*
 * $Workfile: nsc_gx2_dga.c $
 * $Revision: 1.3 $
 * $Author: matthieu $
 * 
 * File contents: DGA(Direct Acess Graphics mode) is feature of
 *                XFree86 that allows the program to access directly to video
 *                memory on the graphics card.DGA supports the double
 *                flickering.This file has the functions to support the DGA
 *                modes.
 *
 * Project:       Geode Xfree Frame buffer device driver.
 *
 */

/* 
 * NSC_LIC_ALTERNATIVE_PREAMBLE
 *
 * Revision 1.0
 *
 * National Semiconductor Alternative GPL-BSD License
 *
 * National Semiconductor Corporation licenses this software 
 * ("Software"):
 *
 * National Xfree frame buffer driver
 *
 * under one of the two following licenses, depending on how the 
 * Software is received by the Licensee.
 * 
 * If this Software is received as part of the Linux Framebuffer or
 * other GPL licensed software, then the GPL license designated 
 * NSC_LIC_GPL applies to this Software; in all other circumstances 
 * then the BSD-style license designated NSC_LIC_BSD shall apply.
 *
 * END_NSC_LIC_ALTERNATIVE_PREAMBLE */

/* NSC_LIC_BSD
 *
 * National Semiconductor Corporation Open Source License for 
 *
 * National Xfree frame buffer driver
 *
 * (BSD License with Export Notice)
 *
 * Copyright (c) 1999-2001
 * National Semiconductor Corporation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer. 
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials provided 
 *     with the distribution. 
 *
 *   * Neither the name of the National Semiconductor Corporation nor 
 *     the names of its contributors may be used to endorse or promote 
 *     products derived from this software without specific prior 
 *     written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * NATIONAL SEMICONDUCTOR CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE,
 * INTELLECTUAL PROPERTY INFRINGEMENT, OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * EXPORT LAWS: THIS LICENSE ADDS NO RESTRICTIONS TO THE EXPORT LAWS OF 
 * YOUR JURISDICTION. It is licensee's responsibility to comply with 
 * any export regulations applicable in licensee's jurisdiction. Under 
 * CURRENT (2001) U.S. export regulations this software 
 * is eligible for export from the U.S. and can be downloaded by or 
 * otherwise exported or reexported worldwide EXCEPT to U.S. embargoed 
 * destinations which include Cuba, Iraq, Libya, North Korea, Iran, 
 * Syria, Sudan, Afghanistan and any other country to which the U.S. 
 * has embargoed goods and services. 
 *
 * END_NSC_LIC_BSD */

/* NSC_LIC_GPL
 *
 * National Semiconductor Corporation Gnu General Public License for 
 *
 * National Xfree frame buffer driver
 *
 * (GPL License with Export Notice)
 *
 * Copyright (c) 1999-2001
 * National Semiconductor Corporation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted under the terms of the GNU General 
 * Public License as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later version  
 *
 * In addition to the terms of the GNU General Public License, neither 
 * the name of the National Semiconductor Corporation nor the names of 
 * its contributors may be used to endorse or promote products derived 
 * from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * NATIONAL SEMICONDUCTOR CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE, 
 * INTELLECTUAL PROPERTY INFRINGEMENT, OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE. See the GNU General Public License for more details. 
 *
 * EXPORT LAWS: THIS LICENSE ADDS NO RESTRICTIONS TO THE EXPORT LAWS OF 
 * YOUR JURISDICTION. It is licensee's responsibility to comply with 
 * any export regulations applicable in licensee's jurisdiction. Under 
 * CURRENT (2001) U.S. export regulations this software 
 * is eligible for export from the U.S. and can be downloaded by or 
 * otherwise exported or reexported worldwide EXCEPT to U.S. embargoed 
 * destinations which include Cuba, Iraq, Libya, North Korea, Iran, 
 * Syria, Sudan, Afghanistan and any other country to which the U.S. 
 * has embargoed goods and services. 
 *
 * You should have received a copy of the GNU General Public License 
 * along with this file; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 *
 * END_NSC_LIC_GPL */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Pci.h"
#include "xf86PciInfo.h"
#include "xaa.h"
#include "xaalocal.h"
#include "nsc.h"
#include "dgaproc.h"

/* forward declarations */
Bool GX2DGAInit(ScreenPtr pScreen);
static Bool GX2_OpenFramebuffer(ScrnInfoPtr, char **, unsigned char **,
				int *, int *, int *);
static void GX2_CloseFramebuffer(ScrnInfoPtr pScrn);
static Bool GX2_SetMode(ScrnInfoPtr, DGAModePtr);
static int GX2_GetViewport(ScrnInfoPtr);
static void GX2_SetViewport(ScrnInfoPtr, int, int, int);
static void GX2_FillRect(ScrnInfoPtr, int, int, int, int, unsigned long);
static void GX2_BlitRect(ScrnInfoPtr, int, int, int, int, int, int);

extern void GX2AdjustFrame(int, int, int, int);
extern Bool GX2SwitchMode(int, DisplayModePtr, int);
extern void GX2AccelSync(ScrnInfoPtr pScreenInfo);

static DGAFunctionRec GX2DGAFuncs = {
   GX2_OpenFramebuffer,
   GX2_CloseFramebuffer,
   GX2_SetMode,
   GX2_SetViewport,
   GX2_GetViewport,
   GX2AccelSync,
   GX2_FillRect,
   GX2_BlitRect,
   NULL
};

/*----------------------------------------------------------------------------
 * GX2DGAInit.
 *
 * Description	:This function is used to intiallize the DGA modes and sets the
			 	 viewport based on the screen mode.
 * Parameters.
 *	pScreeen	:Pointer to screen info structure.
 *
 * Returns		:TRUE on success and FALSE on failure.
 *
 * Comments		:This function prepares the DGA mode settings for
 *				 other func reference.
 *
*----------------------------------------------------------------------------
*/
Bool
GX2DGAInit(ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   GeodePtr pGeode = GEODEPTR(pScrn);
   DGAModePtr modes = NULL, newmodes = NULL, currentMode;
   DisplayModePtr pMode, firstMode;
   int Bpp = pScrn->bitsPerPixel >> 3;
   int num = 0;
   Bool oneMore;

   pMode = firstMode = pScrn->modes;
   DEBUGMSG(0, (0, X_NONE, "GX2DGAInit %d\n", Bpp));
   while (pMode) {

      /* redundant but it can be used in future:if(0). */
      if (0) {				/*pScrn->displayWidth != pMode->HDisplay */
	 /* memory is allocated for dga to
	  *setup the viewport and mode parameters
	  */
	 newmodes = xrealloc(modes, (num + 2) * sizeof(DGAModeRec));
	 oneMore = TRUE;
      } else {
	 /* one record is allocated here */
	 newmodes = xrealloc(modes, (num + 1) * sizeof(DGAModeRec));
	 oneMore = FALSE;
      }
      if (!newmodes) {
	 xfree(modes);
	 return FALSE;
      }
      modes = newmodes;

    SECOND_PASS:			/* DGA mode flgas and viewport parametrs are set here. */

      currentMode = modes + num;
      num++;
      currentMode->mode = pMode;
      currentMode->flags = DGA_CONCURRENT_ACCESS | DGA_PIXMAP_AVAILABLE;
      currentMode->flags |= DGA_FILL_RECT | DGA_BLIT_RECT;
      if (pMode->Flags & V_DBLSCAN)
	 currentMode->flags |= DGA_DOUBLESCAN;
      if (pMode->Flags & V_INTERLACE)
	 currentMode->flags |= DGA_INTERLACED;
      currentMode->byteOrder = pScrn->imageByteOrder;
      currentMode->depth = pScrn->depth;
      currentMode->bitsPerPixel = pScrn->bitsPerPixel;
      currentMode->red_mask = pScrn->mask.red;
      currentMode->green_mask = pScrn->mask.green;
      currentMode->blue_mask = pScrn->mask.blue;
      currentMode->visualClass = (Bpp == 1) ? PseudoColor : TrueColor;
      currentMode->viewportWidth = pMode->HDisplay;
      currentMode->viewportHeight = pMode->VDisplay;
      currentMode->xViewportStep = 1;
      currentMode->yViewportStep = 1;
      currentMode->viewportFlags = DGA_FLIP_RETRACE;
      currentMode->offset = 0;
      currentMode->address = pGeode->FBBase;
      if (oneMore) {			/* first one is narrow width */
	 currentMode->bytesPerScanline = ((pMode->HDisplay * Bpp) + 3) & ~3L;
	 currentMode->imageWidth = pMode->HDisplay;
	 currentMode->imageHeight = pMode->VDisplay;
	 currentMode->pixmapWidth = currentMode->imageWidth;
	 currentMode->pixmapHeight = currentMode->imageHeight;
	 currentMode->maxViewportX = currentMode->imageWidth -
	       currentMode->viewportWidth;
	 /* this might need to get clamped to some maximum */
	 currentMode->maxViewportY = currentMode->imageHeight -
	       currentMode->viewportHeight;
	 oneMore = FALSE;
	 goto SECOND_PASS;
      } else {
	 currentMode->bytesPerScanline =
	       ((pScrn->displayWidth * Bpp) + 3) & ~3L;
	 currentMode->imageWidth = pScrn->displayWidth;
	 currentMode->imageHeight = pMode->VDisplay;
	 currentMode->pixmapWidth = currentMode->imageWidth;
	 currentMode->pixmapHeight = currentMode->imageHeight;
	 currentMode->maxViewportX = currentMode->imageWidth -
	       currentMode->viewportWidth;
	 /* this might need to get clamped to some maximum */
	 currentMode->maxViewportY = currentMode->imageHeight -
	       currentMode->viewportHeight;
      }
      pMode = pMode->next;
      if (pMode == firstMode)
	 break;
   }
   pGeode->numDGAModes = num;
   pGeode->DGAModes = modes;
   return DGAInit(pScreen, &GX2DGAFuncs, modes, num);
}

/*----------------------------------------------------------------------------
 * GX2_SetMode.
 *
 * Description	:This function is sets into the DGA mode.
 *.
 * Parameters.
 *	pScreeen	:Pointer to screen info structure.
 *	pMode		:Points to the DGAmode ptr data
 * Returns		:TRUE on success and FALSE on failure.
 *
 * Comments		:none.
 *			
 *
*----------------------------------------------------------------------------
*/
static Bool
GX2_SetMode(ScrnInfoPtr pScrn, DGAModePtr pMode)
{
   static int OldDisplayWidth[MAXSCREENS];
   int index = pScrn->pScreen->myNum;
   GeodePtr pGeode = GEODEPTR(pScrn);

   DEBUGMSG(0, (0, X_NONE, "GX2_SetMode\n"));
   if (!pMode) {
      /* restore the original mode
       * * put the ScreenParameters back
       */
      pScrn->displayWidth = OldDisplayWidth[index];
      DEBUGMSG(0,
	       (0, X_NONE, "GX2_SetMode !pMode %d\n", pScrn->displayWidth));
      GX2SwitchMode(index, pScrn->currentMode, 0);
      pGeode->DGAactive = FALSE;
   } else {
      if (!pGeode->DGAactive) {		/* save the old parameters */
	 OldDisplayWidth[index] = pScrn->displayWidth;
	 pGeode->DGAactive = TRUE;
	 DEBUGMSG(0,
		  (0, X_NONE, "GX2_SetMode pMode+ NA %d\n",
		   pScrn->displayWidth));
      }
      pScrn->displayWidth = pMode->bytesPerScanline /
	    (pMode->bitsPerPixel >> 3);
      DEBUGMSG(0,
	       (0, X_NONE, "GX2_SetMode pMode+  %d\n", pScrn->displayWidth));
      GX2SwitchMode(index, pMode->mode, 0);
   }
   /* enable/disable Compression */
   if (pGeode->Compression) {
      GFX(set_compression_enable(!pGeode->DGAactive));
   }

   /* enable/disable cursor */
   if (pGeode->HWCursor) {
      GFX(set_cursor_enable(!pGeode->DGAactive));
   }

   return TRUE;
}

/*----------------------------------------------------------------------------
 * GX2_GetViewPort.
 *
 * Description	:This function is Gets the viewport window memory.
 *.
 * Parameters.
 *	pScrn		:Pointer to screen info structure.
 *	
 * Returns		:returns the viewport status.
 *
 * Comments		:none.
 *			
 *
*----------------------------------------------------------------------------
*/
static int
GX2_GetViewport(ScrnInfoPtr pScrn)
{
   GeodePtr pGeode = GEODEPTR(pScrn);

   return pGeode->DGAViewportStatus;
}

/*----------------------------------------------------------------------------
 * GX2_SetViewPort.
 *
 * Description	:This function is Gets the viewport window memory.
 *
 * Parameters.
 *	pScrn		:Pointer to screen info structure.
		x		:x-cordinate of viewport window
 *		y		:y-codinate of the viewport window.
 *	flags		:indicates the viewport to be flipped or not.
 * Returns		:returns the viewport status  as zero.
 *
 * Comments		:none.
 *			
*----------------------------------------------------------------------------
*/
static void
GX2_SetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
   GeodePtr pGeode = GEODEPTR(pScrn);

   GX2AdjustFrame(pScrn->pScreen->myNum, x, y, flags);
   pGeode->DGAViewportStatus = 0;	/*GX2AdjustFrame loops until finished */
}

/*----------------------------------------------------------------------------
 * GX2_FillRect.
 *
 * Description	:This function is Gets the viewport window memory.
 *.
 * Parameters.
 *	pScrn		:Pointer to screen info structure.
 *		x		:x-cordinate of viewport window
 *		y		:y-codinate of the viewport window.
 *		w		:width of the rectangle
 *      h		:height of the rectangle.
 *	color		:color to be filled in rectangle.
 *
 * Returns		:returns the viewport status  as zero.
 *
 * Comments		:This function is implemented by solidfill routines..
 *			
*----------------------------------------------------------------------------
*/
static void
GX2_FillRect(ScrnInfoPtr pScrn, int x, int y,
	     int w, int h, unsigned long color)
{
   GeodePtr pGeode = GEODEPTR(pScrn);

   if (pGeode->AccelInfoRec) {
      (*pGeode->AccelInfoRec->SetupForSolidFill) (pScrn, color, GXcopy, ~0);
      (*pGeode->AccelInfoRec->SubsequentSolidFillRect) (pScrn, x, y, w, h);
      SET_SYNC_FLAG(pGeode->AccelInfoRec);
   }
}

/*----------------------------------------------------------------------------
 * GX2_BlitRect.
 *
 * Description	:This function implementing Blit and it moves a
 *			 	 Rectangular block of data from one location to other
 *			 	 Location.
 *
 * Parameters.
 *	pScrn		:Pointer to screen info structure.
 *	srcx		:x-cordinate of the src rectangle
 *	srcy		:y-codinate of src rectangle.
 *	  w			:width of the rectangle
 *    h			:height of the rectangle.
 *	dstx		:x-cordinate of the dst rectangle.
 *	dsty		:y -coordinates of the dst rectangle.
 * Returns		:none.
 *
 * Comments		:none
 *			
*----------------------------------------------------------------------------
*/
static void
GX2_BlitRect(ScrnInfoPtr pScrn,
	     int srcx, int srcy, int w, int h, int dstx, int dsty)
{
   GeodePtr pGeode = GEODEPTR(pScrn);

   if (pGeode->AccelInfoRec) {
      int xdir = ((srcx < dstx) && (srcy == dsty)) ? -1 : 1;
      int ydir = (srcy < dsty) ? -1 : 1;

      (*pGeode->AccelInfoRec->SetupForScreenToScreenCopy)
	    (pScrn, xdir, ydir, GXcopy, ~0, -1);
      (*pGeode->AccelInfoRec->SubsequentScreenToScreenCopy) (pScrn, srcx,
							     srcy, dstx, dsty,
							     w, h);
      SET_SYNC_FLAG(pGeode->AccelInfoRec);
   }
}

/*----------------------------------------------------------------------------
 * GX2_OpenFramebuffer.
 *
 * Description	:This function open the framebuffer driver for DGA.
 *			
 * Parameters.
 *	pScrn		:Pointer to screen info structure.
 *	srcx		:x-cordinate of the src rectangle
 *	srcy		:y-codinate of src rectangle.
 *		w		:width of the rectangle
 *    	h		:height of the rectangle.
 *	dstx		:x-cordinate of the dst rectangle.
 *	dsty		:y -coordinates of the dst rectangle.
 * Returns		:none.
 *
 * Comments		:none
 *			
*----------------------------------------------------------------------------
*/
static Bool
GX2_OpenFramebuffer(ScrnInfoPtr pScrn,
		    char **name, unsigned char **mem,
		    int *size, int *offset, int *flags)
{
   GeodePtr pGeode = GEODEPTR(pScrn);

   *name = NULL;			/* no special device */
   *mem = (unsigned char *)pGeode->FBLinearAddr;
   *size = pGeode->FBSize;
   *offset = 0;
   *flags = DGA_NEED_ROOT;
   return TRUE;
}

static void
GX2_CloseFramebuffer(ScrnInfoPtr pScrn)
{
}

/* end of file */
