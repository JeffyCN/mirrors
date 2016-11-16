/* GStreamer
 *
 * Copyright (C) 2016 Igalia
 *
 * Authors:
 *  Víctor Manuel Jáquez Leal <vjaquez@igalia.com>
 *  Javier Martin <javiermartin@by.com.es>
 *  Randy Li      <randy.li@rock-chips.com>
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

/**
 * SECTION:element-kmssink
 * @short_description: A KMS/DRM based video sink
 *
 * kmssink is a simple video sink that renders video frames directly
 * in a plane of a DRM device.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 videotestsrc ! kmssink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/video/video.h>
#include <gst/allocators/gstdmabuf.h>

#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include <string.h>

#include <gst/vpudec/gstvpumeta.h>
#include "gstkmssink.h"
#include "gstkmsutils.h"

#define GST_PLUGIN_NAME "kmssink"
#define GST_PLUGIN_DESC "Video sink using the Linux kernel mode setting API"

GST_DEBUG_CATEGORY_STATIC (gst_kms_sink_debug);
GST_DEBUG_CATEGORY_STATIC (CAT_PERFORMANCE);
#define GST_CAT_DEFAULT gst_kms_sink_debug

#define parent_class gst_kms_sink_parent_class
G_DEFINE_TYPE_WITH_CODE (GstKMSSink, gst_kms_sink, GST_TYPE_VIDEO_SINK,
    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_PLUGIN_NAME, 0,
        GST_PLUGIN_DESC);
    GST_DEBUG_CATEGORY_GET (CAT_PERFORMANCE, "GST_PERFORMANCE"));

enum
{
  PROP_DRIVER_NAME = 1,
  PROP_CONNECTOR_ID,
  PROP_PLANE_ID,
  PROP_N
};

static GParamSpec *g_properties[PROP_N] = { NULL, };

static int
kms_open (gchar ** driver)
{
  static const char *drivers[] = { "i915", "radeon", "nouveau", "vmwgfx",
    "exynos", "amdgpu", "imx-drm", "rockchip", "atmel-hlcdc"
  };
  int i, fd = -1;

  for (i = 0; i < G_N_ELEMENTS (drivers); i++) {
    fd = drmOpen (drivers[i], NULL);
    if (fd >= 0) {
      if (driver)
        *driver = g_strdup (drivers[i]);
      break;
    }
  }

  return fd;
}

static drmModePlane *
find_plane_for_crtc (int fd, drmModeRes * res, drmModePlaneRes * pres,
    int crtc_id)
{
  drmModePlane *plane;
  int i, pipe;

  plane = NULL;
  pipe = -1;
  for (i = 0; i < res->count_crtcs; i++) {
    if (crtc_id == res->crtcs[i]) {
      pipe = i;
      break;
    }
  }

  if (pipe == -1)
    return NULL;

  for (i = 0; i < pres->count_planes; i++) {
    plane = drmModeGetPlane (fd, pres->planes[i]);
    if (plane->possible_crtcs & (1 << pipe))
      return plane;
    drmModeFreePlane (plane);
  }

  return NULL;
}

static drmModeCrtc *
find_crtc_for_connector (int fd, drmModeRes * res, drmModeConnector * conn,
    guint * pipe)
{
  int i;
  int crtc_id;
  drmModeEncoder *enc;
  drmModeCrtc *crtc;

  crtc_id = -1;
  for (i = 0; i < res->count_encoders; i++) {
    enc = drmModeGetEncoder (fd, res->encoders[i]);
    if (enc) {
      if (enc->encoder_id == conn->encoder_id) {
        crtc_id = enc->crtc_id;
        drmModeFreeEncoder (enc);
        break;
      }
      drmModeFreeEncoder (enc);
    }
  }

  if (crtc_id == -1)
    return NULL;

  for (i = 0; i < res->count_crtcs; i++) {
    crtc = drmModeGetCrtc (fd, res->crtcs[i]);
    if (crtc) {
      if (crtc_id == crtc->crtc_id) {
        if (pipe)
          *pipe = i;
        return crtc;
      }
      drmModeFreeCrtc (crtc);
    }
  }

  return NULL;
}

static gboolean
connector_is_used (int fd, drmModeRes * res, drmModeConnector * conn)
{
  gboolean result;
  drmModeCrtc *crtc;

  result = FALSE;
  crtc = find_crtc_for_connector (fd, res, conn, NULL);
  if (crtc) {
    result = crtc->buffer_id != 0;
    drmModeFreeCrtc (crtc);
  }

  return result;
}

static drmModeConnector *
find_used_connector_by_type (int fd, drmModeRes * res, int type)
{
  int i;
  drmModeConnector *conn;

  conn = NULL;
  for (i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector (fd, res->connectors[i]);
    if (conn) {
      if ((conn->connector_type == type) && connector_is_used (fd, res, conn))
        return conn;
      drmModeFreeConnector (conn);
    }
  }

  return NULL;
}

static drmModeConnector *
find_first_used_connector (int fd, drmModeRes * res)
{
  int i;
  drmModeConnector *conn;

  conn = NULL;
  for (i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector (fd, res->connectors[i]);
    if (conn) {
      if (connector_is_used (fd, res, conn))
        return conn;
      drmModeFreeConnector (conn);
    }
  }

  return NULL;
}

static drmModeConnector *
find_main_monitor (int fd, drmModeRes * res)
{
  /* Find the LVDS and eDP connectors: those are the main screens. */
  static const int priority[] = { DRM_MODE_CONNECTOR_LVDS,
    DRM_MODE_CONNECTOR_eDP
  };
  int i;
  drmModeConnector *conn;

  conn = NULL;
  for (i = 0; !conn && i < G_N_ELEMENTS (priority); i++)
    conn = find_used_connector_by_type (fd, res, priority[i]);

  /* if we didn't find a connector, grab the first one in use */
  if (!conn)
    conn = find_first_used_connector (fd, res);

  return conn;
}

