/* $XFree86$ */ /* -*- c-basic-offset: 3 -*- */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keithw@valinux.com>
 *   Felix Kuehling <fxkuehl@gmx.de>
 *
 */

#include <stdio.h>
#include <math.h>

#include "glheader.h"
#include "mtypes.h"
#include "colormac.h"
#include "macros.h"

#include "swrast/swrast.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"

#include "savagetris.h"
#include "savagestate.h"
#include "savagetex.h"
#include "savageioctl.h"
#include "savage_bci.h"

static void savageRasterPrimitive( GLcontext *ctx, GLuint prim );
static void savageRenderPrimitive( GLcontext *ctx, GLenum prim );


static GLenum reduced_prim[GL_POLYGON+1] = {
   GL_POINTS,
   GL_LINES,
   GL_LINES,
   GL_LINES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES,
   GL_TRIANGLES
};

 
/***********************************************************************
 *                    Emit primitives                                  *
 ***********************************************************************/

#if defined (USE_X86_ASM)
#define EMIT_VERT( j, vb, vertex_size, start, v )	\
do {	int __tmp;					\
        vb += start;					\
	__asm__ __volatile__( "rep ; movsl"		\
			 : "=%c" (j), "=D" (vb), "=S" (__tmp)		\
			 : "0" (vertex_size-start),	\
			   "D" ((long)vb), 		\
			   "S" ((long)&v->ui[start]));	\
} while (0)
#else
#define EMIT_VERT( j, vb, vertex_size, start, v )	\
do {						\
   for ( j = start ; j < vertex_size ; j++ )	\
      vb[j] = (v)->ui[j];			\
   vb += vertex_size;				\
} while (0)
#endif

static void __inline__ savage_draw_triangle (savageContextPtr imesa,
					     savageVertexPtr v0,
					     savageVertexPtr v1,
					     savageVertexPtr v2) {
   GLuint vertsize = imesa->vertex_size;
   uint32_t *vb = savageAllocDmaLow (imesa, 3*4*vertsize);
   GLuint j;

   EMIT_VERT (j, vb, vertsize, 0, v0);
   EMIT_VERT (j, vb, vertsize, 0, v1);
   EMIT_VERT (j, vb, vertsize, 0, v2);
}

static void __inline__ savage_draw_quad (savageContextPtr imesa,
					 savageVertexPtr v0,
					 savageVertexPtr v1,
					 savageVertexPtr v2,
					 savageVertexPtr v3) {
   GLuint vertsize = imesa->vertex_size;
   uint32_t *vb = savageAllocDmaLow (imesa, 6*4*vertsize);
   GLuint j;

   EMIT_VERT (j, vb, vertsize, 0, v0);
   EMIT_VERT (j, vb, vertsize, 0, v1);
   EMIT_VERT (j, vb, vertsize, 0, v3);
   EMIT_VERT (j, vb, vertsize, 0, v1);
   EMIT_VERT (j, vb, vertsize, 0, v2);
   EMIT_VERT (j, vb, vertsize, 0, v3);
}

static __inline__ void savage_draw_point (savageContextPtr imesa,
					  savageVertexPtr tmp) {
   GLuint vertsize = imesa->vertex_size;
   uint32_t *vb = savageAllocDmaLow (imesa, 6*4*vertsize);
   const GLfloat x = tmp->v.x;
   const GLfloat y = tmp->v.y;
   const GLfloat sz = imesa->glCtx->Point._Size * .5;
   GLuint j;

   *(float *)&vb[0] = x - sz;
   *(float *)&vb[1] = y - sz;
   EMIT_VERT (j, vb, vertsize, 2, tmp);

   *(float *)&vb[0] = x + sz;
   *(float *)&vb[1] = y - sz;
   EMIT_VERT (j, vb, vertsize, 2, tmp);

   *(float *)&vb[0] = x + sz;
   *(float *)&vb[1] = y + sz;
   EMIT_VERT (j, vb, vertsize, 2, tmp);

   *(float *)&vb[0] = x + sz;
   *(float *)&vb[1] = y + sz;
   EMIT_VERT (j, vb, vertsize, 2, tmp);

   *(float *)&vb[0] = x - sz;
   *(float *)&vb[1] = y + sz;
   EMIT_VERT (j, vb, vertsize, 2, tmp);

   *(float *)&vb[0] = x - sz;
   *(float *)&vb[1] = y - sz;
   EMIT_VERT (j, vb, vertsize, 2, tmp);
}

