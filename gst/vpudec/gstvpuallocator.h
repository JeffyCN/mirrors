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
#ifndef _GST_VPU_MEMORY_H
#define _GST_VPU_MEMORY_H

#include <gst/allocators/gstdmabuf.h>
#include <rockchip/vpu_api.h>

G_BEGIN_DECLS
#define	VIDEO_MAX_FRAME	32
#define GST_VPU_MEMORY_QUARK gst_vpu_memory_quark ()
#define GST_TYPE_VPU_ALLOCATOR (gst_vpu_allocator_get_type())

typedef struct _GstVpuMemory GstVpuMemory;
typedef struct _GstVpuAllocator GstVpuAllocator;
typedef struct _GstVpuAllocatorClass GstVpuAllocatorClass;

#define GST_VPU_MEMORY_CAST(mem) \
	((GstVpuMemory *) (mem))

struct _GstVpuMemory
{
  GstMemory mem;

  /* < private > */
  VPUMemLinear_t *vpu_mem;
  gpointer data;
  gint dmafd;
  gsize size;
};

struct _GstVpuAllocator
{
  GstAllocator parent;
  gboolean active;

  guint32 count;       /* number of buffers allocated by the vpu */
  vpu_display_mem_pool *vpu_display_pool;

  GstVpuMemory *mems[VIDEO_MAX_FRAME];
  GstAtomicQueue *free_queue;
};

struct _GstVpuAllocatorClass
{
  GstAllocatorClass parent_class;
};

GType gst_vpu_allocator_get_type (void);

gboolean gst_is_vpu_memory (GstMemory * mem);

GQuark gst_vpu_memory_quark (void);

GstVpuAllocator *
gst_vpu_allocator_new (GstObject * parent);

guint
gst_vpu_allocator_start (GstVpuAllocator * allocator,
    gpointer vpool, gsize size, guint32 count);

gint
gst_vpu_allocator_stop (GstVpuAllocator * allocator);

GstMemory *
gst_vpu_allocator_alloc_dmabuf (GstVpuAllocator * allocator,
    GstAllocator * dmabuf_allocator);


G_END_DECLS
#endif
