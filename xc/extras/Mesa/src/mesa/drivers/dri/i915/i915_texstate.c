/**************************************************************************
 * 
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

#include "glheader.h"
#include "macros.h"
#include "mtypes.h"
#include "simple_list.h"
#include "enums.h"
#include "texformat.h"
#include "texstore.h"

#include "mm.h"

#include "intel_screen.h"
#include "intel_ioctl.h"
#include "intel_tex.h"

#include "i915_context.h"
#include "i915_reg.h"

static GLint initial_offsets[6][2] = { {0,0},
				       {0,2},
				       {1,0},
				       {1,2},
				       {1,1},
				       {1,3} };


static GLint step_offsets[6][2] = { {0,2},
				    {0,2},
				    {-1,2},
				    {-1,2},
				    {-1,1},
				    {-1,1} };


#define I915_TEX_UNIT_ENABLED(unit)		(1<<unit)

static void i915SetTexImages( i915ContextPtr i915, 
			     struct gl_texture_object *tObj )
{
   GLuint total_height, pitch, i, textureFormat;
   i915TextureObjectPtr t = (i915TextureObjectPtr) tObj->DriverData;
   const struct gl_texture_image *baseImage = tObj->Image[0][tObj->BaseLevel];
   GLint firstLevel, lastLevel, numLevels;
   GLint ss2 = 0;

   switch( baseImage->TexFormat->MesaFormat ) {
   case MESA_FORMAT_L8:
      t->intel.texelBytes = 1;
      textureFormat = MAPSURF_8BIT | MT_8BIT_L8;
      break;

   case MESA_FORMAT_I8:
      t->intel.texelBytes = 1;
      textureFormat = MAPSURF_8BIT | MT_8BIT_I8;
      break;

   case MESA_FORMAT_A8:
      t->intel.texelBytes = 1;
      textureFormat = MAPSURF_8BIT | MT_8BIT_A8; 
      break;

   case MESA_FORMAT_AL88:
      t->intel.texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_AY88;
      break;

   case MESA_FORMAT_RGB565:
      t->intel.texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_RGB565;
      break;

   case MESA_FORMAT_ARGB1555:
      t->intel.texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_ARGB1555;
      break;

   case MESA_FORMAT_ARGB4444:
      t->intel.texelBytes = 2;
      textureFormat = MAPSURF_16BIT | MT_16BIT_ARGB4444;
      break;

   case MESA_FORMAT_ARGB8888:
      t->intel.texelBytes = 4;
      textureFormat = MAPSURF_32BIT | MT_32BIT_ARGB8888;
      break;

   case MESA_FORMAT_YCBCR_REV:
      t->intel.texelBytes = 2;
      textureFormat = (MAPSURF_422 | MT_422_YCRCB_NORMAL);
      ss2 |= SS2_COLORSPACE_CONVERSION;
      break;

   case MESA_FORMAT_YCBCR:
      t->intel.texelBytes = 2;
      textureFormat = (MAPSURF_422 | MT_422_YCRCB_SWAPY);
      ss2 |= SS2_COLORSPACE_CONVERSION;
      break;

   case MESA_FORMAT_RGB_FXT1:
   case MESA_FORMAT_RGBA_FXT1:
     t->intel.texelBytes = 2;
     textureFormat = (MAPSURF_COMPRESSED | MT_COMPRESS_FXT1);
     break;

   default:
      fprintf(stderr, "%s: bad image format\n", __FUNCTION__);
      abort();
   }

   /* Compute which mipmap levels we really want to send to the hardware.
    */
   driCalculateTextureFirstLastLevel( (driTextureObject *) t );


   /* Figure out the amount of memory required to hold all the mipmap
    * levels.  Choose the smallest pitch to accomodate the largest
    * mipmap:
    */
   firstLevel = t->intel.base.firstLevel;
   lastLevel = t->intel.base.lastLevel;
   numLevels = lastLevel - firstLevel + 1;



   /* All images must be loaded at this pitch.  Count the number of
    * lines required:
    */
   switch (tObj->Target) {
   case GL_TEXTURE_CUBE_MAP: {
      const GLuint dim = tObj->Image[0][firstLevel]->Width;
      GLuint face;

      pitch = dim * t->intel.texelBytes;
      pitch *= 2;		/* double pitch for cube layouts */
      pitch = (pitch + 3) & ~3;
      
      total_height = dim * 4;

      for ( face = 0 ; face < 6 ; face++) {
	 GLuint x = initial_offsets[face][0] * dim;
	 GLuint y = initial_offsets[face][1] * dim;
	 GLuint d = dim;
	 
	 t->intel.base.dirty_images[face] = ~0;

	 assert(tObj->Image[face][firstLevel]->Width == dim);
	 assert(tObj->Image[face][firstLevel]->Height == dim);

	 for (i = 0; i < numLevels; i++) {
	    t->intel.image[face][i].image = tObj->Image[face][firstLevel + i];
	    if (!t->intel.image[face][i].image) {
	       fprintf(stderr, "no image %d %d\n", face, i);
	       break;		/* can't happen */
	    }
	 
	    t->intel.image[face][i].offset = 
	       y * pitch + x * t->intel.texelBytes;
	    t->intel.image[face][i].internalFormat = baseImage->Format;

	    d >>= 1;
	    x += step_offsets[face][0] * d;
	    y += step_offsets[face][1] * d;
	 }
      }
      break;
   }
   case GL_TEXTURE_3D: {
      GLuint virtual_height;
      GLuint tmp_numLevels = numLevels;
      pitch = tObj->Image[0][firstLevel]->Width * t->intel.texelBytes;
      pitch = (pitch + 3) & ~3;
      t->intel.base.dirty_images[0] = ~0;

      /* Calculate the size of a single slice.  Hardware demands a
       * minimum of 8 mipmaps, some of which might ultimately not be
       * used:
       */
      if (tmp_numLevels < 9)
	 tmp_numLevels = 9;

      virtual_height = tObj->Image[0][firstLevel]->Height;

      for ( total_height = i = 0 ; i < tmp_numLevels ; i++ ) {
	 t->intel.image[0][i].image = tObj->Image[0][firstLevel + i];
	 if (t->intel.image[0][i].image) {
	    t->intel.image[0][i].offset = total_height * pitch;
	    t->intel.image[0][i].internalFormat = baseImage->Format;
	 }

	 total_height += MAX2(2, virtual_height);
	 virtual_height >>= 1;
      }

      t->intel.depth_pitch = total_height * pitch;

      /* Multiply slice size by texture depth for total size.  It's
       * remarkable how wasteful of memory all the i8x0 texture
       * layouts are.
       */
      total_height *= t->intel.image[0][0].image->Depth;
      break;
   }
   default:
      pitch = tObj->Image[0][firstLevel]->Width * t->intel.texelBytes;
      pitch = (pitch + 3) & ~3;
      t->intel.base.dirty_images[0] = ~0;

      for ( total_height = i = 0 ; i < numLevels ; i++ ) {
	 t->intel.image[0][i].image = tObj->Image[0][firstLevel + i];
	 if (!t->intel.image[0][i].image) 
	    break;
	 
	 t->intel.image[0][i].offset = total_height * pitch;
	 t->intel.image[0][i].internalFormat = baseImage->Format;
	 if (t->intel.image[0][i].image->IsCompressed)
	 {
	   if (t->intel.image[0][i].image->Height > 4)
	     total_height += t->intel.image[0][i].image->Height/4;
	   else
	     total_height += 1;
	 }
	 else
	   total_height += MAX2(2, t->intel.image[0][i].image->Height);
      }
      break;
   }

   t->intel.Pitch = pitch;
   t->intel.base.totalSize = total_height*pitch;
   t->intel.max_level = numLevels-1;
   t->Setup[I915_TEXREG_MS3] = 
      (((tObj->Image[0][firstLevel]->Height - 1) << MS3_HEIGHT_SHIFT) |
       ((tObj->Image[0][firstLevel]->Width - 1) << MS3_WIDTH_SHIFT) |
       textureFormat |
       MS3_USE_FENCE_REGS);
   t->Setup[I915_TEXREG_MS4] = 
      ((((pitch / 4) - 1) << MS4_PITCH_SHIFT) | 
       MS4_CUBE_FACE_ENA_MASK |
       (((t->intel.max_level * 4)) << MS4_MAX_LOD_SHIFT) |
       ((tObj->Image[0][firstLevel]->Depth - 1) << MS4_VOLUME_DEPTH_SHIFT));

   t->Setup[I915_TEXREG_SS2] &= ~SS2_COLORSPACE_CONVERSION;
   t->Setup[I915_TEXREG_SS2] |= ss2;

   t->intel.dirty = I915_UPLOAD_TEX_ALL;
}


