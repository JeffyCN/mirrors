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

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gst/gst.h>

#include "gstmppdecbufferpool.h"
#include "gstmppvideodec.h"

GST_DEBUG_CATEGORY (mpp_video_dec_debug);
#define GST_CAT_DEFAULT mpp_video_dec_debug

#define parent_class gst_mpp_video_dec_parent_class
G_DEFINE_TYPE (GstMppVideoDec, gst_mpp_video_dec, GST_TYPE_VIDEO_DECODER);

#define NB_OUTPUT_BUFS 22       /* nb frames necessary for display pipeline */

#define MPP_ALIGN(x, a)         (((x)+(a)-1)&~((a)-1))

/* GstVideoDecoder base class method */
static GstStaticPadTemplate gst_mpp_video_dec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264,"
        "stream-format = (string) { byte-stream },"
        "alignment = (string) { au }"
        ";"
        "video/x-h265,"
        "stream-format = (string) { byte-stream },"
        "alignment = (string) { au }"
        ";"
        "video/mpeg,"
        "mpegversion = (int) { 1, 2, 4 },"
        "systemstream = (boolean) false,"
        "parsed = (boolean) true" ";" "video/x-vp8" ";" "video/x-h263" ";")
    );

static GstStaticPadTemplate gst_mpp_video_dec_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) NV12, "
        "width  = (int) [ 32, 4096 ], " "height =  (int) [ 32, 4096 ]"
        ";"
        "video/x-raw, "
        "format = (string) P010_10LE, "
        "width  = (int) [ 32, 4096 ], " "height =  (int) [ 32, 4096 ]" ";")
    );

static gboolean gst_mpp_video_dec_flush (GstVideoDecoder * decoder);

static void
gst_mpp_video_dec_unlock (GstMppVideoDec * self)
{
  if (self->pool && gst_buffer_pool_is_active (self->pool))
    gst_buffer_pool_set_flushing (self->pool, TRUE);
}

static void
gst_mpp_video_dec_unlock_stop (GstMppVideoDec * self)
{
  if (self->pool && gst_buffer_pool_is_active (self->pool))
    gst_buffer_pool_set_flushing (self->pool, FALSE);
}

static gboolean
gst_mpp_video_dec_close (GstMppVideoDec * self)
{
  if (self->mpp_ctx != NULL) {
    mpp_destroy (self->mpp_ctx);
    self->mpp_ctx = NULL;
  }

  GST_DEBUG_OBJECT (self, "Rockchip MPP context closed");

  return TRUE;
}

/* Open the device */
static gboolean
gst_mpp_video_dec_start (GstVideoDecoder * decoder)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);

  GST_DEBUG_OBJECT (self, "Starting");
  gst_mpp_video_dec_unlock (self);
  self->output_flow = GST_FLOW_OK;

  return TRUE;
}

static gboolean
gst_mpp_video_dec_sendeos (GstVideoDecoder * decoder)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);
  MppPacket mpkt;

  mpp_packet_init (&mpkt, NULL, 0);
  mpp_packet_set_eos (mpkt);

  self->mpi->decode_put_packet (self->mpp_ctx, mpkt);
  mpp_packet_deinit (&mpkt);

  return TRUE;
}

static MppCodingType
to_mpp_codec (GstStructure * s)
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
        case 2:
          return MPP_VIDEO_CodingMPEG2;
          break;
        case 4:
          return MPP_VIDEO_CodingMPEG4;
          break;
        default:
          break;
      }
    }
  }

  if (gst_structure_has_name (s, "video/x-vp8"))
    return MPP_VIDEO_CodingVP8;

  /* add more type here */
  return MPP_VIDEO_CodingUnused;
}

static gboolean
gst_mpp_video_dec_stop (GstVideoDecoder * decoder)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);

  GST_DEBUG_OBJECT (self, "Stopping");

  /* Wait for mpp output thread to stop */
  gst_pad_stop_task (decoder->srcpad);

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  self->output_flow = GST_FLOW_OK;
  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  gst_mpp_video_dec_sendeos (decoder);

  /* Should have been flushed already */
  g_assert (g_atomic_int_get (&self->active) == FALSE);
  g_assert (g_atomic_int_get (&self->processing) == FALSE);

  if (self->input_state)
    gst_video_codec_state_unref (self->input_state);

  if (self->output_state)
    gst_video_codec_state_unref (self->output_state);

  gst_mpp_video_dec_close (self);
  if (self->pool) {
    gst_object_unref (self->pool);
    self->pool = NULL;
  }

  GST_DEBUG_OBJECT (self, "Stopped");

  return TRUE;
}

