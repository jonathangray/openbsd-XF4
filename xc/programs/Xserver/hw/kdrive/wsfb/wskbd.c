/**
 ** Copyright (c) 2002 Matthieu Herrb
 **
 ** $Source: /tmp/OpenBSD-XF4-repo/xc/programs/Xserver/hw/kdrive/wsfb/Attic/wskbd.c,v $
 ** $Revision: 1.1 $
 ** $Date: 2002/06/07 07:02:30 $
 ** 
 **/
#ident "$Id: wskbd.c,v 1.1 2002/06/07 07:02:30 matthieu Exp $"

#include "kdrive.h"
#include "kkeymap.h"
#define XK_PUBLISHING
#include <X11/keysym.h>

void
WsKbdRead(int fd, void *closure)
{
}

void
WsKbdLoad(void)
{
}

int
WsKbdInit(void)
{
	return 1;
}

void
WsKbdLeds(int leds)
{
}

void
WsKbdBell(int volume, int pitch, int duration)
{
}

void
WsKbdFini(void)
{
}

KdKeyboardFuncs WsKbdFuncs = {
    WsKbdLoad,
    WsKbdInit,
    WsKbdLeds,
    WsKbdBell,
    WsKbdFini,
    3,
};

