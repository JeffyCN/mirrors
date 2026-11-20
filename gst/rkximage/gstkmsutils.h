/* GStreamer
 *
 * Copyright (C) 2016 Igalia
 * Copyright (C) <2024> Rockchip Electronics Co., Ltd
 *
 * Based on kmssink
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

#ifndef DRM_FORMAT_NV20
#define DRM_FORMAT_NV20 fourcc_code('N', 'V', '2', '0')
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

#ifndef AFBC_FORMAT_MOD_BLOCK_SIZE_32x8
#define AFBC_FORMAT_MOD_BLOCK_SIZE_32x8 (2ULL)
#endif

#ifndef AFBC_FORMAT_MOD_SPARSE
#define AFBC_FORMAT_MOD_SPARSE (((__u64)1) << 6)
#endif

#define DRM_AFBC_MODIFIER \
  (DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_SPARSE) | \
   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16))

#define DRM_AFBC_MODIFIER_32x8 \
  (DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_SPARSE) | \
   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_32x8))

/* TRUE for the plain ARM AFBC superblock|SPARSE geometries the
 * Rockchip decoders emit: 16x16|SPARSE on RK3588, 32x8|SPARSE on
 * RK3572, where a single hardcoded constant silently disabled AFBC */
gboolean       gst_kms_utils_is_afbc_modifier (guint64 modifier);

#ifndef GST_VIDEO_FLAG_ARM_AFBC
#define GST_VIDEO_FLAG_ARM_AFBC (1UL << 31)
#define GST_VIDEO_INFO_SET_AFBC(i) \
  GST_VIDEO_INFO_FLAG_SET (i, GST_VIDEO_FLAG_ARM_AFBC)
#define GST_VIDEO_INFO_UNSET_AFBC(i) \
  GST_VIDEO_INFO_FLAG_UNSET (i, GST_VIDEO_FLAG_ARM_AFBC)
#define GST_VIDEO_INFO_IS_AFBC(i) \
  GST_VIDEO_INFO_FLAG_IS_SET (i, GST_VIDEO_FLAG_ARM_AFBC)
#endif

#ifndef DRM_FORMAT_MOD_VENDOR_ROCKCHIP
#define DRM_FORMAT_MOD_VENDOR_ROCKCHIP 0x0b
#endif

#ifndef DRM_FORMAT_MOD_ROCKCHIP_TYPE_SHIFT
#define DRM_FORMAT_MOD_ROCKCHIP_TYPE_SHIFT  52
#endif

#ifndef DRM_FORMAT_MOD_ROCKCHIP_TYPE_RFBC
#define DRM_FORMAT_MOD_ROCKCHIP_TYPE_RFBC 0x1
#endif

#ifndef ROCKCHIP_RFBC_BLOCK_SIZE_64x4
#define ROCKCHIP_RFBC_BLOCK_SIZE_64x4   (1ULL)
#endif

#ifndef DRM_FORMAT_MOD_ROCKCHIP_CODE
#define DRM_FORMAT_MOD_ROCKCHIP_CODE(__type, __val) \
  fourcc_mod_code(ROCKCHIP, ((__u64)(__type) << DRM_FORMAT_MOD_ROCKCHIP_TYPE_SHIFT) | \
      ((__val) & 0x000fffffffffffffULL))
#endif

#ifndef DRM_FORMAT_MOD_ROCKCHIP_RFBC
#define DRM_FORMAT_MOD_ROCKCHIP_RFBC(mode) \
  DRM_FORMAT_MOD_ROCKCHIP_CODE(DRM_FORMAT_MOD_ROCKCHIP_TYPE_RFBC, mode)
#endif

#define DRM_RFBC_MODIFIER \
  DRM_FORMAT_MOD_ROCKCHIP_RFBC(ROCKCHIP_RFBC_BLOCK_SIZE_64x4)

#ifndef GST_VIDEO_FLAG_RFBC
#define GST_VIDEO_FLAG_RFBC (1UL << 30)
#define GST_VIDEO_INFO_SET_RFBC(i) \
  GST_VIDEO_INFO_FLAG_SET (i, GST_VIDEO_FLAG_RFBC)
#define GST_VIDEO_INFO_UNSET_RFBC(i) \
  GST_VIDEO_INFO_FLAG_UNSET (i, GST_VIDEO_FLAG_RFBC)
#define GST_VIDEO_INFO_IS_RFBC(i) \
  GST_VIDEO_INFO_FLAG_IS_SET (i, GST_VIDEO_FLAG_RFBC)
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
