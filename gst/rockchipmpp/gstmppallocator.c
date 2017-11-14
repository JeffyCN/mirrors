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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _GNU_SOURCE
# define _GNU_SOURCE            /* O_CLOEXEC */
#endif

#include <gst/allocators/gstdmabuf.h>

#include "gstmppallocator.h"

#define GST_MPP_MEMORY_TYPE "MppMemory"

#define gst_mpp_allocator_parent_class parent_class
G_DEFINE_TYPE (GstMppAllocator, gst_mpp_allocator, GST_TYPE_ALLOCATOR);

GST_DEBUG_CATEGORY_STATIC (mppallocator_debug);
#define GST_CAT_DEFAULT mppallocator_debug

gboolean
gst_is_mpp_memory (GstMemory * mem)
{

  return gst_memory_is_type (mem, GST_MPP_MEMORY_TYPE);
}

GQuark
gst_mpp_memory_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_string ("GstMppMemory");

  return quark;
}

static inline GstMppMemory *
_mppmem_new (GstMemoryFlags flags, GstAllocator * allocator,
    GstMemory * parent, gsize maxsize, gsize align, gsize offset, gsize size,
    gpointer data, int dmafd, MppBuffer * mpp_buf)
{
  GstMppMemory *mem;

  mem = g_slice_new0 (GstMppMemory);
  gst_memory_init (GST_MEMORY_CAST (mem),
      flags, allocator, parent, maxsize, align, offset, size);

  mem->data = data;
  mem->dmafd = dmafd;
  mem->mpp_buf = mpp_buf;
  mem->size = maxsize;

  return mem;
}

static void
_mppmem_free (GstMppMemory * mem)
{
  g_slice_free (GstMppMemory, mem);
}

static gpointer
_mppmem_map (GstMppMemory * mem, gsize maxsize, GstMapFlags flags)
{
  if (!mem->data)
    mem->data = mpp_buffer_get_ptr (mem->mpp_buf);

  return mem->data;
}

static void
_mppmem_unmap (GstMemory * mem)
{
  return;
}

static GstMppMemory *
_mppmem_share (GstMppMemory * mem, gssize offset, gssize size)
{
  GstMppMemory *sub;
  GstMemory *parent;

  /* find the real parent */
  if ((parent = mem->mem.parent) == NULL)
    parent = (GstMemory *) mem;

  if (size == -1)
    size = mem->mem.size - offset;

  /* the shared memory is always readonly */
  sub = _mppmem_new (GST_MINI_OBJECT_FLAGS (parent) |
      GST_MINI_OBJECT_FLAG_LOCK_READONLY, mem->mem.allocator, parent,
      mem->mem.maxsize, mem->mem.align, offset, size, mem->data,
      -1, mem->mpp_buf);

  return sub;
}

