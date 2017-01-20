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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _GNU_SOURCE
# define _GNU_SOURCE            /* O_CLOEXEC */
#endif

#include <gst/allocators/gstdmabuf.h>
#include <rockchip/vpu_api.h>

#include "gstvpuallocator.h"

#define GST_VPU_MEMORY_TYPE "VpuMemory"

#define gst_vpu_allocator_parent_class parent_class
G_DEFINE_TYPE (GstVpuAllocator, gst_vpu_allocator, GST_TYPE_ALLOCATOR);

GST_DEBUG_CATEGORY_STATIC (vpuallocator_debug);
#define GST_CAT_DEFAULT vpuallocator_debug

gboolean
gst_is_vpu_memory (GstMemory * mem)
{
  return gst_memory_is_type (mem, GST_VPU_MEMORY_TYPE);
}

GQuark
gst_vpu_memory_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_string ("GstVpuMemory");

  return quark;
}

static inline GstVpuMemory *
_vpumem_new (GstMemoryFlags flags, GstAllocator * allocator,
    GstMemory * parent, gsize maxsize, gsize align, gsize offset, gsize size,
    gpointer data, int dmafd, VPUMemLinear_t * vpu_mem)
{
  GstVpuMemory *mem;

  mem = g_slice_new0 (GstVpuMemory);
  gst_memory_init (GST_MEMORY_CAST (mem),
      flags, allocator, parent, maxsize, align, offset, size);

  mem->data = data;
  mem->dmafd = dmafd;
  mem->vpu_mem = vpu_mem;
  mem->size = maxsize;

  return mem;
}

static void
_vpumem_free (GstVpuMemory * mem)
{
  g_slice_free (GstVpuMemory, mem);
}

static gpointer
_vpumem_map (GstVpuMemory * mem, gsize maxsize, GstMapFlags flags)
{
  return mem->data;
}

static void
_vpumem_unmap (GstMemory * mem)
{
  return;
}

static GstVpuMemory *
_vpumem_share (GstVpuMemory * mem, gssize offset, gssize size)
{
  GstVpuMemory *sub;
  GstMemory *parent;

  /* find the real parent */
  if ((parent = mem->mem.parent) == NULL)
    parent = (GstMemory *) mem;

  if (size == -1)
    size = mem->mem.size - offset;

  /* the shared memory is always readonly */
  sub = _vpumem_new (GST_MINI_OBJECT_FLAGS (parent) |
      GST_MINI_OBJECT_FLAG_LOCK_READONLY, mem->mem.allocator, parent,
      mem->mem.maxsize, mem->mem.align, offset, size, mem->data,
      -1, mem->vpu_mem);

  return sub;
}

static gboolean
_vpumem_is_span (GstVpuMemory * mem1, GstVpuMemory * mem2, gsize * offset)
{
  if (offset)
    *offset = mem1->mem.offset - mem1->mem.parent->offset;

  /* and memory is contiguous */
  return mem1->mem.offset + mem1->mem.size == mem2->mem.offset;
}

/*
 * GstVpuAllocator Implementation
 */