static void
log_drm_version (GstKMSSink * self)
{
#ifndef GST_DISABLE_GST_DEBUG
  drmVersion *v;

  v = drmGetVersion (self->fd);
  if (v) {
    GST_INFO_OBJECT (self, "DRM v%d.%d.%d [%s — %s — %s]", v->version_major,
        v->version_minor, v->version_patchlevel, GST_STR_NULL (v->name),
        GST_STR_NULL (v->desc), GST_STR_NULL (v->date));
    drmFreeVersion (v);
  } else {
    GST_WARNING_OBJECT (self, "could not get driver information: %s",
        GST_STR_NULL (self->devname));
  }
#endif
  return;
}

static gboolean
get_drm_caps (GstKMSSink * self)
{
  gint ret;
  guint64 has_dumb_buffer;
  guint64 has_prime;
  guint64 has_async_page_flip;

  has_dumb_buffer = 0;
  ret = drmGetCap (self->fd, DRM_CAP_DUMB_BUFFER, &has_dumb_buffer);
  if (ret)
    GST_WARNING_OBJECT (self, "could not get dumb buffer capability");
  if (has_dumb_buffer == 0) {
    GST_ERROR_OBJECT (self, "driver cannot handle dumb buffers");
    return FALSE;
  }

  has_prime = 0;
  ret = drmGetCap (self->fd, DRM_CAP_PRIME, &has_prime);
  if (ret)
    GST_WARNING_OBJECT (self, "could not get prime capability");
  else
    self->has_prime_import = (gboolean) (has_prime & DRM_PRIME_CAP_IMPORT);

  has_async_page_flip = 0;
  ret = drmGetCap (self->fd, DRM_CAP_ASYNC_PAGE_FLIP, &has_async_page_flip);
  if (ret)
    GST_WARNING_OBJECT (self, "could not get async page flip capability");
  else
    self->has_async_page_flip = (gboolean) has_async_page_flip;

  GST_INFO_OBJECT (self, "prime import (%s) / async page flip (%s)",
      self->has_prime_import ? "✓" : "✗",
      self->has_async_page_flip ? "✓" : "✗");

  return TRUE;
}

