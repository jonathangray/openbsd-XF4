/* $Xorg: AllWidgets.c,v 1.3 2000/08/17 19:45:30 cpqbld Exp $ */

/*

Copyright (c) 1991, 1994, 1998  The Open Group

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

*/

#include <X11/IntrinsicP.h>
#include <X11/Xmu/WidgetNode.h>

extern WidgetClass applicationShellWidgetClass;
extern WidgetClass asciiSinkObjectClass;
extern WidgetClass asciiSrcObjectClass;
extern WidgetClass asciiTextWidgetClass;
extern WidgetClass boxWidgetClass;
extern WidgetClass commandWidgetClass;
extern WidgetClass compositeWidgetClass;
extern WidgetClass constraintWidgetClass;
extern WidgetClass coreWidgetClass;
extern WidgetClass dialogWidgetClass;
extern WidgetClass formWidgetClass;
extern WidgetClass gripWidgetClass;
extern WidgetClass labelWidgetClass;
extern WidgetClass listWidgetClass;
extern WidgetClass menuButtonWidgetClass;
extern WidgetClass multiSinkObjectClass;
extern WidgetClass multiSrcObjectClass;
extern WidgetClass objectClass;
extern WidgetClass overrideShellWidgetClass;
extern WidgetClass panedWidgetClass;
extern WidgetClass pannerWidgetClass;
extern WidgetClass portholeWidgetClass;
extern WidgetClass rectObjClass;
extern WidgetClass repeaterWidgetClass;
extern WidgetClass scrollbarWidgetClass;
extern WidgetClass shellWidgetClass;
extern WidgetClass simpleMenuWidgetClass;
extern WidgetClass simpleWidgetClass;
extern WidgetClass smeBSBObjectClass;
extern WidgetClass smeLineObjectClass;
extern WidgetClass smeObjectClass;
extern WidgetClass stripChartWidgetClass;
extern WidgetClass textSinkObjectClass;
extern WidgetClass textSrcObjectClass;
extern WidgetClass textWidgetClass;
extern WidgetClass toggleWidgetClass;
extern WidgetClass topLevelShellWidgetClass;
extern WidgetClass transientShellWidgetClass;
extern WidgetClass treeWidgetClass;
extern WidgetClass vendorShellWidgetClass;
extern WidgetClass viewportWidgetClass;
extern WidgetClass wmShellWidgetClass;

XmuWidgetNode XawWidgetArray[] = {
{ "applicationShell", &applicationShellWidgetClass },
{ "asciiSink", &asciiSinkObjectClass },
{ "asciiSrc", &asciiSrcObjectClass },
{ "asciiText", &asciiTextWidgetClass },
{ "box", &boxWidgetClass },
{ "command", &commandWidgetClass },
{ "composite", &compositeWidgetClass },
{ "constraint", &constraintWidgetClass },
{ "core", &coreWidgetClass },
{ "dialog", &dialogWidgetClass },
{ "form", &formWidgetClass },
{ "grip", &gripWidgetClass },
{ "label", &labelWidgetClass },
{ "list", &listWidgetClass },
{ "menuButton", &menuButtonWidgetClass },
{ "multiSink", &multiSinkObjectClass },
{ "multiSrc", &multiSrcObjectClass },
{ "object", &objectClass },
{ "overrideShell", &overrideShellWidgetClass },
{ "paned", &panedWidgetClass },
{ "panner", &pannerWidgetClass },
{ "porthole", &portholeWidgetClass },
{ "rect", &rectObjClass },
{ "repeater", &repeaterWidgetClass },
{ "scrollbar", &scrollbarWidgetClass },
{ "shell", &shellWidgetClass },
{ "simpleMenu", &simpleMenuWidgetClass },
{ "simple", &simpleWidgetClass },
{ "smeBSB", &smeBSBObjectClass },
{ "smeLine", &smeLineObjectClass },
{ "sme", &smeObjectClass },
{ "stripChart", &stripChartWidgetClass },
{ "textSink", &textSinkObjectClass },
{ "textSrc", &textSrcObjectClass },
{ "text", &textWidgetClass },
{ "toggle", &toggleWidgetClass },
{ "topLevelShell", &topLevelShellWidgetClass },
{ "transientShell", &transientShellWidgetClass },
{ "tree", &treeWidgetClass },
{ "vendorShell", &vendorShellWidgetClass },
{ "viewport", &viewportWidgetClass },
{ "wmShell", &wmShellWidgetClass },
};

int XawWidgetCount = XtNumber(XawWidgetArray);

