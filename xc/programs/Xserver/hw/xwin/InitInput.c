/* $TOG: InitInput.c /main/12 1998/02/10 13:23:52 kaleb $ */
/*

  Copyright 1993, 1998  The Open Group

  All Rights Reserved.

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  Except as contained in this notice, the name of The Open Group shall
  not be used in advertising or otherwise to promote the sale, use or
  other dealings in this Software without prior written authorization
  from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/hw/xwin/InitInput.c,v 1.4 2001/05/02 00:45:26 alanh Exp $ */

#include "win.h"

CARD32		g_c32LastInputEventTime = 0;

/* Called from dix/devices.c */
/*
 * All of our keys generate up and down transition notifications,
 * so all of our keys can be used as modifiers.
 * 
 * An example of a modifier is mapping the A key to the Control key.
 * A has to be a legal modifier.  I think.
 */
Bool
LegalModifier (unsigned int uiKey, DevicePtr pDevice)
{
  return TRUE;
}

/* Called from dix/dispatch.c */
/* We tell mi to dequeue the events that we have sent it */
void
ProcessInputEvents (void)
{
  mieqProcessInputEvents ();
  miPointerUpdate ();
}

int
TimeSinceLastInputEvent ()
{
    if (g_c32LastInputEventTime == 0)
        g_c32LastInputEventTime = GetTickCount ();
    return GetTickCount () - g_c32LastInputEventTime;
}

/* See Porting Layer Definition - p. 17 */
void
InitInput (int argc, char *argv[])
{
  DeviceIntPtr		pMouse, pKeyboard;

#if CYGDEBUG
  ErrorF ("InitInput ()\n");
#endif

  pMouse = AddInputDevice (winMouseProc, TRUE);
  pKeyboard = AddInputDevice (winKeybdProc, TRUE);
  
  RegisterPointerDevice (pMouse);
  RegisterKeyboardDevice (pKeyboard);

  miRegisterPointerDevice (screenInfo.screens[0], pMouse);
  mieqInit ((DevicePtr)pKeyboard, (DevicePtr)pMouse);

  /* Initialize the mode key states */
  winInitializeModeKeyStates ();

  /* Only open the windows message queue device once */
  if (g_fdMessageQueue == WIN_FD_INVALID)
    {
      /* Open a file descriptor for the Windows message queue */
      g_fdMessageQueue = open (WIN_MSG_QUEUE_FNAME, O_RDONLY);
      
      /* Add the message queue as a device to wait for in WaitForSomething */
      AddEnabledDevice (g_fdMessageQueue);
    }
}

#ifdef XTESTEXT1
void
XTestGenerateEvent (int dev_type, int keycode, int keystate,
		    int mousex, int mousey)
{
  ErrorF ("XTestGenerateEvent ()\n");
}

void
XTestGetPointerPos (short *fmousex, short *fmousey)
{
  ErrorF ("XTestGetPointerPos ()\n");
}

void
XTestJumpPointer (int jx, int jy, int dev_type)
{
  ErrorF ("XTestJumpPointer ()\n");
}
#endif

