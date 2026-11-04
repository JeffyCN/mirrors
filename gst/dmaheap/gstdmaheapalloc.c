/*
 * GStreamer dmaheapalloc element
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/**
 * SECTION:element-dmaheapalloc
 * @title: dmaheapalloc
 *
 * Copies system memory video buffers into DMA-BUF heap buffers
 * (video/x-raw(memory:DMABuf)), giving dmabuf-capable consumers
 * such as mpp encoders, RGA or GL importers a zero-copy input.
 *
 * The src pad proposes the standard dma-drm caps
 * (video/x-raw(memory:DMABuf), format=DMA_DRM,
 * drm-format=<FOURCC>) first and falls back to the per-format
 * video/x-raw(memory:DMABuf) caps for downstreams that cannot parse
 * the dma-drm form.  Both forms describe the same linear layout;
 * the dma-drm drm-format strings are bare fourccs, which is how
 * DRM_FORMAT_MOD_LINEAR serializes.
 *
 * The heap can be selected with the #GstDmaHeapAlloc:heap property
 * (default "cma").
 *
 * Plane strides are padded to a multiple of 16 pixels and the vertical
 * stride to a multiple of 16 rows (the minimum pitch EGL dmabuf
 * importers accept and the layout hardware decoders publish, matching
 * what the mpp encoders require for zero-copy import); the caps keep
 * the unpadded geometry and the padded layout travels in each buffer's
 * GstVideoMeta.
 *
 * ## Example launch line
 * |[
 * gst-launch-1.0 videotestsrc ! dmaheapalloc ! mpph265enc ! fakesink
 * ]|
 *
 * Since: 1.0
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "gstdmaheapalloc.h"
#include "gstdmaheapallocator.h"
#include "gstdmaheapformat.h"
#include "gstdmabufdownload.h"

#include <gst/video/video.h>

GST_DEBUG_CATEGORY_STATIC (gst_dma_heap_alloc_debug);
#define GST_CAT_DEFAULT gst_dma_heap_alloc_debug

#define DEFAULT_HEAP "cma"
#define HEAP_TYPES "cma,system,system-uncached"

/* EGL dmabuf importers (mali and others) reject buffers whose plane pitch
 * is not a multiple of 16 pixels, while KMS scanout accepts any pitch, so
 * an unaligned dmabuf displays through kmssink but every EGL consumer
 * (weston linux-dmabuf, gst glupload DirectDmabuf) fails the import.
 * Hardware producers (mpp) already publish 16-pixel-aligned strides
 * (1080p NV12 uses pitch 1088); pad this element's allocations the same
 * way and carry the real geometry in the GstVideoMeta.
 * Formats with a sub-byte pixel size (packed 10-bit et al.) report a 0
 * pixel stride, where "16 pixels" is not expressible as a byte mask;
 * the underlying importer requirement is a 16-byte plane pitch, and the
 * RGA3 raster path additionally requires a 64-byte pitch for the packed
 * 10-bit family (librga rejects any other wstride for them), so pad
 * those planes to 64 bytes - still a multiple of the 16-byte floor,
 * so every EGL importer keeps working. */
#define STRIDE_PIXEL_ALIGN 16
#define STRIDE_SUBBYTE_ALIGN 64

enum
{
  PROP_0,
  PROP_HEAP,
};

static GstStaticPadTemplate gst_dma_heap_alloc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL)));

#define gst_dma_heap_alloc_parent_class parent_class
G_DEFINE_TYPE (GstDmaHeapAlloc, gst_dma_heap_alloc, GST_TYPE_BASE_TRANSFORM);

static void
gst_dma_heap_alloc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (object);

  switch (prop_id) {
    case PROP_HEAP:
      g_free (self->heap);
      self->heap = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_dma_heap_alloc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (object);

  switch (prop_id) {
    case PROP_HEAP:
      g_value_set_string (value, self->heap);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_dma_heap_alloc_finalize (GObject * object)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (object);

  g_free (self->heap);
  gst_clear_object (&self->allocator);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_dma_heap_alloc_start (GstBaseTransform * trans)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (trans);

  gst_clear_object (&self->allocator);
  self->allocator = gst_dma_heap_allocator_new (self->heap);
  if (!self->allocator)
    return FALSE;

  return TRUE;
}

static gboolean
gst_dma_heap_alloc_stop (GstBaseTransform * trans)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (trans);

  if (self->pool)
    gst_buffer_pool_set_active (self->pool, FALSE);
  gst_clear_object (&self->pool);
  gst_clear_caps (&self->pool_caps);
  gst_clear_object (&self->allocator);

  return TRUE;
}

