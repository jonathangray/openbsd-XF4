#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "mac68k.h"

#define LEFTB(b)	(b & 0x1)	/* Mouse buttons are stored reverse. */
#define MIDDLEB(b)	(b & 0x2)
#define RIGHTB(b)	(b & 0x4)

static Bool 
mac68k_msoffscreen(screen, x, y)
	ScreenPtr	*screen;
	int		*x;
	int		*y;
{
	int index;

	if ((screenInfo.numScreens > 1) &&
		((*x >= (*screen)->width) || (*x < 0)))
	{
		index = (*screen)->myNum;

		if (*x < 0) {
			index = (index - 1 + screenInfo.numScreens) %
				screenInfo.numScreens;
			*screen = screenInfo.screens[index];
			*x += (*screen)->width;
		} else {
			*x -= (*screen)->width;
			index = (index + 1 ) % screenInfo.numScreens;
			*screen = screenInfo.screens[index];
		}

		return(TRUE);
	}
	return(FALSE);
}


static void 
mac68k_mscrossscreen(screen, entering)
	ScreenPtr	screen;
	Bool		entering;
{
	/* nothing. (stolen from A/UX X server) */
}

static void
mac68k_mswarp(screen, x, y)
	ScreenPtr	screen;
	int 		x;
	int		y;
{
	/* XXX Why even have a function here? */
	miPointerWarpCursor(screen, x, y);
}

miPointerScreenFuncRec mac_mousefuncs = {
	mac68k_msoffscreen,
	mac68k_mscrossscreen,
	mac68k_mswarp
};

void
mac68k_mousectrl()
{
	return;
}

int
mac68k_mouseproc(mouse, what)
	DevicePtr mouse;
	int what;
{
	BYTE map[4];

	switch (what) {
		case DEVICE_INIT:
			if (mouse != LookupPointerDevice()) {
				ErrorF("Mouse routines can only handle DESKTOP"
					" mice.\n");
				return(!Success);
			}

			map[0] = 0;
			map[1] = 1;
			map[2] = 2;
			map[3] = 3;

			InitPointerDeviceStruct(mouse, map, 3,
				miPointerGetMotionEvents, mac68k_mousectrl,
				miPointerGetMotionBufferSize());
			mouse->on = FALSE;
			break;

		case DEVICE_ON:
			mouse->on = TRUE;
			break;

		case DEVICE_OFF:
			mouse->on = FALSE;
			break;

		case DEVICE_CLOSE:
			/* nothing! */
			break;
	}

	return(Success);
}

static int 
accel_mouse (mouse, delta)
	DevicePtr mouse;
	int delta;
{
	int sgn;
	PtrCtrl *ctrlptr;

	sgn = sign(delta);
	delta = abs(delta);
	ctrlptr = &((DeviceIntPtr) mouse)->ptrfeed->ctrl;

	if (delta > ctrlptr->threshold)
   	    return (sgn * (ctrlptr->threshold + ((delta - ctrlptr->threshold) *
		ctrlptr->num) / ctrlptr->den));
	else
	    return (sgn * delta);
}

void
mac68k_processmouse(mouse, event)
	DevicePtr	mouse;
	adb_event_t	*event;
{
	xEvent	xev;
	int	dx, dy;
	static int	buttons = 0;

	mac_lasteventtime = xev.u.keyButtonPointer.time =
		TVTOMILLI(event->timestamp);

	if(buttons != event->u.m.buttons){
		if(LEFTB(buttons) != LEFTB(event->u.m.buttons)){
			xev.u.u.detail = 1;	/* leftmost */
			xev.u.u.type = LEFTB(event->u.m.buttons) ?
				ButtonPress : ButtonRelease;
			mieqEnqueue(&xev);
		}
		if(MIDDLEB(buttons) != MIDDLEB(event->u.m.buttons)){
			xev.u.u.detail = 2;	/* middle */
			xev.u.u.type = MIDDLEB(event->u.m.buttons) ?
				ButtonPress : ButtonRelease;
			mieqEnqueue(&xev);
		}
		if(RIGHTB(buttons) != RIGHTB(event->u.m.buttons)){
			xev.u.u.detail = 3;	/* right */
			xev.u.u.type = RIGHTB(event->u.m.buttons) ?
				ButtonPress : ButtonRelease;
			mieqEnqueue(&xev);
		}
		buttons = event->u.m.buttons;
	}

	dx = accel_mouse(mouse, event->u.m.dx);
	dy = accel_mouse(mouse, event->u.m.dy);

	miPointerDeltaCursor(dx, dy, mac_lasteventtime);
}

void
mac68k_getmouse()
{
	/* XXX make sure there is a mouse */
	/* FatalError("Cannot run X server without a mouse.\n"); */
}