static __inline__ void savage_draw_line (savageContextPtr imesa,
					 savageVertexPtr v0,
					 savageVertexPtr v1 ) {
   GLuint vertsize = imesa->vertex_size;
   uint32_t *vb = savageAllocDmaLow (imesa, 6*4*vertsize);
   GLfloat width = imesa->glCtx->Line._Width;
   GLfloat dx, dy, ix, iy;
   GLuint j;

   dx = v0->v.x - v1->v.x;
   dy = v0->v.y - v1->v.y;

   ix = width * .5; iy = 0;
   if (dx * dx > dy * dy) {
      iy = ix; ix = 0;
   }

   *(float *)&vb[0] = v0->v.x - ix;
   *(float *)&vb[1] = v0->v.y - iy;
   EMIT_VERT (j, vb, vertsize, 2, v0);

   *(float *)&vb[0] = v1->v.x + ix;
   *(float *)&vb[1] = v1->v.y + iy;
   EMIT_VERT (j, vb, vertsize, 2, v1);

   *(float *)&vb[0] = v0->v.x + ix;
   *(float *)&vb[1] = v0->v.y + iy;
   EMIT_VERT (j, vb, vertsize, 2, v0);

   *(float *)&vb[0] = v0->v.x - ix;
   *(float *)&vb[1] = v0->v.y - iy;
   EMIT_VERT (j, vb, vertsize, 2, v0);

   *(float *)&vb[0] = v1->v.x - ix;
   *(float *)&vb[1] = v1->v.y - iy;
   EMIT_VERT (j, vb, vertsize, 2, v1);

   *(float *)&vb[0] = v1->v.x + ix;
   *(float *)&vb[1] = v1->v.y + iy;
   EMIT_VERT (j, vb, vertsize, 2, v1);
} 
 
/***********************************************************************
 *          Macros for t_dd_tritmp.h to draw basic primitives          *
 ***********************************************************************/

#define TRI( a, b, c )				\
do {						\
   if (DO_FALLBACK)				\
      imesa->draw_tri( imesa, a, b, c );	\
   else						\
      savage_draw_triangle( imesa, a, b, c );	\
} while (0)

#define QUAD( a, b, c, d )			\
do {						\
   if (DO_FALLBACK) {				\
      imesa->draw_tri( imesa, a, b, d );	\
      imesa->draw_tri( imesa, b, c, d );	\
   } else 					\
      savage_draw_quad( imesa, a, b, c, d );	\
} while (0)

#define LINE( v0, v1 )				\
do {						\
   if (DO_FALLBACK)				\
      imesa->draw_line( imesa, v0, v1 );	\
   else 					\
      savage_draw_line( imesa, v0, v1 );	\
} while (0)

#define POINT( v0 )				\
do {						\
   if (DO_FALLBACK)				\
      imesa->draw_point( imesa, v0 );		\
   else 					\
      savage_draw_point( imesa, v0 );		\
} while (0)


/***********************************************************************
 *              Build render functions from dd templates               *
 ***********************************************************************/

#define SAVAGE_OFFSET_BIT	 0x1
#define SAVAGE_TWOSIDE_BIT       0x2
#define SAVAGE_UNFILLED_BIT      0x4
#define SAVAGE_FALLBACK_BIT      0x8
#define SAVAGE_MAX_TRIFUNC       0x10


static struct {
   tnl_points_func	        points;
   tnl_line_func		line;
   tnl_triangle_func	triangle;
   tnl_quad_func		quad;
} rast_tab[SAVAGE_MAX_TRIFUNC];


