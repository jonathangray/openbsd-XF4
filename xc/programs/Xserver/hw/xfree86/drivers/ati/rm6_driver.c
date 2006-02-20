/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6_driver.c,v 1.117 2004/02/19 22:38:12 tsi Exp $ */
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
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Credits:
 *
 *   Thanks to Ani Joshi <ajoshi@shell.unixbox.com> for providing source
 *   code to his Radeon driver.  Portions of this file are based on the
 *   initialization code for that driver.
 *
 * References:
 *
 * !!!! FIXME !!!!
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 * This server does not yet support these XFree86 4.0 features:
 * !!!! FIXME !!!!
 *   DDC1 & DDC2
 *   shadowfb (Note: dri uses shadowfb for another purpose in radeon_dri.c)
 *   overlay planes
 *
 * Modified by Marc Aurele La France (tsi@xfree86.org) for ATI driver merge.
 *
 * Mergedfb and pseudo xinerama support added by Alex Deucher (agd5f@yahoo.com)
 * based on the sis driver by Thomas Winischhofer.
 *
 */

				/* Driver data structures */
#include "rm6.h"
#include "rm6_macros.h"
#include "rm6_probe.h"
#include "radeon_reg.h"
#include "rm6_version.h"
#include "rm6_mergedfb.h"

#include "fb.h"

				/* colormap initialization */
#include "micmap.h"
#include "dixstruct.h"

				/* X and server generic header files */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86PciInfo.h"
#include "xf86RAC.h"
#include "xf86Resources.h"
#include "xf86cmap.h"
#include "vbe.h"

#include "vgaHW.h"

#include "rm6_chipset.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif

				/* Forward definitions for driver functions */
static Bool RM6CloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool RM6SaveScreen(ScreenPtr pScreen, int mode);
static void RM6Save(ScrnInfoPtr pScrn);
static void RM6Restore(ScrnInfoPtr pScrn);
static Bool RM6ModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode);
static void RM6DisplayPowerManagementSet(ScrnInfoPtr pScrn,
					    int PowerManagementMode,
					    int flags);
static void RM6InitDispBandwidth(ScrnInfoPtr pScrn);

static void RM6GetMergedFBOptions(ScrnInfoPtr pScrn);
static int RM6ValidateMergeModes(ScrnInfoPtr pScrn);
static void RM6SetDynamicClock(ScrnInfoPtr pScrn, int mode);

/* psuedo xinerama support */

extern Bool 		RM6noPanoramiXExtension;

typedef enum {
    OPTION_NOACCEL,
    OPTION_SW_CURSOR,
    OPTION_DAC_6BIT,
    OPTION_DAC_8BIT,
    OPTION_PANEL_OFF,
    OPTION_DDC_MODE,
    OPTION_MONITOR_LAYOUT,
    OPTION_IGNORE_EDID,
    OPTION_VIDEO_KEY,
    OPTION_MERGEDFB,
    OPTION_CRT2HSYNC,
    OPTION_CRT2VREFRESH,
    OPTION_CRT2POS,
    OPTION_METAMODES,
    OPTION_MERGEDDPI,
    OPTION_NORADEONXINERAMA,
    OPTION_CRT2ISSCRN0,
    OPTION_DISP_PRIORITY,
    OPTION_PANEL_SIZE,
    OPTION_MIN_DOTCLOCK,
#ifdef RENDER
    OPTION_RENDER_ACCEL,
    OPTION_SUBPIXEL_ORDER,
#endif
    OPTION_SHOWCACHE,
    OPTION_DYNAMIC_CLOCKS,
#ifdef __powerpc__
    OPTION_IBOOKHACKS
#endif
} RADEONOpts;

static const OptionInfoRec RM6Options[] = {
    { OPTION_NOACCEL,        "NoAccel",          OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SW_CURSOR,      "SWcursor",         OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DAC_6BIT,       "Dac6Bit",          OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DAC_8BIT,       "Dac8Bit",          OPTV_BOOLEAN, {0}, TRUE  },
    { OPTION_PANEL_OFF,      "PanelOff",         OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DDC_MODE,       "DDCMode",          OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_MONITOR_LAYOUT, "MonitorLayout",    OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_IGNORE_EDID,    "IgnoreEDID",       OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_VIDEO_KEY,      "VideoKey",         OPTV_INTEGER, {0}, FALSE },
    { OPTION_MERGEDFB,	     "MergedFB",      	 OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_CRT2HSYNC,	     "CRT2HSync",        OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_CRT2VREFRESH,   "CRT2VRefresh",     OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_CRT2POS,        "CRT2Position",	 OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_METAMODES,      "MetaModes",        OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_MERGEDDPI,	     "MergedDPI", 	 OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_NORADEONXINERAMA, "NoMergedXinerama", OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_CRT2ISSCRN0,    "MergedXineramaCRT2IsScreen0", OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DISP_PRIORITY,  "DisplayPriority",  OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_PANEL_SIZE,     "PanelSize",        OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_MIN_DOTCLOCK,   "ForceMinDotClock", OPTV_FREQ,    {0}, FALSE },
#ifdef RENDER
    { OPTION_RENDER_ACCEL,   "RenderAccel",      OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SUBPIXEL_ORDER, "SubPixelOrder",    OPTV_ANYSTR,  {0}, FALSE },
#endif
    { OPTION_SHOWCACHE,      "ShowCache",        OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_DYNAMIC_CLOCKS, "DynamicClocks",    OPTV_BOOLEAN, {0}, FALSE },
#ifdef __powerpc__
    { OPTION_IBOOKHACKS,     "iBookHacks",       OPTV_BOOLEAN, {0}, FALSE },
#endif
    { -1,                    NULL,               OPTV_NONE,    {0}, FALSE }
};

const OptionInfoRec *RM6OptionsWeak(void) { return RM6Options; }

static const char *vgahwSymbols[] = {
    "vgaHWFreeHWRec",
    "vgaHWGetHWRec",
    "vgaHWGetIndex",
    "vgaHWLock",
    "vgaHWRestore",
    "vgaHWSave",
    "vgaHWUnlock",
    "vgaHWGetIOBase",
    NULL
};


static const char *ddcSymbols[] = {
    "xf86PrintEDID",
    "xf86DoEDID_DDC1",
    "xf86DoEDID_DDC2",
    NULL
};

static const char *fbSymbols[] = {
    "fbScreenInit",
    "fbPictureInit",
    NULL
};

static const char *xaaSymbols[] = {
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAInit",
    NULL
};

#if 0
static const char *xf8_32bppSymbols[] = {
    "xf86Overlay8Plus32Init",
    NULL
};
#endif

static const char *ramdacSymbols[] = {
    "xf86CreateCursorInfoRec",
    "xf86DestroyCursorInfoRec",
    "xf86ForceHWCursor",
    "xf86InitCursor",
    NULL
};


static const char *vbeSymbols[] = {
    "VBEInit",
    "vbeDoEDID",
    NULL
};

static const char *int10Symbols[] = {
    "xf86InitInt10",
    "xf86FreeInt10",
    "xf86int10Addr",
    "xf86ExecX86int10",
    NULL
};

static const char *i2cSymbols[] = {
    "xf86CreateI2CBusRec",
    "xf86I2CBusInit",
    NULL
};

void RM6LoaderRefSymLists(void)
{
    /*
     * Tell the loader about symbols from other modules that this module might
     * refer to.
     */
    xf86LoaderRefSymLists(vgahwSymbols,
			  fbSymbols,
			  xaaSymbols,
			  ramdacSymbols,
			  vbeSymbols,
			  int10Symbols,
			  i2cSymbols,
			  ddcSymbols,
			  NULL);
}

/* Established timings from EDID standard */
static struct
{
    int hsize;
    int vsize;
    int refresh;
} est_timings[] = {
    {1280, 1024, 75},
    {1024, 768, 75},
    {1024, 768, 70},
    {1024, 768, 60},
    {1024, 768, 87},
    {832, 624, 75},
    {800, 600, 75},
    {800, 600, 72},
    {800, 600, 60},
    {800, 600, 56},
    {640, 480, 75},
    {640, 480, 72},
    {640, 480, 67},
    {640, 480, 60},
    {720, 400, 88},
    {720, 400, 70},
};

static const RM6TMDSPll default_tmds_pll[CHIP_FAMILY_LAST][4] =
{
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}},				/*CHIP_FAMILY_UNKNOW*/
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}},				/*CHIP_FAMILY_LEGACY*/
    {{12000, 0xa1b}, {0xffffffff, 0xa3f}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RADEON*/
    {{12000, 0xa1b}, {0xffffffff, 0xa3f}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RV100*/
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}},				/*CHIP_FAMILY_RS100*/
    {{15000, 0xa1b}, {0xffffffff, 0xa3f}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RV200*/
    {{12000, 0xa1b}, {0xffffffff, 0xa3f}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RS200*/
    {{15000, 0xa1b}, {0xffffffff, 0xa3f}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_R200*/
    {{15500, 0x81b}, {0xffffffff, 0x83f}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RV250*/
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}},				/*CHIP_FAMILY_RS300*/
    {{13000, 0x400f4}, {15000, 0x400f7}, {0xffffffff, 0x400f7/*0x40111*/}, {0, 0}},	/*CHIP_FAMILY_RV280*/
    {{0xffffffff, 0xb01cb}, {0, 0}, {0, 0}, {0, 0}},		/*CHIP_FAMILY_R300*/
    {{0xffffffff, 0xb01cb}, {0, 0}, {0, 0}, {0, 0}},		/*CHIP_FAMILY_R350*/
    {{15000, 0xb0155}, {0xffffffff, 0xb01cb}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RV350*/
    {{15000, 0xb0155}, {0xffffffff, 0xb01cb}, {0, 0}, {0, 0}},	/*CHIP_FAMILY_RV380*/
    {{0xffffffff, 0xb01cb}, {0, 0}, {0, 0}, {0, 0}},		/*CHIP_FAMILY_R420*/
};

extern int getRM6EntityIndex(void);

struct RM6Int10Save {
	CARD32 MEM_CNTL;
	CARD32 MEMSIZE;
	CARD32 MPP_TB_CONFIG;
};

static Bool RM6MapMMIO(ScrnInfoPtr pScrn);
static Bool RM6UnmapMMIO(ScrnInfoPtr pScrn);

RM6EntPtr RM6EntPriv(ScrnInfoPtr pScrn)
{
    DevUnion     *pPriv;
    RM6InfoPtr  info   = RM6PTR(pScrn);
    pPriv = xf86GetEntityPrivate(info->pEnt->index,
                                 getRM6EntityIndex());
    return pPriv->ptr;
}

static void
RM6PreInt10Save(ScrnInfoPtr pScrn, void **pPtr)
{
    RM6InfoPtr  info   = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32 CardTmp;
    static struct RM6Int10Save SaveStruct = { 0, 0, 0 };

    /* Save the values and zap MEM_CNTL */
    SaveStruct.MEM_CNTL = INREG(RADEON_MEM_CNTL);
    SaveStruct.MEMSIZE = INREG(RADEON_CONFIG_MEMSIZE);
    SaveStruct.MPP_TB_CONFIG = INREG(RADEON_MPP_TB_CONFIG);

    /*
     * Zap MEM_CNTL and set MPP_TB_CONFIG<31:24> to 4
     */
    OUTREG(RADEON_MEM_CNTL, 0);
    CardTmp = SaveStruct.MPP_TB_CONFIG & 0x00ffffffu;
    CardTmp |= 0x04 << 24;
    OUTREG(RADEON_MPP_TB_CONFIG, CardTmp);

    *pPtr = (void *)&SaveStruct;
}

static void
RM6PostInt10Check(ScrnInfoPtr pScrn, void *ptr)
{
    RM6InfoPtr  info   = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    struct RM6Int10Save *pSave = ptr;
    CARD32 CardTmp;

    /* If we don't have a valid (non-zero) saved MEM_CNTL, get out now */
    if (!pSave || !pSave->MEM_CNTL)
	return;

    /*
     * If either MEM_CNTL is currently zero or inconistent (configured for
     * two channels with the two channels configured differently), restore
     * the saved registers.
     */
    CardTmp = INREG(RADEON_MEM_CNTL);
    if (!CardTmp ||
	((CardTmp & 1) &&
	 (((CardTmp >> 8) & 0xff) != ((CardTmp >> 24) & 0xff)))) {
	/* Restore the saved registers */
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Restoring MEM_CNTL (%08lx), setting to %08lx\n",
		   (unsigned long)CardTmp, (unsigned long)pSave->MEM_CNTL);
	OUTREG(RADEON_MEM_CNTL, pSave->MEM_CNTL);

	CardTmp = INREG(RADEON_CONFIG_MEMSIZE);
	if (CardTmp != pSave->MEMSIZE) {
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Restoring CONFIG_MEMSIZE (%08lx), setting to %08lx\n",
		       (unsigned long)CardTmp, (unsigned long)pSave->MEMSIZE);
	    OUTREG(RADEON_CONFIG_MEMSIZE, pSave->MEMSIZE);
	}
    }

    CardTmp = INREG(RADEON_MPP_TB_CONFIG);
    if ((CardTmp & 0xff000000u) != (pSave->MPP_TB_CONFIG & 0xff000000u)) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	           "Restoring MPP_TB_CONFIG<31:24> (%02lx), setting to %02lx\n",
	 	   (unsigned long)CardTmp >> 24,
		   (unsigned long)pSave->MPP_TB_CONFIG >> 24);
	CardTmp &= 0x00ffffffu;
	CardTmp |= (pSave->MPP_TB_CONFIG & 0xff000000u);
	OUTREG(RADEON_MPP_TB_CONFIG, CardTmp);
    }
}

/* Allocate our private RM6InfoRec */
static Bool RM6GetRec(ScrnInfoPtr pScrn)
{
    if (pScrn->driverPrivate) return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(RM6InfoRec), 1);
    return TRUE;
}

/* Free our private RM6InfoRec */
static void RM6FreeRec(ScrnInfoPtr pScrn)
{
    RM6InfoPtr info = RM6PTR(pScrn);
    if(info->CRT2HSync) xfree(info->CRT2HSync);
    info->CRT2HSync = NULL;
    if(info->CRT2VRefresh) xfree(info->CRT2VRefresh);
    info->CRT2VRefresh = NULL;
    if(info->MetaModes) xfree(info->MetaModes);
    info->MetaModes = NULL;
    if(info->CRT2pScrn) {
       if(info->CRT2pScrn->modes) {
          while(info->CRT2pScrn->modes)
             xf86DeleteMode(&info->CRT2pScrn->modes, info->CRT2pScrn->modes);
       }
       if(info->CRT2pScrn->monitor) {
          if(info->CRT2pScrn->monitor->Modes) {
	     while(info->CRT2pScrn->monitor->Modes)
	        xf86DeleteMode(&info->CRT2pScrn->monitor->Modes, info->CRT2pScrn->monitor->Modes);
	  }
	  if(info->CRT2pScrn->monitor->DDC) xfree(info->CRT2pScrn->monitor->DDC);
          xfree(info->CRT2pScrn->monitor);
       }
       xfree(info->CRT2pScrn);
       info->CRT2pScrn = NULL;
    }
    if(info->CRT1Modes) {
       if(info->CRT1Modes != pScrn->modes) {
          if(pScrn->modes) {
             pScrn->currentMode = pScrn->modes;
             do {
                DisplayModePtr p = pScrn->currentMode->next;
                if(pScrn->currentMode->Private)
                   xfree(pScrn->currentMode->Private);
                xfree(pScrn->currentMode);
                pScrn->currentMode = p;
             } while(pScrn->currentMode != pScrn->modes);
          }
          pScrn->currentMode = info->CRT1CurrentMode;
          pScrn->modes = info->CRT1Modes;
          info->CRT1CurrentMode = NULL;
          info->CRT1Modes = NULL;
       }
    }

    if (!pScrn || !pScrn->driverPrivate) return;
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
}

/* Memory map the MMIO region.  Used during pre-init and by RM6MapMem,
 * below
 */
static Bool RM6MapMMIO(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    info->MMIO = xf86MapPciMem(pScrn->scrnIndex,
	VIDMEM_MMIO | VIDMEM_READSIDEEFFECT,
	info->PciTag,
	info->MMIOAddr,
	RADEON_MMIOSIZE);

    if (!info->MMIO) return FALSE;
    return TRUE;
}

/* Unmap the MMIO region.  Used during pre-init and by RM6UnmapMem,
 * below
 */
static Bool RM6UnmapMMIO(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    xf86UnMapVidMem(pScrn->scrnIndex, info->MMIO, RADEON_MMIOSIZE);
    info->MMIO = NULL;
    return TRUE;
}

/* Memory map the frame buffer.  Used by RM6MapMem, below. */
static Bool RM6MapFB(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    info->FB = xf86MapPciMem(pScrn->scrnIndex,
				 VIDMEM_FRAMEBUFFER,
				 info->PciTag,
				 info->LinearAddr,
				 info->FbMapSize);

    if (!info->FB) return FALSE;
    return TRUE;
}

/* Unmap the frame buffer.  Used by RM6UnmapMem, below. */
static Bool RM6UnmapFB(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    xf86UnMapVidMem(pScrn->scrnIndex, info->FB, info->FbMapSize);
    info->FB = NULL;
    return TRUE;
}

/* Memory map the MMIO region and the frame buffer */
static Bool RM6MapMem(ScrnInfoPtr pScrn)
{
    if (!RM6MapMMIO(pScrn)) return FALSE;
    if (!RM6MapFB(pScrn)) {
	RM6UnmapMMIO(pScrn);
	return FALSE;
    }
    return TRUE;
}

/* Unmap the MMIO region and the frame buffer */
static Bool RM6UnmapMem(ScrnInfoPtr pScrn)
{
    if (!RM6UnmapMMIO(pScrn) || !RM6UnmapFB(pScrn)) return FALSE;
    return TRUE;
}

/* This function is required to workaround a hardware bug in some (all?)
 * revisions of the R300.  This workaround should be called after every
 * CLOCK_CNTL_INDEX register access.  If not, register reads afterward
 * may not be correct.
 */
void R300CGWorkaround(ScrnInfoPtr pScrn) {
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32         save, tmp;

    save = INREG(RADEON_CLOCK_CNTL_INDEX);
    tmp = save & ~(0x3f | RADEON_PLL_WR_EN);
    OUTREG(RADEON_CLOCK_CNTL_INDEX, tmp);
    tmp = INREG(RADEON_CLOCK_CNTL_DATA);
    OUTREG(RADEON_CLOCK_CNTL_INDEX, save);
}

/* Read PLL information */
unsigned RM6INPLL(ScrnInfoPtr pScrn, int addr)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32         data;

    OUTREG8(RADEON_CLOCK_CNTL_INDEX, addr & 0x3f);
    data = INREG(RADEON_CLOCK_CNTL_DATA);
    if (info->R300CGWorkaround) R300CGWorkaround(pScrn);

    return data;
}


/* Wait for vertical sync on primary CRTC */
void RM6WaitForVerticalSync(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            i;

    /* Clear the CRTC_VBLANK_SAVE bit */
    OUTREG(RADEON_CRTC_STATUS, RADEON_CRTC_VBLANK_SAVE_CLEAR);

    /* Wait for it to go back up */
    for (i = 0; i < RADEON_TIMEOUT/1000; i++) {
	if (INREG(RADEON_CRTC_STATUS) & RADEON_CRTC_VBLANK_SAVE) break;
	usleep(1);
    }
}

/* Wait for vertical sync on secondary CRTC */
void RM6WaitForVerticalSync2(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            i;

    /* Clear the CRTC2_VBLANK_SAVE bit */
    OUTREG(RADEON_CRTC2_STATUS, RADEON_CRTC2_VBLANK_SAVE_CLEAR);

    /* Wait for it to go back up */
    for (i = 0; i < RADEON_TIMEOUT/1000; i++) {
	if (INREG(RADEON_CRTC2_STATUS) & RADEON_CRTC2_VBLANK_SAVE) break;
	usleep(1);
    }
}

/* Blank screen */
static void RM6Blank(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    if (!info->IsSecondary) {
	switch(info->DisplayType) {
	case MT_LCD:
	case MT_CRT:
	case MT_DFP:
	    OUTREGP(RADEON_CRTC_EXT_CNTL,
		    RADEON_CRTC_DISPLAY_DIS,
		    ~(RADEON_CRTC_DISPLAY_DIS));
	    break;

	case MT_NONE:
	default:
	    break;
	}
	if (info->MergedFB)
	    OUTREGP(RADEON_CRTC2_GEN_CNTL,
		    RADEON_CRTC2_DISP_DIS,
		    ~(RADEON_CRTC2_DISP_DIS));
    } else {
	OUTREGP(RADEON_CRTC2_GEN_CNTL,
		RADEON_CRTC2_DISP_DIS,
		~(RADEON_CRTC2_DISP_DIS));
    }
}

/* Unblank screen */
static void RM6Unblank(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    if (!info->IsSecondary) {
	switch (info->DisplayType) {
	case MT_LCD:
	case MT_CRT:
	case MT_DFP:
	    OUTREGP(RADEON_CRTC_EXT_CNTL,
		    RADEON_CRTC_CRT_ON,
		    ~(RADEON_CRTC_DISPLAY_DIS));
	    break;

	case MT_NONE:
	default:
	    break;
	}
	if (info->MergedFB)
	    OUTREGP(RADEON_CRTC2_GEN_CNTL,
		    0,
		    ~(RADEON_CRTC2_DISP_DIS));
    } else {
	switch (info->DisplayType) {
	case MT_LCD:
	case MT_DFP:
	case MT_CRT:
	    OUTREGP(RADEON_CRTC2_GEN_CNTL,
		    0,
		    ~(RADEON_CRTC2_DISP_DIS));
	    break;

	case MT_NONE:
	default:
	    break;
	}
    }
}

/* Compute log base 2 of val */
int RM6MinBits(int val)
{
    int  bits;

    if (!val) return 1;
    for (bits = 0; val; val >>= 1, ++bits);
    return bits;
}

/* Compute n/d with rounding */
static int RM6Div(int n, int d)
{
    return (n + (d / 2)) / d;
}

static RADEONMonitorType RM6DisplayDDCConnected(ScrnInfoPtr pScrn, RADEONDDCType DDCType, RADEONConnector* port)
{
    RM6InfoPtr info = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    unsigned long DDCReg;
    RADEONMonitorType MonType = MT_NONE;
    xf86MonPtr* MonInfo = &port->MonInfo;
    int i, j;

    DDCReg = info->DDCReg;
    switch(DDCType)
    {
    case DDC_MONID:
	info->DDCReg = RADEON_GPIO_MONID;
	break;
    case DDC_DVI:
	info->DDCReg = RADEON_GPIO_DVI_DDC;
	break;
    case DDC_VGA:
	info->DDCReg = RADEON_GPIO_VGA_DDC;
	break;
    case DDC_CRT2:
	info->DDCReg = RADEON_GPIO_CRT2_DDC;
	break;
    default:
	info->DDCReg = DDCReg;
	return MT_NONE;
    }

    /* Read and output monitor info using DDC2 over I2C bus */
    if (info->pI2CBus && info->ddc2) {
	OUTREG(info->DDCReg, INREG(info->DDCReg) &
	       (CARD32)~(RADEON_GPIO_A_0 | RADEON_GPIO_A_1));

	/* For some old monitors (like Compaq Presario FP500), we need
	 * following process to initialize/stop DDC
	 */
	OUTREG(info->DDCReg, INREG(info->DDCReg) & ~(RADEON_GPIO_EN_1));
	for (j = 0; j < 3; j++) {
	    OUTREG(info->DDCReg,
		   INREG(info->DDCReg) & ~(RADEON_GPIO_EN_0));
	    usleep(13000);

	    OUTREG(info->DDCReg,
		   INREG(info->DDCReg) & ~(RADEON_GPIO_EN_1));
	    for (i = 0; i < 10; i++) {
		usleep(15000);
		if (INREG(info->DDCReg) & RADEON_GPIO_Y_1)
		    break;
	    }
	    if (i == 10) continue;

	    usleep(15000);

	    OUTREG(info->DDCReg, INREG(info->DDCReg) | RADEON_GPIO_EN_0);
	    usleep(15000);

	    OUTREG(info->DDCReg, INREG(info->DDCReg) | RADEON_GPIO_EN_1);
	    usleep(15000);
	    OUTREG(info->DDCReg,
		   INREG(info->DDCReg) & ~(RADEON_GPIO_EN_0));
	    usleep(15000);
	    *MonInfo = xf86DoEDID_DDC2(pScrn->scrnIndex, info->pI2CBus);

	    OUTREG(info->DDCReg, INREG(info->DDCReg) | RADEON_GPIO_EN_1);
	    OUTREG(info->DDCReg, INREG(info->DDCReg) | RADEON_GPIO_EN_0);
	    usleep(15000);
	    OUTREG(info->DDCReg,
		   INREG(info->DDCReg) & ~(RADEON_GPIO_EN_1));
	    for (i = 0; i < 5; i++) {
		usleep(15000);
		if (INREG(info->DDCReg) & RADEON_GPIO_Y_1)
		    break;
	    }
	    usleep(15000);
	    OUTREG(info->DDCReg,
		   INREG(info->DDCReg) & ~(RADEON_GPIO_EN_0));
	    usleep(15000);

	    OUTREG(info->DDCReg, INREG(info->DDCReg) | RADEON_GPIO_EN_1);
	    OUTREG(info->DDCReg, INREG(info->DDCReg) | RADEON_GPIO_EN_0);
	    usleep(15000);
	    if(*MonInfo) break;
	}
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "DDC2/I2C is not properly initialized\n");
	MonType = MT_NONE;
    }

    if (*MonInfo) {
	if ((*MonInfo)->rawData[0x14] & 0x80) {
	    /* Note some laptops have a DVI output that uses internal TMDS,
	     * when its DVI is enabled by hotkey, LVDS panel is not used.
	     * In this case, the laptop is configured as DVI+VGA as a normal 
	     * desktop card.
	     * Also for laptop, when X starts with lid closed (no DVI connection)
	     * both LDVS and TMDS are disable, we still need to treat it as a LVDS panel.
	     */
	    if (port->TMDSType == TMDS_EXT) MonType = MT_DFP;
	    else {
		if ((INREG(RADEON_FP_GEN_CNTL) & (1<<7)) || !info->IsMobility)
		    MonType = MT_DFP;
		else 
		    MonType = MT_LCD;
	    }
	} else MonType = MT_CRT;
    } else MonType = MT_NONE;

    info->DDCReg = DDCReg;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "DDC Type: %d, Detected Type: %d\n", DDCType, MonType);

    return MonType;
}

static RADEONMonitorType
RM6CrtIsPhysicallyConnected(ScrnInfoPtr pScrn, int IsCrtDac)
{
    RM6InfoPtr info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int		  bConnected = 0;

    /* the monitor either wasn't connected or it is a non-DDC CRT.
     * try to probe it
     */
    if(IsCrtDac) {
	unsigned long ulOrigVCLK_ECP_CNTL;
	unsigned long ulOrigDAC_CNTL;
	unsigned long ulOrigDAC_EXT_CNTL;
	unsigned long ulOrigCRTC_EXT_CNTL;
	unsigned long ulData;
	unsigned long ulMask;

	ulOrigVCLK_ECP_CNTL = INPLL(pScrn, RADEON_VCLK_ECP_CNTL);

	ulData              = ulOrigVCLK_ECP_CNTL;
	ulData             &= ~(RADEON_PIXCLK_ALWAYS_ONb
				| RADEON_PIXCLK_DAC_ALWAYS_ONb);
	ulMask              = ~(RADEON_PIXCLK_ALWAYS_ONb
				|RADEON_PIXCLK_DAC_ALWAYS_ONb);
	OUTPLLP(pScrn, RADEON_VCLK_ECP_CNTL, ulData, ulMask);

	ulOrigCRTC_EXT_CNTL = INREG(RADEON_CRTC_EXT_CNTL);
	ulData              = ulOrigCRTC_EXT_CNTL;
	ulData             |= RADEON_CRTC_CRT_ON;
	OUTREG(RADEON_CRTC_EXT_CNTL, ulData);

	ulOrigDAC_EXT_CNTL = INREG(RADEON_DAC_EXT_CNTL);
	ulData             = ulOrigDAC_EXT_CNTL;
	ulData            &= ~RADEON_DAC_FORCE_DATA_MASK;
	ulData            |=  (RADEON_DAC_FORCE_BLANK_OFF_EN
			       |RADEON_DAC_FORCE_DATA_EN
			       |RADEON_DAC_FORCE_DATA_SEL_MASK);
	if ((info->ChipFamily == CHIP_FAMILY_RV250) ||
	    (info->ChipFamily == CHIP_FAMILY_RV280))
	    ulData |= (0x01b6 << RADEON_DAC_FORCE_DATA_SHIFT);
	else
	    ulData |= (0x01ac << RADEON_DAC_FORCE_DATA_SHIFT);

	OUTREG(RADEON_DAC_EXT_CNTL, ulData);

	ulOrigDAC_CNTL     = INREG(RADEON_DAC_CNTL);
	ulData             = ulOrigDAC_CNTL;
	ulData            |= RADEON_DAC_CMP_EN;
	ulData            &= ~(RADEON_DAC_RANGE_CNTL_MASK
			       | RADEON_DAC_PDWN);
	ulData            |= 0x2;
	OUTREG(RADEON_DAC_CNTL, ulData);

	usleep(10000);

	ulData     = INREG(RADEON_DAC_CNTL);
	bConnected =  (RADEON_DAC_CMP_OUTPUT & ulData)?1:0;

	ulData    = ulOrigVCLK_ECP_CNTL;
	ulMask    = 0xFFFFFFFFL;
	OUTPLLP(pScrn, RADEON_VCLK_ECP_CNTL, ulData, ulMask);

	OUTREG(RADEON_DAC_CNTL,      ulOrigDAC_CNTL     );
	OUTREG(RADEON_DAC_EXT_CNTL,  ulOrigDAC_EXT_CNTL );
	OUTREG(RADEON_CRTC_EXT_CNTL, ulOrigCRTC_EXT_CNTL);
    } else { /* TV DAC */

        /* This doesn't seem to work reliably (maybe worse on some OEM cards),
           for now we always return false. If one wants to connected a
           non-DDC monitor on the DVI port when CRT port is also connected,
           he will need to explicitly tell the driver in the config file
           with Option MonitorLayout.
        */
        bConnected = FALSE;

    }

    return(bConnected ? MT_CRT : MT_NONE);
}

#if defined(__powerpc__)
static Bool RM6ProbePLLParameters(ScrnInfoPtr pScrn)
{
    RM6InfoPtr info = RM6PTR(pScrn);
    RM6PLLPtr  pll  = &info->pll;
    unsigned char *RM6MMIO = info->MMIO;
    unsigned char ppll_div_sel;
    unsigned Nx, M;
    unsigned xclk, tmp, ref_div;
    int hTotal, vTotal, num, denom, m, n;
    float hz, vclk, xtal;
    long start_secs, start_usecs, stop_secs, stop_usecs, total_usecs;
    int i;

    for(i=0; i<1000000; i++)
	if (((INREG(RADEON_CRTC_VLINE_CRNT_VLINE) >> 16) & 0x3ff) == 0)
	    break;

    xf86getsecs(&start_secs, &start_usecs);

    for(i=0; i<1000000; i++)
	if (((INREG(RADEON_CRTC_VLINE_CRNT_VLINE) >> 16) & 0x3ff) != 0)
	    break;

    for(i=0; i<1000000; i++)
	if (((INREG(RADEON_CRTC_VLINE_CRNT_VLINE) >> 16) & 0x3ff) == 0)
	    break;

    xf86getsecs(&stop_secs, &stop_usecs);

    total_usecs = abs(stop_usecs - start_usecs);
    hz = 1000000/total_usecs;

    hTotal = ((INREG(RADEON_CRTC_H_TOTAL_DISP) & 0x1ff) + 1) * 8;
    vTotal = ((INREG(RADEON_CRTC_V_TOTAL_DISP) & 0x3ff) + 1);
    vclk = (float)(hTotal * (float)(vTotal * hz));

    switch((INPLL(pScrn, RADEON_PPLL_REF_DIV) & 0x30000) >> 16) {
    case 0:
    default:
        num = 1;
        denom = 1;
        break;
    case 1:
        n = ((INPLL(pScrn, RADEON_X_MPLL_REF_FB_DIV) >> 16) & 0xff);
        m = (INPLL(pScrn, RADEON_X_MPLL_REF_FB_DIV) & 0xff);
        num = 2*n;
        denom = 2*m;
        break;
    case 2:
        n = ((INPLL(pScrn, RADEON_X_MPLL_REF_FB_DIV) >> 8) & 0xff);
        m = (INPLL(pScrn, RADEON_X_MPLL_REF_FB_DIV) & 0xff);
        num = 2*n;
        denom = 2*m;
        break;
     }

    OUTREG(RADEON_CLOCK_CNTL_INDEX, 1);
    ppll_div_sel = INREG8(RADEON_CLOCK_CNTL_DATA + 1) & 0x3;

    n = (INPLL(pScrn, RADEON_PPLL_DIV_0 + ppll_div_sel) & 0x7ff);
    m = (INPLL(pScrn, RADEON_PPLL_REF_DIV) & 0x3ff);

    num *= n;
    denom *= m;

    switch ((INPLL(pScrn, RADEON_PPLL_DIV_0 + ppll_div_sel) >> 16) & 0x7) {
    case 1:
        denom *= 2;
        break;
    case 2:
        denom *= 4;
        break;
    case 3:
        denom *= 8;
        break;
    case 4:
        denom *= 3;
        break;
    case 6:
        denom *= 6;
        break;
    case 7:
        denom *= 12;
        break;
    }

    xtal = (int)(vclk *(float)denom/(float)num);

    if ((xtal > 26900000) && (xtal < 27100000))
        xtal = 2700;
    else if ((xtal > 14200000) && (xtal < 14400000))
        xtal = 1432;
    else if ((xtal > 29400000) && (xtal < 29600000))
        xtal = 2950;
    else
	return FALSE;

    tmp = INPLL(pScrn, RADEON_X_MPLL_REF_FB_DIV);
    ref_div = INPLL(pScrn, RADEON_PPLL_REF_DIV) & 0x3ff;

    Nx = (tmp & 0xff00) >> 8;
    M = (tmp & 0xff);
    xclk = RM6Div((2 * Nx * xtal), (2 * M));

    /* we're done, hopefully these are sane values */
    pll->reference_div = ref_div;
    pll->xclk = xclk;
    pll->reference_freq = xtal;

    return TRUE;
}
#endif

static void RM6GetPanelInfoFromReg (ScrnInfoPtr pScrn)
{
    RM6InfoPtr info     = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32 fp_vert_stretch = INREG(RADEON_FP_VERT_STRETCH);
    CARD32 fp_horz_stretch = INREG(RADEON_FP_HORZ_STRETCH);

    info->PanelPwrDly = 200;
    if (fp_vert_stretch & RADEON_VERT_STRETCH_ENABLE) {
	info->PanelYRes = (fp_vert_stretch>>12) + 1;
    } else {
	info->PanelYRes = (INREG(RADEON_CRTC_V_TOTAL_DISP)>>16) + 1;
    }
    if (fp_horz_stretch & RADEON_HORZ_STRETCH_ENABLE) {
	info->PanelXRes = ((fp_horz_stretch>>16) + 1) * 8;
    } else {
	info->PanelXRes = ((INREG(RADEON_CRTC_H_TOTAL_DISP)>>16) + 1) * 8;
    }
    
    if ((info->PanelXRes < 640) || (info->PanelYRes < 480)) {
	info->PanelXRes = 640;
	info->PanelYRes = 480;
    }
    
    xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
	       "Panel size %dx%d is derived, this may not be correct.\n"
		   "If not, use PanelSize option to overwrite this setting\n",
	       info->PanelXRes, info->PanelYRes);
}