/* The i915 (and related graphics cores) do not support GL_CLAMP.  The
 * Intel drivers for "other operating systems" implement GL_CLAMP as
 * GL_CLAMP_TO_EDGE, so the same is done here.
 */
static GLuint translate_wrap_mode( GLenum wrap )
{
   switch( wrap ) {
   case GL_REPEAT: return TEXCOORDMODE_WRAP;
   case GL_CLAMP:  return TEXCOORDMODE_CLAMP_EDGE; /* not quite correct */
   case GL_CLAMP_TO_EDGE: return TEXCOORDMODE_CLAMP_EDGE;
   case GL_CLAMP_TO_BORDER: return TEXCOORDMODE_CLAMP_BORDER;
   case GL_MIRRORED_REPEAT: return TEXCOORDMODE_MIRROR;
   default: return TEXCOORDMODE_WRAP;
   }
}


/**
 */
static void i915ImportTexObjState( struct gl_texture_object *texObj )
{   
   i915TextureObjectPtr t = (i915TextureObjectPtr)texObj->DriverData;
   int minFilt = 0, mipFilt = 0, magFilt = 0;

   if(INTEL_DEBUG&DEBUG_DRI)
      fprintf(stderr, "%s\n", __FUNCTION__);

   switch (texObj->MinFilter) {
   case GL_NEAREST:
      minFilt = FILTER_NEAREST;
      mipFilt = MIPFILTER_NONE;
      break;
   case GL_LINEAR:
      minFilt = FILTER_LINEAR;
      mipFilt = MIPFILTER_NONE;
      break;
   case GL_NEAREST_MIPMAP_NEAREST:
      minFilt = FILTER_NEAREST;
      mipFilt = MIPFILTER_NEAREST;
      break;
   case GL_LINEAR_MIPMAP_NEAREST:
      minFilt = FILTER_LINEAR;
      mipFilt = MIPFILTER_NEAREST;
      break;
   case GL_NEAREST_MIPMAP_LINEAR:
      minFilt = FILTER_NEAREST;
      mipFilt = MIPFILTER_LINEAR;
      break;
   case GL_LINEAR_MIPMAP_LINEAR:
      minFilt = FILTER_LINEAR;
      mipFilt = MIPFILTER_LINEAR;
      break;
   default:
      break;
   }

   if ( texObj->MaxAnisotropy > 1.0 ) {
      minFilt = FILTER_ANISOTROPIC; 
      magFilt = FILTER_ANISOTROPIC;
   }
   else {
      switch (texObj->MagFilter) {
      case GL_NEAREST:
	 magFilt = FILTER_NEAREST;
	 break;
      case GL_LINEAR:
	 magFilt = FILTER_LINEAR;
	 break;
      default:
	 break;
      }  
   }

   t->Setup[I915_TEXREG_SS2] &= ~SS2_MIN_FILTER_MASK;
   t->Setup[I915_TEXREG_SS2] &= ~SS2_MIP_FILTER_MASK;
   t->Setup[I915_TEXREG_SS2] &= ~SS2_MAG_FILTER_MASK;
   t->Setup[I915_TEXREG_SS2] |= ((minFilt << SS2_MIN_FILTER_SHIFT) |
				(mipFilt << SS2_MIP_FILTER_SHIFT) |
				(magFilt << SS2_MAG_FILTER_SHIFT));

   {
      GLuint ss3 = t->Setup[I915_TEXREG_SS3] & ~(SS3_TCX_ADDR_MODE_MASK |
						SS3_TCY_ADDR_MODE_MASK |
						SS3_TCZ_ADDR_MODE_MASK);
      GLenum ws = texObj->WrapS;
      GLenum wt = texObj->WrapT;
      GLenum wr = texObj->WrapR;
      
      t->refs_border_color = 0;

      if (texObj->Target == GL_TEXTURE_3D &&
	  (texObj->MinFilter != GL_NEAREST ||
	   texObj->MagFilter != GL_NEAREST)) {
	 
	 /* Try to mimic GL_CLAMP functionality a little better -
	  * switch to CLAMP_TO_BORDER whenever a non-NEAREST filter is
	  * in use.  Only do this for 3D textures at the moment --
	  * doing it universally would fix the conform texbc.c
	  * failure, though.
	  */
	 if (ws == GL_CLAMP) ws = GL_CLAMP_TO_BORDER;
	 if (wt == GL_CLAMP) wt = GL_CLAMP_TO_BORDER;
	 if (wr == GL_CLAMP) wr = GL_CLAMP_TO_BORDER;

	 /* 3D textures don't seem to respect the border color.
	  * Fallback if there's ever a danger that they might refer to
	  * it.
	  */
	 if (ws == GL_CLAMP_TO_BORDER) t->refs_border_color = 1;
	 if (wt == GL_CLAMP_TO_BORDER) t->refs_border_color = 1;
	 if (wr == GL_CLAMP_TO_BORDER) t->refs_border_color = 1;
      }

      ss3 |= translate_wrap_mode(ws) << SS3_TCX_ADDR_MODE_SHIFT;
      ss3 |= translate_wrap_mode(wt) << SS3_TCY_ADDR_MODE_SHIFT;
      ss3 |= translate_wrap_mode(wr) << SS3_TCZ_ADDR_MODE_SHIFT;
   
      if (ss3 != t->Setup[I915_TEXREG_SS3]) {
	 t->intel.dirty = I915_UPLOAD_TEX_ALL;
	 t->Setup[I915_TEXREG_SS3] = ss3;
      }
   }

   {   
      const GLubyte *color = texObj->_BorderChan;

      t->Setup[I915_TEXREG_SS4] = INTEL_PACKCOLOR8888(color[0],color[1],
						     color[2],color[3]);
   }
}



