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

#ifndef  __GST_MPP_H__
#define  __GST_MPP_H__

#include <gst/video/video.h>

#include <gst/gst-compat-private.h>
#include <rockchip/rk_mpi.h>

G_BEGIN_DECLS;

#define GST_MPP_VIDEO_INFO_HSTRIDE(i) GST_VIDEO_INFO_PLANE_STRIDE(i, 0)
#define GST_MPP_VIDEO_INFO_VSTRIDE(i) \
    GST_VIDEO_INFO_PLANE_OFFSET(i, 1) / GST_MPP_VIDEO_INFO_HSTRIDE(i)

gboolean gst_mpp_video_info_align (GstVideoInfo * info,
    guint hstride, guint vstride);

G_END_DECLS;

#endif /* __GST_MPP_H__ */
