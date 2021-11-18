#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Our interfaces */
#include <gst/video/video.h>
#include <gst/allocators/gstdmabuf.h>
#include <gst/video/navigation.h>
#include <gst/video/videooverlay.h>

#include <gst/video/gstvideometa.h>

/* Object header */
#include "ximagesink.h"
#include "gstkmsutils.h"

/* Debugging category */
#include <gst/gstinfo.h>

/* for XkbKeycodeToKeysym */
#include <X11/XKBlib.h>

#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "gstkmsbufferpool.h"
#include "gstkmsallocator.h"

/* A random dark color */
#define RKXIMAGE_COLOR_KEY 0x010203
#define RK_COLOR_KEY_EN (1UL << 31)

GST_DEBUG_CATEGORY (gst_debug_x_image_sink);
#define GST_CAT_DEFAULT gst_debug_x_image_sink

#define EMPTY_RECT(r) !((r).x || (r).y || (r).w || (r).h)
#define RECT_EQUAL(r1, r2) \
    ((r1).x == (r2).x && (r1).y == (r2).y && \
     (r1).w == (r2.w) && (r1).h == (r2).h)

typedef struct
{
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  long input_mode;
  unsigned long status;
}
MotifWmHints, MwmHints;

#define MWM_HINTS_DECORATIONS   (1L << 1)

static void gst_x_image_sink_reset (GstRkXImageSink * ximagesink);
static void gst_x_image_sink_xwindow_update_geometry (GstRkXImageSink *
    ximagesink);
static void gst_x_image_sink_expose (GstVideoOverlay * overlay);
static void gst_x_image_sink_xwindow_clear (GstRkXImageSink * ximagesink,
    GstXWindow * xwindow);
static GstFlowReturn
gst_x_image_sink_show_frame (GstVideoSink * vsink, GstBuffer * buf);

static GstStaticPadTemplate gst_x_image_sink_sink_template_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "framerate = (fraction) [ 0, MAX ], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ]")
    );

enum
{
  PROP_0,
  PROP_DISPLAY,
  PROP_SYNCHRONOUS,
  PROP_HANDLE_EVENTS,
  PROP_HANDLE_EXPOSE,
  PROP_WINDOW_WIDTH,
  PROP_WINDOW_HEIGHT,
  PROP_DRIVER_NAME,
  PROP_CONNECTOR_ID,
  PROP_PLANE_ID
};

/* ============================================================= */
/*                                                               */
/*                       Public Methods                          */
/*                                                               */
/* ============================================================= */

/* =========================================== */
/*                                             */
/*          Object typing & Creation           */
/*                                             */
/* =========================================== */
static void gst_x_image_sink_navigation_init (GstNavigationInterface * iface);
static void gst_x_image_sink_video_overlay_init (GstVideoOverlayInterface *
    iface);
#define gst_x_image_sink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstRkXImageSink, gst_x_image_sink, GST_TYPE_VIDEO_SINK,
    G_IMPLEMENT_INTERFACE (GST_TYPE_NAVIGATION,
        gst_x_image_sink_navigation_init);
    G_IMPLEMENT_INTERFACE (GST_TYPE_VIDEO_OVERLAY,
        gst_x_image_sink_video_overlay_init));

/* ============================================================= */
/*                                                               */
/*                       Private Methods                         */
/*                                                               */
/* ============================================================= */
/*drm*/

static int
drm_plane_get_type (int fd, drmModePlane * plane)
{
  drmModeObjectPropertiesPtr props;
  drmModePropertyPtr prop;
  int i, type = -1;

  props = drmModeObjectGetProperties (fd, plane->plane_id,
      DRM_MODE_OBJECT_PLANE);
  if (!props)
    return -1;

  for (i = 0; i < props->count_props; i++) {
    prop = drmModeGetProperty (fd, props->props[i]);
    if (prop && !strcmp (prop->name, "type"))
      type = props->prop_values[i];
    drmModeFreeProperty (prop);
  }

  drmModeFreeObjectProperties (props);
  return type;
}

static drmModePlane *
drm_find_plane_for_crtc_by_type (int fd, drmModeRes * res,
    drmModePlaneRes * pres, int crtc_id, int type)
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
    if (plane->possible_crtcs & (1 << pipe)) {
      if (drm_plane_get_type (fd, plane) == type)
        return plane;
    }
    drmModeFreePlane (plane);
  }

  return NULL;
}

static drmModeCrtc *
drm_find_crtc_for_connector (int fd, drmModeRes * res, drmModeConnector * conn,
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
drm_connector_is_used (int fd, drmModeRes * res, drmModeConnector * conn)
{
  gboolean result;
  drmModeCrtc *crtc;

  result = FALSE;
  crtc = drm_find_crtc_for_connector (fd, res, conn, NULL);
  if (crtc) {
    result = crtc->buffer_id != 0;
    drmModeFreeCrtc (crtc);
  }

  return result;
}

static drmModeConnector *
drm_find_used_connector_by_type (int fd, drmModeRes * res, int type)
{
  int i;
  drmModeConnector *conn;

  conn = NULL;
  for (i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector (fd, res->connectors[i]);
    if (conn) {
      if ((conn->connector_type == type)
          && drm_connector_is_used (fd, res, conn))
        return conn;
      drmModeFreeConnector (conn);
    }
  }

  return NULL;
}

static drmModeConnector *
drm_find_first_used_connector (int fd, drmModeRes * res)
{
  int i;
  drmModeConnector *conn;

  conn = NULL;
  for (i = 0; i < res->count_connectors; i++) {
    conn = drmModeGetConnector (fd, res->connectors[i]);
    if (conn) {
      if (drm_connector_is_used (fd, res, conn))
        return conn;
      drmModeFreeConnector (conn);
    }
  }

  return NULL;
}

static drmModeConnector *
drm_find_main_monitor (int fd, drmModeRes * res)
{
  /* Find the LVDS and eDP connectors: those are the main screens. */
  static const int priority[] = { DRM_MODE_CONNECTOR_LVDS,
    DRM_MODE_CONNECTOR_eDP
  };
  int i;
  drmModeConnector *conn;

  conn = NULL;
  for (i = 0; !conn && i < G_N_ELEMENTS (priority); i++)
    conn = drm_find_used_connector_by_type (fd, res, priority[i]);

  /* if we didn't find a connector, grab the first one in use */
  if (!conn)
    conn = drm_find_first_used_connector (fd, res);

  return conn;
}

static void
drm_log_version (GstRkXImageSink * self)
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
drm_get_caps (GstRkXImageSink * self)
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
  else {
    self->has_prime_import = (gboolean) (has_prime & DRM_PRIME_CAP_IMPORT);
    self->has_prime_export = (gboolean) (has_prime & DRM_PRIME_CAP_EXPORT);
  }

  has_async_page_flip = 0;
  ret = drmGetCap (self->fd, DRM_CAP_ASYNC_PAGE_FLIP, &has_async_page_flip);
  if (ret)
    GST_WARNING_OBJECT (self, "could not get async page flip capability");
  else
    self->has_async_page_flip = (gboolean) has_async_page_flip;

  GST_INFO_OBJECT (self,
      "prime import (%s) / prime export (%s) / async page flip (%s)",
      self->has_prime_import ? "✓" : "✗",
      self->has_prime_export ? "✓" : "✗",
      self->has_async_page_flip ? "✓" : "✗");

  return TRUE;
}

static gboolean
drm_ensure_allowed_caps (GstRkXImageSink * self, drmModePlane * plane,
    drmModeRes * res)
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
drm_plane_set_property (GstRkXImageSink * self, drmModePlane * plane,
    const char *prop_name, uint64_t prop_value)
{
  drmModeObjectPropertiesPtr props;
  drmModePropertyPtr prop;
  int i, ret = -1;

  props = drmModeObjectGetProperties (self->fd, plane->plane_id,
      DRM_MODE_OBJECT_PLANE);
  if (!props)
    return FALSE;

  for (i = 0; i < props->count_props; i++) {
    prop = drmModeGetProperty (self->fd, props->props[i]);
    if (prop && !strcmp (prop->name, prop_name)) {
      ret = drmModeObjectSetProperty (self->fd, plane->plane_id,
          DRM_MODE_OBJECT_PLANE, props->props[i], prop_value);
    }
    drmModeFreeProperty (prop);
  }

  drmModeFreeObjectProperties (props);
  return ret < 0 ? FALSE : TRUE;
}

static gboolean
drm_prepare_planes (GstRkXImageSink * self, drmModeRes * res,
    drmModePlaneRes * pres)
{
  drmModePlane *plane;
  gboolean ret = FALSE;

  if (drmSetClientCap (self->fd, DRM_CLIENT_CAP_ATOMIC, 1))
    return FALSE;

  /* Apply colorkey to primary plane */
  plane = drm_find_plane_for_crtc_by_type (self->fd, res, pres,
      self->crtc_id, DRM_PLANE_TYPE_PRIMARY);
  if (!plane)
    return FALSE;

  if (!drm_plane_set_property (self, plane, "colorkey",
          RKXIMAGE_COLOR_KEY | RK_COLOR_KEY_EN))
    goto out;

  GST_DEBUG_OBJECT (self, "applied colorkey = 0x%x to plane %d",
      RKXIMAGE_COLOR_KEY, plane->plane_id);

  /* Uper primary plane */
  if (!drm_plane_set_property (self, plane, "ZPOS", 1) &&
      !drm_plane_set_property (self, plane, "zpos", 1))
    goto out;

  drmModeFreePlane (plane);

  /* Lower overlay plane */
  plane = drmModeGetPlane (self->fd, self->plane_id);
  if (!plane)
    goto out;

  if (!drm_plane_set_property (self, plane, "ZPOS", 0) &&
      !drm_plane_set_property (self, plane, "zpos", 0))
    goto out;

  GST_DEBUG_OBJECT (self, "set plane %d zpos to 0", plane->plane_id);

  ret = TRUE;

out:
  drmModeFreePlane (plane);
  return ret;
}

/*kms*/
static void
ensure_kms_allocator (GstRkXImageSink * self)
{
  if (self->allocator)
    return;
  self->allocator = gst_kms_allocator_new (self->fd);
}