static Bool RM6GetLVDSInfo (ScrnInfoPtr pScrn)
{
    RM6InfoPtr info     = RM6PTR(pScrn);

    if (!RM6GetLVDSInfoFromBIOS(pScrn))
        RM6GetPanelInfoFromReg(pScrn);

    if (info->DotClock == 0) {
        RM6EntPtr pRM6Ent   = RM6EntPriv(pScrn);
        DisplayModePtr  tmp_mode = NULL;
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                   "No valid timing info from BIOS.\n");
        /* No timing information for the native mode,
           use whatever specified in the Modeline.
           If no Modeline specified, we'll just pick
           the VESA mode at 60Hz refresh rate which
           is likely to be the best for a flat panel.
	*/
        tmp_mode = pScrn->monitor->Modes;
        while(tmp_mode) {
            if ((tmp_mode->HDisplay == info->PanelXRes) &&
                (tmp_mode->VDisplay == info->PanelYRes)) {

                float  refresh =
                    (float)tmp_mode->Clock * 1000.0 / tmp_mode->HTotal / tmp_mode->VTotal;
                if ((abs(60.0 - refresh) < 1.0) ||
                    (tmp_mode->type == 0)) {
                    info->HBlank     = tmp_mode->HTotal - tmp_mode->HDisplay;
                    info->HOverPlus  = tmp_mode->HSyncStart - tmp_mode->HDisplay;
                    info->HSyncWidth = tmp_mode->HSyncEnd - tmp_mode->HSyncStart;
                    info->VBlank     = tmp_mode->VTotal - tmp_mode->VDisplay;
                    info->VOverPlus  = tmp_mode->VSyncStart - tmp_mode->VDisplay;
                    info->VSyncWidth = tmp_mode->VSyncEnd - tmp_mode->VSyncStart;
                    info->DotClock   = tmp_mode->Clock;
                    info->Flags = 0;
                    break;
                }
            }
            tmp_mode = tmp_mode->next;
        }
        if ((info->DotClock == 0) && !pRM6Ent->PortInfo[0].MonInfo) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Panel size is not correctly detected.\n"
                       "Please try to use PanelSize option for correct settings.\n");
            return FALSE;
        }
    }

    return TRUE;
}

static void RM6GetTMDSInfo(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    int i;

    for (i=0; i<4; i++) {
        info->tmds_pll[i].value = 0;
        info->tmds_pll[i].freq = 0;
    }

    if (RM6GetTMDSInfoFromBIOS(pScrn)) return;

    for (i=0; i<4; i++) {
        info->tmds_pll[i].value = default_tmds_pll[info->ChipFamily][i].value;
        info->tmds_pll[i].freq = default_tmds_pll[info->ChipFamily][i].freq;
    }
}

static void RM6GetPanelInfo (ScrnInfoPtr pScrn)
{
    RM6InfoPtr info     = RM6PTR(pScrn);
    char* s;

    if((s = xf86GetOptValString(info->Options, OPTION_PANEL_SIZE))) {
        info->PanelPwrDly = 200;
        if (sscanf (s, "%dx%d", &info->PanelXRes, &info->PanelYRes) != 2) {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "Invalid PanelSize option: %s\n", s);
            RM6GetPanelInfoFromReg(pScrn);
        }
    } else {

        if(info->DisplayType == MT_LCD) {
            RM6GetLVDSInfo(pScrn);
        } else if ((info->DisplayType == MT_DFP) || (info->MergeType == MT_DFP)) {
            RM6GetTMDSInfo(pScrn);
            if (!pScrn->monitor->DDC)
                RM6GetHardCodedEDIDFromBIOS(pScrn);
        }
    }
}

static void RM6GetClockInfo(ScrnInfoPtr pScrn)
{
    RM6InfoPtr info = RM6PTR (pScrn);
    RM6PLLPtr pll = &info->pll;
    double min_dotclock;

    if (RM6GetClockInfoFromBIOS(pScrn)) {
	if (pll->reference_div < 2) {
	    /* retrive it from register setting for fitting into current PLL algorithm.
	       We'll probably need a new routine to calculate the best ref_div from BIOS 
	       provided min_input_pll and max_input_pll 
	    */
	    CARD32 tmp;
	    tmp = INPLL(pScrn, RADEON_PPLL_REF_DIV);
	    if (IS_R300_VARIANT ||
		(info->ChipFamily == CHIP_FAMILY_RS300)) {
		pll->reference_div = (tmp & R300_PPLL_REF_DIV_ACC_MASK) >> R300_PPLL_REF_DIV_ACC_SHIFT;
	    } else {
		pll->reference_div = tmp & RADEON_PPLL_REF_DIV_MASK;
	    }

	    if (pll->reference_div < 2) pll->reference_div = 12;
	}
	
    } else {
	xf86DrvMsg (pScrn->scrnIndex, X_WARNING,
		    "Video BIOS not detected, using default clock settings!\n");

#if defined(__powerpc__)
	if (RM6ProbePLLParameters(pScrn)) return;
#endif
	if (info->IsIGP)
	    pll->reference_freq = 1432;
	else
	    pll->reference_freq = 2700;

	pll->reference_div = 12;
	pll->min_pll_freq = 12500;
	pll->max_pll_freq = 35000;
	pll->xclk = 10300;

        info->sclk = 200.00;
        info->mclk = 200.00;
    }

    xf86DrvMsg (pScrn->scrnIndex, X_INFO,
		"PLL parameters: rf=%d rd=%d min=%ld max=%ld; xclk=%d\n",
		pll->reference_freq,
		pll->reference_div,
		pll->min_pll_freq, pll->max_pll_freq, pll->xclk);

    /* (Some?) Radeon BIOSes seem too lie about their minimum dot
     * clocks.  Allow users to override the detected minimum dot clock
     * value (e.g., and allow it to be suitable for TV sets).
     */
    if (xf86GetOptValFreq(info->Options, OPTION_MIN_DOTCLOCK,
			  OPTUNITS_MHZ, &min_dotclock)) {
	if (min_dotclock < 12 || min_dotclock*100 >= pll->max_pll_freq) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Illegal minimum dotclock specified %.2f MHz "
		       "(option ignored)\n",
		       min_dotclock);
	} else {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Forced minimum dotclock to %.2f MHz "
		       "(instead of detected %.2f MHz)\n",
		       min_dotclock, ((double)pll->min_pll_freq/1000));
	    pll->min_pll_freq = min_dotclock * 1000;
	}
    }
}

static BOOL RM6QueryConnectedMonitors(ScrnInfoPtr pScrn)
{
    RM6InfoPtr info       = RM6PTR(pScrn);
    RM6EntPtr pRM6Ent  = RM6EntPriv(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    const char *s;
    Bool ignore_edid = FALSE;
    int i = 0, second = 0, max_mt;

    const char *MonTypeName[7] =
    {
	"AUTO",
	"NONE",
	"CRT",
	"LVDS",
	"TMDS",
	"CTV",
	"STV"
    };

    const RADEONMonitorType MonTypeID[7] =
    {
	MT_UNKNOWN, /* this is just a dummy value for AUTO DETECTION */
	MT_NONE,    /* NONE -> NONE */
	MT_CRT,     /* CRT -> CRT */
	MT_LCD,     /* Laptop LCDs are driven via LVDS port */
	MT_DFP,     /* DFPs are driven via TMDS */
	MT_CTV,     /* CTV -> CTV */
	MT_STV,     /* STV -> STV */
    };

    const char *TMDSTypeName[3] =
    {
	"NONE",
	"Internal",
    "External"
    };

    const char *DDCTypeName[5] =
    {
	"NONE",
	"MONID",
	"DVI_DDC",
	"VGA_DDC",
    "CRT2_DDC"
    };

    const char *DACTypeName[3] =
    {
	"Unknown",
	"Primary",
	"TVDAC/ExtDAC",
    };

    const char *ConnectorTypeName[8] =
    {
	"None",
	"Proprietary",
	"VGA",
	"DVI-I",
	"DVI-D",
	"CTV",
	"STV",
	"Unsupported"
    };

    const char *ConnectorTypeNameATOM[10] =
    {
	"None",
	"VGA",
	"DVI-I",
	"DVI-D",
	"DVI-A",
	"STV",
	"CTV",
	"LVDS",
	"Digital",
	"Unsupported"
    };

    max_mt = 5;

    if(info->IsSecondary) {
	info->DisplayType = (RADEONMonitorType)pRM6Ent->MonType2;
	if(info->DisplayType == MT_NONE) return FALSE;
	return TRUE;
    }


    /* We first get the information about all connectors from BIOS.
     * This is how the card is phyiscally wired up.
     * The information should be correct even on a OEM card.
     * If not, we may have problem -- need to use MonitorLayout option.
     */
    for (i = 0; i < 2; i++) {
	pRM6Ent->PortInfo[i].MonType = MT_UNKNOWN;
	pRM6Ent->PortInfo[i].MonInfo = NULL;
	pRM6Ent->PortInfo[i].DDCType = DDC_NONE_DETECTED;
	pRM6Ent->PortInfo[i].DACType = DAC_UNKNOWN;
	pRM6Ent->PortInfo[i].TMDSType = TMDS_UNKNOWN;
	pRM6Ent->PortInfo[i].ConnectorType = CONNECTOR_NONE;
    }

    if (!RM6GetConnectorInfoFromBIOS(pScrn)) {
	/* Below is the most common setting, but may not be true */
	pRM6Ent->PortInfo[0].MonType = MT_UNKNOWN;
	pRM6Ent->PortInfo[0].MonInfo = NULL;
	pRM6Ent->PortInfo[0].DDCType = DDC_DVI;
	pRM6Ent->PortInfo[0].DACType = DAC_TVDAC;
	pRM6Ent->PortInfo[0].TMDSType = TMDS_INT;
	pRM6Ent->PortInfo[0].ConnectorType = CONNECTOR_DVI_D;

	pRM6Ent->PortInfo[1].MonType = MT_UNKNOWN;
	pRM6Ent->PortInfo[1].MonInfo = NULL;
	pRM6Ent->PortInfo[1].DDCType = DDC_VGA;
	pRM6Ent->PortInfo[1].DACType = DAC_PRIMARY;
	pRM6Ent->PortInfo[1].TMDSType = TMDS_EXT;
	pRM6Ent->PortInfo[1].ConnectorType = CONNECTOR_CRT;
    }

    /* always make TMDS_INT port first*/
    if (pRM6Ent->PortInfo[1].TMDSType == TMDS_INT) {
        RADEONConnector connector;
        connector = pRM6Ent->PortInfo[0];
        pRM6Ent->PortInfo[0] = pRM6Ent->PortInfo[1];
        pRM6Ent->PortInfo[1] = connector;
    } else if ((pRM6Ent->PortInfo[0].TMDSType != TMDS_INT &&
                pRM6Ent->PortInfo[1].TMDSType != TMDS_INT)) {
        /* no TMDS_INT port, make primary DAC port first */
        if (pRM6Ent->PortInfo[1].DACType == DAC_PRIMARY) {
            RADEONConnector connector;
            connector = pRM6Ent->PortInfo[0];
            pRM6Ent->PortInfo[0] = pRM6Ent->PortInfo[1];
            pRM6Ent->PortInfo[1] = connector;
        }
    }

    if (info->HasSingleDAC) {
        /* For RS300/RS350/RS400 chips, there is no primary DAC. Force VGA port to use TVDAC*/
        if (pRM6Ent->PortInfo[0].ConnectorType == CONNECTOR_CRT) {
            pRM6Ent->PortInfo[0].DACType = DAC_TVDAC;
            pRM6Ent->PortInfo[1].DACType = DAC_PRIMARY;
        } else {
            pRM6Ent->PortInfo[1].DACType = DAC_TVDAC;
            pRM6Ent->PortInfo[0].DACType = DAC_PRIMARY;
        }
    } else if (!info->HasCRTC2) {
        pRM6Ent->PortInfo[0].DACType = DAC_PRIMARY;
    }

    /* IgnoreEDID option is different from the NoDDCxx options used by DDC module
     * When IgnoreEDID is used, monitor detection will still use DDC
     * detection, but all EDID data will not be used in mode validation.
     * You can use this option when you have a DDC monitor but want specify your own
     * monitor timing parameters by using HSync, VRefresh and Modeline,
     */
    if (xf86GetOptValBool(info->Options, OPTION_IGNORE_EDID, &ignore_edid)) {
        if (ignore_edid)
            xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
                       "IgnoreEDID is specified, EDID data will be ignored\n");
    }

    /*
     * MonitorLayout option takes a string for two monitors connected in following format:
     * Option "MonitorLayout" "primary-port-display, secondary-port-display"
     * primary and secondary port displays can have one of following:
     *    NONE, CRT, LVDS, TMDS
     * With this option, driver will bring up monitors as specified,
     * not using auto-detection routines to probe monitors.
     *
     * This option can be used when the false monitor detection occurs.
     *
     * This option can also be used to disable one connected display.
     * For example, if you have a laptop connected to an external CRT
     * and you want to disable the internal LCD panel, you can specify
     * Option "MonitorLayout" "NONE, CRT"
     *
     * This option can also used to disable Clone mode. One there is only
     * one monitor is specified, clone mode will be turned off automatically
     * even you have two monitors connected.
     *
     * Another usage of this option is you want to config the server
     * to start up with a certain monitor arrangement even one monitor
     * is not plugged in when server starts.
     */
    if ((s = xf86GetOptValString(info->Options, OPTION_MONITOR_LAYOUT))) {
        char s1[5], s2[5];
        i = 0;
        /* When using user specified monitor types, we will not do DDC detection
         *
         */
        do {
            switch(*s) {
            case ',':
                s1[i] = '\0';
                i = 0;
                second = 1;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                break;
            default:
                if (second)
                    s2[i] = *s;
                else
                    s1[i] = *s;
                i++;
                break;
            }
            if (i > 4) i = 4;
        } while(*s++);
        s2[i] = '\0';

	for (i = 0; i < max_mt; i++) {
	    if (strcmp(s1, MonTypeName[i]) == 0) {
		pRM6Ent->PortInfo[0].MonType = MonTypeID[i];
		break;
	    }
	}
	for (i = 0; i < max_mt; i++) {
	    if (strcmp(s2, MonTypeName[i]) == 0) {
		pRM6Ent->PortInfo[1].MonType = MonTypeID[i];
		break;
	    }
	}

	if (i ==  max_mt)
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Invalid Monitor type specified for 2nd port \n");

	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		   "MonitorLayout Option: \n\tMonitor1--Type %s, Monitor2--Type %s\n\n", s1, s2);

	if (pRM6Ent->PortInfo[1].MonType == MT_CRT) {
	    pRM6Ent->PortInfo[1].DACType = DAC_PRIMARY;
	    pRM6Ent->PortInfo[1].TMDSType = TMDS_UNKNOWN;
	    pRM6Ent->PortInfo[1].DDCType = DDC_VGA;
	    pRM6Ent->PortInfo[1].ConnectorType = CONNECTOR_CRT;
	    pRM6Ent->PortInfo[0].DACType = DAC_TVDAC;
	    pRM6Ent->PortInfo[0].TMDSType = TMDS_UNKNOWN;
	    pRM6Ent->PortInfo[0].DDCType = DDC_NONE_DETECTED;
	    pRM6Ent->PortInfo[0].ConnectorType = pRM6Ent->PortInfo[0].MonType+1;
	    pRM6Ent->PortInfo[0].MonInfo = NULL;
        }

        if (!ignore_edid) {
            if ((pRM6Ent->PortInfo[0].MonType > MT_NONE) &&
                (pRM6Ent->PortInfo[0].MonType < MT_STV))
		RM6DisplayDDCConnected(pScrn, pRM6Ent->PortInfo[0].DDCType,
					  &pRM6Ent->PortInfo[0]);
            if ((pRM6Ent->PortInfo[1].MonType > MT_NONE) &&
                (pRM6Ent->PortInfo[1].MonType < MT_STV))
		RM6DisplayDDCConnected(pScrn, pRM6Ent->PortInfo[1].DDCType,
					  &pRM6Ent->PortInfo[1]);
        }

    }

    if(((!info->HasCRTC2) || info->IsDellServer)) {
	if (pRM6Ent->PortInfo[0].MonType == MT_UNKNOWN) {
	    if((pRM6Ent->PortInfo[0].MonType = RM6DisplayDDCConnected(pScrn, DDC_DVI, &pRM6Ent->PortInfo[0])));
	    else if((pRM6Ent->PortInfo[0].MonType = RM6DisplayDDCConnected(pScrn, DDC_VGA, &pRM6Ent->PortInfo[0])));
	    else if((pRM6Ent->PortInfo[0].MonType = RM6DisplayDDCConnected(pScrn, DDC_CRT2, &pRM6Ent->PortInfo[0])));
	    else
		pRM6Ent->PortInfo[0].MonType = MT_CRT;
	}

	if (!ignore_edid) {
	    if (pRM6Ent->PortInfo[0].MonInfo) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Monitor1 EDID data ---------------------------\n");
		xf86PrintEDID(pRM6Ent->PortInfo[0].MonInfo );
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "End of Monitor1 EDID data --------------------\n");
	    }
	}

	pRM6Ent->MonType1 = pRM6Ent->PortInfo[0].MonType;
	pRM6Ent->MonInfo1 = pRM6Ent->PortInfo[0].MonInfo;
	pRM6Ent->MonType2 = MT_NONE;
	pRM6Ent->MonInfo2 = NULL;
	info->MergeType = MT_NONE;
	info->DisplayType = pRM6Ent->MonType1;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Primary:\n Monitor   -- %s\n Connector -- %s\n DAC Type  -- %s\n TMDS Type -- %s\n DDC Type  -- %s\n",
		   MonTypeName[pRM6Ent->PortInfo[0].MonType+1],
		   info->IsAtomBios ?
		   ConnectorTypeNameATOM[pRM6Ent->PortInfo[0].ConnectorType]:
		   ConnectorTypeName[pRM6Ent->PortInfo[0].ConnectorType],
		   DACTypeName[pRM6Ent->PortInfo[0].DACType+1],
		   TMDSTypeName[pRM6Ent->PortInfo[0].TMDSType+1],
		   DDCTypeName[pRM6Ent->PortInfo[0].DDCType]);

	return TRUE;
    }

    if (pRM6Ent->PortInfo[0].MonType == MT_UNKNOWN || pRM6Ent->PortInfo[1].MonType == MT_UNKNOWN) {

	/* Primary Head (DVI or Laptop Int. panel)*/
	/* A ddc capable display connected on DVI port */
	if (pRM6Ent->PortInfo[0].MonType == MT_UNKNOWN) {
	    if((pRM6Ent->PortInfo[0].MonType = RM6DisplayDDCConnected(pScrn, pRM6Ent->PortInfo[0].DDCType, &pRM6Ent->PortInfo[0])));
	    else if (info->IsMobility &&
		     (INREG(RADEON_BIOS_4_SCRATCH) & 4)) {
		/* non-DDC laptop panel connected on primary */
		pRM6Ent->PortInfo[0].MonType = MT_LCD;
	    } else {
		/* CRT on DVI, TODO: not reliable, make it always return false for now*/
		pRM6Ent->PortInfo[0].MonType = RM6CrtIsPhysicallyConnected(pScrn, !(pRM6Ent->PortInfo[0].DACType));
	    }
	}

	/* Secondary Head (mostly VGA, can be DVI on some OEM boards)*/
	if (pRM6Ent->PortInfo[1].MonType == MT_UNKNOWN) {
	    if((pRM6Ent->PortInfo[1].MonType =
                RM6DisplayDDCConnected(pScrn, pRM6Ent->PortInfo[1].DDCType, &pRM6Ent->PortInfo[1])));
            else if (info->IsMobility &&
                     (INREG(RADEON_FP2_GEN_CNTL) & RADEON_FP2_ON)) {
                /* non-DDC TMDS panel connected through DVO */
                pRM6Ent->PortInfo[1].MonType = MT_DFP;
            } else
                pRM6Ent->PortInfo[1].MonType = RM6CrtIsPhysicallyConnected(pScrn, !(pRM6Ent->PortInfo[1].DACType));
        }
    }

    if(ignore_edid) {
        pRM6Ent->PortInfo[0].MonInfo = NULL;
        pRM6Ent->PortInfo[1].MonInfo = NULL;
    } else {
        if (pRM6Ent->PortInfo[0].MonInfo) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "EDID data from the display on port 1 ----------------------\n");
            xf86PrintEDID(pRM6Ent->PortInfo[0].MonInfo );
        }

        if (pRM6Ent->PortInfo[1].MonInfo) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "EDID data from the display on port 2-----------------------\n");
            xf86PrintEDID(pRM6Ent->PortInfo[1].MonInfo );
        }
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "\n");

    pRM6Ent->MonType1 = pRM6Ent->PortInfo[0].MonType;
    pRM6Ent->MonInfo1 = pRM6Ent->PortInfo[0].MonInfo;
    pRM6Ent->MonType2 = pRM6Ent->PortInfo[1].MonType;
    pRM6Ent->MonInfo2 = pRM6Ent->PortInfo[1].MonInfo;
    if (pRM6Ent->PortInfo[0].MonType == MT_NONE) {
	if (pRM6Ent->PortInfo[1].MonType == MT_NONE) {
	    pRM6Ent->MonType1 = MT_CRT;
	    pRM6Ent->MonInfo1 = NULL;
	} else {
	    RADEONConnector tmp;
	    pRM6Ent->MonType1 = pRM6Ent->PortInfo[1].MonType;
	    pRM6Ent->MonInfo1 = pRM6Ent->PortInfo[1].MonInfo;
	    tmp = pRM6Ent->PortInfo[0];
	    pRM6Ent->PortInfo[0] = pRM6Ent->PortInfo[1];
	    pRM6Ent->PortInfo[1] = tmp;
	}
	pRM6Ent->MonType2 = MT_NONE;
	pRM6Ent->MonInfo2 = NULL;
    }

    info->DisplayType = pRM6Ent->MonType1;
    pRM6Ent->ReversedDAC = FALSE;
    info->OverlayOnCRTC2 = FALSE;
    info->MergeType = MT_NONE;
    if (pRM6Ent->MonType2 != MT_NONE) {
	if(!pRM6Ent->HasSecondary) {
	    info->MergeType = pRM6Ent->MonType2;
	}

	if (pRM6Ent->PortInfo[1].DACType == DAC_TVDAC) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Reversed DAC decteced\n");
	    pRM6Ent->ReversedDAC = TRUE;
	}
    } else {
	pRM6Ent->HasSecondary = FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Primary:\n Monitor   -- %s\n Connector -- %s\n DAC Type  -- %s\n TMDS Type -- %s\n DDC Type  -- %s\n",
	       MonTypeName[pRM6Ent->PortInfo[0].MonType+1],
	       info->IsAtomBios ?
	       ConnectorTypeNameATOM[pRM6Ent->PortInfo[0].ConnectorType]:
	       ConnectorTypeName[pRM6Ent->PortInfo[0].ConnectorType],
	       DACTypeName[pRM6Ent->PortInfo[0].DACType+1],
	       TMDSTypeName[pRM6Ent->PortInfo[0].TMDSType+1],
	       DDCTypeName[pRM6Ent->PortInfo[0].DDCType]);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Secondary:\n Monitor   -- %s\n Connector -- %s\n DAC Type  -- %s\n TMDS Type -- %s\n DDC Type  -- %s\n",
	       MonTypeName[pRM6Ent->PortInfo[1].MonType+1],
	       info->IsAtomBios ?
	       ConnectorTypeNameATOM[pRM6Ent->PortInfo[1].ConnectorType]:
	       ConnectorTypeName[pRM6Ent->PortInfo[1].ConnectorType],
	       DACTypeName[pRM6Ent->PortInfo[1].DACType+1],
	       TMDSTypeName[pRM6Ent->PortInfo[1].TMDSType+1],
	       DDCTypeName[pRM6Ent->PortInfo[1].DDCType]);

    return TRUE;
}


/* This is called by RM6PreInit to set up the default visual */
static Bool RM6PreInitVisual(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    if (!xf86SetDepthBpp(pScrn, 0, 0, 0, Support32bppFb))
	return FALSE;

    switch (pScrn->depth) {
    case 8:
    case 15:
    case 16:
    case 24:
	break;

    default:
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Given depth (%d) is not supported by %s driver\n",
		   pScrn->depth, RM6_DRIVER_NAME);
	return FALSE;
    }

    xf86PrintDepthBpp(pScrn);

    info->fifo_slots                 = 0;
    info->pix24bpp                   = xf86GetBppFromDepth(pScrn,
							   pScrn->depth);
    info->CurrentLayout.bitsPerPixel = pScrn->bitsPerPixel;
    info->CurrentLayout.depth        = pScrn->depth;
    info->CurrentLayout.pixel_bytes  = pScrn->bitsPerPixel / 8;
    info->CurrentLayout.pixel_code   = (pScrn->bitsPerPixel != 16
				       ? pScrn->bitsPerPixel
				       : pScrn->depth);

    if (info->pix24bpp == 24) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Radeon does NOT support 24bpp\n");
	return FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Pixel depth = %d bits stored in %d byte%s (%d bpp pixmaps)\n",
	       pScrn->depth,
	       info->CurrentLayout.pixel_bytes,
	       info->CurrentLayout.pixel_bytes > 1 ? "s" : "",
	       info->pix24bpp);

    if (!xf86SetDefaultVisual(pScrn, -1)) return FALSE;

    if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Default visual (%s) is not supported at depth %d\n",
		   xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
	return FALSE;
    }
    return TRUE;
}

/* This is called by RM6PreInit to handle all color weight issues */
static Bool RM6PreInitWeight(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

				/* Save flag for 6 bit DAC to use for
				   setting CRTC registers.  Otherwise use
				   an 8 bit DAC, even if xf86SetWeight sets
				   pScrn->rgbBits to some value other than
				   8. */
    info->dac6bits = FALSE;

    if (pScrn->depth > 8) {
	rgb  defaultWeight = { 0, 0, 0 };

	if (!xf86SetWeight(pScrn, defaultWeight, defaultWeight)) return FALSE;
    } else {
	pScrn->rgbBits = 8;
	if (xf86ReturnOptValBool(info->Options, OPTION_DAC_6BIT, FALSE)) {
	    pScrn->rgbBits = 6;
	    info->dac6bits = TRUE;
	}
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Using %d bits per RGB (%d bit DAC)\n",
	       pScrn->rgbBits, info->dac6bits ? 6 : 8);

    return TRUE;
}

/* Set up MC_FB_LOCATION and related registers */
static void
RM6SetFBLocation(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32 mc_fb_location;
    CARD32 mc_agp_location = INREG(RADEON_MC_AGP_LOCATION);


    /* This function has many problems with newer cards.
     * Even with older cards, all registers changed here are not
     * restored properly when X quits, this will also cause 
     * various problems, especially with radeonfb.
     * Since we don't have DRI support for R300 and above cards, 
     * we just hardcode these values for now.
     * Need to revisit this whole function!!!
     */
    if (IS_R300_VARIANT) {
	info->fbLocation = 0;
	
	if (!info->IsSecondary) {
	    RM6WaitForIdleMMIO(pScrn);
	    OUTREG (RADEON_MC_FB_LOCATION, (INREG(RADEON_CONFIG_MEMSIZE) - 1) & 0xffff0000);
	    OUTREG(RADEON_DISPLAY_BASE_ADDR, info->fbLocation);
	    OUTREG(RADEON_DISPLAY2_BASE_ADDR, info->fbLocation);
	    OUTREG(RADEON_OV0_BASE_ADDR, info->fbLocation);
	}
	return;
    }

    if (info->IsIGP) {
	mc_fb_location = INREG(RADEON_NB_TOM);

	OUTREG(RADEON_GRPH2_BUFFER_CNTL,
	       INREG(RADEON_GRPH2_BUFFER_CNTL) & ~0x7f0000);

    } else
    {
	CARD32 aper0_base = INREG(RADEON_CONFIG_APER_0_BASE);

	mc_fb_location = (aper0_base >> 16)
		       | ((aper0_base + (INREG(RADEON_CONFIG_APER_SIZE) - 1)
			   ) & 0xffff0000U);
    }

    info->fbLocation = (mc_fb_location & 0xffff) << 16;

    if (((mc_agp_location & 0xffff) << 16) !=
	((mc_fb_location & 0xffff0000U) + 0x10000)) {
	mc_agp_location = mc_fb_location & 0xffff0000U;
	mc_agp_location |= (mc_agp_location + 0x10000) >> 16;
    }

    RM6WaitForIdleMMIO(pScrn);

    OUTREG(RADEON_MC_FB_LOCATION, mc_fb_location);
    OUTREG(RADEON_MC_AGP_LOCATION, mc_agp_location);
    OUTREG(RADEON_DISPLAY_BASE_ADDR, info->fbLocation);
    if (info->HasCRTC2)
	OUTREG(RADEON_DISPLAY2_BASE_ADDR, info->fbLocation);
    OUTREG(RADEON_OV0_BASE_ADDR, info->fbLocation);
}

static void RM6GetVRamType(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info   = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32 tmp;
 
    if (info->IsIGP || (info->ChipFamily >= CHIP_FAMILY_R300) ||
	(INREG(RADEON_MEM_SDRAM_MODE_REG) & (1<<30))) 
	info->IsDDR = TRUE;
    else
	info->IsDDR = FALSE;

    tmp = INREG(RADEON_MEM_CNTL);
    if (IS_R300_VARIANT) {
	tmp &=  R300_MEM_NUM_CHANNELS_MASK;
	switch (tmp) {
	case 0: info->RamWidth = 64; break;
	case 1: info->RamWidth = 128; break;
	case 2: info->RamWidth = 256; break;
	default: info->RamWidth = 128; break;
	}
    } else if ((info->ChipFamily == CHIP_FAMILY_RV100) ||
	       (info->ChipFamily == CHIP_FAMILY_RS100) ||
	       (info->ChipFamily == CHIP_FAMILY_RS200)){
	if (tmp & RV100_HALF_MODE) info->RamWidth = 32;
	else info->RamWidth = 64;
    } else {
	if (tmp & RADEON_MEM_NUM_CHANNELS_MASK) info->RamWidth = 128;
	else info->RamWidth = 64;
    }

    /* This may not be correct, as some cards can have half of channel disabled 
     * ToDo: identify these cases
     */
}

/* This is called by RM6PreInit to handle config file overrides for
 * things like chipset and memory regions.  Also determine memory size
 * and type.  If memory type ever needs an override, put it in this
 * routine.
 */
