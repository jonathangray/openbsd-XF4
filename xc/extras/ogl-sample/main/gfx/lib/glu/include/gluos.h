/*
** gluos.h - operating system dependencies for GLU
**
** $Header: /tmp/OpenBSD-XF4-repo/xc/extras/ogl-sample/main/gfx/lib/glu/include/gluos.h,v 1.1.1.1 2001/04/05 22:04:25 matthieu Exp $
*/

#if defined(_WIN32) && !defined(__CYGWIN__)

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOIME
#include <windows.h>

/* Disable warnings */
#pragma warning(disable : 4101)
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)

#else

/* Disable Microsoft-specific keywords */
#define GLAPIENTRY
#define WINGDIAPI

#endif
