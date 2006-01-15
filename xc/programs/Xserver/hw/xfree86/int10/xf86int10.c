/* $XFree86: xc/programs/Xserver/hw/xfree86/int10/xf86int10.c,v 1.10tsi Exp $ */
/*
 *                   XFree86 int10 module
 *   execute BIOS int 10h calls in x86 real mode environment
 *                 Copyright 1999 Egbert Eich
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86_ansic.h"
#include "compiler.h"
#define _INT10_PRIVATE
#include "xf86int10.h"
#include "int10Defines.h"

#define REG pInt

xf86Int10InfoPtr Int10Current = NULL;

static int int1A_handler(xf86Int10InfoPtr pInt);
#ifndef _PC
static int int42_handler(xf86Int10InfoPtr pInt);
#endif
static int intE6_handler(xf86Int10InfoPtr pInt);
static PCITAG findPci(xf86Int10InfoPtr pInt, unsigned short bx);
static CARD32 pciSlotBX(pciVideoPtr pvp);

int
int_handler(xf86Int10InfoPtr pInt)
{
    int num = pInt->num;
    int ret = 0;

    switch (num) {
#ifndef _PC
    case 0x10:
    case 0x42:
    case 0x6D:
	if (getIntVect(pInt, num) == I_S_DEFAULT_INT_VECT)
	    ret = int42_handler(pInt);
	break;
#endif
    case 0x1A:
	ret = int1A_handler(pInt);
	break;
    case 0xe6:
	ret = intE6_handler(pInt);
	break;
    default:
	break;
    }

    if (!ret)
	ret = run_bios_int(num, pInt);

    if (!ret) {
	xf86DrvMsg(pInt->scrnIndex, X_ERROR,
	    "Halting on int 0x%2.2x!\n", num);
	dump_registers(pInt);
	stack_trace(pInt);
    }

    return ret;
}

#ifndef _PC
/*
 * This is derived from a number of PC system BIOS'es.  The intent here is to
 * provide very primitive video support, before an EGA/VGA BIOS installs its
 * own interrupt vector.  Here, "Ignored" calls should remain so.  "Not
 * Implemented" denotes functionality that can be implemented should the need
 * arise.  What are "Not Implemented" throughout are video memory accesses.
 * Also, very little input validity checking is done here.
 */
