/*
 * Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#include "gstvpudec.h"

GST_DEBUG_CATEGORY (gst_vpudec_debug);
#define GST_CAT_DEFAULT gst_vpudec_debug

#define parent_class gst_vpudec_parent_class
G_DEFINE_TYPE (GstVpuDec, gst_vpudec, GST_TYPE_VIDEO_DECODER);

#define NB_INPUT_BUFS 4
#define NB_OUTPUT_BUFS 22       /* nb frames necessary for display pipeline */

/* GstVideoDecoder base class method */
static GstStaticPadTemplate gst_vpudec_sink_template =
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

static GstStaticPadTemplate gst_vpudec_src_template =
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

static gboolean gst_vpudec_flush (GstVideoDecoder * decoder);

static void
gst_vpudec_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_vpudec_close (GstVpuDec * vpudec)
{

  if (vpudec->vpu_codec_ctx != NULL) {
    vpu_close_context (&vpudec->vpu_codec_ctx);
    vpudec->vpu_codec_ctx = NULL;
  }

  GST_DEBUG_OBJECT (vpudec, "vpu codec context closed");

  return TRUE;
}

/* Open the device */
static gboolean
gst_vpudec_start (GstVideoDecoder * decoder)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);

  GST_DEBUG_OBJECT (vpudec, "Starting");
  vpudec->output_flow = GST_FLOW_OK;

  return TRUE;
}

static gint
gst_vpudec_sendeos (GstVideoDecoder * decoder)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);
  VpuCodecContext_t *vpu_codec_ctx = vpudec->vpu_codec_ctx;
  VideoPacket_t pkt;
  RK_S32 is_eos = 0;

  pkt.size = 0;
  pkt.data = NULL;
  /* eos flag */
  pkt.nFlags = VPU_API_DEC_OUTPUT_EOS;

  vpu_codec_ctx->decode_sendstream (vpu_codec_ctx, &pkt);

  /* Not need to wait flag here, dec_loop() may have exited */
  vpu_codec_ctx->control (vpu_codec_ctx, VPU_API_DEC_GET_EOS_STATUS, &is_eos);

  return is_eos;
}

static gboolean
gst_vpudec_stop (GstVideoDecoder * decoder)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);

  GST_DEBUG_OBJECT (vpudec, "Stopping");

  /* Wait for mpp/libvpu output thread to stop */
  gst_pad_stop_task (decoder->srcpad);

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  vpudec->output_flow = GST_FLOW_OK;
  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  gst_vpudec_sendeos (decoder);

  /* Should have been flushed already */
  g_assert (g_atomic_int_get (&vpudec->active) == FALSE);
  g_assert (g_atomic_int_get (&vpudec->processing) == FALSE);

  if (vpudec->input_state)
    gst_video_codec_state_unref (vpudec->input_state);

  if (vpudec->output_state)
    gst_video_codec_state_unref (vpudec->output_state);

  gst_vpudec_close (vpudec);
  if (vpudec->pool) {
    gst_object_unref (vpudec->pool);
    vpudec->pool = NULL;
  }

  GST_DEBUG_OBJECT (vpudec, "Stopped");

  return TRUE;
}

static VPU_VIDEO_CODINGTYPE
to_vpu_stream_format (GstStructure * s)
{
  if (gst_structure_has_name (s, "video/x-h264"))
    return VPU_VIDEO_CodingAVC;

  if (gst_structure_has_name (s, "video/x-h265"))
    return VPU_VIDEO_CodingHEVC;

  if (gst_structure_has_name (s, "video/x-h263"))
    return VPU_VIDEO_CodingH263;

  if (gst_structure_has_name (s, "video/mpeg")) {
    gint mpegversion = 0;
    if (gst_structure_get_int (s, "mpegversion", &mpegversion)) {
      switch (mpegversion) {
        case 2:
          return VPU_VIDEO_CodingMPEG2;
          break;
        case 4:
          return VPU_VIDEO_CodingMPEG4;
          break;
        default:
          break;
      }
    }
  }

  if (gst_structure_has_name (s, "video/x-vp8"))
    return VPU_VIDEO_CodingVP8;

  /* add more type here */

  return VPU_VIDEO_CodingUnused;
}

