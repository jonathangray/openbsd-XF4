/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_accel.c,v 1.1.2.1 1999/07/30 11:21:31 hohndel Exp $ */

/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */

/*
 * The accel file for the Savage driver.  
 * 
 * Created 20/03/97 by Sebastien Marineau
 * Revision: 
 *
 * Note: we use a few macros to query the state of the coprocessor. 
 * WaitIdle() waits until the GE is idle.
 * WaitIdleEmpty() waits until the GE is idle and its FIFO is empty.
 *       We should call this 
 *       before starting any GE functions to make sure that there are no
 *       framebuffer writes left in the FIFO. 
 */

#include <math.h>
#include "xf86.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"
#include "xf86xaa.h"
#include "xf86Priv.h"
#include "regs3sav.h"
#include "s3sav_driver.h"
#include "s3sav_rop.h"
#include "s3bci.h"
#include "miline.h"

extern S3VPRIV s3vPriv;

/* Globals used in driver */
extern pointer s3savMmioMem;
#ifdef __alpha__
extern pointer s3savMmioMemSparse;
#endif
static int s3DummyTransferArea;
static int s3LineHWClipSet = 0;

#define BCI_REPEAT 0
static unsigned int * bci_base;
static unsigned int   s3SavedBciCmd = 0;
static unsigned int   s3SavedGbdOffset = 0;
static unsigned int   s3SavedGbd = 0;
static unsigned int   s3SavedSbdOffset = 0;
static unsigned int   s3SavedSbd = 0;
static unsigned int   s3SavedFgColor = 0;
static unsigned int   s3SavedBgColor = 0;

/* These are variables which hold cached values for some virge registers.
 * The important thing to remember is that these registers must be always be 
 * set using the "caching" version of the macros.
 */
static unsigned int s3vCached_CMD_SET;
static unsigned int s3vCached_CLIP_LR;
static unsigned int s3vCached_CLIP_TB;
static unsigned int s3vCached_MONO_PATTERN0;
static unsigned int s3vCached_MONO_PATTERN1;
static unsigned int s3vCached_PAT_FGCLR;
static unsigned int s3vCached_PAT_BGCLR;
static unsigned int s3vCached_RWIDTH_HEIGHT;

/* Temporary to see if caching works */
static int s3vCacheHit = 0, s3vCacheMiss = 0;

/* Forward declaration of fucntions used in the driver */
void S3SAVAccelSync();
void S3SAVAccelInit();
void S3SAVSetupForScreenToScreenCopy();
void S3SAVSubsequentScreenToScreenCopy();
void S3SAVSetupForFillRectSolid();
void S3SAVSubsequentFillRectSolid();
void SavageSubsequentBresenhamLine();
void S3SAVSetupForScreenToScreenColorExpand();
void S3SAVSubsequentScreenToScreenColorExpand();
void S3SAVSetupForCPUToScreenColorExpand();
void S3SAVSubsequentCPUToScreenColorExpand();
void S3SAVSetupFor8x8PatternColorExpand();
void S3SAVSubsequent8x8PatternColorExpand();
void S3SAVSetupForFill8x8Pattern();
void S3SAVSubsequentFill8x8Pattern();
void S3SAVSubsequentTwoPointLine();
void S3SAVSetClippingRectangle();
void S3SAVSetGBD();
#include "s3bitmap.c"



