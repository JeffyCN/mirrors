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

GST_DEBUG_CATEGORY (mppvideoenc_debug);
#define GST_CAT_DEFAULT mppvideoenc_debug

#define parent_class gst_mpp_video_enc_parent_class
G_DEFINE_ABSTRACT_TYPE (GstMppVideoEnc, gst_mpp_video_enc,
    GST_TYPE_VIDEO_ENCODER);

static GstStaticPadTemplate gst_mpp_video_enc_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw,"
        "format = (string) NV12, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1088 ], "
        "framerate = (fraction) [0/1, 60/1]"
        ";"
        "video/x-raw,"
        "format = (string) I420, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1088 ], "
        "framerate = (fraction) [0/1, 60/1]"
        ";"
        "video/x-raw,"
        "format = (string) YUY2, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1088 ], "
        "framerate = (fraction) [0/1, 60/1]"
        ";"
        "video/x-raw,"
        "format = (string) UYVY, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1088 ], "
        "framerate = (fraction) [0/1, 60/1]" ";"));

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
  self->outcaps = NULL;

  return TRUE;
}

static void
gst_mpp_video_enc_process_buffer_stopped (GstMppVideoEnc * self)
{
  if (g_atomic_int_get (&self->processing)) {
    GST_DEBUG_OBJECT (self, "Early stop of encoding thread");
    self->output_flow = GST_FLOW_FLUSHING;
    g_atomic_int_set (&self->processing, FALSE);
  }

  GST_DEBUG_OBJECT (self, "Encoding task destroyed: %s",
      gst_flow_get_name (self->output_flow));
}


static gboolean
gst_mpp_video_enc_stop (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  gint8 i;

  GST_DEBUG_OBJECT (self, "Stopping");

  gst_mpp_video_enc_process_buffer_stopped (self);

  GST_VIDEO_ENCODER_STREAM_LOCK (encoder);
  self->output_flow = GST_FLOW_OK;
  GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);

  g_assert (g_atomic_int_get (&self->active) == FALSE);
  g_assert (g_atomic_int_get (&self->processing) == FALSE);

  if (self->outcaps)
    gst_caps_unref (self->outcaps);

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

  /* Must be destroy before input_group */
  if (self->mpp_frame)
    mpp_frame_deinit (&self->mpp_frame);

  if (self->input_group) {
    mpp_buffer_group_put (self->input_group);
    self->input_group = NULL;
  }

  if (self->output_group) {
    mpp_buffer_group_put (self->output_group);
    self->output_group = NULL;
  }

  GST_DEBUG_OBJECT (self, "Stopped");

  return TRUE;
}

static MppFrameFormat
to_mpp_pixel (GstCaps * caps, GstVideoInfo * info)
{
  GstStructure *structure;
  const gchar *mimetype;

  structure = gst_caps_get_structure (caps, 0);
  mimetype = gst_structure_get_name (structure);

  if (g_str_equal (mimetype, "video/x-raw")) {
    switch (GST_VIDEO_INFO_FORMAT (info)) {
      case GST_VIDEO_FORMAT_I420:
        return MPP_FMT_YUV420P;
        break;
      case GST_VIDEO_FORMAT_NV12:
        return MPP_FMT_YUV420SP;
        break;
      case GST_VIDEO_FORMAT_YUY2:
        return MPP_FMT_YUV422_YUYV;
        break;
      case GST_VIDEO_FORMAT_UYVY:
        return MPP_FMT_YUV422_UYVY;
        break;
      default:
        break;
    }
  }
  return MPP_FMT_BUTT;
}

