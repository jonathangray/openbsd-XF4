/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_video.c,v 3.45 2001/10/28 03:34:00 tsi Exp $ */
/* $OpenBSD: sparc64_video.c,v 1.6 2002/07/27 21:41:41 matthieu Exp $ */
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
#include <sys/pciio.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)-1)
#endif

static char *devPrefix[] = {
	"/dev/ttyC",
	"/dev/ttyD",
};

#define N_DEV_PREFIX (sizeof(devPrefix)/sizeof(char *))
#define MAX_DEV_PER_PREFIX 8

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

static pointer sparc64MapVidMem(int, unsigned long, unsigned long, int);
static pointer sparc64MapVidMemTag(int, unsigned long, unsigned long, int, 
	PCITAG);
static void sparc64UnmapVidMem(int, pointer, unsigned long);

void
xf86OSInitVidMem(VidMemInfoPtr pVidMem)
{
	pVidMem->linearSupported = TRUE;
	pVidMem->mapMem = sparc64MapVidMem;
	pVidMem->mapMemTag = sparc64MapVidMemTag;
	pVidMem->unmapMem = sparc64UnmapVidMem;
	pVidMem->initialised = TRUE;
}


volatile unsigned char *ioBase = MAP_FAILED;

static pointer
sparc64MapVidMem(int ScreenNum, unsigned long Base, unsigned long Size, 
		 int Flags)
{
	int fd = xf86Info.screenFd;
	pointer base;

	base = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, Base);
	if (base == MAP_FAILED)
		FatalError("%s: could not mmap screen [s=%x,a=%x] (%s)\n",
			   "xf86MapVidMem", Size, Base, strerror(errno));

	return base;
}

static pointer
sparc64MapVidMemTag(int ScreenNum, unsigned long Base, 
		    unsigned long Size, int Flags, PCITAG tag)
{
	int fd = -1;
	pointer base;
	char devName[20];
	int i, j;
	struct pcisel sel;

	xf86DrvMsg(ScreenNum, X_INFO, 
		   "sparc64MapVidMem %lx, %lx, tag=%x:%x:%x\n", 
		   Base, Size, PCI_BUS_FROM_TAG(tag), PCI_DEV_FROM_TAG(tag),
		   PCI_FUNC_FROM_TAG(tag));

	/* Look  for the /dev entry that matches the PCI tag for this device */
	for (i = 0; i < N_DEV_PREFIX; i++) {
		for (j = 0; j < MAX_DEV_PER_PREFIX; j++) {
			snprintf(devName, sizeof(devName),
				 "%s%d", devPrefix[i], j);
			fd = open(devName, O_RDWR);
			if (fd >= 0) {
				if (ioctl(fd, WSDISPLAYIO_GPCIID, &sel) == 0 &&
					sel.pc_bus == PCI_BUS_FROM_TAG(tag) &&
				    	sel.pc_dev == PCI_DEV_FROM_TAG(tag) && 
				    	sel.pc_func == PCI_FUNC_FROM_TAG(tag)) {
					break;
				} else {
					close(fd);
				}
			}
		} /* for */
		if (fd != -1) {
			break;
		}
	} /* for */
	if (fd == -1) {
		FatalError("sparc64MapVideMem can't find /dev " 
			   "entry for PCI tag %x:%x:%x",
			   PCI_BUS_FROM_TAG(tag),
			   PCI_DEV_FROM_TAG(tag),
			   PCI_FUNC_FROM_TAG(tag));
	}
	xf86DrvMsg(ScreenNum, X_INFO, "sparc64MapVidMem using %s fd=%d\n", 
		   devName, fd);

	base = mmap(0, Size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, Base);
	if (base == MAP_FAILED)
		FatalError("%s: could not mmap screen [s=%x,a=%x] (%s)",
			   "xf86MapVidMem", Size, Base, strerror(errno));
	close(fd);
	return base;
}

static void
sparc64UnmapVidMem(int ScreenNum, pointer Base, unsigned long Size)
{
	munmap(Base, Size);
}

int
xf86ReadBIOS(unsigned long Base, unsigned long Offset, unsigned char *Buf,
	     int Len)
{
	return (0);
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
 * Do all things that need root privileges early 
 * and revoke those privileges 
 */
void
xf86DropPriv(void)
{
	pciInit();
	/* revoke privileges */
	seteuid(getuid());
	setuid(getuid());
}