#define DO_FALLBACK (IND & SAVAGE_FALLBACK_BIT)
#define DO_OFFSET   (IND & SAVAGE_OFFSET_BIT)
#define DO_UNFILLED (IND & SAVAGE_UNFILLED_BIT)
#define DO_TWOSIDE  (IND & SAVAGE_TWOSIDE_BIT)
#define DO_FLAT      0
#define DO_TRI       1
#define DO_QUAD      1
#define DO_LINE      1
#define DO_POINTS    1
#define DO_FULL_QUAD 1

#define HAVE_RGBA   1
#define HAVE_SPEC   1
#define HAVE_BACK_COLORS  0
#define HAVE_HW_FLATSHADE 1
#define VERTEX savageVertex
#define TAB rast_tab

#define DEPTH_SCALE imesa->depth_scale
#define UNFILLED_TRI unfilled_tri
#define UNFILLED_QUAD unfilled_quad
#define VERT_X(_v) _v->v.x
#define VERT_Y(_v) _v->v.y
#define VERT_Z(_v) _v->v.z
#define AREA_IS_CCW( a ) (a > 0)
#define GET_VERTEX(e) (imesa->verts + (e * imesa->vertex_size * sizeof(int)))

#define VERT_SET_RGBA( v, c )					\
do {								\
   savage_color_t *color = (savage_color_t *)&((v)->ub4[coloroffset]);	\
   UNCLAMPED_FLOAT_TO_UBYTE(color->red, (c)[0]);		\
   UNCLAMPED_FLOAT_TO_UBYTE(color->green, (c)[1]);		\
   UNCLAMPED_FLOAT_TO_UBYTE(color->blue, (c)[2]);		\
   UNCLAMPED_FLOAT_TO_UBYTE(color->alpha, (c)[3]);		\
} while (0)
#define VERT_COPY_RGBA( v0, v1 ) v0->ui[coloroffset] = v1->ui[coloroffset]
#define VERT_SAVE_RGBA( idx )    color[idx] = v[idx]->ui[coloroffset]
#define VERT_RESTORE_RGBA( idx ) v[idx]->ui[coloroffset] = color[idx]

#define VERT_SET_SPEC( v, c )					\
do {								\
   if (specoffset) {						\
      savage_color_t *spec = (savage_color_t *)&((v)->ub4[specoffset]);	\
      UNCLAMPED_FLOAT_TO_UBYTE(spec->red, (c)[0]);		\
      UNCLAMPED_FLOAT_TO_UBYTE(spec->green, (c)[1]);		\
      UNCLAMPED_FLOAT_TO_UBYTE(spec->blue, (c)[2]);		\
   }								\
} while (0)
#define VERT_COPY_SPEC( v0, v1 )					\
   if (specoffset) COPY_3V(v0->ub4[specoffset], v1->ub4[specoffset])
#define VERT_SAVE_SPEC( idx )						\
   if (specoffset) spec[idx] = v[idx]->ui[specoffset]
#define VERT_RESTORE_SPEC( idx )					\
   if (specoffset) v[idx]->ui[specoffset] = spec[idx]

#define LOCAL_VARS(n)						\
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);		\
   GLuint color[n], spec[n];					\
   GLuint coloroffset =						\
      ((imesa->DrawPrimitiveCmd & SAVAGE_HW_NO_W) ? 3 : 4);	\
   GLboolean specoffset =					\
      ((imesa->DrawPrimitiveCmd & SAVAGE_HW_NO_CS) ? 0 : coloroffset+1);\
   (void) color; (void) spec; (void) coloroffset; (void) specoffset;

/***********************************************************************
 *                Helpers for rendering unfilled primitives            *
 ***********************************************************************/

#define RASTERIZE(x) if (imesa->raster_primitive != reduced_prim[x]) \
                        savageRasterPrimitive( ctx, x )
#define RENDER_PRIMITIVE imesa->render_primitive
#define IND SAVAGE_FALLBACK_BIT
#define TAG(x) x
#include "tnl_dd/t_dd_unfilled.h"
#undef IND


/***********************************************************************
 *                      Generate GL render functions                   *
 ***********************************************************************/