static void i915_import_tex_unit( i915ContextPtr i915, 
				 i915TextureObjectPtr t,
				 GLuint unit )
{
   GLuint state[I915_TEX_SETUP_SIZE];

   if(INTEL_DEBUG&DEBUG_TEXTURE)
      fprintf(stderr, "%s unit(%d)\n", __FUNCTION__, unit);
   
   if (i915->intel.CurrentTexObj[unit]) 
      i915->intel.CurrentTexObj[unit]->base.bound &= ~(1U << unit);

   i915->intel.CurrentTexObj[unit] = (intelTextureObjectPtr)t;
   t->intel.base.bound |= (1 << unit);

   if (t->intel.dirty & I915_UPLOAD_TEX(unit)) {
      i915ImportTexObjState( t->intel.base.tObj );
      t->intel.dirty &= ~I915_UPLOAD_TEX(unit);
   }

   state[I915_TEXREG_MS2] = t->intel.TextureOffset;
   state[I915_TEXREG_MS3] = t->Setup[I915_TEXREG_MS3];
   state[I915_TEXREG_MS4] = t->Setup[I915_TEXREG_MS4];

   state[I915_TEXREG_SS2] = (i915->state.Tex[unit][I915_TEXREG_SS2] &
			    SS2_LOD_BIAS_MASK);
   state[I915_TEXREG_SS2] |= (t->Setup[I915_TEXREG_SS2] & ~SS2_LOD_BIAS_MASK);

   state[I915_TEXREG_SS3] = (i915->state.Tex[unit][I915_TEXREG_SS3] &
			    SS3_NORMALIZED_COORDS);
   state[I915_TEXREG_SS3] |= (t->Setup[I915_TEXREG_SS3] &
			     ~(SS3_NORMALIZED_COORDS|
			       SS3_TEXTUREMAP_INDEX_MASK));

   state[I915_TEXREG_SS3] |= (unit<<SS3_TEXTUREMAP_INDEX_SHIFT);

   state[I915_TEXREG_SS4] = t->Setup[I915_TEXREG_SS4];


   if (memcmp(state, i915->state.Tex[unit], sizeof(state)) != 0) {
      I915_STATECHANGE( i915, I915_UPLOAD_TEX(unit) );
      memcpy(i915->state.Tex[unit], state, sizeof(state));
   }
}



