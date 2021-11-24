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
#include "gstmpph265enc.h"
#include "gstmppvp8enc.h"
#include "gstmppjpegenc.h"
#include "gstmppjpegdec.h"
#include "gstmppvideodec.h"
#include "gstmppvpxalphadecodebin.h"

GST_DEBUG_CATEGORY_STATIC (mpp_debug);
#define GST_CAT_DEFAULT mpp_debug

struct gst_mpp_format
{
  GstVideoFormat gst_format;
  MppFrameFormat mpp_format;
#ifdef HAVE_RGA
  RgaSURF_FORMAT rga_format;
#endif
  gint stride;
  gboolean is_yuv;
};

#ifdef HAVE_RGA
#define GST_MPP_FORMAT(gst, mpp, rga, stride, yuv) \
  { GST_VIDEO_FORMAT_ ## gst, MPP_FMT_ ## mpp, RK_FORMAT_ ## rga, stride, yuv }
#else
#define GST_MPP_FORMAT(gst, mpp, rga, stride, yuv) \
  { GST_VIDEO_FORMAT_ ## gst, MPP_FMT_ ## mpp, stride, yuv}
#endif

struct gst_mpp_format gst_mpp_formats[] = {
  GST_MPP_FORMAT (I420, YUV420P, YCbCr_420_P, 1, 1),
  GST_MPP_FORMAT (YV12, BUTT, YCrCb_420_P, 1, 1),
  GST_MPP_FORMAT (NV12, YUV420SP, YCbCr_420_SP, 1, 1),
  GST_MPP_FORMAT (NV21, YUV420SP_VU, YCrCb_420_SP, 1, 1),
#ifdef HAVE_NV12_10LE40
  GST_MPP_FORMAT (NV12_10LE40, YUV420SP_10BIT, YCbCr_420_SP_10B, 1, 1),
#else
  GST_MPP_FORMAT (UNKNOWN, YUV420SP_10BIT, YCbCr_420_SP_10B, 1, 1),
#endif
  GST_MPP_FORMAT (Y42B, YUV422P, YCbCr_422_P, 1, 1),
  GST_MPP_FORMAT (NV16, YUV422SP, YCbCr_422_SP, 1, 1),
  GST_MPP_FORMAT (NV61, YUV422SP_VU, YCrCb_422_SP, 1, 1),
  GST_MPP_FORMAT (YUY2, YUV422_YUYV, UNKNOWN, 1, 1),
  GST_MPP_FORMAT (YVYU, YUV422_YVYU, UNKNOWN, 1, 1),
  GST_MPP_FORMAT (UYVY, YUV422_UYVY, UNKNOWN, 1, 1),
  GST_MPP_FORMAT (VYUY, YUV422_VYUY, UNKNOWN, 1, 1),
  GST_MPP_FORMAT (RGB16, RGB565LE, UNKNOWN, 2, 0),
  GST_MPP_FORMAT (BGR16, BGR565LE, RGB_565, 2, 0),
  GST_MPP_FORMAT (RGB, RGB888, RGB_888, 3, 0),
  GST_MPP_FORMAT (BGR, BGR888, BGR_888, 3, 0),
  GST_MPP_FORMAT (ARGB, ARGB8888, UNKNOWN, 4, 0),
  GST_MPP_FORMAT (ABGR, ABGR8888, UNKNOWN, 4, 0),
  GST_MPP_FORMAT (RGBA, RGBA8888, RGBA_8888, 4, 0),
  GST_MPP_FORMAT (BGRA, BGRA8888, BGRA_8888, 4, 0),
  GST_MPP_FORMAT (xRGB, ARGB8888, UNKNOWN, 4, 0),
  GST_MPP_FORMAT (xBGR, ABGR8888, UNKNOWN, 4, 0),
  GST_MPP_FORMAT (RGBx, RGBA8888, RGBX_8888, 4, 0),
  GST_MPP_FORMAT (BGRx, BGRA8888, BGRX_8888, 4, 0),
};

#define GST_MPP_GET_FORMAT(type, format) ({ \
  struct gst_mpp_format *_tmp; \
  for (guint i = 0; i < ARRAY_SIZE (gst_mpp_formats) || (_tmp = NULL); i++) { \
    _tmp = &gst_mpp_formats[i]; \
    if (_tmp->type ## _format == format) break;\
  }; _tmp; \
})

GstVideoFormat
gst_mpp_mpp_format_to_gst_format (MppFrameFormat mpp_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (mpp, mpp_format);
  return format ? format->gst_format : GST_VIDEO_FORMAT_UNKNOWN;
}

MppFrameFormat
gst_mpp_gst_format_to_mpp_format (GstVideoFormat gst_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (gst, gst_format);
  return format ? format->mpp_format : MPP_FMT_BUTT;
}

#ifdef HAVE_RGA
static RgaSURF_FORMAT
gst_mpp_mpp_format_to_rga_format (MppFrameFormat mpp_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (mpp, mpp_format);
  return format ? format->rga_format : RK_FORMAT_UNKNOWN;
}

static RgaSURF_FORMAT
gst_mpp_gst_format_to_rga_format (GstVideoFormat gst_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (gst, gst_format);
  return format ? format->rga_format : RK_FORMAT_UNKNOWN;
}

static gboolean
gst_mpp_set_rga_info (rga_info_t * info, RgaSURF_FORMAT rga_format,
    guint width, guint height, guint hstride, guint vstride)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (rga, rga_format);

  g_assert (format);
  if (format->is_yuv) {
    /* RGA requires yuv image rect align to 2 */
    width &= ~1;
    height &= ~1;

    if (vstride % 2)
      return FALSE;
  }

  if (info->fd < 0 && !info->virAddr)
    return FALSE;

  /* HACK: The MPP might provide pixel stride in some cases */
  if (hstride / format->stride >= width)
    hstride /= format->stride;

  info->mmuFlag = 1;
  rga_set_rect (&info->rect, 0, 0, width, height, hstride, vstride, rga_format);
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

static gint
gst_mpp_rga_get_rotation (gint rotation)
{
  switch (rotation) {
    case 0:
      return 0;
    case 90:
      return HAL_TRANSFORM_ROT_90;
    case 180:
      return HAL_TRANSFORM_ROT_180;
    case 270:
      return HAL_TRANSFORM_ROT_270;
    default:
      return -1;
  }
}

gboolean
gst_mpp_rga_convert (GstBuffer * inbuf, GstVideoInfo * src_vinfo,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo, gint rotation)
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

  src_info.rotation = gst_mpp_rga_get_rotation (rotation);
  if (src_info.rotation < 0)
    return FALSE;

  ret = gst_mpp_rga_do_convert (&src_info, &dst_info);

  gst_buffer_unmap (inbuf, &mapinfo);
  return ret;
}

