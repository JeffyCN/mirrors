/* GStreamer
 *
 * Copyright (C) 2016 Igalia
 *
 * Authors:
 *  Víctor Manuel Jáquez Leal <vjaquez@igalia.com>
 *  Javier Martin <javiermartin@by.com.es>
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

#ifndef __GST_KMS_UTILS_H__
#define __GST_KMS_UTILS_H__

#include <gst/video/video.h>

G_BEGIN_DECLS

#ifndef DRM_FORMAT_NV12_10
#define DRM_FORMAT_NV12_10 fourcc_code('N', 'A', '1', '2')
#endif

#ifndef DRM_FORMAT_NV15
#define DRM_FORMAT_NV15 fourcc_code('N', 'V', '1', '5')
#endif

#ifndef DRM_FORMAT_YUV420_8BIT
#define DRM_FORMAT_YUV420_8BIT fourcc_code('Y', 'U', '0', '8')
#endif

#ifndef DRM_FORMAT_YUV420_10BIT
#define DRM_FORMAT_YUV420_10BIT fourcc_code('Y', 'U', '1', '0')
#endif

#ifndef DRM_FORMAT_MOD_VENDOR_ARM
#define DRM_FORMAT_MOD_VENDOR_ARM 0x08
#endif

#ifndef DRM_FORMAT_MOD_ARM_AFBC
#define DRM_FORMAT_MOD_ARM_AFBC(__afbc_mode) fourcc_mod_code(ARM, __afbc_mode)
#endif

#ifndef AFBC_FORMAT_MOD_BLOCK_SIZE_16x16
#define AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 (1ULL)
#endif

#ifndef AFBC_FORMAT_MOD_SPARSE
#define AFBC_FORMAT_MOD_SPARSE (((__u64)1) << 6)
#endif

#define DRM_AFBC_MODIFIER \
  (DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_SPARSE) | \
   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16))

#ifndef GST_VIDEO_FLAG_ARM_AFBC
#define GST_VIDEO_FLAG_ARM_AFBC (1UL << 31)
#define GST_VIDEO_INFO_SET_AFBC(i) \
  GST_VIDEO_INFO_FLAG_SET (i, GST_VIDEO_FLAG_ARM_AFBC)
#define GST_VIDEO_INFO_UNSET_AFBC(i) \
  GST_VIDEO_INFO_FLAG_UNSET (i, GST_VIDEO_FLAG_ARM_AFBC)
#define GST_VIDEO_INFO_IS_AFBC(i) \
  GST_VIDEO_INFO_FLAG_IS_SET (i, GST_VIDEO_FLAG_ARM_AFBC)
#endif

#ifndef HAVE_NV12_10LE40
/* HACK: Support fake format for nv12_10le40 */
#define GST_VIDEO_FORMAT_NV12_10LE40 GST_VIDEO_FORMAT_NV12_10LE32
#endif

GstVideoFormat gst_video_format_from_drm (guint32 drmfmt);
guint32        gst_drm_format_from_video (GstVideoFormat fmt);
guint32        gst_drm_bpp_from_drm (guint32 drmfmt);
guint32        gst_drm_height_from_drm (guint32 drmfmt, guint32 height);
GstCaps *      gst_kms_sink_caps_template_fill (void);
void           gst_video_calculate_device_ratio (guint dev_width,
						 guint dev_height,
						 guint dev_width_mm,
						 guint dev_height_mm,
						 guint * dpy_par_n,
						 guint * dpy_par_d);

G_END_DECLS

#endif
