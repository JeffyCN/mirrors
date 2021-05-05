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

/* GstVideoDecoder base class method */
static GstStaticPadTemplate gst_mpp_jpeg_dec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jpeg," "parsed = (boolean) true" ";")
    );

static GstStaticPadTemplate gst_mpp_jpeg_dec_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) NV12, "
        "width  = (int) [ 48, 8176 ], " "height =  (int) [ 48, 8176 ]"
        ";"
        "video/x-raw, "
        "format = (string) NV16, "
        "width  = (int) [ 48, 8176 ], " "height =  (int) [ 48, 8176 ]" ";")
    );

static GstVideoFormat
gst_mpp_jpeg_dec_get_format (GstStructure * structure)
{
  const gchar *s;

  if ((s = gst_structure_get_string (structure, "format")))
    return gst_video_format_from_string (s);

  return GST_VIDEO_FORMAT_UNKNOWN;
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
  GstVideoInfo *info = &state->info;
  GstStructure *structure;
  GstVideoFormat format;

  if (!pclass->set_format (decoder, state))
    return FALSE;

  structure = gst_caps_get_structure (state->caps, 0);
  format = gst_mpp_jpeg_dec_get_format (structure);
  switch (format) {
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      format = GST_VIDEO_FORMAT_NV12;
      break;
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_NV16:
      format = GST_VIDEO_FORMAT_NV16;
      break;
    default:
      g_assert_not_reached ();
      return FALSE;
  }

  if (!gst_mpp_dec_update_video_info (decoder, format,
          GST_VIDEO_INFO_WIDTH (info), GST_VIDEO_INFO_HEIGHT (info), 0, 0, 0))
    return FALSE;

  /* The MPP requires hor_stride * ver_stride / 2 for extra info */
  info = &mppdec->info;
  self->buf_size =
      GST_VIDEO_INFO_SIZE (info) + GST_VIDEO_INFO_PLANE_OFFSET (info, 1) / 2;

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_shutdown (GstVideoDecoder * decoder, gboolean unused)
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

static void
gst_mpp_jpeg_dec_init (GstMppJpegDec * self)
{
}

static void
gst_mpp_jpeg_dec_class_init (GstMppJpegDecClass * klass)
{
  GstVideoDecoderClass *decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GstMppDecClass *pclass = GST_MPP_DEC_CLASS (klass);
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
