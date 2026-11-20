/*
 * Copyright 2022 Rockchip Electronics Co., Ltd
 *     Author: Jeffy Chen <jeffy.chen@rock-chips.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include <gst/video/video.h>
#include <gst/base/gstpushsrc.h>
#include <gst/allocators/gstdmabuf.h>

#include <gst/video/video-info-dma.h>

#ifndef DRM_FORMAT_NV12_10
#define DRM_FORMAT_NV12_10 fourcc_code('N', 'A', '1', '2')
#endif

#ifndef DRM_FORMAT_NV15
#define DRM_FORMAT_NV15 fourcc_code('N', 'V', '1', '5')
#endif

#define DEFAULT_PROP_FRAMERATE_LIMIT 120

/* Re-emit an unchanged FB after this long without a page flip, so a
 * static screen keeps streaming instead of blocking forever. */
#define KMS_SRC_STATIC_GRACE (250 * GST_MSECOND)

#define GST_TYPE_KMS_SRC (gst_kms_src_get_type())
G_DECLARE_FINAL_TYPE (GstKmsSrc, gst_kms_src, GST, KMS_SRC, GstPushSrc);

struct _GstKmsSrc
{
  GstBaseSrc element;

  GstAllocator *allocator;
  GstVideoInfo info;

  gchar *devname;
  gchar *bus_id;

  gint fd;

  guint crtc_id;
  guint encoder_id;
  guint connector_id;
  guint plane_id;
  guint fb_id;


  guint framerate_limit;
  guint fps_n;
  guint fps_d;
  gboolean sync_fb;
  gboolean sync_vblank;

  GstPoll *poll;
  GstPollFD pollfd;

  guint last_fb_id;
  GstClockTime last_frame_time;

  /* When the FB content was first seen unchanged (0 = just changed) */
  GstClockTime static_since;

  /* DRM fourcc of the current FB, and of the negotiated DMA_DRM caps */
  guint32 fb_fourcc;
  guint32 caps_fourcc;

  /* Layout modifier of the current FB (LINEAR when the driver reports
   * none) and the one the caps were negotiated with */
  guint64 fb_modifier;
  guint64 neg_modifier;

  /* Caps geometry currently negotiated downstream */
  GstVideoFormat neg_format;
  gint neg_width, neg_height;
  gboolean neg_dmabuf;

  GstClockTime start_time;
};

#define parent_class gst_kms_src_parent_class
G_DEFINE_TYPE (GstKmsSrc, gst_kms_src, GST_TYPE_PUSH_SRC);

#define GST_KMS_SRC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_KMS_SRC, GstKmsSrc))

#define GST_CAT_DEFAULT kms_src_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

enum
{
  PROP_DRIVER_NAME = 1,
  PROP_BUS_ID,
  PROP_CRTC_ID,
  PROP_ENCODER_ID,
  PROP_CONNECTOR_ID,
  PROP_PLANE_ID,
  PROP_FB_ID,
  PROP_FRAMERATE_LIMIT,
  PROP_SYNC_FB,
  PROP_SYNC_VBLANK,
  PROP_LAST,
};

struct kmssrc_fb
{
  guint handles[4];
  guint pitches[4];
  guint offsets[4];
  guint width;
  guint height;
  guint fourcc;
  /* Layout modifier as reported by GETFB2 (LINEAR when the driver
   * has no fb modifiers); the caps must describe the captured layout. */
  guint64 modifier;
};

