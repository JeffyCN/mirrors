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

#include "gstmppvideoenc.h"

GST_DEBUG_CATEGORY_STATIC (gst_mpp_video_enc_debug);
#define GST_CAT_DEFAULT gst_mpp_video_enc_debug

#define parent_class gst_mpp_video_enc_parent_class;
G_DEFINE_TYPE (GstMppVideoEnc, gst_mpp_video_enc, GST_TYPE_VIDEO_ENCODER);


static GstStaticPadTemplate gst_mpp_video_enc_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) NV12, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1920 ], " "framerate = (fraction) [0/1, MAX]; "));

static GstStaticPadTemplate gst_mpp_video_enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1920 ], "
        "framerate = (fraction) [0/1, MAX], "
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, " "profile = (string) { high }")
    );

static gboolean
gst_mpp_video_enc_open (GstVideoEncoder * encoder)
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
gst_mpp_video_enc_close (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  if (self->mpp_ctx)
    mpp_destroy (self->mpp_ctx);

  return TRUE;
}

static gboolean
gst_mpp_video_enc_start (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  GST_DEBUG_OBJECT (self, "Starting");
  g_atomic_int_set (&self->active, TRUE);
  self->output_flow = GST_FLOW_OK;

  return TRUE;
}

static gboolean
gst_mpp_video_enc_stop (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  gint8 i;

  GST_DEBUG_OBJECT (self, "Stopping");

  gst_pad_stop_task (GST_VIDEO_ENCODER_SRC_PAD (encoder));

  GST_VIDEO_ENCODER_STREAM_LOCK (encoder);
  self->output_flow = GST_FLOW_OK;
  GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);

  g_assert (g_atomic_int_get (&self->active) == FALSE);
  g_assert (g_atomic_int_get (&self->processing) == FALSE);

  if (self->input_state) {
    gst_video_codec_state_unref (self->input_state);
    self->input_state = NULL;
  }

  for (i = 0; i < MPP_MAX_BUFFERS; i++) {
    if (self->input_buffer[i]) {
      mpp_buffer_put (self->input_buffer[i]);
      self->input_buffer[i] = NULL;
    }
    if (self->output_buffer[i]) {
      mpp_buffer_put (self->output_buffer[i]);
      self->output_buffer[i] = NULL;
    }
  }

  if (self->input_group) {
    mpp_buffer_group_put (self->input_group);
    self->input_group = NULL;
  }

  if (self->output_group) {
    mpp_buffer_group_put (self->output_group);
    self->output_group = NULL;
  }

  if (self->mpp_frame) {
    mpp_frame_deinit (&self->mpp_frame);
  }

  GST_DEBUG_OBJECT (self, "Stopped");

  return TRUE;
}

static gboolean
gst_mpp_video_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  MppEncConfig mpp_cfg;

  GST_DEBUG_OBJECT (self, "Setting format: %" GST_PTR_FORMAT, state->caps);

  if (self->input_state) {
    if (gst_caps_is_strictly_equal (self->input_state->caps, state->caps)) {
      GST_DEBUG_OBJECT (self, "Compatible caps");
      goto done;
    }

    GST_DEBUG_OBJECT (self, "Dynmaic update input caps in unsupported");
    return FALSE;
  }

  memset (&mpp_cfg, 0, sizeof (mpp_cfg));

  mpp_cfg.size = sizeof (mpp_cfg);
  mpp_cfg.width = GST_VIDEO_INFO_WIDTH (&state->info);
  mpp_cfg.height = GST_VIDEO_INFO_HEIGHT (&state->info);
  mpp_cfg.rc_mode = 1;
  mpp_cfg.skip_cnt = 0;
  mpp_cfg.fps_in = GST_VIDEO_INFO_FPS_N (&state->info) /
      GST_VIDEO_INFO_FPS_D (&state->info);
  mpp_cfg.fps_out = GST_VIDEO_INFO_FPS_N (&state->info) /
      GST_VIDEO_INFO_FPS_D (&state->info);
  mpp_cfg.gop = GST_VIDEO_INFO_FPS_N (&state->info) /
      GST_VIDEO_INFO_FPS_D (&state->info);
  mpp_cfg.bps = mpp_cfg.width * mpp_cfg.height * 2 * mpp_cfg.fps_in;

  mpp_cfg.format = MPP_FMT_YUV420SP;
  mpp_cfg.qp = 24;
  mpp_cfg.profile = 100;
  mpp_cfg.level = 41;
  mpp_cfg.cabac_en = 0;

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_SET_CFG, &mpp_cfg)) {
    GST_DEBUG_OBJECT (self, "Setting format for rockcip mpp failed");
    return FALSE;
  }

  self->input_state = gst_video_codec_state_ref (state);

