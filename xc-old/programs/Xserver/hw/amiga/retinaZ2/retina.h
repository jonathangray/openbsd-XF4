/***********************************************************

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

Author: Andy Heffernan

******************************************************************/

#ifndef _RETINA_H_
#define _RETINA_H_

#include    <errno.h>
#include    <sys/param.h>
#include    <sys/types.h>
#include    <sys/ioctl.h>
#include    <sys/queue.h>
#include    <sys/device.h>
#ifndef GRFIOCGINFO	/* not protected against multiple includes */
#include    "/sys/arch/amiga/dev/grfioctl.h"
#endif
#ifndef GF_ALIVE	/* not protected against multiple inclusion */
#include    "/sys/arch/amiga/dev/grfvar.h"
#endif
#include    "/sys/arch/amiga/dev/grf_rtreg.h"

/*
 * Define the following symbol if you want the vga bank routines
 * inlined into the cfb.banked source.  This is great for speed
 * but makes debugging difficult, especially when a modification
 * to the inlines causes all of cfb.banked to be recompiled.
 */
#define WANT_INLINES

#define DFLT_RETINA_MON_FILE	"RetinaMonDef"

extern void *vgaReadBottom, *vgaReadTop;
extern void *vgaWriteBottom, *vgaWriteTop;
extern Bool vgaReadFlag;
extern Bool vgaWriteFlag;
extern u_long VGABASE;
extern void *vgaregs;
extern u_long retina_writeseg, retina_readseg, retina_saveseg;

extern char *mondeffile;

#define MDB_DBL		0
#define MDB_LACE	1
#define MDB_CLKDIV2	2

#define MDF_DBL		(1 << MDB_DBL)
#define MDF_LACE	(1 << MDB_LACE)
#define MDF_CLKDIV2	(1 << MDB_CLKDIV2)

static __inline void
retina_setbank(int bank)
{
    WSeq(vgaregs, SEQ_ID_PRIM_HOST_OFF_LO, (unsigned char) (bank << 10));
    WSeq(vgaregs, SEQ_ID_PRIM_HOST_OFF_HI, (unsigned char) (bank << 2));
}

static __inline void
retina_setotherbank(int bank)
{
    WSeq(vgaregs, SEQ_ID_SEC_HOST_OFF_LO, (unsigned char) (bank << 10));
    WSeq(vgaregs, SEQ_ID_SEC_HOST_OFF_HI, (unsigned char) (bank << 2));
}

#define vgaSegmentShift	16
#define vgaSegmentMask	0xffff
#define vgaSegmentSize	65536

#ifdef WANT_INLINES
#define INLINE	static __inline
#include "retina_inlines.c"
#else
#define INLINE
extern void *vgaSetReadWrite();
extern void *vgaReadWriteNext();
extern void *vgaReadWritePrev();
extern void *vgaSetRead();
extern void *vgaReadNext();
extern void *vgaReadPrev();
extern void *vgaSetWrite();
extern void *vgaWriteNext();
extern void *vgaWritePrev();
extern void vgaSaveBank();
extern void vgaRestoreBank();
extern void vgaPushRead();
extern void vgaPopRead();
#endif

#endif /* _RETINA_H_ */
