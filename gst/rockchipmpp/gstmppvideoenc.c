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

#define DEFAULT_PROP_HEADER_MODE MPP_ENC_HEADER_MODE_DEFAULT    /* First frame */
#define DEFAULT_PROP_SEI_MODE MPP_ENC_SEI_MODE_DISABLE
#define DEFAULT_PROP_RC_MODE MPP_ENC_RC_MODE_CBR
#define DEFAULT_PROP_ROTATION MPP_ENC_ROT_0
#define DEFAULT_PROP_GOP -1     /* Same as FPS */
#define DEFAULT_PROP_MAX_REENC 1
#define DEFAULT_PROP_BPS 0      /* Auto */
#define DEFAULT_PROP_BPS_MIN 0  /* Auto */
#define DEFAULT_PROP_BPS_MAX 0  /* Auto */

enum
{
  PROP_0,
  PROP_HEADER_MODE,
  PROP_RC_MODE,
  PROP_ROTATION,
  PROP_SEI_MODE,
  PROP_GOP,
  PROP_MAX_REENC,
  PROP_BPS,
  PROP_BPS_MIN,
  PROP_BPS_MAX,
  PROP_LAST,
};

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

static void
gst_mpp_video_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  switch (prop_id) {
    case PROP_HEADER_MODE:{
      MppEncHeaderMode header_mode = g_value_get_enum (value);
      if (self->header_mode == header_mode)
        return;

      self->header_mode = header_mode;
      break;
    }
    case PROP_SEI_MODE:{
      MppEncSeiMode sei_mode = g_value_get_enum (value);
      if (self->sei_mode == sei_mode)
        return;

      self->sei_mode = sei_mode;
      break;
    }
    case PROP_RC_MODE:{
      MppEncRcMode rc_mode = g_value_get_enum (value);
      if (self->rc_mode == rc_mode)
        return;

      self->rc_mode = rc_mode;
      break;
    }
    case PROP_ROTATION:{
      MppEncRotationCfg rotation = g_value_get_enum (value);
      if (self->rotation == rotation)
        return;

      self->rotation = rotation;
      break;
    }
    case PROP_GOP:{
      gint gop = g_value_get_int (value);
      if (self->gop == gop)
        return;

      self->gop = gop;
      break;
    }
    case PROP_MAX_REENC:{
      guint max_reenc = g_value_get_uint (value);
      if (self->max_reenc == max_reenc)
        return;

      self->max_reenc = max_reenc;
      break;
    }
    case PROP_BPS:{
      gint bps = g_value_get_uint (value);
      if (self->bps == bps)
        return;

      self->bps = bps;
      break;
    }
    case PROP_BPS_MIN:{
      gint bps_min = g_value_get_uint (value);
      if (self->bps_min == bps_min)
        return;

      self->bps_min = bps_min;
      break;
    }
    case PROP_BPS_MAX:{
      gint bps_max = g_value_get_uint (value);
      if (self->bps_max == bps_max)
        return;

      self->bps_max = bps_max;
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }

  self->prop_dirty = TRUE;
}

