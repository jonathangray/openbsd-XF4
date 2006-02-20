/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/rm6.h,v 1.43 2003/11/06 18:38:00 tsi Exp $ */
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
 */

#ifndef _RM6_H_
#define _RM6_H_

#include "xf86str.h"

				/* PCI support */
#include "xf86Pci.h"

				/* XAA and Cursor Support */
#include "xaa.h"
#include "vbe.h"
#include "xf86Cursor.h"

				/* DDC support */
#include "xf86DDC.h"

				/* Xv support */
#include "xf86xv.h"

#include "rm6_probe.h"

				/* Render support */
#ifdef RENDER
#include "picturestr.h"
#endif

/* ------- mergedfb support ------------- */
		/* Psuedo Xinerama support */
#define NEED_REPLIES  		/* ? */
#define EXTENSION_PROC_ARGS void *
#include "extnsionst.h"  	/* required */
#include "panoramiXproto.h"  	/* required */
#define RM6_XINERAMA_MAJOR_VERSION  1
#define RM6_XINERAMA_MINOR_VERSION  1


/* Relative merge position */
typedef enum {
   rm6LeftOf,
   rm6RightOf,
   rm6Above,
   rm6Below,
   rm6Clone
} RM6Scrn2Rel;

/* ------------------------------------- */

#define RADEON_DEBUG            0 /* Turn off debugging output               */
#define RADEON_IDLE_RETRY      16 /* Fall out of idle loops after this count */
#define RADEON_TIMEOUT    2000000 /* Fall out of wait loops after this count */
#define RADEON_MMIOSIZE   0x80000

#define RADEON_VBIOS_SIZE 0x00010000
#define RADEON_USE_RMX 0x80000000 /* mode flag for using RMX
				   * Need to comfirm this is not used
				   * for something else.
				   */

#if RADEON_DEBUG
#define RADEONTRACE(x)							\
do {									\
    ErrorF("(**) %s(%d): ", RM6_NAME, pScrn->scrnIndex);		\
    ErrorF x;								\
} while (0);
#else
#define RADEONTRACE(x)
#endif


/* Other macros */
#define RM6_ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))
#define RM6_ALIGN(x,bytes) (((x) + ((bytes) - 1)) & ~((bytes) - 1))
#define RM6PTR(pScrn)      ((RM6InfoPtr)(pScrn)->driverPrivate)

typedef struct {
				/* Common registers */
    CARD32            ovr_clr;
    CARD32            ovr_wid_left_right;
    CARD32            ovr_wid_top_bottom;
    CARD32            ov0_scale_cntl;
    CARD32            mpp_tb_config;
    CARD32            mpp_gp_config;
    CARD32            subpic_cntl;
    CARD32            viph_control;
    CARD32            i2c_cntl_1;
    CARD32            gen_int_cntl;
    CARD32            cap0_trig_cntl;
    CARD32            cap1_trig_cntl;
    CARD32            bus_cntl;
    CARD32            surface_cntl;
    CARD32            bios_4_scratch;
    CARD32            bios_5_scratch;
    CARD32            bios_6_scratch;

				/* Other registers to save for VT switches */
    CARD32            dp_datatype;
    CARD32            rbbm_soft_reset;
    CARD32            clock_cntl_index;
    CARD32            amcgpio_en_reg;
    CARD32            amcgpio_mask;

				/* CRTC registers */
    CARD32            crtc_gen_cntl;
    CARD32            crtc_ext_cntl;
    CARD32            dac_cntl;
    CARD32            crtc_h_total_disp;
    CARD32            crtc_h_sync_strt_wid;
    CARD32            crtc_v_total_disp;
    CARD32            crtc_v_sync_strt_wid;
    CARD32            crtc_offset;
    CARD32            crtc_offset_cntl;
    CARD32            crtc_pitch;
    CARD32            disp_merge_cntl;
    CARD32            grph_buffer_cntl;
    CARD32            crtc_more_cntl;

				/* CRTC2 registers */
    CARD32            crtc2_gen_cntl;

    CARD32            dac2_cntl;
    CARD32            disp_output_cntl;
    CARD32            disp_hw_debug;
    CARD32            disp2_merge_cntl;
    CARD32            grph2_buffer_cntl;
    CARD32            crtc2_h_total_disp;
    CARD32            crtc2_h_sync_strt_wid;
    CARD32            crtc2_v_total_disp;
    CARD32            crtc2_v_sync_strt_wid;
    CARD32            crtc2_offset;
    CARD32            crtc2_offset_cntl;
    CARD32            crtc2_pitch;
				/* Flat panel registers */
    CARD32            fp_crtc_h_total_disp;
    CARD32            fp_crtc_v_total_disp;
    CARD32            fp_gen_cntl;
    CARD32            fp2_gen_cntl;
    CARD32            fp_h_sync_strt_wid;
    CARD32            fp2_h_sync_strt_wid;
    CARD32            fp_horz_stretch;
    CARD32            fp_panel_cntl;
    CARD32            fp_v_sync_strt_wid;
    CARD32            fp2_v_sync_strt_wid;
    CARD32            fp_vert_stretch;
    CARD32            lvds_gen_cntl;
    CARD32            lvds_pll_cntl;
    CARD32            tmds_pll_cntl;
    CARD32            tmds_transmitter_cntl;

				/* Computed values for PLL */
    CARD32            dot_clock_freq;
    CARD32            pll_output_freq;
    int               feedback_div;
    int               post_div;

				/* PLL registers */
    unsigned          ppll_ref_div;
    unsigned          ppll_div_3;
    CARD32            htotal_cntl;

				/* Computed values for PLL2 */
    CARD32            dot_clock_freq_2;
    CARD32            pll_output_freq_2;
    int               feedback_div_2;
    int               post_div_2;

				/* PLL2 registers */
    CARD32            p2pll_ref_div;
    CARD32            p2pll_div_0;
    CARD32            htotal_cntl2;

				/* Pallet */
    Bool              palette_valid;
    CARD32            palette[256];
    CARD32            palette2[256];

    CARD32            tv_dac_cntl;

} RM6SaveRec, *RM6SavePtr;

