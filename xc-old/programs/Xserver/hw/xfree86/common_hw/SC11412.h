/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/SC11412.h,v 3.0 1996/11/18 13:11:10 dawes Exp $ */


/* Norbert Distler ndistler@physik.tu-muenchen.de */


/* $XConsortium: SC11412.h /main/3 1996/02/21 17:40:44 kaleb $ */

typedef int Bool;
#define TRUE 1
#define FALSE 0
#define QUARZFREQ	        14318
#define MIN_SC11412_FREQ        45000
#define MAX_SC11412_FREQ       100000

Bool SC11412SetClock( 
#if NeedFunctionPrototypes
   long
#endif
);     

