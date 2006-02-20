/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6_accelfuncs.c,v 1.7tsi Exp $ */
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
 *   Michel Dänzer <michel@daenzer.net>
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


#if !defined(UNIXCPP) || defined(ANSICPP)
#define FUNC_NAME_CAT(prefix,suffix) prefix##suffix
#else
#define FUNC_NAME_CAT(prefix,suffix) prefix/**/suffix
#endif

#ifdef ACCEL_MMIO
#define FUNC_NAME(prefix) FUNC_NAME_CAT(prefix,MMIO)
#else
#error No accel type defined!
#endif

/* MMIO:
 *
 * Wait for the graphics engine to be completely idle: the FIFO has
 * drained, the Pixel Cache is flushed, and the engine is idle.  This is
 * a standard "sync" function that will make the hardware "quiescent".
 *
 */
void
FUNC_NAME(RM6WaitForIdle)(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;
    int            i    = 0;


    RADEONTRACE(("WaitForIdle (entering): %d entries, stat=0x%08x\n",
		     INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK,
		     INREG(RADEON_RBBM_STATUS)));

    /* Wait for the engine to go idle */
    RM6WaitForFifoFunction(pScrn, 64);

    for (;;) {
	for (i = 0; i < RADEON_TIMEOUT; i++) {
	    if (!(INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_ACTIVE)) {
		RM6EngineFlush(pScrn);
		return;
	    }
	}
	RADEONTRACE(("Idle timed out: %d entries, stat=0x%08x\n",
		     INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK,
		     INREG(RADEON_RBBM_STATUS)));
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Idle timed out, resetting engine...\n");
	RM6EngineReset(pScrn);
	RM6EngineRestore(pScrn);
    }
}

/* This callback is required for multiheader cards using XAA */
static void
FUNC_NAME(RM6RestoreAccelState)(ScrnInfoPtr pScrn)
{
    RM6InfoPtr  info       = RM6PTR(pScrn);
    unsigned char *RM6MMIO = info->MMIO;

#ifdef ACCEL_MMIO

    CARD32         pitch64;

    pitch64 = ((pScrn->displayWidth * (pScrn->bitsPerPixel / 8) + 0x3f)) >> 6;

    OUTREG(RADEON_DEFAULT_OFFSET, ((info->fbLocation + pScrn->fbOffset) >> 10)
				 | (pitch64 << 22));

    /* FIXME: May need to restore other things, like BKGD_CLK FG_CLK... */

    RM6WaitForIdleMMIO(pScrn);

#endif
}

/* Setup for XAA SolidFill */
static void
FUNC_NAME(RM6SetupForSolidFill)(ScrnInfoPtr pScrn,
				   int color,
				   int rop,
				   unsigned int planemask)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | RADEON_GMC_BRUSH_SOLID_COLOR
				     | RADEON_GMC_SRC_DATATYPE_COLOR
				     | RADEON_ROP[rop].pattern);

    BEGIN_ACCEL(4);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_BRUSH_FRGD_CLR,  color);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_ACCEL_REG(RADEON_DP_CNTL,            (RADEON_DST_X_LEFT_TO_RIGHT
					      | RADEON_DST_Y_TOP_TO_BOTTOM));

    FINISH_ACCEL();
}

/* Subsequent XAA SolidFillRect
 *
 * Tests: xtest CH06/fllrctngl, xterm
 */
static void
FUNC_NAME(RM6SubsequentSolidFillRect)(ScrnInfoPtr pScrn,
					 int x, int y,
					 int w, int h)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    BEGIN_ACCEL(2);

    OUT_ACCEL_REG(RADEON_DST_Y_X,          (y << 16) | x);
    OUT_ACCEL_REG(RADEON_DST_WIDTH_HEIGHT, (w << 16) | h);

    FINISH_ACCEL();
}

/* Setup for XAA solid lines */
static void
FUNC_NAME(RM6SetupForSolidLine)(ScrnInfoPtr pScrn,
				   int color,
				   int rop,
				   unsigned int planemask)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | RADEON_GMC_BRUSH_SOLID_COLOR
				     | RADEON_GMC_SRC_DATATYPE_COLOR
				     | RADEON_ROP[rop].pattern);

    if (info->ChipFamily >= CHIP_FAMILY_RV200) {
	BEGIN_ACCEL(1);
	OUT_ACCEL_REG(RADEON_DST_LINE_PATCOUNT,
		      0x55 << RADEON_BRES_CNTL_SHIFT);
	FINISH_ACCEL();
    }

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_BRUSH_FRGD_CLR,  color);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);

    FINISH_ACCEL();
}

/* Subsequent XAA solid horizontal and vertical lines */
static void
FUNC_NAME(RM6SubsequentSolidHorVertLine)(ScrnInfoPtr pScrn,
					    int x, int y,
					    int len,
					    int dir)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    int            w    = 1;
    int            h    = 1;
    ACCEL_PREAMBLE();

    if (dir == DEGREES_0) w = len;
    else                  h = len;

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DP_CNTL,          (RADEON_DST_X_LEFT_TO_RIGHT
					    | RADEON_DST_Y_TOP_TO_BOTTOM));
    OUT_ACCEL_REG(RADEON_DST_Y_X,          (y << 16) | x);
    OUT_ACCEL_REG(RADEON_DST_WIDTH_HEIGHT, (w << 16) | h);

    FINISH_ACCEL();
}

