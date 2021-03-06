/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3_virge/s3plypt.c,v 3.6.2.3 1997/05/24 08:36:02 dawes Exp $ */
/************************************************************

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)

KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

********************************************************/


/*
 * Modified by Amancio Hasty and Jon Tombs
 *
 */
/* $XConsortium: s3plypt.c /main/3 1996/10/25 15:37:48 kaleb $ */


#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfb16.h"
#include "cfb24.h"
#include "cfb32.h"
#include "cfbmskbits.h"
#include "misc.h"
#include "xf86.h"
#include "s3v.h"

#define isClipped(c,ul,lr)  ((((c) - (ul)) | ((lr) - (c))) & ClipMask)

void
s3PolyPoint(pDrawable, pGC, mode, npt, pptInit)
     DrawablePtr pDrawable;
     GCPtr pGC;
     int   mode;
     int   npt;
     xPoint *pptInit;
{
   register int pt;
   register int c1, c2;
   register unsigned int ClipMask = 0x80008000;
   register int *ppt;
   RegionPtr cclip;
   int   nbox;
   register int i;
   register BoxPtr pbox;
   int   off;
   cfbPrivGCPtr devPriv;
   xPoint *pptPrev;
   int s3_clr, s3_rop = -1;

   if (!pGC->planemask || pGC->alu == GXnoop)  /* for xgc "benchmarks" ;-) */
      return;

   devPriv = (cfbPrivGC *) (pGC->devPrivates[cfbGCPrivateIndex].ptr);
   s3_rop = s3ConvertPlanemask(pGC, &s3_clr);

   if (!xf86VTSema || (s3_rop == -1)) {
      if (xf86VTSema) WaitIdleEmpty();
      switch (s3InfoRec.bitsPerPixel) {
      case 8:
	 cfbPolyPoint(pDrawable, pGC, mode, npt, pptInit);
         break;
      case 16:
	 cfb16PolyPoint(pDrawable, pGC, mode, npt, pptInit);
         break;
      case 24:
	 cfb24PolyPoint(pDrawable, pGC, mode, npt, pptInit);
         break;
      case 32:
	 cfb32PolyPoint(pDrawable, pGC, mode, npt, pptInit);
	 break;
      }
      if (xf86VTSema) WaitIdleEmpty();
      return;
   }

   cclip = devPriv->pCompositeClip;
   if ((mode == CoordModePrevious) && (npt > 1)) {
      for (pptPrev = pptInit + 1, i = npt - 1; --i >= 0; pptPrev++) {
	 pptPrev->x += (pptPrev - 1)->x;
	 pptPrev->y += (pptPrev - 1)->y;
      }
   }
   off = *((int *)&pDrawable->x);
   off -= (off & 0x8000) << 1;

   BLOCK_CURSOR;

   if (npt > 7) { /* 16 (fifo) - 7 (No. of cmds) - 2 (buffer in WaitQueue) */
      int nwait=0;
      WaitQueue(3);
      SETB_PAT_FG_CLR(s3_clr);
      SETB_CMD_SET(s3_gcmd | CMD_BITBLT | MIX_MONO_PATT | CMD_AUTOEXEC | s3_rop);
      SETB_RWIDTH_HEIGHT(0,1);

      for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
           --nbox >= 0;
           pbox++) {
         c1 = *((int *)&pbox->x1) - off;
         c2 = *((int *)&pbox->x2) - off - 0x00010001;
         for (ppt = (int *)pptInit, i = npt; --i >= 0;) {
            pt = *ppt++;
            if (!isClipped(pt, c1, c2)) {
	       if (nwait-- == 0) {
		  nwait = 7;
		  WaitQueue(8);
	       }
               SETB_RDEST_XY((short)intToX(pt) + pDrawable->x, (short)intToY(pt) + pDrawable->y);
            }
        }
      }
      WaitQueue(4);

   } else {
      /* Use only 1 WaitQueue for a small number of points. This speeds
       * things up a bit.
       */
      WaitQueue(7 + npt);
      SETB_PAT_FG_CLR(s3_clr);
      SETB_CMD_SET(s3_gcmd | CMD_BITBLT | MIX_MONO_PATT | CMD_AUTOEXEC | s3_rop);
      SETB_RWIDTH_HEIGHT(0,1);

      for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip);
           --nbox >= 0;
           pbox++) {
         c1 = *((int *)&pbox->x1) - off;
         c2 = *((int *)&pbox->x2) - off - 0x00010001;
         for (ppt = (int *)pptInit, i = npt; --i >= 0;) {
            pt = *ppt++;
            if (!isClipped(pt, c1, c2)) {
               SETB_RDEST_XY((short)intToX(pt) + pDrawable->x, (short)intToY(pt) + pDrawable->y);
            }
         }
      }
   }

   SETB_CMD_SET(CMD_NOP);

   /* avoid system hangs again :-( */
   SETB_RSRC_XY(0,0);
   SETB_RDEST_XY(0,0);
   SETB_CMD_SET(s3_gcmd | CMD_BITBLT | ROP_S);
   WaitIdle();

   UNBLOCK_CURSOR;
}