static Bool RM6PreInitConfig(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info   = RM6PTR(pScrn);
    EntityInfoPtr  pEnt   = info->pEnt;
    GDevPtr        dev    = pEnt->device;
    MessageType    from;
    unsigned char *RM6MMIO = info->MMIO;

				/* Chipset */
    from = X_PROBED;
    if (dev->chipset && *dev->chipset) {
	info->Chipset  = xf86StringToToken(RM6Chipsets, dev->chipset);
	from           = X_CONFIG;
    } else if (dev->chipID >= 0) {
	info->Chipset  = dev->chipID;
	from           = X_CONFIG;
    } else {
	info->Chipset = info->PciInfo->chipType;
    }

    pScrn->chipset = (char *)xf86TokenToString(RM6Chipsets, info->Chipset);
    if (!pScrn->chipset) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "ChipID 0x%04x is not recognized\n", info->Chipset);
	return FALSE;
    }
    if (info->Chipset < 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Chipset \"%s\" is not recognized\n", pScrn->chipset);
	return FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from,
	       "Chipset: \"%s\" (ChipID = 0x%04x)\n",
	       pScrn->chipset,
	       info->Chipset);

    info->HasCRTC2 = TRUE;
    info->IsMobility = FALSE;
    info->IsIGP = FALSE;
    info->IsDellServer = FALSE;
    info->HasSingleDAC = FALSE;
    switch (info->Chipset) {
    case PCI_CHIP_RADEON_LY:
    case PCI_CHIP_RADEON_LZ:
	info->IsMobility = TRUE;
	info->ChipFamily = CHIP_FAMILY_RV100;
	break;

    case PCI_CHIP_RV100_QY:
    case PCI_CHIP_RV100_QZ:
	info->ChipFamily = CHIP_FAMILY_RV100;

	/* DELL triple-head configuration. */
	if ((info->PciInfo->subsysVendor == PCI_VENDOR_DELL) &&
	    ((info->PciInfo->subsysCard  == 0x016c) ||
	    (info->PciInfo->subsysCard   == 0x016d) ||
	    (info->PciInfo->subsysCard   == 0x016e) ||
	    (info->PciInfo->subsysCard   == 0x016f) ||
	    (info->PciInfo->subsysCard   == 0x0170) ||
	    (info->PciInfo->subsysCard   == 0x017d) ||
	    (info->PciInfo->subsysCard   == 0x017e) ||
	    (info->PciInfo->subsysCard   == 0x0183) ||
	    (info->PciInfo->subsysCard   == 0x018a) ||
	    (info->PciInfo->subsysCard   == 0x019a))) {
	    info->IsDellServer = TRUE;
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "DELL server detected, force to special setup\n");
	}

	break;

    case PCI_CHIP_RS100_4336:
	info->IsMobility = TRUE;
    case PCI_CHIP_RS100_4136:
	info->ChipFamily = CHIP_FAMILY_RS100;
	info->IsIGP = TRUE;
	break;

    case PCI_CHIP_RS200_4337:
	info->IsMobility = TRUE;
    case PCI_CHIP_RS200_4137:
	info->ChipFamily = CHIP_FAMILY_RS200;
	info->IsIGP = TRUE;
	break;

    case PCI_CHIP_RS250_4437:
	info->IsMobility = TRUE;
    case PCI_CHIP_RS250_4237:
	info->ChipFamily = CHIP_FAMILY_RS200;
	info->IsIGP = TRUE;
	break;

    case PCI_CHIP_R200_BB:
    case PCI_CHIP_R200_BC:
    case PCI_CHIP_R200_QH:
    case PCI_CHIP_R200_QL:
    case PCI_CHIP_R200_QM:
	info->ChipFamily = CHIP_FAMILY_R200;
	break;

    case PCI_CHIP_RADEON_LW:
    case PCI_CHIP_RADEON_LX:
	info->IsMobility = TRUE;
    case PCI_CHIP_RV200_QW: /* RV200 desktop */
    case PCI_CHIP_RV200_QX:
	info->ChipFamily = CHIP_FAMILY_RV200;
	break;

    case PCI_CHIP_RV250_Ld:
    case PCI_CHIP_RV250_Lf:
    case PCI_CHIP_RV250_Lg:
	info->IsMobility = TRUE;
    case PCI_CHIP_RV250_If:
    case PCI_CHIP_RV250_Ig:
	info->ChipFamily = CHIP_FAMILY_RV250;
	break;

    case PCI_CHIP_RS300_5835:
    case PCI_CHIP_RS350_7835:
	info->IsMobility = TRUE;
    case PCI_CHIP_RS300_5834:
    case PCI_CHIP_RS350_7834:
	info->ChipFamily = CHIP_FAMILY_RS300;
	info->IsIGP = TRUE;
	info->HasSingleDAC = TRUE;
	break;

    case PCI_CHIP_RV280_5C61:
    case PCI_CHIP_RV280_5C63:
	info->IsMobility = TRUE;
    case PCI_CHIP_RV280_5960:
    case PCI_CHIP_RV280_5961:
    case PCI_CHIP_RV280_5962:
    case PCI_CHIP_RV280_5964:
	info->ChipFamily = CHIP_FAMILY_RV280;
	break;

    case PCI_CHIP_R300_AD:
    case PCI_CHIP_R300_AE:
    case PCI_CHIP_R300_AF:
    case PCI_CHIP_R300_AG:
    case PCI_CHIP_R300_ND:
    case PCI_CHIP_R300_NE:
    case PCI_CHIP_R300_NF:
    case PCI_CHIP_R300_NG:
	info->ChipFamily = CHIP_FAMILY_R300;
        break;

    case PCI_CHIP_RV350_NP:
    case PCI_CHIP_RV350_NQ:
    case PCI_CHIP_RV350_NR:
    case PCI_CHIP_RV350_NS:
    case PCI_CHIP_RV350_NT:
    case PCI_CHIP_RV350_NV:
	info->IsMobility = TRUE;
    case PCI_CHIP_RV350_AP:
    case PCI_CHIP_RV350_AQ:
    case PCI_CHIP_RV360_AR:
    case PCI_CHIP_RV350_AS:
    case PCI_CHIP_RV350_AT:
    case PCI_CHIP_RV350_AV:
	info->ChipFamily = CHIP_FAMILY_RV350;
        break;

    case PCI_CHIP_R350_AH:
    case PCI_CHIP_R350_AI:
    case PCI_CHIP_R350_AJ:
    case PCI_CHIP_R350_AK:
    case PCI_CHIP_R350_NH:
    case PCI_CHIP_R350_NI:
    case PCI_CHIP_R350_NK:
    case PCI_CHIP_R360_NJ:
	info->ChipFamily = CHIP_FAMILY_R350;
        break;

    case PCI_CHIP_RV380_3150:
    case PCI_CHIP_RV380_3154:
        info->IsMobility = TRUE;
    case PCI_CHIP_RV380_3E50:
    case PCI_CHIP_RV380_3E54:
        info->ChipFamily = CHIP_FAMILY_RV380;
        break;

    case PCI_CHIP_RV370_5460:
    case PCI_CHIP_RV370_5464:
        info->IsMobility = TRUE;
    case PCI_CHIP_RV370_5B60:
    case PCI_CHIP_RV370_5B64:
    case PCI_CHIP_RV370_5B65:
        info->ChipFamily = CHIP_FAMILY_RV380;
        break;

    case PCI_CHIP_R420_JN:
        info->IsMobility = TRUE;
    case PCI_CHIP_R420_JH:
    case PCI_CHIP_R420_JI:
    case PCI_CHIP_R420_JJ:
    case PCI_CHIP_R420_JK:
    case PCI_CHIP_R420_JL:
    case PCI_CHIP_R420_JM:
    case PCI_CHIP_R420_JP:
        info->ChipFamily = CHIP_FAMILY_R420;
        break;

    case PCI_CHIP_R423_UH:
    case PCI_CHIP_R423_UI:
    case PCI_CHIP_R423_UJ:
    case PCI_CHIP_R423_UK:
    case PCI_CHIP_R423_UQ:
    case PCI_CHIP_R423_UR:
    case PCI_CHIP_R423_UT:
    case PCI_CHIP_R423_5D57:
        info->ChipFamily = CHIP_FAMILY_R420;
        break;

    default:
	/* Original Radeon/7200 */
	info->ChipFamily = CHIP_FAMILY_RADEON;
	info->HasCRTC2 = FALSE;
    }

				/* Framebuffer */

    from               = X_PROBED;
    info->LinearAddr   = info->PciInfo->memBase[0] & 0xfe000000;
    pScrn->memPhysBase = info->LinearAddr;
    if (dev->MemBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Linear address override, using 0x%08lx instead of 0x%08lx\n",
		   dev->MemBase,
		   info->LinearAddr);
	info->LinearAddr = dev->MemBase;
	from             = X_CONFIG;
    } else if (!info->LinearAddr) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "No valid linear framebuffer address\n");
	return FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from,
	       "Linear framebuffer at 0x%08lx\n", info->LinearAddr);

				/* BIOS */
    from              = X_PROBED;
    info->BIOSAddr    = info->PciInfo->biosBase & 0xfffe0000;
    if (dev->BiosBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "BIOS address override, using 0x%08lx instead of 0x%08lx\n",
		   dev->BiosBase,
		   info->BIOSAddr);
	info->BIOSAddr = dev->BiosBase;
	from           = X_CONFIG;
    }
    if (info->BIOSAddr) {
	xf86DrvMsg(pScrn->scrnIndex, from,
		   "BIOS at 0x%08lx\n", info->BIOSAddr);
    }

				/* Read registers used to determine options */
    from                     = X_PROBED;
    if ((info->ChipFamily == CHIP_FAMILY_RS100) ||
	     (info->ChipFamily == CHIP_FAMILY_RS200) ||
	     (info->ChipFamily == CHIP_FAMILY_RS300)) {
        CARD32 tom = INREG(RADEON_NB_TOM);

	pScrn->videoRam = (((tom >> 16) -
			    (tom & 0xffff) + 1) << 6);

	OUTREG(RADEON_CONFIG_MEMSIZE, pScrn->videoRam * 1024);
    } else {
        /* There are different HDP mapping schemes depending on single/multi funciton setting,
         * chip family, HDP mode, and the generation of HDP mapping scheme.
         * To make things simple, we only allow maximum 128M addressable FB. Anything more than
         * 128M is configured as invisible FB to CPU that can only be accessed from chip side.
         */
        pScrn->videoRam      = INREG(RADEON_CONFIG_MEMSIZE) / 1024;
        if (pScrn->videoRam > 128*1024) pScrn->videoRam = 128*1024;
        if ((info->ChipFamily == CHIP_FAMILY_RV350) ||
            (info->ChipFamily == CHIP_FAMILY_RV380) ||
            (info->ChipFamily == CHIP_FAMILY_R420)) {
	    OUTREGP (RADEON_HOST_PATH_CNTL, (1<<23), ~(1<<23));
        }
    }

    /* Some production boards of m6 will return 0 if it's 8 MB */
    if (pScrn->videoRam == 0) pScrn->videoRam = 8192;

    if (info->IsSecondary) {
	/* FIXME: For now, split FB into two equal sections. This should
	 * be able to be adjusted by user with a config option. */
        RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);
	RM6InfoPtr  info1;

	pScrn->videoRam /= 2;
	pRM6Ent->pPrimaryScrn->videoRam = pScrn->videoRam;

	info1 = RM6PTR(pRM6Ent->pPrimaryScrn);
	info1->FbMapSize  = pScrn->videoRam * 1024;
	info->LinearAddr += pScrn->videoRam * 1024;
	info1->MergedFB = FALSE;
    }

    info->R300CGWorkaround =
	(info->ChipFamily == CHIP_FAMILY_R300 &&
	 (INREG(RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK)
	 == RADEON_CFG_ATI_REV_A11);

    info->MemCntl            = INREG(RADEON_SDRAM_MODE_REG);
    info->BusCntl            = INREG(RADEON_BUS_CNTL);

    RM6GetVRamType(pScrn);

    if (dev->videoRam) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Video RAM override, using %d kB instead of %d kB\n",
		   dev->videoRam,
		   pScrn->videoRam);
	from             = X_CONFIG;
	pScrn->videoRam  = dev->videoRam;
    }
    pScrn->videoRam  &= ~1023;
    info->FbMapSize  = pScrn->videoRam * 1024;
    xf86DrvMsg(pScrn->scrnIndex, from,
	       "VideoRAM: %d kByte (%d bit %s SDRAM)\n", pScrn->videoRam, info->RamWidth, info->IsDDR?"DDR":"SDR");

    xf86GetOptValBool(info->Options, OPTION_SHOWCACHE, &info->showCache);
    if (info->showCache)
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		   "Option ShowCache enabled\n");

#ifdef RENDER
    info->RenderAccel = xf86ReturnOptValBool (info->Options,
					      OPTION_RENDER_ACCEL, TRUE);
#endif

    return TRUE;
}

static void RM6I2CGetBits(I2CBusPtr b, int *Clock, int *data)
{
    ScrnInfoPtr    pScrn      = xf86Screens[b->scrnIndex];
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned long  val;
    unsigned char *RM6MMIO = info->MMIO;

    /* Get the result */
    val = INREG(info->DDCReg);

    *Clock = (val & RADEON_GPIO_Y_1) != 0;
    *data  = (val & RADEON_GPIO_Y_0) != 0;
}

static void RM6I2CPutBits(I2CBusPtr b, int Clock, int data)
{
    ScrnInfoPtr    pScrn      = xf86Screens[b->scrnIndex];
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned long  val;
    unsigned char *RM6MMIO = info->MMIO;

    val = INREG(info->DDCReg) & (CARD32)~(RADEON_GPIO_EN_0 | RADEON_GPIO_EN_1);
    val |= (Clock ? 0:RADEON_GPIO_EN_1);
    val |= (data ? 0:RADEON_GPIO_EN_0);
    OUTREG(info->DDCReg, val);

    /* read back to improve reliability on some cards. */
    val = INREG(info->DDCReg);
}

static Bool RM6I2cInit(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    info->pI2CBus = xf86CreateI2CBusRec();
    if (!info->pI2CBus) return FALSE;

    info->pI2CBus->BusName    = "DDC";
    info->pI2CBus->scrnIndex  = pScrn->scrnIndex;
    info->pI2CBus->I2CPutBits = RM6I2CPutBits;
    info->pI2CBus->I2CGetBits = RM6I2CGetBits;
    info->pI2CBus->AcknTimeout = 5;

    if (!xf86I2CBusInit(info->pI2CBus)) return FALSE;
    return TRUE;
}

static void RM6PreInitDDC(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
 /* vbeInfoPtr     pVbe; */

    info->ddc1     = FALSE;
    info->ddc_bios = FALSE;
    if (!xf86LoadSubModule(pScrn, "ddc")) {
	info->ddc2 = FALSE;
    } else {
	xf86LoaderReqSymLists(ddcSymbols, NULL);
	info->ddc2 = TRUE;
    }

    /* DDC can use I2C bus */
    /* Load I2C if we have the code to use it */
    if (info->ddc2) {
	if (xf86LoadSubModule(pScrn, "i2c")) {
	    xf86LoaderReqSymLists(i2cSymbols,NULL);
	    info->ddc2 = RM6I2cInit(pScrn);
	}
	else info->ddc2 = FALSE;
    }
}


/* BIOS may not have right panel size, we search through all supported
 * DDC modes looking for the maximum panel size.
 */
static void RM6UpdatePanelSize(ScrnInfoPtr pScrn)
{
    int             j;
    RM6InfoPtr   info = RM6PTR (pScrn);
    xf86MonPtr      ddc  = pScrn->monitor->DDC;
    DisplayModePtr  p;

    /* Go thru detailed timing table first */
    for (j = 0; j < 4; j++) {
	if (ddc->det_mon[j].type == 0) {
	    struct detailed_timings *d_timings =
		&ddc->det_mon[j].section.d_timings;
	    if (info->PanelXRes <= d_timings->h_active &&
		info->PanelYRes <= d_timings->v_active) {

		if (info->DotClock) continue; /* Timings already inited */

		info->PanelXRes  = d_timings->h_active;
		info->PanelYRes  = d_timings->v_active;
		info->DotClock   = d_timings->clock / 1000;
		info->HOverPlus  = d_timings->h_sync_off;
		info->HSyncWidth = d_timings->h_sync_width;
		info->HBlank     = d_timings->h_blanking;
		info->VOverPlus  = d_timings->v_sync_off;
		info->VSyncWidth = d_timings->v_sync_width;
		info->VBlank     = d_timings->v_blanking;
	    }
	}
    }

    /* Search thru standard VESA modes from EDID */
    for (j = 0; j < 8; j++) {
	if ((info->PanelXRes < ddc->timings2[j].hsize) &&
	    (info->PanelYRes < ddc->timings2[j].vsize)) {
	    for (p = pScrn->monitor->Modes; p && p->next; p = p->next->next) {
		if ((ddc->timings2[j].hsize == p->HDisplay) &&
		    (ddc->timings2[j].vsize == p->VDisplay)) {
		    float  refresh =
			(float)p->Clock * 1000.0 / p->HTotal / p->VTotal;

		    if (abs((float)ddc->timings2[j].refresh - refresh) < 1.0) {
			/* Is this good enough? */
			info->PanelXRes  = ddc->timings2[j].hsize;
			info->PanelYRes  = ddc->timings2[j].vsize;
			info->HBlank     = p->HTotal - p->HDisplay;
			info->HOverPlus  = p->HSyncStart - p->HDisplay;
			info->HSyncWidth = p->HSyncEnd - p->HSyncStart;
			info->VBlank     = p->VTotal - p->VDisplay;
			info->VOverPlus  = p->VSyncStart - p->VDisplay;
			info->VSyncWidth = p->VSyncEnd - p->VSyncStart;
			info->DotClock   = p->Clock;
			info->Flags      =
			    (ddc->det_mon[j].section.d_timings.interlaced
			     ? V_INTERLACE
			     : 0);
			if (ddc->det_mon[j].section.d_timings.sync == 3) {
			    switch (ddc->det_mon[j].section.d_timings.misc) {
			    case 0: info->Flags |= V_NHSYNC | V_NVSYNC; break;
			    case 1: info->Flags |= V_PHSYNC | V_NVSYNC; break;
			    case 2: info->Flags |= V_NHSYNC | V_PVSYNC; break;
			    case 3: info->Flags |= V_PHSYNC | V_PVSYNC; break;
			    }
			}
		    }
		}
	    }
	}
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Panel size found from DDC: %dx%d\n",
	       info->PanelXRes, info->PanelYRes);
}

/* This function will sort all modes according to their resolution.
 * Highest resolution first.
 */
static void RM6SortModes(DisplayModePtr *new, DisplayModePtr *first,
			    DisplayModePtr *last)
{
    DisplayModePtr  p;

    p = *last;
    while (p) {
	if ((((*new)->HDisplay < p->HDisplay) &&
	     ((*new)->VDisplay < p->VDisplay)) ||
	    (((*new)->HDisplay == p->HDisplay) &&
	     ((*new)->VDisplay == p->VDisplay) &&
	     ((*new)->Clock < p->Clock))) {

	    if (p->next) p->next->prev = *new;
	    (*new)->prev = p;
	    (*new)->next = p->next;
	    p->next = *new;
	    if (!((*new)->next)) *last = *new;
	    break;
	}
	if (!p->prev) {
	    (*new)->prev = NULL;
	    (*new)->next = p;
	    p->prev = *new;
	    *first = *new;
	    break;
	}
	p = p->prev;
    }

    if (!*first) {
	*first = *new;
	(*new)->prev = NULL;
	(*new)->next = NULL;
	*last = *new;
    }
}

static void RM6SetPitch (ScrnInfoPtr pScrn)
{
    int  dummy = pScrn->virtualX;

    /* FIXME: May need to validate line pitch here */
    switch (pScrn->depth / 8) {
    case 1: dummy = (pScrn->virtualX + 127) & ~127; break;
    case 2: dummy = (pScrn->virtualX +  31) &  ~31; break;
    case 3:
    case 4: dummy = (pScrn->virtualX +  15) &  ~15; break;
    }
    pScrn->displayWidth = dummy;
}

/* When no mode provided in config file, this will add all modes supported in
 * DDC date the pScrn->modes list
 */
static DisplayModePtr RM6DDCModes(ScrnInfoPtr pScrn)
{
    DisplayModePtr  p;
    DisplayModePtr  last  = NULL;
    DisplayModePtr  new   = NULL;
    DisplayModePtr  first = NULL;
    int             count = 0;
    int             j, tmp;
    char            stmp[32];
    xf86MonPtr      ddc   = pScrn->monitor->DDC;

    /* Go thru detailed timing table first */
    for (j = 0; j < 4; j++) {
	if (ddc->det_mon[j].type == 0) {
	    struct detailed_timings *d_timings =
		&ddc->det_mon[j].section.d_timings;

	    if (d_timings->h_active == 0 || d_timings->v_active == 0) break;

	    new = xnfcalloc(1, sizeof (DisplayModeRec));
	    memset(new, 0, sizeof (DisplayModeRec));

	    new->HDisplay   = d_timings->h_active;
	    new->VDisplay   = d_timings->v_active;

	    sprintf(stmp, "%dx%d", new->HDisplay, new->VDisplay);
	    new->name       = xnfalloc(strlen(stmp) + 1);
	    strcpy(new->name, stmp);

	    new->HTotal     = new->HDisplay + d_timings->h_blanking;
	    new->HSyncStart = new->HDisplay + d_timings->h_sync_off;
	    new->HSyncEnd   = new->HSyncStart + d_timings->h_sync_width;
	    new->VTotal     = new->VDisplay + d_timings->v_blanking;
	    new->VSyncStart = new->VDisplay + d_timings->v_sync_off;
	    new->VSyncEnd   = new->VSyncStart + d_timings->v_sync_width;
	    new->Clock      = d_timings->clock / 1000;
	    new->Flags      = (d_timings->interlaced ? V_INTERLACE : 0);
	    new->status     = MODE_OK;
	    new->type       = M_T_DEFAULT;

	    if (d_timings->sync == 3) {
		switch (d_timings->misc) {
		case 0: new->Flags |= V_NHSYNC | V_NVSYNC; break;
		case 1: new->Flags |= V_PHSYNC | V_NVSYNC; break;
		case 2: new->Flags |= V_NHSYNC | V_PVSYNC; break;
		case 3: new->Flags |= V_PHSYNC | V_PVSYNC; break;
		}
	    }
	    count++;

	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Valid Mode from Detailed timing table: %s\n",
		       new->name);

	    RM6SortModes(&new, &first, &last);
	}
    }

    /* Search thru standard VESA modes from EDID */
    for (j = 0; j < 8; j++) {
	for (p = pScrn->monitor->Modes; p && p->next; p = p->next->next) {
	    /* Ignore all double scan modes */
	    if ((ddc->timings2[j].hsize == p->HDisplay) &&
		(ddc->timings2[j].vsize == p->VDisplay)) {
		float  refresh =
		    (float)p->Clock * 1000.0 / p->HTotal / p->VTotal;

		if (abs((float)ddc->timings2[j].refresh - refresh) < 1.0) {
		    /* Is this good enough? */
		    new = xnfcalloc(1, sizeof (DisplayModeRec));
		    memcpy(new, p, sizeof(DisplayModeRec));
		    new->name = xnfalloc(strlen(p->name) + 1);
		    strcpy(new->name, p->name);
		    new->status = MODE_OK;
		    new->type   = M_T_DEFAULT;

		    count++;

		    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			       "Valid Mode from standard timing table: %s\n",
			       new->name);

		    RM6SortModes(&new, &first, &last);
		    break;
		}
	    }
	}
    }

    /* Search thru established modes from EDID */
    tmp = (ddc->timings1.t1 << 8) | ddc->timings1.t2;
    for (j = 0; j < 16; j++) {
	if (tmp & (1 << j)) {
	    for (p = pScrn->monitor->Modes; p && p->next; p = p->next->next) {
		if ((est_timings[j].hsize == p->HDisplay) &&
		    (est_timings[j].vsize == p->VDisplay)) {
		    float  refresh =
			(float)p->Clock * 1000.0 / p->HTotal / p->VTotal;

		    if (abs((float)est_timings[j].refresh - refresh) < 1.0) {
			/* Is this good enough? */
			new = xnfcalloc(1, sizeof (DisplayModeRec));
			memcpy(new, p, sizeof(DisplayModeRec));
			new->name = xnfalloc(strlen(p->name) + 1);
			strcpy(new->name, p->name);
			new->status = MODE_OK;
			new->type   = M_T_DEFAULT;

			count++;

			xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				   "Valid Mode from established timing "
				   "table: %s\n", new->name);

			RM6SortModes(&new, &first, &last);
			break;
		    }
		}
	    }
	}
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Total of %d mode(s) found.\n", count);

    return first;
}

/* XFree86's xf86ValidateModes routine doesn't work well with DDC modes,
 * so here is our own validation routine.
 */
static int RM6ValidateDDCModes(ScrnInfoPtr pScrn1, char **ppModeName,
				  RADEONMonitorType DisplayType, int crtc2)
{
    RM6InfoPtr   info       = RM6PTR(pScrn1);
    DisplayModePtr  p;
    DisplayModePtr  last       = NULL;
    DisplayModePtr  first      = NULL;
    DisplayModePtr  ddcModes   = NULL;
    int             count      = 0;
    int             i, width, height;
    ScrnInfoPtr pScrn = pScrn1;

    if (crtc2)
	pScrn = info->CRT2pScrn;

    pScrn->virtualX = pScrn1->display->virtualX;
    pScrn->virtualY = pScrn1->display->virtualY;

    if (pScrn->monitor->DDC && !info->UseBiosDividers) {
	int  maxVirtX = pScrn->virtualX;
	int  maxVirtY = pScrn->virtualY;

	if ((DisplayType != MT_CRT) && (!info->IsSecondary) && (!crtc2)) {
	    /* The panel size we collected from BIOS may not be the
	     * maximum size supported by the panel.  If not, we update
	     * it now.  These will be used if no matching mode can be
	     * found from EDID data.
	     */
	    RM6UpdatePanelSize(pScrn);
	}

	/* Collect all of the DDC modes */
	first = last = ddcModes = RM6DDCModes(pScrn);

	for (p = ddcModes; p; p = p->next) {

	    /* If primary head is a flat panel, use RMX by default */
	    if ((!info->IsSecondary && DisplayType != MT_CRT) &&
		(!info->ddc_mode) && (!crtc2)) {
		/* These values are effective values after expansion.
		 * They are not really used to set CRTC registers.
		 */
		p->HTotal     = info->PanelXRes + info->HBlank;
		p->HSyncStart = info->PanelXRes + info->HOverPlus;
		p->HSyncEnd   = p->HSyncStart + info->HSyncWidth;
		p->VTotal     = info->PanelYRes + info->VBlank;
		p->VSyncStart = info->PanelYRes + info->VOverPlus;
		p->VSyncEnd   = p->VSyncStart + info->VSyncWidth;
		p->Clock      = info->DotClock;

		p->Flags     |= RADEON_USE_RMX;
	    }

	    maxVirtX = MAX(maxVirtX, p->HDisplay);
	    maxVirtY = MAX(maxVirtY, p->VDisplay);
	    count++;

	    last = p;
	}

	/* Match up modes that are specified in the XF86Config file */
	if (ppModeName[0]) {
	    DisplayModePtr  next;

	    /* Reset the max virtual dimensions */
	    maxVirtX = pScrn->virtualX;
	    maxVirtY = pScrn->virtualY;

	    /* Reset list */
	    first = last = NULL;

	    for (i = 0; ppModeName[i]; i++) {
		/* FIXME: Use HDisplay and VDisplay instead of mode string */
		if (sscanf(ppModeName[i], "%dx%d", &width, &height) == 2) {
		    for (p = ddcModes; p; p = next) {
			next = p->next;

			if (p->HDisplay == width && p->VDisplay == height) {
			    /* We found a DDC mode that matches the one
                               requested in the XF86Config file */
			    p->type |= M_T_USERDEF;

			    /* Update  the max virtual setttings */
			    maxVirtX = MAX(maxVirtX, width);
			    maxVirtY = MAX(maxVirtY, height);

			    /* Unhook from DDC modes */
			    if (p->prev) p->prev->next = p->next;
			    if (p->next) p->next->prev = p->prev;
			    if (p == ddcModes) ddcModes = p->next;

			    /* Add to used modes */
			    if (last) {
				last->next = p;
				p->prev = last;
			    } else {
				first = p;
				p->prev = NULL;
			    }
			    p->next = NULL;
			    last = p;

			    break;
			}
		    }
		}
	    }

	    /*
	     * Add remaining DDC modes if they're smaller than the user
	     * specified modes
	     */
	    for (p = ddcModes; p; p = next) {
		next = p->next;
		if (p->HDisplay <= maxVirtX && p->VDisplay <= maxVirtY) {
		    /* Unhook from DDC modes */
		    if (p->prev) p->prev->next = p->next;
		    if (p->next) p->next->prev = p->prev;
		    if (p == ddcModes) ddcModes = p->next;

		    /* Add to used modes */
		    if (last) {
			last->next = p;
			p->prev = last;
		    } else {
			first = p;
			p->prev = NULL;
		    }
		    p->next = NULL;
		    last = p;
		}
	    }

	    /* Delete unused modes */
	    while (ddcModes)
		xf86DeleteMode(&ddcModes, ddcModes);
	} else {
	    /*
	     * No modes were configured, so we make the DDC modes
	     * available for the user to cycle through.
	     */
	    for (p = ddcModes; p; p = p->next)
		p->type |= M_T_USERDEF;
	}

        if (crtc2) {
            pScrn->virtualX = maxVirtX;
            pScrn->virtualY = maxVirtY;
	} else {
	    pScrn->virtualX = pScrn->display->virtualX = maxVirtX;
	    pScrn->virtualY = pScrn->display->virtualY = maxVirtY;
	}
    }

    /* Close the doubly-linked mode list, if we found any usable modes */
    if (last) {
	last->next   = first;
	first->prev  = last;
	pScrn->modes = first;
	RM6SetPitch(pScrn);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Total number of valid DDC mode(s) found: %d\n", count);

    return count;
}

/* This is used only when no mode is specified for FP and no ddc is
 * available.  We force it to native mode, if possible.
 */
static DisplayModePtr RM6FPNativeMode(ScrnInfoPtr pScrn)
{
    RM6InfoPtr   info  = RM6PTR(pScrn);
    DisplayModePtr  new   = NULL;
    char            stmp[32];

    if (info->PanelXRes != 0 &&
	info->PanelYRes != 0 &&
	info->DotClock != 0) {

	/* Add native panel size */
	new             = xnfcalloc(1, sizeof (DisplayModeRec));
	sprintf(stmp, "%dx%d", info->PanelXRes, info->PanelYRes);
	new->name       = xnfalloc(strlen(stmp) + 1);
	strcpy(new->name, stmp);
	new->HDisplay   = info->PanelXRes;
	new->VDisplay   = info->PanelYRes;

	new->HTotal     = new->HDisplay + info->HBlank;
	new->HSyncStart = new->HDisplay + info->HOverPlus;
	new->HSyncEnd   = new->HSyncStart + info->HSyncWidth;
	new->VTotal     = new->VDisplay + info->VBlank;
	new->VSyncStart = new->VDisplay + info->VOverPlus;
	new->VSyncEnd   = new->VSyncStart + info->VSyncWidth;

	new->Clock      = info->DotClock;
	new->Flags      = 0;
	new->type       = M_T_USERDEF;

	new->next       = NULL;
	new->prev       = NULL;

	pScrn->display->virtualX =
	    pScrn->virtualX = MAX(pScrn->virtualX, info->PanelXRes);
	pScrn->display->virtualY =
	    pScrn->virtualY = MAX(pScrn->virtualY, info->PanelYRes);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "No valid mode specified, force to native mode\n");
    }

    return new;
}

/* FP mode initialization routine for using on-chip RMX to scale
 */
static int RM6ValidateFPModes(ScrnInfoPtr pScrn, char **ppModeName)
{
    RM6InfoPtr   info       = RM6PTR(pScrn);
    DisplayModePtr  last       = NULL;
    DisplayModePtr  new        = NULL;
    DisplayModePtr  first      = NULL;
    DisplayModePtr  p, tmp;
    int             count      = 0;
    int             i, width, height;

    pScrn->virtualX = pScrn->display->virtualX;
    pScrn->virtualY = pScrn->display->virtualY;

    /* We have a flat panel connected to the primary display, and we
     * don't have any DDC info.
     */
    for (i = 0; ppModeName[i] != NULL; i++) {

	if (sscanf(ppModeName[i], "%dx%d", &width, &height) != 2) continue;

	/* Note: We allow all non-standard modes as long as they do not
	 * exceed the native resolution of the panel.  Since these modes
	 * need the internal RMX unit in the video chips (and there is
	 * only one per card), this will only apply to the primary head.
	 */
	if (width < 320 || width > info->PanelXRes ||
	    height < 200 || height > info->PanelYRes) {
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Mode %s is out of range.\n", ppModeName[i]);
	    xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		       "Valid modes must be between 320x200-%dx%d\n",
		       info->PanelXRes, info->PanelYRes);
	    continue;
	}

	new             = xnfcalloc(1, sizeof(DisplayModeRec));
	new->name       = xnfalloc(strlen(ppModeName[i]) + 1);
	strcpy(new->name, ppModeName[i]);
	new->HDisplay   = width;
	new->VDisplay   = height;

	/* These values are effective values after expansion They are
	 * not really used to set CRTC registers.
	 */
	new->HTotal     = info->PanelXRes + info->HBlank;
	new->HSyncStart = info->PanelXRes + info->HOverPlus;
	new->HSyncEnd   = new->HSyncStart + info->HSyncWidth;
	new->VTotal     = info->PanelYRes + info->VBlank;
	new->VSyncStart = info->PanelYRes + info->VOverPlus;
	new->VSyncEnd   = new->VSyncStart + info->VSyncWidth;
	new->Clock      = info->DotClock;
	new->Flags     |= RADEON_USE_RMX;

	new->type      |= M_T_USERDEF;

	new->next       = NULL;
	new->prev       = last;

	if (last) last->next = new;
	last = new;
	if (!first) first = new;

	pScrn->display->virtualX =
	    pScrn->virtualX = MAX(pScrn->virtualX, width);
	pScrn->display->virtualY =
	    pScrn->virtualY = MAX(pScrn->virtualY, height);
	count++;
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Valid mode using on-chip RMX: %s\n", new->name);
    }

    /* If all else fails, add the native mode */
    if (!count) {
	first = last = RM6FPNativeMode(pScrn);
	if (first) count = 1;
    }

    /* add in all default vesa modes smaller than panel size, used for randr*/
    for (p = pScrn->monitor->Modes; p && p->next; p = p->next->next) {
	if ((p->HDisplay <= info->PanelXRes) && (p->VDisplay <= info->PanelYRes)) {
	    tmp = first;
	    while (tmp) {
		if ((p->HDisplay == tmp->HDisplay) && (p->VDisplay == tmp->VDisplay)) break;
		tmp = tmp->next;
	    }
	    if (!tmp) {
		new             = xnfcalloc(1, sizeof(DisplayModeRec));
		new->name       = xnfalloc(strlen(p->name) + 1);
		strcpy(new->name, p->name);
		new->HDisplay   = p->HDisplay;
		new->VDisplay   = p->VDisplay;

		/* These values are effective values after expansion They are
		 * not really used to set CRTC registers.
		 */
		new->HTotal     = info->PanelXRes + info->HBlank;
		new->HSyncStart = info->PanelXRes + info->HOverPlus;
		new->HSyncEnd   = new->HSyncStart + info->HSyncWidth;
		new->VTotal     = info->PanelYRes + info->VBlank;
		new->VSyncStart = info->PanelYRes + info->VOverPlus;
		new->VSyncEnd   = new->VSyncStart + info->VSyncWidth;
		new->Clock      = info->DotClock;
		new->Flags     |= RADEON_USE_RMX;

		new->type      |= M_T_DEFAULT;

		new->next       = NULL;
		new->prev       = last;

		if (last) last->next = new;
		last = new;
		if (!first) first = new;
	    }
	}
    }

    /* Close the doubly-linked mode list, if we found any usable modes */
    if (last) {
	last->next   = first;
	first->prev  = last;
	pScrn->modes = first;
	RM6SetPitch(pScrn);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Total number of valid FP mode(s) found: %d\n", count);

    return count;
}

/* This is called by RM6PreInit to initialize gamma correction */
static Bool RM6PreInitGamma(ScrnInfoPtr pScrn)
{
    Gamma  zeros = { 0.0, 0.0, 0.0 };

    if (!xf86SetGamma(pScrn, zeros)) return FALSE;
    return TRUE;
}

static void RM6SetSyncRangeFromEdid(ScrnInfoPtr pScrn, int flag)
{
    MonPtr      mon = pScrn->monitor;
    xf86MonPtr  ddc = mon->DDC;
    int         i;

    if (flag) { /* HSync */
	for (i = 0; i < 4; i++) {
	    if (ddc->det_mon[i].type == DS_RANGES) {
		mon->nHsync = 1;
		mon->hsync[0].lo = ddc->det_mon[i].section.ranges.min_h;
		mon->hsync[0].hi = ddc->det_mon[i].section.ranges.max_h;
		return;
	    }
	}
	/* If no sync ranges detected in detailed timing table, let's
	 * try to derive them from supported VESA modes.  Are we doing
	 * too much here!!!?  */
	i = 0;
	if (ddc->timings1.t1 & 0x02) { /* 800x600@56 */
	    mon->hsync[i].lo = mon->hsync[i].hi = 35.2;
	    i++;
	}
	if (ddc->timings1.t1 & 0x04) { /* 640x480@75 */
	    mon->hsync[i].lo = mon->hsync[i].hi = 37.5;
	    i++;
	}
	if ((ddc->timings1.t1 & 0x08) || (ddc->timings1.t1 & 0x01)) {
	    mon->hsync[i].lo = mon->hsync[i].hi = 37.9;
	    i++;
	}
	if (ddc->timings1.t2 & 0x40) {
	    mon->hsync[i].lo = mon->hsync[i].hi = 46.9;
	    i++;
	}
	if ((ddc->timings1.t2 & 0x80) || (ddc->timings1.t2 & 0x08)) {
	    mon->hsync[i].lo = mon->hsync[i].hi = 48.1;
	    i++;
	}
	if (ddc->timings1.t2 & 0x04) {
	    mon->hsync[i].lo = mon->hsync[i].hi = 56.5;
	    i++;
	}
	if (ddc->timings1.t2 & 0x02) {
	    mon->hsync[i].lo = mon->hsync[i].hi = 60.0;
	    i++;
	}
	if (ddc->timings1.t2 & 0x01) {
	    mon->hsync[i].lo = mon->hsync[i].hi = 64.0;
	    i++;
	}
	mon->nHsync = i;
    } else {  /* Vrefresh */
	for (i = 0; i < 4; i++) {
	    if (ddc->det_mon[i].type == DS_RANGES) {
		mon->nVrefresh = 1;
		mon->vrefresh[0].lo = ddc->det_mon[i].section.ranges.min_v;
		mon->vrefresh[0].hi = ddc->det_mon[i].section.ranges.max_v;
		return;
	    }
	}

	i = 0;
	if (ddc->timings1.t1 & 0x02) { /* 800x600@56 */
	    mon->vrefresh[i].lo = mon->vrefresh[i].hi = 56;
	    i++;
	}
	if ((ddc->timings1.t1 & 0x01) || (ddc->timings1.t2 & 0x08)) {
	    mon->vrefresh[i].lo = mon->vrefresh[i].hi = 60;
	    i++;
	}
	if (ddc->timings1.t2 & 0x04) {
	    mon->vrefresh[i].lo = mon->vrefresh[i].hi = 70;
	    i++;
	}
	if ((ddc->timings1.t1 & 0x08) || (ddc->timings1.t2 & 0x80)) {
	    mon->vrefresh[i].lo = mon->vrefresh[i].hi = 72;
	    i++;
	}
	if ((ddc->timings1.t1 & 0x04) || (ddc->timings1.t2 & 0x40) ||
	    (ddc->timings1.t2 & 0x02) || (ddc->timings1.t2 & 0x01)) {
	    mon->vrefresh[i].lo = mon->vrefresh[i].hi = 75;
	    i++;
	}
	mon->nVrefresh = i;
    }
}

