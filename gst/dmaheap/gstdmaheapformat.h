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

#ifndef __GST_DMA_HEAP_FORMAT_H__
#define __GST_DMA_HEAP_FORMAT_H__

#include <gst/gst.h>

G_BEGIN_DECLS;

/* drm_fourcc.h is not necessarily on this plugin's include path; these
 * are the stable values every <drm/drm_fourcc.h> defines. */
#ifndef DRM_FORMAT_INVALID
#define DRM_FORMAT_INVALID 0
#endif
#ifndef DRM_FORMAT_MOD_LINEAR
#define DRM_FORMAT_MOD_LINEAR 0ULL
#endif

/* Append one string value of a caps field, which may be a single
 * string or a list, to @strings. */
void gst_dma_heap_caps_collect_strings (const GValue * value,
    GPtrArray * strings);

/* Build a string-or-list GValue from @strings, mirroring how kmssink
 * serializes its drm-format field. */
void gst_dma_heap_caps_take_string_list_value (GValue * value,
    GPtrArray * strings);

/* Append @str to @strings unless an equal string is already present. */
void gst_dma_heap_caps_add_unique (GPtrArray * strings, gchar * str);

/* Append the standard dma-drm variant of one per-format video/x-raw
 * structure to @caps.  Only formats with a linear drm fourcc can be
 * described; formats without one keep the per-format form only. */
void gst_dma_heap_caps_append_dma_drm (GstCaps * caps,
    const GstStructure * structure);

/* Append the per-format system memory structures matching one dma-drm
 * structure to @caps, the reverse of gst_dma_heap_caps_append_dma_drm().
 * Only linear bare-fourcc entries map back. */
void gst_dma_heap_caps_append_raw_from_dma_drm (GstCaps * caps,
    const GstStructure * structure);

G_END_DECLS;

#endif /* __GST_DMA_HEAP_FORMAT_H__ */
