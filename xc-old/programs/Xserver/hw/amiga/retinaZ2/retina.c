/*
 * Graphics routines for the Retina board, 
 *   using the NCR 77C22E+ VGA controller.
 */

/***********************************************************

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

Author: Andy Heffernan

Most of this code is extracted from the NetBSD driver written
by Lutz Vieweg and embellished by Markus Wild.  None of this
would have been possible without the efforts of these two.

******************************************************************/

#include "amiga.h"
#include "retina.h"

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

void *vgaReadBottom, *vgaReadTop;
void *vgaWriteBottom, *vgaWriteTop;
Bool vgaReadFlag;
Bool vgaWriteFlag;
u_long VGABASE;
void *vgaregs;
u_long retina_writeseg, retina_readseg, retina_saveseg;

char *mondeffile;

void
retina_clear_screen(u_char *buffer, struct grfinfo *gi)
{
    int bufsize, i;

    bufsize = gi->gd_fbwidth * gi->gd_fbheight;
    for (i = 0; bufsize; i++) {
	int fillsize = (bufsize > gi->gd_fbsize) ? gi->gd_fbsize : bufsize;

	retina_setbank(i);
	memset((void *) buffer, 1, fillsize);	/* will be black under X */
	bufsize -= fillsize;
    }
}

/* Right-justified mask 'l' bits long */
#define MASK(l)		(~0 & ((1 << (l)) - 1))
/* How much to left-shift to get from bit 'n' to 'd' (or zero if 'n' is to left of 'd' */
#define LEFTSHIFT(n,d)	(((d) > (n)) ? ((d) - (n)) : 0)
/* How much to right-shift to get from bit 'n' to 'd' (or zero if 'n' is to right of 'd' */
#define RIGHTSHIFT(n,d)	(((d) > (n)) ? 0 : ((n) - (d)))
/* Return 'l' bit(s) of 'x' at bit 'n' moved to bit 'd' */
#define BITS(x,n,l,d)	(((((unsigned long) (x)) & (MASK(l) << (n))) << \
			 LEFTSHIFT(n,d)) >> RIGHTSHIFT(n,d))

