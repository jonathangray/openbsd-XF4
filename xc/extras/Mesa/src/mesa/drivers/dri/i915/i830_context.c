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

#include "i830_context.h"
#include "imports.h"
#include "texmem.h"
#include "intel_tex.h"
#include "tnl/tnl.h"
#include "tnl/t_vertex.h"
#include "tnl/t_context.h"


/***************************************
 * Mesa's Driver Functions
 ***************************************/

/* This is the extension list explicitly enabled by the client and
 * excludes functionality available in Mesa and also excludes legacy
 * extensions.  It is recognized that in an open source driver, those
 * extensions will probably be re-enabled.
 */
static const GLubyte *i830GetString( GLcontext *ctx, GLenum name )
{
#if 0
   if (name == GL_EXTENSIONS)
      return 
	 "GL_ARB_multitexture "
	 "GL_ARB_texture_border_clamp "
	 "GL_ARB_texture_compression "
	 "GL_ARB_texture_env_add "
	 "GL_ARB_texture_env_combine "
	 "GL_ARB_texture_env_dot3 "
	 "GL_ARB_texture_mirrored_repeat "
	 "GL_ARB_transpose_matrix "
	 "GL_ARB_vertex_buffer_object "
	 "GL_ARB_vertex_program "
	 "GL_ARB_window_pos "
	 "GL_EXT_abgr "
	 "GL_EXT_bgra "
	 "GL_EXT_blend_color "
	 "GL_EXT_blend_func_separate "
	 "GL_EXT_blend_minmax "
	 "GL_EXT_blend_subtract "
	 "GL_EXT_clip_volume_hint "
	 "GL_EXT_compiled_vertex_array "
	 "GL_EXT_draw_range_elements "
	 "GL_EXT_fog_coord "
	 "GL_EXT_multi_draw_arrays "
	 "GL_EXT_packed_pixels "
	 "GL_EXT_rescale_normal "
	 "GL_EXT_secondary_color "
	 "GL_EXT_separate_specular_color "
	 "GL_EXT_stencil_wrap "
	 "GL_EXT_texture_env_add "
	 "GL_EXT_texture_env_combine "
	 "GL_EXT_texture_filter_anisotropic "
	 "GL_IBM_texture_mirrored_repeat "
	 "GL_MESA_ycbcr_texture "
	 "GL_MESA_window_pos "
	 "GL_NV_texgen_reflection "
	 "GL_SGIS_generate_mipmap ";
#endif 
   return intelGetString( ctx, name );
}


static void i830InitDriverFunctions( struct dd_function_table *functions )
{
   intelInitDriverFunctions( functions );
   i830InitStateFuncs( functions );
   i830InitTextureFuncs( functions );
   functions->GetString = i830GetString;
}


GLboolean i830CreateContext( const __GLcontextModes *mesaVis,
			    __DRIcontextPrivate *driContextPriv,
			    void *sharedContextPrivate)
{
   struct dd_function_table functions;
   i830ContextPtr i830 = (i830ContextPtr) CALLOC_STRUCT(i830_context);
   intelContextPtr intel = &i830->intel;
   GLcontext *ctx = &intel->ctx;
   if (!i830) return GL_FALSE;

   i830InitVtbl( i830 );
   i830InitDriverFunctions( &functions );

   if (!intelInitContext( intel, mesaVis, driContextPriv,
			  sharedContextPrivate, &functions )) {
      FREE(i830);
      return GL_FALSE;
   }

   intel->ctx.Const.MaxTextureUnits = I830_TEX_UNITS;
   intel->ctx.Const.MaxTextureImageUnits = I830_TEX_UNITS;
   intel->ctx.Const.MaxTextureCoordUnits = I830_TEX_UNITS;

   intel->nr_heaps = 1;
   intel->texture_heaps[0] = 
      driCreateTextureHeap( 0, intel,
			    intel->intelScreen->textureSize,
			    12,
			    I830_NR_TEX_REGIONS,
			    intel->sarea->texList,
			    & intel->sarea->texAge,
			    & intel->swapped,
			    sizeof( struct i830_texture_object ),
			    (destroy_texture_object_t *)intelDestroyTexObj );

   /* FIXME: driCalculateMaxTextureLevels assumes that mipmaps are tightly
    * FIXME: packed, but they're not in Intel graphics hardware.
    */
   intel->ctx.Const.MaxTextureUnits = 1;
   driCalculateMaxTextureLevels( intel->texture_heaps,
				 intel->nr_heaps,
				 &intel->ctx.Const,
				 4,
				 11, /* max 2D texture size is 2048x2048 */
				 8,  /* max 3D texture size is 256^3 */
				 0,  /* max CUBE. not supported */
				 11, /* max RECT. supported */
				 12,
				 GL_FALSE );
   intel->ctx.Const.MaxTextureUnits = I830_TEX_UNITS;

   _tnl_init_vertices( ctx, ctx->Const.MaxArrayLockSize + 12, 
		       18 * sizeof(GLfloat) );

   intel->verts = TNL_CONTEXT(ctx)->clipspace.vertex_buf;

   i830InitState( i830 );


   _tnl_allow_vertex_fog( ctx, 1 ); 
   _tnl_allow_pixel_fog( ctx, 0 ); 

   return GL_TRUE;
}

