/* $XFree86$ */
/*
 * Copyright 2003 Alex Deucher.
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ALEX DEUCHER, OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Alex Deucher <agd5f@yahoo.com>
 *
 */

#ifndef _RM6_MERGEDFB_H_
#define _RM6_MERGEDFB_H_

#include "xf86.h"

#include "rm6.h"


#define SDMPTR(x) ((RM6MergedDisplayModePtr)(x->currentMode->Private))
#define CDMPTR    ((RM6MergedDisplayModePtr)(info->CurrentLayout.mode->Private))

#define BOUND(test,low,hi) { \
    if(test < low) test = low; \
    if(test > hi) test = hi; }

#define REBOUND(low,hi,test) { \
    if(test < low) { \
        hi += test-low; \
        low = test; } \
    if(test > hi) { \
        low += test-hi; \
        hi = test; } }

typedef struct _MergedDisplayModeRec {
    DisplayModePtr CRT1;
    DisplayModePtr CRT2;
    RM6Scrn2Rel    CRT2Position;
} RM6MergedDisplayModeRec, *RM6MergedDisplayModePtr;

typedef struct _region {
    int x0,x1,y0,y1;
} region;

typedef struct _RM6XineramaData {
    int x;
    int y;
    int width;
    int height;
} RM6XineramaData;

/* needed by rm6_driver.c */
extern void
RM6AdjustFrameMerged(int scrnIndex, int x, int y, int flags);
extern void
RM6MergePointerMoved(int scrnIndex, int x, int y);
extern DisplayModePtr
RM6GenerateModeList(ScrnInfoPtr pScrn, char* str,
		    DisplayModePtr i, DisplayModePtr j,
		    RM6Scrn2Rel srel);
extern int
RM6StrToRanges(range *r, char *s, int max);
extern void
RM6XineramaExtensionInit(ScrnInfoPtr pScrn);
extern void
RM6UpdateXineramaScreenInfo(ScrnInfoPtr pScrn1);
extern void
RM6MergedFBSetDpi(ScrnInfoPtr pScrn1, ScrnInfoPtr pScrn2, RM6Scrn2Rel srel);
extern void
RM6RecalcDefaultVirtualSize(ScrnInfoPtr pScrn);

/* needed by rm6_cursor.c */
extern void
RM6SetCursorPositionMerged(ScrnInfoPtr pScrn, int x, int y);

/* needed by rm6_video.c */
extern void
RM6ChooseOverlayCRTC(ScrnInfoPtr, BoxPtr);

#endif /* _RM6_MERGEDFB_H_ */
