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

#ifndef  GSTVPUDEC_H
#define  GSTVPUDEC_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideodecoder.h>
#include <gst/video/gstvideopool.h>

#include <rockchip/vpu_api.h>

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_VPUDEC	(gst_vpudec_get_type())
#define GST_VPUDEC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VPUDEC, GstVpuDec))
#define GST_VPUDEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_VPUDEC, GstVpuDecClass))
#define GST_IS_VPUDEC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_VPUDEC))
#define GST_IS_VPUDEC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_VPUDEC))

typedef struct _GstVpuDec GstVpuDec;
typedef struct _GstVpuDecClass GstVpuDecClass;

#include "gstvpudecbufferpool.h"

struct _GstVpuDec
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

  /* VPU definitions */
  VpuCodecContext_t *vpu_codec_ctx;
  VPUMemLinear_t front_vpumem;
  VideoPacket_t access_unit;

  GstBufferPool *pool;          /* Pool of output frames */
};

struct _GstVpuDecClass
{
  GstVideoDecoderClass parent_class;
};

GType gst_vpudec_get_type (void);


G_END_DECLS
#endif /* __GST_vpudec_H__ */