void
S3SAVInitialize2DEngine()
{
    outw(vgaCRIndex, 0x0140);
    outw(vgaCRIndex, 0x0c31);

    /* Setup plane masks */
    S3_OUT32(0x8128, ~0); /* enable all write planes */
    S3_OUT32(0x812C, ~0); /* enable all read planes */
    S3_OUT16(0x8134, 0x27);
    S3_OUT16(0x8136, 0x07);

    switch( s3vPriv.chip ) {

    case S3_SAVAGE3D:
    case S3_SAVAGE_MX:
	/* Disable BCI */
	S3_OUT32(0x48C18, S3_IN32(0x48C18) & 0x3FF0);
	/* Setup BCI command overflow buffer */
	S3_OUT32(0x48C14, (s3vPriv.cobOffset >> 11) | (s3vPriv.cobIndex << 29));
	/* Program shadow status update. */
	S3_OUT32(0x48C10, 0x78207220);
	if( s3vPriv.ShadowPhysical )
	{
	    S3_OUT32(0x48C0C, s3vPriv.ShadowPhysical | 1 );
	    /* Enable BCI and command overflow buffer */
	    S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x0E);
	}
	else
	{
	    S3_OUT32(0x48C0C, 0);
	    /* Enable BCI and command overflow buffer */
	    S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x0C);
	}
	break;

    case S3_SAVAGE4:
    case S3_PROSAVAGE:
	/* Disable BCI */
	S3_OUT32(0x48C18, S3_IN32(0x48C18) & 0x3FF0);
	/* Program shadow status update */
	S3_OUT32(0x48C10, 0x00700040);
	if( s3vPriv.ShadowPhysical )
	{
	    S3_OUT32(0x48C0C, s3vPriv.ShadowPhysical | 1 );
	    /* Enable BCI without the COB */
	    S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x0a);
	}
	else
	{
	    S3_OUT32(0x48C0C, 0);
	    /* Enable BCI without the COB */
	    S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x08);
	}
	break;

    case S3_SAVAGE2000:
	/* Disable BCI */
	S3_OUT32(0x48C18, 0);
	/* Setup BCI command overflow buffer */
	S3_OUT32(0x48C18, (s3vPriv.cobOffset >> 7) | (s3vPriv.cobIndex));
	if( s3vPriv.ShadowPhysical )
	{
	    /* Set shadow update threshholds. */
	    S3_OUT32(0x48C10, 0x6090 );
	    S3_OUT32(0x48C14, 0x70A8 );
	    /* Enable shadow status update */
	    S3_OUT32(0x48A30, s3vPriv.ShadowPhysical );
	    /* Enable BCI, command overflow buffer and shadow status. */
	    S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x00380000 );
	}
	else
	{
	    /* Disable shadow status update */
	    S3_OUT32(0x48A30, 0);
	    /* Enable BCI and command overflow buffer */
	    S3_OUT32(0x48C18, S3_IN32(0x48C18) | 0x00280000 );
	}
	break;
    }

    /* Use and set global bitmap descriptor. */

    /* For reasons I do not fully understand yet, on the Savage4, the */
    /* write to the GBD register, MM816C, does not "take" at this time. */
    /* Only the low-order byte is acknowledged, resulting in an incorrect */
    /* stride.  Writing the register later, after the mode switch, works */
    /* correctly.  This needs to get resolved. */

    S3SAVSetGBD();
} 


void
S3SAVSetGBD( )
{
    s3SavedGbd = 1 | 8 | BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(s3SavedGbd, vgaBitsPerPixel);
    BCI_BD_SET_STRIDE(s3SavedGbd, s3vPriv.Width);

    /* Turn on 16-bit register access. */

    outb(vgaCRIndex, 0x31);
    outb(vgaCRReg, 0x0c);

    /* Set stride to use GBD. */

    outb(vgaCRIndex, 0x50);
    outb(vgaCRReg, inb(vgaCRReg) | 0xC1);

    /* Enable 2D engine. */

    outw(vgaCRIndex, 0x0140);

    /* Now set the GBD and SBDs. */

    S3_OUT32(0x8168,0);
    S3_OUT32(0x816C,s3SavedGbd);
    S3_OUT32(0x8170,0);
    S3_OUT32(0x8174,s3SavedGbd);
    S3_OUT32(0x8178,0);
    S3_OUT32(0x817C,s3SavedGbd);

    S3_OUT32(0x81C8, s3vPriv.Width << 4);
    S3_OUT32(0x81D8, s3vPriv.Width << 4);
}

