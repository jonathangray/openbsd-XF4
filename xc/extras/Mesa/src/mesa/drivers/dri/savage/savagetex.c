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


#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>

#include "mm.h"
#include "savagecontext.h"
#include "savagetex.h"
#include "savagetris.h"
#include "savageioctl.h"
#include "simple_list.h"
#include "enums.h"
#include "savage_bci.h"

#include "macros.h"
#include "texformat.h"
#include "texstore.h"
#include "texobj.h"

#include "swrast/swrast.h"

/* Size 1, 2 and 4 images are packed into the last subtile. Each image
 * is repeated to fill a 4x4 pixel area. The figure below shows the
 * layout of those 4x4 pixel areas in the 8x8 subtile.
 *
 *    4 2
 *    x 1
 *
 * Yuck! 8-bit texture formats use 4x8 subtiles. See below.
 */
static const savageTileInfo tileInfo_pro[5] = {
    {64, 64,  8, 8, 8, 8, {0x12, 0x02}}, /* 4-bit */
    {64, 32, 16, 4, 4, 8, {0x30, 0x20}}, /* 8-bit */
    {64, 16,  8, 2, 8, 8, {0x48, 0x08}}, /* 16-bit */
    { 0,  0,  0, 0, 0, 0, {0x00, 0x00}}, /* 24-bit */
    {32, 16,  4, 2, 8, 8, {0x90, 0x10}}, /* 32-bit */
};

/* Size 1, 2 and 4 images are packed into the last two subtiles. Each
 * image is repeated to fill a 4x4 pixel area. The figures below show
 * the layout of those 4x4 pixel areas in the two 4x8 subtiles.
 *
 * second last subtile: 4   last subtile: 2
 *                      x                 1
 */
static const savageTileInfo tileInfo_s3d_s4[5] = {
    {64, 64, 16, 8, 4, 8, {0x18, 0x10}}, /* 4-bit */
    {64, 32, 16, 4, 4, 8, {0x30, 0x20}}, /* 8-bit */
    {64, 16, 16, 2, 4, 8, {0x60, 0x40}}, /* 16-bit */
    { 0,  0,  0, 0, 0, 0, {0x00, 0x00}}, /* 24-bit */
    {32, 16,  8, 2, 4, 8, {0xc0, 0x80}}, /* 32-bit */
};

/** \brief Upload a complete tile from src (srcStride) to dest
 *
 * \param tileInfo     Pointer to tiling information
 * \param wInSub       Width of source/dest image in subtiles
 * \param hInSub       Height of source/dest image in subtiles
 * \param bpp          Bytes per pixel
 * \param src          Pointer to source data
 * \param srcStride    Byte stride of rows in the source data
 * \param dest         Pointer to destination
 *
 * Writes linearly to the destination memory in order to exploit write
 * combining.
 *
 * For a complete tile wInSub and hInSub are set to the same values as
 * in tileInfo. If the source image is smaller than a whole tile in
 * one or both dimensions then they are set to the values of the
 * source image. This only works as long as the source image is bigger
 * than 8x8 pixels.
 */
static void savageUploadTile (const savageTileInfo *tileInfo,
			      GLuint wInSub, GLuint hInSub, GLuint bpp,
			      GLubyte *src, GLuint srcStride, GLubyte *dest) {
    GLuint subStride = tileInfo->subWidth * bpp;
    GLubyte *srcSRow = src, *srcSTile = src;
    GLuint sx, sy, y;
    for (sy = 0; sy < hInSub; ++sy) {
	srcSTile = srcSRow;
	for (sx = 0; sx < wInSub; ++sx) {
	    src = srcSTile;
	    for (y = 0; y < tileInfo->subHeight; ++y) {
		memcpy (dest, src, subStride);
		src += srcStride;
		dest += subStride;
	    }
	    srcSTile += subStride;
	}
	srcSRow += srcStride * tileInfo->subHeight;
    }
}

/** \brief Upload a image that is smaller than 8 pixels in either dimension.
 *
 * \param tileInfo    Pointer to tiling information
 * \param width       Width of the image
 * \param height      Height of the image
 * \param bpp         Bytes per pixel
 * \param src         Pointer to source data
 * \param dest        Pointer to destination
 *
 * This function handles all the special cases that need to be taken
 * care off. The caller may need to call this function multiple times
 * with the destination offset in different ways since small texture
 * images must be repeated in order to fill a whole tile (or 4x4 for
 * the last 3 levels).
 *
 * FIXME: Repeating inside this function would be more efficient.
 */
static void savageUploadTiny (const savageTileInfo *tileInfo,
			      GLuint width, GLuint height, GLuint bpp,
			      GLubyte *src, GLubyte *dest) {
    GLuint size = MAX2(width, height);

    if (width > tileInfo->subWidth) { /* assert: height <= subtile height */
	GLuint wInSub = width / tileInfo->subWidth;
	GLuint srcStride = width * bpp;
	GLuint subStride = tileInfo->subWidth * bpp;
	GLuint subSkip = (tileInfo->subHeight - height) * subStride;
	GLubyte *srcSTile = src;
	GLuint sx, y;
	for (sx = 0; sx < wInSub; ++sx) {
	    src = srcSTile;
	    for (y = 0; y < height; ++y) {
		memcpy (dest, src, subStride);
		src += srcStride;
		dest += subStride;
	    }
	    dest += subSkip;
	    srcSTile += subStride;
	}
    } else if (size > 4) { /* a tile or less wide, except the last 3 levels */
	GLuint srcStride = width * bpp;
	GLuint subStride = tileInfo->subWidth * bpp;
	/* if the subtile width is 4 we have to skip every other subtile */
	GLuint subSkip = tileInfo->subWidth == 4 ?
	    subStride * tileInfo->subHeight : 0;
	GLuint y;
	for (y = 0; y < height; ++y) {
	    memcpy (dest, src, srcStride);
	    src += srcStride;
	    dest += subStride;
	    if ((y & 7) == 7)
		dest += subSkip;
	}
    } else { /* the last 3 mipmap levels */
	GLuint offset = (size <= 2 ? tileInfo->tinyOffset[size-1] : 0);
	GLuint subStride = tileInfo->subWidth * bpp;
	GLuint y;
	dest += offset;
	for (y = 0; y < height; ++y) {
	    memcpy (dest, src, bpp*width);
	    src += width * bpp;
	    dest += subStride;
	}
    }
}

/** \brief Upload an image from mesa's internal copy.
 */
