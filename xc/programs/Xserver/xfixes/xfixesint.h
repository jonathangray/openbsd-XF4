/*
 * $Id: xfixesint.h,v 1.2 2006/01/08 21:18:25 matthieu Exp $
 *
 * Copyright © 2002 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _XFIXESINT_H_
#define _XFIXESINT_H_

#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include <X11/extensions/xfixesproto.h>
#include "windowstr.h"
#include "selection.h"
#include "xfixes.h"

extern unsigned char	XFixesReqCode;
extern int		XFixesEventBase;
extern int		XFixesClientPrivateIndex;

typedef struct _XFixesClient {
    CARD32	major_version;
    CARD32	minor_version;
} XFixesClientRec, *XFixesClientPtr;

#define GetXFixesClient(pClient)    ((XFixesClientPtr) (pClient)->devPrivates[XFixesClientPrivateIndex].ptr)

extern int	(*ProcXFixesVector[XFixesNumberRequests])(ClientPtr);
extern int	(*SProcXFixesVector[XFixesNumberRequests])(ClientPtr);

/* Initialize extension at server startup time */

void
XFixesExtensionInit(void);

/* Save set */
int
ProcXFixesChangeSaveSet(ClientPtr client);
    
int
SProcXFixesChangeSaveSet(ClientPtr client);
    
/* Selection events */
int
ProcXFixesSelectSelectionInput (ClientPtr client);

int
SProcXFixesSelectSelectionInput (ClientPtr client);

void
SXFixesSelectionNotifyEvent (xXFixesSelectionNotifyEvent *from,
			     xXFixesSelectionNotifyEvent *to);
Bool
XFixesSelectionInit (void);

/* Cursor notification */
Bool
XFixesCursorInit (void);
    
int
ProcXFixesSelectCursorInput (ClientPtr client);

int
SProcXFixesSelectCursorInput (ClientPtr client);

void
SXFixesCursorNotifyEvent (xXFixesCursorNotifyEvent *from,
			  xXFixesCursorNotifyEvent *to);

int
ProcXFixesGetCursorImage (ClientPtr client);

int
SProcXFixesGetCursorImage (ClientPtr client);

/* Cursor names (Version 2) */

int
ProcXFixesSetCursorName (ClientPtr client);

int
SProcXFixesSetCursorName (ClientPtr client);

int
ProcXFixesGetCursorName (ClientPtr client);

int
SProcXFixesGetCursorName (ClientPtr client);

int
ProcXFixesGetCursorImageAndName (ClientPtr client);

int
SProcXFixesGetCursorImageAndName (ClientPtr client);

/* Cursor replacement (Version 2) */

int
ProcXFixesChangeCursor (ClientPtr client);

int
SProcXFixesChangeCursor (ClientPtr client);

int
ProcXFixesChangeCursorByName (ClientPtr client);

int
SProcXFixesChangeCursorByName (ClientPtr client);

/* Region objects (Version 2* */
Bool
XFixesRegionInit (void);

int
ProcXFixesCreateRegion (ClientPtr client);

int
SProcXFixesCreateRegion (ClientPtr client);

int
ProcXFixesCreateRegionFromBitmap (ClientPtr client);

int
SProcXFixesCreateRegionFromBitmap (ClientPtr client);

int
ProcXFixesCreateRegionFromWindow (ClientPtr client);

int
SProcXFixesCreateRegionFromWindow (ClientPtr client);

int
ProcXFixesCreateRegionFromGC (ClientPtr client);

int
SProcXFixesCreateRegionFromGC (ClientPtr client);

int
ProcXFixesCreateRegionFromPicture (ClientPtr client);

int
SProcXFixesCreateRegionFromPicture (ClientPtr client);

int
ProcXFixesDestroyRegion (ClientPtr client);

int
SProcXFixesDestroyRegion (ClientPtr client);

int
ProcXFixesSetRegion (ClientPtr client);

int
SProcXFixesSetRegion (ClientPtr client);

int
ProcXFixesCopyRegion (ClientPtr client);

int
SProcXFixesCopyRegion (ClientPtr client);

int
ProcXFixesCombineRegion (ClientPtr client);

int
SProcXFixesCombineRegion (ClientPtr client);

int
ProcXFixesInvertRegion (ClientPtr client);

int
SProcXFixesInvertRegion (ClientPtr client);

int
ProcXFixesTranslateRegion (ClientPtr client);

int
SProcXFixesTranslateRegion (ClientPtr client);

int
ProcXFixesRegionExtents (ClientPtr client);

int
SProcXFixesRegionExtents (ClientPtr client);

int
ProcXFixesFetchRegion (ClientPtr client);

int
SProcXFixesFetchRegion (ClientPtr client);

int
ProcXFixesSetGCClipRegion (ClientPtr client);

int
SProcXFixesSetGCClipRegion (ClientPtr client);

int
ProcXFixesSetWindowShapeRegion (ClientPtr client);

int
SProcXFixesSetWindowShapeRegion (ClientPtr client);

int
ProcXFixesSetPictureClipRegion (ClientPtr client);

int
SProcXFixesSetPictureClipRegion (ClientPtr client);

int
ProcXFixesExpandRegion (ClientPtr client);

int
SProcXFixesExpandRegion (ClientPtr client);

#endif /* _XFIXESINT_H_ */
