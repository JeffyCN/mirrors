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

#include "gstmpph264enc.h"

#define GST_MPP_H264_ENC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_H264_ENC, GstMppH264Enc))

#define GST_CAT_DEFAULT mpp_h264_enc_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

typedef enum
{
  GST_MPP_H264_PROFILE_BASELINE = 66,
  GST_MPP_H264_PROFILE_MAIN = 77,
  GST_MPP_H264_PROFILE_HIGH = 100,
} GstMppH264Profile;

struct _GstMppH264Enc
{
  GstMppEnc parent;

  GstMppH264Profile profile;
  gint level;

  guint qp_init;
  guint qp_min;
  guint qp_max;
  gint qp_max_step;
};

#define parent_class gst_mpp_h264_enc_parent_class
G_DEFINE_TYPE (GstMppH264Enc, gst_mpp_h264_enc, GST_TYPE_MPP_ENC);

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

#define GST_MPP_H264_ENC_SIZE_CAPS \
    "width  = (int) [ 96, 1920 ], height = (int) [ 64, 2176 ]"

static GstStaticPadTemplate gst_mpp_h264_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        GST_MPP_H264_ENC_SIZE_CAPS ","
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, "
        "profile = (string) { baseline, main, high }"));

static GstStaticPadTemplate gst_mpp_h264_enc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw,"
        "format = (string) { " MPP_ENC_FORMATS " }, "
        GST_MPP_H264_ENC_SIZE_CAPS));

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
      {0, NULL, NULL},
    };
    profile = g_enum_register_static ("GstMppH264Profile", profiles);
  }
  return profile;
}

#define GST_TYPE_MPP_H264_ENC_LEVEL (gst_mpp_h264_enc_level_get_type ())
static GType
gst_mpp_h264_enc_level_get_type (void)
{
  static GType level = 0;

  if (!level) {
    static const GEnumValue levels[] = {
      {10, "1", "1"},
      {99, "1b", "1b"},
      {11, "1.1", "1.1"},
      {12, "1.2", "1.2"},
      {13, "1.3", "1.3"},
      {20, "2", "2"},
      {21, "2.1", "2.1"},
      {22, "2.2", "2.2"},
      {30, "3", "3"},
      {31, "3.1", "3.1"},
      {32, "3.2", "3.2"},
      {40, "4", "4"},
      {41, "4.1", "4.1"},
      {42, "4.2", "4.2"},
      {50, "5", "5"},
      {51, "5.1", "5.1"},
      {52, "5.2", "5.2"},
      {60, "6", "6"},
      {61, "6.1", "6.1"},
      {62, "6.2", "6.2"},
      {0, NULL, NULL},
    };
    level = g_enum_register_static ("GstMppH264Level", levels);
  }
  return level;
}

