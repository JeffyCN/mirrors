/*
 * Copyright 2025 Rockchip Electronics Co., Ltd
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
#include "config.h"
#endif

#include <string.h>

#include <gst/allocators/gstdmabuf.h>

#include "gstmpp.h"

#include "gstrgautil.h"

#ifdef HAVE_RGA

#include <rga/rga.h>
#include <rga/im2d.h>

GST_DEBUG_CATEGORY_STATIC (mpp_rga_debug);
#define GST_CAT_DEFAULT mpp_rga_debug

gboolean
gst_mpp_use_rga ()
{
  static int mpp_use_rga = -1;

  if (mpp_use_rga < 0) {
    const gchar *buf = g_getenv ("GST_MPP_NO_RGA");

    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mpprga", 0, "MPP RGA converter");

    if (!buf || buf[0] == '0') {
      GST_INFO ("RGA enabled");
      mpp_use_rga = 1;
      return 1;
    }

    GST_INFO ("RGA disabled by env (GST_MPP_NO_RGA)");
    mpp_use_rga = 0;
  }

  return mpp_use_rga > 0;
}

static gint
gst_mpp_rga_get_rotation (gint rotation)
{
  switch (rotation) {
    case 0:
      return 0;
    case 90:
      return IM_HAL_TRANSFORM_ROT_90;
    case 180:
      return IM_HAL_TRANSFORM_ROT_180;
    case 270:
      return IM_HAL_TRANSFORM_ROT_270;
    default:
      return -1;
  }
}

/* Frame description adapters over the shared rga core (gstrgautil),
 * extracted from the rgavideoconvert element so both users apply the
 * same mapping, stride and alignment rules. */
static rga_buffer_t
gst_mpp_rga_buffer_from_info (GstVideoInfo * info, gint fd, void *vir_addr)
{
  return gst_rga_wrap_info (gst_rga_format_index (GST_VIDEO_INFO_FORMAT (info)),
      fd, vir_addr, info);
}

static rga_buffer_t
gst_mpp_rga_buffer_from_mpp_frame (MppFrame mframe)
{
  MppBuffer mbuf = mpp_frame_get_buffer (mframe);
  MppFrameFormat mpp_format = mpp_frame_get_fmt (mframe);
  RgaSURF_FORMAT rga_format = gst_mpp_mpp_format_to_rga_format (mpp_format);
  struct gst_mpp_format *format;
  GstVideoInfo info;
  guint width = mpp_frame_get_width (mframe);
  guint height = mpp_frame_get_height (mframe);
  guint hstride = mpp_frame_get_hor_stride (mframe);
  guint vstride = mpp_frame_get_ver_stride (mframe);
  guint i;

  format = GST_MPP_GET_FORMAT (mpp, mpp_format & MPP_FRAME_FMT_MASK);
  if (!format || rga_format == RK_FORMAT_UNKNOWN || !mbuf)
    return (rga_buffer_t) {
    0};

  gst_video_info_init (&info);
  if (!gst_video_info_set_format (&info, format->gst_format, width, height))
    return (rga_buffer_t) {
    0};

  /* Overlay the MPP strides: plane 0 carries the byte hstride, the
   * uv plane follows at the vstride row.  Single-plane packed formats
   * carry the ver_stride through the wrap override: their gst form
   * has no uv offset to derive it from. */
  for (i = 0; i < GST_VIDEO_INFO_N_PLANES (&info); i++)
    GST_VIDEO_INFO_PLANE_STRIDE (&info, i) = hstride;
  if (GST_VIDEO_INFO_N_PLANES (&info) > 1)
    GST_VIDEO_INFO_PLANE_OFFSET (&info, 1) = (gsize) (hstride * vstride);

  return
      gst_rga_wrap_info_vs (gst_rga_format_index (GST_VIDEO_INFO_FORMAT
          (&info)), mpp_buffer_get_fd (mbuf), NULL, &info, vstride);
}

gboolean
gst_mpp_rga_convert (GstBuffer * inbuf, GstVideoInfo * src_vinfo,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo, gint rotation)
{
  GstMapInfo mapinfo = { 0, };
  rga_buffer_t src, dst;
  gboolean ret;
  gint fd = -1;
  void *vir_addr = NULL;

  /* Prefer using dma fd */
  if (gst_buffer_n_memory (inbuf) == 1) {
    GstMemory *mem = gst_buffer_peek_memory (inbuf, 0);
    gsize offset;

    if (gst_is_dmabuf_memory (mem)) {
      gst_memory_get_sizes (mem, &offset, NULL);
      if (!offset)
        fd = gst_dmabuf_memory_get_fd (mem);
    }
  }

  if (fd < 0) {
    gst_buffer_map (inbuf, &mapinfo, GST_MAP_READ);
    vir_addr = mapinfo.data;
  }

  src = gst_mpp_rga_buffer_from_info (src_vinfo, fd, vir_addr);
  dst = gst_mpp_rga_buffer_from_info (dst_vinfo,
      gst_dmabuf_memory_get_fd (out_mem), NULL);

  ret = src.wstride && dst.wstride &&
      gst_rga_process (&src, &dst, NULL, NULL,
      gst_mpp_rga_get_rotation (rotation)) > 0;

  gst_buffer_unmap (inbuf, &mapinfo);
  return ret;
}

gboolean
gst_mpp_rga_convert_from_mpp_frame (MppFrame mframe,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo, gint rotation)
{
  rga_buffer_t src, dst;

  src = gst_mpp_rga_buffer_from_mpp_frame (mframe);
  dst = gst_mpp_rga_buffer_from_info (dst_vinfo,
      gst_dmabuf_memory_get_fd (out_mem), NULL);

  if (!src.wstride || !dst.wstride)
    return FALSE;

  return gst_rga_process (&src, &dst, NULL, NULL,
      gst_mpp_rga_get_rotation (rotation)) > 0;
}

#else

gboolean
gst_mpp_use_rga ()
{
  return FALSE;
}

#endif