static GstVideoFormat
to_gst_pix_format (VPU_VIDEO_PIXEL_FMT pix_fmt)
{
  switch (pix_fmt) {
    case VPU_VIDEO_PIXEL_FMT_NV12:
      return GST_VIDEO_FORMAT_NV12;
    case VPU_VIDEO_PIXEL_FMT_P010LE:
      return GST_VIDEO_FORMAT_P010_10LE;
    default:
      return GST_VIDEO_FORMAT_UNKNOWN;
  }
}

static gboolean
gst_vpudec_open (GstVpuDec * vpudec, VPU_VIDEO_CODINGTYPE codingType)
{
  if (vpu_open_context (&vpudec->vpu_codec_ctx)
      || vpudec->vpu_codec_ctx == NULL)
    goto vpu_codec_ctx_error;

  GST_DEBUG_OBJECT (vpudec, "created vpu context %p", vpudec->vpu_codec_ctx);

  vpudec->vpu_codec_ctx->codecType = CODEC_DECODER;
  vpudec->vpu_codec_ctx->videoCoding = codingType;
  vpudec->vpu_codec_ctx->width = vpudec->width;
  vpudec->vpu_codec_ctx->height = vpudec->height;
  vpudec->vpu_codec_ctx->no_thread = 0;
  vpudec->vpu_codec_ctx->enableparsing = 1;

  if (vpudec->vpu_codec_ctx->init (vpudec->vpu_codec_ctx, NULL, 0) != 0)
    goto init_error;

  return TRUE;

vpu_codec_ctx_error:
  {
    GST_ERROR_OBJECT (vpudec, "libvpu open context failed");
    return FALSE;
  }
init_error:
  {
    GST_ERROR_OBJECT (vpudec, "libvpu init failed");
    if (!vpudec->vpu_codec_ctx)
      vpu_close_context (&vpudec->vpu_codec_ctx);
    return FALSE;
  }
}

static gboolean
gst_vpudec_set_format (GstVideoDecoder * decoder, GstVideoCodecState * state)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);
  GstStructure *structure;
  gint width, height;
  VPU_VIDEO_CODINGTYPE codingtype;
  const gchar *codec_profile;

  GST_DEBUG_OBJECT (vpudec, "Setting format: %" GST_PTR_FORMAT, state->caps);

  structure = gst_caps_get_structure (state->caps, 0);

  if (vpudec->input_state) {
    GstQuery *query = gst_query_new_drain ();

    if (gst_caps_is_strictly_equal (vpudec->input_state->caps, state->caps))
      goto done;

    gst_video_codec_state_unref (vpudec->input_state);
    vpudec->input_state = NULL;

    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
    gst_vpudec_sendeos (decoder);
    do {
      /* Wait all the reminded buffers are pushed to downstream */
    } while (g_atomic_int_get (&vpudec->processing));

    gst_vpudec_flush (decoder);
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    /* Query the downstream to release buffers from buffer pool */
    if (!gst_pad_peer_query (GST_VIDEO_DECODER_SRC_PAD (vpudec), query))
      GST_DEBUG_OBJECT (vpudec, "drain query failed");
    gst_query_unref (query);

    if (vpudec->pool) {
      gst_object_unref (vpudec->pool);
      vpudec->pool = NULL;
    }

    vpudec->output_flow = GST_FLOW_OK;

    g_atomic_int_set (&vpudec->active, FALSE);
  }

  g_return_val_if_fail (gst_structure_get_int (structure,
          "width", &width) != 0, FALSE);
  g_return_val_if_fail (gst_structure_get_int (structure,
          "height", &height) != 0, FALSE);

  codingtype = to_vpu_stream_format (structure);
  if (!codingtype)
    goto format_error;

  codec_profile = gst_structure_get_string (structure, "profile");

  if (codec_profile && g_strrstr (codec_profile, "10"))
    /* The mpp vpu api use this way to mark it as 10bit video */
    vpudec->width = width | 0x80000000;
  else
    vpudec->width = width;

  vpudec->height = height;

  if (vpudec->vpu_codec_ctx) {
    VPU_GENERIC vpug;

    vpug.CodecType = codingtype;
    vpug.ImgWidth = width;
    vpug.ImgHeight = height;

    vpudec->vpu_codec_ctx->control
        (vpudec->vpu_codec_ctx, VPU_API_SET_DEFAULT_WIDTH_HEIGH, &vpug);
  } else {
    if (!gst_vpudec_open (vpudec, codingtype))
      goto device_error;
  }

  vpudec->input_state = gst_video_codec_state_ref (state);

done:
  return TRUE;

  /* Errors */
