/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/ibm8514/imread.s,v 3.0 1996/11/18 13:09:10 dawes Exp $ */





/* $XConsortium: imread.s /main/3 1996/02/21 17:24:53 kaleb $ */
/******************************************************************************

This is a assembly language version of the ibm8514ImageRead routine.
Written by Hans Nasten ( nasten@everyware.se ) AUG 29, 1993.

void
ibm8514ImageRead(x, y, w, h, psrc, pwidth, px, py, planemask)
    int			x;
    int			y;
    int			w;
    int			h;
    unsigned char	*psrc;
    int			pwidth;
    int			px;
    int			py;
    short		planemask;

******************************************************************************/

#define _8514_ASM_
#include "assyntax.h"
#include "reg8514.h"

	AS_BEGIN

/*
 * Defines for in arguments.
 */
#define x_arg		REGOFF(8,EBP)
#define y_arg		REGOFF(12,EBP)
#define w_arg		REGOFF(16,EBP)
#define h_arg		REGOFF(20,EBP)
#define psrc_arg	REGOFF(24,EBP)
#define pwidth_arg	REGOFF(28,EBP)
#define px_arg		REGOFF(32,EBP)
#define py_arg		REGOFF(36,EBP)
#define planemask_arg	REGOFF(40,EBP)


	SEG_TEXT
	ALIGNTEXT4

GLOBL	GLNAME(ibm8514ImageRead)

GLNAME(ibm8514ImageRead):
	PUSH_L	(EBP)
	MOV_L	(ESP,EBP)
	PUSH_L	(EDI)
	PUSH_L	(ESI)
	PUSH_L	(EBX)
/*
 * Check if height or width is 0.
 */
	MOV_L	(w_arg,EDI)
	MOV_L	(h_arg,EBX)
	OR_L	(EDI,EDI)
	JZ	(.finish)
	OR_L	(EBX,EBX)
	JZ	(.finish)
/*
 * Wait for 6 queue entries
 */
        MOV_L   (GP_STAT,EDX)
.wait_queue_0:
        IN_B
        TEST_B  (CONST(0x04),AL)
        JNZ     (.wait_queue_0)
/*
 * Init 8514 registers.
 */
	MOV_L	(FRGD_MIX,EDX)
	MOV_W	(CONST(0x0047),AX)
	OUT_W

	MOV_L	(CUR_X,EDX)
	MOV_W	(x_arg,AX)
	OUT_W

	MOV_L	(CUR_Y,EDX)
	MOV_W	(y_arg,AX)
	OUT_W
/*
 * If the width is odd, program the 8514 registers for width+1.
 */
	MOV_L	(MAJ_AXIS_PCNT,EDX)
	MOV_W	(w_arg,AX)
	TEST_W	(CONST(1),AX)
	JNZ	(.odd)

	DEC_W	(AX)
.odd:
	OUT_W

.cont1:
/*
 * Set height registers.
 */
	MOV_L	(MULTIFUNC_CNTL,EDX)
	MOV_W	(h_arg,AX)
	DEC_W	(AX)
/*	OR_W	(MIN_AXIS_PCNT,AX)*/
	OUT_W

/*
 * Give command to 8514.
 * The command is : CMD_RECT | INC_Y | INC_X | DRAW
 *                  | PCDATA | _16BIT | BYTSEQ);
 */
	MOV_L	(CMD,EDX)
	MOV_W	(GP_READ_CMD,AX)
	OUT_W
/*
 * Calculate a pointer to the first pixel on the first line.
 */
	MOV_L	(pwidth_arg,EAX)
	MUL_L	(py_arg)
	ADD_L	(px_arg,EAX)
	MOV_L	(psrc_arg,EDI)
	ADD_L	(EAX,EDI)
/*
 * Wait until data is ready in the fifo.
 */
	MOV_L	(GP_STAT,EDX)
