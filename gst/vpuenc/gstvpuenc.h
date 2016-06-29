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

#ifndef  GSTVPUENC_H
#define  GSTVPUENC_H

#include <gst/gst.h>
#include <gst/video/video.h>

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_VPUENC	(gst_vpuenc_get_type())
#define GST_VPUENC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_VPUENC, GstVpuEnc))
#define GST_VPUENC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_VPUENC, GstVPUEncClass))
#define GST_IS_VPUENC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_VPUENC))
#define GST_IS_VPUENC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_VPUENC))

typedef struct _GstVpuEnc GstVpuEnc;
typedef struct _GstVpuEncClass GstVpuEncClass;

struct _GstVpuEnc {
  GstVideoEncoder parent;

  gchar *device_name;

  gint fd;

  /* Structure representing the state of an incoming or outgoing video stream
  for encoders and decoders. */
  GstVideoCodecState *input_state;
  GstVideoCodecState *output_state;

  gint width;
  gint height;

  /* properties */
  guint level;
  guint profile;

  GValue framerate;
};

struct _GstVpuEncClass {

  GstVideoEncoderClass parent_class;
};

GType gst_vpuenc_get_type(void);


G_END_DECLS

#endif /* GSTVPUENC_H */