static GLboolean enable_tex_common( GLcontext *ctx, GLuint unit )
{
   i915ContextPtr i915 = I915_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];
   struct gl_texture_object *tObj = texUnit->_Current;
   i915TextureObjectPtr t = (i915TextureObjectPtr)tObj->DriverData;

   if (0) fprintf(stderr, "%s %d\n", __FUNCTION__, unit);

   if (!(i915->state.active & I915_UPLOAD_TEX(unit))) {
      I915_ACTIVESTATE(i915, I915_UPLOAD_TEX(unit), GL_TRUE);
   }

   /* Fallback if there's a texture border */
   if ( tObj->Image[0][tObj->BaseLevel]->Border > 0 ) {
      return GL_FALSE;
   }


   /* Update state if this is a different texture object to last
    * time.
    */
   if (i915->intel.CurrentTexObj[unit] != &t->intel || 
       (t->intel.dirty & I915_UPLOAD_TEX(unit))) {
      i915_import_tex_unit( i915, t, unit);
      i915->tex_program.translated = 0;
   }

   return GL_TRUE;
}

static GLboolean enable_tex_rect( GLcontext *ctx, GLuint unit )
{
   i915ContextPtr i915 = I915_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];
   struct gl_texture_object *tObj = texUnit->_Current;
   i915TextureObjectPtr t = (i915TextureObjectPtr)tObj->DriverData;
   GLuint ss3 = i915->state.Tex[unit][I915_TEXREG_SS3];

   ss3 &= ~SS3_NORMALIZED_COORDS;

   if (ss3 != i915->state.Tex[unit][I915_TEXREG_SS3]) {
      I915_STATECHANGE(i915, I915_UPLOAD_TEX(unit));
      i915->state.Tex[unit][I915_TEXREG_SS3] = ss3;
   }

   /* Upload teximages (not pipelined)
    */
   if (t->intel.base.dirty_images[0]) {
      i915SetTexImages( i915, tObj );
      if (!intelUploadTexImages( &i915->intel, &t->intel, 0 )) {
	 return GL_FALSE;
      }
   }

   return GL_TRUE;
}


