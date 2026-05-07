/*
 *  Copyright (c) 2020, Rockchip Electronics Co., Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <xf86drm.h>

#ifdef HAS_GBM
#include <gbm.h>
#endif

#ifdef HAS_EGL
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0
#endif

#ifndef DRM_FORMAT_MOD_INVALID
#define DRM_FORMAT_MOD_INVALID ((1ULL<<56) - 1)
#endif

/* A stub symbol to ensure that the hook library would not be removed as unused */
int mali_injected = 0;

/* Override libmali symbols */

#ifdef HAS_GBM
static struct gbm_surface * (* _gbm_surface_create)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t) = NULL;
#ifdef HAS_gbm_surface_create_with_modifiers
static struct gbm_surface *(* _gbm_surface_create_with_modifiers) (struct gbm_device *gbm, uint32_t width, uint32_t height, uint32_t format, const uint64_t *modifiers, const unsigned int count);
#endif
static struct gbm_bo * (* _gbm_bo_create)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t) = NULL;
#ifdef HAS_gbm_bo_create_with_modifiers
static struct gbm_bo * (* _gbm_bo_create_with_modifiers)(struct gbm_device *gbm, uint32_t width, uint32_t height, uint32_t format, const uint64_t *modifiers, const unsigned int count) = NULL;
#endif
#ifdef HAS_gbm_bo_get_modifier
static uint64_t (* _gbm_bo_get_modifier)(struct gbm_bo *bo) = NULL;
#endif
#endif // HAS_GBM

#ifdef HAS_EGL
static PFNEGLGETCURRENTSURFACEPROC _eglGetCurrentSurface = NULL;
static PFNEGLGETDISPLAYPROC _eglGetDisplay = NULL;
static PFNEGLGETPROCADDRESSPROC _eglGetProcAddress = NULL;
static PFNEGLCHOOSECONFIGPROC _eglChooseConfig = NULL;
static EGLBoolean (* _eglDestroySurface)(EGLDisplay dpy, EGLSurface surface) = NULL;
static PFNEGLMAKECURRENTPROC _eglMakeCurrent = NULL;
#endif // HAS_EGL

#define MALI_SYMBOL(func) { #func, (void **)(&_ ## func), }
static struct {
   const char *func;
   void **symbol;
} mali_symbols[] = {
#ifdef HAS_GBM
   MALI_SYMBOL(gbm_surface_create),
#ifdef HAS_gbm_surface_create_with_modifiers
   MALI_SYMBOL(gbm_surface_create_with_modifiers),
#endif
   MALI_SYMBOL(gbm_bo_create),
#ifdef HAS_gbm_bo_create_with_modifiers
   MALI_SYMBOL(gbm_bo_create_with_modifiers),
#endif
#ifdef HAS_gbm_bo_get_modifier
   MALI_SYMBOL(gbm_bo_get_modifier),
#endif
#endif // HAS_GBM
#ifdef HAS_EGL
   MALI_SYMBOL(eglGetCurrentSurface),
   MALI_SYMBOL(eglGetDisplay),
   MALI_SYMBOL(eglGetProcAddress),
   MALI_SYMBOL(eglChooseConfig),
   MALI_SYMBOL(eglDestroySurface),
   MALI_SYMBOL(eglMakeCurrent),
#endif // HAS_EGL
};

__attribute__((constructor)) static void
load_mali_symbols(void)
{
   void *handle, *symbol;
   int i;

   /* The libmali should be already loaded */
   handle = dlopen(LIBMALI_SO, RTLD_LAZY | RTLD_NOLOAD);
   if (!handle) {
      /* Should not reach here */
      fprintf(stderr, "[MALI-HOOK] FATAL: dlopen(" LIBMALI_SO ") failed(%s)\n",
              dlerror());
      exit(-1);
   }

   for (i = 0; i < ARRAY_SIZE(mali_symbols); i++) {
      const char *func = mali_symbols[i].func;

      /* Clear error */
      dlerror();

      symbol = dlsym(handle, func);
      if (!symbol) {
         /* Should not reach here */
         fprintf(stderr, "[MALI-HOOK] FATAL: " LIBMALI_SO
                 " dlsym(%s) failed(%s)\n", func, dlerror());
         exit(-1);
      }

      *mali_symbols[i].symbol = symbol;
   }

   dlclose(handle);
}

#ifdef HAS_GBM

/* Implement new GBM APIs */

__attribute__((unused)) static inline bool
can_ignore_modifiers(const uint64_t *modifiers,
                     const unsigned int count)
{
   for (int i = 0; i < count; i++) {
      if (modifiers[i] == DRM_FORMAT_MOD_LINEAR ||
          modifiers[i] == DRM_FORMAT_MOD_INVALID) {
         return true;
      }
   }

   return !count;
}

#ifndef HAS_gbm_bo_create_with_modifiers2
struct gbm_bo *
gbm_bo_create_with_modifiers2(struct gbm_device *gbm,
                              uint32_t width, uint32_t height,
                              uint32_t format,
                              const uint64_t *modifiers,
                              const unsigned int count,
                              uint32_t flags)
{
#ifdef HAS_gbm_bo_create_with_modifiers
   /* flags ignored */
   return _gbm_bo_create_with_modifiers(gbm, width, height, format,
                                        modifiers, count);
#else
   if (!can_ignore_modifiers(modifiers, count))
      return NULL;

   return gbm_bo_create(gbm, width, height, format, flags);
#endif
}
#endif

