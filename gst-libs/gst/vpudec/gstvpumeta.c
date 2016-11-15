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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "gstvpumeta.h"

#define GST_VPU_DEC_META(obj) \
  ((GstVpuDecMeta *) (obj))
#define GST_VPU_IS_DEC_META(obj) \
  (GST_VPU_DEC_META (obj) != NULL)

struct _GstVpuDecMeta
{
  GstMeta meta;
  gpointer mem;
  guint index;
  gint size;
  gint dmabuf_fd;
  VPUMemLinear_t vpumem;
};

GType
gst_vpudec_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstVpuDecMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

static gboolean
gst_vpudec_meta_init (GstMeta * meta, gpointer params, GstBuffer * buffer)
{
  GstVpuDecMeta *emeta = (GstVpuDecMeta *) meta;

  emeta->mem = NULL;
  emeta->index = *(gint *) params;
  emeta->size = 0;
  emeta->dmabuf_fd = -1;
  memset (&emeta->vpumem, 0, sizeof (VPUMemLinear_t));

  return TRUE;
}

static void
gst_vpudec_meta_free (GstMeta * meta, GstBuffer * buffer)
{
  GstVpuDecMeta *emeta = (GstVpuDecMeta *) meta;

  VPUFreeLinear (&emeta->vpumem);
  emeta->mem = NULL;
  emeta->index = 0;
  emeta->size = 0;
  emeta->dmabuf_fd = -1;
}

#define GST_VPUDEC_META_INFO gst_vpudec_meta_get_info()
const GstMetaInfo *
gst_vpudec_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_vpudec_meta_api_get_type (), "GstVpuDecMeta",
        sizeof (GstVpuDecMeta), (GstMetaInitFunction) gst_vpudec_meta_init,
        (GstMetaFreeFunction) gst_vpudec_meta_free,
        (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

guint
gst_vpudec_meta_get_index (GstVpuDecMeta * meta)
{
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), 0);

  return meta->index;
}

gint
gst_vpudec_meta_get_fd (GstVpuDecMeta * meta)
{
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), 0);

  return meta->dmabuf_fd;
}

gpointer
gst_vpudec_meta_get_mem (GstVpuDecMeta * meta)
{
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), NULL);

  return meta->mem;
}

gpointer
gst_vpudec_meta_get_vpumem (GstVpuDecMeta * meta)
{
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), NULL);

  return &meta->vpumem;
}

gint
gst_vpudec_meta_get_size (GstVpuDecMeta * meta)
{
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), 0);

  return meta->size;
}

gpointer
gst_vpudec_meta_get_offset (GstVpuDecMeta * meta)
{
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), NULL);

  return meta->vpumem.offset;
}

gboolean
    gst_vpudec_meta_alloc_mem
    (vpu_display_mem_pool * vpool, GstVpuDecMeta * meta, gint size) {
  gint ret;
  g_return_val_if_fail (GST_VPU_IS_DEC_META (meta), FALSE);

  meta->size = size;
  /*
   * create vpumem from libvpu
   * includes mvc data
   */
  VPUMallocLinearOutside (&meta->vpumem, meta->size);
  meta->vpumem.index = meta->index;

  ret = vpool->commit_vpu (vpool, &meta->vpumem);
  meta->mem = (gpointer) meta->vpumem.vir_addr;
  meta->dmabuf_fd = VPUMemGetFD (&meta->vpumem);

  if (ret > 0)
    return TRUE;
  else
    return FALSE;
}
