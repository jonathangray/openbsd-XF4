/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/ppc_video.c,v 1.3 2002/11/09 17:28:08 herrb Exp $ */
/* $OpenBSD: ppc_video.c,v 1.8 2003/04/01 22:36:52 matthieu Exp $ */
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

#include "xf86_OSlib.h"
#include "xf86OSpriv.h"

#include "bus/Pci.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif


/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static pointer ppcMapVidMem(int, unsigned long, unsigned long, int);
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
	ErrorF("mapVidMem %lx, %lx, fd = %d\n", Base, Size, fd);
#endif

	base = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, Base);
	if (base == MAP_FAILED)
		FatalError("%s: could not mmap screen [s=%x,a=%x] (%s)",
			   "xf86MapVidMem", Size, Base, strerror(errno));

	return base;
}


static void
ppcUnmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{

	munmap(Base, Size);
}

static int kmem;

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	int rv;

#ifdef DEBUG
	ErrorF("xf86ReadBIOS() %lx %lx, %x\n", Base, Offset, Len);
#endif

	if (Base < 0x80000000) {
		xf86Msg(X_WARNING, "No VGA Base=%#lx\n", Base);
		close(kmem);
		return 0;
	}


	lseek(kmem, Base + Offset, 0);
	rv = read(kmem, Buf, Len);
	close(kmem);

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

/*
 * Do all initialisation that need root privileges 
 */
void
xf86PrivilegedInit(void)
{
 	kmem = open("/dev/xf86", 2);
 	if (kmem == -1) {
		ErrorF("errno: %d\n", errno);
 		FatalError("xf86PrivilegedInit: open /dev/xf86");
 	}
	pciInit();
	xf86OpenConsole();
}
