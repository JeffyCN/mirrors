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

#include "gstmppallocator.h"
#include "gstmppjpegdec.h"

#define GST_MPP_JPEG_DEC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_JPEG_DEC, GstMppJpegDec))

#define GST_CAT_DEFAULT mpp_jpeg_dec_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

struct _GstMppJpegDec
{
  GstMppDec parent;

  /* size of output buffer */
  guint buf_size;

  /* group for input packet buffer allocations */
  MppBufferGroup input_group;

  MppPacket eos_packet;
};

#define parent_class gst_mpp_jpeg_dec_parent_class
G_DEFINE_TYPE (GstMppJpegDec, gst_mpp_jpeg_dec, GST_TYPE_MPP_DEC);

/* Default output format is auto */
static GstVideoFormat DEFAULT_PROP_FORMAT = GST_VIDEO_FORMAT_UNKNOWN;

enum
{
  PROP_0,
  PROP_FORMAT,
  PROP_LAST,
};

/* GstVideoDecoder base class method */
static GstStaticPadTemplate gst_mpp_jpeg_dec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jpeg," "parsed = (boolean) true" ";")
    );

#define MPP_JPEGD_FORMATS \
    "NV12, " \
    "BGR16, RGB16, " \
    "ABGR, ARGB, BGRA, RGBA, xBGR, xRGB, BGRx, RGBx"

static GstStaticPadTemplate gst_mpp_jpeg_dec_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) { " MPP_DEC_FORMATS ", " MPP_JPEGD_FORMATS " }, "
        "width  = (int) [ 48, 8176 ], " "height =  (int) [ 48, 8176 ]" ";")
    );

static const GstVideoFormat gst_mpp_jpeg_dec_pp_formats[] = {
  GST_VIDEO_FORMAT_UNKNOWN,
  GST_VIDEO_FORMAT_NV12,
  GST_VIDEO_FORMAT_RGB16,
  GST_VIDEO_FORMAT_BGR16,
  GST_VIDEO_FORMAT_ARGB,
  GST_VIDEO_FORMAT_ABGR,
  GST_VIDEO_FORMAT_RGBA,
  GST_VIDEO_FORMAT_BGRA,
  GST_VIDEO_FORMAT_xRGB,
  GST_VIDEO_FORMAT_xBGR,
  GST_VIDEO_FORMAT_RGBx,
  GST_VIDEO_FORMAT_BGRx,
};