static int RM6ValidateMergeModes(ScrnInfoPtr pScrn1)
{
    RM6InfoPtr   info             = RM6PTR(pScrn1);
    ClockRangePtr   clockRanges;
    int             modesFound;
    ScrnInfoPtr pScrn = info->CRT2pScrn;

    /* fill in pScrn2 */
    pScrn->videoRam = pScrn1->videoRam;
    pScrn->depth = pScrn1->depth;
    pScrn->numClocks = pScrn1->numClocks;
    pScrn->progClock = pScrn1->progClock;
    pScrn->fbFormat = pScrn1->fbFormat;
    pScrn->videoRam = pScrn1->videoRam;
    pScrn->maxHValue = pScrn1->maxHValue;
    pScrn->maxVValue = pScrn1->maxVValue;
    pScrn->xInc = pScrn1->xInc;

    if (info->NoVirtual) {
	pScrn1->display->virtualX = 0;
        pScrn1->display->virtualY = 0;
    }

    if (pScrn->monitor->DDC) {
        /* If we still don't know sync range yet, let's try EDID.
         *
         * Note that, since we can have dual heads, Xconfigurator
         * may not be able to probe both monitors correctly through
         * vbe probe function (RADEONProbeDDC). Here we provide an
         * additional way to auto-detect sync ranges if they haven't
         * been added to XF86Config manually.
         */
        if (pScrn->monitor->nHsync <= 0)
            RM6SetSyncRangeFromEdid(pScrn, 1);
        if (pScrn->monitor->nVrefresh <= 0)
            RM6SetSyncRangeFromEdid(pScrn, 0);
    }

    /* Get mode information */
    pScrn->progClock               = TRUE;
    clockRanges                    = xnfcalloc(sizeof(*clockRanges), 1);
    clockRanges->next              = NULL;
    clockRanges->minClock          = info->pll.min_pll_freq;
    clockRanges->maxClock          = info->pll.max_pll_freq * 10;
    clockRanges->clockIndex        = -1;
    clockRanges->interlaceAllowed  = (info->MergeType == MT_CRT);
    clockRanges->doubleScanAllowed = (info->MergeType == MT_CRT);

    /* We'll use our own mode validation routine for DFP/LCD, since
     * xf86ValidateModes does not work correctly with the DFP/LCD modes
     * 'stretched' from their native mode.
     */
    if (info->MergeType == MT_CRT && !info->ddc_mode) {
 
	modesFound =
	    xf86ValidateModes(pScrn,
			      pScrn->monitor->Modes,
			      pScrn1->display->modes,
			      clockRanges,
			      NULL,                  /* linePitches */
			      8 * 64,                /* minPitch */
			      8 * 1024,              /* maxPitch */
			      64 * pScrn1->bitsPerPixel, /* pitchInc */
			      128,                   /* minHeight */
			      8 * 1024, /*2048,*/    /* maxHeight */
			      pScrn1->display->virtualX ? pScrn1->virtualX : 0,
			      pScrn1->display->virtualY ? pScrn1->virtualY : 0,
			      info->FbMapSize,
			      LOOKUP_BEST_REFRESH);

	if (modesFound == -1) return 0;

	xf86PruneDriverModes(pScrn);
	if (!modesFound || !pScrn->modes) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
	    return 0;
	}

    } else {
	/* First, free any allocated modes during configuration, since
	 * we don't need them
	 */
	while (pScrn->modes)
	    xf86DeleteMode(&pScrn->modes, pScrn->modes);
	while (pScrn->modePool)
	    xf86DeleteMode(&pScrn->modePool, pScrn->modePool);

	/* Next try to add DDC modes */
	modesFound = RM6ValidateDDCModes(pScrn, pScrn1->display->modes,
					    info->MergeType, 1);

	/* If that fails and we're connect to a flat panel, then try to
         * add the flat panel modes
	 */
	if (info->MergeType != MT_CRT) {
	    
	    /* some panels have DDC, but don't have internal scaler.
	     * in this case, we need to validate additional modes
	     * by using on-chip RMX.
	     */
	    int user_modes_asked = 0, user_modes_found = 0, i;
	    DisplayModePtr  tmp_mode = pScrn->modes;
	    while (pScrn1->display->modes[user_modes_asked]) user_modes_asked++;	    
	    if (tmp_mode) {
		for (i = 0; i < modesFound; i++) {
		    if (tmp_mode->type & M_T_USERDEF) user_modes_found++;
		    tmp_mode = tmp_mode->next;
		}
	    }

 	    if ((modesFound <= 1) || (user_modes_found < user_modes_asked)) {
		/* when panel size is not valid, try to validate 
		 * mode using xf86ValidateModes routine
		 * This can happen when DDC is disabled.
		 */
		/* if (info->PanelXRes < 320 || info->PanelYRes < 200) */
		    modesFound =
			xf86ValidateModes(pScrn,
					  pScrn->monitor->Modes,
					  pScrn1->display->modes,
					  clockRanges,
					  NULL,                  /* linePitches */
					  8 * 64,                /* minPitch */
					  8 * 1024,              /* maxPitch */
					  64 * pScrn1->bitsPerPixel, /* pitchInc */
					  128,                   /* minHeight */
					  8 * 1024, /*2048,*/    /* maxHeight */
					  pScrn1->display->virtualX,
					  pScrn1->display->virtualY,
					  info->FbMapSize,
					  LOOKUP_BEST_REFRESH);

	    } 
        }

	/* Setup the screen's clockRanges for the VidMode extension */
	if (!pScrn->clockRanges) {
	    pScrn->clockRanges = xnfcalloc(sizeof(*(pScrn->clockRanges)), 1);
	    memcpy(pScrn->clockRanges, clockRanges, sizeof(*clockRanges));
	    pScrn->clockRanges->strategy = LOOKUP_BEST_REFRESH;
	}

	/* Fail if we still don't have any valid modes */
	if (modesFound < 1) {
	    if (info->MergeType == MT_CRT) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "No valid DDC modes found for this CRT\n");
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Try turning off the \"DDCMode\" option\n");
	    } else {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "No valid mode found for this DFP/LCD\n");
	    }
	    return 0;
	}
    }
    return modesFound;
}


/* This is called by RM6PreInit to validate modes and compute
 * parameters for all of the valid modes.
 */
static Bool RM6PreInitModes(ScrnInfoPtr pScrn, xf86Int10InfoPtr pInt10)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ClockRangePtr  clockRanges;
    int            modesFound;
    RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);
    char           *s;

    /* This option has two purposes:
     *
     * 1. For CRT, if this option is on, xf86ValidateModes (to
     *    LOOKUP_BEST_REFRESH) is not going to be used for mode
     *    validation.  Instead, we'll validate modes by matching exactly
     *    the modes supported from the DDC data.  This option can be
     *    used (a) to enable non-standard modes listed in the Detailed
     *    Timings block of EDID, like 2048x1536 (not included in
     *    xf86DefModes), (b) to avoid unstable modes for some flat
     *    panels working in analog mode (some modes validated by
     *    xf86ValidateModes don't really work with these panels).
     *
     * 2. For DFP on primary head, with this option on, the validation
     *    routine will try to use supported modes from DDC data first
     *    before trying on-chip RMX streching.  By default, native mode
     *    + RMX streching is used for all non-native modes, it appears
     *    more reliable. Some non-native modes listed in the DDC data
     *    may not work properly if they are used directly. This seems to
     *    only happen to a few panels (haven't nailed this down yet, it
     *    may related to the incorrect setting in TMDS_PLL_CNTL when
     *    pixel clock is changed).  Use this option may give you better
     *    refresh rate for some non-native modes.  The 2nd DVI port will
     *    always use DDC modes directly (only have one on-chip RMX
     *    unit).
     *
     * Note: This option will be dismissed if no DDC data is available.
     */

    if (info->MergedFB) {
	if (!(pScrn->display->virtualX))
	    info->NoVirtual = TRUE;
	else
	    info->NoVirtual = FALSE;
    }

    info->ddc_mode =
	xf86ReturnOptValBool(info->Options, OPTION_DDC_MODE, FALSE);

    /* don't use RMX if we have a dual-tmds panels */
   if (pRM6Ent->MonType2 == MT_DFP)
	info->ddc_mode = TRUE;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Validating modes on %s head ---------\n",
	       info->IsSecondary ? "Secondary" : "Primary");

    if (info->IsSecondary)
        pScrn->monitor->DDC = pRM6Ent->MonInfo2;
    else
        pScrn->monitor->DDC = pRM6Ent->MonInfo1;

    if (!pScrn->monitor->DDC && info->ddc_mode) {
	info->ddc_mode = FALSE;
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "No DDC data available, DDCMode option is dismissed\n");
    }

    if ((info->DisplayType == MT_DFP) ||
	(info->DisplayType == MT_LCD)) {
	if ((s = xf86GetOptValString(info->Options, OPTION_PANEL_SIZE))) {
	    int PanelX, PanelY;
	    DisplayModePtr  tmp_mode         = NULL;
	    if (sscanf(s, "%dx%d", &PanelX, &PanelY) == 2) {
		info->PanelXRes = PanelX;
		info->PanelYRes = PanelY;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
			   "Panel size is forced to: %s\n", s);

		/* We can't trust BIOS or DDC timings anymore, 
		   Use whatever specified in the Modeline.
		   If no Modeline specified, we'll just pick the VESA mode at 
		   60Hz refresh rate which is likely to be the best for a flat panel.
		*/ 
		info->ddc_mode = FALSE;
		pScrn->monitor->DDC = NULL;
		tmp_mode = pScrn->monitor->Modes;
		while(tmp_mode) {
		    if ((tmp_mode->HDisplay == PanelX) && 
			(tmp_mode->VDisplay == PanelY)) {

			float  refresh =
			    (float)tmp_mode->Clock * 1000.0 / tmp_mode->HTotal / tmp_mode->VTotal;
			if ((abs(60.0 - refresh) < 1.0) ||
			    (tmp_mode->type == 0)) {
			    info->HBlank     = tmp_mode->HTotal - tmp_mode->HDisplay;
			    info->HOverPlus  = tmp_mode->HSyncStart - tmp_mode->HDisplay;
			    info->HSyncWidth = tmp_mode->HSyncEnd - tmp_mode->HSyncStart;
			    info->VBlank     = tmp_mode->VTotal - tmp_mode->VDisplay;
			    info->VOverPlus  = tmp_mode->VSyncStart - tmp_mode->VDisplay;
			    info->VSyncWidth = tmp_mode->VSyncEnd - tmp_mode->VSyncStart;
			    info->DotClock   = tmp_mode->Clock;
			    info->Flags = 0;
			    break;
			}
		    }
		    tmp_mode = tmp_mode->next;
		}
		if (info->DotClock == 0) {
		    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			       "No valid timing info for specified panel size.\n"
			       "Please specify the Modeline for this panel\n");
		    return FALSE;
		}
	    } else {
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
			   "Invalid PanelSize value: %s\n", s);
	    }
	}
    }

    if (pScrn->monitor->DDC) {
        /* If we still don't know sync range yet, let's try EDID.
         *
         * Note that, since we can have dual heads, Xconfigurator
         * may not be able to probe both monitors correctly through
         * vbe probe function (RM6ProbeDDC). Here we provide an
         * additional way to auto-detect sync ranges if they haven't
         * been added to XF86Config manually.
         */
        if (pScrn->monitor->nHsync <= 0)
            RM6SetSyncRangeFromEdid(pScrn, 1);
        if (pScrn->monitor->nVrefresh <= 0)
            RM6SetSyncRangeFromEdid(pScrn, 0);
    }

    /* Get mode information */
    pScrn->progClock               = TRUE;
    clockRanges                    = xnfcalloc(sizeof(*clockRanges), 1);
    clockRanges->next              = NULL;
    clockRanges->minClock          = info->pll.min_pll_freq;
    clockRanges->maxClock          = info->pll.max_pll_freq * 10;
    clockRanges->clockIndex        = -1;
    clockRanges->interlaceAllowed  = (info->DisplayType == MT_CRT);
    clockRanges->doubleScanAllowed = (info->DisplayType == MT_CRT);

    /* We'll use our own mode validation routine for DFP/LCD, since
     * xf86ValidateModes does not work correctly with the DFP/LCD modes
     * 'stretched' from their native mode.
     */
    if (info->DisplayType == MT_CRT && !info->ddc_mode) {

	modesFound =
	    xf86ValidateModes(pScrn,
			      pScrn->monitor->Modes,
			      pScrn->display->modes,
			      clockRanges,
			      NULL,                  /* linePitches */
			      8 * 64,                /* minPitch */
			      8 * 1024,              /* maxPitch */
			      64 * pScrn->bitsPerPixel, /* pitchInc */
			      128,                   /* minHeight */
			      2048,                  /* maxHeight */
			      pScrn->display->virtualX,
			      pScrn->display->virtualY,
			      info->FbMapSize,
			      LOOKUP_BEST_REFRESH);


	if (modesFound == -1) return FALSE;

	xf86PruneDriverModes(pScrn);
	if (!modesFound || !pScrn->modes) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
	    return FALSE;
	}

    } else {
	/* First, free any allocated modes during configuration, since
	 * we don't need them
	 */
	while (pScrn->modes)
	    xf86DeleteMode(&pScrn->modes, pScrn->modes);
	while (pScrn->modePool)
	    xf86DeleteMode(&pScrn->modePool, pScrn->modePool);

	/* Next try to add DDC modes */
	modesFound = RM6ValidateDDCModes(pScrn, pScrn->display->modes,
					    info->DisplayType, 0);

	/* If that fails and we're connect to a flat panel, then try to
         * add the flat panel modes
	 */
	if (info->DisplayType != MT_CRT) {

	    /* some panels have DDC, but don't have internal scaler.
	     * in this case, we need to validate additional modes
	     * by using on-chip RMX.
	     */
	    int user_modes_asked = 0, user_modes_found = 0, i;
	    DisplayModePtr  tmp_mode = pScrn->modes;
	    while (pScrn->display->modes[user_modes_asked]) user_modes_asked++;
	    if (tmp_mode) {
		for (i = 0; i < modesFound; i++) {
		    if (tmp_mode->type & M_T_USERDEF) user_modes_found++;
		    tmp_mode = tmp_mode->next;
		}
	    }

	    if ((modesFound <= 1) || (user_modes_found < user_modes_asked)) {
		/* when panel size is not valid, try to validate
		 * mode using xf86ValidateModes routine
		 * This can happen when DDC is disabled.
		 */
		if (info->PanelXRes < 320 || info->PanelYRes < 200)
		    modesFound =
			xf86ValidateModes(pScrn,
					  pScrn->monitor->Modes,
					  pScrn->display->modes,
					  clockRanges,
					  NULL,                  /* linePitches */
					  8 * 64,                /* minPitch */
					  8 * 1024,              /* maxPitch */
					  64 * pScrn->bitsPerPixel, /* pitchInc */
					  128,                   /* minHeight */
					  2048,                  /* maxHeight */
					  pScrn->display->virtualX,
					  pScrn->display->virtualY,
					  info->FbMapSize,
					  LOOKUP_BEST_REFRESH);
		else if (!info->IsSecondary)
		    modesFound = RM6ValidateFPModes(pScrn, pScrn->display->modes);
	    }
        }

	/* Setup the screen's clockRanges for the VidMode extension */
	if (!pScrn->clockRanges) {
	    pScrn->clockRanges = xnfcalloc(sizeof(*(pScrn->clockRanges)), 1);
	    memcpy(pScrn->clockRanges, clockRanges, sizeof(*clockRanges));
	    pScrn->clockRanges->strategy = LOOKUP_BEST_REFRESH;
	}

	/* Fail if we still don't have any valid modes */
	if (modesFound < 1) {
	    if (info->DisplayType == MT_CRT) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "No valid DDC modes found for this CRT\n");
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Try turning off the \"DDCMode\" option\n");
	    } else {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "No valid mode found for this DFP/LCD\n");
	    }
	    return FALSE;
	}
    }

    xf86SetCrtcForModes(pScrn, 0);

    if (info->HasCRTC2) {
	if (info->MergedFB) {

	    /* If we have 2 screens from the config file, we don't need
	     * to do clone thing, let each screen handles one head.
	     */
	    if (!pRM6Ent->HasSecondary) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "Validating CRTC2 modes for MergedFB ------------ \n");

		modesFound = RM6ValidateMergeModes(pScrn);
		if (modesFound < 1) {
		    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			       "No valid mode found for CRTC2, disabling MergedFB\n");
		    info->MergedFB = FALSE;
		}
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "Total of %d CRTC2 modes found for MergedFB------------ \n",
			   modesFound);
	    }
	}
    }

    pScrn->currentMode = pScrn->modes;
    if(info->MergedFB) {
       xf86DrvMsg(pScrn->scrnIndex, X_INFO,
       	   "Modes for CRT1: ********************\n");
    }
    xf86PrintModes(pScrn);

    if(info->MergedFB) {

       xf86SetCrtcForModes(info->CRT2pScrn, INTERLACE_HALVE_V);

       xf86DrvMsg(pScrn->scrnIndex, X_INFO,
           "Modes for CRT2: ********************\n");

       xf86PrintModes(info->CRT2pScrn);

       info->CRT1Modes = pScrn->modes;
       info->CRT1CurrentMode = pScrn->currentMode;

       xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Generating MergedFB mode list\n");

       if (info->NoVirtual) {
           pScrn->display->virtualX = 0;
           pScrn->display->virtualY = 0;
       }
       pScrn->modes = RM6GenerateModeList(pScrn, info->MetaModes,
	            	                  info->CRT1Modes, info->CRT2pScrn->modes,
					  info->CRT2Position);

       if(!pScrn->modes) {

	  xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	      "Failed to parse MetaModes or no modes found. MergeFB mode disabled.\n");
	  if(info->CRT2pScrn) {
	     if(info->CRT2pScrn->modes) {
	        while(info->CRT2pScrn->modes)
		   xf86DeleteMode(&info->CRT2pScrn->modes, info->CRT2pScrn->modes);
	     }
	     if(info->CRT2pScrn->monitor) {
	        if(info->CRT2pScrn->monitor->Modes) {
	           while(info->CRT2pScrn->monitor->Modes)
		      xf86DeleteMode(&info->CRT2pScrn->monitor->Modes, info->CRT2pScrn->monitor->Modes);
	        }
		if(info->CRT2pScrn->monitor->DDC) xfree(info->CRT2pScrn->monitor->DDC);
	        xfree(info->CRT2pScrn->monitor);
	     }
             xfree(info->CRT2pScrn);
	  }
	  pScrn->modes = info->CRT1Modes;
	  info->CRT1Modes = NULL;
	  info->MergedFB = FALSE;

       }
    }

    if (info->MergedFB) {
       /* If no virtual dimension was given by the user,
        * calculate a sane one now. Adapts pScrn->virtualX,
	* pScrn->virtualY and pScrn->displayWidth.
	*/
       RM6RecalcDefaultVirtualSize(pScrn);
       info->CRT2pScrn->virtualX = pScrn->virtualX;
       info->CRT2pScrn->virtualY = pScrn->virtualY;
       RM6SetPitch(pScrn);
       RM6SetPitch(info->CRT2pScrn);

       pScrn->modes = pScrn->modes->next;  /* We get the last from GenerateModeList() */
       pScrn->currentMode = pScrn->modes;

       /* Update CurrentLayout */
       info->CurrentLayout.mode = pScrn->currentMode;
       info->CurrentLayout.displayWidth = pScrn->displayWidth;
    }

				/* Set DPI */
    /* xf86SetDpi(pScrn, 0, 0); */

    if(info->MergedFB)
	RM6MergedFBSetDpi(pScrn, info->CRT2pScrn, info->CRT2Position);
    else
	xf86SetDpi(pScrn, 0, 0);

				/* Get ScreenInit function */
    if (!xf86LoadSubModule(pScrn, "fb")) return FALSE;

    xf86LoaderReqSymLists(fbSymbols, NULL);

    info->CurrentLayout.displayWidth = pScrn->displayWidth;
    info->CurrentLayout.mode = pScrn->currentMode;

    return TRUE;
}

/* This is called by RM6PreInit to initialize the hardware cursor */
static Bool RM6PreInitCursor(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
	if (!xf86LoadSubModule(pScrn, "ramdac")) return FALSE;
	xf86LoaderReqSymLists(ramdacSymbols, NULL);
    }
    return TRUE;
}

/* This is called by RM6PreInit to initialize hardware acceleration */
static Bool RM6PreInitAccel(ScrnInfoPtr pScrn)
{
#ifdef XFree86LOADER
    RM6InfoPtr  info = RM6PTR(pScrn);

    if (!xf86ReturnOptValBool(info->Options, OPTION_NOACCEL, FALSE)) {
	int errmaj = 0, errmin = 0;

	info->xaaReq.majorversion = 1;
	info->xaaReq.minorversion = 2;

	if (!LoadSubModule(pScrn->module, "xaa", NULL, NULL, NULL,
			   &info->xaaReq, &errmaj, &errmin)) {
	    info->xaaReq.minorversion = 1;

	    if (!LoadSubModule(pScrn->module, "xaa", NULL, NULL, NULL,
			       &info->xaaReq, &errmaj, &errmin)) {
		info->xaaReq.minorversion = 0;

		if (!LoadSubModule(pScrn->module, "xaa", NULL, NULL, NULL,
			       &info->xaaReq, &errmaj, &errmin)) {
		    LoaderErrorMsg(NULL, "xaa", errmaj, errmin);
		    return FALSE;
		}
	    }
	}
	xf86LoaderReqSymLists(xaaSymbols, NULL);
    }
#endif

    return TRUE;
}

static Bool RM6PreInitInt10(ScrnInfoPtr pScrn, xf86Int10InfoPtr *ppInt10)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

#if !defined(__powerpc__)
    if (xf86LoadSubModule(pScrn, "int10")) {
	xf86LoaderReqSymLists(int10Symbols, NULL);
	xf86DrvMsg(pScrn->scrnIndex,X_INFO,"initializing int10\n");
	*ppInt10 = xf86InitInt10(info->pEnt->index);
    }
#endif
    return TRUE;
}


static void
RM6ProbeDDC(ScrnInfoPtr pScrn, int indx)
{
    vbeInfoPtr  pVbe;

    if (xf86LoadSubModule(pScrn, "vbe")) {
	pVbe = VBEInit(NULL,indx);
	ConfiguredMonitor = vbeDoEDID(pVbe, NULL);
    }
}

/* RM6PreInit is called once at server startup */
Bool RM6PreInit(ScrnInfoPtr pScrn, int flags)
{
    RM6InfoPtr     info;
    xf86Int10InfoPtr  pInt10 = NULL;
    void *int10_save = NULL;
    const char *s;

    RADEONTRACE(("RM6PreInit\n"));
    if (pScrn->numEntities != 1) return FALSE;

    if (!RM6GetRec(pScrn)) return FALSE;

    info               = RM6PTR(pScrn);
    info->IsSecondary  = FALSE;
    info->MergedFB     = FALSE;
    info->IsSwitching  = FALSE;
    info->MMIO         = NULL;

    info->pEnt         = xf86GetEntityInfo(pScrn->entityList[pScrn->numEntities - 1]);
    if (info->pEnt->location.type != BUS_PCI) goto fail;

    info->PciInfo = xf86GetPciInfoForEntity(info->pEnt->index);
    info->PciTag  = pciTag(info->PciInfo->bus,
			   info->PciInfo->device,
			   info->PciInfo->func);
    info->MMIOAddr   = info->PciInfo->memBase[2] & 0xffffff00;
    if (info->pEnt->device->IOBase) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		   "MMIO address override, using 0x%08lx instead of 0x%08lx\n",
		   info->pEnt->device->IOBase,
		   info->MMIOAddr);
	info->MMIOAddr = info->pEnt->device->IOBase;
    } else if (!info->MMIOAddr) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid MMIO address\n");
	goto fail1;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "MMIO registers at 0x%08lx\n", info->MMIOAddr);

    if(!RM6MapMMIO(pScrn)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Memory map the MMIO region failed\n");
	goto fail1;
    }

#if !defined(__alpha__)
    if (xf86GetPciDomain(info->PciTag) ||
	!xf86IsPrimaryPci(info->PciInfo))
	RM6PreInt10Save(pScrn, &int10_save);
#else
    /* [Alpha] On the primary, the console already ran the BIOS and we're
     *         going to run it again - so make sure to "fix up" the card
     *         so that (1) we can read the BIOS ROM and (2) the BIOS will
     *         get the memory config right.
     */
    RM6PreInt10Save(pScrn, &int10_save);
#endif

    if (xf86IsEntityShared(info->pEnt->index)) {
	if (xf86IsPrimInitDone(info->pEnt->index)) {

	    RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);

	    info->IsSecondary = TRUE;
	    if (!pRM6Ent->HasSecondary) {
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "Only one monitor detected, Second screen "
			   "will NOT be created\n");
		goto fail2;
	    }
	    pRM6Ent->pSecondaryScrn = pScrn;
	} else {
	    RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);

	    xf86SetPrimInitDone(info->pEnt->index);

	    pRM6Ent->pPrimaryScrn        = pScrn;
	    pRM6Ent->RestorePrimary      = FALSE;
	    pRM6Ent->IsSecondaryRestored = FALSE;
	}
    }

    if (flags & PROBE_DETECT) {
	RM6ProbeDDC(pScrn, info->pEnt->index);
	RM6PostInt10Check(pScrn, int10_save);
	if(info->MMIO) RM6UnmapMMIO(pScrn);
	return TRUE;
    }

    if (!xf86LoadSubModule(pScrn, "vgahw")) return FALSE;
    xf86LoaderReqSymLists(vgahwSymbols, NULL);
    if (!vgaHWGetHWRec(pScrn)) {
	RM6FreeRec(pScrn);
	goto fail2;
    }

    vgaHWGetIOBase(VGAHWPTR(pScrn));

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "PCI bus %d card %d func %d\n",
	       info->PciInfo->bus,
	       info->PciInfo->device,
	       info->PciInfo->func);

    if (xf86RegisterResources(info->pEnt->index, 0, ResExclusive))
	goto fail;

    if (xf86SetOperatingState(resVga, info->pEnt->index, ResUnusedOpr))
	goto fail;

    pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_VIEWPORT | RAC_CURSOR;
    pScrn->monitor     = pScrn->confScreen->monitor;

    if (!RM6PreInitVisual(pScrn))
	goto fail;

				/* We can't do this until we have a
				   pScrn->display. */
    xf86CollectOptions(pScrn, NULL);
    if (!(info->Options = xalloc(sizeof(RM6Options))))
	goto fail;

    memcpy(info->Options, RM6Options, sizeof(RM6Options));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, info->Options);

    if (!RM6PreInitWeight(pScrn))
	goto fail;

    if (xf86GetOptValInteger(info->Options, OPTION_VIDEO_KEY,
			     &(info->videoKey))) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "video key set to 0x%x\n",
		   info->videoKey);
    } else {
	info->videoKey = 0x1E;
    }

    info->DispPriority = 1; 
    if ((s = xf86GetOptValString(info->Options, OPTION_DISP_PRIORITY))) {
	if (strcmp(s, "AUTO") == 0) {
	    info->DispPriority = 1;
	} else if (strcmp(s, "BIOS") == 0) {
	    info->DispPriority = 0;
	} else if (strcmp(s, "HIGH") == 0) {
	    info->DispPriority = 2;
	} else
	    info->DispPriority = 1; 
    }


    if (!RM6PreInitInt10(pScrn, &pInt10))
	    goto fail;

    RM6PostInt10Check(pScrn, int10_save);

    if (!RM6PreInitConfig(pScrn))
	goto fail;

    RM6PreInitDDC(pScrn);

    RM6GetBIOSInfo(pScrn, pInt10);
    if (!RM6QueryConnectedMonitors(pScrn))    goto fail;
    RM6GetClockInfo(pScrn);
    RM6GetPanelInfo(pScrn);

    /* collect MergedFB options */
    /* only parse mergedfb options on the primary head. 
       Mergedfb is already disabled in xinerama/screen based
       multihead */
    if (!info->IsSecondary)
	RM6GetMergedFBOptions(pScrn);

    if (!RM6PreInitGamma(pScrn))              goto fail;

    if (!RM6PreInitModes(pScrn, pInt10))      goto fail;

    if (!RM6PreInitCursor(pScrn))             goto fail;

    if (!RM6PreInitAccel(pScrn))              goto fail;


				/* Free the video bios (if applicable) */
    if (info->VBIOS) {
	xfree(info->VBIOS);
	info->VBIOS = NULL;
    }

				/* Free int10 info */
    if (pInt10)
	xf86FreeInt10(pInt10);

    if(info->MMIO) RM6UnmapMMIO(pScrn);
    info->MMIO = NULL;

    xf86DrvMsg(pScrn->scrnIndex, X_NOTICE,
	       "For information on using the multimedia capabilities\n\tof this"
	       " adapter, please see http://gatos.sf.net.\n");

    return TRUE;

fail:
				/* Pre-init failed. */
    if (info->IsSecondary) {
        RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);
	pRM6Ent->HasSecondary = FALSE;
    }
				/* Free the video bios (if applicable) */
    if (info->VBIOS) {
	xfree(info->VBIOS);
	info->VBIOS = NULL;
    }

				/* Free int10 info */
    if (pInt10)
	xf86FreeInt10(pInt10);

    vgaHWFreeHWRec(pScrn);

 fail2:
    if(info->MMIO) RM6UnmapMMIO(pScrn);
    info->MMIO = NULL;

 fail1:
    RM6FreeRec(pScrn);

    return FALSE;
}

/* Load a palette */
static void RM6LoadPalette(ScrnInfoPtr pScrn, int numColors,
			      int *indices, LOCO *colors, VisualPtr pVisual)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            i;
    int            idx, j;
    unsigned char  r, g, b;


    if (info->accelOn && pScrn->pScreen) info->accel->Sync(pScrn);

    /* If the second monitor is connected, we also need to deal with
     * the secondary palette
     */
    if (info->IsSecondary) j = 1;
    else j = 0;
    
    PAL_SELECT(j);

    if (info->CurrentLayout.depth == 15) {
	    /* 15bpp mode.  This sends 32 values. */
	    for (i = 0; i < numColors; i++) {
		    idx = indices[i];
		    r   = colors[idx].red;
		    g   = colors[idx].green;
		    b   = colors[idx].blue;
		    OUTPAL(idx * 8, r, g, b);
	    }
    } else if (info->CurrentLayout.depth == 16) {
	    /* 16bpp mode.  This sends 64 values.
	     *
	     * There are twice as many green values as there are values
	     * for red and blue.  So, we take each red and blue pair,
	     * and combine it with each of the two green values.
	     */
	    for (i = 0; i < numColors; i++) {
		idx = indices[i];
		r   = colors[idx / 2].red;
		g   = colors[idx].green;
		b   = colors[idx / 2].blue;
		RM6WaitForFifo(pScrn, 32); /* delay */
		OUTPAL(idx * 4, r, g, b);

		/* AH - Added to write extra green data - How come this isn't
		 * needed on R128?  We didn't load the extra green data in the
		 * other routine
		 */
		if (idx <= 31) {
		    r   = colors[idx].red;
		    g   = colors[(idx * 2) + 1].green;
		    b   = colors[idx].blue;
		    RM6WaitForFifo(pScrn, 32); /* delay */
		    OUTPAL(idx * 8, r, g, b);
		}
	    }
	} else {
	    /* 8bpp mode.  This sends 256 values. */
	    for (i = 0; i < numColors; i++) {
		idx = indices[i];
		r   = colors[idx].red;
		b   = colors[idx].blue;
		g   = colors[idx].green;
		RM6WaitForFifo(pScrn, 32); /* delay */
		OUTPAL(idx, r, g, b);
	    }
	}

	if (info->MergedFB) {
	    PAL_SELECT(1);
	    if (info->CurrentLayout.depth == 15) {
		/* 15bpp mode.  This sends 32 values. */
		for (i = 0; i < numColors; i++) {
		    idx = indices[i];
		    r   = colors[idx].red;
		    g   = colors[idx].green;
		    b   = colors[idx].blue;
		    OUTPAL(idx * 8, r, g, b);
		}
	    } else if (info->CurrentLayout.depth == 16) {
		/* 16bpp mode.  This sends 64 values.
		 *
		 * There are twice as many green values as there are values
		 * for red and blue.  So, we take each red and blue pair,
		 * and combine it with each of the two green values.
		 */
		for (i = 0; i < numColors; i++) {
		    idx = indices[i];
		    r   = colors[idx / 2].red;
		    g   = colors[idx].green;
		    b   = colors[idx / 2].blue;
		    OUTPAL(idx * 4, r, g, b);

		    /* AH - Added to write extra green data - How come
		     * this isn't needed on R128?  We didn't load the
		     * extra green data in the other routine.
		     */
		    if (idx <= 31) {
			r   = colors[idx].red;
			g   = colors[(idx * 2) + 1].green;
			b   = colors[idx].blue;
			OUTPAL(idx * 8, r, g, b);
		    }
		}
	    } else {
		/* 8bpp mode.  This sends 256 values. */
		for (i = 0; i < numColors; i++) {
		    idx = indices[i];
		    r   = colors[idx].red;
		    b   = colors[idx].blue;
		    g   = colors[idx].green;
		    OUTPAL(idx, r, g, b);
		}
	    }
	}
    

}

static void RM6BlockHandler(int i, pointer blockData,
			       pointer pTimeout, pointer pReadmask)
{
    ScreenPtr      pScreen = screenInfo.screens[i];
    ScrnInfoPtr    pScrn   = xf86Screens[i];
    RM6InfoPtr  info    = RM6PTR(pScrn);


    pScreen->BlockHandler = info->BlockHandler;
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);
    pScreen->BlockHandler = RM6BlockHandler;

    if (info->VideoTimerCallback)
	(*info->VideoTimerCallback)(pScrn, currentTime.milliseconds);

#ifdef RENDER
    if(info->RenderCallback)
	(*info->RenderCallback)(pScrn);
#endif

}

/* Called at the start of each server generation. */
Bool RM6ScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr    pScrn = xf86Screens[pScreen->myNum];
    RM6InfoPtr  info  = RM6PTR(pScrn);
    BoxRec         MemBox;
    int            y2;
#ifdef RENDER
    int            subPixelOrder = SubPixelUnknown;
    char*          s;