static gboolean
ensure_allowed_caps (GstKMSSink * self, drmModePlane * plane, drmModeRes * res)
{
  GstCaps *out_caps, *caps;
  int i;
  GstVideoFormat fmt;
  const gchar *format;

  if (self->allowed_caps)
    return TRUE;

  out_caps = gst_caps_new_empty ();
  if (!out_caps)
    return FALSE;

  for (i = 0; i < plane->count_formats; i++) {
    fmt = gst_video_format_from_drm (plane->formats[i]);
    if (fmt == GST_VIDEO_FORMAT_UNKNOWN) {
      GST_INFO_OBJECT (self, "ignoring format %" GST_FOURCC_FORMAT,
          GST_FOURCC_ARGS (plane->formats[i]));
      continue;
    }

    format = gst_video_format_to_string (fmt);
    caps = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, format,
        "width", GST_TYPE_INT_RANGE, res->min_width, res->max_width,
        "height", GST_TYPE_INT_RANGE, res->min_height, res->max_height,
        "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
    if (!caps)
      continue;

    out_caps = gst_caps_merge (out_caps, caps);
  }

  self->allowed_caps = gst_caps_simplify (out_caps);

  GST_DEBUG_OBJECT (self, "allowed caps = %" GST_PTR_FORMAT,
      self->allowed_caps);

  return TRUE;
}

static gboolean
gst_kms_sink_start (GstBaseSink * bsink)
{
  GstKMSSink *self;
  drmModeRes *res;
  drmModeConnector *conn;
  drmModeCrtc *crtc;
  drmModePlaneRes *pres;
  drmModePlane *plane;
  gboolean universal_planes;
  gboolean ret;

  self = GST_KMS_SINK (bsink);
  universal_planes = FALSE;
  ret = FALSE;
  res = NULL;
  conn = NULL;
  crtc = NULL;
  pres = NULL;
  plane = NULL;

  if (self->devname)
    self->fd = drmOpen (self->devname, NULL);
  else
    self->fd = kms_open (&self->devname);
  if (self->fd < 0)
    goto open_failed;

  log_drm_version (self);
  if (!get_drm_caps (self))
    goto bail;

  res = drmModeGetResources (self->fd);
  if (!res)
    goto resources_failed;

  if (self->conn_id == -1)
    conn = find_main_monitor (self->fd, res);
  else
    conn = drmModeGetConnector (self->fd, self->conn_id);
  if (!conn)
    goto connector_failed;

  crtc = find_crtc_for_connector (self->fd, res, conn, &self->pipe);
  if (!crtc)
    goto crtc_failed;

retry_find_plane:
  if (universal_planes &&
      drmSetClientCap (self->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1))
    goto set_cap_failed;

  pres = drmModeGetPlaneResources (self->fd);
  if (!pres)
    goto plane_resources_failed;

  if (self->plane_id == -1)
    plane = find_plane_for_crtc (self->fd, res, pres, crtc->crtc_id);
  else
    plane = drmModeGetPlane (self->fd, self->plane_id);
  if (!plane)
    goto plane_failed;

  /* let's get the available color formats in plane */
  if (!ensure_allowed_caps (self, plane, res))
    goto bail;

  self->conn_id = conn->connector_id;
  self->crtc_id = crtc->crtc_id;
  self->plane_id = plane->plane_id;

  GST_INFO_OBJECT (self, "connector id = %d / crtc id = %d / plane id = %d",
      self->conn_id, self->crtc_id, self->plane_id);

  self->hdisplay = crtc->mode.hdisplay;
  self->vdisplay = crtc->mode.vdisplay;
  self->buffer_id = crtc->buffer_id;

  self->mm_width = conn->mmWidth;
  self->mm_height = conn->mmHeight;

  GST_INFO_OBJECT (self, "display size: pixels = %dx%d / millimeters = %dx%d",
      self->hdisplay, self->vdisplay, self->mm_width, self->mm_height);

  self->pollfd.fd = self->fd;
  gst_poll_add_fd (self->poll, &self->pollfd);
  gst_poll_fd_ctl_read (self->poll, &self->pollfd, TRUE);

  ret = TRUE;

bail:
  if (plane)
    drmModeFreePlane (plane);
  if (pres)
    drmModeFreePlaneResources (pres);
  if (crtc)
    drmModeFreeCrtc (crtc);
  if (conn)
    drmModeFreeConnector (conn);
  if (res)
    drmModeFreeResources (res);

  if (!ret && self->fd >= 0) {
    drmClose (self->fd);
    self->fd = -1;
  }

  return ret;

  /* ERRORS */
open_failed:
  {
    GST_ERROR_OBJECT (self, "Could not open DRM module %s: %s",
        GST_STR_NULL (self->devname), strerror (errno));
    return FALSE;
  }

resources_failed:
  {
    GST_ERROR_OBJECT (self, "drmModeGetResources failed: %s (%d)",
        strerror (errno), errno);
    goto bail;
  }

connector_failed:
  {
    GST_ERROR_OBJECT (self, "Could not find a valid monitor connector");
    goto bail;
  }

crtc_failed:
  {
    GST_ERROR_OBJECT (self, "Could not find a crtc for connector");
    goto bail;
  }

set_cap_failed:
  {
    GST_ERROR_OBJECT (self, "Could not set universal planes capability bit");
    goto bail;
  }

plane_resources_failed:
  {
    GST_ERROR_OBJECT (self, "drmModeGetPlaneResources failed: %s (%d)",
        strerror (errno), errno);
    goto bail;
  }

plane_failed:
  {
    if (universal_planes) {
      GST_ERROR_OBJECT (self, "Could not find a plane for crtc");
      goto bail;
    } else {
      universal_planes = TRUE;
      goto retry_find_plane;
    }
  }
}

