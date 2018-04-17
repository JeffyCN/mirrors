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

#ifndef  __GST_MPP_VIDEO_ENC_H__
#define  __GST_MPP_VIDEO_ENC_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideoencoder.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

#include <rockchip/rk_mpi.h>

GST_DEBUG_CATEGORY_EXTERN (mppvideoenc_debug);

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_MPP_VIDEO_ENC	(gst_mpp_video_enc_get_type())
#define GST_MPP_VIDEO_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPP_VIDEO_ENC, GstMppVideoEnc))
#define GST_MPP_VIDEO_ENC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_MPP_VIDEO_ENC, GstMppVideoEncClass))
#define GST_IS_MPP_VIDEO_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_MPP_VIDEO_ENC))
#define GST_IS_MPP_VIDEO_ENC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_MPP_VIDEO_ENC))
typedef struct _GstMppVideoEnc GstMppVideoEnc;
typedef struct _GstMppVideoEncClass GstMppVideoEncClass;

#define MPP_MAX_BUFFERS                 4
#define MAX_CODEC_FRAME                 (1<<16)
#define MAX_EXTRA_DATA                  (1<<9)

struct _GstMppVideoEnc
{
  GstVideoEncoder parent;

  /* < private > */
  MppCtx mpp_ctx;
  MppApi *mpi;
  /* Buffer */
  MppBufferGroup input_group;
  MppBufferGroup output_group;
  MppBuffer input_buffer[MPP_MAX_BUFFERS];
  MppBuffer output_buffer[MPP_MAX_BUFFERS];
  MppFrame mpp_frame;
  MppPacket sps_packet;
  GstCaps *outcaps;

  /* the currently format */
  GstVideoInfo info;

  /* pads */
  GstCaps *probed_srccaps;
  GstCaps *probed_sinkcaps;

  /* State */
  GstVideoCodecState *input_state;
  gboolean processing;
  gboolean active;
  GstFlowReturn output_flow;
};

struct _GstMppVideoEncClass
{
  GstVideoEncoderClass parent_class;
  gboolean (*set_format) (GstVideoEncoder * encoder,
    GstVideoCodecState * state);
  GstFlowReturn (*handle_frame) (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame, GstCaps * outcaps);
};

GType gst_mpp_video_enc_get_type (void);


G_END_DECLS
#endif /* __GST_MPP_VIDEO_ENC_H__ */
