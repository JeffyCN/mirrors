/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
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

#include "gstmppjpegenc.h"

#define GST_MPP_JPEG_ENC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_JPEG_ENC, GstMppJpegEnc))

#define GST_CAT_DEFAULT mpp_jpeg_enc_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

struct _GstMppJpegEnc
{
  GstMppEnc parent;

  guint quant;
};

#define parent_class gst_mpp_jpeg_enc_parent_class
G_DEFINE_TYPE (GstMppJpegEnc, gst_mpp_jpeg_enc, GST_TYPE_MPP_ENC);

#define DEFAULT_PROP_QUANT 10

enum
{
  PROP_0,
  PROP_QUANT,
  PROP_LAST,
};

#define GST_MPP_JPEG_ENC_SIZE_CAPS \
    "width  = (int) [ 16, 8176 ], height = (int) [ 16, 8176 ]"

static GstStaticPadTemplate gst_mpp_jpeg_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jpeg, "
        GST_MPP_JPEG_ENC_SIZE_CAPS "," "sof-marker = { 0 }"));

static GstStaticPadTemplate gst_mpp_jpeg_enc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw,"
        "format = (string) { " MPP_ENC_FORMATS " }, "
        GST_MPP_JPEG_ENC_SIZE_CAPS));

static void
gst_mpp_h264_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppJpegEnc *self = GST_MPP_JPEG_ENC (encoder);
  GstMppEnc *mppenc = GST_MPP_ENC (encoder);

  switch (prop_id) {
    case PROP_QUANT:{
      guint quant = g_value_get_uint (value);
      if (self->quant == quant)
        return;

      self->quant = quant;
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
  GstMppJpegEnc *self = GST_MPP_JPEG_ENC (encoder);

  switch (prop_id) {
    case PROP_QUANT:
      g_value_set_uint (value, self->quant);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_mpp_jpeg_enc_apply_properties (GstVideoEncoder * encoder)
{
  GstMppJpegEnc *self = GST_MPP_JPEG_ENC (encoder);
  GstMppEnc *mppenc = GST_MPP_ENC (encoder);

  if (!mppenc->prop_dirty)
    return TRUE;

  mpp_enc_cfg_set_s32 (mppenc->mpp_cfg, "jpeg:quant", self->quant);

  return gst_mpp_enc_apply_properties (encoder);
}

static gboolean
gst_mpp_jpeg_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  GstVideoEncoderClass *pclass = GST_VIDEO_ENCODER_CLASS (parent_class);
  GstCaps *caps;

  if (!pclass->set_format (encoder, state))
    return FALSE;

  if (!gst_mpp_jpeg_enc_apply_properties (encoder))
    return FALSE;

  caps = gst_caps_new_empty_simple ("image/jpeg");
  return gst_mpp_enc_set_src_caps (encoder, caps);
}

static GstFlowReturn
gst_mpp_jpeg_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{
  GstVideoEncoderClass *pclass = GST_VIDEO_ENCODER_CLASS (parent_class);

  if (G_UNLIKELY (!gst_mpp_jpeg_enc_apply_properties (encoder))) {
    gst_video_codec_frame_unref (frame);
    return GST_FLOW_NOT_NEGOTIATED;
  }

  return pclass->handle_frame (encoder, frame);
}

static void
gst_mpp_jpeg_enc_init (GstMppJpegEnc * self)
{
  self->parent.mpp_type = MPP_VIDEO_CodingMJPEG;

  self->quant = DEFAULT_PROP_QUANT;
}

static void
gst_mpp_jpeg_enc_class_init (GstMppJpegEncClass * klass)
{
  GstVideoEncoderClass *encoder_class = GST_VIDEO_ENCODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppjpegenc", 0,
      "MPP JPEG encoder");

  encoder_class->set_format = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_enc_set_format);
  encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_enc_handle_frame);

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_h264_enc_get_property);

  g_object_class_install_property (gobject_class, PROP_QUANT,
      g_param_spec_uint ("quant", "Quant",
          "JPEG Quantization", 0, 10, DEFAULT_PROP_QUANT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_enc_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_enc_sink_template));

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp JPEG Encoder", "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Randy Li <randy.li@rock-chips.com>, "
      "Jeffy Chen <jeffy.chen@rock-chips.com>");
}
