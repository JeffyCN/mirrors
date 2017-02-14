/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
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

#ifndef  _GST_MPP_VIDEO_DEC_H_
#define  _GST_MPP_VIDEO_DEC_H_

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideodecoder.h>
#include <gst/video/gstvideopool.h>

#include <rockchip/rk_mpi.h>

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_MPP_VIDEO_DEC	(gst_mpp_video_dec_get_type())
#define GST_MPP_VIDEO_DEC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPP_VIDEO_DEC, GstMppVideoDec))
#define GST_MPP_VIDEO_DEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_MPP_VIDEO_DEC, GstMppVideoDecClass))
#define GST_IS_MPP_VIDEO_DEC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_MPP_VIDEO_DEC))
#define GST_IS_MPP_VIDEO_DEC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_MPP_VIDEO_DEC))

typedef struct _GstMppVideoDec GstMppVideoDec;
typedef struct _GstMppVideoDecClass GstMppVideoDecClass;

struct _GstMppVideoDec
{
  GstVideoDecoder parent;

  /* add private members here */
  gint width;
  gint height;
  gint framesize;

  GstVideoCodecState *input_state;
  GstVideoCodecState *output_state;

  /* the currently format */
  GstVideoInfo info;
  GstVideoAlignment align;

  /* State */
  gboolean processing;
  gboolean active;
  GstFlowReturn output_flow;

  /* Rockchip Mpp definitions */
  MppCtx mpp_ctx;
  MppApi *mpi;

  GstBufferPool *pool;          /* Pool of output frames */
};

struct _GstMppVideoDecClass
{
  GstVideoDecoderClass parent_class;
};

GType gst_mpp_video_dec_get_type (void);

G_END_DECLS
#endif /* _GST_MPP_VIDEO_DEC_H_ */
