#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "dix.h"
#include "dixstruct.h"
#include "mac68k.h"
#include "opaque.h"

Bool
mac68kMonoSave(screen, on)
	ScreenPtr	screen;
	int		on;
{
	/* XXX Implement this */
	/* if(on == SCREEN_SAVER_FORCER)
		Do something with event time */
	return(FALSE);
}

Bool
mac68kMonoInit(index, screen, argc, argv)
	int	index;
	ScreenPtr	screen;
	int	argc;
	char	**argv;
{
	struct grfmode *id;
	char scrstr[150];

	if (mac_fbs[index].added != 0)
		return(TRUE);

	screen->SaveScreen = mac68kMonoSave;
	screen->whitePixel = 0;
	screen->blackPixel = 1;
	id = &mac_fbs[index].idata;
	printf("Calling ScreenInit to add screen %d...\n", index);
	sprintf(scrstr, "Screen %d at %#08x, %d by %d, rowB %d, fbbase %#x.\n",
		index, mac_fbs[index].vaddr, id->width,
		id->height, id->rowbytes, id->fbbase);
	ErrorF(scrstr);
	if (!mfbScreenInit(screen,
		mac_fbs[index].vaddr,		/* BARF */
		id->width,
		id->height,
		/* id->vRes >> 16 */ 75,	/* BARF */
		/* id->vRes >> 16 */ 75,	/* BARF */
		id->rowbytes*8))
			return(FALSE);
	mac_fbs[index].added = 1;
	return(mac68k_screeninit(screen) && mfbCreateDefColormap(screen));
}
