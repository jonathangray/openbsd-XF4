/*
 * misprite.h
 *
 * software-sprite/sprite drawing interface spec
 *
 * mi versions of these routines exist.
 */

/* $TOG: misprite.h /main/8 1998/02/09 14:49:03 kaleb $ */

/*

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
*/

typedef struct {
    Bool	(*RealizeCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/
#endif
);
    Bool	(*UnrealizeCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/
#endif
);
    Bool	(*PutUpCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/,
		int /*x*/,
		int /*y*/,
		unsigned long /*source*/,
		unsigned long /*mask*/
#endif
);
    Bool	(*SaveUnderCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/
#endif
);
    Bool	(*RestoreUnderCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/
#endif
);
    Bool	(*MoveCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/,
		int /*dx*/,
		int /*dy*/,
		unsigned long /*source*/,
		unsigned long /*mask*/
#endif
);
    Bool	(*ChangeSave)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/,
		int /*dx*/,
		int /*dy*/
#endif
);

} miSpriteCursorFuncRec, *miSpriteCursorFuncPtr;

extern Bool miSpriteInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miSpriteCursorFuncPtr /*cursorFuncs*/,
    miPointerScreenFuncPtr /*screenFuncs*/
#endif
);
