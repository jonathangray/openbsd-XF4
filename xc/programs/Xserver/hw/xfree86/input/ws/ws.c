/*
 * Copyright (c) 2005 Matthieu Herrb
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* $OpenBSD: ws.c,v 1.1 2005/02/16 20:04:16 matthieu Exp $ */

#ifndef XFree86LOADER
#include <unistd.h>
#include <errno.h>
#endif
#include <sys/time.h>
#include <dev/wscons/wsconsio.h>

#include <misc.h>
#include <xf86.h>

#include <xf86_ansic.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
#include <xisb.h>
#include <exevents.h>
#include <extnsionst.h>
#include <extinit.h>

#ifdef XFree86LOADER
#include "xf86Module.h"
#endif

#define NAXES 2			/* X and Y axes only */
#define NBUTTONS 32		/* max theoretical buttons */
#define DFLTBUTTONS 3		/* default number of buttons */
#define NUMEVENTS 16		/* max # of ws events to read at once */

typedef struct WSDevice {
	char *devName;		/* device name */
	unsigned int buttons;	/* # of buttons */
	unsigned int lastButtons; /* last state of buttons */
	int x, y;		/* current abs coordinates */
	int min_x, max_x, min_y, max_y; /* coord space */
	int swap_axes;
	int inv_x, inv_y;
	int screen_width, screen_height;
	int screen_no;
	int num, den, threshold; /* relative accel params */
	pointer buffer;
} WSDeviceRec, *WSDevicePtr;

#ifdef XFree86LOADER
static MODULESETUPPROTO(SetupProc);
static void TearDownProc(pointer);
static const OptionInfoRec *wsAvailableOptions(void *);
#endif

static InputInfoPtr wsPreInit(InputDriverPtr, IDevPtr, int);
static int wsProc(DeviceIntPtr, int);
static void wsReadInput(InputInfoPtr);
static void wsSendButtons(InputInfoPtr, int, int, int);
static int wsChangeControl(InputInfoPtr, xDeviceCtl *);
static int wsSwitchMode(ClientPtr, DeviceIntPtr, int);
static Bool wsOpen(InputInfoPtr);
static void wsClose(InputInfoPtr);
static Bool wsConvert(InputInfoPtr, int, int, int, int, int, int, int, int,
    int *, int *);
static void wsControlProc(DeviceIntPtr , PtrCtrl *);


static XF86ModuleVersionInfo VersionRec = {
	"ws",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	1, 0, 0,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}
};

typedef enum {
	WSOPT_DEVICE,
	WSOPT_DEBUG_LEVEL,
	WSOPT_HISTORY_SIZE,
	WSOPT_MINX,
	WSOPT_MAXX,
	WSOPT_MINY,
	WSOPT_MAXY,
	WSOPT_ROTATE,
	WSOPT_SWAPXY,
	WSOPT_SCREENNO,
} WSOpts;

static const OptionInfoRec WSOptions[] = {
	{ WSOPT_DEVICE, "device", OPTV_STRING, {0}, FALSE },
	{ WSOPT_DEBUG_LEVEL, "debugLevel", OPTV_INTEGER, {0}, FALSE },
	{ WSOPT_HISTORY_SIZE, "historysize", OPTV_INTEGER, {0}, FALSE },
	{ WSOPT_MINX, "minX", OPTV_INTEGER, {0}, FALSE },
	{ WSOPT_MAXX, "maxX", OPTV_INTEGER, {0}, FALSE },
	{ WSOPT_MINY, "minY", OPTV_INTEGER, {0}, FALSE },
	{ WSOPT_MAXY, "maxY", OPTV_INTEGER, {0}, FALSE },
	{ WSOPT_ROTATE, "rotate", OPTV_STRING, {0}, FALSE },
	{ WSOPT_SWAPXY, "swapxy", OPTV_BOOLEAN, {0}, FALSE },
	{ WSOPT_SCREENNO, "ScreenNo", OPTV_INTEGER, {0}, FALSE },
	{ -1, NULL, OPTV_NONE, {0}, FALSE }
};

