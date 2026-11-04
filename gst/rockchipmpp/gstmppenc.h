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

#ifndef  __GST_MPP_ENC_H__
#define  __GST_MPP_ENC_H__

#include <gst/video/gstvideoencoder.h>

#include "gstmpp.h"

G_BEGIN_DECLS;

#define GST_TYPE_MPP_ENC (gst_mpp_enc_get_type())
G_DECLARE_FINAL_TYPE (GstMppEnc, gst_mpp_enc, GST, MPP_ENC, GstVideoEncoder);

#define GST_MPP_ENC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_ENC, GstMppEnc))

struct _GstMppEnc
{
  GstVideoEncoder parent;

  GMutex mutex;
  GstAllocator *allocator;
  GstVideoCodecState *input_state;

  /* final input video info */
  GstVideoInfo info;

  /* stop handling new frame when flushing */
  gboolean flushing;

  /* drop frames when flushing but not draining */
  gboolean draining;

  /* frame system numbers that are ready for sending to MPP */
  GList *frames;

  /* Max number of pending frames */
  guint32 max_pending;

  guint pending_frames;
  GMutex event_mutex;
  GCond event_cond;

  /* flow return from pad task */
  GstFlowReturn task_ret;

  MppEncHeaderMode header_mode;
  MppEncRcMode rc_mode;
  MppEncSeiMode sei_mode;

  gint rotation;
  gint width;
  gint height;

  gint gop;
  guint max_reenc;

  guint bps;
  guint bps_min;
  guint bps_max;

  gboolean zero_copy_pkt;

  /* Unwrap the legacy IVF container framing some MPP codec HALs (VP8)
   * wrap around the encoded stream. Set by the subclass in init(). */
  gboolean strip_ivf;

  /* IVF framing detection state, valid after the first packet */
  gboolean ivf_checked;
  gboolean ivf_output;

  gboolean arm_afbc;
  gboolean prop_arm_afbc;
  gboolean format_configured;

  gboolean prop_dirty;

  MppEncCfg mpp_cfg;
  MppFrame mpp_frame;

  MppCodingType mpp_type;
  MppCtx mpp_ctx;
  MppApi *mpi;
};

/* NOTE: the vepu on rk3588 cannot encode the 10-bit YUV input formats,
 * so they are not advertised here. */
#define MPP_ENC_IN_FORMATS \
    "NV12, I420, YUY2, UYVY, " \
    "BGR16, RGB16, BGR, RGB, " \
    "ABGR, ARGB, BGRA, RGBA, xBGR, xRGB, BGRx, RGBx"

#ifdef HAVE_RGA
#define MPP_ENC_FORMATS MPP_ENC_IN_FORMATS "," GST_RGA_FORMATS
#else
#define MPP_ENC_FORMATS MPP_ENC_IN_FORMATS
#endif

/* Standard dma-drm form of the AFBC input; the modifier literal must
 * stay in sync with GST_MPP_AFBC_MODIFIER and the fourccs with
 * gst_mpp_enc_video_format_from_afbc_fourcc(). */
#define MPP_ENC_CAPS_DMA_DRM_AFBC \
    "video/x-raw(memory:DMABuf), format = (string) DMA_DRM, " \
    "drm-format = (string) { " \
    "YU08:0x0800000000000041, YUYV:0x0800000000000041 }, " \
    "width = (int) [ 1, MAX ], height = (int) [ 1, MAX ], " \
    "framerate = (fraction) [ 0/1, MAX ]"

gboolean gst_mpp_enc_apply_properties (GstVideoEncoder * encoder);
gboolean gst_mpp_enc_set_src_caps (GstVideoEncoder * encoder, GstCaps * caps);

gboolean gst_mpp_enc_supported (MppCodingType mpp_type);

G_END_DECLS;

#endif /* __GST_MPP_ENC_H__ */
