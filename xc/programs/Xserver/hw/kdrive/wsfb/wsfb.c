/**
 ** Copyright (c) 2002 Matthieu Herrb
 **
 ** $Source: /tmp/OpenBSD-XF4-repo/xc/programs/Xserver/hw/kdrive/wsfb/Attic/wsfb.c,v $
 ** $Revision: 1.1 $
 ** $Date: 2002/06/07 07:02:30 $
 ** 
 **/
#ident "$Id: wsfb.c,v 1.1 2002/06/07 07:02:30 matthieu Exp $"

#include "wsfb.h"

Bool
wsfbCardInit(KdCardInfo *card)
{
	return TRUE;
}

Bool
wsfbScreenInit(KdScreenInfo *screen)
{
	return TRUE;
}

Bool
wsfbInitScreen(ScreenPtr pScreen)
{
	return TRUE;
}

void
wsfbPreserve(KdCardInfo *card)
{
}

Bool
wsfbEnable(ScreenPtr pScreen)
{
	return TRUE;
}
 
Bool
wsfbDPMS (ScreenPtr pScreen, int mode)
{
	return TRUE;
}

void
wsfbDisable(ScreenPtr pScreen)
{
}

void
wsfbRestore(KdCardInfo *card)
{
}

void
wsfbScreenFini(KdScreenInfo *screen)
{
}

void
wsfbCardFini(KdCardInfo *card)
{
}

void
wsfbPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
}

void
wsfbGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
}

Bool
wsfbFinishInitScreen(ScreenPtr pScreen)
{
	return TRUE;
}

int
wsfbProcessArgument (int argc, char **argv, int i)
{
	return 0;
}