.wait_ready_1:
	IN_W
	TEST_W	(DATARDY,AX)
	JZ	(.wait_ready_1)
/*
 * Read the pixels line by line from the fifo.
 */
	CLD
	MOV_L	(PIX_TRANS,EDX)

	MOV_L	(pwidth_arg,ESI)
	SHR_L	(CONST(1),ESI)
	MOV_L	(w_arg,EBX)
	SHR_L	(CONST(1),EBX)
	SUB_L	(EBX,ESI)
	SHL_L	(CONST(1),ESI)

	MOV_W   (planemask_arg,BX)
	CMP_W   (CONST(0xff),BX)
	JZ      (.get_all_planes)

	MOV_L   (ESI,pwidth_arg)
	MOV_L   (h_arg,ESI)
	MOV_B   (BL,BH)
	TEST_W  (CONST(1),w_arg)
	JNZ     (.odd_width_masked)
/*
 * Even number of pixels on each line, but some of the bitplanes
 * should be masked out.
 */
	SHR_L   (CONST(1),w_arg)
.next_even_line_masked:
	MOV_L   (w_arg,ECX)
.next_even_word_masked:
	IN_W
	AND_W   (BX,AX)
	STOS_W
	LOOP    (.next_even_word_masked)

	ADD_L   (pwidth_arg,EDI)
	DEC_W   (SI)
	JNZ     (.next_even_line_masked)

	JMP     (.all_done)
/*
 * Odd number of pixels on each line, but some of the bitplanes
 * should be masked out. Read an additional pixel on each line.
 */
.odd_width_masked:
	SHR_L   (CONST(1),w_arg)
.next_odd_line_masked:
	MOV_L   (w_arg,ECX)
	AND_L   (ECX,ECX)
	JZ      (.odd_masked_skip)

.next_odd_word_masked:
	IN_W
	AND_W   (BX,AX)
	STOS_W
	LOOP    (.next_odd_word_masked)

.odd_masked_skip:
	IN_W
	AND_B   (BL,AL)
	MOV_B   (AL,REGIND(EDI))
	ADD_L   (pwidth_arg,EDI)
	DEC_W   (SI)
	JNZ     (.next_odd_line_masked)

	JMP     (.all_done)
/*
 * All bitplanes is to be stored. Skip planemask anding.
 */
.get_all_planes:
	MOV_W	(h_arg,BX)
	TEST_W	(CONST(1),w_arg)
	JNZ	(.odd_width)
/*
 * Even number of pixels on each line,
 * read entire line with insw.
 */
	MOV_L	(w_arg,EBP)
	SHR_L	(CONST(1),EBP)
.next_even_line:
	MOV_L	(EBP,ECX)
	REP
	INS_W
	ADD_L	(ESI,EDI)
	DEC_W	(BX)
	JNZ	(.next_even_line)

	JMP	(.all_done)
/*
 * Odd number of pixels on each line,
 * read line with insw but read last pixel with inw.
 */
.odd_width:
	MOV_L	(w_arg,EBP)
	SHR_L	(CONST(1),EBP)
.next_odd_line:
	MOV_L	(EBP,ECX)
	REP
	INS_W
	IN_W
	MOV_B	(AL,REGIND(EDI))
	ADD_L	(ESI,EDI)
	DEC_W	(BX)
	JNZ	(.next_odd_line)

/*
 * Wait until room for 1 entry in the fifo.
 */

.all_done:
	MOV_L	(GP_STAT,EDX)
.wait_queue_2:
	IN_B
	TEST_B	(CONST(0x80),AL)
	JNZ	(.wait_queue_2)
/*
 * Reset FRGD_MIX to default.
 */
	MOV_L	(FRGD_MIX,EDX)
	MOV_W	(GP_DEF_FRGD_MIX,AX)
	OUT_W

.finish:
	POP_L	(EBX)
	POP_L	(ESI)
	POP_L	(EDI)
	POP_L	(EBP)
	RET


