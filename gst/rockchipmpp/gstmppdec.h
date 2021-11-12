/*
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

#ifndef  __GST_MPP_DEC_H__
#define  __GST_MPP_DEC_H__

#include <gst/video/gstvideodecoder.h>

#include "gstmpp.h"

G_BEGIN_DECLS;

#define GST_TYPE_MPP_DEC	(gst_mpp_dec_get_type())
#define GST_MPP_DEC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPP_DEC, GstMppDec))
#define GST_MPP_DEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_MPP_DEC, GstMppDecClass))
typedef struct _GstMppDec GstMppDec;
typedef struct _GstMppDecClass GstMppDecClass;

struct _GstMppDec
{
  GstVideoDecoder parent;

  GMutex mutex;
  GstAllocator *allocator;
  GstVideoCodecState *input_state;

  /* final output video info */
  GstVideoInfo info;

  /* specified output format */
  GstVideoFormat format;

  gint rotation;
  gint width;
  gint height;

  /* stop handling new frame when flushing */
  gboolean flushing;

  /* drop frames when flushing but not draining */
  gboolean draining;

  /* flow return from pad task */
  GstFlowReturn task_ret;

  GstVideoInterlaceMode interlace_mode;

  /* seen valid PTS in input frames */
  gboolean seen_valid_pts;

  /* for using MPP generated PTS */
  gboolean use_mpp_pts;
  GstClockTime mpp_delta_pts;

  MppCodingType mpp_type;
  MppCtx mpp_ctx;
  MppApi *mpi;
};

struct _GstMppDecClass
{
  GstVideoDecoderClass parent_class;

    gboolean (*startup) (GstVideoDecoder * decoder);
    MppPacket (*get_mpp_packet) (GstVideoDecoder * decoder,
      GstMapInfo * mapinfo);
    gboolean (*send_mpp_packet) (GstVideoDecoder * decoder,
      MppPacket mpkt, gint timeout_ms);
    MppFrame (*poll_mpp_frame) (GstVideoDecoder * decoder, gint timeout_ms);
    gboolean (*shutdown) (GstVideoDecoder * decoder, gboolean drain);
};

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GstMppDec, gst_object_unref);
GType gst_mpp_dec_get_type (void);

#define GST_FLOW_TIMEOUT GST_FLOW_CUSTOM_ERROR_1

#define MPP_DEC_OUT_FORMATS "NV12, NV16"

#ifdef HAVE_RGA
#define MPP_DEC_FORMATS MPP_DEC_OUT_FORMATS "," GST_RGA_FORMATS
#else
#define MPP_DEC_FORMATS MPP_DEC_OUT_FORMATS
#endif

void gst_mpp_dec_fixup_video_info (GstVideoDecoder * decoder,
    GstVideoFormat format, gint width, gint height);

gboolean gst_mpp_dec_update_video_info (GstVideoDecoder * decoder,
    GstVideoFormat format, guint width, guint height,
    gint hstride, gint vstride, guint align);

G_END_DECLS;

#endif /* __GST_MPP_DEC_H__ */