typedef struct {
    CARD16            reference_freq;
    CARD16            reference_div;
    CARD32            min_pll_freq;
    CARD32            max_pll_freq;
    CARD16            xclk;
} RM6PLLRec, *RM6PLLPtr;

typedef struct {
    int               bitsPerPixel;
    int               depth;
    int               displayWidth;
    int               pixel_code;
    int               pixel_bytes;
    DisplayModePtr    mode;
} RM6FBLayout;

typedef enum {
    CHIP_FAMILY_UNKNOW,
    CHIP_FAMILY_LEGACY,
    CHIP_FAMILY_RADEON,
    CHIP_FAMILY_RV100,
    CHIP_FAMILY_RS100,    /* U1 (IGP320M) or A3 (IGP320)*/
    CHIP_FAMILY_RV200,
    CHIP_FAMILY_RS200,    /* U2 (IGP330M/340M/350M) or A4 (IGP330/340/345/350), RS250 (IGP 7000) */
    CHIP_FAMILY_R200,
    CHIP_FAMILY_RV250,
    CHIP_FAMILY_RS300,    /* RS300/RS350 */
    CHIP_FAMILY_RV280,
    CHIP_FAMILY_R300,
    CHIP_FAMILY_R350,
    CHIP_FAMILY_RV350,
    CHIP_FAMILY_RV380,    /* RV370/RV380/M22/M24 */
    CHIP_FAMILY_R420,     /* R420/R423/M18 */
    CHIP_FAMILY_LAST
} RADEONChipFamily;

#define IS_RV100_VARIANT ((info->ChipFamily == CHIP_FAMILY_RV100)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RV200)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RS100)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RS200)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RV250)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RV280)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RS300))


#define IS_R300_VARIANT ((info->ChipFamily == CHIP_FAMILY_R300)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RV350) ||  \
        (info->ChipFamily == CHIP_FAMILY_R350)  ||  \
        (info->ChipFamily == CHIP_FAMILY_RV380) ||  \
        (info->ChipFamily == CHIP_FAMILY_R420))


typedef struct {
    CARD32 freq;
    CARD32 value;
}RM6TMDSPll;

