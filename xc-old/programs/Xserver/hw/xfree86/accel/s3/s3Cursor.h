/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3Cursor.h,v 3.1 1996/12/23 06:41:30 dawes Exp $ */




/* $XConsortium: s3Cursor.h /main/2 1995/11/12 19:05:50 kaleb $ */

extern Bool s3BlockCursor;
extern Bool s3ReloadCursor;

#define BLOCK_CURSOR	s3BlockCursor = TRUE;

/* BL 0815150096:
 * UNBLOCK_CURSOR is called at the end of every graphics function, mem_barrier()
 * guarantees update of regs and memory (on e.g. Alpha) 
 */ 
#define UNBLOCK_CURSOR	{  mem_barrier(); \
			   if (s3ReloadCursor) \
			      s3RestoreCursor(s3savepScreen); \
			   s3BlockCursor = FALSE; \
			}