static gboolean
gst_kms_sink_stop (GstBaseSink * bsink)
{
  GstKMSSink *self;

  self = GST_KMS_SINK (bsink);

  gst_buffer_replace (&self->last_buffer, NULL);
  gst_caps_replace (&self->allowed_caps, NULL);
  gst_object_replace ((GstObject **) & self->pool, NULL);
  gst_object_replace ((GstObject **) & self->allocator, NULL);

  gst_poll_remove_fd (self->poll, &self->pollfd);
  gst_poll_restart (self->poll);
  gst_poll_fd_init (&self->pollfd);

  if (self->fd >= 0) {
    drmClose (self->fd);
    self->fd = -1;
  }

  return TRUE;
}

static GstCaps *
gst_kms_sink_get_allowed_caps (GstKMSSink * self)
{
  if (!self->allowed_caps)
    return NULL;                /* base class will return the template caps */
  return gst_caps_ref (self->allowed_caps);
}

static GstCaps *
gst_kms_sink_get_caps (GstBaseSink * bsink, GstCaps * filter)
{
  GstKMSSink *self;
  GstCaps *caps, *out_caps;

  self = GST_KMS_SINK (bsink);

  caps = gst_kms_sink_get_allowed_caps (self);
  if (caps && filter) {
    out_caps = gst_caps_intersect_full (caps, filter, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
  } else {
    out_caps = caps;
  }

  return out_caps;
}

static gboolean
gst_kms_sink_calculate_display_ratio (GstKMSSink * self, GstVideoInfo * vinfo)
{
  guint dar_n, dar_d;
  guint video_width, video_height;
  guint video_par_n, video_par_d;
  guint dpy_par_n, dpy_par_d;

  video_width = GST_VIDEO_INFO_WIDTH (vinfo);
  video_height = GST_VIDEO_INFO_HEIGHT (vinfo);
  video_par_n = GST_VIDEO_INFO_PAR_N (vinfo);
  video_par_d = GST_VIDEO_INFO_PAR_D (vinfo);

  gst_video_calculate_device_ratio (self->hdisplay, self->vdisplay,
      self->mm_width, self->mm_height, &dpy_par_n, &dpy_par_d);

  if (!gst_video_calculate_display_ratio (&dar_n, &dar_d, video_width,
          video_height, video_par_n, video_par_d, dpy_par_n, dpy_par_d))
    return FALSE;

  GST_DEBUG_OBJECT (self, "video calculated display ratio: %d/%d", dar_n,
      dar_d);

  /* now find a width x height that respects this display ratio.
   * prefer those that have one of w/h the same as the incoming video
   * using wd / hd = dar_n / dar_d */

  /* start with same height, because of interlaced video */
  /* check hd / dar_d is an integer scale factor, and scale wd with the PAR */
  if (video_height % dar_d == 0) {
    GST_DEBUG_OBJECT (self, "keeping video height");
    GST_VIDEO_SINK_WIDTH (self) = (guint)
        gst_util_uint64_scale_int (video_height, dar_n, dar_d);
    GST_VIDEO_SINK_HEIGHT (self) = video_height;
  } else if (video_width % dar_n == 0) {
    GST_DEBUG_OBJECT (self, "keeping video width");
    GST_VIDEO_SINK_WIDTH (self) = video_width;
    GST_VIDEO_SINK_HEIGHT (self) = (guint)
        gst_util_uint64_scale_int (video_width, dar_d, dar_n);
  } else {
    GST_DEBUG_OBJECT (self, "approximating while keeping video height");
    GST_VIDEO_SINK_WIDTH (self) = (guint)
        gst_util_uint64_scale_int (video_height, dar_n, dar_d);
    GST_VIDEO_SINK_HEIGHT (self) = video_height;
  }
  GST_DEBUG_OBJECT (self, "scaling to %dx%d", GST_VIDEO_SINK_WIDTH (self),
      GST_VIDEO_SINK_HEIGHT (self));

  return TRUE;
}

static gboolean
gst_kms_sink_set_caps (GstBaseSink * bsink, GstCaps * caps)
{
  GstKMSSink *self;
  GstVideoInfo vinfo;

  self = GST_KMS_SINK (bsink);

  if (!gst_video_info_from_caps (&vinfo, caps))
    goto invalid_format;

  if (!gst_kms_sink_calculate_display_ratio (self, &vinfo))
    goto no_disp_ratio;

  if (GST_VIDEO_SINK_WIDTH (self) <= 0 || GST_VIDEO_SINK_HEIGHT (self) <= 0)
    goto invalid_size;

  self->vinfo = vinfo;

  GST_DEBUG_OBJECT (self, "negotiated caps = %" GST_PTR_FORMAT, caps);

  return TRUE;

  /* ERRORS */
invalid_format:
  {
    GST_ERROR_OBJECT (self, "caps invalid");
    return FALSE;
  }

invalid_size:
  {
    GST_ELEMENT_ERROR (self, CORE, NEGOTIATION, (NULL),
        ("Invalid image size."));
    return FALSE;
  }

no_disp_ratio:
  {
    GST_ELEMENT_ERROR (self, CORE, NEGOTIATION, (NULL),
        ("Error calculating the output display ratio of the video."));
    return FALSE;
  }
}

static gboolean
gst_kms_sink_propose_allocation (GstBaseSink * bsink, GstQuery * query)
{
  GstCaps *caps;
  gboolean need_pool;
  GstVideoInfo vinfo;

  gst_query_parse_allocation (query, &caps, &need_pool);
  if (!caps)
    goto no_caps;
  if (!gst_video_info_from_caps (&vinfo, caps))
    goto invalid_caps;

  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);
  gst_query_add_allocation_meta (query, GST_VIDEO_CROP_META_API_TYPE, NULL);

  return TRUE;

  /* ERRORS */
no_caps:
  {
    GST_DEBUG_OBJECT (bsink, "no caps specified");
    return FALSE;
  }
invalid_caps:
  {
    GST_DEBUG_OBJECT (bsink, "invalid caps specified");
    return FALSE;
  }
}