done:
  return TRUE;
}

static gboolean
gst_mpp_video_enc_flush (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  GST_DEBUG_OBJECT (self, "Flushing");

  if (g_atomic_int_get (&self->processing)) {
    GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);
    self->mpi->reset (self->mpp_ctx);

    gst_pad_stop_task GST_VIDEO_ENCODER_SRC_PAD ((encoder));

    GST_VIDEO_ENCODER_STREAM_LOCK (encoder);
  }
  self->output_flow = GST_FLOW_OK;

  self->mpi->reset (self->mpp_ctx);

  return TRUE;
}

static GstFlowReturn
gst_mpp_video_enc_process_buffer (GstMppVideoEnc * self, GstBuffer * buffer)
{
  static gint8 current_index = 0;
  gpointer ptr = NULL;
  MppFrame frame = self->mpp_frame;
  MppTask task = NULL;
  MppBuffer frame_in = self->input_buffer[current_index];

  /* Eos buffer */
  if (0 == gst_buffer_get_size (buffer)) {
    mpp_frame_set_buffer (frame, NULL);
    mpp_frame_set_eos (frame, 1);
  } else {
    ptr = mpp_buffer_get_ptr (frame_in);
    gst_buffer_extract (buffer, 0, ptr, gst_buffer_get_size (buffer));
    mpp_frame_set_buffer (frame, frame_in);
    mpp_frame_set_eos (frame, 0);
  }

  do {
    if (self->mpi->dequeue (self->mpp_ctx, MPP_PORT_INPUT, &task)) {
      GST_ERROR_OBJECT (self, "mpp task input dequeue failed");
      return GST_FLOW_ERROR;
    }
    if (NULL == task) {
      g_usleep (1000);
    } else {
      break;
    }
  } while (1);
  mpp_task_meta_set_frame (task, MPP_META_KEY_INPUT_FRM, frame);
  self->last_input_task = task;

  if (self->mpi->enqueue (self->mpp_ctx, MPP_PORT_INPUT, task)) {
    GST_ERROR_OBJECT (self, "mpp task input enqueu failed");
  }

  return GST_FLOW_OK;
}

static GstVideoCodecFrame *
gst_mpp_video_enc_get_oldest_frame (GstVideoEncoder * encoder)
{
  GstVideoCodecFrame *frame = NULL;
  GList *frames, *l;
  gint count = 0;

  frames = gst_video_encoder_get_frames (encoder);

  for (l = frames; l != NULL; l = l->next) {
    GstVideoCodecFrame *f = l->data;

    if (!frame || frame->pts > f->pts)
      frame = f;

    count++;
  }
  if (frame) {
    GST_LOG_OBJECT (encoder,
        "Oldest frame is %d %" GST_TIME_FORMAT
        " and %d frames left",
        frame->system_frame_number, GST_TIME_ARGS (frame->pts), count - 1);
    gst_video_codec_frame_ref (frame);
  }

  g_list_free_full (frames, (GDestroyNotify) gst_video_codec_frame_unref);

  return frame;
}

static void
gst_mpp_video_enc_loop (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  static gint8 current_index = 0;
  GstVideoCodecFrame *frame;
  GstBuffer *buffer = NULL;
  MppTask task = NULL;
  MppBuffer pkt_buf_out = self->output_buffer[current_index];
  MppPacket packet = NULL;
  GstFlowReturn ret;

  GST_LOG_OBJECT (encoder, "Allocate output buffer");

  buffer = gst_video_encoder_allocate_output_buffer (encoder, MAX_CODEC_FRAME);

  if (NULL == buffer) {
    ret = GST_FLOW_FLUSHING;
    goto beach;
  }

  mpp_packet_init_with_buffer (&packet, pkt_buf_out);

  GST_LOG_OBJECT (encoder, "Process output buffer");

  mpp_task_meta_set_packet (self->last_input_task, MPP_META_KEY_OUTPUT_PKT,
      packet);

  do {
    MppFrame packet_out = NULL;
    ret = GST_FLOW_OK;

    if (self->mpi->dequeue (self->mpp_ctx, MPP_PORT_OUTPUT, &task)) {
      GST_ERROR_OBJECT (self, "mpp task output dequeue failed");
      ret = GST_FLOW_ERROR;
    }

    if (task) {
      mpp_task_meta_get_packet (task, MPP_META_KEY_OUTPUT_PKT, &packet_out);
      g_assert (packet_out == packet);

      /* Get result */
      if (packet) {
        gconstpointer *ptr = mpp_packet_get_pos (packet);
        gsize len = mpp_packet_get_length (packet);
        /* Fill the buffer */
        gst_buffer_fill (buffer, 0, ptr, len);

        mpp_packet_deinit (&packet);
      }

      if (self->mpi->enqueue (self->mpp_ctx, MPP_PORT_OUTPUT, task)) {
        GST_ERROR_OBJECT (self, "mpp task output enqueue failed");
        ret = GST_FLOW_ERROR;
      }

      current_index++;
      if (current_index >= MPP_MAX_BUFFERS)
        current_index = 0;

      break;
    }
  } while (1);
  if (ret != GST_FLOW_OK)
    goto beach;

  frame = gst_mpp_video_enc_get_oldest_frame (encoder);
  if (frame) {
    frame->output_buffer = buffer;
    buffer = NULL;
    ret = gst_video_encoder_finish_frame (encoder, frame);

    if (ret != GST_FLOW_OK)
      goto beach;
  } else {
    GST_WARNING_OBJECT (encoder, "Encoder is producing too many buffers");
    gst_buffer_unref (buffer);
  }

  return;

beach:
  GST_DEBUG_OBJECT (encoder, "Leaving output thread");

  gst_buffer_replace (&buffer, NULL);
  self->output_flow = ret;
  g_atomic_int_set (&self->processing, FALSE);
  /* FIXME maybe I need to inform the rockchip mpp */
  gst_pad_pause_task (GST_VIDEO_ENCODER_SRC_PAD (encoder));

}