static int
int42_handler(xf86Int10InfoPtr pInt)
{
    switch (X86_AH) {
    case 0x00:
	/* Set Video Mode                                     */
	/* Enter:  AL = video mode number                     */
	/* Leave:  Nothing                                    */
	/* Implemented (except for clearing the screen)       */
	{                                         /* Localise */
	    IOADDRESS ioport;
	    int i;
	    CARD16 int1d, regvals, tmp;
	    CARD8 mode, cgamode, cgacolour;

	    /*
	     * Ignore all mode numbers but 0x00-0x13.  Some systems also ignore
	     * 0x0B and 0x0C, but don't do that here.
	     */
	    if (X86_AL > 0x13)
		break;

	    /*
	     * You didn't think that was really the mode set, did you?  There
	     * are only so many slots in the video parameter table...
	     */
	    mode = X86_AL;
	    ioport = 0x03D4;
	    switch (MEM_RB(pInt, 0x0410) & 0x30) {
	    case 0x30:                  /* MDA */
		mode = 0x07;            /* Force mode to 0x07 */
		ioport = 0x03B4;
		break;
	    case 0x10:                  /* CGA 40x25 */
		if (mode >= 0x07)
		    mode = 0x01;
		break;
	    case 0x20:                  /* CGA 80x25 (MCGA?) */
		if (mode >= 0x07)
		    mode = 0x03;
		break;
	    case 0x00:                  /* EGA/VGA */
		if (mode >= 0x07)       /* Don't try MDA timings */
		    mode = 0x01;        /* !?!?! */
		break;
	    }

	    /* Locate data in video parameter table */
	    int1d = MEM_RW(pInt, 0x1d << 2);
	    regvals = ((mode >> 1) << 4) + int1d;
	    cgacolour = 0x30;
	    if (mode == 0x06) {
		regvals -= 0x10;
		cgacolour = 0x3F;
	    }

	    /** Update BIOS Data Area **/

	    /* Video mode */
	    MEM_WB(pInt, 0x0449, mode);

	    /* Columns */
	    tmp = MEM_RB(pInt, mode + int1d + 0x48);
	    MEM_WW(pInt, 0x044A, tmp);

	    /* Page length */
	    tmp = MEM_RW(pInt, (mode & 0x06) + int1d + 0x40);
	    MEM_WW(pInt, 0x044C, tmp);

	    /* Start Address */
	    MEM_WW(pInt, 0x044E, 0);

	    /* Cursor positions, one for each display page */
	    for (i = 0x0450; i < 0x0460; i += 2)
		MEM_WW(pInt, i, 0);

	    /* Cursor start & end scanlines */
	    tmp = MEM_RB(pInt, regvals + 0x0B);
	    MEM_WB(pInt, 0x0460, tmp);
	    tmp = MEM_RB(pInt, regvals + 0x0A);
	    MEM_WB(pInt, 0x0461, tmp);

	    /* Current display page number */
	    MEM_WB(pInt, 0x0462, 0);

	    /* CRTC I/O address */
	    MEM_WW(pInt, 0x0463, ioport);

	    /* CGA Mode register value */
	    cgamode = MEM_RB(pInt, mode + int1d + 0x50);
	    MEM_WB(pInt, 0x0465, cgamode);

	    /* CGA Colour register value */
	    MEM_WB(pInt, 0x0466, cgacolour);

	    /* Rows */
	    MEM_WB(pInt, 0x0484, (25 - 1));

	    /* Remap I/O port number into its domain */
	    ioport += pInt->ioBase;

	    /* Programme the mode */
	    outb(ioport + 4, cgamode & 0x37);   /* Turn off screen */
	    for (i = 0; i < 0x10; i++) {
		tmp = MEM_RB(pInt, regvals + i);
		outb(ioport, i);
		outb(ioport + 1, tmp);
	    }
	    outb(ioport + 5, cgacolour);        /* Select colour mode */
	    outb(ioport + 4, cgamode);          /* Turn on screen */
	}
	break;

    case 0x01:
	/* Set Cursor Type                                    */
	/* Enter:  CH = starting line for cursor              */
	/*         CL = ending line for cursor                */
	/* Leave:  Nothing                                    */
	/* Implemented                                        */
	{                                         /* Localise */
	    IOADDRESS ioport = MEM_RW(pInt, 0x0463) + pInt->ioBase;

	    MEM_WB(pInt, 0x0460, X86_CL);
	    MEM_WB(pInt, 0x0461, X86_CH);

	    outb(ioport, 0x0A);
	    outb(ioport + 1, X86_CH);
	    outb(ioport, 0x0B);
	    outb(ioport + 1, X86_CL);
	}
	break;

    case 0x02:
	/* Set Cursor Position                                */
	/* Enter:  BH = display page number                   */
	/*         DH = row                                   */
	/*         DL = column                                */
	/* Leave:  Nothing                                    */
	/* Implemented                                        */
	{                                         /* Localise */
	    IOADDRESS ioport;
	    CARD16 offset;

	    MEM_WB(pInt, (X86_BH << 1) + 0x0450, X86_DL);
	    MEM_WB(pInt, (X86_BH << 1) + 0x0451, X86_DH);

	    if (X86_BH != MEM_RB(pInt, 0x0462))
		break;

	    offset = (X86_DH * MEM_RW(pInt, 0x044A)) + X86_DL;
	    offset += MEM_RW(pInt, 0x044E) << 1;

	    ioport = MEM_RW(pInt, 0x0463) + pInt->ioBase;
	    outb(ioport, 0x0E);
	    outb(ioport + 1, offset >> 8);
	    outb(ioport, 0x0F);
	    outb(ioport + 1, offset & 0xFF);
	}
	break;

    case 0x03:
	/* Get Cursor Position                                */
	/* Enter:  BH = display page number                   */
	/* Leave:  CH = starting line for cursor              */
	/*         CL = ending line for cursor                */
	/*         DH = row                                   */
	/*         DL = column                                */
	/* Implemented                                        */
	{                                         /* Localise */
	    X86_CL = MEM_RB(pInt, 0x0460);
	    X86_CH = MEM_RB(pInt, 0x0461);
	    X86_DL = MEM_RB(pInt, (X86_BH << 1) + 0x0450);
	    X86_DH = MEM_RB(pInt, (X86_BH << 1) + 0x0451);
	}
	break;

    case 0x04:
	/* Get Light Pen Position                             */
	/* Enter:  Nothing                                    */
	/* Leave:  AH = 0x01 (down/triggered) or 0x00 (not)   */
	/*         BX = pixel column                          */
	/*         CX = pixel row                             */
	/*         DH = character row                         */
	/*         DL = character column                      */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x04) -- Get Light Pen Position\n", pInt->num);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	    X86_AH = X86_BX = X86_CX = X86_DX = 0;
	}
	break;

    case 0x05:
	/* Set Display Page                                   */
	/* Enter:  AL = display page number                   */
	/* Leave:  Nothing                                    */
	/* Implemented                                        */
	{                                         /* Localise */
	    IOADDRESS ioport = MEM_RW(pInt, 0x0463) + pInt->ioBase;
	    CARD16 start;
	    CARD8 x, y;

	    /* Calculate new start address */
	    MEM_WB(pInt, 0x0462, X86_AL);
	    start = X86_AL * MEM_RW(pInt, 0x044C);
	    MEM_WW(pInt, 0x044E, start);
	    start <<= 1;

	    /* Update start address */
	    outb(ioport, 0x0C);
	    outb(ioport + 1, start >> 8);
	    outb(ioport, 0x0D);
	    outb(ioport + 1, start & 0xFF);

	    /* Switch cursor position */
	    y = MEM_RB(pInt, (X86_AL << 1) + 0x0450);
	    x = MEM_RB(pInt, (X86_AL << 1) + 0x0451);
	    start += (y * MEM_RW(pInt, 0x044A)) + x;

	    /* Update cursor position */
	    outb(ioport, 0x0E);
	    outb(ioport + 1, start >> 8);
	    outb(ioport, 0x0F);
	    outb(ioport + 1, start & 0xFF);
	}
	break;

    case 0x06:
	/* Initialise or Scroll Window Up                     */
	/* Enter:  AL = lines to scroll up                    */
	/*         BH = attribute for blank                   */
	/*         CH = upper y of window                     */
	/*         CL = left x of window                      */
	/*         DH = lower y of window                     */
	/*         DL = right x of window                     */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x06) -- Initialise or Scroll Window Up\n",
		pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		" AL=0x%2.2x, BH=0x%2.2x,"
		" CH=0x%2.2x, CL=0x%2.2x, DH=0x%2.2x, DL=0x%2.2x\n",
		X86_AL, X86_BH, X86_CH, X86_CL, X86_DH, X86_DL);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    case 0x07:
	/* Initialise or Scroll Window Down                   */
	/* Enter:  AL = lines to scroll down                  */
	/*         BH = attribute for blank                   */
	/*         CH = upper y of window                     */
	/*         CL = left x of window                      */
	/*         DH = lower y of window                     */
	/*         DL = right x of window                     */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x07) -- Initialise or Scroll Window Down\n",
		pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		" AL=0x%2.2x, BH=0x%2.2x,"
		" CH=0x%2.2x, CL=0x%2.2x, DH=0x%2.2x, DL=0x%2.2x\n",
		X86_AL, X86_BH, X86_CH, X86_CL, X86_DH, X86_DL);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    case 0x08:
	/* Read Character and Attribute at Cursor             */
	/* Enter:  BH = display page number                   */
	/* Leave:  AH = attribute                             */
	/*         AL = character                             */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x08) -- Read Character and Attribute at"
		" Cursor\n", pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"BH=0x%2.2x\n", X86_BH);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	    X86_AX = 0;
	}
	break;

    case 0x09:
	/* Write Character and Attribute at Cursor            */
	/* Enter:  AL = character                             */
	/*         BH = display page number                   */
	/*         BL = attribute (text) or colour (graphics) */
	/*         CX = replication count                     */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x09) -- Write Character and Attribute at"
		" Cursor\n", pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"AL=0x%2.2x, BH=0x%2.2x, BL=0x%2.2x, CX=0x%4.4x\n",
		X86_AL, X86_BH, X86_BL, X86_CX);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    case 0x0a:
	/* Write Character at Cursor                          */
	/* Enter:  AL = character                             */
	/*         BH = display page number                   */
	/*         BL = colour                                */
	/*         CX = replication count                     */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x0A) -- Write Character at Cursor\n",
		pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"AL=0x%2.2x, BH=0x%2.2x, BL=0x%2.2x, CX=0x%4.4x\n",
		X86_AL, X86_BH, X86_BL, X86_CX);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    case 0x0b:
	/* Set Palette, Background or Border                  */
	/* Enter:  BH = 0x00 or 0x01                          */
	/*         BL = colour or palette (respectively)      */
	/* Leave:  Nothing                                    */
	/* Implemented                                        */
	{                                         /* Localise */
	    IOADDRESS ioport = MEM_RW(pInt, 0x0463) + 5 + pInt->ioBase;
	    CARD8 cgacolour = MEM_RB(pInt, 0x0466);

	    if (X86_BH) {
		cgacolour &= 0xDF;
		cgacolour |= (X86_BL & 0x01) << 5;
	    } else {
		cgacolour &= 0xE0;
		cgacolour |= X86_BL & 0x1F;
	    }

	    MEM_WB(pInt, 0x0466, cgacolour);
	    outb(ioport, cgacolour);
	}
	break;

    case 0x0c:
	/* Write Graphics Pixel                               */
	/* Enter:  AL = pixel value                           */
	/*         BH = display page number                   */
	/*         CX = column                                */
	/*         DX = row                                   */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x0C) -- Write Graphics Pixel\n", pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"AL=0x%2.2x, BH=0x%2.2x, CX=0x%4.4x, DX=0x%4.4x\n",
		X86_AL, X86_BH, X86_CX, X86_DX);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    case 0x0d:
	/* Read Graphics Pixel                                */
	/* Enter:  BH = display page number                   */
	/*         CX = column                                */
	/*         DX = row                                   */
	/* Leave:  AL = pixel value                           */
	/* Not Implemented                                    */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x0D) -- Read Graphics Pixel\n", pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"BH=0x%2.2x, CX=0x%4.4x, DX=0x%4.4x\n",
		X86_BH, X86_CX, X86_DX);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	    X86_AL = 0;
	}
	break;

    case 0x0e:
	/* Write Character in Teletype Mode                   */
	/* Enter:  AL = character                             */
	/*         BH = display page number                   */
	/*         BL = foreground colour                     */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	/* WARNING:  Emulation of BEL characters will require */
	/*           emulation of RTC and PC speaker I/O.     */
	/*           Also, this recurses through int 0x10     */
	/*           which might or might not have been       */
	/*           installed yet.                           */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x0E) -- Write Character in Teletype Mode\n",
		pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"AL=0x%2.2x, BH=0x%2.2x, BL=0x%2.2x\n",
		X86_AL, X86_BH, X86_BL);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    case 0x0f:
	/* Get Video Mode                                     */
	/* Enter:  Nothing                                    */
	/* Leave:  AH = number of columns                     */
	/*         AL = video mode number                     */
	/*         BH = display page number                   */
	/* Implemented                                        */
	{                                         /* Localise */
	    X86_AH = MEM_RW(pInt, 0x044A);
	    X86_AL = MEM_RB(pInt, 0x0449);
	    X86_BH = MEM_RB(pInt, 0x0462);
	}
	break;

    case 0x10:
	/* Colour Control (subfunction in AL)                 */
	/* Enter:  Various                                    */
	/* Leave:  Various                                    */
	/* Ignored                                            */
	break;

    case 0x11:
	/* Font Control (subfunction in AL)                   */
	/* Enter:  Various                                    */
	/* Leave:  Various                                    */
	/* Ignored                                            */
	break;

    case 0x12:
	/* Miscellaneous (subfunction in BL)                  */
	/* Enter:  Various                                    */
	/* Leave:  Various                                    */
	/* Ignored.  Previous code here optionally allowed    */
	/* the enabling and disabling of VGA, but no system   */
	/* BIOS I've come across actually implements it.      */
	break;

    case 0x13:
	/* Write String in Teletype Mode                      */
	/* Enter:  AL = write mode                            */
	/*         BL = attribute (if (AL & 0x02) == 0)       */
	/*         CX = string length                         */
	/*         DH = row                                   */
	/*         DL = column                                */
	/*         ES:BP = string segment:offset              */
	/* Leave:  Nothing                                    */
	/* Not Implemented                                    */
	/* WARNING:  Emulation of BEL characters will require */
	/*           emulation of RTC and PC speaker I/O.     */
	/*           Also, this recurses through int 0x10     */
	/*           which might or might not have been       */
	/*           installed yet.                           */
	{                                         /* Localise */
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
		"int 0x%2.2x(AH=0x13) -- Write String in Teletype Mode\n",
		pInt->num);
	    xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 3,
		"AL=0x%2.2x, BL=0x%2.2x, CX=0x%4.4x,"
		" DH=0x%2.2x, DL=0x%2.2x, ES:BP=0x%4.4x:0x%4.4x\n",
		X86_AL, X86_BL, X86_CX, X86_DH, X86_DL, X86_ES, X86_BP);
	    if (xf86GetVerbosity() > 3) {
		dump_registers(pInt);
		stack_trace(pInt);
	    }
	}
	break;

    default:
	/* Various extensions                                 */
	/* Enter:  Various                                    */
	/* Leave:  Various                                    */
	/* Ignored                                            */
	break;
    }

    return 1;
}
#endif

