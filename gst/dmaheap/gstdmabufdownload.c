/*
 * GStreamer dmabufdownload element
 * Copyright (C) 2026 Rockchip Electronics Co., Ltd.
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
 * Free Software Foundation, Inc., 51 Franklin St, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

/**
 * SECTION:element-dmabufdownload
 * @title: dmabufdownload
 *
 * Downloads DMA-BUF video buffers into system memory
 * (video/x-raw), giving CPU-side consumers such as videoconvert a
 * buffer they can process after a dmabuf producer.  The sink pad
 * accepts both dma-drm caps (video/x-raw(memory:DMABuf), format=DMA_DRM,
 * drm-format=<FOURCC>) and the per-format video/x-raw(memory:DMABuf)
 * caps; only the linear layout can be downloaded.
 *
 * The copy prefers the RGA blitter when librga is available and the
 * format and geometry are RGA-capable: the blitter reads the source
 * through its dma-buf fd, so the CPU never maps the input.  Any RGA
 * failure falls back to the plain CPU copy for the rest of the
 * stream; the "use-rga" property disables the blitter entirely.
 *
 * ## Example launch line
 * |[
 * gst-launch-1.0 videotestsrc ! dmaheapalloc ! dmabufdownload ! videoconvert ! video/x-raw,format=BGRx ! fakesink
 * ]|
 *
 * Since: 1.0
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>

#include "gstdmabufdownload.h"
#include "gstdmaheapformat.h"

#ifdef HAVE_RGA_UTIL
#include "gstrgautil.h"
#endif

#include <gst/video/video.h>
#include <gst/allocators/gstdmabuf.h>
#include <gst/video/gstvideometa.h>

GST_DEBUG_CATEGORY_STATIC (gst_dmabuf_download_debug);
#define GST_CAT_DEFAULT gst_dmabuf_download_debug

static GstStaticPadTemplate gst_dmabuf_download_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL)));

#define gst_dmabuf_download_parent_class parent_class
G_DEFINE_TYPE (GstDmaBufDownload, gst_dmabuf_download, GST_TYPE_BASE_TRANSFORM);

enum
{
  PROP_0,
  PROP_USE_RGA,
};

#define DEFAULT_USE_RGA TRUE

/* Sink pad template caps: the dma-drm variant first (matching what
 * dmabuf producers prefer to negotiate) followed by the legacy
 * per-format DMABuf variant, both derived from
 * GST_VIDEO_FORMATS_ALL. */
static GstCaps *
gst_dmabuf_download_sink_template_caps (void)
{
  GstCaps *per_format, *res;
  guint i, n;

  per_format = gst_caps_from_string (GST_VIDEO_CAPS_MAKE
      (GST_VIDEO_FORMATS_ALL));

  res = gst_caps_new_empty ();
  n = gst_caps_get_size (per_format);

  for (i = 0; i < n; i++)
    gst_dma_heap_caps_append_dma_drm (res,
        gst_caps_get_structure (per_format, i));

  for (i = 0; i < n; i++)
    gst_caps_append_structure_full (res,
        gst_structure_copy (gst_caps_get_structure (per_format, i)),
        gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

  gst_caps_unref (per_format);

  return res;
}

static GstCaps *
gst_dmabuf_download_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstCaps *res;
  guint i, n;

  (void) trans;

  n = gst_caps_get_size (caps);

  if (direction == GST_PAD_SINK) {
    /* caps on the sink pad: the src pad can produce the system
     * memory equivalent of every dmabuf structure. */
    res = gst_caps_new_empty ();

    for (i = 0; i < n; i++) {
      const GstStructure *structure = gst_caps_get_structure (caps, i);
      const gchar *format = gst_structure_get_string (structure, "format");

      if (format && g_str_equal (format, "DMA_DRM"))
        gst_dma_heap_caps_append_raw_from_dma_drm (res, structure);
      else
        gst_caps_append_structure_full (res,
            gst_structure_copy (structure),
            gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY,
                NULL));
    }
  } else {
    /* caps on the src pad: the sink pad accepts the dmabuf form of
     * every system memory structure, both variants. */
    res = gst_caps_new_empty ();

    for (i = 0; i < n; i++)
      gst_dma_heap_caps_append_dma_drm (res, gst_caps_get_structure (caps, i));

    for (i = 0; i < n; i++)
      gst_caps_append_structure_full (res,
          gst_structure_copy (gst_caps_get_structure (caps, i)),
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));
  }

  if (filter) {
    GstCaps *tmp = gst_caps_intersect_full (res, filter,
        GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (res);
    res = tmp;
  }

  return res;
}

