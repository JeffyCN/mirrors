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

#ifdef HAVE_RGA
#include <rga/rga.h>
#include <rga/RgaApi.h>
#endif

#include <gst/gst-compat-private.h>
#include <rockchip/rk_mpi.h>

G_BEGIN_DECLS;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#define MPP_FMT_LE(f) (f | MPP_FRAME_FMT_LE_MASK)
#define MPP_FMT_RGB565LE MPP_FMT_LE(MPP_FMT_RGB565)
#define MPP_FMT_BGR565LE MPP_FMT_LE(MPP_FMT_BGR565)

#define GST_MPP_VIDEO_INFO_HSTRIDE(i) GST_VIDEO_INFO_PLANE_STRIDE(i, 0)
#define GST_MPP_VIDEO_INFO_VSTRIDE(i) \
    (GST_VIDEO_INFO_N_PLANES(i) == 1 ? GST_VIDEO_INFO_HEIGHT(i) : \
    (gint) (GST_VIDEO_INFO_PLANE_OFFSET(i, 1) / GST_MPP_VIDEO_INFO_HSTRIDE(i)))

GstVideoFormat gst_mpp_mpp_format_to_gst_format (MppFrameFormat mpp_format);

MppFrameFormat gst_mpp_gst_format_to_mpp_format (GstVideoFormat format);

#ifdef HAVE_RGA
gboolean gst_mpp_rga_convert (GstBuffer * inbuf, GstVideoInfo * src_vinfo,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo);

gboolean gst_mpp_rga_convert_from_mpp_frame (MppFrame * mframe,
    GstMemory * out_mem, GstVideoInfo * dst_vinfo);
#endif

gboolean gst_mpp_video_info_align (GstVideoInfo * info,
    gint hstride, gint vstride);

G_END_DECLS;

#endif /* __GST_MPP_H__ */
