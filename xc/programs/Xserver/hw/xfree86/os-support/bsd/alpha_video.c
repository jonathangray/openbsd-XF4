/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_video.c,v 3.45 2001/10/28 03:34:00 tsi Exp $ */
/* $OpenBSD: alpha_video.c,v 1.5 2002/08/05 19:10:41 matthieu Exp $ */
/*
 * Copyright 1992 by Rich Murphey <Rich@Rice.edu>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Rich Murphey and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Rich Murphey and
 * David Wexelblat make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * RICH MURPHEY AND DAVID WEXELBLAT DISCLAIM ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL RICH MURPHEY OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XConsortium: bsd_video.c /main/10 1996/10/25 11:37:57 kaleb $ */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"

#include <sys/param.h>
#include <sys/sysctl.h>
#ifdef __OpenBSD__
#include <machine/cpu.h>
#endif
#include "xf86Axp.h"

#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#if defined(__NetBSD__) && !defined(MAP_FILE)
#define MAP_FLAGS MAP_SHARED
#else
#define MAP_FLAGS (MAP_FILE | MAP_SHARED)
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif


extern unsigned long dense_base(void);

static int axpSystem = -1;
static unsigned long hae_thresh;
static unsigned long hae_mask;
static unsigned long bus_base;
static unsigned long sparse_size;

static unsigned long
memory_base(void)
{
    static unsigned long base = 0;

    if (base == 0) {
	size_t len = sizeof(base);
	int error;
#ifdef __OpenBSD__
	int mib[3];

	mib[0] = CTL_MACHDEP;
	mib[1] = CPU_CHIPSET;
	mib[2] = CPU_CHIPSET_MEM;

	if ((error = sysctl(mib, 3, &base, &len, NULL, 0)) < 0)
#else
	if ((error = sysctlbyname("hw.chipset.memory", &base, &len,
				  0, 0)) < 0)
#endif
	    FatalError("xf86MapVidMem: can't find memory\n");
    }

    return base;
}

static int
has_bwx(void)
{
    static int bwx = 0;
    size_t len = sizeof(bwx);
    int error;
#ifdef __OpenBSD__
    int mib[3];

    mib[0] = CTL_MACHDEP;
    mib[1] = CPU_CHIPSET;
    mib[2] = CPU_CHIPSET_BWX;

    if ((error = sysctl(mib, 3, &bwx, &len, NULL, 0)) < 0)
#else
    if ((error = sysctlbyname("hw.chipset.bwx", &bwx, &len, 0, 0)) < 0)
#endif
	return FALSE;
    else
	return bwx;
}



#define BUS_BASE	dense_base()
#define BUS_BASE_BWX	memory_base()

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static Bool useDevMem = FALSE;
static int  devMemFd = -1;

#define DEV_MEM "/dev/mem"

static pointer mapVidMem(int, unsigned long, unsigned long, int);
static void unmapVidMem(int, pointer, unsigned long);
static pointer mapVidMemSparse(int, unsigned long, unsigned long, int);
static void unmapVidMemSparse(int, pointer, unsigned long);

/*
 * Check if /dev/mem can be mmap'd.  If it can't print a warning when
 * "warn" is TRUE.
 */
