/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *     Author: Randy Li <randy.li@rock-chips.com>
 *
 * Copyright 2021 Rockchip Electronics Co., Ltd
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
#include "gstmpph264enc.h"
#include "gstmppvp8enc.h"
#include "gstmppjpegenc.h"
#include "gstmppjpegdec.h"
#include "gstmppvideodec.h"

GST_DEBUG_CATEGORY_STATIC (mpp_debug);
#define GST_CAT_DEFAULT mpp_debug

#define MPP_CASE_RETURN(a, b) case a: return b

GstVideoFormat
gst_mpp_mpp_format_to_gst_format (MppFrameFormat mpp_format)
{
  switch ((gint) mpp_format) {
      MPP_CASE_RETURN (MPP_FMT_YUV420P, GST_VIDEO_FORMAT_I420);
      MPP_CASE_RETURN (MPP_FMT_YUV420SP, GST_VIDEO_FORMAT_NV12);
      MPP_CASE_RETURN (MPP_FMT_YUV420SP_VU, GST_VIDEO_FORMAT_NV21);
#ifdef HAVE_NV12_10LE40
      MPP_CASE_RETURN (MPP_FMT_YUV420SP_10BIT, GST_VIDEO_FORMAT_NV12_10LE40);
#endif
      MPP_CASE_RETURN (MPP_FMT_YUV422P, GST_VIDEO_FORMAT_Y42B);
      MPP_CASE_RETURN (MPP_FMT_YUV422SP, GST_VIDEO_FORMAT_NV16);
      MPP_CASE_RETURN (MPP_FMT_YUV422SP_VU, GST_VIDEO_FORMAT_NV61);
      MPP_CASE_RETURN (MPP_FMT_YUV422_YUYV, GST_VIDEO_FORMAT_YUY2);
      MPP_CASE_RETURN (MPP_FMT_YUV422_YVYU, GST_VIDEO_FORMAT_YVYU);
      MPP_CASE_RETURN (MPP_FMT_YUV422_UYVY, GST_VIDEO_FORMAT_UYVY);
      MPP_CASE_RETURN (MPP_FMT_YUV422_VYUY, GST_VIDEO_FORMAT_VYUY);
      MPP_CASE_RETURN (MPP_FMT_RGB565LE, GST_VIDEO_FORMAT_RGB16);
      MPP_CASE_RETURN (MPP_FMT_BGR565LE, GST_VIDEO_FORMAT_BGR16);
      MPP_CASE_RETURN (MPP_FMT_RGB888, GST_VIDEO_FORMAT_RGB);
      MPP_CASE_RETURN (MPP_FMT_BGR888, GST_VIDEO_FORMAT_BGR);
      MPP_CASE_RETURN (MPP_FMT_ARGB8888, GST_VIDEO_FORMAT_ARGB);
      MPP_CASE_RETURN (MPP_FMT_ABGR8888, GST_VIDEO_FORMAT_ABGR);
      MPP_CASE_RETURN (MPP_FMT_RGBA8888, GST_VIDEO_FORMAT_RGBA);
      MPP_CASE_RETURN (MPP_FMT_BGRA8888, GST_VIDEO_FORMAT_BGRA);
    default:
      return GST_VIDEO_FORMAT_UNKNOWN;
  }
}

MppFrameFormat
gst_mpp_gst_format_to_mpp_format (GstVideoFormat format)
{
  switch (format) {
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_I420, MPP_FMT_YUV420P);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV12, MPP_FMT_YUV420SP);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV21, MPP_FMT_YUV420SP_VU);
#ifdef HAVE_NV12_10LE40
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV12_10LE40, MPP_FMT_YUV420SP_10BIT);
#endif
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_Y42B, MPP_FMT_YUV422P);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV16, MPP_FMT_YUV422SP);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV61, MPP_FMT_YUV422SP_VU);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_YUY2, MPP_FMT_YUV422_YUYV);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_YVYU, MPP_FMT_YUV422_YVYU);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_UYVY, MPP_FMT_YUV422_UYVY);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_VYUY, MPP_FMT_YUV422_VYUY);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGB16, MPP_FMT_RGB565LE);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGR16, MPP_FMT_BGR565LE);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGB, MPP_FMT_RGB888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGR, MPP_FMT_BGR888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_ARGB, MPP_FMT_ARGB8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_ABGR, MPP_FMT_ABGR8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGBA, MPP_FMT_RGBA8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGRA, MPP_FMT_BGRA8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_xRGB, MPP_FMT_ARGB8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_xBGR, MPP_FMT_ABGR8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGBx, MPP_FMT_RGBA8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGRx, MPP_FMT_BGRA8888);
    default:
      return MPP_FMT_BUTT;
  }
}