static GstVideoFormat
gst_mpp_jpeg_dec_try_pp_convert (GstVideoDecoder * decoder,
    GstVideoFormat format, gboolean force)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppFrameFormat mpp_format = force ? MPP_FMT_YUV420SP : MPP_FMT_BUTT;
  guint i;

  for (i = 0; i < ARRAY_SIZE (gst_mpp_jpeg_dec_pp_formats); i++) {
    if (format == gst_mpp_jpeg_dec_pp_formats[i]) {
      mpp_format = gst_mpp_gst_format_to_mpp_format (format);
      break;
    }
  }

  /* Using MPP internal format conversion (PP) */
  if (mpp_format != MPP_FMT_BUTT) {
    if (mppdec->mpi->control (mppdec->mpp_ctx, MPP_DEC_SET_OUTPUT_FORMAT,
            &mpp_format) >= 0)
      return gst_mpp_mpp_format_to_gst_format (mpp_format);
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

static GstVideoFormat
gst_mpp_jpeg_dec_get_format (GstStructure * structure)
{
  const gchar *s;

  if ((s = gst_structure_get_string (structure, "format")))
    return gst_video_format_from_string (s);

  return GST_VIDEO_FORMAT_UNKNOWN;
}

static void
gst_mpp_jpeg_dec_set_property (GObject * object,
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
gst_mpp_jpeg_dec_get_property (GObject * object,
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
gst_mpp_jpeg_dec_start (GstVideoDecoder * decoder)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppBuffer mbuf;

  GST_DEBUG_OBJECT (self, "starting");

  if (mpp_buffer_group_get_internal (&self->input_group, MPP_BUFFER_TYPE_DRM))
    return FALSE;

  /* Prepare EOS packet */
  mpp_buffer_get (self->input_group, &mbuf, 1);
  mpp_packet_init_with_buffer (&self->eos_packet, mbuf);
  mpp_buffer_put (mbuf);
  mpp_packet_set_size (self->eos_packet, 0);
  mpp_packet_set_length (self->eos_packet, 0);
  mpp_packet_set_eos (self->eos_packet);

  if (!pclass->start (decoder)) {
    mpp_packet_deinit (&self->eos_packet);
    mpp_buffer_group_put (self->input_group);
    return FALSE;
  }

  mppdec->mpp_type = MPP_VIDEO_CodingMJPEG;

  GST_DEBUG_OBJECT (self, "started");

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_stop (GstVideoDecoder * decoder)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);

  GST_DEBUG_OBJECT (self, "stopping");

  pclass->stop (decoder);

  mpp_packet_deinit (&self->eos_packet);
  mpp_buffer_group_put (self->input_group);

  GST_DEBUG_OBJECT (self, "stopped");

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &mppdec->info;
  GstStructure *structure;
  GstVideoFormat src_format, dst_format;
  gint width = GST_VIDEO_INFO_WIDTH (&state->info);
  gint height = GST_VIDEO_INFO_HEIGHT (&state->info);
  gint dst_width, dst_height;
  guint align = GST_MPP_ALIGNMENT;

  if (!width || !height) {
    GST_ERROR_OBJECT (self, "invalid input video info");
    return FALSE;
  }

  if (!pclass->set_format (decoder, state))
    return FALSE;

  /* Figure out original output format */
  structure = gst_caps_get_structure (state->caps, 0);
  src_format = gst_mpp_jpeg_dec_get_format (structure);
  switch (src_format) {
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_I420:
      src_format = GST_VIDEO_FORMAT_NV12;
      break;
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_Y42B:
    case GST_VIDEO_FORMAT_NV16:
      src_format = GST_VIDEO_FORMAT_NV16;
      break;
    default:
      /* FIXME: Gst doesn't support semi-planar version of Y444/Y41B...etc */
      src_format = GST_VIDEO_FORMAT_UNKNOWN;
      break;
  }

  /* Figure out final output info */
  gst_mpp_dec_fixup_video_info (decoder, src_format, width, height);
  dst_format = GST_VIDEO_INFO_FORMAT (info);
  dst_width = GST_VIDEO_INFO_WIDTH (info);
  dst_height = GST_VIDEO_INFO_HEIGHT (info);

  /* Prefer MPP internal format conversion (PP) */
  if (src_format != dst_format) {
    GstVideoFormat pp_format = GST_VIDEO_FORMAT_UNKNOWN;

    if (src_format == GST_VIDEO_FORMAT_UNKNOWN) {
      /* PP conversion is required for unknown formats */
      pp_format = gst_mpp_jpeg_dec_try_pp_convert (decoder, dst_format, TRUE);
      if (pp_format == GST_VIDEO_FORMAT_UNKNOWN) {
        GST_ERROR_OBJECT (self, "unsupported video format");
        return FALSE;
      }
    } else if (dst_width == width && dst_height == height) {
      /* Prefer PP conversion */
      pp_format = gst_mpp_jpeg_dec_try_pp_convert (decoder, dst_format, TRUE);
    }

    /* MPP is going to provide the converted format */
    if (pp_format != GST_VIDEO_FORMAT_UNKNOWN)
      src_format = pp_format;
  }

  if (dst_format != src_format || dst_width != width || dst_height != height) {
    /* Conversion required */
    GST_INFO_OBJECT (self, "convert from %s (%dx%d) to %s (%dx%d)",
        gst_video_format_to_string (src_format), width, height,
        gst_video_format_to_string (dst_format), dst_width, dst_height);

    align = 0;
  }

  /* Original output buffer size calculation */
  gst_video_info_set_format (info, src_format, width, height);
  if (!gst_mpp_video_info_align (info, 0, 0))
    return FALSE;

  self->buf_size = GST_VIDEO_INFO_SIZE (info);

  /* FIXME: Workaround MPP's JPEG parser size requirement issue (w * h * 2) */
  self->buf_size =
      MAX (self->buf_size, GST_VIDEO_INFO_PLANE_OFFSET (info, 1) * 2);

  /* Update final output info */
  return gst_mpp_dec_update_video_info (decoder, dst_format,
      dst_width, dst_height, 0, 0, align);
}

static MppPacket
gst_mpp_jpeg_dec_get_mpp_packet (GstVideoDecoder * decoder,
    GstMapInfo * mapinfo)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  MppBuffer mbuf = NULL;
  MppPacket mpkt = NULL;

  mpp_buffer_get (self->input_group, &mbuf, mapinfo->size);
  if (G_UNLIKELY (!mbuf))
    return NULL;

  /* FIXME: performance bad */
  memcpy (mpp_buffer_get_ptr (mbuf), mapinfo->data, mapinfo->size);

  mpp_packet_init_with_buffer (&mpkt, mbuf);
  mpp_buffer_put (mbuf);
  if (G_UNLIKELY (!mpkt))
    return NULL;

  mpp_packet_set_size (mpkt, mapinfo->size);
  mpp_packet_set_length (mpkt, mapinfo->size);

  return mpkt;
}