static GLboolean enable_tex_2d( GLcontext *ctx, GLuint unit )
{
   i915ContextPtr i915 = I915_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];
   struct gl_texture_object *tObj = texUnit->_Current;
   i915TextureObjectPtr t = (i915TextureObjectPtr)tObj->DriverData;
   GLuint ss3 = i915->state.Tex[unit][I915_TEXREG_SS3];

   ss3 |= SS3_NORMALIZED_COORDS;

   if (ss3 != i915->state.Tex[unit][I915_TEXREG_SS3]) {
      I915_STATECHANGE(i915, I915_UPLOAD_TEX(unit));
      i915->state.Tex[unit][I915_TEXREG_SS3] = ss3;
   }

   /* Upload teximages (not pipelined)
    */
   if (t->intel.base.dirty_images[0]) {
      i915SetTexImages( i915, tObj );
      if (!intelUploadTexImages( &i915->intel, &t->intel, 0 )) {
	 return GL_FALSE;
      }
   }

   return GL_TRUE;
}

static GLboolean enable_tex_cube( GLcontext *ctx, GLuint unit )
{
   i915ContextPtr i915 = I915_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];
   struct gl_texture_object *tObj = texUnit->_Current;
   i915TextureObjectPtr t = (i915TextureObjectPtr)tObj->DriverData;
   GLuint ss3 = i915->state.Tex[unit][I915_TEXREG_SS3];
   GLuint face;

   ss3 |= SS3_NORMALIZED_COORDS;

   if (ss3 != i915->state.Tex[unit][I915_TEXREG_SS3]) {
      I915_STATECHANGE(i915, I915_UPLOAD_TEX(unit));
      i915->state.Tex[unit][I915_TEXREG_SS3] = ss3;
   }

   /* Upload teximages (not pipelined)
    */
   if ( t->intel.base.dirty_images[0] || t->intel.base.dirty_images[1] ||
        t->intel.base.dirty_images[2] || t->intel.base.dirty_images[3] ||
        t->intel.base.dirty_images[4] || t->intel.base.dirty_images[5] ) {
      i915SetTexImages( i915, tObj );
   }

   /* upload (per face) */
   for (face = 0; face < 6; face++) {
      if (t->intel.base.dirty_images[face]) {
	 if (!intelUploadTexImages( &i915->intel, &t->intel, face )) {
	    return GL_FALSE;
	 }
      }
   }


   return GL_TRUE;
}