/* Routines for debugging. */
unsigned long
writedw( unsigned long addr, unsigned long value )
{
  S3_OUT32( addr, value );
  return S3_IN32( addr );
}

unsigned long
readdw( unsigned long addr )
{
  return S3_IN32( addr );
}

unsigned long
readfb( unsigned long addr )
{
   char * videobuffer = (char *) xf86AccelInfoRec.FramebufferBase;
   return *(volatile unsigned long*)(videobuffer + (addr & ~3) );
}

unsigned long
writefb( unsigned long addr, unsigned long value )
{
   char * videobuffer = (char *) xf86AccelInfoRec.FramebufferBase;
   *(unsigned long*)(videobuffer + (addr & ~3)) = value;
   return *(volatile unsigned long*)(videobuffer + (addr & ~3) );
}

void
writescan( unsigned long scan, unsigned long color )
{
    int i;
    char * videobuffer = (char *) xf86AccelInfoRec.FramebufferBase;
    videobuffer += scan * vga256InfoRec.displayWidth * vgaBitsPerPixel >> 3;
    for( i = vga256InfoRec.displayWidth; --i; ) {
	switch( vgaBitsPerPixel ) {
	    case 8: 
		*videobuffer++ = color; 
		break;
	    case 16: 
		*(unsigned short*)videobuffer = color;
		videobuffer += 2;
		break;
	    case 32:
		*(unsigned long*)videobuffer = color;
		videobuffer += 4;
		break;
	}
    }
}


/* Acceleration init function, sets up pointers to our accelerated functions */

