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

#ifndef _VIAFB_INC
#define _VIAFB_INC

#include "mtypes.h"
#include "swrast/swrast.h"
extern GLboolean via_alloc_front_buffer(viaContextPtr vmesa);
extern GLboolean via_alloc_back_buffer(viaContextPtr vmesa);
extern void via_free_back_buffer(viaContextPtr vmesa);
extern void via_free_front_buffer(viaContextPtr vmesa);
extern GLboolean via_alloc_depth_buffer(viaContextPtr vmesa);
extern void via_free_depth_buffer(viaContextPtr vmesa);
extern GLboolean via_alloc_dma_buffer(viaContextPtr vmesa);
extern void via_free_dma_buffer(viaContextPtr vmesa);
extern GLboolean via_alloc_texture(viaContextPtr vmesa, viaTextureObjectPtr t);
/*=* John Sheng [2003.5.31]  agp tex *=*/
extern GLboolean via_alloc_texture_agp(viaContextPtr vmesa, viaTextureObjectPtr t);
extern void via_free_texture(viaContextPtr vmesa, viaTextureObjectPtr t);
#endif
