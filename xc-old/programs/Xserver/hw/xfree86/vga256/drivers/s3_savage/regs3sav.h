/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/regs3sav.h,v 1.1.2.1 1999/07/30 11:21:28 hohndel Exp $ */

/* regs3v.h
 *
 * Written by Jake Richter Copyright (c) 1989, 1990 Panacea Inc., Londonderry,
 * NH - All Rights Reserved
 *
 * This code may be freely incorporated in any program without royalty, as long
 * as the copyright notice stays intact.
 *
 * Additions by Kevin E. Martin (martin@cs.unc.edu)
 *
 * KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: regs3v.h /main/6 1996/10/27 18:06:49 kaleb $ */

/* Taken from accel/s3_virge code */
/* 23/03/97 S. Marineau: fixed bug with first Doubleword Offset macros 
 * and added macro CommandWaitIdle to wait for the command FIFO to empty 
 */


#ifndef _REGS3V_H
#define _REGS3V_H

/* for OUT instructions */
#include "compiler.h"

/* for new trio64V+ and 968 mmio */
#include "newmmio.h"


/* S3 chipset definitions */


#define UNLOCK_SYS_REGS	          do { \
				   outb(vgaCRIndex, 0x39); \
				   outb(vgaCRReg, 0xa5); } while (0)


#define VerticalRetraceWait() \
{ \
   outb(vgaCRIndex, 0x17); \
   if ( inb(vgaCRReg) & 0x80 ) { \
       while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
       while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x08) ; \
       while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
       }\
}

#if defined(__alpha__)
#define mb() __asm__ __volatile__("mb": : :"memory")
#define S3_IN8(addr)  xf86ReadSparse8(s3savMmioMemSparse, (addr))
#define S3_IN16(addr) xf86ReadSparse16(s3savMmioMemSparse, (addr))
#define S3_IN32(addr) *(volatile CARD32 *)((char*)s3savMmioMem + (addr))
#define S3_OUT8(addr, val)  do { xf86WriteSparse8((val),s3savMmioMemSparse,(addr)); \
				mb();} while(0)
#define S3_OUT16(addr, val) do { xf86WriteSparse16((val),s3savMmioMemSparse,(addr)); \
				mb();} while(0)
#define S3_OUT32(addr, val) do { *(volatile CARD32 *)((char*)s3savMmioMem + (addr)) = (val); \
				mb();} while(0)
#else /* __alpha__ */
#define S3_IN8(addr)  *(volatile CARD8 *)((char*)s3savMmioMem + (addr))
#define S3_IN16(addr) *(volatile CARD16 *)((char*)s3savMmioMem + (addr))
#define S3_IN32(addr) *(volatile CARD32 *)((char*)s3savMmioMem + (addr))
#define S3_OUT8(addr, val)  *(volatile CARD8 *)((char*)s3savMmioMem + (addr)) = (val)
#define S3_OUT16(addr, val) *(volatile CARD16 *)((char*)s3savMmioMem + (addr)) = (val)
#define S3_OUT32(addr, val) *(volatile CARD32 *)((char*)s3savMmioMem + (addr)) = (val)
#endif /* __alpha__ */

#define S3_SAVAGE3D_SERIES(chip)  ((chip>=S3_SAVAGE3D) && (chip<=S3_SAVAGE_MX))

#define S3_SAVAGE4_SERIES(chip)   ((chip==S3_SAVAGE4) || (chip==S3_PROSAVAGE))

#define S3_SAVAGE_SERIES(chip)    ((chip>=S3_SAVAGE3D) && (chip<=S3_SAVAGE2000))

/* PCI data */
#define PCI_S3_VENDOR_ID	0x5333
#define PCI_SAVAGE3D            0x8A20
#define PCI_SAVAGE3D_MV         0x8A21
#define PCI_SAVAGE4             0x8A22
#define PCI_SAVAGE2000		0x9102
#define PCI_SAVAGE_MX_MV	0x8C10
#define PCI_SAVAGE_MX		0x8C11
#define PCI_SAVAGE_IX_MV	0x8C12
#define PCI_SAVAGE_IX		0x8C13
#define PCI_PROSAVAGE_133	0x8A25
#define PCI_PROSAVAGE_K133	0x8A26
#define PCI_S3TWISTER		0x8D01
#define PCI_S3TWISTERK		0x8D02

/* Chip tags */
#define S3_UNKNOWN		 0
#define S3_SAVAGE3D              1
#define S3_SAVAGE_MX		 2
#define S3_SAVAGE4               3
#define S3_PROSAVAGE		 4
#define S3_SAVAGE2000            5


/* VESA Approved Register Definitions */
#define	DAC_MASK	0x03c6
#define	DAC_R_INDEX	0x03c7
#define	DAC_W_INDEX	0x03c8
#define	DAC_DATA	0x03c9


/* Subsystem Control Register */
#define	GPCTRL_NC	0x0000
#define	GPCTRL_ENAB	0x4000
#define	GPCTRL_RESET	0x8000


