/*
 */

/* $XFree86$ */
	
#include "X.h"
#include "misc.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"

#include "vga.h"

/*
 *-----------------------------------------------------------------------
 * vgaBankReadWrite ---
 *     select a memory bank of the VGA board for read & write access
 *
 * Side Effects:
 *      None.
 *-----------------------------------------------------------------------
 */
void
vgaBankReadWrite(n)
    register int n;
{
  (vgaSetReadWriteFunc)(n);
}

/*
 *-----------------------------------------------------------------------
 * vgaBankRead ---
 *     select a memory bank of the VGA board for read access
 *
 * Side Effects:
 *      None.
 *-----------------------------------------------------------------------
 */
void
vgaBankRead(n)
    register int n;
{
  (vgaSetReadFunc)(n);
}

/*
 *-----------------------------------------------------------------------
 * vgaBankWrite ---
 *     select a memory bank of the VGA board for write access
 *
 * Side Effects:
 *      None.
 *-----------------------------------------------------------------------
 */
void
vgaBankWrite(n)
    register int n;
{
  (vgaSetWriteFunc)(n);
}