static void
checkDevMem(Bool warn)
{
	static Bool devMemChecked = FALSE;
	int fd;
	pointer base;

	if (devMemChecked)
	    return;
	devMemChecked = TRUE;

	if ((fd = open(DEV_MEM, O_RDWR)) >= 0)
	{
	    /* Try to map a page at the VGA address */
	    base = mmap((caddr_t)0, 4096, PROT_READ|PROT_WRITE,
				 MAP_FLAGS, fd, (off_t)0xA0000 + BUS_BASE);
	
	    if (base != MAP_FAILED)
	    {
		munmap((caddr_t)base, 4096);
		devMemFd = fd;
		useDevMem = TRUE;
		return;
	    } else {
		/* This should not happen */
		if (warn)
		{
		    xf86Msg(X_WARNING, "checkDevMem: failed to mmap %s (%s)\n",
			    DEV_MEM, strerror(errno));
		}
		useDevMem = FALSE;
		return;
	    }
	}
	if (warn)
	{ 
	    xf86Msg(X_WARNING, "checkDevMem: failed to open %s (%s)\n",
		    DEV_MEM, strerror(errno));
	    xf86ErrorF("\tlinear framebuffer access unavailable\n");
	} 
	useDevMem = FALSE;
	return;
}

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
	checkDevMem(TRUE);
	pVidMem->linearSupported = useDevMem;

	if (has_bwx()) {
	    xf86Msg(X_INFO,"Machine type has 8/16 bit access\n");
	    pVidMem->mapMem = mapVidMem;
	    pVidMem->unmapMem = unmapVidMem;
	} else {
	    xf86Msg(X_INFO,"Machine needs sparse mapping\n");
	    pVidMem->mapMem = mapVidMemSparse;
	    pVidMem->unmapMem = unmapVidMemSparse;
	    if (axpSystem == -1)
                axpSystem = bsdGetAXP(); 
	    hae_thresh = xf86AXPParams[axpSystem].hae_thresh;
            hae_mask = xf86AXPParams[axpSystem].hae_mask;
            sparse_size = xf86AXPParams[axpSystem].size;
	}
	pVidMem->initialised = TRUE;
}

static pointer
mapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	pointer base;

	checkDevMem(FALSE);

#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "mapVidMem  Base %lx Size %ld\n", Base, Size);
#endif

	Base = Base & ((1L<<32) - 1);

	if (useDevMem)
	{
	    if (devMemFd < 0) 
	    {
		FatalError("xf86MapVidMem: failed to open %s (%s)\n",
			   DEV_MEM, strerror(errno));
	    }
	    base = mmap((caddr_t)0, Size, PROT_READ|PROT_WRITE,
				 MAP_FLAGS, devMemFd, (off_t)Base + BUS_BASE_BWX);
	    if (base == MAP_FAILED)
	    {
		FatalError("%s: could not mmap %s [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", DEV_MEM, Size, Base, 
			   strerror(errno));
	    }
	    return(base);
	}
		
	/* else, mmap /dev/vga */
	if ((unsigned long)Base < 0xA0000 || (unsigned long)Base >= 0xC0000)
	{
		FatalError("%s: Address 0x%x outside allowable range\n",
			   "xf86MapVidMem", Base);
	}
	base = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_FLAGS,
			     xf86Info.screenFd,
			     (unsigned long)Base + BUS_BASE);
	if (base == MAP_FAILED)
	{
	    FatalError("xf86MapVidMem: Could not mmap /dev/vga (%s)\n",
		       strerror(errno));
	}
	return(base);
}

static void
unmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap((caddr_t)Base, Size);
}

/*
 * Read BIOS via mmap()ing DEV_MEM
 */

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	unsigned char *ptr;
	int psize;
	int mlen;

	checkDevMem(TRUE);
	if (devMemFd == -1) {
	    return(-1);
	}

	psize = xf86getpagesize();
	Offset += Base & (psize - 1);
	Base &= ~(psize - 1);
	mlen = (Offset + Len + psize - 1) & ~(psize - 1);
	ptr = (unsigned char *)mmap((caddr_t)0, mlen, PROT_READ,
					MAP_SHARED, devMemFd, (off_t)Base+BUS_BASE);
	if ((long)ptr == -1)
	{
		xf86Msg(X_WARNING, 
			"xf86ReadBIOS: %s mmap[s=%x,a=%x,o=%x] failed (%s)\n",
			DEV_MEM, Len, Base, Offset, strerror(errno));
		return(-1);
	}
#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "xf86ReadBIOS: BIOS at 0x%08x has signature 0x%04x\n",
		Base, ptr[0] | (ptr[1] << 8));
#endif
	(void)memcpy(Buf, (void *)(ptr + Offset), Len);
	(void)munmap((caddr_t)ptr, mlen);
#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "xf86ReadBIOS(%x, %x, Buf, %x)"
		"-> %02x %02x %02x %02x...\n",
		Base, Offset, Len, Buf[0], Buf[1], Buf[2], Buf[3]);
