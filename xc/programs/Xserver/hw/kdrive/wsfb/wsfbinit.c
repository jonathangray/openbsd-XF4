/* $OpenBSD: wsfbinit.c,v 1.2 2002/06/07 18:48:49 matthieu Exp $ */
/*
 * Copyright (c) 2001 Matthieu Herrb
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

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
