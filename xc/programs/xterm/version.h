/* $XFree86: xc/programs/xterm/version.h,v 3.77 2002/01/08 01:40:29 dawes Exp $ */
/* $OpenBSD: version.h,v 1.3 2003/02/25 21:53:09 matthieu Exp $ */
/*
 * These definitions are used to build the string that's printed in response to
 * "xterm -version", or embedded in "xterm -help".  It is the version of
 * XFree86 to which this version of xterm has been built.  The number in
 * parentheses is my patch number (T.Dickey).
 */
/* Add a specific version for OpenBSD, since there are local changes */
#define XTERM_PATCH   165
#define XFREE86_VERSION "XFree86 4.2.1/OpenBSD 3.3"
