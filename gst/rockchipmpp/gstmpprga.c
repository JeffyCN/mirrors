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

static gboolean
gst_mpp_rga_do_convert (rga_buffer_t * src, rga_buffer_t * dst, gint rotation)
{
  im_rect srect = { 0, };
  im_rect drect = { 0, };
  im_rect prect = { 0, };
  IM_STATUS ret;

  if (!gst_mpp_use_rga ())
    return FALSE;

  ret = improcess (*src, *dst, (rga_buffer_t) {
      0}
      , srect, drect, prect, gst_mpp_rga_get_rotation (rotation));
  if (ret <= 0) {
    GST_WARNING ("failed to blit: %s", imStrError (ret));
    return FALSE;
  }

  GST_DEBUG ("converted with RGA");
  return TRUE;
}

/* Build an rga_buffer_t over a dma fd or a mapped virtual address.
 * For the packed 10-bit formats the horizontal stride is in bytes,
 * which is what the driver expects in wstride. */
static rga_buffer_t
gst_mpp_rga_buffer (RgaSURF_FORMAT rga_format, gint fd, void *vir_addr,
    guint width, guint height, guint hstride, guint vstride)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (rga, rga_format);

  if (!format || rga_format == RK_FORMAT_UNKNOWN)
    return (rga_buffer_t) {
    0};

  if (format->is_yuv) {
    /* RGA requires yuv image rects to be 2-aligned: reject odd sizes so
     * the caller falls back to the GPU tier instead of silently writing
     * a truncated frame */
    if (width % 2 || height % 2 || vstride % 2)
      return (rga_buffer_t) {
      0};
  }

  if (fd >= 0)
    return wrapbuffer_fd_t (fd, width, height, hstride, vstride, rga_format);

  if (vir_addr)
    return wrapbuffer_virtualaddr_t (vir_addr, width, height, hstride,
        vstride, rga_format);

  return (rga_buffer_t) {
  0};
}

static rga_buffer_t
gst_mpp_rga_buffer_from_mpp_frame (MppFrame mframe)
{
  MppFrameFormat mpp_format = mpp_frame_get_fmt (mframe);
  MppBuffer mbuf = mpp_frame_get_buffer (mframe);
  struct gst_mpp_format *format;
  guint width = mpp_frame_get_width (mframe);
  guint height = mpp_frame_get_height (mframe);
  guint hstride = mpp_frame_get_hor_stride (mframe);
  guint vstride = mpp_frame_get_ver_stride (mframe);
  RgaSURF_FORMAT rga_format = gst_mpp_mpp_format_to_rga_format (mpp_format);

  format = GST_MPP_GET_FORMAT (mpp, mpp_format & MPP_FRAME_FMT_MASK);
  if (format)
    hstride = hstride / format->pixel_stride0;

  if (!mbuf)
    return (rga_buffer_t) {
    0};

  return gst_mpp_rga_buffer (rga_format, mpp_buffer_get_fd (mbuf), NULL,
      width, height, hstride, vstride);
}

static rga_buffer_t
gst_mpp_rga_buffer_from_video_info (GstVideoInfo * vinfo, gint fd,
    void *vir_addr)
{
  GstVideoFormat format = GST_VIDEO_INFO_FORMAT (vinfo);
  guint width = GST_VIDEO_INFO_WIDTH (vinfo);
  guint height = GST_VIDEO_INFO_HEIGHT (vinfo);
  guint hstride = gst_mpp_get_pixel_stride (vinfo);
  guint vstride = GST_MPP_VIDEO_INFO_VSTRIDE (vinfo);

  return gst_mpp_rga_buffer (gst_mpp_gst_format_to_rga_format (format),
      fd, vir_addr, width, height, hstride, vstride);
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

  src = gst_mpp_rga_buffer_from_video_info (src_vinfo, fd, vir_addr);
  dst = gst_mpp_rga_buffer_from_video_info (dst_vinfo,
      gst_dmabuf_memory_get_fd (out_mem), NULL);

  ret = src.wstride && dst.wstride &&
      gst_mpp_rga_do_convert (&src, &dst, rotation);

  gst_buffer_unmap (inbuf, &mapinfo);
  return ret;
}

gboolean
gst_mpp_rga_convert_from_mpp_frame (MppFrame mframe,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo, gint rotation)
{
  rga_buffer_t src, dst;

  src = gst_mpp_rga_buffer_from_mpp_frame (mframe);
  dst = gst_mpp_rga_buffer_from_video_info (dst_vinfo,
      gst_dmabuf_memory_get_fd (out_mem), NULL);

  if (!src.wstride || !dst.wstride)
    return FALSE;

  return gst_mpp_rga_do_convert (&src, &dst, rotation);
}

#endif
