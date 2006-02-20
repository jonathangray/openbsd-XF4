/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6_accel.c,v 1.36 2003/11/10 18:41:22 tsi Exp $ */
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
 * Notes on unimplemented XAA optimizations:
 *
 *   SetClipping:   This has been removed as XAA expects 16bit registers
 *                  for full clipping.
 *   TwoPointLine:  The Radeon supports this. Not Bresenham.
 *   DashedLine with non-power-of-two pattern length: Apparently, there is
 *                  no way to set the length of the pattern -- it is always
 *                  assumed to be 8 or 32 (or 1024?).
 *   ScreenToScreenColorExpandFill: See p. 4-17 of the Technical Reference
 *                  Manual where it states that monochrome expansion of frame
 *                  buffer data is not supported.
 *   CPUToScreenColorExpandFill, direct: The implementation here uses a hybrid
 *                  direct/indirect method.  If we had more data registers,
 *                  then we could do better.  If XAA supported a trigger write
 *                  address, the code would be simpler.
 *   Color8x8PatternFill: Apparently, an 8x8 color brush cannot take an 8x8
 *                  pattern from frame buffer memory.
 *   ImageWrites:   Same as CPUToScreenColorExpandFill
 *
 */

				/* Driver data structures */
#include "rm6.h"
#include "rm6_macros.h"
#include "rm6_probe.h"
#include "radeon_reg.h"
#include "rm6_version.h"

				/* Line support */
#include "miline.h"

				/* X and server generic header files */
#include "xf86.h"

static struct {
    int rop;
    int pattern;
} RADEON_ROP[] = {
    { RADEON_ROP3_ZERO, RADEON_ROP3_ZERO }, /* GXclear        */
    { RADEON_ROP3_DSa,  RADEON_ROP3_DPa  }, /* Gxand          */
    { RADEON_ROP3_SDna, RADEON_ROP3_PDna }, /* GXandReverse   */
    { RADEON_ROP3_S,    RADEON_ROP3_P    }, /* GXcopy         */
    { RADEON_ROP3_DSna, RADEON_ROP3_DPna }, /* GXandInverted  */
    { RADEON_ROP3_D,    RADEON_ROP3_D    }, /* GXnoop         */
    { RADEON_ROP3_DSx,  RADEON_ROP3_DPx  }, /* GXxor          */
    { RADEON_ROP3_DSo,  RADEON_ROP3_DPo  }, /* GXor           */
    { RADEON_ROP3_DSon, RADEON_ROP3_DPon }, /* GXnor          */
    { RADEON_ROP3_DSxn, RADEON_ROP3_PDxn }, /* GXequiv        */
    { RADEON_ROP3_Dn,   RADEON_ROP3_Dn   }, /* GXinvert       */
    { RADEON_ROP3_SDno, RADEON_ROP3_PDno }, /* GXorReverse    */
    { RADEON_ROP3_Sn,   RADEON_ROP3_Pn   }, /* GXcopyInverted */
    { RADEON_ROP3_DSno, RADEON_ROP3_DPno }, /* GXorInverted   */
    { RADEON_ROP3_DSan, RADEON_ROP3_DPan }, /* GXnand         */
    { RADEON_ROP3_ONE,  RADEON_ROP3_ONE  }  /* GXset          */
};

/* The FIFO has 64 slots.  This routines waits until at least `entries'
 * of these slots are empty.
 */
void RM6WaitForFifoFunction(ScrnInfoPtr pScrn, int entries)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            i;

    for (;;) {
	for (i = 0; i < RADEON_TIMEOUT; i++) {
	    info->fifo_slots =
		INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK;
	    if (info->fifo_slots >= entries) return;
	}
	RADEONTRACE(("FIFO timed out: %d entries, stat=0x%08x\n",
		     INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK,
		     INREG(RADEON_RBBM_STATUS)));
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "FIFO timed out, resetting engine...\n");
	RM6EngineReset(pScrn);
	RM6EngineRestore(pScrn);
    }
}

/* Flush all dirty data in the Pixel Cache to memory */
void RM6EngineFlush(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            i;

    OUTREGP(RADEON_RB2D_DSTCACHE_CTLSTAT,
	    RADEON_RB2D_DC_FLUSH_ALL,
	    ~RADEON_RB2D_DC_FLUSH_ALL);
    for (i = 0; i < RADEON_TIMEOUT; i++) {
	if (!(INREG(RADEON_RB2D_DSTCACHE_CTLSTAT) & RADEON_RB2D_DC_BUSY))
	    break;
    }
}