static GstBufferPool *
gst_kms_sink_create_pool (GstRkXImageSink * self, GstCaps * caps, gsize size,
    gint min)
{
  GstBufferPool *pool;
  GstStructure *config;

  pool = gst_kms_buffer_pool_new ();
  if (!pool)
    goto pool_failed;

  config = gst_buffer_pool_get_config (pool);
  gst_buffer_pool_config_set_params (config, caps, size, min, 0);
  gst_buffer_pool_config_add_option (config, GST_BUFFER_POOL_OPTION_VIDEO_META);

  ensure_kms_allocator (self);
  gst_buffer_pool_config_set_allocator (config, self->allocator, NULL);

  if (!gst_buffer_pool_set_config (pool, config))
    goto config_failed;

  return pool;

  /* ERRORS */
pool_failed:
  {
    GST_ERROR_OBJECT (self, "failed to create buffer pool");
    return NULL;
  }
config_failed:
  {
    GST_ERROR_OBJECT (self, "failed to set config");
    gst_object_unref (pool);
    return NULL;
  }
}

static gboolean
gst_kms_sink_propose_allocation (GstBaseSink * bsink, GstQuery * query)
{
  GstRkXImageSink *self;
  GstCaps *caps;
  gboolean need_pool;
  GstVideoInfo vinfo;
  GstBufferPool *pool;
  gsize size;

  self = GST_X_IMAGE_SINK (bsink);

  gst_query_parse_allocation (query, &caps, &need_pool);
  if (!caps)
    goto no_caps;
  if (!gst_video_info_from_caps (&vinfo, caps))
    goto invalid_caps;

  size = GST_VIDEO_INFO_SIZE (&vinfo);

  pool = NULL;
  if (need_pool) {
    pool = gst_kms_sink_create_pool (self, caps, size, 0);
    if (!pool)
      goto no_pool;

    /* Only export for pool used upstream */
    if (self->has_prime_export) {
      GstStructure *config = gst_buffer_pool_get_config (pool);
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_KMS_PRIME_EXPORT);
      gst_buffer_pool_set_config (pool, config);
    }
  }

  /* we need at least 2 buffer because we hold on to the last one */
  gst_query_add_allocation_pool (query, pool, size, 2, 0);
  if (pool)
    gst_object_unref (pool);

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
no_pool:
  {
    /* Already warned in create_pool */
    return FALSE;
  }
}

static gboolean
gst_kms_sink_import_dmabuf (GstRkXImageSink * self, GstBuffer * inbuf,
    GstBuffer ** outbuf)
{
  gint prime_fds[GST_VIDEO_MAX_PLANES] = { 0, };
  GstVideoMeta *meta;
  guint i, n_mem, n_planes;
  GstKMSMemory *kmsmem;
  guint mems_idx[GST_VIDEO_MAX_PLANES];
  gsize mems_skip[GST_VIDEO_MAX_PLANES];
  GstMemory *mems[GST_VIDEO_MAX_PLANES];

  if (!self->has_prime_import)
    return FALSE;

  /* This will eliminate most non-dmabuf out there */
  if (!gst_is_dmabuf_memory (gst_buffer_peek_memory (inbuf, 0)))
    return FALSE;

  n_planes = GST_VIDEO_INFO_N_PLANES (&self->vinfo);
  n_mem = gst_buffer_n_memory (inbuf);
  meta = gst_buffer_get_video_meta (inbuf);

  GST_TRACE_OBJECT (self, "Found a dmabuf with %u planes and %u memories",
      n_planes, n_mem);

  /* We cannot have multiple dmabuf per plane */
  if (n_mem > n_planes)
    return FALSE;
  g_assert (n_planes != 0);

  /* Update video info based on video meta */
  if (meta) {
    GST_VIDEO_INFO_WIDTH (&self->vinfo) = meta->width;
    GST_VIDEO_INFO_HEIGHT (&self->vinfo) = meta->height;

    for (i = 0; i < meta->n_planes; i++) {
      GST_VIDEO_INFO_PLANE_OFFSET (&self->vinfo, i) = meta->offset[i];
      GST_VIDEO_INFO_PLANE_STRIDE (&self->vinfo, i) = meta->stride[i];
    }
  }

  /* Find and validate all memories */
  for (i = 0; i < n_planes; i++) {
    guint length;

    if (!gst_buffer_find_memory (inbuf,
            GST_VIDEO_INFO_PLANE_OFFSET (&self->vinfo, i), 1,
            &mems_idx[i], &length, &mems_skip[i]))
      return FALSE;

    mems[i] = gst_buffer_peek_memory (inbuf, mems_idx[i]);

    /* adjust for memory offset, in case data does not
     * start from byte 0 in the dmabuf fd */
    mems_skip[i] += mems[i]->offset;

    /* And all memory found must be dmabuf */
    if (!gst_is_dmabuf_memory (mems[i]))
      return FALSE;
  }

  kmsmem = (GstKMSMemory *) gst_kms_allocator_get_cached (mems[0]);
  if (kmsmem) {
    GST_LOG_OBJECT (self, "found KMS mem %p in DMABuf mem %p with fb id = %d",
        kmsmem, mems[0], kmsmem->fb_id);
    goto wrap_mem;
  }

  for (i = 0; i < n_planes; i++)
    prime_fds[i] = gst_dmabuf_memory_get_fd (mems[i]);

  GST_LOG_OBJECT (self, "found these prime ids: %d, %d, %d, %d", prime_fds[0],
      prime_fds[1], prime_fds[2], prime_fds[3]);

  kmsmem = gst_kms_allocator_dmabuf_import (self->allocator,
      prime_fds, n_planes, mems_skip, &self->vinfo);
  if (!kmsmem)
    return FALSE;

  GST_LOG_OBJECT (self, "setting KMS mem %p to DMABuf mem %p with fb id = %d",
      kmsmem, mems[0], kmsmem->fb_id);
  gst_kms_allocator_cache (self->allocator, mems[0], GST_MEMORY_CAST (kmsmem));

wrap_mem:
  *outbuf = gst_buffer_new ();
  if (!*outbuf)
    return FALSE;
  gst_buffer_append_memory (*outbuf, gst_memory_ref (GST_MEMORY_CAST (kmsmem)));
  gst_buffer_add_parent_buffer_meta (*outbuf, inbuf);

  return TRUE;
}

static GstBuffer *
gst_kms_sink_copy_to_dumb_buffer (GstRkXImageSink * self, GstBuffer * inbuf)
{
  GstFlowReturn ret;
  GstVideoFrame inframe, outframe;
  gboolean success;
  GstBuffer *buf = NULL;

  if (!gst_buffer_pool_set_active (self->pool, TRUE))
    goto activate_pool_failed;

  ret = gst_buffer_pool_acquire_buffer (self->pool, &buf, NULL);
  if (ret != GST_FLOW_OK)
    goto create_buffer_failed;

  if (!gst_video_frame_map (&inframe, &self->vinfo, inbuf, GST_MAP_READ))
    goto error_map_src_buffer;

  if (!gst_video_frame_map (&outframe, &self->vinfo, buf, GST_MAP_WRITE))
    goto error_map_dst_buffer;

  success = gst_video_frame_copy (&outframe, &inframe);
  gst_video_frame_unmap (&outframe);
  gst_video_frame_unmap (&inframe);
  if (!success)
    goto error_copy_buffer;

  return buf;

bail:
  {
    if (buf)
      gst_buffer_unref (buf);
    return NULL;
  }

  /* ERRORS */
activate_pool_failed:
  {
    GST_ELEMENT_ERROR (self, STREAM, FAILED, ("failed to activate buffer pool"),
        ("failed to activate buffer pool"));
    return NULL;
  }
create_buffer_failed:
  {
    GST_ELEMENT_ERROR (self, STREAM, FAILED, ("allocation failed"),
        ("failed to create buffer"));
    return NULL;
  }
error_copy_buffer:
  {
    GST_WARNING_OBJECT (self, "failed to upload buffer");
    goto bail;
  }
error_map_dst_buffer:
  {
    gst_video_frame_unmap (&inframe);
    /* fall-through */
  }
error_map_src_buffer:
  {
    GST_WARNING_OBJECT (self, "failed to map buffer");
    goto bail;
  }
}

static GstBuffer *
gst_kms_sink_get_input_buffer (GstRkXImageSink * self, GstBuffer * inbuf)
{
  GstMemory *mem;
  GstBuffer *buf = NULL;

  mem = gst_buffer_peek_memory (inbuf, 0);
  if (!mem)
    return NULL;

  if (gst_is_kms_memory (mem))
    return gst_buffer_ref (inbuf);

  if (gst_kms_sink_import_dmabuf (self, inbuf, &buf))
    goto done;

  buf = gst_kms_sink_copy_to_dumb_buffer (self, inbuf);

done:
  /* Copy all the non-memory related metas, this way CropMeta will be
   * available upon GstVideoOverlay::expose calls. */
  if (buf)
    gst_buffer_copy_into (buf, inbuf, GST_BUFFER_COPY_METADATA, 0, -1);

  return buf;
}

static void
sync_handler (gint fd, guint frame, guint sec, guint usec, gpointer data)
{
  gboolean *waiting;

  waiting = data;
  *waiting = FALSE;
}

static gboolean
gst_kms_sink_sync (GstRkXImageSink * self)
{
  gint ret;
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

  if (self->pipe == 1)
    vbl.request.type |= DRM_VBLANK_SECONDARY;
  else if (self->pipe > 1)
    vbl.request.type |= self->pipe << DRM_VBLANK_HIGH_CRTC_SHIFT;

  waiting = TRUE;
  ret = drmWaitVBlank (self->fd, &vbl);
  if (ret)
    goto vblank_failed;

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
    GST_WARNING_OBJECT (self, "drmWaitVBlank failed: %s (%d)",
        g_strerror (errno), errno);
    return FALSE;
  }
event_failed:
  {
    GST_ERROR_OBJECT (self, "drmHandleEvent failed: %s (%d)",
        g_strerror (errno), errno);
    return FALSE;
  }
}