static void
gst_mpp_video_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (object);
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  switch (prop_id) {
    case PROP_HEADER_MODE:
      g_value_set_enum (value, self->header_mode);
      break;
    case PROP_SEI_MODE:
      g_value_set_enum (value, self->sei_mode);
      break;
    case PROP_RC_MODE:
      g_value_set_enum (value, self->rc_mode);
      break;
    case PROP_ROTATION:
      g_value_set_enum (value, self->rotation);
      break;
    case PROP_GOP:
      g_value_set_int (value, self->gop);
      break;
    case PROP_MAX_REENC:
      g_value_set_uint (value, self->max_reenc);
      break;
    case PROP_BPS:
      g_value_set_uint (value, self->bps);
      break;
    case PROP_BPS_MIN:
      g_value_set_uint (value, self->bps_min);
      break;
    case PROP_BPS_MAX:
      g_value_set_uint (value, self->bps_max);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static gboolean
gst_mpp_video_enc_close (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  if (self->mpp_ctx)
    mpp_destroy (self->mpp_ctx);

  return TRUE;
}

static void
gst_mpp_video_enc_update_properties (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);
  MppEncCfg cfg;
  gint fps_num = GST_VIDEO_INFO_FPS_N (&self->input_state->info);
  gint fps_denorm = GST_VIDEO_INFO_FPS_D (&self->input_state->info);
  gint fps = fps_num / fps_denorm;

  if (!self->prop_dirty)
    return;

  if (mpp_enc_cfg_init (&cfg)) {
    GST_WARNING_OBJECT (self, "Init enc cfg failed");
    goto out;
  }

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_GET_CFG, cfg)) {
    GST_WARNING_OBJECT (self, "Get enc cfg failed");
    mpp_enc_cfg_deinit (cfg);
    goto out;
  }

  mpp_enc_cfg_set_s32 (cfg, "prep:rotation", self->rotation);
  mpp_enc_cfg_set_s32 (cfg, "rc:gop", self->gop < 0 ? fps : self->gop);
  mpp_enc_cfg_set_u32 (cfg, "rc:max_reenc_times", self->max_reenc);
  mpp_enc_cfg_set_s32 (cfg, "rc:mode", self->rc_mode);

  if (!self->bps)
    self->bps = self->info.width * self->info.height / 8 * fps;

  if (self->rc_mode == MPP_ENC_RC_MODE_CBR) {
    /* CBR mode has narrow bound */
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_target", self->bps);
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_max",
        self->bps_max ? self->bps_max : self->bps * 17 / 16);
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_min",
        self->bps_min ? self->bps_min : self->bps * 15 / 16);
  } else if (self->rc_mode == MPP_ENC_RC_MODE_VBR) {
    /* VBR mode has wide bound */
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_target", self->bps);
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_max",
        self->bps_max ? self->bps_max : self->bps * 17 / 16);
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_min",
        self->bps_min ? self->bps_min : self->bps * 1 / 16);
  } else {
    /* BPS settings are ignored in FIXQP mode */
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_target", -1);
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_max", -1);
    mpp_enc_cfg_set_s32 (cfg, "rc:bps_min", -1);
  }

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_SET_CFG, cfg))
    GST_WARNING_OBJECT (self, "Set enc cfg failed");

  mpp_enc_cfg_deinit (cfg);

out:
  if (self->mpi->control (self->mpp_ctx, MPP_ENC_SET_SEI_CFG, &self->sei_mode))
    GST_WARNING_OBJECT (self, "Set sei mode failed");

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_SET_HEADER_MODE,
          &self->header_mode))
    GST_WARNING_OBJECT (self, "Set header mode failed");

  self->prop_dirty = FALSE;
}

