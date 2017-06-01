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

#ifndef _GST_MPP_MEMORY_H
#define _GST_MPP_MEMORY_H

#include <gst/allocators/gstdmabuf.h>
#include <rockchip/rk_mpi.h>

G_BEGIN_DECLS
#define	VIDEO_MAX_FRAME	32
#define GST_MPP_MEMORY_QUARK gst_mpp_memory_quark ()

#define GST_TYPE_MPP_ALLOCATOR            (gst_mpp_allocator_get_type())
#define GST_IS_MPP_ALLOCATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MPP_ALLOCATOR))
#define GST_IS_MPP_ALLOCATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_MPP_ALLOCATOR))
#define GST_MPP_ALLOCATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_MPP_ALLOCATOR, GstMppAllocator))
#define GST_MPP_ALLOCATOR_CLASS(obj)      (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_MPP_ALLOCATOR, GstMppAllocatorClass))
#define GST_MPP_ALLOCATOR_CAST(obj)       ((GstMppAllocator *)(obj))

typedef struct _GstMppMemory GstMppMemory;
typedef struct _GstMppAllocator GstMppAllocator;
typedef struct _GstMppAllocatorClass GstMppAllocatorClass;

#define GST_MPP_MEMORY_CAST(mem) \
	((GstMppMemory *) (mem))

struct _GstMppMemory
{
  GstMemory mem;

  /* < private > */
  MppBuffer *mpp_buf;
  gpointer data;
  gint dmafd;
  gsize size;
};

struct _GstMppAllocator
{
  GstAllocator parent;
  gboolean active;

  guint32 count;       /* number of buffers allocated by the mpp */
  MppBufferGroup mpp_mem_pool;

  GstMppMemory *mems[VIDEO_MAX_FRAME];
  GstAtomicQueue *free_queue;
};

struct _GstMppAllocatorClass
{
  GstAllocatorClass parent_class;
};

GType gst_mpp_allocator_get_type (void);

gboolean gst_is_mpp_memory (GstMemory * mem);

GQuark gst_mpp_memory_quark (void);

GstMppAllocator *
gst_mpp_allocator_new (GstObject * parent);

guint
gst_mpp_allocator_start (GstMppAllocator * allocator,
		gsize size, guint32 count);

gint
gst_mpp_allocator_stop (GstMppAllocator * allocator);

GstMemory *
gst_mpp_allocator_alloc_dmabuf (GstMppAllocator * allocator,
    GstAllocator * dmabuf_allocator);


G_END_DECLS
#endif
