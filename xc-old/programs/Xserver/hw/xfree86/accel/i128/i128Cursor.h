/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/i128/i128Cursor.h,v 3.1 1996/02/04 09:01:06 dawes Exp $ */






/* $XConsortium: i128Cursor.h /main/2 1996/02/21 17:23:03 kaleb $ */

extern Bool i128BlockCursor;
extern Bool i128ReloadCursor;

#define BLOCK_CURSOR	i128BlockCursor = TRUE;

#define UNBLOCK_CURSOR	{ \
			   if (i128ReloadCursor) \
			      i128RestoreCursor(i128savepScreen); \
			   i128BlockCursor = FALSE; \
			}
