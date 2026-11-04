/*
 * GStreamer rgavideoconvert element
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
 * SECTION:element-rgavideoconvert
 * @title: rgavideoconvert
 *
 * Converts video frames between the formats the Rockchip RGA 2D
 * accelerator supports, offloading the color conversion from the CPU.
 *
 * The sink pad accepts system memory video/x-raw buffers and both
 * DMA-BUF variants (the per-format video/x-raw(memory:DMABuf) caps
 * and the standard dma-drm caps), so dmabuf producers such as
 * dmaheapalloc or mppvideodec can convert straight through RGA.  The
 * src pad produces system memory video/x-raw; the RGA driver is given
 * the dma fd when the input is a dmabuf and a mapped virtual address
 * otherwise.
 *
 * Only the linear dmabuf layout can be described to the driver; the
 * dma-drm caps with a non-linear modifier are rejected.
 *
 * The packed 10-bit YUV formats (NV12_10LE40, NV16_10LE40) are
 * accepted when the plane-0 byte stride is 64-byte aligned, which the
 * RGA3 raster path and librga require for the 10-bit format family.
 * Since this element allocates tight-stride system memory output
 * buffers, that alignment follows from the output caps width alone
 * (a multiple of 256 pixels); output conversions whose fixed width
 * misses it are refused at caps time rather than failing every
 * frame, except same-format links, which run in passthrough without
 * submitting an RGA job.  Input geometry is judged at runtime, from
 * the strides the buffers carry in their video meta: producers may
 * push dmabuf buffers with padded strides under plain system-memory
 * caps.
 *
 * ## Example launch line
 * |[
 * gst-launch-1.0 videotestsrc ! video/x-raw,format=NV12 ! rgavideoconvert ! video/x-raw,format=BGRx ! fakesink
 * ]|
 *
 * Since: 1.0
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>

#include "gstrgavideoconvert.h"

#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/allocators/gstdmabuf.h>
#include <gst/video/videodirection.h>

#include <rga/rga.h>
#include <rga/im2d.h>

#include "../rockchipmpp/gstrgautil.h"
#include "../dmaheap/gstdmaheapallocator.h"

/* drm_fourcc.h is not necessarily on this plugin's include path; this
 * is the stable value every <drm/drm_fourcc.h> defines. */
#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0ULL
#endif

GST_DEBUG_CATEGORY_STATIC (gst_rga_video_convert_debug);
#define GST_CAT_DEFAULT gst_rga_video_convert_debug

#define gst_rga_video_convert_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstRgaVideoConvert, gst_rga_video_convert,
    GST_TYPE_BASE_TRANSFORM,
    G_IMPLEMENT_INTERFACE (GST_TYPE_VIDEO_DIRECTION, NULL));

enum
{
  PROP_VIDEO_DIRECTION = 1,
  PROP_OUTPUT_MEMORY,
};

/* output-memory selector */
enum
{
  OUTPUT_MEMORY_AUTO = 0,
  OUTPUT_MEMORY_SYSTEM,
  OUTPUT_MEMORY_DMABUF,
};

#define OUTPUT_MEMORY_TYPE (gst_rga_output_memory_get_type ())
static GType
gst_rga_output_memory_get_type (void)
{
  static GType t = 0;

  if (!t) {
    static const GEnumValue vals[] = {
      {OUTPUT_MEMORY_AUTO, "auto", "auto"},
      {OUTPUT_MEMORY_SYSTEM, "system-memory", "system-memory"},
      {OUTPUT_MEMORY_DMABUF, "dmabuf", "dmabuf"},
      {0, NULL, NULL},
    };
    t = g_enum_register_static ("GstRgaOutputMemory", vals);
  }
  return t;
}

static void gst_rga_video_convert_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_rga_video_convert_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

/* IM_HAL_TRANSFORM bits of the video-direction; the transposes are
 * composed from a rotation and a flip (pixel-verified on rk3588). */
static gint
gst_rga_video_convert_direction_usage (GstVideoOrientationMethod direction)
{
  switch (direction) {
    case GST_VIDEO_ORIENTATION_IDENTITY:
      return 0;
    case GST_VIDEO_ORIENTATION_90R:
      return IM_HAL_TRANSFORM_ROT_90;
    case GST_VIDEO_ORIENTATION_180:
      return IM_HAL_TRANSFORM_ROT_180;
    case GST_VIDEO_ORIENTATION_90L:
      return IM_HAL_TRANSFORM_ROT_270;
    case GST_VIDEO_ORIENTATION_HORIZ:
      return IM_HAL_TRANSFORM_FLIP_H;
    case GST_VIDEO_ORIENTATION_VERT:
      return IM_HAL_TRANSFORM_FLIP_V;
    case GST_VIDEO_ORIENTATION_UL_LR:
      return IM_HAL_TRANSFORM_ROT_90 | IM_HAL_TRANSFORM_FLIP_H;
    case GST_VIDEO_ORIENTATION_UR_LL:
      return IM_HAL_TRANSFORM_ROT_90 | IM_HAL_TRANSFORM_FLIP_V;
    default:
      return 0;
  }
}

