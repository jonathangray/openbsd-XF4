
/* $XConsortium: sun.h,v 5.39.1.1 95/01/05 19:58:43 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/sun/sun.h,v 3.2 1995/02/12 02:36:21 dawes Exp $ */

/*-
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef _MACPPC_H_
#define _MACPPC_H_

/* X headers */
#include "Xos.h"
#include "X.h"
#include "Xproto.h"

/* general system headers */
#ifndef NOSTDHDRS
# include <stdlib.h>
#else
# include <malloc.h>
extern char *getenv();
#endif

/* system headers common to both SunOS and Solaris */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <memory.h>

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

#include <dev/wscons/wsconsio.h>

typedef struct wscons_event Firm_event;

extern int gettimeofday();

/*
 * Server specific headers
 */
#include "misc.h"
#undef abs /* don't munge function prototypes in headers, sigh */
#include "scrnintstr.h"
#ifdef NEED_EVENTS
# include "inputstr.h"
#endif
#include "input.h"
#include "colormapst.h"
#include "colormap.h"
#include "cursorstr.h"
#include "cursor.h"
#include "dixstruct.h"
#include "dix.h"
#include "opaque.h"
#include "resource.h"
#include "servermd.h"
#include "windowstr.h"

/*
 * ddx specific headers
 */
#ifndef PSZ
#define PSZ 8
#endif

#include "mipointer.h"

extern int monitorResolution;

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any macppc keyboard.
 */
typedef struct {
    int		fd;
    int		type;		/* Type of keyboard */
    int		layout;		/* The layout of the keyboard */
    int		click;		/* kbd click save state */
    Leds	leds;		/* last known LED state */
} macppcKbdPrivRec, *macppcKbdPrivPtr;

extern macppcKbdPrivRec macppcKbdPriv;

/*
 * Data private to any macppc pointer device.
 */
typedef struct {
    int		fd;
    int		bmask;		/* last known button state */
} macppcPtrPrivRec, *macppcPtrPrivPtr;

extern macppcPtrPrivRec macppcPtrPriv;

typedef struct {
    BYTE	key;
    CARD8	modifiers;
} macppcModmapRec;

typedef struct {
    ColormapPtr	    installedMap;
    CloseScreenProcPtr CloseScreen;
    void	    (*UpdateColormap)();
    Bool	    hasHardwareCursor;
} macppcScreenRec, *macppcScreenPtr;

#define GetScreenPrivate(s)   ((macppcScreenPtr) ((s)->devPrivates[macppcScreenIndex].ptr))
#define SetupScreen(s)	macppcScreenPtr	pPrivate = GetScreenPrivate(s)

typedef struct {
    unsigned char*  fb;		/* Frame buffer itself */
    int		    fd;		/* frame buffer for ioctl()s, */
    struct wsdisplay_fbinfo   info;	/* Frame buffer characteristics */
    int		    linebytes;	/* number of bytes per row */
    void	    (*EnterLeave)();/* screen switch */
} fbFd;

typedef Bool (*macppcFbInitProc)(
    int /* screen */,
    ScreenPtr /* pScreen */,
    int /* argc */,
    char** /* argv */
);

typedef struct {
    macppcFbInitProc	init;	/* init procedure for this fb */
    char*		name;	/* /usr/include/fbio names */
} macppcFbDataRec;

#ifdef XKB
extern Bool		noXkbExtension;
#endif

#if 0
extern Bool		sunAutoRepeatHandlersInstalled;
extern long		sunAutoRepeatInitiate;
extern long		sunAutoRepeatDelay;
#endif
extern macppcFbDataRec	macppcFbData[];
extern fbFd		macppcFbs[];
extern int		macppcScreenIndex;

extern void macppcDisableCursor(ScreenPtr /* pScreen */);

extern void macppcEnqueueEvents(void);

extern Bool macppcSaveScreen(ScreenPtr /* pScreen */, int /* on */);

extern Bool macppcScreenInit(ScreenPtr /* pScreen */);

extern pointer macppcMemoryMap(size_t /* len */, off_t /* off */, 
			       int /* fd */);

extern Bool macppcScreenAllocate(ScreenPtr /* pScreen */);

extern Firm_event* macppcKbdGetEvents(int /* fd */, int* /* pNumEvents */,
				      Bool* /* pAgain */);

extern Firm_event* macppcMouseGetEvents(int /* fd */, int* /* pNumEvents */,
					Bool* /* pAgain */);

extern void macppcKbdEnqueueEvent(DeviceIntPtr /* device */,
				  Firm_event* /* fe */);

extern void macppcMouseEnqueueEvent(DeviceIntPtr /* device */,
				    Firm_event* /* fe */);

extern int macppcKbdProc(DeviceIntPtr /* pKeyboard */,    int /* what */);

extern int macppcMouseProc(DeviceIntPtr /* pMouse */, int /* what */);

extern void macppcKbdWait(void);

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))
#define TSTOMILLI(ts)	(((ts).tv_nsec/1000000)+((ts).tv_sec*1000))

extern void macppcInstallColormap(ColormapPtr /* cmap */);

extern void macppcUninstallColormap(ColormapPtr /* cmap */);

extern int macppcListInstalledColormaps(ScreenPtr /* pScreen */,
					Colormap* /* pCmapList */);

#endif