static void savageUploadTexLevel( savageTextureObjectPtr t, int level )
{
    const struct gl_texture_image *image = t->image[level].image;
    const savageTileInfo *tileInfo = t->tileInfo;
    GLuint width = image->Width2, height = image->Height2;
    GLuint bpp = t->texelBytes;

    /* FIXME: Need triangle (rather than pixel) fallbacks to simulate
     * this using normal textured triangles.
     *
     * DO THIS IN DRIVER STATE MANAGMENT, not hardware state.
     */
    if(image->Border != 0) 
	fprintf (stderr, "Not supported texture border %d.\n",
		 (int) image->Border);

    if (width >= 8 && height >= tileInfo->subHeight) {
	if (width >= tileInfo->width && height >= tileInfo->height) {
	    GLuint wInTiles = width / tileInfo->width;
	    GLuint hInTiles = height / tileInfo->height;
	    GLubyte *srcTRow = image->Data, *src;
	    GLubyte *dest = (GLubyte *)(t->BufAddr + t->image[level].offset);
	    GLuint x, y;
	    for (y = 0; y < hInTiles; ++y) {
		src = srcTRow;
		for (x = 0; x < wInTiles; ++x) {
		    savageUploadTile (tileInfo,
				      tileInfo->wInSub, tileInfo->hInSub, bpp,
				      src, width * bpp, dest);
		    src += tileInfo->width * bpp;
		    dest += 2048; /* tile size is always 2k */
		}
		srcTRow += width * tileInfo->height * bpp;
	    }
	} else {
	    savageUploadTile (tileInfo, width / tileInfo->subWidth,
			      height / tileInfo->subHeight, bpp,
			      image->Data, width * bpp,
			      (GLubyte *)(t->BufAddr+t->image[level].offset));
	}
    } else {
	GLuint minHeight, minWidth, hRepeat, vRepeat, x, y;
	if (width > 4 || height > 4) {
	    minWidth = tileInfo->subWidth;
	    minHeight = tileInfo->subHeight;
	} else {
	    minWidth = 4;
	    minHeight = 4;
	}
	hRepeat = width  >= minWidth  ? 1 : minWidth  / width;
	vRepeat = height >= minHeight ? 1 : minHeight / height;
	for (y = 0; y < vRepeat; ++y) {
	    GLuint offset = y * tileInfo->subWidth*height * bpp;
	    for (x = 0; x < hRepeat; ++x) {
		savageUploadTiny (tileInfo, width, height, bpp, image->Data,
				  (GLubyte *)(t->BufAddr +
					      t->image[level].offset+offset));
		offset += width * bpp;
	    }
	}
    }
}

/** \brief Compute the destination size of a texture image
 */
static GLuint savageTexImageSize (GLuint width, GLuint height, GLuint bpp) {
    /* full subtiles */
    if (width >= 8 && height >= 8)
	return width * height * bpp;
    /* special case for the last three mipmap levels: the hardware computes
     * the offset internally */
    else if (width <= 4 && height <= 4)
	return 0;
    /* partially filled sub tiles waste memory
     * on Savage3D and Savage4 with subtile width 4 every other subtile is
     * skipped if width < 8 so we can assume a uniform subtile width of 8 */
    else if (width >= 8)
	return width * 8 * bpp;
    else if (height >= 8)
	return 8 * height * bpp;
    else
	return 64 * bpp;
}

static void savageSetTexWrapping(savageTextureObjectPtr tex, GLenum s, GLenum t)
{
    tex->texParams.sWrapMode = s;
    tex->texParams.tWrapMode = t;
}

static void savageSetTexFilter(savageTextureObjectPtr t, 
			       GLenum minf, GLenum magf)
{
   t->texParams.minFilter = minf;
   t->texParams.magFilter = magf;
}


/* Need a fallback ?
 */
static void savageSetTexBorderColor(savageTextureObjectPtr t, GLubyte color[4])
{
/*    t->Setup[SAVAGE_TEXREG_TEXBORDERCOL] =  */
      t->texParams.boarderColor = SAVAGEPACKCOLOR8888(color[0],color[1],color[2],color[3]); 
}



static savageTextureObjectPtr
savageAllocTexObj( struct gl_texture_object *texObj ) 
{
   savageTextureObjectPtr t;

   t = (savageTextureObjectPtr) calloc(1,sizeof(*t));
   texObj->DriverData = t;
   if ( t != NULL ) {

      /* Initialize non-image-dependent parts of the state:
       */
      t->globj = texObj;

      /* FIXME Something here to set initial values for other parts of
       * FIXME t->setup?
       */
  
      make_empty_list( t );

      savageSetTexWrapping(t,texObj->WrapS,texObj->WrapT);
      savageSetTexFilter(t,texObj->MinFilter,texObj->MagFilter);
      savageSetTexBorderColor(t,texObj->_BorderChan);
   }

   return t;
}

/* Called by the _mesa_store_teximage[123]d() functions. */
static const struct gl_texture_format *
savageChooseTextureFormat( GLcontext *ctx, GLint internalFormat,
			   GLenum format, GLenum type )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   const GLboolean do32bpt = GL_FALSE;
   const GLboolean force16bpt = GL_FALSE;
   const GLboolean isSavage4 = (imesa->savageScreen->chipset >= S3_SAVAGE4);
   (void) format;
   (void) type;

   switch ( internalFormat ) {
   case 4:
   case GL_RGBA:
   case GL_COMPRESSED_RGBA:
      switch ( type ) {
      case GL_UNSIGNED_INT_10_10_10_2:
      case GL_UNSIGNED_INT_2_10_10_10_REV:
	 return do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb1555;
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	 return &_mesa_texformat_argb4444;
      case GL_UNSIGNED_SHORT_5_5_5_1:
      case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	 return &_mesa_texformat_argb1555;
      default:
         return do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444;
      }

   case 3:
   case GL_RGB:
   case GL_COMPRESSED_RGB:
      switch ( type ) {
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	 return &_mesa_texformat_argb4444;
      case GL_UNSIGNED_SHORT_5_5_5_1:
      case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	 return &_mesa_texformat_argb1555;
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_5_6_5_REV:
	 return &_mesa_texformat_rgb565;
      default:
         return do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_rgb565;
      }

   case GL_RGBA8:
   case GL_RGBA12:
   case GL_RGBA16:
      return !force16bpt ?
	  &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444;

   case GL_RGB10_A2:
      return !force16bpt ?
	  &_mesa_texformat_argb8888 : &_mesa_texformat_argb1555;

   case GL_RGBA4:
   case GL_RGBA2:
      return &_mesa_texformat_argb4444;

   case GL_RGB5_A1:
      return &_mesa_texformat_argb1555;

   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      return !force16bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_rgb565;

   case GL_RGB5:
   case GL_RGB4:
   case GL_R3_G3_B2:
      return &_mesa_texformat_rgb565;

   case GL_ALPHA:
   case GL_COMPRESSED_ALPHA:
      return isSavage4 ? &_mesa_texformat_a8 : (
	 do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444);
   case GL_ALPHA4:
      return isSavage4 ? &_mesa_texformat_a8 : &_mesa_texformat_argb4444;
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      return isSavage4 ? &_mesa_texformat_a8 : (
	 !force16bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444);

   case 1:
   case GL_LUMINANCE:
   case GL_COMPRESSED_LUMINANCE:
      /* no alpha, but use argb1555 in 16bit case to get pure grey values */
      return isSavage4 ? &_mesa_texformat_l8 : (
	 do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb1555);
   case GL_LUMINANCE4:
      return isSavage4 ? &_mesa_texformat_l8 : &_mesa_texformat_argb1555;
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      return isSavage4 ? &_mesa_texformat_l8 : (
	 !force16bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb1555);

   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_COMPRESSED_LUMINANCE_ALPHA:
      /* Savage4 has a al44 texture format. But it's not supported by Mesa. */
      return do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444;
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
      return &_mesa_texformat_argb4444;
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      return !force16bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444;

   case GL_INTENSITY:
   case GL_COMPRESSED_INTENSITY:
      return isSavage4 ? &_mesa_texformat_i8 : (
	 do32bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444);
   case GL_INTENSITY4:
      return isSavage4 ? &_mesa_texformat_i8 : &_mesa_texformat_argb4444;
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      return isSavage4 ? &_mesa_texformat_i8 : (
	 !force16bpt ? &_mesa_texformat_argb8888 : &_mesa_texformat_argb4444);
/*
   case GL_COLOR_INDEX:
   case GL_COLOR_INDEX1_EXT:
   case GL_COLOR_INDEX2_EXT:
   case GL_COLOR_INDEX4_EXT:
   case GL_COLOR_INDEX8_EXT:
   case GL_COLOR_INDEX12_EXT:
   case GL_COLOR_INDEX16_EXT:
      return &_mesa_texformat_ci8;
*/
   default:
      _mesa_problem(ctx, "unexpected texture format in %s", __FUNCTION__);
      return NULL;
   }
}

