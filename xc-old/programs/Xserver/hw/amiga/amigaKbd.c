/* $XConsortium: amigaKbd.c,v 5.44 94/04/17 20:29:41 erik Exp $ */
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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#define NEED_EVENTS
#include "amiga.h"
#include "keysym.h"

#ifdef CV64_SUPPORT
#include "s3/amigaCV.h"
#endif

#ifdef XKB
#include <X11/extensions/XKB.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBsrv.h>
#endif

#ifdef CV64_SUPPORT
extern ScreenPtr amigaCVsavepScreen;
extern void amigaCVadjustVirtual (volatile char *);
extern void amigaCVRestoreCursor();
#endif

extern int amigaVideoMode;

int amigaXdebug;

#ifdef CV64_SUPPORT
extern void
amigaCVSetPanning2 (fbFd *inf, unsigned short xoff, unsigned short yoff);

/* Change the videomode (only with the same depth) */

int amigaCVChangeMode (int mode)
{
  fbFd *inf = &amigaFbs[amigaCVsavepScreen->myNum];
  int depth = inf->info.gd_planes;
  volatile char *ba = (volatile char *) (inf->regs);
  struct grfvideo_mode gfvm, *gv;

  gv = &gfvm;
  gv->mode_num = mode;

  if (ioctl(inf->fd, GRFGETVMODE, gv) < 0)
	return FALSE;
  if (gv->depth != depth)
	return FALSE;
  if (ioctl (inf->fd, GRFSETVMODE, &mode) < 0)
	return FALSE;

  amigaRealHeight = gv->disp_height;
  amigaRealWidth = gv->disp_width;

  amigaCVadjustVirtual (ba);

  amigaCVSetPanning2 (inf, 0, 0);

  /* Turn cursor on */

  amigaCVRestoreCursor(amigaCVsavepScreen);

  return TRUE;
}
#endif /*CV64_SUPPORT */

#define MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */
#define MAX_KEYCODE	255	/* limited by the protocol */

#define AUTOREPEAT_INITIATE	200
#define AUTOREPEAT_DELAY	50

#define tvminus(tv, tv1, tv2)   /* tv = tv1 - tv2 */ \
		if ((tv1).tv_usec < (tv2).tv_usec) { \
		    (tv1).tv_usec += 1000000; \
		    (tv1).tv_sec -= 1; \
		} \
		(tv).tv_usec = (tv1).tv_usec - (tv2).tv_usec; \
		(tv).tv_sec = (tv1).tv_sec - (tv2).tv_sec;

#define tvplus(tv, tv1, tv2)    /* tv = tv1 + tv2 */ \
		(tv).tv_sec = (tv1).tv_sec + (tv2).tv_sec; \
		(tv).tv_usec = (tv1).tv_usec + (tv2).tv_usec; \
		if ((tv).tv_usec > 1000000) { \
		    (tv).tv_usec -= 1000000; \
		    (tv).tv_sec += 1; \
		}

extern KeySymsRec amigaKeySyms[];
extern AmigaModmapRec* amigaModMaps[];

Bool amigaDontZap = FALSE;
long	  	  amigaAutoRepeatInitiate = 1000 * AUTOREPEAT_INITIATE;
long	  	  amigaAutoRepeatDelay = 1000 * AUTOREPEAT_DELAY;

static int		autoRepeatKeyDown = 0;
static int		autoRepeatReady;
static int		autoRepeatFirst;
static struct timeval	autoRepeatLastKeyDownTv;
static struct timeval	autoRepeatDeltaTv;
static int killed;

void amigaKbdWait()
{
    static struct timeval lastChngKbdTransTv;
    struct timeval tv;
    struct timeval lastChngKbdDeltaTv;
    unsigned int lastChngKbdDelta;

    X_GETTIMEOFDAY(&tv);
    if (!lastChngKbdTransTv.tv_sec)
	lastChngKbdTransTv = tv;
    tvminus(lastChngKbdDeltaTv, tv, lastChngKbdTransTv);
    lastChngKbdDelta = TVTOMILLI(lastChngKbdDeltaTv);
    if (lastChngKbdDelta < 750) {
	unsigned wait;
	/*
         * We need to guarantee at least 750 milliseconds between
	 * calls to KIOCTRANS. YUCK!
	 */
	wait = (750L - lastChngKbdDelta) * 1000L;
        usleep (wait);
        X_GETTIMEOFDAY(&tv);
    }
    lastChngKbdTransTv = tv;
}


