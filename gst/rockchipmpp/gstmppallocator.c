/*
 * Copyright 2021 Rockchip Electronics Co., Ltd
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <gst/allocators/gstdmabuf.h>

#include "gstmppallocator.h"

#define GST_TYPE_MPP_ALLOCATOR (gst_mpp_allocator_get_type())
G_DECLARE_FINAL_TYPE (GstMppAllocator, gst_mpp_allocator, GST,
    MPP_ALLOCATOR, GstDmaBufAllocator);

#define GST_MPP_ALLOCATOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_ALLOCATOR, GstMppAllocator))

#define GST_CAT_DEFAULT mppallocator_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define GST_ALLOCATOR_MPP "mpp"

struct _GstMppAllocator
{
  GstDmaBufAllocator parent;

  /* group for buffer-alloc */
  MppBufferGroup group;

  /* group for buffer-import */
  MppBufferGroup ext_group;

  /* unique group ID */
  gint index;
};

#define gst_mpp_allocator_parent_class parent_class
G_DEFINE_TYPE (GstMppAllocator, gst_mpp_allocator, GST_TYPE_DMABUF_ALLOCATOR);

static GQuark
gst_mpp_buffer_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_string ("mpp-buf");

  return quark;
}

static GQuark
gst_mpp_ext_buffer_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_string ("mpp-ext-buf");

  return quark;
}

gint
gst_mpp_allocator_get_index (GstAllocator * allocator)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (allocator);
  return self->index;
}

MppBufferGroup
gst_mpp_allocator_get_mpp_group (GstAllocator * allocator)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (allocator);
  return self->group;
}

MppBuffer
gst_mpp_mpp_buffer_from_gst_memory (GstMemory * mem)
{
  if (mem->parent)
    return gst_mpp_mpp_buffer_from_gst_memory (mem->parent);

  return gst_mini_object_get_qdata (GST_MINI_OBJECT (mem),
      gst_mpp_buffer_quark ());
}

static void
gst_mpp_mem_destroy (gpointer ptr)
{
  MppBuffer mbuf = ptr;

  mpp_buffer_put (mbuf);
}

static GstMemory *
gst_mpp_allocator_import_dmafd (GstAllocator * allocator, gint fd, guint size)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (allocator);
  GstMemory *mem;
  MppBufferInfo info = { 0, };
  MppBuffer mbuf = NULL;

  GST_DEBUG_OBJECT (self, "import dmafd: %d (%d)", fd, size);

  info.type = MPP_BUFFER_TYPE_DRM;
  info.size = size;
  info.fd = fd;

  /* Avoid caching too much external buffers */
  mpp_buffer_group_clear (self->ext_group);

  mpp_buffer_import_with_tag (self->ext_group, &info, &mbuf, NULL, __func__);
  if (!mbuf)
    return NULL;

  mpp_buffer_set_index (mbuf, self->index);

  mem = gst_mpp_allocator_import_mppbuf (allocator, mbuf);
  mpp_buffer_put (mbuf);

  return mem;
}

GstMemory *
gst_mpp_allocator_import_mppbuf (GstAllocator * allocator, MppBuffer mbuf)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (allocator);
  GstMemory *mem;
  GQuark quark;
  guint size;
  gint fd;

  GST_DEBUG_OBJECT (self, "import MPP buffer");

  fd = mpp_buffer_get_fd (mbuf);
  if (fd < 0) {
    GST_ERROR_OBJECT (self, "failed to get dmafd");
    return NULL;
  }

  /* HACK: DRM buffers are actually aligned to 4096 (page) */
  size = GST_ROUND_UP_N (mpp_buffer_get_size (mbuf), 4096);

  if (mpp_buffer_get_index (mbuf) != self->index) {
    GST_DEBUG_OBJECT (self, "import from other group");
    mem = gst_mpp_allocator_import_dmafd (allocator, fd, size);
    quark = gst_mpp_ext_buffer_quark ();
  } else {
    mem = gst_fd_allocator_alloc (allocator, dup (fd), size,
        GST_FD_MEMORY_FLAG_KEEP_MAPPED);
    quark = gst_mpp_buffer_quark ();
  }

  mpp_buffer_inc_ref (mbuf);
  gst_mini_object_set_qdata (GST_MINI_OBJECT (mem), quark, mbuf,
      gst_mpp_mem_destroy);

  return mem;
}