#endif

    RADEONTRACE(("RM6ScreenInit %x %d\n",
		 pScrn->memPhysBase, pScrn->fbOffset));

    info->accelOn      = FALSE;
    pScrn->fbOffset    = 0;
    if (info->IsSecondary) pScrn->fbOffset = pScrn->videoRam * 1024;
    if (!RM6MapMem(pScrn)) return FALSE;


    info->PaletteSavedOnVT = FALSE;

    RM6Save(pScrn);

    if ((!info->IsSecondary) && info->IsMobility) {
        if (xf86ReturnOptValBool(info->Options, OPTION_DYNAMIC_CLOCKS, FALSE)) {
	    RM6SetDynamicClock(pScrn, 1);
        } else {
	    RM6SetDynamicClock(pScrn, 0);
        }
    }

    if (!RM6ModeInit(pScrn, pScrn->currentMode)) return FALSE;

    RM6SaveScreen(pScreen, SCREEN_SAVER_ON);

    pScrn->AdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

				/* Visual setup */
    miClearVisualTypes();
    if (!miSetVisualTypes(pScrn->depth,
			  miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits,
			  pScrn->defaultVisual)) return FALSE;
    miSetPixmapDepths ();


    RM6SetFBLocation(pScrn);

    if (!fbScreenInit(pScreen, info->FB,
		      pScrn->virtualX, pScrn->virtualY,
		      pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
		      pScrn->bitsPerPixel))
	return FALSE;

    xf86SetBlackWhitePixels(pScreen);

    if (pScrn->bitsPerPixel > 8) {
	VisualPtr  visual;

	visual = pScreen->visuals + pScreen->numVisuals;
	while (--visual >= pScreen->visuals) {
	    if ((visual->class | DynamicClass) == DirectColor) {
		visual->offsetRed   = pScrn->offset.red;
		visual->offsetGreen = pScrn->offset.green;
		visual->offsetBlue  = pScrn->offset.blue;
		visual->redMask     = pScrn->mask.red;
		visual->greenMask   = pScrn->mask.green;
		visual->blueMask    = pScrn->mask.blue;
	    }
	}
    }

    /* Must be after RGB order fixed */
    fbPictureInit (pScreen, 0, 0);

#ifdef RENDER
    if ((s = xf86GetOptValString(info->Options, OPTION_SUBPIXEL_ORDER))) {
	if (strcmp(s, "RGB") == 0) subPixelOrder = SubPixelHorizontalRGB;
	else if (strcmp(s, "BGR") == 0) subPixelOrder = SubPixelHorizontalBGR;
	else if (strcmp(s, "NONE") == 0) subPixelOrder = SubPixelNone;
	PictureSetSubpixelOrder (pScreen, subPixelOrder);
    } 

    if (PictureGetSubpixelOrder (pScreen) == SubPixelUnknown) {
	switch (info->DisplayType) {
	case MT_NONE:	subPixelOrder = SubPixelUnknown; break;
	case MT_LCD:	subPixelOrder = SubPixelHorizontalRGB; break;
	case MT_DFP:	subPixelOrder = SubPixelHorizontalRGB; break;
	default:	subPixelOrder = SubPixelNone; break;
	}
	PictureSetSubpixelOrder (pScreen, subPixelOrder);
    }
#endif
				/* Memory manager setup */
    {
	MemBox.x1 = 0;
	MemBox.y1 = 0;
	MemBox.x2 = pScrn->displayWidth;
	y2        = (info->FbMapSize
		     / (pScrn->displayWidth *
			info->CurrentLayout.pixel_bytes));
	if (y2 >= 32768) y2 = 32767; /* because MemBox.y2 is signed short */
	MemBox.y2 = y2;

				/* The acceleration engine uses 14 bit
				   signed coordinates, so we can't have any
				   drawable caches beyond this region. */
	if (MemBox.y2 > 8191) MemBox.y2 = 8191;

	if (!xf86InitFBManager(pScreen, &MemBox)) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Memory manager initialization to "
		       "(%d,%d) (%d,%d) failed\n",
		       MemBox.x1, MemBox.y1, MemBox.x2, MemBox.y2);
	    return FALSE;
	} else {
	    int       width, height;
	    FBAreaPtr fbarea;

	    xf86DrvMsg(scrnIndex, X_INFO,
		       "Memory manager initialized to (%d,%d) (%d,%d)\n",
		       MemBox.x1, MemBox.y1, MemBox.x2, MemBox.y2);
	    if ((fbarea = xf86AllocateOffscreenArea(pScreen,
						    pScrn->displayWidth,
						    2, 0, NULL, NULL,
						    NULL))) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Reserved area from (%d,%d) to (%d,%d)\n",
			   fbarea->box.x1, fbarea->box.y1,
			   fbarea->box.x2, fbarea->box.y2);
	    } else {
		xf86DrvMsg(scrnIndex, X_ERROR, "Unable to reserve area\n");
	    }
	    if (xf86QueryLargestOffscreenArea(pScreen, &width, &height,
					      0, 0, 0)) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Largest offscreen area available: %d x %d\n",
			   width, height);
	    }
	}
    }

				/* Acceleration setup */
    if (!xf86ReturnOptValBool(info->Options, OPTION_NOACCEL, FALSE)) {
	if (RM6AccelInit(pScreen)) {
	    xf86DrvMsg(scrnIndex, X_INFO, "Acceleration enabled\n");
	    info->accelOn = TRUE;
	} else {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Acceleration initialization failed\n");
	    xf86DrvMsg(scrnIndex, X_INFO, "Acceleration disabled\n");
	    info->accelOn = FALSE;
	}
    } else {
	xf86DrvMsg(scrnIndex, X_INFO, "Acceleration disabled\n");
	info->accelOn = FALSE;
    }

				/* DGA setup */
    RM6DGAInit(pScreen);

				/* Backing store setup */
    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);

				/* Set Silken Mouse */
    xf86SetSilkenMouse(pScreen);

				/* Cursor setup */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

				/* Hardware cursor setup */
    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
	if (RM6CursorInit(pScreen)) {
	    int  width, height;

	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Using hardware cursor (scanline %ld)\n",
		       info->cursor_start / pScrn->displayWidth
		       / info->CurrentLayout.pixel_bytes);
	    if (xf86QueryLargestOffscreenArea(pScreen, &width, &height,
					      0, 0, 0)) {
		xf86DrvMsg(scrnIndex, X_INFO,
			   "Largest offscreen area available: %d x %d\n",
			   width, height);
	    }
	} else {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Hardware cursor initialization failed\n");
	    xf86DrvMsg(scrnIndex, X_INFO, "Using software cursor\n");
	}
    } else {
	info->cursor_start = 0;
	xf86DrvMsg(scrnIndex, X_INFO, "Using software cursor\n");
    }

				/* Colormap setup */
    if (!miCreateDefColormap(pScreen)) return FALSE;
    if (!xf86HandleColormaps(pScreen, 256, info->dac6bits ? 6 : 8,
			     RM6LoadPalette, NULL,
			     CMAP_PALETTED_TRUECOLOR
#if 0 /* This option messes up text mode! (eich@suse.de) */
			     | CMAP_LOAD_EVEN_IF_OFFSCREEN
#endif
			     | CMAP_RELOAD_ON_MODE_SWITCH)) return FALSE;

				/* DPMS setup */
    xf86DPMSInit(pScreen, RM6DisplayPowerManagementSet, 0);

    RM6InitVideo(pScreen);

				/* Provide SaveScreen */
    pScreen->SaveScreen  = RM6SaveScreen;

				/* Wrap CloseScreen */
    info->CloseScreen    = pScreen->CloseScreen;
    pScreen->CloseScreen = RM6CloseScreen;

    /* Wrap some funcs for MergedFB */
    if(info->MergedFB) {
       info->PointerMoved = pScrn->PointerMoved;
       pScrn->PointerMoved = RM6MergePointerMoved;
       /* Psuedo xinerama */
       if(info->UseRADEONXinerama) {
          RM6noPanoramiXExtension = FALSE;
          RM6XineramaExtensionInit(pScrn);
       }
    }

				/* Note unused options */
    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);


    info->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = RM6BlockHandler;

    return TRUE;
}

/* Write common registers (initialized to 0) */
static void RM6RestoreCommonRegisters(ScrnInfoPtr pScrn,
					 RM6SavePtr restore)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    OUTREG(RADEON_OVR_CLR,            restore->ovr_clr);
    OUTREG(RADEON_OVR_WID_LEFT_RIGHT, restore->ovr_wid_left_right);
    OUTREG(RADEON_OVR_WID_TOP_BOTTOM, restore->ovr_wid_top_bottom);
    OUTREG(RADEON_OV0_SCALE_CNTL,     restore->ov0_scale_cntl);
    OUTREG(RADEON_SUBPIC_CNTL,        restore->subpic_cntl);
    OUTREG(RADEON_VIPH_CONTROL,       restore->viph_control);
    OUTREG(RADEON_I2C_CNTL_1,         restore->i2c_cntl_1);
    OUTREG(RADEON_GEN_INT_CNTL,       restore->gen_int_cntl);
    OUTREG(RADEON_CAP0_TRIG_CNTL,     restore->cap0_trig_cntl);
    OUTREG(RADEON_CAP1_TRIG_CNTL,     restore->cap1_trig_cntl);
    OUTREG(RADEON_BUS_CNTL,           restore->bus_cntl);
    OUTREG(RADEON_SURFACE_CNTL,       restore->surface_cntl);

    /* Workaround for the VT switching problem in dual-head mode.  This
     * problem only occurs on RV style chips, typically when a FP and
     * CRT are connected.
     */
    if (info->HasCRTC2 &&
	!info->IsSwitching &&
	info->ChipFamily != CHIP_FAMILY_R200 &&
	!IS_R300_VARIANT) {
	CARD32        tmp;
        RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);

	if (pRM6Ent->HasSecondary || info->MergedFB) {
	    tmp = INREG(RADEON_DAC_CNTL2);
	    OUTREG(RADEON_DAC_CNTL2, tmp & ~RADEON_DAC2_DAC_CLK_SEL);
	    usleep(100000);
	}
    }
}


/* Write CRTC registers */
static void RM6RestoreCrtcRegisters(ScrnInfoPtr pScrn,
				       RM6SavePtr restore)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    OUTREG(RADEON_CRTC_GEN_CNTL, restore->crtc_gen_cntl);

    OUTREGP(RADEON_CRTC_EXT_CNTL,
	    restore->crtc_ext_cntl,
	    RADEON_CRTC_VSYNC_DIS |
	    RADEON_CRTC_HSYNC_DIS |
	    RADEON_CRTC_DISPLAY_DIS);

    OUTREGP(RADEON_DAC_CNTL,
	    restore->dac_cntl,
	    RADEON_DAC_RANGE_CNTL |
	    RADEON_DAC_BLANKING);

    OUTREG(RADEON_CRTC_H_TOTAL_DISP,    restore->crtc_h_total_disp);
    OUTREG(RADEON_CRTC_H_SYNC_STRT_WID, restore->crtc_h_sync_strt_wid);
    OUTREG(RADEON_CRTC_V_TOTAL_DISP,    restore->crtc_v_total_disp);
    OUTREG(RADEON_CRTC_V_SYNC_STRT_WID, restore->crtc_v_sync_strt_wid);
    OUTREG(RADEON_CRTC_OFFSET,          restore->crtc_offset);
    OUTREG(RADEON_CRTC_OFFSET_CNTL,     restore->crtc_offset_cntl);
    OUTREG(RADEON_CRTC_PITCH,           restore->crtc_pitch);
    OUTREG(RADEON_DISP_MERGE_CNTL,      restore->disp_merge_cntl);
    OUTREG(RADEON_CRTC_MORE_CNTL,       restore->crtc_more_cntl);

    if (info->IsDellServer) {
	OUTREG(RADEON_TV_DAC_CNTL, restore->tv_dac_cntl);
	OUTREG(RADEON_DISP_HW_DEBUG, restore->disp_hw_debug);
	OUTREG(RADEON_DAC_CNTL2, restore->dac2_cntl);
	OUTREG(RADEON_CRTC2_GEN_CNTL, restore->crtc2_gen_cntl);
    }
}

/* Write CRTC2 registers */
static void RM6RestoreCrtc2Registers(ScrnInfoPtr pScrn,
					RM6SavePtr restore)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    OUTREGP(RADEON_CRTC2_GEN_CNTL,
	    restore->crtc2_gen_cntl,
	    RADEON_CRTC2_VSYNC_DIS |
	    RADEON_CRTC2_HSYNC_DIS |
	    RADEON_CRTC2_DISP_DIS);

    OUTREG(RADEON_DAC_CNTL2, restore->dac2_cntl);

    OUTREG(RADEON_TV_DAC_CNTL, 0x00280203);
    if ((info->ChipFamily == CHIP_FAMILY_R200) ||
	IS_R300_VARIANT) {
	OUTREG(RADEON_DISP_OUTPUT_CNTL, restore->disp_output_cntl);
    } else {
	OUTREG(RADEON_DISP_HW_DEBUG, restore->disp_hw_debug);
    }

    OUTREG(RADEON_CRTC2_H_TOTAL_DISP,    restore->crtc2_h_total_disp);
    OUTREG(RADEON_CRTC2_H_SYNC_STRT_WID, restore->crtc2_h_sync_strt_wid);
    OUTREG(RADEON_CRTC2_V_TOTAL_DISP,    restore->crtc2_v_total_disp);
    OUTREG(RADEON_CRTC2_V_SYNC_STRT_WID, restore->crtc2_v_sync_strt_wid);
    OUTREG(RADEON_CRTC2_OFFSET,          restore->crtc2_offset);
    OUTREG(RADEON_CRTC2_OFFSET_CNTL,     restore->crtc2_offset_cntl);
    OUTREG(RADEON_CRTC2_PITCH,           restore->crtc2_pitch);
    OUTREG(RADEON_DISP2_MERGE_CNTL,      restore->disp2_merge_cntl);

    if ((info->DisplayType == MT_DFP && info->IsSecondary) || 
	info->MergeType == MT_DFP) {	
	OUTREG(RADEON_FP_H2_SYNC_STRT_WID, restore->fp2_h_sync_strt_wid);
	OUTREG(RADEON_FP_V2_SYNC_STRT_WID, restore->fp2_v_sync_strt_wid);
	OUTREG(RADEON_FP2_GEN_CNTL,        restore->fp2_gen_cntl);
    }
#if 0
    /* Hack for restoring text mode -- fixed elsewhere */
    usleep(100000);
#endif
}

/* Write flat panel registers */
static void RM6RestoreFPRegisters(ScrnInfoPtr pScrn, RM6SavePtr restore)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    unsigned long  tmp;

    OUTREG(RADEON_FP_CRTC_H_TOTAL_DISP, restore->fp_crtc_h_total_disp);
    OUTREG(RADEON_FP_CRTC_V_TOTAL_DISP, restore->fp_crtc_v_total_disp);
    OUTREG(RADEON_FP_H_SYNC_STRT_WID,   restore->fp_h_sync_strt_wid);
    OUTREG(RADEON_FP_V_SYNC_STRT_WID,   restore->fp_v_sync_strt_wid);
    OUTREG(RADEON_TMDS_PLL_CNTL,        restore->tmds_pll_cntl);
    OUTREG(RADEON_TMDS_TRANSMITTER_CNTL,restore->tmds_transmitter_cntl);
    OUTREG(RADEON_FP_HORZ_STRETCH,      restore->fp_horz_stretch);
    OUTREG(RADEON_FP_VERT_STRETCH,      restore->fp_vert_stretch);
    OUTREG(RADEON_FP_GEN_CNTL,          restore->fp_gen_cntl);

    /* old AIW Radeon has some BIOS initialization problem
     * with display buffer underflow, only occurs to DFP
     */
    if (!info->HasCRTC2)
	OUTREG(RADEON_GRPH_BUFFER_CNTL,
	       INREG(RADEON_GRPH_BUFFER_CNTL) & ~0x7f0000);

    if (info->IsMobility) {
	OUTREG(RADEON_BIOS_4_SCRATCH, restore->bios_4_scratch);
	OUTREG(RADEON_BIOS_5_SCRATCH, restore->bios_5_scratch);
	OUTREG(RADEON_BIOS_6_SCRATCH, restore->bios_6_scratch);
    }

    if (info->DisplayType != MT_DFP) {
	unsigned long tmpPixclksCntl = INPLL(pScrn, RADEON_PIXCLKS_CNTL);

	if (info->IsMobility || info->IsIGP) {
	    /* Asic bug, when turning off LVDS_ON, we have to make sure
	       RADEON_PIXCLK_LVDS_ALWAYS_ON bit is off
	    */
	    if (!(restore->lvds_gen_cntl & RADEON_LVDS_ON)) {
		OUTPLLP(pScrn, RADEON_PIXCLKS_CNTL, 0, ~RADEON_PIXCLK_LVDS_ALWAYS_ONb);
	    }
	}

	tmp = INREG(RADEON_LVDS_GEN_CNTL);
	if ((tmp & (RADEON_LVDS_ON | RADEON_LVDS_BLON)) ==
	    (restore->lvds_gen_cntl & (RADEON_LVDS_ON | RADEON_LVDS_BLON))) {
	    OUTREG(RADEON_LVDS_GEN_CNTL, restore->lvds_gen_cntl);
	} else {
	    if (restore->lvds_gen_cntl & (RADEON_LVDS_ON | RADEON_LVDS_BLON)) {
		usleep(RM6PTR(pScrn)->PanelPwrDly * 1000);
		OUTREG(RADEON_LVDS_GEN_CNTL, restore->lvds_gen_cntl);
	    } else {
		OUTREG(RADEON_LVDS_GEN_CNTL,
		       restore->lvds_gen_cntl | RADEON_LVDS_BLON);
		usleep(RM6PTR(pScrn)->PanelPwrDly * 1000);
		OUTREG(RADEON_LVDS_GEN_CNTL, restore->lvds_gen_cntl);
	    }
	}

	if (info->IsMobility || info->IsIGP) {
	    if (!(restore->lvds_gen_cntl & RADEON_LVDS_ON)) {
		OUTPLL(RADEON_PIXCLKS_CNTL, tmpPixclksCntl);
	    }
	}
    }
}

static void RM6PLLWaitForReadUpdateComplete(ScrnInfoPtr pScrn)
{
    int i = 0;

    /* FIXME: Certain revisions of R300 can't recover here.  Not sure of
       the cause yet, but this workaround will mask the problem for now.
       Other chips usually will pass at the very first test, so the
       workaround shouldn't have any effect on them. */
    for (i = 0;
	 (i < 10000 &&
	  INPLL(pScrn, RADEON_PPLL_REF_DIV) & RADEON_PPLL_ATOMIC_UPDATE_R);
	 i++);
}

static void RM6PLLWriteUpdate(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    while (INPLL(pScrn, RADEON_PPLL_REF_DIV) & RADEON_PPLL_ATOMIC_UPDATE_R);

    OUTPLLP(pScrn, RADEON_PPLL_REF_DIV,
	    RADEON_PPLL_ATOMIC_UPDATE_W,
	    ~(RADEON_PPLL_ATOMIC_UPDATE_W));
}

static void RM6PLL2WaitForReadUpdateComplete(ScrnInfoPtr pScrn)
{
    int i = 0;

    /* FIXME: Certain revisions of R300 can't recover here.  Not sure of
       the cause yet, but this workaround will mask the problem for now.
       Other chips usually will pass at the very first test, so the
       workaround shouldn't have any effect on them. */
    for (i = 0;
	 (i < 10000 &&
	  INPLL(pScrn, RADEON_P2PLL_REF_DIV) & RADEON_P2PLL_ATOMIC_UPDATE_R);
	 i++);
}

static void RM6PLL2WriteUpdate(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    while (INPLL(pScrn, RADEON_P2PLL_REF_DIV) & RADEON_P2PLL_ATOMIC_UPDATE_R);

    OUTPLLP(pScrn, RADEON_P2PLL_REF_DIV,
	    RADEON_P2PLL_ATOMIC_UPDATE_W,
	    ~(RADEON_P2PLL_ATOMIC_UPDATE_W));
}

/* Write PLL registers */
static void RM6RestorePLLRegisters(ScrnInfoPtr pScrn,
				      RM6SavePtr restore)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    /* 
     * Never do it on Apple iBook to avoid a blank screen.
     */
#ifdef __powerpc__
    if (xf86ReturnOptValBool(info->Options, OPTION_IBOOKHACKS, FALSE))
        return;
#endif

    if (info->IsMobility) {
        /* A temporal workaround for the occational blanking on certain laptop panels.
           This appears to related to the PLL divider registers (fail to lock?).
	   It occurs even when all dividers are the same with their old settings.
           In this case we really don't need to fiddle with PLL registers.
           By doing this we can avoid the blanking problem with some panels.
        */
        if ((restore->ppll_ref_div == (INPLL(pScrn, RADEON_PPLL_REF_DIV) & RADEON_PPLL_REF_DIV_MASK)) &&
	    (restore->ppll_div_3 == (INPLL(pScrn, RADEON_PPLL_DIV_3) & (RADEON_PPLL_POST3_DIV_MASK | RADEON_PPLL_FB3_DIV_MASK))))
            return;
    }

    OUTPLLP(pScrn, RADEON_VCLK_ECP_CNTL,
	    RADEON_VCLK_SRC_SEL_CPUCLK,
	    ~(RADEON_VCLK_SRC_SEL_MASK));

    OUTPLLP(pScrn,
	    RADEON_PPLL_CNTL,
	    RADEON_PPLL_RESET
	    | RADEON_PPLL_ATOMIC_UPDATE_EN
	    | RADEON_PPLL_VGA_ATOMIC_UPDATE_EN,
	    ~(RADEON_PPLL_RESET
	      | RADEON_PPLL_ATOMIC_UPDATE_EN
	      | RADEON_PPLL_VGA_ATOMIC_UPDATE_EN));

    OUTREGP(RADEON_CLOCK_CNTL_INDEX,
	    RADEON_PLL_DIV_SEL,
	    ~(RADEON_PLL_DIV_SEL));

    if (IS_R300_VARIANT ||
	(info->ChipFamily == CHIP_FAMILY_RS300)) {
	if (restore->ppll_ref_div & R300_PPLL_REF_DIV_ACC_MASK) {
	    /* When restoring console mode, use saved PPLL_REF_DIV
	     * setting.
	     */
	    OUTPLLP(pScrn, RADEON_PPLL_REF_DIV,
		    restore->ppll_ref_div,
		    0);
	} else {
	    /* R300 uses ref_div_acc field as real ref divider */
	    OUTPLLP(pScrn, RADEON_PPLL_REF_DIV,
		    (restore->ppll_ref_div << R300_PPLL_REF_DIV_ACC_SHIFT),
		    ~R300_PPLL_REF_DIV_ACC_MASK);
	}
    } else {
	OUTPLLP(pScrn, RADEON_PPLL_REF_DIV,
		restore->ppll_ref_div,
		~RADEON_PPLL_REF_DIV_MASK);
    }

    OUTPLLP(pScrn, RADEON_PPLL_DIV_3,
	    restore->ppll_div_3,
	    ~RADEON_PPLL_FB3_DIV_MASK);

    OUTPLLP(pScrn, RADEON_PPLL_DIV_3,
	    restore->ppll_div_3,
	    ~RADEON_PPLL_POST3_DIV_MASK);

    RM6PLLWriteUpdate(pScrn);
    RM6PLLWaitForReadUpdateComplete(pScrn);

    OUTPLL(RADEON_HTOTAL_CNTL, restore->htotal_cntl);

    OUTPLLP(pScrn, RADEON_PPLL_CNTL,
	    0,
	    ~(RADEON_PPLL_RESET
	      | RADEON_PPLL_SLEEP
	      | RADEON_PPLL_ATOMIC_UPDATE_EN
	      | RADEON_PPLL_VGA_ATOMIC_UPDATE_EN));

    RADEONTRACE(("Wrote: 0x%08x 0x%08x 0x%08x (0x%08x)\n",
	       restore->ppll_ref_div,
	       restore->ppll_div_3,
	       restore->htotal_cntl,
	       INPLL(pScrn, RADEON_PPLL_CNTL)));
    RADEONTRACE(("Wrote: rd=%d, fd=%d, pd=%d\n",
	       restore->ppll_ref_div & RADEON_PPLL_REF_DIV_MASK,
	       restore->ppll_div_3 & RADEON_PPLL_FB3_DIV_MASK,
	       (restore->ppll_div_3 & RADEON_PPLL_POST3_DIV_MASK) >> 16));

    usleep(50000); /* Let the clock to lock */

    OUTPLLP(pScrn, RADEON_VCLK_ECP_CNTL,
	    RADEON_VCLK_SRC_SEL_PPLLCLK,
	    ~(RADEON_VCLK_SRC_SEL_MASK));
}


/* Write PLL2 registers */
static void RM6RestorePLL2Registers(ScrnInfoPtr pScrn,
				       RM6SavePtr restore)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    OUTPLLP(pScrn, RADEON_PIXCLKS_CNTL,
	    RADEON_PIX2CLK_SRC_SEL_CPUCLK,
	    ~(RADEON_PIX2CLK_SRC_SEL_MASK));

    OUTPLLP(pScrn,
	    RADEON_P2PLL_CNTL,
	    RADEON_P2PLL_RESET
	    | RADEON_P2PLL_ATOMIC_UPDATE_EN
	    | RADEON_P2PLL_VGA_ATOMIC_UPDATE_EN,
	    ~(RADEON_P2PLL_RESET
	      | RADEON_P2PLL_ATOMIC_UPDATE_EN
	      | RADEON_P2PLL_VGA_ATOMIC_UPDATE_EN));

    OUTPLLP(pScrn, RADEON_P2PLL_REF_DIV,
	    restore->p2pll_ref_div,
	    ~RADEON_P2PLL_REF_DIV_MASK);

    OUTPLLP(pScrn, RADEON_P2PLL_DIV_0,
	    restore->p2pll_div_0,
	    ~RADEON_P2PLL_FB0_DIV_MASK);

    OUTPLLP(pScrn, RADEON_P2PLL_DIV_0,
	    restore->p2pll_div_0,
	    ~RADEON_P2PLL_POST0_DIV_MASK);

    RM6PLL2WriteUpdate(pScrn);
    RM6PLL2WaitForReadUpdateComplete(pScrn);

    OUTPLL(RADEON_HTOTAL2_CNTL, restore->htotal_cntl2);

    OUTPLLP(pScrn, RADEON_P2PLL_CNTL,
	    0,
	    ~(RADEON_P2PLL_RESET
	      | RADEON_P2PLL_SLEEP
	      | RADEON_P2PLL_ATOMIC_UPDATE_EN
	      | RADEON_P2PLL_VGA_ATOMIC_UPDATE_EN));

    RADEONTRACE(("Wrote: 0x%08x 0x%08x 0x%08x (0x%08x)\n",
	       restore->p2pll_ref_div,
	       restore->p2pll_div_0,
	       restore->htotal_cntl2,
	       INPLL(pScrn, RADEON_P2PLL_CNTL)));
    RADEONTRACE(("Wrote: rd=%d, fd=%d, pd=%d\n",
	       restore->p2pll_ref_div & RADEON_P2PLL_REF_DIV_MASK,
	       restore->p2pll_div_0 & RADEON_P2PLL_FB0_DIV_MASK,
	       (restore->p2pll_div_0 & RADEON_P2PLL_POST0_DIV_MASK) >>16));

    usleep(5000); /* Let the clock to lock */

    OUTPLLP(pScrn, RADEON_PIXCLKS_CNTL,
	    RADEON_PIX2CLK_SRC_SEL_P2PLLCLK,
	    ~(RADEON_PIX2CLK_SRC_SEL_MASK));
}


/* Write out state to define a new video mode */
static void RM6RestoreMode(ScrnInfoPtr pScrn, RM6SavePtr restore)
{
    RM6InfoPtr      info = RM6PTR(pScrn);
    RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);
    static RM6SaveRec  restore0;

    /* For Non-dual head card, we don't have private field in the Entity */
    if (!info->HasCRTC2) {
	RM6RestoreCommonRegisters(pScrn, restore);
	RM6RestoreCrtcRegisters(pScrn, restore);
	RM6RestoreFPRegisters(pScrn, restore);
	RM6RestorePLLRegisters(pScrn, restore);
	return;
    }

    RADEONTRACE(("RM6RestoreMode(%p)\n", restore));

    /* When changing mode with Dual-head card, care must be taken for
     * the special order in setting registers. CRTC2 has to be set
     * before changing CRTC_EXT register.  In the dual-head setup, X
     * server calls this routine twice with primary and secondary pScrn
     * pointers respectively. The calls can come with different
     * order. Regardless the order of X server issuing the calls, we
     * have to ensure we set registers in the right order!!!  Otherwise
     * we may get a blank screen.
     */
    if (info->IsSecondary) {
	if (!pRM6Ent->RestorePrimary  && !info->IsSwitching)
	    RM6RestoreCommonRegisters(pScrn, restore);
	RM6RestoreCrtc2Registers(pScrn, restore);
	RM6RestorePLL2Registers(pScrn, restore);

	if(info->IsSwitching) return;

	pRM6Ent->IsSecondaryRestored = TRUE;

	if (pRM6Ent->RestorePrimary) {
	    pRM6Ent->RestorePrimary = FALSE;

	    RM6RestoreCrtcRegisters(pScrn, &restore0);
	    RM6RestoreFPRegisters(pScrn, &restore0);
	    RM6RestorePLLRegisters(pScrn, &restore0);
	    pRM6Ent->IsSecondaryRestored = FALSE;
	}
    } else {
	if (!pRM6Ent->IsSecondaryRestored)
	    RM6RestoreCommonRegisters(pScrn, restore);

	if (info->MergedFB) {
	    RM6RestoreCrtc2Registers(pScrn, restore);
	    RM6RestorePLL2Registers(pScrn, restore);
	}

	if (!pRM6Ent->HasSecondary || pRM6Ent->IsSecondaryRestored ||
	    info->IsSwitching) {
	    pRM6Ent->IsSecondaryRestored = FALSE;

	    RM6RestoreCrtcRegisters(pScrn, restore);
	    RM6RestoreFPRegisters(pScrn, restore);
	    RM6RestorePLLRegisters(pScrn, restore);
	} else {
	    memcpy(&restore0, restore, sizeof(restore0));
	    pRM6Ent->RestorePrimary = TRUE;
	}
    }

}

/* Read common registers */
static void RM6SaveCommonRegisters(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    save->ovr_clr            = INREG(RADEON_OVR_CLR);
    save->ovr_wid_left_right = INREG(RADEON_OVR_WID_LEFT_RIGHT);
    save->ovr_wid_top_bottom = INREG(RADEON_OVR_WID_TOP_BOTTOM);
    save->ov0_scale_cntl     = INREG(RADEON_OV0_SCALE_CNTL);
    save->subpic_cntl        = INREG(RADEON_SUBPIC_CNTL);
    save->viph_control       = INREG(RADEON_VIPH_CONTROL);
    save->i2c_cntl_1         = INREG(RADEON_I2C_CNTL_1);
    save->gen_int_cntl       = INREG(RADEON_GEN_INT_CNTL);
    save->cap0_trig_cntl     = INREG(RADEON_CAP0_TRIG_CNTL);
    save->cap1_trig_cntl     = INREG(RADEON_CAP1_TRIG_CNTL);
    save->bus_cntl           = INREG(RADEON_BUS_CNTL);
    save->surface_cntl	     = INREG(RADEON_SURFACE_CNTL);
    save->grph_buffer_cntl   = INREG(RADEON_GRPH_BUFFER_CNTL);
    save->grph2_buffer_cntl  = INREG(RADEON_GRPH2_BUFFER_CNTL);
}


/* Read CRTC registers */
static void RM6SaveCrtcRegisters(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    save->crtc_gen_cntl        = INREG(RADEON_CRTC_GEN_CNTL);
    save->crtc_ext_cntl        = INREG(RADEON_CRTC_EXT_CNTL);
    save->dac_cntl             = INREG(RADEON_DAC_CNTL);
    save->crtc_h_total_disp    = INREG(RADEON_CRTC_H_TOTAL_DISP);
    save->crtc_h_sync_strt_wid = INREG(RADEON_CRTC_H_SYNC_STRT_WID);
    save->crtc_v_total_disp    = INREG(RADEON_CRTC_V_TOTAL_DISP);
    save->crtc_v_sync_strt_wid = INREG(RADEON_CRTC_V_SYNC_STRT_WID);
    save->crtc_offset          = INREG(RADEON_CRTC_OFFSET);
    save->crtc_offset_cntl     = INREG(RADEON_CRTC_OFFSET_CNTL);
    save->crtc_pitch           = INREG(RADEON_CRTC_PITCH);
    save->disp_merge_cntl      = INREG(RADEON_DISP_MERGE_CNTL);
    save->crtc_more_cntl       = INREG(RADEON_CRTC_MORE_CNTL);

    if (info->IsDellServer) {
	save->tv_dac_cntl      = INREG(RADEON_TV_DAC_CNTL);
	save->dac2_cntl        = INREG(RADEON_DAC_CNTL2);
	save->disp_hw_debug    = INREG (RADEON_DISP_HW_DEBUG);
	save->crtc2_gen_cntl   = INREG(RADEON_CRTC2_GEN_CNTL);
    }
}

/* Read flat panel registers */
static void RM6SaveFPRegisters(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    save->fp_crtc_h_total_disp = INREG(RADEON_FP_CRTC_H_TOTAL_DISP);
    save->fp_crtc_v_total_disp = INREG(RADEON_FP_CRTC_V_TOTAL_DISP);
    save->fp_gen_cntl          = INREG(RADEON_FP_GEN_CNTL);
    save->fp_h_sync_strt_wid   = INREG(RADEON_FP_H_SYNC_STRT_WID);
    save->fp_horz_stretch      = INREG(RADEON_FP_HORZ_STRETCH);
    save->fp_v_sync_strt_wid   = INREG(RADEON_FP_V_SYNC_STRT_WID);
    save->fp_vert_stretch      = INREG(RADEON_FP_VERT_STRETCH);
    save->lvds_gen_cntl        = INREG(RADEON_LVDS_GEN_CNTL);
    save->lvds_pll_cntl        = INREG(RADEON_LVDS_PLL_CNTL);
    save->tmds_pll_cntl        = INREG(RADEON_TMDS_PLL_CNTL);
    save->tmds_transmitter_cntl= INREG(RADEON_TMDS_TRANSMITTER_CNTL);
    save->bios_4_scratch       = INREG(RADEON_BIOS_4_SCRATCH);
    save->bios_5_scratch       = INREG(RADEON_BIOS_5_SCRATCH);
    save->bios_6_scratch       = INREG(RADEON_BIOS_6_SCRATCH);

    if (info->ChipFamily == CHIP_FAMILY_RV280) {
	/* bit 22 of TMDS_PLL_CNTL is read-back inverted */
	save->tmds_pll_cntl ^= (1 << 22);
    }
}

/* Read CRTC2 registers */
static void RM6SaveCrtc2Registers(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    save->dac2_cntl             = INREG(RADEON_DAC_CNTL2);
    save->disp_output_cntl      = INREG(RADEON_DISP_OUTPUT_CNTL);
    save->disp_hw_debug         = INREG (RADEON_DISP_HW_DEBUG);

    save->crtc2_gen_cntl        = INREG(RADEON_CRTC2_GEN_CNTL);
    save->crtc2_h_total_disp    = INREG(RADEON_CRTC2_H_TOTAL_DISP);
    save->crtc2_h_sync_strt_wid = INREG(RADEON_CRTC2_H_SYNC_STRT_WID);
    save->crtc2_v_total_disp    = INREG(RADEON_CRTC2_V_TOTAL_DISP);
    save->crtc2_v_sync_strt_wid = INREG(RADEON_CRTC2_V_SYNC_STRT_WID);
    save->crtc2_offset          = INREG(RADEON_CRTC2_OFFSET);
    save->crtc2_offset_cntl     = INREG(RADEON_CRTC2_OFFSET_CNTL);
    save->crtc2_pitch           = INREG(RADEON_CRTC2_PITCH);

    save->fp2_h_sync_strt_wid   = INREG (RADEON_FP_H2_SYNC_STRT_WID);
    save->fp2_v_sync_strt_wid   = INREG (RADEON_FP_V2_SYNC_STRT_WID);
    save->fp2_gen_cntl          = INREG (RADEON_FP2_GEN_CNTL);
    save->disp2_merge_cntl      = INREG(RADEON_DISP2_MERGE_CNTL);
}

