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

#define DEFAULT_PROP_LEVEL 40   /* 1080p@30fps */
#define DEFAULT_PROP_PROFILE GST_MPP_H264_PROFILE_HIGH
#define DEFAULT_PROP_QP_INIT 26
#define DEFAULT_PROP_QP_MIN 0   /* Auto */
#define DEFAULT_PROP_QP_MAX 0   /* Auto */
#define DEFAULT_PROP_QP_MAX_STEP -1     /* Auto */

enum
{
  PROP_0,
  PROP_PROFILE,
  PROP_LEVEL,
  PROP_QP_INIT,
  PROP_QP_MIN,
  PROP_QP_MAX,
  PROP_QP_MAX_STEP,
  PROP_LAST,
};

static GstStaticPadTemplate gst_mpp_h264_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1088 ], "
        "framerate = (fraction) [0/1, 60/1], "
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, "
        "profile = (string) { baseline, main, high }")
    );

static void
gst_mpp_h264_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);

  switch (prop_id) {
    case PROP_PROFILE:{
      GstMppH264Profile profile = g_value_get_enum (value);
      if (self->profile == profile)
        return;

      self->profile = profile;
      break;
    }
    case PROP_LEVEL:{
      guint level = g_value_get_uint (value);
      if (self->level == level)
        return;

      self->level = level;
      break;
    }
    case PROP_QP_INIT:{
      guint qp_init = g_value_get_uint (value);
      if (self->qp_init == qp_init)
        return;

      self->qp_init = qp_init;
      break;
    }
    case PROP_QP_MIN:{
      guint qp_min = g_value_get_uint (value);
      if (self->qp_min == qp_min)
        return;

      self->qp_min = qp_min;
      break;
    }
    case PROP_QP_MAX:{
      guint qp_max = g_value_get_uint (value);
      if (self->qp_max == qp_max)
        return;

      self->qp_max = qp_max;
      break;
    }
    case PROP_QP_MAX_STEP:{
      gint qp_max_step = g_value_get_int (value);
      if (self->qp_max_step == qp_max_step)
        return;

      self->qp_max_step = qp_max_step;
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }

  self->prop_dirty = TRUE;
}

static void
gst_mpp_h264_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);

  switch (prop_id) {
    case PROP_PROFILE:
      g_value_set_enum (value, self->profile);
      break;
    case PROP_LEVEL:
      g_value_set_uint (value, self->level);
      break;
    case PROP_QP_INIT:
      g_value_set_uint (value, self->qp_init);
      break;
    case PROP_QP_MIN:
      g_value_set_uint (value, self->qp_min);
      break;
    case PROP_QP_MAX:
      g_value_set_uint (value, self->qp_max);
      break;
    case PROP_QP_MAX_STEP:
      g_value_set_int (value, self->qp_max_step);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

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

static void
gst_mpp_h264_enc_update_properties (GstVideoEncoder * encoder)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstMppVideoEnc *mppenc = GST_MPP_VIDEO_ENC (encoder);
  MppEncCfg cfg;

  if (!mppenc->prop_dirty && !self->prop_dirty)
    return;

  if (mpp_enc_cfg_init (&cfg)) {
    GST_WARNING_OBJECT (self, "Init enc cfg failed");
    return;
  }

  if (mppenc->mpi->control (mppenc->mpp_ctx, MPP_ENC_GET_CFG, cfg)) {
    GST_WARNING_OBJECT (self, "Get enc cfg failed");
    mpp_enc_cfg_deinit (cfg);
    return;
  }

  mpp_enc_cfg_set_s32 (cfg, "h264:qp_init", self->qp_init);

  if (mppenc->rc_mode == MPP_ENC_RC_MODE_FIXQP) {
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_max", self->qp_init);
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_min", self->qp_init);
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_step", 0);
  } else if (mppenc->rc_mode == MPP_ENC_RC_MODE_CBR) {
    /* NOTE: These settings have been tuned for better quality */
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_max", self->qp_max ? self->qp_max : 28);
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_min", self->qp_min ? self->qp_min : 4);
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_step",
        self->qp_max_step >= 0 ? self->qp_max_step : 8);
  } else if (mppenc->rc_mode == MPP_ENC_RC_MODE_VBR) {
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_max", self->qp_max ? self->qp_max : 40);
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_min", self->qp_min ? self->qp_min : 12);
    mpp_enc_cfg_set_s32 (cfg, "h264:qp_step",
        self->qp_max_step >= 0 ? self->qp_max_step : 8);
  }

  if (self->prop_dirty) {
    mpp_enc_cfg_set_s32 (cfg, "h264:profile", self->profile);
    mpp_enc_cfg_set_s32 (cfg, "h264:level", self->level);

    mpp_enc_cfg_set_s32 (cfg, "h264:trans8x8",
        self->profile == GST_MPP_H264_PROFILE_HIGH);
    mpp_enc_cfg_set_s32 (cfg, "h264:cabac_en",
        self->profile != GST_MPP_H264_PROFILE_BASELINE);
    mpp_enc_cfg_set_s32 (cfg, "h264:cabac_idc", 0);
  }

  if (mppenc->mpi->control (mppenc->mpp_ctx, MPP_ENC_SET_CFG, cfg))
    GST_WARNING_OBJECT (self, "Set enc cfg failed");

  mpp_enc_cfg_deinit (cfg);

  self->prop_dirty = FALSE;
}