static gboolean
gst_mpp_video_enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  GstVideoInfo *info;
  gsize ver_stride, cr_h;
  GstVideoFormat format;
  MppEncPrepCfg prep_cfg;

  GST_DEBUG_OBJECT (self, "Setting format: %" GST_PTR_FORMAT, state->caps);

  if (self->input_state) {
    if (gst_caps_is_strictly_equal (self->input_state->caps, state->caps)) {
      GST_DEBUG_OBJECT (self, "Compatible caps");
      goto done;
    }
  }

  format = state->info.finfo->format;

  info = &self->info;
  gst_video_info_init (info);
  gst_video_info_set_format (info, format, GST_VIDEO_INFO_WIDTH (&state->info),
      GST_VIDEO_INFO_HEIGHT (&state->info));

  switch (info->finfo->format) {
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_I420:
      ver_stride = GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (info));
      info->offset[0] = 0;
      info->offset[1] = info->stride[0] * ver_stride;
      cr_h = GST_ROUND_UP_2 (ver_stride) / 2;
      info->size = info->offset[1] + info->stride[0] * cr_h;
      break;
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
      ver_stride = GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (info));
      cr_h = GST_ROUND_UP_2 (ver_stride);
      info->size = info->stride[0] * cr_h;
      break;
    default:
      g_assert_not_reached ();
      return FALSE;
  }

  memset (&prep_cfg, 0, sizeof (prep_cfg));
  prep_cfg.change = MPP_ENC_PREP_CFG_CHANGE_INPUT |
      MPP_ENC_PREP_CFG_CHANGE_FORMAT;
  prep_cfg.width = GST_VIDEO_INFO_WIDTH (&state->info);
  prep_cfg.height = GST_VIDEO_INFO_HEIGHT (&state->info);
  prep_cfg.format = to_mpp_pixel (state->caps, &state->info);
  prep_cfg.hor_stride = info->stride[0];
  prep_cfg.ver_stride = ver_stride;

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_SET_PREP_CFG, &prep_cfg)) {
    GST_DEBUG_OBJECT (self, "Setting input format for rockchip mpp failed");
    return FALSE;
  }

  if (self->mpi->control
      (self->mpp_ctx, MPP_ENC_GET_EXTRA_INFO, &self->sps_packet))
    self->sps_packet = NULL;

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

    gst_mpp_video_enc_process_buffer_stopped (self);

    GST_VIDEO_ENCODER_STREAM_LOCK (encoder);
  }
  self->output_flow = GST_FLOW_OK;

  self->mpi->reset (self->mpp_ctx);

  return TRUE;
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

