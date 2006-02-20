/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6_probe.c,v 1.30 2003/10/07 22:47:12 martin Exp $ */
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
 *   Rickard E. Faith <faith@valinux.com>
 *
 * Modified by Marc Aurele La France <tsi@xfree86.org> for ATI driver merge.
 */

#include "atimodule.h"
#include "ativersion.h"

#include "rm6_probe.h"
#include "rm6_version.h"

#include "xf86PciInfo.h"

#include "xf86.h"
#include "xf86_ansic.h"
#define _XF86MISC_SERVER_
#include "xf86misc.h"
#include "xf86Resources.h"

#include "rm6_chipset.h"

PciChipsets RM6PciChipsets[] = {
#ifdef RM67
    { PCI_CHIP_RADEON_LW, PCI_CHIP_RADEON_LW, RES_SHARED_VGA },
#endif
    { PCI_CHIP_RADEON_LY, PCI_CHIP_RADEON_LY, RES_SHARED_VGA },

    { -1,                 -1,                 RES_UNDEFINED }
};

int gRM6EntityIndex = -1;

const int getRM6EntityIndex(void) { return gRM6EntityIndex; }

/* Return the options for supported chipset 'n'; NULL otherwise */
const OptionInfoRec *
RM6AvailableOptions(int chipid, int busid)
{
    int  i;

    /*
     * Return options defined in the radeon submodule which will have been
     * loaded by this point.
     */
    if ((chipid >> 16) == PCI_VENDOR_ATI)
	chipid -= PCI_VENDOR_ATI << 16;
    for (i = 0; RM6PciChipsets[i].PCIid > 0; i++) {
	if (chipid == RM6PciChipsets[i].PCIid)
	    return RM6OptionsWeak();
    }
    return NULL;
}

/* Return the string name for supported chipset 'n'; NULL otherwise. */
void
RM6Identify(int flags)
{
    xf86PrintChipsets(RM6_NAME,
		      "Driver for ATI Radeon Mobility M6 chipsets",
		      RM6Chipsets);
}

/* Return TRUE if chipset is present; FALSE otherwise. */
Bool
RM6Probe(DriverPtr drv, int flags)
{
    int      numUsed;
    int      numDevSections, nATIGDev, nRm6GDev;
    int     *usedChips;
    GDevPtr *devSections, *ATIGDevs, *Rm6GDevs;
    Bool     foundScreen = FALSE;
    int      i;

    if (!xf86GetPciVideoInfo()) return FALSE;

    /* Collect unclaimed device sections for both driver names */
    nATIGDev    = xf86MatchDevice(ATI_NAME, &ATIGDevs);
    nRm6GDev = xf86MatchDevice(RM6_NAME, &Rm6GDevs);

    if (!(numDevSections = nATIGDev + nRm6GDev)) return FALSE;

    if (!ATIGDevs) {
	if (!(devSections = Rm6GDevs)) numDevSections = 1;
	else                              numDevSections = nRm6GDev;
    } if (!Rm6GDevs) {
	devSections    = ATIGDevs;
	numDevSections = nATIGDev;
    } else {
	/* Combine into one list */
	devSections = xnfalloc((numDevSections + 1) * sizeof(GDevPtr));
	(void)memcpy(devSections,
		     ATIGDevs, nATIGDev * sizeof(GDevPtr));
	(void)memcpy(devSections + nATIGDev,
		     Rm6GDevs, nRm6GDev * sizeof(GDevPtr));
	devSections[numDevSections] = NULL;
	xfree(ATIGDevs);
	xfree(Rm6GDevs);
    }

    numUsed = xf86MatchPciInstances(RM6_NAME,
				    PCI_VENDOR_ATI,
				    RM6Chipsets,
				    RM6PciChipsets,
				    devSections,
				    numDevSections,
				    drv,
				    &usedChips);

    if (numUsed <= 0) return FALSE;

    if (flags & PROBE_DETECT) {
	foundScreen = TRUE;
    } else {
	for (i = 0; i < numUsed; i++) {
	    ScrnInfoPtr    pScrn = NULL;
	    EntityInfoPtr  pEnt;
	    pEnt = xf86GetEntityInfo(usedChips[i]);
	    if ((pScrn = xf86ConfigPciEntity(pScrn, 0, usedChips[i],
					     RM6PciChipsets, 0, 0, 0,
					     0, 0))) {
#ifdef XFree86LOADER
		if (!xf86LoadSubModule(pScrn, "rm6")) {
		    xf86Msg(X_ERROR, RM6_NAME
			    ":  Failed to load \"rm6\" module.\n");
		    xf86DeleteScreen(pScrn->scrnIndex, 0);
		    continue;
		}

		xf86LoaderReqSymLists(RM6Symbols, NULL);
#endif

		pScrn->Probe         = RM6Probe;
		RM6FillInScreenInfo(pScrn);
		foundScreen          = TRUE;
	    }

	    pEnt = xf86GetEntityInfo(usedChips[i]);

            /* create a RM6Entity for all chips, even with
               old single head Radeon, need to use pRM6Ent
               for new monitor detection routines
            */
            {
		DevUnion   *pPriv;
		RM6EntPtr pRM6Ent;

		xf86SetEntitySharable(usedChips[i]);

		if (gRM6EntityIndex == -1)
		    gRM6EntityIndex = xf86AllocateEntityPrivateIndex();

		pPriv = xf86GetEntityPrivate(pEnt->index,
					     gRM6EntityIndex);

		if (!pPriv->ptr) {
		    int j;
		    int instance = xf86GetNumEntityInstances(pEnt->index);

		    for (j = 0; j < instance; j++)
			xf86SetEntityInstanceForScreen(pScrn, pEnt->index, j);

		    pPriv->ptr = xnfcalloc(sizeof(RM6EntRec), 1);
		    pRM6Ent = pPriv->ptr;
		    pRM6Ent->HasSecondary = FALSE;
		} else {
		    pRM6Ent = pPriv->ptr;
		    pRM6Ent->HasSecondary = TRUE;
		}
	    }
	    xfree(pEnt);
	}
    }

    xfree(usedChips);
    xfree(devSections);

    return foundScreen;
}
