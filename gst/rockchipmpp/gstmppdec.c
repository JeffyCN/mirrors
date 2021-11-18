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
#include "gstmppdec.h"

#define GST_CAT_DEFAULT mpp_dec_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

#define parent_class gst_mpp_dec_parent_class
G_DEFINE_ABSTRACT_TYPE (GstMppDec, gst_mpp_dec, GST_TYPE_VIDEO_DECODER);

#define GST_MPP_DEC_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
    GST_TYPE_MPP_DEC, GstMppDecClass))

#define MPP_OUTPUT_TIMEOUT_MS 200       /* Block timeout for MPP output queue */
#define MPP_INPUT_TIMEOUT_MS 2000       /* Block timeout for MPP input queue */

#define MPP_TO_GST_PTS(pts) ((pts) * GST_MSECOND)

#define GST_MPP_DEC_TASK_STARTED(decoder) \
    (gst_pad_get_task_state ((decoder)->srcpad) == GST_TASK_STARTED)

#define GST_MPP_DEC_MUTEX(decoder) (&GST_MPP_DEC (decoder)->mutex)

#define GST_MPP_DEC_LOCK(decoder) \
  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder); \
  g_mutex_lock (GST_MPP_DEC_MUTEX (decoder)); \
  GST_VIDEO_DECODER_STREAM_LOCK (decoder);

#define GST_MPP_DEC_UNLOCK(decoder) \
  g_mutex_unlock (GST_MPP_DEC_MUTEX (decoder));

#define DEFAULT_PROP_ROTATION 0
#define DEFAULT_PROP_WIDTH 0    /* Original */
#define DEFAULT_PROP_HEIGHT 0   /* Original */

enum
{
  PROP_0,
  PROP_ROTATION,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_LAST,
};