static void
gst_mpp_h264_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstMppEnc *mppenc = GST_MPP_ENC (encoder);

  switch (prop_id) {
    case PROP_PROFILE:{
      GstMppH264Profile profile = g_value_get_enum (value);
      if (self->profile == profile)
        return;

      self->profile = profile;
      break;
    }
    case PROP_LEVEL:{
      gint level = g_value_get_enum (value);
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

  mppenc->prop_dirty = TRUE;
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
      g_value_set_enum (value, self->level);
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
gst_mpp_h264_enc_set_src_caps (GstVideoEncoder * encoder)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstStructure *structure;
  GstCaps *caps;
  gchar *string;

  caps = gst_caps_new_empty_simple ("video/x-h264");

  structure = gst_caps_get_structure (caps, 0);
  gst_structure_set (structure, "stream-format",
      G_TYPE_STRING, "byte-stream", NULL);
  gst_structure_set (structure, "alignment", G_TYPE_STRING, "au", NULL);

  string = g_enum_to_string (GST_TYPE_MPP_H264_ENC_PROFILE, self->profile);
  gst_structure_set (structure, "profile", G_TYPE_STRING, string, NULL);
  g_free (string);

  string = g_enum_to_string (GST_TYPE_MPP_H264_ENC_LEVEL, self->level);
  gst_structure_set (structure, "level", G_TYPE_STRING, string, NULL);
  g_free (string);

  return gst_mpp_enc_set_src_caps (encoder, caps);
}

static gboolean
gst_mpp_h264_enc_apply_properties (GstVideoEncoder * encoder)
{
  GstMppH264Enc *self = GST_MPP_H264_ENC (encoder);
  GstMppEnc *mppenc = GST_MPP_ENC (encoder);

  if (G_LIKELY (!mppenc->prop_dirty))
    return TRUE;

  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_init", self->qp_init);

  if (mppenc->rc_mode == MPP_ENC_RC_MODE_FIXQP) {
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_max", self->qp_init);
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_min", self->qp_init);
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_step", 0);
  } else if (mppenc->rc_mode == MPP_ENC_RC_MODE_CBR) {
    /* NOTE: These settings have been tuned for better quality */
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_max",
        self->qp_max ? self->qp_max : 28);
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_min",
        self->qp_min ? self->qp_min : 4);
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_step",
        self->qp_max_step >= 0 ? self->qp_max_step : 8);
  } else if (mppenc->rc_mode == MPP_ENC_RC_MODE_VBR) {
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_max",
        self->qp_max ? self->qp_max : 40);
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_min",
        self->qp_min ? self->qp_min : 12);
    mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:qp_step",
        self->qp_max_step >= 0 ? self->qp_max_step : 8);
  }

  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:profile", self->profile);
  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:level", self->level);

  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:trans8x8",
      self->profile == GST_MPP_H264_PROFILE_HIGH);
  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:cabac_en",
      self->profile != GST_MPP_H264_PROFILE_BASELINE);
  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "h264:cabac_idc", 0);

  if (!gst_mpp_enc_apply_properties (encoder))
    return FALSE;

  return gst_mpp_h264_enc_set_src_caps (encoder);
}

static gboolean
gst_mpp_h264_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  GstVideoEncoderClass *pclass = GST_VIDEO_ENCODER_CLASS (parent_class);

  if (!pclass->set_format (encoder, state))
    return FALSE;

  return gst_mpp_h264_enc_apply_properties (encoder);
}

static GstFlowReturn
gst_mpp_h264_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
  GstVideoEncoderClass *pclass = GST_VIDEO_ENCODER_CLASS (parent_class);

  if (G_UNLIKELY (!gst_mpp_h264_enc_apply_properties (encoder))) {
    gst_video_codec_frame_unref (frame);
    return GST_FLOW_NOT_NEGOTIATED;
  }

  return pclass->handle_frame (encoder, frame);
}

static void
gst_mpp_h264_enc_init (GstMppH264Enc * self)
{
  self->parent.mpp_type = MPP_VIDEO_CodingAVC;

  self->profile = DEFAULT_PROP_PROFILE;
  self->level = DEFAULT_PROP_LEVEL;
  self->qp_init = DEFAULT_PROP_QP_INIT;
  self->qp_min = DEFAULT_PROP_QP_MIN;
  self->qp_max = DEFAULT_PROP_QP_MAX;
  self->qp_max_step = DEFAULT_PROP_QP_MAX_STEP;
}

static void
gst_mpp_h264_enc_class_init (GstMppH264EncClass * klass)
{
  GstVideoEncoderClass *encoder_class = GST_VIDEO_ENCODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mpph264enc", 0,
      "MPP H264 encoder");

  encoder_class->set_format = GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_format);
  encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_handle_frame);

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_get_property);

  g_object_class_install_property (gobject_class, PROP_PROFILE,
      g_param_spec_enum ("profile", "H264 profile",
          "H264 profile",
          GST_TYPE_MPP_H264_ENC_PROFILE, DEFAULT_PROP_PROFILE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_LEVEL,
      g_param_spec_enum ("level", "H264 level",
          "H264 level (40~41 = 1080p@30fps, 42 = 1080p60fps, 50~52 = 4K@30fps)",
          GST_TYPE_MPP_H264_ENC_LEVEL, DEFAULT_PROP_LEVEL,
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

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_h264_enc_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_h264_enc_sink_template));

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp H264 Encoder", "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Randy Li <randy.li@rock-chips.com>, "
      "Jeffy Chen <jeffy.chen@rock-chips.com>");
}