static void
gst_kms_sink_drain (GstRkXImageSink * self)
{
  GstParentBufferMeta *parent_meta;

  GST_DEBUG_OBJECT (self, "draining");

  if (!self->last_buffer)
    return;

  /* We only need to return the last_buffer if it depends on upstream buffer.
   * In this case, the last_buffer will have a GstParentBufferMeta set. */
  parent_meta = gst_buffer_get_parent_buffer_meta (self->last_buffer);
  if (parent_meta) {
    GstBuffer *dumb_buf;
    dumb_buf = gst_kms_sink_copy_to_dumb_buffer (self, parent_meta->buffer);

    gst_kms_allocator_clear_cache (self->allocator);
    gst_x_image_sink_show_frame (GST_VIDEO_SINK (self), dumb_buf);
    gst_buffer_unref (dumb_buf);
  }
}

static gboolean
gst_kms_sink_query (GstBaseSink * bsink, GstQuery * query)
{
  GstRkXImageSink *self = GST_X_IMAGE_SINK (bsink);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_ALLOCATION:
    case GST_QUERY_DRAIN:
    {
      gst_kms_sink_drain (self);
      break;
    }
    default:
      break;
  }

  return GST_BASE_SINK_CLASS (parent_class)->query (bsink, query);
}

/*ximagesink*/
static gboolean
xwindow_calculate_display_ratio (GstRkXImageSink * self, int *x, int *y,
    gint * window_width, gint * window_height)
{
  guint dar_n, dar_d;
  guint video_par_n, video_par_d;
  guint dpy_par_n, dpy_par_d;
  gint video_width, video_height;

  video_width = GST_VIDEO_INFO_WIDTH (&self->vinfo);
  video_height = GST_VIDEO_INFO_HEIGHT (&self->vinfo);

  video_par_n = self->par_n;
  video_par_d = self->par_d;

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
  video_width = gst_util_uint64_scale_int (self->xwindow->height, dar_n, dar_d);
  video_height = gst_util_uint64_scale_int (self->xwindow->width, dar_d, dar_n);
  if (video_width < *window_width) {
    *x += (self->xwindow->width - video_width) / 2;
    *window_width = video_width;
    *window_height = self->xwindow->height;
  } else {
    *y += (self->xwindow->height - video_height) / 2;
    *window_height = video_height;
    *window_width = self->xwindow->width;
  }

  GST_DEBUG_OBJECT (self, "scaling to %dx%d", *window_width, *window_height);

  return TRUE;
}

/* X11 stuff */

static gboolean
xwindow_get_window_position (GstRkXImageSink * ximagesink, int *x, int *y)
{
  XWindowAttributes attr;
  Window child;
  int tmp_x, tmp_y;
  static int last_x = 0, last_y = 0;

  if (x == NULL || y == NULL) {
    x = &tmp_x;
    y = &tmp_y;
  }

  XGetWindowAttributes (ximagesink->xcontext->disp,
      ximagesink->xwindow->win, &attr);

  XTranslateCoordinates (ximagesink->xcontext->disp, ximagesink->xwindow->win,
      ximagesink->xcontext->root, 0, 0, x, y, &child);

  if (last_x != *x || last_y != *y) {
    last_x = *x;
    last_y = *y;
    /* moved */
    return TRUE;
  }

  return FALSE;
}

static void
xwindow_get_render_rectangle (GstRkXImageSink * ximagesink,
    gint * x, gint * y, gint * width, gint * height)
{
  if (ximagesink->save_rect.w != 0 && ximagesink->save_rect.h != 0) {
    *width = ximagesink->save_rect.w;
    *height = ximagesink->save_rect.h;
    *x += ximagesink->save_rect.x;
    *y += ximagesink->save_rect.y;
  }
}

static void
gst_x_image_sink_xwindow_fill_key (GstRkXImageSink * ximagesink,
    GstXWindow * xwindow, guint32 color)
{
  XSetForeground (ximagesink->xcontext->disp, xwindow->gc, color);
  XFillRectangle (ximagesink->xcontext->disp, xwindow->win, xwindow->gc,
      ximagesink->clip_rect.x, ximagesink->clip_rect.y,
      ximagesink->clip_rect.x + ximagesink->clip_rect.w,
      ximagesink->clip_rect.y + ximagesink->clip_rect.h);
}

/* We are called with the x_lock taken */
static void
gst_x_image_sink_xwindow_draw_borders (GstRkXImageSink * ximagesink,
    GstXWindow * xwindow, GstVideoRectangle rect)
{
  g_return_if_fail (GST_IS_X_IMAGE_SINK (ximagesink));
  g_return_if_fail (xwindow != NULL);

  XSetForeground (ximagesink->xcontext->disp, xwindow->gc,
      ximagesink->xcontext->black);

  /* Left border */
  if (rect.x > 0) {
    XFillRectangle (ximagesink->xcontext->disp, xwindow->win, xwindow->gc,
        0, 0, rect.x, xwindow->height);
  }

  /* Right border */
  if ((rect.x + rect.w) < xwindow->width) {
    XFillRectangle (ximagesink->xcontext->disp, xwindow->win, xwindow->gc,
        rect.x + rect.w, 0, xwindow->width, xwindow->height);
  }

  /* Top border */
  if (rect.y > 0) {
    XFillRectangle (ximagesink->xcontext->disp, xwindow->win, xwindow->gc,
        0, 0, xwindow->width, rect.y);
  }

  /* Bottom border */
  if ((rect.y + rect.h) < xwindow->height) {
    XFillRectangle (ximagesink->xcontext->disp, xwindow->win, xwindow->gc,
        0, rect.y + rect.h, xwindow->width, xwindow->height);
  }
}

/* This function puts a GstXImageBuffer on a GstRkXImageSink's window */
static gboolean
gst_x_image_sink_ximage_put (GstRkXImageSink * ximagesink, GstBuffer * buf)
{
  GstVideoCropMeta *crop;
  GstVideoRectangle src = { 0, };
  GstVideoRectangle result = { 0, };
  GstBuffer *buffer = NULL;
  gboolean draw_border = FALSE;
  gboolean res = FALSE;
  gboolean expose = buf == NULL;
  guint32 fb_id;
  gint ret;

  /* We take the flow_lock. If expose is in there we don't want to run
     concurrently from the data flow thread */
  g_mutex_lock (&ximagesink->flow_lock);

  if (G_UNLIKELY (ximagesink->xwindow == NULL)) {
    g_mutex_unlock (&ximagesink->flow_lock);
    return FALSE;
  }

  /* Draw borders when displaying the first frame. After this
     draw borders only on expose event or caps change (ximagesink->draw_border = TRUE). */
  if (expose || !ximagesink->last_buffer || ximagesink->draw_border)
    draw_border = TRUE;

  if (buf)
    buffer = gst_kms_sink_get_input_buffer (ximagesink, buf);
  else if (ximagesink->last_buffer)
    buffer = gst_buffer_ref (ximagesink->last_buffer);

  /* Make sure buf is not used accidentally */
  buf = NULL;

  if (!buffer)
    goto buffer_invalid;
  fb_id = gst_kms_memory_get_fb_id (gst_buffer_peek_memory (buffer, 0));
  if (fb_id == 0)
    goto buffer_invalid;

  GST_TRACE_OBJECT (ximagesink, "displaying fb %d", fb_id);

  crop = gst_buffer_get_video_crop_meta (buffer);
  if (crop) {
    src.x = crop->x;
    src.y = crop->y;
    src.w = crop->width;
    src.h = crop->height;
    GST_LOG_OBJECT (ximagesink,
        "crop %dx%d-%dx%d", crop->x, crop->y, crop->width, crop->height);
  } else {
    src.w = GST_VIDEO_SINK_WIDTH (ximagesink);
    src.h = GST_VIDEO_SINK_HEIGHT (ximagesink);
  }
  result.w = ximagesink->xwindow->width;
  result.h = ximagesink->xwindow->height;

  g_mutex_lock (&ximagesink->x_lock);

  /* Fill color key when clip rect changed */
  if (draw_border || !RECT_EQUAL (ximagesink->clip_rect, result)) {
    ximagesink->clip_rect = result;
    gst_x_image_sink_xwindow_fill_key (ximagesink,
        ximagesink->xwindow, RKXIMAGE_COLOR_KEY);

    /* Do an extra repaint when handing expose */
    if (expose)
      memset (&ximagesink->clip_rect, 0, sizeof (GstVideoRectangle));
  }

  if (draw_border) {
    gst_x_image_sink_xwindow_draw_borders (ximagesink, ximagesink->xwindow,
        result);
    ximagesink->draw_border = FALSE;
  }

  xwindow_get_window_position (ximagesink, &result.x, &result.y);

  xwindow_get_render_rectangle (ximagesink, &result.x, &result.y, &result.w,
      &result.h);
  xwindow_calculate_display_ratio (ximagesink, &result.x, &result.y, &result.w,
      &result.h);

  GST_TRACE_OBJECT (ximagesink,
      "drmModeSetPlane at (%i,%i) %ix%i sourcing at (%i,%i) %ix%i",
      result.x, result.y, result.w, result.h, src.x, src.y, src.w, src.h);

  ret = drmModeSetPlane (ximagesink->fd, ximagesink->plane_id,
      ximagesink->crtc_id, fb_id, 0, result.x, result.y, result.w, result.h,
      /* source/cropping coordinates are given in Q16 */
      src.x << 16, src.y << 16, src.w << 16, src.h << 16);

  if (ret) {
    GST_ERROR_OBJECT (ximagesink, "drmModesetplane failed: %d", ret);
    goto out;
  }


  /* HACK: Disable vsync might cause tearing */
  if (!g_getenv ("KMSSINK_DISABLE_VSYNC"))
  /* Wait for the previous frame to complete redraw */
  if (!gst_kms_sink_sync (ximagesink))
    goto out;

  if (buffer != ximagesink->last_buffer)
    gst_buffer_replace (&ximagesink->last_buffer, buffer);

  res = TRUE;

out:
  if (buffer)
    gst_buffer_unref (buffer);
  g_mutex_unlock (&ximagesink->x_lock);
  g_mutex_unlock (&ximagesink->flow_lock);
  return res;

buffer_invalid:
  gst_x_image_sink_xwindow_clear (ximagesink, ximagesink->xwindow);
  if (buffer)
    gst_buffer_unref (buffer);
  g_mutex_unlock (&ximagesink->flow_lock);

  return TRUE;
}