/* Reset graphics card to known state */
void RM6EngineReset(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    CARD32         clock_cntl_index;
    CARD32         mclk_cntl;
    CARD32         rbbm_soft_reset;
    CARD32         host_path_cntl;

    RM6EngineFlush(pScrn);

    clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
    if (info->R300CGWorkaround) R300CGWorkaround(pScrn);

#if 0 /* taken care of by new PM code */
    /* Some ASICs have bugs with dynamic-on feature, which are
     * ASIC-version dependent, so we force all blocks on for now
     */
    if (info->HasCRTC2) {
	CARD32 tmp;

	tmp = INPLL(pScrn, RADEON_SCLK_CNTL);
	OUTPLL(RADEON_SCLK_CNTL, ((tmp & ~RADEON_DYN_STOP_LAT_MASK) |
				  RADEON_CP_MAX_DYN_STOP_LAT |
				  RADEON_SCLK_FORCEON_MASK));

	if (info->ChipFamily == CHIP_FAMILY_RV200) {
	    tmp = INPLL(pScrn, RADEON_SCLK_MORE_CNTL);
	    OUTPLL(RADEON_SCLK_MORE_CNTL, tmp | RADEON_SCLK_MORE_FORCEON);
	}
    }
#endif /* new PM code */

    mclk_cntl = INPLL(pScrn, RADEON_MCLK_CNTL);

#if 0 /* handled by new PM code */
    OUTPLL(RADEON_MCLK_CNTL, (mclk_cntl |
			      RADEON_FORCEON_MCLKA |
			      RADEON_FORCEON_MCLKB |
			      RADEON_FORCEON_YCLKA |
			      RADEON_FORCEON_YCLKB |
			      RADEON_FORCEON_MC |
			      RADEON_FORCEON_AIC));
#endif /* new PM code */

    /* Soft resetting HDP thru RBBM_SOFT_RESET register can cause some
     * unexpected behaviour on some machines.  Here we use
     * RADEON_HOST_PATH_CNTL to reset it.
     */
    host_path_cntl = INREG(RADEON_HOST_PATH_CNTL);
    rbbm_soft_reset = INREG(RADEON_RBBM_SOFT_RESET);

    if (IS_R300_VARIANT) {
	CARD32 tmp;

	OUTREG(RADEON_RBBM_SOFT_RESET, (rbbm_soft_reset |
					RADEON_SOFT_RESET_CP |
					RADEON_SOFT_RESET_HI |
					RADEON_SOFT_RESET_E2));
	INREG(RADEON_RBBM_SOFT_RESET);
	OUTREG(RADEON_RBBM_SOFT_RESET, 0);
	tmp = INREG(RADEON_RB2D_DSTCACHE_MODE);
	OUTREG(RADEON_RB2D_DSTCACHE_MODE, tmp | (1 << 17)); /* FIXME */
    } else {
	OUTREG(RADEON_RBBM_SOFT_RESET, (rbbm_soft_reset |
					RADEON_SOFT_RESET_CP |
					RADEON_SOFT_RESET_SE |
					RADEON_SOFT_RESET_RE |
					RADEON_SOFT_RESET_PP |
					RADEON_SOFT_RESET_E2 |
					RADEON_SOFT_RESET_RB));
	INREG(RADEON_RBBM_SOFT_RESET);
	OUTREG(RADEON_RBBM_SOFT_RESET, (rbbm_soft_reset & (CARD32)
					~(RADEON_SOFT_RESET_CP |
					  RADEON_SOFT_RESET_SE |
					  RADEON_SOFT_RESET_RE |
					  RADEON_SOFT_RESET_PP |
					  RADEON_SOFT_RESET_E2 |
					  RADEON_SOFT_RESET_RB)));
	INREG(RADEON_RBBM_SOFT_RESET);
    }

    OUTREG(RADEON_HOST_PATH_CNTL, host_path_cntl | RADEON_HDP_SOFT_RESET);
    INREG(RADEON_HOST_PATH_CNTL);
    OUTREG(RADEON_HOST_PATH_CNTL, host_path_cntl);

    if (IS_R300_VARIANT)
	OUTREG(RADEON_RBBM_SOFT_RESET, rbbm_soft_reset);

    OUTREG(RADEON_CLOCK_CNTL_INDEX, clock_cntl_index);
    OUTPLL(RADEON_MCLK_CNTL, mclk_cntl);
    if (info->R300CGWorkaround) R300CGWorkaround(pScrn);
}

