#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "inputstr.h"
#include "mac68k.h"
#include "Xproto.h"

extern CARD8		*macIIModMap[];	/* Borrowed from A/UX code */
extern KeySymsRec	macIIKeySyms[];	/* Borrowed from A/UX code */

void
mac68k_bell()
{
	printf("");
	fflush(stdout);
}

void
mac68k_kbdctrl()
{
}

int
mac68k_kbdproc(kbd, what)
	DevicePtr kbd;
	int what;
{
	switch (what) {
		case DEVICE_INIT:
			if (kbd != LookupKeyboardDevice()) {
				ErrorF("Kbd routines can only handle DESKTOP"
					" keyboards.\n");
				return(!Success);
			}

			InitKeyboardDeviceStruct(kbd, &macIIKeySyms[0],
				macIIModMap[1], mac68k_bell, mac68k_kbdctrl);
			kbd->on = FALSE;
			break;

		case DEVICE_ON:
			kbd->on = TRUE;
			break;

		case DEVICE_OFF:
			kbd->on = FALSE;
			break;

		case DEVICE_CLOSE:
			/* nothing! */
			break;
	}

	return(Success);
}

void
mac68k_getkbd()
{
}

void
mac68k_processkbd(kbd, event)
	DevicePtr	kbd;
	adb_event_t	*event;
{
	xEvent	xev;
	int	d;

	mac_lasteventtime = xev.u.keyButtonPointer.time =
		TVTOMILLI(event->timestamp);

	xev.u.u.type = (event->u.k.key & 0x80) ? KeyRelease : KeyPress;
	d = xev.u.u.detail = (event->u.k.key & 0x7f);
	if (d < 8) {
		switch (d) {
		default:
		case 0: xev.u.u.detail = 0x0a; break;
		case 1: xev.u.u.detail = 0x34; break;
		case 2: xev.u.u.detail = 0x3f; break;
		case 3: xev.u.u.detail = 0x40; break;
		case 4: xev.u.u.detail = 0x42; break;
		case 5: xev.u.u.detail = 0x44; break;
		case 6: xev.u.u.detail = 0x46; break;
		case 7: xev.u.u.detail = 0x48; break;
		}
	}

	mieqEnqueue(&xev);
}

Bool
LegalModifier(key, dev)
	unsigned int key;
	DevicePtr dev;
{
	return(TRUE);
}
