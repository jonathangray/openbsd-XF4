/*
 * $XFree86$
 */

/* 
 * Copyright (C) 2002 Red Hat, Inc.
 * Developer:  Havoc Pennington, Red Hat, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the X Consortium
 * shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written
 * authorization from the X Consortium.
 *
 */

/* The DAMAGE extension lets you track modified portions of the
 * framebuffer. i.e. anytime a pixel changes onscreen you get
 * notification, and you can ask the server for a region describing
 * changes since you last asked.
 *
 * The function prototypes etc. look like they work for any window,
 * but right now it really only works for root windows.
 */

#ifndef _X_DAMAGE_H_
#define _X_DAMAGE_H_

#include <X11/Xfuncproto.h>
#include <X11/extensions/damageshared.h>

/* A server-side resource that tracks a damage region for a given
 * window.
 */
typedef unsigned long XDamageRecorder;

typedef struct {
    int	type;		      /* of event */
    unsigned long serial;     /* # of last request processed by server */
    Bool send_event;	      /* true if this came frome a SendEvent request */
    Display *display;	      /* Display the event was read from */
    Window window;	      /* window on which damage recorder is recording */
    XDamageRecorder recorder; /* the damage recorder this event is for */
} XDamageNotifyEvent;

_XFUNCPROTOBEGIN

Bool XDamageQueryExtension (
    Display *dpy,
    int *event_base,
    int *error_base
);

Status XDamageQueryVersion (
    Display *dpy,
    int *major,
    int *minor
);

Bool XDamageScreenSupportsRootRecorder (
    Display *dpy,
    int      screen_num
);

XDamageRecorder XDamageCreateRecorder (
    Display *dpy,
    Window   window
);

void XDamageDestroyRecorder (
    Display *dpy,
    XDamageRecorder recorder
);

/* Gets damage recorded for given rectangle since last time we got damage
 * for that rectangle. i.e. returns the intersection of the current
 * damage region and the rectangle, and subtracts the rectangle
 * from the current damage region.
 */
int XDamageGetDamage (
    Display *dpy,
    XDamageRecorder recorder,
    _Xconst XRectangle *rect,
    XRectangle **rects,
    int *n
);

_XFUNCPROTOEND

#endif /* _X_DAMAGE_H_ */