#ifdef XFree86LOADER
XF86ModuleData wsModuleData = {&VersionRec,
			       SetupProc, TearDownProc };

ModuleInfoRec wsInfo = {
	1,
	"WS",
	NULL,
	0,
	wsAvailableOptions,
};
#endif

InputDriverRec WS = {
	1,
	"ws",
	NULL,
	wsPreInit,
	NULL,
	0
};

/* #undef DEBUG */
#define DEBUG
#undef DBG
static int debug_level = 0;
#define DEBUG
#ifdef DEBUG
# define DBG(lvl, f) { if ((lvl) <= debug_level) f;}
#else
# define DBG(lvl, f)
#endif

#ifdef XFree86LOADER
static pointer
SetupProc(pointer module, pointer options, int *errmaj, int *errmin)
{
	static Bool Initialised = FALSE;
	
	if (!Initialised) {
#ifndef REMOVE_LOADER_CHECK_MODULE_INFO
		if (xf86LoaderCheckSymbol("xf86AddModuleInfo"))
#endif
			xf86AddModuleInfo(&wsInfo, module);
		xf86Msg(X_INFO, "wscons input driver\n");
		xf86AddInputDriver(&WS, module, 0);
		Initialised = TRUE;
	}
	return module;
}

static void
TearDownProc(pointer p)
{
	DBG(1, ErrorF("WS TearDownProc called\n"));
}

static const OptionInfoRec *
wsAvailableOptions(void *unused)
{
	return WSOptions;
}
#endif /* XFree86LOADER */

static InputInfoPtr
wsPreInit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	InputInfoPtr pInfo = NULL;
	WSDevicePtr priv;
	MessageType buttons_from = X_CONFIG;
	char *s;
	
	pInfo = xf86AllocateInput(drv, 0);
	if (pInfo == NULL) {
		return NULL;
	}
	priv = (WSDevicePtr)xcalloc(1, sizeof(WSDeviceRec));
	if (priv == NULL) 
		goto fail;
	pInfo->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
	pInfo->conf_idev = dev;
	pInfo->name = XI_TOUCHSCREEN;
	pInfo->private = priv;

	xf86CollectInputOptions(pInfo, NULL, NULL);
	xf86ProcessCommonOptions(pInfo, pInfo->options);
#ifdef DEBUG
	debug_level = xf86SetIntOption(pInfo->options, "DebugLevel", 
	    debug_level);
	xf86Msg(X_INFO, "%s: debuglevel %d\n", dev->identifier, debug_level);
