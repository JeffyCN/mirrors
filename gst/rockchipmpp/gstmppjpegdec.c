/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
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

#include <gst/gst.h>

#include "gstmppbarebufferpool.h"
#include "gstmppjpegdec.h"

GST_DEBUG_CATEGORY (mpp_jpeg_dec_debug);
#define GST_CAT_DEFAULT mpp_jpeg_dec_debug

#define parent_class gst_mpp_jpeg_dec_parent_class
G_DEFINE_TYPE (GstMppJpegDec, gst_mpp_jpeg_dec, GST_TYPE_VIDEO_DECODER);

#define NB_OUTPUT_BUFS 4        /* nb frames necessary for display pipeline */

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

static MppCodingType
to_mpp_codec (GstStructure * s)
{
  if (gst_structure_has_name (s, "image/jpeg"))
    return MPP_VIDEO_CodingMJPEG;

  /* add more type here */
  return MPP_VIDEO_CodingUnused;
}

static GstVideoFormat
gst_mpp_get_jpeg_color (GstStructure * structure)
{
  const gchar *s;
  GstVideoFormat format = GST_VIDEO_FORMAT_UNKNOWN;

  if (gst_structure_has_name (structure, "image/jpeg")) {
    if (!(s = gst_structure_get_string (structure, "format")))
      return GST_VIDEO_FORMAT_UNKNOWN;

    format = gst_video_format_from_string (s);
  }
  return format;
}

static void
gst_mpp_jpeg_dec_unlock (GstMppJpegDec * self)
{
  if (self->pool && gst_buffer_pool_is_active (self->pool))
    gst_buffer_pool_set_flushing (self->pool, TRUE);
}

static void
gst_mpp_jpeg_dec_unlock_stop (GstMppJpegDec * self)
{
  if (self->pool && gst_buffer_pool_is_active (self->pool))
    gst_buffer_pool_set_flushing (self->pool, FALSE);
}

static gboolean
gst_mpp_jpeg_dec_close (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  if (self->mpp_ctx != NULL) {
    mpp_destroy (self->mpp_ctx);
    self->mpp_ctx = NULL;
  }

  GST_DEBUG_OBJECT (self, "Rockchip MPP context closed");

  return TRUE;
}

/* Open the device */
static gboolean
gst_mpp_jpeg_dec_open (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  if (mpp_create (&self->mpp_ctx, &self->mpi))
    return FALSE;

  GST_DEBUG_OBJECT (self, "created mpp context %p", self->mpp_ctx);
  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_start (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);

  GST_DEBUG_OBJECT (self, "Starting");
  gst_mpp_jpeg_dec_unlock (self);
  g_atomic_int_set (&self->active, TRUE);
  self->output_flow = GST_FLOW_OK;

  return TRUE;
}

static gboolean
gst_mpp_video_set_format (GstMppJpegDec * self, MppCodingType codec_format)
{
  if (mpp_init (self->mpp_ctx, MPP_CTX_DEC, codec_format))
    return FALSE;

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_finish (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstFlowReturn ret = GST_FLOW_OK;

done:
  return ret;
}

static GstStateChangeReturn
gst_mpp_jpeg_dec_change_state (GstElement * element, GstStateChange transition)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (element);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);

  if (transition == GST_STATE_CHANGE_PAUSED_TO_READY) {
    g_atomic_int_set (&self->active, FALSE);
    gst_mpp_jpeg_dec_unlock (self);
  }

  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static gboolean
gst_mpp_jpeg_dec_stop (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);

  GST_DEBUG_OBJECT (self, "Stopping");

  gst_mpp_jpeg_dec_unlock (self);

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  self->output_flow = GST_FLOW_OK;
  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  /* Should have been flushed already */
  g_assert (g_atomic_int_get (&self->active) == FALSE);

  /* Release all the internal references of the buffer */
  if (self->pool) {
    gst_object_unref (self->pool);
    self->pool = NULL;
  }

  mpp_buffer_put (self->input_buffer[0]);
  self->input_buffer[0] = NULL;
  if (self->input_group) {
    mpp_buffer_group_put (self->input_group);
    self->input_group = NULL;
  }

  if (self->input_state)
    gst_video_codec_state_unref (self->input_state);

  GST_DEBUG_OBJECT (self, "Stopped");

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_flush (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  gint ret = 0;
  ret = self->mpi->reset (self->mpp_ctx);

  /* Ensure the processing thread has stopped for the reverse playback
   * discount case */
  if (gst_pad_get_task_state (decoder->srcpad) == GST_TASK_STARTED) {
    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
    gst_mpp_jpeg_dec_unlock (self);
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  }
  self->output_flow = GST_FLOW_OK;

  gst_mpp_jpeg_dec_unlock_stop (self);
  return !ret;
}