static void
gst_mpp_video_enc_loop_stopped (GstMppVideoEnc * self)
{
  if (g_atomic_int_get (&self->processing)) {
    GST_DEBUG_OBJECT (self, "Early stop of encoding thread");
    self->output_flow = GST_FLOW_FLUSHING;
    g_atomic_int_set (&self->processing, FALSE);
  }

  GST_DEBUG_OBJECT (self, "Encoding task destroyed: %s",
      gst_flow_get_name (self->output_flow));
}

static GstFlowReturn
gst_mpp_video_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame)
{

  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  GstFlowReturn ret = GST_FLOW_OK;

  GST_DEBUG_OBJECT (self, "Handling frame %d", frame->system_frame_number);

  if (G_UNLIKELY (!g_atomic_int_get (&self->active)))
    goto flushing;

  if (self->outcaps == NULL) {
    gint i = 0;
    gint32 hor_stride, ver_stride;
    gsize frame_size, packet_size;
    GstStructure *structure;
    GstCaps *outcaps;

    hor_stride = MPP_ALIGN (self->input_state->info.width, 16);
    ver_stride = MPP_ALIGN (self->input_state->info.height, 16);

    GST_DEBUG_OBJECT (self, "Filling src caps with output dimensions %ux%u",
        hor_stride, ver_stride);
    frame_size = hor_stride * ver_stride * 3 / 2;
    packet_size =
        self->input_state->info.width * self->input_state->info.height;

    outcaps = gst_caps_new_empty_simple ("video/x-h264");
    structure = gst_caps_get_structure (outcaps, 0);
    gst_structure_set (structure, "stream-format",
        G_TYPE_STRING, "byte-stream", NULL);
    gst_structure_set (structure, "alignment", G_TYPE_STRING, "au", NULL);
    gst_caps_set_simple (outcaps,
        "width", G_TYPE_INT, hor_stride,
        "height", G_TYPE_INT, ver_stride, NULL);

    if (mpp_buffer_group_get_internal (&self->input_group, MPP_BUFFER_TYPE_ION))
      goto activate_failed;
    if (mpp_buffer_group_get_internal (&self->output_group,
            MPP_BUFFER_TYPE_ION))
      goto activate_failed;

    for (i = 0; i < MPP_MAX_BUFFERS; i++) {
      if (mpp_buffer_get (self->input_group, &self->input_buffer[i],
              frame_size))
        goto activate_failed;
      if (mpp_buffer_get (self->output_group, &self->output_buffer[i],
              packet_size))
        goto activate_failed;
    }

    if (mpp_frame_init (&self->mpp_frame)) {
      GST_DEBUG_OBJECT (self, "failed to set up mpp frame");
      goto activate_failed;
    }

    mpp_frame_set_width (self->mpp_frame, self->input_state->info.width);
    mpp_frame_set_height (self->mpp_frame, self->input_state->info.height);
    mpp_frame_set_hor_stride (self->mpp_frame, hor_stride);
    mpp_frame_set_ver_stride (self->mpp_frame, ver_stride);

    gst_video_encoder_set_output_state (encoder, outcaps, self->input_state);
    self->outcaps = outcaps;

    if (!gst_video_encoder_negotiate (encoder)) {
      if (GST_PAD_IS_FLUSHING (GST_VIDEO_ENCODER_SRC_PAD (encoder)))
        goto flushing;
      else
        goto not_negotiated;
    }

  }
  if (g_atomic_int_get (&self->processing) == FALSE) {
    if (self->output_flow != GST_FLOW_OK &&
        self->output_flow != GST_FLOW_FLUSHING) {
      GST_DEBUG_OBJECT (self, "Processing loop stopped with error, leaving");
      ret = self->output_flow;
      goto drop;
    }
    GST_DEBUG_OBJECT (self, "Starting encoding thread");
    g_atomic_int_set (&self->processing, TRUE);
    if (!gst_pad_start_task (encoder->srcpad,
            (GstTaskFunction) gst_mpp_video_enc_loop, self,
            (GDestroyNotify) gst_mpp_video_enc_loop_stopped))
      goto start_task_failed;
  }

  if (frame->input_buffer) {
    GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);
    ret = gst_mpp_video_enc_process_buffer (self, frame->input_buffer);
    GST_VIDEO_ENCODER_STREAM_LOCK (encoder);

    if (ret == GST_FLOW_FLUSHING) {
      if (g_atomic_int_get (&self->processing) == FALSE)
        ret = self->output_flow;
      goto drop;
    } else if (ret != GST_FLOW_OK) {
      goto process_failed;
    }
    gst_buffer_replace (&frame->input_buffer, NULL);
  }
  gst_video_codec_frame_unref (frame);

  return ret;
  /* ERROR */
