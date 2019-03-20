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
        "height = (int) [ 32, 1088 ], "
        "framerate = (fraction) [0/1, 60/1], "
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, " "profile = (string) { high }")
    );

static gboolean
gst_mpp_h264_enc_open (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  GST_DEBUG_OBJECT (self, "Opening");

  if (mpp_create (&self->mpp_ctx, &self->mpi))
    goto failure;
  if (mpp_init (self->mpp_ctx, MPP_CTX_ENC, MPP_VIDEO_CodingAVC))
    goto failure;

  return TRUE;

failure:
  return FALSE;
}

static gboolean
gst_mpp_h264_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstMppVideoEnc *mpp_video_enc = GST_MPP_VIDEO_ENC (encoder);
  MppEncCodecCfg codec_cfg;
  MppEncRcCfg rc_cfg;

  memset (&rc_cfg, 0, sizeof (rc_cfg));
  memset (&codec_cfg, 0, sizeof (codec_cfg));

  rc_cfg.change = MPP_ENC_RC_CFG_CHANGE_ALL;
  rc_cfg.rc_mode = MPP_ENC_RC_MODE_CBR;
  rc_cfg.quality = MPP_ENC_RC_QUALITY_MEDIUM;

  rc_cfg.fps_in_flex = 0;
  rc_cfg.fps_in_num = GST_VIDEO_INFO_FPS_N (&state->info);
  rc_cfg.fps_in_denorm = GST_VIDEO_INFO_FPS_D (&state->info);
  rc_cfg.fps_out_flex = 0;
  rc_cfg.fps_out_num = GST_VIDEO_INFO_FPS_N (&state->info);
  rc_cfg.fps_out_denorm = GST_VIDEO_INFO_FPS_D (&state->info);
  rc_cfg.gop = GST_VIDEO_INFO_FPS_N (&state->info)
      / GST_VIDEO_INFO_FPS_D (&state->info);
  rc_cfg.skip_cnt = 0;

  codec_cfg.h264.qp_init = 26;

  if (rc_cfg.rc_mode == MPP_ENC_RC_MODE_CBR) {
    codec_cfg.h264.qp_max = 28;
    codec_cfg.h264.qp_min = 4;
    codec_cfg.h264.qp_max_step = 8;

    /* Bits of a GOP */
    rc_cfg.bps_target = GST_VIDEO_INFO_WIDTH (&state->info)
        * GST_VIDEO_INFO_HEIGHT (&state->info)
        / 8 * GST_VIDEO_INFO_FPS_N (&state->info)
        / GST_VIDEO_INFO_FPS_D (&state->info);
    rc_cfg.bps_max = rc_cfg.bps_target * 17 / 16;
    rc_cfg.bps_min = rc_cfg.bps_target * 15 / 16;
  } else if (rc_cfg.rc_mode == MPP_ENC_RC_MODE_VBR) {
    if (rc_cfg.quality == MPP_ENC_RC_QUALITY_CQP) {
      codec_cfg.h264.qp_max = 26;
      codec_cfg.h264.qp_min = 26;
      codec_cfg.h264.qp_max_step = 0;

      rc_cfg.bps_target = -1;
      rc_cfg.bps_max = -1;
      rc_cfg.bps_min = -1;
    } else {
      codec_cfg.h264.qp_max = 40;
      codec_cfg.h264.qp_min = 12;
      codec_cfg.h264.qp_max_step = 0;
      codec_cfg.h264.qp_init = 0;

      rc_cfg.bps_target = GST_VIDEO_INFO_WIDTH (&state->info)
          * GST_VIDEO_INFO_HEIGHT (&state->info)
          / 8 * GST_VIDEO_INFO_FPS_N (&state->info)
          / GST_VIDEO_INFO_FPS_D (&state->info);
      rc_cfg.bps_max = rc_cfg.bps_target * 17 / 16;
      rc_cfg.bps_min = rc_cfg.bps_target * 1 / 16;
    }
  }

  if (mpp_video_enc->mpi->control (mpp_video_enc->mpp_ctx, MPP_ENC_SET_RC_CFG,
          &rc_cfg)) {
    GST_DEBUG_OBJECT (self, "Setting rate control for rockchip mpp failed");
    return FALSE;
  }

  codec_cfg.coding = MPP_VIDEO_CodingAVC;
  codec_cfg.h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
      MPP_ENC_H264_CFG_CHANGE_ENTROPY |
      MPP_ENC_H264_CFG_CHANGE_TRANS_8x8 | MPP_ENC_H264_CFG_CHANGE_QP_LIMIT;
  codec_cfg.h264.profile = 100;
  codec_cfg.h264.level = 40;
  codec_cfg.h264.entropy_coding_mode = 1;
  codec_cfg.h264.cabac_init_idc = 0;
  codec_cfg.h264.transform8x8_mode = 1;

  if (mpp_video_enc->mpi->control (mpp_video_enc->mpp_ctx,
          MPP_ENC_SET_CODEC_CFG, &codec_cfg)) {
    GST_DEBUG_OBJECT (self, "Setting codec info for rockchip mpp failed");
    return FALSE;
  }

  return GST_MPP_VIDEO_ENC_CLASS (parent_class)->set_format (encoder, state);
}

static GstFlowReturn
gst_mpp_h264_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
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

  video_encoder_class->open = GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_open);
  video_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_handle_frame);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_h264_enc_src_template));
}
