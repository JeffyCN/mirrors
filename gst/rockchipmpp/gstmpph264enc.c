/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *     Author: Randy Li <randy.li@rock-chips.com>
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

#include "gstmpph264enc.h"

#define GST_CAT_DEFAULT mppvideoenc_debug

#define parent_class gst_mpp_h264_enc_parent_class
G_DEFINE_TYPE (GstMppH264Enc, gst_mpp_h264_enc, GST_TYPE_MPP_VIDEO_ENC);

static GstStaticPadTemplate gst_mpp_h264_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1080 ], "
        "framerate = (fraction) [0/1, 30/1], "
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, " "profile = (string) { high }")
    );

static gboolean
gst_mpp_h264_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  MppEncConfig mpp_cfg;

  memset (&mpp_cfg, 0, sizeof (mpp_cfg));

  mpp_cfg.size = sizeof (mpp_cfg);
  mpp_cfg.width = GST_VIDEO_INFO_WIDTH (&state->info);
  mpp_cfg.height = GST_VIDEO_INFO_HEIGHT (&state->info);
  mpp_cfg.rc_mode = 1;
  mpp_cfg.skip_cnt = 0;
  mpp_cfg.fps_in = GST_VIDEO_INFO_FPS_N (&state->info) /
      GST_VIDEO_INFO_FPS_D (&state->info);
  mpp_cfg.fps_out = GST_VIDEO_INFO_FPS_N (&state->info) /
      GST_VIDEO_INFO_FPS_D (&state->info);
  mpp_cfg.gop = GST_VIDEO_INFO_FPS_N (&state->info) /
      GST_VIDEO_INFO_FPS_D (&state->info);
  mpp_cfg.bps = mpp_cfg.width * mpp_cfg.height * 2 * mpp_cfg.fps_in;

  mpp_cfg.format = MPP_FMT_YUV420SP;
  mpp_cfg.qp = 24;
  mpp_cfg.profile = 100;
  mpp_cfg.level = 41;
  mpp_cfg.cabac_en = 0;

  return GST_MPP_VIDEO_ENC_CLASS (parent_class)->set_format (encoder, state,
      &mpp_cfg);
}

static GstFlowReturn
gst_mpp_h264_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstCaps *outcaps;
  GstStructure *structure;

  outcaps = gst_caps_new_empty_simple ("video/x-h264");
  structure = gst_caps_get_structure (outcaps, 0);
  gst_structure_set (structure, "stream-format",
      G_TYPE_STRING, "byte-stream", NULL);
  gst_structure_set (structure, "alignment", G_TYPE_STRING, "au", NULL);

  return GST_MPP_VIDEO_ENC_CLASS (parent_class)->handle_frame (encoder, frame,
      outcaps);
}

static void
gst_mpp_h264_enc_init (GstMppH264Enc * self)
{
}

static void
gst_mpp_h264_enc_class_init (GstMppH264EncClass * klass)
{
  GstElementClass *element_class;
  GstVideoEncoderClass *video_encoder_class;

  element_class = (GstElementClass *) klass;
  video_encoder_class = (GstVideoEncoderClass *) klass;

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp H264 Encoder",
      "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Randy Li <randy.li@rock-chips.com>");

  video_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_handle_frame);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_h264_enc_src_template));
}
