/* $TOG: mivalidate.h /main/4 1998/02/09 14:49:11 kaleb $ */
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


#ifndef MIVALIDATE_H
#define MIVALIDATE_H

#include "miscstruct.h"
#include "regionstr.h"

typedef union _Validate {
    struct BeforeValidate {
	DDXPointRec	oldAbsCorner;	/* old window position */
	RegionPtr	borderVisible;	/* visible region of border, */
					/* non-null when size changes */
	Bool		resized;	/* unclipped winSize has changed - */
					/* don't call SaveDoomedAreas */
    } before;
    struct AfterValidate {
	RegionRec	exposed;	/* exposed regions, absolute pos */
	RegionRec	borderExposed;
    } after;
} ValidateRec;

#endif /* MIVALIDATE_H */
