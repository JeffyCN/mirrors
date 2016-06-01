/*
 * Copyright 2016 Rockchip Electronics S.LSI Co. LTD
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

#include "gstvpudec.h"
#include "vpu_api.h"
#include "vpu_mem_pool.h"

G_BEGIN_DECLS typedef struct _GstVpuDecBufferPool GstVpuDecBufferPool;
typedef struct _GstVpuDecBufferPoolClass GstVpuDecBufferPoolClass;
typedef struct _GstVpuDecMeta GstVpuDecMeta;

#define GST_TYPE_VPUDEC_BUFFER_POOL      (gst_vpudec_buffer_pool_get_type())
#define GST_IS_VPUDEC_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_VPUDEC_BUFFER_POOL))
#define GST_VPUDEC_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_VPUDEC_BUFFER_POOL, GstVpuDecBufferPool))
#define GST_VPUDEC_BUFFER_POOL_CAST(obj) ((GstVpuDecBufferPool *)(obj))

struct _GstVpuDecBufferPool
{
  GstBufferPool parent;
  GstVpuDec *dec;

  guint nb_buffers;             /* number of buffers used */
  guint nb_buffers_alloc;       /* number of buffers allocated by the vpu */
  guint nb_queued;              /* number of buffers queued in the vpu */

  guint buf_alloc_size;         /* size of allocation */
  GstBuffer **buffers;
};

struct _GstVpuDecBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

struct _GstVpuDecMeta
{
  GstMeta meta;
  gpointer mem;
  guint index;
  gint size;
  gint dmabuf_fd;
  VPUMemLinear_t vpumem;
};

GType gst_vpudec_meta_api_get_type (void);
const GstMetaInfo *gst_vpudec_meta_get_info (void);
#define GST_VPUDEC_META_GET(buf) ((GstVpuDecMeta *)gst_buffer_get_meta(buf, gst_vpudec_meta_api_get_type()))
#define GST_VPUDEC_META_ADD(buf) ((GstVpuDecMeta *)gst_buffer_add_meta(buf, gst_vpudec_meta_get_info(),NULL))

GType gst_vpudec_buffer_pool_get_type (void);

GstBufferPool *gst_vpudec_buffer_pool_new (GstVpuDec * dec, GstCaps * caps,
    guint max, guint size, GstVideoAlignment * align);

G_END_DECLS
#endif /*__GST_VPUDEC_BUFFER_POOL_H__ */