#define IND (0)
#define TAG(x) x
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_OFFSET_BIT)
#define TAG(x) x##_offset
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT)
#define TAG(x) x##_twoside
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_OFFSET_BIT)
#define TAG(x) x##_twoside_offset
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_UNFILLED_BIT)
#define TAG(x) x##_unfilled
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_OFFSET_BIT|SAVAGE_UNFILLED_BIT)
#define TAG(x) x##_offset_unfilled
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_UNFILLED_BIT)
#define TAG(x) x##_twoside_unfilled
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_OFFSET_BIT|SAVAGE_UNFILLED_BIT)
#define TAG(x) x##_twoside_offset_unfilled
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_OFFSET_BIT|SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_offset_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_twoside_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_OFFSET_BIT|SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_twoside_offset_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_UNFILLED_BIT|SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_unfilled_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_OFFSET_BIT|SAVAGE_UNFILLED_BIT|SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_offset_unfilled_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_UNFILLED_BIT|SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_twoside_unfilled_fallback
#include "tnl_dd/t_dd_tritmp.h"

#define IND (SAVAGE_TWOSIDE_BIT|SAVAGE_OFFSET_BIT|SAVAGE_UNFILLED_BIT| \
	     SAVAGE_FALLBACK_BIT)
#define TAG(x) x##_twoside_offset_unfilled_fallback
#include "tnl_dd/t_dd_tritmp.h"


static void init_rast_tab( void )
{
   init();
   init_offset();
   init_twoside();
   init_twoside_offset();
   init_unfilled();
   init_offset_unfilled();
   init_twoside_unfilled();
   init_twoside_offset_unfilled();
   init_fallback();
   init_offset_fallback();
   init_twoside_fallback();
   init_twoside_offset_fallback();
   init_unfilled_fallback();
   init_offset_unfilled_fallback();
   init_twoside_unfilled_fallback();
   init_twoside_offset_unfilled_fallback();
}



/***********************************************************************
 *                    Rasterization fallback helpers                   *
 ***********************************************************************/


/* This code is hit only when a mix of accelerated and unaccelerated
 * primitives are being drawn, and only for the unaccelerated
 * primitives.
 */
static void
savage_fallback_tri( savageContextPtr imesa,
		     savageVertexPtr v0,
		     savageVertexPtr v1,
		     savageVertexPtr v2 )
{
   GLcontext *ctx = imesa->glCtx;
   SWvertex v[3];
   _swsetup_Translate( ctx, v0, &v[0] );
   _swsetup_Translate( ctx, v1, &v[1] );
   _swsetup_Translate( ctx, v2, &v[2] );
   _swrast_Triangle( ctx, &v[0], &v[1], &v[2] );
}


static void
savage_fallback_line( savageContextPtr imesa,
		      savageVertexPtr v0,
		      savageVertexPtr v1 )
{
   GLcontext *ctx = imesa->glCtx;
   SWvertex v[2];
   _swsetup_Translate( ctx, v0, &v[0] );
   _swsetup_Translate( ctx, v1, &v[1] );
   _swrast_Line( ctx, &v[0], &v[1] );
}


static void
savage_fallback_point( savageContextPtr imesa,
		       savageVertexPtr v0 )
{
   GLcontext *ctx = imesa->glCtx;
   SWvertex v[1];
   _swsetup_Translate( ctx, v0, &v[0] );
   _swrast_Point( ctx, &v[0] );
}



/**********************************************************************/
/*               Render unclipped begin/end objects                   */
/**********************************************************************/

#define VERT(x) (savageVertexPtr)(savageVerts + (x * vertsize * sizeof(int)))
#define RENDER_POINTS( start, count )		\
   for ( ; start < count ; start++)		\
      savage_draw_point( imesa, VERT(start) )
#define RENDER_LINE( v0, v1 ) \
   savage_draw_line( imesa, VERT(v0), VERT(v1) )
#define RENDER_TRI( v0, v1, v2 )  \
   savage_draw_triangle( imesa, VERT(v0), VERT(v1), VERT(v2) )
#define RENDER_QUAD( v0, v1, v2, v3 ) \
   savage_draw_quad( imesa, VERT(v0), VERT(v1), VERT(v2), VERT(v3) )