/*-
 *-----------------------------------------------------------------------
 * amigaBell --
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

#if NeedFunctionPrototypes
static void bell (
    int fd,
    int duration)
#else
static void bell (fd, duration)
    int fd;
    int duration;
#endif
{
    int		kbdCmd;		/* Command to give keyboard */

#if 0
    kbdCmd = KBD_CMD_BELL;
    if (ioctl (fd, KIOCCMD, &kbdCmd) == -1) {
      Error("Failed to activate bell");
      return;
    }
    if (duration) usleep (duration);
    kbdCmd = KBD_CMD_NOBELL;
    if (ioctl (fd, KIOCCMD, &kbdCmd) == -1)
      Error ("Failed to deactivate bell");
#endif
}

#if NeedFunctionPrototypes
static void amigaBell (
    int		    percent,
    DeviceIntPtr    device,
    pointer	    ctrl,
    int		    unused)
#else
static void amigaBell (percent, device, ctrl, unused)
    int		    percent;	    /* Percentage of full volume */
    DeviceIntPtr    device;	    /* Keyboard to ring */
    pointer	    ctrl;
    int		    unused;
#endif
{
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
    amigaKbdPrivPtr   pPriv = (amigaKbdPrivPtr) device->public.devicePrivate;
 
    if (percent == 0 || kctrl->bell == 0)
 	return;

    bell (pPriv->fd, kctrl->bell_duration * 1000);
}

static void amigaEnqueueEvent (xE)
    xEvent* xE;
{
    sigset_t holdmask;

    (void) sigaddset (&holdmask, SIGIO);
    (void) sigprocmask (SIG_BLOCK, &holdmask, (sigset_t*)NULL);
    mieqEnqueue (xE);
    (void) sigprocmask (SIG_UNBLOCK, &holdmask, (sigset_t*)NULL);
}

static KeyCode LookupKeyCode (keysym, keysymsrec)
    KeySym keysym;
    KeySymsPtr keysymsrec;
{
    KeyCode i;
    int ii, index = 0;

    for (i = keysymsrec->minKeyCode; i < keysymsrec->maxKeyCode; i++)
	for (ii = 0; ii < keysymsrec->mapWidth; ii++)
	    if (keysymsrec->map[index++] == keysym)
		return i;
}

static void pseudoKey(device, down, keycode)
    DeviceIntPtr device;
    Bool down;
    KeyCode keycode;
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

/*-
 *-----------------------------------------------------------------------
 * amigaKbdCtrl --
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

#if NeedFunctionPrototypes
static void amigaKbdCtrl (
    DeviceIntPtr    device,
    KeybdCtrl*	    ctrl)
#else
static void amigaKbdCtrl (device, ctrl)
    DeviceIntPtr    device;	    /* Keyboard to alter */
    KeybdCtrl*	    ctrl;
#endif
{
    amigaKbdPrivPtr pPriv = (amigaKbdPrivPtr) device->public.devicePrivate;

    if (pPriv->fd < 0) return;

#if 0
    if (ctrl->click != pPriv->click) {
    	int kbdClickCmd;

	pPriv->click = ctrl->click;
	kbdClickCmd = pPriv->click ? KBD_CMD_CLICK : KBD_CMD_NOCLICK;
    	if (ioctl (pPriv->fd, KIOCCMD, &kbdClickCmd) == -1)
 	    Error("Failed to set keyclick");
    }
#endif
}

/*-
 *-----------------------------------------------------------------------
 * amigaInitKbdNames --
 *    Handle the XKB initialization
 *
 * Results:
 *    None.
 *
 * Comments: 
 *     This function needs considerable work, in conjunctions with
 *     the need to add geometry descriptions of Sun Keyboards.
 *     It would also be nice to have #defines for all the keyboard
 *     layouts so that we don't have to have these hard-coded
 *     numbers.
 *
 *-----------------------------------------------------------------------
 */