static void
sync_handler (gint fd, guint frame, guint sec, guint usec, gpointer data)
{
  gboolean *waiting;

  waiting = data;
  *waiting = FALSE;
}

static gboolean
gst_kms_sink_sync (GstKMSSink * self)
{
  gint ret;
  gboolean waiting;
  drmEventContext evctxt = {
    .version = DRM_EVENT_CONTEXT_VERSION,
    .page_flip_handler = sync_handler,
    .vblank_handler = sync_handler,
  };
  drmVBlank vbl = {
    .request = {
          .type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT,
          .sequence = 1,
          .signal = (gulong) & waiting,
        },
  };

  if (self->pipe == 1)
    vbl.request.type |= DRM_VBLANK_SECONDARY;
  else if (self->pipe > 1)
    vbl.request.type |= self->pipe << DRM_VBLANK_HIGH_CRTC_SHIFT;

  waiting = TRUE;
  if (!self->has_async_page_flip) {
    ret = drmWaitVBlank (self->fd, &vbl);
    if (ret)
      goto vblank_failed;
  } else {
    ret = drmModePageFlip (self->fd, self->crtc_id, self->buffer_id,
        DRM_MODE_PAGE_FLIP_EVENT, &waiting);
    if (ret)
      goto pageflip_failed;
  }

  while (waiting) {
    do {
      ret = gst_poll_wait (self->poll, 3 * GST_SECOND);
    } while (ret == -1 && (errno == EAGAIN || errno == EINTR));

    ret = drmHandleEvent (self->fd, &evctxt);
    if (ret)
      goto event_failed;
  }

  return TRUE;

  /* ERRORS */
vblank_failed:
  {
    GST_WARNING_OBJECT (self, "drmWaitVBlank failed: %s (%d)", strerror (-ret),
        ret);
    return FALSE;
  }
pageflip_failed:
  {
    GST_WARNING_OBJECT (self, "drmModePageFlip failed: %s (%d)",
        strerror (-ret), ret);
    return FALSE;
  }
event_failed:
  {
    GST_ERROR_OBJECT (self, "drmHandleEvent failed: %s (%d)", strerror (-ret),
        ret);
    return FALSE;
  }
}

