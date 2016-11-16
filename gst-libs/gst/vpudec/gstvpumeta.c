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

GST_DEBUG_CATEGORY_STATIC (gst_vpudec_meta_debug);
#define GST_CAT_DEFAULT gst_vpudec_meta_debug

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
  static GType g_type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&g_type)) {
    GType type = gst_meta_api_type_register ("GstVpuDecMetaAPI", tags);
    g_once_init_leave (&g_type, type);
  }
  return g_type;
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

  g_print ("meta try free %d: %x\n", emeta->index, *emeta->vpumem.offset);

  VPUFreeLinear (&emeta->vpumem);
  emeta->mem = NULL;
  emeta->index = 0;
  emeta->size = 0;
  emeta->dmabuf_fd = -1;
}

const GstMetaInfo *
gst_vpudec_meta_get_info (void)
{
  static gsize g_meta_info;

  if (g_once_init_enter (&g_meta_info)) {
    gsize meta_info =
        GPOINTER_TO_SIZE (gst_meta_register (gst_vpudec_meta_api_get_type (),
            "GstVpuDecMeta",
            sizeof (GstVpuDecMeta), (GstMetaInitFunction) gst_vpudec_meta_init,
            (GstMetaFreeFunction) gst_vpudec_meta_free,
            (GstMetaTransformFunction) NULL));
    g_once_init_leave (&g_meta_info, meta_info);
  }
  return GSIZE_TO_POINTER (g_meta_info);
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
gst_vpudec_meta_alloc_mem (vpu_display_mem_pool * vpool, GstVpuDecMeta * meta,
    gint size)
{
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

  if (ret > 0) {
    vpool->inc_used (vpool, &meta->vpumem);
    return TRUE;
  } else {
    return FALSE;
  }
}

GstVpuDecMeta *
gst_buffer_get_vpudec_meta (GstBuffer * buffer)
{
  GstVpuDecMeta *meta;
  GstMeta *m;

  m = gst_buffer_get_meta (buffer, GST_VPUDEC_META_INFO_API_TYPE);
  if (!m)
    return NULL;

  meta = (GstVpuDecMeta *) m;

  return meta;
}
