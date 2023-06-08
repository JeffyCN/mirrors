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

#ifndef DRM_FORMAT_NV12_10
#define DRM_FORMAT_NV12_10 fourcc_code('N', 'A', '1', '2')
#endif

#ifndef DRM_FORMAT_NV15
#define DRM_FORMAT_NV15 fourcc_code('N', 'V', '1', '5')
#endif

#define DEFAULT_PROP_FRAMERATE_LIMIT 120

static gboolean DEFAULT_PROP_DMA_FEATURE = FALSE;

#define GST_TYPE_KMS_SRC (gst_kms_src_get_type())
G_DECLARE_FINAL_TYPE (GstKmsSrc, gst_kms_src, GST, KMS_SRC, GstPushSrc);

struct _GstKmsSrc {
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

  gboolean dma_feature;

  guint framerate_limit;
  guint fps_n;
  guint fps_d;
  gboolean sync_fb;
  gboolean sync_vblank;

  GstPoll *poll;
  GstPollFD pollfd;

  guint last_fb_id;
  GstClockTime last_frame_time;

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
  PROP_DMA_FEATURE,
  PROP_FRAMERATE_LIMIT,
  PROP_SYNC_FB,
  PROP_SYNC_VBLANK,
  PROP_LAST,
};

struct kmssrc_fb {
  guint handles[4];
  guint pitches[4];
  guint offsets[4];
  guint width;
  guint height;
  guint fourcc;
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
    case PROP_DMA_FEATURE:
      self->dma_feature = g_value_get_boolean (value);
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
    case PROP_DMA_FEATURE:
      g_value_set_boolean (value, self->dma_feature);
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
  gboolean sync_fb = self->sync_fb && !self->fb_id;
  guint duration = 0;
  guint fb_id;

  if (self->sync_vblank)
    gst_kms_src_sync_vblank (self);

  if (self->fps_d && self->fps_n)
    duration =
        gst_util_uint64_scale (GST_SECOND, self->fps_d, self->fps_n);
  else if (self->framerate_limit)
    duration = GST_SECOND / self->framerate_limit;

  while (1) {
    diff = GST_CLOCK_DIFF (self->last_frame_time, gst_util_get_timestamp ());

    fb_id = gst_kms_src_get_fb_id (self);
    if (!fb_id)
      return 0;

    if (!sync_fb || fb_id != self->last_fb_id) {
      sync_fb = FALSE;

      if (diff >= duration)
        return fb_id;
    }

    g_usleep (1000);
  }

  return 0;
}

#ifndef HAS_DRM_CLOSE_HANDLE
/* From libdrm 2.4.109 : xf86drm.c */
static int
drmCloseBufferHandle(int fd, uint32_t handle)
{
  struct drm_gem_close args;

  memset(&args, 0, sizeof(args));
  args.handle = handle;
  return drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &args);
}
#endif

static void
gst_kms_src_free_fb (GstKmsSrc * self, struct kmssrc_fb * fb)
{
  guint i;

  for (i = 0; i < 4; i++) {
    if (fb->handles[i])
      drmCloseBufferHandle (self->fd, fb->handles[i]);

    fb->handles[i] = 0;
  }
}

static gboolean
gst_kms_src_update_info (GstKmsSrc * self, struct kmssrc_fb * fb)
{
  GstVideoInfo *info = &self->info;
  GstVideoFormat format;
  guint i;

#define KMSSRC_CASE_FOURCC(fourcc, gst) \
  case DRM_FORMAT_ ## fourcc: format = GST_VIDEO_FORMAT_ ## gst; break;

  switch (fb->fourcc) {
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
gst_kms_src_get_fb (GstKmsSrc * self, guint fb_id, struct kmssrc_fb * kmssrc_fb)
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

  /* The only thing that can change is the framerate downstream wants */
  structure = gst_caps_get_structure (caps, 0);
  new_fps = gst_structure_get_value (structure, "framerate");
  if (!new_fps)
    return FALSE;

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
gst_kms_src_get_caps (GstBaseSrc * basesrc, GstCaps * filter)
{
  GstKmsSrc *self = GST_KMS_SRC (basesrc);
  GstStructure *s;
  GstCaps *caps;
  struct kmssrc_fb fb;
  guint fb_id;

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

  caps = gst_video_info_to_caps (&self->info);

  if (self->dma_feature)
    gst_caps_set_features (caps, 0,
        gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

  if (filter) {
    GstCaps *intersection;

    intersection =
      gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
    caps = intersection;
  }

  s = gst_caps_get_structure (caps, 0);
  gst_structure_set (s, "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT,
      1, NULL);

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
      crtc_id = res->crtcs[i];
      break;
    }
    drmModeFreeCrtc (crtc);
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

  if (!self->fb_id && !self->plane_id && !self->connector_id &&
      !self->encoder_id && !self->crtc_id) {
    self->crtc_id = gst_kms_src_find_best_crtc (self);
    if (!self->crtc_id) {
      GST_ERROR_OBJECT (self, "could not find a valid CRTC");
      gst_kms_src_stop (basesrc);
      return FALSE;
    }
  }

  self->last_fb_id = 0;
  self->last_frame_time = gst_util_get_timestamp ();
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
  const gchar *env;

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
          0, G_MAXINT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ENCODER_ID,
      g_param_spec_uint ("encoder-id", "DRM plane ID",
          "DRM encoder ID (0 = unspecified)",
          0, G_MAXINT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CONNECTOR_ID,
      g_param_spec_uint ("connector-id", "DRM connector ID",
          "DRM connector ID (0 = unspecified)",
          0, G_MAXINT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_ID,
      g_param_spec_uint ("plane-id", "DRM plane ID",
          "DRM plane ID (0 = unspecified)",
          0, G_MAXINT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_FB_ID,
      g_param_spec_uint ("fb-id", "DRM FB ID",
          "DRM FB ID (0 = unspecified)",
          0, G_MAXINT, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  env = g_getenv ("GST_KMSSRC_DMA_FEATURE");
  if (env && !strcmp (env, "1"))
    DEFAULT_PROP_DMA_FEATURE = TRUE;

  g_object_class_install_property (gobject_class, PROP_DMA_FEATURE,
      g_param_spec_boolean ("dma-feature", "DMA feature",
          "Enable GST DMA feature", DEFAULT_PROP_DMA_FEATURE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

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
      "KMS Video Source",
      "Jeffy Chen <jeffy.chen@rock-chips.com>");

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