static gboolean
gst_dmabuf_download_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstDmaBufDownload *self = GST_DMABUF_DOWNLOAD (trans);
  GstVideoInfoDmaDrm drm_info;

  /* dma-drm caps only carry the geometry as a drm fourcc; the plain
   * parser would leave a zero size.  Only the linear layout has a
   * plane layout this download can describe. */
  if (gst_video_is_dma_drm_caps (incaps)) {
    if (!gst_video_info_dma_drm_from_caps (&drm_info, incaps))
      return FALSE;
    if (drm_info.drm_modifier != DRM_FORMAT_MOD_LINEAR)
      return FALSE;
    self->in_info = drm_info.vinfo;
  } else if (!gst_video_info_from_caps (&self->in_info, incaps)) {
    return FALSE;
  }

  if (!gst_video_info_from_caps (&self->out_info, outcaps))
    return FALSE;

  /* The download is a plain plane-by-plane copy: the translated
   * format and the geometry must match on both pads. */
  if (GST_VIDEO_INFO_FORMAT (&self->in_info) !=
      GST_VIDEO_INFO_FORMAT (&self->out_info) ||
      GST_VIDEO_INFO_WIDTH (&self->in_info) !=
      GST_VIDEO_INFO_WIDTH (&self->out_info) ||
      GST_VIDEO_INFO_HEIGHT (&self->in_info) !=
      GST_VIDEO_INFO_HEIGHT (&self->out_info))
    return FALSE;

#ifdef HAVE_RGA_UTIL
  /* The RGA blitter is offered when the format sits in the shared
   * rga format table and the caps geometry passes the im2d stride
   * gate; the runtime re-checks per frame and falls back to the CPU
   * copy on the first driver-level failure. */
  self->rga_fmt_index = -1;
  self->rga_active = FALSE;
  self->rga_failed = FALSE;
  if (self->use_rga) {
    gint idx = (gint) gst_rga_format_index (
        GST_VIDEO_INFO_FORMAT (&self->in_info));

    /* Only the format membership is decided here; the stride/rect
     * alignment rules are enforced per frame by
     * gst_rga_wrap_info(), whose failure falls back to the CPU copy.
     * The producer's actual strides live in the buffer's video meta,
     * so a caps-time pitch guess would reject valid padded frames
     * (the packed 10-bit producer pads rows to 64 bytes). */
    if (idx >= 0) {
      self->rga_fmt_index = idx;
      self->rga_active = TRUE;
      GST_INFO_OBJECT (self, "download will use the RGA blitter");
    } else {
      GST_INFO_OBJECT (self, "format not RGA-capable, CPU copy");
    }
  }
#endif

  return TRUE;
}

/* Serve the output from our own system memory pool: basetransform
 * would otherwise allocate from the first pool the downstream query
 * returned (usually the sink's own pool). */
static GstFlowReturn
gst_dmabuf_download_prepare_output_buffer (GstBaseTransform * trans,
    GstBuffer * input, GstBuffer ** outbuf)
{
  GstDmaBufDownload *self = GST_DMABUF_DOWNLOAD (trans);
  GstCaps *caps;
  gsize size;

  caps = gst_pad_get_current_caps (GST_BASE_TRANSFORM_SRC_PAD (trans));
  if (!caps)
    return GST_FLOW_NOT_NEGOTIATED;

  size = GST_VIDEO_INFO_SIZE (&self->out_info);

  if (!self->pool || !gst_caps_is_equal (caps, self->pool_caps)) {
    GstBufferPool *pool = gst_video_buffer_pool_new ();
    GstStructure *config = gst_buffer_pool_get_config (pool);

    gst_buffer_pool_config_set_params (config, caps, size, 0, 0);
    gst_buffer_pool_config_add_option (config,
        GST_BUFFER_POOL_OPTION_VIDEO_META);
    if (!gst_buffer_pool_set_config (pool, config)) {
      gst_caps_unref (caps);
      gst_object_unref (pool);
      return GST_FLOW_ERROR;
    }
    if (!gst_buffer_pool_set_active (pool, TRUE)) {
      gst_caps_unref (caps);
      gst_object_unref (pool);
      return GST_FLOW_ERROR;
    }

    if (self->pool)
      gst_buffer_pool_set_active (self->pool, FALSE);
    gst_clear_object (&self->pool);
    gst_clear_caps (&self->pool_caps);

    self->pool = pool;
    self->pool_caps = caps;
  } else {
    gst_caps_unref (caps);
  }

  {
    GstBaseTransformClass *bclass = GST_BASE_TRANSFORM_GET_CLASS (trans);
    GstFlowReturn ret =
        gst_buffer_pool_acquire_buffer (self->pool, outbuf, NULL);

    if (ret != GST_FLOW_OK)
      return ret;

    /* The pool buffer carries no timestamps yet; the base class only
     * copies them from the input in its own prepare path, which we
     * replaced. */
    if (bclass->copy_metadata)
      if (!bclass->copy_metadata (trans, input, *outbuf))
        GST_ELEMENT_WARNING (trans, STREAM, NOT_IMPLEMENTED,
            ("could not copy metadata"), (NULL));

    return GST_FLOW_OK;
  }
}

