/**
 ** Copyright (c) 2002 Matthieu Herrb
 **
 ** $Source: /tmp/OpenBSD-XF4-repo/xc/programs/Xserver/hw/kdrive/wsfb/Attic/wsfbinit.c,v $
 ** $Revision: 1.1 $
 ** $Date: 2002/06/07 07:02:30 $
 ** 
 **/
#ident "$Id: wsfbinit.c,v 1.1 2002/06/07 07:02:30 matthieu Exp $"

#include "wsfb.h"

const KdCardFuncs wsfbFuncs = {
	wsfbCardInit,               /* cardinit */
	wsfbScreenInit,             /* scrinit */
	wsfbInitScreen,             /* initScreen */
	wsfbPreserve,               /* preserve */
	wsfbEnable,                 /* enable */
	wsfbDPMS,                   /* dpms */
	wsfbDisable,                /* disable */
	wsfbRestore,                /* restore */
	wsfbScreenFini,             /* scrfini */
	wsfbCardFini,               /* cardfini */
	
	0,                          /* initCursor */
	0,                          /* enableCursor */
	0,                          /* disableCursor */
	0,                          /* finiCursor */
	0,                          /* recolorCursor */
	
	0,                          /* initAccel */
	0,                          /* enableAccel */
	0,                          /* syncAccel */
	0,                          /* disableAccel */
	0,                          /* finiAccel */
	
	wsfbGetColors,              /* getColors */
	wsfbPutColors,              /* putColors */
	
	wsfbFinishInitScreen,       /* finishInitScreen */
};


void
InitCard(char *name)
{
    KdCardAttr attr;
    KdCardInfoAdd((KdCardFuncs *) &wsfbFuncs, &attr, 0);
}

void
InitOutput (ScreenInfo *pScreenInfo, int argc, char **argv)
{
    KdInitOutput (pScreenInfo, argc, argv);
}

void
InitInput (int argc, char **argv)
{
    KdInitInput(&WsMouseFuncs, &WsKbdFuncs);
}

int
ddxProcessArgument (int argc, char **argv, int i)
{
    int ret;
    
    if (!(ret = wsfbProcessArgument (argc, argv, i)))
	    ret = KdProcessArgument(argc, argv, i);
    return ret;
}