#define INIT(x) do {					\
   if (0) fprintf(stderr, "%s\n", __FUNCTION__);	\
   savageRenderPrimitive( ctx, x );                     \
   /*SAVAGE_CONTEXT(ctx)->render_primitive = x;*/       \
} while (0)
#undef LOCAL_VARS
#define LOCAL_VARS						\
    savageContextPtr imesa = SAVAGE_CONTEXT(ctx);		\
    const GLuint vertsize = imesa->vertex_size;			\
    const char *savageVerts = (char *)imesa->verts;		\
    const GLuint * const elt = TNL_CONTEXT(ctx)->vb.Elts;	\
    (void) elt;
#define RESET_STIPPLE
#define RESET_OCCLUSION
#define PRESERVE_VB_DEFS
#define ELT(x) (x)
#define TAG(x) savage_##x##_verts
#include "tnl/t_vb_rendertmp.h"
#undef ELT
#undef TAG
#define TAG(x) savage_##x##_elts
#define ELT(x) elt[x]
#include "tnl/t_vb_rendertmp.h"


/**********************************************************************/
/*                    Render clipped primitives                       */
/**********************************************************************/

static void savageRenderClippedPoly( GLcontext *ctx, const GLuint *elts,
				     GLuint n )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   struct vertex_buffer *VB = &TNL_CONTEXT(ctx)->vb;

   /* Render the new vertices as an unclipped polygon.
    */
   {
      GLuint *tmp = VB->Elts;
      VB->Elts = (GLuint *)elts;
      tnl->Driver.Render.PrimTabElts[GL_POLYGON]( ctx, 0, n, PRIM_BEGIN|PRIM_END );
      VB->Elts = tmp;
   }
}

static void savageRenderClippedLine( GLcontext *ctx, GLuint ii, GLuint jj )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   tnl->Driver.Render.Line( ctx, ii, jj );
}
/*
static void savageFastRenderClippedPoly( GLcontext *ctx, const GLuint *elts,
					 GLuint n )
{
   r128ContextPtr rmesa = R128_CONTEXT( ctx );
   GLuint vertsize = rmesa->vertex_size;
   GLuint *vb = r128AllocDmaLow( rmesa, (n-2) * 3 * 4 * vertsize );
   GLubyte *r128verts = (GLubyte *)rmesa->verts;
   const GLuint shift = rmesa->vertex_stride_shift;
   const GLuint *start = (const GLuint *)VERT(elts[0]);
   int i,j;

   rmesa->num_verts += (n-2) * 3;

   for (i = 2 ; i < n ; i++) {
      COPY_DWORDS( j, vb, vertsize, (r128VertexPtr) start );
      COPY_DWORDS( j, vb, vertsize, (r128VertexPtr) VERT(elts[i-1]) );
      COPY_DWORDS( j, vb, vertsize, (r128VertexPtr) VERT(elts[i]) );
   }
}
*/



/**********************************************************************/
/*                    Choose render functions                         */
/**********************************************************************/

#define _SAVAGE_NEW_RENDER_STATE (_DD_NEW_LINE_STIPPLE |	\
			          _DD_NEW_LINE_SMOOTH |		\
			          _DD_NEW_POINT_SMOOTH |	\
			          _DD_NEW_TRI_SMOOTH |		\
			          _DD_NEW_TRI_UNFILLED |	\
			          _DD_NEW_TRI_LIGHT_TWOSIDE |	\
			          _DD_NEW_TRI_OFFSET)		\

/* original driver didn't have DD_POINT_SMOOTH. really needed? */
#define POINT_FALLBACK (DD_POINT_SMOOTH)
#define LINE_FALLBACK (DD_LINE_STIPPLE|DD_LINE_SMOOTH)
#define TRI_FALLBACK (DD_TRI_SMOOTH)
#define ANY_FALLBACK_FLAGS (POINT_FALLBACK|LINE_FALLBACK|TRI_FALLBACK)
#define ANY_RASTER_FLAGS (DD_TRI_LIGHT_TWOSIDE|DD_TRI_OFFSET|DD_TRI_UNFILLED)