static void
gst_rga_video_convert_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (object);

  switch (prop_id) {
    case PROP_VIDEO_DIRECTION:
      self->direction_value = g_value_get_enum (value);
      self->direction =
          gst_rga_video_convert_direction_usage (self->direction_value);
      /* A transform must not passthrough identical caps while an
       * orientation is active. */
      gst_base_transform_set_passthrough (GST_BASE_TRANSFORM (self),
          self->direction == 0 &&
          gst_video_info_is_equal (&self->in_info, &self->out_info));
      break;
    case PROP_OUTPUT_MEMORY:
      self->out_memory = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gst_rga_video_convert_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (object);

  switch (prop_id) {
    case PROP_VIDEO_DIRECTION:
      g_value_set_enum (value, self->direction_value);
      break;
    case PROP_OUTPUT_MEMORY:
      g_value_set_enum (value, self->out_memory);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}



#define GST_RGA_FORMATS_UNKNOWN ((guint) - 1)

static void
gst_rga_take_format_list (GstStructure * structure, const gchar * field,
    GPtrArray * formats)
{
  GValue value = G_VALUE_INIT;

  if (formats->len == 1) {
    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, g_ptr_array_index (formats, 0));
  } else {
    GValue item = G_VALUE_INIT;
    guint i;

    gst_value_list_init (&value, formats->len);

    for (i = 0; i < formats->len; i++) {
      g_value_init (&item, G_TYPE_STRING);
      g_value_set_string (&item, g_ptr_array_index (formats, i));
      gst_value_list_append_value (&value, &item);
      g_value_unset (&item);
    }
  }

  gst_structure_take_value (structure, field, &value);
}

/* Fill @formats with every supported format name, in table order. */
static void
gst_rga_full_format_names (GPtrArray * formats)
{
  guint i;

  for (i = 0; i < gst_rga_formats_count (); i++)
    g_ptr_array_add (formats,
        g_strdup (gst_video_format_to_string (gst_rga_format_at
                (i)->gst_format)));
}

/* Set @field to the full supported format list. */
static void
gst_rga_set_full_format_list (GstStructure * structure, const gchar * field)
{
  GPtrArray *formats = g_ptr_array_new_with_free_func (g_free);

  gst_rga_full_format_names (formats);
  gst_rga_take_format_list (structure, field, formats);

  g_ptr_array_unref (formats);
}

/* Index of the single format named by @structure's @field, or
 * UNKNOWN when the field is missing, is a list, or names a format
 * outside the table. */
static guint
gst_rga_structure_format_index (const GstStructure * structure,
    const gchar * field)
{
  const gchar *format = gst_structure_get_string (structure, field);

  if (!format)
    return GST_RGA_FORMATS_UNKNOWN;

  return gst_rga_format_index (gst_video_format_from_string (format));
}

/* Set @field to the formats servable at a fixed @width: formats
 * whose tight plane-0 pitch misses the wstride alignment at that
 * width are dropped, except @keep.  The element only ever
 * allocates tight-pitch output buffers, so proposing such a
 * conversion would negotiate caps that then fail every frame.
 * @keep names the passthrough exception: with identical caps on both
 * pads the element runs in passthrough and submits no RGA job, so a
 * same-format 10-bit link stays negotiable at any width. */
/* Full format list with @keep first when valid, so fixation prefers
 * the passthrough form. */
static void
gst_rga_set_full_format_list_keep (GstStructure * structure,
    const gchar * field, guint keep)
{
  GPtrArray *formats = g_ptr_array_new_with_free_func (g_free);
  guint i;

  if (keep != GST_RGA_FORMATS_UNKNOWN)
    g_ptr_array_add (formats,
        g_strdup (gst_video_format_to_string (gst_rga_format_at
                (keep)->gst_format)));

  for (i = 0; i < gst_rga_formats_count (); i++) {
    if (i == keep)
      continue;
    g_ptr_array_add (formats,
        g_strdup (gst_video_format_to_string (gst_rga_format_at
                (i)->gst_format)));
  }

  gst_rga_take_format_list (structure, field, formats);
  g_ptr_array_unref (formats);
}