/* Src pad template caps: the dma-drm variant first (preferred during
 * negotiation) followed by the legacy per-format DMABuf variant, both
 * derived from GST_VIDEO_FORMATS_ALL. */
static GstCaps *
gst_dma_heap_alloc_src_template_caps (void)
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
gst_dma_heap_alloc_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstCaps *res;
  guint i, n;

  n = gst_caps_get_size (caps);

  if (direction == GST_PAD_SINK) {
    res = gst_caps_new_empty ();

    /* The dma-drm form comes first so fixation prefers it whenever the
     * peer can intersect with it; the per-format form stays available
     * for downstreams that only understand that. */
    for (i = 0; i < n; i++)
      gst_dma_heap_caps_append_dma_drm (res, gst_caps_get_structure (caps, i));

    for (i = 0; i < n; i++)
      gst_caps_append_structure_full (res,
          gst_structure_copy (gst_caps_get_structure (caps, i)),
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));
  } else {
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
  }

  if (filter) {
    GstCaps *tmp = gst_caps_intersect_full (res, filter,
        GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (res);
    res = tmp;
  }

  return res;
}

/* The peer caps intersection keeps the peer's structure order, so move
 * the dma-drm structure to the front before fixation: the standard form
 * wins exactly when the peer accepted it, otherwise fixation falls back
 * to the per-format form. */
static GstCaps *
gst_dma_heap_alloc_fixate_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
  if (direction == GST_PAD_SINK) {
    guint i, n = gst_caps_get_size (othercaps);

    for (i = 0; i < n; i++) {
      const GstStructure *structure = gst_caps_get_structure (othercaps, i);
      const gchar *format = gst_structure_get_string (structure, "format");

      if (format && g_str_equal (format, "DMA_DRM") &&
          gst_structure_has_field (structure, "drm-format")) {
        GST_DEBUG_OBJECT (trans,
            "preferring the dma-drm caps over the per-format form");
        othercaps = gst_caps_merge (gst_caps_copy_nth (othercaps, i),
            othercaps);
        break;
      }
    }
  }

  return gst_caps_fixate (othercaps);
}

/* Fill @align so the pool pads every plane stride to a multiple of
 * STRIDE_PIXEL_ALIGN pixels and the vertical stride to a multiple of
 * STRIDE_PIXEL_ALIGN rows (the layout hardware decoders publish, e.g.
 * 1920x1080 NV12 with vstride 1088).  Without the vertical padding the
 * mpp encoders reject the dmabuf import (their expected 1088 vstride
 * does not match 1080) and fall back to a full software copy into their
 * own buffers, doubling the memory traffic of the path. */
static void
dma_heap_alloc_init_alignment (GstVideoAlignment * align,
    const GstVideoInfo * info)
{
  guint i;
  gint vstride;

  gst_video_alignment_reset (align);
  for (i = 0; i < GST_VIDEO_INFO_N_PLANES (info); i++) {
    gint pstride = GST_VIDEO_INFO_COMP_PSTRIDE (info, i);

    if (pstride > 0)
      align->stride_align[i] = STRIDE_PIXEL_ALIGN * pstride - 1;
    else
      align->stride_align[i] = STRIDE_SUBBYTE_ALIGN - 1;
  }

  vstride = GST_ROUND_UP_N (GST_VIDEO_INFO_HEIGHT (info), STRIDE_PIXEL_ALIGN);
  align->padding_bottom = vstride - GST_VIDEO_INFO_HEIGHT (info);
}

/* Parse @caps into @info the same way for both caps forms: dma-drm caps
 * carry the geometry as a drm fourcc, the plain parser would leave a zero
 * size for those. */