/* Subsequent XAA solid TwoPointLine line
 *
 * Tests: xtest CH06/drwln, ico, Mark Vojkovich's linetest program
 *
 * [See http://www.xfree86.org/devel/archives/devel/1999-Jun/0102.shtml for
 * Mark Vojkovich's linetest program, posted 2Jun99 to devel@xfree86.org.]
 */
static void
FUNC_NAME(RM6SubsequentSolidTwoPointLine)(ScrnInfoPtr pScrn,
					     int xa, int ya,
					     int xb, int yb,
					     int flags)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    /* TODO: Check bounds -- RADEON only has 14 bits */

    if (!(flags & OMIT_LAST))
	FUNC_NAME(RM6SubsequentSolidHorVertLine)(pScrn,
						    xb, yb, 1,
						    DEGREES_0);

    BEGIN_ACCEL(2);

    OUT_ACCEL_REG(RADEON_DST_LINE_START, (ya << 16) | xa);
    OUT_ACCEL_REG(RADEON_DST_LINE_END,   (yb << 16) | xb);

    FINISH_ACCEL();
}

/* Setup for XAA dashed lines
 *
 * Tests: xtest CH05/stdshs, XFree86/drwln
 *
 * NOTE: Since we can only accelerate lines with power-of-2 patterns of
 * length <= 32
 */
static void
FUNC_NAME(RM6SetupForDashedLine)(ScrnInfoPtr pScrn,
				    int fg,
				    int bg,
				    int rop,
				    unsigned int planemask,
				    int length,
				    unsigned char *pattern)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    CARD32         pat  = *(CARD32 *)(pointer)pattern;
    ACCEL_PREAMBLE();

    /* Save for determining whether or not to draw last pixel */
    info->dashLen = length;
    info->dashPattern = pat;

#if X_BYTE_ORDER == X_BIG_ENDIAN
# define PAT_SHIFT(pat, shift) (pat >> shift)
#else
# define PAT_SHIFT(pat, shift) (pat << shift)
#endif

    switch (length) {
    case  2: pat |= PAT_SHIFT(pat,  2);  /* fall through */
    case  4: pat |= PAT_SHIFT(pat,  4);  /* fall through */
    case  8: pat |= PAT_SHIFT(pat,  8);  /* fall through */
    case 16: pat |= PAT_SHIFT(pat, 16);
    }

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | (bg == -1
					? RADEON_GMC_BRUSH_32x1_MONO_FG_LA
					: RADEON_GMC_BRUSH_32x1_MONO_FG_BG)
				     | RADEON_ROP[rop].pattern
				     | RADEON_GMC_BYTE_LSB_TO_MSB);
    info->dash_fg = fg;
    info->dash_bg = bg;

    BEGIN_ACCEL((bg == -1) ? 4 : 5);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_ACCEL_REG(RADEON_DP_BRUSH_FRGD_CLR,  fg);
    if (bg != -1)
	OUT_ACCEL_REG(RADEON_DP_BRUSH_BKGD_CLR, bg);
    OUT_ACCEL_REG(RADEON_BRUSH_DATA0,        pat);

    FINISH_ACCEL();
}

/* Helper function to draw last point for dashed lines */
static void
FUNC_NAME(RM6DashedLastPel)(ScrnInfoPtr pScrn,
			       int x, int y,
			       int fg)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    CARD32         dp_gui_master_cntl = info->dp_gui_master_cntl_clip;
    ACCEL_PREAMBLE();

    dp_gui_master_cntl &= ~RADEON_GMC_BRUSH_DATATYPE_MASK;
    dp_gui_master_cntl |=  RADEON_GMC_BRUSH_SOLID_COLOR;

    dp_gui_master_cntl &= ~RADEON_GMC_SRC_DATATYPE_MASK;
    dp_gui_master_cntl |=  RADEON_GMC_SRC_DATATYPE_COLOR;

    BEGIN_ACCEL(7);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, dp_gui_master_cntl);
    OUT_ACCEL_REG(RADEON_DP_BRUSH_FRGD_CLR,  fg);
    OUT_ACCEL_REG(RADEON_DP_CNTL,            (RADEON_DST_X_LEFT_TO_RIGHT
					      | RADEON_DST_Y_TOP_TO_BOTTOM));
    OUT_ACCEL_REG(RADEON_DST_Y_X,            (y << 16) | x);
    OUT_ACCEL_REG(RADEON_DST_WIDTH_HEIGHT,   (1 << 16) | 1);

    /* Restore old values */
    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_BRUSH_FRGD_CLR,  info->dash_fg);

    FINISH_ACCEL();
}