static void savageSetTexImages( savageContextPtr imesa,
				const struct gl_texture_object *tObj )
{
   savageTextureObjectPtr t = (savageTextureObjectPtr) tObj->DriverData;
   struct gl_texture_image *image = tObj->Image[0][tObj->BaseLevel];
   GLuint offset, i, textureFormat, size;

   assert(t);
   assert(image);

   switch (image->TexFormat->MesaFormat) {
   case MESA_FORMAT_ARGB8888:
      textureFormat = TFT_ARGB8888;
      t->texelBytes = 4;
      break;
   case MESA_FORMAT_ARGB1555:
      textureFormat = TFT_ARGB1555;
      t->texelBytes = 2;
      break;
   case MESA_FORMAT_ARGB4444:
      textureFormat = TFT_ARGB4444;
      t->texelBytes = 2;
      break;
   case MESA_FORMAT_RGB565:
      textureFormat = TFT_RGB565;
      t->texelBytes = 2;
      break;
   case MESA_FORMAT_L8:
      textureFormat = TFT_L8;
      t->texelBytes = 1;
      break;
   case MESA_FORMAT_I8:
      textureFormat = TFT_I8;
      t->texelBytes = 1;
      break;
   case MESA_FORMAT_A8:
      textureFormat = TFT_A8;
      t->texelBytes = 1;
      break;
   default:
      _mesa_problem(imesa->glCtx, "Bad texture format in %s", __FUNCTION__);
      return;
   }

   /* Select tiling format depending on the chipset and bytes per texel */
   if (imesa->savageScreen->chipset <= S3_SAVAGE4)
       t->tileInfo = &tileInfo_s3d_s4[t->texelBytes];
   else
       t->tileInfo = &tileInfo_pro[t->texelBytes];

   /* Figure out the size now (and count the levels).  Upload won't be done
    * until later.
    */ 
   t->dirty_images = 0;
   offset = 0;
   size = 1;
   for ( i = 0 ; i < SAVAGE_TEX_MAXLEVELS && tObj->Image[0][i] ; i++ ) {
      image = tObj->Image[0][i];
      t->image[i].image = image;
      t->image[i].offset = offset;
      t->image[i].internalFormat = textureFormat;
      t->dirty_images |= (1<<i);
      size = savageTexImageSize (image->Width2, image->Height2,
				 t->texelBytes);
      offset += size;
   }

   t->totalSize = offset;
   /* the last three mipmap levels don't add to the offset. They are packed
    * into 64 pixels. */
   if (size == 0)
       t->totalSize += 64 * t->texelBytes;
   /* 2k-aligned */
   t->totalSize = (t->totalSize + 2047UL) & ~2047UL;
   t->max_level = i-1;
   t->min_level = 0;
}

void savageDestroyTexObj(savageContextPtr imesa, savageTextureObjectPtr t)
{
   if (!t) return;

   /* This is sad - need to sync *in case* we upload a texture
    * to this newly free memory...
    */
   if (t->MemBlock) {
      mmFreeMem(t->MemBlock);
      t->MemBlock = 0;

      if (t->age > imesa->dirtyAge)
	 imesa->dirtyAge = t->age;
   }

   if (t->globj)
      t->globj->DriverData = 0;

   remove_from_list(t);
   free(t);
}


static void savageSwapOutTexObj(savageContextPtr imesa, savageTextureObjectPtr t)
{
   if (t->MemBlock) {
      mmFreeMem(t->MemBlock);
      t->MemBlock = 0;      

      if (t->age > imesa->dirtyAge)
	 imesa->dirtyAge = t->age;
   }

   t->dirty_images = ~0;
   move_to_tail(&(imesa->SwappedOut), t);
}



void savagePrintLocalLRU( savageContextPtr imesa , GLuint heap) 
{
   savageTextureObjectPtr t;
   int sz = 1 << (imesa->savageScreen->logTextureGranularity[heap]);
   
   foreach( t, &imesa->TexObjList[heap] ) {
      if (!t->globj)
	 fprintf(stderr, "Placeholder %d at %x sz %x\n", 
		 t->MemBlock->ofs / sz,
		 t->MemBlock->ofs,
		 t->MemBlock->size);      
      else
	 fprintf(stderr, "Texture (bound %d) at %x sz %x\n", 
		 t->bound,
		 t->MemBlock->ofs,
		 t->MemBlock->size);      

   }
}

void savagePrintGlobalLRU( savageContextPtr imesa , GLuint heap)
{
   int i, j;

   drm_savage_tex_region_t *list = imesa->sarea->texList[heap];
   

   for (i = 0, j = SAVAGE_NR_TEX_REGIONS ; i < SAVAGE_NR_TEX_REGIONS ; i++) {
      fprintf(stderr, "list[%d] age %d next %d prev %d\n",
	      j, list[j].age, list[j].next, list[j].prev);
      j = list[j].next;
      if (j == SAVAGE_NR_TEX_REGIONS) break;
   }
   
   if (j != SAVAGE_NR_TEX_REGIONS)
      fprintf(stderr, "Loop detected in global LRU\n");
       for (i = 0 ; i < SAVAGE_NR_TEX_REGIONS ; i++) 
       {
          fprintf(stderr,"list[%d] age %d next %d prev %d\n",
          i, list[i].age, list[i].next, list[i].prev);
       }
}


void savageResetGlobalLRU( savageContextPtr imesa, GLuint heap )
{
    drm_savage_tex_region_t *list = imesa->sarea->texList[heap];
   int sz = 1 << imesa->savageScreen->logTextureGranularity[heap];
   int i;

   /* (Re)initialize the global circular LRU list.  The last element
    * in the array (SAVAGE_NR_TEX_REGIONS) is the sentinal.  Keeping it
    * at the end of the array allows it to be addressed rationally
    * when looking up objects at a particular location in texture
    * memory.  
    */
   for (i = 0 ; (i+1) * sz <= imesa->savageScreen->textureSize[heap]; i++) {
      list[i].prev = i-1;
      list[i].next = i+1;
      list[i].age = 0;
   }

   i--;
   list[0].prev = SAVAGE_NR_TEX_REGIONS;
   list[i].prev = i-1;
   list[i].next = SAVAGE_NR_TEX_REGIONS;
   list[SAVAGE_NR_TEX_REGIONS].prev = i;
   list[SAVAGE_NR_TEX_REGIONS].next = 0;
   imesa->sarea->texAge[heap] = 0;
}