/* Command Register */
#define	CMD_OP_MSK	(0xf << 27)
#define	CMD_BITBLT	(0x0 << 27)
#define	CMD_RECT       ((0x2 << 27) | 0x0100)
#define	CMD_LINE	(0x3 << 27)
#define	CMD_POLYFILL	(0x5 << 27)
#define	CMD_NOP		(0xf << 27)

#define	BYTSEQ		0
#define	_16BIT		0
#define	PCDATA		0x80
#define	INC_Y		CMD_YP
#define	YMAJAXIS	0
#define	INC_X		CMD_XP
#define	DRAW		0x0020
#define	LINETYPE	0x0008
#define	LASTPIX		0
#define	PLANAR		0 /* MIX_MONO_SRC */
#define	WRTDATA		0

/*
 * Short Stroke Vector Transfer Register (The angular Defs also apply to the
 * Command Register
 */
#define	VECDIR_000	0x0000
#define	VECDIR_045	0x0020
#define	VECDIR_090	0x0040
#define	VECDIR_135	0x0060
#define	VECDIR_180	0x0080
#define	VECDIR_225	0x00a0
#define	VECDIR_270	0x00c0
#define	VECDIR_315	0x00e0
#define	SSVDRAW		0x0010

/* Command AutoExecute */
#define CMD_AUTOEXEC	0x01

/* Command Hardware Clipping Enable */
#define CMD_HWCLIP	0x02

/* Destination Color Format */
#define DST_8BPP	0x00
#define DST_16BPP	0x04
#define DST_24BPP	0x08

/* BLT Mix modes */
#define	MIX_BITBLT	0x0000
#define	MIX_MONO_SRC	0x0040
#define	MIX_CPUDATA	0x0080
#define	MIX_MONO_PATT	0x0100
#define MIX_COLOR_PATT  0x0000
#define	MIX_MONO_TRANSP	0x0200

/* Image Transfer Alignments */
#define CMD_ITA_BYTE	0x0000
#define CMD_ITA_WORD	0x0400
#define CMD_ITA_DWORD	0x0800

/* First Doubleword Offset (Image Transfer) */
#define CMD_FDO_BYTE0	0x00000
#define CMD_FDO_BYTE1	0x01000
#define CMD_FDO_BYTE2	0x02000
#define CMD_FDO_BYTE3	0x03000

/* X Positive, Y Positive (Bit BLT) */
#define CMD_XP		0x2000000
#define CMD_YP		0x4000000

/* 2D or 3D Select */
#define CMD_2D		0x00000000
#define CMD_3D		0x80000000

/* The Mix ROPs (selected ones, not all 256)  */

#define	ROP_0				(0x00<<16)
#define	ROP_DSon			(0x11<<16)
#define	ROP_DSna			(0x22<<16)
#define	ROP_Sn				(0x33<<16)
#define	ROP_SDna			(0x44<<16)
#define	ROP_Dn				(0x55<<16)
#define	ROP_DSx				(0x66<<16)
#define	ROP_DSan			(0x77<<16)
#define	ROP_DSa				(0x88<<16)
#define	ROP_DSxn			(0x99<<16)
#define	ROP_D				(0xaa<<16)
#define	ROP_DSno			(0xbb<<16)
#define	ROP_S				(0xcc<<16)
#define	ROP_SDno			(0xdd<<16)
#define	ROP_DSo				(0xee<<16)
#define	ROP_1				(0xff<<16)

/* ROP  ->  (ROP & P) | (D & ~P) */
#define	ROP_0_PaDPnao    /* DPna     */	(0x0a<<16)
#define	ROP_DSon_PaDPnao /* PDSPaox  */	(0x1a<<16)
#define	ROP_DSna_PaDPnao /* DPSana   */	(0x2a<<16)
#define	ROP_Sn_PaDPnao   /* SPDSxox  */	(0x3a<<16)
#define	ROP_SDna_PaDPnao /* DPSDoax  */	(0x4a<<16)
#define	ROP_Dn_PaDPnao   /* DPx      */	(0x5a<<16)
#define	ROP_DSx_PaDPnao  /* DPSax    */	(0x6a<<16)
#define	ROP_DSan_PaDPnao /* DPSDnoax */	(0x7a<<16)
#define	ROP_DSa_PaDPnao  /* DSPnoa   */	(0x8a<<16)
#define	ROP_DSxn_PaDPnao /* DPSnax   */	(0x9a<<16)
#define	ROP_D_PaDPnao    /* D        */	(0xaa<<16)
#define	ROP_DSno_PaDPnao /* DPSnao   */	(0xba<<16)
#define	ROP_S_PaDPnao    /* DPSDxax  */	(0xca<<16)
#define	ROP_SDno_PaDPnao /* DPSDanax */	(0xda<<16)
#define	ROP_DSo_PaDPnao  /* DPSao    */ (0xea<<16)
#define	ROP_1_PaDPnao    /* DPo      */	(0xfa<<16)