/* Subsequent XAA dashed line */
static void
FUNC_NAME(RM6SubsequentDashedTwoPointLine)(ScrnInfoPtr pScrn,
					      int xa, int ya,
					      int xb, int yb,
					      int flags,
					      int phase)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    /* TODO: Check bounds -- RADEON only has 14 bits */

    if (!(flags & OMIT_LAST)) {
	int deltax = abs(xa - xb);
	int deltay = abs(ya - yb);
	int shift;

	if (deltax > deltay) shift = deltax;
	else                 shift = deltay;

	shift += phase;
	shift %= info->dashLen;

	if ((info->dashPattern >> shift) & 1)
	    FUNC_NAME(RM6DashedLastPel)(pScrn, xb, yb, info->dash_fg);
	else if (info->dash_bg != -1)
	    FUNC_NAME(RM6DashedLastPel)(pScrn, xb, yb, info->dash_bg);
    }

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DST_LINE_START,   (ya << 16) | xa);
    OUT_ACCEL_REG(RADEON_DST_LINE_PATCOUNT, phase);
    OUT_ACCEL_REG(RADEON_DST_LINE_END,     (yb << 16) | xb);

    FINISH_ACCEL();
}

/* Set up for transparency
 *
 * Mmmm, Seems as though the transparency compare is opposite to r128.
 * It should only draw when source != trans_color, this is the opposite
 * of that.
 */
static void
FUNC_NAME(RM6SetTransparency)(ScrnInfoPtr pScrn,
				 int trans_color)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

    if ((trans_color != -1) || (info->XAAForceTransBlit == TRUE)) {
	ACCEL_PREAMBLE();

	BEGIN_ACCEL(3);

	OUT_ACCEL_REG(RADEON_CLR_CMP_CLR_SRC, trans_color);
	OUT_ACCEL_REG(RADEON_CLR_CMP_MASK,    RADEON_CLR_CMP_MSK);
	OUT_ACCEL_REG(RADEON_CLR_CMP_CNTL,    (RADEON_SRC_CMP_EQ_COLOR
					       | RADEON_CLR_CMP_SRC_SOURCE));

	FINISH_ACCEL();
    }
}

/* Setup for XAA screen-to-screen copy
 *
 * Tests: xtest CH06/fllrctngl (also tests transparency)
 */
static void
FUNC_NAME(RM6SetupForScreenToScreenCopy)(ScrnInfoPtr pScrn,
					    int xdir, int ydir,
					    int rop,
					    unsigned int planemask,
					    int trans_color)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    info->xdir = xdir;
    info->ydir = ydir;

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | RADEON_GMC_BRUSH_NONE
				     | RADEON_GMC_SRC_DATATYPE_COLOR
				     | RADEON_ROP[rop].rop
				     | RADEON_DP_SRC_SOURCE_MEMORY);

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_ACCEL_REG(RADEON_DP_CNTL,
		  ((xdir >= 0 ? RADEON_DST_X_LEFT_TO_RIGHT : 0) |
		   (ydir >= 0 ? RADEON_DST_Y_TOP_TO_BOTTOM : 0)));

    FINISH_ACCEL();

    info->trans_color = trans_color;
    FUNC_NAME(RM6SetTransparency)(pScrn, trans_color);
}

/* Subsequent XAA screen-to-screen copy */
static void
FUNC_NAME(RM6SubsequentScreenToScreenCopy)(ScrnInfoPtr pScrn,
					      int xa, int ya,
					      int xb, int yb,
					      int w, int h)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    if (info->xdir < 0) xa += w - 1, xb += w - 1;
    if (info->ydir < 0) ya += h - 1, yb += h - 1;

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_SRC_Y_X,          (ya << 16) | xa);
    OUT_ACCEL_REG(RADEON_DST_Y_X,          (yb << 16) | xb);
    OUT_ACCEL_REG(RADEON_DST_HEIGHT_WIDTH, (h  << 16) | w);

    FINISH_ACCEL();
}

/* Setup for XAA mono 8x8 pattern color expansion.  Patterns with
 * transparency use `bg == -1'.  This routine is only used if the XAA
 * pixmap cache is turned on.
 *
 * Tests: xtest XFree86/fllrctngl (no other test will test this routine with
 *                                 both transparency and non-transparency)
 */
static void
FUNC_NAME(RM6SetupForMono8x8PatternFill)(ScrnInfoPtr pScrn,
					    int patternx,
					    int patterny,
					    int fg,
					    int bg,
					    int rop,
					    unsigned int planemask)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
#if X_BYTE_ORDER == X_BIG_ENDIAN
    unsigned char  pattern[8];
#endif
    ACCEL_PREAMBLE();

#if X_BYTE_ORDER == X_BIG_ENDIAN
    /* Take care of endianness */
    pattern[0] = (patternx & 0x000000ff);
    pattern[1] = (patternx & 0x0000ff00) >> 8;
    pattern[2] = (patternx & 0x00ff0000) >> 16;
    pattern[3] = (patternx & 0xff000000) >> 24;
    pattern[4] = (patterny & 0x000000ff);
    pattern[5] = (patterny & 0x0000ff00) >> 8;
    pattern[6] = (patterny & 0x00ff0000) >> 16;
    pattern[7] = (patterny & 0xff000000) >> 24;