#ifdef HAVE_RGA
static RgaSURF_FORMAT
gst_mpp_mpp_format_to_rga_format (MppFrameFormat mpp_format)
{
  switch ((gint) mpp_format) {
      MPP_CASE_RETURN (MPP_FMT_YUV420P, RK_FORMAT_YCbCr_420_P);
      MPP_CASE_RETURN (MPP_FMT_YUV420SP, RK_FORMAT_YCbCr_420_SP);
      MPP_CASE_RETURN (MPP_FMT_YUV420SP_VU, RK_FORMAT_YCrCb_420_SP);
      MPP_CASE_RETURN (MPP_FMT_YUV420SP_10BIT, RK_FORMAT_YCbCr_420_SP_10B);
      MPP_CASE_RETURN (MPP_FMT_YUV422P, RK_FORMAT_YCbCr_422_P);
      MPP_CASE_RETURN (MPP_FMT_YUV422SP, RK_FORMAT_YCbCr_422_SP);
      MPP_CASE_RETURN (MPP_FMT_YUV422SP_VU, RK_FORMAT_YCrCb_422_SP);
      MPP_CASE_RETURN (MPP_FMT_RGB565LE, RK_FORMAT_RGB_565);
      MPP_CASE_RETURN (MPP_FMT_BGR888, RK_FORMAT_BGR_888);
      MPP_CASE_RETURN (MPP_FMT_RGB888, RK_FORMAT_RGB_888);
      MPP_CASE_RETURN (MPP_FMT_BGRA8888, RK_FORMAT_BGRA_8888);
      MPP_CASE_RETURN (MPP_FMT_RGBA8888, RK_FORMAT_RGBA_8888);
    default:
      return RK_FORMAT_UNKNOWN;
  }
}

static RgaSURF_FORMAT
gst_mpp_gst_format_to_rga_format (GstVideoFormat format)
{
  switch (format) {
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_I420, RK_FORMAT_YCbCr_420_P);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_YV12, RK_FORMAT_YCrCb_420_P);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV12, RK_FORMAT_YCbCr_420_SP);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV21, RK_FORMAT_YCrCb_420_SP);
#ifdef HAVE_NV12_10LE40
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV12_10LE40,
          RK_FORMAT_YCbCr_420_SP_10B);
#endif
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_Y42B, RK_FORMAT_YCbCr_422_P);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV16, RK_FORMAT_YCbCr_422_SP);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_NV61, RK_FORMAT_YCrCb_422_SP);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGB16, RK_FORMAT_RGB_565);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGR, RK_FORMAT_BGR_888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGB, RK_FORMAT_RGB_888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGRA, RK_FORMAT_BGRA_8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGBA, RK_FORMAT_RGBA_8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_BGRx, RK_FORMAT_BGRX_8888);
      MPP_CASE_RETURN (GST_VIDEO_FORMAT_RGBx, RK_FORMAT_RGBX_8888);
    default:
      return RK_FORMAT_UNKNOWN;
  }
}