void 
S3SAVAccelInit() 
{

/* Set-up our GE command primitive */
    
    if (vgaBitsPerPixel == 8) {
      s3vPriv.PlaneMask = 0xFF;
      }
    else if (vgaBitsPerPixel == 16) {
      s3vPriv.PlaneMask = 0xFFFF;
      }
    else if (vgaBitsPerPixel == 24) {
      s3vPriv.PlaneMask = 0xFFFFFF;
      }
    else if (vgaBitsPerPixel == 32) {
      s3vPriv.PlaneMask = 0xFFFFFFFF;
      }


    /* General acceleration flags */

    xf86AccelInfoRec.Flags = PIXMAP_CACHE
         | BACKGROUND_OPERATIONS
         | COP_FRAMEBUFFER_CONCURRENCY
         | NO_SYNC_AFTER_CPU_COLOR_EXPAND
         | HARDWARE_PATTERN_BIT_ORDER_MSBFIRST
         | HARDWARE_PATTERN_PROGRAMMED_BITS
         | HARDWARE_PATTERN_SCREEN_ORIGIN
         | HARDWARE_PATTERN_MONO_TRANSPARENCY
	 | USE_TWO_POINT_LINE 
	 | TWO_POINT_LINE_NOT_LAST
	 | HARDWARE_CLIP_LINE
	 ;

     xf86AccelInfoRec.Sync = S3SAVAccelSync;


    /* ScreenToScreen copies */

#if 1
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
        S3SAVSetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
        S3SAVSubsequentScreenToScreenCopy;
    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK;
#endif


    /* Filled rectangles */

#if 1
    xf86AccelInfoRec.SetupForFillRectSolid = 
        S3SAVSetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = 
        S3SAVSubsequentFillRectSolid;
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
#endif

    /* ImageWrite no sync required */

#if 1
    xf86AccelInfoRec.ImageWrite =
        SavageImageWrite;
#endif

    /* Bresenham lines */

#if 1
    xf86AccelInfoRec.SubsequentBresenhamLine =
        SavageSubsequentBresenhamLine;
    xf86AccelInfoRec.ErrorTermBits =
	16;
#endif

    xf86AccelInfoRec.ColorExpandFlags = SCANLINE_PAD_DWORD |
					CPU_TRANSFER_PAD_DWORD | 
					BIT_ORDER_IN_BYTE_MSBFIRST |
					VIDEO_SOURCE_GRANULARITY_PIXEL |
					LEFT_EDGE_CLIPPING;
    /* WriteBitmap color expand */

#if 1
    xf86AccelInfoRec.WriteBitmap =
        SavageWriteBitmapCPUToScreenColorExpand;
#endif

#if 0
    xf86AccelInfoRec.SetupForScreenToScreenColorExpand =
             S3SAVSetupForScreenToScreenColorExpand;
    xf86AccelInfoRec.SubsequentScreenToScreenColorExpand =
             S3SAVSubsequentCPUToScreenColorExpand;
#endif

#if 1
    xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
             S3SAVSetupForCPUToScreenColorExpand;
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
             S3SAVSubsequentCPUToScreenColorExpand;
    xf86AccelInfoRec.CPUToScreenColorExpandBase = s3vPriv.BciMem;
    xf86AccelInfoRec.CPUToScreenColorExpandRange = 128 * 1024;
#endif

 
    /* These are the 8x8 pattern fills using color expansion */

#if 1
    xf86AccelInfoRec.SetupFor8x8PatternColorExpand = 
            S3SAVSetupFor8x8PatternColorExpand;
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand = 
            S3SAVSubsequent8x8PatternColorExpand;  
#endif


    /* These are the 8x8 color pattern fills */

#if 1
    xf86AccelInfoRec.SetupForFill8x8Pattern = 
            S3SAVSetupForFill8x8Pattern;
    xf86AccelInfoRec.SubsequentFill8x8Pattern = 
            S3SAVSubsequentFill8x8Pattern; 
#endif


    /* These are the accelerated line functions */

    xf86AccelInfoRec.SubsequentTwoPointLine = 
	    S3SAVSubsequentTwoPointLine;
    xf86AccelInfoRec.SetClippingRectangle = 
            S3SAVSetClippingRectangle;


    /*
     * Finally, we set up the video memory space available to the pixmap
     * cache. In this case, all memory from the end of the virtual screen
     * to the end of the command overflow buffer can be used. If you haven't
     * enabled the PIXMAP_CACHE flag, then these lines can be omitted.
     */

     xf86InitPixmapCache(&vga256InfoRec, vga256InfoRec.virtualY *
	vga256InfoRec.displayWidth * vga256InfoRec.bitsPerPixel / 8,
	s3vPriv.cobOffset);

     /* And these are screen parameters used to setup the GE */

     s3vPriv.Width = vga256InfoRec.displayWidth;
     s3vPriv.Bpp = vgaBitsPerPixel / 8;
     s3vPriv.Bpl = s3vPriv.Width * s3vPriv.Bpp;
     s3vPriv.ScissB = (vga256InfoRec.videoRam * 1024 - 4096 - s3vPriv.cobSize) / s3vPriv.Bpl;
     if (s3vPriv.ScissB > 2047)
         s3vPriv.ScissB = 2047;
}




/* The sync function for the GE */
void
S3SAVAccelSync()
{
    WaitIdleEmpty();
}


/* This next function performs a reset of the graphics engine and 
 * fills in some GE registers with default values.                  
 */

