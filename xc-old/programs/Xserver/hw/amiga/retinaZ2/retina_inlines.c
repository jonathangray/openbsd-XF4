/* Inlines used by the banked version of cfb to manage the Retina's VGA banks. */

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

#ifndef _RETINA_INLINES_
#define _RETINA_INLINES_

INLINE void *
vgaSetReadWrite(p)
    register pointer p;
{
    retina_writeseg = ((u_long) p - VGABASE) >> vgaSegmentShift;
#if 1
#if 0
    /* write mode 0 */
    WGfx(vgaregs, GCT_ID_GRAPHICS_MODE, (RGfx(vgaregs, GCT_ID_GRAPHICS_MODE) & 0xfc) | 0);
#endif
    /* read/write to primary on A0, secondary on B0 */
    WSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA, (RSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA)&0x1f)|0x40);
#endif
    retina_setbank(retina_writeseg);
    retina_setotherbank(retina_writeseg);
#ifdef ORIGINAL_CODE
#if 0
    WGfx(vgaregs, GCT_ID_GRAPHICS_MODE, (RGfx(vgaregs, GCT_ID_GRAPHICS_MODE) & 0xfc) | 0);
#endif
    /* write to primary, write to secondary */
    WSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA,
	 (RSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA) & 0x1f) | 1);
#endif
    return (void *) ((u_long) vgaWriteBottom + (((u_long) p - (u_long) vgaWriteBottom) %
						vgaSegmentSize));
}

INLINE void *
vgaReadWriteNext(p)
    register pointer p;
{
    retina_setbank(++retina_writeseg);
    retina_setotherbank(retina_writeseg);
    return (void *) ((u_long) p - vgaSegmentSize);
}

INLINE void *
vgaReadWritePrev(p)
    register pointer p;
{
    retina_setbank(--retina_writeseg);
    retina_setotherbank(retina_writeseg);
    return (void *) ((u_long) p + vgaSegmentSize);
}

INLINE void *
vgaSetRead(p)
    register pointer p;
{
    retina_readseg = ((unsigned long) p - VGABASE) >> vgaSegmentShift;
#if 1
    /* write to primary, read from secondary */
    WSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA,
	 (RSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA) & 0x1f) | 0);
#if 0
    /* set write mode 1, "[...] data in the read latches is written
       to memory during CPU memory write cycles. [...]" */
    WGfx(vgaregs, GCT_ID_GRAPHICS_MODE, (RGfx(vgaregs, GCT_ID_GRAPHICS_MODE) & 0xfc) | 1);
#endif
#endif
    retina_setotherbank(retina_readseg);
#ifdef ORIGINAL_CODE
#if 0
    WGfx(vgaregs, GCT_ID_GRAPHICS_MODE, (RGfx(vgaregs, GCT_ID_GRAPHICS_MODE) & 0xfc) | 1);
#endif
#if 0
    /* write to primary, read from secondary */
    WSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA,
	 (RSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA) & 0x1f) | 0);
#endif
#endif
    return (void *) ((u_long) vgaReadBottom + (((u_long) p - (u_long) vgaReadBottom) %
					       vgaSegmentSize));
}

INLINE void *
vgaReadNext(p)
    register pointer p;
{
    retina_setotherbank(++retina_readseg);
    return (void *) ((u_long) p - vgaSegmentSize);
}

INLINE void *
vgaReadPrev(p)
    register pointer p;
{
    retina_setotherbank(--retina_readseg);
    return (void *) ((u_long) p + vgaSegmentSize);
}

INLINE void *
vgaSetWrite(p)
    register pointer p;
{
    retina_writeseg = ((unsigned long) p - VGABASE) >> vgaSegmentShift;
    retina_setbank(retina_writeseg);
    return (void *) ((u_long) vgaWriteBottom + (((u_long) p - (u_long) vgaWriteBottom) %
						vgaSegmentSize));
}

INLINE void *
vgaWriteNext(p)
    register pointer p;
{
    retina_setbank(++retina_writeseg);
    return (void *) ((u_long) p - vgaSegmentSize);
}

INLINE void *
vgaWritePrev(p)
    register pointer p;
{
    retina_setbank(--retina_writeseg);
    return (void *) ((u_long) p + vgaSegmentSize);
}

INLINE void
vgaSaveBank()
{
    retina_saveseg = retina_writeseg;
}

INLINE void
vgaRestoreBank()
{
    retina_writeseg = retina_readseg = retina_saveseg;
    retina_setbank(retina_saveseg);
    retina_setotherbank(retina_saveseg);
}

INLINE void
vgaPushRead(p)
{
    retina_setbank(retina_writeseg);
    retina_setotherbank(retina_writeseg);
}

INLINE void
vgaPopRead(p)
{
    retina_setbank(retina_writeseg);
    retina_setotherbank(retina_readseg);
}

#endif /* _RETINA_INLINES_ */
