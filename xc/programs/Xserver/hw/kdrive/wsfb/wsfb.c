/* $OpenBSD: wsfb.c,v 1.2 2002/06/07 18:48:49 matthieu Exp $ */
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