static gboolean
gst_x_image_sink_xwindow_decorate (GstRkXImageSink * ximagesink,
    GstXWindow * window)
{
  Atom hints_atom = None;
  MotifWmHints *hints;

  g_return_val_if_fail (GST_IS_X_IMAGE_SINK (ximagesink), FALSE);
  g_return_val_if_fail (window != NULL, FALSE);

  g_mutex_lock (&ximagesink->x_lock);

  hints_atom = XInternAtom (ximagesink->xcontext->disp, "_MOTIF_WM_HINTS",
      True);
  if (hints_atom == None) {
    g_mutex_unlock (&ximagesink->x_lock);
    return FALSE;
  }

  hints = g_malloc0 (sizeof (MotifWmHints));

  hints->flags |= MWM_HINTS_DECORATIONS;
  hints->decorations = 1 << 0;

  XChangeProperty (ximagesink->xcontext->disp, window->win,
      hints_atom, hints_atom, 32, PropModeReplace,
      (guchar *) hints, sizeof (MotifWmHints) / sizeof (long));

  XSync (ximagesink->xcontext->disp, FALSE);

  g_mutex_unlock (&ximagesink->x_lock);

  g_free (hints);

  return TRUE;
}

static void
gst_x_image_sink_xwindow_set_title (GstRkXImageSink * ximagesink,
    GstXWindow * xwindow, const gchar * media_title)
{
  if (media_title) {
    g_free (ximagesink->media_title);
    ximagesink->media_title = g_strdup (media_title);
  }
  if (xwindow) {
    /* we have a window */
    if (xwindow->internal) {
      XTextProperty xproperty;
      XClassHint *hint = XAllocClassHint ();
      const gchar *app_name;
      const gchar *title = NULL;
      gchar *title_mem = NULL;

      /* set application name as a title */
      app_name = g_get_application_name ();

      if (app_name && ximagesink->media_title) {
        title = title_mem = g_strconcat (ximagesink->media_title, " : ",
            app_name, NULL);
      } else if (app_name) {
        title = app_name;
      } else if (ximagesink->media_title) {
        title = ximagesink->media_title;
      }

      if (title) {
        if ((XStringListToTextProperty (((char **) &title), 1,
                    &xproperty)) != 0) {
          XSetWMName (ximagesink->xcontext->disp, xwindow->win, &xproperty);
          XFree (xproperty.value);
        }

        g_free (title_mem);
      }

      if (hint) {
        hint->res_name = (char *) app_name;
        hint->res_class = (char *) "GStreamer";
        XSetClassHint (ximagesink->xcontext->disp, xwindow->win, hint);
      }
      XFree (hint);
    }
  }
}

/* This function handles a GstXWindow creation */
static GstXWindow *
gst_x_image_sink_xwindow_new (GstRkXImageSink * ximagesink, gint width,
    gint height)
{
  GstXWindow *xwindow = NULL;
  XGCValues values;

  g_return_val_if_fail (GST_IS_X_IMAGE_SINK (ximagesink), NULL);

  xwindow = g_new0 (GstXWindow, 1);

  xwindow->width = width;
  xwindow->height = height;
  xwindow->internal = TRUE;

  g_mutex_lock (&ximagesink->x_lock);

  xwindow->win = XCreateSimpleWindow (ximagesink->xcontext->disp,
      ximagesink->xcontext->root,
      0, 0, width, height, 0, 0, ximagesink->xcontext->black);

  /* We have to do that to prevent X from redrawing the background on
     ConfigureNotify. This takes away flickering of video when resizing. */
  XSetWindowBackgroundPixmap (ximagesink->xcontext->disp, xwindow->win, None);

  /* set application name as a title */
  gst_x_image_sink_xwindow_set_title (ximagesink, xwindow, NULL);

  if (ximagesink->handle_events) {
    Atom wm_delete;

    XSelectInput (ximagesink->xcontext->disp, xwindow->win, ExposureMask |
        StructureNotifyMask | PointerMotionMask | KeyPressMask |
        KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);

    /* Tell the window manager we'd like delete client messages instead of
     * being killed */
    wm_delete = XInternAtom (ximagesink->xcontext->disp,
        "WM_DELETE_WINDOW", False);
    (void) XSetWMProtocols (ximagesink->xcontext->disp, xwindow->win,
        &wm_delete, 1);
  }

  xwindow->gc = XCreateGC (ximagesink->xcontext->disp, xwindow->win,
      0, &values);

  XMapRaised (ximagesink->xcontext->disp, xwindow->win);

  XSync (ximagesink->xcontext->disp, FALSE);

  g_mutex_unlock (&ximagesink->x_lock);

  gst_x_image_sink_xwindow_decorate (ximagesink, xwindow);

  gst_video_overlay_got_window_handle (GST_VIDEO_OVERLAY (ximagesink),
      xwindow->win);

  return xwindow;
}

/* This function destroys a GstXWindow */
static void
gst_x_image_sink_xwindow_destroy (GstRkXImageSink * ximagesink,
    GstXWindow * xwindow)
{
  g_return_if_fail (xwindow != NULL);
  g_return_if_fail (GST_IS_X_IMAGE_SINK (ximagesink));

  g_mutex_lock (&ximagesink->x_lock);

  /* clean screen */
  if (ximagesink->last_fb_id) {
    drmModeRmFB (ximagesink->fd, ximagesink->last_fb_id);
    ximagesink->last_fb_id = 0;
  }

  /* If we did not create that window we just free the GC and let it live */
  if (xwindow->internal)
    XDestroyWindow (ximagesink->xcontext->disp, xwindow->win);
  else
    XSelectInput (ximagesink->xcontext->disp, xwindow->win, 0);

  XFreeGC (ximagesink->xcontext->disp, xwindow->gc);

  XSync (ximagesink->xcontext->disp, FALSE);

  g_mutex_unlock (&ximagesink->x_lock);

  g_free (xwindow);
}

static void
gst_x_image_sink_xwindow_update_geometry (GstRkXImageSink * ximagesink)
{
  XWindowAttributes attr;
  gboolean reconfigure;

  g_return_if_fail (GST_IS_X_IMAGE_SINK (ximagesink));

  /* Update the window geometry */
  g_mutex_lock (&ximagesink->x_lock);
  if (G_UNLIKELY (ximagesink->xwindow == NULL)) {
    g_mutex_unlock (&ximagesink->x_lock);
    return;
  }

  XGetWindowAttributes (ximagesink->xcontext->disp,
      ximagesink->xwindow->win, &attr);

  /* Check if we would suggest a different width/height now */
  reconfigure = (ximagesink->xwindow->width != attr.width)
      || (ximagesink->xwindow->height != attr.height);
  ximagesink->xwindow->width = attr.width;
  ximagesink->xwindow->height = attr.height;

  g_mutex_unlock (&ximagesink->x_lock);

  if (reconfigure)
    gst_pad_push_event (GST_BASE_SINK (ximagesink)->sinkpad,
        gst_event_new_reconfigure ());
}

static void
gst_x_image_sink_xwindow_clear (GstRkXImageSink * ximagesink,
    GstXWindow * xwindow)
{
  g_return_if_fail (xwindow != NULL);
  g_return_if_fail (GST_IS_X_IMAGE_SINK (ximagesink));

  g_mutex_lock (&ximagesink->x_lock);

  XSetForeground (ximagesink->xcontext->disp, xwindow->gc,
      ximagesink->xcontext->black);

  XFillRectangle (ximagesink->xcontext->disp, xwindow->win, xwindow->gc,
      0, 0, xwindow->width, xwindow->height);

  /* clean screen */
  if (ximagesink->last_fb_id) {
    drmModeRmFB (ximagesink->fd, ximagesink->last_fb_id);
    ximagesink->last_fb_id = 0;
  }

  g_mutex_unlock (&ximagesink->x_lock);
}

/* This function handles XEvents that might be in the queue. It generates
   GstEvent that will be sent upstream in the pipeline to handle interactivity
   and navigation.*/