/* Read PLL registers */
static void RM6SavePLLRegisters(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    save->ppll_ref_div = INPLL(pScrn, RADEON_PPLL_REF_DIV);
    save->ppll_div_3   = INPLL(pScrn, RADEON_PPLL_DIV_3);
    save->htotal_cntl  = INPLL(pScrn, RADEON_HTOTAL_CNTL);

    RADEONTRACE(("Read: 0x%08x 0x%08x 0x%08x\n",
		 save->ppll_ref_div,
		 save->ppll_div_3,
		 save->htotal_cntl));
    RADEONTRACE(("Read: rd=%d, fd=%d, pd=%d\n",
		 save->ppll_ref_div & RADEON_PPLL_REF_DIV_MASK,
		 save->ppll_div_3 & RADEON_PPLL_FB3_DIV_MASK,
		 (save->ppll_div_3 & RADEON_PPLL_POST3_DIV_MASK) >> 16));
}

/* Read PLL registers */
static void RM6SavePLL2Registers(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    save->p2pll_ref_div = INPLL(pScrn, RADEON_P2PLL_REF_DIV);
    save->p2pll_div_0   = INPLL(pScrn, RADEON_P2PLL_DIV_0);
    save->htotal_cntl2  = INPLL(pScrn, RADEON_HTOTAL2_CNTL);

    RADEONTRACE(("Read: 0x%08x 0x%08x 0x%08x\n",
		 save->p2pll_ref_div,
		 save->p2pll_div_0,
		 save->htotal_cntl2));
    RADEONTRACE(("Read: rd=%d, fd=%d, pd=%d\n",
		 save->p2pll_ref_div & RADEON_P2PLL_REF_DIV_MASK,
		 save->p2pll_div_0 & RADEON_P2PLL_FB0_DIV_MASK,
		 (save->p2pll_div_0 & RADEON_P2PLL_POST0_DIV_MASK) >> 16));
}

/* Save state that defines current video mode */
static void RM6SaveMode(ScrnInfoPtr pScrn, RM6SavePtr save)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    RADEONTRACE(("RM6SaveMode(%p)\n", save));
    RM6SaveCommonRegisters(pScrn, save);
    if (info->IsSecondary) {
	RM6SaveCrtc2Registers(pScrn, save);
	RM6SavePLL2Registers(pScrn, save);
    } else {
	RM6SavePLLRegisters(pScrn, save);
	RM6SaveCrtcRegisters(pScrn, save);
	RM6SaveFPRegisters(pScrn, save);

	if (info->MergedFB) {
	    RM6SaveCrtc2Registers(pScrn, save);
	    RM6SavePLL2Registers(pScrn, save);
	}
     /* RM6SavePalette(pScrn, save); */
    }

    RADEONTRACE(("RM6SaveMode returns %p\n", save));
}

/* Save everything needed to restore the original VC state */
static void RM6Save(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    RM6SavePtr  save       = &info->SavedReg;
    vgaHWPtr       hwp        = VGAHWPTR(pScrn);

    RADEONTRACE(("RM6Save\n"));

    if (!info->IsSecondary) {
	vgaHWUnlock(hwp);
#if defined(__powerpc__)
	/* temporary hack to prevent crashing on PowerMacs when trying to
	 * read VGA fonts and colormap, will find a better solution
	 * in the future
	 */
	vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_MODE); /* Save mode only */
#else
	vgaHWSave(pScrn, &hwp->SavedReg, VGA_SR_ALL); /* Save mode
						       * & fonts & cmap
						       */
#endif
	vgaHWLock(hwp);
	save->dp_datatype      = INREG(RADEON_DP_DATATYPE);
	save->rbbm_soft_reset  = INREG(RADEON_RBBM_SOFT_RESET);
	save->clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
	if (info->R300CGWorkaround) R300CGWorkaround(pScrn);
    }

    RM6SaveMode(pScrn, save);
}

/* Restore the original (text) mode */
static void RM6Restore(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    RM6SavePtr  restore    = &info->SavedReg;
    vgaHWPtr       hwp        = VGAHWPTR(pScrn);

    RADEONTRACE(("RM6Restore\n"));

#if X_BYTE_ORDER == X_BIG_ENDIAN
    RM6WaitForFifo(pScrn, 1);
    OUTREG(RADEON_RBBM_GUICNTL, RADEON_HOST_DATA_SWAP_NONE);
#endif

    RM6Blank(pScrn);

    OUTREG(RADEON_CLOCK_CNTL_INDEX, restore->clock_cntl_index);
    if (info->R300CGWorkaround) R300CGWorkaround(pScrn);
    OUTREG(RADEON_RBBM_SOFT_RESET,  restore->rbbm_soft_reset);
    OUTREG(RADEON_DP_DATATYPE,      restore->dp_datatype);
    OUTREG(RADEON_GRPH_BUFFER_CNTL, restore->grph_buffer_cntl);
    OUTREG(RADEON_GRPH2_BUFFER_CNTL, restore->grph2_buffer_cntl);

#if 0
    /* M6 card has trouble restoring text mode for its CRT.
     * This is fixed elsewhere and will be removed in the future.
     */
    if ((xf86IsEntityShared(info->pEnt->index) || info->MergedFB)
	&& info->IsM6)
	OUTREG(RADEON_DAC_CNTL2, restore->dac2_cntl);
#endif

    RM6RestoreMode(pScrn, restore);

#if 0
    /* Temp fix to "solve" VT switch problems.  When switching VTs on
     * some systems, the console can either hang or the fonts can be
     * corrupted.  This hack solves the problem 99% of the time.  A
     * correct fix is being worked on.
     */
    usleep(100000);
#endif

    if (!info->IsSecondary) {
	vgaHWUnlock(hwp);
#if defined(__powerpc__)
	/* Temporary hack to prevent crashing on PowerMacs when trying to
	 * write VGA fonts, will find a better solution in the future
	 */
	vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_MODE );
#else
	vgaHWRestore(pScrn, &hwp->SavedReg, VGA_SR_ALL );
#endif
	vgaHWLock(hwp);
    } else {
        RM6EntPtr pRM6Ent = RM6EntPriv(pScrn);
	ScrnInfoPtr   pScrn0;
	vgaHWPtr      hwp0;

	pScrn0 = pRM6Ent->pPrimaryScrn;
	hwp0   = VGAHWPTR(pScrn0);
	vgaHWUnlock(hwp0);
	vgaHWRestore(pScrn0, &hwp0->SavedReg, VGA_SR_MODE | VGA_SR_FONTS );
	vgaHWLock(hwp0);
    }
    RM6Unblank(pScrn);

#if 0
    RM6WaitForVerticalSync(pScrn);
#endif
}

/* Define common registers for requested video mode */
static void RM6InitCommonRegisters(RM6SavePtr save, RM6InfoPtr info)
{
    save->ovr_clr            = 0;
    save->ovr_wid_left_right = 0;
    save->ovr_wid_top_bottom = 0;
    save->ov0_scale_cntl     = 0;
    save->subpic_cntl        = 0;
    save->viph_control       = 0;
    save->i2c_cntl_1         = 0;
    save->rbbm_soft_reset    = 0;
    save->cap0_trig_cntl     = 0;
    save->cap1_trig_cntl     = 0;
    save->bus_cntl           = info->BusCntl;
    /*
     * If bursts are enabled, turn on discards
     * Radeon doesn't have write bursts
     */
    if (save->bus_cntl & (RADEON_BUS_READ_BURST))
	save->bus_cntl |= RADEON_BUS_RD_DISCARD_EN;
}


/* Calculate display buffer watermark to prevent buffer underflow */
static void RM6InitDispBandwidth(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    RM6EntPtr pRM6Ent   = RM6EntPriv(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    RM6InfoPtr  info2 = NULL;

    DisplayModePtr mode1, mode2;

    CARD32 temp, data, mem_trcd, mem_trp, mem_tras, mem_trbs=0;
    float mem_tcas;
    int k1, c;
    CARD32 MemTrcdExtMemCntl[4]     = {1, 2, 3, 4};
    CARD32 MemTrpExtMemCntl[4]      = {1, 2, 3, 4};
    CARD32 MemTrasExtMemCntl[8]     = {1, 2, 3, 4, 5, 6, 7, 8};

    CARD32 MemTrcdMemTimingCntl[8]     = {1, 2, 3, 4, 5, 6, 7, 8};
    CARD32 MemTrpMemTimingCntl[8]      = {1, 2, 3, 4, 5, 6, 7, 8};
    CARD32 MemTrasMemTimingCntl[16]    = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

    float MemTcas[8]  = {0, 1, 2, 3, 0, 1.5, 2.5, 0};
    float MemTcas2[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    float MemTrbs[8]  = {1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5};

    float mem_bw, peak_disp_bw;
    float min_mem_eff = 0.8;
    float sclk_eff, sclk_delay;
    float mc_latency_mclk, mc_latency_sclk, cur_latency_mclk, cur_latency_sclk;
    float disp_latency, disp_latency_overhead, disp_drain_rate, disp_drain_rate2;
    float pix_clk, pix_clk2; /* in MHz */
    int cur_size = 16;       /* in octawords */
    int critical_point, critical_point2;
    int stop_req, max_stop_req;
    float read_return_rate, time_disp1_drop_priority;

    /* R420 family not supported yet */
    if (info->ChipFamily == CHIP_FAMILY_R420) return; 

    if (pRM6Ent->pSecondaryScrn) {
	if (info->IsSecondary) return;
	info2 = RM6PTR(pRM6Ent->pSecondaryScrn);
    }  else if (info->MergedFB) info2 = info;

    /*
     * Determine if there is enough bandwidth for current display mode
     */
    mem_bw = info->mclk * (info->RamWidth / 8) * (info->IsDDR ? 2 : 1);

    mode1 = info->CurrentLayout.mode;
    if (info->MergedFB) {
	mode1 = ((RM6MergedDisplayModePtr)info->CurrentLayout.mode->Private)->CRT1;
	mode2 = ((RM6MergedDisplayModePtr)info->CurrentLayout.mode->Private)->CRT2;
    } else if ((pRM6Ent->HasSecondary) && info2) {
	mode2 = info2->CurrentLayout.mode;
    } else {
	mode2 = NULL;
    }

    pix_clk = mode1->Clock/1000.0;
    if (mode2)
	pix_clk2 = mode2->Clock/1000.0;
    else
	pix_clk2 = 0;

    peak_disp_bw = (pix_clk * info->CurrentLayout.pixel_bytes);
    if (info2) 
	peak_disp_bw +=	(pix_clk2 * info2->CurrentLayout.pixel_bytes);

    if (peak_disp_bw >= mem_bw * min_mem_eff) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING, 
		   "You may not have enough display bandwidth for current mode\n"
		   "If you have flickering problem, try to lower resolution, refresh rate, or color depth\n");
    } 

    /*  CRTC1
        Set GRPH_BUFFER_CNTL register using h/w defined optimal values.
    	GRPH_STOP_REQ <= MIN[ 0x7C, (CRTC_H_DISP + 1) * (bit depth) / 0x10 ]
    */
    stop_req = mode1->HDisplay * info->CurrentLayout.pixel_bytes / 16;

    /* setup Max GRPH_STOP_REQ default value */
    if (IS_RV100_VARIANT)
	max_stop_req = 0x5c;
    else
	max_stop_req  = 0x7c;
    if (stop_req > max_stop_req)
	stop_req = max_stop_req;
      
    /*  Get values from the EXT_MEM_CNTL register...converting its contents. */
    temp = INREG(RADEON_MEM_TIMING_CNTL);
    if ((info->ChipFamily == CHIP_FAMILY_RV100) || info->IsIGP) { /* RV100, M6, IGPs */
	mem_trcd      = MemTrcdExtMemCntl[(temp & 0x0c) >> 2];
	mem_trp       = MemTrpExtMemCntl[ (temp & 0x03) >> 0];
	mem_tras      = MemTrasExtMemCntl[(temp & 0x70) >> 4];
    } else { /* RV200 and later */
	mem_trcd      = MemTrcdMemTimingCntl[(temp & 0x07) >> 0];
	mem_trp       = MemTrpMemTimingCntl[ (temp & 0x700) >> 8];
	mem_tras      = MemTrasMemTimingCntl[(temp & 0xf000) >> 12];
    }
    
    /* Get values from the MEM_SDRAM_MODE_REG register...converting its */ 
    temp = INREG(RADEON_MEM_SDRAM_MODE_REG);
    data = (temp & (7<<20)) >> 20;
    if ((info->ChipFamily == CHIP_FAMILY_RV100) || info->IsIGP) { /* RV100, M6, IGPs */
	mem_tcas = MemTcas [data];
    } else {
	mem_tcas = MemTcas2 [data];
    }

    if (IS_R300_VARIANT) {

	/* on the R300, Tcas is included in Trbs.
	*/
	temp = INREG(RADEON_MEM_CNTL);
	data = (R300_MEM_NUM_CHANNELS_MASK & temp);
	if (data == 2) {
	    if (R300_MEM_USE_CD_CH_ONLY & temp) {
		temp  = INREG(R300_MC_IND_INDEX);
		temp &= ~R300_MC_IND_ADDR_MASK;
		temp |= R300_MC_READ_CNTL_CD_mcind;
		OUTREG(R300_MC_IND_INDEX, temp);
		temp  = INREG(R300_MC_IND_DATA);
		data = (R300_MEM_RBS_POSITION_C_MASK & temp);
	    } else {
		temp = INREG(R300_MC_READ_CNTL_AB);
		data = (R300_MEM_RBS_POSITION_A_MASK & temp);
	    }
	} else {
	    temp = INREG(R300_MC_READ_CNTL_AB);
	    data = (R300_MEM_RBS_POSITION_A_MASK & temp);
	}

	mem_trbs = MemTrbs[data];
	mem_tcas += mem_trbs;
    }

    if ((info->ChipFamily == CHIP_FAMILY_RV100) || info->IsIGP) { /* RV100, M6, IGPs */
	/* DDR64 SCLK_EFF = SCLK for analysis */
	sclk_eff = info->sclk;
    } else {
	    sclk_eff = info->sclk;
    }

    /* Find the memory controller latency for the display client.
    */
    if (IS_R300_VARIANT) {
	/*not enough for R350 ???*/
	/*
	if (!mode2) sclk_delay = 150;
	else {
	    if (info->RamWidth == 256) sclk_delay = 87;
	    else sclk_delay = 97;
	}
	*/
	sclk_delay = 250;
    } else {
	if ((info->ChipFamily == CHIP_FAMILY_RV100) ||
	    info->IsIGP) {
	    if (info->IsDDR) sclk_delay = 41;
	    else sclk_delay = 33;
	} else {
	    if (info->RamWidth == 128) sclk_delay = 57;
	    else sclk_delay = 41;
	}
    }

    mc_latency_sclk = sclk_delay / sclk_eff;
	
    if (info->IsDDR) {
	if (info->RamWidth == 32) {
	    k1 = 40;
	    c  = 3;
	} else {
	    k1 = 20;
	    c  = 1;
	}
    } else {
	k1 = 40;
	c  = 3;
    }
    mc_latency_mclk = ((2.0*mem_trcd + mem_tcas*c + 4.0*mem_tras + 4.0*mem_trp + k1) /
		       info->mclk) + (4.0 / sclk_eff);

    /*
      HW cursor time assuming worst case of full size colour cursor.
    */
    cur_latency_mclk = (mem_trp + MAX(mem_tras, (mem_trcd + 2*(cur_size - (info->IsDDR+1))))) / info->mclk;
    cur_latency_sclk = cur_size / sclk_eff;

    /*
      Find the total latency for the display data.
    */
    disp_latency_overhead = 8.0 / info->sclk;
    mc_latency_mclk = mc_latency_mclk + disp_latency_overhead + cur_latency_mclk;
    mc_latency_sclk = mc_latency_sclk + disp_latency_overhead + cur_latency_sclk;
    disp_latency = MAX(mc_latency_mclk, mc_latency_sclk);

    /*
      Find the drain rate of the display buffer.
    */
    disp_drain_rate = pix_clk / (16.0/info->CurrentLayout.pixel_bytes);
    if (info2)
	disp_drain_rate2 = pix_clk2 / (16.0/info2->CurrentLayout.pixel_bytes);
    else
	disp_drain_rate2 = 0;

    /*
      Find the critical point of the display buffer.
    */
    critical_point= (CARD32)(disp_drain_rate * disp_latency + 0.5); 

    /* ???? */
    /*
    temp = (info->SavedReg.grph_buffer_cntl & RADEON_GRPH_CRITICAL_POINT_MASK) >> RADEON_GRPH_CRITICAL_POINT_SHIFT;
    if (critical_point < temp) critical_point = temp;
    */
    if (info->DispPriority == 2) {
	if (mode2) {
	    /*??some R300 cards have problem with this set to 0, when CRTC2 is enabled.*/
	    if (info->ChipFamily == CHIP_FAMILY_R300) 
	        critical_point += 0x10;
	    else
	        critical_point = 0;
	}
	else
	    critical_point = 0;
    }

    /*
      The critical point should never be above max_stop_req-4.  Setting
      GRPH_CRITICAL_CNTL = 0 will thus force high priority all the time.
    */
    if (max_stop_req - critical_point < 4) critical_point = 0; 

    temp = info->SavedReg.grph_buffer_cntl;
    temp &= ~(RADEON_GRPH_STOP_REQ_MASK);
    temp |= (stop_req << RADEON_GRPH_STOP_REQ_SHIFT);
    temp &= ~(RADEON_GRPH_START_REQ_MASK);
    if ((info->ChipFamily == CHIP_FAMILY_R350) &&
	(stop_req > 0x15)) {
	stop_req -= 0x10;
    }
    temp |= (stop_req << RADEON_GRPH_START_REQ_SHIFT);

    temp |= RADEON_GRPH_BUFFER_SIZE;
    temp &= ~(RADEON_GRPH_CRITICAL_CNTL   |
	      RADEON_GRPH_CRITICAL_AT_SOF |
	      RADEON_GRPH_STOP_CNTL);
    /*
      Write the result into the register.
    */
    OUTREG(RADEON_GRPH_BUFFER_CNTL, ((temp & ~RADEON_GRPH_CRITICAL_POINT_MASK) |
				     (critical_point << RADEON_GRPH_CRITICAL_POINT_SHIFT)));

    RADEONTRACE(("GRPH_BUFFER_CNTL from %x to %x\n",
	       info->SavedReg.grph_buffer_cntl, INREG(RADEON_GRPH_BUFFER_CNTL)));

    if (mode2) {
	stop_req = mode2->HDisplay * info2->CurrentLayout.pixel_bytes / 16;

	if (stop_req > max_stop_req) stop_req = max_stop_req;

	temp = info->SavedReg.grph2_buffer_cntl;
	temp &= ~(RADEON_GRPH_STOP_REQ_MASK);
	temp |= (stop_req << RADEON_GRPH_STOP_REQ_SHIFT);
	temp &= ~(RADEON_GRPH_START_REQ_MASK);
	if ((info->ChipFamily == CHIP_FAMILY_R350) &&
	    (stop_req > 0x15)) {
	    stop_req -= 0x10;
	}
	temp |= (stop_req << RADEON_GRPH_START_REQ_SHIFT);
	temp |= RADEON_GRPH_BUFFER_SIZE;
	temp &= ~(RADEON_GRPH_CRITICAL_CNTL   |
		  RADEON_GRPH_CRITICAL_AT_SOF |
		  RADEON_GRPH_STOP_CNTL);

	if ((info->ChipFamily == CHIP_FAMILY_RS100) || 
	    (info->ChipFamily == CHIP_FAMILY_RS200))
	    critical_point2 = 0;
	else {
	    read_return_rate = MIN(info->sclk, info->mclk*(info->RamWidth*(info->IsDDR+1)/128));
	    time_disp1_drop_priority = critical_point / (read_return_rate - disp_drain_rate);

	    critical_point2 = (CARD32)((disp_latency + time_disp1_drop_priority + 
					disp_latency) * disp_drain_rate2 + 0.5);

	    if (info->DispPriority == 2) {
		if (info->ChipFamily == CHIP_FAMILY_R300) 
		    critical_point2 += 0x10;
		else
		    critical_point2 = 0;
	    }

	    if (max_stop_req - critical_point2 < 4) critical_point2 = 0;

	}

	OUTREG(RADEON_GRPH2_BUFFER_CNTL, ((temp & ~RADEON_GRPH_CRITICAL_POINT_MASK) |
					  (critical_point2 << RADEON_GRPH_CRITICAL_POINT_SHIFT)));

	RADEONTRACE(("GRPH2_BUFFER_CNTL from %x to %x\n",
		     info->SavedReg.grph2_buffer_cntl, INREG(RADEON_GRPH2_BUFFER_CNTL)));
    }
}   

/* Define CRTC registers for requested video mode */
static Bool RM6InitCrtcRegisters(ScrnInfoPtr pScrn, RM6SavePtr save,
				  DisplayModePtr mode, RM6InfoPtr info)
{
    unsigned char *RM6MMIO = info->MMIO;

    int  format;
    int  hsync_start;
    int  hsync_wid;
    int  vsync_wid;

    switch (info->CurrentLayout.pixel_code) {
    case 4:  format = 1; break;
    case 8:  format = 2; break;
    case 15: format = 3; break;      /*  555 */
    case 16: format = 4; break;      /*  565 */
    case 24: format = 5; break;      /*  RGB */
    case 32: format = 6; break;      /* xRGB */
    default:
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Unsupported pixel depth (%d)\n",
		   info->CurrentLayout.bitsPerPixel);
	return FALSE;
    }

    if ((info->DisplayType == MT_DFP) ||
	(info->DisplayType == MT_LCD)) {
	if (mode->Flags & RADEON_USE_RMX) {
#if 0
	    mode->CrtcHDisplay   = info->PanelXRes;
	    mode->CrtcVDisplay   = info->PanelYRes;
#endif
	    mode->CrtcHTotal     = mode->CrtcHDisplay + info->HBlank;
	    mode->CrtcHSyncStart = mode->CrtcHDisplay + info->HOverPlus;
	    mode->CrtcHSyncEnd   = mode->CrtcHSyncStart + info->HSyncWidth;
	    mode->CrtcVTotal     = mode->CrtcVDisplay + info->VBlank;
	    mode->CrtcVSyncStart = mode->CrtcVDisplay + info->VOverPlus;
	    mode->CrtcVSyncEnd   = mode->CrtcVSyncStart + info->VSyncWidth;
	    mode->Clock          = info->DotClock;
	    mode->Flags          = info->Flags | RADEON_USE_RMX;
	}
    }

    save->crtc_gen_cntl = (RADEON_CRTC_EXT_DISP_EN
			   | RADEON_CRTC_EN
			   | (format << 8)
			   | ((mode->Flags & V_DBLSCAN)
			      ? RADEON_CRTC_DBL_SCAN_EN
			      : 0)
			   | ((mode->Flags & V_CSYNC)
			      ? RADEON_CRTC_CSYNC_EN
			      : 0)
			   | ((mode->Flags & V_INTERLACE)
			      ? RADEON_CRTC_INTERLACE_EN
			      : 0));

    if ((info->DisplayType == MT_DFP) ||
	(info->DisplayType == MT_LCD)) {
	save->crtc_ext_cntl = RADEON_VGA_ATI_LINEAR | RADEON_XCRT_CNT_EN;
	save->crtc_gen_cntl &= ~(RADEON_CRTC_DBL_SCAN_EN |
				 RADEON_CRTC_CSYNC_EN |
				 RADEON_CRTC_INTERLACE_EN);
    } else {
	save->crtc_ext_cntl = (RADEON_VGA_ATI_LINEAR |
			       RADEON_XCRT_CNT_EN |
			       RADEON_CRTC_CRT_ON);
    }

    save->dac_cntl = (RADEON_DAC_MASK_ALL
		      | RADEON_DAC_VGA_ADR_EN
		      | (info->dac6bits ? 0 : RADEON_DAC_8BIT_EN));

    save->crtc_h_total_disp = ((((mode->CrtcHTotal / 8) - 1) & 0x3ff)
			       | ((((mode->CrtcHDisplay / 8) - 1) & 0x1ff)
				  << 16));

    hsync_wid = (mode->CrtcHSyncEnd - mode->CrtcHSyncStart) / 8;
    if (!hsync_wid) hsync_wid = 1;
    hsync_start = mode->CrtcHSyncStart - 8;

    save->crtc_h_sync_strt_wid = ((hsync_start & 0x1fff)
				  | ((hsync_wid & 0x3f) << 16)
				  | ((mode->Flags & V_NHSYNC)
				     ? RADEON_CRTC_H_SYNC_POL
				     : 0));

#if 1
				/* This works for double scan mode. */
    save->crtc_v_total_disp = (((mode->CrtcVTotal - 1) & 0xffff)
			       | ((mode->CrtcVDisplay - 1) << 16));
#else
				/* This is what cce/nbmode.c example code
				 * does -- is this correct?
				 */
    save->crtc_v_total_disp = (((mode->CrtcVTotal - 1) & 0xffff)
			       | ((mode->CrtcVDisplay
				   * ((mode->Flags & V_DBLSCAN) ? 2 : 1) - 1)
				  << 16));
#endif

    vsync_wid = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;
    if (!vsync_wid) vsync_wid = 1;

    save->crtc_v_sync_strt_wid = (((mode->CrtcVSyncStart - 1) & 0xfff)
				  | ((vsync_wid & 0x1f) << 16)
				  | ((mode->Flags & V_NVSYNC)
				     ? RADEON_CRTC_V_SYNC_POL
				     : 0));

    save->crtc_offset      = 0;
    save->crtc_offset_cntl = INREG(RADEON_CRTC_OFFSET_CNTL);

    save->crtc_pitch  = (((pScrn->displayWidth * pScrn->bitsPerPixel) +
			  ((pScrn->bitsPerPixel * 8) -1)) /
			 (pScrn->bitsPerPixel * 8));
    save->crtc_pitch |= save->crtc_pitch << 16;
    
    save->crtc_more_cntl = 0;
    if ((info->ChipFamily == CHIP_FAMILY_RS100) ||
        (info->ChipFamily == CHIP_FAMILY_RS200)) {
        /* This is to workaround the asic bug for RMX, some versions
           of BIOS dosen't have this register initialized correctly.
	*/
        save->crtc_more_cntl |= RADEON_CRTC_H_CUTOFF_ACTIVE_EN;
    }

    save->surface_cntl = 0;
    save->disp_merge_cntl = info->SavedReg.disp_merge_cntl;
    save->disp_merge_cntl &= ~RADEON_DISP_RGB_OFFSET_EN;

#if X_BYTE_ORDER == X_BIG_ENDIAN
    /* Alhought we current onlu use aperture 0, also setting aperture 1 should not harm -ReneR */
    switch (pScrn->bitsPerPixel) {
    case 16:
	save->surface_cntl |= RADEON_NONSURF_AP0_SWP_16BPP;
	save->surface_cntl |= RADEON_NONSURF_AP1_SWP_16BPP;
	break;

    case 32:
	save->surface_cntl |= RADEON_NONSURF_AP0_SWP_32BPP;
	save->surface_cntl |= RADEON_NONSURF_AP1_SWP_32BPP;
	break;
    }
#endif

    if (info->IsDellServer) {
	save->dac2_cntl = info->SavedReg.dac2_cntl;
	save->tv_dac_cntl = info->SavedReg.tv_dac_cntl;
	save->crtc2_gen_cntl = info->SavedReg.crtc2_gen_cntl;
	save->disp_hw_debug = info->SavedReg.disp_hw_debug;

	save->dac2_cntl &= ~RADEON_DAC2_DAC_CLK_SEL;
	save->dac2_cntl |= RADEON_DAC2_DAC2_CLK_SEL;

	/* For CRT on DAC2, don't turn it on if BIOS didn't
	   enable it, even it's detected.
	*/
	save->disp_hw_debug |= RADEON_CRT2_DISP1_SEL;
	save->tv_dac_cntl &= ~((1<<2) | (3<<8) | (7<<24) | (0xff<<16));
	save->tv_dac_cntl |= (0x03 | (2<<8) | (0x58<<16));
    }

    RADEONTRACE(("Pitch = %d bytes (virtualX = %d, displayWidth = %d)\n",
		 save->crtc_pitch, pScrn->virtualX,
		 info->CurrentLayout.displayWidth));
    return TRUE;
}

/* Define CRTC2 registers for requested video mode */
static Bool RM6InitCrtc2Registers(ScrnInfoPtr pScrn, RM6SavePtr save,
				     DisplayModePtr mode, RM6InfoPtr info)
{
    unsigned char *RM6MMIO = info->MMIO;
    RM6EntPtr pRM6Ent   = RM6EntPriv(pScrn);

    int  format;
    int  hsync_start;
    int  hsync_wid;
    int  vsync_wid;

    switch (info->CurrentLayout.pixel_code) {
    case 4:  format = 1; break;
    case 8:  format = 2; break;
    case 15: format = 3; break;      /*  555 */
    case 16: format = 4; break;      /*  565 */
    case 24: format = 5; break;      /*  RGB */
    case 32: format = 6; break;      /* xRGB */
    default:
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Unsupported pixel depth (%d)\n",
		   info->CurrentLayout.bitsPerPixel);
	return FALSE;
    }

    save->crtc2_gen_cntl = (RADEON_CRTC2_EN
			    | RADEON_CRTC2_CRT2_ON
			    | (format << 8)
			    | ((mode->Flags & V_DBLSCAN)
			       ? RADEON_CRTC2_DBL_SCAN_EN
			       : 0)
			    | ((mode->Flags & V_CSYNC)
			       ? RADEON_CRTC2_CSYNC_EN
			       : 0)
			    | ((mode->Flags & V_INTERLACE)
			       ? RADEON_CRTC2_INTERLACE_EN
			       : 0));

    /* Turn CRT on in case the first head is a DFP */
    save->crtc_ext_cntl |= RADEON_CRTC_CRT_ON;
    save->dac2_cntl = info->SavedReg.dac2_cntl;
    /* always let TVDAC drive CRT2, we don't support tvout yet */
    save->dac2_cntl |= RADEON_DAC2_DAC2_CLK_SEL;
    save->disp_output_cntl = info->SavedReg.disp_output_cntl;
    if (info->ChipFamily == CHIP_FAMILY_R200 ||
	IS_R300_VARIANT) {
	save->disp_output_cntl &= ~(RADEON_DISP_DAC_SOURCE_MASK |
				    RADEON_DISP_DAC2_SOURCE_MASK);
	if (pRM6Ent->MonType1 != MT_CRT) {
	    save->disp_output_cntl |= (RADEON_DISP_DAC_SOURCE_CRTC2 |
				       RADEON_DISP_DAC2_SOURCE_CRTC2);
	} else {
	    if (pRM6Ent->ReversedDAC) {
		save->disp_output_cntl |= RADEON_DISP_DAC2_SOURCE_CRTC2;
	    } else {
		save->disp_output_cntl |= RADEON_DISP_DAC_SOURCE_CRTC2;
	    }
	}
    } else {
	save->disp_hw_debug = info->SavedReg.disp_hw_debug;
	/* Turn on 2nd CRT */
	if (pRM6Ent->MonType1 != MT_CRT) {
	    /* This is for some sample boards with the VGA port
	       connected to the TVDAC, but BIOS doesn't reflect this.
	       Here we configure both DACs to use CRTC2.
	       Not sure if this happens in any retail board.
	    */
	    save->disp_hw_debug &= ~RADEON_CRT2_DISP1_SEL;
	    save->dac2_cntl |= RADEON_DAC2_DAC_CLK_SEL;
	} else {
	    if (pRM6Ent->ReversedDAC) {
		save->disp_hw_debug &= ~RADEON_CRT2_DISP1_SEL;
		save->dac2_cntl &= ~RADEON_DAC2_DAC_CLK_SEL;
	    } else {
		save->disp_hw_debug |= RADEON_CRT2_DISP1_SEL;
		save->dac2_cntl |= RADEON_DAC2_DAC_CLK_SEL;
	    }
	}
    }

    save->crtc2_h_total_disp =
	((((mode->CrtcHTotal / 8) - 1) & 0x3ff)
	 | ((((mode->CrtcHDisplay / 8) - 1) & 0x1ff) << 16));

    hsync_wid = (mode->CrtcHSyncEnd - mode->CrtcHSyncStart) / 8;
    if (!hsync_wid) hsync_wid = 1;
    hsync_start = mode->CrtcHSyncStart - 8;

    save->crtc2_h_sync_strt_wid = ((hsync_start & 0x1fff)
				   | ((hsync_wid & 0x3f) << 16)
				   | ((mode->Flags & V_NHSYNC)
				      ? RADEON_CRTC_H_SYNC_POL
				      : 0));

#if 1
				/* This works for double scan mode. */
    save->crtc2_v_total_disp = (((mode->CrtcVTotal - 1) & 0xffff)
				| ((mode->CrtcVDisplay - 1) << 16));
#else
				/* This is what cce/nbmode.c example code
				 * does -- is this correct?
				 */
    save->crtc2_v_total_disp = (((mode->CrtcVTotal - 1) & 0xffff)
				| ((mode->CrtcVDisplay
				    * ((mode->Flags & V_DBLSCAN) ? 2 : 1) - 1)
				   << 16));
#endif

    vsync_wid = mode->CrtcVSyncEnd - mode->CrtcVSyncStart;
    if (!vsync_wid) vsync_wid = 1;

    save->crtc2_v_sync_strt_wid = (((mode->CrtcVSyncStart - 1) & 0xfff)
				   | ((vsync_wid & 0x1f) << 16)
				   | ((mode->Flags & V_NVSYNC)
				      ? RADEON_CRTC2_V_SYNC_POL
				      : 0));

    save->crtc2_offset      = 0;
    save->crtc2_offset_cntl = INREG(RADEON_CRTC2_OFFSET_CNTL);
    /* this should be right */
    if (info->MergedFB) {
    save->crtc2_pitch  = (((info->CRT2pScrn->displayWidth * pScrn->bitsPerPixel) +
			   ((pScrn->bitsPerPixel * 8) -1)) /
			  (pScrn->bitsPerPixel * 8));
    save->crtc2_pitch |= save->crtc2_pitch << 16;
    } else {
    save->crtc2_pitch  = (((pScrn->displayWidth * pScrn->bitsPerPixel) +
			   ((pScrn->bitsPerPixel * 8) -1)) /
			  (pScrn->bitsPerPixel * 8));
    save->crtc2_pitch |= save->crtc2_pitch << 16;
    }
    save->disp2_merge_cntl = info->SavedReg.disp2_merge_cntl;
    save->disp2_merge_cntl &= ~(RADEON_DISP2_RGB_OFFSET_EN);

    if ((info->DisplayType == MT_DFP && info->IsSecondary) || 
	info->MergeType == MT_DFP) {
	save->crtc2_gen_cntl      = (RADEON_CRTC2_EN | (format << 8));
	save->fp2_h_sync_strt_wid = save->crtc2_h_sync_strt_wid;
	save->fp2_v_sync_strt_wid = save->crtc2_v_sync_strt_wid;
	save->fp2_gen_cntl        = info->SavedReg.fp2_gen_cntl | RADEON_FP2_ON;

	if (info->ChipFamily == CHIP_FAMILY_R200 ||
	    IS_R300_VARIANT) {
	    save->fp2_gen_cntl   &= ~(R200_FP2_SOURCE_SEL_MASK |
				      RADEON_FP2_DVO_RATE_SEL_SDR);

	    save->fp2_gen_cntl |= (R200_FP2_SOURCE_SEL_CRTC2 | 
				   RADEON_FP2_DVO_EN);
	} else {
	    save->fp2_gen_cntl &= ~RADEON_FP2_SRC_SEL_MASK;
	    save->fp2_gen_cntl |= RADEON_FP2_SRC_SEL_CRTC2;
	}

	if (pScrn->rgbBits == 8)
	    save->fp2_gen_cntl |= RADEON_FP2_PANEL_FORMAT; /* 24 bit format */
	else
	    save->fp2_gen_cntl &= ~RADEON_FP2_PANEL_FORMAT;/* 18 bit format */

    }

    RADEONTRACE(("Pitch = %d bytes (virtualX = %d, displayWidth = %d)\n",
		 save->crtc2_pitch, pScrn->virtualX,
		 info->CurrentLayout.displayWidth));

    return TRUE;
}