#endif
	priv->devName = xf86FindOptionValue(pInfo->options, "Device");
	if (priv->devName == NULL) {
		xf86Msg(X_ERROR, "%s: No Device specified.\n", 
			dev->identifier);
		goto fail;
	}
	priv->buttons = xf86SetIntOption(pInfo->options, "Buttons", 0);
	if (priv->buttons == 0) {
		priv->buttons = DFLTBUTTONS;
		buttons_from = X_DEFAULT;
	}
	priv->screen_no = xf86SetIntOption(pInfo->options, "ScreenNo", 0);
	xf86Msg(X_CONFIG, "%s associated screen: %d\n", 
	    dev->identifier, priv->screen_no);  
	if (priv->screen_no >= screenInfo.numScreens ||
	    priv->screen_no < 0) {
		priv->screen_no = 0;
	}

	priv->max_x = xf86SetIntOption(pInfo->options, "MaxX", 3000);
	xf86Msg(X_CONFIG, "%s maximum x position: %d\n", 
	    dev->identifier, priv->max_x);
	priv->min_x = xf86SetIntOption(pInfo->options, "MinX", 0);
	xf86Msg(X_CONFIG, "%s minimum x position: %d\n", 
	    dev->identifier, priv->min_x);
	priv->max_y = xf86SetIntOption(pInfo->options, "MaxY", 3000);
	xf86Msg(X_CONFIG, "%s maximum y position: %d\n", 
	    dev->identifier, priv->max_y);
	priv->min_y = xf86SetIntOption(pInfo->options, "MinY", 0);
	xf86Msg(X_CONFIG, "%s minimum y position: %d\n", 
	    dev->identifier, priv->min_y);

	priv->swap_axes = xf86SetBoolOption(pInfo->options, "SwapXY", 0);
	if (priv->swap_axes) {
		xf86Msg(X_CONFIG, 
		    "%s device will work with X and Y axes swapped\n",
		    dev->identifier);
	}
	priv->inv_x = 0;
	priv->inv_y = 0;
	s = xf86FindOptionValue(pInfo->options, "Rotate");
	if (s) {
		if (xf86NameCmp(s, "CW") == 0) {
			priv->inv_x = 1;
			priv->inv_y = 0;
			priv->swap_axes = 1;
		} else if (xf86NameCmp(s, "CCW") == 0) {
			priv->inv_x = 0;
			priv->inv_y = 1;
			priv->swap_axes = 1;
		} else if (xf86NameCmp(s, "UD") == 0) {
			priv->inv_x = 1;
			priv->inv_y = 1;
		} else {
			xf86Msg(X_ERROR, "\"%s\" is not a valid value "
				"for Option \"Rotate\"\n", s);
			xf86Msg(X_ERROR, "Valid options are \"CW\", \"CCW\","
				" or \"UD\"\n");
		}
	}

	pInfo->name = dev->identifier;
	pInfo->type_name = "wscons pointer";
	pInfo->device_control = wsProc;
	pInfo->read_input = wsReadInput;
	pInfo->control_proc = wsChangeControl;
	pInfo->switch_mode = wsSwitchMode;
	pInfo->conversion_proc = wsConvert;
	pInfo->reverse_conversion_proc = NULL;
	pInfo->fd = -1;
	pInfo->private = priv;
	pInfo->old_x = -1;
	pInfo->old_y = -1;
	xf86Msg(buttons_from, "%s: Buttons: %d\n", pInfo->name, priv->buttons);
	
	/* mark the device configured */
	pInfo->flags |= XI86_CONFIGURED;
	return pInfo;
fail:
	if (priv != NULL)
		xfree(priv);
	if (pInfo != NULL) 
		xfree(pInfo);
	return NULL;
}

static int
wsProc(DeviceIntPtr pWS, int what)
{
	InputInfoPtr pInfo = (InputInfoPtr)pWS->public.devicePrivate;
	WSDevicePtr priv = (WSDevicePtr)XI_PRIVATE(pWS);
	unsigned char map[NBUTTONS + 1];
	int i;
 
	switch (what) {
	case DEVICE_INIT:
		DBG(1, ErrorF("WS DEVICE_INIT\n"));

		priv->screen_width = 
		    screenInfo.screens[priv->screen_no]->width;
		priv->screen_height = 
		    screenInfo.screens[priv->screen_no]->height;

		for (i = 0; i < NBUTTONS; i++)
			map[i + 1] = i + 1;
		xf86MotionHistoryAllocate(pInfo);
		InitPointerDeviceStruct((DevicePtr)pWS, map,
				min(priv->buttons, NBUTTONS),
		    xf86GetMotionEvents, wsControlProc,
		    pInfo->history_size);
		xf86InitValuatorAxisStruct(pWS, 0, 0, -1, 1, 0, 1);
		xf86InitValuatorDefaults(pWS, 0);
		
		xf86InitValuatorAxisStruct(pWS, 1, 0, -1, 1, 0, 1);
		xf86InitValuatorDefaults(pWS, 1);
		AssignTypeAndName(pWS, pInfo->atom, pInfo->name);
		pWS->public.on = FALSE;
		if (wsOpen(pInfo) != Success) {
			return !Success;
		}
		break;

	case DEVICE_ON:
		DBG(1, ErrorF("WS DEVICE ON\n"));
		if ((pInfo->fd < 0) && (wsOpen(pInfo) != Success)) {
			xf86Msg(X_ERROR, "wsOpen failed %s\n",
				strerror(errno));
			return !Success;
		}
		priv->buffer = XisbNew(pInfo->fd, 
		    sizeof(struct wscons_event) * NUMEVENTS);
		if (priv->buffer == NULL) {
			xf86Msg(X_ERROR, "cannot alloc xisb buffer\n");
			wsClose(pInfo);
			return !Success;
		}
		xf86AddEnabledDevice(pInfo);
		pWS->public.on = TRUE;

		break;

	case DEVICE_OFF:
		DBG(1, ErrorF("WS DEVICE OFF\n"));
		if (pInfo->fd >= 0) {
			xf86RemoveEnabledDevice(pInfo);
			wsClose(pInfo);
		}
		if (priv->buffer) {
			XisbFree(priv->buffer);
			priv->buffer = NULL;
		}
		pWS->public.on = FALSE;
		break;

	case DEVICE_CLOSE:
		DBG(1, ErrorF("WS DEVICE_CLOSE\n"));
		wsClose(pInfo);
		break;
		
	default:
		xf86Msg(X_ERROR, "WS: unknown command %d\n", what);
		return !Success;
	} /* switch */
	return Success;
} /* wsProc */