#endif

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | (bg == -1
					? RADEON_GMC_BRUSH_8X8_MONO_FG_LA
					: RADEON_GMC_BRUSH_8X8_MONO_FG_BG)
				     | RADEON_ROP[rop].pattern
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
				     | RADEON_GMC_BYTE_MSB_TO_LSB
#endif
				    );

    BEGIN_ACCEL((bg == -1) ? 5 : 6);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_ACCEL_REG(RADEON_DP_BRUSH_FRGD_CLR,  fg);
    if (bg != -1)
	OUT_ACCEL_REG(RADEON_DP_BRUSH_BKGD_CLR, bg);
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
    OUT_ACCEL_REG(RADEON_BRUSH_DATA0,        patternx);
    OUT_ACCEL_REG(RADEON_BRUSH_DATA1,        patterny);
#else
    OUT_ACCEL_REG(RADEON_BRUSH_DATA0,        *(CARD32 *)(pointer)&pattern[0]);
    OUT_ACCEL_REG(RADEON_BRUSH_DATA1,        *(CARD32 *)(pointer)&pattern[4]);
#endif

    FINISH_ACCEL();
}

/* Subsequent XAA 8x8 pattern color expansion.  Because they are used in
 * the setup function, `patternx' and `patterny' are not used here.
 */
static void
FUNC_NAME(RM6SubsequentMono8x8PatternFillRect)(ScrnInfoPtr pScrn,
						  int patternx,
						  int patterny,
						  int x, int y,
						  int w, int h)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_BRUSH_Y_X,        (patterny << 8) | patternx);
    OUT_ACCEL_REG(RADEON_DST_Y_X,          (y << 16) | x);
    OUT_ACCEL_REG(RADEON_DST_HEIGHT_WIDTH, (h << 16) | w);

    FINISH_ACCEL();
}

#if 0
/* Setup for XAA color 8x8 pattern fill
 *
 * Tests: xtest XFree86/fllrctngl (with Mono8x8PatternFill off)
 */
static void
FUNC_NAME(RM6SetupForColor8x8PatternFill)(ScrnInfoPtr pScrn,
					     int patx, int paty,
					     int rop,
					     unsigned int planemask,
					     int trans_color)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | RADEON_GMC_BRUSH_8x8_COLOR
				     | RADEON_GMC_SRC_DATATYPE_COLOR
				     | RADEON_ROP[rop].pattern
				     | RADEON_DP_SRC_SOURCE_MEMORY);

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_ACCEL_REG(RADEON_SRC_Y_X,            (paty << 16) | patx);

    FINISH_ACCEL();

    info->trans_color = trans_color;
    FUNC_NAME(RM6SetTransparency)(pScrn, trans_color);
}

/* Subsequent XAA 8x8 pattern color expansion */
static void
FUNC_NAME(RM6SubsequentColor8x8PatternFillRect)(ScrnInfoPtr pScrn,
						   int patx, int paty,
						   int x, int y,
						   int w, int h)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_BRUSH_Y_X,        (paty << 16) | patx);
    OUT_ACCEL_REG(RADEON_DST_Y_X,          (y << 16) | x);
    OUT_ACCEL_REG(RADEON_DST_HEIGHT_WIDTH, (h << 16) | w);

    FINISH_ACCEL();
}
#endif


/* Setup for XAA indirect CPU-to-screen color expansion (indirect).
 * Because of how the scratch buffer is initialized, this is really a
 * mainstore-to-screen color expansion.  Transparency is supported when
 * `bg == -1'.
 */
static void
FUNC_NAME(RM6SetupForScanlineCPUToScreenColorExpandFill)(ScrnInfoPtr pScrn,
							    int fg,
							    int bg,
							    int rop,
							    unsigned int
							    planemask)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | RADEON_GMC_DST_CLIPPING
				     | RADEON_GMC_BRUSH_NONE
				     | (bg == -1
					? RADEON_GMC_SRC_DATATYPE_MONO_FG_LA
					: RADEON_GMC_SRC_DATATYPE_MONO_FG_BG)
				     | RADEON_ROP[rop].rop
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
				     | RADEON_GMC_BYTE_LSB_TO_MSB
#else
				     | RADEON_GMC_BYTE_MSB_TO_LSB
#endif
				     | RADEON_DP_SRC_SOURCE_HOST_DATA);

#ifdef ACCEL_MMIO

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
    BEGIN_ACCEL(4);
#else
    BEGIN_ACCEL(5);

    OUT_ACCEL_REG(RADEON_RBBM_GUICNTL,       RADEON_HOST_DATA_SWAP_NONE);
#endif
    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_ACCEL_REG(RADEON_DP_SRC_FRGD_CLR,    fg);
    OUT_ACCEL_REG(RADEON_DP_SRC_BKGD_CLR,    bg);

#endif

    FINISH_ACCEL();
}

/* Subsequent XAA indirect CPU-to-screen color expansion.  This is only
 * called once for each rectangle.
 */
static void
FUNC_NAME(RM6SubsequentScanlineCPUToScreenColorExpandFill)(ScrnInfoPtr
							      pScrn,
							      int x, int y,
							      int w, int h,
							      int skipleft)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
