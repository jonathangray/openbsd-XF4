/* $XdotOrg: xc/programs/Xserver/hw/xfree86/drivers/via/via_priv.h,v 1.3 2004/07/16 04:38:54 anholt Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_priv.h,v 1.3 2003/08/27 15:16:12 tsi Exp $ */

#ifndef _VIA_PRIV_H_
#define _VIA_PRIV_H_ 1

#include "ddmpeg.h"
#ifdef XF86DRI
#include "via_common.h"
#endif

#define MEM_BLOCKS		4

typedef struct {
    unsigned long   base;		/* Offset into fb */
    int    pool;			/* Pool we drew from */
#ifdef XF86DRI
    int    drm_fd;			/* Fd in DRM mode */
    drmViaMem drm;			/* DRM management object */
#endif
    int    slot;			/* Pool 3 slot */
    void  *pVia;			/* VIA driver pointer */
    FBLinearPtr linear;			/* X linear pool info ptr */
} VIAMem;

typedef VIAMem *VIAMemPtr;



typedef struct  {
    unsigned long   gdwVideoFlagTV1;
    unsigned long   gdwVideoFlagSW;
    unsigned long   gdwVideoFlagMPEG;
    unsigned long   gdwAlphaEnabled;		/* For Alpha blending use*/

    VIAMem SWOVMem;
    VIAMem HQVMem;
    VIAMem SWfbMem;

    DDPIXELFORMAT DPFsrc; 
    DDUPDATEOVERLAY UpdateOverlayBackup;    /* For HQVcontrol func use
					    // To save MPEG updateoverlay info.*/

/* device struct */
    SWDEVICE   SWDevice;
    SUBDEVICE   SUBDevice;
    MPGDEVICE   MPGDevice;
    OVERLAYRECORD   overlayRecordV1;
    OVERLAYRECORD   overlayRecordV3;

    BoxRec  AvailFBArea;
    FBLinearPtr   SWOVlinear;

    Bool MPEG_ON;
    Bool SWVideo_ON;

/*To solve the bandwidth issue */
    unsigned long   gdwUseExtendedFIFO;

/* For panning mode use */
    int panning_old_x;
    int panning_old_y;
    int panning_x;
    int panning_y;

/*To solve the bandwidth issue */
    unsigned char Save_3C4_16;
    unsigned char Save_3C4_17;
    unsigned char Save_3C4_18;

} swovRec, *swovPtr;

#endif /* _VIA_PRIV_H_ */
