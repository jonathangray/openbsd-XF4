/*************************************************************************************
 * $Id: theatre_detect.h,v 1.1 2006/01/15 22:07:50 matthieu Exp $
 * 
 * Copyright (C) 2005 Bogdan D. bogdand@users.sourceforge.net
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the author shall not be used in advertising or 
 * otherwise to promote the sale, use or other dealings in this Software without prior written 
 * authorization from the author.
 *
 * $Log: theatre_detect.h,v $
 * Revision 1.1  2006/01/15 22:07:50  matthieu
 * Sync with X.Org 6.9.0: merge xfree86 ddx and drivers.
 *
 * Revision 1.2  2005/07/01 22:43:11  daniels
 * Change all misc.h and os.h references to <X11/foo.h>.
 *
 *
 ************************************************************************************/

#ifndef __THEATRE_DETECT_H__
#define __THEATRE_DETECT_H__

/*
 * Created by Bogdan D. bogdand@users.sourceforge.net
 */


TheatrePtr DetectTheatre(GENERIC_BUS_Ptr b);


#define TheatreDetectSymbolsList  \
		"DetectTheatre"

#ifdef XFree86LOADER

#define xf86_DetectTheatre         ((TheatrePtr (*)(GENERIC_BUS_Ptr))LoaderSymbol("DetectTheatre"))

#else

#define xf86_DetectTheatre             DetectTheatre

#endif		

#endif
