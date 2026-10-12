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

#ifdef HAVE_RGA
#define GST_MPP_FORMAT(gst, mpp, rga, pixel_stride0, yuv) \
  { GST_VIDEO_FORMAT_ ## gst, MPP_FMT_ ## mpp, RK_FORMAT_ ## rga, \
    pixel_stride0, yuv }
#else
#define GST_MPP_FORMAT(gst, mpp, rga, pixel_stride0, yuv) \
  { GST_VIDEO_FORMAT_ ## gst, MPP_FMT_ ## mpp, pixel_stride0, yuv}
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
#ifdef HAVE_NV16_10LE40
  GST_MPP_FORMAT (NV16_10LE40, YUV422SP_10BIT, YCbCr_422_SP_10B, 1, 1),
#else
  GST_MPP_FORMAT (UNKNOWN, YUV422SP_10BIT, YCbCr_422_SP_10B, 1, 1),
#endif
  GST_MPP_FORMAT (Y42B, YUV422P, YCbCr_422_P, 1, 1),
  GST_MPP_FORMAT (NV16, YUV422SP, YCbCr_422_SP, 1, 1),
  GST_MPP_FORMAT (NV61, YUV422SP_VU, YCrCb_422_SP, 1, 1),
  GST_MPP_FORMAT (NV24, YUV444SP, UNKNOWN, 1, 1),
  GST_MPP_FORMAT (Y444, YUV444P, UNKNOWN, 1, 1),
  GST_MPP_FORMAT (YUY2, YUV422_YUYV, UNKNOWN, 2, 1),
  GST_MPP_FORMAT (YVYU, YUV422_YVYU, UNKNOWN, 2, 1),
  GST_MPP_FORMAT (UYVY, YUV422_UYVY, UNKNOWN, 2, 1),
  GST_MPP_FORMAT (VYUY, YUV422_VYUY, UNKNOWN, 2, 1),
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
  /* sentinel */
  GST_MPP_FORMAT (UNKNOWN, BUTT, UNKNOWN, 0, 0),
};

const gchar *
gst_mpp_video_format_to_string (GstVideoFormat format)
{
  if (format == GST_VIDEO_FORMAT_UNKNOWN)
    return "UNKNOWN";

  return gst_video_format_to_string (format);
}

GstVideoFormat
gst_mpp_mpp_format_to_gst_format (MppFrameFormat mpp_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (mpp,
      mpp_format & MPP_FRAME_FMT_MASK);
  return format ? format->gst_format : GST_VIDEO_FORMAT_UNKNOWN;
}

MppFrameFormat
gst_mpp_gst_format_to_mpp_format (GstVideoFormat gst_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (gst, gst_format);
  return format ? format->mpp_format : MPP_FMT_BUTT;
}

#ifdef HAVE_RGA
RgaSURF_FORMAT
gst_mpp_mpp_format_to_rga_format (MppFrameFormat mpp_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (mpp,
      mpp_format & MPP_FRAME_FMT_MASK);
  return format ? format->rga_format : RK_FORMAT_UNKNOWN;
}

RgaSURF_FORMAT
gst_mpp_gst_format_to_rga_format (GstVideoFormat gst_format)
{
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (gst, gst_format);
  return format ? format->rga_format : RK_FORMAT_UNKNOWN;
}

#endif

void
gst_mpp_video_info_update_format (GstVideoInfo * info, GstVideoFormat format,
    guint width, guint height)
{
  GstCaps *caps;
  const gchar *fmt = gst_mpp_video_format_to_string (format);

  GST_DEBUG ("update format to %s (%dx%d)", fmt, width, height);

  if (GST_VIDEO_INFO_FORMAT (info) == GST_VIDEO_FORMAT_UNKNOWN) {
    gst_video_info_set_format (info, format, width, height);
    return;
  }

  caps = gst_video_info_to_caps (info);
  gst_caps_set_simple (caps, "format", G_TYPE_STRING, fmt,
      "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, NULL);
  gst_video_info_from_caps (info, caps);
  gst_caps_unref (caps);
}

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

  if (hstride == GST_MPP_VIDEO_INFO_HSTRIDE (info) &&
      vstride == GST_MPP_VIDEO_INFO_VSTRIDE (info))
    return TRUE;

  GST_DEBUG ("aligning %dx%d to %dx%d", GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info), hstride, vstride);

  if (hstride < GST_VIDEO_INFO_WIDTH (info) ||
      vstride < GST_VIDEO_INFO_HEIGHT (info)) {
    GST_ERROR ("unable to align %dx%d to %dx%d", GST_VIDEO_INFO_WIDTH (info),
        GST_VIDEO_INFO_HEIGHT (info), hstride, vstride);
    return FALSE;
  }

  gst_video_alignment_reset (&align);

  /* Apply vstride */
  align.padding_bottom = vstride - GST_VIDEO_INFO_HEIGHT (info);
  if (!gst_video_info_align (info, &align))
    return FALSE;

  /* Apply vstride for single-plane */
  if (GST_VIDEO_INFO_N_PLANES (info) == 1)
    GST_VIDEO_INFO_SIZE (info) =
        GST_VIDEO_INFO_PLANE_STRIDE (info, 0) * vstride;

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

