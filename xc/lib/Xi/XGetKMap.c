/* $TOG: XGetKMap.c /main/7 1998/02/06 15:03:16 kaleb $ */

/************************************************************

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

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/
/* $XFree86: xc/lib/Xi/XGetKMap.c,v 3.1 1998/10/03 09:06:08 dawes Exp $ */

/***********************************************************************
 *
 * XGetDeviceKeyMapping - get the keymap of an extension device.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"
#include "extutil.h"
#include "XIint.h"

KeySym 
#if NeedFunctionPrototypes
*XGetDeviceKeyMapping (
    register	Display 	*dpy,
    XDevice			*dev,
#if NeedWidePrototypes
    unsigned int                first,
#else
    KeyCode			first,
#endif
    int				keycount,
    int				*syms_per_code)
#else
*XGetDeviceKeyMapping (dpy, dev, first, keycount, syms_per_code)
    register	Display 	*dpy;
    XDevice			*dev;
    KeyCode			first;
    int				keycount;
    int				*syms_per_code;		/* RETURN */
#endif
    {
    long nbytes;
    register KeySym *mapping = NULL;
    xGetDeviceKeyMappingReq *req;
    xGetDeviceKeyMappingReply rep;
    XExtDisplayInfo *info = XInput_find_display (dpy);

    LockDisplay (dpy);
    if (_XiCheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((KeySym *) NoSuchExtension);

    GetReq(GetDeviceKeyMapping,req);
    req->reqType = info->codes->major_opcode;
    req->ReqType = X_GetDeviceKeyMapping;
    req->deviceid = dev->device_id;
    req->firstKeyCode = first;
    req->count = keycount;

    if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) 
	{
	UnlockDisplay(dpy);
	SyncHandle();
	return (KeySym *) NULL;
	}
    if (rep.length > 0) {
        *syms_per_code = rep.keySymsPerKeyCode;
	nbytes = (long)rep.length << 2;
	mapping = (KeySym *) Xmalloc((unsigned) nbytes);
	if (mapping)
	    _XRead (dpy, (char *)mapping, nbytes);
	else
	    _XEatData (dpy, (unsigned long) nbytes);
      }

    UnlockDisplay(dpy);
    SyncHandle();
    return (mapping);
    }