static gboolean
gst_mpp_video_dec_open (GstMppVideoDec * self, MppCodingType codec_format)
{
  mpp_create (&self->mpp_ctx, &self->mpi);

  GST_DEBUG_OBJECT (self, "created mpp context %p", self->mpp_ctx);

  if (mpp_init (self->mpp_ctx, MPP_CTX_DEC, codec_format))
    goto mpp_init_error;

  return TRUE;

mpp_init_error:
  {
    GST_ERROR_OBJECT (self, "rockchip context init failed");
    if (!self->mpp_ctx)
      mpp_destroy (self->mpp_ctx);
    return FALSE;
  }
}

static gboolean
gst_mpp_video_dec_flush (GstVideoDecoder * decoder)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);

  /* Ensure the processing thread has stopped for the reverse playback
   * discount case */
  if (g_atomic_int_get (&self->processing)) {
    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
    gst_mpp_video_dec_unlock (self);
    gst_pad_stop_task (decoder->srcpad);
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  }

  self->output_flow = GST_FLOW_OK;

  gst_mpp_video_dec_unlock_stop (self);
  return !self->mpi->reset (self->mpp_ctx);
}

static gboolean
gst_mpp_video_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);
  GstStructure *structure;
  GstVideoInfo *info;
  GstVideoAlignment *align;
  GstVideoCodecState *output_state;
  gint width, height, hor_stride, ver_stride, mv_size;
  MppFrame mframe = NULL;
  MppCodingType codingtype;
  const gchar *codec_profile;

  GST_DEBUG_OBJECT (self, "Setting format: %" GST_PTR_FORMAT, state->caps);

  structure = gst_caps_get_structure (state->caps, 0);

  if (self->input_state) {
    GstQuery *query = gst_query_new_drain ();

    if (gst_caps_is_strictly_equal (self->input_state->caps, state->caps))
      goto done;

    gst_video_codec_state_unref (self->input_state);
    self->input_state = NULL;

    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
    gst_mpp_video_dec_sendeos (decoder);
    do {
      /* Wait all the reminded buffers are pushed to downstream */
    } while (g_atomic_int_get (&self->processing));

    gst_mpp_video_dec_flush (decoder);
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    /* Query the downstream to release buffers from buffer pool */
    if (!gst_pad_peer_query (GST_VIDEO_DECODER_SRC_PAD (self), query))
      GST_DEBUG_OBJECT (self, "drain query failed");
    gst_query_unref (query);

    if (self->pool) {
      gst_object_unref (self->pool);
      self->pool = NULL;
    }

    self->output_flow = GST_FLOW_OK;

    g_atomic_int_set (&self->active, FALSE);
  }

  codingtype = to_mpp_codec (structure);
  if (MPP_VIDEO_CodingUnused == codingtype)
    goto format_error;

  if (!gst_mpp_video_dec_open (self, codingtype))
    goto device_error;

  self->input_state = gst_video_codec_state_ref (state);

  g_return_val_if_fail (gst_structure_get_int (structure,
          "width", &width) != 0, FALSE);
  g_return_val_if_fail (gst_structure_get_int (structure,
          "height", &height) != 0, FALSE);
  hor_stride = width;
  ver_stride = height;

  info = &self->info;
  gst_video_info_init (info);

  mpp_frame_init (&mframe);

  codec_profile = gst_structure_get_string (structure, "profile");
  if (codec_profile && g_strrstr (codec_profile, "10")) {
    /* FIXME the correct format is not P010 but a private format */
    info->finfo = gst_video_format_get_info (GST_VIDEO_FORMAT_P010_10LE);
    hor_stride = (hor_stride * 10) >> 3;
    mpp_frame_set_fmt (mframe, MPP_FMT_YUV420SP_10BIT);
  } else {
    info->finfo = gst_video_format_get_info (GST_VIDEO_FORMAT_NV12);
    mpp_frame_set_fmt (mframe, MPP_FMT_YUV420SP);
  }
  if (gst_structure_has_name (structure, "video/x-h265")) {
    hor_stride = MPP_ALIGN (hor_stride, 256) | 256;
    ver_stride = MPP_ALIGN (ver_stride, 8);
  } else {
    hor_stride = MPP_ALIGN (hor_stride, 16);
    ver_stride = MPP_ALIGN (ver_stride, 16);
  }

  mpp_frame_set_width (mframe, width);
  mpp_frame_set_height (mframe, height);
  mpp_frame_set_hor_stride (mframe, hor_stride);
  mpp_frame_set_ver_stride (mframe, ver_stride);

  if (self->mpi->control
      (self->mpp_ctx, MPP_DEC_SET_FRAME_INFO, (MppParam) mframe))
    goto device_error;

  mpp_frame_deinit (&mframe);

  info->width = width;
  info->height = height;
  info->offset[0] = 0;
  info->offset[1] = hor_stride * ver_stride;
  info->stride[0] = hor_stride;
  info->stride[1] = hor_stride;
  info->size = (hor_stride * ver_stride) * 3 / 2;
  mv_size = info->size * 2 / 6;
  info->size += mv_size;

  GST_INFO_OBJECT (self,
      "video info stride %d, offset %d, stride %d, offset %d",
      GST_VIDEO_INFO_PLANE_STRIDE (&self->info, 0),
      GST_VIDEO_INFO_PLANE_OFFSET (&self->info, 0),
      GST_VIDEO_INFO_PLANE_STRIDE (&self->info, 1),
      GST_VIDEO_INFO_PLANE_OFFSET (&self->info, 1));

  output_state =
      gst_video_decoder_set_output_state (GST_VIDEO_DECODER (self),
      info->finfo->format, info->width, info->height, self->input_state);

  if (self->output_state)
    gst_video_codec_state_unref (self->output_state);
  self->output_state = gst_video_codec_state_ref (output_state);

  align = &self->align;
  gst_video_alignment_reset (align);
  /* Compute padding and set buffer alignment */
  align->padding_right = hor_stride - width;
  align->padding_bottom = ver_stride - height;

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

