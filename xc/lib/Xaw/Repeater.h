/*
 * $TOG: Repeater.h /main/6 1998/02/11 14:54:43 kaleb $
 *
Copyright 1989, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */
/* $XFree86: xc/lib/Xaw/Repeater.h,v 1.4 1998/10/03 08:42:16 dawes Exp $ */

#ifndef _XawRepeater_h
#define _XawRepeater_h

#include <X11/Xaw/Command.h>

/*****************************************************************************
 * 
 * Repeater Widget (subclass of Command)
 * 
 * This widget is a push button that repeatedly fires when held down
 * 
 * Parameters:
 * 
 *  Name		Class		Type		Default
 *  ----		-----		----		-------
 * 
 *  decay		Decay		int		5 milliseconds
 *  flash		Boolean		Boolean		FALSE
 *  initialDelay	Delay		int		200 milliseconds
 *  minimumDelay	MinimumDelay	int		10 milliseconds
 *  repeatDelay		Delay		int		50 milliseconds
 *  startCallback	StartCallback	XtCallbackList	NULL
 *  stopCallback	StopCallback	XtCallbackList	NULL
 * 
 *****************************************************************************/

#define XtNdecay "decay"
#define XtCDecay "Decay"
#define XtNinitialDelay "initialDelay"
#define XtCDelay "Delay"
#define XtNminimumDelay "minimumDelay"
#define XtCMinimumDelay "MinimumDelay"
#define XtNrepeatDelay "repeatDelay"
#define XtNflash "flash"
#define XtNstartCallback "startCallback"
#define XtCStartCallback "StartCallback"
#define XtNstopCallback "stopCallback"
#define XtCStopCallback "StopCallback"


extern WidgetClass repeaterWidgetClass;

typedef struct _RepeaterClassRec *RepeaterWidgetClass;
typedef struct _RepeaterRec      *RepeaterWidget;

#endif /* _XawRepeater_h */