/* Restore the acceleration hardware to its previous state */
void RM6EngineRestore(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            pitch64;

    RADEONTRACE(("EngineRestore (%d/%d)\n",
		 info->CurrentLayout.pixel_code,
		 info->CurrentLayout.bitsPerPixel));

    RM6WaitForFifo(pScrn, 1);

    /* NOTE: The following RB2D_DSTCACHE_MODE setting will cause the
     * R300 to hang.  ATI does not see a reason to change it from the
     * default BIOS settings (even on non-R300 cards).  This setting
     * might be removed in future versions of the Radeon driver.
     */

    /* Turn of all automatic flushing - we'll do it all */
    if (!IS_R300_VARIANT)
	OUTREG(RADEON_RB2D_DSTCACHE_MODE, 0);

    pitch64 = ((pScrn->displayWidth * (pScrn->bitsPerPixel / 8) + 0x3f)) >> 6;

    RM6WaitForFifo(pScrn, 1);
    OUTREG(RADEON_DEFAULT_OFFSET, ((info->fbLocation >> 10)
				  | (pitch64 << 22)));

    RM6WaitForFifo(pScrn, 1);
#if X_BYTE_ORDER == X_BIG_ENDIAN
    OUTREGP(RADEON_DP_DATATYPE,
	    RADEON_HOST_BIG_ENDIAN_EN,
	    ~RADEON_HOST_BIG_ENDIAN_EN);
#else
    OUTREGP(RADEON_DP_DATATYPE, 0, ~RADEON_HOST_BIG_ENDIAN_EN);
#endif

    /* Restore SURFACE_CNTL - only the first head contains valid data -ReneR */
    if (!info->IsSecondary) {
	OUTREG(RADEON_SURFACE_CNTL, info->ModeReg.surface_cntl);
    }

    RM6WaitForFifo(pScrn, 1);
    OUTREG(RADEON_DEFAULT_SC_BOTTOM_RIGHT, (RADEON_DEFAULT_SC_RIGHT_MAX
					    | RADEON_DEFAULT_SC_BOTTOM_MAX));
    RM6WaitForFifo(pScrn, 1);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_BRUSH_SOLID_COLOR
				       | RADEON_GMC_SRC_DATATYPE_COLOR));

    RM6WaitForFifo(pScrn, 7);
    OUTREG(RADEON_DST_LINE_START,    0);
    OUTREG(RADEON_DST_LINE_END,      0);
    OUTREG(RADEON_DP_BRUSH_FRGD_CLR, 0xffffffff);
    OUTREG(RADEON_DP_BRUSH_BKGD_CLR, 0x00000000);
    OUTREG(RADEON_DP_SRC_FRGD_CLR,   0xffffffff);
    OUTREG(RADEON_DP_SRC_BKGD_CLR,   0x00000000);
    OUTREG(RADEON_DP_WRITE_MASK,     0xffffffff);

    RM6WaitForIdleMMIO(pScrn);

#ifdef RENDER
    info->RenderInited3D = FALSE;
#endif
}

/* Initialize the acceleration hardware */
void RM6EngineInit(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

    RADEONTRACE(("EngineInit (%d/%d)\n",
		 info->CurrentLayout.pixel_code,
		 info->CurrentLayout.bitsPerPixel));

    OUTREG(RADEON_RB3D_CNTL, 0);

    RM6EngineReset(pScrn);

    switch (info->CurrentLayout.pixel_code) {
    case 8:  info->datatype = 2; break;
    case 15: info->datatype = 3; break;
    case 16: info->datatype = 4; break;
    case 24: info->datatype = 5; break;
    case 32: info->datatype = 6; break;
    default:
	RADEONTRACE(("Unknown depth/bpp = %d/%d (code = %d)\n",
		     info->CurrentLayout.depth,
		     info->CurrentLayout.bitsPerPixel,
		     info->CurrentLayout.pixel_code));
    }
    info->pitch = ((info->CurrentLayout.displayWidth / 8) *
		   (info->CurrentLayout.pixel_bytes == 3 ? 3 : 1));

    RADEONTRACE(("Pitch for acceleration = %d\n", info->pitch));

    info->dp_gui_master_cntl =
	((info->datatype << RADEON_GMC_DST_DATATYPE_SHIFT)
	 | RADEON_GMC_CLR_CMP_CNTL_DIS);


    RM6EngineRestore(pScrn);
}

#define ACCEL_MMIO
#define ACCEL_PREAMBLE()        unsigned char *RM6MMIO = info->MMIO
#define BEGIN_ACCEL(n)          RM6WaitForFifo(pScrn, (n))
#define OUT_ACCEL_REG(reg, val) OUTREG(reg, val)
#define FINISH_ACCEL()

#ifdef RENDER
#include "rm6_render.c"
#endif
#include "rm6_accelfuncs.c"

#undef ACCEL_MMIO
#undef ACCEL_PREAMBLE
#undef BEGIN_ACCEL
#undef OUT_ACCEL_REG
#undef FINISH_ACCEL


/* Initialize XAA for supported acceleration and also initialize the
 * graphics hardware for acceleration
 */
Bool RM6AccelInit(ScreenPtr pScreen)
{
    ScrnInfoPtr    pScrn = xf86Screens[pScreen->myNum];
    RM6InfoPtr  info  = RM6PTR(pScrn);
    XAAInfoRecPtr  a;

    if (!(a = info->accel = XAACreateInfoRec())) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "XAACreateInfoRec Error\n");
	return FALSE;
    }
    
    RM6AccelInitMMIO(pScreen, a);

    RM6EngineInit(pScrn);

    if (!XAAInit(pScreen, a)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "XAAInit Error\n");
	return FALSE;
    }

    return TRUE;
}