int
retina_load_mon(struct MonDef *md, struct grfinfo *gi)
{
    short FW, clksel, HDE, VDE;
    static const long FQTab[16] = {
	25175000,  28322000,  36000000,  65000000,
	44900000,  50000000,  80000000,  75000000,
	56644000,  63000000,  72000000, 130000000,
	90000000, 100000000, 110000000, 120000000
    };

    for (clksel = 15; clksel; clksel--) {
	if (FQTab[clksel] == md->FQ)
	    break;
    }
    if (clksel < 0)
	return -1;

    FW = 0;
    HDE = (md->MW + 3) / 4;
    VDE = md->MH - 1;

    /* Enable extension registers. */
    WSeq(vgaregs, SEQ_ID_EXTENDED_ENABLE, 0x05);
    WSeq(vgaregs, SEQ_ID_RESET, 0x01);  

    /* Set font width + rest of clocks. */
    WSeq(vgaregs, SEQ_ID_EXT_CLOCK_MODE, (BITS(clksel, 2, 1, 6) |
					  BITS(0x30,   4, 2, 4) |
					  BITS(FW,     0, 4, 0)));
    /* Another clock bit, plus hw stuff. */
    WSeq(vgaregs, SEQ_ID_MISC_FEATURE_SEL, (BITS(0xf0,   4, 4, 4) |
					    BITS(clksel, 3, 1, 3) |
					    BITS(0x04,   2, 1, 2)));

    /* Program the clock oscillator. */
    vgaw(vgaregs, GREG_MISC_OUTPUT_W, (BITS(0xe0,   4, 4, 4) |
				       BITS(clksel, 0, 2, 2) |
				       BITS(0x03,   0, 2, 0)));
    vgaw(vgaregs, GREG_FEATURE_CONTROL_W, 0x00);
	
    WSeq(vgaregs, SEQ_ID_CLOCKING_MODE, (BITS(0x20,    5,           1, 5) |
					 BITS(md->FLG, MDB_CLKDIV2, 1, 3) |
					 BITS(0x01,    0,           1, 0)));
    WSeq(vgaregs, SEQ_ID_MAP_MASK, 0x0f);  
    WSeq(vgaregs, SEQ_ID_CHAR_MAP_SELECT, 0x00);

    /* Odd/even write select + extended memory. */
    WSeq(vgaregs, SEQ_ID_MEMORY_MODE, 0x06);
    WSeq(vgaregs, SEQ_ID_RESET, 0x03);

    /* Monochrome cursor. */
    WSeq(vgaregs, SEQ_ID_CURSOR_CONTROL, 0x00);

    /* Bank0. */
    WSeq(vgaregs, SEQ_ID_PRIM_HOST_OFF_HI, 0x00);
    WSeq(vgaregs, SEQ_ID_PRIM_HOST_OFF_LO, 0x00);
    WSeq(vgaregs, SEQ_ID_DISP_OFF_HI, 0x00);
    WSeq(vgaregs, SEQ_ID_DISP_OFF_LO, 0x00);
    WSeq(vgaregs, SEQ_ID_SEC_HOST_OFF_HI, 0x00);
    WSeq(vgaregs, SEQ_ID_SEC_HOST_OFF_LO, 0x00);

    /* 1M-chips + ena SEC + ena EMem + rw PrimA0/rw Sec/B0. */
    WSeq(vgaregs, SEQ_ID_EXTENDED_MEM_ENA, (BITS(0x40, 6, 1, 6) |
					    BITS(0x10, 4, 1, 4) |
					    BITS(0x4,  2, 1, 2) |
					    BITS(0x3,  0, 2, 0)));
    WSeq(vgaregs, SEQ_ID_EXT_VIDEO_ADDR, 0x02);

    /* 256bit gfx format. */
    WSeq(vgaregs, SEQ_ID_EXT_PIXEL_CNTL, 0x01);

    /* AT-interface. */
    WSeq(vgaregs, SEQ_ID_BUS_WIDTH_FEEDB, 0x06);

    /* See fg/bg color expansion. */
    WSeq(vgaregs, SEQ_ID_COLOR_EXP_WFG, 0x01);
    WSeq(vgaregs, SEQ_ID_COLOR_EXP_WBG, 0x00);
    WSeq(vgaregs, SEQ_ID_EXT_RW_CONTROL, 0x00);

    /* Don't tristate PCLK and PIX. */
    WSeq(vgaregs, SEQ_ID_COLOR_KEY_CNTL, 0x40);
    /* Reset CRC circuit. */
    WSeq(vgaregs, SEQ_ID_CRC_CONTROL, 0x00);
    /* Set RAS/CAS swap. */
    WSeq(vgaregs, SEQ_ID_PERF_SELECT, 0x20);
	
    WCrt(vgaregs, CRT_ID_END_VER_RETR, (BITS(0x20,    5, 1, 5) |
					BITS(md->VSE, 0, 4, 0)));
    WCrt(vgaregs, CRT_ID_HOR_TOTAL, md->HT & 0xff);
    WCrt(vgaregs, CRT_ID_HOR_DISP_ENA_END, (HDE-1)  & 0xff);
    WCrt(vgaregs, CRT_ID_START_HOR_BLANK, md->HBS  & 0xff);
    WCrt(vgaregs, CRT_ID_END_HOR_BLANK, (BITS(0x80,    7, 1, 7) |
					 BITS(md->HBE, 0, 5, 0)));

    WCrt(vgaregs, CRT_ID_START_HOR_RETR, BITS(md->HSS, 0, 8, 0));
    WCrt(vgaregs, CRT_ID_END_HOR_RETR, (BITS(md->HSE, 0, 5, 0) |
					BITS(md->HBE, 5, 1, 7)));
    WCrt(vgaregs, CRT_ID_VER_TOTAL, BITS(md->VT, 0, 8, 0));
    WCrt(vgaregs, CRT_ID_OVERFLOW, (BITS(md->VSS, 9, 1, 7) |
				    BITS(VDE,     9, 1, 6) |
				    BITS(md->VT,  9, 1, 5) |
				    BITS(0x10,    4, 1, 4) |
				    BITS(md->VBS, 8, 1, 3) |
				    BITS(md->VSS, 8, 1, 2) |
				    BITS(VDE,     8, 1, 1) |
				    BITS(md->VT,  8, 1, 0)));
    WCrt(vgaregs, CRT_ID_PRESET_ROW_SCAN, 0x00);
	
    WCrt(vgaregs, CRT_ID_MAX_SCAN_LINE, (BITS(md->FLG, MDB_DBL, 1, 7) |
					 BITS(0x40,    6,       1, 6) |
					 BITS(md->VBS, 9,       1, 5) |
					 BITS(0,       0,       5, 0)));

    WCrt(vgaregs, CRT_ID_CURSOR_START, BITS(md->FY, 0, 5, 0) - 2);
    WCrt(vgaregs, CRT_ID_CURSOR_END, BITS(md->FY, 0, 5, 0) - 1);

    WCrt(vgaregs, CRT_ID_START_ADDR_HIGH, 0x00);
    WCrt(vgaregs, CRT_ID_START_ADDR_LOW, 0x00);
    WCrt(vgaregs, CRT_ID_CURSOR_LOC_HIGH, 0x00);
    WCrt(vgaregs, CRT_ID_CURSOR_LOC_LOW, 0x00);

    WCrt(vgaregs, CRT_ID_START_VER_RETR, BITS(md->VSS, 0, 8, 0));
    WCrt(vgaregs, CRT_ID_END_VER_RETR, (BITS(0x80,    7, 1, 7) |
					BITS(0,       0, 1, 6) |
					BITS(0x20,    5, 1, 5) |
					BITS(md->VSE, 0, 4, 0)));
    WCrt(vgaregs, CRT_ID_VER_DISP_ENA_END, BITS(VDE, 0, 8, 0));
    WCrt(vgaregs, CRT_ID_OFFSET, BITS(md->TX, 3, 8, 0));

    WCrt(vgaregs, CRT_ID_UNDERLINE_LOC, (md->FY - 1) & 0x1f);
    WCrt(vgaregs, CRT_ID_START_VER_BLANK, md->VBS & 0xff);
    WCrt(vgaregs, CRT_ID_END_VER_BLANK, md->VBE & 0xff);
    /* Byte mode + wrap + select row scan counter + cms. */
    WCrt(vgaregs, CRT_ID_MODE_CONTROL, 0xe3);
    WCrt(vgaregs, CRT_ID_LINE_COMPARE, 0xff);

    /* enable extended end bits + those bits */
    WCrt(vgaregs, CRT_ID_EXT_HOR_TIMING1, (BITS(0x20,    5,        1, 5) |
					   BITS(md->FLG, MDB_LACE, 1, 4) |
					   BITS(md->HSS, 8,        1, 3) |
					   BITS(md->HBS, 8,        1, 2) |
					   BITS(HDE - 1, 8,        1, 1) |
					   BITS(md->HT,  8,        1, 0)));
	             
    WCrt(vgaregs, CRT_ID_EXT_START_ADDR, (BITS(md->TX, 11, 1, 4)));
	
    WCrt(vgaregs, CRT_ID_EXT_HOR_TIMING2, (BITS(md->HSE, 5, 2, 6) |
					   BITS(md->HBE, 6, 2, 4) |
					   BITS(md->HSS, 9, 1, 3) |
					   BITS(md->HBS, 9, 1, 2) |
					   BITS(HDE - 1, 9, 1, 1) |
					   BITS(md->HT,  9, 1, 0)));

    WCrt(vgaregs, CRT_ID_EXT_VER_TIMING, (BITS(md->VSE, 4, 1, 7) |
					  BITS(md->VBE, 8, 2, 5) |
					  BITS(0x10,    4, 1, 4) |
					  BITS(md->VSS,10, 1, 3) |
					  BITS(md->VBS,10, 1, 2) |
					  BITS(VDE,    10, 1, 1) |
					  BITS(md->VT, 10, 1, 0)));

    WGfx(vgaregs, GCT_ID_SET_RESET, 0x00);
    WGfx(vgaregs, GCT_ID_ENABLE_SET_RESET, 0x00);
    WGfx(vgaregs, GCT_ID_COLOR_COMPARE, 0x00);
    WGfx(vgaregs, GCT_ID_DATA_ROTATE, 0x00);
    WGfx(vgaregs, GCT_ID_READ_MAP_SELECT, 0x00);
    WGfx(vgaregs, GCT_ID_GRAPHICS_MODE, 0x00);
    WGfx(vgaregs, GCT_ID_MISC, 0x05);
    WGfx(vgaregs, GCT_ID_COLOR_XCARE, 0xff);
    WGfx(vgaregs, GCT_ID_BITMASK, 0xff);

    /* Reset the Attribute Controller flipflop. */
    vgar(vgaregs, GREG_STATUS1_R);
    WAttr(vgaregs, ACT_ID_PALETTE0, 0x00);
    WAttr(vgaregs, ACT_ID_PALETTE1, 0x01);
    WAttr(vgaregs, ACT_ID_PALETTE2, 0x02);
    WAttr(vgaregs, ACT_ID_PALETTE3, 0x03);
    WAttr(vgaregs, ACT_ID_PALETTE4, 0x04);
    WAttr(vgaregs, ACT_ID_PALETTE5, 0x05);
    WAttr(vgaregs, ACT_ID_PALETTE6, 0x06);
    WAttr(vgaregs, ACT_ID_PALETTE7, 0x07);
    WAttr(vgaregs, ACT_ID_PALETTE8, 0x08);
    WAttr(vgaregs, ACT_ID_PALETTE9, 0x09);
    WAttr(vgaregs, ACT_ID_PALETTE10, 0x0a);
    WAttr(vgaregs, ACT_ID_PALETTE11, 0x0b);
    WAttr(vgaregs, ACT_ID_PALETTE12, 0x0c);
    WAttr(vgaregs, ACT_ID_PALETTE13, 0x0d);
    WAttr(vgaregs, ACT_ID_PALETTE14, 0x0e);
    WAttr(vgaregs, ACT_ID_PALETTE15, 0x0f);

    vgar(vgaregs, GREG_STATUS1_R);
    WAttr(vgaregs, ACT_ID_ATTR_MODE_CNTL, 0x09);

    WAttr(vgaregs, ACT_ID_OVERSCAN_COLOR, 0x01);
    WAttr(vgaregs, ACT_ID_COLOR_PLANE_ENA, 0x0f);
    WAttr(vgaregs, ACT_ID_HOR_PEL_PANNING, 0x00);
    WAttr(vgaregs, ACT_ID_COLOR_SELECT, 0x00);

    vgar(vgaregs, GREG_STATUS1_R);
    /* I have *NO* idea what strobing reg-0x20 might do... */
    vgaw(vgaregs, ACT_ADDRESS_W, 0x20);

    WCrt(vgaregs, CRT_ID_MAX_SCAN_LINE, (BITS(md->FLG, MDB_DBL, 1, 7) |
					 BITS(0x40,    6,       1, 6) |
					 BITS(md->VBS, 9,       1, 5) |
					 BITS(0,       0,       5, 0)));

    /* Now it's time for guessing... */

    vgaw(vgaregs, VDAC_REG_D, 0x02);
	
    /* If this does what I think it does, it selects DAC 
       register 0, and writes the palette in subsequent
       registers, thus it works similar to the WD33C93 
       select/data mechanism. */
    vgaw(vgaregs, VDAC_REG_SELECT, 0x00);

    /* Load white into color register 0. */
    vgaw(vgaregs, VDAC_REG_DATA, 255);
    vgaw(vgaregs, VDAC_REG_DATA, 255);
    vgaw(vgaregs, VDAC_REG_DATA, 255);
    /* Load black into color register 1. */
    vgaw(vgaregs, VDAC_REG_DATA, 0);
    vgaw(vgaregs, VDAC_REG_DATA, 0);
    vgaw(vgaregs, VDAC_REG_DATA, 0);

    /* Select map 0. */
    WGfx(vgaregs, GCT_ID_READ_MAP_SELECT, 0);
    /* Allow writes into all maps. */
    WSeq(vgaregs, SEQ_ID_MAP_MASK, 0x0f);

    /* Select extended chain4 addressing:
       !A0/!A1	map 0	character to be displayed
       !A1/ A1	map 1	attribute of that character
       A0/!A1	map 2	not used (masked out, ignored)
       A0/ A1 	map 3	not used (masked out, ignored) */
    WSeq(vgaregs, SEQ_ID_EXT_VIDEO_ADDR, RSeq(vgaregs, SEQ_ID_EXT_VIDEO_ADDR) | 0x02);

    gi->gd_regaddr = NULL;
    gi->gd_regsize = 64*1024;

    gi->gd_fbaddr = NULL;
    gi->gd_fbsize = 64*1024;	/* larger, but that's what's mappable */
  
    gi->gd_colors = 1 << md->DEP;
    gi->gd_planes = md->DEP;
  
    gi->gd_fbwidth = md->MW;
    gi->gd_fbheight = md->MH;
    gi->gd_fbx = 0;
    gi->gd_fby = 0;
    gi->gd_dwidth = md->TX * md->FX;
    gi->gd_dheight = md->TY * md->FY;
    gi->gd_dx = 0;
    gi->gd_dy = 0;
  
    return 0;
}

