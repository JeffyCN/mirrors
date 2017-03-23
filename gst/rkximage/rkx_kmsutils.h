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

#ifndef __RKX_KMS_UTILS_H__
#define __RKX_KMS_UTILS_H__

#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>

G_BEGIN_DECLS

GstVideoFormat rkx_video_format_from_drm (guint32 drmfmt);
guint32        rkx_drm_format_from_video (GstVideoFormat fmt);
GstCaps *      rkx_kms_sink_caps_template_fill (void);
void           rkx_video_calculate_device_ratio (guint dev_width,
						 guint dev_height,
						 guint dev_width_mm,
						 guint dev_height_mm,
						 guint * dpy_par_n,
						 guint * dpy_par_d);

G_END_DECLS

#endif