static void
gst_mpp_dec_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (object);
  GstMppDec *self = GST_MPP_DEC (decoder);

  switch (prop_id) {
    case PROP_ROTATION:{
      if (self->input_state)
        GST_WARNING_OBJECT (decoder, "unable to change rotation");
      else
        self->rotation = g_value_get_enum (value);
      break;
    }

    case PROP_WIDTH:{
      if (self->input_state)
        GST_WARNING_OBJECT (decoder, "unable to change width");
      else
        self->width = g_value_get_uint (value);
      break;
    }
    case PROP_HEIGHT:{
      if (self->input_state)
        GST_WARNING_OBJECT (decoder, "unable to change height");
      else
        self->height = g_value_get_uint (value);
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_mpp_dec_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (object);
  GstMppDec *self = GST_MPP_DEC (decoder);

  switch (prop_id) {
    case PROP_ROTATION:
      g_value_set_enum (value, self->rotation);
      break;
    case PROP_WIDTH:
      g_value_set_uint (value, self->width);
      break;
    case PROP_HEIGHT:
      g_value_set_uint (value, self->height);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static void
gst_mpp_dec_stop_task (GstVideoDecoder * decoder, gboolean drain)
{
  GstMppDecClass *klass = GST_MPP_DEC_GET_CLASS (decoder);

  if (!GST_MPP_DEC_TASK_STARTED (decoder))
    return;

  GST_DEBUG_OBJECT (decoder, "stopping decoding thread");

  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
  if (klass->shutdown && klass->shutdown (decoder, drain)) {
    /* Wait for task thread to pause */
    GstTask *task = decoder->srcpad->task;
    if (task) {
      GST_OBJECT_LOCK (task);
      while (GST_TASK_STATE (task) == GST_TASK_STARTED)
        GST_TASK_WAIT (task);
      GST_OBJECT_UNLOCK (task);
    }
  }

  gst_pad_stop_task (decoder->srcpad);
  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
}

static void
gst_mpp_dec_reset (GstVideoDecoder * decoder, gboolean drain, gboolean final)
{
  GstMppDec *self = GST_MPP_DEC (decoder);

  GST_MPP_DEC_LOCK (decoder);

  GST_DEBUG_OBJECT (self, "resetting");

  self->flushing = TRUE;
  self->draining = drain;

  gst_mpp_dec_stop_task (decoder, drain);

  self->flushing = final;
  self->draining = FALSE;

  self->mpi->reset (self->mpp_ctx);
  self->task_ret = GST_FLOW_OK;

  GST_MPP_DEC_UNLOCK (decoder);
}

static gboolean
gst_mpp_dec_start (GstVideoDecoder * decoder)
{
  GstMppDec *self = GST_MPP_DEC (decoder);

  GST_DEBUG_OBJECT (self, "starting");

  self->allocator = gst_mpp_allocator_new ();
  if (!self->allocator)
    return FALSE;

  if (mpp_create (&self->mpp_ctx, &self->mpi)) {
    gst_object_unref (self->allocator);
    return FALSE;
  }

  self->interlace_mode = GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;
  self->mpp_type = MPP_VIDEO_CodingUnused;
  self->seen_valid_pts = FALSE;

  self->input_state = NULL;

  self->task_ret = GST_FLOW_OK;
  self->flushing = FALSE;

  /* Prefer using MPP PTS */
  self->use_mpp_pts = TRUE;
  self->mpp_delta_pts = 0;

  g_mutex_init (&self->mutex);

  GST_DEBUG_OBJECT (self, "started");

  return TRUE;
}

static gboolean
gst_mpp_dec_stop (GstVideoDecoder * decoder)
{
  GstMppDec *self = GST_MPP_DEC (decoder);

  GST_DEBUG_OBJECT (self, "stopping");

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  gst_mpp_dec_reset (decoder, FALSE, TRUE);
  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  g_mutex_clear (&self->mutex);

  mpp_destroy (self->mpp_ctx);

  gst_object_unref (self->allocator);

  if (self->input_state) {
    gst_video_codec_state_unref (self->input_state);
    self->input_state = NULL;
  }

  GST_DEBUG_OBJECT (self, "stopped");

  return TRUE;
}

static gboolean
gst_mpp_dec_flush (GstVideoDecoder * decoder)
{
  GST_DEBUG_OBJECT (decoder, "flushing");
  gst_mpp_dec_reset (decoder, FALSE, FALSE);
  return TRUE;
}

static GstFlowReturn
gst_mpp_dec_drain (GstVideoDecoder * decoder)
{
  GST_DEBUG_OBJECT (decoder, "draining");
  gst_mpp_dec_reset (decoder, TRUE, FALSE);
  return GST_FLOW_OK;
}

static GstFlowReturn
gst_mpp_dec_finish (GstVideoDecoder * decoder)
{
  GST_DEBUG_OBJECT (decoder, "finishing");
  gst_mpp_dec_reset (decoder, TRUE, FALSE);
  return GST_FLOW_OK;
}

static gboolean
gst_mpp_dec_set_format (GstVideoDecoder * decoder, GstVideoCodecState * state)
{
  GstMppDec *self = GST_MPP_DEC (decoder);

  GST_DEBUG_OBJECT (self, "setting format: %" GST_PTR_FORMAT, state->caps);

  if (self->input_state) {
    if (gst_caps_is_strictly_equal (self->input_state->caps, state->caps))
      return TRUE;

    gst_mpp_dec_reset (decoder, TRUE, FALSE);

    gst_video_codec_state_unref (self->input_state);
    self->input_state = NULL;
  } else {
    MppBufferGroup group;

    if (mpp_init (self->mpp_ctx, MPP_CTX_DEC, self->mpp_type)) {
      GST_ERROR_OBJECT (self, "failed to init mpp ctx");
      return FALSE;
    }

    group = gst_mpp_allocator_get_mpp_group (self->allocator);
    self->mpi->control (self->mpp_ctx, MPP_DEC_SET_EXT_BUF_GROUP, group);
  }

  self->input_state = gst_video_codec_state_ref (state);

  return TRUE;
}

gboolean
gst_mpp_dec_update_video_info (GstVideoDecoder * decoder, GstVideoFormat format,
    guint width, guint height, gint hstride, gint vstride, guint align)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &self->info;
  GstVideoCodecState *output_state;

  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, FALSE);

  /* Sinks like kmssink require width and height align to 2 */
  output_state = gst_video_decoder_set_output_state (decoder, format,
      GST_ROUND_UP_2 (width), GST_ROUND_UP_2 (height), self->input_state);
  *info = output_state->info;
  gst_video_codec_state_unref (output_state);

  align = align ? : 2;

  hstride = hstride ? : GST_MPP_VIDEO_INFO_HSTRIDE (info);
  hstride = GST_ROUND_UP_N (hstride, align);

  vstride = vstride ? : GST_MPP_VIDEO_INFO_VSTRIDE (info);
  vstride = GST_ROUND_UP_N (vstride, align);

  return gst_mpp_video_info_align (info, hstride, vstride);
}

void
gst_mpp_dec_fixup_video_info (GstVideoDecoder * decoder, GstVideoFormat format,
    gint width, gint height)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &self->info;

  if (self->rotation % 180)
    SWAP (width, height);

  /* Figure out output format */
  if (self->format != GST_VIDEO_FORMAT_UNKNOWN)
    /* Use specified format */
    format = self->format;
  if (format == GST_VIDEO_FORMAT_UNKNOWN)
    /* Fallback to NV12 */
    format = GST_VIDEO_FORMAT_NV12;

  gst_video_info_set_format (info, format,
      self->width ? : width, self->height ? : height);
}

