/*
 * $XConsortium: RA16.c,v 1.4 94/04/17 20:16:37 gildea Exp $
 *
 * 
Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 * *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86$ */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

int
XdmcpReadARRAY16 (buffer, array)
    XdmcpBufferPtr  buffer;
    ARRAY16Ptr	    array;
{
    int	    i;

    if (!XdmcpReadCARD8 (buffer, &array->length))
	return FALSE;
    if (!array->length)
    {
	array->data = 0;
	return TRUE;
    }
    array->data = (CARD16 *) Xalloc (array->length * sizeof (CARD16));
    if (!array->data)
	return FALSE;
    for (i = 0; i < (int)array->length; i++)
    {
	if (!XdmcpReadCARD16 (buffer, &array->data[i]))
	{
	    Xfree (array->data);
	    array->data = 0;
	    return FALSE;
	}
    }
    return TRUE;
}
