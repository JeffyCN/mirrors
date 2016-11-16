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
#ifndef __GST_VPU_META_H__
#define __GST_VPU_META_H__

#include <gst/gst.h>
#if 0
#include <rockchip/vpu_api.h>
#else
#include "vpu_api.h"
#endif

G_BEGIN_DECLS

typedef struct _GstVpuDecMeta GstVpuDecMeta;

G_GNUC_INTERNAL
GType gst_vpudec_meta_api_get_type (void);

#define GST_VPUDEC_META_INFO_API_TYPE gst_vpudec_meta_api_get_type()
G_GNUC_INTERNAL
const GstMetaInfo *gst_vpudec_meta_get_info (void);

#define GST_VPUDEC_META_INFO gst_vpudec_meta_get_info()

#define GST_VPUDEC_META_GET(buf) \
  ((GstVpuDecMeta *)gst_buffer_get_meta(buf, GST_VPUDEC_META_INFO_API_TYPE))
#define GST_VPUDEC_META_ADD(buf, index) \
  ((GstVpuDecMeta *)gst_buffer_add_meta \
    (buf, GST_VPUDEC_META_INFO, index))

G_GNUC_INTERNAL
guint gst_vpudec_meta_get_index (GstVpuDecMeta *meta);

G_GNUC_INTERNAL
gint gst_vpudec_meta_get_fd (GstVpuDecMeta *meta);

G_GNUC_INTERNAL
gpointer gst_vpudec_meta_get_vpumem (GstVpuDecMeta *meta);

G_GNUC_INTERNAL
gpointer gst_vpudec_meta_get_mem (GstVpuDecMeta *meta);

G_GNUC_INTERNAL
gint gst_vpudec_meta_get_size (GstVpuDecMeta *meta);

G_GNUC_INTERNAL
gpointer gst_vpudec_meta_get_offset (GstVpuDecMeta *meta);

G_GNUC_INTERNAL
gboolean gst_vpudec_meta_alloc_mem
(vpu_display_mem_pool *vpool, GstVpuDecMeta *meta, gint size);

G_GNUC_INTERNAL
GstVpuDecMeta *
gst_buffer_get_vpudec_meta (GstBuffer * buffer);

G_END_DECLS
#endif /*__GST_VPU_META_H__ */
