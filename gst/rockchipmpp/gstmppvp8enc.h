/*
 * Copyright 2020 Rockchip Electronics Co., Ltd
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

#ifndef  __GST_MPP_VP8_ENC_H__
#define  __GST_MPP_VP8_ENC_H__

#include "gstmppvideoenc.h"

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_MPP_VP8_ENC	(gst_mpp_vp8_enc_get_type())
#define GST_MPP_VP8_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPP_VP8_ENC, GstMppVP8Enc))
#define GST_MPP_VP8_ENC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_MPP_VP8_ENC, GstMppVP8EncClass))
#define GST_IS_MPP_VP8_ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_MPP_VP8_ENC))
#define GST_IS_MPP_VP8_ENC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_MPP_VP8_ENC))
typedef struct _GstMppVP8Enc GstMppVP8Enc;
typedef struct _GstMppVP8EncClass GstMppVP8EncClass;

struct _GstMppVP8Enc
{
  GstMppVideoEnc parent;

  guint qp_init;
  guint qp_min;
  guint qp_max;

  gboolean prop_dirty;
};

struct _GstMppVP8EncClass
{
  GstMppVideoEncClass parent_class;
};

GType gst_mpp_vp8_enc_get_type (void);

G_END_DECLS
#endif /* __GST_MPP_VP8_ENC_H__ */