/* Auto clean up methods */
static void
gst_vpu_allocator_dispose (GObject * obj)
{
  GST_LOG_OBJECT (obj, "called");
  /* Don't need cleanup buffers from allocator again,
   * the free() method have done that */

  G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
gst_vpu_allocator_finalize (GObject * obj)
{
  GstVpuAllocator *allocator = (GstVpuAllocator *) obj;

  GST_LOG_OBJECT (obj, "called");

  if (allocator->vpu_display_pool)
    close_vpu_memory_pool (allocator->vpu_display_pool);

  gst_atomic_queue_unref (allocator->free_queue);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

/* Manually clean way */
gint
gst_vpu_allocator_stop (GstVpuAllocator * allocator)
{
  guint i = 0;
  gint ret = 0;
  GST_DEBUG_OBJECT (allocator, "stop allocator");

  GST_OBJECT_LOCK (allocator);

  if (!g_atomic_int_get (&allocator->active))
    goto done;

  if (gst_atomic_queue_length (allocator->free_queue) != allocator->count) {
    GST_DEBUG_OBJECT (allocator, "allocator is still in use");
    ret = -EBUSY;
    goto done;
  }
  while (gst_atomic_queue_pop (allocator->free_queue)) {
    /* Nothing */
  };

  for (i = 0; i < allocator->count; i++) {
    GstVpuMemory *mem = allocator->mems[i];
    allocator->mems[i] = NULL;
    if (mem)
      _vpumem_free (mem);
  }

  allocator->count = 0;

  if (allocator->vpu_display_pool) {
    close_vpu_memory_pool (allocator->vpu_display_pool);
    allocator->vpu_display_pool = NULL;
  }

  g_atomic_int_set (&allocator->active, FALSE);
done:
  GST_OBJECT_UNLOCK (allocator);
  return ret;
}

static void
gst_vpu_allocator_free (GstAllocator * gallocator, GstMemory * gmem)
{
  GstVpuMemory *mem = (GstVpuMemory *) gmem;

  g_slice_free (GstVpuMemory, mem);
}

GstMemory *
gst_vpu_allocator_alloc_dmabuf (GstVpuAllocator * allocator,
    GstAllocator * dmabuf_allocator)
{
  GstVpuMemory *mem;
  GstMemory *dma_mem;

  mem = gst_atomic_queue_pop (allocator->free_queue);
  if (mem == NULL)
    return NULL;

  if (mem->dmafd < 0) {
    GST_ERROR_OBJECT (allocator, "Failed to get dmafd");
    gst_atomic_queue_push (allocator->free_queue, mem);

    return NULL;
  }

  dma_mem = gst_dmabuf_allocator_alloc (dmabuf_allocator, mem->dmafd,
      mem->size);
  gst_mini_object_set_qdata (GST_MINI_OBJECT (dma_mem),
      GST_VPU_MEMORY_QUARK, mem, (GDestroyNotify) gst_memory_unref);

  return dma_mem;
}

GstVpuAllocator *
gst_vpu_allocator_new (GstObject * parent)
{
  GstVpuAllocator *allocator = NULL;
  gchar *name, *parent_name;

  parent_name = gst_object_get_name (parent);
  name = g_strconcat (parent_name, ":allocator", NULL);
  g_free (parent_name);

  allocator = g_object_new (GST_TYPE_VPU_ALLOCATOR, "name", name, NULL);
  g_free (name);

  return allocator;
}

guint
gst_vpu_allocator_start (GstVpuAllocator * allocator,
    gpointer vpu, gsize size, guint32 count)
{
  VpuCodecContext_t *vpu_codec_ctx = (VpuCodecContext_t *) vpu;
  vpu_display_mem_pool *vpu_display_pool;
  gint i, ret;
  VPU_SYNC sync;

  g_return_val_if_fail (count != 0, 0);
  g_return_val_if_fail (size != 0, 0);

  GST_OBJECT_LOCK (allocator);
  if (g_atomic_int_get (&allocator->active))
    goto already_active;

  allocator->vpu_display_pool = open_vpu_memory_pool ();
  vpu_display_pool = allocator->vpu_display_pool;
  if (vpu_display_pool == NULL)
    goto vpu_display_pool_error;
  vpu_codec_ctx->control (vpu_codec_ctx, VPU_API_SET_VPUMEM_CONTEXT,
      vpu_display_pool);
  sync.flag = 1;
  /* If no buffers is ready, block the dequeue operation */
  vpu_codec_ctx->control (vpu_codec_ctx,
      VPU_API_SET_OUTPUT_BLOCK, (void *) &sync);

  for (i = 0; i < count; i++) {
    VPUMemLinear_t *vpu_mem;
    VPUMemLinear_t temp_vpu_mem;
    /*
     * Create vpumem from mpp/libvpu
     * included mvc data
     */
    VPUMallocLinearOutside (&temp_vpu_mem, size);
    vpu_mem = g_slice_copy (sizeof (VPUMemLinear_t), &temp_vpu_mem);
    vpu_mem->index = allocator->count;

    ret = vpu_display_pool->commit_vpu (allocator->vpu_display_pool, vpu_mem);
    /* Release the old buffer reference in the other memory group */
    VPUFreeLinear (&temp_vpu_mem);
    if (ret > 0)
      vpu_display_pool->inc_used (vpu_display_pool, vpu_mem);

    allocator->mems[i] = _vpumem_new (0, GST_ALLOCATOR (allocator), NULL,
        vpu_mem->size, 0, 0, vpu_mem->size, NULL, VPUMemGetFD (vpu_mem),
        vpu_mem);

    if (gst_is_vpu_memory ((GstMemory *) allocator->mems[i])) {
      gst_atomic_queue_push (allocator->free_queue, allocator->mems[i]);
      allocator->count++;
    } else {
      GST_ERROR_OBJECT (allocator, "allocate buffer %d failed", vpu_mem->index);
      goto error;
    }
  }

  g_atomic_int_set (&allocator->active, TRUE);

done:
  GST_OBJECT_UNLOCK (allocator);

  return (gst_atomic_queue_length (allocator->free_queue));
vpu_display_pool_error:
  {
    GST_ERROR_OBJECT (allocator, "failed to create vpu display pool");
    goto error;
  }
already_active:
  {
    GST_ERROR_OBJECT (allocator, "allocator already active");
    goto error;
  }
error:
  {
    allocator->count = 0;
    goto done;
  }
}

static void
gst_vpu_allocator_class_init (GstVpuAllocatorClass * klass)
{
  GObjectClass *object_class;
  GstAllocatorClass *allocator_class;

  allocator_class = (GstAllocatorClass *) klass;
  object_class = (GObjectClass *) klass;

  allocator_class->alloc = NULL;
  allocator_class->free = gst_vpu_allocator_free;

  object_class->dispose = gst_vpu_allocator_dispose;
  object_class->finalize = gst_vpu_allocator_finalize;

  GST_DEBUG_CATEGORY_INIT (vpuallocator_debug, "vpuallocator", 0,
      "VPU Allocator");
}

static void
gst_vpu_allocator_init (GstVpuAllocator * allocator)
{
  GstAllocator *alloc = GST_ALLOCATOR_CAST (allocator);

  alloc->mem_type = GST_VPU_MEMORY_TYPE;
  alloc->mem_map = (GstMemoryMapFunction) _vpumem_map;
  alloc->mem_unmap = (GstMemoryUnmapFunction) _vpumem_unmap;
  alloc->mem_share = (GstMemoryShareFunction) _vpumem_share;
  alloc->mem_is_span = (GstMemoryIsSpanFunction) _vpumem_is_span;

  allocator->free_queue = gst_atomic_queue_new (VIDEO_MAX_FRAME);

  GST_OBJECT_FLAG_SET (allocator, GST_ALLOCATOR_FLAG_CUSTOM_ALLOC);
}