static void
gst_mpp_video_dec_loop (GstVideoDecoder * decoder)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);
  GstBuffer *output_buffer;
  GstVideoCodecFrame *frame;
  GstFlowReturn ret = GST_FLOW_OK;

  ret = gst_buffer_pool_acquire_buffer (self->pool, &output_buffer, NULL);
  if (ret != GST_FLOW_OK)
    goto beach;

  frame = gst_video_decoder_get_oldest_frame (decoder);
  if (frame) {
    frame->output_buffer = output_buffer;

    output_buffer = NULL;
    ret = gst_video_decoder_finish_frame (decoder, frame);

    GST_TRACE_OBJECT (self, "finish buffer ts=%" GST_TIME_FORMAT,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (frame->output_buffer)));

    if (ret != GST_FLOW_OK)
      goto beach;
  } else {
    GST_WARNING_OBJECT (self, "Decoder is producing too many buffers");
    gst_buffer_unref (output_buffer);
  }

  return;

beach:
  GST_DEBUG_OBJECT (self, "Leaving output thread: %s", gst_flow_get_name (ret));

  gst_buffer_replace (&output_buffer, NULL);
  self->output_flow = ret;
  g_atomic_int_set (&self->processing, FALSE);
  gst_pad_pause_task (decoder->srcpad);
}

static void
gst_mpp_video_dec_loop_stopped (GstMppVideoDec * self)
{
  if (g_atomic_int_get (&self->processing)) {
    GST_DEBUG_OBJECT (self, "Early stop of decoding thread");
    self->output_flow = GST_FLOW_FLUSHING;
    g_atomic_int_set (&self->processing, FALSE);
  }

  GST_DEBUG_OBJECT (self, "Decoding task destroyed: %s",
      gst_flow_get_name (self->output_flow));
}