static GLboolean enable_tex_3d( GLcontext *ctx, GLuint unit )
{
   struct gl_texture_object *tObj = ctx->Texture.Unit[unit]._Current;
   i915TextureObjectPtr t = (i915TextureObjectPtr)tObj->DriverData;

   /* 3D textures on I915 seem to get bogus border colors, hence this
    * fallback:
    */
   if (t->refs_border_color)
      return GL_FALSE;

   return GL_TRUE;
}



 
static GLboolean disable_tex( GLcontext *ctx, GLuint unit )
{
   i915ContextPtr i915 = I915_CONTEXT(ctx);

   if (i915->state.active & I915_UPLOAD_TEX(unit)) {
      I915_ACTIVESTATE(i915, I915_UPLOAD_TEX(unit), GL_FALSE);
   }

   /* The old texture is no longer bound to this texture unit.
    * Mark it as such.
    */
   if ( i915->intel.CurrentTexObj[unit] != NULL ) {
      i915->intel.CurrentTexObj[unit]->base.bound &= ~(1U << 0);
      i915->intel.CurrentTexObj[unit] = NULL;
   }

   return GL_TRUE;
}

static GLboolean i915UpdateTexUnit( GLcontext *ctx, GLuint unit )
{
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];

   if (texUnit->_ReallyEnabled &&
       INTEL_CONTEXT(ctx)->intelScreen->textureSize < 2048 * 1024)
      return GL_FALSE;

   switch (texUnit->_ReallyEnabled) {
   case TEXTURE_1D_BIT:
   case TEXTURE_2D_BIT:
      return (enable_tex_2d( ctx, unit ) &&
	      enable_tex_common( ctx, unit ));
   case TEXTURE_RECT_BIT:
      return (enable_tex_rect( ctx, unit ) &&
	      enable_tex_common( ctx, unit ));
   case TEXTURE_CUBE_BIT:
      return (enable_tex_cube( ctx, unit ) &&
	      enable_tex_common( ctx, unit ));
   case TEXTURE_3D_BIT:
       return (enable_tex_2d( ctx, unit ) && 
	       enable_tex_common( ctx, unit ) &&
	       enable_tex_3d( ctx, unit)); 
   case 0:
      return disable_tex( ctx, unit );
   default:
      return GL_FALSE;
   }
}


void i915UpdateTextureState( intelContextPtr intel )
{
   GLcontext *ctx = &intel->ctx;
   GLboolean ok = GL_TRUE;
   GLuint i;

   for (i = 0 ; i < I915_TEX_UNITS && ok ; i++) {
      ok = i915UpdateTexUnit( ctx, i );
   }

   FALLBACK( intel, I915_FALLBACK_TEXTURE, !ok );
}



