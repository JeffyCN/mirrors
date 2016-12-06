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
#ifndef __GST_VPUDEC_BUFFER_POOL_H__
#define __GST_VPUDEC_BUFFER_POOL_H__

#include <gst/gst.h>

#include "gstvpuallocator.h"
#include "gstvpudec.h"

G_BEGIN_DECLS typedef struct _GstVpuDecBufferPool GstVpuDecBufferPool;
typedef struct _GstVpuDecBufferPoolClass GstVpuDecBufferPoolClass;

#define GST_TYPE_VPUDEC_BUFFER_POOL      (gst_vpudec_buffer_pool_get_type())
#define GST_IS_VPUDEC_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_VPUDEC_BUFFER_POOL))
#define GST_VPUDEC_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_VPUDEC_BUFFER_POOL, GstVpuDecBufferPool))
#define GST_VPUDEC_BUFFER_POOL_CAST(obj) ((GstVpuDecBufferPool *)(obj))

struct _GstVpuDecBufferPool
{
  GstBufferPool parent;
  GstVpuDec *dec;

  guint num_queued;  /* number of buffers queued in the mpp/libvpu and gstvpudecbufferpool */

  guint size;
  GstBuffer * buffers[VIDEO_MAX_FRAME];

  GstVpuAllocator *vallocator;
  GstAllocator *allocator;
  GstAllocationParams params;
};

struct _GstVpuDecBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

GType gst_vpudec_buffer_pool_get_type (void);

GstBufferPool *gst_vpudec_buffer_pool_new (GstVpuDec * dec, GstCaps * caps);

G_END_DECLS
#endif /*__GST_VPUDEC_BUFFER_POOL_H__ */
