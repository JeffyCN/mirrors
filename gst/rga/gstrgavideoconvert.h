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

#ifndef __GST_RGA_VIDEO_CONVERT_H__
#define __GST_RGA_VIDEO_CONVERT_H__

#include <gst/base/gstbasetransform.h>
#include <gst/video/video.h>

G_BEGIN_DECLS;

#define GST_TYPE_RGA_VIDEO_CONVERT              (gst_rga_video_convert_get_type ())
#define GST_IS_RGA_VIDEO_CONVERT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_RGA_VIDEO_CONVERT))
#define GST_RGA_VIDEO_CONVERT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_RGA_VIDEO_CONVERT, GstRgaVideoConvert))
#define GST_IS_RGA_VIDEO_CONVERT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_RGA_VIDEO_CONVERT))
#define GST_RGA_VIDEO_CONVERT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_RGA_VIDEO_CONVERT, GstRgaVideoConvertClass))
#define GST_RGA_VIDEO_CONVERT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_RGA_VIDEO_CONVERT, GstRgaVideoConvertClass))

typedef struct _GstRgaVideoConvert GstRgaVideoConvert;
typedef struct _GstRgaVideoConvertClass GstRgaVideoConvertClass;

struct _GstRgaVideoConvert
{
  GstBaseTransform parent;

  GstVideoInfo in_info;
  GstVideoInfo out_info;

  /* IM_HAL_TRANSFORM_* bits of the video-direction property; zero
   * means identity (rotation and flipping disabled) */
  GstVideoOrientationMethod direction_value;
  gint direction;
  gboolean out_dmabuf;          /* negotiated output feature */
  gint out_memory;              /* OUTPUT_MEMORY_* selector */
  GstAllocator *heap_alloc;     /* dma heap for the dmabuf output */
  GstBufferPool *pool;
  GstCaps *pool_caps;
  guint8 *scratch_mem;          /* g_malloc block behind scratch */
  guint8 *scratch;              /* 64-byte aligned copy of misaligned input */
  gsize scratch_size;
};

struct _GstRgaVideoConvertClass
{
  GstBaseTransformClass parent_class;
};

GType gst_rga_video_convert_get_type (void);

G_END_DECLS;

#endif /* __GST_RGA_VIDEO_CONVERT_H__ */
