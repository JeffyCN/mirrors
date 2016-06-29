/*
 * Copyright 2016 Rockchip Electronics S.LSI Co. LTD
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <inttypes.h>

#include <gst/gst.h>

#include "gstvpuenc.h"

GST_DEBUG_CATEGORY (gst_vpuenc_debug);
#define GST_CAT_DEFAULT gst_vpuenc_debug

enum
{
  PROP_0,
  PROP_LAST
};

#define DEFAULT_LEVEL 0
#define DEFAULT_PROFILE 0;

#define NB_BUF_INPUT  1
#define NB_BUF_OUTPUT 1

#define WIDTH_MIN 32
#define WIDTH_MAX 1920
#define HEIGHT_MIN 32
#define HEIGHT_MAX 1920

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

/* GstVideoEncoder base class method */
static gboolean gst_vpuenc_start (GstVideoEncoder * encoder);
static gboolean gst_vpuenc_stop (GstVideoEncoder * video_encoder);
static gboolean gst_vpuenc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state);
static GstFlowReturn gst_vpuenc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame);
static void gst_vpuenc_finalize (GObject * object);

gboolean plugin_init (GstPlugin * plugin);

static GstStaticPadTemplate gst_vpuenc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
    "format = (string) NV12, "
    "width  = (int) [ 32, 1920 ], "
    "height = (int) [ 32, 1920 ], "
    "framerate = (fraction) [0/1, MAX]"));

static GstStaticPadTemplate gst_vpuenc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1920 ], "
        "framerate = (fraction) [0/1, MAX], "
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, "
        "profile = (string) { baseline }"
        )
    );

#define parent_class gst_vpuenc_parent_class
G_DEFINE_TYPE (GstVpuEnc, gst_vpuenc, GST_TYPE_VIDEO_ENCODER);


gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "vpuenc", GST_RANK_PRIMARY + 1,
          GST_TYPE_VPUENC))
    return FALSE;
  return TRUE;
}

static void
gst_vpuenc_class_init (GstVpuEncClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoEncoderClass *video_encoder_class = GST_VIDEO_ENCODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_vpuenc_finalize;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_vpuenc_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_vpuenc_sink_template));

  video_encoder_class->start = GST_DEBUG_FUNCPTR (gst_vpuenc_start);
  video_encoder_class->stop = GST_DEBUG_FUNCPTR (gst_vpuenc_stop);
  video_encoder_class->set_format = GST_DEBUG_FUNCPTR (gst_vpuenc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_vpuenc_handle_frame);

  GST_DEBUG_CATEGORY_INIT (gst_vpuenc_debug, "vpuenc", 0,
      "vpu video encoder");

  gst_element_class_set_static_metadata (element_class,
      "VPU encoder", "Encoder/Video", "A VPU encoder", "");
}

/* Init the vpuenc structure */
static void
gst_vpuenc_init (GstVpuEnc * vpuenc)
{
  vpuenc->level = DEFAULT_LEVEL;
  vpuenc->profile = DEFAULT_PROFILE;
}

static void
gst_vpuenc_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Open the device */
static gboolean
gst_vpuenc_start (GstVideoEncoder * encoder)
{
  GstVpuEnc *enc = GST_VPUENC (encoder);

  GST_DEBUG_OBJECT (enc, "Starting");

  return TRUE;
}

static gboolean
gst_vpuenc_stop (GstVideoEncoder * encoder)
{
  GstVpuEnc *enc = GST_VPUENC (encoder);
  GST_DEBUG_OBJECT (enc, "Stopping");


  if (enc->input_state) {
    gst_video_codec_state_unref (enc->input_state);
    enc->input_state = NULL;
  }

  if (enc->output_state) {
    gst_video_codec_state_unref (enc->output_state);
    enc->output_state = NULL;
  }

  /* close the device */
  GST_DEBUG_OBJECT (enc, "Stopped !!");

  return TRUE;
}