static void
wsReadInput(InputInfoPtr pInfo)
{
	WSDevicePtr priv;
	static struct wscons_event eventList[NUMEVENTS];
	int n, c; 
	struct wscons_event *event = eventList;
	unsigned char *pBuf;
	int ax, ay;
	
	priv = pInfo->private;

	XisbBlockDuration(priv->buffer, -1);
	pBuf = (unsigned char *)eventList;
	n = 0;
	while ((c = XisbRead(priv->buffer)) >= 0 && n < sizeof(eventList)) {
		pBuf[n++] = (unsigned char)c;
	}
	
	if (n == 0)
		return;
	
	n /= sizeof(struct wscons_event);
	while( n-- ) {
		int buttons = priv->lastButtons;
		int dx = 0, dy = 0;
		ax = 0; ay = 0;
		switch (event->type) {
		case WSCONS_EVENT_MOUSE_UP:

			buttons &= ~(1 << event->value);
			DBG(4, ErrorF("Button %d up %x\n", event->value,
				buttons));
		break;
		case WSCONS_EVENT_MOUSE_DOWN:
			buttons |= (1 << event->value);
			DBG(4, ErrorF("Button %d down %x\n", event->value,
				buttons));
			break;
		case WSCONS_EVENT_MOUSE_DELTA_X:
			dx = event->value;
			DBG(4, ErrorF("Relative X %d\n", event->value));
			break;
		case WSCONS_EVENT_MOUSE_DELTA_Y:
			dy = -event->value;
			DBG(4, ErrorF("Relative Y %d\n", event->value));
			break;
		case WSCONS_EVENT_MOUSE_ABSOLUTE_X:
			DBG(4, ErrorF("Absolute X %d\n", event->value));
			if (event->value != 4095) {
				ax = event->value;
				if (priv->inv_x)
					ax = priv->max_x - ax + priv->min_x;
			}
			break;
		case WSCONS_EVENT_MOUSE_ABSOLUTE_Y:
			DBG(4, ErrorF("Absolute Y %d\n", event->value));
			ay = event->value;
			if (priv->inv_y)
				ay = priv->max_y - ay + priv->min_y;
			break;
		default:
			xf86Msg(X_WARNING, "%s: bad wsmouse event type=%d\n", 
			    pInfo->name,
			    event->type);
			++event;
			continue;
		} /* case */

		if (dx || dy) {
			/* relative motion event */
			xf86PostMotionEvent(pInfo->dev, 0, 0, 2, dx, dy);
			priv->x += dx;
			priv->y += dy;
		}
		if (priv->lastButtons != buttons) {
			/* button event */
			wsSendButtons(pInfo, buttons, priv->x, priv->y);
			priv->lastButtons = buttons;
		}
		if (ax) {
			/* absolute position event */
			DBG(3, ErrorF("postMotionEvent X %d %d\n", 
				      ax, priv->y));
			xf86PostMotionEvent(pInfo->dev, 1, 0, 2, ax, priv->y);
			priv->x = ax;
		}
		if (ay) {
			/* absolute position event */
			DBG(3, ErrorF("postMotionEvent y %d %d\n", 
				      priv->x, ay));
			xf86PostMotionEvent(pInfo->dev, 1, 0, 2, priv->x, ay);
			priv->y = ay;
		}
		++event;
	}
	return;
} /* wsReadInput */