static gboolean
gst_mpp_set_rga_info (rga_info_t * info, RgaSURF_FORMAT format,
    guint width, guint height, guint hstride, guint vstride)
{
  gint pixel_stride;

  switch (format) {
    case RK_FORMAT_RGBX_8888:
    case RK_FORMAT_BGRX_8888:
    case RK_FORMAT_RGBA_8888:
    case RK_FORMAT_BGRA_8888:
      pixel_stride = 4;
      break;
    case RK_FORMAT_RGB_888:
    case RK_FORMAT_BGR_888:
      pixel_stride = 3;
      break;
    case RK_FORMAT_RGB_565:
      pixel_stride = 2;
      break;
    case RK_FORMAT_YCbCr_420_SP_10B:
    case RK_FORMAT_YCbCr_422_SP:
    case RK_FORMAT_YCrCb_422_SP:
    case RK_FORMAT_YCbCr_422_P:
    case RK_FORMAT_YCrCb_422_P:
    case RK_FORMAT_YCbCr_420_SP:
    case RK_FORMAT_YCrCb_420_SP:
    case RK_FORMAT_YCbCr_420_P:
    case RK_FORMAT_YCrCb_420_P:
      pixel_stride = 1;

      /* RGA requires yuv image rect align to 2 */
      width &= ~1;
      height &= ~1;

      if (vstride % 2)
        return FALSE;
      break;
    default:
      return FALSE;
  }

  if (info->fd < 0 && !info->virAddr)
    return FALSE;

  /* HACK: The MPP might provide pixel stride in some cases */
  if (hstride / pixel_stride >= width)
    hstride /= pixel_stride;

  info->mmuFlag = 1;
  rga_set_rect (&info->rect, 0, 0, width, height, hstride, vstride, format);
  return TRUE;
}

static gboolean
gst_mpp_rga_info_from_mpp_frame (rga_info_t * info, MppFrame mframe)
{
  MppFrameFormat mpp_format = mpp_frame_get_fmt (mframe);
  MppBuffer mbuf = mpp_frame_get_buffer (mframe);
  guint width = mpp_frame_get_width (mframe);
  guint height = mpp_frame_get_height (mframe);
  guint hstride = mpp_frame_get_hor_stride (mframe);
  guint vstride = mpp_frame_get_ver_stride (mframe);
  RgaSURF_FORMAT rga_format = gst_mpp_mpp_format_to_rga_format (mpp_format);

  g_return_val_if_fail (mbuf, FALSE);

  info->fd = mpp_buffer_get_fd (mbuf);

  return gst_mpp_set_rga_info (info, rga_format, width, height,
      hstride, vstride);
}

static gboolean
gst_mpp_rga_info_from_video_info (rga_info_t * info, GstVideoInfo * vinfo)
{
  GstVideoFormat format = GST_VIDEO_INFO_FORMAT (vinfo);
  guint width = GST_VIDEO_INFO_WIDTH (vinfo);
  guint height = GST_VIDEO_INFO_HEIGHT (vinfo);
  guint hstride = GST_MPP_VIDEO_INFO_HSTRIDE (vinfo);
  guint vstride = GST_MPP_VIDEO_INFO_VSTRIDE (vinfo);
  RgaSURF_FORMAT rga_format = gst_mpp_gst_format_to_rga_format (format);

  return gst_mpp_set_rga_info (info, rga_format, width, height,
      hstride, vstride);
}

static gboolean
gst_mpp_rga_do_convert (rga_info_t * src_info, rga_info_t * dst_info)
{
  static gint rga_supported = 1;
  static gint rga_inited = 0;

  if (!rga_supported)
    return FALSE;

  if (!rga_inited) {
    if (c_RkRgaInit () < 0) {
      rga_supported = 0;
      GST_WARNING ("failed to init RGA");
      return FALSE;
    }
    rga_inited = 1;
  }

  if (c_RkRgaBlit (src_info, dst_info, NULL) < 0) {
    GST_WARNING ("failed to blit");
    return FALSE;
  }

  GST_DEBUG ("converted with RGA");
  return TRUE;
}

