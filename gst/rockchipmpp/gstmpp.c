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
#include "gstmpp.h"
#include "gstmpph264enc.h"
#include "gstmppvp8enc.h"
#include "gstmppjpegenc.h"
#include "gstmppjpegdec.h"
#include "gstmppvideodec.h"

gboolean
gst_mpp_video_info_align (GstVideoInfo * info, guint hstride, guint vstride)
{
  GstVideoAlignment align;
  gint i, n_planes;
  guint stride;

  /* The MPP requires alignment 16 by default */
  if (!hstride)
    hstride = GST_ROUND_UP_16 (GST_VIDEO_INFO_PLANE_STRIDE (info, 0));

  if (!vstride)
    vstride = GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (info));

  GST_DEBUG ("aligning %dx%d to %dx%d", GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info), hstride, vstride);

  gst_video_alignment_reset (&align);

  /* Apply vstride */
  align.padding_bottom = vstride - info->height;
  if (!gst_video_info_align (info, &align))
    return FALSE;

  if (info->stride[0] == hstride)
    return TRUE;

  /* Apply hstride */
  stride = info->stride[0];
  n_planes = GST_VIDEO_INFO_N_PLANES (info);
  for (i = 0; i < n_planes; i++) {
    info->stride[i] = info->stride[i] * hstride / stride;
    info->offset[i] = info->offset[i] / stride * hstride;

    GST_DEBUG ("plane %d, stride %d, offset %" G_GSIZE_FORMAT,
        i, info->stride[i], info->offset[i]);
  }
  info->size = info->size / stride * hstride;

  GST_DEBUG ("aligned size %" G_GSIZE_FORMAT, info->size);

  return TRUE;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
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