static GstFlowReturn
gst_mpp_dec_apply_info_change (GstVideoDecoder * decoder, MppFrame mframe)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoFormat dst_format, src_format;
  GstVideoInfo *info = &self->info;
  MppFrameFormat mpp_format;
  gint width = mpp_frame_get_width (mframe);
  gint height = mpp_frame_get_height (mframe);
  gint hstride = mpp_frame_get_hor_stride (mframe);
  gint vstride = mpp_frame_get_ver_stride (mframe);
  gint dst_width, dst_height;

  if (hstride % 2 || vstride % 2)
    return GST_FLOW_NOT_NEGOTIATED;

  mpp_format = mpp_frame_get_fmt (mframe);
  src_format = gst_mpp_mpp_format_to_gst_format (mpp_format);

  /* Figure out final output info */
  gst_mpp_dec_fixup_video_info (decoder, src_format, width, height);
  dst_format = GST_VIDEO_INFO_FORMAT (info);
  dst_width = GST_VIDEO_INFO_WIDTH (info);
  dst_height = GST_VIDEO_INFO_HEIGHT (info);

  if (dst_format != src_format || dst_width != width || dst_height != height) {
    /* Conversion required */
    GST_INFO_OBJECT (self, "convert from %s (%dx%d) to %s (%dx%d)",
        gst_video_format_to_string (src_format), width, height,
        gst_video_format_to_string (dst_format), dst_width, dst_height);

    hstride = 0;
    vstride = 0;
  }

  if (!gst_mpp_dec_update_video_info (decoder, dst_format,
          dst_width, dst_height, hstride, vstride, 0))
    return GST_FLOW_NOT_NEGOTIATED;

  return GST_FLOW_OK;
}