static gboolean
dma_heap_alloc_info_from_caps (GstCaps * caps, GstVideoInfo * info)
{
  if (gst_video_is_dma_drm_caps (caps)) {
    GstVideoInfoDmaDrm drm_info;

    if (!gst_video_info_dma_drm_from_caps (&drm_info, caps))
      return FALSE;

    *info = drm_info.vinfo;
    return TRUE;
  }

  return gst_video_info_from_caps (info, caps);
}

static gboolean
gst_dma_heap_alloc_propose_allocation (GstBaseTransform * trans,
    GstQuery * decide_query, GstQuery * query)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (trans);
  GstCaps *caps;
  GstBufferPool *pool;
  GstStructure *config;
  gsize size;
  GstVideoInfo info;
  GstVideoAlignment align;

  gst_query_parse_allocation (query, &caps, NULL);

  if (!caps)
    return FALSE;

  if (!dma_heap_alloc_info_from_caps (caps, &info))
    return FALSE;

  dma_heap_alloc_init_alignment (&align, &info);
  gst_video_info_align (&info, &align);
  size = GST_VIDEO_INFO_SIZE (&info);

  pool = gst_video_buffer_pool_new ();
  config = gst_buffer_pool_get_config (pool);
  gst_buffer_pool_config_set_params (config, caps, size, 0, 0);
  gst_buffer_pool_config_set_allocator (config, self->allocator, NULL);
  if (!gst_buffer_pool_set_config (pool, config)) {
    gst_object_unref (pool);
    return FALSE;
  }

  gst_query_add_allocation_pool (query, pool, size, 0, 0);
  gst_query_add_allocation_param (query, self->allocator, NULL);
  gst_object_unref (pool);

  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  return TRUE;
}

/* Basetransform would otherwise allocate the output buffers from the
 * first pool the downstream query returned (usually the sink's own
 * pool), so the dmabuf wrapping this element exists for would never
 * happen: the copy lands in shm or dumb buffers again.  Allocate the
 * output from our own dmaheap pool instead. */