static void
gst_x_image_sink_handle_xevents (GstRkXImageSink * ximagesink)
{
  XEvent e;
  guint pointer_x = 0, pointer_y = 0;
  gboolean pointer_moved = FALSE;
  gboolean exposed = FALSE, configured = FALSE;

  g_return_if_fail (GST_IS_X_IMAGE_SINK (ximagesink));

  /* Then we get all pointer motion events, only the last position is
     interesting. */
  g_mutex_lock (&ximagesink->flow_lock);
  g_mutex_lock (&ximagesink->x_lock);
  while (XCheckWindowEvent (ximagesink->xcontext->disp,
          ximagesink->xwindow->win, PointerMotionMask, &e)) {
    g_mutex_unlock (&ximagesink->x_lock);
    g_mutex_unlock (&ximagesink->flow_lock);

    switch (e.type) {
      case MotionNotify:
        pointer_x = e.xmotion.x;
        pointer_y = e.xmotion.y;
        pointer_moved = TRUE;
        break;
      default:
        break;
    }
    g_mutex_lock (&ximagesink->flow_lock);
    g_mutex_lock (&ximagesink->x_lock);
  }

  if (pointer_moved) {
    g_mutex_unlock (&ximagesink->x_lock);
    g_mutex_unlock (&ximagesink->flow_lock);

    GST_DEBUG ("ximagesink pointer moved over window at %d,%d",
        pointer_x, pointer_y);
    gst_navigation_send_mouse_event (GST_NAVIGATION (ximagesink),
        "mouse-move", 0, pointer_x, pointer_y);

    g_mutex_lock (&ximagesink->flow_lock);
    g_mutex_lock (&ximagesink->x_lock);
  }

  /* We get all remaining events on our window to throw them upstream */
  while (XCheckWindowEvent (ximagesink->xcontext->disp,
          ximagesink->xwindow->win,
          KeyPressMask | KeyReleaseMask |
          ButtonPressMask | ButtonReleaseMask, &e)) {
    KeySym keysym;
    const char *key_str = NULL;

    /* We lock only for the X function call */
    g_mutex_unlock (&ximagesink->x_lock);
    g_mutex_unlock (&ximagesink->flow_lock);

    switch (e.type) {
      case ButtonPress:
        /* Mouse button pressed/released over our window. We send upstream
           events for interactivity/navigation */
        GST_DEBUG ("ximagesink button %d pressed over window at %d,%d",
            e.xbutton.button, e.xbutton.x, e.xbutton.x);
        gst_navigation_send_mouse_event (GST_NAVIGATION (ximagesink),
            "mouse-button-press", e.xbutton.button, e.xbutton.x, e.xbutton.y);
        break;
      case ButtonRelease:
        GST_DEBUG ("ximagesink button %d release over window at %d,%d",
            e.xbutton.button, e.xbutton.x, e.xbutton.x);
        gst_navigation_send_mouse_event (GST_NAVIGATION (ximagesink),
            "mouse-button-release", e.xbutton.button, e.xbutton.x, e.xbutton.y);
        break;
      case KeyPress:
      case KeyRelease:
        /* Key pressed/released over our window. We send upstream
           events for interactivity/navigation */
        g_mutex_lock (&ximagesink->x_lock);
        keysym = XkbKeycodeToKeysym (ximagesink->xcontext->disp,
            e.xkey.keycode, 0, 0);
        if (keysym != NoSymbol) {
          key_str = XKeysymToString (keysym);
        } else {
          key_str = "unknown";
        }
        g_mutex_unlock (&ximagesink->x_lock);
        GST_DEBUG_OBJECT (ximagesink,
            "key %d pressed over window at %d,%d (%s)",
            e.xkey.keycode, e.xkey.x, e.xkey.y, key_str);
        gst_navigation_send_key_event (GST_NAVIGATION (ximagesink),
            e.type == KeyPress ? "key-press" : "key-release", key_str);
        break;
      default:
        GST_DEBUG_OBJECT (ximagesink, "ximagesink unhandled X event (%d)",
            e.type);
    }
    g_mutex_lock (&ximagesink->flow_lock);
    g_mutex_lock (&ximagesink->x_lock);
  }

  /* Handle Expose */
  while (XCheckWindowEvent (ximagesink->xcontext->disp,
          ximagesink->xwindow->win, ExposureMask | StructureNotifyMask, &e)) {
    switch (e.type) {
      case Expose:
        exposed = TRUE;
        break;
      case ConfigureNotify:
        g_mutex_unlock (&ximagesink->x_lock);
        gst_x_image_sink_xwindow_update_geometry (ximagesink);
        g_mutex_lock (&ximagesink->x_lock);
        configured = TRUE;
        break;
      default:
        break;
    }

  }

  if (ximagesink->handle_expose && (exposed || configured)) {
    g_mutex_unlock (&ximagesink->x_lock);
    g_mutex_unlock (&ximagesink->flow_lock);

    gst_x_image_sink_expose (GST_VIDEO_OVERLAY (ximagesink));

    g_mutex_lock (&ximagesink->flow_lock);
    g_mutex_lock (&ximagesink->x_lock);
  }

  /* Handle Display events */
  while (XPending (ximagesink->xcontext->disp)) {
    XNextEvent (ximagesink->xcontext->disp, &e);

    switch (e.type) {
      case ClientMessage:{
        Atom wm_delete;

        wm_delete = XInternAtom (ximagesink->xcontext->disp,
            "WM_DELETE_WINDOW", False);
        if (wm_delete == (Atom) e.xclient.data.l[0]) {
          /* Handle window deletion by posting an error on the bus */
          GST_ELEMENT_ERROR (ximagesink, RESOURCE, NOT_FOUND,
              ("Output window was closed"), (NULL));

          g_mutex_unlock (&ximagesink->x_lock);
          gst_x_image_sink_xwindow_destroy (ximagesink, ximagesink->xwindow);
          ximagesink->xwindow = NULL;
          g_mutex_lock (&ximagesink->x_lock);
        }
        break;
      }
      default:
        break;
    }
  }

  /* Handle DRM display */
  if (ximagesink->paused
      && xwindow_get_window_position (ximagesink, NULL, NULL)) {
    /* if window stop moving, redraw display */
    g_mutex_unlock (&ximagesink->x_lock);
    g_mutex_unlock (&ximagesink->flow_lock);

    gst_x_image_sink_expose (GST_VIDEO_OVERLAY (ximagesink));
    g_mutex_lock (&ximagesink->flow_lock);
    g_mutex_lock (&ximagesink->x_lock);
  }

  g_mutex_unlock (&ximagesink->x_lock);
  g_mutex_unlock (&ximagesink->flow_lock);
}

static gpointer
gst_x_image_sink_event_thread (GstRkXImageSink * ximagesink)
{
  g_return_val_if_fail (GST_IS_X_IMAGE_SINK (ximagesink), NULL);

  GST_OBJECT_LOCK (ximagesink);
  while (ximagesink->running) {
    GST_OBJECT_UNLOCK (ximagesink);

    if (ximagesink->xwindow) {
      gst_x_image_sink_handle_xevents (ximagesink);
    }
    /* FIXME: do we want to align this with the framerate or anything else? */
    g_usleep (G_USEC_PER_SEC / 100);

    GST_OBJECT_LOCK (ximagesink);
  }
  GST_OBJECT_UNLOCK (ximagesink);

  return NULL;
}

static void
gst_x_image_sink_manage_event_thread (GstRkXImageSink * ximagesink)
{
  GThread *thread = NULL;

  /* don't start the thread too early */
  if (ximagesink->xcontext == NULL) {
    return;
  }

  GST_OBJECT_LOCK (ximagesink);
  if (ximagesink->handle_expose || ximagesink->handle_events) {
    if (!ximagesink->event_thread) {
      /* Setup our event listening thread */
      GST_DEBUG_OBJECT (ximagesink, "run xevent thread, expose %d, events %d",
          ximagesink->handle_expose, ximagesink->handle_events);
      ximagesink->running = TRUE;
      ximagesink->event_thread = g_thread_try_new ("ximagesink-events",
          (GThreadFunc) gst_x_image_sink_event_thread, ximagesink, NULL);
    }
  } else {
    if (ximagesink->event_thread) {
      GST_DEBUG_OBJECT (ximagesink, "stop xevent thread, expose %d, events %d",
          ximagesink->handle_expose, ximagesink->handle_events);
      ximagesink->running = FALSE;
      /* grab thread and mark it as NULL */
      thread = ximagesink->event_thread;
      ximagesink->event_thread = NULL;
    }
  }
  GST_OBJECT_UNLOCK (ximagesink);

  /* Wait for our event thread to finish */
  if (thread)
    g_thread_join (thread);

}

/* This function gets the X Display and global info about it. Everything is
   stored in our object and will be cleaned when the object is disposed. Note
   here that caps for supported format are generated without any window or
   image creation */
static GstXContext *
gst_x_image_sink_xcontext_get (GstRkXImageSink * ximagesink)
{
  GstXContext *xcontext = NULL;
  XPixmapFormatValues *px_formats = NULL;
  gint nb_formats = 0, i;
  gint endianness;
  GstVideoFormat vformat;
  guint32 alpha_mask;

  g_return_val_if_fail (GST_IS_X_IMAGE_SINK (ximagesink), NULL);

  xcontext = g_new0 (GstXContext, 1);

  g_mutex_lock (&ximagesink->x_lock);

  xcontext->disp = XOpenDisplay (ximagesink->display_name);

  if (!xcontext->disp) {
    g_mutex_unlock (&ximagesink->x_lock);
    g_free (xcontext);
    GST_ELEMENT_ERROR (ximagesink, RESOURCE, WRITE,
        ("Could not initialise X output"), ("Could not open display"));
    return NULL;
  }

  xcontext->screen = DefaultScreenOfDisplay (xcontext->disp);
  xcontext->screen_num = DefaultScreen (xcontext->disp);
  xcontext->visual = DefaultVisual (xcontext->disp, xcontext->screen_num);
  xcontext->root = DefaultRootWindow (xcontext->disp);
  xcontext->white = XWhitePixel (xcontext->disp, xcontext->screen_num);
  xcontext->black = XBlackPixel (xcontext->disp, xcontext->screen_num);
  xcontext->depth = DefaultDepthOfScreen (xcontext->screen);

  xcontext->width = DisplayWidth (xcontext->disp, xcontext->screen_num);
  xcontext->height = DisplayHeight (xcontext->disp, xcontext->screen_num);
  xcontext->widthmm = DisplayWidthMM (xcontext->disp, xcontext->screen_num);
  xcontext->heightmm = DisplayHeightMM (xcontext->disp, xcontext->screen_num);

  GST_DEBUG_OBJECT (ximagesink, "X reports %dx%d pixels and %d mm x %d mm",
      xcontext->width, xcontext->height, xcontext->widthmm, xcontext->heightmm);

  /* We get supported pixmap formats at supported depth */
  px_formats = XListPixmapFormats (xcontext->disp, &nb_formats);

  if (!px_formats) {
    XCloseDisplay (xcontext->disp);
    g_mutex_unlock (&ximagesink->x_lock);
    g_free (xcontext);
    GST_ELEMENT_ERROR (ximagesink, RESOURCE, SETTINGS,
        ("Could not get supported pixmap formats"), (NULL));
    return NULL;
  }

  /* We get bpp value corresponding to our running depth */
  for (i = 0; i < nb_formats; i++) {
    if (px_formats[i].depth == xcontext->depth)
      xcontext->bpp = px_formats[i].bits_per_pixel;
  }

  XFree (px_formats);

  endianness = (ImageByteOrder (xcontext->disp) ==
      LSBFirst) ? G_LITTLE_ENDIAN : G_BIG_ENDIAN;

  /* extrapolate alpha mask */
  if (xcontext->depth == 32) {
    alpha_mask = ~(xcontext->visual->red_mask
        | xcontext->visual->green_mask | xcontext->visual->blue_mask);
  } else {
    alpha_mask = 0;
  }

  vformat =
      gst_video_format_from_masks (xcontext->depth, xcontext->bpp, endianness,
      xcontext->visual->red_mask, xcontext->visual->green_mask,
      xcontext->visual->blue_mask, alpha_mask);

  if (vformat == GST_VIDEO_FORMAT_UNKNOWN)
    goto unknown_format;

  g_mutex_unlock (&ximagesink->x_lock);

  return xcontext;

  /* ERRORS */
unknown_format:
  {
    GST_ERROR_OBJECT (ximagesink, "unknown format");
    return NULL;
  }
}