#ifdef ACCEL_MMIO
    ACCEL_PREAMBLE();

    info->scanline_h      = h;
    info->scanline_words  = (w + 31) >> 5;

#ifdef __alpha__
    /* Always use indirect for Alpha */
    if (0)
#else
    if ((info->scanline_words * h) <= 9)
#endif
    {
	/* Turn on direct for less than 9 dword colour expansion */
	info->scratch_buffer[0] =
	    (unsigned char *)(ADDRREG(RADEON_HOST_DATA_LAST)
			      - (info->scanline_words - 1));
	info->scanline_direct   = 1;
    } else {
	/* Use indirect for anything else */
	info->scratch_buffer[0] = info->scratch_save;
	info->scanline_direct   = 0;
    }

    BEGIN_ACCEL(4 + (info->scanline_direct ?
		     (info->scanline_words * h) : 0));

    OUT_ACCEL_REG(RADEON_SC_TOP_LEFT,      (y << 16)     | ((x+skipleft)
							    & 0xffff));
    OUT_ACCEL_REG(RADEON_SC_BOTTOM_RIGHT,  ((y+h) << 16) | ((x+w) & 0xffff));
    OUT_ACCEL_REG(RADEON_DST_Y_X,          (y << 16)     | (x & 0xffff));
    /* Have to pad the width here and use clipping engine */
    OUT_ACCEL_REG(RADEON_DST_HEIGHT_WIDTH, (h << 16)     | ((w + 31) & ~31));

    FINISH_ACCEL();

#endif
}

/* Subsequent XAA indirect CPU-to-screen color expansion and indirect
 * image write.  This is called once for each scanline.
 */
static void
FUNC_NAME(RM6SubsequentScanline)(ScrnInfoPtr pScrn,
				    int bufno)
{
    RM6InfoPtr    info = RM6PTR(pScrn);
#ifdef ACCEL_MMIO
    CARD32          *p    = (pointer)info->scratch_buffer[bufno];
    int              i;
    int              left = info->scanline_words;
    volatile CARD32 *d;
    ACCEL_PREAMBLE();

    if (info->scanline_direct) return;

    --info->scanline_h;

    while (left) {
	write_mem_barrier();
	if (left <= 8) {
	  /* Last scanline - finish write to DATA_LAST */
	  if (info->scanline_h == 0) {
	    BEGIN_ACCEL(left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA_LAST) - (left - 1); left; --left)
		*d++ = *p++;
	    return;
	  } else {
	    BEGIN_ACCEL(left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA7) - (left - 1); left; --left)
		*d++ = *p++;
	  }
	} else {
	    BEGIN_ACCEL(8);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA0), i = 0; i < 8; i++)
		*d++ = *p++;
	    left -= 8;
	}
    }

    FINISH_ACCEL();

#endif
}

/* Setup for XAA indirect image write */
static void
FUNC_NAME(RM6SetupForScanlineImageWrite)(ScrnInfoPtr pScrn,
					    int rop,
					    unsigned int planemask,
					    int trans_color,
					    int bpp,
					    int depth)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    info->scanline_bpp = bpp;

    /* Save for later clipping */
    info->dp_gui_master_cntl_clip = (info->dp_gui_master_cntl
				     | RADEON_GMC_DST_CLIPPING
				     | RADEON_GMC_BRUSH_NONE
				     | RADEON_GMC_SRC_DATATYPE_COLOR
				     | RADEON_ROP[rop].rop
				     | RADEON_GMC_BYTE_MSB_TO_LSB
				     | RADEON_DP_SRC_SOURCE_HOST_DATA);

#ifdef ACCEL_MMIO

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
    BEGIN_ACCEL(2);
#else
    BEGIN_ACCEL(3);

    if (bpp == 16)
	OUT_ACCEL_REG(RADEON_RBBM_GUICNTL,   RADEON_HOST_DATA_SWAP_16BIT);
    else if (bpp == 32)
	OUT_ACCEL_REG(RADEON_RBBM_GUICNTL,   RADEON_HOST_DATA_SWAP_32BIT);
    else
	OUT_ACCEL_REG(RADEON_RBBM_GUICNTL,   RADEON_HOST_DATA_SWAP_NONE);
#endif
    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);

#endif
    OUT_ACCEL_REG(RADEON_DP_WRITE_MASK,      planemask);

    FINISH_ACCEL();

    info->trans_color = trans_color;
    FUNC_NAME(RM6SetTransparency)(pScrn, trans_color);
}

/* Subsequent XAA indirect image write. This is only called once for
 * each rectangle.
 */