static gboolean
gst_mpp_jpeg_dec_send_mpp_packet (GstVideoDecoder * decoder,
    MppPacket mpkt, gint timeout_ms)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppBuffer mbuf;
  MppFrame mframe = NULL;
  MppTask mtask = NULL;
  MppMeta meta;

  mppdec->mpi->poll (mppdec->mpp_ctx, MPP_PORT_INPUT, timeout_ms);
  mppdec->mpi->dequeue (mppdec->mpp_ctx, MPP_PORT_INPUT, &mtask);
  if (G_UNLIKELY (!mtask))
    goto error;

  mpp_task_meta_set_packet (mtask, KEY_INPUT_PACKET, mpkt);

  mbuf = gst_mpp_allocator_alloc_mppbuf (mppdec->allocator, self->buf_size);
  if (G_UNLIKELY (!mbuf))
    goto error;

  mpp_frame_init (&mframe);
  mpp_frame_set_buffer (mframe, mbuf);
  mpp_buffer_put (mbuf);

  meta = mpp_frame_get_meta (mframe);
  mpp_meta_set_packet (meta, KEY_INPUT_PACKET, mpkt);

  mpp_task_meta_set_frame (mtask, KEY_OUTPUT_FRAME, mframe);

  if (mppdec->mpi->enqueue (mppdec->mpp_ctx, MPP_PORT_INPUT, mtask))
    goto error;

  return TRUE;

error:
  if (mtask) {
    mpp_task_meta_set_packet (mtask, KEY_INPUT_PACKET, NULL);
    mpp_task_meta_set_frame (mtask, KEY_OUTPUT_FRAME, NULL);
    mppdec->mpi->enqueue (mppdec->mpp_ctx, MPP_PORT_INPUT, mtask);
  }

  if (mframe)
    mpp_frame_deinit (&mframe);

  return FALSE;
}

static MppFrame
gst_mpp_jpeg_dec_poll_mpp_frame (GstVideoDecoder * decoder, gint timeout_ms)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppPacket mpkt = NULL;
  MppTask mtask = NULL;
  MppFrame mframe = NULL;
  MppMeta meta;

  if (mppdec->mpi->poll (mppdec->mpp_ctx, MPP_PORT_OUTPUT, timeout_ms))
    return NULL;

  mppdec->mpi->dequeue (mppdec->mpp_ctx, MPP_PORT_OUTPUT, &mtask);
  if (!mtask)
    return NULL;

  mpp_task_meta_get_frame (mtask, KEY_OUTPUT_FRAME, &mframe);
  if (!mframe) {
    mppdec->mpi->enqueue (mppdec->mpp_ctx, MPP_PORT_OUTPUT, mtask);
    return NULL;
  }

  meta = mpp_frame_get_meta (mframe);
  mpp_meta_get_packet (meta, KEY_INPUT_PACKET, &mpkt);
  if (mpkt)
    mpp_packet_deinit (&mpkt);

  mppdec->mpi->enqueue (mppdec->mpp_ctx, MPP_PORT_OUTPUT, mtask);

  return mframe;
}

static gboolean
gst_mpp_jpeg_dec_shutdown (GstVideoDecoder * decoder, gboolean drain UNUSED)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppFrame mframe = NULL;
  MppTask mtask = NULL;

  GST_DEBUG_OBJECT (self, "sending EOS");

  mppdec->mpi->poll (mppdec->mpp_ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
  mppdec->mpi->dequeue (mppdec->mpp_ctx, MPP_PORT_INPUT, &mtask);
  if (!mtask)
    goto error;

  mpp_task_meta_set_packet (mtask, KEY_INPUT_PACKET, self->eos_packet);

  mpp_frame_init (&mframe);
  if (!mframe)
    goto error;

  mpp_task_meta_set_frame (mtask, KEY_OUTPUT_FRAME, mframe);

  if (mppdec->mpi->enqueue (mppdec->mpp_ctx, MPP_PORT_INPUT, mtask))
    goto error;

  return TRUE;

error:
  GST_WARNING_OBJECT (self, "failed to send EOS");

  if (mtask) {
    mpp_task_meta_set_packet (mtask, KEY_INPUT_PACKET, NULL);
    mpp_task_meta_set_frame (mtask, KEY_OUTPUT_FRAME, NULL);
    mppdec->mpi->enqueue (mppdec->mpp_ctx, MPP_PORT_INPUT, mtask);
  }

  if (mframe)
    mpp_frame_deinit (&mframe);

  return FALSE;
}