GstMemory *
gst_mpp_allocator_import_gst_memory (GstAllocator * allocator, GstMemory * mem)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (allocator);
  MppBuffer mbuf;
  gsize offset;
  guint size;
  gint fd;

  GST_DEBUG_OBJECT (self, "import gst memory");

  if (!gst_is_dmabuf_memory (mem))
    return NULL;

  mbuf = gst_mpp_mpp_buffer_from_gst_memory (mem);
  if (mbuf)
    return gst_mpp_allocator_import_mppbuf (allocator, mbuf);

  fd = gst_dmabuf_memory_get_fd (mem);
  if (fd < 0) {
    GST_ERROR_OBJECT (self, "failed to get dmafd");
    return NULL;
  }

  size = gst_memory_get_sizes (mem, &offset, NULL);
  if (offset)
    return NULL;

  return gst_mpp_allocator_import_dmafd (allocator, fd, size);
}

MppBuffer
gst_mpp_allocator_alloc_mppbuf (GstAllocator * allocator, gsize size)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (allocator);
  MppBuffer mbuf = NULL;

  mpp_buffer_get (self->group, &mbuf, size);
  mpp_buffer_set_index (mbuf, self->index);

  return mbuf;
}

static GstMemory *
gst_mpp_allocator_alloc (GstAllocator * allocator, gsize size,
    GstAllocationParams * params UNUSED)
{
  GstMemory *mem;
  MppBuffer mbuf;

  mbuf = gst_mpp_allocator_alloc_mppbuf (allocator, size);
  if (!mbuf)
    return NULL;

  mem = gst_mpp_allocator_import_mppbuf (allocator, mbuf);
  mpp_buffer_put (mbuf);

  gst_memory_resize (mem, 0, size);
  return mem;
}

GstAllocator *
gst_mpp_allocator_new (void)
{
  GstMppAllocator *alloc;
  MppBufferGroup group, ext_group;

  static gint num_mpp_alloc = 0;

  if (mpp_buffer_group_get_internal (&group, MPP_BUFFER_TYPE_DRM))
    return FALSE;

  if (mpp_buffer_group_get_external (&ext_group, MPP_BUFFER_TYPE_DRM)) {
    mpp_buffer_group_put (group);
    return FALSE;
  }

  alloc = g_object_new (GST_TYPE_MPP_ALLOCATOR, NULL);
  gst_object_ref_sink (alloc);

  alloc->group = group;
  alloc->ext_group = ext_group;
  alloc->index = num_mpp_alloc++;

  return GST_ALLOCATOR_CAST (alloc);
}

static void
gst_mpp_allocator_finalize (GObject * obj)
{
  GstMppAllocator *self = GST_MPP_ALLOCATOR (obj);

  mpp_buffer_group_put (self->group);
  mpp_buffer_group_put (self->ext_group);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_mpp_allocator_class_init (GstMppAllocatorClass * klass)
{
  GstAllocatorClass *allocator_class = GST_ALLOCATOR_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppallocator", 0, "MPP allocator");

  allocator_class->alloc = GST_DEBUG_FUNCPTR (gst_mpp_allocator_alloc);

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_mpp_allocator_finalize);
}

static void
gst_mpp_allocator_init (GstMppAllocator * allocator)
{
  GstAllocator *alloc = GST_ALLOCATOR_CAST (allocator);

  alloc->mem_type = GST_ALLOCATOR_MPP;

  GST_OBJECT_FLAG_SET (allocator, GST_ALLOCATOR_FLAG_CUSTOM_ALLOC);
}