static void savageChooseRenderState(GLcontext *ctx)
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   GLuint flags = ctx->_TriangleCaps;
   GLuint index = 0;

   if (flags & (ANY_RASTER_FLAGS|ANY_FALLBACK_FLAGS)) {
      imesa->draw_point = savage_draw_point;
      imesa->draw_line = savage_draw_line;
      imesa->draw_tri = savage_draw_triangle;

      if (flags & ANY_RASTER_FLAGS) {
	 if (flags & DD_TRI_LIGHT_TWOSIDE) index |= SAVAGE_TWOSIDE_BIT;
	 if (flags & DD_TRI_OFFSET)        index |= SAVAGE_OFFSET_BIT;
	 if (flags & DD_TRI_UNFILLED)      index |= SAVAGE_UNFILLED_BIT;
      }

      /* Hook in fallbacks for specific primitives.
       */
      if (flags & (POINT_FALLBACK|LINE_FALLBACK|TRI_FALLBACK)) {
	 if (flags & POINT_FALLBACK) imesa->draw_point = savage_fallback_point;
	 if (flags & LINE_FALLBACK)  imesa->draw_line = savage_fallback_line;
	 if (flags & TRI_FALLBACK)   imesa->draw_tri = savage_fallback_tri;
	 index |= SAVAGE_FALLBACK_BIT;
      }
   }

   if (index != imesa->RenderIndex) {
      TNLcontext *tnl = TNL_CONTEXT(ctx);
      tnl->Driver.Render.Points = rast_tab[index].points;
      tnl->Driver.Render.Line = rast_tab[index].line;
      tnl->Driver.Render.Triangle = rast_tab[index].triangle;
      tnl->Driver.Render.Quad = rast_tab[index].quad;

      if (index == 0) {
	 tnl->Driver.Render.PrimTabVerts = savage_render_tab_verts;
	 tnl->Driver.Render.PrimTabElts = savage_render_tab_elts;
	 tnl->Driver.Render.ClippedLine = rast_tab[index].line;
	 tnl->Driver.Render.ClippedPolygon = savageRenderClippedPoly/*r128FastRenderClippedPoly*/;
      } else {
	 tnl->Driver.Render.PrimTabVerts = _tnl_render_tab_verts;
	 tnl->Driver.Render.PrimTabElts = _tnl_render_tab_elts;
	 tnl->Driver.Render.ClippedLine = savageRenderClippedLine;
	 tnl->Driver.Render.ClippedPolygon = savageRenderClippedPoly;
      }

      imesa->RenderIndex = index;
   }
}

/**********************************************************************/
/*                 Validate state at pipeline start                   */
/**********************************************************************/

static void savageRunPipeline( GLcontext *ctx )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);

   if (imesa->new_state)
      savageDDUpdateHwState( ctx );

   if (!imesa->Fallback && imesa->new_gl_state) {
      if (imesa->new_gl_state & _SAVAGE_NEW_RENDER_STATE)
	 savageChooseRenderState( ctx );

      imesa->new_gl_state = 0;
   }

   _tnl_run_pipeline( ctx );
}

/**********************************************************************/
/*                 High level hooks for t_vb_render.c                 */
/**********************************************************************/

/* This is called when Mesa switches between rendering triangle
 * primitives (such as GL_POLYGON, GL_QUADS, GL_TRIANGLE_STRIP, etc),
 * and lines, points and bitmaps.
 *
 * As the r128 uses triangles to render lines and points, it is
 * necessary to turn off hardware culling when rendering these
 * primitives.
 */

static void savageRasterPrimitive( GLcontext *ctx, GLuint prim )
{
   savageContextPtr imesa = SAVAGE_CONTEXT( ctx );

   /* Update culling */
   if (imesa->raster_primitive != prim) {
      imesa->raster_primitive = prim;
      imesa->new_state |= SAVAGE_NEW_CULL;
      savageDDUpdateHwState (ctx);
   }

#if 0
   if (ctx->Polygon.StippleFlag && mmesa->haveHwStipple)
   {
      mmesa->dirty |= MGA_UPLOAD_CONTEXT;
      mmesa->setup.dwgctl &= ~(0xf<<20);
      if (mmesa->raster_primitive == GL_TRIANGLES)
	 mmesa->setup.dwgctl |= mmesa->poly_stipple;
   }
#endif
}