static void
wsSendButtons(InputInfoPtr pInfo, int buttons, int rx, int ry)
{
	WSDevicePtr priv = (WSDevicePtr)pInfo->private;
	int button, mask;

	for (button = 1; button < NBUTTONS; button++) {
		mask = 1 << (button - 1);
		if ((mask & priv->lastButtons) != (mask & buttons)) {
			xf86PostButtonEvent(pInfo->dev, TRUE,
			    button, (buttons & mask) != 0, 
					    0, 0); /*2, priv->x, priv->y);*/
			DBG(3, ErrorF("post button event %d %d %d %d\n",
				      button, (buttons & mask) != 0,
				      priv->x, priv->y));
		}
	} /* for */
} /* wsSendButtons */


static int
wsChangeControl(InputInfoPtr pInfo, xDeviceCtl *control)
{
	return BadMatch;
}

static int
wsSwitchMode(ClientPtr client, DeviceIntPtr dev, int mode)
{
	return BadMatch;
}

static Bool
wsOpen(InputInfoPtr pInfo)
{
	WSDevicePtr priv = (WSDevicePtr)pInfo->private;

	DBG(1, ErrorF("WS open %s\n", priv->devName));
	pInfo->fd = xf86OpenSerial(pInfo->options);
	if (pInfo->fd == -1)
	    xf86Msg(X_WARNING, "%s: cannot open input device\n", pInfo->name);
	return Success;
}

static void
wsClose(InputInfoPtr pInfo)
{
	xf86CloseSerial(pInfo->fd);
	pInfo->fd = -1;
}

static Bool
wsConvert(InputInfoPtr pInfo, int first, int num, 
	  int v0, int v1, int v2, int v3, int v4, int v5,
	  int *x, int *y)
{
	WSDevicePtr priv = (WSDevicePtr) pInfo->private;
	if (first != 0 || num != 2) {
		return FALSE;
	}

	DBG(3, ErrorF("WSConvert: v0(%d), v1(%d)\n", v0, v1));

	if (priv->swap_axes != 0) {
		*x = xf86ScaleAxis(v1, 0, priv->screen_width, 
				   priv->min_y, priv->max_y);
		*y = xf86ScaleAxis(v0, 0, priv->screen_height, 
				   priv->min_x, priv->max_x);
	} else {
		*x = xf86ScaleAxis(v0, 0, priv->screen_width, 
				   priv->min_x, priv->max_x);
		*y = xf86ScaleAxis(v1, 0, priv->screen_height, 
				   priv->min_y, priv->max_y);
	}
  
	/*
	 * Need to check if still on the correct screen.
	 * This call is here so that this work can be done after
	 * calib and before posting the event.
	 */
	xf86XInputSetScreen(pInfo, priv->screen_no, *x, *y);
	
	DBG(3, ErrorF("WSConvert: x(%d), y(%d)\n", *x, *y));
	
	return TRUE;
}

static void
wsControlProc(DeviceIntPtr device, PtrCtrl *ctrl)
{
	InputInfoPtr pInfo = device->public.devicePrivate;
	WSDevicePtr priv = (WSDevicePtr)pInfo->private;

	DBG(1, ErrorF("wsControlProc\n"));
	priv->num = ctrl->num;
	priv->den = ctrl->den;
	priv->threshold = ctrl->threshold;
}