gboolean
gst_mpp_rga_convert_from_mpp_frame (MppFrame * mframe,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo, gint rotation)
{
  rga_info_t src_info = { 0, };
  rga_info_t dst_info = { 0, };

  if (!gst_mpp_rga_info_from_mpp_frame (&src_info, mframe))
    return FALSE;

  if (!gst_mpp_rga_info_from_video_info (&dst_info, dst_vinfo))
    return FALSE;

  dst_info.fd = gst_dmabuf_memory_get_fd (out_mem);

  src_info.rotation = gst_mpp_rga_get_rotation (rotation);
  if (src_info.rotation < 0)
    return FALSE;

  return gst_mpp_rga_do_convert (&src_info, &dst_info);
}
#endif

gboolean
gst_mpp_video_info_align (GstVideoInfo * info, gint hstride, gint vstride)
{
  GstVideoAlignment align;
  guint stride;
  guint i;

  if (!hstride)
    hstride = GST_MPP_ALIGN (GST_MPP_VIDEO_INFO_HSTRIDE (info));

  if (!vstride)
    vstride = GST_MPP_ALIGN (GST_MPP_VIDEO_INFO_VSTRIDE (info));

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

  if (!gst_element_register (plugin, "mpph265enc", GST_RANK_PRIMARY + 1,
          gst_mpp_h265_enc_get_type ()))
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

  /* Both codecalphademux and alphacombine elements were added in 1.19 */
  if (GST_VERSION_MAJOR == 1 && GST_VERSION_MINOR >= 19) {
    if (!gst_element_register (plugin, "mppvpxalphadecodebin",
            GST_RANK_PRIMARY + GST_MPP_ALPHA_DECODE_BIN_RANK_OFFSET,
            gst_mpp_vpx_alpha_decode_bin_get_type ()))
      return FALSE;
  }

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rockchipmpp,
    "Rockchip Mpp Video Plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
