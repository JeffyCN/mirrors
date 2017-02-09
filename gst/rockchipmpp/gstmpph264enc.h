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

#ifndef  __GST_MPP_H264_ENC_H__
#define  __GST_MPP_H264_ENC_H__

#include "gstmppvideoenc.h"

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_MPP_H264_ENC	(gst_mpp_h264_enc_get_type())
#define GST_MPP_H264_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPP_H264_ENC, GstMppH264Enc))
#define GST_MPP_H264_ENC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_MPP_H264_ENC, GstMppH264EncClass))
#define GST_IS_MPP_H264_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_MPP_H264_ENC))
#define GST_IS_MPP_H264_ENC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_MPP_H264_ENC))
typedef struct _GstMppH264Enc GstMppH264Enc;
typedef struct _GstMppH264EncClass GstMppH264EncClass;

struct _GstMppH264Enc
{
  GstMppVideoEnc parent;
};

struct _GstMppH264EncClass
{
  GstMppVideoEncClass parent_class;
};

GType gst_mpp_h264_enc_get_type (void);

G_END_DECLS
#endif /* __GST_MPP_H264_ENC_H__ */