static void
FUNC_NAME(RM6SubsequentScanlineImageWriteRect)(ScrnInfoPtr pScrn,
						  int x, int y,
						  int w, int h,
						  int skipleft)
{
    RM6InfoPtr  info = RM6PTR(pScrn);

#ifdef ACCEL_MMIO

    int            shift = 0; /* 32bpp */
    ACCEL_PREAMBLE();

    if (pScrn->bitsPerPixel == 8) shift = 3;
    else if (pScrn->bitsPerPixel == 16) shift = 1;

    info->scanline_h      = h;
    info->scanline_words  = (w * info->scanline_bpp + 31) >> 5;

#ifdef __alpha__
    /* Always use indirect for Alpha */
    if (0)
#else
    if ((info->scanline_words * h) <= 9)
#endif
    {
	/* Turn on direct for less than 9 dword colour expansion */
	info->scratch_buffer[0]
	    = (unsigned char *)(ADDRREG(RADEON_HOST_DATA_LAST)
				- (info->scanline_words - 1));
	info->scanline_direct = 1;
    } else {
	/* Use indirect for anything else */
	info->scratch_buffer[0] = info->scratch_save;
	info->scanline_direct = 0;
    }

    BEGIN_ACCEL(4 + (info->scanline_direct ?
		     (info->scanline_words * h) : 0));

    OUT_ACCEL_REG(RADEON_SC_TOP_LEFT,      (y << 16)     | ((x+skipleft)
							    & 0xffff));
    OUT_ACCEL_REG(RADEON_SC_BOTTOM_RIGHT,  ((y+h) << 16) | ((x+w) & 0xffff));
    OUT_ACCEL_REG(RADEON_DST_Y_X,          (y << 16)     | (x & 0xffff));
    /* Have to pad the width here and use clipping engine */
    OUT_ACCEL_REG(RADEON_DST_HEIGHT_WIDTH, (h << 16)     | ((w + shift) &
							    ~shift));

    FINISH_ACCEL();

#endif
}

/* Set up the clipping rectangle */
static void
FUNC_NAME(RM6SetClippingRectangle)(ScrnInfoPtr pScrn,
				      int xa, int ya,
				      int xb, int yb)
{
    RM6InfoPtr  info = RM6PTR(pScrn);
    unsigned long  tmp1 = 0;
    unsigned long  tmp2 = 0;
    ACCEL_PREAMBLE();

    if (xa < 0) {
	tmp1 = (-xa) & 0x3fff;
	tmp1 |= RADEON_SC_SIGN_MASK_LO;
    } else {
	tmp1 = xa;
    }

    if (ya < 0) {
	tmp1 |= (((-ya) & 0x3fff) << 16);
	tmp1 |= RADEON_SC_SIGN_MASK_HI;
    } else {
	tmp1 |= (ya << 16);
    }

    xb++; yb++;

    if (xb < 0) {
	tmp2 = (-xb) & 0x3fff;
	tmp2 |= RADEON_SC_SIGN_MASK_LO;
    } else {
	tmp2 = xb;
    }

    if (yb < 0) {
	tmp2 |= (((-yb) & 0x3fff) << 16);
	tmp2 |= RADEON_SC_SIGN_MASK_HI;
    } else {
	tmp2 |= (yb << 16);
    }

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl_clip
					      | RADEON_GMC_DST_CLIPPING));
    OUT_ACCEL_REG(RADEON_SC_TOP_LEFT,        tmp1);
    OUT_ACCEL_REG(RADEON_SC_BOTTOM_RIGHT,    tmp2);

    FINISH_ACCEL();

    FUNC_NAME(RM6SetTransparency)(pScrn, info->trans_color);
}

/* Disable the clipping rectangle */
static void
FUNC_NAME(RM6DisableClipping)(ScrnInfoPtr pScrn)
{
    RM6InfoPtr info  = RM6PTR(pScrn);
    ACCEL_PREAMBLE();

    BEGIN_ACCEL(3);

    OUT_ACCEL_REG(RADEON_DP_GUI_MASTER_CNTL, info->dp_gui_master_cntl_clip);
    OUT_ACCEL_REG(RADEON_SC_TOP_LEFT,        0);
    OUT_ACCEL_REG(RADEON_SC_BOTTOM_RIGHT,    (RADEON_DEFAULT_SC_RIGHT_MAX |
					      RADEON_DEFAULT_SC_BOTTOM_MAX));

    FINISH_ACCEL();

    FUNC_NAME(RM6SetTransparency)(pScrn, info->trans_color);
}