#ifdef XKB
#if NeedFunctionPrototypes
static void amigaInitKbdNames (
    XkbComponentNamesRec* names,
    amigaKbdPrivPtr pKbd)
#else
static void amigaInitKbdNames (names, pKbd)
    XkbComponentNamesRec* names;
    amigaKbdPrivPtr pKbd;
#endif
{
#ifndef XKBBUFSIZE
#define XKBBUFSIZE 64
#endif
    static char keycodesbuf[XKBBUFSIZE];
    static char geometrybuf[XKBBUFSIZE];
    static char  symbolsbuf[XKBBUFSIZE];

    names->keymap = NULL;
    names->compat = "compat/complete";
    names->types  = "types/complete";
    names->keycodes = keycodesbuf;
    names->geometry = geometrybuf;
    names->symbols = symbolsbuf;
    (void) strcpy (keycodesbuf, "keycodes/");
    (void) strcpy (geometrybuf, "geometry/");
    (void) strcpy (symbolsbuf, "symbols/");

#if 0
    /* keycodes & geometry */
    switch (pKbd->type) {
    case KB_SUN2:
      (void) strcat (names->keycodes, "sun(type2)");
      (void) strcat (names->geometry, "sun(type2)");
      (void) strcat (names->symbols, "us(sun2)");
      break;
    case KB_SUN3:
      (void) strcat (names->keycodes, "sun(type3)");
      (void) strcat (names->geometry, "sun(type3)");
      (void) strcat (names->symbols, "us(sun3)");
      break;
    case KB_SUN4:
      if (pKbd->layout == 19) {
          (void) strcat (names->keycodes, "sun(US101A)");
          (void) strcat (names->geometry, "pc101-NG"); /* XXX */
          (void) strcat (names->symbols, "us(pc101)");
      } else if (pKbd->layout < 33) {
          (void) strcat (names->keycodes, "sun(type4)");
          (void) strcat (names->geometry, "sun(type4)");
          if (sunSwapLkeys)
              (void) strcat (names->symbols, "sun/us(sun4ol)");
          else
              (void) strcat (names->symbols, "sun/us(sun4)");
      } else {
          (void) strcat (names->keycodes, "sun(type5)");
          if (pKbd->layout == 34 || pKbd->layout == 81)
              (void) strcat (names->geometry, "sun(type5unix)");
          else
              (void) strcat (names->geometry, "sun(type5)");
          if (sunSwapLkeys)
              (void) strcat (names->symbols, "sun/us(sun5ol)");
          else
              (void) strcat (names->symbols, "sun/us(sun5)");
      }
      break;
    default:
      names->keycodes = names->geometry = NULL;
      break;
    }

    /* extra symbols */
    if (pKbd->type == KB_SUN4) {
      switch (pKbd->layout) {
      case  0: case  1: case 33: case 34: case 80: case 81: 
          break;
      case  3:
          (void) strcat (names->symbols, "+ca"); break;
      case  4: case 36: case 83: 
          (void) strcat (names->symbols, "+dk"); break;
      case  5: case 37: case 84: 
          (void) strcat (names->symbols, "+de"); break;
      case  6: case 38: case 85: 
          (void) strcat (names->symbols, "+it"); break;
      case  8: case 40: case 87: 
          (void) strcat (names->symbols, "+no"); break;
      case  9: case 41: case 88: 
          (void) strcat (names->symbols, "+pt"); break;
      case 10: case 42: case 89: 
          (void) strcat (names->symbols, "+es"); break;
      case 11: case 43: case 90: 
          (void) strcat (names->symbols, "+se"); break;
      case 12: case 44: case 91: 
          (void) strcat (names->symbols, "+fr_CH"); break;
      case 13: case 45: case 92: 
          (void) strcat (names->symbols, "+de_CH"); break;
      case 14: case 46: case 93: 
          (void) strcat (names->symbols, "+gb"); break; /* s/b en_UK */
      case 52:
          (void) strcat (names->symbols, "+pl"); break;
      case 53:
          (void) strcat (names->symbols, "+cs"); break;
      case 54:
          (void) strcat (names->symbols, "+ru"); break;
#if 0
      /* don't have symbols defined for these yet, let them default */
      case  2:
          (void) strcat (names->symbols, "+fr_BE"); break;
      case  7: case 39: case 86: 
          (void) strcat (names->symbols, "+nl"); break;
      case 50: case 97:
          (void) strcat (names->symbols, "+fr_CA"); break;
      case 16: case 47: case 94: 
          (void) strcat (names->symbols, "+ko"); break;
      case 17: case 48: case 95: 
          (void) strcat (names->symbols, "+tw"); break;
      case 32: case 49: case 96: 
          (void) strcat (names->symbols, "+jp"); break;
      case 51:
          (void) strcat (names->symbols, "+hu"); break;
#endif
      /* 
       * by setting the symbols to NULL XKB will use the symbols in
       * the "default" keymap.
       */
      default: 
          names->symbols = NULL; return; break;
      }
    }
#else
      (void) strcat (names->keycodes, "amiga(usa1)");
      (void) strcat (names->geometry, "amiga(usa1)");
      (void) strcat (names->symbols, "amiga(usa1)");
#endif
}
#endif /* XKB */