#ifdef HAVE_RGA_UTIL
/* One RGA blit: the dma-buf is read by the blitter through its fd
 * (no CPU mmap of the source), the destination is the mapped
 * system-memory output.  Returns GST_FLOW_OK on success; any
 * failure makes the caller stick to the CPU copy for the stream. */
static GstFlowReturn
gst_dmabuf_download_transform_rga (GstDmaBufDownload * self,
    GstBuffer * in, GstBuffer * out)
{
  rga_buffer_t src, dst;
  im_rect srect = { 0, }, drect = { 0, };
  GstVideoInfo src_info = self->in_info;
  GstVideoInfo dst_info = self->out_info;
  GstMapInfo map = { 0, };
  gint fd = -1;
  IM_STATUS ret;

  gst_rga_info_apply_video_meta (&src_info, in);
  gst_rga_info_apply_video_meta (&dst_info, out);

  /* The blitter reads the source through its dma-buf fd, so the
   * download never mmaps (and never caches-in) the input. */
  if (gst_buffer_n_memory (in) == 1) {
    GstMemory *mem = gst_buffer_peek_memory (in, 0);

    if (gst_is_dmabuf_memory (mem)) {
      gsize offset;

      gst_memory_get_sizes (mem, &offset, NULL);
      if (!offset)
        fd = gst_dmabuf_memory_get_fd (mem);
    }
  }

  if (fd < 0)
    return GST_FLOW_NOT_SUPPORTED;

  if (!gst_buffer_map (out, &map, GST_MAP_WRITE)) {
    GST_ERROR_OBJECT (self, "failed to map output buffer");
    return GST_FLOW_ERROR;
  }

  src = gst_rga_wrap_info (self->rga_fmt_index, fd, NULL, &src_info);
  dst = gst_rga_wrap_info (self->rga_fmt_index, -1, map.data, &dst_info);

  if (!src.wstride || !dst.wstride) {
    GST_WARNING_OBJECT (self, "frame geometry not RGA-capable");
    gst_buffer_unmap (out, &map);
    return GST_FLOW_NOT_SUPPORTED;
  }

  ret = gst_rga_process (&src, &dst, &srect, &drect, 0);
  gst_buffer_unmap (out, &map);

  if (ret <= 0) {
    GST_WARNING_OBJECT (self, "RGA download failed: %s", imStrError (ret));
    return GST_FLOW_NOT_SUPPORTED;
  }

  GST_LOG_OBJECT (self, "downloaded via the RGA blitter");
  return GST_FLOW_OK;
}
#endif

