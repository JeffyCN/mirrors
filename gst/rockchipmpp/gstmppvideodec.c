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

#include "gstmppvideodec.h"

#define GST_MPP_VIDEO_DEC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_VIDEO_DEC, GstMppVideoDec))

#define GST_CAT_DEFAULT mpp_video_dec_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

struct _GstMppVideoDec
{
  GstMppDec parent;

  gint poll_timeout;
};

#define parent_class gst_mpp_video_dec_parent_class
G_DEFINE_TYPE (GstMppVideoDec, gst_mpp_video_dec, GST_TYPE_MPP_DEC);

/* Default output format is auto */
static GstVideoFormat DEFAULT_PROP_FORMAT = GST_VIDEO_FORMAT_UNKNOWN;

enum
{
  PROP_0,
  PROP_FORMAT,
  PROP_LAST,
};

/* GstVideoDecoder base class method */
static GstStaticPadTemplate gst_mpp_video_dec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264,"
        "stream-format = (string) { avc, avc3, byte-stream },"
        "alignment = (string) { au },"
        "parsed = (boolean) true"
        ";"
        "video/x-h265,"
        "stream-format = (string) { hvc1, hev1, byte-stream },"
        "alignment = (string) { au },"
        "parsed = (boolean) true"
        ";"
        "video/mpeg,"
        "mpegversion = (int) { 1, 2, 4 },"
        "parsed = (boolean) true,"
        "systemstream = (boolean) false"
        ";" "video/x-vp8" ";" "video/x-vp9" ";"));

static GstStaticPadTemplate gst_mpp_video_dec_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) { " MPP_DEC_FORMATS ", NV12_10LE40 }, "
        "width  = (int) [ 32, 4096 ], " "height =  (int) [ 32, 4096 ]" ";"));

static MppCodingType
gst_mpp_video_dec_get_mpp_type (GstStructure * s)
{
  if (gst_structure_has_name (s, "video/x-h264"))
    return MPP_VIDEO_CodingAVC;

  if (gst_structure_has_name (s, "video/x-h265"))
    return MPP_VIDEO_CodingHEVC;

  if (gst_structure_has_name (s, "video/x-h263"))
    return MPP_VIDEO_CodingH263;

  if (gst_structure_has_name (s, "video/mpeg")) {
    gint mpegversion = 0;
    if (gst_structure_get_int (s, "mpegversion", &mpegversion)) {
      switch (mpegversion) {
        case 1:
        case 2:
          return MPP_VIDEO_CodingMPEG2;
        case 4:
          return MPP_VIDEO_CodingMPEG4;
        default:
          g_assert_not_reached ();
          return MPP_VIDEO_CodingUnused;
      }
    }
  }

  if (gst_structure_has_name (s, "video/x-vp8"))
    return MPP_VIDEO_CodingVP8;

  if (gst_structure_has_name (s, "video/x-vp9"))
    return MPP_VIDEO_CodingVP9;

  return MPP_VIDEO_CodingUnused;
}

static void
gst_mpp_video_dec_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (object);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);

  switch (prop_id) {
    case PROP_FORMAT:{
      GstVideoFormat format = g_value_get_enum (value);
      if (mppdec->format == format)
        return;

      if (mppdec->input_state) {
        GST_WARNING_OBJECT (decoder, "unable to change output format");
        return;
      }

      mppdec->format = format;
      break;
    }

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static void
gst_mpp_video_dec_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (object);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);

  switch (prop_id) {
    case PROP_FORMAT:
      g_value_set_enum (value, mppdec->format);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_mpp_video_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  GstStructure *structure;
  const gchar *chroma_format;

  structure = gst_caps_get_structure (state->caps, 0);
  mppdec->mpp_type = gst_mpp_video_dec_get_mpp_type (structure);
  g_return_val_if_fail (mppdec->mpp_type != MPP_VIDEO_CodingUnused, FALSE);

  /* MPP doesn't support YUV444 for h264 */
  if (mppdec->mpp_type == MPP_VIDEO_CodingAVC) {
    chroma_format = gst_structure_get_string (structure, "chroma-format");
    if (g_strcmp0 (chroma_format, "4:4:4") == 0)
      return FALSE;
  }

  return pclass->set_format (decoder, state);
}