static void
gst_rga_set_width_format_list (GstStructure * structure, const gchar * field,
    gint width, guint keep)
{
  GPtrArray *formats = g_ptr_array_new_with_free_func (g_free);
  guint i;

  for (i = 0; i < gst_rga_formats_count (); i++) {
    if (i != keep && gst_rga_pitch_unsupported_at (i, width))
      continue;

    g_ptr_array_add (formats,
        g_strdup (gst_video_format_to_string (gst_rga_format_at
                (i)->gst_format)));
  }

  gst_rga_take_format_list (structure, field, formats);

  g_ptr_array_unref (formats);
}

/* Append the dma-drm variant of @structure covering every supported
 * format with a linear drm fourcc. */
static void gst_rga_append_dma_drm_structure (GstCaps * caps,
    const GstStructure * structure, GPtrArray * formats);

static void
gst_rga_append_dma_drm_full (GstCaps * caps, const GstStructure * structure)
{
  GPtrArray *formats = g_ptr_array_new_with_free_func (g_free);

  gst_rga_full_format_names (formats);
  gst_rga_append_dma_drm_structure (caps, structure, formats);

  g_ptr_array_unref (formats);
}

/* Collect the drm-format entries of a dma-drm structure as RGA
 * capable plain format names; only linear bare-fourcc entries map. */
static gboolean
gst_rga_collect_formats_from_dma_drm (const GstStructure * structure,
    GPtrArray * formats)
{
  const GValue *drm_formats = gst_structure_get_value (structure,
      "drm-format");
  GPtrArray *drm_strings;
  gboolean found = FALSE;
  guint i;

  if (!drm_formats)
    return FALSE;

  drm_strings = g_ptr_array_new_with_free_func (g_free);

  if (GST_VALUE_HOLDS_LIST (drm_formats)) {
    guint n = gst_value_list_get_size (drm_formats);

    for (i = 0; i < n; i++) {
      const GValue *item = gst_value_list_get_value (drm_formats, i);

      if (G_VALUE_HOLDS_STRING (item))
        g_ptr_array_add (drm_strings, g_value_dup_string (item));
    }
  } else if (G_VALUE_HOLDS_STRING (drm_formats)) {
    g_ptr_array_add (drm_strings, g_value_dup_string (drm_formats));
  }

  for (i = 0; i < drm_strings->len; i++) {
    const gchar *drm_str = g_ptr_array_index (drm_strings, i);
    guint64 modifier = DRM_FORMAT_MOD_LINEAR;
    GstVideoFormat format;
    guint32 fourcc;

    fourcc = gst_video_dma_drm_fourcc_from_string (drm_str, &modifier);
    if (fourcc == 0 || modifier != DRM_FORMAT_MOD_LINEAR)
      continue;

    format = gst_video_dma_drm_format_to_gst_format (fourcc, modifier);
    if (format == GST_VIDEO_FORMAT_UNKNOWN)
      continue;
    if (gst_rga_format_index (format) == GST_RGA_FORMATS_UNKNOWN)
      continue;

    g_ptr_array_add (formats, g_strdup (gst_video_format_to_string (format)));
    found = TRUE;
  }

  g_ptr_array_unref (drm_strings);

  return found;
}