int
retina_read_mondef(char *filename, struct MonDef *md)
{
    FILE *f;
    char buf[80];
    unsigned long ID, MD;
    struct var {
	char *token;
	void *value;
	int size;
    } vars[] = {
	"ID", &ID, sizeof(ID),
	"FQ", &md->FQ, sizeof(md->FQ),
	"MD", &MD, sizeof(MD),
	"FLG", &md->FLG, sizeof(md->FLG),
	"SW", &md->TX, sizeof(md->TX),
	"SH", &md->TY, sizeof(md->TY),
	"MW", &md->MW, sizeof(md->MW),
	"MH", &md->MH, sizeof(md->MH),
	"HBS", &md->HBS, sizeof(md->HBS),
	"HSS", &md->HSS, sizeof(md->HSS),
	"HSE", &md->HSE, sizeof(md->HSE),
	"HBE", &md->HBE, sizeof(md->HBE),
	"HT", &md->HT, sizeof(md->HT),
	"VBS", &md->VBS, sizeof(md->VBS),
	"VSS", &md->VSS, sizeof(md->VSS),
	"VSE", &md->VSE, sizeof(md->VSE),
	"VBE", &md->VBE, sizeof(md->VBE),
	"VT", &md->VT, sizeof(md->VT),
	NULL, NULL
    };

    if ((f = fopen(filename, "r")) == NULL)
	return -1;

    bzero(md, sizeof(struct MonDef));
    md->DEP = 8;
    md->PAL = NULL;
    md->FY = 0; /* XXX */
    ID = MD = -1;

    while (fgets(buf, 80, f)) {
	char *bufp;
	struct var *var;

	if (buf[0] == ';' || buf[0] == '\n')
	    continue;

	bufp = strtok(buf, " \t\n");
	while (bufp) {
	    for (var = &vars[0]; var->token; var++) {
		if (strcmp(bufp, var->token) == 0) {
		    bufp = strtok(NULL, " \t\n");
		    if (var->size == sizeof(u_char))
			* (u_char *) var->value = atoi(bufp);
		    else if (var->size == sizeof(u_short))
			* (u_short *) var->value = atoi(bufp);
		    if (var->size == sizeof(u_long))
			* (u_long *) var->value = atoi(bufp);
		    break;
		}
	    }
	    bufp = strtok(NULL, " \t\n");
	}
    }

    fclose(f);
    return 0;
}