#ifndef HAS_gbm_bo_create_with_modifiers
struct gbm_bo *
gbm_bo_create_with_modifiers(struct gbm_device *gbm,
                             uint32_t width, uint32_t height,
                             uint32_t format,
                             const uint64_t *modifiers,
                             const unsigned int count)
{
   return gbm_bo_create_with_modifiers2(gbm, width, height, format,
                                        modifiers, count, GBM_BO_USE_SCANOUT);
}
#endif

#ifndef HAS_gbm_surface_create_with_modifiers2
struct gbm_surface *
gbm_surface_create_with_modifiers2(struct gbm_device *gbm,
                                   uint32_t width, uint32_t height,
                                   uint32_t format,
                                   const uint64_t *modifiers,
                                   const unsigned int count,
                                   uint32_t flags)
{
#ifdef HAS_gbm_surface_create_with_modifiers
   /* flags ignored */
   return _gbm_surface_create_with_modifiers(gbm, width, height, format,
                                             modifiers, count);
#else
   if (!can_ignore_modifiers(modifiers, count))
      return NULL;

   return gbm_surface_create(gbm, width, height, format, 0);
#endif
}
#endif

#ifndef HAS_gbm_surface_create_with_modifiers
struct gbm_surface *
gbm_surface_create_with_modifiers(struct gbm_device *gbm,
                                  uint32_t width, uint32_t height,
                                  uint32_t format,
                                  const uint64_t *modifiers,
                                  const unsigned int count)
{
   return gbm_surface_create_with_modifiers2(gbm, width, height, format,
                                             modifiers, count,
                                             GBM_BO_USE_SCANOUT);
}
#endif

/* Wrappers for invalid modifier */

uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo)
{
#ifdef HAS_gbm_bo_get_modifier
   uint64_t modifier = _gbm_bo_get_modifier(bo);
   if (modifier != DRM_FORMAT_MOD_INVALID)
      return modifier;
#endif
   return DRM_FORMAT_MOD_LINEAR;
}

/* Wrappers for unsupported flags */

struct gbm_surface *
gbm_surface_create(struct gbm_device *gbm,
                   uint32_t width, uint32_t height,
                   uint32_t format, uint32_t flags)
{
   struct gbm_surface *surface;

   surface = _gbm_surface_create(gbm, width, height, format, flags);
   if (surface)
      return surface;

   flags &= GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
   return _gbm_surface_create(gbm, width, height, format, flags);
}

struct gbm_bo *
gbm_bo_create(struct gbm_device *gbm,
              uint32_t width, uint32_t height,
              uint32_t format, uint32_t flags)
{
   struct gbm_bo *bo;

   bo = _gbm_bo_create(gbm, width, height, format, flags);
   if (bo)
      return bo;

   flags &= GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING |
      GBM_BO_USE_WRITE | GBM_BO_USE_CURSOR_64X64;
   return _gbm_bo_create(gbm, width, height, format, flags);
}

#endif // HAS_GBM

#ifdef HAS_EGL

/* Override EGL symbols */

EGLAPI EGLDisplay EGLAPIENTRY
eglGetDisplay(EGLNativeDisplayType display_id)
{
   /* HACK: For chromium angle with in-process-gpu. */
   if (getenv("MALI_FORCE_DEFAULT_DISPLAY") &&
       display_id != EGL_DEFAULT_DISPLAY) {
      fprintf(stderr, "[MALI-HOOK] WARN: Native display(%p) ignored!\n",
              display_id);
      display_id = EGL_DEFAULT_DISPLAY;
   }

   return _eglGetDisplay(display_id);
}

/* Export for EGL 1.5 */

/* HACK: Unset current surface before destroying it */

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
   if (_eglGetCurrentSurface(EGL_DRAW) == surface ||
       _eglGetCurrentSurface(EGL_READ) == surface)
      _eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

   return _eglDestroySurface(dpy, surface);
}

/* HACK: Fixup EGL_OPENGL_BIT */

EGLBoolean eglChooseConfig (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
#define MAX_EGL_ATTRS 1024
   EGLint list[MAX_EGL_ATTRS];
   int i = 0;

   if (!attrib_list)
      return _eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);

   while (attrib_list[i] != EGL_NONE) {
      if (i > MAX_EGL_ATTRS - 2)
         return EGL_FALSE;

      list[i] = attrib_list[i];
      list[i + 1] = attrib_list[i + 1];

      if (list[i] == EGL_RENDERABLE_TYPE && list[i + 1] == EGL_OPENGL_BIT)
         list[i + 1] = EGL_OPENGL_ES_BIT;

      i += 2;
   }
   list[i] = EGL_NONE;

   return _eglChooseConfig(dpy, list, configs, config_size, num_config);
}

/* Override proc addesses */

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
   if (!procname)
      return NULL;

   if (!strcmp(procname, __func__))
      return (__eglMustCastToProperFunctionPointerType)eglGetProcAddress;

   if (!strcmp(procname, "eglGetDisplay"))
      return (__eglMustCastToProperFunctionPointerType)eglGetDisplay;

   if (!strcmp(procname, "eglChooseConfig"))
      return (__eglMustCastToProperFunctionPointerType)eglChooseConfig;

   if (!strcmp(procname, "eglDestroySurface"))
      return (__eglMustCastToProperFunctionPointerType)eglDestroySurface;

   return _eglGetProcAddress(procname);
}

#endif // HAS_EGL
