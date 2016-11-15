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
#include <rockchip/vpu_api.h>

G_BEGIN_DECLS

typedef struct _GstVpuDecMeta GstVpuDecMeta;

#define GST_VPUDEC_META_GET(buf) \
  ((GstVpuDecMeta *)gst_buffer_get_meta(buf, gst_vpudec_meta_api_get_type()))
#define GST_VPUDEC_META_ADD(buf, index) \
  ((GstVpuDecMeta *)gst_buffer_add_meta \
    (buf, gst_vpudec_meta_get_info(), index))

G_GNUC_INTERNAL
GType gst_vpudec_meta_api_get_type (void);

G_GNUC_INTERNAL
const GstMetaInfo *gst_vpudec_meta_get_info (void);

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

G_END_DECLS
#endif /*__GST_VPU_META_H__ */