Bool
retina_setup(int fd, ScreenInfo *pScreenInfo, int index, int fbNum, int argc, char **argv)
{
    caddr_t mapaddr;
    struct grfinfo gi;
    int bufsize, i;
    char monfile[512];
    struct MonDef mondef;

    /*
     * If no mondef file given on the command line, use the default
     * "/usr/X11R6/lib/X11/RetinaMonDef".
     */
    if (mondeffile == NULL) {
	sprintf(monfile, "%s/%s", MONDEFDIR, DFLT_RETINA_MON_FILE);
	mondeffile = monfile;
    }

    /*
     * Read the contents of the monitor definition file into the Mondef struct.
     */
    if (retina_read_mondef(mondeffile, &mondef) == -1) {
	perror(mondeffile);
	return FALSE;
    }

    /*
     * Turn graphics mode on.
     * Get some information about the size of the frame buffer mappable.
     * Map the frame buffer and registers.
     */
    if (ioctl(fd, GRFIOCON, 0)) {
	perror("ioctl GRFIOCON");
	return FALSE;
    }

    if (ioctl(fd, GRFIOCGINFO, &gi)) {
	perror("ioctl GRFIOCGINFO");
	return FALSE;
    }

    if ((mapaddr = mmap(0, gi.gd_regsize + gi.gd_fbsize, PROT_READ | PROT_WRITE,
			MAP_FILE, fd, (off_t) 0)) == (caddr_t) -1) {
	perror("mmap");
	ioctl(fd, GRFIOCOFF, 0);
	return FALSE;
    }

    /*
     * Set up some globals.
     */
    vgaregs = mapaddr;
    VGABASE = (u_long) (mapaddr + gi.gd_regsize);
    vgaReadBottom = vgaWriteBottom = (void *) VGABASE;
    vgaReadTop = vgaWriteTop = (void *) (VGABASE + gi.gd_fbsize);
#if 0
    TRACE(("vgaReadBottom %08lx vgaWriteBottom %08lx vgaReadTop %08lx vgaWriteTop %08lx\n",
	   vgaReadBottom, vgaWriteBottom, vgaReadTop, vgaWriteTop));
#endif

    /*
     * Set up the card with the read-in monitor definition.
     */
    if (retina_load_mon(&mondef, &gi) == -1) {
	Error("Setting up retina");
	munmap(mapaddr, gi.gd_regsize + gi.gd_fbsize);
	ioctl(fd, GRFIOCOFF, 0);
	return FALSE;
    }

    /*
     * Clear the screen and then unblank it.
     */
    retina_clear_screen((u_char *) VGABASE, &gi);
#if 0
    retinaSaveScreen(NULL, SCREEN_SAVER_OFF);
#endif

    amigaFbs[index].fd = fd;
    amigaFbs[index].info = gi;
    amigaFbs[index].fb = (pointer) VGABASE;
    amigaFbs[index].EnterLeave = NULL;
#if 0
    amigaSupportsDepth8 = TRUE;
#endif

    return TRUE;
}

#ifndef WANT_INLINES
#define INLINE
#include "retina_inlines.c"
#endif