/* This function cleans the X context. Closing the Display and unrefing the
   caps for supported formats. */
static void
gst_x_image_sink_xcontext_clear (GstRkXImageSink * ximagesink)
{
  GstXContext *xcontext;

  g_return_if_fail (GST_IS_X_IMAGE_SINK (ximagesink));

  GST_OBJECT_LOCK (ximagesink);
  if (ximagesink->xcontext == NULL) {
    GST_OBJECT_UNLOCK (ximagesink);
    return;
  }

  /* Take the xcontext reference and NULL it while we
   * clean it up, so that any buffer-alloced buffers
   * arriving after this will be freed correctly */
  xcontext = ximagesink->xcontext;
  ximagesink->xcontext = NULL;

  GST_OBJECT_UNLOCK (ximagesink);

  g_mutex_lock (&ximagesink->x_lock);

  XCloseDisplay (xcontext->disp);

  g_mutex_unlock (&ximagesink->x_lock);

  g_free (xcontext);
}

/* Element stuff */

static GstCaps *
gst_x_image_sink_get_allowed_caps (GstRkXImageSink * self)
{
  if (!self->allowed_caps)
    return NULL;                /* base class will return the template caps */
  return gst_caps_ref (self->allowed_caps);
}

static GstCaps *
gst_x_image_sink_getcaps (GstBaseSink * bsink, GstCaps * filter)
{
  GstRkXImageSink *ximagesink;
  GstCaps *caps, *out_caps;

  ximagesink = GST_X_IMAGE_SINK (bsink);

  caps = gst_x_image_sink_get_allowed_caps (ximagesink);
  if (caps && filter) {
    out_caps = gst_caps_intersect_full (caps, filter, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
  } else {
    out_caps = caps;
  }

  return out_caps;
}

static gboolean
gst_x_image_sink_setcaps (GstBaseSink * bsink, GstCaps * caps)
{
  GstRkXImageSink *ximagesink;
  GstVideoInfo info;
  GstBufferPool *newpool, *oldpool;

  ximagesink = GST_X_IMAGE_SINK (bsink);

  if (!ximagesink->xcontext)
    return FALSE;

  /* We are going to change the internal buffer pool, which means it will no
   * longer be compatbile with the last_buffer size. Drain now, as we won't be
   * able to do that later on. */
  gst_kms_sink_drain (ximagesink);

  GST_DEBUG_OBJECT (ximagesink, "given caps %" GST_PTR_FORMAT, caps);

  if (!gst_video_info_from_caps (&info, caps))
    goto invalid_format;

  GST_VIDEO_SINK_WIDTH (ximagesink) = info.width;
  GST_VIDEO_SINK_HEIGHT (ximagesink) = info.height;
  ximagesink->fps_n = info.fps_n;
  ximagesink->fps_d = info.fps_d;
  ximagesink->par_n = info.par_n;
  ximagesink->par_d = info.par_d;

  /* Notify application to set xwindow id now */
  g_mutex_lock (&ximagesink->flow_lock);
  if (!ximagesink->xwindow) {
    g_mutex_unlock (&ximagesink->flow_lock);
    gst_video_overlay_prepare_window_handle (GST_VIDEO_OVERLAY (ximagesink));
  } else {
    g_mutex_unlock (&ximagesink->flow_lock);
  }

  /* Creating our window and our image */
  if (GST_VIDEO_SINK_WIDTH (ximagesink) <= 0 ||
      GST_VIDEO_SINK_HEIGHT (ximagesink) <= 0)
    goto invalid_size;

  /* create a new pool for the new configuration */
  newpool = gst_kms_sink_create_pool (ximagesink, caps,
      GST_VIDEO_INFO_SIZE (&info), 2);
  if (!newpool)
    goto no_pool;

  /* we don't activate the internal pool yet as it may not be needed */
  oldpool = ximagesink->pool;
  ximagesink->pool = newpool;

  if (oldpool) {
    gst_buffer_pool_set_active (oldpool, FALSE);
    gst_object_unref (oldpool);
  }

  g_mutex_lock (&ximagesink->flow_lock);
  if (!ximagesink->xwindow) {
    ximagesink->xwindow = gst_x_image_sink_xwindow_new (ximagesink,
        GST_VIDEO_SINK_WIDTH (ximagesink), GST_VIDEO_SINK_HEIGHT (ximagesink));
  }

  ximagesink->vinfo = info;

  /* Remember to draw borders for next frame */
  ximagesink->draw_border = TRUE;

  g_mutex_unlock (&ximagesink->flow_lock);

  return TRUE;

  /* ERRORS */
invalid_format:
  {
    GST_ERROR_OBJECT (ximagesink, "caps invalid");
    return FALSE;
  }
invalid_size:
  {
    GST_ELEMENT_ERROR (ximagesink, CORE, NEGOTIATION, (NULL),
        ("Invalid image size."));
    return FALSE;
  }
no_pool:
  {
    /* Already warned in create_pool */
    return FALSE;
  }
}

static GstStateChangeReturn
gst_x_image_sink_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstRkXImageSink *ximagesink;
  GstXContext *xcontext = NULL;

  ximagesink = GST_X_IMAGE_SINK (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      /* Initializing the XContext */
      if (ximagesink->xcontext == NULL) {
        xcontext = gst_x_image_sink_xcontext_get (ximagesink);
        if (xcontext == NULL) {
          ret = GST_STATE_CHANGE_FAILURE;
          goto beach;
        }
        GST_OBJECT_LOCK (ximagesink);
        if (xcontext)
          ximagesink->xcontext = xcontext;
        GST_OBJECT_UNLOCK (ximagesink);
      }

      /* call XSynchronize with the current value of synchronous */
      GST_DEBUG_OBJECT (ximagesink, "XSynchronize called with %s",
          ximagesink->synchronous ? "TRUE" : "FALSE");
      g_mutex_lock (&ximagesink->x_lock);
      XSynchronize (ximagesink->xcontext->disp, ximagesink->synchronous);
      g_mutex_unlock (&ximagesink->x_lock);
      gst_x_image_sink_manage_event_thread (ximagesink);
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      ximagesink->paused = TRUE;
      g_mutex_lock (&ximagesink->flow_lock);
      if (ximagesink->xwindow)
        gst_x_image_sink_xwindow_clear (ximagesink, ximagesink->xwindow);
      g_mutex_unlock (&ximagesink->flow_lock);
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      ximagesink->paused = FALSE;
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      ximagesink->paused = TRUE;
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      ximagesink->paused = FALSE;
      ximagesink->fps_n = 0;
      ximagesink->fps_d = 1;
      GST_VIDEO_SINK_WIDTH (ximagesink) = 0;
      GST_VIDEO_SINK_HEIGHT (ximagesink) = 0;
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_x_image_sink_reset (ximagesink);
      break;
    default:
      break;
  }

beach:
  return ret;
}

static void
gst_x_image_sink_get_times (GstBaseSink * bsink, GstBuffer * buf,
    GstClockTime * start, GstClockTime * end)
{
  GstRkXImageSink *ximagesink;

  ximagesink = GST_X_IMAGE_SINK (bsink);

  if (GST_BUFFER_TIMESTAMP_IS_VALID (buf)) {
    *start = GST_BUFFER_TIMESTAMP (buf);
    if (GST_BUFFER_DURATION_IS_VALID (buf)) {
      *end = *start + GST_BUFFER_DURATION (buf);
    } else {
      if (ximagesink->fps_n > 0) {
        *end = *start +
            gst_util_uint64_scale_int (GST_SECOND, ximagesink->fps_d,
            ximagesink->fps_n);
      }
    }
  }
}

static GstFlowReturn
gst_x_image_sink_show_frame (GstVideoSink * vsink, GstBuffer * buf)
{
  GstFlowReturn res = GST_FLOW_OK;
  GstRkXImageSink *ximagesink;

  ximagesink = GST_X_IMAGE_SINK (vsink);

  if (!gst_x_image_sink_ximage_put (ximagesink, buf)) {
    /* No Window available to put our image into */
    GST_WARNING_OBJECT (ximagesink, "could not output image - no window");
    res = GST_FLOW_ERROR;
  }

  return res;
}

static gboolean
gst_x_image_sink_event (GstBaseSink * sink, GstEvent * event)
{
  GstRkXImageSink *ximagesink = GST_X_IMAGE_SINK (sink);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_TAG:{
      GstTagList *l;
      gchar *title = NULL;

      gst_event_parse_tag (event, &l);
      gst_tag_list_get_string (l, GST_TAG_TITLE, &title);

      if (title) {
        GST_DEBUG_OBJECT (ximagesink, "got tags, title='%s'", title);
        gst_x_image_sink_xwindow_set_title (ximagesink, ximagesink->xwindow,
            title);

        g_free (title);
      }
      break;
    }
    default:
      break;
  }

  return GST_BASE_SINK_CLASS (parent_class)->event (sink, event);
}