static GstVideoCodecFrame *
gst_mpp_dec_get_frame (GstVideoDecoder * decoder, GstClockTime pts)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoCodecFrame *frame;
  GList *frames, *l;
  gint i;

  frames = gst_video_decoder_get_frames (decoder);
  if (!frames) {
    GST_DEBUG_OBJECT (self, "missing frame");
    return NULL;
  }

  frame = frames->data;
  if (!frame->system_frame_number) {
    /* Choose PTS source when getting the first frame */

    if (self->use_mpp_pts) {
      if (!GST_CLOCK_TIME_IS_VALID (pts)) {
        GST_WARNING_OBJECT (self, "MPP is not able to generate pts");
        self->use_mpp_pts = FALSE;
      } else {
        if (GST_CLOCK_TIME_IS_VALID (frame->pts))
          self->mpp_delta_pts = frame->pts - MPP_TO_GST_PTS (pts);

        GST_DEBUG_OBJECT (self, "MPP delta pts=%" GST_TIME_FORMAT,
            GST_TIME_ARGS (self->mpp_delta_pts));
      }
    }

    if (self->use_mpp_pts)
      GST_DEBUG_OBJECT (self, "using MPP pts");
    else
      GST_DEBUG_OBJECT (self, "using original pts");

    GST_DEBUG_OBJECT (self, "using first frame (#%d)",
        frame->system_frame_number);
    goto out;
  }

  if (!pts)
    pts = GST_CLOCK_TIME_NONE;

  /* Fixup MPP PTS */
  if (self->use_mpp_pts && GST_CLOCK_TIME_IS_VALID (pts)) {
    pts = MPP_TO_GST_PTS (pts);
    pts += self->mpp_delta_pts;
  }

  GST_DEBUG_OBJECT (self, "receiving pts=%" GST_TIME_FORMAT,
      GST_TIME_ARGS (pts));

  if (!self->seen_valid_pts) {
    /* No frame with valid PTS, choose the oldest one */
    frame = frames->data;

    GST_DEBUG_OBJECT (self, "using oldest frame (#%d)",
        frame->system_frame_number);
    goto out;
  }

  /* MPP outputs frames in display order, so let's find the earliest one */
  for (frame = NULL, l = frames, i = 0; l != NULL; l = l->next, i++) {
    GstVideoCodecFrame *f = l->data;

    if (GST_CLOCK_TIME_IS_VALID (f->pts)) {
      /* Prefer frame with close PTS */
      if (abs ((gint) f->pts - (gint) pts) < 3000000) {
        frame = f;

        GST_DEBUG_OBJECT (self, "using matched frame (#%d)",
            frame->system_frame_number);
        goto out;
      }

      /* Filter out future frames */
      if (GST_CLOCK_TIME_IS_VALID (pts) && f->pts > pts)
        continue;
    } else if (self->interlace_mode == GST_VIDEO_INTERLACE_MODE_MIXED) {
      /* Consider frames with invalid PTS are decode-only when deinterlaced */

      /* Delay discarding frames for some broken videos */
      if (i >= 16) {
        GST_WARNING_OBJECT (self, "discarding decode-only frame (#%d)",
            f->system_frame_number);

        gst_video_codec_frame_ref (f);
        gst_video_decoder_release_frame (decoder, f);
        continue;
      }
    }

    /* Find the frame with earliest PTS (including invalid PTS) */
    if (!frame || frame->pts > f->pts)
      frame = f;
  }

  if (frame)
    GST_DEBUG_OBJECT (self, "using guested frame (#%d)",
        frame->system_frame_number);

out:
  if (frame) {
    gst_video_codec_frame_ref (frame);

    if (GST_CLOCK_TIME_IS_VALID (pts))
      frame->pts = pts;
  }

  g_list_free_full (frames, (GDestroyNotify) gst_video_codec_frame_unref);
  return frame;
}

static gboolean
gst_mpp_dec_info_matched (GstVideoDecoder * decoder, MppFrame mframe)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &self->info;
  GstVideoFormat format = GST_VIDEO_INFO_FORMAT (info);
  MppFrameFormat mpp_format = mpp_frame_get_fmt (mframe);
  gint width = mpp_frame_get_width (mframe);
  gint height = mpp_frame_get_height (mframe);
  gint hstride = mpp_frame_get_hor_stride (mframe);
  gint vstride = mpp_frame_get_ver_stride (mframe);

  if (self->rotation)
    return FALSE;

  /* NOTE: The output video info is aligned to 2 */
  width = GST_ROUND_UP_2 (width);
  height = GST_ROUND_UP_2 (height);

  if (mpp_format != gst_mpp_gst_format_to_mpp_format (format))
    return FALSE;

  if (width != GST_VIDEO_INFO_WIDTH (info))
    return FALSE;

  if (height != GST_VIDEO_INFO_HEIGHT (info))
    return FALSE;

  if (hstride != GST_MPP_VIDEO_INFO_HSTRIDE (info))
    return FALSE;

  if (vstride != GST_MPP_VIDEO_INFO_VSTRIDE (info))
    return FALSE;

  return TRUE;
}

#ifdef HAVE_RGA
static gboolean
gst_mpp_dec_rga_convert (GstVideoDecoder * decoder, MppFrame mframe,
    GstBuffer * buffer)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &self->info;
  GstMemory *mem;
  gboolean ret;

  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  mem = gst_allocator_alloc (self->allocator, GST_VIDEO_INFO_SIZE (info), NULL);
  g_return_val_if_fail (mem, FALSE);

  if (!gst_mpp_rga_convert_from_mpp_frame (mframe, mem, info, self->rotation)) {
    GST_WARNING_OBJECT (self, "failed to convert");
    gst_memory_unref (mem);
    ret = FALSE;
  } else {
    gst_buffer_replace_all_memory (buffer, mem);
    ret = TRUE;
  }

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  return ret;
}
#endif