format_error:
  {
    GST_ERROR_OBJECT (vpudec, "Unsupported format in caps: %" GST_PTR_FORMAT,
        state->caps);
    return FALSE;
  }
device_error:
  {
    GST_ERROR_OBJECT (vpudec, "Failed to open the device");
    return FALSE;
  }
}

static void
gst_vpudec_dec_loop (GstVideoDecoder * decoder)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);
  GstBuffer *output_buffer;
  GstVideoCodecFrame *frame;
  GstFlowReturn ret = GST_FLOW_OK;

  ret = gst_buffer_pool_acquire_buffer (vpudec->pool, &output_buffer, NULL);
  if ((ret != GST_FLOW_OK) && (ret != GST_FLOW_CUSTOM_ERROR_1))
    goto beach;

  frame = gst_video_decoder_get_oldest_frame (decoder);

  if (frame) {
    if (ret == GST_FLOW_CUSTOM_ERROR_1) {
      gst_video_decoder_drop_frame (decoder, frame);
      GST_DEBUG_OBJECT (vpudec, "I want to drop a frame");
      return;
    }

    frame->output_buffer = output_buffer;

    output_buffer = NULL;

    ret = gst_video_decoder_finish_frame (decoder, frame);

    GST_TRACE_OBJECT (vpudec, "finish buffer ts=%" GST_TIME_FORMAT,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (frame->output_buffer)));

    if (ret != GST_FLOW_OK)
      goto beach;

  } else {
    GST_WARNING_OBJECT (vpudec, "Decoder is producing too many buffers");
    gst_buffer_unref (output_buffer);
  }

  return;

beach:
  GST_DEBUG_OBJECT (vpudec, "Leaving output thread: %s",
      gst_flow_get_name (ret));

  gst_buffer_replace (&output_buffer, NULL);
  vpudec->output_flow = ret;
  g_atomic_int_set (&vpudec->processing, FALSE);
  gst_pad_pause_task (decoder->srcpad);
}

static void
gst_vpudec_dec_loop_stopped (GstVpuDec * self)
{
  if (g_atomic_int_get (&self->processing)) {
    GST_DEBUG_OBJECT (self, "Early stop of decoding thread");
    self->output_flow = GST_FLOW_FLUSHING;
    g_atomic_int_set (&self->processing, FALSE);
  }

  GST_DEBUG_OBJECT (self, "Decoding task destroyed: %s",
      gst_flow_get_name (self->output_flow));
}

static gboolean
gst_vpudec_default_output_info (GstVpuDec * vpudec)
{
  GstVideoCodecState *output_state;
  GstVideoAlignment *align;
  GstVideoInfo *info;
  DecoderFormat_t fmt;
  gboolean ret = TRUE;

  GST_DEBUG_OBJECT (vpudec, "Setting output");

  vpudec->vpu_codec_ctx->control (vpudec->vpu_codec_ctx, VPU_API_DEC_GETFORMAT,
      &fmt);
  info = &vpudec->info;

  GST_INFO_OBJECT (vpudec,
      "Format found from vpu :pixelfmt:%s, aligned_width:%d, aligned_height:%d, stride:%d, sizeimage:%d",
      gst_video_format_to_string (to_gst_pix_format (fmt.format)),
      fmt.aligned_width, fmt.aligned_height, fmt.aligned_stride,
      fmt.aligned_frame_size);

  /* FIXME not complete video information */
  gst_video_info_init (info);
  info->finfo = gst_video_format_get_info (to_gst_pix_format (fmt.format));
  info->width = fmt.width;
  info->height = fmt.height;
  info->size = fmt.aligned_frame_size;
  info->offset[0] = 0;
  info->offset[1] = fmt.aligned_stride * fmt.aligned_height;
  info->stride[0] = fmt.aligned_stride;
  info->stride[1] = fmt.aligned_stride;

  GST_INFO_OBJECT (vpudec,
      "video info stride %d, offset %d, stride %d, offset %d",
      GST_VIDEO_INFO_PLANE_STRIDE (&vpudec->info, 0),
      GST_VIDEO_INFO_PLANE_OFFSET (&vpudec->info, 0),
      GST_VIDEO_INFO_PLANE_STRIDE (&vpudec->info, 1),
      GST_VIDEO_INFO_PLANE_OFFSET (&vpudec->info, 1));

#if 1
  output_state =
      gst_video_decoder_set_output_state (GST_VIDEO_DECODER (vpudec),
      to_gst_pix_format (fmt.format), fmt.width, fmt.height,
      vpudec->input_state);

  if (vpudec->output_state)
    gst_video_codec_state_unref (vpudec->output_state);
  vpudec->output_state = gst_video_codec_state_ref (output_state);

  align = &vpudec->align;
  gst_video_alignment_reset (align);
  /* Compute padding and set buffer alignment */
  align->padding_right = fmt.aligned_stride - fmt.width;
  align->padding_bottom = fmt.aligned_height - fmt.height;
#endif

  /* construct a new buffer pool */
  vpudec->pool = gst_vpudec_buffer_pool_new (vpudec, NULL);
  if (vpudec->pool == NULL)
    goto error_new_pool;

  g_atomic_int_set (&vpudec->active, TRUE);

  return ret;

  /* Errors */
error_new_pool:
  {
    GST_ERROR_OBJECT (vpudec, "Unable to construct a new buffer pool");
    return FALSE;
  }
}