/* Interfaces stuff */
static void
gst_x_image_sink_navigation_send_event (GstNavigation * navigation,
    GstStructure * structure)
{
  GstRkXImageSink *ximagesink = GST_X_IMAGE_SINK (navigation);
  GstEvent *event = NULL;
  gint x_offset, y_offset;
  gdouble x, y;
  gboolean handled = FALSE;

  /* We are not converting the pointer coordinates as there's no hardware
     scaling done here. The only possible scaling is done by videoscale and
     videoscale will have to catch those events and tranform the coordinates
     to match the applied scaling. So here we just add the offset if the image
     is centered in the window.  */

  /* We take the flow_lock while we look at the window */
  g_mutex_lock (&ximagesink->flow_lock);

  if (!ximagesink->xwindow) {
    g_mutex_unlock (&ximagesink->flow_lock);
    gst_structure_free (structure);
    return;
  }

  x_offset = ximagesink->xwindow->width - GST_VIDEO_SINK_WIDTH (ximagesink);
  y_offset = ximagesink->xwindow->height - GST_VIDEO_SINK_HEIGHT (ximagesink);

  g_mutex_unlock (&ximagesink->flow_lock);

  if (x_offset > 0 && gst_structure_get_double (structure, "pointer_x", &x)) {
    x -= x_offset / 2;
    gst_structure_set (structure, "pointer_x", G_TYPE_DOUBLE, x, NULL);
  }
  if (y_offset > 0 && gst_structure_get_double (structure, "pointer_y", &y)) {
    y -= y_offset / 2;
    gst_structure_set (structure, "pointer_y", G_TYPE_DOUBLE, y, NULL);
  }

  event = gst_event_new_navigation (structure);
  if (event) {
    gst_event_ref (event);
    handled = gst_pad_push_event (GST_VIDEO_SINK_PAD (ximagesink), event);

    if (!handled)
      gst_element_post_message (GST_ELEMENT_CAST (ximagesink),
          gst_navigation_message_new_event (GST_OBJECT_CAST (ximagesink),
              event));

    gst_event_unref (event);
  }
}

static void
gst_x_image_sink_navigation_init (GstNavigationInterface * iface)
{
  iface->send_event = gst_x_image_sink_navigation_send_event;
}

static void
gst_x_image_sink_set_window_handle (GstVideoOverlay * overlay, guintptr id)
{
  XID xwindow_id = id;
  GstRkXImageSink *ximagesink = GST_X_IMAGE_SINK (overlay);
  GstXWindow *xwindow = NULL;

  /* We acquire the stream lock while setting this window in the element.
     We are basically cleaning tons of stuff replacing the old window, putting
     images while we do that would surely crash */
  g_mutex_lock (&ximagesink->flow_lock);

  /* If we already use that window return */
  if (ximagesink->xwindow && (xwindow_id == ximagesink->xwindow->win)) {
    g_mutex_unlock (&ximagesink->flow_lock);
    return;
  }

  /* If the element has not initialized the X11 context try to do so */
  if (!ximagesink->xcontext &&
      !(ximagesink->xcontext = gst_x_image_sink_xcontext_get (ximagesink))) {
    g_mutex_unlock (&ximagesink->flow_lock);
    /* we have thrown a GST_ELEMENT_ERROR now */
    return;
  }

  /* If a window is there already we destroy it */
  if (ximagesink->xwindow) {
    gst_x_image_sink_xwindow_destroy (ximagesink, ximagesink->xwindow);
    ximagesink->xwindow = NULL;
  }

  /* If the xid is 0 we go back to an internal window */
  if (xwindow_id == 0) {
    /* If no width/height caps nego did not happen window will be created
       during caps nego then */
    if (GST_VIDEO_SINK_WIDTH (ximagesink) && GST_VIDEO_SINK_HEIGHT (ximagesink)) {
      xwindow = gst_x_image_sink_xwindow_new (ximagesink,
          GST_VIDEO_SINK_WIDTH (ximagesink),
          GST_VIDEO_SINK_HEIGHT (ximagesink));
    }
  } else {
    xwindow = g_new0 (GstXWindow, 1);

    xwindow->win = xwindow_id;

    /* We set the events we want to receive and create a GC. */
    g_mutex_lock (&ximagesink->x_lock);
    xwindow->internal = FALSE;
    if (ximagesink->handle_events) {
      XSelectInput (ximagesink->xcontext->disp, xwindow->win, ExposureMask |
          StructureNotifyMask | PointerMotionMask | KeyPressMask |
          KeyReleaseMask);
    }

    xwindow->gc = XCreateGC (ximagesink->xcontext->disp, xwindow->win, 0, NULL);
    g_mutex_unlock (&ximagesink->x_lock);
  }

  if (xwindow) {
    ximagesink->xwindow = xwindow;
    /* Update the window geometry, possibly generating a reconfigure event. */
    gst_x_image_sink_xwindow_update_geometry (ximagesink);
  }

  g_mutex_unlock (&ximagesink->flow_lock);
}

static void
gst_x_image_sink_expose (GstVideoOverlay * overlay)
{
  GstRkXImageSink *ximagesink = GST_X_IMAGE_SINK (overlay);

  gst_x_image_sink_xwindow_update_geometry (ximagesink);
  gst_x_image_sink_ximage_put (ximagesink, NULL);
}

