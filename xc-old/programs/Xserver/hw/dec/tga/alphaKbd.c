/* $XConsortium: sunKbd.c,v 5.47 94/08/16 13:45:30 dpw Exp $ */
/* $OpenBSD: alphaKbd.c,v 1.2 2002/04/01 19:58:12 matthieu Exp $ */
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

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#define NEED_EVENTS
#include "alpha.h"
#include "keysym.h"
#include <stdio.h>
#define MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */ /* XXX */
#define MAX_KEYCODE	255	/* limited by the protocol */ /* XXX */

extern KeySymsRec alphaKeySyms[];
extern AlphaModmapRec *alphaModMaps[];


static void 
SetLights (KeybdCtrl* ctrl, int fd)
{
    /*
     * XXX Since this code is only ever
     * XXX being called for LK401 keyboards, which don't have num
     * XXX lock, I interpret all leds as being caps lock. --KS
     */
    int lockled;

    lockled = (ctrl->leds != 0) * WSKBD_LED_CAPS;
    if (ioctl (fd, WSKBDIO_SETLEDS, (caddr_t)&lockled) == -1)
	Error("Failed to set keyboard lights");
}


static void 
ModLight(DeviceIntPtr device, Bool on, int led)
{
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    alphaKbdPrivPtr pPriv = (alphaKbdPrivPtr) device->public.devicePrivate;

    if(on) {
	ctrl->leds |= led;
	pPriv->leds |= led;
    } else {
	ctrl->leds &= ~led;
	pPriv->leds &= ~led;
    }
    SetLights (ctrl, pPriv->fd);
}

/*-
 *-----------------------------------------------------------------------
 * alphaBell --
 *	Ring the terminal/keyboard bell
 *
 * Results:
 *	Ring the keyboard bell for an amount of time proportional to
 *	"loudness."
 *
 * Side Effects:
 *	None, really...
 *
 *-----------------------------------------------------------------------
 */

static void 
alphaBell(int percent,
	  DeviceIntPtr device,
	  pointer ctrl,
	  int unused)
{
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
    alphaKbdPrivPtr   pPriv = (alphaKbdPrivPtr) device->public.devicePrivate;
    struct wskbd_bell_data wbd;
 
    if (percent == 0 || kctrl->bell == 0)
 	return;

    wbd.which = WSKBD_BELL_DOVOLUME | WSKBD_BELL_DOPITCH |
	WSKBD_BELL_DOPERIOD;
    wbd.volume = percent;
    wbd.pitch = kctrl->bell_pitch;
    wbd.period = kctrl->bell_duration;

    if (ioctl (pPriv->fd, WSKBDIO_COMPLEXBELL, &wbd) == -1) {
 	Error("Failed to activate bell");
	return;
    }
}


#define XLED_NUM_LOCK    0x1
#define XLED_COMPOSE     0x4
#define XLED_SCROLL_LOCK 0x2
#define XLED_CAPS_LOCK   0x8

static KeyCode 
LookupKeyCode(KeySym keysym, KeySymsPtr keysymsrec)
{
    KeyCode i;
    int ii, index = 0;

    for (i = keysymsrec->minKeyCode; i < keysymsrec->maxKeyCode; i++)
	for (ii = 0; ii < keysymsrec->mapWidth; ii++)
	    if (keysymsrec->map[index++] == keysym)
		return i;
}

