/* $XFree86: ppc_video.c,v 1.5 2003/03/14 13:46:04 tsi Exp $ */
/* $OpenBSD: ppc_video.c,v 1.11 2006/01/15 22:08:12 matthieu Exp $ */
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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"

#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#include "bus/Pci.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif


/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

#ifndef __OpenBSD__
#define DEV_MEM "/dev/mem"
#else
#define DEV_MEM "/dev/xf86"
#endif

static pointer ppcMapVidMem(int, unsigned long, unsigned long, int flags);
static void ppcUnmapVidMem(int, pointer, unsigned long);

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
	pVidMem->linearSupported = TRUE;
	pVidMem->mapMem = ppcMapVidMem;
	pVidMem->unmapMem = ppcUnmapVidMem;
	pVidMem->initialised = TRUE;
}


volatile unsigned char *ioBase = MAP_FAILED;

static pointer
ppcMapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, int flags)
{
	int fd = xf86Info.screenFd;
	pointer base;
#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "mapVidMem %lx, %lx, fd = %d", 
		    Base, Size, fd);
#endif

	base = mmap(0, Size,
		    (flags & VIDMEM_READONLY) ?
		     PROT_READ : (PROT_READ | PROT_WRITE),
		    MAP_SHARED, fd, Base);
	if (base == MAP_FAILED)
		FatalError("%s: could not mmap screen [s=%lx,a=%lx] (%s)",
			   "xf86MapVidMem", Size, Base, strerror(errno));

	return base;
}

static void
ppcUnmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap(Base, Size);
}

static int kmem = -1;

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	int rv;

	if (Base < 0x80000000) {
		xf86Msg(X_WARNING, "No VGA Base=%#lx\n", Base);
		return 0;
	}

	if (kmem == -1) {
		kmem = open(DEV_MEM, 2);
		if (kmem == -1) {
			FatalError("xf86ReadBIOS: open %s", DEV_MEM);
		}
	}

#ifdef DEBUG
	xf86MsgVerb(X_INFO, 3, "xf86ReadBIOS() %lx %lx, %x\n", 
		    Base, Offset, Len);
#endif


	lseek(kmem, Base + Offset, 0);
	rv = read(kmem, Buf, Len);

	return rv;
}

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

#ifdef X_PRIVSEP
/*
 * Do all initialisation that need root privileges 
 */
void
xf86PrivilegedInit(void)
{
 	kmem = open(DEV_MEM, 2);
 	if (kmem == -1) {
		ErrorF("errno: %d\n", errno);
 		FatalError("xf86PrivilegedInit: open %s", DEV_MEM);
 	}
	pciInit();
	xf86OpenConsole();
}
#endif
