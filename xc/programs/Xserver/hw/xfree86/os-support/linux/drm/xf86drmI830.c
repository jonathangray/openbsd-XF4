/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/drm/xf86drmI830.c,v 1.1 2001/10/04 18:28:22 alanh Exp $ */

#ifdef XFree86Server
# include "xf86.h"
# include "xf86_OSproc.h"
# include "xf86_ansic.h"
# include "xf86Priv.h"
# define _DRM_MALLOC xalloc
# define _DRM_FREE   xfree
# ifndef XFree86LOADER
#  include <sys/stat.h>
#  include <sys/mman.h>
# endif
#else
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <ctype.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# include <sys/mman.h>
# include <sys/time.h>
# ifdef DRM_USE_MALLOC
#  define _DRM_MALLOC malloc
#  define _DRM_FREE   free
extern int xf86InstallSIGIOHandler(int fd, void (*f)(int, void *), void *);
extern int xf86RemoveSIGIOHandler(int fd);
# else
#  include <Xlibint.h>
#  define _DRM_MALLOC Xmalloc
#  define _DRM_FREE   Xfree
# endif
#endif

/* Not all systems have MAP_FAILED defined */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#ifdef __linux__
#include <sys/sysmacros.h>	/* for makedev() */
#endif

#include "xf86drm.h"
#include "drm.h"
#include "xf86drmI830.h"

Bool drmI830CleanupDma(int driSubFD)
{
   drm_i830_init_t init;
   
   memset(&init, 0, sizeof(drm_i830_init_t));
   init.func = I810_CLEANUP_DMA;
   
   if(ioctl(driSubFD, DRM_IOCTL_I830_INIT, &init)) {
      return FALSE;
   }
   
   return TRUE;
}

Bool drmI830InitDma(int driSubFD, drmI830Init *info)
{
   drm_i830_init_t init;
   
   memset(&init, 0, sizeof(drm_i830_init_t));

   init.func = I810_INIT_DMA;
   init.mmio_offset = info->mmio_offset;
   init.buffers_offset = info->buffers_offset;
   init.ring_start = info->start;
   init.ring_end = info->end;
   init.ring_size = info->size;
   init.sarea_priv_offset = info->sarea_off;
   init.front_offset = info->front_offset;
   init.back_offset = info->back_offset;
   init.depth_offset = info->depth_offset;
   init.w = info->w;
   init.h = info->h;
   init.pitch = info->pitch;
   init.pitch_bits = info->pitch_bits;
   init.back_pitch = info->pitch;
   init.depth_pitch = info->pitch;
   init.cpp = info->cpp;

   if(ioctl(driSubFD, DRM_IOCTL_I830_INIT, &init)) {
      return FALSE;
   }
   return TRUE;
}