/* Define CRTC registers for requested video mode */
static void RM6InitFPRegisters(ScrnInfoPtr pScrn, RM6SavePtr orig,
				  RM6SavePtr save, DisplayModePtr mode,
				  RM6InfoPtr info)
{
    int    xres = mode->HDisplay;
    int    yres = mode->VDisplay;
    float  Hratio, Vratio;

    /* If the FP registers have been initialized before for a panel,
     * but the primary port is a CRT, we need to reinitialize
     * FP registers in order for CRT to work properly
     */

    if ((info->DisplayType != MT_DFP) && (info->DisplayType != MT_LCD)) {
        save->fp_crtc_h_total_disp = orig->fp_crtc_h_total_disp;
        save->fp_crtc_v_total_disp = orig->fp_crtc_v_total_disp;
        save->fp_gen_cntl          = 0;
        save->fp_h_sync_strt_wid   = orig->fp_h_sync_strt_wid;
        save->fp_horz_stretch      = 0;
        save->fp_v_sync_strt_wid   = orig->fp_v_sync_strt_wid;
        save->fp_vert_stretch      = 0;
        save->lvds_gen_cntl        = orig->lvds_gen_cntl;
        save->lvds_pll_cntl        = orig->lvds_pll_cntl;
        save->tmds_pll_cntl        = orig->tmds_pll_cntl;
        save->tmds_transmitter_cntl= orig->tmds_transmitter_cntl;

        save->lvds_gen_cntl |= ( RADEON_LVDS_DISPLAY_DIS | (1 << 23));
        save->lvds_gen_cntl &= ~(RADEON_LVDS_BLON | RADEON_LVDS_ON);
        save->fp_gen_cntl &= ~(RADEON_FP_FPON | RADEON_FP_TMDS_EN);

        return;
    }

    if (info->PanelXRes == 0 || info->PanelYRes == 0) {
	Hratio = 1.0;
	Vratio = 1.0;
    } else {
	if (xres > info->PanelXRes) xres = info->PanelXRes;
	if (yres > info->PanelYRes) yres = info->PanelYRes;

	Hratio = (float)xres/(float)info->PanelXRes;
	Vratio = (float)yres/(float)info->PanelYRes;
    }

    if (Hratio == 1.0 || !(mode->Flags & RADEON_USE_RMX)) {
	save->fp_horz_stretch = orig->fp_horz_stretch;
	save->fp_horz_stretch &= ~(RADEON_HORZ_STRETCH_BLEND |
				   RADEON_HORZ_STRETCH_ENABLE);
	save->fp_horz_stretch &= ~(RADEON_HORZ_AUTO_RATIO |
				   RADEON_HORZ_PANEL_SIZE);
	save->fp_horz_stretch |= ((xres/8-1)<<16);

    } else {
	save->fp_horz_stretch =
	    ((((unsigned long)(Hratio * RADEON_HORZ_STRETCH_RATIO_MAX +
			       0.5)) & RADEON_HORZ_STRETCH_RATIO_MASK)) |
	    (orig->fp_horz_stretch & (RADEON_HORZ_PANEL_SIZE |
				      RADEON_HORZ_FP_LOOP_STRETCH |
				      RADEON_HORZ_AUTO_RATIO_INC));
	save->fp_horz_stretch |= (RADEON_HORZ_STRETCH_BLEND |
				  RADEON_HORZ_STRETCH_ENABLE);

	save->fp_horz_stretch &= ~(RADEON_HORZ_AUTO_RATIO |
				   RADEON_HORZ_PANEL_SIZE);
	save->fp_horz_stretch |= ((info->PanelXRes / 8 - 1) << 16);

    }

    if (Vratio == 1.0 || !(mode->Flags & RADEON_USE_RMX)) {
	save->fp_vert_stretch = orig->fp_vert_stretch;
	save->fp_vert_stretch &= ~(RADEON_VERT_STRETCH_ENABLE|
				   RADEON_VERT_STRETCH_BLEND);
	save->fp_vert_stretch &= ~(RADEON_VERT_AUTO_RATIO_EN |
				   RADEON_VERT_PANEL_SIZE);
	save->fp_vert_stretch |= ((yres-1) << 12);
    } else {
	save->fp_vert_stretch =
	    (((((unsigned long)(Vratio * RADEON_VERT_STRETCH_RATIO_MAX +
				0.5)) & RADEON_VERT_STRETCH_RATIO_MASK)) |
	     (orig->fp_vert_stretch & (RADEON_VERT_PANEL_SIZE |
				       RADEON_VERT_STRETCH_RESERVED)));
	save->fp_vert_stretch |= (RADEON_VERT_STRETCH_ENABLE |
				  RADEON_VERT_STRETCH_BLEND);

	save->fp_vert_stretch &= ~(RADEON_VERT_AUTO_RATIO_EN |
				   RADEON_VERT_PANEL_SIZE);
	save->fp_vert_stretch |= ((info->PanelYRes-1) << 12);

    }

    save->fp_gen_cntl = (orig->fp_gen_cntl & (CARD32)
			 ~(RADEON_FP_SEL_CRTC2 |
			   RADEON_FP_RMX_HVSYNC_CONTROL_EN |
			   RADEON_FP_DFP_SYNC_SEL |
			   RADEON_FP_CRT_SYNC_SEL |
			   RADEON_FP_CRTC_LOCK_8DOT |
			   RADEON_FP_USE_SHADOW_EN |
			   RADEON_FP_CRTC_USE_SHADOW_VEND |
			   RADEON_FP_CRT_SYNC_ALT));
    save->fp_gen_cntl |= (RADEON_FP_CRTC_DONT_SHADOW_VPAR |
			  RADEON_FP_CRTC_DONT_SHADOW_HEND );

    if (pScrn->rgbBits == 8)
        save->fp_gen_cntl |= RADEON_FP_PANEL_FORMAT;  /* 24 bit format */
    else
        save->fp_gen_cntl &= ~RADEON_FP_PANEL_FORMAT;/* 18 bit format */

    if (IS_R300_VARIANT ||
	(info->ChipFamily == CHIP_FAMILY_R200)) {
	save->fp_gen_cntl &= ~R200_FP_SOURCE_SEL_MASK;
	if (mode->Flags & RADEON_USE_RMX) 
	    save->fp_gen_cntl |= R200_FP_SOURCE_SEL_RMX;
	else
	    save->fp_gen_cntl |= R200_FP_SOURCE_SEL_CRTC1;
    } else 
	save->fp_gen_cntl |= RADEON_FP_SEL_CRTC1;

    save->lvds_gen_cntl = orig->lvds_gen_cntl;
    save->lvds_pll_cntl = orig->lvds_pll_cntl;

    info->PanelOff = FALSE;
    /* This option is used to force the ONLY DEVICE in XFConfig to use
     * CRT port, instead of default DVI port.
     */
    if (xf86ReturnOptValBool(info->Options, OPTION_PANEL_OFF, FALSE)) {
	info->PanelOff = TRUE;
    }

    save->tmds_pll_cntl = orig->tmds_pll_cntl;
    save->tmds_transmitter_cntl= orig->tmds_transmitter_cntl;
    if (info->PanelOff && info->MergedFB) {
	info->OverlayOnCRTC2 = TRUE;
	if (info->DisplayType == MT_LCD) {
	    /* Turning off LVDS_ON seems to make panel white blooming.
	     * For now we just turn off display data ???
	     */
	    save->lvds_gen_cntl |= (RADEON_LVDS_DISPLAY_DIS);
	    save->lvds_gen_cntl &= ~(RADEON_LVDS_BLON | RADEON_LVDS_ON);

	} else if (info->DisplayType == MT_DFP)
	    save->fp_gen_cntl &= ~(RADEON_FP_FPON | RADEON_FP_TMDS_EN);
    } else {
	if (info->DisplayType == MT_LCD) {

	    save->lvds_gen_cntl |= (RADEON_LVDS_ON | RADEON_LVDS_BLON);
	    save->fp_gen_cntl   &= ~(RADEON_FP_FPON | RADEON_FP_TMDS_EN);

	} else if (info->DisplayType == MT_DFP) {
	    int i;
	    CARD32 tmp = orig->tmds_pll_cntl & 0xfffff;
	    for (i=0; i<4; i++) {
		if (info->tmds_pll[i].freq == 0) break;
		if (save->dot_clock_freq < info->tmds_pll[i].freq) {
		    tmp = info->tmds_pll[i].value ;
		    break;
		}
	    }
	    if (IS_R300_VARIANT ||
		(info->ChipFamily == CHIP_FAMILY_RV280)) {
		if (tmp & 0xfff00000)
		    save->tmds_pll_cntl = tmp;
		else
		    save->tmds_pll_cntl = (orig->tmds_pll_cntl & 0xfff00000) | tmp;
	    } else save->tmds_pll_cntl = tmp;

	    RADEONTRACE(("TMDS_PLL from %x to %x\n", 
			 orig->tmds_pll_cntl, 
			 save->tmds_pll_cntl));

            save->tmds_transmitter_cntl &= ~(RADEON_TMDS_TRANSMITTER_PLLRST);
            if (IS_R300_VARIANT ||
		(info->ChipFamily == CHIP_FAMILY_R200) || !info->HasCRTC2)
		save->tmds_transmitter_cntl &= ~(RADEON_TMDS_TRANSMITTER_PLLEN);
            else /* weird, RV chips got this bit reversed? */
                save->tmds_transmitter_cntl |= (RADEON_TMDS_TRANSMITTER_PLLEN);

	    save->fp_gen_cntl   |= (RADEON_FP_FPON | RADEON_FP_TMDS_EN);
        }
    }

    if (info->IsMobility) {
	RM6EntPtr pRM6Ent   = RM6EntPriv(pScrn);

	/* To work correctly with laptop hotkeys.
	 * Since there is no machnism for accessing ACPI evnets
	 * and the driver currently doesn't know how to validate
	 * a mode dynamically, we have to tell BIOS don't do
	 * display switching after X has started.  
	 * If LCD is on, lid close/open should still work 
	 * with below settings
	 */
	if (info->DisplayType == MT_LCD) {
	    if (pRM6Ent->MonType2 == MT_CRT)
		save->bios_5_scratch = 0x0201;
	    else if (pRM6Ent->MonType2 == MT_DFP)
		save->bios_5_scratch = 0x0801;
	    else
		save->bios_5_scratch = orig->bios_5_scratch;
	} else {
	    if (pRM6Ent->MonType2 == MT_CRT)
		save->bios_5_scratch = 0x0200;
	    else if (pRM6Ent->MonType2 == MT_DFP)
		save->bios_5_scratch = 0x0800;
	    else
		save->bios_5_scratch = 0x0; 
	}
	save->bios_4_scratch = 0x4;
	save->bios_6_scratch = orig->bios_6_scratch | 0x40000000;
    }

    save->fp_crtc_h_total_disp = save->crtc_h_total_disp;
    save->fp_crtc_v_total_disp = save->crtc_v_total_disp;
    save->fp_h_sync_strt_wid   = save->crtc_h_sync_strt_wid;
    save->fp_v_sync_strt_wid   = save->crtc_v_sync_strt_wid;
}

/* Define PLL registers for requested video mode */
static void RM6InitPLLRegisters(RM6SavePtr save, RM6InfoPtr info,
				   double dot_clock)
{
    unsigned long  freq = dot_clock * 100;
    RM6PLLPtr pll = &info->pll;

    struct {
	int divider;
	int bitvalue;
    } *post_div, post_divs[]   = {
				/* From RAGE 128 VR/RAGE 128 GL Register
				 * Reference Manual (Technical Reference
				 * Manual P/N RRG-G04100-C Rev. 0.04), page
				 * 3-17 (PLL_DIV_[3:0]).
				 */
	{  1, 0 },              /* VCLK_SRC                 */
	{  2, 1 },              /* VCLK_SRC/2               */
	{  4, 2 },              /* VCLK_SRC/4               */
	{  8, 3 },              /* VCLK_SRC/8               */
	{  3, 4 },              /* VCLK_SRC/3               */
	{ 16, 5 },              /* VCLK_SRC/16              */
	{  6, 6 },              /* VCLK_SRC/6               */
	{ 12, 7 },              /* VCLK_SRC/12              */
	{  0, 0 }
    };

    if (freq > pll->max_pll_freq)      freq = pll->max_pll_freq;
    if (freq * 12 < pll->min_pll_freq) freq = pll->min_pll_freq / 12;

    for (post_div = &post_divs[0]; post_div->divider; ++post_div) {
	save->pll_output_freq = post_div->divider * freq;

	if (save->pll_output_freq >= pll->min_pll_freq
	    && save->pll_output_freq <= pll->max_pll_freq) break;
    }

    if (!post_div->divider) {
	save->pll_output_freq = freq;
	post_div = &post_divs[0];
    }

    save->dot_clock_freq = freq;
    save->feedback_div   = RM6Div(pll->reference_div
				     * save->pll_output_freq,
				     pll->reference_freq);
    save->post_div       = post_div->divider;

    RADEONTRACE(("dc=%d, of=%d, fd=%d, pd=%d\n",
	       save->dot_clock_freq,
	       save->pll_output_freq,
	       save->feedback_div,
	       save->post_div));

    save->ppll_ref_div   = pll->reference_div;

    /* 
     * on iBooks the LCD pannel needs tweaked PLL timings 
     */
#ifdef __powerpc__
    if (xf86ReturnOptValBool(info->Options, OPTION_IBOOKHACKS, FALSE))
        save->ppll_div_3 = 0x000600ad;
    else
#endif
        save->ppll_div_3 = (save->feedback_div | (post_div->bitvalue << 16));

    save->htotal_cntl    = 0;
}

/* Define PLL2 registers for requested video mode */
static void RM6InitPLL2Registers(RM6SavePtr save, RM6PLLPtr pll,
				    double dot_clock)
{
    unsigned long  freq = dot_clock * 100;

    struct {
	int divider;
	int bitvalue;
    } *post_div, post_divs[]   = {
				/* From RAGE 128 VR/RAGE 128 GL Register
				 * Reference Manual (Technical Reference
				 * Manual P/N RRG-G04100-C Rev. 0.04), page
				 * 3-17 (PLL_DIV_[3:0]).
				 */
	{  1, 0 },              /* VCLK_SRC                 */
	{  2, 1 },              /* VCLK_SRC/2               */
	{  4, 2 },              /* VCLK_SRC/4               */
	{  8, 3 },              /* VCLK_SRC/8               */
	{  3, 4 },              /* VCLK_SRC/3               */
	{  6, 6 },              /* VCLK_SRC/6               */
	{ 12, 7 },              /* VCLK_SRC/12              */
	{  0, 0 }
    };

    if (freq > pll->max_pll_freq)      freq = pll->max_pll_freq;
    if (freq * 12 < pll->min_pll_freq) freq = pll->min_pll_freq / 12;

    for (post_div = &post_divs[0]; post_div->divider; ++post_div) {
	save->pll_output_freq_2 = post_div->divider * freq;
	if (save->pll_output_freq_2 >= pll->min_pll_freq
	    && save->pll_output_freq_2 <= pll->max_pll_freq) break;
    }

    if (!post_div->divider) {
	save->pll_output_freq_2 = freq;
	post_div = &post_divs[0];
    }

    save->dot_clock_freq_2 = freq;
    save->feedback_div_2   = RM6Div(pll->reference_div
				       * save->pll_output_freq_2,
				       pll->reference_freq);
    save->post_div_2       = post_div->divider;

    RADEONTRACE(("dc=%d, of=%d, fd=%d, pd=%d\n",
	       save->dot_clock_freq_2,
	       save->pll_output_freq_2,
	       save->feedback_div_2,
	       save->post_div_2));

    save->p2pll_ref_div    = pll->reference_div;
    save->p2pll_div_0      = (save->feedback_div_2 |
			      (post_div->bitvalue << 16));
    save->htotal_cntl2     = 0;
}


/* Define registers for a requested video mode */
static Bool RM6Init(ScrnInfoPtr pScrn, DisplayModePtr mode,
		       RM6SavePtr save)
{
    RM6InfoPtr  info      = RM6PTR(pScrn);
    double         dot_clock = mode->Clock/1000.0;

#if RADEON_DEBUG
    ErrorF("%-12.12s %7.2f  %4d %4d %4d %4d  %4d %4d %4d %4d (%d,%d)",
	   mode->name,
	   dot_clock,

	   mode->HDisplay,
	   mode->HSyncStart,
	   mode->HSyncEnd,
	   mode->HTotal,

	   mode->VDisplay,
	   mode->VSyncStart,
	   mode->VSyncEnd,
	   mode->VTotal,
	   pScrn->depth,
	   pScrn->bitsPerPixel);
    if (mode->Flags & V_DBLSCAN)   ErrorF(" D");
    if (mode->Flags & V_CSYNC)     ErrorF(" C");
    if (mode->Flags & V_INTERLACE) ErrorF(" I");
    if (mode->Flags & V_PHSYNC)    ErrorF(" +H");
    if (mode->Flags & V_NHSYNC)    ErrorF(" -H");
    if (mode->Flags & V_PVSYNC)    ErrorF(" +V");
    if (mode->Flags & V_NVSYNC)    ErrorF(" -V");
    ErrorF("\n");
    ErrorF("%-12.12s %7.2f  %4d %4d %4d %4d  %4d %4d %4d %4d (%d,%d)",
	   mode->name,
	   dot_clock,

	   mode->CrtcHDisplay,
	   mode->CrtcHSyncStart,
	   mode->CrtcHSyncEnd,
	   mode->CrtcHTotal,

	   mode->CrtcVDisplay,
	   mode->CrtcVSyncStart,
	   mode->CrtcVSyncEnd,
	   mode->CrtcVTotal,
	   pScrn->depth,
	   pScrn->bitsPerPixel);
    if (mode->Flags & V_DBLSCAN)   ErrorF(" D");
    if (mode->Flags & V_CSYNC)     ErrorF(" C");
    if (mode->Flags & V_INTERLACE) ErrorF(" I");
    if (mode->Flags & V_PHSYNC)    ErrorF(" +H");
    if (mode->Flags & V_NHSYNC)    ErrorF(" -H");
    if (mode->Flags & V_PVSYNC)    ErrorF(" +V");
    if (mode->Flags & V_NVSYNC)    ErrorF(" -V");
    ErrorF("\n");
#endif

    info->Flags = mode->Flags;

    RM6InitCommonRegisters(save, info);
    if (info->IsSecondary) {
	if (!RM6InitCrtc2Registers(pScrn, save, mode, info))
	    return FALSE;
	RM6InitPLL2Registers(save, &info->pll, dot_clock);
    } else if (info->MergedFB) {
        RM6InitCommonRegisters(save, info);
        if (!RM6InitCrtcRegisters(pScrn, save, 
			((RM6MergedDisplayModePtr)mode->Private)->CRT1, info))
            return FALSE;
        dot_clock = (((RM6MergedDisplayModePtr)mode->Private)->CRT1)->Clock / 1000.0;
        if (dot_clock) {
            RM6InitPLLRegisters(save, info, dot_clock);
        } else {
            save->ppll_ref_div = info->SavedReg.ppll_ref_div;
            save->ppll_div_3   = info->SavedReg.ppll_div_3;
            save->htotal_cntl  = info->SavedReg.htotal_cntl;
        }
        RM6InitCrtc2Registers(pScrn, save, 
			((RM6MergedDisplayModePtr)mode->Private)->CRT2, info);
        dot_clock = (((RM6MergedDisplayModePtr)mode->Private)->CRT2)->Clock / 1000.0;
        RM6InitPLL2Registers(save, &info->pll, dot_clock);
    } else {
	if (!RM6InitCrtcRegisters(pScrn, save, mode, info))
	    return FALSE;
	dot_clock = mode->Clock/1000.0;
	if (dot_clock) {
            if (info->UseBiosDividers) {
                save->ppll_ref_div = info->RefDivider;
                save->ppll_div_3   = info->FeedbackDivider | (info->PostDivider << 16);
                save->htotal_cntl  = 0;
            }
            else
		RM6InitPLLRegisters(save, info, dot_clock);
	} else {
	    save->ppll_ref_div = info->SavedReg.ppll_ref_div;
	    save->ppll_div_3   = info->SavedReg.ppll_div_3;
	    save->htotal_cntl  = info->SavedReg.htotal_cntl;
	}

	/* Not used for now: */
     /* if (!info->PaletteSavedOnVT) RM6InitPalette(save); */
    }

    /* make RMX work for mergedfb modes on the LCD */
    if (info->MergedFB) {
	if ((info->MergeType == MT_LCD) || (info->MergeType == MT_DFP)) {
            /* I suppose crtc2 could drive the FP as well... */
	    RM6InitFPRegisters(pScrn, &info->SavedReg, save, 
		((RM6MergedDisplayModePtr)mode->Private)->CRT2, info);
	}
	else {
	    RM6InitFPRegisters(pScrn, &info->SavedReg, save, 
		((RM6MergedDisplayModePtr)mode->Private)->CRT1, info);
	}
    }
    else {
    	RM6InitFPRegisters(pScrn, &info->SavedReg, save, mode, info);
    }

    RADEONTRACE(("RM6Init returns %p\n", save));
    return TRUE;
}

/* Initialize a new mode */
static Bool RM6ModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    if (!RM6Init(pScrn, mode, &info->ModeReg)) return FALSE;

    pScrn->vtSema = TRUE;
    RM6Blank(pScrn);
    RM6RestoreMode(pScrn, &info->ModeReg);
    RM6Unblank(pScrn);

    info->CurrentLayout.mode = mode;

    if (info->DispPriority)
	RM6InitDispBandwidth(pScrn);

    return TRUE;
}

static Bool RM6SaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr  pScrn = xf86Screens[pScreen->myNum];
    Bool         unblank;

    unblank = xf86IsUnblank(mode);
    if (unblank) SetTimeSinceLastInputEvent();

    if ((pScrn != NULL) && pScrn->vtSema) {
	if (unblank)  RM6Unblank(pScrn);
	else          RM6Blank(pScrn);
    }
    return TRUE;
}

Bool RM6SwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    ScrnInfoPtr    pScrn       = xf86Screens[scrnIndex];
    RM6InfoPtr  info        = RM6PTR(pScrn);
    Bool           ret;

    if (info->accelOn) info->accel->Sync(pScrn);

    info->IsSwitching = TRUE;
    ret = RM6ModeInit(xf86Screens[scrnIndex], mode);
    info->IsSwitching = FALSE;
    

    if (info->accelOn) {
	info->accel->Sync(pScrn);
	RM6EngineRestore(pScrn);
    }


    /* Since RandR (indirectly) uses SwitchMode(), we need to
     * update our Xinerama info here, too, in case of resizing
     */
    if(info->MergedFB) {
       RM6UpdateXineramaScreenInfo(pScrn);
    }

    return ret;
}

#ifdef X_XF86MiscPassMessage
Bool RM6HandleMessage(int scrnIndex, const char* msgtype,
			 const char* msgval, char** retmsg)
{
    ErrorF("RM6HandleMessage(%d, \"%s\", \"%s\", retmsg)\n", scrnIndex,
		    msgtype, msgval);
    *retmsg = "";
    return 0;
}
#endif

/* Used to disallow modes that are not supported by the hardware */
ModeStatus RM6ValidMode(int scrnIndex, DisplayModePtr mode,
			   Bool verbose, int flag)
{
    /* There are problems with double scan mode at high clocks
     * They're likely related PLL and display buffer settings.
     * Disable these modes for now.
     */
    if (mode->Flags & V_DBLSCAN) {
	if ((mode->CrtcHDisplay >= 1024) || (mode->CrtcVDisplay >= 768))
	    return MODE_CLOCK_RANGE;
    }
    return MODE_OK;
}

/* Adjust viewport into virtual desktop such that (0,0) in viewport
 * space is (x,y) in virtual space.
 */
void RM6DoAdjustFrame(ScrnInfoPtr pScrn, int x, int y, int clone)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            reg, Base;

    if (info->showCache && y) {
	        int lastline = info->FbMapSize /
		    ((pScrn->displayWidth * pScrn->bitsPerPixel) / 8);

		lastline -= pScrn->currentMode->VDisplay;
		y += (pScrn->virtualY - 1) * (y / 3 + 1);
		if (y > lastline) y = lastline;
    }
    Base = y * info->CurrentLayout.displayWidth + x;
    
    switch (info->CurrentLayout.pixel_code) {
    case 15:
    case 16: Base *= 2; break;
    case 24: Base *= 3; break;
    case 32: Base *= 4; break;
    }

    Base &= ~7;                 /* 3 lower bits are always 0 */

    if (clone || info->IsSecondary) {
	Base += pScrn->fbOffset;
	reg = RADEON_CRTC2_OFFSET;
    } else {
	reg = RADEON_CRTC_OFFSET;
    }


    OUTREG(reg, Base);
}

void RM6AdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr    pScrn      = xf86Screens[scrnIndex];
    RM6InfoPtr  info       = RM6PTR(pScrn);


    if (info->accelOn) info->accel->Sync(pScrn);

    if(info->MergedFB) {
    	RM6AdjustFrameMerged(scrnIndex, x, y, flags);
    } else {
	RM6DoAdjustFrame(pScrn, x, y, FALSE);
    }

    RM6SetFBLocation (pScrn);
}

/* Called when VT switching back to the X server.  Reinitialize the
 * video mode.
 */
Bool RM6EnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr    pScrn = xf86Screens[scrnIndex];
    RM6InfoPtr  info  = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    RADEONTRACE(("RM6EnterVT\n"));

    if (INREG(RADEON_CONFIG_MEMSIZE) == 0) { /* Softboot V_BIOS */
       xf86Int10InfoPtr pInt;
       xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                  "zero MEMSIZE, probably at D3cold. Re-POSTing via int10.\n");
       pInt = xf86InitInt10 (info->pEnt->index);
       if (pInt) {
           pInt->num = 0xe6;
           xf86ExecX86int10 (pInt);
           xf86FreeInt10 (pInt);
       }
    }

    if (!RM6ModeInit(pScrn, pScrn->currentMode)) return FALSE;

    RM6SetFBLocation (pScrn);

    /* this will get XVideo going again, but only if XVideo was initialised
       during server startup (hence the info->adaptor if). */
    if (info->adaptor)
	RM6ResetVideo(pScrn);

    if (info->accelOn)
	RM6EngineRestore(pScrn);


    pScrn->AdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

    return TRUE;
}

/* Called when VT switching away from the X server.  Restore the
 * original text mode.
 */
void RM6LeaveVT(int scrnIndex, int flags)
{
    ScrnInfoPtr    pScrn = xf86Screens[scrnIndex];
    RM6InfoPtr  info  = RM6PTR(pScrn);
    RM6SavePtr  save  = &info->ModeReg;

    RADEONTRACE(("RM6LeaveVT\n"));

    RM6Restore(pScrn);
}

/* Called at the end of each server generation.  Restore the original
 * text mode, unmap video memory, and unwrap and call the saved
 * CloseScreen function.
 */
static Bool RM6CloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr    pScrn = xf86Screens[scrnIndex];
    RM6InfoPtr  info  = RM6PTR(pScrn);

    RADEONTRACE(("RM6CloseScreen\n"));


    if(info->RenderTex) {
        xf86FreeOffscreenLinear(info->RenderTex);
        info->RenderTex = NULL;
    }

    if (pScrn->vtSema) {
	RM6Restore(pScrn);
    }
    RM6UnmapMem(pScrn);

    if (info->accel) XAADestroyInfoRec(info->accel);
    info->accel = NULL;

    if (info->scratch_save) xfree(info->scratch_save);
    info->scratch_save = NULL;

    if (info->cursor) xf86DestroyCursorInfoRec(info->cursor);
    info->cursor = NULL;

    if (info->DGAModes) xfree(info->DGAModes);
    info->DGAModes = NULL;

    pScrn->vtSema = FALSE;

    xf86ClearPrimInitDone(info->pEnt->index);

    pScreen->BlockHandler = info->BlockHandler;
    pScreen->CloseScreen = info->CloseScreen;
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}

void RM6FreeScreen(int scrnIndex, int flags)
{
    ScrnInfoPtr  pScrn = xf86Screens[scrnIndex];
    RM6InfoPtr  info  = RM6PTR(pScrn);
    
    RADEONTRACE(("RM6FreeScreen\n"));

    /* when server quits at PreInit, we don't need do this anymore*/
    if (!info) return;

    if(info->MergedFB) {
       if(pScrn->modes) {
          pScrn->currentMode = pScrn->modes;
          do {
            DisplayModePtr p = pScrn->currentMode->next;
            if(pScrn->currentMode->Private)
                xfree(pScrn->currentMode->Private);
            xfree(pScrn->currentMode);
            pScrn->currentMode = p;
          } while(pScrn->currentMode != pScrn->modes);
       }
       pScrn->currentMode = info->CRT1CurrentMode;
       pScrn->modes = info->CRT1Modes;
       info->CRT1CurrentMode = NULL;
       info->CRT1Modes = NULL;

       if(info->CRT2pScrn) {
          if(info->CRT2pScrn->modes) {
             while(info->CRT2pScrn->modes)
                xf86DeleteMode(&info->CRT2pScrn->modes, info->CRT2pScrn->modes);
          }
          if(info->CRT2pScrn->monitor) {
	     if(info->CRT2pScrn->monitor->Modes) {
	        while(info->CRT2pScrn->monitor->Modes)
		   xf86DeleteMode(&info->CRT2pScrn->monitor->Modes, info->CRT2pScrn->monitor->Modes);
	     }
	     if(info->CRT2pScrn->monitor->DDC) xfree(info->CRT2pScrn->monitor->DDC);
             xfree(info->CRT2pScrn->monitor);
	  }
          xfree(info->CRT2pScrn);
          info->CRT2pScrn = NULL;
       }
    }

    if (xf86LoaderCheckSymbol("vgaHWFreeHWRec"))
	vgaHWFreeHWRec(pScrn);
    RM6FreeRec(pScrn);
}

/*
 * Powering done DAC, needed for DPMS problem with ViewSonic P817 (or its variant).
 *
 * Note for current DAC mapping when calling this function:
 * For most of cards:
 * single CRT:  Driver doesn't change the existing CRTC->DAC mapping, 
 *              CRTC1 could be driving either DAC or both DACs.
 * CRT+CRT:     CRTC1->TV DAC, CRTC2->Primary DAC
 * DFP/LCD+CRT: CRTC2->TV DAC, CRTC2->Primary DAC.
 * Some boards have two DACs reversed or don't even have a primary DAC,
 * this is reflected in pRM6Ent->ReversedDAC. And radeon 7200 doesn't 
 * have a second DAC.
 * It's kind of messy, we'll need to redo DAC mapping part some day.
 */
static void RM6DacPowerSet(ScrnInfoPtr pScrn, Bool IsOn, Bool IsPrimaryDAC)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    if (IsPrimaryDAC) {
	CARD32 dac_cntl;
	CARD32 dac_macro_cntl = 0;
	dac_cntl = INREG(RADEON_DAC_CNTL);
	if ((!info->IsMobility) || (info->ChipFamily == CHIP_FAMILY_RV350)) 
	    dac_macro_cntl = INREG(RADEON_DAC_MACRO_CNTL);
	if (IsOn) {
	    dac_cntl &= ~RADEON_DAC_PDWN;
	    dac_macro_cntl &= ~(RADEON_DAC_PDWN_R |
				RADEON_DAC_PDWN_G |
				RADEON_DAC_PDWN_B);
	} else {
	    dac_cntl |= RADEON_DAC_PDWN;
	    dac_macro_cntl |= (RADEON_DAC_PDWN_R |
			       RADEON_DAC_PDWN_G |
			       RADEON_DAC_PDWN_B);
	}
	OUTREG(RADEON_DAC_CNTL, dac_cntl);
	if ((!info->IsMobility) || (info->ChipFamily == CHIP_FAMILY_RV350)) 
	    OUTREG(RADEON_DAC_MACRO_CNTL, dac_macro_cntl);
    } else {
	if (info->ChipFamily != CHIP_FAMILY_R200) {
	    CARD32 tv_dac_cntl = INREG(RADEON_TV_DAC_CNTL);
	    if (IsOn) {
		tv_dac_cntl &= ~(RADEON_TV_DAC_RDACPD |
				 RADEON_TV_DAC_GDACPD |
				 RADEON_TV_DAC_BDACPD |
				 RADEON_TV_DAC_BGSLEEP);
	    } else {
		tv_dac_cntl |= (RADEON_TV_DAC_RDACPD |
				RADEON_TV_DAC_GDACPD |
				RADEON_TV_DAC_BDACPD |
				RADEON_TV_DAC_BGSLEEP);
	    }
	    OUTREG(RADEON_TV_DAC_CNTL, tv_dac_cntl);
	} else {
	    CARD32 fp2_gen_cntl = INREG(RADEON_FP2_GEN_CNTL);
	    if (IsOn) {
		fp2_gen_cntl |= RADEON_FP2_DVO_EN;
	    } else {
		fp2_gen_cntl &= ~RADEON_FP2_DVO_EN;
	    }
	    OUTREG(RADEON_FP2_GEN_CNTL, fp2_gen_cntl);
	}
    }
}