gboolean
gst_mpp_rga_convert (GstBuffer * inbuf, GstVideoInfo * src_vinfo,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo)
{
  GstMapInfo mapinfo = { 0, };
  gboolean ret;

  rga_info_t src_info = { 0, };
  rga_info_t dst_info = { 0, };

  if (!gst_mpp_rga_info_from_video_info (&src_info, src_vinfo))
    return FALSE;

  if (!gst_mpp_rga_info_from_video_info (&dst_info, dst_vinfo))
    return FALSE;

  /* Prefer using dma fd */
  if (gst_buffer_n_memory (inbuf) == 1) {
    GstMemory *mem = gst_buffer_peek_memory (inbuf, 0);
    gsize offset;

    if (gst_is_dmabuf_memory (mem)) {
      gst_memory_get_sizes (mem, &offset, NULL);
      if (!offset)
        src_info.fd = gst_dmabuf_memory_get_fd (mem);
    }
  }

  if (src_info.fd <= 0) {
    gst_buffer_map (inbuf, &mapinfo, GST_MAP_READ);
    src_info.virAddr = mapinfo.data;
  }

  dst_info.fd = gst_dmabuf_memory_get_fd (out_mem);

  ret = gst_mpp_rga_do_convert (&src_info, &dst_info);

  gst_buffer_unmap (inbuf, &mapinfo);
  return ret;
}

gboolean
gst_mpp_rga_convert_from_mpp_frame (MppFrame * mframe,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo)
{
  rga_info_t src_info = { 0, };
  rga_info_t dst_info = { 0, };

  if (!gst_mpp_rga_info_from_mpp_frame (&src_info, mframe))
    return FALSE;

  if (!gst_mpp_rga_info_from_video_info (&dst_info, dst_vinfo))
    return FALSE;

  dst_info.fd = gst_dmabuf_memory_get_fd (out_mem);

  return gst_mpp_rga_do_convert (&src_info, &dst_info);
}
#endif

gboolean
gst_mpp_video_info_align (GstVideoInfo * info, gint hstride, gint vstride)
{
  GstVideoAlignment align;
  guint stride;
  guint i;

  /* The MPP requires alignment 16 by default */
  if (!hstride)
    hstride = GST_ROUND_UP_16 (GST_VIDEO_INFO_PLANE_STRIDE (info, 0));

  if (!vstride)
    vstride = GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (info));

  GST_DEBUG ("aligning %dx%d to %dx%d", GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info), hstride, vstride);

  gst_video_alignment_reset (&align);

  /* Apply vstride */
  align.padding_bottom = vstride - GST_VIDEO_INFO_HEIGHT (info);
  if (!gst_video_info_align (info, &align))
    return FALSE;

  if (GST_VIDEO_INFO_PLANE_STRIDE (info, 0) == hstride)
    return TRUE;

  /* Apply hstride */
  stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 0);
  for (i = 0; i < GST_VIDEO_INFO_N_PLANES (info); i++) {
    GST_VIDEO_INFO_PLANE_STRIDE (info, i) =
        GST_VIDEO_INFO_PLANE_STRIDE (info, i) * hstride / stride;
    GST_VIDEO_INFO_PLANE_OFFSET (info, i) =
        GST_VIDEO_INFO_PLANE_OFFSET (info, i) / stride * hstride;

    GST_DEBUG ("plane %d, stride %d, offset %" G_GSIZE_FORMAT, i,
        GST_VIDEO_INFO_PLANE_STRIDE (info, i),
        GST_VIDEO_INFO_PLANE_OFFSET (info, i));
  }
  GST_VIDEO_INFO_SIZE (info) = GST_VIDEO_INFO_SIZE (info) / stride * hstride;

  GST_DEBUG ("aligned size %" G_GSIZE_FORMAT, GST_VIDEO_INFO_SIZE (info));

  return TRUE;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mpp", 0, "MPP");

  if (!gst_element_register (plugin, "mppvideodec", GST_RANK_PRIMARY + 1,
          gst_mpp_video_dec_get_type ()))
    return FALSE;

  if (!gst_element_register (plugin, "mpph264enc", GST_RANK_PRIMARY + 1,
          gst_mpp_h264_enc_get_type ()))
    return FALSE;

  if (!gst_element_register (plugin, "mppvp8enc", GST_RANK_PRIMARY + 1,
          gst_mpp_vp8_enc_get_type ()))
    return FALSE;

  if (!gst_element_register (plugin, "mppjpegenc", GST_RANK_PRIMARY + 1,
          gst_mpp_jpeg_enc_get_type ()))
    return FALSE;

  if (!gst_element_register (plugin, "mppjpegdec", GST_RANK_PRIMARY + 1,
          gst_mpp_jpeg_dec_get_type ()))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rockchipmpp,
    "Rockchip Mpp Video Plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