static void savageUpdateTexLRU( savageContextPtr imesa, savageTextureObjectPtr t ) 
{
   int i;
   int heap = t->heap;
   int logsz = imesa->savageScreen->logTextureGranularity[heap];
   int start = t->MemBlock->ofs >> logsz;
   int end = (t->MemBlock->ofs + t->MemBlock->size - 1) >> logsz;
   drm_savage_tex_region_t *list = imesa->sarea->texList[heap];
   
   imesa->texAge[heap] = ++imesa->sarea->texAge[heap];

   /* Update our local LRU
    */
   move_to_head( &(imesa->TexObjList[heap]), t );

   /* Update the global LRU
    */
   for (i = start ; i <= end ; i++) {

      list[i].in_use = 1;
      list[i].age = imesa->texAge[heap];

      /* remove_from_list(i)
       */
      list[(unsigned)list[i].next].prev = list[i].prev;
      list[(unsigned)list[i].prev].next = list[i].next;
      
      /* insert_at_head(list, i)
       */
      list[i].prev = SAVAGE_NR_TEX_REGIONS;
      list[i].next = list[SAVAGE_NR_TEX_REGIONS].next;
      list[(unsigned)list[SAVAGE_NR_TEX_REGIONS].next].prev = i;
      list[SAVAGE_NR_TEX_REGIONS].next = i;
   }
}


/* Called for every shared texture region which has increased in age
 * since we last held the lock.
 *
 * Figures out which of our textures have been ejected by other clients,
 * and pushes a placeholder texture onto the LRU list to represent 
 * the other client's textures.  
 */
void savageTexturesGone( savageContextPtr imesa,
		       GLuint heap,
		       GLuint offset, 
		       GLuint size,
		       GLuint in_use ) 
{
   savageTextureObjectPtr t, tmp;
   
   foreach_s ( t, tmp, &imesa->TexObjList[heap] ) {

      if (t->MemBlock->ofs >= offset + size ||
	  t->MemBlock->ofs + t->MemBlock->size <= offset)
	 continue;

      /* It overlaps - kick it off.  Need to hold onto the currently bound
       * objects, however.
       */
      if (t->bound)
	 savageSwapOutTexObj( imesa, t );
      else
	 savageDestroyTexObj( imesa, t );
   }

   
   if (in_use) {
      t = (savageTextureObjectPtr) calloc(1,sizeof(*t));
      if (!t) return;

      t->heap = heap;
      t->MemBlock = mmAllocMem( imesa->texHeap[heap], size, 0, offset);      
      if(!t->MemBlock)
      {
          free(t);
          return;
      }
      insert_at_head( &imesa->TexObjList[heap], t );
   }
}





/* This is called with the lock held.  May have to eject our own and/or
 * other client's texture objects to make room for the upload.
 */
int savageUploadTexImages( savageContextPtr imesa, savageTextureObjectPtr t )
{
   int heap;
   int i;
   int ofs;
   
   heap = t->heap = SAVAGE_CARD_HEAP;

   /* Do we need to eject LRU texture objects?
    */
   if (!t->MemBlock) {
      while (1)
      {
	 t->MemBlock = mmAllocMem( imesa->texHeap[heap], t->totalSize, 12, 0 ); 
	 if (t->MemBlock)
	    break;
	 else
	 {
	     heap = t->heap = SAVAGE_AGP_HEAP;
	     t->MemBlock = mmAllocMem( imesa->texHeap[heap], t->totalSize, 12, 0 ); 
	     
	     if (t->MemBlock)
	         break;
	 }

	 if (imesa->TexObjList[heap].prev->bound) {
  	    fprintf(stderr, "Hit bound texture in upload\n"); 
	    savagePrintLocalLRU( imesa,heap );
	    return -1;
	 }

	 if (imesa->TexObjList[heap].prev == &(imesa->TexObjList[heap])) {
 	    fprintf(stderr, "Failed to upload texture, sz %d\n", t->totalSize);
	    mmDumpMemInfo( imesa->texHeap[heap] );
	    return -1;
	 }
	 
	 savageSwapOutTexObj( imesa, imesa->TexObjList[heap].prev );
      }
 
      ofs = t->MemBlock->ofs;
      t->texParams.hwPhysAddress = imesa->savageScreen->textureOffset[heap] + ofs;
      t->BufAddr = (char *)((GLuint) imesa->savageScreen->texVirtual[heap] + ofs);
      imesa->dirty |= SAVAGE_UPLOAD_CTX;
   }

   /* Let the world know we've used this memory recently.
    */
   savageUpdateTexLRU( imesa, t );

   if (t->dirty_images) {
      LOCK_HARDWARE(imesa);
      savageFlushVerticesLocked (imesa);
      savageDmaFinish (imesa);
      if (SAVAGE_DEBUG & DEBUG_VERBOSE_LRU)
	 fprintf(stderr, "*");

      for (i = t->min_level ; i <= t->max_level ; i++)
	 if (t->dirty_images & (1<<i)) 
	    savageUploadTexLevel( t, i );
      UNLOCK_HARDWARE(imesa);
   }


   t->dirty_images = 0;
   return 0;
}

static void savageTexSetUnit( savageTextureObjectPtr t, GLuint unit )
{
   if (t->current_unit == unit) return;

   t->current_unit = unit;
}