static void 
pseudoKey(DeviceIntPtr device, Bool down, KeyCode keycode)
{
    int bit;
    CARD8 modifiers;
    CARD16 mask;
    BYTE* kptr;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    modifiers = device->key->modifierMap[keycode];
    if (down) {
	/* fool dix into thinking this key is now "down" */
	int i;
	*kptr |= bit;
	device->key->prev_state = device->key->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    if (mask & modifiers) {
		device->key->modifierKeyCount[i]++;
		device->key->state += mask;
		modifiers &= ~mask;
	    }
    } else {
	/* fool dix into thinking this key is now "up" */
	if (*kptr & bit) {
	    int i;
	    *kptr &= ~bit;
	    device->key->prev_state = device->key->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
		if (mask & modifiers) {
		    if (--device->key->modifierKeyCount[i] <= 0) {
			device->key->state &= ~mask;
			device->key->modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	}
    }
}

static void 
DoLEDs(DeviceIntPtr device,	    /* Keyboard to alter */
       KeybdCtrl *ctrl,
       alphaKbdPrivPtr pPriv)
{
#ifdef XKB
    if (noXkbExtension) {
#endif /* XKB */
    if ((ctrl->leds & XLED_CAPS_LOCK) && !(pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Caps_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_CAPS_LOCK) && (pPriv->leds & XLED_CAPS_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Caps_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_NUM_LOCK) && !(pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Num_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_NUM_LOCK) && (pPriv->leds & XLED_NUM_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Num_Lock, &device->key->curKeySyms));

    if ((ctrl->leds & XLED_SCROLL_LOCK) && !(pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, TRUE,
		LookupKeyCode(XK_Scroll_Lock, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_SCROLL_LOCK) && (pPriv->leds & XLED_SCROLL_LOCK))
	    pseudoKey(device, FALSE,
		LookupKeyCode(XK_Scroll_Lock, &device->key->curKeySyms));
#if 0
    if ((ctrl->leds & XLED_COMPOSE) && !(pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, TRUE,
		LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));

    if (!(ctrl->leds & XLED_COMPOSE) && (pPriv->leds & XLED_COMPOSE))
	    pseudoKey(device, FALSE,
		LookupKeyCode(SunXK_Compose, &device->key->curKeySyms));
#endif /* 0 */
#ifdef XKB
    }
#endif /* XKB */
    pPriv->leds = ctrl->leds & 0x0f;
    SetLights (ctrl, pPriv->fd);
}

/*-
 *-----------------------------------------------------------------------
 * alphaKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 *
 *-----------------------------------------------------------------------
 */

static void 
alphaKbdCtrl (DeviceIntPtr device,	    /* Keyboard to alter */
	      KeybdCtrl *ctrl)
{
    alphaKbdPrivPtr pPriv = (alphaKbdPrivPtr) device->public.devicePrivate;

    if (pPriv->fd < 0) return;

#if 0 /* XXX */
    if (ctrl->click != pPriv->click) {
    	int kbdClickCmd;

	pPriv->click = ctrl->click;
	kbdClickCmd = pPriv->click ? KBD_CMD_CLICK : KBD_CMD_NOCLICK;
    	if (ioctl (pPriv->fd, KIOCCMD, &kbdClickCmd) == -1)
 	    Error("Failed to set keyclick");
    }
#endif /* 0 XXX */
    if (pPriv->type <= WSKBD_TYPE_LK401 && pPriv->leds != ctrl->leds & 0x0f)
        DoLEDs(device, ctrl, pPriv);

    /* Bell info change needs nothing done here. */
}

/*-
 *-----------------------------------------------------------------------
 * alphaKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 *-----------------------------------------------------------------------
 */

int 
alphaKbdProc (DeviceIntPtr device, /* Keyboard to manipulate */
	      int what)		/* What to do to it */
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    alphaKbdPrivPtr pPriv;
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    extern int XkbDfltRepeatDelay, XkbDfltRepeatInterval;

    static CARD8 *workingModMap = NULL;
    static KeySymsRec *workingKeySyms;

    switch (what) {
    case DEVICE_INIT:
	if (pKeyboard != LookupKeyboardDevice()) {
	    ErrorF ("Cannot open non-system keyboard\n");
	    return (!Success);
	}
	    
	if (!workingKeySyms) {
	    workingKeySyms = &alphaKeySyms[alphaKbdPriv.type];


#if MIN_KEYCODE > 0
	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
#endif
	    if (workingKeySyms->maxKeyCode > MAX_KEYCODE ||
		workingKeySyms->maxKeyCode < workingKeySyms->minKeyCode)
		workingKeySyms->maxKeyCode = MAX_KEYCODE;
	}

	if (!workingModMap) {
            workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
            (void) memset(workingModMap, 0, MAP_LENGTH);
            for(i=0; alphaModMaps[alphaKbdPriv.type][i].key != 0; i++)
                workingModMap[alphaModMaps[alphaKbdPriv.type][i].key +
                              MIN_KEYCODE] = 
                alphaModMaps[alphaKbdPriv.type][i].modifiers;
 	}

	(void) memset ((void *) defaultKeyboardControl.autoRepeats,
			~0, sizeof defaultKeyboardControl.autoRepeats);

	pKeyboard->devicePrivate = (pointer)&alphaKbdPriv;
	pKeyboard->on = FALSE;

	InitKeyboardDeviceStruct(pKeyboard, 
				 workingKeySyms, workingModMap,
				 alphaBell, alphaKbdCtrl);
	break;

    case DEVICE_ON:
	pPriv = (alphaKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Set the keyboard into "direct" mode and turn on
	 * event translation.
	 */
	(void) AddEnabledDevice(pPriv->fd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (alphaKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Restore original keyboard directness and translation.
	 */
	RemoveEnabledDevice(pPriv->fd);
	pKeyboard->on = FALSE;
	break;
    default:
	FatalError("Unknown keyboard operation\n");
    }
    return Success;
}

/*-
 *-----------------------------------------------------------------------
 * alphaKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 *      #ifdef USE_WSCONS implies that we use struct wscons_event, not
 *      Firm_event.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

#ifdef USE_WSCONS
struct wscons_event* 
#else
Firm_event* 
#endif
alphaKbdGetEvents(int fd,
		  int*	pNumEvents,
		  Bool* pAgain)
{
    int	    	  nBytes;	    /* number of bytes of events available. */
#ifdef USE_WSCONS
    static struct wscons_event	evBuf[MAXEVENTS];   /* Buffer for wscons_events */
#else
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */
#endif

    if ((nBytes = read (fd, evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("Reading keyboard");
	    FatalError ("Could not read the keyboard");
	}
    } else {
#ifdef USE_WSCONS
	*pNumEvents = nBytes / sizeof (struct wscons_event);
#else
	*pNumEvents = nBytes / sizeof (Firm_event);
#endif
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * alphaKbdEnqueueEvent --
 *
 *-----------------------------------------------------------------------
 */
static xEvent	autoRepeatEvent;
static int	composeCount;

static Bool 
DoSpecialKeys(DeviceIntPtr device,
	      xEvent* xE,
	      Firm_event* fe)
{
    int	shift_index, map_index, bit;
    KeySym ksym;
    BYTE* kptr;
    alphaKbdPrivPtr pPriv = (alphaKbdPrivPtr)device->public.devicePrivate;
    BYTE keycode = xE->u.u.detail;
    CARD8 keyModifiers = device->key->modifierMap[keycode];

    /* look up the present idea of the keysym */
    shift_index = 0;
    if (device->key->state & ShiftMask) 
	shift_index ^= 1;
    if (device->key->state & LockMask) 
	shift_index ^= 1;
#ifdef USE_WSCONS
    map_index = (fe->value - 1) * device->key->curKeySyms.mapWidth;
#else
    map_index = (fe->id - 1) * device->key->curKeySyms.mapWidth;
#endif
    ksym = device->key->curKeySyms.map[shift_index + map_index];
    if (ksym == NoSymbol)
	ksym = device->key->curKeySyms.map[map_index];

    /*
     * Toggle functionality is hardcoded. This is achieved by always
     * discarding KeyReleases on these keys, and converting every other
     * KeyPress into a KeyRelease.
     */
    if (xE->u.u.type == KeyRelease 
	&& (ksym == XK_Num_Lock 
	|| ksym == XK_Scroll_Lock 
	|| (keyModifiers & LockMask))) 
	return TRUE;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    if ((*kptr & bit) &&
	(ksym == XK_Num_Lock || ksym == XK_Scroll_Lock ||
	(keyModifiers & LockMask)))
	xE->u.u.type = KeyRelease;

    return FALSE;
}

void 
alphaKbdEnqueueEvent (DeviceIntPtr device,
#ifdef USE_WSCONS
		      struct wscons_event *fe)
#else
		      Firm_event *fe)
#endif
{
    xEvent		xE;
    BYTE		keycode;
    CARD8		keyModifiers;
    int			i;

#ifdef USE_WSCONS
    if (alphaKbdPriv.type <= WSKBD_TYPE_LK401)
	    keycode = (fe->value) + MIN_KEYCODE;
    else
	    keycode = (fe->value & 0x7f) + MIN_KEYCODE;
#else
    if (alphaKbdPriv.type <= WSKBD_TYPE_LK401)
	    keycode = (fe->id) + MIN_KEYCODE;
    else
	    keycode = (fe->id & 0x7f) + MIN_KEYCODE;
#endif

    keyModifiers = device->key->modifierMap[keycode];
#ifdef USE_WSCONS
    /*
     * For lk201, we need to keep track of which keys are down so we can
     * process "all keys up" events.
     */
    if (alphaKbdPriv.type <= WSKBD_TYPE_LK401) {
	    if (fe->type == WSCONS_EVENT_KEY_DOWN) {
		    for (i = 0; i < LK_KLL; i++)
			    if (alphaKbdPriv.keys_down[i] == (KeyCode)-1) {
				    alphaKbdPriv.keys_down[i] = keycode;
				    break;
			    }
	    } else if (fe->type == WSCONS_EVENT_KEY_UP) {
		    for (i = 0; i < LK_KLL; i++)
			    if (alphaKbdPriv.keys_down[i] == keycode) {
				    alphaKbdPriv.keys_down[i] = (KeyCode)-1;
				    break;
			    }
	    } else if (fe->type == WSCONS_EVENT_ALL_KEYS_UP) {
		    /* Recursively send all key up events */
		    fe->type = WSCONS_EVENT_KEY_UP;
		    for (i = 0; i < LK_KLL; i++) {
			    if (alphaKbdPriv.keys_down[i] != (KeyCode)-1) {
				    fe->value = alphaKbdPriv.keys_down[i] -
					    MIN_KEYCODE;
				    alphaKbdEnqueueEvent(device, fe);
			    }
		    }
		    return;
	    }
    }
    xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);
    xE.u.u.type = ((fe->type == WSCONS_EVENT_KEY_UP) ? KeyRelease : KeyPress);
#else
    xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);
    xE.u.u.type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
#endif
    xE.u.u.detail = keycode;
    mieqEnqueue (&xE);
}


/*ARGSUSED*/
Bool 
LegalModifier(unsigned int key,
	      DevicePtr	pDev)
{
    return TRUE;
}