#endif
	return(Len);
}

#if defined(__FreeBSD__) || defined(__OpenBSD__)

extern int ioperm(unsigned long from, unsigned long num, int on);

void
xf86EnableIO()
{
	ioperm(0, 65536, TRUE);
	return;
}

void
xf86DisableIO()
{
	return;
}

#endif /* __FreeBSD__  */


/***************************************************************************/
/* Interrupt Handling section                                              */
/***************************************************************************/

Bool
xf86DisableInterrupts()
{

	return(TRUE);
}

void
xf86EnableInterrupts()
{

	return;
}



#define vuip    volatile unsigned int *

static unsigned long msb_set = 0;
static pointer memSBase = 0;
static pointer memBase = 0;

extern int readDense8(pointer Base, register unsigned long Offset);
extern int readDense16(pointer Base, register unsigned long Offset);
extern int readDense32(pointer Base, register unsigned long Offset);
extern void
writeDenseNB8(int Value, pointer Base, register unsigned long Offset);
extern void
writeDenseNB16(int Value, pointer Base, register unsigned long Offset);
extern void
writeDenseNB32(int Value, pointer Base, register unsigned long Offset);
extern void
writeDense8(int Value, pointer Base, register unsigned long Offset);
extern void
writeDense16(int Value, pointer Base, register unsigned long Offset);
extern void
writeDense32(int Value, pointer Base, register unsigned long Offset);

static int readSparse8(pointer Base, register unsigned long Offset);
static int readSparse16(pointer Base, register unsigned long Offset);
static int readSparse32(pointer Base, register unsigned long Offset);
static void
writeSparseNB8(int Value, pointer Base, register unsigned long Offset);
static void
writeSparseNB16(int Value, pointer Base, register unsigned long Offset);
static void
writeSparseNB32(int Value, pointer Base, register unsigned long Offset);
static void
writeSparse8(int Value, pointer Base, register unsigned long Offset);
static void
writeSparse16(int Value, pointer Base, register unsigned long Offset);
static void
writeSparse32(int Value, pointer Base, register unsigned long Offset);

#ifndef __OpenBSD__
#include <machine/sysarch.h>

extern int sysarch(int, char *);

struct parms {
	u_int64_t hae;
};

static int
sethae(u_int64_t hae)
{
	struct parms p;
	p.hae = hae;
	return (sysarch(ALPHA_SETHAE, (char *)&p));
}
#else

static int
sethae(u_int64_t hae)
{
	/* TBD */
	return -1;
}

#endif

static pointer
mapVidMemSparse(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
    static Bool was_here = FALSE;

    if (!was_here) {
      was_here = TRUE;

      checkDevMem(FALSE);

      xf86WriteMmio8 = writeSparse8;
      xf86WriteMmio16 = writeSparse16;
      xf86WriteMmio32 = writeSparse32;
      xf86WriteMmioNB8 = writeSparseNB8;
      xf86WriteMmioNB16 = writeSparseNB16;
      xf86WriteMmioNB32 = writeSparseNB32;
      xf86ReadMmio8 = readSparse8;
      xf86ReadMmio16 = readSparse16;
      xf86ReadMmio32 = readSparse32;
	
      memBase = mmap((caddr_t)0, 0x100000000,
		     PROT_READ | PROT_WRITE,
		     MAP_SHARED, devMemFd,
		     (off_t) dense_base());
      memSBase = mmap((caddr_t)0, 0x100000000,
		      PROT_READ | PROT_WRITE,
		      MAP_SHARED, devMemFd,
		      (off_t) memory_base());
      
      if (memSBase == MAP_FAILED || memBase == MAP_FAILED)	{
	FatalError("xf86MapVidMem: Could not mmap framebuffer (%s)\n",
		   strerror(errno));
      }
    }
    return (pointer)((unsigned long)memBase + Base);
}

static void
unmapVidMemSparse(int ScreenNum, pointer Base, unsigned long Size)
{
}