static void savageUpdateTex0State_s4( GLcontext *ctx )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   struct gl_texture_object	*tObj;
   savageTextureObjectPtr t;
   GLuint format;

   /* disable */
   if (ctx->Texture.Unit[0]._ReallyEnabled == 0) {
      imesa->regs.s4.texDescr.ni.tex0En = GL_FALSE;
      imesa->regs.s4.texBlendCtrl[0].ui = TBC_NoTexMap;
      imesa->regs.s4.texCtrl[0].ui = 0x20f040;
      imesa->regs.s4.texAddr[0].ui = 0;
      return;
   }

   tObj = ctx->Texture.Unit[0]._Current;
   if (ctx->Texture.Unit[0]._ReallyEnabled != TEXTURE_2D_BIT ||
       tObj->Image[0][tObj->BaseLevel]->Border > 0) {
      /* 1D or 3D texturing enabled, or texture border - fallback */
      FALLBACK (ctx, SAVAGE_FALLBACK_TEXTURE, GL_TRUE);
      return;
   }

   /* Do 2D texture setup */

   t = tObj->DriverData;
   if (!t) {
      t = savageAllocTexObj( tObj );
      if (!t)
         return;
   }

   if (t->current_unit != 0)
      savageTexSetUnit( t, 0 );
    
   imesa->CurrentTexObj[0] = t;
   t->bound |= 1;

   if (t->dirty_images) {
       savageSetTexImages(imesa, tObj);
       savageUploadTexImages(imesa, imesa->CurrentTexObj[0]); 
   }
   
   if (t->MemBlock)
      savageUpdateTexLRU( imesa, t );

   format = tObj->Image[0][tObj->BaseLevel]->Format;

   switch (ctx->Texture.Unit[0].EnvMode) {
   case GL_REPLACE:
      imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_FALSE;
      switch(format)
      {
          case GL_LUMINANCE:
          case GL_RGB:
               imesa->regs.s4.texBlendCtrl[0].ui = TBC_Decal;
               break;

          case GL_LUMINANCE_ALPHA:
          case GL_RGBA:
          case GL_INTENSITY:
               imesa->regs.s4.texBlendCtrl[0].ui = TBC_Copy;
               break;

          case GL_ALPHA:
               imesa->regs.s4.texBlendCtrl[0].ui = TBC_CopyAlpha;
               break;
      }
       __HWEnvCombineSingleUnitScale(imesa, 0, 0,
				     &imesa->regs.s4.texBlendCtrl[0]);
      break;

    case GL_DECAL:
        imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_FALSE;
        switch (format)
        {
            case GL_RGB:
            case GL_LUMINANCE:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_Decal;
                break;

            case GL_RGBA:
            case GL_INTENSITY:
            case GL_LUMINANCE_ALPHA:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_DecalAlpha;
                break;

            /*
             GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_ALPHA, GL_INTENSITY
             are undefined with GL_DECAL
            */

            case GL_ALPHA:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_CopyAlpha;
                break;
        }
        __HWEnvCombineSingleUnitScale(imesa, 0, 0,
				      &imesa->regs.s4.texBlendCtrl[0]);
        break;

    case GL_MODULATE:
        imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_FALSE;
        imesa->regs.s4.texBlendCtrl[0].ui = TBC_ModulAlpha;
        __HWEnvCombineSingleUnitScale(imesa, 0, 0,
				      &imesa->regs.s4.texBlendCtrl[0]);
        break;

    case GL_BLEND:

        switch (format)
        {
            case GL_ALPHA:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_ModulAlpha;
                imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_FALSE;
                break;

            case GL_LUMINANCE:
            case GL_RGB:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_Blend0;
                imesa->regs.s4.texDescr.ni.tex1En = GL_TRUE;
                imesa->regs.s4.texDescr.ni.texBLoopEn = GL_TRUE;
                imesa->regs.s4.texDescr.ni.tex1Width  =
		    imesa->regs.s4.texDescr.ni.tex0Width;
                imesa->regs.s4.texDescr.ni.tex1Height =
		    imesa->regs.s4.texDescr.ni.tex0Height;
                imesa->regs.s4.texDescr.ni.tex1Fmt =
		    imesa->regs.s4.texDescr.ni.tex0Fmt;

		imesa->regs.s4.texAddr[1].ui = imesa->regs.s4.texAddr[0].ui;
		imesa->regs.s4.texBlendCtrl[1].ui = TBC_Blend1;

                imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_TRUE;
                imesa->bTexEn1 = GL_TRUE;
                break;

            case GL_LUMINANCE_ALPHA:
            case GL_RGBA:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_BlendAlpha0;
                imesa->regs.s4.texDescr.ni.tex1En = GL_TRUE;
                imesa->regs.s4.texDescr.ni.texBLoopEn = GL_TRUE;
                imesa->regs.s4.texDescr.ni.tex1Width  =
		    imesa->regs.s4.texDescr.ni.tex0Width;
                imesa->regs.s4.texDescr.ni.tex1Height =
		    imesa->regs.s4.texDescr.ni.tex0Height;
                imesa->regs.s4.texDescr.ni.tex1Fmt =
		    imesa->regs.s4.texDescr.ni.tex0Fmt;

		imesa->regs.s4.texAddr[1].ui = imesa->regs.s4.texAddr[0].ui;
		imesa->regs.s4.texBlendCtrl[1].ui = TBC_BlendAlpha1;

                imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_TRUE;
                imesa->bTexEn1 = GL_TRUE;
                break;

            case GL_INTENSITY:
                imesa->regs.s4.texBlendCtrl[0].ui = TBC_BlendInt0;
                imesa->regs.s4.texDescr.ni.tex1En = GL_TRUE;
                imesa->regs.s4.texDescr.ni.texBLoopEn = GL_TRUE;
                imesa->regs.s4.texDescr.ni.tex1Width  =
		    imesa->regs.s4.texDescr.ni.tex0Width;
                imesa->regs.s4.texDescr.ni.tex1Height =
		    imesa->regs.s4.texDescr.ni.tex0Height;
                imesa->regs.s4.texDescr.ni.tex1Fmt =
		    imesa->regs.s4.texDescr.ni.tex0Fmt;

		imesa->regs.s4.texAddr[1].ui = imesa->regs.s4.texAddr[0].ui;
		imesa->regs.s4.texBlendCtrl[1].ui = TBC_BlendInt1;

                imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_TRUE;
                imesa->regs.s4.texCtrl[0].ni.alphaArg1Invert = GL_TRUE;
                imesa->bTexEn1 = GL_TRUE;
                break;
        }
        __HWEnvCombineSingleUnitScale(imesa, 0, 0,
				      &imesa->regs.s4.texBlendCtrl[0]);
        break;

        /*
         GL_ADD
        */
    case GL_ADD:
        printf("Add\n");
        imesa->regs.s4.texCtrl[0].ni.clrArg1Invert = GL_FALSE;
        imesa->regs.s4.texBlendCtrl[0].ui = TBC_AddAlpha;
        __HWEnvCombineSingleUnitScale(imesa, 0, 0,
				      &imesa->regs.s4.texBlendCtrl[0]);
        break;

#if GL_ARB_texture_env_combine
    case GL_COMBINE_ARB:
        __HWParseTexEnvCombine(imesa, 0, &imesa->regs.s4.texCtrl[0],
			       &imesa->regs.s4.texBlendCtrl[0]);
        break;
#endif

   default:
      fprintf(stderr, "unknown tex env mode");
      exit(1);
      break;			
   }

    imesa->regs.s4.texCtrl[0].ni.uMode = !(t->texParams.sWrapMode & 0x01);
    imesa->regs.s4.texCtrl[0].ni.vMode = !(t->texParams.tWrapMode & 0x01);

    switch (t->texParams.minFilter)
    {
        case GL_NEAREST:
            imesa->regs.s4.texCtrl[0].ni.filterMode   = TFM_Point;
            imesa->regs.s4.texCtrl[0].ni.mipmapEnable = GL_FALSE;
            break;

        case GL_LINEAR:
            imesa->regs.s4.texCtrl[0].ni.filterMode   = TFM_Bilin;
            imesa->regs.s4.texCtrl[0].ni.mipmapEnable = GL_FALSE;
            break;

        case GL_NEAREST_MIPMAP_NEAREST:
            imesa->regs.s4.texCtrl[0].ni.filterMode   = TFM_Point;
            imesa->regs.s4.texCtrl[0].ni.mipmapEnable = GL_TRUE;
            break;

        case GL_LINEAR_MIPMAP_NEAREST:
            imesa->regs.s4.texCtrl[0].ni.filterMode   = TFM_Bilin;
            imesa->regs.s4.texCtrl[0].ni.mipmapEnable = GL_TRUE;
            break;

        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
            imesa->regs.s4.texCtrl[0].ni.filterMode   = TFM_Trilin;
            imesa->regs.s4.texCtrl[0].ni.mipmapEnable = GL_TRUE;
            break;
    }

    if((ctx->Texture.Unit[0].LodBias !=0.0F) ||
       (imesa->regs.s4.texCtrl[0].ni.dBias != 0))
    {
	int bias = (int)(ctx->Texture.Unit[0].LodBias * 32.0);
	if (bias < -256)
	    bias = -256;
	else if (bias > 255)
	    bias = 255;
	imesa->regs.s4.texCtrl[0].ni.dBias = bias & 0x1ff;
    }

    imesa->regs.s4.texDescr.ni.tex0En = GL_TRUE;
    imesa->regs.s4.texDescr.ni.tex0Width  = t->image[0].image->WidthLog2;
    imesa->regs.s4.texDescr.ni.tex0Height = t->image[0].image->HeightLog2;
    imesa->regs.s4.texDescr.ni.tex0Fmt = t->image[0].internalFormat;
    imesa->regs.s4.texCtrl[0].ni.dMax = t->max_level;

    if (imesa->regs.s4.texDescr.ni.tex1En)
        imesa->regs.s4.texDescr.ni.texBLoopEn = GL_TRUE;

    imesa->regs.s4.texAddr[0].ui = (uint32_t) t->texParams.hwPhysAddress | 0x2;
    if(t->heap == SAVAGE_AGP_HEAP)
	imesa->regs.s4.texAddr[0].ui |= 0x1;
    
    return;
}
static void savageUpdateTex1State_s4( GLcontext *ctx )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   struct gl_texture_object	*tObj;
   savageTextureObjectPtr t;
   GLuint format;

   /* disable */
   if(imesa->bTexEn1)
   {
       imesa->bTexEn1 = GL_FALSE;
       return;
   }

   if (ctx->Texture.Unit[1]._ReallyEnabled == 0) {
      imesa->regs.s4.texDescr.ni.tex1En = GL_FALSE;
      imesa->regs.s4.texBlendCtrl[1].ui = TBC_NoTexMap1;
      imesa->regs.s4.texCtrl[1].ui = 0x20f040;
      imesa->regs.s4.texAddr[1].ui = 0;
      imesa->regs.s4.texDescr.ni.texBLoopEn = GL_FALSE;
      return;
   }

   tObj = ctx->Texture.Unit[1]._Current;

   if (ctx->Texture.Unit[1]._ReallyEnabled != TEXTURE_2D_BIT ||
       tObj->Image[0][tObj->BaseLevel]->Border > 0) {
      /* 1D or 3D texturing enabled, or texture border - fallback */
      FALLBACK (ctx, SAVAGE_FALLBACK_TEXTURE, GL_TRUE);
      return;
   }

   /* Do 2D texture setup */

   t = tObj->DriverData;
   if (!t) {
      t = savageAllocTexObj( tObj );
      if (!t)
         return;
   }
    
   if (t->current_unit != 1)
      savageTexSetUnit( t, 1 );

   imesa->CurrentTexObj[1] = t;

   t->bound |= 2;

   if (t->dirty_images) {
       savageSetTexImages(imesa, tObj);
       savageUploadTexImages(imesa, imesa->CurrentTexObj[1]);
   }
   
   if (t->MemBlock)
      savageUpdateTexLRU( imesa, t );

   format = tObj->Image[0][tObj->BaseLevel]->Format;

   switch (ctx->Texture.Unit[1].EnvMode) {
   case GL_REPLACE:
        imesa->regs.s4.texCtrl[1].ni.clrArg1Invert = GL_FALSE;
        switch (format)
        {
            case GL_LUMINANCE:
            case GL_RGB:
                imesa->regs.s4.texBlendCtrl[1].ui = TBC_Decal;
                break;

            case GL_LUMINANCE_ALPHA:
            case GL_INTENSITY:
            case GL_RGBA:
                imesa->regs.s4.texBlendCtrl[1].ui = TBC_Copy;
                break;

            case GL_ALPHA:
                imesa->regs.s4.texBlendCtrl[1].ui = TBC_CopyAlpha1;
                break;
        }
        __HWEnvCombineSingleUnitScale(imesa, 0, 1, &imesa->regs.s4.texBlendCtrl);
      break;
   case GL_MODULATE:
       imesa->regs.s4.texCtrl[1].ni.clrArg1Invert = GL_FALSE;
       imesa->regs.s4.texBlendCtrl[1].ui = TBC_ModulAlpha1;
       __HWEnvCombineSingleUnitScale(imesa, 0, 1, &imesa->regs.s4.texBlendCtrl);
       break;

/*#if GL_EXT_texture_env_add*/
    case GL_ADD:
        imesa->regs.s4.texCtrl[1].ni.clrArg1Invert = GL_FALSE;
        imesa->regs.s4.texBlendCtrl[1].ui = TBC_AddAlpha1;
        __HWEnvCombineSingleUnitScale(imesa, 0, 1, &imesa->regs.s4.texBlendCtrl);
        break;
/*#endif*/

#if GL_ARB_texture_env_combine
    case GL_COMBINE_ARB:
        __HWParseTexEnvCombine(imesa, 1, &texCtrl, &imesa->regs.s4.texBlendCtrl);
        break;
#endif

   case GL_DECAL:
        imesa->regs.s4.texCtrl[1].ni.clrArg1Invert = GL_FALSE;

        switch (format)
        {
            case GL_LUMINANCE:
            case GL_RGB:
                imesa->regs.s4.texBlendCtrl[1].ui = TBC_Decal1;
                break;
            case GL_LUMINANCE_ALPHA:
            case GL_INTENSITY:
            case GL_RGBA:
                imesa->regs.s4.texBlendCtrl[1].ui = TBC_DecalAlpha1;
                break;

                /*
                // GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_ALPHA, GL_INTENSITY
                // are undefined with GL_DECAL
                */
            case GL_ALPHA:
                imesa->regs.s4.texBlendCtrl[1].ui = TBC_CopyAlpha1;
                break;
        }
        __HWEnvCombineSingleUnitScale(imesa, 0, 1, &imesa->regs.s4.texBlendCtrl);
        break;

   case GL_BLEND:
        if (format == GL_LUMINANCE)
        {
            /*
            // This is a hack for GLQuake, invert.
            */
            imesa->regs.s4.texCtrl[1].ni.clrArg1Invert = GL_TRUE;
            imesa->regs.s4.texBlendCtrl[1].ui = 0;
        }
        __HWEnvCombineSingleUnitScale(imesa, 0, 1, &imesa->regs.s4.texBlendCtrl);
      break;

   default:
      fprintf(stderr, "unkown tex 1 env mode\n");
      exit(1);
      break;			
   }

    imesa->regs.s4.texCtrl[1].ni.uMode = !(t->texParams.sWrapMode & 0x01);
    imesa->regs.s4.texCtrl[1].ni.vMode = !(t->texParams.tWrapMode & 0x01);

    switch (t->texParams.minFilter)
    {
        case GL_NEAREST:
            imesa->regs.s4.texCtrl[1].ni.filterMode   = TFM_Point;
            imesa->regs.s4.texCtrl[1].ni.mipmapEnable = GL_FALSE;
            break;

        case GL_LINEAR:
            imesa->regs.s4.texCtrl[1].ni.filterMode   = TFM_Bilin;
            imesa->regs.s4.texCtrl[1].ni.mipmapEnable = GL_FALSE;
            break;

        case GL_NEAREST_MIPMAP_NEAREST:
            imesa->regs.s4.texCtrl[1].ni.filterMode   = TFM_Point;
            imesa->regs.s4.texCtrl[1].ni.mipmapEnable = GL_TRUE;
            break;

        case GL_LINEAR_MIPMAP_NEAREST:
            imesa->regs.s4.texCtrl[1].ni.filterMode   = TFM_Bilin;
            imesa->regs.s4.texCtrl[1].ni.mipmapEnable = GL_TRUE;
            break;

        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
            imesa->regs.s4.texCtrl[1].ni.filterMode   = TFM_Trilin;
            imesa->regs.s4.texCtrl[1].ni.mipmapEnable = GL_TRUE;
            break;
    }
    
    if((ctx->Texture.Unit[1].LodBias !=0.0F) ||
       (imesa->regs.s4.texCtrl[1].ni.dBias != 0))
    {
	int bias = (int)(ctx->Texture.Unit[1].LodBias * 32.0);
	if (bias < -256)
	    bias = -256;
	else if (bias > 255)
	    bias = 255;
	imesa->regs.s4.texCtrl[1].ni.dBias = bias & 0x1ff;
    }

    imesa->regs.s4.texDescr.ni.tex1En = GL_TRUE;
    imesa->regs.s4.texDescr.ni.tex1Width  = t->image[0].image->WidthLog2;
    imesa->regs.s4.texDescr.ni.tex1Height = t->image[0].image->HeightLog2;
    imesa->regs.s4.texDescr.ni.tex1Fmt = t->image[0].internalFormat;
    imesa->regs.s4.texCtrl[1].ni.dMax = t->max_level;
    imesa->regs.s4.texDescr.ni.texBLoopEn = GL_TRUE;

    imesa->regs.s4.texAddr[1].ui = (uint32_t) t->texParams.hwPhysAddress| 2;
    if(t->heap == SAVAGE_AGP_HEAP)
	imesa->regs.s4.texAddr[1].ui |= 0x1;
}
static void savageUpdateTexState_s3d( GLcontext *ctx )
{
    savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
    struct gl_texture_object *tObj;
    savageTextureObjectPtr t;
    GLuint format;

    /* disable */
    if (ctx->Texture.Unit[0]._ReallyEnabled == 0) {
	imesa->regs.s3d.texCtrl.ui = 0;
	imesa->regs.s3d.texCtrl.ni.texEn = GL_FALSE;
	imesa->regs.s3d.texCtrl.ni.dBias = 0x08;
	imesa->regs.s3d.texCtrl.ni.texXprEn = GL_TRUE;
	imesa->regs.s3d.texAddr.ui = 0;
	return;
    }

    tObj = ctx->Texture.Unit[0]._Current;
    if (ctx->Texture.Unit[0]._ReallyEnabled != TEXTURE_2D_BIT ||
	tObj->Image[0][tObj->BaseLevel]->Border > 0) {
	/* 1D or 3D texturing enabled, or texture border - fallback */
	FALLBACK (ctx, SAVAGE_FALLBACK_TEXTURE, GL_TRUE);
	return;
    }

    /* Do 2D texture setup */
    t = tObj->DriverData;
    if (!t) {
	t = savageAllocTexObj( tObj );
	if (!t)
	    return;
    }

    if (t->current_unit != 0)
	savageTexSetUnit( t, 0 );

    imesa->CurrentTexObj[0] = t;
    t->bound |= 1;

    if (t->dirty_images) {
	savageSetTexImages(imesa, tObj);
	savageUploadTexImages(imesa, imesa->CurrentTexObj[0]); 
    }

    if (t->MemBlock)
	savageUpdateTexLRU( imesa, t );

    format = tObj->Image[0][tObj->BaseLevel]->Format;

    /* FIXME: copied from utah-glx, probably needs some tuning */
    switch (ctx->Texture.Unit[0].EnvMode) {
    case GL_DECAL:
	imesa->regs.s3d.drawCtrl.ni.texBlendCtrl = SAVAGETBC_DECAL_S3D;
	break;
    case GL_REPLACE:
	imesa->regs.s3d.drawCtrl.ni.texBlendCtrl = SAVAGETBC_COPY_S3D;
	break;
    case GL_BLEND: /* FIXIT */
    case GL_MODULATE:
	imesa->regs.s3d.drawCtrl.ni.texBlendCtrl = SAVAGETBC_MODULATEALPHA_S3D;
	break;
    default:
	fprintf(stderr, "unkown tex env mode\n");
	/*exit(1);*/
	break;			
    }

    imesa->regs.s3d.drawCtrl.ni.flushPdDestWrites = GL_TRUE;
    imesa->regs.s3d.drawCtrl.ni.flushPdZbufWrites = GL_TRUE;

    /* FIXME: this is how the utah-driver works. I doubt it's the ultimate 
       truth. */
    imesa->regs.s3d.texCtrl.ni.uWrapEn = 0;
    imesa->regs.s3d.texCtrl.ni.vWrapEn = 0;
    if (t->texParams.sWrapMode == GL_CLAMP)
	imesa->regs.s3d.texCtrl.ni.wrapMode = TAM_Clamp;
    else
	imesa->regs.s3d.texCtrl.ni.wrapMode = TAM_Wrap;

    switch (t->texParams.minFilter) {
    case GL_NEAREST:
	imesa->regs.s3d.texCtrl.ni.filterMode    = TFM_Point;
	imesa->regs.s3d.texCtrl.ni.mipmapDisable = GL_TRUE;
	break;

    case GL_LINEAR:
	imesa->regs.s3d.texCtrl.ni.filterMode    = TFM_Bilin;
	imesa->regs.s3d.texCtrl.ni.mipmapDisable = GL_TRUE;
	break;

    case GL_NEAREST_MIPMAP_NEAREST:
	imesa->regs.s3d.texCtrl.ni.filterMode    = TFM_Point;
	imesa->regs.s3d.texCtrl.ni.mipmapDisable = GL_FALSE;
	break;

    case GL_LINEAR_MIPMAP_NEAREST:
	imesa->regs.s3d.texCtrl.ni.filterMode    = TFM_Bilin;
	imesa->regs.s3d.texCtrl.ni.mipmapDisable = GL_FALSE;
	break;

    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:
	imesa->regs.s3d.texCtrl.ni.filterMode    = TFM_Trilin;
	imesa->regs.s3d.texCtrl.ni.mipmapDisable = GL_FALSE;
	break;
    }

    /* There is no way to specify a maximum mipmap level. We may have to
       disable mipmapping completely. */
    /*
    if (t->max_level < t->image[0].image->WidthLog2 ||
	t->max_level < t->image[0].image->HeightLog2) {
	texCtrl.ni.mipmapEnable = GL_TRUE;
	if (texCtrl.ni.filterMode == TFM_Trilin)
	    texCtrl.ni.filterMode = TFM_Bilin;
	texCtrl.ni.filterMode = TFM_Point;
    }
    */

    if((ctx->Texture.Unit[0].LodBias !=0.0F) ||
       (imesa->regs.s3d.texCtrl.ni.dBias != 0))
    {
	int bias = (int)(ctx->Texture.Unit[0].LodBias * 16.0);
	if (bias < -256)
	    bias = -256;
	else if (bias > 255)
	    bias = 255;
	imesa->regs.s3d.texCtrl.ni.dBias = bias & 0x1ff;
    }

    imesa->regs.s3d.texCtrl.ni.texEn = GL_TRUE;
    imesa->regs.s3d.texDescr.ni.texWidth  = t->image[0].image->WidthLog2;
    imesa->regs.s3d.texDescr.ni.texHeight = t->image[0].image->HeightLog2;
    assert (t->image[0].internalFormat <= 7);
    imesa->regs.s3d.texDescr.ni.texFmt = t->image[0].internalFormat;

    imesa->regs.s3d.texAddr.ui = (uint32_t) t->texParams.hwPhysAddress| 2;
    if(t->heap == SAVAGE_AGP_HEAP)
	imesa->regs.s3d.texAddr.ui |= 0x1;
}