static GstFlowReturn
gst_vpudec_handle_frame (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);
  GstBufferPool *pool = NULL;
  GstMapInfo mapinfo = { 0, };
  GstFlowReturn ret = GST_FLOW_OK;
  VpuCodecContext_t *vpu_codec_ctx;
  VideoPacket_t access_unit;

  GST_DEBUG_OBJECT (vpudec, "Handling frame %d", frame->system_frame_number);

  if (!g_atomic_int_get (&vpudec->active)) {
    if (!vpudec->input_state)
      goto not_negotiated;
  }

  if (!vpudec->pool) {
    if (!gst_vpudec_default_output_info (vpudec))
      goto not_negotiated;
  }

  pool = GST_BUFFER_POOL (vpudec->pool);
  vpu_codec_ctx = vpudec->vpu_codec_ctx;
  if (!gst_buffer_pool_is_active (pool)) {
    GstBuffer *codec_data;

    GstStructure *config = gst_buffer_pool_get_config (pool);
    /* FIXME if you suffer from the reconstruction of buffer pool which slows
     * down the decoding, then don't allocate more than 10 buffers here */
    gst_buffer_pool_config_set_params (config, vpudec->output_state->caps,
        vpudec->info.size, 22, 22);

    if (!gst_buffer_pool_set_config (pool, config))
      goto error_activate_pool;
    /* activate the pool: the buffers are allocated */
    if (gst_buffer_pool_set_active (vpudec->pool, TRUE) == FALSE)
      goto error_activate_pool;

    codec_data = vpudec->input_state->codec_data;
    if (codec_data) {
      gst_buffer_ref (codec_data);
    } else {
      codec_data = frame->input_buffer;
      frame->input_buffer = NULL;
    }

    memset (&access_unit, 0, sizeof (VideoPacket_t));
    gst_buffer_map (codec_data, &mapinfo, GST_MAP_READ);
    access_unit.data = mapinfo.data;
    access_unit.size = mapinfo.size;
    access_unit.nFlags = VPU_API_DEC_INPUT_SYNC;

    gst_buffer_unmap (codec_data, &mapinfo);

    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

    if (vpu_codec_ctx->decode_sendstream (vpu_codec_ctx, &access_unit) != 0)
      goto send_stream_error;
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    gst_buffer_unref (codec_data);
  }

  /* Start the output thread if it is not started before */
  if (g_atomic_int_get (&vpudec->processing) == FALSE) {
    /* It's possible that the processing thread stopped due to an error */
    if (vpudec->output_flow != GST_FLOW_OK &&
        vpudec->output_flow != GST_FLOW_FLUSHING) {
      GST_DEBUG_OBJECT (vpudec, "Processing loop stopped with error, leaving");
      ret = vpudec->output_flow;
      goto drop;
    }

    GST_DEBUG_OBJECT (vpudec, "Starting decoding thread");

    g_atomic_int_set (&vpudec->processing, TRUE);
    if (!gst_pad_start_task (decoder->srcpad,
            (GstTaskFunction) gst_vpudec_dec_loop, vpudec,
            (GDestroyNotify) gst_vpudec_dec_loop_stopped))
      goto start_task_failed;
  }

  if (frame->input_buffer) {
    /* send access unit to VPU */
    vpu_codec_ctx = vpudec->vpu_codec_ctx;
    memset (&access_unit, 0, sizeof (VideoPacket_t));
    gst_buffer_map (frame->input_buffer, &mapinfo, GST_MAP_READ);
    access_unit.data = mapinfo.data;
    access_unit.size = mapinfo.size;
    access_unit.nFlags = VPU_API_DEC_INPUT_SYNC;

    gst_buffer_unmap (frame->input_buffer, &mapinfo);

    /* Unlock decoder before decode_sendstream call:
     * decode_sendstream must  block till frames
     * recycled, so decode_task must execute lock free..
     */

    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

    if (vpu_codec_ctx->decode_sendstream (vpu_codec_ctx, &access_unit) != 0)
      goto send_stream_error;

    GST_VIDEO_DECODER_STREAM_LOCK (decoder);

    /* No need to keep input arround */
    gst_buffer_replace (&frame->input_buffer, NULL);
  }

  gst_video_codec_frame_unref (frame);

  return ret;

  /* ERRORS */