static GstFlowReturn
gst_dma_heap_alloc_prepare_output_buffer (GstBaseTransform * trans,
    GstBuffer * input, GstBuffer ** outbuf)
{
  GstDmaHeapAlloc *self = GST_DMA_HEAP_ALLOC (trans);
  GstCaps *caps;
  GstVideoInfo info, alloc_info;
  GstVideoAlignment align;
  gsize size;

  caps = gst_pad_get_current_caps (GST_BASE_TRANSFORM_SRC_PAD (trans));
  if (!caps)
    return GST_FLOW_NOT_NEGOTIATED;

  if (!dma_heap_alloc_info_from_caps (caps, &info)) {
    gst_caps_unref (caps);
    return GST_FLOW_NOT_NEGOTIATED;
  }

  /* Pad the strides for the EGL importers, keeping the caps geometry. */
  alloc_info = info;
  dma_heap_alloc_init_alignment (&align, &alloc_info);
  gst_video_info_align (&alloc_info, &align);
  size = MAX (GST_VIDEO_INFO_SIZE (&alloc_info), gst_buffer_get_size (input));

  if (!self->pool || !gst_caps_is_equal (caps, self->pool_caps)) {
    GstBufferPool *pool = gst_video_buffer_pool_new ();
    GstStructure *config = gst_buffer_pool_get_config (pool);

    gst_buffer_pool_config_set_params (config, caps, size, 0, 0);
    gst_buffer_pool_config_set_allocator (config, self->allocator, NULL);
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

    /* Describe the padded layout (the pool knows nothing about it): the
     * real caps geometry with the aligned strides and offsets, which is
     * how hardware decoders publish their buffers. */
    gst_buffer_add_video_meta_full (*outbuf, GST_VIDEO_FRAME_FLAG_NONE,
        GST_VIDEO_INFO_FORMAT (&info),
        GST_VIDEO_INFO_WIDTH (&info), GST_VIDEO_INFO_HEIGHT (&info),
        GST_VIDEO_INFO_N_PLANES (&info), alloc_info.offset, alloc_info.stride);

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

static GstFlowReturn
gst_dma_heap_alloc_transform (GstBaseTransform * trans, GstBuffer * in,
    GstBuffer * out)
{
  GstVideoInfo in_info, out_info;
  GstVideoFrame src, dst;
  GstCaps *caps;

  /* The output strides are padded, so a contiguous memcpy would skew
   * every plane; copy through GstVideoFrame, which follows each side's
   * own layout (the output's padded GstVideoMeta overrides the caps
   * geometry during the map). */
  caps = gst_pad_get_current_caps (GST_BASE_TRANSFORM_SINK_PAD (trans));
  if (!caps || !dma_heap_alloc_info_from_caps (caps, &in_info)) {
    gst_caps_unref (caps);
    return GST_FLOW_NOT_NEGOTIATED;
  }
  gst_caps_unref (caps);

  caps = gst_pad_get_current_caps (GST_BASE_TRANSFORM_SRC_PAD (trans));
  if (!caps || !dma_heap_alloc_info_from_caps (caps, &out_info)) {
    gst_caps_unref (caps);
    return GST_FLOW_NOT_NEGOTIATED;
  }
  gst_caps_unref (caps);

  if (!gst_video_frame_map (&src, &in_info, in, GST_MAP_READ)) {
    GST_ERROR_OBJECT (trans, "failed to map input");
    return GST_FLOW_ERROR;
  }

  if (!gst_video_frame_map (&dst, &out_info, out, GST_MAP_WRITE)) {
    GST_ERROR_OBJECT (trans, "failed to map output dmabuf");
    gst_video_frame_unmap (&src);
    return GST_FLOW_ERROR;
  }

  if (!gst_video_frame_copy (&dst, &src)) {
    GST_ERROR_OBJECT (trans, "failed to copy frame into the dmabuf");
    gst_video_frame_unmap (&dst);
    gst_video_frame_unmap (&src);
    return GST_FLOW_ERROR;
  }

  gst_video_frame_unmap (&dst);
  gst_video_frame_unmap (&src);

  return GST_FLOW_OK;
}

static void
gst_dma_heap_alloc_class_init (GstDmaHeapAllocClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GstBaseTransformClass *trans_class = GST_BASE_TRANSFORM_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (gst_dma_heap_alloc_debug, "dmaheapalloc", 0,
      "dmaheapalloc");

  gobject_class->set_property = gst_dma_heap_alloc_set_property;
  gobject_class->get_property = gst_dma_heap_alloc_get_property;
  gobject_class->finalize = gst_dma_heap_alloc_finalize;

  g_object_class_install_property (gobject_class, PROP_HEAP,
      g_param_spec_string ("heap", "DMA heap",
          "DMA-BUF heap to allocate from, e.g. " HEAP_TYPES,
          DEFAULT_HEAP,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
          GST_PARAM_MUTABLE_READY));

  gst_element_class_set_static_metadata (gstelement_class,
      "DMA heap allocator", "Filter/Video",
      "Copy video buffers into DMA-BUF heap buffers", "Rockchip Electronics");

  gst_element_class_add_static_pad_template (gstelement_class,
      &gst_dma_heap_alloc_sink_template);
  gst_element_class_add_pad_template (gstelement_class,
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
          gst_dma_heap_alloc_src_template_caps ()));

  trans_class->prepare_output_buffer =
      GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_prepare_output_buffer);
  trans_class->start = GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_start);
  trans_class->stop = GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_stop);
  trans_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_transform_caps);
  trans_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_fixate_caps);
  trans_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_propose_allocation);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_dma_heap_alloc_transform);

  trans_class->passthrough_on_same_caps = FALSE;
}

static void
gst_dma_heap_alloc_init (GstDmaHeapAlloc * self)
{
  self->heap = g_strdup (DEFAULT_HEAP);
  self->allocator = NULL;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_dmabuf_download_plugin_init (plugin))
    return FALSE;

  return gst_element_register (plugin, "dmaheapalloc", GST_RANK_NONE,
      GST_TYPE_DMA_HEAP_ALLOC);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    dmaheap,
    "DMA-BUF heaps based allocator plugin",
    plugin_init, VERSION, "LGPL", GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
