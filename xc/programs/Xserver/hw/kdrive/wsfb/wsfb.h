/*
 * Copyright (C) 2002 Matthieu Herrb
 *
 * $Source: /tmp/OpenBSD-XF4-repo/xc/programs/Xserver/hw/kdrive/wsfb/Attic/wsfb.h,v $
 * $Revision: 1.1 $
 * $Date: 2002/06/07 07:02:30 $
 * 
 */
#ifndef _WSFB_H
#define _WSFB_H

#include "kdrive.h"
#include "layer.h"
#ifdef RANDR
#include "randrstr.h"
#endif

Bool
wsfbCardInit(KdCardInfo *card);

Bool
wsfbScreenInit(KdScreenInfo *screen);    

Bool
wsfbInitScreen(ScreenPtr pScreen);

void
wsfbPreserve(KdCardInfo *card);

Bool
wsfbEnable(ScreenPtr pScreen);
 
Bool
wsfbDPMS (ScreenPtr pScreen, int mode);

void
wsfbDisable(ScreenPtr pScreen);

void
wsfbRestore(KdCardInfo *card);

void
wsfbScreenFini(KdScreenInfo *screen);

void
wsfbCardFini(KdCardInfo *card);

void
wsfbPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

void
wsfbGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

Bool
wsfbFinishInitScreen(ScreenPtr pScreen);

int
wsfbProcessArgument (int argc, char **argv, int i);

#endif