typedef struct {
    EntityInfoPtr     pEnt;
    pciVideoPtr       PciInfo;
    PCITAG            PciTag;
    int               Chipset;
    RADEONChipFamily  ChipFamily;

    Bool              FBDev;

    unsigned long     LinearAddr;       /* Frame buffer physical address     */
    unsigned long     MMIOAddr;         /* MMIO region physical address      */
    unsigned long     BIOSAddr;         /* BIOS physical address             */
    unsigned int      fbLocation;

    unsigned char     *MMIO;            /* Map of MMIO region                */
    unsigned char     *FB;              /* Map of frame buffer               */
    CARD8             *VBIOS;           /* Video BIOS pointer                */

    Bool              IsAtomBios;       /* New BIOS used in R420 etc.        */
    int               ROMHeaderStart;   /* Start of the ROM Info Table       */
    int               MasterDataStart;  /* Offset for Master Data Table for ATOM BIOS */

    CARD32            MemCntl;
    CARD32            BusCntl;
    unsigned long     FbMapSize;        /* Size of frame buffer, in bytes    */
    int               Flags;            /* Saved copy of mode flags          */

				/* VE/M6 support */
    RADEONMonitorType DisplayType;      /* Monitor connected on              */
    RADEONDDCType     DDCType;
    RADEONConnectorType ConnectorType;
    Bool              HasCRTC2;         /* All cards except original Radeon  */
    Bool              IsMobility;       /* Mobile chips for laptops */
    Bool	      IsIBook;		/* iBook tweaks */
    Bool              IsIGP;            /* IGP chips */
    Bool              HasSingleDAC;     /* only TVDAC on chip */
    Bool              IsSecondary;      /* Second Screen                     */
    Bool              IsSwitching;      /* Flag for switching mode           */
    Bool              OverlayOnCRTC2;
    Bool              PanelOff;         /* Force panel (LCD/DFP) off         */
    Bool              ddc_mode;         /* Validate mode by matching exactly
					 * the modes supported in DDC data
					 */
    Bool              R300CGWorkaround;

				/* EDID or BIOS values for FPs */
    int               PanelXRes;
    int               PanelYRes;
    int               HOverPlus;
    int               HSyncWidth;
    int               HBlank;
    int               VOverPlus;
    int               VSyncWidth;
    int               VBlank;
    int               PanelPwrDly;
    int               DotClock;
    int               RefDivider;
    int               FeedbackDivider;
    int               PostDivider;
    Bool              UseBiosDividers;
				/* EDID data using DDC interface */
    Bool              ddc_bios;
    Bool              ddc1;
    Bool              ddc2;
    I2CBusPtr         pI2CBus;
    CARD32            DDCReg;

    RM6PLLRec      pll;
    RM6TMDSPll     tmds_pll[4];
    int               RamWidth;
    float	      sclk;		/* in MHz */
    float	      mclk;		/* in MHz */
    Bool	      IsDDR;
    int               DispPriority;

    RM6SaveRec     SavedReg;         /* Original (text) mode              */
    RM6SaveRec     ModeReg;          /* Current mode                      */
    Bool              (*CloseScreen)(int, ScreenPtr);

    void              (*BlockHandler)(int, pointer, pointer, pointer);

    Bool              PaletteSavedOnVT; /* Palette saved on last VT switch   */

    XAAInfoRecPtr     accel;
    Bool              accelOn;
    xf86CursorInfoPtr cursor;
    unsigned long     cursor_start;
    unsigned long     cursor_end;
#ifdef ARGB_CURSOR
    Bool	      cursor_argb;
#endif
    int               cursor_fg;
    int               cursor_bg;

    /*
     * XAAForceTransBlit is used to change the behavior of the XAA
     * SetupForScreenToScreenCopy function, to make it DGA-friendly.
     */
    Bool              XAAForceTransBlit;

    int               fifo_slots;       /* Free slots in the FIFO (64 max)   */
    int               pix24bpp;         /* Depth of pixmap for 24bpp fb      */
    Bool              dac6bits;         /* Use 6 bit DAC?                    */

				/* Computed values for Radeon */
    int               pitch;
    int               datatype;
    CARD32            dp_gui_master_cntl;
    CARD32            dp_gui_master_cntl_clip;
    CARD32            trans_color;

				/* Saved values for ScreenToScreenCopy */
    int               xdir;
    int               ydir;

				/* ScanlineScreenToScreenColorExpand support */
    unsigned char     *scratch_buffer[1];
    unsigned char     *scratch_save;
    int               scanline_x;
    int               scanline_y;
    int               scanline_w;
    int               scanline_h;
    int               scanline_h_w;
    int               scanline_words;
    int               scanline_direct;
    int               scanline_bpp;     /* Only used for ImageWrite */
    int               scanline_fg;
    int               scanline_bg;
    int               scanline_hpass;
    int               scanline_x1clip;
    int               scanline_x2clip;

				/* Saved values for DashedTwoPointLine */
    int               dashLen;
    CARD32            dashPattern;
    int               dash_fg;
    int               dash_bg;

    DGAModePtr        DGAModes;
    int               numDGAModes;
    Bool              DGAactive;
    int               DGAViewportStatus;
    DGAFunctionRec    DGAFuncs;

    RM6FBLayout    CurrentLayout;

				/* XVideo */
    XF86VideoAdaptorPtr adaptor;
    void              (*VideoTimerCallback)(ScrnInfoPtr, Time);
    FBLinearPtr       videoLinear;
    int               videoKey;

				/* Render */
    Bool              RenderAccel;
    Bool              RenderInited3D;
    FBLinearPtr       RenderTex;
    Bool              RenderTexValidR100;
    void              (*RenderCallback)(ScrnInfoPtr);
    Time              RenderTimeout;

				/* general */
    Bool              showCache;
    OptionInfoPtr     Options;
#ifdef XFree86LOADER
    XF86ModReqInfo    xaaReq;
#endif

    /* merged fb stuff, also covers clone modes */
    Bool		MergedFB;
    RM6Scrn2Rel	CRT2Position;
    char *		CRT2HSync;
    char *		CRT2VRefresh;
    char *		MetaModes;
    ScrnInfoPtr		CRT2pScrn;
    DisplayModePtr	CRT1Modes;
    DisplayModePtr	CRT1CurrentMode;
    int			CRT1frameX0;
    int			CRT1frameY0;
    int			CRT1frameX1;
    int			CRT1frameY1;
    RADEONMonitorType   MergeType;
    RADEONDDCType       MergeDDCType;
    void        	(*PointerMoved)(int index, int x, int y);
    /* pseudo xinerama support for mergedfb */
    int			maxCRT1_X1, maxCRT1_X2, maxCRT1_Y1, maxCRT1_Y2;
    int			maxCRT2_X1, maxCRT2_X2, maxCRT2_Y1, maxCRT2_Y2;
    int			maxClone_X1, maxClone_X2, maxClone_Y1, maxClone_Y2;
    Bool		UseRADEONXinerama;
    Bool		CRT2IsScrn0;
    ExtensionEntry 	*XineramaExtEntry;
    int			RADEONXineramaVX, RADEONXineramaVY;
    Bool		AtLeastOneNonClone;
    int			MergedFBXDPI, MergedFBYDPI;
    Bool		NoVirtual;

    /* special handlings for DELL triple-head server */
    Bool		IsDellServer; 
} RM6InfoRec, *RM6InfoPtr;