static GstBuffer *
gst_mpp_dec_get_gst_buffer (GstVideoDecoder * decoder, MppFrame mframe)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &self->info;
  GstBuffer *buffer;
  GstMemory *mem;
  MppBuffer mbuf;

  mbuf = mpp_frame_get_buffer (mframe);
  if (!mbuf)
    return NULL;

  /* Allocated from this MPP group in MPP */
  mpp_buffer_set_index (mbuf, gst_mpp_allocator_get_index (self->allocator));

  mem = gst_mpp_allocator_import_mppbuf (self->allocator, mbuf);
  if (!mem)
    return NULL;

  buffer = gst_buffer_new ();
  if (!buffer) {
    gst_memory_unref (mem);
    return NULL;
  }

  gst_buffer_append_memory (buffer, mem);

  gst_buffer_add_video_meta_full (buffer, GST_VIDEO_FRAME_FLAG_NONE,
      GST_VIDEO_INFO_FORMAT (info),
      GST_VIDEO_INFO_WIDTH (info), GST_VIDEO_INFO_HEIGHT (info),
      GST_VIDEO_INFO_N_PLANES (info), info->offset, info->stride);

  if (gst_mpp_dec_info_matched (decoder, mframe))
    return buffer;

  GST_DEBUG_OBJECT (self, "frame format not matched");

#ifdef HAVE_RGA
  if (gst_mpp_dec_rga_convert (decoder, mframe, buffer))
    return buffer;
#endif

  GST_WARNING_OBJECT (self, "unable to convert frame");

  gst_buffer_unref (buffer);
  return NULL;
}

