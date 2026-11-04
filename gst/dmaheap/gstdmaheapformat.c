/*
 * GStreamer dmaheap plugin shared caps helpers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "gstdmaheapformat.h"

#include <gst/video/video.h>
#include <gst/allocators/gstdmabuf.h>

void
gst_dma_heap_caps_collect_strings (const GValue * value, GPtrArray * strings)
{
  if (GST_VALUE_HOLDS_LIST (value)) {
    guint i, n = gst_value_list_get_size (value);

    for (i = 0; i < n; i++) {
      const GValue *item = gst_value_list_get_value (value, i);

      if (G_VALUE_HOLDS_STRING (item))
        g_ptr_array_add (strings, g_value_dup_string (item));
    }
  } else if (G_VALUE_HOLDS_STRING (value)) {
    g_ptr_array_add (strings, g_value_dup_string (value));
  }
}

void
gst_dma_heap_caps_take_string_list_value (GValue * value, GPtrArray * strings)
{
  if (strings->len == 1) {
    g_value_init (value, G_TYPE_STRING);
    g_value_set_string (value, g_ptr_array_index (strings, 0));
  } else {
    GValue item = G_VALUE_INIT;
    guint i;

    gst_value_list_init (value, strings->len);

    for (i = 0; i < strings->len; i++) {
      g_value_init (&item, G_TYPE_STRING);
      g_value_set_string (&item, g_ptr_array_index (strings, i));
      gst_value_list_append_value (value, &item);
      g_value_unset (&item);
    }
  }
}

void
gst_dma_heap_caps_add_unique (GPtrArray * strings, gchar * str)
{
  if (!g_ptr_array_find_with_equal_func (strings, str, g_str_equal, NULL))
    g_ptr_array_add (strings, str);
  else
    g_free (str);
}

void
gst_dma_heap_caps_append_dma_drm (GstCaps * caps,
    const GstStructure * structure)
{
  const GValue *formats;
  GPtrArray *drm_formats;
  GstStructure *s;
  GValue drm_value = G_VALUE_INIT;

  formats = gst_structure_get_value (structure, "format");
  if (!formats)
    return;

  drm_formats = g_ptr_array_new_with_free_func (g_free);

  {
    GPtrArray *names = g_ptr_array_new_with_free_func (g_free);
    guint i;

    gst_dma_heap_caps_collect_strings (formats, names);

    for (i = 0; i < names->len; i++) {
      const gchar *name = g_ptr_array_index (names, i);
      GstVideoFormat format = gst_video_format_from_string (name);
      guint32 fourcc;
      gchar *str;

      if (format == GST_VIDEO_FORMAT_UNKNOWN)
        continue;

      fourcc = gst_video_dma_drm_fourcc_from_format (format);
      if (fourcc == DRM_FORMAT_INVALID)
        continue;

      /* Linear serializes as the bare fourcc, no modifier suffix. */
      str = gst_video_dma_drm_fourcc_to_string (fourcc, DRM_FORMAT_MOD_LINEAR);
      if (str)
        gst_dma_heap_caps_add_unique (drm_formats, str);
    }

    g_ptr_array_unref (names);
  }

  if (drm_formats->len == 0) {
    g_ptr_array_unref (drm_formats);
    return;
  }

  s = gst_structure_copy (structure);
  gst_structure_set (s, "format", G_TYPE_STRING, "DMA_DRM", NULL);
  gst_dma_heap_caps_take_string_list_value (&drm_value, drm_formats);
  gst_structure_take_value (s, "drm-format", &drm_value);
  gst_caps_append_structure_full (caps, s,
      gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_DMABUF, NULL));

  g_ptr_array_unref (drm_formats);
}

void
gst_dma_heap_caps_append_raw_from_dma_drm (GstCaps * caps,
    const GstStructure * structure)
{
  const GValue *drm_formats;
  GPtrArray *formats;
  GstStructure *s;
  GValue format_value = G_VALUE_INIT;

  drm_formats = gst_structure_get_value (structure, "drm-format");
  if (!drm_formats)
    return;

  formats = g_ptr_array_new_with_free_func (g_free);

  {
    GPtrArray *drm_strings = g_ptr_array_new_with_free_func (g_free);
    guint i;

    gst_dma_heap_caps_collect_strings (drm_formats, drm_strings);

    for (i = 0; i < drm_strings->len; i++) {
      const gchar *drm_str = g_ptr_array_index (drm_strings, i);
      /* Only read when the fourcc parses; from_string then always
       * stores the modifier. */
      guint64 modifier = DRM_FORMAT_MOD_LINEAR;
      guint32 fourcc;
      GstVideoFormat format;

      fourcc = gst_video_dma_drm_fourcc_from_string (drm_str, &modifier);
      if (fourcc == DRM_FORMAT_INVALID)
        continue;
      if (modifier != DRM_FORMAT_MOD_LINEAR)
        /* Only the linear form maps back to a plain raw layout. */
        continue;

      format = gst_video_dma_drm_format_to_gst_format (fourcc, modifier);
      if (format == GST_VIDEO_FORMAT_UNKNOWN)
        continue;

      gst_dma_heap_caps_add_unique (formats,
          g_strdup (gst_video_format_to_string (format)));
    }

    g_ptr_array_unref (drm_strings);
  }

  if (formats->len == 0) {
    g_ptr_array_unref (formats);
    return;
  }

  s = gst_structure_copy (structure);
  gst_structure_remove_field (s, "drm-format");
  gst_dma_heap_caps_take_string_list_value (&format_value, formats);
  gst_structure_take_value (s, "format", &format_value);
  gst_caps_append_structure_full (caps, s,
      gst_caps_features_new (GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY, NULL));

  g_ptr_array_unref (formats);
}
