/* $Xorg: bitmap.h,v 1.3 2000/08/17 19:46:38 cpqbld Exp $ */

/*

Copyright 1990, 1998  The Open Group

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
/* $XFree86: xc/lib/font/include/bitmap.h,v 1.9 2001/01/17 19:43:31 dawes Exp $ */

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "fntfilio.h"
#ifndef FONTMODULE
#include <stdio.h>  /* just for NULL */
#else
#include "xf86_ansic.h"
#endif

/*
 * Internal format used to store bitmap fonts
 */

/* number of encoding entries in one segment */
#define BITMAP_FONT_SEGMENT_SIZE 128

typedef struct _BitmapExtra {
    Atom       *glyphNames;
    int        *sWidths;
    CARD32      bitmapsSizes[GLYPHPADOPTIONS];
    FontInfoRec info;
}           BitmapExtraRec, *BitmapExtraPtr;

typedef struct _BitmapFont {
    unsigned    version_num;
    int         num_chars;
    int         num_tables;
    CharInfoPtr metrics;	/* font metrics, including glyph pointers */
    xCharInfo  *ink_metrics;	/* ink metrics */
    char       *bitmaps;	/* base of bitmaps, useful only to free */
    CharInfoPtr **encoding;	/* array of arrays of char info pointers */
    CharInfoPtr pDefault;	/* default character */
    BitmapExtraPtr bitmapExtra;	/* stuff not used by X server */
}           BitmapFontRec, *BitmapFontPtr;

#define ACCESSENCODING(enc,i) \
(enc[(i)/BITMAP_FONT_SEGMENT_SIZE]?\
(enc[(i)/BITMAP_FONT_SEGMENT_SIZE][(i)%BITMAP_FONT_SEGMENT_SIZE]):\
0)
#define ACCESSENCODINGL(enc,i) \
(enc[(i)/BITMAP_FONT_SEGMENT_SIZE][(i)%BITMAP_FONT_SEGMENT_SIZE])

#define SEGMENT_MAJOR(n) ((n)/BITMAP_FONT_SEGMENT_SIZE)
#define SEGMENT_MINOR(n) ((n)%BITMAP_FONT_SEGMENT_SIZE)
#define NUM_SEGMENTS(n) \
  (((n)+BITMAP_FONT_SEGMENT_SIZE-1)/BITMAP_FONT_SEGMENT_SIZE)

extern int bitmapGetGlyphs ( FontPtr pFont, unsigned long count, 
			     unsigned char *chars, FontEncoding charEncoding, 
			     unsigned long *glyphCount, CharInfoPtr *glyphs );
extern int bitmapGetMetrics ( FontPtr pFont, unsigned long count, 
			      unsigned char *chars, FontEncoding charEncoding,
			      unsigned long *glyphCount, xCharInfo **glyphs );

extern void bitmapComputeFontBounds ( FontPtr pFont );
extern void bitmapComputeFontInkBounds ( FontPtr pFont );
extern Bool bitmapAddInkMetrics ( FontPtr pFont );
extern int bitmapComputeWeight ( FontPtr pFont );

extern int BitmapOpenBitmap ( FontPathElementPtr fpe, FontPtr *ppFont, 
			      int flags, FontEntryPtr entry, char *fileName, 
			      fsBitmapFormat format, fsBitmapFormatMask fmask,
			      FontPtr non_cachable_font );
extern int BitmapGetInfoBitmap ( FontPathElementPtr fpe, 
				 FontInfoPtr pFontInfo, FontEntryPtr entry, 
				 char *fileName );
extern void BitmapRegisterFontFileFunctions ( void );
extern int BitmapGetRenderIndex ( FontRendererPtr renderer );

extern int BitmapOpenScalable ( FontPathElementPtr fpe, FontPtr *pFont, 
				int flags, FontEntryPtr entry, char *fileName,
				FontScalablePtr vals, fsBitmapFormat format, 
				fsBitmapFormatMask fmask, 
				FontPtr non_cachable_font );
extern int BitmapGetInfoScalable ( FontPathElementPtr fpe, 
				   FontInfoPtr pFontInfo, FontEntryPtr entry, 
				   FontNamePtr fontName, char *fileName, 
				   FontScalablePtr vals );

#endif				/* _BITMAP_H_ */