void
S3SAVGEReset(int from_timeout, int line, char *file)
{
    unsigned char tmp;
    int r;
    int32  fifo_control, miu_control, streams_timeout, misc_timeout;

    if (from_timeout) {
      static int n=0;
      if (n++ < 10 || xf86Verbose > 1)
	ErrorF("\tS3SAVGEReset called from %s line %d\n",file,line);
    }
    else
      WaitIdleEmpty();


    if (from_timeout) {
      /* reset will trash these registers, so save them */
      fifo_control    = ((mmtr)s3savMmioMem)->memport_regs.regs.fifo_control;
      miu_control     = ((mmtr)s3savMmioMem)->memport_regs.regs.miu_control;
      streams_timeout = ((mmtr)s3savMmioMem)->memport_regs.regs.streams_timeout;
      misc_timeout    = ((mmtr)s3savMmioMem)->memport_regs.regs.misc_timeout;
    }

    outb(vgaCRIndex, 0x66);
    tmp = inb(vgaCRReg);

    usleep(10000);
    for (r=1; r<10; r++) {  /* try multiple times to avoid lockup */
      int success;
      outb(vgaCRReg, tmp | 0x02);
      usleep(10000);
      outb(vgaCRReg, tmp & ~0x02);
      usleep(10000);

      if (!from_timeout) 
	WaitIdleEmpty();
      SETB_DEST_SRC_STR(s3vPriv.Bpl, s3vPriv.Bpl); 

      usleep(10000);
      switch( s3vPriv.chip ) {
        case S3_SAVAGE3D:
        case S3_SAVAGE_MX:
	  success = (STATUS_WORD0 & 0x0008ffff) == 0x00080000;
	  break;
	case S3_SAVAGE4:
	case S3_PROSAVAGE:
	  success = (ALT_STATUS_WORD0 & 0x0081ffff) == 0x00800000;
	  break;
	case S3_SAVAGE2000:
	  success = (ALT_STATUS_WORD0 & 0x008fffff) == 0;
	  break;
      }
      if( !success ) {
        usleep(10000);
	ErrorF("restarting S3 graphics engine reset %2d ...\n",r);
      }
      else
	break;
    }

    if (from_timeout) {
      /* restore trashed registers */
      ((mmtr)s3savMmioMem)->memport_regs.regs.fifo_control    = fifo_control;
      ((mmtr)s3savMmioMem)->memport_regs.regs.miu_control     = miu_control;
      ((mmtr)s3savMmioMem)->memport_regs.regs.streams_timeout = streams_timeout;
      ((mmtr)s3savMmioMem)->memport_regs.regs.misc_timeout    = misc_timeout;
    }

    SETB_SRC_BASE(0);
    SETB_DEST_BASE(0);   

    /* Now write some default rgisters and reset cached values */
    s3vCached_CLIP_LR = -1;
    s3vCached_CLIP_TB = -1;
    CACHE_SETB_CLIP_L_R(0, s3vPriv.Width);
    CACHE_SETB_CLIP_T_B(0, s3vPriv.ScissB);
    s3vCached_MONO_PATTERN0 = 0;
    s3vCached_MONO_PATTERN1 = 0;
    CACHE_SETB_MONO_PAT0(~0);
    CACHE_SETB_MONO_PAT1(~0);   

    s3vCached_RWIDTH_HEIGHT = -1;
    s3vCached_PAT_FGCLR = -1;
    s3vCached_PAT_BGCLR = -1;
    s3vCached_CMD_SET = -1;
    if (xf86Verbose > 1)
        ErrorF("Savage register cache hits: %d misses: %d\n",
            s3vCacheHit, s3vCacheMiss);    
    s3vCacheHit = 0; s3vCacheMiss = 0;
    S3SAVSetGBD();
}



/* These are the ScreenToScreen bitblt functions. We support all ROPs, all
 * directions, and a planemask by adjusting the ROP and using the mono pattern
 * registers. There is no support for transparency. 
 */

