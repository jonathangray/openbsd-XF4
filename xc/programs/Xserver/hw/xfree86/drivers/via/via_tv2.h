/* $XFree86$ */
/*
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * VIA, S3 GRAPHICS, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _VIA_TV2_H_
#define _VIA_TV2_H_ 1

static const VIABIOSTVMASKTableRec tv2MaskTable = {
    { 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0, 0XFF, 0, 0, 0, 0, 0, 0, 0XFF, 0, 0XFF, 0, 0, 0XFF, 0XFF, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0, 0, 0, 0, 0, 0XFF, 0XFF, 0XFF, 0, 0, 0XFF, 0XFF, 0XFF, 0, 0, 0 },
    0X3F, 0X38, 61, 13, 22
};

static const VIABIOSTV2TableRec tv2Table[] = {
    {
        { 0X64, 0X3, 0X22, 0X33, 0X43, 0, 0X10, 0X7D, 0XAC, 0X5, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0XA, 0XCD, 0X80, 0X28, 0XBE, 0XFF, 0X7F, 0X20, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0X1, 0X2, 0, 0XFC, 0XF9, 0XFF, 0X10, 0X23, 0X2C, 0X9, 0X8, 0XA, 0XC, 0XD, 0XD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X3, 0X22, 0X33, 0X43, 0, 0X10, 0X7D, 0XAC, 0X5, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0XA, 0XCD, 0X80, 0X28, 0XBE, 0XFF, 0X7F, 0X20, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0X2, 0X2, 0XFD, 0X6, 0XF8, 0XB, 0XF3, 0XF, 0X70, 0X5, 0XF9, 0XB, 0XF1, 0X11, 0X6E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X5D, 0X4F, 0X4F, 0X81, 0X52, 0X9E, 0X56, 0XBA, 0, 0X40, 0, 0, 0, 0, 0, 0, 0X8, 0, 0XDF, 0, 0, 0XDF, 0X57, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X20, 0X40, 0X80, 0, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0XF, 0X7F, 0X7F, 0XF, 0X9A, 0X23, 0X8F, 0XEF, 0X57, 0XDF, 0XDF, 0X57, 0X11, 0XA, 0X8, 0X50, 0, 0, 0, 0, 0, 0X28, 0X50, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XF, 0X7F, 0X7F, 0XF, 0X9A, 0X23, 0X8F, 0XEF, 0X57, 0XDF, 0XDF, 0X57, 0X11, 0XA, 0X8, 0X50, 0, 0, 0, 0, 0, 0X50, 0XA0, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XF, 0X7F, 0X7F, 0XF, 0X9A, 0X23, 0X8F, 0XEF, 0X57, 0XDF, 0XDF, 0X57, 0X11, 0XA, 0X8, 0X50, 0, 0, 0, 0, 0, 0XA0, 0X40, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X7E07, 0, 0, 0, 0, 0, 0, 0 },
        { 0X2, 0X811, 0XF617, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X1, 0X2, 0X33, 0X40, 0, 0X10, 0XAD, 0XD3, 0X37, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0X7, 0X26, 0X2C, 0X20, 0X50, 0X63, 0XD5, 0X25, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0, 0, 0XFE, 0XFC, 0XFD, 0X5, 0X12, 0X1F, 0X25, 0XB, 0X8, 0XA, 0XC, 0XD, 0XD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X1, 0X2, 0X33, 0X40, 0, 0X10, 0XAD, 0XD3, 0X37, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0X7, 0X26, 0X2C, 0X20, 0X50, 0X63, 0XD5, 0X25, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0XFE, 0X3, 0XFB, 0X6, 0XF8, 0XA, 0XF5, 0XC, 0X73, 0X6, 0XF8, 0XB, 0XF2, 0X10, 0X6F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X4F, 0X4F, 0X88, 0X53, 0X83, 0X6F, 0XBA, 0, 0X40, 0, 0, 0, 0, 0, 0, 0X11, 0, 0XDF, 0, 0, 0XDF, 0X70, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X20, 0X40, 0X80, 0X4, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0X47, 0X7F, 0X7F, 0X47, 0X9A, 0X23, 0X95, 0X1A, 0X70, 0XDF, 0XDF, 0X70, 0X51, 0XA, 0X11, 0X5B, 0, 0, 0, 0, 0, 0X28, 0X50, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X47, 0X7F, 0X7F, 0X47, 0X9A, 0X23, 0X95, 0X1A, 0X70, 0XDF, 0XDF, 0X70, 0X51, 0XA, 0X11, 0X5B, 0, 0, 0, 0, 0, 0X50, 0XA0, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X47, 0X7F, 0X7F, 0X47, 0X9A, 0X23, 0X95, 0X1A, 0X70, 0XDF, 0XDF, 0X70, 0X51, 0XA, 0X11, 0X5B, 0, 0, 0, 0, 0, 0XA0, 0X40, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XB607, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0X84, 0X3, 0X2A, 0X33, 0X43, 0, 0X10, 0XDD, 0XB9, 0X15, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0XA, 0XED, 0X98, 0X1C, 0X96, 0X50, 0X5E, 0X1B, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0, 0XFF, 0XFD, 0XFC, 0XFF, 0X7, 0X13, 0X1E, 0X22, 0XD, 0X8, 0X9, 0XA, 0XB, 0XC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X84, 0X3, 0X2A, 0X33, 0X43, 0, 0X10, 0XDD, 0XB9, 0X15, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0XA, 0XED, 0X98, 0X1C, 0X96, 0X50, 0X5E, 0X1B, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0X2, 0X2, 0XFD, 0X6, 0XF8, 0XB, 0XF3, 0XF, 0X70, 0X5, 0XF9, 0XB, 0XF1, 0X11, 0X6E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X80, 0X63, 0X63, 0X84, 0X69, 0X1A, 0XEC, 0XF0, 0, 0X60, 0, 0, 0, 0, 0, 0, 0X5C, 0, 0X57, 0, 0, 0X57, 0XED, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0X40, 0X80, 0X1E, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0X27, 0X1F, 0X1F, 0X27, 0XE3, 0X34, 0X48, 0X83, 0XED, 0X57, 0X57, 0XED, 0X52, 0X12, 0X5C, 0X51, 0, 0, 0, 0, 0, 0X32, 0X64, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X27, 0X1F, 0X1F, 0X27, 0XE3, 0X34, 0X48, 0X83, 0XED, 0X57, 0X57, 0XED, 0X52, 0X12, 0X5C, 0X51, 0, 0, 0, 0, 0, 0X64, 0XC8, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X27, 0X1F, 0X1F, 0X27, 0XE3, 0X34, 0X48, 0X83, 0XED, 0X57, 0X57, 0XED, 0X52, 0X12, 0X5C, 0X51, 0, 0, 0, 0, 0, 0XC8, 0X90, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XDE07, 0, 0, 0, 0, 0, 0, 0 },
        { 0X2, 0X811, 0X5717, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X84, 0X3, 0X1A, 0X33, 0X40, 0, 0X10, 0X85, 0XF1, 0X4B, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0X7, 0X25, 0X2C, 0X1C, 0X18, 0X28, 0X87, 0X1F, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0XFF, 0XFE, 0XFD, 0XFE, 0X2, 0XA, 0X13, 0X1A, 0X1D, 0XF, 0X8, 0X9, 0XA, 0XB, 0XB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X84, 0X3, 0X1A, 0X33, 0X40, 0, 0X10, 0X85, 0XF1, 0X4B, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0X7, 0X25, 0X2C, 0X1C, 0X18, 0X28, 0X87, 0X1F, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0XFB, 0X4, 0XFB, 0X7, 0XF8, 0X9, 0XF6, 0XA, 0X74, 0X6, 0XF8, 0XB, 0XF2, 0X10, 0X6F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X73, 0X63, 0X63, 0X97, 0X67, 0X91, 0XEC, 0XF0, 0, 0X60, 0, 0, 0, 0, 0, 0, 0X5C, 0, 0X57, 0, 0, 0X57, 0XED, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X20, 0X40, 0X80, 0X4, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0XBF, 0X1F, 0X1F, 0XBF, 0XDB, 0X33, 0X38, 0X88, 0XED, 0X57, 0X57, 0XED, 0X52, 0X12, 0X5C, 0X51, 0, 0, 0, 0, 0, 0X32, 0X64, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XBF, 0X1F, 0X1F, 0XBF, 0XDB, 0X33, 0X38, 0X88, 0XED, 0X57, 0X57, 0XED, 0X52, 0X12, 0X5C, 0X51, 0, 0, 0, 0, 0, 0X64, 0XC8, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XBF, 0X1F, 0X1F, 0XBF, 0XDB, 0X33, 0X38, 0X88, 0XED, 0X57, 0X57, 0XED, 0X52, 0X12, 0X5C, 0X51, 0, 0, 0, 0, 0, 0XC8, 0X90, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X8607, 0, 0, 0, 0, 0, 0, 0 }
    }
};

static const VIABIOSTV2TableRec tv2OverTable[] = {
    {
        { 0X64, 0X3, 0X2, 0X33, 0X43, 0, 0X10, 0X7D, 0X72, 0X5, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0XD, 0X45, 0X38, 0X34, 0XF1, 0X91, 0X24, 0X25, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0, 0X1, 0X2, 0XFF, 0XF9, 0XFA, 0XC, 0X26, 0X32, 0X7, 0X8, 0XA, 0XD, 0XE, 0XF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X3, 0X2, 0X33, 0X43, 0, 0X10, 0X7D, 0X72, 0X5, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0XD, 0X45, 0X38, 0X34, 0XF1, 0X91, 0X24, 0X25, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0X2, 0X2, 0XFD, 0X6, 0XF8, 0XB, 0XF3, 0XF, 0X70, 0X5, 0XF9, 0XB, 0XF1, 0X11, 0X6E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X5D, 0X4F, 0X4F, 0X81, 0X52, 0X9E, 0XB, 0X3E, 0, 0X40, 0, 0, 0, 0, 0, 0, 0XEE, 0, 0XDF, 0, 0, 0XDF, 0XC, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X20, 0X40, 0X80, 0X1, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0XF, 0X7F, 0X7F, 0XF, 0X9A, 0X23, 0X8F, 0XEF, 0XC, 0XDF, 0XDF, 0XC, 0X11, 0XA, 0XEE, 0X31, 0, 0, 0, 0, 0, 0X28, 0X50, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XF, 0X7F, 0X7F, 0XF, 0X9A, 0X23, 0X8F, 0XEF, 0XC, 0XDF, 0XDF, 0XC, 0X11, 0XA, 0XEE, 0X31, 0, 0, 0, 0, 0, 0X50, 0XA0, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XF, 0X7F, 0X7F, 0XF, 0X9A, 0X23, 0X8F, 0XEF, 0XC, 0XDF, 0XDF, 0XC, 0X11, 0XA, 0XEE, 0X31, 0, 0, 0, 0, 0, 0XA0, 0X40, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X7E07, 0, 0, 0, 0, 0, 0, 0 },
        { 0X2, 0X811, 0X9917, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X1, 0X12, 0X33, 0X40, 0, 0X10, 0X1D, 0X68, 0X26, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0XF, 0X67, 0X58, 0X3C, 0X24, 0XBC, 0X4A, 0X2F, 0, 0X1, 0XA, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0, 0X1, 0X1, 0XFE, 0XFA, 0XFD, 0XE, 0X24, 0X2E, 0X7, 0X7, 0XA, 0XD, 0XF, 0XF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X64, 0X1, 0X12, 0X33, 0X40, 0, 0X10, 0X1D, 0X68, 0X26, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0XF, 0X67, 0X58, 0X3C, 0X24, 0XBC, 0X4A, 0X2F, 0, 0X1, 0XA, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0XFD, 0X3, 0XFB, 0X7, 0XF8, 0XA, 0XF5, 0XB, 0X74, 0X6, 0XF8, 0XB, 0XF2, 0X10, 0X6F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X73, 0X4F, 0X4F, 0X97, 0X54, 0X8F, 0XF2, 0X1F, 0, 0X40, 0, 0, 0, 0, 0, 0, 0XE5, 0, 0XDF, 0, 0, 0XDF, 0XF3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X20, 0X40, 0X80, 0X1, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0XBF, 0X7F, 0X7F, 0XBF, 0X9A, 0X23, 0X90, 0X65, 0XF3, 0XDF, 0XDF, 0XF3, 0X49, 0X9, 0XE5, 0X26, 0, 0, 0, 0, 0, 0X28, 0X50, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XBF, 0X7F, 0X7F, 0XBF, 0X9A, 0X23, 0X90, 0X65, 0XF3, 0XDF, 0XDF, 0XF3, 0X49, 0X9, 0XE5, 0X26, 0, 0, 0, 0, 0, 0X50, 0XA0, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XBF, 0X7F, 0X7F, 0XBF, 0X9A, 0X23, 0X90, 0X65, 0XF3, 0XDF, 0XDF, 0XF3, 0X49, 0X9, 0XE5, 0X26, 0, 0, 0, 0, 0, 0XA0, 0X40, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X1E07, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0X84, 0X3, 0XA, 0X33, 0X43, 0, 0X10, 0XC5, 0XAD, 0X10, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0X4, 0X7, 0X20, 0XC, 0X8, 0, 0, 0X1C, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0, 0XFF, 0XFD, 0XFC, 0XFE, 0X6, 0X13, 0X1E, 0X23, 0XD, 0X8, 0XA, 0XB, 0XC, 0XC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X84, 0X3, 0XA, 0X33, 0X43, 0, 0X10, 0XC5, 0XAD, 0X10, 0X99, 0X17, 0X93, 0XA5, 0X3, 0XBA, 0, 0, 0X4, 0X7, 0X20, 0XC, 0X8, 0, 0, 0X1C, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X6D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0X2, 0X2, 0XFD, 0X6, 0XF8, 0XB, 0XF3, 0XF, 0X70, 0X5, 0XF9, 0XB, 0XF1, 0X11, 0X6E, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X7D, 0X63, 0X63, 0X81, 0X69, 0X18, 0XBA, 0XF0, 0, 0X60, 0, 0, 0, 0, 0, 0, 0X5A, 0, 0X57, 0, 0, 0X57, 0XBB, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0X40, 0X80, 0X1, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0XF, 0X1F, 0X1F, 0XF, 0XE3, 0X34, 0X44, 0XBC, 0XBB, 0X57, 0X57, 0XBB, 0X52, 0X12, 0X5A, 0X48, 0, 0, 0, 0, 0, 0X32, 0X64, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XF, 0X1F, 0X1F, 0XF, 0XE3, 0X34, 0X44, 0XBC, 0XBB, 0X57, 0X57, 0XBB, 0X52, 0X12, 0X5A, 0X48, 0, 0, 0, 0, 0, 0X64, 0XC8, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XF, 0X1F, 0X1F, 0XF, 0XE3, 0X34, 0X44, 0XBC, 0XBB, 0X57, 0X57, 0XBB, 0X52, 0X12, 0X5A, 0X48, 0, 0, 0, 0, 0, 0XC8, 0X90, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XCE07, 0, 0, 0, 0, 0, 0, 0 },
        { 0X2, 0X811, 0X817, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X84, 0X3, 0X2, 0X33, 0X40, 0, 0X10, 0X75, 0X7B, 0X34, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0X8, 0XFD, 0XEF, 0X20, 0XC, 0X8C, 0X79, 0X26, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0XFF, 0XFE, 0XFD, 0XFE, 0X2, 0XA, 0X13, 0X1A, 0X1D, 0XF, 0X8, 0X9, 0XA, 0XB, 0XB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X84, 0X3, 0X2, 0X33, 0X40, 0, 0X10, 0X75, 0X7B, 0X34, 0XA3, 0, 0X94, 0XFF, 0X3, 0XBA, 0, 0, 0X8, 0XFD, 0XEF, 0X20, 0XC, 0X8C, 0X79, 0X26, 0, 0X1, 0X2, 0X80, 0, 0, 0X75, 0XC, 0X4, 0X76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0X48, 0XFB, 0X4, 0XFB, 0X7, 0XF8, 0X9, 0XF6, 0XA, 0X74, 0X6, 0XF8, 0XB, 0XF2, 0X10, 0X6F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X71, 0X63, 0X63, 0X95, 0X67, 0X90, 0X6F, 0XF0, 0, 0X60, 0, 0, 0, 0, 0, 0, 0X57, 0, 0X57, 0, 0, 0X57, 0X70, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0X20, 0X40, 0X80, 0X1, 0X47, 0X1C, 0, 0 },
        { 0, 0, 0, 0X47, 0X1C, 0, 0, 0 },
        { 0XAF, 0X1F, 0X1F, 0XAF, 0XDB, 0X33, 0X35, 0X7D, 0X70, 0X57, 0X57, 0X70, 0X52, 0X12, 0X58, 0X5B, 0, 0, 0, 0, 0, 0X32, 0X64, 0, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XAF, 0X1F, 0X1F, 0XAF, 0XDB, 0X33, 0X35, 0X7D, 0X70, 0X57, 0X57, 0X70, 0X52, 0X12, 0X58, 0X5B, 0, 0, 0, 0, 0, 0X64, 0XC8, 0X40, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0XAF, 0X1F, 0X1F, 0XAF, 0XDB, 0X33, 0X35, 0X7D, 0X70, 0X57, 0X57, 0X70, 0X52, 0X12, 0X58, 0X5B, 0, 0, 0, 0, 0, 0XC8, 0X90, 0X81, 0, 0, 0X80, 0X20, 0X80, 0, 0, 0 },
        { 0X7E07, 0, 0, 0, 0, 0, 0, 0 }
    }
};

#endif /* _VIA_TV2_H_ */