static gboolean
_mppmem_is_span (GstMppMemory * mem1, GstMppMemory * mem2, gsize * offset)
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
gst_mpp_allocator_dispose (GObject * obj)
{
  GST_LOG_OBJECT (obj, "called");

  G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
gst_mpp_allocator_finalize (GObject * obj)
{
  GstMppAllocator *allocator = (GstMppAllocator *) obj;

  GST_LOG_OBJECT (obj, "called");

  if (allocator->mpp_mem_pool) {
    mpp_buffer_group_put (allocator->mpp_mem_pool);
    allocator->mpp_mem_pool = NULL;
  }

  gst_atomic_queue_unref (allocator->free_queue);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

/* Manually clean way */
gint
gst_mpp_allocator_stop (GstMppAllocator * allocator)
{
  guint i = 0;
  gint ret = 0;
  GST_DEBUG_OBJECT (allocator, "stop allocator");

  GST_OBJECT_LOCK (allocator);

  if (!g_atomic_int_get (&allocator->active))
    goto done;

  if (allocator->mpp_mem_pool) {
    mpp_buffer_group_put (allocator->mpp_mem_pool);
    allocator->mpp_mem_pool = NULL;
  }

  if (gst_atomic_queue_length (allocator->free_queue) != allocator->count) {
    GST_DEBUG_OBJECT (allocator, "allocator is still in use");
    ret = -EBUSY;
    goto done;
  }
  while (gst_atomic_queue_pop (allocator->free_queue)) {
    /* Nothing */
  };

  for (i = 0; i < allocator->count; i++) {
    GstMppMemory *mem = allocator->mems[i];
    allocator->mems[i] = NULL;
    if (mem)
      _mppmem_free (mem);
  }

  allocator->count = 0;

  g_atomic_int_set (&allocator->active, FALSE);
done:
  GST_OBJECT_UNLOCK (allocator);
  return ret;
}

static void
gst_mpp_allocator_free (GstAllocator * gallocator, GstMemory * gmem)
{
  GstMppMemory *mem = (GstMppMemory *) gmem;

  _mppmem_free (mem);
}

GstMemory *
gst_mpp_allocator_alloc_dmabuf (GstMppAllocator * allocator,
    GstAllocator * dmabuf_allocator)
{
  GstMppMemory *mem;
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
      GST_MPP_MEMORY_QUARK, mem, (GDestroyNotify) gst_memory_unref);

  return dma_mem;
}

GstMppAllocator *
gst_mpp_allocator_new (GstObject * parent)
{
  GstMppAllocator *allocator = NULL;
  gchar *name, *parent_name;

  parent_name = gst_object_get_name (parent);
  name = g_strconcat (parent_name, ":allocator", NULL);
  g_free (parent_name);

  allocator = g_object_new (GST_TYPE_MPP_ALLOCATOR, "name", name, NULL);
  g_free (name);

  return allocator;
}

static guint
gst_mpp_allocator_drm_buf (GstMppAllocator * allocator, gsize size,
    guint32 count)
{
  MppBufferGroup group;
  MppBuffer temp_buf[VIDEO_MAX_FRAME];

  /* FIXME the rockchip mpp should support DRM type properly */
  mpp_buffer_group_get_internal (&group, MPP_BUFFER_TYPE_ION);

  mpp_buffer_group_get_external (&allocator->mpp_mem_pool, MPP_BUFFER_TYPE_ION);
  if (allocator->mpp_mem_pool == NULL)
    goto mpp_mem_pool_error;

  /* Create DRM buffer from rockchip mpp internally */
  for (gint i = 0; i < count; i++) {
    /*
     * Create MppBuffer from Rockchip Mpp
     * included mvc data
     */
    if (mpp_buffer_get (group, &temp_buf[i], size)) {
      GST_ERROR_OBJECT (allocator, "allocate internal buffer %d failed", i);
      goto error;
    }
  }

  for (gint i = 0; i < count; i++) {
    MppBuffer mpp_buf;
    MppBufferInfo commit;

    mpp_buf = temp_buf[i];
    mpp_buffer_set_index (mpp_buf, i);
    mpp_buffer_info_get (mpp_buf, &commit);

    if (mpp_buffer_commit (allocator->mpp_mem_pool, &commit)) {
      GST_DEBUG_OBJECT (allocator, "commit buffer %d failed", i);
      continue;
    }

    mpp_buffer_put (mpp_buf);
    /* Remember to release the reference of this buffer */
    if (!mpp_buffer_get (allocator->mpp_mem_pool, &mpp_buf, size)) {

      allocator->mems[i] = _mppmem_new (0, GST_ALLOCATOR (allocator), NULL,
          mpp_buffer_get_size (mpp_buf), 0, 0, mpp_buffer_get_size (mpp_buf),
          NULL, mpp_buffer_get_fd (mpp_buf), mpp_buf);

    }
    if (gst_is_mpp_memory ((GstMemory *) allocator->mems[i])) {
      gst_atomic_queue_push (allocator->free_queue, allocator->mems[i]);
      allocator->count++;
    } else {
      GST_ERROR_OBJECT (allocator, "allocate buffer %d failed",
          mpp_buffer_get_index (mpp_buf));
      goto error;
    }
  }

  mpp_buffer_group_put (group);
  return (gst_atomic_queue_length (allocator->free_queue));

mpp_mem_pool_error:
  {
    GST_ERROR_OBJECT (allocator, "failed to create mpp memory pool");
    goto error;
  }
error:
  {
    allocator->count = 0;
    return 0;
  }
}

guint
gst_mpp_allocator_start (GstMppAllocator * allocator, gsize size, guint32 count)
{
  g_return_val_if_fail (count != 0, 0);
  g_return_val_if_fail (size != 0, 0);

  GST_OBJECT_LOCK (allocator);
  if (g_atomic_int_get (&allocator->active))
    goto already_active;

  gst_mpp_allocator_drm_buf (allocator, size, count);

  g_atomic_int_set (&allocator->active, TRUE);

done:
  GST_OBJECT_UNLOCK (allocator);

  return (gst_atomic_queue_length (allocator->free_queue));
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
gst_mpp_allocator_class_init (GstMppAllocatorClass * klass)
{
  GObjectClass *object_class;
  GstAllocatorClass *allocator_class;

  allocator_class = (GstAllocatorClass *) klass;
  object_class = (GObjectClass *) klass;

  allocator_class->alloc = NULL;
  allocator_class->free = gst_mpp_allocator_free;

  object_class->dispose = gst_mpp_allocator_dispose;
  object_class->finalize = gst_mpp_allocator_finalize;

  GST_DEBUG_CATEGORY_INIT (mppallocator_debug, "mppallocator", 0,
      "MPP Allocator");
}

static void
gst_mpp_allocator_init (GstMppAllocator * allocator)
{
  GstAllocator *alloc = GST_ALLOCATOR_CAST (allocator);

  alloc->mem_type = GST_MPP_MEMORY_TYPE;
  alloc->mem_map = (GstMemoryMapFunction) _mppmem_map;
  alloc->mem_unmap = (GstMemoryUnmapFunction) _mppmem_unmap;
  alloc->mem_share = (GstMemoryShareFunction) _mppmem_share;
  alloc->mem_is_span = (GstMemoryIsSpanFunction) _mppmem_is_span;

  allocator->free_queue = gst_atomic_queue_new (VIDEO_MAX_FRAME);

  GST_OBJECT_FLAG_SET (allocator, GST_ALLOCATOR_FLAG_CUSTOM_ALLOC);
}
