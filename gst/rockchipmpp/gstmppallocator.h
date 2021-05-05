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

#ifndef  __GST_MPP_ALLOCATOR_H__
#define  __GST_MPP_ALLOCATOR_H__

#include <gst/video/video.h>

#include <rockchip/rk_mpi.h>

G_BEGIN_DECLS;

GstAllocator *gst_mpp_allocator_new (void);

gint gst_mpp_allocator_get_index (GstAllocator * allocator);

MppBufferGroup gst_mpp_allocator_get_mpp_group (GstAllocator * allocator);

MppBuffer gst_mpp_mpp_buffer_from_gst_memory (GstMemory * mem);

GstMemory *gst_mpp_allocator_import_mppbuf (GstAllocator * allocator,
    MppBuffer mbuf);

GstMemory *gst_mpp_allocator_import_gst_memory (GstAllocator * allocator,
    GstMemory * mem);

MppBuffer gst_mpp_allocator_alloc_mppbuf (GstAllocator * allocator, gsize size);

G_END_DECLS;

#endif /* __GST_MPP_ALLOCATOR_H__ */