static GstFlowReturn
gst_kms_sink_show_frame (GstVideoSink * vsink, GstBuffer * buf)
{
  static guint32 last_fb_id = 0;
  gint ret;
  GstBuffer *buffer = NULL;
  guint32 fb_id;
  GstKMSSink *self;
  GstVideoCropMeta *crop;
  GstVideoRectangle src = { 0, };
  GstVideoRectangle dst = { 0, };
  GstVideoRectangle result;
  GstFlowReturn res;
  GstVpuDecMeta *meta = NULL;
  guint32 gem_handle, w, h, fmt, bo_handles[4] = { 0, };
  guint32 offsets[4] = { 0, };
  guint32 pitches[4] = { 0, };

  self = GST_KMS_SINK (vsink);

  res = GST_FLOW_ERROR;

  meta = gst_buffer_get_vpudec_meta (buf);
  if (meta)
    buffer = buf;
  else
    goto buffer_invalid;

  gst_buffer_ref (buffer);

  ret = drmPrimeFDToHandle
      (self->fd, gst_vpudec_meta_get_fd (meta), &gem_handle);
  if (ret < 0) {
    GST_ERROR_OBJECT (self, "drmPrimeFDToHandle failed: %s (%d)",
        strerror (-ret), ret);
    return GST_FLOW_ERROR;
  }

  w = GST_VIDEO_INFO_WIDTH (&self->vinfo);
  h = GST_VIDEO_INFO_HEIGHT (&self->vinfo);
  fmt = gst_drm_format_from_video (GST_VIDEO_INFO_FORMAT (&self->vinfo));

  pitches[0] = w;
  offsets[0] = 0;
  bo_handles[0] = gem_handle;
  pitches[1] = w;
  offsets[1] = w * h;
  bo_handles[1] = gem_handle;

  ret = drmModeAddFB2 (self->fd, w, h, fmt, bo_handles, pitches,
      offsets, &fb_id, 0);
  if (ret < 0) {
    GST_ERROR_OBJECT (self, "drmModeAddFB2 failed: %s (%d)",
        strerror (-ret), ret);
    return GST_FLOW_ERROR;
  }

  GST_TRACE_OBJECT (self, "displaying fb %d", fb_id);

  if ((crop = gst_buffer_get_video_crop_meta (buffer))) {
    GstVideoInfo vinfo = self->vinfo;
    vinfo.width = crop->width;
    vinfo.height = crop->height;

    if (!gst_kms_sink_calculate_display_ratio (self, &vinfo))
      goto no_disp_ratio;

    src.x = crop->x;
    src.y = crop->y;
  }

  src.w = GST_VIDEO_SINK_WIDTH (self);
  src.h = GST_VIDEO_SINK_HEIGHT (self);

  dst.w = self->hdisplay;
  dst.h = self->vdisplay;

  gst_video_sink_center_rect (src, dst, &result, TRUE);

  if (crop) {
    src.w = crop->width;
    src.h = crop->height;
  } else {
    src.w = GST_VIDEO_INFO_WIDTH (&self->vinfo);
    src.h = GST_VIDEO_INFO_HEIGHT (&self->vinfo);
  }

  GST_TRACE_OBJECT (self,
      "drmModeSetPlane at (%i,%i) %ix%i sourcing at (%i,%i) %ix%i",
      result.x, result.y, result.w, result.h, src.x, src.y, src.w, src.h);

  ret = drmModeSetPlane (self->fd, self->plane_id, self->crtc_id, fb_id, 0,
      result.x, result.y, result.w, result.h,
      /* source/cropping coordinates are given in Q16 */
      src.x << 16, src.y << 16, src.w << 16, src.h << 16);
  if (ret)
    goto set_plane_failed;

  /* Wait for the previous frame to complete redraw */
  if (!gst_kms_sink_sync (self))
    goto bail;

  if (last_fb_id) {
    drmModeRmFB (self->fd, last_fb_id);
  }

  last_fb_id = fb_id;

  res = GST_FLOW_OK;

bail:
  gst_buffer_unref (buffer);
  return res;

  /* ERRORS */
buffer_invalid:
  {
    GST_ERROR_OBJECT (self, "invalid buffer: it doesn't have a fb id");
    goto bail;
  }
set_plane_failed:
  {
    GST_DEBUG_OBJECT (self, "result = { %d, %d, %d, %d} / "
        "src = { %d, %d, %d %d } / dst = { %d, %d, %d %d }", result.x, result.y,
        result.w, result.h, src.x, src.y, src.w, src.h, dst.x, dst.y, dst.w,
        dst.h);
    GST_ELEMENT_ERROR (self, RESOURCE, FAILED,
        (NULL), ("drmModeSetPlane failed: %s (%d)", strerror (-ret), ret));
    goto bail;
  }
no_disp_ratio:
  {
    GST_ELEMENT_ERROR (self, CORE, NEGOTIATION, (NULL),
        ("Error calculating the output display ratio of the video."));
    goto bail;
  }
}