static void savageRenderPrimitive( GLcontext *ctx, GLenum prim )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   GLuint rprim = reduced_prim[prim];

   imesa->render_primitive = prim;

   if (rprim == GL_TRIANGLES && (ctx->_TriangleCaps & DD_TRI_UNFILLED))
      return;
       
   if (imesa->raster_primitive != rprim) {
      savageRasterPrimitive( ctx, rprim );
   }
}


#define EMIT_ATTR( ATTR, STYLE, SKIP )					\
do {									\
   imesa->vertex_attrs[imesa->vertex_attr_count].attrib = (ATTR);	\
   imesa->vertex_attrs[imesa->vertex_attr_count].format = (STYLE);	\
   imesa->vertex_attr_count++;						\
   drawCmd &= ~SKIP;							\
} while (0)

static void savageRenderStart( GLcontext *ctx )
{
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   struct vertex_buffer *VB = &tnl->vb;
   GLuint index = tnl->render_inputs;
   GLuint drawCmd = SAVAGE_HW_SKIPFLAGS;
   if (imesa->savageScreen->chipset < S3_SAVAGE4)
      drawCmd &= ~SAVAGE_HW_NO_UV1;
   drawCmd &= ~SAVAGE_HW_NO_Z; /* all mesa vertices have a z coordinate */

   /* Important:
    */
   VB->AttribPtr[VERT_ATTRIB_POS] = VB->NdcPtr;
   imesa->vertex_attr_count = 0;
 
   /* EMIT_ATTR's must be in order as they tell t_vertex.c how to
    * build up a hardware vertex.
    */
   if ((index & _TNL_BITS_TEX_ANY) || !(ctx->_TriangleCaps & DD_FLATSHADE)) {
      EMIT_ATTR( _TNL_ATTRIB_POS, EMIT_4F_VIEWPORT, SAVAGE_HW_NO_W );
   }
   else {
      EMIT_ATTR( _TNL_ATTRIB_POS, EMIT_3F_VIEWPORT, 0 );
   }

   /* t_context.c always includes a diffuse color */
   EMIT_ATTR( _TNL_ATTRIB_COLOR0, EMIT_4UB_4F_BGRA, SAVAGE_HW_NO_CD );
      
   if (index & (_TNL_BIT_COLOR1|_TNL_BIT_FOG)) {
      EMIT_ATTR( _TNL_ATTRIB_COLOR1, EMIT_3UB_3F_BGR, SAVAGE_HW_NO_CS );
      EMIT_ATTR( _TNL_ATTRIB_FOG, EMIT_1UB_1F, SAVAGE_HW_NO_CS );
   }

   if (index & _TNL_BIT_TEX(0)) {
      if (VB->TexCoordPtr[0]->size > 2) {
	 /* projective textures are not supported by the hardware */
	 FALLBACK(ctx, SAVAGE_FALLBACK_TEXTURE, GL_TRUE);
      }
      if (VB->TexCoordPtr[0]->size == 2)
	 EMIT_ATTR( _TNL_ATTRIB_TEX0, EMIT_2F, SAVAGE_HW_NO_UV0 );
      else
	 EMIT_ATTR( _TNL_ATTRIB_TEX0, EMIT_1F, SAVAGE_HW_NO_U0 );
   }
   if (index & _TNL_BIT_TEX(1)) {
      if (VB->TexCoordPtr[1]->size > 2) {
	 /* projective textures are not supported by the hardware */
	 FALLBACK(ctx, SAVAGE_FALLBACK_TEXTURE, GL_TRUE);
      }
      if (VB->TexCoordPtr[1]->size == 2)
	 EMIT_ATTR( _TNL_ATTRIB_TEX1, EMIT_2F, SAVAGE_HW_NO_UV1 );
      else
	 EMIT_ATTR( _TNL_ATTRIB_TEX1, EMIT_1F, SAVAGE_HW_NO_U1 );
   }

   /* Only need to change the vertex emit code if there has been a
    * statechange to a new hardware vertex format and also when the
    * vertex format is set for the first time. This is indicated by
    * imesa->vertex_size == 0.
    */
   if (drawCmd != (imesa->DrawPrimitiveCmd & SAVAGE_HW_SKIPFLAGS) ||
       imesa->vertex_size == 0) {
      imesa->vertex_size = 
	 _tnl_install_attrs( ctx, 
			     imesa->vertex_attrs, 
			     imesa->vertex_attr_count,
			     imesa->hw_viewport, 0 );
      imesa->vertex_size >>= 2;

      imesa->DrawPrimitiveCmd = drawCmd;
   }

   if (!SAVAGE_CONTEXT(ctx)->Fallback) {
      /* Update hardware state and get the lock */
      savageDDRenderStart( ctx );
   } else {
      tnl->Driver.Render.Start(ctx);
   }
}

