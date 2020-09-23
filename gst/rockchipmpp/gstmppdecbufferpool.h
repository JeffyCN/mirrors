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

#ifndef __GST_MPP_DEC_BUFFER_POOL_H__
#define __GST_MPP_DEC_BUFFER_POOL_H__

#include <gst/gst.h>

#include "gstmppallocator.h"
#include "gstmppvideodec.h"

G_BEGIN_DECLS typedef struct _GstMppDecBufferPool GstMppDecBufferPool;
typedef struct _GstMppDecBufferPoolClass GstMppDecBufferPoolClass;

#define GST_TYPE_MPP_DEC_BUFFER_POOL      (gst_mpp_dec_buffer_pool_get_type())
#define GST_IS_MPP_DEC_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MPP_DEC_BUFFER_POOL))
#define GST_MPP_DEC_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_MPP_DEC_BUFFER_POOL, GstMppDecBufferPool))
#define GST_MPP_DEC_BUFFER_POOL_CAST(obj) ((GstMppDecBufferPool *)(obj))

#define GST_FLOW_CUSTOM_DROP    GST_FLOW_CUSTOM_ERROR_1
#define GST_FLOW_CUSTOM_TIMEOUT GST_FLOW_CUSTOM_ERROR_2

struct _GstMppDecBufferPool
{
  GstBufferPool parent;
  GstMppVideoDec *dec;

  guint num_queued;             /* number of buffers queued in the mpp/libvpu and gstvpudecbufferpool */

  guint size;
  GstBuffer *buffers[VIDEO_MAX_FRAME];

  GstMppAllocator *vallocator;
  GstAllocator *allocator;
  GstAllocationParams params;
};

struct _GstMppDecBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

GType gst_mpp_dec_buffer_pool_get_type (void);

GstBufferPool *gst_mpp_dec_buffer_pool_new (GstMppVideoDec * dec,
    GstCaps * caps);

G_END_DECLS
#endif /*__GST_MPP_DEC_BUFFER_POOL_H__ */