static int
readSparse8(pointer Base, register unsigned long Offset)
{
    register unsigned long result, shift;
    register unsigned long msb;

    mem_barrier();
    Offset += (unsigned long)Base - (unsigned long)memBase;
    shift = (Offset & 0x3) << 3;
      if (Offset >= (hae_thresh)) {
        msb = Offset & hae_mask;
        Offset -= msb;
	if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
	}
      }

    result = *(vuip) ((unsigned long)memSBase + (Offset << 5));
    result >>= shift;
    return 0xffUL & result;
}

static int
readSparse16(pointer Base, register unsigned long Offset)
{
    register unsigned long result, shift;
    register unsigned long msb;

    mem_barrier();
    Offset += (unsigned long)Base - (unsigned long)memBase;
    shift = (Offset & 0x2) << 3;
    if (Offset >= (hae_thresh)) {
        msb = Offset & hae_mask;
        Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    result = *(vuip)((unsigned long)memSBase+(Offset<<5)+(1<<(5-2)));
    result >>= shift;
    return 0xffffUL & result;
}

static int
readSparse32(pointer Base, register unsigned long Offset)
{
    mem_barrier();
    return *(vuip)((unsigned long)Base+(Offset));
}

static void
writeSparse8(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int b = Value & 0xffU;

    write_mem_barrier();
    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (hae_thresh)) {
      msb = Offset & hae_mask;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip) ((unsigned long)memSBase + (Offset << 5)) = b * 0x01010101;
}

static void
writeSparse16(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int w = Value & 0xffffU;

    write_mem_barrier();
    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (hae_thresh)) {
      msb = Offset & hae_mask;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip)((unsigned long)memSBase+(Offset<<5)+(1<<(5-2))) =
      w * 0x00010001;

}

static void
writeSparse32(int Value, pointer Base, register unsigned long Offset)
{
    write_mem_barrier();
    *(vuip)((unsigned long)Base + (Offset)) = Value;
    return;
}

static void
writeSparseNB8(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int b = Value & 0xffU;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (hae_thresh)) {
      msb = Offset & hae_mask;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip) ((unsigned long)memSBase + (Offset << 5)) = b * 0x01010101;
}

static void
writeSparseNB16(int Value, pointer Base, register unsigned long Offset)
{
    register unsigned long msb;
    register unsigned int w = Value & 0xffffU;

    Offset += (unsigned long)Base - (unsigned long)memBase;
    if (Offset >= (hae_thresh)) {
      msb = Offset & hae_mask ;
      Offset -= msb;
      if (msb_set != msb) {
	sethae(msb);
	msb_set = msb;
      }
    }
    *(vuip)((unsigned long)memSBase+(Offset<<5)+(1<<(5-2))) =
      w * 0x00010001;
}

static void
writeSparseNB32(int Value, pointer Base, register unsigned long Offset)
{
    *(vuip)((unsigned long)Base + (Offset)) = Value;
    return;
}

void (*xf86WriteMmio8)(int Value, pointer Base, unsigned long Offset) 
     = writeDense8;
void (*xf86WriteMmio16)(int Value, pointer Base, unsigned long Offset)
     = writeDense16;
void (*xf86WriteMmio32)(int Value, pointer Base, unsigned long Offset)
     = writeDense32;
void (*xf86WriteMmioNB8)(int Value, pointer Base, unsigned long Offset) 
     = writeDenseNB8;
void (*xf86WriteMmioNB16)(int Value, pointer Base, unsigned long Offset)
     = writeDenseNB16;
void (*xf86WriteMmioNB32)(int Value, pointer Base, unsigned long Offset)
     = writeDenseNB32;
int  (*xf86ReadMmio8)(pointer Base, unsigned long Offset) 
     = readDense8;
int  (*xf86ReadMmio16)(pointer Base, unsigned long Offset)
     = readDense16;
int  (*xf86ReadMmio32)(pointer Base, unsigned long Offset)
     = readDense32;

/*
 * Do all things that need root privileges early 
 * and revoke those priviledges 
 */
void
xf86DropPriv(void)
{
	checkDevMem(TRUE);
	xf86EnableIO();
	pciInit();
	/* revoke privileges */
	seteuid(getuid());
	setuid(getuid());
}