error_activate_pool:
  {
    GST_ERROR_OBJECT (vpudec, "Unable to activate the pool");
    ret = GST_FLOW_ERROR;
    goto drop;
  }
start_task_failed:
  {
    GST_ELEMENT_ERROR (vpudec, RESOURCE, FAILED,
        ("Failed to start decoding thread."), (NULL));
    g_atomic_int_set (&vpudec->processing, FALSE);
    ret = GST_FLOW_ERROR;
    goto drop;
  }
not_negotiated:
  {
    GST_ERROR_OBJECT (vpudec, "not negotiated");
    gst_video_decoder_drop_frame (decoder, frame);
    return GST_FLOW_NOT_NEGOTIATED;
  }
send_stream_error:
  {
    GST_ERROR_OBJECT (vpudec, "send packet failed");
    return GST_FLOW_ERROR;
  }
drop:
  {
    gst_video_decoder_drop_frame (decoder, frame);
    return ret;
  }
}

static gboolean
gst_vpudec_flush (GstVideoDecoder * decoder)
{
  GstVpuDec *vpudec = GST_VPUDEC (decoder);
  VpuCodecContext_t *vpu_codec_ctx;

  /* Ensure the processing thread has stopped for the reverse playback
   * discount case */
  if (g_atomic_int_get (&vpudec->processing)) {
    GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
    gst_pad_stop_task (decoder->srcpad);
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  }

  vpudec->output_flow = GST_FLOW_OK;

  vpu_codec_ctx = vpudec->vpu_codec_ctx;
  return !vpu_codec_ctx->flush (vpu_codec_ctx);
}

static GstStateChangeReturn
gst_vpudec_change_state (GstElement * element, GstStateChange transition)
{
  GstVpuDec *self = GST_VPUDEC (element);
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (element);

  if (transition == GST_STATE_CHANGE_PAUSED_TO_READY) {
    g_atomic_int_set (&self->active, FALSE);
    gst_vpudec_sendeos (self);
    gst_pad_stop_task (decoder->srcpad);
  }
  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static void
gst_vpudec_class_init (GstVpuDecClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoDecoderClass *video_decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_vpudec_finalize;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_vpudec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_vpudec_sink_template));

  video_decoder_class->start = GST_DEBUG_FUNCPTR (gst_vpudec_start);
  video_decoder_class->stop = GST_DEBUG_FUNCPTR (gst_vpudec_stop);
  video_decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_vpudec_set_format);
  video_decoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_vpudec_handle_frame);
  video_decoder_class->flush = GST_DEBUG_FUNCPTR (gst_vpudec_flush);

  element_class->change_state = GST_DEBUG_FUNCPTR (gst_vpudec_change_state);

  GST_DEBUG_CATEGORY_INIT (gst_vpudec_debug, "vpudec", 0, "vpu video decoder");

  gst_element_class_set_static_metadata (element_class,
      "Rockchip's VPU video decoder", "Decoder/Video",
      "Multicodec (MPEG-2/4 / AVC / VP8 / HEVC) hardware decoder",
      "Sudip Jain <sudip.jain@@intel.com>, "
      "Jun Zhao <jung.zhao@rock-chips.com>, "
      "Herman Chen <herman.chen@rock-chips.com>, "
      "Randy Li <randy.li@rock-chips.com>");
}

/* Init the vpudec structure */
static void
gst_vpudec_init (GstVpuDec * vpudec)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) vpudec;

  gst_video_decoder_set_packetized (decoder, TRUE);

  vpudec->vpu_codec_ctx = NULL;

  vpudec->active = FALSE;

  vpudec->input_state = NULL;
  vpudec->output_state = NULL;
}