gboolean
gst_mpp_video_info_matched (GstVideoInfo * info, GstVideoInfo * other)
{
  guint i;

  if (GST_VIDEO_INFO_FORMAT (info) != GST_VIDEO_INFO_FORMAT (other))
    return FALSE;

  if (GST_VIDEO_INFO_WIDTH (info) != GST_VIDEO_INFO_WIDTH (other))
    return FALSE;

  if (GST_VIDEO_INFO_HEIGHT (info) != GST_VIDEO_INFO_HEIGHT (other))
    return FALSE;

  for (i = 0; i < GST_VIDEO_INFO_N_PLANES (info); i++) {
    if (GST_VIDEO_INFO_PLANE_STRIDE (info,
            i) != GST_VIDEO_INFO_PLANE_STRIDE (other, i))
      return FALSE;
    if (GST_VIDEO_INFO_PLANE_OFFSET (info,
            i) != GST_VIDEO_INFO_PLANE_OFFSET (other, i))
      return FALSE;
  }

  return TRUE;
}

gboolean
gst_mpp_frame_info_changed (MppFrame * frame, MppFrame * other)
{
  if (!frame || !other)
    return TRUE;

  if (mpp_frame_get_fmt (frame) != mpp_frame_get_fmt (other))
    return TRUE;

  if (mpp_frame_get_width (frame) != mpp_frame_get_width (other))
    return TRUE;

  if (mpp_frame_get_height (frame) != mpp_frame_get_height (other))
    return TRUE;

  if (mpp_frame_get_offset_x (frame) != mpp_frame_get_offset_x (other))
    return TRUE;

  if (mpp_frame_get_offset_y (frame) != mpp_frame_get_offset_y (other))
    return TRUE;

  if (mpp_frame_get_hor_stride (frame) != mpp_frame_get_hor_stride (other))
    return TRUE;

  if (mpp_frame_get_ver_stride (frame) != mpp_frame_get_ver_stride (other))
    return TRUE;

  return FALSE;
}

guint
gst_mpp_get_pixel_stride (GstVideoInfo * info)
{
  GstVideoFormat gst_format = GST_VIDEO_INFO_FORMAT (info);
  struct gst_mpp_format *format = GST_MPP_GET_FORMAT (gst, gst_format);
  guint hstride = GST_MPP_VIDEO_INFO_HSTRIDE (info);

  if (!format)
    return hstride;

  return hstride / format->pixel_stride0;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mpp", 0, "MPP");

  gst_mpp_h264_enc_register (plugin, GST_RANK_PRIMARY + 1);
  gst_mpp_h265_enc_register (plugin, GST_RANK_PRIMARY + 1);
  gst_mpp_vp8_enc_register (plugin, GST_RANK_PRIMARY + 1);
  gst_mpp_jpeg_enc_register (plugin, GST_RANK_PRIMARY + 1);

  gst_mpp_video_dec_register (plugin, GST_RANK_PRIMARY + 1);
  gst_mpp_jpeg_dec_register (plugin, GST_RANK_PRIMARY + 1);

#ifdef USE_VPXALPHADEC
  gst_mpp_vpx_alpha_decode_bin_register (plugin,
      GST_RANK_PRIMARY + GST_MPP_ALPHA_DECODE_BIN_RANK_OFFSET);
#endif

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rockchipmpp,
    "Rockchip Mpp Video Plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