static gboolean
gst_mpp_h264_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  gst_mpp_h264_enc_update_properties (encoder);

  return GST_MPP_VIDEO_ENC_CLASS (parent_class)->set_format (encoder, state);
}

static GstFlowReturn
gst_mpp_h264_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstCaps *outcaps;
  GstStructure *structure;
  GstFlowReturn ret;

  static const char *profile_names[] = {
    [GST_MPP_H264_PROFILE_BASELINE] = "baseline",
    [GST_MPP_H264_PROFILE_MAIN] = "main",
    [GST_MPP_H264_PROFILE_HIGH] = "high",
  };

  outcaps = gst_caps_new_empty_simple ("video/x-h264");
  structure = gst_caps_get_structure (outcaps, 0);
  gst_structure_set (structure, "stream-format",
      G_TYPE_STRING, "byte-stream", NULL);
  gst_structure_set (structure, "alignment", G_TYPE_STRING, "au", NULL);
  gst_structure_set (structure, "profile", G_TYPE_STRING,
      profile_names[self->profile], NULL);

  gst_mpp_h264_enc_update_properties (encoder);

  ret = GST_MPP_VIDEO_ENC_CLASS (parent_class)->handle_frame (encoder, frame,
      outcaps);
  gst_caps_unref (outcaps);
  return ret;
}

static void
gst_mpp_h264_enc_init (GstMppH264Enc * self)
{
  self->profile = DEFAULT_PROP_PROFILE;
  self->level = DEFAULT_PROP_LEVEL;
  self->qp_init = DEFAULT_PROP_QP_INIT;
  self->qp_min = DEFAULT_PROP_QP_MIN;
  self->qp_max = DEFAULT_PROP_QP_MAX;
  self->qp_max_step = DEFAULT_PROP_QP_MAX_STEP;
  self->prop_dirty = TRUE;
}

#define GST_TYPE_MPP_H264_ENC_PROFILE (gst_mpp_h264_enc_profile_get_type ())
static GType
gst_mpp_h264_enc_profile_get_type (void)
{
  static GType profile = 0;

  if (!profile) {
    static const GEnumValue profiles[] = {
      {GST_MPP_H264_PROFILE_BASELINE, "Baseline", "baseline"},
      {GST_MPP_H264_PROFILE_MAIN, "Main", "main"},
      {GST_MPP_H264_PROFILE_HIGH, "High", "high"},
      {0, NULL, NULL}
    };
    profile = g_enum_register_static ("GstMppH264Profile", profiles);
  }
  return profile;
}

static void
gst_mpp_h264_enc_class_init (GstMppH264EncClass * klass)
{
  GstElementClass *element_class;
  GObjectClass *gobject_class;
  GstVideoEncoderClass *video_encoder_class;

  element_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;
  video_encoder_class = (GstVideoEncoderClass *) klass;

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp H264 Encoder",
      "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Randy Li <randy.li@rock-chips.com>");

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_get_property);

  video_encoder_class->open = GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_open);
  video_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_handle_frame);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_h264_enc_src_template));

  g_object_class_install_property (gobject_class, PROP_PROFILE,
      g_param_spec_enum ("profile", "H264 profile",
          "H264 profile",
          GST_TYPE_MPP_H264_ENC_PROFILE, DEFAULT_PROP_PROFILE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_LEVEL,
      g_param_spec_uint ("level", "H264 level",
          "H264 level (40~41 = 1080p@30fps, 42 = 1080p60fps, 50~52 = 4K@30fps)",
          10, 62, DEFAULT_PROP_LEVEL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QP_INIT,
      g_param_spec_uint ("qp-init", "Initial QP",
          "Initial QP (lower value means higher quality)",
          0, 51, DEFAULT_PROP_QP_INIT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QP_MIN,
      g_param_spec_uint ("qp-min", "Min QP",
          "Min QP (0 = default)", 0, 51, DEFAULT_PROP_QP_MIN,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QP_MAX,
      g_param_spec_uint ("qp-max", "Max QP",
          "Max QP (0 = default)", 0, 51, DEFAULT_PROP_QP_MAX,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QP_MAX_STEP,
      g_param_spec_int ("qp-max-step", "Max QP step",
          "Max delta QP step between two frames (-1 = default)", -1, 51,
          DEFAULT_PROP_QP_MAX_STEP,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
