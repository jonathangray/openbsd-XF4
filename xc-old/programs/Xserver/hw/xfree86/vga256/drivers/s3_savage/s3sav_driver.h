/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_driver.h,v 1.1.2.1 1999/07/30 11:21:34 hohndel Exp $ */

/* Header file for ViRGE server */

/* Declared in s3v_driver.c */
extern vgaVideoChipRec S3V;
extern vgaCRIndex, vgaCRReg;

/* Driver variables */

#if defined(linux) && defined(__i386__)
#define	USEBIOS
#endif

/* Driver data structure; this should contain all neeeded info for a mode */
typedef struct {     
   vgaHWRec std;
   unsigned int mode;
   unsigned int refresh;

   unsigned char SR8, SR10, SR11, SR12, SR13, SR15, SR18, SR29; /* SR9-SR1C, ext seq. */
   unsigned char SR54[8];
   unsigned char Clock;
   unsigned char s3DacRegs[0x101];
   unsigned char CR31, CR32, CR33, CR34, CR36, CR3A, CR3B, CR3C;
   unsigned char CR40, CR42, CR43, CR45;
   unsigned char CR50, CR51, CR53, CR58, CR5B, CR5D, CR5E;
   unsigned char CR60, CR65, CR66, CR67, CR68, CR69, CR6F; /* Video attrib. */
   unsigned char CR86, CR88;
   unsigned char CR90, CR91, CRB0;
   unsigned char ColorStack[8]; /* S3 hw cursor color stack CR4A/CR4B */
   unsigned int  STREAMS[22];   /* Streams regs */
   unsigned int  MMPR0, MMPR1, MMPR2, MMPR3;   /* MIU regs */
} vgaS3VRec, *vgaS3VPtr;

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'new->xxx'.
 */

#define new ((vgaS3VPtr)vgaNewVideoState)


/* PCI info structure */

typedef struct S3PCIInformation {
   int DevID;
   int ChipType;
   int ChipRev;
   unsigned char * MemBase;
   unsigned char * MemBase1;
} S3PCIInformation;

/* Private data structure used for storing all variables needed in driver */
/* This is not exported outside of here, so no need to worry about packing */

typedef struct {
   int chip;
   unsigned short ChipId;
   unsigned char * MmioBase;
   unsigned char * FrameBufferBase;
   pointer MmioMem;
   pointer BciMem;
   unsigned long CursorKByte;
   unsigned long ShadowPhysical;
   unsigned long volatile * ShadowVirtual;
   unsigned long ShadowCounter;

   /* Support for the command overflow buffer. */
   unsigned long cobIndex;	/* size index */
   unsigned long cobSize;	/* size in bytes */
   unsigned long cobOffset;	/* offset in frame buffer */

   int MemOffScreen;
   int HorizScaleFactor;
   Bool STREAMSRunning;
   Bool NeedSTREAMS;
   int Width, Bpp,Bpl, ScissB;
   unsigned PlaneMask;
   int MCLK;
   Bool NoPCIRetry;
   int statusDelay;

   int (*WaitQueue)(int);
   int (*WaitIdle)(void);
   int (*WaitIdleEmpty)(void);

} S3VPRIV;


/* VESA BIOS Mode information. */

#ifdef USEBIOS

typedef struct _S3VMODEENTRY {
   unsigned short Width;
   unsigned short Height;
   unsigned short VesaMode;
   unsigned char RefreshCount;
   unsigned char * RefreshRate;
} S3VMODEENTRY, * PS3VMODEENTRY;

typedef struct _S3VMODETABLE {
   unsigned short NumModes;
   S3VMODEENTRY Modes[1];
} S3VMODETABLE, * PS3VMODETABLE;

#endif

/* Function prototypes */

S3PCIInformation * S3SAVGetPCIInfo();
extern Bool S3SAVCursorInit();
extern void S3SAVRestoreCursor();
extern void S3SAVWarpCursor();
extern void S3SAVQueryBestSize();
#ifdef USEBIOS
extern S3VMODETABLE* S3SAVGetBIOSModeTable( int );
extern void S3SAVFreeBIOSModeTable( S3VMODETABLE** );
extern void S3SAVSetVESAMode( int, int );
extern void S3SAVSetTextMode( );
#endif

/* Constants for CR69. */

#define CRT_ACTIVE	0x01
#define LCD_ACTIVE	0x02
#define TV_ACTIVE	0x04
#define CRT_ATTACHED	0x10
#define LCD_ATTACHED	0x20
#define TV_ATTACHED	0x40

/* Various defines which are used to pass flags between the Setup and 
 * Subsequent functions. 
 */

#define NO_MONO_FILL      0x00
#define NEED_MONO_FILL    0x01
#define MONO_TRANSPARENCY 0x02


extern S3VPRIV s3vPriv;