static void savageUpdateTextureState_s4( GLcontext *ctx )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   if (imesa->CurrentTexObj[0]) imesa->CurrentTexObj[0]->bound &= ~1;
   if (imesa->CurrentTexObj[1]) imesa->CurrentTexObj[1]->bound &= ~2;
   imesa->CurrentTexObj[0] = 0;
   imesa->CurrentTexObj[1] = 0;   
   FALLBACK (ctx, SAVAGE_FALLBACK_TEXTURE, GL_FALSE);
   savageUpdateTex0State_s4( ctx );
   savageUpdateTex1State_s4( ctx );
   imesa->dirty |= (SAVAGE_UPLOAD_CTX |
		    SAVAGE_UPLOAD_TEX0 | 
		    SAVAGE_UPLOAD_TEX1);
}
static void savageUpdateTextureState_s3d( GLcontext *ctx )
{
    savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
    if (imesa->CurrentTexObj[0]) imesa->CurrentTexObj[0]->bound &= ~1;
    imesa->CurrentTexObj[0] = 0;
    if (ctx->Texture.Unit[1]._ReallyEnabled) {
	FALLBACK (ctx, SAVAGE_FALLBACK_TEXTURE, GL_TRUE);
    } else {
	FALLBACK (ctx, SAVAGE_FALLBACK_TEXTURE, GL_FALSE);
	savageUpdateTexState_s3d( ctx );
	imesa->dirty |= (SAVAGE_UPLOAD_CTX |
			 SAVAGE_UPLOAD_TEX0);
    }
}
void savageUpdateTextureState( GLcontext *ctx)
{
    savageContextPtr imesa = SAVAGE_CONTEXT( ctx );
    if (imesa->savageScreen->chipset >= S3_SAVAGE4)
	savageUpdateTextureState_s4 (ctx);
    else
	savageUpdateTextureState_s3d (ctx);
}