static gboolean
gst_mpp_jpeg_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstStructure *structure;
  GstVideoFormat format;
  GstVideoInfo *info;
  gsize ver_stride, cr_h, mv_size;

  GST_DEBUG_OBJECT (self, "Setting format: %" GST_PTR_FORMAT, state->caps);

  structure = gst_caps_get_structure (state->caps, 0);

  if (self->input_state) {
    if (gst_caps_is_strictly_equal (self->input_state->caps, state->caps))
      goto done;
  } else {
    MppCodingType codingtype;
    codingtype = to_mpp_codec (structure);
    if (MPP_VIDEO_CodingUnused == codingtype)
      goto format_error;

    if (!gst_mpp_video_set_format (self, codingtype))
      goto device_error;
  }

  format = gst_mpp_get_jpeg_color (structure);
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

  info = &self->info;
  gst_video_info_init (info);
  gst_video_info_set_format (info, format, GST_VIDEO_INFO_WIDTH (&state->info),
      GST_VIDEO_INFO_HEIGHT (&state->info));

  switch (format) {
    case GST_VIDEO_FORMAT_NV12:
      info->stride[0] = GST_ROUND_UP_16 (info->stride[0]);
      ver_stride = GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (info));
      info->offset[0] = 0;
      info->offset[1] = info->stride[0] * ver_stride;
      cr_h = GST_ROUND_UP_2 (ver_stride) / 2;
      info->size = info->offset[1] + info->stride[0] * cr_h;
      mv_size = info->size / 3;
      info->size += mv_size;
      break;
    case GST_VIDEO_FORMAT_NV16:
      info->stride[0] = GST_ROUND_UP_16 (info->stride[0]);
      ver_stride = GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (info));
      info->offset[0] = 0;
      info->offset[1] = info->stride[0] * ver_stride;
      cr_h = GST_ROUND_UP_2 (ver_stride);
      info->size = info->stride[0] * cr_h * 2;
      break;
    default:
      g_assert_not_reached ();
      return FALSE;
  }

  self->input_state = gst_video_codec_state_ref (state);

done:
  return TRUE;

  /* Errors */
format_error:
  {
    GST_ERROR_OBJECT (self, "Unsupported format in caps: %" GST_PTR_FORMAT,
        state->caps);
    return FALSE;
  }
device_error:
  {
    GST_ERROR_OBJECT (self, "Failed to open the device");
    return FALSE;
  }
}