void 
S3SAVSetupForScreenToScreenCopy(xdir, ydir, rop, planemask,
transparency_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int transparency_color;
{
    BCI_GET_PTR;
    int cmd;

    cmd = BCI_CMD_RECT | BCI_CMD_DEST_GBD | BCI_CMD_SRC_GBD;
    cmd |= s3vAlu[rop];
    if (transparency_color != -1)
        cmd |= BCI_CMD_SEND_COLOR | BCI_CMD_SRC_TRANSPARENT;

    if (xdir == 1 ) cmd |= BCI_CMD_RECT_XP;
    if (ydir == 1 ) cmd |= BCI_CMD_RECT_YP;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(cmd);
    if (transparency_color != -1) {
        BCI_SEND(transparency_color);
    }
#endif
    s3SavedBciCmd = cmd;
    s3SavedBgColor = transparency_color;
}

void 
S3SAVSubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
int x1, y1, x2, y2, w, h;
{
    BCI_GET_PTR;

    if (!w || !h) return;
    if (!(s3SavedBciCmd & BCI_CMD_RECT_XP)) {
        w --;
        x1 += w;
        x2 += w;
        w ++;
    }
    if (!(s3SavedBciCmd & BCI_CMD_RECT_YP)) {
        h --;
        y1 += h;
        y2 += h;
        h ++;
    }

#if BCI_REPEAT
    WaitQueue(4);
    BCI_SEND(BCI_X_Y(x1, y1));
    BCI_SEND(BCI_X_Y(x2, y2));
    BCI_SEND(BCI_W_H(w, h));
#else
    WaitQueue(6);
    BCI_SEND(s3SavedBciCmd);
    if (s3SavedBgColor != -1) 
	BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x1, y1));
    BCI_SEND(BCI_X_Y(x2, y2));
    BCI_SEND(BCI_W_H(w, h));
#endif
}


/*
 * SetupForFillRectSolid is also called to set up for TwoPointLine.
 */ 

void 
S3SAVSetupForFillRectSolid(color, rop, planemask)
int color, rop;
unsigned planemask;
{
    BCI_GET_PTR;
    int cmd;
    static unsigned int MaxMask[5] = { 0, 0xff, 0xffff, 0xffffff, 0xffffffff };

    cmd = BCI_CMD_RECT
        | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_SOLID;

    /* Don't send a color if we don't have to. */

    if( rop == GXcopy )
    {
	if( color == 0 )
	    rop = GXclear;
	else if( color == MaxMask[s3vPriv.Bpp] )
	    rop = GXset;
    }

    if(
      (rop != GXclear) &&
      (rop != GXnoop) &&
      (rop != GXinvert) &&
      (rop != GXset)
    )
	cmd |= BCI_CMD_SEND_COLOR;

    /* In general, we ignore the planemask.  This is badness of a */
    /* high magnitude, but most apps do not care.  We handle one special */
    /* case here: xv uses a GXinvert ROP with a planemask to highlight */
    /* its marquee, and we can adjust for that. */

    if( 
        (rop == GXinvert) && 
	(((planemask + 1) & MaxMask[s3vPriv.Bpp]) != 0 )
    )
    {
	cmd |= BCI_CMD_SEND_COLOR;
	color = planemask;
	rop = GXxor;
    }

    cmd |= s3vAlu[rop];

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(cmd);
    /*ErrorF("FillRect command 0x%.8x sent\n", cmd);*/
    BCI_SEND(color);
#endif
    s3SavedBciCmd = cmd;
    s3SavedFgColor = color;
}
    
    
void 
S3SAVSubsequentFillRectSolid(x, y, w, h)
int x, y, w, h;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    WaitQueue(5);
    BCI_SEND(s3SavedBciCmd);
    if( s3SavedBciCmd & BCI_CMD_SEND_COLOR )
	BCI_SEND(s3SavedFgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#endif
    s3LineHWClipSet = 0;
}

void
SavageSubsequentBresenhamLine(x1, y1, octant, err, e1, e2, length)
    int x1, y1, octant, err, e1, e2, length;
{
    BCI_GET_PTR;
    unsigned int cmd;

    cmd = (s3SavedBciCmd & 0x00ffffff);
    cmd |= BCI_CMD_LINE_LAST_PIXEL;

    WaitQueue( 5 );
    BCI_SEND(cmd);
    if( cmd & BCI_CMD_SEND_COLOR )
	BCI_SEND( s3SavedFgColor );
    BCI_SEND(BCI_LINE_X_Y(x1, y1));
    BCI_SEND(BCI_LINE_STEPS(e2, e1));
    BCI_SEND(BCI_LINE_MISC(length, 
    			   !!(octant & YMAJOR),
			   !(octant & XDECREASING),
			   !(octant & YDECREASING),
			   err));
}