/*-
 *-----------------------------------------------------------------------
 * amigaKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
int amigaKbdProc (
    DeviceIntPtr  device,
    int	    	  what)
#else
int amigaKbdProc (device, what)
    DeviceIntPtr  device;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
#endif
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    amigaKbdPrivPtr pPriv;
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
	    workingKeySyms = &amigaKeySyms[amigaKbdPriv.type];

	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
	    if (workingKeySyms->maxKeyCode > MAX_KEYCODE)
		workingKeySyms->maxKeyCode = MAX_KEYCODE;
	}

	if (!workingModMap) {
	    workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
	    (void) memset(workingModMap, 0, MAP_LENGTH);
	    for(i=0; amigaModMaps[amigaKbdPriv.type][i].key != 0; i++)
		workingModMap[amigaModMaps[amigaKbdPriv.type][i].key + MIN_KEYCODE] = 
		amigaModMaps[amigaKbdPriv.type][i].modifiers;
	}

	(void) memset ((void *) defaultKeyboardControl.autoRepeats,
			~0, sizeof defaultKeyboardControl.autoRepeats);

#ifdef XKB
	if (noXkbExtension) {
	    amigaAutoRepeatInitiate = XkbDfltRepeatDelay * 1000;
	    amigaAutoRepeatDelay = XkbDfltRepeatInterval * 1000;
#endif
	autoRepeatKeyDown = 0;
#ifdef XKB
	}
#endif
	pKeyboard->devicePrivate = (pointer)&amigaKbdPriv;
	pKeyboard->on = FALSE;

#ifdef XKB
	if (noXkbExtension) {
#endif
	InitKeyboardDeviceStruct(pKeyboard, 
				 workingKeySyms, workingModMap,
				 amigaBell, amigaKbdCtrl);
#ifdef XKB
	} else {
		XkbComponentNamesRec names;
		amigaInitKbdNames (&names, &amigaKbdPriv);
		XkbInitKeyboardDeviceStruct((DeviceIntPtr) pKeyboard, &names,
						workingKeySyms, workingModMap,
						amigaBell, amigaKbdCtrl);
	}
#endif
	break;

    case DEVICE_ON:
	pPriv = (amigaKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Set the keyboard into "direct" mode and turn on
	 * event translation.
	 */
	if (amigaChangeKbdTranslation(pPriv->fd,TRUE) == -1)
	    FatalError("Can't set keyboard translation\n");
	(void) AddEnabledDevice(pPriv->fd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (amigaKbdPrivPtr)pKeyboard->devicePrivate;
	/*
	 * Restore original keyboard directness and translation.
	 */
	if (amigaChangeKbdTranslation(pPriv->fd,FALSE) == -1)
	    FatalError("Can't reset keyboard translation\n");
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
 * amigaKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */

#if NeedFunctionPrototypes
Firm_event* amigaKbdGetEvents (
    int		fd,
    Bool	on,
    int*	pNumEvents,
    Bool*	pAgain)
#else
Firm_event* amigaKbdGetEvents (fd, on, pNumEvents, pAgain)
    int		fd;
    Bool	on;
    int*	pNumEvents;
    Bool*	pAgain;
#endif
{
    int	    	  nBytes;	    /* number of bytes of events available. */
    static Firm_event	evBuf[MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read (fd, evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    Error ("Reading keyboard");
	    FatalError ("Could not read the keyboard");
	}
    } else {
	if (on) {
	    *pNumEvents = nBytes / sizeof (Firm_event);
	    *pAgain = (nBytes == sizeof (evBuf));
	} else {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	}
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * amigaKbdEnqueueEvent --
 *
 *-----------------------------------------------------------------------
 */
static xEvent	autoRepeatEvent;
static int	composeCount;
#define AmigaXK_Compose XK_Meta_R

static Bool DoSpecialKeys(device, xE, fe)
    DeviceIntPtr  device;
    xEvent*       xE;
    Firm_event* fe;
{
    int	shift_index, map_index, bit;
    KeySym ksym;
    BYTE* kptr;
    amigaKbdPrivPtr pPriv = (amigaKbdPrivPtr)device->public.devicePrivate;
    BYTE keycode = xE->u.u.detail;
    CARD8 keyModifiers = device->key->modifierMap[keycode];

    /* look up the present idea of the keysym */
    shift_index = 0;
    if (device->key->state & ShiftMask) 
	shift_index ^= 1;
    if (device->key->state & LockMask) 
	shift_index ^= 1;
    map_index = (fe->id - 1) * device->key->curKeySyms.mapWidth;
    ksym = device->key->curKeySyms.map[shift_index + map_index];
    if (ksym == NoSymbol)
	ksym = device->key->curKeySyms.map[map_index];

    /*
     * Toggle functionality is hardcoded. This is achieved by always
     * discarding KeyReleases on these keys, and converting every other
     * KeyPress into a KeyRelease.
     */
    if (xE->u.u.type == KeyRelease 
	&& (/*ksym == XK_Num_Lock 
	||*/ ksym == XK_Scroll_Lock 
	|| ksym == AmigaXK_Compose
	/*|| (keyModifiers & LockMask)*/)) 
	return TRUE;

    kptr = &device->key->down[keycode >> 3];
    bit = 1 << (keycode & 7);
    if ((*kptr & bit) &&
	(ksym == XK_Num_Lock || ksym == XK_Scroll_Lock ||
	ksym == AmigaXK_Compose || (keyModifiers & LockMask)))
	xE->u.u.type = KeyRelease;

    if (ksym == AmigaXK_Compose) {
	if (xE->u.u.type == KeyPress) composeCount = 2;
	else composeCount = 0;
    }
    if (xE->u.u.type == KeyRelease) {
	if (composeCount > 0 && --composeCount == 0) {
	    pseudoKey(device, FALSE,
		     LookupKeyCode(AmigaXK_Compose, &device->key->curKeySyms));
	}
    }

    if ((xE->u.u.type == KeyPress) && (keyModifiers == 0)) {
	/* initialize new AutoRepeater event & mark AutoRepeater on */
	autoRepeatEvent = *xE;
	autoRepeatFirst = TRUE;
	autoRepeatKeyDown++;
	autoRepeatLastKeyDownTv = fe->time;
    }
    {
	static int ctrl, alt;

	if (keyModifiers & ControlMask)
	    if (xE->u.u.type == KeyPress)
		ctrl = 1;
	    else if (xE->u.u.type == KeyRelease)
		ctrl = 0;
	if (keyModifiers & Mod1Mask)
	    if (xE->u.u.type == KeyPress)
		alt = 1;
	    else if (xE->u.u.type == KeyRelease)
		alt = 0;

	if (!amigaDontZap && keycode == 73 && xE->u.u.type == KeyRelease && ctrl && alt) {
	    TRACE(("panic keys pressed -- server aborting\n"));
	    killed = 1;
	    ddxGiveUp();
	}
#ifdef CV64_SUPPORT
	if (amigaCVsavepScreen) /* Only on Cybervision */ {
		if (keycode == 102  && ctrl && alt) {   /* KP_+ */
			if (xE->u.u.type == KeyRelease) /* only on keyrelease */
				if (amigaCVChangeMode(amigaVideoMode + 1))
					amigaVideoMode += 1;
			return TRUE; /* don't handle this event */
		}
		if (keycode == 82  && ctrl && alt) {    /* KP_- */
			if (xE->u.u.type == KeyRelease) /* only on keyrelease */
				if (amigaCVChangeMode(amigaVideoMode - 1))
					amigaVideoMode -= 1;
			return TRUE; /* don't handle this event */
		}   
                if (keycode == 42  && ctrl && alt) {   /* d for debug */
                        if (xE->u.u.type == KeyRelease) /* only on keyrelease */
                                amigaXdebug = amigaXdebug ? 0 : 1; /* toggle debug flag */
                                        
                        return TRUE; /* don't handle this event */
                }

	}
#endif /* CV64_SUPPORT */

    }
    return FALSE;
}

#if NeedFunctionPrototypes
void amigaKbdEnqueueEvent (
    DeviceIntPtr  device,
    Firm_event	  *fe)
#else
void amigaKbdEnqueueEvent (device, fe)
    DeviceIntPtr  device;
    Firm_event	  *fe;
#endif
{
    xEvent		xE;
    BYTE		keycode;
    CARD8		keyModifiers;

    if (killed)
	return;

    keycode = (fe->id & 0x7f) + MIN_KEYCODE;

    keyModifiers = device->key->modifierMap[keycode];
#ifdef XKB
    if (noXkbExtension) {
#endif
    if (autoRepeatKeyDown && (keyModifiers == 0) &&
	((fe->value == VKEY_DOWN) || (keycode == autoRepeatEvent.u.u.detail))) {
	/*
	 * Kill AutoRepeater on any real non-modifier key down, or auto key up
	 */
	autoRepeatKeyDown = 0;
    }
#ifdef XKB
    }
#endif
    xE.u.keyButtonPointer.time = TVTOMILLI(fe->time);
    xE.u.u.type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = keycode;
#ifdef XKB
    if (1) /* (noXkbExtension) */ {
#endif
    if (DoSpecialKeys(device, &xE, fe))
	return;
#ifdef XKB
    }
#endif /* ! XKB */
    mieqEnqueue (&xE);
}

void amigaEnqueueAutoRepeat ()
{
    int	delta;
    int	i, mask;
    DeviceIntPtr device = (DeviceIntPtr)LookupKeyboardDevice();
    KeybdCtrl* ctrl = &device->kbdfeed->ctrl;
    amigaKbdPrivPtr   pPriv = (amigaKbdPrivPtr) device->public.devicePrivate;

    if (ctrl->autoRepeat != AutoRepeatModeOn) {
	autoRepeatKeyDown = 0;
	return;
    }
    i=(autoRepeatEvent.u.u.detail >> 3);
    mask=(1 << (autoRepeatEvent.u.u.detail & 7));
    if (!(ctrl->autoRepeats[i] & mask)) {
	autoRepeatKeyDown = 0;
	return;
    }

    /*
     * Generate auto repeat event.	XXX one for now.
     * Update time & pointer location of saved KeyPress event.
     */

    delta = TVTOMILLI(autoRepeatDeltaTv);
    autoRepeatFirst = FALSE;

    /*
     * Fake a key up event and a key down event
     * for the last key pressed.
     */
    autoRepeatEvent.u.keyButtonPointer.time += delta;
    autoRepeatEvent.u.u.type = KeyRelease;

    /*
     * hold off any more inputs while we get these safely queued up
     * further SIGIO are 
     */
    amigaEnqueueEvent (&autoRepeatEvent);
    autoRepeatEvent.u.u.type = KeyPress;
    amigaEnqueueEvent (&autoRepeatEvent);
    if (ctrl->click) bell (pPriv->fd, 0);

    /* Update time of last key down */
    tvplus(autoRepeatLastKeyDownTv, autoRepeatLastKeyDownTv, 
		    autoRepeatDeltaTv);
}

/*-
 *-----------------------------------------------------------------------
 * amigaChangeKbdTranslation
 *	Makes operating system calls to set keyboard translation 
 *	and direction on or off.
 *
 * Results:
 *	-1 if failure, else 0.
 *
 * Side Effects:
 * 	Changes kernel management of keyboard.
 *
 *-----------------------------------------------------------------------
 */
#if NeedFunctionPrototypes
int amigaChangeKbdTranslation(
    int fd,
    Bool makeTranslated)

#else
int amigaChangeKbdTranslation(fd, makeTranslated)
    int fd;
    Bool makeTranslated;
#endif
{   
    int 	tmp;
    sigset_t	hold_mask, old_mask;
    int		toread;
    char	junk[8192];

    (void) sigfillset(&hold_mask);
    (void) sigprocmask(SIG_BLOCK, &hold_mask, &old_mask);
    amigaKbdWait();
    if (makeTranslated) {
        /*
         * Next set the keyboard into "direct" mode and turn on
         * event translation. If either of these fails, we can't go
         * on.
         */
	tmp = 1;
	if (ioctl (fd, KIOCSDIRECT, &tmp) == -1) {
	    Error ("Setting keyboard direct mode");
	    return -1;
	}
	tmp = TR_UNTRANS_EVENT;
	if (ioctl (fd, KIOCTRANS, &tmp) == -1) {
	    Error ("Setting keyboard translation");
	    ErrorF ("amigaChangeKbdTranslation: kbdFd=%d\n", fd);
	    return -1;
	}
    } else {
        /*
         * Next set the keyboard into "indirect" mode and turn off
         * event translation.
         */
	tmp = 0;
	(void)ioctl (fd, KIOCSDIRECT, &tmp);
#ifdef TR_ASCII
	tmp = TR_ASCII;
	(void)ioctl (fd, KIOCTRANS, &tmp);
#endif
    }
    if (ioctl (fd, FIONREAD, &toread) != -1 && toread > 0) {
	while (toread) {
	    tmp = toread;
	    if (toread > sizeof (junk))
		tmp = sizeof (junk);
	    (void) read (fd, junk, tmp);
	    toread -= tmp;
	}
    }
    (void) sigprocmask(SIG_SETMASK, &old_mask, (sigset_t *)NULL);
    return 0;
}

/*ARGSUSED*/
Bool LegalModifier(key, pDev)
    unsigned int key;
    DevicePtr	pDev;
{
    return TRUE;
}

/*ARGSUSED*/
void amigaBlockHandler(nscreen, pbdata, pptv, pReadmask)
    int nscreen;
    pointer pbdata;
    struct timeval **pptv;
    pointer pReadmask;
{
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;
    static struct timeval artv = { 0, 0 };	/* autorepeat timeval */

    if (!autoRepeatKeyDown)
	return;

    if (ctrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatFirst == TRUE)
	artv.tv_usec = amigaAutoRepeatInitiate;
    else
	artv.tv_usec = amigaAutoRepeatDelay;
    *pptv = &artv;

}

/*ARGSUSED*/
void amigaWakeupHandler(nscreen, pbdata, err, pReadmask)
    int nscreen;
    pointer pbdata;
    unsigned long err;
    pointer pReadmask;
{
    KeybdCtrl* ctrl = &((DeviceIntPtr)LookupKeyboardDevice())->kbdfeed->ctrl;
    struct timeval tv;

    if (ctrl->autoRepeat != AutoRepeatModeOn)
	return;

    if (autoRepeatKeyDown) {
	X_GETTIMEOFDAY(&tv);
	tvminus(autoRepeatDeltaTv, tv, autoRepeatLastKeyDownTv);
	if (autoRepeatDeltaTv.tv_sec > 0 ||
			(!autoRepeatFirst && autoRepeatDeltaTv.tv_usec >
				amigaAutoRepeatDelay) ||
			(autoRepeatDeltaTv.tv_usec >
				amigaAutoRepeatInitiate))
		autoRepeatReady++;
    }
    
    if (autoRepeatReady)
    {
	amigaEnqueueAutoRepeat ();
	autoRepeatReady = 0;
    }
}