static GstFlowReturn
gst_dmabuf_download_transform (GstBaseTransform * trans, GstBuffer * in,
    GstBuffer * out)
{
  GstDmaBufDownload *self = GST_DMABUF_DOWNLOAD (trans);
  GstVideoFrame src, dst;

#ifdef HAVE_RGA_UTIL
  /* Preferred path: the RGA blitter reads the dma-buf directly.
   * A driver-level failure sticks the stream to the CPU copy: the
   * failure modes (geometry the check layer rejects, a wedged
   * blitter) are per-configuration, not transient. */
  if (self->rga_active && !self->rga_failed) {
    GstFlowReturn ret = gst_dmabuf_download_transform_rga (self, in, out);

    if (ret == GST_FLOW_OK)
      return GST_FLOW_OK;
    if (ret != GST_FLOW_NOT_SUPPORTED) {
      self->rga_failed = TRUE;
      GST_WARNING_OBJECT (trans, "RGA download failed, falling back "
          "to the CPU copy for this stream");
    }
  }
#endif

  /* Mapping a dmabuf memory mmaps it, which is the actual download;
   * gst_video_frame_map honors a GstVideoMeta on the buffer, so
   * padded frames are copied from the right addresses. */
  if (!gst_video_frame_map (&src, &self->in_info, in, GST_MAP_READ)) {
    GST_ERROR_OBJECT (trans, "failed to map input dmabuf");
    return GST_FLOW_ERROR;
  }

  if (!gst_video_frame_map (&dst, &self->out_info, out, GST_MAP_WRITE)) {
    GST_ERROR_OBJECT (trans, "failed to map output buffer");
    gst_video_frame_unmap (&src);
    return GST_FLOW_ERROR;
  }

  if (!gst_video_frame_copy (&dst, &src)) {
    GST_ERROR_OBJECT (trans, "failed to copy frame");
    gst_video_frame_unmap (&dst);
    gst_video_frame_unmap (&src);
    return GST_FLOW_ERROR;
  }

  gst_video_frame_unmap (&dst);
  gst_video_frame_unmap (&src);

  return GST_FLOW_OK;
}

static gboolean
gst_dmabuf_download_stop (GstBaseTransform * trans)
{
  GstDmaBufDownload *self = GST_DMABUF_DOWNLOAD (trans);

  if (self->pool)
    gst_buffer_pool_set_active (self->pool, FALSE);
  gst_clear_object (&self->pool);
  gst_clear_caps (&self->pool_caps);

  return TRUE;
}

static void
gst_dmabuf_download_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstDmaBufDownload *self = GST_DMABUF_DOWNLOAD (object);

  switch (prop_id) {
    case PROP_USE_RGA:
      self->use_rga = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_dmabuf_download_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstDmaBufDownload *self = GST_DMABUF_DOWNLOAD (object);

  switch (prop_id) {
    case PROP_USE_RGA:
      g_value_set_boolean (value, self->use_rga);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_dmabuf_download_class_init (GstDmaBufDownloadClass * klass)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *trans_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstCaps *sink_caps;

  GST_DEBUG_CATEGORY_INIT (gst_dmabuf_download_debug, "dmabufdownload", 0,
      "dmabufdownload");

  gobject_class->set_property = gst_dmabuf_download_set_property;
  gobject_class->get_property = gst_dmabuf_download_get_property;

  g_object_class_install_property (gobject_class, PROP_USE_RGA,
      g_param_spec_boolean ("use-rga", "Use RGA",
          "Prefer the RGA blitter for the dma-buf download copy when "
          "the format and geometry are RGA-capable (CPU copy fallback)",
          DEFAULT_USE_RGA,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (gstelement_class,
      "DMA-BUF downloader", "Filter/Video",
      "Download video buffers from DMA-BUF into system memory",
      "Rockchip Electronics");

  gst_element_class_add_static_pad_template (gstelement_class,
      &gst_dmabuf_download_src_template);

  sink_caps = gst_dmabuf_download_sink_template_caps ();
  gst_element_class_add_pad_template (gstelement_class,
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS, sink_caps));

  trans_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_dmabuf_download_transform_caps);
  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_dmabuf_download_set_caps);
  trans_class->prepare_output_buffer =
      GST_DEBUG_FUNCPTR (gst_dmabuf_download_prepare_output_buffer);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_dmabuf_download_transform);
  trans_class->stop = GST_DEBUG_FUNCPTR (gst_dmabuf_download_stop);

  trans_class->passthrough_on_same_caps = FALSE;
}

static void
gst_dmabuf_download_init (GstDmaBufDownload * self)
{
  gst_video_info_init (&self->in_info);
  gst_video_info_init (&self->out_info);
  self->use_rga = DEFAULT_USE_RGA;
}

gboolean
gst_dmabuf_download_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "dmabufdownload", GST_RANK_NONE,
      GST_TYPE_DMABUF_DOWNLOAD);
}