static GstFlowReturn
gst_mpp_video_dec_handle_frame (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame)
{
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);
  GstBufferPool *pool = NULL;
  GstMapInfo mapinfo = { 0, };
  GstFlowReturn ret = GST_FLOW_OK;
  MppPacket mpkt = NULL;
  MPP_RET mret = 0;

  GST_DEBUG_OBJECT (self, "Handling frame %d", frame->system_frame_number);

  if (!g_atomic_int_get (&self->active)) {
    if (!self->input_state)
      goto not_negotiated;
    self->pool = gst_mpp_dec_buffer_pool_new (self, NULL);
    g_atomic_int_set (&self->active, TRUE);
  }

  if (!self->pool) {
    goto not_negotiated;
  }

  pool = GST_BUFFER_POOL (self->pool);

  if (!gst_buffer_pool_is_active (pool)) {
    GstBuffer *codec_data;

    GstStructure *config = gst_buffer_pool_get_config (pool);
    /* FIXME if you suffer from the reconstruction of buffer pool which slows
     * down the decoding, then don't allocate more than 10 buffers here */
    gst_buffer_pool_config_set_params (config, self->output_state->caps,
        self->info.size, 22, 22);
    gst_buffer_pool_config_set_video_alignment (config, &self->align);

    if (!gst_buffer_pool_set_config (pool, config))
      goto error_activate_pool;
    /* activate the pool: the buffers are allocated */
    if (gst_buffer_pool_set_active (self->pool, TRUE) == FALSE)
      goto error_activate_pool;

    codec_data = self->input_state->codec_data;
    if (codec_data) {
      gst_buffer_ref (codec_data);
    } else {
      codec_data = frame->input_buffer;
      frame->input_buffer = NULL;
    }

    gst_buffer_map (codec_data, &mapinfo, GST_MAP_READ);
    mpp_packet_init (&mpkt, mapinfo.data, mapinfo.size);

    gst_buffer_unmap (codec_data, &mapinfo);

    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

    do {
      mret = self->mpi->decode_put_packet (self->mpp_ctx, mpkt);
    } while (MPP_ERR_BUFFER_FULL == mret);
    if (mret != 0)
      goto send_stream_error;

    mpp_packet_deinit (&mpkt);

    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    gst_buffer_unref (codec_data);
  }

  /* Start the output thread if it is not started before */
  if (g_atomic_int_get (&self->processing) == FALSE) {
    /* It's possible that the processing thread stopped due to an error */
    if (self->output_flow != GST_FLOW_OK &&
        self->output_flow != GST_FLOW_FLUSHING) {
      GST_DEBUG_OBJECT (self, "Processing loop stopped with error, leaving");
      ret = self->output_flow;
      goto drop;
    }

    GST_DEBUG_OBJECT (self, "Starting decoding thread");

    g_atomic_int_set (&self->processing, TRUE);
    if (!gst_pad_start_task (decoder->srcpad,
            (GstTaskFunction) gst_mpp_video_dec_loop, self,
            (GDestroyNotify) gst_mpp_video_dec_loop_stopped))
      goto start_task_failed;
  }

  if (frame->input_buffer) {
    gst_buffer_map (frame->input_buffer, &mapinfo, GST_MAP_READ);

    mpp_packet_init (&mpkt, mapinfo.data, mapinfo.size);

    gst_buffer_unmap (frame->input_buffer, &mapinfo);

    /* Unlock decoder before decode_sendstream call:
     * decode_sendstream must  block till frames
     * recycled, so decode_task must execute lock free..
     */

    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

    do {
      mret = self->mpi->decode_put_packet (self->mpp_ctx, mpkt);
    } while (MPP_ERR_BUFFER_FULL == mret);
    if (mret != 0)
      goto send_stream_error;

    mpp_packet_deinit (&mpkt);

    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    /* No need to keep input arround */
    gst_buffer_replace (&frame->input_buffer, NULL);
  }

  gst_video_codec_frame_unref (frame);

  return ret;

  /* ERRORS */
error_activate_pool:
  {
    GST_ERROR_OBJECT (self, "Unable to activate the pool");
    ret = GST_FLOW_ERROR;
    goto drop;
  }
start_task_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, FAILED,
        ("Failed to start decoding thread."), (NULL));
    g_atomic_int_set (&self->processing, FALSE);
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
    gst_video_decoder_drop_frame (decoder, frame);
    return ret;
  }
}

static GstStateChangeReturn
gst_mpp_video_dec_change_state (GstElement * element, GstStateChange transition)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (element);
  GstMppVideoDec *self = GST_MPP_VIDEO_DEC (decoder);

  if (transition == GST_STATE_CHANGE_PAUSED_TO_READY) {
    g_atomic_int_set (&self->active, FALSE);
    gst_mpp_video_dec_unlock (self);
    self->mpi->reset (self->mpp_ctx);
    gst_mpp_video_dec_sendeos (decoder);
    gst_pad_stop_task (decoder->srcpad);
  }

  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static void
gst_mpp_video_dec_class_init (GstMppVideoDecClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoDecoderClass *video_decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_dec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_dec_sink_template));

  video_decoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_start);
  video_decoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_stop);
  video_decoder_class->set_format = GST_DEBUG_FUNCPTR
      (gst_mpp_video_dec_set_format);
  video_decoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_video_dec_handle_frame);
  video_decoder_class->flush = GST_DEBUG_FUNCPTR (gst_mpp_video_dec_flush);

  element_class->change_state = GST_DEBUG_FUNCPTR
      (gst_mpp_video_dec_change_state);

  GST_DEBUG_CATEGORY_INIT (mpp_video_dec_debug, "mppvideodec", 0,
      "mpp video decoder");

  gst_element_class_set_static_metadata (element_class,
      "Rockchip's MPP video decoder", "Decoder/Video",
      "Multicodec (MPEG-2/4 / AVC / VP8 / HEVC) hardware decoder",
      "Randy Li <randy.li@rock-chips.com>");
}

static void
gst_mpp_video_dec_init (GstMppVideoDec * self)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) self;

  gst_video_decoder_set_packetized (decoder, TRUE);

  self->active = FALSE;

  self->input_state = NULL;
  self->output_state = NULL;
}
