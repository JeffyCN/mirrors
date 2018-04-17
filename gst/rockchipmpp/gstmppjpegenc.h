/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
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

#ifndef  __GST_MPP_JPEG_ENC_H__
#define  __GST_MPP_JPEG_ENC_H__

#include "gstmppvideoenc.h"

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_MPP_JPEG_ENC	(gst_mpp_jpeg_enc_get_type())
#define GST_MPP_JPEG_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPP_JPEG_ENC, GstMppJpegEnc))
#define GST_MPP_JPEG_ENC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_MPP_JPEG_ENC, GstMppJpegEncClass))
#define GST_IS_MPP_JPEG_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_MPP_JPEG_ENC))
#define GST_IS_MPP_JPEG_ENC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_MPP_JPEG_ENC))
typedef struct _GstMppJpegEnc GstMppJpegEnc;
typedef struct _GstMppJpegEncClass GstMppJpegEncClass;

struct _GstMppJpegEnc
{
  GstMppVideoEnc parent;
};

struct _GstMppJpegEncClass
{
  GstMppVideoEncClass parent_class;
};

GType gst_mpp_jpeg_enc_get_type (void);

G_END_DECLS
#endif /* __GST_MPP_JPEG_ENC_H__ */