#define RM6WaitForFifo(pScrn, entries)				\
do {									\
    if (info->fifo_slots < entries)					\
	RM6WaitForFifoFunction(pScrn, entries);			\
    info->fifo_slots -= entries;					\
} while (0)

extern RM6EntPtr RM6EntPriv(ScrnInfoPtr pScrn);
extern void        RM6WaitForFifoFunction(ScrnInfoPtr pScrn, int entries);
extern void        RM6WaitForIdleMMIO(ScrnInfoPtr pScrn);

extern void        RM6DoAdjustFrame(ScrnInfoPtr pScrn, int x, int y,
				       int clone);

extern void        RM6EngineReset(ScrnInfoPtr pScrn);
extern void        RM6EngineFlush(ScrnInfoPtr pScrn);
extern void        RM6EngineRestore(ScrnInfoPtr pScrn);

extern unsigned    RM6INPLL(ScrnInfoPtr pScrn, int addr);
extern void        RM6WaitForVerticalSync(ScrnInfoPtr pScrn);
extern void        RM6WaitForVerticalSync2(ScrnInfoPtr pScrn);

extern void        RM6SelectBuffer(ScrnInfoPtr pScrn, int buffer);

extern Bool        RM6AccelInit(ScreenPtr pScreen);
extern void        RM6AccelInitMMIO(ScreenPtr pScreen, XAAInfoRecPtr a);
extern void        RM6EngineInit(ScrnInfoPtr pScrn);
extern Bool        RM6CursorInit(ScreenPtr pScreen);
extern Bool        RM6DGAInit(ScreenPtr pScreen);

extern int         RM6MinBits(int val);

extern void        RM6InitVideo(ScreenPtr pScreen);
extern void        RM6ResetVideo(ScrnInfoPtr pScrn);
extern void        R300CGWorkaround(ScrnInfoPtr pScrn);


#endif /* _RM6_H_ */