void
FUNC_NAME(RM6AccelInit)(ScreenPtr pScreen, XAAInfoRecPtr a)
{
    ScrnInfoPtr    pScrn = xf86Screens[pScreen->myNum];
    RM6InfoPtr  info  = RM6PTR(pScrn);

    a->Flags                            = (PIXMAP_CACHE
					   | OFFSCREEN_PIXMAPS
					   | LINEAR_FRAMEBUFFER);

				/* Sync */
    a->Sync                             = FUNC_NAME(RM6WaitForIdle);

				/* Solid Filled Rectangle */
    a->PolyFillRectSolidFlags           = 0;
    a->SetupForSolidFill
	= FUNC_NAME(RM6SetupForSolidFill);
    a->SubsequentSolidFillRect
	= FUNC_NAME(RM6SubsequentSolidFillRect);

				/* Screen-to-screen Copy */
    a->ScreenToScreenCopyFlags          = 0;
    a->SetupForScreenToScreenCopy
	= FUNC_NAME(RM6SetupForScreenToScreenCopy);
    a->SubsequentScreenToScreenCopy
	= FUNC_NAME(RM6SubsequentScreenToScreenCopy);

				/* Mono 8x8 Pattern Fill (Color Expand) */
    a->SetupForMono8x8PatternFill
	= FUNC_NAME(RM6SetupForMono8x8PatternFill);
    a->SubsequentMono8x8PatternFillRect
	= FUNC_NAME(RM6SubsequentMono8x8PatternFillRect);
    a->Mono8x8PatternFillFlags          = (HARDWARE_PATTERN_PROGRAMMED_BITS
					   | HARDWARE_PATTERN_PROGRAMMED_ORIGIN
					   | HARDWARE_PATTERN_SCREEN_ORIGIN);

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
    if (info->ChipFamily >= CHIP_FAMILY_RV200)
	a->Mono8x8PatternFillFlags |= BIT_ORDER_IN_BYTE_MSBFIRST;
    else
	a->Mono8x8PatternFillFlags |= BIT_ORDER_IN_BYTE_LSBFIRST;
#else
    a->Mono8x8PatternFillFlags |= BIT_ORDER_IN_BYTE_LSBFIRST;
#endif

				/* Indirect CPU-To-Screen Color Expand */

    /* RADEON gets upset, when using HOST provided data without a source
       rop.  To show run 'xtest's drwarc. */
    a->ScanlineCPUToScreenColorExpandFillFlags
	= (LEFT_EDGE_CLIPPING
	   | ROP_NEEDS_SOURCE
	   | LEFT_EDGE_CLIPPING_NEGATIVE_X);
    a->NumScanlineColorExpandBuffers    = 1;
    a->ScanlineColorExpandBuffers       = info->scratch_buffer;
    if (!info->scratch_save)
	info->scratch_save
	    = xalloc(((pScrn->virtualX+31)/32*4)
		     + (pScrn->virtualX * info->CurrentLayout.pixel_bytes));
    info->scratch_buffer[0]             = info->scratch_save;
    a->SetupForScanlineCPUToScreenColorExpandFill
	= FUNC_NAME(RM6SetupForScanlineCPUToScreenColorExpandFill);
    a->SubsequentScanlineCPUToScreenColorExpandFill
	= FUNC_NAME(RM6SubsequentScanlineCPUToScreenColorExpandFill);
    a->SubsequentColorExpandScanline    = FUNC_NAME(RM6SubsequentScanline);

				/* Solid Lines */
    a->SetupForSolidLine
	= FUNC_NAME(RM6SetupForSolidLine);
    a->SubsequentSolidHorVertLine
	= FUNC_NAME(RM6SubsequentSolidHorVertLine);

#ifdef XFree86LOADER
    if (info->xaaReq.minorversion >= 1) {
#endif

    /* RADEON only supports 14 bits for lines and clipping and only
     * draws lines that are completely on-screen correctly.  This will
     * cause display corruption problem in the cases when out-of-range
     * commands are issued, like when dimming screen during GNOME logout
     * in dual-head setup.  Solid and dashed lines are therefore limited
     * to the virtual screen.
     */

    a->SolidLineFlags = LINE_LIMIT_COORDS;
    a->SolidLineLimits.x1 = 0;
    a->SolidLineLimits.y1 = 0;
    a->SolidLineLimits.x2 = pScrn->virtualX-1;
    a->SolidLineLimits.y2 = pScrn->virtualY-1;

    /* Call miSetZeroLineBias() to have mi/mfb/cfb/fb routines match
       hardware accel two point lines */
    miSetZeroLineBias(pScreen, (OCTANT5 | OCTANT6 | OCTANT7 | OCTANT8));

    a->SubsequentSolidTwoPointLine
	= FUNC_NAME(RM6SubsequentSolidTwoPointLine);

    /* Disabled on RV200 and newer because it does not pass XTest */
    if (info->ChipFamily < CHIP_FAMILY_RV200) {
	a->SetupForDashedLine
	    = FUNC_NAME(RM6SetupForDashedLine);
	a->SubsequentDashedTwoPointLine
	    = FUNC_NAME(RM6SubsequentDashedTwoPointLine);
	a->DashPatternMaxLength         = 32;
	/* ROP3 doesn't seem to work properly for dashedline with GXinvert */
	a->DashedLineFlags              = (LINE_PATTERN_LSBFIRST_LSBJUSTIFIED
					   | LINE_PATTERN_POWER_OF_2_ONLY
					   | LINE_LIMIT_COORDS
					   | ROP_NEEDS_SOURCE);
	a->DashedLineLimits.x1 = 0;
	a->DashedLineLimits.y1 = 0;
	a->DashedLineLimits.x2 = pScrn->virtualX-1;
	a->DashedLineLimits.y2 = pScrn->virtualY-1;
    }

#ifdef XFree86LOADER
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "libxaa too old, can't accelerate TwoPoint lines\n");
    }