not_negotiated:
  {
    GST_ERROR_OBJECT (self, "not negotiated");
    ret = GST_FLOW_NOT_NEGOTIATED;
    goto drop;
  }
activate_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS,
        ("Failed to allocate required memory."),
        ("Buffer pool activation failed"));
    return GST_FLOW_ERROR;
  }
flushing:
  {
    ret = GST_FLOW_FLUSHING;
    goto drop;
  }
start_task_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, FAILED,
        ("Failed to start encoding thread."), (NULL));
    g_atomic_int_set (&self->processing, FALSE);
    ret = GST_FLOW_ERROR;
    goto drop;
  }
process_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, FAILED,
        ("Failed to process frame."),
        ("Maybe be due to not enough memory or failing driver"));
    ret = GST_FLOW_ERROR;
    goto drop;
  }
drop:
  {
    gst_video_encoder_finish_frame (encoder, frame);
    return ret;
  }
}

static gboolean
gst_mpp_video_enc_finish (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  GstBuffer *buffer;

  if (!g_atomic_int_get (&self->processing))
    goto done;

  GST_DEBUG_OBJECT (self, "Finishing encoding");

  GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);

  /* Send a empty buffer to send eos frame to rockchip mpp */
  buffer = gst_buffer_new ();
  gst_mpp_video_enc_process_buffer (self, buffer);
  gst_buffer_unref (buffer);

  /* Wait the task in srcpad get eos package */
  while (g_atomic_int_get (&self->processing));

  gst_pad_stop_task (GST_VIDEO_ENCODER_SRC_PAD (encoder));
  GST_VIDEO_ENCODER_STREAM_LOCK (encoder);

  GST_DEBUG_OBJECT (encoder, "Done draning buffers");

done:
  return TRUE;
}

static void
gst_mpp_video_enc_init (GstMppVideoEnc * self)
{
}

static void
gst_mpp_video_enc_class_init (GstMppVideoEncClass * klass)
{
  GstElementClass *element_class;
  GstVideoEncoderClass *video_encoder_class;

  element_class = (GstElementClass *) klass;
  video_encoder_class = (GstVideoEncoderClass *) klass;

  GST_DEBUG_CATEGORY_INIT (gst_mpp_video_enc_debug, "mppvideoenc", 0,
      "Rockchip Mpp Video Encoder");

  gst_element_class_set_static_metadata (element_class,
      "Rockchip Mpp Video Encoder",
      "Codec/Encoder/Video",
      "Encode video streams via Rockchip Mpp",
      "Randy Li <randy.li@rock-chips.com>");

  video_encoder_class->open = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_open);
  video_encoder_class->close = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_close);
  video_encoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_start);
  video_encoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_stop);
  video_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (gst_mpp_video_enc_set_format);
  video_encoder_class->flush = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_flush);
  video_encoder_class->finish = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_finish);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_mpp_video_enc_handle_frame);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_enc_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_enc_sink_template));

}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "mppvideoenc", GST_RANK_PRIMARY + 1,
          gst_mpp_video_enc_get_type ()))
    return FALSE;
  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    mppvideoenc,
    "Rockchip Mpp Video Encoder",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