static void
gst_kms_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstKmsSrc *self = GST_KMS_SRC (object);

  switch (prop_id) {
    case PROP_DRIVER_NAME:
      g_free (self->devname);
      self->devname = g_value_dup_string (value);
      break;
    case PROP_BUS_ID:
      g_free (self->bus_id);
      self->bus_id = g_value_dup_string (value);
      break;
    case PROP_CRTC_ID:
      self->crtc_id = g_value_get_uint (value);
      break;
    case PROP_ENCODER_ID:
      self->encoder_id = g_value_get_uint (value);
      break;
    case PROP_CONNECTOR_ID:
      self->connector_id = g_value_get_uint (value);
      break;
    case PROP_PLANE_ID:
      self->plane_id = g_value_get_uint (value);
      break;
    case PROP_FB_ID:
      self->fb_id = g_value_get_uint (value);
      break;
    case PROP_FRAMERATE_LIMIT:
      self->framerate_limit = g_value_get_uint (value);
      break;
    case PROP_SYNC_FB:
      self->sync_fb = g_value_get_boolean (value);
      break;
    case PROP_SYNC_VBLANK:
      self->sync_vblank = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_kms_src_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstKmsSrc *self = GST_KMS_SRC (object);

  switch (prop_id) {
    case PROP_DRIVER_NAME:
      g_value_set_string (value, self->devname);
      break;
    case PROP_BUS_ID:
      g_value_set_string (value, self->bus_id);
      break;
    case PROP_CRTC_ID:
      g_value_set_uint (value, self->crtc_id);
      break;
    case PROP_ENCODER_ID:
      g_value_set_uint (value, self->encoder_id);
      break;
    case PROP_CONNECTOR_ID:
      g_value_set_uint (value, self->connector_id);
      break;
    case PROP_PLANE_ID:
      g_value_set_uint (value, self->plane_id);
      break;
    case PROP_FB_ID:
      g_value_set_uint (value, self->fb_id);
      break;
    case PROP_FRAMERATE_LIMIT:
      g_value_set_uint (value, self->framerate_limit);
      break;
    case PROP_SYNC_FB:
      g_value_set_boolean (value, self->sync_fb);
      break;
    case PROP_SYNC_VBLANK:
      g_value_set_boolean (value, self->sync_vblank);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static guint
gst_kms_src_get_crtc_fb (GstKmsSrc * self, guint crtc_id)
{
  drmModeCrtcPtr crtc;
  guint fb_id;

  crtc = drmModeGetCrtc (self->fd, crtc_id);
  if (!crtc)
    return 0;

  fb_id = crtc->buffer_id;

  drmModeFreeCrtc (crtc);
  return fb_id;
}

static guint
gst_kms_src_get_encoder_fb (GstKmsSrc * self, guint encoder_id)
{
  drmModeEncoderPtr encoder;
  guint fb_id;

  encoder = drmModeGetEncoder (self->fd, encoder_id);
  if (!encoder)
    return 0;

  fb_id = gst_kms_src_get_crtc_fb (self, encoder->crtc_id);

  drmModeFreeEncoder (encoder);
  return fb_id;
}

static guint
gst_kms_src_get_connector_fb (GstKmsSrc * self, guint connector_id)
{
  drmModeConnectorPtr connector;
  guint fb_id;

  connector = drmModeGetConnector (self->fd, connector_id);
  if (!connector)
    return 0;

  fb_id = gst_kms_src_get_encoder_fb (self, connector->encoder_id);

  drmModeFreeConnector (connector);
  return fb_id;
}

static guint
gst_kms_src_get_plane_fb (GstKmsSrc * self, guint plane_id)
{
  drmModePlanePtr plane;
  guint fb_id;

  plane = drmModeGetPlane (self->fd, plane_id);
  if (!plane)
    return 0;

  fb_id = plane->fb_id;

  drmModeFreePlane (plane);
  return fb_id;
}

static guint
gst_kms_src_get_fb_id (GstKmsSrc * self)
{
  if (self->fb_id)
    return self->fb_id;

  if (self->plane_id)
    return gst_kms_src_get_plane_fb (self, self->plane_id);

  if (self->connector_id)
    return gst_kms_src_get_connector_fb (self, self->connector_id);

  if (self->encoder_id)
    return gst_kms_src_get_encoder_fb (self, self->encoder_id);

  if (self->crtc_id)
    return gst_kms_src_get_crtc_fb (self, self->crtc_id);

  return 0;
}

static guint
gst_kms_src_get_encoder_crtc (GstKmsSrc * self, guint encoder_id)
{
  drmModeEncoderPtr encoder;
  guint crtc_id;

  encoder = drmModeGetEncoder (self->fd, encoder_id);
  if (!encoder)
    return 0;

  crtc_id = encoder->crtc_id;

  drmModeFreeEncoder (encoder);
  return crtc_id;
}

static guint
gst_kms_src_get_connector_crtc (GstKmsSrc * self, guint connector_id)
{
  drmModeConnectorPtr connector;
  guint crtc_id;

  connector = drmModeGetConnector (self->fd, connector_id);
  if (!connector)
    return 0;

  crtc_id = gst_kms_src_get_encoder_crtc (self, connector->encoder_id);

  drmModeFreeConnector (connector);
  return crtc_id;
}

static guint
gst_kms_src_get_plane_crtc (GstKmsSrc * self, guint plane_id)
{
  drmModePlanePtr plane;
  guint crtc_id;

  plane = drmModeGetPlane (self->fd, plane_id);
  if (!plane)
    return 0;

  crtc_id = plane->crtc_id;

  drmModeFreePlane (plane);
  return crtc_id;
}

static guint
gst_kms_src_get_crtc_id (GstKmsSrc * self)
{
  if (self->crtc_id)
    return self->crtc_id;

  if (self->plane_id)
    return gst_kms_src_get_plane_crtc (self, self->plane_id);

  if (self->connector_id)
    return gst_kms_src_get_connector_crtc (self, self->connector_id);

  if (self->encoder_id)
    return gst_kms_src_get_encoder_crtc (self, self->encoder_id);

  return 0;
}

static guint
gst_kms_src_get_crtc_pipe (GstKmsSrc * self, guint crtc_id)
{
  drmModeResPtr res;
  guint crtc_pipe = 0;
  gint i;

  if (!crtc_id)
    return 0;

  res = drmModeGetResources (self->fd);
  if (!res)
    return 0;

  for (i = 0; i < res->count_crtcs; i++) {
    if (res->crtcs[i] == crtc_id) {
      crtc_pipe = i;
      break;
    }
  }
  drmModeFreeResources (res);
  return crtc_pipe;
}

static void
sync_handler (gint fd, guint frame, guint sec, guint usec, gpointer data)
{
  gboolean *waiting;

  (void) fd;
  (void) frame;
  (void) sec;
  (void) usec;

  waiting = data;
  *waiting = FALSE;
}

static gboolean
gst_kms_src_sync_vblank (GstKmsSrc * self)
{
  gboolean waiting;
  drmEventContext evctxt = {
    .version = DRM_EVENT_CONTEXT_VERSION,
    .vblank_handler = sync_handler,
  };
  drmVBlank vbl = {
    .request = {
          .type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT,
          .sequence = 1,
          .signal = (gulong) & waiting,
        },
  };
  guint crtc_id = gst_kms_src_get_crtc_id (self);
  guint crtc_pipe = gst_kms_src_get_crtc_pipe (self, crtc_id);
  gint ret;

  if (crtc_pipe == 1)
    vbl.request.type |= DRM_VBLANK_SECONDARY;
  else if (crtc_pipe > 1)
    vbl.request.type |= crtc_pipe << DRM_VBLANK_HIGH_CRTC_SHIFT;

  waiting = TRUE;
  ret = drmWaitVBlank (self->fd, &vbl);
  if (ret)
    return FALSE;

  while (waiting) {
    do {
      ret = gst_poll_wait (self->poll, 3 * GST_SECOND);
    } while (ret == -1 && (errno == EAGAIN || errno == EINTR));

    ret = drmHandleEvent (self->fd, &evctxt);
    if (ret)
      return FALSE;
  }

  GST_DEBUG_OBJECT (self, "sync vblank with CRTC: %d(%d)", crtc_id, crtc_pipe);
  return TRUE;
}

static guint
gst_kms_src_get_next_fb_id (GstKmsSrc * self)
{
  GstClockTimeDiff diff;
  GstClockTime now;
  gboolean sync_fb = self->sync_fb && !self->fb_id;
  guint duration = 0;
  guint fb_id;

  if (self->sync_vblank)
    gst_kms_src_sync_vblank (self);

  if (self->fps_d && self->fps_n)
    duration = gst_util_uint64_scale (GST_SECOND, self->fps_d, self->fps_n);
  else if (self->framerate_limit)
    duration = GST_SECOND / self->framerate_limit;

  while (1) {
    now = gst_util_get_timestamp ();
    diff = GST_CLOCK_DIFF (self->last_frame_time, now);

    fb_id = gst_kms_src_get_fb_id (self);
    if (!fb_id) {
      /* The compositor detaches every plane when fully idle, leaving no
       * legacy FB id on the CRTC. Fall back to the last FB: if it was
       * freed in the meantime the import fails cleanly, otherwise the
       * capture keeps streaming the last shown frame. */
      fb_id = self->last_fb_id;
      if (!fb_id)
        return 0;
    }

    if (!sync_fb || fb_id != self->last_fb_id) {
      sync_fb = FALSE;
      self->static_since = 0;

      if (diff >= duration)
        return fb_id;
    } else {
      /* Static screen: no flip will ever come. After a grace period
       * re-emit the unchanged FB so the capture keeps flowing. */
      if (!self->static_since) {
        self->static_since = now;
      } else if (now - self->static_since >= KMS_SRC_STATIC_GRACE &&
          diff >= duration) {
        GST_DEBUG_OBJECT (self, "FB %d unchanged, re-emitting", fb_id);
        return fb_id;
      }
    }

    g_usleep (1000);
  }

  return 0;
}

#ifndef HAS_DRM_CLOSE_HANDLE
/* From libdrm 2.4.109 : xf86drm.c */
static int
drmCloseBufferHandle (int fd, uint32_t handle)
{
  struct drm_gem_close args;

  memset (&args, 0, sizeof (args));
  args.handle = handle;
  return drmIoctl (fd, DRM_IOCTL_GEM_CLOSE, &args);
}
#endif

static void
gst_kms_src_free_fb (GstKmsSrc * self, struct kmssrc_fb *fb)
{
  guint i;

  for (i = 0; i < 4; i++) {
    if (fb->handles[i])
      drmCloseBufferHandle (self->fd, fb->handles[i]);

    fb->handles[i] = 0;
  }
}

static GstVideoFormat
gst_kms_src_fourcc_to_format (guint32 fourcc)
{
  GstVideoFormat format;

#define KMSSRC_CASE_FOURCC(fourcc, gst) \
  case DRM_FORMAT_ ## fourcc: format = GST_VIDEO_FORMAT_ ## gst; break;

  switch (fourcc) {
      KMSSRC_CASE_FOURCC (ARGB8888, BGRA);
      KMSSRC_CASE_FOURCC (XRGB8888, BGRx);
      KMSSRC_CASE_FOURCC (ABGR8888, RGBA);
      KMSSRC_CASE_FOURCC (XBGR8888, RGBx);
      KMSSRC_CASE_FOURCC (RGB888, BGR);
      KMSSRC_CASE_FOURCC (BGR888, RGB);
      KMSSRC_CASE_FOURCC (YUV422, Y42B);
      KMSSRC_CASE_FOURCC (NV16, NV16);
      KMSSRC_CASE_FOURCC (NV61, NV61);
      KMSSRC_CASE_FOURCC (UYVY, UYVY);
      KMSSRC_CASE_FOURCC (YVYU, YVYU);
      KMSSRC_CASE_FOURCC (YUYV, YUY2);
      KMSSRC_CASE_FOURCC (YUV420, I420);
      KMSSRC_CASE_FOURCC (YVU420, YV12);
      KMSSRC_CASE_FOURCC (NV12, NV12);
      KMSSRC_CASE_FOURCC (NV21, NV21);
      KMSSRC_CASE_FOURCC (RGB565, RGB16);
      KMSSRC_CASE_FOURCC (BGR565, BGR16);
#ifdef HAVE_NV12_10LE40
      KMSSRC_CASE_FOURCC (NV15, NV12_10LE40);
      KMSSRC_CASE_FOURCC (NV12_10, NV12_10LE40);
#endif
    default:
      return GST_VIDEO_FORMAT_UNKNOWN;
  }

  return format;
}

static gboolean
gst_kms_src_update_info (GstKmsSrc * self, struct kmssrc_fb *fb)
{
  GstVideoInfo *info = &self->info;
  GstVideoFormat format;
  guint i;

  self->fb_fourcc = fb->fourcc;
  self->fb_modifier = fb->modifier;

  format = gst_kms_src_fourcc_to_format (fb->fourcc);
  if (format == GST_VIDEO_FORMAT_UNKNOWN) {
    GST_ERROR_OBJECT (self, "format not supported %x", fb->fourcc);
    return FALSE;
  }

  gst_video_info_set_format (info, format, fb->width, fb->height);

  GST_VIDEO_INFO_SIZE (info) = 0;
  for (i = 0; i < GST_VIDEO_INFO_N_PLANES (info); i++) {
    GST_VIDEO_INFO_PLANE_STRIDE (info, i) = fb->pitches[i];
    GST_VIDEO_INFO_PLANE_OFFSET (info, i) = fb->offsets[i];

    GST_DEBUG_OBJECT (self, "plane %d, stride %d, offset %" G_GSIZE_FORMAT, i,
        GST_VIDEO_INFO_PLANE_STRIDE (info, i),
        GST_VIDEO_INFO_PLANE_OFFSET (info, i));

    GST_VIDEO_INFO_SIZE (info) += fb->pitches[i] * fb->height;
  }
  GST_DEBUG_OBJECT (self, "size %" G_GSIZE_FORMAT, GST_VIDEO_INFO_SIZE (info));

  return TRUE;
}

static gboolean
gst_kms_src_get_fb (GstKmsSrc * self, guint fb_id, struct kmssrc_fb *kmssrc_fb)
{
  drmModeFBPtr fb;
#ifdef HAS_DRM_MODE_FB2
  drmModeFB2Ptr fb2;
  guint i;
#endif

  memset (kmssrc_fb, 0, sizeof (*kmssrc_fb));

#ifdef HAS_DRM_MODE_FB2
  fb2 = drmModeGetFB2 (self->fd, fb_id);
  if (fb2) {
    for (i = 0; i < 4; i++) {
      kmssrc_fb->handles[i] = fb2->handles[i];
      kmssrc_fb->pitches[i] = fb2->pitches[i];
      kmssrc_fb->offsets[i] = fb2->offsets[i];
    }
    kmssrc_fb->fourcc = fb2->pixel_format;
    kmssrc_fb->width = fb2->width;
    kmssrc_fb->height = fb2->height;

    /* GETFB2 reports the real layout modifier when the driver supports
     * fb modifiers; normalize INVALID the same way everywhere so the
     * caps build and the set_caps comparison see identical values. */
    if ((fb2->flags & DRM_MODE_FB_MODIFIERS) &&
        fb2->modifier != DRM_FORMAT_MOD_INVALID)
      kmssrc_fb->modifier = fb2->modifier;
    else
      kmssrc_fb->modifier = DRM_FORMAT_MOD_LINEAR;

    drmModeFreeFB2 (fb2);
    return TRUE;
  }
#endif

  fb = drmModeGetFB (self->fd, fb_id);
  if (!fb)
    return FALSE;

  kmssrc_fb->handles[0] = fb->handle;
  kmssrc_fb->pitches[0] = fb->pitch;

  switch (fb->bpp) {
    case 32:
      if (fb->depth == 32)
        kmssrc_fb->fourcc = DRM_FORMAT_ARGB8888;
      else
        kmssrc_fb->fourcc = DRM_FORMAT_XRGB8888;
      break;
    case 16:
      kmssrc_fb->fourcc = DRM_FORMAT_RGB565;
      break;
    default:
      GST_ERROR_OBJECT (self, "FB format unsupported");
      gst_kms_src_free_fb (self, kmssrc_fb);
      drmModeFreeFB (fb);
      return FALSE;
  }

  kmssrc_fb->width = fb->width;
  kmssrc_fb->height = fb->height;
  drmModeFreeFB (fb);
  return TRUE;
}

static GstBuffer *
gst_kms_src_import_drm_fb (GstKmsSrc * self, guint fb_id)
{
  GstVideoInfo *info = &self->info;
  GstBuffer *buf = NULL;
  GstMemory *mem;
  struct kmssrc_fb fb;
  struct stat st[4];
  gint i, dmafd[4], size[4];

  if (!gst_kms_src_get_fb (self, fb_id, &fb)) {
    GST_ERROR_OBJECT (self, "could not get DRM FB %d", fb_id);
    return NULL;
  }

  if (!gst_kms_src_update_info (self, &fb))
    goto err;

  buf = gst_buffer_new ();
  if (!buf)
    goto err;

  for (i = 0; i < 4; i++) {
    if (!fb.handles[i])
      break;

    size[i] = fb.pitches[i] * fb.height;

    GST_DEBUG_OBJECT (self, "importing DRM handle %d(%d) for %dx%d",
        fb.handles[i], i, fb.pitches[i], fb.height);

    if (drmPrimeHandleToFD (self->fd, fb.handles[i], DRM_CLOEXEC | DRM_RDWR,
            &dmafd[i]) < 0) {
      GST_ERROR_OBJECT (self, "could not import DRM handle %d", fb.handles[i]);
      goto err;
    }

    fstat (dmafd[i], &st[i]);
    if (i && !memcmp (&st[i], &st[i - 1], sizeof (struct stat))) {
      /* Reuse the old fd */
      close (dmafd[i]);
      dmafd[i] = dmafd[i - 1];

      /* Check for contig planes */
      if (fb.offsets[i] == fb.offsets[i - 1] + size[i - 1]) {
        gsize mem_size, mem_offset;

        mem_size = gst_memory_get_sizes (mem, &mem_offset, NULL);
        gst_memory_resize (mem, mem_offset, mem_size + size[i]);
        continue;
      }
    }

    mem = gst_fd_allocator_alloc (self->allocator, dmafd[i], st[i].st_size,
        GST_FD_MEMORY_FLAG_NONE);
    if (!mem) {
      close (dmafd[i]);
      goto err;
    }

    gst_memory_resize (mem, fb.offsets[i], size[i]);
    gst_buffer_append_memory (buf, mem);
  }

  gst_buffer_add_video_meta_full (buf, GST_VIDEO_FRAME_FLAG_NONE,
      GST_VIDEO_INFO_FORMAT (info),
      GST_VIDEO_INFO_WIDTH (info), GST_VIDEO_INFO_HEIGHT (info),
      GST_VIDEO_INFO_N_PLANES (info), info->offset, info->stride);

  gst_kms_src_free_fb (self, &fb);
  return buf;
err:
  if (buf)
    gst_buffer_unref (buf);

  gst_kms_src_free_fb (self, &fb);
  return NULL;
}

static GstCaps *gst_kms_src_build_dma_drm_caps (GstKmsSrc * self,
    guint32 fourcc);

static GstFlowReturn
gst_kms_src_create (GstPushSrc * src, GstBuffer ** ret)
{
  GstKmsSrc *self = GST_KMS_SRC (src);
  GstBuffer *buf;
  guint fb_id;

  GST_DEBUG_OBJECT (self, "creating buffer");

  fb_id = gst_kms_src_get_next_fb_id (self);
  if (!fb_id) {
    GST_ERROR_OBJECT (self, "could not get valid FB");
    goto err;
  }

  GST_DEBUG_OBJECT (self, "importing DRM FB %d (old: %d)",
      fb_id, self->last_fb_id);

  buf = gst_kms_src_import_drm_fb (self, fb_id);
  if (!buf) {
    GST_ERROR_OBJECT (self, "could not import FB %d", fb_id);
    goto err;
  }

  /* The scanout FB can change format or geometry mid-stream, e.g. when
   * the compositor promotes a fullscreen video from an RGB composition
   * to direct NV12 scanout. Renegotiate instead of pushing buffers
   * that contradict the caps. */
  if (GST_VIDEO_INFO_FORMAT (&self->info) != self->neg_format ||
      GST_VIDEO_INFO_WIDTH (&self->info) != self->neg_width ||
      GST_VIDEO_INFO_HEIGHT (&self->info) != self->neg_height ||
      self->fb_modifier != self->neg_modifier) {
    GstCaps *new_caps;

    GST_DEBUG_OBJECT (self, "FB changed to %s %dx%d, renegotiating",
        gst_video_format_to_string (GST_VIDEO_INFO_FORMAT (&self->info)),
        GST_VIDEO_INFO_WIDTH (&self->info),
        GST_VIDEO_INFO_HEIGHT (&self->info));

    if (self->caps_fourcc)
      new_caps = gst_kms_src_build_dma_drm_caps (self, self->fb_fourcc);
    else
      new_caps = gst_video_info_to_caps (&self->info);

    if (!new_caps) {
      gst_buffer_unref (buf);
      goto err;
    }

    if (self->neg_dmabuf)
      gst_caps_set_features (new_caps, 0,
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

    /* Caps events must carry fixed caps: fixate the framerate range
     * advertised for negotiation. */
    if (self->fps_n && self->fps_d)
      gst_caps_set_simple (new_caps, "framerate", GST_TYPE_FRACTION,
          self->fps_n, self->fps_d, NULL);

    GstCaps *fixed_caps = gst_caps_fixate (new_caps);

    gst_caps_unref (new_caps);
    new_caps = fixed_caps;

    if (!gst_pad_push_event (GST_BASE_SRC_PAD (GST_BASE_SRC (src)),
            gst_event_new_caps (new_caps))) {
      GST_ERROR_OBJECT (self, "downstream refused caps %" GST_PTR_FORMAT,
          new_caps);
      gst_caps_unref (new_caps);
      gst_buffer_unref (buf);
      return GST_FLOW_NOT_NEGOTIATED;
    }
    gst_caps_unref (new_caps);

    self->neg_format = GST_VIDEO_INFO_FORMAT (&self->info);
    self->neg_width = GST_VIDEO_INFO_WIDTH (&self->info);
    self->neg_modifier = self->fb_modifier;
    self->neg_height = GST_VIDEO_INFO_HEIGHT (&self->info);
    if (self->caps_fourcc)
      self->caps_fourcc = self->fb_fourcc;
  }

  self->last_frame_time = gst_util_get_timestamp ();
  self->last_fb_id = fb_id;

  GST_BUFFER_DTS (buf) = GST_BUFFER_PTS (buf) =
      self->last_frame_time - self->start_time;

  *ret = buf;

  return GST_FLOW_OK;
err:
  if (self->last_fb_id)
    return GST_FLOW_EOS;

  return GST_FLOW_ERROR;
}

/* Based on gst-plugins-good/sys/ximage/gstximagesrc.c */
static gboolean
gst_kms_src_set_caps (GstBaseSrc * basesrc, GstCaps * caps)
{
  GstKmsSrc *self = GST_KMS_SRC (basesrc);
  GstVideoInfo *info = &self->info;
  GstStructure *structure;
  const GValue *new_fps;
  const gchar *format_str;
  gint width = 0, height = 0;
  guint32 fourcc = 0;

  structure = gst_caps_get_structure (caps, 0);

  /* The only thing that can change is the framerate downstream wants */
  new_fps = gst_structure_get_value (structure, "framerate");
  if (!new_fps)
    return FALSE;

  /* The caps must describe the FB actually being captured, otherwise
   * the pushed buffers would contradict the caps (wrong format or
   * geometry for every consumer). */
  format_str = gst_structure_get_string (structure, "format");

  if (g_strcmp0 (format_str, "DMA_DRM") == 0) {
    const gchar *drm_format = gst_structure_get_string (structure,
        "drm-format");
    guint64 modifier = DRM_FORMAT_MOD_INVALID;

    if (drm_format)
      fourcc = gst_video_dma_drm_fourcc_from_string (drm_format, &modifier);

    if (!fourcc || modifier != self->fb_modifier) {
      GST_ERROR_OBJECT (self, "caps modifier 0x%012" G_GINT64_MODIFIER
          "x doesn't match the FB modifier 0x%012" G_GINT64_MODIFIER "x",
          (guint64) modifier, self->fb_modifier);
      return FALSE;
    }

    /* set_caps is the enforcement point for what get_caps offered:
     * the scanout FB can change format between the caps query and the
     * negotiation, and buffers captured under the new fourcc would
     * contradict the negotiated caps. */
    if (fourcc != self->fb_fourcc) {
      GST_ERROR_OBJECT (self, "caps drm-format %s doesn't match the FB "
          "format %s", drm_format,
          gst_video_format_to_string (GST_VIDEO_INFO_FORMAT (info)));
      return FALSE;
    }
  } else if (format_str) {
    if (gst_video_format_from_string (format_str) !=
        GST_VIDEO_INFO_FORMAT (info)) {
      GST_ERROR_OBJECT (self, "caps format %s doesn't match the FB format "
          "%s", format_str,
          gst_video_format_to_string (GST_VIDEO_INFO_FORMAT (info)));
      return FALSE;
    }
  } else {
    GST_ERROR_OBJECT (self, "caps without format field %" GST_PTR_FORMAT, caps);
    return FALSE;
  }

  if (!gst_structure_get (structure, "width", G_TYPE_INT, &width,
          "height", G_TYPE_INT, &height, NULL) ||
      width != GST_VIDEO_INFO_WIDTH (info) ||
      height != GST_VIDEO_INFO_HEIGHT (info)) {
    GST_ERROR_OBJECT (self, "caps geometry %dx%d doesn't match the FB "
        "geometry %dx%d", width, height, GST_VIDEO_INFO_WIDTH (info),
        GST_VIDEO_INFO_HEIGHT (info));
    return FALSE;
  }

  self->caps_fourcc = fourcc;
  self->neg_format = GST_VIDEO_INFO_FORMAT (info);
  self->neg_width = GST_VIDEO_INFO_WIDTH (info);
  self->neg_height = GST_VIDEO_INFO_HEIGHT (info);
  self->neg_modifier = self->fb_modifier;
  self->neg_dmabuf =
      gst_caps_features_contains (gst_caps_get_features (caps, 0),
      GST_CAPS_FEATURE_MEMORY_DMABUF);

  /* Store this FPS for use when generating buffers */
  info->fps_n = self->fps_n = gst_value_get_fraction_numerator (new_fps);
  info->fps_d = self->fps_d = gst_value_get_fraction_denominator (new_fps);

  GST_DEBUG_OBJECT (self, "peer wants %d/%d fps", self->fps_n, self->fps_d);

  if (self->framerate_limit * self->fps_d > self->fps_n)
    GST_WARNING_OBJECT (self, "framerate-limit ignored");

  if (self->sync_fb)
    GST_WARNING_OBJECT (self, "sync-fb enabled, framerate might be inaccurate");

  return TRUE;
}

static GstCaps *
gst_kms_src_build_dma_drm_caps (GstKmsSrc * self, guint32 fourcc)
{
  GstCaps *caps;
  gchar *drm_str;

  /* GETFB2 gives the FB's real modifier (LINEAR when the driver has
   * no fb modifiers); the caps must describe what is captured. */
  drm_str = gst_video_dma_drm_fourcc_to_string (fourcc,
      self->fb_modifier == DRM_FORMAT_MOD_INVALID ?
      DRM_FORMAT_MOD_LINEAR : self->fb_modifier);
  if (!drm_str)
    return NULL;

  caps = gst_caps_new_simple ("video/x-raw",
      "format", G_TYPE_STRING, "DMA_DRM",
      "drm-format", G_TYPE_STRING, drm_str,
      "width", G_TYPE_INT, GST_VIDEO_INFO_WIDTH (&self->info),
      "height", G_TYPE_INT, GST_VIDEO_INFO_HEIGHT (&self->info),
      "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
  g_free (drm_str);

  return caps;
}

static GstCaps *
gst_kms_src_get_caps (GstBaseSrc * basesrc, GstCaps * filter)
{
  GstKmsSrc *self = GST_KMS_SRC (basesrc);
  GstCaps *caps, *raw_caps, *drm_caps, *dma_caps;
  struct kmssrc_fb fb;
  guint fb_id, i;

  fb_id = gst_kms_src_get_fb_id (self);
  if (!fb_id)
    return gst_pad_get_pad_template_caps (GST_BASE_SRC_PAD (basesrc));

  if (!gst_kms_src_get_fb (self, fb_id, &fb)) {
    GST_ERROR_OBJECT (self, "could not get DRM FB %d", fb_id);
    return NULL;
  }

  if (!gst_kms_src_update_info (self, &fb)) {
    gst_kms_src_free_fb (self, &fb);
    return NULL;
  }

  gst_kms_src_free_fb (self, &fb);

  caps = gst_caps_new_empty ();

  /* Offer the scanout FB as dma-drm caps first: DRM-aware consumers
   * (kmssink) then import it zero-copy. */
  drm_caps = gst_kms_src_build_dma_drm_caps (self, self->fb_fourcc);
  if (drm_caps) {
    /* gst_caps_set_features() consumes the features reference */
    gst_caps_set_features (drm_caps, 0,
        gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

    caps = gst_caps_merge (caps, drm_caps);
  }

  raw_caps = gst_video_info_to_caps (&self->info);

  /* The exported memories are dma-bufs either way, so also allow the
   * plain-format dmabuf caps expected by waylandsink and glupload. */
  dma_caps = gst_caps_copy (raw_caps);

  /* gst_caps_set_features() consumes the features reference */
  gst_caps_set_features (dma_caps, 0,
      gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

  caps = gst_caps_merge (caps, dma_caps);

  /* System-memory caps last: the dma-buf memories are mmap-able, so
   * CPU consumers keep working through the generic memory API. */
  caps = gst_caps_merge (caps, raw_caps);

  if (filter) {
    GstCaps *intersection;

    intersection =
        gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
    caps = intersection;
  }

  /* Advertise a variable framerate on every structure, the capture
   * pace follows the compositor flips, not a fixed clock. */
  for (i = 0; i < gst_caps_get_size (caps); i++) {
    gst_structure_set (gst_caps_get_structure (caps, i),
        "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
  }

  GST_DEBUG_OBJECT (self, "returning caps: %" GST_PTR_FORMAT, caps);
  return caps;
}

static gboolean
gst_kms_src_stop (GstBaseSrc * basesrc)
{
  GstKmsSrc *self = GST_KMS_SRC (basesrc);

  gst_poll_remove_fd (self->poll, &self->pollfd);
  gst_poll_restart (self->poll);
  gst_poll_fd_init (&self->pollfd);

  if (self->allocator)
    g_object_unref (self->allocator);

  if (self->fd >= 0)
    drmClose (self->fd);

  return TRUE;
}

/* TRUE when the plane's current FB uses a raw format this element can
 * describe in caps (compressed pseudo-fourccs like YU08 cannot be
 * consumed downstream and must not win auto-discovery). */
static gboolean
gst_kms_src_plane_capturable (GstKmsSrc * self, drmModePlane * plane)
{
#ifdef HAS_DRM_MODE_FB2
  drmModeFB2Ptr fb2 = drmModeGetFB2 (self->fd, plane->fb_id);
  gboolean ok;

  if (!fb2)
    return FALSE;

  ok = gst_kms_src_fourcc_to_format (fb2->pixel_format) !=
      GST_VIDEO_FORMAT_UNKNOWN;
  drmModeFreeFB2 (fb2);
  return ok;
#else
  /* No FB2 metadata on old libdrm: do not filter here and let
   * update_info reject what it cannot describe (legacy behaviour). */
  (void) self;
  (void) plane;
  return TRUE;
#endif
}

/* TRUE when any plane scanning out on crtc_id carries a capturable
 * raw-format FB (see gst_kms_src_plane_capturable). */
static gboolean
gst_kms_src_crtc_capturable (GstKmsSrc * self, guint32 crtc_id)
{
  drmModePlaneRes *pres = drmModeGetPlaneResources (self->fd);
  gboolean ok = FALSE;
  guint i;

  if (!pres)
    return FALSE;

  for (i = 0; i < pres->count_planes && !ok; i++) {
    drmModePlanePtr plane = drmModeGetPlane (self->fd, pres->planes[i]);

    if (!plane)
      continue;
    if (plane->crtc_id == crtc_id && plane->fb_id &&
        gst_kms_src_plane_capturable (self, plane))
      ok = TRUE;
    drmModeFreePlane (plane);
  }
  drmModeFreePlaneResources (pres);
  return ok;
}

/* Auto-discover the plane currently scanning out on crtc_id.  The
 * immutable "type" property breaks the tie when several planes hold a
 * FB on the same CRTC (video overlay, software cursor): primary beats
 * overlay beats cursor; compressed pseudo-format FBs are skipped for
 * the pick but listed in the warning so the user can pin plane-id. */
static guint32
gst_kms_src_find_scanout_plane (GstKmsSrc * self, guint32 crtc_id)
{
  drmModePlaneRes *pres = drmModeGetPlaneResources (self->fd);
  guint32 primary = 0, overlay = 0, cursor = 0;
  GString *candidates = NULL;
  guint i, j, hits = 0;

  if (!pres)
    return 0;

  for (i = 0; i < pres->count_planes; i++) {
    drmModePlanePtr plane = drmModeGetPlane (self->fd, pres->planes[i]);
    drmModeObjectPropertiesPtr props;
    guint64 type = DRM_PLANE_TYPE_OVERLAY;

    if (!plane)
      continue;

    if (plane->crtc_id != crtc_id || !plane->fb_id) {
      drmModeFreePlane (plane);
      continue;
    }
    hits++;

    if (!gst_kms_src_plane_capturable (self, plane)) {
      if (!candidates)
        candidates = g_string_new ("");
      else
        g_string_append (candidates, ", ");
      g_string_append_printf (candidates, "%u(skipped-compressed)",
          plane->plane_id);
      drmModeFreePlane (plane);
      continue;
    }

    props = drmModeObjectGetProperties (self->fd, plane->plane_id,
        DRM_MODE_OBJECT_PLANE);
    if (props) {
      for (j = 0; j < props->count_props; j++) {
        drmModePropertyPtr prop =
            drmModeGetProperty (self->fd, props->props[j]);

        if (prop) {
          if (!g_strcmp0 (prop->name, "type"))
            type = props->prop_values[j];
          drmModeFreeProperty (prop);
        }
      }
      drmModeFreeObjectProperties (props);
    }

    if (type == DRM_PLANE_TYPE_PRIMARY) {
      if (!primary)
        primary = plane->plane_id;
    } else if (type == DRM_PLANE_TYPE_CURSOR) {
      if (!cursor)
        cursor = plane->plane_id;
    } else if (!overlay) {
      overlay = plane->plane_id;
    }

    if (!candidates)
      candidates = g_string_new ("");
    else
      g_string_append (candidates, ", ");
    g_string_append_printf (candidates, "%u(%s)", plane->plane_id,
        type == DRM_PLANE_TYPE_PRIMARY ? "primary" :
        type == DRM_PLANE_TYPE_CURSOR ? "cursor" : "overlay");

    drmModeFreePlane (plane);
  }
  drmModeFreePlaneResources (pres);

  if (candidates && hits > 1) {
    guint32 pick = primary ? primary : (overlay ? overlay : cursor);

    GST_WARNING_OBJECT (self, "%u planes hold a FB on crtc %u [%s]; "
        "capturing plane %u (set plane-id to override)", hits, crtc_id,
        candidates->str, pick);
    g_string_free (candidates, TRUE);
  } else if (candidates) {
    g_string_free (candidates, TRUE);
  }

  return primary ? primary : (overlay ? overlay : cursor);
}

static guint
gst_kms_src_find_best_crtc (GstKmsSrc * self)
{
  drmModeCrtcPtr crtc;
  drmModeResPtr res;
  guint crtc_id;
  gint i;

  res = drmModeGetResources (self->fd);
  if (!res)
    return 0;

  for (i = 0, crtc_id = 0; i < res->count_crtcs; i++) {
    crtc = drmModeGetCrtc (self->fd, res->crtcs[i]);
    if (crtc && crtc->mode_valid) {
      drmModeFreeCrtc (crtc);
      /* Prefer a CRTC whose scanout plane carries a capturable raw
       * FB: with several outputs the first-valid pick would
       * otherwise land on a compressed video plane this element
       * cannot describe. */
      gboolean capturable = gst_kms_src_crtc_capturable (self,
          res->crtcs[i]);
      if (!crtc_id || capturable)
        crtc_id = res->crtcs[i];
      if (capturable && crtc_id == res->crtcs[i])
        break;
    } else {
      drmModeFreeCrtc (crtc);
    }
  }
  drmModeFreeResources (res);

  if (crtc_id)
    GST_DEBUG_OBJECT (self, "using best CRTC %d", crtc_id);

  return crtc_id;
}

static gboolean
gst_kms_src_start (GstBaseSrc * basesrc)
{
  GstKmsSrc *self = GST_KMS_SRC (basesrc);

  self->allocator = gst_dmabuf_allocator_new ();
  if (!self->allocator)
    return FALSE;

  if (self->devname || self->bus_id)
    self->fd = drmOpen (self->devname, self->bus_id);

  if (self->fd < 0)
    self->fd = open ("/dev/dri/card0", O_RDWR | O_CLOEXEC);

  if (self->fd < 0) {
    GST_ELEMENT_ERROR (self, RESOURCE, OPEN_READ_WRITE,
        ("Could not open DRM module %s", GST_STR_NULL (self->devname)),
        ("reason: %s (%d)", g_strerror (errno), errno));
    gst_kms_src_stop (basesrc);
    return FALSE;
  }

  /* Without universal planes the kernel hides the primary planes
   * (Esmart on VOP2) and the scanout discovery below only ever sees
   * overlay-class planes with crtc_id 0, so it could never match.
   * Ancient kernels reject the cap; keep the legacy CRTC anchor
   * and skip plane discovery in that case. */
  gboolean has_universal_planes =
      (drmSetClientCap (self->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) == 0);

  if (!self->fb_id && !self->plane_id && !self->connector_id &&
      !self->encoder_id && !self->crtc_id) {
    self->crtc_id = gst_kms_src_find_best_crtc (self);
    if (!self->crtc_id) {
      GST_ERROR_OBJECT (self, "could not find a valid CRTC");
      gst_kms_src_stop (basesrc);
      return FALSE;
    }
  }

  /* Atomic compositors keep the CRTC's legacy fb_id at 0, so the
   * plane is the only reliable capture anchor there: discover the
   * plane currently scanning out on the CRTC (primary beats overlay
   * beats cursor) unless the user pinned one. */
  if (has_universal_planes && self->crtc_id && !self->plane_id &&
      !self->fb_id) {
    guint32 plane_id = gst_kms_src_find_scanout_plane (self,
        self->crtc_id);

    if (plane_id) {
      self->plane_id = plane_id;
    } else {
      /* Every plane on this CRTC carries a compressed or unusable
       * FB.  The legacy CRTC anchor would fetch the very same
       * compressed FB and die in update_info with an indirect
       * "format not supported" - fail here with the real cause. */
      GST_ELEMENT_ERROR (self, RESOURCE, NOT_FOUND,
          ("no capturable scanout plane on crtc %u "
              "(compressed or unsupported formats only)", self->crtc_id),
          ("set plane-id to an uncompressed plane, or point crtc-id "
              "at another output"));
      gst_kms_src_stop (basesrc);
      return FALSE;
    }
  }

  self->last_fb_id = 0;
  self->last_frame_time = gst_util_get_timestamp ();
  self->static_since = 0;
  self->fb_fourcc = 0;
  self->fb_modifier = DRM_FORMAT_MOD_LINEAR;
  self->caps_fourcc = 0;
  self->neg_format = GST_VIDEO_FORMAT_UNKNOWN;
  self->neg_width = 0;
  self->neg_modifier = DRM_FORMAT_MOD_LINEAR;
  self->neg_height = 0;
  self->neg_dmabuf = FALSE;
  self->start_time = gst_util_get_timestamp ();

  self->pollfd.fd = self->fd;
  gst_poll_add_fd (self->poll, &self->pollfd);
  gst_poll_fd_ctl_read (self->poll, &self->pollfd, TRUE);

  return TRUE;
}

static void
gst_kms_src_finalize (GObject * object)
{
  GstKmsSrc *self;

  self = GST_KMS_SRC (object);
  g_clear_pointer (&self->devname, g_free);
  g_clear_pointer (&self->bus_id, g_free);

  gst_poll_free (self->poll);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_kms_src_init (GstKmsSrc * self)
{
  self->devname = g_strdup ("rockchip");

  self->fd = -1;

  self->crtc_id = 0;
  self->encoder_id = 0;
  self->connector_id = 0;
  self->plane_id = 0;
  self->fb_id = 0;

  self->framerate_limit = DEFAULT_PROP_FRAMERATE_LIMIT;
  self->sync_fb = TRUE;
  self->sync_vblank = TRUE;
  self->fps_n = 0;
  self->fps_d = 1;

  gst_video_info_init (&self->info);

  gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);
  gst_base_src_set_live (GST_BASE_SRC (self), TRUE);

  gst_poll_fd_init (&self->pollfd);
  self->poll = gst_poll_new (TRUE);
}

static void
gst_kms_src_class_init (GstKmsSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbase_src_class;
  GstPushSrcClass *gstpush_src_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gstelement_class = GST_ELEMENT_CLASS (klass);
  gstbase_src_class = GST_BASE_SRC_CLASS (klass);
  gstpush_src_class = GST_PUSH_SRC_CLASS (klass);

  gobject_class->finalize = gst_kms_src_finalize;
  gobject_class->set_property = gst_kms_src_set_property;
  gobject_class->get_property = gst_kms_src_get_property;

  g_object_class_install_property (gobject_class, PROP_DRIVER_NAME,
      g_param_spec_string ("driver-name",
          "device name", "DRM device driver name", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_BUS_ID,
      g_param_spec_string ("bus-id", "Bus ID", "DRM bus ID", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_CRTC_ID,
      g_param_spec_uint ("crtc-id", "DRM crtc ID",
          "DRM crtc ID (0 = unspecified)",
          0, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ENCODER_ID,
      g_param_spec_uint ("encoder-id", "DRM plane ID",
          "DRM encoder ID (0 = unspecified)",
          0, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CONNECTOR_ID,
      g_param_spec_uint ("connector-id", "DRM connector ID",
          "DRM connector ID (0 = unspecified)",
          0, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_ID,
      g_param_spec_uint ("plane-id", "DRM plane ID",
          "DRM plane ID (0 = unspecified)",
          0, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FB_ID,
      g_param_spec_uint ("fb-id", "DRM FB ID",
          "DRM FB ID (0 = unspecified)",
          0, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


  g_object_class_install_property (gobject_class, PROP_FRAMERATE_LIMIT,
      g_param_spec_uint ("framerate-limit", "Limited framerate",
          "Limited framerate",
          0, G_MAXINT, DEFAULT_PROP_FRAMERATE_LIMIT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SYNC_FB,
      g_param_spec_boolean ("sync-fb", "Sync with FB flip",
          "Sync with FB flip", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SYNC_VBLANK,
      g_param_spec_boolean ("sync-vblank", "Sync with vblank",
          "Sync with vblank", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (gstelement_class,
      "KMS Video Source",
      "Source/Video",
      "KMS Video Source", "Jeffy Chen <jeffy.chen@rock-chips.com>");

  gst_element_class_add_static_pad_template (gstelement_class, &srctemplate);

  gstbase_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_kms_src_set_caps);
  gstbase_src_class->get_caps = GST_DEBUG_FUNCPTR (gst_kms_src_get_caps);
  gstbase_src_class->start = GST_DEBUG_FUNCPTR (gst_kms_src_start);
  gstbase_src_class->stop = GST_DEBUG_FUNCPTR (gst_kms_src_stop);
  gstpush_src_class->create = GST_DEBUG_FUNCPTR (gst_kms_src_create);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "kmssrc", GST_RANK_NONE,
          gst_kms_src_get_type ()))
    return FALSE;

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "kmssrc", 0, "KmsSrc");
  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    kmssrc,
    "KMS Src Plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