static int
gst_vpuenc_get_controls_from_caps (GstVideoEncoder * encoder, GstVideoInfo * info)
{
  GstVpuEnc *enc = GST_VPUENC (encoder);

  GstCaps *allowed_caps = NULL;

  /* get intersection of src caps and its peer */
  allowed_caps = gst_pad_get_allowed_caps (GST_VIDEO_ENCODER_SRC_PAD (encoder));

  if (allowed_caps) {
    GstStructure *s;

    if (gst_caps_is_empty (allowed_caps)) {
      gst_caps_unref (allowed_caps);
      return FALSE;
    }

    allowed_caps = gst_caps_make_writable (allowed_caps);
    allowed_caps = gst_caps_fixate (allowed_caps);
    s = gst_caps_get_structure (allowed_caps, 0);

    GST_INFO_OBJECT (enc, "downstream caps: %" GST_PTR_FORMAT, allowed_caps);
    GST_INFO_OBJECT (enc, "structure %p", s); //avoid compilation error, FIXME

    /* to test, keep the default profile */
    enc->profile = DEFAULT_PROFILE; //get profile from defined enum to_vpu_profile(s);
    enc->level = DEFAULT_LEVEL;     //get level from defined enum to_vpu_level(s);

    gst_caps_unref (allowed_caps);
  }

  return TRUE;
}

static gboolean
gst_vpuenc_set_format (GstVideoEncoder * encoder, GstVideoCodecState * state)
{
  GstVpuEnc *vpuenc = GST_VPUENC (encoder);

  GstCaps *caps;
  GstVideoInfo *info = &state->info;

  GST_DEBUG_OBJECT (vpuenc, "Setting format: %" GST_PTR_FORMAT, state->caps);

  if (vpuenc->input_state)
    gst_video_codec_state_unref (vpuenc->input_state);
  vpuenc->input_state = gst_video_codec_state_ref (state);

  /* Find profile & level from caps */
  if (!gst_vpuenc_get_controls_from_caps (encoder, info))
    goto error_controls;

  caps = gst_caps_new_empty_simple ("video/x-h264");

  vpuenc->output_state =
    gst_video_encoder_set_output_state (GST_VIDEO_ENCODER (vpuenc), caps, state);
  gst_video_codec_state_unref (vpuenc->output_state);
  vpuenc->output_state = NULL;

  gst_video_encoder_negotiate (GST_VIDEO_ENCODER (vpuenc));

  return TRUE;

  /* Errors */
error_controls:
  {
    GST_ERROR_OBJECT (vpuenc, "Error getting controls from caps");
    return FALSE;
  }
}

static GstFlowReturn
gst_vpuenc_handle_frame (GstVideoEncoder * encoder, GstVideoCodecFrame * frame)
{
  GstVpuEnc *vpuenc = GST_VPUENC (encoder);
  GstFlowReturn ret = GST_FLOW_OK;

  GST_DEBUG_OBJECT (vpuenc, "handle frame");

  /*get frame from VPU here
    VPU_FRAME and size
    The frame should have the sps_pps_header
  */

#if 0
  ret = gst_video_encoder_allocate_output_frame (encoder, frame, size);
  if (ret)
    goto error_alloc_frame;

  gst_buffer_fill (frame->output_buffer, offset, data, size);

  /* mark gst frame as key */
  if (flags & VPU_BUF_FLAG_KEYFRAME)
      GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);

  ret = gst_video_encoder_finish_frame (encoder, frame);
  if (ret)
    goto error_finish_frame;
#endif

  GST_DEBUG_OBJECT (vpuenc, "-->Access unit pushed");

  return ret;

#if 0
error_finish_frame:
  {
    GST_ERROR_OBJECT (enc, "Error when finishing frame err=0x%x", ret);
    return GST_FLOW_ERROR;
  }
#endif
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vpuenc,
    "VPU encoder",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