void
S3SAVSetupForScreenToScreenColorExpand(bg, fg, rop, planemask)
int bg, fg, rop;
unsigned planemask;
{
}

void
S3SAVSubsequentScreenToScreenColorExpand(x, y, w, h, skipleft)
int x, y, w, h, skipleft;
{
}

void
S3SAVSetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
int bg, fg, rop;
unsigned planemask;
{
    BCI_GET_PTR;
    int cmd;

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
	| BCI_CMD_CLIP_LR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_MONO;

    if(
      (rop != GXclear) &&
      (rop != GXnoop) &&
      (rop != GXinvert) &&
      (rop != GXset)
    )
        cmd |= BCI_CMD_SEND_COLOR;

    cmd |= s3vAlu[rop];

    if (bg != -1)
        cmd |= BCI_CMD_SEND_COLOR;
    else 
	cmd |= BCI_CMD_SRC_TRANSPARENT;

#if BCI_REPEAT    
    WaitQueue(3);
    BCI_SEND(cmd);
    BCI_SEND(fg);
    if(bg != -1) BCI_SEND(bg);
    s3SavedBciCmd = cmd;
#endif
    s3SavedBciCmd = cmd;
    s3SavedFgColor = fg;
    s3SavedBgColor = bg;
}

void
S3SAVSubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
int x, y, w, h, skipleft;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(4);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    /* 7 is not enough.  XAA will be sending bitmap data next.  We */
    /* should probably wait for empty/idle here. */

    WaitQueue(7);

    BCI_SEND(s3SavedBciCmd);
    BCI_SEND(BCI_CLIP_LR(x, x+w-1));
    x -= skipleft;
    w += skipleft;
    w = (w + 31) & ~31;
    if( s3SavedBciCmd & BCI_CMD_SEND_COLOR )
	BCI_SEND(s3SavedFgColor);
    if( s3SavedBgColor != -1 )
	BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#endif
}


void
S3SAVSetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, rop, planemask)
unsigned patternx, patterny;
int bg, fg, rop;
unsigned planemask;
{
    BCI_GET_PTR;
    int cmd;
    unsigned int bd;

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_DEST_GBD | BCI_CMD_PAT_MONO;

    if(
      (rop != GXclear) &&
      (rop != GXnoop) &&
      (rop != GXinvert) &&
      (rop != GXset)
    )
	cmd |= BCI_CMD_SEND_COLOR;

    if (bg == -1)
	cmd |= BCI_CMD_PAT_TRANSPARENT;

    cmd |= s3vAlu_sp[rop];

    bd = BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(bd, 1);
    BCI_BD_SET_STRIDE(bd, 8);

#if BCI_REPEAT
    WaitQueue(5);
    BCI_SEND(cmd);
    BCI_SEND(pat_offset);
    BCI_SEND(bd);
    if( cmd & BCI_CMD_SEND_COLOR )
	BCI_SEND(fg);
    if (bg != -1) BCI_SEND(bg);
#endif
    s3SavedBciCmd = cmd;
    s3SavedFgColor = fg;
    s3SavedBgColor = bg;
}


void
S3SAVSubsequent8x8PatternColorExpand(pattern0, pattern1, x, y, w, h)
unsigned pattern0, pattern1;
int x, y, w, h;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    WaitQueue(7);
    BCI_SEND(s3SavedBciCmd);
    if( s3SavedBciCmd & BCI_CMD_SEND_COLOR )
	BCI_SEND(s3SavedFgColor);
    if( s3SavedBgColor != -1 )
	BCI_SEND(s3SavedBgColor);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    BCI_SEND(pattern0);
    BCI_SEND(pattern1);
#endif
}


