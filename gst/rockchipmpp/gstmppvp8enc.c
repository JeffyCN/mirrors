/*
 * Copyright 2020 Rockchip Electronics Co., Ltd
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

#include "gstmppvp8enc.h"

#define GST_CAT_DEFAULT mppvideoenc_debug

#define parent_class gst_mpp_vp8_enc_parent_class
G_DEFINE_TYPE (GstMppVP8Enc, gst_mpp_vp8_enc, GST_TYPE_MPP_VIDEO_ENC);

#define DEFAULT_PROP_QP_INIT 40
#define DEFAULT_PROP_QP_MIN 0
#define DEFAULT_PROP_QP_MAX 127

enum
{
  PROP_0,
  PROP_QP_INIT,
  PROP_QP_MIN,
  PROP_QP_MAX,
  PROP_LAST,
};

static GstStaticPadTemplate gst_mpp_vp8_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-vp8, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1088 ], " "framerate = (fraction) [0/1, 60/1]")
    );

static void
gst_mpp_vp8_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppVP8Enc *self = GST_MPP_VP8_ENC (encoder);

  switch (prop_id) {
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
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }

  self->prop_dirty = TRUE;
}

static void
gst_mpp_vp8_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppVP8Enc *self = GST_MPP_VP8_ENC (encoder);

  switch (prop_id) {
    case PROP_QP_INIT:
      g_value_set_uint (value, self->qp_init);
      break;
    case PROP_QP_MIN:
      g_value_set_uint (value, self->qp_min);
      break;
    case PROP_QP_MAX:
      g_value_set_uint (value, self->qp_max);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_mpp_vp8_enc_open (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  GST_DEBUG_OBJECT (self, "Opening");

  if (mpp_create (&self->mpp_ctx, &self->mpi))
    goto failure;
  if (mpp_init (self->mpp_ctx, MPP_CTX_ENC, MPP_VIDEO_CodingVP8))
    goto failure;

  return TRUE;

failure:
  return FALSE;
}

static void
gst_mpp_vp8_enc_update_properties (GstVideoEncoder * encoder)
{
  GstMppVP8Enc *self = GST_MPP_VP8_ENC (encoder);
  GstMppVideoEnc *mppenc = GST_MPP_VIDEO_ENC (encoder);
  MppEncCfg cfg;

  if (!self->prop_dirty)
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

  mpp_enc_cfg_set_s32 (cfg, "vp8:qp_init", self->qp_init);
  mpp_enc_cfg_set_s32 (cfg, "vp8:qp_max", self->qp_max);
  mpp_enc_cfg_set_s32 (cfg, "vp8:qp_min", self->qp_min);

  if (mppenc->mpi->control (mppenc->mpp_ctx, MPP_ENC_SET_CFG, cfg))
    GST_WARNING_OBJECT (self, "Set enc cfg failed");

  mpp_enc_cfg_deinit (cfg);

  self->prop_dirty = FALSE;
}

static gboolean
gst_mpp_vp8_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  gst_mpp_vp8_enc_update_properties (encoder);

  return GST_MPP_VIDEO_ENC_CLASS (parent_class)->set_format (encoder, state);
}

static GstFlowReturn
gst_mpp_vp8_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
  GstCaps *outcaps;
  GstFlowReturn ret;

  outcaps = gst_caps_new_empty_simple ("video/x-vp8");

  gst_mpp_vp8_enc_update_properties (encoder);

  ret = GST_MPP_VIDEO_ENC_CLASS (parent_class)->handle_frame (encoder, frame,
      outcaps);
  gst_caps_unref (outcaps);
  return ret;
}

static void
gst_mpp_vp8_enc_init (GstMppVP8Enc * self)
{
  self->qp_init = DEFAULT_PROP_QP_INIT;
  self->qp_min = DEFAULT_PROP_QP_MIN;
  self->qp_max = DEFAULT_PROP_QP_MAX;
  self->prop_dirty = TRUE;
}

static void
gst_mpp_vp8_enc_class_init (GstMppVP8EncClass * klass)
{
  GstElementClass *element_class;
  GObjectClass *gobject_class;
  GstVideoEncoderClass *video_encoder_class;

  element_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;
  video_encoder_class = (GstVideoEncoderClass *) klass;

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp VP8 Encoder",
      "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Jeffy Chen <jeffy.chen@rock-chips.com>");

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_vp8_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_vp8_enc_get_property);

  video_encoder_class->open = GST_DEBUG_FUNCPTR (gst_mpp_vp8_enc_open);
  video_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (gst_mpp_vp8_enc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_vp8_enc_handle_frame);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_vp8_enc_src_template));

  g_object_class_install_property (gobject_class, PROP_QP_INIT,
      g_param_spec_uint ("qp-init", "Initial QP",
          "Initial QP (lower value means higher quality)",
          0, 127, DEFAULT_PROP_QP_INIT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QP_MIN,
      g_param_spec_uint ("qp-min", "Min QP",
          "Min QP", 0, 127, DEFAULT_PROP_QP_MIN,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QP_MAX,
      g_param_spec_uint ("qp-max", "Max QP",
          "Max QP", 0, 127, DEFAULT_PROP_QP_MAX,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
