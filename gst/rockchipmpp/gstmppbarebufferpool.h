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

#ifndef __GST_MPP_BARE_BUFFER_POOL_H__
#define __GST_MPP_BARE_BUFFER_POOL_H__

#include <gst/gst.h>

#include "gstmppallocator.h"
#include "gstmppjpegdec.h"

G_BEGIN_DECLS typedef struct _GstMppBareBufferPool GstMppBareBufferPool;
typedef struct _GstMppBareBufferPoolClass GstMppBareBufferPoolClass;

#define GST_TYPE_MPP_BARE_BUFFER_POOL      (gst_mpp_bare_buffer_pool_get_type())
#define GST_IS_MPP_BARE_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MPP_BARE_BUFFER_POOL))
#define GST_MPP_BARE_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_MPP_BARE_BUFFER_POOL, GstMppBareBufferPool))
#define GST_MPP_BARE_BUFFER_POOL_CAST(obj) ((GstMppBareBufferPool *)(obj))

struct _GstMppBareBufferPool
{
  GstBufferPool parent;
  GstMppJpegDec *dec;

  guint num_queued;
  guint count;

  guint size;
  GstBuffer *buffers[VIDEO_MAX_FRAME];

  GstMppAllocator *vallocator;
  GstAllocator *allocator;
  GstAllocationParams params;
};

struct _GstMppBareBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

GType gst_mpp_bare_buffer_pool_get_type (void);

GstBufferPool *gst_mpp_bare_buffer_pool_new (GstMppJpegDec * dec,
    GstCaps * caps);

GstFlowReturn
gst_mpp_bare_buffer_pool_fill_frame (MppFrame mframe, GstBuffer * buffer);

G_END_DECLS
#endif /*__GST_MPP_BARE_BUFFER_POOL_H__ */