static void
gst_kms_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstKMSSink *sink;

  sink = GST_KMS_SINK (object);

  switch (prop_id) {
    case PROP_DRIVER_NAME:
      sink->devname = g_value_dup_string (value);
      break;
    case PROP_CONNECTOR_ID:
      sink->conn_id = g_value_get_int (value);
      break;
    case PROP_PLANE_ID:
      sink->plane_id = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_kms_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstKMSSink *sink;

  sink = GST_KMS_SINK (object);

  switch (prop_id) {
    case PROP_DRIVER_NAME:
      g_value_take_string (value, sink->devname);
      break;
    case PROP_CONNECTOR_ID:
      g_value_set_int (value, sink->conn_id);
      break;
    case PROP_PLANE_ID:
      g_value_set_int (value, sink->plane_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_kms_sink_finalize (GObject * object)
{
  GstKMSSink *sink;

  sink = GST_KMS_SINK (object);
  g_clear_pointer (&sink->devname, g_free);
  gst_poll_free (sink->poll);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_kms_sink_init (GstKMSSink * sink)
{
  sink->fd = -1;
  sink->conn_id = -1;
  sink->plane_id = -1;
  gst_poll_fd_init (&sink->pollfd);
  sink->poll = gst_poll_new (TRUE);
  gst_video_info_init (&sink->vinfo);
}

static void
gst_kms_sink_class_init (GstKMSSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;
  GstBaseSinkClass *basesink_class;
  GstVideoSinkClass *videosink_class;
  GstCaps *caps;

  gobject_class = G_OBJECT_CLASS (klass);
  element_class = GST_ELEMENT_CLASS (klass);
  basesink_class = GST_BASE_SINK_CLASS (klass);
  videosink_class = GST_VIDEO_SINK_CLASS (klass);

  gst_element_class_set_static_metadata (element_class, "KMS video sink",
      "Sink/Video", GST_PLUGIN_DESC, "Víctor Jáquez <vjaquez@igalia.com>");

  caps = gst_kms_sink_caps_template_fill ();
  gst_element_class_add_pad_template (element_class,
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS, caps));
  gst_caps_unref (caps);

  basesink_class->start = GST_DEBUG_FUNCPTR (gst_kms_sink_start);
  basesink_class->stop = GST_DEBUG_FUNCPTR (gst_kms_sink_stop);
  basesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_kms_sink_set_caps);
  basesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_kms_sink_get_caps);
  basesink_class->propose_allocation = gst_kms_sink_propose_allocation;

  videosink_class->show_frame = gst_kms_sink_show_frame;

  gobject_class->finalize = gst_kms_sink_finalize;
  gobject_class->set_property = gst_kms_sink_set_property;
  gobject_class->get_property = gst_kms_sink_get_property;

  /**
   * kmssink:driver-name:
   *
   * If you have a system with multiple GPUs, you can choose which GPU
   * to use setting the DRM device driver name. Otherwise, the first
   * one from an internal list is used.
   */
  g_properties[PROP_DRIVER_NAME] = g_param_spec_string ("driver-name",
      "device name", "DRM device driver name", NULL,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  /**
   * kmssink:connector-id:
   *
   * A GPU has several output connectors, for example: LVDS, VGA,
   * HDMI, etc. By default the first LVDS is tried, then the first
   * eDP, and at the end, the first connected one.
   */
  g_properties[PROP_CONNECTOR_ID] = g_param_spec_int ("connector-id",
      "Connector ID", "DRM connector id", -1, G_MAXINT32, -1,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

   /**
   * kmssink:plane-id:
   *
   * There could be several planes associated with a CRTC.
   * By default the first plane that's possible to use with a given
   * CRTC is tried.
   */
  g_properties[PROP_PLANE_ID] = g_param_spec_int ("plane-id",
      "Plane ID", "DRM plane id", -1, G_MAXINT32, -1,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (gobject_class, PROP_N, g_properties);
}

#if 0
static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, GST_PLUGIN_NAME, GST_RANK_SECONDARY,
          GST_TYPE_KMS_SINK))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, kms,
    GST_PLUGIN_DESC, plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN)
#endif
