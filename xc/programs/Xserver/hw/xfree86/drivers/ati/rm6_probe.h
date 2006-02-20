/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6_probe.h,v 1.13 2003/10/30 17:37:00 tsi Exp $ */
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
 * Modified by Marc Aurele La France <tsi@xfree86.org> for ATI driver merge.
 */

#ifndef _RM6_PROBE_H_
#define _RM6_PROBE_H_ 1

#include "atiproto.h"

#include "xf86str.h"
#include "xf86DDC.h"

#define _XF86MISC_SERVER_
#include "xf86misc.h"

#include "radeon_probe.h"


typedef struct
{
    Bool HasSecondary;

    /*
     * The next two are used to make sure CRTC2 is restored before CRTC_EXT,
     * otherwise it could lead to blank screens.
     */
    Bool IsSecondaryRestored;
    Bool RestorePrimary;

    ScrnInfoPtr pSecondaryScrn;
    ScrnInfoPtr pPrimaryScrn;

    int MonType1;
    int MonType2;
    xf86MonPtr MonInfo1;
    xf86MonPtr MonInfo2;
    Bool ReversedDAC;	  /* TVDAC used as primary dac */
    Bool ReversedTMDS;    /* DDC_DVI is used for external TMDS */
    RADEONConnector PortInfo[2];
} RM6EntRec, *RM6EntPtr;

/* rm6_probe.c */
extern const OptionInfoRec *RM6AvailableOptions(int, int);
extern void                 RM6Identify(int);
extern Bool                 RM6Probe(DriverPtr, int);

extern SymTabRec            RM6Chipsets[];
extern PciChipsets          RM6PciChipsets[];

/* rm6_driver.c */
extern void                 RM6LoaderRefSymLists(void);
extern Bool                 RM6PreInit(ScrnInfoPtr, int);
extern Bool                 RM6ScreenInit(int, ScreenPtr, int, char **);
extern Bool                 RM6SwitchMode(int, DisplayModePtr, int);
#ifdef X_XF86MiscPassMessage
extern Bool                 RM6HandleMessage(int, const char*, const char*,
					       char**);
#endif
extern void                 RM6AdjustFrame(int, int, int, int);
extern Bool                 RM6EnterVT(int, int);
extern void                 RM6LeaveVT(int, int);
extern void                 RM6FreeScreen(int, int);
extern ModeStatus           RM6ValidMode(int, DisplayModePtr, Bool, int);

extern const OptionInfoRec *RM6OptionsWeak(void);

extern void                 RM6FillInScreenInfo(ScrnInfoPtr);

#endif /* _RM6_PROBE_H_ */
