/* $XTermId: version.h,v 1.262 2006/03/20 00:36:19 tom Exp $ */
/* $XFree86: xc/programs/xterm/version.h,v 3.122 2006/03/20 00:36:19 dickey Exp $ */

/*
 * These definitions are used to build the string that's printed in response to
 * "xterm -version", or embedded in "xterm -help".  It usually indicates the
 * version of X to which this version of xterm has been built.  The number in
 * parentheses is my patch number (T.Dickey).
 */
#define XTERM_PATCH   211

#ifndef __vendorversion__
#define __vendorversion__ "XTerm/OpenBSD"
#endif