/* S -> P */
#define	ROP_DPon			(0x05<<16)
#define	ROP_DPna			(0x0a<<16)
#define	ROP_Pn				(0x0f<<16)
#define	ROP_PDna			(0x50<<16)
#define	ROP_DPx				(0x5a<<16)
#define	ROP_DPan			(0x5f<<16)
#define	ROP_DPa				(0xa0<<16)
#define	ROP_DPxn			(0xa5<<16)
#define	ROP_DPno			(0xaf<<16)
#define	ROP_P				(0xf0<<16)
#define	ROP_PDno			(0xf5<<16)
#define	ROP_DPo				(0xfa<<16)

/* ROP -> (ROP & S) | (~ROP & D) */
#define ROP_DPSDxax			(0xca<<16)
#define ROP_DSPnoa			(0x8a<<16)
#define ROP_DPSao			(0xea<<16)
#define ROP_DPSoa			(0xa8<<16)
#define ROP_DSa				(0x88<<16)
#define ROP_SSPxDSxax			(0xe8<<16)
#define ROP_SDPoa			(0xc8<<16)
#define ROP_DSPnao			(0xae<<16)
#define ROP_SSDxPDxax			(0x8e<<16)
#define ROP_DSo				(0xee<<16)
#define ROP_SDPnao			(0xce<<16)
#define ROP_SPDSxax			(0xac<<16)
#define ROP_SDPnoa			(0x8c<<16)
#define ROP_SDPao			(0xec<<16)

/* ROP_sp -> (ROP_sp & S) | (D & ~S) */
#define	ROP_0_SaDSnao    /* DSna     */	(0x22<<16)
#define	ROP_DPa_SaDSnao  /* DPSnoa   */	(0xa2<<16)
#define	ROP_PDna_SaDSnao /* DSPDoax  */	(0x62<<16)
#define	ROP_P_SaDSnao    /* DSPDxax  */	(0xe2<<16)
#define	ROP_DPna_SaDSnao /* DPSana   */	(0x2a<<16)
#define	ROP_D_SaDSnao    /* D        */	(0xaa<<16)
#define	ROP_DPx_SaDSnao  /* DPSax    */	(0x6a<<16)
#define	ROP_DPo_SaDSnao  /* DPSao    */	(0xea<<16)
#define	ROP_DPon_SaDSnao /* SDPSaox  */	(0x26<<16)
#define	ROP_DPxn_SaDSnao /* DSPnax   */	(0xa6<<16)
#define	ROP_Dn_SaDSnao   /* DSx      */	(0x66<<16)
#define	ROP_PDno_SaDSnao /* SDPSanax */	(0xe6<<16)
#define	ROP_Pn_SaDSnao   /* PSDPxox  */	(0x2e<<16)
#define	ROP_DPno_SaDSnao /* DSPnao   */	(0xae<<16)
#define	ROP_DPan_SaDSnao /* SDPSnoax */	(0x6e<<16)
#define	ROP_1_SaDSnao    /* DSo      */	(0xee<<16)


typedef struct {
   unsigned char r, g, b;
}
LUTENTRY;

#define MAXLOOP 0xfffff /* timeout value for engine waits, ~6 secs */
/*
 * The correct value for MAXFIFO actually depends on the size of the 
 * command-overflow-buffer in MM48C14.  We happen to use 32K always.
 */
#define MAXFIFO 0x7f00  /* Number of on-chip and off-chip FIFO slots */
void S3SAVGEReset(int from_timeout, int line, char *file);

/* Wait until "v" queue entries are free */

#define WaitQueue(v) \
  if( s3vPriv.NoPCIRetry ) { \
    if( (*s3vPriv.WaitQueue)(v) ) \
      S3SAVGEReset(1,__LINE__,__FILE__); \
  }

/* Wait until GP is idle and queue is empty */

#define WaitIdleEmpty() \
  if( (*s3vPriv.WaitIdleEmpty)() ) { \
   /*S3SAVGEReset(1,__LINE__,__FILE__);*/ \
  }

/* Wait until GP is idle */

#define WaitIdle() \
  if( s3vPriv.WaitIdle() ) { \
    /*S3SAVGEReset(1,__LINE__,__FILE__);*/ \
  }

/* Wait until Command FIFO is empty */

#define WaitCommandEmpty() \
  if( s3vPriv.WaitCommandEmpty() ) { \
    /* S3SAVGEReset(1,__LINE__,__FILE__); */ \
  }

/* Wait until a DMA transfer is done */ 
#define WaitDMAEmpty() \
  do { int loop=0; mem_barrier(); \
       while  (((((mmtr)s3savMmioMem)->dma_regs.regs.cmd.write_pointer) != (((mmtr)s3savMmioMem)->dma_regs.regs.cmd.read_pointer)) && (loop++<MAXLOOP)); \
       if (loop >= MAXLOOP) S3SAVGEReset(1,__LINE__,__FILE__); \
  } while(0)

#ifndef NULL
#define NULL	0
#endif

#define RGB8_PSEUDO      (-1)
#define RGB16_565         0
#define RGB16_555         1
#define RGB32_888         2

#endif /* _REGS3V_H */