#endif

    /* Clipping, note that without this, all line accelerations will
     * not be called
     */
    a->SetClippingRectangle
	= FUNC_NAME(RM6SetClippingRectangle);
    a->DisableClipping
	= FUNC_NAME(RM6DisableClipping);
    a->ClippingFlags
	= (HARDWARE_CLIP_SOLID_LINE
	   | HARDWARE_CLIP_DASHED_LINE
	/* | HARDWARE_CLIP_SOLID_FILL -- seems very slow with this on */
	   | HARDWARE_CLIP_MONO_8x8_FILL
	   | HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY);

    if (xf86IsEntityShared(info->pEnt->index)) {
	/* If there are more than one devices sharing this entity, we
	 * have to assign this call back, otherwise the XAA will be
	 * disabled
	 */
	if (xf86GetNumEntityInstances(info->pEnt->index) > 1)
	    a->RestoreAccelState        = FUNC_NAME(RM6RestoreAccelState);
    }

				/* ImageWrite */
    a->NumScanlineImageWriteBuffers     = 1;
    a->ScanlineImageWriteBuffers        = info->scratch_buffer;
    a->SetupForScanlineImageWrite
	= FUNC_NAME(RM6SetupForScanlineImageWrite);
    a->SubsequentScanlineImageWriteRect
	= FUNC_NAME(RM6SubsequentScanlineImageWriteRect);
    a->SubsequentImageWriteScanline     = FUNC_NAME(RM6SubsequentScanline);
    a->ScanlineImageWriteFlags          = (CPU_TRANSFER_PAD_DWORD
#ifdef ACCEL_MMIO
		/* Performance tests show that we shouldn't use GXcopy
		 * for uploads as a memcpy is faster
		 */
					  | NO_GXCOPY
#endif
		/* RADEON gets upset, when using HOST provided data
		 * without a source rop. To show run 'xtest's ptimg
		 */
					  | ROP_NEEDS_SOURCE
					  | SCANLINE_PAD_DWORD
					  | LEFT_EDGE_CLIPPING
					  | LEFT_EDGE_CLIPPING_NEGATIVE_X);

#if 0
				/* Color 8x8 Pattern Fill */
    a->SetupForColor8x8PatternFill
	= FUNC_NAME(RM6SetupForColor8x8PatternFill);
    a->SubsequentColor8x8PatternFillRect
	= FUNC_NAME(RM6SubsequentColor8x8PatternFillRect);
    a->Color8x8PatternFillFlags         = (HARDWARE_PATTERN_PROGRAMMED_ORIGIN
					   | HARDWARE_PATTERN_SCREEN_ORIGIN
					   | BIT_ORDER_IN_BYTE_LSBFIRST);
#endif

#ifdef RENDER
    if (info->RenderAccel
#ifdef XFree86LOADER
	&& info->xaaReq.minorversion >= 2
#endif
	) {

	a->CPUToScreenAlphaTextureFlags = XAA_RENDER_POWER_OF_2_TILE_ONLY;
	a->CPUToScreenAlphaTextureFormats = RM6TextureFormats;
	a->CPUToScreenAlphaTextureDstFormats = RM6DstFormats;
	a->CPUToScreenTextureFlags = XAA_RENDER_POWER_OF_2_TILE_ONLY;
	a->CPUToScreenTextureFormats = RM6TextureFormats;
	a->CPUToScreenTextureDstFormats = RM6DstFormats;

	if (IS_R300_VARIANT) {
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Render acceleration "
		       "unsupported on Radeon 9500/9700 and newer.\n");
	} else if ((info->ChipFamily == CHIP_FAMILY_RV250) || 
		   (info->ChipFamily == CHIP_FAMILY_RV280) || 
		   (info->ChipFamily == CHIP_FAMILY_RS300) || 
		   (info->ChipFamily == CHIP_FAMILY_R200)) {
	    a->SetupForCPUToScreenAlphaTexture2 =
		FUNC_NAME(R200SetupForCPUToScreenAlphaTexture);
	    a->SubsequentCPUToScreenAlphaTexture = 
		FUNC_NAME(R200SubsequentCPUToScreenTexture);

	    a->SetupForCPUToScreenTexture2 =
		FUNC_NAME(R200SetupForCPUToScreenTexture);
	    a->SubsequentCPUToScreenTexture =
		FUNC_NAME(R200SubsequentCPUToScreenTexture);
	} else {
	    a->SetupForCPUToScreenAlphaTexture2 =
		FUNC_NAME(R100SetupForCPUToScreenAlphaTexture);
	    a->SubsequentCPUToScreenAlphaTexture =
		FUNC_NAME(R100SubsequentCPUToScreenTexture);

	    a->SetupForCPUToScreenTexture2 =
		FUNC_NAME(R100SetupForCPUToScreenTexture);
	    a->SubsequentCPUToScreenTexture =
		FUNC_NAME(R100SubsequentCPUToScreenTexture);
	}
    } else if (info->RenderAccel) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Render acceleration currently "
		   "requires XAA v1.2 or newer.\n");
    }

    if (!a->SetupForCPUToScreenAlphaTexture2 && !a->SetupForCPUToScreenTexture2)
	info->RenderAccel = FALSE;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Render acceleration %s\n",
	       info->RenderAccel ? "enabled" : "disabled");
#endif /* RENDER */
}

#undef FUNC_NAME
