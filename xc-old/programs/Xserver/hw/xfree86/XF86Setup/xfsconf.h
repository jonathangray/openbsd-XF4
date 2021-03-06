/* $XConsortium: xfsconf.h /main/2 1996/10/19 19:06:59 kaleb $ */





/* $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/xfsconf.h,v 3.1 1996/06/30 10:44:16 dawes Exp $ */

/* Fake some stuff to get xf86Config.c to compile here */

#include "tcl.h"
#define NEED_RETURN_VALUE
#define RET_OKAY	TCL_OK
#define RET_ERROR	TCL_ERROR


#define Xcalloc(size)	calloc(1,size)
#define Xfree		free
#define Xalloc		malloc
#define Xrealloc	realloc

#ifndef XF86SETUP_NO_FUNC_RENAME
#define FatalError	return XF86SetupFatalError
#define xf86ConfigError	return XF86SetupXF86ConfigError
#endif