static gboolean
gst_mpp_video_dec_startup (GstVideoDecoder * decoder)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  GstVideoCodecState *state = mppdec->input_state;
  GstBuffer *codec_data = state->codec_data;
  GstMapInfo mapinfo = { 0, };
  MppFrame mframe;
  MppPacket mpkt;

  /* Send extra codec data */
  if (codec_data) {
    gst_buffer_ref (codec_data);
    gst_buffer_map (codec_data, &mapinfo, GST_MAP_READ);
    mpp_packet_init (&mpkt, mapinfo.data, mapinfo.size);
    mpp_packet_set_extra_data (mpkt);

    mppdec->mpi->decode_put_packet (mppdec->mpp_ctx, mpkt);

    mpp_packet_deinit (&mpkt);
    gst_buffer_unmap (codec_data, &mapinfo);
    gst_buffer_unref (codec_data);
  }

  /* Legacy way to inform MPP codec of video info, needed by RKVDEC */
  mpp_frame_init (&mframe);
  mpp_frame_set_width (mframe, GST_VIDEO_INFO_WIDTH (&state->info));
  mpp_frame_set_height (mframe, GST_VIDEO_INFO_HEIGHT (&state->info));
  mpp_frame_set_fmt (mframe, (MppFrameFormat) mppdec->mpp_type);
  mppdec->mpi->control (mppdec->mpp_ctx, MPP_DEC_SET_FRAME_INFO,
      (MppParam) mframe);
  mpp_frame_deinit (&mframe);

  self->poll_timeout = 0;

  return TRUE;
}

static MppPacket
gst_mpp_video_dec_get_mpp_packet (GstVideoDecoder * decoder UNUSED,
    GstMapInfo * mapinfo)
{
  MppPacket mpkt = NULL;
  mpp_packet_init (&mpkt, mapinfo->data, mapinfo->size);
  return mpkt;
}

static gboolean
gst_mpp_video_dec_send_mpp_packet (GstVideoDecoder * decoder,
    MppPacket mpkt, gint timeout_ms)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  gint interval_ms = 2;
  MPP_RET ret;

  do {
    ret = mppdec->mpi->decode_put_packet (mppdec->mpp_ctx, mpkt);
    if (!ret) {
      mpp_packet_deinit (&mpkt);
      return TRUE;
    }

    g_usleep (interval_ms * 1000);
    timeout_ms -= interval_ms;
  } while (timeout_ms > 0);

  return FALSE;
}

static MppFrame
gst_mpp_video_dec_poll_mpp_frame (GstVideoDecoder * decoder, gint timeout_ms)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppFrame mframe = NULL;

  if (self->poll_timeout != timeout_ms) {
    self->poll_timeout = timeout_ms;
    mppdec->mpi->control (mppdec->mpp_ctx, MPP_SET_OUTPUT_TIMEOUT, &timeout_ms);
  }

  mppdec->mpi->decode_get_frame (mppdec->mpp_ctx, &mframe);

  return mframe;
}

static gboolean
gst_mpp_video_dec_shutdown (GstVideoDecoder * decoder, gboolean drain)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppPacket mpkt;
  MPP_RET ret;

  /* It's safe to stop decoding immediately */
  if (!drain)
    return FALSE;

  mpp_packet_init (&mpkt, NULL, 0);
  mpp_packet_set_eos (mpkt);

  while (1) {
    ret = mppdec->mpi->decode_put_packet (mppdec->mpp_ctx, mpkt);
    if (!ret)
      break;

    g_usleep (1000);
  }

  mpp_packet_deinit (&mpkt);
  return TRUE;
}

