/*
 * Copyright (C) 2001 Matthieu Herrb
 *
 * $Source: /tmp/OpenBSD-XF4-repo/xc-old/programs/Xserver/hw/xfree86/XF86Setup/xf86setup.h,v $
 * $Revision: 1.1 $
 * $Date: 2001/04/11 19:49:07 $
 * 
 */
#ifndef _XF86SETUP_H
#define _XF86SETUP_H

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef int Boolean;

#define XtMalloc(n) malloc(n)
#define XtFree(p) free(p)
#define XtRealloc(p,n) realloc(p, n)
#define XtCalloc(n) calloc(n) 

#endif