static GstFlowReturn
gst_mpp_jpeg_dec_handle_frame (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstBufferPool *pool = NULL;
  GstFlowReturn ret = GST_FLOW_OK;
  GstBuffer *outbuf = NULL;
  MppPacket mpkt = NULL;
  MPP_RET mret = 0;
  MppTask mtask = NULL;
  MppFrame mframe = NULL;

  GST_DEBUG_OBJECT (self, "Handling frame %d", frame->system_frame_number);

  if (G_UNLIKELY (!g_atomic_int_get (&self->active)))
    goto flushing;

  if (self->pool == NULL) {
    if (!self->input_state)
      goto not_negotiated;

    self->pool = gst_mpp_bare_buffer_pool_new (self, NULL);
    if (!self->pool)
      goto not_negotiated;
  }

  pool = GST_BUFFER_POOL (self->pool);
  if (!gst_buffer_pool_is_active (pool)) {
    GstVideoCodecState *output_state;
    GstVideoInfo *info = &self->info;
    GstStructure *config = gst_buffer_pool_get_config (pool);

    output_state =
        gst_video_decoder_set_output_state (decoder,
        info->finfo->format, info->width, info->height, self->input_state);
    gst_video_codec_state_unref (output_state);

    gst_buffer_pool_config_set_params (config, output_state->caps,
        self->info.size, NB_OUTPUT_BUFS, NB_OUTPUT_BUFS);

    if (!gst_buffer_pool_set_config (pool, config))
      goto error_activate_pool;
    /* activate the pool: the buffers are allocated */
    if (gst_buffer_pool_set_active (self->pool, TRUE) == FALSE)
      goto error_activate_pool;

    if (mpp_buffer_group_get_internal (&self->input_group, MPP_BUFFER_TYPE_ION))
      goto error_activate_pool;

    mpp_buffer_get (self->input_group, &self->input_buffer[0], self->info.size);

  }
#if 0
  ret = gst_buffer_pool_acquire_buffer (self->pool, &tmp, NULL);
  if (ret != GST_FLOW_OK)
    goto drop;
#endif

  ret = gst_buffer_pool_acquire_buffer (self->pool, &outbuf, NULL);
  if (ret != GST_FLOW_OK)
    goto drop;

  if (self->mpi->poll (self->mpp_ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK))
    goto start_task_failed;
  if (self->mpi->dequeue (self->mpp_ctx, MPP_PORT_INPUT, &mtask))
    goto drop;

#if 0
  mpp_packet_new (&mpkt);
  gst_buffer_copy_into (tmp, frame->input_buffer, GST_BUFFER_COPY_MEMORY, 0,
      -1);
  if (ret != GST_FLOW_OK)
    goto drop;
#else
  /* FIXME: performance bad */
  gst_buffer_extract (frame->input_buffer, 0,
      mpp_buffer_get_ptr (self->input_buffer[0]),
      gst_buffer_get_size (frame->input_buffer));

  mpp_packet_init_with_buffer (&mpkt, self->input_buffer[0]);
#endif
  mpp_task_meta_set_packet (mtask, KEY_INPUT_PACKET, mpkt);

  mpp_frame_init (&mframe);
  ret = gst_mpp_bare_buffer_pool_fill_frame (mframe, outbuf);
  if (ret != GST_FLOW_OK)
    goto drop;

  mpp_task_meta_set_frame (mtask, KEY_OUTPUT_FRAME, mframe);

  if (self->mpi->enqueue (self->mpp_ctx, MPP_PORT_INPUT, mtask))
    goto send_stream_error;

  if (self->mpi->poll (self->mpp_ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK))
    goto send_stream_error;

  mpp_packet_deinit (&mpkt);

  mret = self->mpi->dequeue (self->mpp_ctx, MPP_PORT_OUTPUT, &mtask);
  if (mtask) {
    GstVideoCodecFrame *frame = NULL;

    frame = gst_video_decoder_get_oldest_frame (decoder);
    frame->output_buffer = outbuf;
    ret = gst_video_decoder_finish_frame (decoder, frame);
    mret = self->mpi->enqueue (self->mpp_ctx, MPP_PORT_OUTPUT, mtask);
    mpp_frame_deinit (&mframe);
  }

  /* No need to keep input arround */

  gst_video_codec_frame_unref (frame);
  return ret;

  /* ERRORS */
error_activate_pool:
  {
    GST_ERROR_OBJECT (self, "Unable to activate the pool");
    ret = GST_FLOW_ERROR;
    goto drop;
  }
flushing:
  {
    ret = GST_FLOW_FLUSHING;
    goto drop;
  }
start_task_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, FAILED,
        ("Failed to start decoding thread."), (NULL));
    ret = GST_FLOW_ERROR;
    goto drop;
  }
not_negotiated:
  {
    GST_ERROR_OBJECT (self, "not negotiated");
    gst_video_decoder_drop_frame (decoder, frame);
    return GST_FLOW_NOT_NEGOTIATED;
  }
send_stream_error:
  {
    GST_ERROR_OBJECT (self, "send packet failed %d", mret);
    mpp_packet_deinit (&mpkt);
    return GST_FLOW_ERROR;
  }
drop:
  {
    GST_ERROR_OBJECT (self, "can't process this frame");
    gst_video_decoder_drop_frame (decoder, frame);
    return ret;
  }
}

static void
gst_mpp_jpeg_dec_class_init (GstMppJpegDecClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoDecoderClass *video_decoder_class = GST_VIDEO_DECODER_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_dec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_dec_sink_template));

  video_decoder_class->open = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_open);
  video_decoder_class->close = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_close);
  video_decoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_start);
  video_decoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_stop);
  video_decoder_class->finish = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_finish);
  video_decoder_class->set_format = GST_DEBUG_FUNCPTR
      (gst_mpp_jpeg_dec_set_format);
  video_decoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_handle_frame);
  video_decoder_class->flush = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_flush);

  element_class->change_state = GST_DEBUG_FUNCPTR
      (gst_mpp_jpeg_dec_change_state);

  GST_DEBUG_CATEGORY_INIT (mpp_jpeg_dec_debug, "mppjpegdec", 0,
      "mpp jpeg decoder");

  gst_element_class_set_static_metadata (element_class,
      "Rockchip's MPP JPEG image decoder", "Decoder/Image",
      "JPEG hardware decoder", "Randy Li <randy.li@rock-chips.com>");
}

static void
gst_mpp_jpeg_dec_init (GstMppJpegDec * self)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) self;

  gst_video_decoder_set_packetized (decoder, TRUE);

  self->active = FALSE;

  self->input_state = NULL;
}