static void
gst_mpp_dec_update_interlace_mode (GstVideoDecoder * decoder,
    GstBuffer * buffer, guint mode)
{
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoInterlaceMode interlace_mode;
  GstVideoCodecState *output_state;

  interlace_mode = GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;

  switch (mode & MPP_FRAME_FLAG_FIELD_ORDER_MASK) {
    case MPP_FRAME_FLAG_BOT_FIRST:
      GST_BUFFER_FLAG_SET (buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_UNSET (buffer, GST_VIDEO_BUFFER_FLAG_TFF);
      interlace_mode = GST_VIDEO_INTERLACE_MODE_INTERLEAVED;
      break;
    case MPP_FRAME_FLAG_TOP_FIRST:
      GST_BUFFER_FLAG_SET (buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_SET (buffer, GST_VIDEO_BUFFER_FLAG_TFF);
      interlace_mode = GST_VIDEO_INTERLACE_MODE_INTERLEAVED;
      break;
    case MPP_FRAME_FLAG_DEINTERLACED:
      interlace_mode = GST_VIDEO_INTERLACE_MODE_MIXED;
      /* fall-through */
    default:
      GST_BUFFER_FLAG_UNSET (buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_UNSET (buffer, GST_VIDEO_BUFFER_FLAG_TFF);
      break;
  }

  if (self->interlace_mode != interlace_mode) {
    output_state = gst_video_decoder_get_output_state (decoder);
    if (output_state) {
      GST_VIDEO_INFO_INTERLACE_MODE (&output_state->info) = interlace_mode;
      self->interlace_mode = interlace_mode;
      gst_video_codec_state_unref (output_state);
    }
  }
}

static void
gst_mpp_dec_loop (GstVideoDecoder * decoder)
{
  GstMppDecClass *klass = GST_MPP_DEC_GET_CLASS (decoder);
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstVideoCodecFrame *frame;
  GstBuffer *buffer;
  MppFrame mframe;
  int mode;

  mframe = klass->poll_mpp_frame (decoder, MPP_OUTPUT_TIMEOUT_MS);
  /* Likely due to timeout */
  if (!mframe)
    return;

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);

  if (mpp_frame_get_eos (mframe))
    goto eos;

  if (mpp_frame_get_info_change (mframe)) {
    self->mpi->control (self->mpp_ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
    self->task_ret = gst_mpp_dec_apply_info_change (decoder, mframe);
    goto info_change;
  }

  mode = mpp_frame_get_mode (mframe);
#ifdef MPP_FRAME_FLAG_IEP_DEI_MASK
  /* IEP deinterlaced */
  if (mode & MPP_FRAME_FLAG_IEP_DEI_MASK) {
#ifdef MPP_FRAME_FLAG_IEP_DEI_I4O2
    if (mode & MPP_FRAME_FLAG_IEP_DEI_I4O2) {
      /* 1 input frame generates 2 deinterlaced MPP frames */
      static int mpp_i4o2_frames = 0;
      if (mpp_i4o2_frames++ % 2) {
        GST_DEBUG_OBJECT (self, "ignore extra MPP frame");
        goto out;
      }
    }
#endif
    mode = MPP_FRAME_FLAG_DEINTERLACED;
  }
#endif

  frame = gst_mpp_dec_get_frame (decoder, mpp_frame_get_pts (mframe));
  if (!frame)
    goto no_frame;

  if (mpp_frame_get_discard (mframe) || mpp_frame_get_errinfo (mframe))
    goto error;

  buffer = gst_mpp_dec_get_gst_buffer (decoder, mframe);
  if (!buffer)
    goto error;

  gst_buffer_resize (buffer, 0, GST_VIDEO_INFO_SIZE (&self->info));

  gst_mpp_dec_update_interlace_mode (decoder, buffer, mode);

  frame->output_buffer = buffer;

  if (self->flushing && !self->draining)
    goto drop;

  GST_DEBUG_OBJECT (self, "finish frame ts=%" GST_TIME_FORMAT,
      GST_TIME_ARGS (frame->pts));

  gst_video_decoder_finish_frame (decoder, frame);

out:
  mpp_frame_deinit (&mframe);

  if (self->task_ret != GST_FLOW_OK) {
    GST_DEBUG_OBJECT (self, "leaving output thread: %s",
        gst_flow_get_name (self->task_ret));

    gst_pad_pause_task (decoder->srcpad);
  }

  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
  return;
eos:
  GST_INFO_OBJECT (self, "got eos");
  self->task_ret = GST_FLOW_EOS;
  goto out;
info_change:
  GST_INFO_OBJECT (self, "video info changed");
  goto out;
no_frame:
  GST_WARNING_OBJECT (self, "no matched frame");
  goto out;
error:
  GST_WARNING_OBJECT (self, "can't process this frame");
  goto drop;
drop:
  GST_DEBUG_OBJECT (self, "drop frame");
  gst_video_decoder_release_frame (decoder, frame);
  goto out;
}

static GstFlowReturn
gst_mpp_dec_handle_frame (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstMppDecClass *klass = GST_MPP_DEC_GET_CLASS (decoder);
  GstMppDec *self = GST_MPP_DEC (decoder);
  GstMapInfo mapinfo = { 0, };
  GstBuffer *tmp;
  GstFlowReturn ret;
  gint timeout_ms = MPP_INPUT_TIMEOUT_MS;
  gint interval_ms = 5;
  MppPacket mpkt = NULL;

  GST_MPP_DEC_LOCK (decoder);

  GST_DEBUG_OBJECT (self, "handling frame %d", frame->system_frame_number);

  if (G_UNLIKELY (self->flushing))
    goto flushing;

  if (G_UNLIKELY (!GST_MPP_DEC_TASK_STARTED (decoder))) {
    if (klass->startup && !klass->startup (decoder))
      goto not_negotiated;

    GST_DEBUG_OBJECT (self, "starting decoding thread");

    gst_pad_start_task (decoder->srcpad,
        (GstTaskFunction) gst_mpp_dec_loop, decoder, NULL);
  }

  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
  gst_buffer_map (frame->input_buffer, &mapinfo, GST_MAP_READ);
  mpkt = klass->get_mpp_packet (decoder, &mapinfo);
  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  if (!mpkt)
    goto no_packet;

  mpp_packet_set_pts (mpkt, self->use_mpp_pts ? -1 : (gint64) frame->pts);

  if (GST_CLOCK_TIME_IS_VALID (frame->pts))
    self->seen_valid_pts = TRUE;

  while (1) {
    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
    if (klass->send_mpp_packet (decoder, mpkt, interval_ms)) {
      GST_VIDEO_DECODER_STREAM_LOCK (decoder);
      break;
    }
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    timeout_ms -= interval_ms;
    if (timeout_ms <= 0)
      goto send_error;
  }

  /* NOTE: Sub-class takes over the MPP packet when success */
  mpkt = NULL;
  gst_buffer_unmap (frame->input_buffer, &mapinfo);

  /* No need to keep input arround */
  tmp = frame->input_buffer;
  frame->input_buffer = gst_buffer_new ();
  gst_buffer_copy_into (frame->input_buffer, tmp,
      GST_BUFFER_COPY_FLAGS | GST_BUFFER_COPY_TIMESTAMPS |
      GST_BUFFER_COPY_META, 0, 0);
  gst_buffer_unref (tmp);

  gst_video_codec_frame_unref (frame);

  GST_MPP_DEC_UNLOCK (decoder);

  return self->task_ret;

flushing:
  GST_WARNING_OBJECT (self, "flushing");
  ret = GST_FLOW_FLUSHING;
  goto drop;
not_negotiated:
  GST_ERROR_OBJECT (self, "not negotiated");
  ret = GST_FLOW_NOT_NEGOTIATED;
  goto drop;
no_packet:
  GST_ERROR_OBJECT (self, "failed to get packet");
  ret = GST_FLOW_ERROR;
  goto drop;
send_error:
  GST_ERROR_OBJECT (self, "failed to send packet");
  ret = GST_FLOW_ERROR;
  goto drop;
drop:
  GST_WARNING_OBJECT (self, "can't handle this frame");

  if (mpkt)
    mpp_packet_deinit (&mpkt);

  gst_buffer_unmap (frame->input_buffer, &mapinfo);
  gst_video_decoder_release_frame (decoder, frame);

  GST_MPP_DEC_UNLOCK (decoder);

  return ret;
}

static GstStateChangeReturn
gst_mpp_dec_change_state (GstElement * element, GstStateChange transition)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (element);

  if (transition == GST_STATE_CHANGE_PAUSED_TO_READY) {
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);
    gst_mpp_dec_reset (decoder, FALSE, TRUE);
    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
  }

  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static void
gst_mpp_dec_init (GstMppDec * self)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (self);

  gst_video_decoder_set_packetized (decoder, TRUE);
}

#ifdef HAVE_RGA
#define GST_TYPE_MPP_DEC_ROTATION (gst_mpp_dec_rotation_get_type ())
static GType
gst_mpp_dec_rotation_get_type (void)
{
  static GType rotation = 0;

  if (!rotation) {
    static const GEnumValue rotations[] = {
      {0, "Rotate 0", "0"},
      {90, "Rotate 90", "90"},
      {180, "Rotate 180", "180"},
      {270, "Rotate 270", "270"},
      {0, NULL, NULL}
    };
    rotation = g_enum_register_static ("GstMppDecRotation", rotations);
  }
  return rotation;
}
#endif

static void
gst_mpp_dec_class_init (GstMppDecClass * klass)
{
  GstVideoDecoderClass *decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppdec", 0, "MPP decoder");

  decoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_dec_start);
  decoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_dec_stop);
  decoder_class->flush = GST_DEBUG_FUNCPTR (gst_mpp_dec_flush);
  decoder_class->drain = GST_DEBUG_FUNCPTR (gst_mpp_dec_drain);
  decoder_class->finish = GST_DEBUG_FUNCPTR (gst_mpp_dec_finish);
  decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_mpp_dec_set_format);
  decoder_class->handle_frame = GST_DEBUG_FUNCPTR (gst_mpp_dec_handle_frame);

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_mpp_dec_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_mpp_dec_get_property);

#ifdef HAVE_RGA
  g_object_class_install_property (gobject_class, PROP_ROTATION,
      g_param_spec_enum ("rotation", "Rotation",
          "Rotation",
          GST_TYPE_MPP_DEC_ROTATION, DEFAULT_PROP_ROTATION,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_WIDTH,
      g_param_spec_uint ("width", "Width",
          "Width (0 = original)",
          0, G_MAXINT, DEFAULT_PROP_WIDTH,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_HEIGHT,
      g_param_spec_uint ("height", "Height",
          "Height (0 = original)",
          0, G_MAXINT, DEFAULT_PROP_HEIGHT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#endif

  element_class->change_state = GST_DEBUG_FUNCPTR (gst_mpp_dec_change_state);
}
