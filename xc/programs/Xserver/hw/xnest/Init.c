/* $TOG: Init.c /main/8 1997/11/12 14:38:50 kaleb $ */
/*

Copyright 1993 by Davor Matic

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.  Davor Matic makes no representations about
the suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

*/
/* $XFree86: xc/programs/Xserver/hw/xnest/Init.c,v 3.19 2000/11/27 17:45:56 dawes Exp $ */

#include "X.h"
#include "Xproto.h"
#include "screenint.h"
#include "input.h"
#include "misc.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"

#include "Xnest.h"

#include "Display.h"
#include "Screen.h"
#include "Pointer.h"
#include "Keyboard.h"
#include "Handlers.h"
#include "Init.h"
#include "Args.h"
#include "Drawable.h"
#include "XNGC.h"
#include "XNFont.h"

#ifdef XFree86Server
/*
 * when building the loader, we add some code that tries to 
 * switch bit ordering based on xf86bpp; since Xnest doesn't
 * use that, we have to add this dummy here
 */
int xf86bpp = 8;
#endif

Bool xnestDoFullGeneration = True;

void InitOutput(screenInfo, argc, argv)
     ScreenInfo *screenInfo;
     int argc;
     char *argv[];
{
  int i, j;

  xnestOpenDisplay(argc, argv);
  
  screenInfo->imageByteOrder = ImageByteOrder(xnestDisplay);
  screenInfo->bitmapScanlineUnit = BitmapUnit(xnestDisplay);
  screenInfo->bitmapScanlinePad = BitmapPad(xnestDisplay);
  screenInfo->bitmapBitOrder = BitmapBitOrder(xnestDisplay);
  
  screenInfo->numPixmapFormats = 0;
  for (i = 0; i < xnestNumPixmapFormats; i++) 
    for (j = 0; j < xnestNumDepths; j++)
      if ((xnestPixmapFormats[i].depth == 1) ||
          (xnestPixmapFormats[i].depth == xnestDepths[j])) {
	screenInfo->formats[screenInfo->numPixmapFormats].depth = 
	  xnestPixmapFormats[i].depth;
	screenInfo->formats[screenInfo->numPixmapFormats].bitsPerPixel = 
	  xnestPixmapFormats[i].bits_per_pixel;
	screenInfo->formats[screenInfo->numPixmapFormats].scanlinePad = 
	  xnestPixmapFormats[i].scanline_pad;
	screenInfo->numPixmapFormats++;
	break;
      }
  
  xnestWindowPrivateIndex = AllocateWindowPrivateIndex();
  xnestGCPrivateIndex = AllocateGCPrivateIndex();
  xnestFontPrivateIndex = AllocateFontPrivateIndex();
  
  if (!xnestNumScreens) xnestNumScreens = 1;

  for (i = 0; i < xnestNumScreens; i++)
    AddScreen(xnestOpenScreen, argc, argv);

  xnestNumScreens = screenInfo->numScreens;

  xnestDoFullGeneration = xnestFullGeneration;
}

void InitInput(argc, argv)
     int argc;
     char *argv[];
{
  DeviceIntPtr ptr, kbd;

  ptr = AddInputDevice(xnestPointerProc, TRUE);
  kbd = AddInputDevice(xnestKeyboardProc, TRUE);

  RegisterPointerDevice(ptr);
  RegisterKeyboardDevice(kbd);

  mieqInit(kbd, ptr);

  AddEnabledDevice(XConnectionNumber(xnestDisplay));

  RegisterBlockAndWakeupHandlers(xnestBlockHandler, xnestWakeupHandler, NULL);
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void AbortDDX()
{
  xnestDoFullGeneration = True;
  xnestCloseDisplay();
}

/* Called by GiveUp(). */
void ddxGiveUp()
{
  AbortDDX();
}

void OsVendorInit()
{
    return;
}

void OsVendorFatalError()
{
    return;
}

/* this is just to get the server to link on AIX */
#ifdef AIXV3
int SelectWaitTime = 10000; /* usec */
#endif

#ifdef DPMSExtension
/**************************************************************
 * DPMSSet(), DPMSGet(), DPMSSupported()
 *
 * stubs
 *
 ***************************************************************/

void DPMSSet (level)
    int level;
{
}

int DPMSGet (level)
    int* level;
{
    return -1;
}

Bool DPMSSupported ()
{
    return FALSE;
}
#endif
