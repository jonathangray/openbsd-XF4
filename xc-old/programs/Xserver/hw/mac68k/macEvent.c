#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "mac68k.h"

void
mac68k_enqevents()
{
	int i, len;
	char str[100];
	adb_event_t e[128];

	for (;;) {
		len = read (mac_adbfd, e, sizeof(e));
		if (len == -1) {
		    ErrorF("Failure to read from ADB device\n");
		    FatalError(sys_errlist[errno]);
		}

		if (len == 0)
		    break;

		len /= sizeof(e[0]);
		for (i = 0; i < len; i++)
		    if (e[i].def_addr == ADBADDR_MS)
			mac68k_processmouse(mac68k_mouse, &e[i]);
		    else if (e[i].def_addr == ADBADDR_KBD)
			mac68k_processkbd(mac68k_kbd, &e[i]);
	}
}

void
ProcessInputEvents()
{
	mieqProcessInputEvents();
	miPointerUpdate();
}