static void savageRenderFinish( GLcontext *ctx )
{
   /* Release the lock */
   savageDDRenderEnd( ctx );

   /* Flush the last primitive now, before any state is changed.
    * Alternatively state could be emitted in all state-changing
    * functions in savagestate.c. */
   FLUSH_BATCH(SAVAGE_CONTEXT(ctx));

   if (SAVAGE_CONTEXT(ctx)->RenderIndex & SAVAGE_FALLBACK_BIT)
      _swrast_flush( ctx );
}


/**********************************************************************/
/*           Transition to/from hardware rasterization.               */
/**********************************************************************/

void savageFallback( GLcontext *ctx, GLuint bit, GLboolean mode )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   savageContextPtr imesa = SAVAGE_CONTEXT(ctx);
   GLuint oldfallback = imesa->Fallback;

   if (mode) {
      imesa->Fallback |= bit;
      if (oldfallback == 0) {
	 /* the first fallback */
	 FLUSH_BATCH( imesa );
	 _swsetup_Wakeup( ctx );
	 imesa->RenderIndex = ~0;
      }
   }
   else {
      imesa->Fallback &= ~bit;
      if (oldfallback == bit) {
	 /* the last fallback */
	 _swrast_flush( ctx );
	 tnl->Driver.Render.Start = savageRenderStart;
	 tnl->Driver.Render.PrimitiveNotify = savageRenderPrimitive;
	 tnl->Driver.Render.Finish = savageRenderFinish;

	 tnl->Driver.Render.BuildVertices = _tnl_build_vertices;
	 tnl->Driver.Render.CopyPV = _tnl_copy_pv;
	 tnl->Driver.Render.Interp = _tnl_interp;

	 _tnl_invalidate_vertex_state( ctx, ~0 );
	 _tnl_invalidate_vertices( ctx, ~0 );
	 _tnl_install_attrs( ctx, 
			     imesa->vertex_attrs, 
			     imesa->vertex_attr_count,
			     imesa->hw_viewport, 0 ); 

	 imesa->new_gl_state |= _SAVAGE_NEW_RENDER_STATE;
      }
   }
}


/**********************************************************************/
/*                            Initialization.                         */
/**********************************************************************/

void savageInitTriFuncs( GLcontext *ctx )
{
   TNLcontext *tnl = TNL_CONTEXT(ctx);
   static int firsttime = 1;

   if (firsttime) {
      init_rast_tab();
      firsttime = 0;
   }

   tnl->Driver.RunPipeline = savageRunPipeline;
   tnl->Driver.Render.Start = savageRenderStart;
   tnl->Driver.Render.Finish = savageRenderFinish;
   tnl->Driver.Render.PrimitiveNotify = savageRenderPrimitive;
   tnl->Driver.Render.ResetLineStipple = _swrast_ResetLineStipple;

   tnl->Driver.Render.BuildVertices = _tnl_build_vertices;
   tnl->Driver.Render.CopyPV = _tnl_copy_pv;
   tnl->Driver.Render.Interp = _tnl_interp;

   _tnl_init_vertices( ctx, ctx->Const.MaxArrayLockSize + 12, 
		       (6 + 2*ctx->Const.MaxTextureUnits) * sizeof(GLfloat) );
   
   SAVAGE_CONTEXT(ctx)->verts = (char *)tnl->clipspace.vertex_buf;
}
