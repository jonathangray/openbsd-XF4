/* $XConsortium: lbxsrvopts.h /main/6 1996/11/15 21:14:37 rws $ */
/*
 * Copyright 1994 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Network Computing Devices, Inc. not be
 * used in advertising or publicity pertaining to distribution of this
 * software without specific, written prior permission.
 *
 * THIS SOFTWARE IS PROVIDED `AS-IS'.  NETWORK COMPUTING DEVICES, INC.,
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NONINFRINGEMENT.  IN NO EVENT SHALL NETWORK
 * COMPUTING DEVICES, INC., BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE, DATA,
 * OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND REGARDLESS OF
 * WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XFree86: xc/programs/Xserver/lbx/lbxsrvopts.h,v 1.2 2000/05/18 23:46:24 dawes Exp $ */

#ifndef _LBX_SRVOPTS_H_
#define _LBX_SRVOPTS_H_

#include "lbxopts.h"

typedef struct _LbxNegOpts {
    int		nopts;
    short	proxyDeltaN;
    short	proxyDeltaMaxLen;
    short	serverDeltaN;
    short	serverDeltaMaxLen;
    LbxStreamOpts streamOpts;
    int		numBitmapCompMethods;
    unsigned char	*bitmapCompMethods;   /* array of indices */
    int		numPixmapCompMethods;
    unsigned char	*pixmapCompMethods;   /* array of indices */
    int		**pixmapCompDepths;   /* depths supported from each method */
    Bool	squish;
    Bool	useTags;
} LbxNegOptsRec;

typedef LbxNegOptsRec *LbxNegOptsPtr;


extern void LbxOptionInit ( LbxNegOptsPtr pno );
extern int LbxOptionParse ( LbxNegOptsPtr pno, unsigned char *popt, 
			    int optlen, unsigned char *preply );
extern LbxBitmapCompMethod * 
LbxSrvrLookupBitmapCompMethod ( LbxProxyPtr proxy, int methodOpCode );
extern LbxPixmapCompMethod * 
LbxSrvrLookupPixmapCompMethod ( LbxProxyPtr proxy, int methodOpCode );
extern LbxBitmapCompMethod * 
LbxSrvrFindPreferredBitmapCompMethod ( LbxProxyPtr proxy );
extern LbxPixmapCompMethod * 
LbxSrvrFindPreferredPixmapCompMethod ( LbxProxyPtr proxy, int format, int depth );


#endif /* _LBX_SRVOPTS_H_ */