#define GST_TYPE_MPP_JPEG_DEC_FORMAT (gst_mpp_jpeg_dec_format_get_type ())
static GType
gst_mpp_jpeg_dec_format_get_type (void)
{
  static GType format = 0;

  if (!format) {
    static const GEnumValue formats[] = {
      {GST_VIDEO_FORMAT_UNKNOWN, "Auto", "auto"},
      {GST_VIDEO_FORMAT_NV12, "NV12", "NV12"},
#ifdef HAVE_RGA
      {GST_VIDEO_FORMAT_NV21, "NV21", "NV21"},
      {GST_VIDEO_FORMAT_I420, "I420", "I420"},
      {GST_VIDEO_FORMAT_YV12, "YV12", "YV12"},
      {GST_VIDEO_FORMAT_NV16, "NV16", "NV16"},
      {GST_VIDEO_FORMAT_NV61, "NV61", "NV61"},
      {GST_VIDEO_FORMAT_RGB, "RGB", "RGB"},
      {GST_VIDEO_FORMAT_BGR, "BGR", "BGR"},
#endif
      {GST_VIDEO_FORMAT_RGB16, "RGB565", "RGB16"},
      {GST_VIDEO_FORMAT_BGR16, "BGR565", "BGR16"},
      {GST_VIDEO_FORMAT_ARGB, "ARGB8888", "ARGB"},
      {GST_VIDEO_FORMAT_ABGR, "ABGR8888", "ABGR"},
      {GST_VIDEO_FORMAT_RGBA, "RGBA8888", "RGBA"},
      {GST_VIDEO_FORMAT_BGRA, "BGRA8888", "BGRA"},
      {GST_VIDEO_FORMAT_xRGB, "XRGB8888", "xRGB"},
      {GST_VIDEO_FORMAT_xBGR, "XBGR8888", "xBGR"},
      {GST_VIDEO_FORMAT_RGBx, "RGBX8888", "RGBx"},
      {GST_VIDEO_FORMAT_BGRx, "BGRX8888", "BGRx"},
      {0, NULL, NULL}
    };
    format = g_enum_register_static ("GstMppJpegDecFormat", formats);
  }
  return format;
}

static void
gst_mpp_jpeg_dec_init (GstMppJpegDec * self)
{
  GstMppDec *mppdec = GST_MPP_DEC (self);
  mppdec->format = DEFAULT_PROP_FORMAT;
}

static void
gst_mpp_jpeg_dec_setup_default_format (void)
{
  GEnumClass *class;
  GEnumValue *value;
  const gchar *env;

  env = g_getenv ("GST_MPP_JPEGDEC_DEFAULT_FORMAT");
  if (!env)
    return;

  class = g_type_class_ref (GST_TYPE_MPP_JPEG_DEC_FORMAT);

  value = g_enum_get_value_by_nick (class, env);
  if (value)
    DEFAULT_PROP_FORMAT = value->value;

  g_type_class_unref (class);
}

static void
gst_mpp_jpeg_dec_class_init (GstMppJpegDecClass * klass)
{
  GstVideoDecoderClass *decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GstMppDecClass *pclass = GST_MPP_DEC_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppjpegdec", 0,
      "MPP JPEG decoder");

  decoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_start);
  decoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_stop);
  decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_set_format);

  pclass->startup = NULL;
  pclass->get_mpp_packet = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_get_mpp_packet);
  pclass->send_mpp_packet =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_send_mpp_packet);
  pclass->poll_mpp_frame = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_poll_mpp_frame);
  pclass->shutdown = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_shutdown);

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_get_property);

  gst_mpp_jpeg_dec_setup_default_format ();

  g_object_class_install_property (gobject_class, PROP_FORMAT,
      g_param_spec_enum ("format", "Prefered output format",
          "Prefered output format",
          GST_TYPE_MPP_JPEG_DEC_FORMAT, DEFAULT_PROP_FORMAT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_dec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_dec_sink_template));

  gst_element_class_set_static_metadata (element_class,
      "Rockchip's MPP JPEG image decoder", "Decoder/Image",
      "JPEG hardware decoder",
      "Randy Li <randy.li@rock-chips.com>, "
      "Jeffy Chen <jeffy.chen@rock-chips.com>");
}