#define SUCCESSFUL              0x00
#define DEVICE_NOT_FOUND        0x86
#define BAD_REGISTER_NUMBER     0x87

static int
int1A_handler(xf86Int10InfoPtr pInt)
{
    PCITAG tag;
    pciVideoPtr pvp;

    if (!(pvp = xf86GetPciInfoForEntity(pInt->entityIndex)))
	return 0; /* oops */

#ifdef PRINT_INT
    ErrorF("int 0x1a: ax=0x%x bx=0x%x cx=0x%x dx=0x%x di=0x%x es=0x%x\n",
	    X86_EAX, X86_EBX, X86_ECX, X86_EDX, X86_EDI, X86_ESI);
#endif
    switch (X86_AX) {
    case 0xb101:
	X86_EAX &= 0xFF00;   /* no config space/special cycle support */
	X86_EDX = 0x20494350; /* " ICP" */
	X86_EBX = 0x0210;    /* Version 2.10 */
	X86_ECX &= 0xFF00;
	X86_ECX |= (pciNumBuses & 0xFF);   /* Max bus number in system */
	X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
#ifdef PRINT_INT
	ErrorF("ax=0x%x dx=0x%x bx=0x%x cx=0x%x flags=0x%x\n",
		 X86_EAX, X86_EDX, X86_EBX, X86_ECX, X86_EFLAGS);
#endif
	return 1;
    case 0xb102:
	if (X86_DX == pvp->vendor && X86_CX == pvp->chipType && X86_ESI == 0) {
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	    X86_EBX = pciSlotBX(pvp);
	}
#ifdef SHOW_ALL_DEVICES
	else
	if ((pvp = xf86FindPciDeviceVendor(X86_EDX, X86_ECX, X86_ESI, pvp))) {
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	    X86_EBX = pciSlotBX(pvp);
	}
#endif
	else {
	    X86_EAX = X86_AL | (DEVICE_NOT_FOUND << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x bx=0x%x flags=0x%x\n", X86_EAX, X86_EBX, X86_EFLAGS);
#endif
	return 1;
    case 0xb103:
	if (X86_CL == pvp->interface &&
	    X86_CH == pvp->subclass &&
	    ((X86_ECX & 0xFFFF0000) >> 16) == pvp->class) {
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EBX = pciSlotBX(pvp);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	}
#ifdef SHOW_ALL_DEVICES
	else if ((pvp = xf86FindPciClass(X86_CL, X86_CH,
					 (X86_ECX & 0xffff0000) >> 16,
					 X86_ESI, pvp))) {
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	    X86_EBX = pciSlotBX(pvp);
	}
#endif
	else {
	    X86_EAX = X86_AL | (DEVICE_NOT_FOUND << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX, X86_EFLAGS);
#endif
	return 1;
    case 0xb108:
	if ((tag = findPci(pInt, X86_EBX)) != PCI_NOT_FOUND) {
	    X86_CL = pciReadByte(tag, X86_EDI);
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = X86_AL | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x cx=0x%x flags=0x%x\n", X86_EAX, X86_ECX, X86_EFLAGS);
#endif
	return 1;
    case 0xb109:
	if ((tag = findPci(pInt, X86_EBX)) != PCI_NOT_FOUND) {
	    X86_CX = pciReadWord(tag, X86_EDI);
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = X86_AL | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x cx=0x%x flags=0x%x\n", X86_EAX, X86_ECX, X86_EFLAGS);
#endif
	return 1;
    case 0xb10a:
	if ((tag = findPci(pInt, X86_EBX)) != PCI_NOT_FOUND) {
	    X86_ECX = pciReadLong(tag, X86_EDI);
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = X86_AL | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x cx=0x%x flags=0x%x\n", X86_EAX, X86_ECX, X86_EFLAGS);
#endif
	return 1;
    case 0xb10b:
	if ((tag = findPci(pInt, X86_EBX)) != PCI_NOT_FOUND) {
	    pciWriteByte(tag, X86_EDI, X86_CL);
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = X86_AL | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX, X86_EFLAGS);
#endif
	return 1;
    case 0xb10c:
	if ((tag = findPci(pInt, X86_EBX)) != PCI_NOT_FOUND) {
	    pciWriteWord(tag, X86_EDI, X86_CX);
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = X86_AL | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX, X86_EFLAGS);
#endif
	return 1;
    case 0xb10d:
	if ((tag = findPci(pInt, X86_EBX)) != PCI_NOT_FOUND) {
	    pciWriteLong(tag, X86_EDI, X86_ECX);
	    X86_EAX = X86_AL | (SUCCESSFUL << 8);
	    X86_EFLAGS &= ~((unsigned long)0x01); /* clear carry flag */
	} else {
	    X86_EAX = X86_AL | (BAD_REGISTER_NUMBER << 8);
	    X86_EFLAGS |= ((unsigned long)0x01); /* set carry flag */
	}
#ifdef PRINT_INT
	ErrorF("ax=0x%x flags=0x%x\n", X86_EAX, X86_EFLAGS);
#endif
	return 1;
    default:
	xf86DrvMsgVerb(pInt->scrnIndex, X_NOT_IMPLEMENTED, 2,
	    "int 0x1a subfunction\n");
	dump_registers(pInt);
	if (xf86GetVerbosity() > 3)
	    stack_trace(pInt);
	return 0;
    }
}

static PCITAG
findPci(xf86Int10InfoPtr pInt, unsigned short bx)
{
    int bus = ((pInt->Tag >> 16) & ~0x00FF) | ((bx >> 8) & 0x00FF);
    int dev = (bx >> 3) & 0x1F;
    int func = bx & 0x7;
    if (xf86IsPciDevPresent(bus, dev, func))
	return pciTag(bus, dev, func);
    return PCI_NOT_FOUND;
}

static CARD32
pciSlotBX(pciVideoPtr pvp)
{
    return ((pvp->bus << 8) & 0x00FF00) | (pvp->device << 3) | (pvp->func);
}

/*
 * handle initialization
 */
static int
intE6_handler(xf86Int10InfoPtr pInt)
{
    pciVideoPtr pvp;

    if ((pvp = xf86GetPciInfoForEntity(pInt->entityIndex)))
	X86_AX = (pvp->bus << 8) | (pvp->device << 3) | (pvp->func & 0x7);
    pushw(pInt, X86_CS);
    pushw(pInt, X86_IP);
    X86_CS = pInt->BIOSseg;
    X86_EIP = 0x0003;
    X86_ES = 0;                  /* standard pc es */
    return 1;
}
