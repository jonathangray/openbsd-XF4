/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/et4000/tseng_cursor.h,v 1.2.2.1 1997/05/09 15:36:58 hohndel Exp $ */


/* Variables defined in tseng_cursor.c. */

extern int tsengCursorHotX;
extern int tsengCursorHotY;
extern int tsengCursorWidth;
extern int tsengCursorHeight;
extern int tsengCursorAddress;

/* Functions defined in tseng_cursor.c. */

extern Bool TsengCursorInit();
extern void TsengRestoreCursor();
extern void TsengWarpCursor();
extern void TsengQueryBestSize();