static GstFlowReturn
gst_mpp_video_enc_process_buffer (GstMppVideoEnc * self, GstBuffer * buffer)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (self);
  GstBuffer *new_buffer = NULL;
  static gint8 current_index = 0;
  GstVideoCodecFrame *frame;
  gpointer ptr = NULL;
  MppFrame mpp_frame = self->mpp_frame;
  MppTask task = NULL;
  MppBuffer frame_in = self->input_buffer[current_index];
  MppBuffer pkt_buf_out = self->output_buffer[current_index];
  MppPacket packet = NULL;
  GstFlowReturn ret;

  mpp_frame_set_buffer (mpp_frame, frame_in);
  /* Eos buffer */
  if (0 == gst_buffer_get_size (buffer)) {
    mpp_frame_set_eos (mpp_frame, 1);
  } else {
    ptr = mpp_buffer_get_ptr (frame_in);

    gst_buffer_ref (buffer);
    gst_buffer_extract (buffer, 0, ptr, gst_buffer_get_size (buffer));
    gst_buffer_unref (buffer);

    mpp_frame_set_eos (mpp_frame, 0);
  }

  do {
    if (self->mpi->dequeue (self->mpp_ctx, MPP_PORT_INPUT, &task)) {
      GST_ERROR_OBJECT (self, "mpp task input dequeue failed");
      return GST_FLOW_ERROR;
    }
    if (NULL == task) {
      GST_LOG_OBJECT (self, "mpp input failed, try again");
      g_usleep (2);
    } else {
      break;
    }
  } while (1);
  mpp_task_meta_set_frame (task, KEY_INPUT_FRAME, mpp_frame);

  mpp_packet_init_with_buffer (&packet, pkt_buf_out);
  mpp_task_meta_set_packet (task, KEY_OUTPUT_PACKET, packet);

  if (self->mpi->enqueue (self->mpp_ctx, MPP_PORT_INPUT, task)) {
    GST_ERROR_OBJECT (self, "mpp task input enqueu failed");
  }

  GST_LOG_OBJECT (self, "Process output buffer");

  do {
    MppFrame packet_out = NULL;
    ret = GST_FLOW_OK;

    if (self->mpi->dequeue (self->mpp_ctx, MPP_PORT_OUTPUT, &task)) {
      g_usleep (2);
      continue;
    }

    if (task) {
      mpp_task_meta_get_packet (task, KEY_OUTPUT_PACKET, &packet_out);
      g_assert (packet_out == packet);

      /* Get result */
      if (packet) {
        gconstpointer *ptr = mpp_packet_get_pos (packet);
        gsize len = mpp_packet_get_length (packet);
        gint intra_flag = 0;

        if (mpp_packet_get_eos (packet))
          ret = GST_FLOW_EOS;

        mpp_task_meta_get_s32 (task, KEY_OUTPUT_INTRA, &intra_flag, 0);

        GST_LOG_OBJECT (self, "Allocate output buffer");
        if (intra_flag)
          new_buffer = gst_video_encoder_allocate_output_buffer (encoder,
              MAX_EXTRA_DATA + len);
        else
          new_buffer = gst_video_encoder_allocate_output_buffer (encoder, len);
        if (NULL == new_buffer) {
          ret = GST_FLOW_FLUSHING;
          goto beach;
        }

        /* Fill the buffer */
        if (intra_flag && self->sps_packet) {
          const gpointer *sps_ptr = mpp_packet_get_pos (self->sps_packet);
          gsize sps_len = mpp_packet_get_length (self->sps_packet);

          gst_buffer_fill (new_buffer, 0, sps_ptr, sps_len);

          gst_buffer_fill (new_buffer, sps_len, ptr, len);
        } else {
          gst_buffer_fill (new_buffer, 0, ptr, len);
        }

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
    frame->output_buffer = new_buffer;
    new_buffer = NULL;
    ret = gst_video_encoder_finish_frame (encoder, frame);

    if (ret != GST_FLOW_OK)
      goto beach;
  } else {
    GST_WARNING_OBJECT (self, "Encoder is producing too many buffers");
    gst_buffer_unref (new_buffer);
  }
  return GST_FLOW_OK;

beach:
  GST_DEBUG_OBJECT (self, "Leaving output thread");

  gst_buffer_replace (&buffer, NULL);
  self->output_flow = ret;
  g_atomic_int_set (&self->processing, FALSE);
  /* FIXME maybe I need to inform the rockchip mpp */
  return ret;
}

static GstFlowReturn
gst_mpp_video_enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame, GstCaps * outcaps)
{

  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  GstFlowReturn ret = GST_FLOW_OK;

  GST_DEBUG_OBJECT (self, "Handling frame %d", frame->system_frame_number);

  if (G_UNLIKELY (!g_atomic_int_get (&self->active)))
    goto flushing;

  /* FIXME don't use this as a flag */
  if (self->outcaps == NULL) {
    gint i = 0;
    gsize packet_size;

    GST_DEBUG_OBJECT (self, "Filling src caps with output dimensions %ux%u",
        self->info.width, self->info.height);
    packet_size = self->info.width * self->info.height;

    if (!outcaps)
      goto not_negotiated;

    gst_caps_set_simple (outcaps,
        "width", G_TYPE_INT, self->input_state->info.width,
        "height", G_TYPE_INT, self->input_state->info.height, NULL);

    /* Allocator buffers */
    if (mpp_buffer_group_get_internal (&self->input_group, MPP_BUFFER_TYPE_ION))
      goto activate_failed;
    if (mpp_buffer_group_get_internal (&self->output_group,
            MPP_BUFFER_TYPE_ION))
      goto activate_failed;

    for (i = 0; i < MPP_MAX_BUFFERS; i++) {
      if (mpp_buffer_get (self->input_group, &self->input_buffer[i],
              self->info.size))
        goto activate_failed;
      if (mpp_buffer_get (self->output_group, &self->output_buffer[i],
              packet_size))
        goto activate_failed;
    }

    if (mpp_frame_init (&self->mpp_frame)) {
      GST_DEBUG_OBJECT (self, "failed to set up mpp frame");
      goto activate_failed;
    }

    mpp_frame_set_width (self->mpp_frame, GST_VIDEO_INFO_WIDTH (&self->info));
    mpp_frame_set_height (self->mpp_frame, GST_VIDEO_INFO_HEIGHT (&self->info));
    mpp_frame_set_hor_stride (self->mpp_frame,
        GST_VIDEO_INFO_PLANE_STRIDE (&self->info, 0));
    mpp_frame_set_ver_stride (self->mpp_frame,
        GST_ROUND_UP_16 (GST_VIDEO_INFO_HEIGHT (&self->info)));

    if (self->mpi->poll (self->mpp_ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK))
      GST_ERROR_OBJECT (self, "mpp input poll failed");

    gst_video_encoder_set_output_state (encoder, outcaps, self->input_state);
    self->outcaps = gst_caps_ref (outcaps);

    if (!gst_video_encoder_negotiate (encoder)) {
      if (GST_PAD_IS_FLUSHING (GST_VIDEO_ENCODER_SRC_PAD (encoder)))
        goto flushing;
      else
        goto not_negotiated;
    }

  }
  if (g_atomic_int_get (&self->processing) == FALSE) {
    g_atomic_int_set (&self->processing, TRUE);
  }

  if (frame->input_buffer) {
    GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);
    ret = gst_mpp_video_enc_process_buffer (self, frame->input_buffer);
    GST_VIDEO_ENCODER_STREAM_LOCK (encoder);

    if (ret == GST_FLOW_FLUSHING) {
      if (g_atomic_int_get (&self->processing) == FALSE)
        ret = self->output_flow;
    } else if (ret == GST_FLOW_EOS) {
      ret = GST_FLOW_EOS;
      return ret;
    } else if (ret != GST_FLOW_OK) {
      goto process_failed;
    }
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
gst_mpp_video_enc_propose_allocation (GstVideoEncoder * encoder,
    GstQuery * query)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  GstBufferPool *pool;
  GstCaps *caps;
  GstStructure *config;
  GstVideoInfo info;
  guint size;

  gst_query_parse_allocation (query, &caps, NULL);
  if (caps == NULL)
    return FALSE;
  if (!gst_video_info_from_caps (&info, caps))
    return FALSE;

  size = GST_VIDEO_INFO_SIZE (&self->info);

  pool = gst_video_buffer_pool_new ();

  gst_query_add_allocation_pool (query, pool, size, 0, 0);
  config = gst_buffer_pool_get_config (pool);
  gst_buffer_pool_config_set_params (config, caps, size, 1, 0);
  gst_buffer_pool_set_config (pool, config);

  gst_object_unref (pool);
  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  return TRUE;
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
  /* buffer would be release here */
  gst_mpp_video_enc_process_buffer (self, buffer);

  /* Wait the task in srcpad get eos package */
  while (g_atomic_int_get (&self->processing));

  gst_mpp_video_enc_process_buffer_stopped (self);
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

  GST_DEBUG_CATEGORY_INIT (mppvideoenc_debug, "mppvideoenc", 0,
      "Rockchip MPP Video Encoder");

  video_encoder_class->close = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_close);
  video_encoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_start);
  video_encoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_stop);
  video_encoder_class->flush = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_flush);
  video_encoder_class->finish = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_finish);
  video_encoder_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_mpp_video_enc_propose_allocation);
  klass->handle_frame = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_handle_frame);
  klass->set_format = GST_DEBUG_FUNCPTR (gst_mpp_video_enc_set_format);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_video_enc_sink_template));
}