/* Append the dma-drm variant for a set of plain format names. */
static void
gst_rga_append_dma_drm_structure (GstCaps * caps,
    const GstStructure * structure, GPtrArray * formats)
{
  GPtrArray *drm_formats = g_ptr_array_new_with_free_func (g_free);
  GstStructure *s;
  guint i;

  for (i = 0; i < formats->len; i++) {
    GstVideoFormat format =
        gst_video_format_from_string (g_ptr_array_index (formats, i));
    guint32 fourcc = gst_video_dma_drm_fourcc_from_format (format);
    gchar *str;

    if (fourcc == 0)
      continue;

    str = gst_video_dma_drm_fourcc_to_string (fourcc, DRM_FORMAT_MOD_LINEAR);
    if (str) {
      if (!g_ptr_array_find_with_equal_func (drm_formats, str, g_str_equal,
              NULL))
        g_ptr_array_add (drm_formats, str);
      else
        g_free (str);
    }
  }

  if (drm_formats->len == 0) {
    g_ptr_array_unref (drm_formats);
    return;
  }

  s = gst_structure_copy (structure);
  gst_structure_set (s, "format", G_TYPE_STRING, "DMA_DRM", NULL);
  gst_rga_take_format_list (s, "drm-format", drm_formats);
  gst_caps_append_structure_full (caps, s,
      gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

  g_ptr_array_unref (drm_formats);
}

/* Build the sink template caps: system memory first, then the
 * per-format DMABuf variant, then the dma-drm variant. */
static GstCaps *
gst_rga_video_convert_sink_template_caps (void)
{
  GstCaps *res = gst_caps_new_empty ();
  GPtrArray *formats = g_ptr_array_new_with_free_func (g_free);
  GstStructure *s;
  guint i;

  for (i = 0; i < gst_rga_formats_count (); i++)
    g_ptr_array_add (formats,
        g_strdup (gst_video_format_to_string (gst_rga_format_at
                (i)->gst_format)));

  /* system memory */
  s = gst_structure_new_empty ("video/x-raw");
  gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
      "height", GST_TYPE_INT_RANGE, 1, G_MAXINT,
      "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
  gst_rga_take_format_list (s, "format", formats);
  gst_caps_append_structure (res, s);

  /* per-format DMABuf */
  s = gst_structure_new_empty ("video/x-raw");
  gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
      "height", GST_TYPE_INT_RANGE, 1, G_MAXINT,
      "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
  gst_rga_take_format_list (s, "format", formats);
  gst_caps_append_structure_full (res, s,
      gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

  /* dma-drm: only the formats with a linear drm fourcc */
  {
    GstStructure *s = gst_structure_new_empty ("video/x-raw");

    gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
        "height", GST_TYPE_INT_RANGE, 1, G_MAXINT,
        "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);
    gst_rga_append_dma_drm_structure (res, s, formats);
    gst_structure_free (s);
  }

  g_ptr_array_unref (formats);

  return res;
}

/* Prefer the opposite pad's geometry (swapped for 90-degree
 * directions) when the peer leaves the size open, so the generic
 * fixation does not fall back to the range minimum. */
static GstCaps *
gst_rga_video_convert_fixate_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (trans);
  GstVideoInfo info;
  gint w = 0, h = 0, i, n;

  if (gst_video_info_from_caps (&info, caps)) {
    w = GST_VIDEO_INFO_WIDTH (&info);
    h = GST_VIDEO_INFO_HEIGHT (&info);

    if (self->direction & (IM_HAL_TRANSFORM_ROT_90 | IM_HAL_TRANSFORM_ROT_270)) {
      gint t = w;
      w = h;
      h = t;
    }
  }

  othercaps = gst_caps_make_writable (othercaps);
  n = gst_caps_get_size (othercaps);
  for (i = 0; i < n; i++) {
    GstStructure *s = gst_caps_get_structure (othercaps, i);
    const GValue *v;

    v = gst_structure_get_value (s, "width");
    if (v && GST_VALUE_HOLDS_INT_RANGE (v) && w >= 1)
      gst_structure_fixate_field_nearest_int (s, "width", w);

    v = gst_structure_get_value (s, "height");
    if (v && GST_VALUE_HOLDS_INT_RANGE (v) && h >= 1)
      gst_structure_fixate_field_nearest_int (s, "height", h);
  }

  return
      GST_BASE_TRANSFORM_CLASS (gst_rga_video_convert_parent_class)->fixate_caps
      (trans, direction, caps, othercaps);
}

static GstCaps *
gst_rga_video_convert_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (trans);
  GstCaps *res;
  guint i, n;

  n = gst_caps_get_size (caps);

  if (direction == GST_PAD_SINK) {
    /* Caps on the sink pad: every supported input structure converts
     * to any supported format on the src pad, so answer with the full
     * set the way videoconvert does; restricting the answer to the
     * input formats would empty the peer-filter chain of downstream
     * capsfilters during allowed-caps queries. */
    res = gst_caps_new_empty ();

    /* NOTE: no dma-drm output variant here.  set_caps can resolve the
     * dma-drm convention on the input side only; advertising it as an
     * output candidate made the fixate land on a structure the element
     * then rejected, killing the link before PLAYING.  The per-format
     * dmabuf variant below is the supported dmabuf output form. */

    for (i = 0; i < n; i++) {
      const GstStructure *structure = gst_caps_get_structure (caps, i);
      gint width = 0;
      guint keep = GST_RGA_FORMATS_UNKNOWN;
      GstStructure *s;

      if (gst_structure_get_int (structure, "width", &width) && width >= 1
          && !gst_caps_features_contains (gst_caps_get_features (caps, i),
              GST_CAPS_FEATURE_MEMORY_DMABUF))
        keep = gst_rga_structure_format_index (structure, "format");

      /* The output geometry is free: the element scales, and the
       * fixed output width the peer fixates is alignment-checked in
       * set_caps.  The dmabuf form leads: this is a hardware
       * converter and the dma-drm consumers prefer it; a sysmem-only
       * peer still intersects down to the system-memory form, whose
       * own format comes first so same-format links passthrough. */
      s = gst_structure_copy (structure);
      gst_structure_remove_field (s, "drm-format");
      gst_structure_remove_field (s, "width");
      gst_structure_remove_field (s, "height");
      gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
          "height", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
      gst_rga_set_full_format_list_keep (s, "format", GST_RGA_FORMATS_UNKNOWN);
      gst_caps_append_structure_full (res, s,
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

      s = gst_structure_copy (structure);
      gst_structure_remove_field (s, "drm-format");
      gst_structure_remove_field (s, "width");
      gst_structure_remove_field (s, "height");
      gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
          "height", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
      gst_rga_set_full_format_list_keep (s, "format", keep);
      gst_caps_append_structure_full (res, s,
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY, NULL));
    }
  } else {
    /* Caps on the src pad: the sink pad accepts every input form of
     * the supported formats, since any of them converts to any other. */
    res = gst_caps_new_empty ();

    for (i = 0; i < n; i++) {
      const GstStructure *structure = gst_caps_get_structure (caps, i);
      GstStructure *s;

      /* Every input form is accepted: an input-side width gate would
       * be unsound, because producers may push dmabuf buffers with
       * padded video-meta strides under plain system-memory caps
       * (mppvideodec does).  The input geometry is judged at runtime,
       * from the strides the buffers actually carry. */
      /* The input geometry is free too: the element scales between
       * whichever geometries the two sides negotiate. */
      s = gst_structure_copy (structure);
      gst_structure_remove_field (s, "drm-format");
      gst_structure_remove_field (s, "width");
      gst_structure_remove_field (s, "height");
      gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
          "height", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
      gst_rga_set_full_format_list (s, "format");
      gst_caps_append_structure_full (res, s,
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY, NULL));

      s = gst_structure_copy (structure);
      gst_structure_remove_field (s, "drm-format");
      gst_structure_remove_field (s, "width");
      gst_structure_remove_field (s, "height");
      gst_structure_set (s, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
          "height", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
      gst_rga_set_full_format_list (s, "format");
      gst_caps_append_structure_full (res, s,
          gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));
    }

    /* NOTE: no dma-drm output variant here either - the element can
     * only resolve the dma-drm convention on its input side. */
  }

  /* A 90-degree direction swaps the geometry between the two pads;
   * the swap is its own inverse, so it serves the forward and the
   * reverse negotiation alike. */
  if (self->direction & (IM_HAL_TRANSFORM_ROT_90 | IM_HAL_TRANSFORM_ROT_270)) {
    guint k;

    for (k = 0; k < gst_caps_get_size (res); k++) {
      GstStructure *st = gst_caps_get_structure (res, k);
      gint w = 0, h = 0;

      if (gst_structure_get_int (st, "width", &w) &&
          gst_structure_get_int (st, "height", &h)) {
        gst_structure_set (st, "width", G_TYPE_INT, h,
            "height", G_TYPE_INT, w, NULL);
      } else {
        gst_structure_remove_field (st, "width");
        gst_structure_remove_field (st, "height");
        gst_structure_set (st, "width", GST_TYPE_INT_RANGE, 1, G_MAXINT,
            "height", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
      }
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

static gboolean
gst_rga_video_convert_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (trans);
  GstVideoInfoDmaDrm drm_info;
  GstVideoFormat format;

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

  self->out_dmabuf =
      gst_caps_features_contains (gst_caps_get_features (outcaps, 0),
      GST_CAPS_FEATURE_MEMORY_DMABUF);
  if (self->out_memory == OUTPUT_MEMORY_DMABUF)
    self->out_dmabuf = TRUE;
  else if (self->out_memory == OUTPUT_MEMORY_SYSTEM)
    self->out_dmabuf = FALSE;

  /* An active direction transform, or a dmabuf output serving
   * sysmem-only input, must not passthrough even on identical caps:
   * the passthrough would push input-memory buffers under the
   * negotiated output caps. */
  gst_base_transform_set_passthrough (trans,
      self->direction == 0 && !self->out_dmabuf &&
      gst_caps_is_equal (incaps, outcaps));

  format = GST_VIDEO_INFO_FORMAT (&self->in_info);
  if (format == GST_VIDEO_FORMAT_UNKNOWN ||
      gst_rga_format_index (format) == GST_RGA_FORMATS_UNKNOWN)
    return FALSE;

  format = GST_VIDEO_INFO_FORMAT (&self->out_info);
  if (format == GST_VIDEO_FORMAT_UNKNOWN ||
      gst_rga_format_index (format) == GST_RGA_FORMATS_UNKNOWN)
    return FALSE;

  /* The element only allocates tight-pitch output buffers, so a
   * 10-bit output geometry whose plane-0 pitch misses the 64-byte
   * alignment librga demands is decided by the caps alone: refuse it
   * here instead of failing every frame at runtime.  Identical caps
   * on both pads are exempt - the element runs in passthrough and
   * submits no RGA job.  The input side is not gated here: producers
   * may push dmabuf buffers with padded video-meta strides under
   * plain system-memory caps (mppvideodec does), so the real input
   * geometry is only known at runtime, from the strides the buffers
   * carry. */
  if (!gst_caps_is_equal (incaps, outcaps) &&
      gst_rga_pitch_unsupported_at (gst_rga_format_index
          (GST_VIDEO_INFO_FORMAT (&self->out_info)),
          GST_VIDEO_INFO_WIDTH (&self->out_info)))
    return FALSE;

  return TRUE;
}

static gboolean
gst_rga_video_convert_get_unit_size (GstBaseTransform * trans,
    GstCaps * caps, gsize * size)
{
  GstVideoInfo info;

  (void) trans;

  if (!gst_video_info_from_caps (&info, caps))
    return FALSE;

  *size = GST_VIDEO_INFO_SIZE (&info);

  return TRUE;
}

/* Override the buffer's negotiated layout with the one it actually
 * carries, so padded frames (strides larger than the caps describe)
 * are described to the driver with the right geometry. */
static GstFlowReturn
gst_rga_video_convert_transform (GstBaseTransform * trans, GstBuffer * in,
    GstBuffer * out)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (trans);
  GstVideoInfo in_info = self->in_info, out_info = self->out_info;
  GstMapInfo mapin = { 0, }, mapout = { 0, };
  rga_buffer_t src, dst;
  im_rect srect = { 0, };
  im_rect drect = { 0, };
  IM_STATUS ret;
  guint in_index, out_index;
  gint fd = -1;
  void *vir_addr = NULL;
  gboolean mapped = FALSE;

  in_index = gst_rga_format_index (GST_VIDEO_INFO_FORMAT (&in_info));
  out_index = gst_rga_format_index (GST_VIDEO_INFO_FORMAT (&out_info));
  if (in_index == GST_RGA_FORMATS_UNKNOWN ||
      out_index == GST_RGA_FORMATS_UNKNOWN)
    return GST_FLOW_NOT_NEGOTIATED;

  gst_rga_info_apply_video_meta (&in_info, in);
  gst_rga_info_apply_video_meta (&out_info, out);

  /* Crop from the input meta; the source rect maps onto the full
   * destination, so a cropped or differently sized output scales. */
  {
    GstVideoCropMeta *cmeta = gst_buffer_get_video_crop_meta (in);

    if (cmeta && cmeta->width >= 2 && cmeta->height >= 2 &&
        cmeta->x + cmeta->width <= GST_VIDEO_INFO_WIDTH (&in_info) &&
        cmeta->y + cmeta->height <= GST_VIDEO_INFO_HEIGHT (&in_info)) {
      srect.x = cmeta->x;
      srect.y = cmeta->y;
      srect.width = cmeta->width;
      srect.height = cmeta->height;
    }
  }

  /* Prefer the dma fd so the driver does not have to migrate the
   * frame through system memory. */
  if (gst_buffer_n_memory (in) == 1) {
    GstMemory *mem = gst_buffer_peek_memory (in, 0);
    gsize offset;

    if (gst_is_dmabuf_memory (mem)) {
      gst_memory_get_sizes (mem, &offset, NULL);
      if (!offset)
        fd = gst_dmabuf_memory_get_fd (mem);
    }
  }

  if (fd < 0) {
    if (!gst_buffer_map (in, &mapin, GST_MAP_READ)) {
      GST_ERROR_OBJECT (trans, "failed to map input");
      return GST_FLOW_ERROR;
    }
    vir_addr = mapin.data;
    mapped = TRUE;

    /* The RGA driver mishandles raster reads whose plane-0 user
     * address is not 16-byte aligned (measured on rk3588, RGA3
     * raster core: a 10-bit base at %16==8 reads as garbage, and an
     * 8-bit or RGB base at %16==8 starts the read byte stream eight
     * bytes late, so every converted row opens with the previous
     * row's tail and the plane lead-in stays unwritten), and
     * gstreamer system-memory buffers routinely carry such
     * addresses.  Route misaligned input of every format through an
     * aligned scratch copy; the memcpy is still far cheaper than the
     * CPU conversion this element offloads.  Dma-buf input skips
     * this path entirely: its mapping is page-aligned. */
    if ((((guintptr) vir_addr +
                GST_VIDEO_INFO_PLANE_OFFSET (&in_info, 0)) & 0xf) != 0) {
      gsize offset = GST_VIDEO_INFO_PLANE_OFFSET (&in_info, 0);
      gsize size = gst_rga_frame_size (in_index, &in_info);
      guint p;

      /* Rebase the copy at the scratch base instead of preserving
       * the plane-0 offset: an offset that is itself misaligned
       * would keep the RGA read misaligned, which is exactly what
       * this copy works around. */
      if (self->scratch_size < size - offset) {
        guint8 *mem = g_malloc (size - offset + 64);

        g_free (self->scratch_mem);
        self->scratch_mem = mem;
        self->scratch = (guint8 *) (((guintptr) mem + 63) & ~(guintptr) 63);
        self->scratch_size = size - offset;
      }

      memcpy (self->scratch, (guint8 *) vir_addr + offset, size - offset);
      vir_addr = self->scratch;
      for (p = 0; p < GST_VIDEO_MAX_PLANES; p++)
        if (in_info.offset[p] >= offset)
          in_info.offset[p] -= offset;
    }
  }

  if (!gst_buffer_map (out, &mapout, GST_MAP_WRITE)) {
    GST_ERROR_OBJECT (trans, "failed to map output");
    if (mapped)
      gst_buffer_unmap (in, &mapin);
    return GST_FLOW_ERROR;
  }

  src = gst_rga_wrap_info (in_index, fd, vir_addr, &in_info);
  dst = gst_rga_wrap_info (out_index, -1, mapout.data, &out_info);

  if (!src.wstride || !dst.wstride) {
    GST_ELEMENT_ERROR (trans, STREAM, FORMAT,
        ("unsupported frame geometry for RGA"), (NULL));
    gst_buffer_unmap (out, &mapout);
    if (mapped)
      gst_buffer_unmap (in, &mapin);
    return GST_FLOW_ERROR;
  }

  ret = gst_rga_process (&src, &dst, &srect, &drect, self->direction);
  if (ret <= 0) {
    GST_ELEMENT_ERROR (trans, STREAM, FORMAT,
        ("RGA conversion failed: %s", imStrError (ret)), (NULL));
    gst_buffer_unmap (out, &mapout);
    if (mapped)
      gst_buffer_unmap (in, &mapin);
    return GST_FLOW_ERROR;
  }

  gst_buffer_unmap (out, &mapout);
  if (mapped)
    gst_buffer_unmap (in, &mapin);

  return GST_FLOW_OK;
}

/* Serve the output from our own system memory pool: basetransform
 * would otherwise allocate from the first pool the downstream query
 * returned (usually the sink's own pool). */
static GstFlowReturn
gst_rga_video_convert_prepare_output_buffer (GstBaseTransform * trans,
    GstBuffer * input, GstBuffer ** outbuf)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (trans);
  GstCaps *caps;
  gsize size;

  /* In passthrough the base class pushes the prepared buffer without
   * calling transform, so hand it the input buffer instead of an
   * unwritten pool buffer. */
  if (gst_base_transform_is_passthrough (trans)) {
    *outbuf = gst_buffer_ref (input);
    return GST_FLOW_OK;
  }

  if (self->out_dmabuf) {
    GstBaseTransformClass *bclass = GST_BASE_TRANSFORM_GET_CLASS (trans);
    const struct gst_rga_format_entry *e;
    GstVideoInfo info = self->out_info;
    GstBuffer *out;
    GstMemory *mem;
    gsize stride, offset1, size;
    gint height = GST_VIDEO_INFO_HEIGHT (&info);
    gint rows = 1;

    e = gst_rga_format_at (gst_rga_format_index (GST_VIDEO_INFO_FORMAT
            (&info)));
    if (!e)
      return GST_FLOW_NOT_NEGOTIATED;

    /* The heap output carries RGA-aligned strides: the caps describe
     * the tight geometry, the video meta the padded layout. */
    stride = (GST_VIDEO_INFO_PLANE_STRIDE (&info, 0) + e->walign - 1) /
        e->walign * e->walign;
    offset1 = (gsize) stride *height;
    size = offset1;
    if (GST_VIDEO_INFO_N_PLANES (&info) > 1) {
      rows = (height + (1 << e->uv_vshift) - 1) >> e->uv_vshift;
      size += (gsize) stride *rows;
    }

    if (!self->heap_alloc) {
      self->heap_alloc = gst_dma_heap_allocator_new (NULL);
      if (!self->heap_alloc)
        return GST_FLOW_ERROR;
    }

    GST_DEBUG_OBJECT (trans,
        "dmabuf output %" G_GSIZE_FORMAT " bytes, stride %" G_GSIZE_FORMAT,
        size, stride);

    mem = gst_allocator_alloc (self->heap_alloc, size, NULL);
    if (!mem)
      return GST_FLOW_ERROR;

    out = gst_buffer_new ();
    gst_buffer_append_memory (out, mem);
    if (GST_VIDEO_INFO_N_PLANES (&info) > 1)
      gst_buffer_add_video_meta_full (out, GST_VIDEO_FRAME_FLAG_NONE,
          GST_VIDEO_INFO_FORMAT (&info),
          GST_VIDEO_INFO_WIDTH (&info), GST_VIDEO_INFO_HEIGHT (&info), 2,
          (gsize[2]) {
          0, offset1}
        , (gint[2]) {
        (gint) stride, (gint) stride}
    );
    else
    gst_buffer_add_video_meta_full (out, GST_VIDEO_FRAME_FLAG_NONE,
        GST_VIDEO_INFO_FORMAT (&info),
        GST_VIDEO_INFO_WIDTH (&info), GST_VIDEO_INFO_HEIGHT (&info), 1,
        (gsize[1]) {
        0}
        , (gint[1]) {
        (gint) stride}
    );

    if (bclass->copy_metadata && !bclass->copy_metadata (trans, input, out))
      GST_ELEMENT_WARNING (trans, STREAM, NOT_IMPLEMENTED,
          ("could not copy metadata"), (NULL));

    *outbuf = out;
    return GST_FLOW_OK;
  }

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

static gboolean
gst_rga_video_convert_stop (GstBaseTransform * trans)
{
  GstRgaVideoConvert *self = GST_RGA_VIDEO_CONVERT (trans);

  if (self->pool)
    gst_buffer_pool_set_active (self->pool, FALSE);
  gst_clear_object (&self->pool);
  gst_clear_caps (&self->pool_caps);

  gst_clear_object (&self->heap_alloc);

  g_clear_pointer (&self->scratch_mem, g_free);
  self->scratch = NULL;
  self->scratch_size = 0;

  return TRUE;
}

static void
gst_rga_video_convert_class_init (GstRgaVideoConvertClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GstBaseTransformClass *trans_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstCaps *sink_caps;
  GstCaps *src_caps;

  GST_DEBUG_CATEGORY_INIT (gst_rga_video_convert_debug, "rgavideoconvert",
      0, "rgavideoconvert");

  gobject_class->set_property = gst_rga_video_convert_set_property;
  gobject_class->get_property = gst_rga_video_convert_get_property;
  g_object_class_install_property (gobject_class,
      PROP_OUTPUT_MEMORY,
      g_param_spec_enum ("output-memory", "Output Memory",
          "Where the converted frames are allocated (auto follows the "
          "negotiation)",
          OUTPUT_MEMORY_TYPE, OUTPUT_MEMORY_AUTO,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
      PROP_VIDEO_DIRECTION,
      g_param_spec_enum ("video-direction", "Video Direction",
          "Rotation and flipping applied to the output (identity "
          "keeps the frame untouched)",
          GST_TYPE_VIDEO_ORIENTATION_METHOD, GST_VIDEO_ORIENTATION_IDENTITY,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (gstelement_class,
      "RGA video converter", "Filter/Converter/Video/Hardware",
      "Convert video frames using the Rockchip RGA 2D accelerator",
      "Rockchip Electronics");

  sink_caps = gst_rga_video_convert_sink_template_caps ();
  gst_element_class_add_pad_template (gstelement_class,
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS, sink_caps));

  /* Mirror the sink's sysmem and per-format dmabuf variants: the
   * element converts both to and from dmabuf.  The dma-drm structure
   * is an input-only convention - set_caps cannot resolve it on the
   * output side - so drop it to keep fixate away from it. */
  src_caps = gst_caps_copy (sink_caps);
  for (guint i = gst_caps_get_size (src_caps); i > 0; i--) {
    GstStructure *st = gst_caps_get_structure (src_caps, i - 1);
    const gchar *fmt = gst_structure_get_string (st, "format");

    if (fmt && !g_strcmp0 (fmt, "DMA_DRM"))
      gst_caps_remove_structure (src_caps, i - 1);
  }
  gst_element_class_add_pad_template (gstelement_class,
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS, src_caps));

  trans_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_rga_video_convert_transform_caps);
  trans_class->fixate_caps =
      GST_DEBUG_FUNCPTR (gst_rga_video_convert_fixate_caps);
  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_rga_video_convert_set_caps);
  trans_class->get_unit_size =
      GST_DEBUG_FUNCPTR (gst_rga_video_convert_get_unit_size);
  trans_class->prepare_output_buffer =
      GST_DEBUG_FUNCPTR (gst_rga_video_convert_prepare_output_buffer);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_rga_video_convert_transform);
  trans_class->stop = GST_DEBUG_FUNCPTR (gst_rga_video_convert_stop);

  trans_class->passthrough_on_same_caps = TRUE;
}

static void
gst_rga_video_convert_init (GstRgaVideoConvert * self)
{
  gst_video_info_init (&self->in_info);
  gst_video_info_init (&self->out_info);
  self->direction_value = GST_VIDEO_ORIENTATION_IDENTITY;
  self->out_memory = OUTPUT_MEMORY_AUTO;
  self->direction = 0;
  self->out_dmabuf = FALSE;
  self->heap_alloc = NULL;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rgavideoconvert", GST_RANK_PRIMARY,
      GST_TYPE_RGA_VIDEO_CONVERT);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rga,
    "Rockchip RGA 2D accelerator plugin",
    plugin_init, VERSION, "LGPL", GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