/* Sets VESA Display Power Management Signaling (DPMS) Mode */
static void RM6DisplayPowerManagementSet(ScrnInfoPtr pScrn,
					    int PowerManagementMode,
					    int flags)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    RM6EntPtr pRM6Ent   = RM6EntPriv(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    if (!pScrn->vtSema) return;


    if (info->accelOn) info->accel->Sync(pScrn);

    {
	int             mask1     = (RADEON_CRTC_DISPLAY_DIS |
				     RADEON_CRTC_HSYNC_DIS |
				     RADEON_CRTC_VSYNC_DIS);
	int             mask2     = (RADEON_CRTC2_DISP_DIS |
				     RADEON_CRTC2_VSYNC_DIS |
				     RADEON_CRTC2_HSYNC_DIS);

	switch (PowerManagementMode) {
	case DPMSModeOn:
	    /* Screen: On; HSync: On, VSync: On */
	    if (info->IsSecondary)
		OUTREGP(RADEON_CRTC2_GEN_CNTL, 0, ~mask2);
	    else {
		if (info->MergedFB)
		    OUTREGP(RADEON_CRTC2_GEN_CNTL, 0, ~mask2);
		OUTREGP(RADEON_CRTC_EXT_CNTL, 0, ~mask1);
	    }
	    break;

	case DPMSModeStandby:
	    /* Screen: Off; HSync: Off, VSync: On */
	    if (info->IsSecondary)
		OUTREGP(RADEON_CRTC2_GEN_CNTL,
			RADEON_CRTC2_DISP_DIS | RADEON_CRTC2_HSYNC_DIS,
			~mask2);
	    else {
		if (info->MergedFB)
		    OUTREGP(RADEON_CRTC2_GEN_CNTL,
			    RADEON_CRTC2_DISP_DIS | RADEON_CRTC2_HSYNC_DIS,
			    ~mask2);
		OUTREGP(RADEON_CRTC_EXT_CNTL,
			RADEON_CRTC_DISPLAY_DIS | RADEON_CRTC_HSYNC_DIS,
			~mask1);
	    }
	    break;

	case DPMSModeSuspend:
	    /* Screen: Off; HSync: On, VSync: Off */
	    if (info->IsSecondary)
		OUTREGP(RADEON_CRTC2_GEN_CNTL,
			RADEON_CRTC2_DISP_DIS | RADEON_CRTC2_VSYNC_DIS,
			~mask2);
	    else {
		if (info->MergedFB)
		    OUTREGP(RADEON_CRTC2_GEN_CNTL,
			    RADEON_CRTC2_DISP_DIS | RADEON_CRTC2_VSYNC_DIS,
			    ~mask2);
		OUTREGP(RADEON_CRTC_EXT_CNTL,
			RADEON_CRTC_DISPLAY_DIS | RADEON_CRTC_VSYNC_DIS,
			~mask1);
	    }
	    break;

	case DPMSModeOff:
	    /* Screen: Off; HSync: Off, VSync: Off */
	    if (info->IsSecondary)
		OUTREGP(RADEON_CRTC2_GEN_CNTL, mask2, ~mask2);
	    else {
		if (info->MergedFB)
		    OUTREGP(RADEON_CRTC2_GEN_CNTL, mask2, ~mask2);
		OUTREGP(RADEON_CRTC_EXT_CNTL, mask1, ~mask1);
	    }
	    break;
	}

	if (PowerManagementMode == DPMSModeOn) {
	    if (info->IsSecondary) {
		if (info->DisplayType == MT_DFP) {
		    OUTREGP (RADEON_FP2_GEN_CNTL, 0, ~RADEON_FP2_BLANK_EN);
		    OUTREGP (RADEON_FP2_GEN_CNTL, RADEON_FP2_ON, ~RADEON_FP2_ON);
		    if (info->ChipFamily >= CHIP_FAMILY_R200) {
			OUTREGP (RADEON_FP2_GEN_CNTL, RADEON_FP2_DVO_EN, ~RADEON_FP2_DVO_EN);
		    }
		} else if (info->DisplayType == MT_CRT) {
		    RM6DacPowerSet(pScrn, TRUE, !pRM6Ent->ReversedDAC);
		}
	    } else {
		if ((info->MergedFB) && (info->MergeType == MT_DFP)) {
		    OUTREGP (RADEON_FP2_GEN_CNTL, 0, ~RADEON_FP2_BLANK_EN);
		    OUTREGP (RADEON_FP2_GEN_CNTL, RADEON_FP2_ON, ~RADEON_FP2_ON);
		    if (info->ChipFamily >= CHIP_FAMILY_R200) {
			OUTREGP (RADEON_FP2_GEN_CNTL, RADEON_FP2_DVO_EN, ~RADEON_FP2_DVO_EN);
		    }
		}
		if (info->DisplayType == MT_DFP) {
		    OUTREGP (RADEON_FP_GEN_CNTL, (RADEON_FP_FPON | RADEON_FP_TMDS_EN),
			     ~(RADEON_FP_FPON | RADEON_FP_TMDS_EN));
		} else if (info->DisplayType == MT_LCD) {

		    OUTREGP (RADEON_LVDS_GEN_CNTL, RADEON_LVDS_BLON, ~RADEON_LVDS_BLON);
		    usleep (info->PanelPwrDly * 1000);
		    OUTREGP (RADEON_LVDS_GEN_CNTL, RADEON_LVDS_ON, ~RADEON_LVDS_ON);
		} else if (info->DisplayType == MT_CRT) {
		    if ((pRM6Ent->HasSecondary) || info->MergedFB) {
			RM6DacPowerSet(pScrn, TRUE, pRM6Ent->ReversedDAC);
		    } else {
			RM6DacPowerSet(pScrn, TRUE, TRUE);
			if (info->HasCRTC2)
			    RM6DacPowerSet(pScrn, TRUE, FALSE);
		    }
		}
	    }
	} else if ((PowerManagementMode == DPMSModeOff) ||
		   (PowerManagementMode == DPMSModeSuspend) ||
		   (PowerManagementMode == DPMSModeStandby)) {
	    if (info->IsSecondary) {
		if (info->DisplayType == MT_DFP) {
		    OUTREGP (RADEON_FP2_GEN_CNTL, RADEON_FP2_BLANK_EN, ~RADEON_FP2_BLANK_EN);
		    OUTREGP (RADEON_FP2_GEN_CNTL, 0, ~RADEON_FP2_ON);
		    if (info->ChipFamily >= CHIP_FAMILY_R200) {
			OUTREGP (RADEON_FP2_GEN_CNTL, 0, ~RADEON_FP2_DVO_EN);
		    }
		} else if (info->DisplayType == MT_CRT) {
		    RM6DacPowerSet(pScrn, FALSE, !pRM6Ent->ReversedDAC);
		}
	    } else {
		if ((info->MergedFB) && (info->MergeType == MT_DFP)) {
		    OUTREGP (RADEON_FP2_GEN_CNTL, RADEON_FP2_BLANK_EN, ~RADEON_FP2_BLANK_EN);
		    OUTREGP (RADEON_FP2_GEN_CNTL, 0, ~RADEON_FP2_ON);
		    if (info->ChipFamily >= CHIP_FAMILY_R200) {
			OUTREGP (RADEON_FP2_GEN_CNTL, 0, ~RADEON_FP2_DVO_EN);
		    }
		}
		if (info->DisplayType == MT_DFP) {
		    OUTREGP (RADEON_FP_GEN_CNTL, 0, ~(RADEON_FP_FPON | RADEON_FP_TMDS_EN));
		} else if (info->DisplayType == MT_LCD) {
		    unsigned long tmpPixclksCntl = INPLL(pScrn, RADEON_PIXCLKS_CNTL);

		    if (info->IsMobility || info->IsIGP) {
			/* Asic bug, when turning off LVDS_ON, we have to make sure
			   RADEON_PIXCLK_LVDS_ALWAYS_ON bit is off
			*/
			OUTPLLP(pScrn, RADEON_PIXCLKS_CNTL, 0, ~RADEON_PIXCLK_LVDS_ALWAYS_ONb);
		    }

		    OUTREGP (RADEON_LVDS_GEN_CNTL, 0,
			     ~(RADEON_LVDS_BLON | RADEON_LVDS_ON));

		    if (info->IsMobility || info->IsIGP) {
			OUTPLL(RADEON_PIXCLKS_CNTL, tmpPixclksCntl);
		    }
		} else if (info->DisplayType == MT_CRT) {
		    if ((pRM6Ent->HasSecondary) || info->MergedFB) {
			RM6DacPowerSet(pScrn, FALSE, pRM6Ent->ReversedDAC);
		    } else {
			/* single CRT, turning both DACs off, we don't really know 
			 * which DAC is actually connected.
			 */
			RM6DacPowerSet(pScrn, FALSE, TRUE);
			if (info->HasCRTC2) /* don't apply this to old radeon (singel CRTC) card */
			    RM6DacPowerSet(pScrn, FALSE, FALSE);
		    }
		}
	    }
	}
    }

}

static void
RM6GetMergedFBOptions(ScrnInfoPtr pScrn)
{
    RM6InfoPtr      info       = RM6PTR(pScrn);
    RM6EntPtr pRM6Ent   = RM6EntPriv(pScrn);
    char        *strptr;
    char	*default_hsync = "28-33";
    char	*default_vrefresh = "43-72";
    Bool	val;
    Bool	default_range = FALSE;
    static const char *mybadparm = "\"%s\" is is not a valid parameter for option \"%s\"\n";


			/* collect MergedFB options */
    info->MergedFB = TRUE;
    info->UseRADEONXinerama = TRUE;
    info->CRT2IsScrn0 = FALSE;
    info->CRT2Position = rm6Clone;
    info->MergedFBXDPI = info->MergedFBYDPI = 0;

    if (info->MergeType == MT_NONE) {
	info->MergedFB = FALSE;
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
        "Failed to detect secondary monitor, MergedFB/Clone mode disabled\n");
    } else if (!pRM6Ent->MonInfo2) {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	"Failed to detect secondary monitor DDC, default HSync and VRefresh used\n");
	default_range = TRUE;
    }

    if (xf86GetOptValBool(info->Options, OPTION_MERGEDFB, &val)) {
        if (val) {
	    info->MergedFB = TRUE;
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	    "MergedFB mode forced on.\n");
        } else {
	    info->MergedFB = FALSE;
            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	    "MergedFB mode forced off.\n");
        }
    }

    /* Do some MergedFB mode initialisation */
    if(info->MergedFB) {
       info->CRT2pScrn = xalloc(sizeof(ScrnInfoRec));
       if(!info->CRT2pScrn) {
          xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	  	"Failed to allocate memory for merged pScrn, MergedFB mode is disabled\n");
	  info->MergedFB = FALSE;
       } else {
          memcpy(info->CRT2pScrn, pScrn, sizeof(ScrnInfoRec));
       }
    }
    if(info->MergedFB) {
	  strptr = (char *)xf86GetOptValString(info->Options, OPTION_CRT2POS);
      	  if(strptr) {
       	     if((!strcmp(strptr,"LeftOf")) || (!strcmp(strptr,"leftof"))) {
                info->CRT2Position = rm6LeftOf;
		info->CRT2IsScrn0 = TRUE;
	     }
 	     else if((!strcmp(strptr,"RightOf")) || (!strcmp(strptr,"rightof"))) {
                info->CRT2Position = rm6RightOf;
                info->CRT2IsScrn0 = FALSE;
	     }
	     else if((!strcmp(strptr,"Above")) || (!strcmp(strptr,"above"))) {
                info->CRT2Position = rm6Above;
                info->CRT2IsScrn0 = FALSE;
	     }
	     else if((!strcmp(strptr,"Below")) || (!strcmp(strptr,"below"))) {
                info->CRT2Position = rm6Below;
                info->CRT2IsScrn0 = TRUE;
	     }
	     else if((!strcmp(strptr,"Clone")) || (!strcmp(strptr,"clone"))) {
                info->CRT2Position = rm6Clone;
                /*info->CRT2IsScrn0 = FALSE; */
                 info->CRT2IsScrn0 = TRUE; 
	     }
	     else {
	        xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
	            "\"%s\" is not a valid parameter for Option \"CRT2Position\"\n", strptr);
	    	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	            "Valid parameters are \"RightOf\", \"LeftOf\", \"Above\", \"Below\", or \"Clone\"\n");
	     }
	  }
	  strptr = (char *)xf86GetOptValString(info->Options, OPTION_METAMODES);
	  if(strptr) {
	     info->MetaModes = xalloc(strlen(strptr) + 1);
	     if(info->MetaModes) memcpy(info->MetaModes, strptr, strlen(strptr) + 1);
	  }
	  strptr = (char *)xf86GetOptValString(info->Options, OPTION_CRT2HSYNC);
	  if(strptr) {
	      info->CRT2HSync = xalloc(strlen(strptr) + 1);
	      if(info->CRT2HSync) memcpy(info->CRT2HSync, strptr, strlen(strptr) + 1);
	  }
	  strptr = (char *)xf86GetOptValString(info->Options, OPTION_CRT2VREFRESH);
	  if(strptr) {
	      info->CRT2VRefresh = xalloc(strlen(strptr) + 1);
	      if(info->CRT2VRefresh) memcpy(info->CRT2VRefresh, strptr, strlen(strptr) + 1);
	  }

	if(xf86GetOptValBool(info->Options, OPTION_NORADEONXINERAMA, &val)) {
	    if (val)
		info->UseRADEONXinerama = FALSE;
	}
	if(info->UseRADEONXinerama) {
	        if(xf86GetOptValBool(info->Options, OPTION_CRT2ISSCRN0, &val)) {
		   if(val) info->CRT2IsScrn0 = TRUE;
		   else    info->CRT2IsScrn0 = FALSE;
		}
	}
	strptr = (char *)xf86GetOptValString(info->Options, OPTION_MERGEDDPI);
	if(strptr) {
	    int val1 = 0, val2 = 0;
	    sscanf(strptr, "%d %d", &val1, &val2);
	    if(val1 && val2) {
	        info->MergedFBXDPI = val1;
		info->MergedFBYDPI = val2;
	     } else {
	        xf86DrvMsg(pScrn->scrnIndex, X_WARNING, mybadparm, strptr, "MergedDPI");
	     }
 	}
    }

    if(info->MergedFB) {
          /* fill in monitor */
       info->CRT2pScrn->monitor = xalloc(sizeof(MonRec));
       if(info->CRT2pScrn->monitor) {
          DisplayModePtr tempm = NULL, currentm = NULL, newm = NULL;
          memcpy(info->CRT2pScrn->monitor, pScrn->monitor, sizeof(MonRec));
          info->CRT2pScrn->monitor->DDC = NULL;
	  info->CRT2pScrn->monitor->Modes = NULL;
	  info->CRT2pScrn->monitor->id = "CRT2 Monitor";
	  tempm = pScrn->monitor->Modes;
	  while(tempm) {
	         if(!(newm = xalloc(sizeof(DisplayModeRec)))) break;
	         memcpy(newm, tempm, sizeof(DisplayModeRec));
	         if(!(newm->name = xalloc(strlen(tempm->name) + 1))) {
	            xfree(newm);
		    break;
	         }
	         strcpy(newm->name, tempm->name);
	         if(!info->CRT2pScrn->monitor->Modes) 
		    info->CRT2pScrn->monitor->Modes = newm;
	         if(currentm) {
	            currentm->next = newm;
		    newm->prev = currentm;
	         }
	         currentm = newm;
	         tempm = tempm->next;
	  }

	  /* xf86SetDDCproperties(info->CRT2pScrn, pRM6Ent->MonInfo2); */
          info->CRT2pScrn->monitor->DDC = pRM6Ent->MonInfo2;
          if (default_range) {
             RM6StrToRanges(info->CRT2pScrn->monitor->hsync, default_hsync, MAX_HSYNC);
             RM6StrToRanges(info->CRT2pScrn->monitor->vrefresh, default_vrefresh, MAX_VREFRESH);
          }
          if(info->CRT2HSync) {
             info->CRT2pScrn->monitor->nHsync =
	    	RM6StrToRanges(info->CRT2pScrn->monitor->hsync, info->CRT2HSync, MAX_HSYNC);
          }
          if(info->CRT2VRefresh) {
             info->CRT2pScrn->monitor->nVrefresh =
	    	RM6StrToRanges(info->CRT2pScrn->monitor->vrefresh, info->CRT2VRefresh, MAX_VREFRESH);
          }

       } else {
          xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	  	"Failed to allocate memory for CRT2 monitor, MergedFB mode disabled.\n");
	  if(info->CRT2pScrn) xfree(info->CRT2pScrn);
    	  info->CRT2pScrn = NULL;
	  info->MergedFB = FALSE;
       }
    }
}

static void RM6SetDynamicClock(ScrnInfoPtr pScrn, int mode)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32 tmp;
    switch(mode) {
        case 0: /* Turn everything OFF (ForceON to everything)*/
            if ( !info->HasCRTC2 ) {
                tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
                tmp |= (RADEON_SCLK_FORCE_CP   | RADEON_SCLK_FORCE_HDP |
			RADEON_SCLK_FORCE_DISP1 | RADEON_SCLK_FORCE_TOP |
                        RADEON_SCLK_FORCE_E2   | RADEON_SCLK_FORCE_SE  |
			RADEON_SCLK_FORCE_IDCT | RADEON_SCLK_FORCE_VIP |
			RADEON_SCLK_FORCE_RE   | RADEON_SCLK_FORCE_PB  |
			RADEON_SCLK_FORCE_TAM  | RADEON_SCLK_FORCE_TDM |
                        RADEON_SCLK_FORCE_RB);
                OUTPLL(RADEON_SCLK_CNTL, tmp);
            } else if (info->ChipFamily == CHIP_FAMILY_RV350) {
                /* for RV350/M10, no delays are required. */
                tmp = INPLL(pScrn, R300_SCLK_CNTL2);
                tmp |= (R300_SCLK_FORCE_TCL |
                        R300_SCLK_FORCE_GA  |
			R300_SCLK_FORCE_CBA);
                OUTPLL(R300_SCLK_CNTL2, tmp);

                tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
                tmp |= (RADEON_SCLK_FORCE_DISP2 | RADEON_SCLK_FORCE_CP      |
                        RADEON_SCLK_FORCE_HDP   | RADEON_SCLK_FORCE_DISP1   |
                        RADEON_SCLK_FORCE_TOP   | RADEON_SCLK_FORCE_E2      |
                        R300_SCLK_FORCE_VAP     | RADEON_SCLK_FORCE_IDCT    |
			RADEON_SCLK_FORCE_VIP   | R300_SCLK_FORCE_SR        |
			R300_SCLK_FORCE_PX      | R300_SCLK_FORCE_TX        |
			R300_SCLK_FORCE_US      | RADEON_SCLK_FORCE_TV_SCLK |
                        R300_SCLK_FORCE_SU      | RADEON_SCLK_FORCE_OV0);
                OUTPLL(RADEON_SCLK_CNTL, tmp);

                tmp = INPLL(pScrn, RADEON_SCLK_MORE_CNTL);
                tmp |= RADEON_SCLK_MORE_FORCEON;
                OUTPLL(RADEON_SCLK_MORE_CNTL, tmp);

                tmp = INPLL(pScrn, RADEON_MCLK_CNTL);
                tmp |= (RADEON_FORCEON_MCLKA |
                        RADEON_FORCEON_MCLKB |
                        RADEON_FORCEON_YCLKA |
			RADEON_FORCEON_YCLKB |
                        RADEON_FORCEON_MC);
                OUTPLL(RADEON_MCLK_CNTL, tmp);

                tmp = INPLL(pScrn, RADEON_VCLK_ECP_CNTL);
                tmp &= ~(RADEON_PIXCLK_ALWAYS_ONb  | 
                         RADEON_PIXCLK_DAC_ALWAYS_ONb | 
			 R300_DISP_DAC_PIXCLK_DAC_BLANK_OFF); 
                OUTPLL(RADEON_VCLK_ECP_CNTL, tmp);

                tmp = INPLL(pScrn, RADEON_PIXCLKS_CNTL);
                tmp &= ~(RADEON_PIX2CLK_ALWAYS_ONb         | 
			 RADEON_PIX2CLK_DAC_ALWAYS_ONb     | 
			 RADEON_DISP_TVOUT_PIXCLK_TV_ALWAYS_ONb | 
			 R300_DVOCLK_ALWAYS_ONb            | 
			 RADEON_PIXCLK_BLEND_ALWAYS_ONb    | 
			 RADEON_PIXCLK_GV_ALWAYS_ONb       | 
			 R300_PIXCLK_DVO_ALWAYS_ONb        | 
			 RADEON_PIXCLK_LVDS_ALWAYS_ONb     | 
			 RADEON_PIXCLK_TMDS_ALWAYS_ONb     | 
			 R300_PIXCLK_TRANS_ALWAYS_ONb      | 
			 R300_PIXCLK_TVO_ALWAYS_ONb        | 
			 R300_P2G2CLK_ALWAYS_ONb            | 
			 R300_P2G2CLK_ALWAYS_ONb           | 
			 R300_DISP_DAC_PIXCLK_DAC2_BLANK_OFF); 
                OUTPLL(RADEON_PIXCLKS_CNTL, tmp);
            }  else {
                tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
                tmp |= (RADEON_SCLK_FORCE_CP | RADEON_SCLK_FORCE_E2);
                tmp |= RADEON_SCLK_FORCE_SE;

		if ( !info->HasCRTC2 ) {
                     tmp |= ( RADEON_SCLK_FORCE_RB    |
			      RADEON_SCLK_FORCE_TDM   |
			      RADEON_SCLK_FORCE_TAM   |
			      RADEON_SCLK_FORCE_PB    |
			      RADEON_SCLK_FORCE_RE    |
			      RADEON_SCLK_FORCE_VIP   |
			      RADEON_SCLK_FORCE_IDCT  |
			      RADEON_SCLK_FORCE_TOP   |
			      RADEON_SCLK_FORCE_DISP1 |
			      RADEON_SCLK_FORCE_DISP2 |
			      RADEON_SCLK_FORCE_HDP    );
		} else if ((info->ChipFamily == CHIP_FAMILY_R300) ||
			   (info->ChipFamily == CHIP_FAMILY_R350)) {
		    tmp |= ( RADEON_SCLK_FORCE_HDP   |
			     RADEON_SCLK_FORCE_DISP1 |
			     RADEON_SCLK_FORCE_DISP2 |
			     RADEON_SCLK_FORCE_TOP   |
			     RADEON_SCLK_FORCE_IDCT  |
			     RADEON_SCLK_FORCE_VIP);
		}
                OUTREG(RADEON_SCLK_CNTL, tmp);
            
                usleep(16000);

		if ((info->ChipFamily == CHIP_FAMILY_R300) ||
		    (info->ChipFamily == CHIP_FAMILY_R350)) {
                    tmp = INPLL(pScrn, R300_SCLK_CNTL2);
                    tmp |= ( R300_SCLK_FORCE_TCL |
			     R300_SCLK_FORCE_GA  |
			     R300_SCLK_FORCE_CBA);
                    OUTPLL(R300_SCLK_CNTL2, tmp);
		    usleep(16000);
		}

                if (info->IsIGP) {
                    tmp = INPLL(pScrn, RADEON_MCLK_CNTL);
                    tmp &= ~(RADEON_FORCEON_MCLKA |
			     RADEON_FORCEON_YCLKA);
                    OUTREG(RADEON_MCLK_CNTL, tmp);
		    usleep(16000);
		}
  
		if ((info->ChipFamily == CHIP_FAMILY_RV200) ||
		    (info->ChipFamily == CHIP_FAMILY_RV250) ||
		    (info->ChipFamily == CHIP_FAMILY_RV280)) {
                    tmp = INPLL(pScrn, RADEON_SCLK_MORE_CNTL);
		    tmp |= RADEON_SCLK_MORE_FORCEON;
                    OUTPLL(RADEON_SCLK_MORE_CNTL, tmp);
		    usleep(16000);
		}

                tmp = INPLL(pScrn, RADEON_PIXCLKS_CNTL);
                tmp &= ~(RADEON_PIX2CLK_ALWAYS_ONb         |
                         RADEON_PIX2CLK_DAC_ALWAYS_ONb     |
                         RADEON_PIXCLK_BLEND_ALWAYS_ONb    |
                         RADEON_PIXCLK_GV_ALWAYS_ONb       |
                         RADEON_PIXCLK_DIG_TMDS_ALWAYS_ONb |
                         RADEON_PIXCLK_LVDS_ALWAYS_ONb     |
                         RADEON_PIXCLK_TMDS_ALWAYS_ONb);

		OUTREG(RADEON_PIXCLKS_CNTL, tmp);
		usleep(16000);

                tmp = INPLL(pScrn, RADEON_VCLK_ECP_CNTL);
                tmp &= ~(RADEON_PIXCLK_ALWAYS_ONb  |
			 RADEON_PIXCLK_DAC_ALWAYS_ONb); 
                OUTPLL(RADEON_VCLK_ECP_CNTL, tmp);
	    }
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Dynamic Clock Scaling Disabled\n");
            break;
        case 1:
            if (!info->HasCRTC2) {
                tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
		if ((INREG(RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK) >
		    RADEON_CFG_ATI_REV_A13) { 
                    tmp &= ~(RADEON_SCLK_FORCE_CP | RADEON_SCLK_FORCE_RB);
                }
                tmp &= ~(RADEON_SCLK_FORCE_HDP  | RADEON_SCLK_FORCE_DISP1 |
			 RADEON_SCLK_FORCE_TOP  | RADEON_SCLK_FORCE_SE   |
			 RADEON_SCLK_FORCE_IDCT | RADEON_SCLK_FORCE_RE   |
			 RADEON_SCLK_FORCE_PB   | RADEON_SCLK_FORCE_TAM  |
			 RADEON_SCLK_FORCE_TDM);
                OUTPLL (RADEON_SCLK_CNTL, tmp);
	    } else if ((info->ChipFamily == CHIP_FAMILY_R300) ||
		       (info->ChipFamily == CHIP_FAMILY_R350) ||
		       (info->ChipFamily == CHIP_FAMILY_RV350)) {
		if (info->ChipFamily == CHIP_FAMILY_RV350) {
		    tmp = INPLL(pScrn, R300_SCLK_CNTL2);
		    tmp &= ~(R300_SCLK_FORCE_TCL |
			     R300_SCLK_FORCE_GA  |
			     R300_SCLK_FORCE_CBA);
		    tmp |=  (R300_SCLK_TCL_MAX_DYN_STOP_LAT |
			     R300_SCLK_GA_MAX_DYN_STOP_LAT  |
			     R300_SCLK_CBA_MAX_DYN_STOP_LAT);
		    OUTPLL(R300_SCLK_CNTL2, tmp);

		    tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
		    tmp &= ~(RADEON_SCLK_FORCE_DISP2 | RADEON_SCLK_FORCE_CP      |
			     RADEON_SCLK_FORCE_HDP   | RADEON_SCLK_FORCE_DISP1   |
			     RADEON_SCLK_FORCE_TOP   | RADEON_SCLK_FORCE_E2      |
			     R300_SCLK_FORCE_VAP     | RADEON_SCLK_FORCE_IDCT    |
			     RADEON_SCLK_FORCE_VIP   | R300_SCLK_FORCE_SR        |
			     R300_SCLK_FORCE_PX      | R300_SCLK_FORCE_TX        |
			     R300_SCLK_FORCE_US      | RADEON_SCLK_FORCE_TV_SCLK |
			     R300_SCLK_FORCE_SU      | RADEON_SCLK_FORCE_OV0);
		    tmp |=  RADEON_DYN_STOP_LAT_MASK;
		    OUTPLL(RADEON_SCLK_CNTL, tmp);

		    tmp = INPLL(pScrn, RADEON_SCLK_MORE_CNTL);
		    tmp &= ~RADEON_SCLK_MORE_FORCEON;
		    tmp |=  RADEON_SCLK_MORE_MAX_DYN_STOP_LAT;
		    OUTPLL(RADEON_SCLK_MORE_CNTL, tmp);

		    tmp = INPLL(pScrn, RADEON_VCLK_ECP_CNTL);
		    tmp |= (RADEON_PIXCLK_ALWAYS_ONb |
			    RADEON_PIXCLK_DAC_ALWAYS_ONb);   
		    OUTPLL(RADEON_VCLK_ECP_CNTL, tmp);

		    tmp = INPLL(pScrn, RADEON_PIXCLKS_CNTL);
		    tmp |= (RADEON_PIX2CLK_ALWAYS_ONb         |
			    RADEON_PIX2CLK_DAC_ALWAYS_ONb     |
			    RADEON_DISP_TVOUT_PIXCLK_TV_ALWAYS_ONb |
			    R300_DVOCLK_ALWAYS_ONb            |   
			    RADEON_PIXCLK_BLEND_ALWAYS_ONb    |
			    RADEON_PIXCLK_GV_ALWAYS_ONb       |
			    R300_PIXCLK_DVO_ALWAYS_ONb        | 
			    RADEON_PIXCLK_LVDS_ALWAYS_ONb     |
			    RADEON_PIXCLK_TMDS_ALWAYS_ONb     |
			    R300_PIXCLK_TRANS_ALWAYS_ONb      |
			    R300_PIXCLK_TVO_ALWAYS_ONb        |
			    R300_P2G2CLK_ALWAYS_ONb           |
			    R300_P2G2CLK_ALWAYS_ONb);
		    OUTPLL(RADEON_PIXCLKS_CNTL, tmp);

		    tmp = INPLL(pScrn, RADEON_MCLK_MISC);
		    tmp |= (RADEON_MC_MCLK_DYN_ENABLE |
			    RADEON_IO_MCLK_DYN_ENABLE);
		    OUTPLL(RADEON_MCLK_MISC, tmp);

		    tmp = INPLL(pScrn, RADEON_MCLK_CNTL);
		    tmp |= (RADEON_FORCEON_MCLKA |
			    RADEON_FORCEON_MCLKB);

		    tmp &= ~(RADEON_FORCEON_YCLKA  |
			     RADEON_FORCEON_YCLKB  |
			     RADEON_FORCEON_MC);

		    /* Some releases of vbios have set DISABLE_MC_MCLKA
		       and DISABLE_MC_MCLKB bits in the vbios table.  Setting these
		       bits will cause H/W hang when reading video memory with dynamic clocking
		       enabled. */
		    if ((tmp & R300_DISABLE_MC_MCLKA) &&
			(tmp & R300_DISABLE_MC_MCLKB)) {
			/* If both bits are set, then check the active channels */
			tmp = INPLL(pScrn, RADEON_MCLK_CNTL);
			if (info->RamWidth == 64) {
			    if (INREG(RADEON_MEM_CNTL) & R300_MEM_USE_CD_CH_ONLY)
				tmp &= ~R300_DISABLE_MC_MCLKB;
			    else
				tmp &= ~R300_DISABLE_MC_MCLKA;
			} else {
			    tmp &= ~(R300_DISABLE_MC_MCLKA |
				     R300_DISABLE_MC_MCLKB);
			}
		    }

		    OUTPLL(RADEON_MCLK_CNTL, tmp);
		} else {
		    tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
		    tmp &= ~(R300_SCLK_FORCE_VAP);
		    tmp |= RADEON_SCLK_FORCE_CP;
		    OUTPLL(RADEON_SCLK_CNTL, tmp);
		    usleep(15000);

		    tmp = INPLL(pScrn, R300_SCLK_CNTL2);
		    tmp &= ~(R300_SCLK_FORCE_TCL |
			     R300_SCLK_FORCE_GA  |
			     R300_SCLK_FORCE_CBA);
		    OUTPLL(R300_SCLK_CNTL2, tmp);
		}
	    } else {
                tmp = INPLL(pScrn, RADEON_CLK_PWRMGT_CNTL);

                tmp &= ~(RADEON_ACTIVE_HILO_LAT_MASK     | 
			 RADEON_DISP_DYN_STOP_LAT_MASK   | 
			 RADEON_DYN_STOP_MODE_MASK); 

                tmp |= (RADEON_ENGIN_DYNCLK_MODE |
			(0x01 << RADEON_ACTIVE_HILO_LAT_SHIFT));
                OUTPLL(RADEON_CLK_PWRMGT_CNTL, tmp);
		usleep(15000);

                tmp = INPLL(pScrn, RADEON_CLK_PIN_CNTL);
                tmp |= RADEON_SCLK_DYN_START_CNTL; 
                OUTPLL(RADEON_CLK_PIN_CNTL, tmp);
		usleep(15000);

		/* When DRI is enabled, setting DYN_STOP_LAT to zero can cause some R200 
		   to lockup randomly, leave them as set by BIOS.
		*/
                tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
                /*tmp &= RADEON_SCLK_SRC_SEL_MASK;*/
		tmp &= ~RADEON_SCLK_FORCEON_MASK;

                /*RAGE_6::A11 A12 A12N1 A13, RV250::A11 A12, R300*/
		if (((info->ChipFamily == CHIP_FAMILY_RV250) &&
		     ((INREG(RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK) <
		      RADEON_CFG_ATI_REV_A13)) || 
		    ((info->ChipFamily == CHIP_FAMILY_RV100) &&
		     ((INREG(RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK) <=
		      RADEON_CFG_ATI_REV_A13))){
                    tmp |= RADEON_SCLK_FORCE_CP;
                    tmp |= RADEON_SCLK_FORCE_VIP;
                }

                OUTPLL(RADEON_SCLK_CNTL, tmp);

		if ((info->ChipFamily == CHIP_FAMILY_RV200) ||
		    (info->ChipFamily == CHIP_FAMILY_RV250) ||
		    (info->ChipFamily == CHIP_FAMILY_RV280)) {
                    tmp = INPLL(pScrn, RADEON_SCLK_MORE_CNTL);
                    tmp &= ~RADEON_SCLK_MORE_FORCEON;

                    /* RV200::A11 A12 RV250::A11 A12 */
		    if (((info->ChipFamily == CHIP_FAMILY_RV200) ||
			 (info->ChipFamily == CHIP_FAMILY_RV250)) &&
			((INREG(RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK) <
			 RADEON_CFG_ATI_REV_A13)) {
                        tmp |= RADEON_SCLK_MORE_FORCEON;
		    }
                    OUTPLL(RADEON_SCLK_MORE_CNTL, tmp);
		    usleep(15000);
                }

                /* RV200::A11 A12, RV250::A11 A12 */
                if (((info->ChipFamily == CHIP_FAMILY_RV200) ||
		     (info->ChipFamily == CHIP_FAMILY_RV250)) &&
		    ((INREG(RADEON_CONFIG_CNTL) & RADEON_CFG_ATI_REV_ID_MASK) <
		     RADEON_CFG_ATI_REV_A13)) {
                    tmp = INPLL(pScrn, RADEON_PLL_PWRMGT_CNTL);
                    tmp |= RADEON_TCL_BYPASS_DISABLE;
                    OUTREG(RADEON_PLL_PWRMGT_CNTL, tmp);
                }
		usleep(15000);

                /*enable dynamic mode for display clocks (PIXCLK and PIX2CLK)*/
		tmp = INPLL(pScrn, RADEON_PIXCLKS_CNTL);
		tmp |=  (RADEON_PIX2CLK_ALWAYS_ONb         |
			 RADEON_PIX2CLK_DAC_ALWAYS_ONb     |
			 RADEON_PIXCLK_BLEND_ALWAYS_ONb    |
			 RADEON_PIXCLK_GV_ALWAYS_ONb       |
			 RADEON_PIXCLK_DIG_TMDS_ALWAYS_ONb |
			 RADEON_PIXCLK_LVDS_ALWAYS_ONb     |
			 RADEON_PIXCLK_TMDS_ALWAYS_ONb);

		OUTPLL(RADEON_PIXCLKS_CNTL, tmp);
		usleep(15000);

		tmp = INPLL(pScrn, RADEON_VCLK_ECP_CNTL);
		tmp |= (RADEON_PIXCLK_ALWAYS_ONb  |
		        RADEON_PIXCLK_DAC_ALWAYS_ONb); 

                OUTPLL(RADEON_VCLK_ECP_CNTL, tmp);
		usleep(15000);
            }    
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Dynamic Clock Scaling Enabled\n");
	    break;
        default:
	    break;
    }
}

void RM6FillInScreenInfo(ScrnInfoPtr pScrn)
{
    pScrn->driverVersion = RM6_VERSION_CURRENT;
    pScrn->driverName    = RM6_DRIVER_NAME;
    pScrn->name          = RM6_NAME;
    pScrn->PreInit       = RM6PreInit;
    pScrn->ScreenInit    = RM6ScreenInit;
    pScrn->SwitchMode    = RM6SwitchMode;
#ifdef X_XF86MiscPassMessage
    pScrn->HandleMessage = RM6HandleMessage;
#endif
    pScrn->AdjustFrame   = RM6AdjustFrame;
    pScrn->EnterVT       = RM6EnterVT;
    pScrn->LeaveVT       = RM6LeaveVT;
    pScrn->FreeScreen    = RM6FreeScreen;
    pScrn->ValidMode     = RM6ValidMode;
}