void 
S3SAVSetupForFill8x8Pattern(patternx, patterny, rop, planemask, trans_col)
unsigned patternx, patterny;
int rop; 
unsigned planemask;
int trans_col;
{
    BCI_GET_PTR;

    int cmd;
    unsigned int bd;
    int pat_offset;
    
    /* ViRGEs and Savages do not support transparent color patterns. */
    /* We do not set the HARDWARE_PATTERN_TRANSPARENT bit, so we should */
    /* never receive one. */

    pat_offset = (int) (patternx * s3vPriv.Bpp + patterny * s3vPriv.Bpl);

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_DEST_GBD | BCI_CMD_PAT_SBD_COLOR_NEW;
        
    cmd |= s3vAlu_sp[rop];

    bd = BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(bd, vgaBitsPerPixel);
    BCI_BD_SET_STRIDE(bd, 8);

#if BCI_REPEAT
    WaitQueue(6);
    BCI_SEND(cmd);
    BCI_SEND(pat_offset);
    BCI_SEND(bd);
    if (trans_col != -1) BCI_SEND(trans_col);
#endif
    s3SavedBciCmd = cmd;
    s3SavedSbdOffset = pat_offset;
    s3SavedSbd = bd;
    s3SavedBgColor = trans_col;
}


void S3SAVSubsequentFill8x8Pattern(patternx, patterny, x, y, w, h)
unsigned patternx, patterny;
int x, y, w, h;
{
    BCI_GET_PTR;

#if BCI_REPEAT
    WaitQueue(2);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#else
    WaitQueue(5);
    BCI_SEND(s3SavedBciCmd);
    BCI_SEND(s3SavedSbdOffset);
    BCI_SEND(s3SavedSbd);
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
#endif
}



void S3SAVSubsequentTwoPointLine(x1, y1, x2, y2, bias)
int x1, x2, y1, y2, bias;
{
    BCI_GET_PTR;

    int cmd;
    int dx, dy;
    int min, max, xp, yp, ym;

    dx = x2 - x1;
    dy = y2 - y1;

#ifdef DEBUG_EXTRA
    ErrorF("TwoPointLine, (%4d,%4d)-(%4d,%4d), clr %08x, last pt %s\n",
        x1, y1, x2, y2, s3SavedFgColor, (bias & 0x100)?"NO ":"YES");
#endif

    xp = (dx >= 0);
    if( !xp ) {
	dx = -dx;
    }

    yp = (dy >= 0);
    if( !yp ) {
	dy = -dy;
    }

    ym = (dy > dx);
    if( ym ) {
	max = dy;
	min = dx;
    }
    else {
	max = dx;
	min = dy;
    }

    if( !(bias & 0x100) ) {
	max++;
    }

    cmd = (s3SavedBciCmd & 0x00ffffff);

    cmd |= BCI_CMD_LINE_LAST_PIXEL;

    if( s3LineHWClipSet ) {
	cmd |= BCI_CMD_CLIP_CURRENT;
	s3LineHWClipSet = FALSE;
    }

    WaitQueue(5);
    BCI_SEND( cmd );
    if( cmd & BCI_CMD_SEND_COLOR )
	BCI_SEND( s3SavedFgColor );
    BCI_SEND( BCI_LINE_X_Y( x1, y1 ) );
    BCI_SEND( BCI_LINE_STEPS( 2 * (min - max), 2 * min ) );
    BCI_SEND( BCI_LINE_MISC( max, ym, xp, yp, 2 * min - max ) );
}


void S3SAVSetClippingRectangle(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    BCI_GET_PTR;
    int cmd;

#ifdef DEBUG_EXTRA
    ErrorF("ClipRect, (%4d,%4d)-(%4d,%4d) \n", x1, y1, x2, y2 );
#endif

    cmd = BCI_CMD_NOP | BCI_CMD_CLIP_NEW;
    WaitQueue(3);
    BCI_SEND(cmd);
    BCI_SEND(BCI_CLIP_TL(y1, x1));
    BCI_SEND(BCI_CLIP_BR(y2, x2));
    s3LineHWClipSet = TRUE;
}