/*****************************************
 * DRIVER functions
 *****************************************/

static void savageTexEnv( GLcontext *ctx, GLenum target, 
			GLenum pname, const GLfloat *param )
{
   savageContextPtr imesa = SAVAGE_CONTEXT( ctx );

   if (pname == GL_TEXTURE_ENV_MODE) {

      imesa->new_state |= SAVAGE_NEW_TEXTURE;

   } else if (pname == GL_TEXTURE_ENV_COLOR) {

      struct gl_texture_unit *texUnit = 
	 &ctx->Texture.Unit[ctx->Texture.CurrentUnit];
      const GLfloat *fc = texUnit->EnvColor;
      GLuint r, g, b, a, col;
      CLAMPED_FLOAT_TO_UBYTE(r, fc[0]);
      CLAMPED_FLOAT_TO_UBYTE(g, fc[1]);
      CLAMPED_FLOAT_TO_UBYTE(b, fc[2]);
      CLAMPED_FLOAT_TO_UBYTE(a, fc[3]);

      col = ((a << 24) | 
	     (r << 16) | 
	     (g <<  8) | 
	     (b <<  0));
    

   } 
}

static void savageTexImage2D( GLcontext *ctx, GLenum target, GLint level,
			      GLint internalFormat,
			      GLint width, GLint height, GLint border,
			      GLenum format, GLenum type, const GLvoid *pixels,
			      const struct gl_pixelstore_attrib *packing,
			      struct gl_texture_object *texObj,
			      struct gl_texture_image *texImage )
{
   savageTextureObjectPtr t = (savageTextureObjectPtr) texObj->DriverData;
   if (t) {
      savageSwapOutTexObj( SAVAGE_CONTEXT(ctx), t );
   } else {
      t = savageAllocTexObj(texObj);
      if (!t) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glTexImage2D");
         return;
      }
   }
   _mesa_store_teximage2d( ctx, target, level, internalFormat,
			   width, height, border, format, type,
			   pixels, packing, texObj, texImage );
   t->dirty_images |= (1 << level);
   SAVAGE_CONTEXT(ctx)->new_state |= SAVAGE_NEW_TEXTURE;
}