static void
gst_x_image_sink_set_event_handling (GstVideoOverlay * overlay,
    gboolean handle_events)
{
  GstRkXImageSink *ximagesink = GST_X_IMAGE_SINK (overlay);

  ximagesink->handle_events = handle_events;

  g_mutex_lock (&ximagesink->flow_lock);

  if (G_UNLIKELY (!ximagesink->xwindow)) {
    g_mutex_unlock (&ximagesink->flow_lock);
    return;
  }

  g_mutex_lock (&ximagesink->x_lock);

  if (handle_events) {
    if (ximagesink->xwindow->internal) {
      XSelectInput (ximagesink->xcontext->disp, ximagesink->xwindow->win,
          ExposureMask | StructureNotifyMask | PointerMotionMask |
          KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
    } else {
      XSelectInput (ximagesink->xcontext->disp, ximagesink->xwindow->win,
          ExposureMask | StructureNotifyMask | PointerMotionMask |
          KeyPressMask | KeyReleaseMask);
    }
  } else {
    XSelectInput (ximagesink->xcontext->disp, ximagesink->xwindow->win, 0);
  }

  g_mutex_unlock (&ximagesink->x_lock);

  g_mutex_unlock (&ximagesink->flow_lock);
}

static void
gst_x_image_sink_set_render_rectangle (GstVideoOverlay * overlay,
    gint x, gint y, gint width, gint height)
{
  GstRkXImageSink *ximagesink = GST_X_IMAGE_SINK (overlay);
  GST_DEBUG_OBJECT (ximagesink, "Set Render Rectangle"
      "x %d y %d width %d height %d", x, y, width, height);

  ximagesink->save_rect.w = width;
  ximagesink->save_rect.h = height;
  ximagesink->save_rect.x = x;
  ximagesink->save_rect.y = y;

  gst_x_image_sink_expose (GST_VIDEO_OVERLAY (ximagesink));
}

static void
gst_x_image_sink_video_overlay_init (GstVideoOverlayInterface * iface)
{
  iface->set_window_handle = gst_x_image_sink_set_window_handle;
  iface->expose = gst_x_image_sink_expose;
  iface->set_render_rectangle = gst_x_image_sink_set_render_rectangle;
  iface->handle_events = gst_x_image_sink_set_event_handling;
}

/* =========================================== */
/*                                             */
/*              Init & Class init              */
/*                                             */
/* =========================================== */

static void
gst_x_image_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstRkXImageSink *ximagesink;

  g_return_if_fail (GST_IS_X_IMAGE_SINK (object));

  ximagesink = GST_X_IMAGE_SINK (object);

  switch (prop_id) {
    case PROP_DISPLAY:
      ximagesink->display_name = g_strdup (g_value_get_string (value));
      break;
    case PROP_SYNCHRONOUS:
      ximagesink->synchronous = g_value_get_boolean (value);
      if (ximagesink->xcontext) {
        GST_DEBUG_OBJECT (ximagesink, "XSynchronize called with %s",
            ximagesink->synchronous ? "TRUE" : "FALSE");
        g_mutex_lock (&ximagesink->x_lock);
        XSynchronize (ximagesink->xcontext->disp, ximagesink->synchronous);
        g_mutex_unlock (&ximagesink->x_lock);
      }
      break;
    case PROP_HANDLE_EVENTS:
      gst_x_image_sink_set_event_handling (GST_VIDEO_OVERLAY (ximagesink),
          g_value_get_boolean (value));
      gst_x_image_sink_manage_event_thread (ximagesink);
      break;
    case PROP_HANDLE_EXPOSE:
      ximagesink->handle_expose = g_value_get_boolean (value);
      gst_x_image_sink_manage_event_thread (ximagesink);
      break;
    case PROP_DRIVER_NAME:
      ximagesink->devname = g_value_dup_string (value);
      break;
    case PROP_CONNECTOR_ID:
      ximagesink->conn_id = g_value_get_int (value);
      break;
    case PROP_PLANE_ID:
      ximagesink->plane_id = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_x_image_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstRkXImageSink *ximagesink;

  g_return_if_fail (GST_IS_X_IMAGE_SINK (object));

  ximagesink = GST_X_IMAGE_SINK (object);

  switch (prop_id) {
    case PROP_DISPLAY:
      g_value_set_string (value, ximagesink->display_name);
      break;
    case PROP_SYNCHRONOUS:
      g_value_set_boolean (value, ximagesink->synchronous);
      break;
    case PROP_HANDLE_EVENTS:
      g_value_set_boolean (value, ximagesink->handle_events);
      break;
    case PROP_HANDLE_EXPOSE:
      g_value_set_boolean (value, ximagesink->handle_expose);
      break;
    case PROP_WINDOW_WIDTH:
      if (ximagesink->xwindow)
        g_value_set_uint64 (value, ximagesink->xwindow->width);
      else
        g_value_set_uint64 (value, 0);
      break;
    case PROP_WINDOW_HEIGHT:
      if (ximagesink->xwindow)
        g_value_set_uint64 (value, ximagesink->xwindow->height);
      else
        g_value_set_uint64 (value, 0);
      break;
    case PROP_DRIVER_NAME:
      g_value_take_string (value, ximagesink->devname);
      break;
    case PROP_CONNECTOR_ID:
      g_value_set_int (value, ximagesink->conn_id);
      break;
    case PROP_PLANE_ID:
      g_value_set_int (value, ximagesink->plane_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_x_image_sink_reset (GstRkXImageSink * ximagesink)
{
  GThread *thread;

  GST_OBJECT_LOCK (ximagesink);
  ximagesink->running = FALSE;
  /* grab thread and mark it as NULL */
  thread = ximagesink->event_thread;
  ximagesink->event_thread = NULL;
  GST_OBJECT_UNLOCK (ximagesink);

  /* Wait for our event thread to finish before we clean up our stuff. */
  if (thread)
    g_thread_join (thread);

  gst_buffer_replace (&ximagesink->last_buffer, NULL);

  g_mutex_lock (&ximagesink->flow_lock);

  if (ximagesink->xwindow) {
    gst_x_image_sink_xwindow_clear (ximagesink, ximagesink->xwindow);
    gst_x_image_sink_xwindow_destroy (ximagesink, ximagesink->xwindow);
    ximagesink->xwindow = NULL;
  }
  g_mutex_unlock (&ximagesink->flow_lock);

  gst_x_image_sink_xcontext_clear (ximagesink);
}

static void
gst_x_image_sink_finalize (GObject * object)
{
  GstRkXImageSink *ximagesink;

  ximagesink = GST_X_IMAGE_SINK (object);

  gst_x_image_sink_reset (ximagesink);

  if (ximagesink->display_name) {
    g_free (ximagesink->display_name);
    ximagesink->display_name = NULL;
  }
  g_mutex_clear (&ximagesink->x_lock);
  g_mutex_clear (&ximagesink->flow_lock);

  g_free (ximagesink->media_title);

  gst_poll_free (ximagesink->poll);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_x_image_sink_init (GstRkXImageSink * ximagesink)
{
  ximagesink->display_name = NULL;
  ximagesink->xcontext = NULL;
  ximagesink->xwindow = NULL;
  ximagesink->last_buffer = NULL;

  ximagesink->event_thread = NULL;
  ximagesink->running = FALSE;

  ximagesink->fps_n = 0;
  ximagesink->fps_d = 1;

  g_mutex_init (&ximagesink->x_lock);
  g_mutex_init (&ximagesink->flow_lock);

  ximagesink->synchronous = FALSE;
  ximagesink->handle_events = TRUE;
  ximagesink->handle_expose = TRUE;

  ximagesink->fd = -1;
  ximagesink->conn_id = -1;
  ximagesink->plane_id = -1;
  gst_poll_fd_init (&ximagesink->pollfd);
  ximagesink->poll = gst_poll_new (TRUE);
  gst_video_info_init (&ximagesink->vinfo);

  ximagesink->save_rect.x = 0;
  ximagesink->save_rect.y = 0;
  ximagesink->save_rect.w = 0;
  ximagesink->save_rect.h = 0;

  ximagesink->paused = FALSE;
}

static gboolean
gst_x_image_sink_start (GstBaseSink * bsink)
{
  GstRkXImageSink *self;
  drmModeRes *res;
  drmModeConnector *conn;
  drmModeCrtc *crtc;
  drmModePlaneRes *pres;
  drmModePlane *plane;
  gboolean ret;

  self = GST_X_IMAGE_SINK (bsink);
  ret = FALSE;
  res = NULL;
  conn = NULL;
  crtc = NULL;
  pres = NULL;
  plane = NULL;

  self->fd = open ("/dev/dri/card0", O_RDWR);

  if (self->fd < 0)
    goto open_failed;

  drm_log_version (self);
  if (!drm_get_caps (self))
    goto bail;

  res = drmModeGetResources (self->fd);
  if (!res)
    goto resources_failed;

  if (self->conn_id == -1)
    conn = drm_find_main_monitor (self->fd, res);
  else
    conn = drmModeGetConnector (self->fd, self->conn_id);
  if (!conn)
    goto connector_failed;

  crtc = drm_find_crtc_for_connector (self->fd, res, conn, &self->pipe);
  if (!crtc)
    goto crtc_failed;

  if (drmSetClientCap (self->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1))
    goto set_cap_failed;

  pres = drmModeGetPlaneResources (self->fd);
  if (!pres)
    goto plane_resources_failed;

  if (self->plane_id == -1)
    plane = drm_find_plane_for_crtc_by_type (self->fd, res, pres,
        crtc->crtc_id, DRM_PLANE_TYPE_OVERLAY);
  else {
    plane = drmModeGetPlane (self->fd, self->plane_id);
    if (drm_plane_get_type (self->fd, plane) == DRM_PLANE_TYPE_PRIMARY) {
      GST_ERROR_OBJECT (self, "Not support using primary plane");
      goto bail;
    }
  }
  if (!plane)
    goto plane_failed;

  /* let's get the available color formats in plane */
  if (!drm_ensure_allowed_caps (self, plane, res))
    goto bail;

  self->conn_id = conn->connector_id;
  self->crtc_id = crtc->crtc_id;
  self->plane_id = plane->plane_id;

  GST_INFO_OBJECT (self, "connector id = %d / crtc id = %d / plane id = %d",
      self->conn_id, self->crtc_id, self->plane_id);

  if (g_getenv ("GST_RKXIMAGE_USE_COLORKEY"))
    drm_prepare_planes (self, res, pres);

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
    GST_ERROR_OBJECT (self, "Could not find a plane for crtc");
    goto bail;
  }
}

static gboolean
gst_x_image_sink_stop (GstBaseSink * bsink)
{
  GstRkXImageSink *self;

  self = GST_X_IMAGE_SINK (bsink);

  if (!EMPTY_RECT (self->clip_rect))
    gst_x_image_sink_xwindow_fill_key (self, self->xwindow, 0);

  gst_buffer_replace (&self->last_buffer, NULL);
  gst_caps_replace (&self->allowed_caps, NULL);
  gst_object_replace ((GstObject **) & self->pool, NULL);
  gst_object_replace ((GstObject **) & self->allocator, NULL);

  gst_poll_remove_fd (self->poll, &self->pollfd);
  gst_poll_restart (self->poll);
  gst_poll_fd_init (&self->pollfd);

  if (self->fd >= 0) {
    close (self->fd);
    self->fd = -1;
  }

  return TRUE;
}


static void
gst_x_image_sink_class_init (GstRkXImageSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  GstVideoSinkClass *videosink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;
  videosink_class = (GstVideoSinkClass *) klass;

  gobject_class->finalize = gst_x_image_sink_finalize;
  gobject_class->set_property = gst_x_image_sink_set_property;
  gobject_class->get_property = gst_x_image_sink_get_property;

  g_object_class_install_property (gobject_class, PROP_DISPLAY,
      g_param_spec_string ("display", "Display", "X Display name",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_SYNCHRONOUS,
      g_param_spec_boolean ("synchronous", "Synchronous",
          "When enabled, runs the X display in synchronous mode. "
          "(unrelated to A/V sync, used only for debugging)", FALSE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_HANDLE_EVENTS,
      g_param_spec_boolean ("handle-events", "Handle XEvents",
          "When enabled, XEvents will be selected and handled", TRUE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_HANDLE_EXPOSE,
      g_param_spec_boolean ("handle-expose", "Handle expose",
          "When enabled, "
          "the current frame will always be drawn in response to X Expose "
          "events", TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstRkXImageSink:window-width
   *
   * Actual width of the video window.
   */
  g_object_class_install_property (gobject_class, PROP_WINDOW_WIDTH,
      g_param_spec_uint64 ("window-width", "window-width",
          "Width of the window", 0, G_MAXUINT64, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * GstRkXImageSink:window-height
   *
   * Actual height of the video window.
   */
  g_object_class_install_property (gobject_class, PROP_WINDOW_HEIGHT,
      g_param_spec_uint64 ("window-height", "window-height",
          "Height of the window", 0, G_MAXUINT64, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * kmssink:driver-name:
   *
   * If you have a system with multiple GPUs, you can choose which GPU
   * to use setting the DRM device driver name. Otherwise, the first
   * one from an internal list is used.
   */
  g_object_class_install_property (gobject_class, PROP_DRIVER_NAME,
      g_param_spec_string ("driver-name", "device name",
          "DRM device driver name", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));

  /**
   * kmssink:connector-id:
   *
   * A GPU has several output connectors, for example: LVDS, VGA,
   * HDMI, etc. By default the first LVDS is tried, then the first
   * eDP, and at the end, the first connected one.
   */
  g_object_class_install_property (gobject_class, PROP_CONNECTOR_ID,
      g_param_spec_int ("connector-id", "Connector ID", "DRM connector id", -1,
          G_MAXINT32, -1,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));

   /**
   * kmssink:plane-id:
   *
   * There could be several planes associated with a CRTC.
   * By default the first plane that's possible to use with a given
   * CRTC is tried.
   */
  g_object_class_install_property (gobject_class, PROP_PLANE_ID,
      g_param_spec_int ("plane-id", "Plane ID", "DRM plane id", -1, G_MAXINT32,
          -1, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT));

  gst_element_class_set_static_metadata (gstelement_class,
      "Video sink", "Sink/Video",
      "A standard X based videosink", "Julien Moutte <julien@moutte.net>");

  gst_element_class_add_static_pad_template (gstelement_class,
      &gst_x_image_sink_sink_template_factory);

  gstelement_class->change_state = gst_x_image_sink_change_state;

  gstbasesink_class->start = GST_DEBUG_FUNCPTR (gst_x_image_sink_start);
  gstbasesink_class->stop = GST_DEBUG_FUNCPTR (gst_x_image_sink_stop);
  gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_x_image_sink_getcaps);
  gstbasesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_x_image_sink_setcaps);
  gstbasesink_class->get_times = GST_DEBUG_FUNCPTR (gst_x_image_sink_get_times);
  gstbasesink_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_kms_sink_propose_allocation);
  gstbasesink_class->query = GST_DEBUG_FUNCPTR (gst_kms_sink_query);
  gstbasesink_class->event = GST_DEBUG_FUNCPTR (gst_x_image_sink_event);

  videosink_class->show_frame = GST_DEBUG_FUNCPTR (gst_x_image_sink_show_frame);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "rkximagesink",
          GST_RANK_SECONDARY, GST_TYPE_X_IMAGE_SINK))
    return FALSE;

  GST_DEBUG_CATEGORY_INIT (gst_debug_x_image_sink, "rkximagesink", 0,
      "rkximagesink element");

  return TRUE;
}


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rkximage,
    "Rockchip X/DRM Video Sink",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