#define GST_TYPE_MPP_VIDEO_DEC_FORMAT (gst_mpp_video_dec_format_get_type ())
static GType
gst_mpp_video_dec_format_get_type (void)
{
  static GType format = 0;

  if (!format) {
    static const GEnumValue formats[] = {
      {GST_VIDEO_FORMAT_UNKNOWN, "Auto", "auto"},
      {GST_VIDEO_FORMAT_NV12, "NV12", "NV12"},
      {GST_VIDEO_FORMAT_NV21, "NV21", "NV21"},
      {GST_VIDEO_FORMAT_I420, "I420", "I420"},
      {GST_VIDEO_FORMAT_YV12, "YV12", "YV12"},
      {GST_VIDEO_FORMAT_NV16, "NV16", "NV16"},
      {GST_VIDEO_FORMAT_NV61, "NV61", "NV61"},
      {GST_VIDEO_FORMAT_BGR16, "BGR565", "BGR16"},
      {GST_VIDEO_FORMAT_RGB, "RGB", "RGB"},
      {GST_VIDEO_FORMAT_BGR, "BGR", "BGR"},
      {GST_VIDEO_FORMAT_RGBA, "RGBA8888", "RGBA"},
      {GST_VIDEO_FORMAT_BGRA, "BGRA8888", "BGRA"},
      {GST_VIDEO_FORMAT_RGBx, "RGBX8888", "RGBx"},
      {GST_VIDEO_FORMAT_BGRx, "BGRX8888", "BGRx"},
      {0, NULL, NULL}
    };
    format = g_enum_register_static ("GstMppVideoDecFormat", formats);
  }
  return format;
}

static void
gst_mpp_video_dec_init (GstMppVideoDec * self)
{
  GstMppDec *mppdec = GST_MPP_DEC (self);
  mppdec->format = DEFAULT_PROP_FORMAT;
}

static void
gst_mpp_video_dec_setup_default_format (void)
{
  GEnumClass *class;
  GEnumValue *value;
  const gchar *env;

  env = g_getenv ("GST_MPP_VIDEODEC_DEFAULT_FORMAT");
  if (!env)
    return;

  class = g_type_class_ref (GST_TYPE_MPP_VIDEO_DEC_FORMAT);

  value = g_enum_get_value_by_nick (class, env);
  if (value)
    DEFAULT_PROP_FORMAT = value->value;

  g_type_class_unref (class);
}

static void
gst_mpp_video_dec_class_init (GstMppVideoDecClass * klass)
{
  GstVideoDecoderClass *decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GstMppDecClass *pclass = GST_MPP_DEC_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppvideodec", 0,
      "MPP video decoder");

  decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_set_format);

  pclass->startup = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_startup);
  pclass->get_mpp_packet = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_get_mpp_packet);
  pclass->send_mpp_packet =
      GST_DEBUG_FUNCPTR (gst_mpp_video_dec_send_mpp_packet);
  pclass->poll_mpp_frame = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_poll_mpp_frame);
  pclass->shutdown = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_shutdown);

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_video_dec_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_video_dec_get_property);

  gst_mpp_video_dec_setup_default_format ();

#ifdef HAVE_RGA
  g_object_class_install_property (gobject_class, PROP_FORMAT,
      g_param_spec_enum ("format", "Prefered output format",
          "Prefered output format",
          GST_TYPE_MPP_VIDEO_DEC_FORMAT, DEFAULT_PROP_FORMAT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#endif

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_dec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_dec_sink_template));

  gst_element_class_set_static_metadata (element_class,
      "Rockchip's MPP video decoder", "Decoder/Video",
      "Multicodec (HEVC / AVC / VP8 / VP9) hardware decoder",
      "Randy Li <randy.li@rock-chips.com>, "
      "Jeffy Chen <jeffy.chen@rock-chips.com>");
}
