/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/et4000w32/asm/w32it.s,v 3.3 1996/02/04 08:59:20 dawes Exp $ */
/*******************************************************************************
                        Copyright 1994 by Glenn G. Lai

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyr notice appear in all copies and that
both that copyr notice and this permission notice appear in
supporting documentation, and that the name of Glenn G. Lai not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

Glenn G. Lai DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

Glenn G. Lai
P.O. Box 4314
Austin, Tx 78765
glenn@cs.utexas.edu
9/8/94
*******************************************************************************/
/* $XConsortium: w32it.s /main/4 1996/02/21 17:19:42 kaleb $ */
/*
 *	W32ImageText[1-4](h, char1)
 */


#include "assyntax.h"

	FILE("w32it.s")
	AS_BEGIN

#define h		REGOFF(8,ESP)
#define char1		REGOFF(12,ESP)

	GLOBL	GLNAME(W32ImageText1)
	GLOBL	GLNAME(W32ImageText2)
	GLOBL	GLNAME(W32ImageText3)
	GLOBL	GLNAME(W32ImageText4)

	SEG_TEXT
	ALIGNTEXT4

GLNAME(W32ImageText1):
	PUSH_L	(ESI)

	MOV_L	(char1, ESI)
	MOV_L	(CONTENT(GLNAME(ACL)), EDX)
	MOV_L	(h, ECX)
	INC_L	(ECX)
	JMP	(ita1)
	ALIGNTEXT4
ita2:
	MOV_L	(REGIND(ESI), EAX)
	ADD_L	(CONST(4), ESI)
	MOV_B	(AL, REGIND(EDX))
ita1:
	DEC_L	(ECX)
	JNZ	(ita2)

	POP_L	(ESI)
	RET

	ALIGNTEXT4
GLNAME(W32ImageText2):
	PUSH_L	(ESI)

	MOV_L	(char1, ESI)
	MOV_L	(CONTENT(GLNAME(ACL)), EDX)
	MOV_L	(h, ECX)
	INC_L	(ECX)
	JMP	(itb1)
	ALIGNTEXT4
itb2:
	MOV_L	(REGIND(ESI), EAX)
	ADD_L	(CONST(4), ESI)
	MOV_B	(AL, REGIND(EDX))
	MOV_B	(AH, REGIND(EDX))
itb1:
	DEC_L	(ECX)
	JNZ	(itb2)

	POP_L	(ESI)
	RET

	ALIGNTEXT4
GLNAME(W32ImageText3):
	PUSH_L	(ESI)

	MOV_L	(char1, ESI)
	MOV_L	(CONTENT(GLNAME(ACL)), EDX)
	MOV_L	(h, ECX)
	INC_L	(ECX)
	JMP	(itc1)
	ALIGNTEXT4
itc2:
	MOV_L	(REGIND(ESI), EAX)
	ADD_L	(CONST(4), ESI)
	MOV_B	(AL, REGIND(EDX))
	MOV_B	(AH, REGIND(EDX))
	SHR_L	(CONST(16), EAX)
	MOV_B	(AL, REGIND(EDX))
itc1:
	DEC_L	(ECX)
	JNZ	(itc2)

	POP_L	(ESI)
	RET

	ALIGNTEXT4
GLNAME(W32ImageText4):
	PUSH_L	(ESI)

	MOV_L	(char1, ESI)
	MOV_L	(CONTENT(GLNAME(ACL)), EDX)
	MOV_L	(h, ECX)
	INC_L	(ECX)
	JMP	(itd1)
	ALIGNTEXT4
itd2:
	MOV_L	(REGIND(ESI), EAX)
	ADD_L	(CONST(4), ESI)
	MOV_B	(AL, REGIND(EDX))
	MOV_B	(AH, REGIND(EDX))
	SHR_L	(CONST(16), EAX)
	MOV_B	(AL, REGIND(EDX))
	MOV_B	(AH, REGIND(EDX))
itd1:
	DEC_L	(ECX)
	JNZ	(itd2)

	POP_L	(ESI)
	RET
