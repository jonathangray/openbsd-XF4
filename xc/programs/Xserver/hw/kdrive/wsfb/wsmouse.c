/**
 ** Copyright (c) 2002 Matthieu Herrb
 **
 ** $Source: /tmp/OpenBSD-XF4-repo/xc/programs/Xserver/hw/kdrive/wsfb/Attic/wsmouse.c,v $
 ** $Revision: 1.1 $
 ** $Date: 2002/06/07 07:02:30 $
 ** 
 **/
#ident "$Id: wsmouse.c,v 1.1 2002/06/07 07:02:30 matthieu Exp $"

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "kdrive.h"
#include "Xpoll.h"

int 
WsMouseInit(void)
{
	return 1;
}

void
WsMouseFini(void)
{
}

KdMouseFuncs WsMouseFuncs = {
    WsMouseInit,
    WsMouseFini
};