static gboolean
gst_mpp_video_enc_start (GstVideoEncoder * encoder)
{
  GstMppVideoEnc *self = GST_MPP_VIDEO_ENC (encoder);

  GST_DEBUG_OBJECT (self, "Starting");
  g_atomic_int_set (&self->active, TRUE);
  self->output_flow = GST_FLOW_OK;
  self->negotiated = FALSE;

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

  self->negotiated = FALSE;

  if (self->input_state) {
    gst_video_codec_state_unref (self->input_state);
    self->input_state = NULL;
  }

  for (i = 0; i < MPP_MAX_BUFFERS; i++) {
    if (self->input_buffer[i]) {
      mpp_buffer_put (self->input_buffer[i]);
      self->input_buffer[i] = NULL;
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
  MppEncCfg cfg;

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

  if (mpp_enc_cfg_init (&cfg)) {
    GST_WARNING_OBJECT (self, "Init enc cfg failed");
    return FALSE;
  }

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_GET_CFG, cfg)) {
    GST_WARNING_OBJECT (self, "Get enc cfg failed");
    mpp_enc_cfg_deinit (cfg);
    return FALSE;
  }

  mpp_enc_cfg_set_s32 (cfg, "prep:format",
      to_mpp_pixel (state->caps, &state->info));
  mpp_enc_cfg_set_s32 (cfg, "prep:width", GST_VIDEO_INFO_WIDTH (&state->info));
  mpp_enc_cfg_set_s32 (cfg, "prep:height",
      GST_VIDEO_INFO_HEIGHT (&state->info));
  mpp_enc_cfg_set_s32 (cfg, "prep:hor_stride", info->stride[0]);
  mpp_enc_cfg_set_s32 (cfg, "prep:ver_stride", ver_stride);

  mpp_enc_cfg_set_s32 (cfg, "rc:fps_in_flex", 0);
  mpp_enc_cfg_set_s32 (cfg, "rc:fps_in_num",
      GST_VIDEO_INFO_FPS_N (&state->info));
  mpp_enc_cfg_set_s32 (cfg, "rc:fps_in_denorm",
      GST_VIDEO_INFO_FPS_D (&state->info));
  mpp_enc_cfg_set_s32 (cfg, "rc:fps_out_flex", 0);
  mpp_enc_cfg_set_s32 (cfg, "rc:fps_out_num",
      GST_VIDEO_INFO_FPS_N (&state->info));
  mpp_enc_cfg_set_s32 (cfg, "rc:fps_out_denorm",
      GST_VIDEO_INFO_FPS_D (&state->info));

  if (self->mpi->control (self->mpp_ctx, MPP_ENC_SET_CFG, cfg)) {
    GST_WARNING_OBJECT (self, "Set enc cfg failed");
    mpp_enc_cfg_deinit (cfg);
    return FALSE;
  }

  mpp_enc_cfg_deinit (cfg);

  self->input_state = gst_video_codec_state_ref (state);

  gst_mpp_video_enc_update_properties (encoder);

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
gst_mpp_video_enc_process_buffer (GstMppVideoEnc * self, GstBuffer ** buffer)
{
  GstVideoEncoder *encoder = GST_VIDEO_ENCODER (self);
  GstBuffer *new_buffer = NULL;
  static gint8 current_index = 0;
  GstVideoCodecFrame *frame;
  MppFrame mpp_frame = self->mpp_frame;
  MppBuffer frame_in = self->input_buffer[current_index];
  MppPacket packet = NULL;
  GstFlowReturn ret = GST_FLOW_OK;

  mpp_frame_set_buffer (mpp_frame, frame_in);
  /* Eos buffer */
  if (0 == gst_buffer_get_size (*buffer)) {
    mpp_frame_set_eos (mpp_frame, 1);
  } else {
    /* Convert input buffer to mpp frame */
    /* TODO: Use RGA */

    GstVideoMeta *meta = gst_buffer_get_video_meta (*buffer);
    GstVideoInfo *info = &self->info;
    GstMapInfo mapinfo = { 0, };
    gconstpointer *ptr = mpp_buffer_get_ptr (frame_in);
    guint8 *src, *dst;
    gint i, j, src_stride, dst_stride, cr_h;

    cr_h = GST_VIDEO_INFO_HEIGHT (info);
    if (info->finfo->format == GST_VIDEO_FORMAT_NV12 ||
        info->finfo->format == GST_VIDEO_FORMAT_I420)
      cr_h /= 2;

    gst_buffer_map (*buffer, &mapinfo, GST_MAP_READ);
    for (i = 0, src = mapinfo.data; i < GST_VIDEO_INFO_N_PLANES (info); i++) {
      gint height = i ? cr_h : GST_VIDEO_INFO_HEIGHT (info);
      dst = (guint8 *) ptr + GST_VIDEO_INFO_PLANE_OFFSET (info, i);

      src_stride = dst_stride = GST_VIDEO_INFO_PLANE_STRIDE (info, i);
      if (meta) {
        src = mapinfo.data + meta->offset[i];
        src_stride = meta->stride[i];
      }

      for (j = 0; j < height; j++) {
        memcpy (dst, src, src_stride > dst_stride ? dst_stride : src_stride);
        src += src_stride;
        dst += dst_stride;
      }
    }
    gst_buffer_unmap (*buffer, &mapinfo);

    mpp_frame_set_eos (mpp_frame, 0);
  }

  if (self->mpi->encode_put_frame (self->mpp_ctx, mpp_frame)) {
    GST_ERROR_OBJECT (self, "mpp put frame failed");
    return GST_FLOW_ERROR;
  }

  GST_LOG_OBJECT (self, "Process output buffer");

  if (self->mpi->encode_get_packet (self->mpp_ctx, &packet)) {
    GST_ERROR_OBJECT (self, "mpp get packet failed");
    return GST_FLOW_ERROR;
  }

  /* Get result */
  if (packet) {
    gconstpointer *ptr = mpp_packet_get_pos (packet);
    gsize len = mpp_packet_get_length (packet);

    if (mpp_packet_get_eos (packet))
      ret = GST_FLOW_EOS;

    GST_LOG_OBJECT (self, "Allocate output buffer");
    new_buffer = gst_video_encoder_allocate_output_buffer (encoder, len);
    if (NULL == new_buffer) {
      ret = GST_FLOW_FLUSHING;
      goto beach;
    }

    gst_buffer_fill (new_buffer, 0, ptr, len);
    mpp_packet_deinit (&packet);
  }

  current_index++;
  if (current_index >= MPP_MAX_BUFFERS)
    current_index = 0;
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

  gst_buffer_replace (buffer, NULL);
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
  GstVideoCodecState *output;
  GstFlowReturn ret = GST_FLOW_OK;

  GST_DEBUG_OBJECT (self, "Handling frame %d", frame->system_frame_number);

  gst_mpp_video_enc_update_properties (encoder);

  if (G_UNLIKELY (!g_atomic_int_get (&self->active)))
    goto flushing;

  if (!self->negotiated) {
    gint i = 0;

    GST_DEBUG_OBJECT (self, "Filling src caps with output dimensions %ux%u",
        self->info.width, self->info.height);

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

    output = gst_video_encoder_set_output_state (encoder,
        gst_caps_ref (outcaps), self->input_state);
    gst_video_codec_state_unref (output);

    if (!gst_video_encoder_negotiate (encoder)) {
      if (GST_PAD_IS_FLUSHING (GST_VIDEO_ENCODER_SRC_PAD (encoder)))
        goto flushing;
      else
        goto not_negotiated;
    }

    self->negotiated = TRUE;
  }
  if (g_atomic_int_get (&self->processing) == FALSE) {
    g_atomic_int_set (&self->processing, TRUE);
  }

  if (frame->input_buffer) {
    GST_VIDEO_ENCODER_STREAM_UNLOCK (encoder);
    ret = gst_mpp_video_enc_process_buffer (self, &frame->input_buffer);
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
  gst_mpp_video_enc_process_buffer (self, &buffer);

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
  self->header_mode = DEFAULT_PROP_HEADER_MODE;
  self->sei_mode = DEFAULT_PROP_SEI_MODE;
  self->rc_mode = DEFAULT_PROP_RC_MODE;
  self->rotation = DEFAULT_PROP_ROTATION;
  self->gop = DEFAULT_PROP_GOP;
  self->max_reenc = DEFAULT_PROP_MAX_REENC;
  self->bps = DEFAULT_PROP_BPS;
  self->bps_min = DEFAULT_PROP_BPS_MIN;
  self->bps_max = DEFAULT_PROP_BPS_MAX;
  self->prop_dirty = TRUE;
}

#define GST_TYPE_MPP_ENC_HEADER_MODE (gst_mpp_enc_header_mode_get_type ())
static GType
gst_mpp_enc_header_mode_get_type (void)
{
  static GType header_mode = 0;

  if (!header_mode) {
    static const GEnumValue modes[] = {
      {MPP_ENC_HEADER_MODE_DEFAULT, "Only in the first frame", "first-frame"},
      {MPP_ENC_HEADER_MODE_EACH_IDR, "In every IDR frames", "each-idr"},
      {0, NULL, NULL}
    };
    header_mode = g_enum_register_static ("MppEncHeaderMode", modes);
  }
  return header_mode;
}

#define GST_TYPE_MPP_ENC_SEI_MODE (gst_mpp_enc_sei_mode_get_type ())
static GType
gst_mpp_enc_sei_mode_get_type (void)
{
  static GType sei_mode = 0;

  if (!sei_mode) {
    static const GEnumValue modes[] = {
      {MPP_ENC_SEI_MODE_DISABLE, "SEI disabled", "disable"},
      {MPP_ENC_SEI_MODE_ONE_SEQ, "One SEI per sequence", "one-seq"},
      {MPP_ENC_SEI_MODE_ONE_FRAME, "One SEI per frame(if changed)",
          "one-frame"},
      {0, NULL, NULL}
    };
    sei_mode = g_enum_register_static ("GstMppEncSeiMode", modes);
  }
  return sei_mode;
}

#define GST_TYPE_MPP_ENC_RC_MODE (gst_mpp_enc_rc_mode_get_type ())
static GType
gst_mpp_enc_rc_mode_get_type (void)
{
  static GType rc_mode = 0;

  if (!rc_mode) {
    static const GEnumValue modes[] = {
      {MPP_ENC_RC_MODE_VBR, "Variable bitrate", "vbr"},
      {MPP_ENC_RC_MODE_CBR, "Constant bitrate", "cbr"},
      {MPP_ENC_RC_MODE_FIXQP, "Fixed QP", "fixqp"},
      {0, NULL, NULL}
    };
    rc_mode = g_enum_register_static ("GstMppEncRcMode", modes);
  }
  return rc_mode;
}

#define GST_TYPE_MPP_ENC_ROTATION (gst_mpp_enc_rotation_get_type ())
static GType
gst_mpp_enc_rotation_get_type (void)
{
  static GType rotation = 0;

  if (!rotation) {
    static const GEnumValue rotations[] = {
      {MPP_ENC_ROT_0, "Rotate 0", "0"},
      {MPP_ENC_ROT_90, "Rotate 90", "90"},
      {MPP_ENC_ROT_180, "Rotate 180", "180"},
      {MPP_ENC_ROT_270, "Rotate 270", "270"},
      {0, NULL, NULL}
    };
    rotation = g_enum_register_static ("GstMppEncRotation", rotations);
  }
  return rotation;
}

static void
gst_mpp_video_enc_class_init (GstMppVideoEncClass * klass)
{
  GstElementClass *element_class;
  GObjectClass *gobject_class;
  GstVideoEncoderClass *video_encoder_class;

  element_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;
  video_encoder_class = (GstVideoEncoderClass *) klass;

  GST_DEBUG_CATEGORY_INIT (mppvideoenc_debug, "mppvideoenc", 0,
      "Rockchip MPP Video Encoder");

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_video_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_video_enc_get_property);

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

  g_object_class_install_property (gobject_class, PROP_HEADER_MODE,
      g_param_spec_enum ("header-mode", "Header mode",
          "Header mode",
          GST_TYPE_MPP_ENC_HEADER_MODE, DEFAULT_PROP_HEADER_MODE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SEI_MODE,
      g_param_spec_enum ("sei-mode", "SEI mode",
          "SEI mode",
          GST_TYPE_MPP_ENC_SEI_MODE, DEFAULT_PROP_SEI_MODE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_RC_MODE,
      g_param_spec_enum ("rc-mode", "RC mode",
          "RC mode",
          GST_TYPE_MPP_ENC_RC_MODE, DEFAULT_PROP_RC_MODE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ROTATION,
      g_param_spec_enum ("rotation", "Rotation",
          "Rotation",
          GST_TYPE_MPP_ENC_ROTATION, DEFAULT_PROP_ROTATION,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_GOP,
      g_param_spec_int ("gop", "Group of pictures",
          "Group of pictures starting with I frame (-1 = FPS, 1 = all I frames)",
          -1, G_MAXINT, DEFAULT_PROP_GOP,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MAX_REENC,
      g_param_spec_uint ("max-reenc", "Max re-encode times",
          "Max re-encode times for one frame",
          0, 3, DEFAULT_PROP_MAX_REENC,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BPS,
      g_param_spec_uint ("bps", "Target BPS",
          "Target BPS (0 = auto calculate)",
          0, G_MAXINT, DEFAULT_PROP_BPS,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BPS_MIN,
      g_param_spec_uint ("bps-min", "Min BPS",
          "Min BPS (0 = auto calculate)",
          0, G_MAXINT, DEFAULT_PROP_BPS_MIN,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BPS_MAX,
      g_param_spec_uint ("bps-max", "Max BPS",
          "Max BPS (0 = auto calculate)",
          0, G_MAXINT, DEFAULT_PROP_BPS_MAX,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}
