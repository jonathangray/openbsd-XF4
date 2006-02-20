/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_version.h,v 1.10 2003/09/28 20:15:57 alanh Exp $ */
/*
 * Copyright 2000 through 2004 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _RM6_VERSION_H_
#define _RM6_VERSION_H_ 1

#undef  RM6_NAME
#undef  RM6_DRIVER_NAME
#undef  R200_DRIVER_NAME
#undef  RM6_VERSION_MAJOR
#undef  RM6_VERSION_MINOR
#undef  RM6_VERSION_PATCH
#undef  RM6_VERSION_CURRENT
#undef  RM6_VERSION_EVALUATE
#undef  RM6_VERSION_STRINGIFY
#undef  RM6_VERSION_NAME

#define RM6_NAME          "RM6"
#define RM6_DRIVER_NAME   "rm6"
#define R200_DRIVER_NAME     "r200"

#define RM6_VERSION_MAJOR 4
#define RM6_VERSION_MINOR 0
#define RM6_VERSION_PATCH 1

#ifndef RM6_VERSION_EXTRA
#define RM6_VERSION_EXTRA ""
#endif

#define RM6_VERSION_CURRENT \
    ((RM6_VERSION_MAJOR << 20) | \
     (RM6_VERSION_MINOR << 10) | \
     (RM6_VERSION_PATCH))

#define RM6_VERSION_EVALUATE(__x) #__x
#define RM6_VERSION_STRINGIFY(_x) RM6_VERSION_EVALUATE(_x)
#define RM6_VERSION_NAME                                             \
    RM6_VERSION_STRINGIFY(RM6_VERSION_MAJOR) "."                  \
    RM6_VERSION_STRINGIFY(RM6_VERSION_MINOR) "."                  \
    RM6_VERSION_STRINGIFY(RM6_VERSION_PATCH) RM6_VERSION_EXTRA

#endif /* _RM6_VERSION_H_ */