static void savageTexSubImage2D( GLcontext *ctx, 
				 GLenum target,
				 GLint level,	
				 GLint xoffset, GLint yoffset,
				 GLsizei width, GLsizei height,
				 GLenum format, GLenum type,
				 const GLvoid *pixels,
				 const struct gl_pixelstore_attrib *packing,
				 struct gl_texture_object *texObj,
				 struct gl_texture_image *texImage )
{
   savageTextureObjectPtr t = (savageTextureObjectPtr) texObj->DriverData;
   assert( t ); /* this _should_ be true */
   if (t) {
      savageSwapOutTexObj( SAVAGE_CONTEXT(ctx), t );
   } else {
      t = savageAllocTexObj(texObj);
      if (!t) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glTexImage2D");
         return;
      }
   }
   _mesa_store_texsubimage2d(ctx, target, level, xoffset, yoffset, width, 
			     height, format, type, pixels, packing, texObj,
			     texImage);
   t->dirty_images |= (1 << level);
   SAVAGE_CONTEXT(ctx)->new_state |= SAVAGE_NEW_TEXTURE;
}

static void savageTexParameter( GLcontext *ctx, GLenum target,
			      struct gl_texture_object *tObj,
			      GLenum pname, const GLfloat *params )
{
   savageTextureObjectPtr t = (savageTextureObjectPtr) tObj->DriverData;
   savageContextPtr imesa = SAVAGE_CONTEXT( ctx );

   if (!t || target != GL_TEXTURE_2D)
      return;

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
      savageSetTexFilter(t,tObj->MinFilter,tObj->MagFilter);
      break;

   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
      savageSetTexWrapping(t,tObj->WrapS,tObj->WrapT);
      break;
  
   case GL_TEXTURE_BORDER_COLOR:
      savageSetTexBorderColor(t,tObj->_BorderChan);
      break;

   default:
      return;
   }

   imesa->new_state |= SAVAGE_NEW_TEXTURE;
}

static void savageBindTexture( GLcontext *ctx, GLenum target,
			       struct gl_texture_object *tObj )
{
   savageContextPtr imesa = SAVAGE_CONTEXT( ctx );
   
   assert( (target != GL_TEXTURE_2D) || (tObj->DriverData != NULL) );

   imesa->new_state |= SAVAGE_NEW_TEXTURE;
}

static void savageDeleteTexture( GLcontext *ctx, struct gl_texture_object *tObj )
{
   savageTextureObjectPtr t = (savageTextureObjectPtr)tObj->DriverData;
   savageContextPtr imesa = SAVAGE_CONTEXT( ctx );

   if (t) {

      if (t->bound) {
	 imesa->CurrentTexObj[t->bound-1] = 0;
	 imesa->new_state |= SAVAGE_NEW_TEXTURE;
      }

      savageDestroyTexObj(imesa,t);
      tObj->DriverData=0;
   }
   /* Free mipmap images and the texture object itself */
   _mesa_delete_texture_object(ctx, tObj);
}


static GLboolean savageIsTextureResident( GLcontext *ctx, 
					struct gl_texture_object *t )
{
   savageTextureObjectPtr mt;

/*     LOCK_HARDWARE; */
   mt = (savageTextureObjectPtr)t->DriverData;
/*     UNLOCK_HARDWARE; */

   return mt && mt->MemBlock;
}


static struct gl_texture_object *
savageNewTextureObject( GLcontext *ctx, GLuint name, GLenum target )
{
    struct gl_texture_object *obj;
    obj = _mesa_new_texture_object(ctx, name, target);
    savageAllocTexObj( obj );

    return obj;
}

void savageDDInitTextureFuncs( struct dd_function_table *functions )
{
   functions->TexEnv = savageTexEnv;
   functions->ChooseTextureFormat = savageChooseTextureFormat;
   functions->TexImage2D = savageTexImage2D;
   functions->TexSubImage2D = savageTexSubImage2D;
   functions->BindTexture = savageBindTexture;
   functions->NewTextureObject = savageNewTextureObject;
   functions->DeleteTexture = savageDeleteTexture;
   functions->IsTextureResident = savageIsTextureResident;
   functions->TexParameter = savageTexParameter;
}
