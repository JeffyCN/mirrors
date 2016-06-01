/*
 * Copyright 2016 Rockchip Electronics S.LSI Co. LTD
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

#include "gstvpudecbufferpool.h"

GST_DEBUG_CATEGORY_EXTERN (gst_vpudec_debug);
#define GST_CAT_DEFAULT gst_vpudec_debug


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

const GstMetaInfo *
gst_vpudec_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_vpudec_meta_api_get_type (), "GstVpuDecMeta",
        sizeof (GstVpuDecMeta), (GstMetaInitFunction) NULL,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

/*
 * GstVpuDecBufferPool:
 */
#define gst_vpudec_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (GstVpuDecBufferPool, gst_vpudec_buffer_pool,
    GST_TYPE_BUFFER_POOL);

static void gst_vpudec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer);

static void
gst_vpudec_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstVpuDecMeta *meta;

  meta = GST_VPUDEC_META_GET (buffer);
  g_assert (meta != NULL);

  GST_DEBUG_OBJECT (pool,
      "free buffer %p idx %d (data %p, len %u)", buffer,
      meta->vpumem.index, meta->mem, meta->size);

  VPUFreeLinear (&meta->vpumem);
  meta->mem = NULL;

  gst_buffer_unref (buffer);
}

static GstFlowReturn
gst_vpudec_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstVpuDec *dec = pool->dec;
  GstBuffer *newbuf;
  GstVpuDecMeta *meta;
  vpu_display_mem_pool *vpool;

  newbuf = gst_buffer_new ();
  meta = GST_VPUDEC_META_ADD (newbuf);
  meta->index = pool->nb_buffers_alloc;
  meta->size = pool->buf_alloc_size;

  memset (&meta->vpumem, 0, sizeof (VPUMemLinear_t));

  /* create vpumem from libvpu */
  VPUMallocLinearOutside (&meta->vpumem, meta->size);   /* excludes mvc data */
  meta->mem = (gpointer) meta->vpumem.phy_addr;
  meta->vpumem.index = meta->index;
  meta->dmabuf_fd = VPUMemGetFD (&meta->vpumem);

  /* commit vpumem to libvpu  */
  vpool = (vpu_display_mem_pool *) dec->vpu_mem_pool;
  if (vpool->commit_hdl (vpool, VPUMemGetFD (&meta->vpumem), meta->size,
          meta->index) < 0)
    GST_WARNING_OBJECT (pool, "mempool commit error");

  gst_buffer_append_memory (newbuf,
      gst_memory_new_wrapped (GST_MEMORY_FLAG_NO_SHARE,
          meta->mem, meta->size, 0, meta->size, meta->mem, NULL));

  /* TODO: add dmabuf allocator when RGA can accept dmabuf fd */

  GST_DEBUG_OBJECT (pool, "created buffer %u of size %d, %p (%p) for pool %p",
      meta->index, meta->size, newbuf, meta->mem, pool);

  *buffer = newbuf;
  pool->nb_buffers_alloc++;

  return GST_FLOW_OK;

  /* ERRORS */
}

static gboolean
gst_vpudec_buffer_pool_start (GstBufferPool * bpool)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);

  GST_DEBUG_OBJECT (pool, "start pool %p", pool);

  pool->buffers = g_new0 (GstBuffer *, pool->nb_buffers);
  pool->nb_buffers_alloc = 0;

  /* allocate the buffers */
  if (!GST_BUFFER_POOL_CLASS (parent_class)->start (bpool))
    goto start_failed;

  return TRUE;

  /* ERRORS */
start_failed:
  {
    GST_ERROR_OBJECT (pool, "failed to start pool %p", pool);
    return FALSE;
  }
}

static gboolean
gst_vpudec_buffer_pool_stop (GstBufferPool * bpool)
{
  gboolean ret;
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  guint n;

  GST_DEBUG_OBJECT (pool, "stop pool %p", pool);

  /* free the buffers in the queue */
  ret = GST_BUFFER_POOL_CLASS (parent_class)->stop (bpool);

  /* free the remaining buffers */
  for (n = 0; n < pool->nb_buffers; n++) {
    if (pool->buffers[n])
      gst_vpudec_buffer_pool_free_buffer (bpool, pool->buffers[n]);
  }
  g_free (pool->buffers);
  pool->buffers = NULL;

  pool->nb_queued = 0;
  pool->nb_buffers = 0;

  return ret;
}

static GstFlowReturn
gst_vpudec_buffer_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstVpuDec *dec = pool->dec;
  VpuCodecContext_t *ctx;
  GstBuffer *outbuf;
  DecoderOut_t dec_out;
  VPU_FRAME *vpu_frame;
  VPUMemLinear_t vpu_mem;
  GstVpuDecMeta *meta = NULL;
  VPU_API_ERR ret;

  memset (&dec_out, 0, sizeof (DecoderOut_t));
  dec_out.data = (guint8 *) g_malloc (sizeof (VPU_FRAME));
  ctx = (VpuCodecContext_t *) dec->ctx;

  if ((ret = ctx->decode_getframe_sync (ctx, &dec_out)) < 0)
    goto vpu_error;

  vpu_frame = (VPU_FRAME *) dec_out.data;
  vpu_mem = vpu_frame->vpumem;

  /* get from the pool the GstBuffer associated with the index */
  outbuf = pool->buffers[vpu_mem.index];
  if (outbuf == NULL)
    goto no_buffer;

  /* FIX ME: libvpu, the offset set in alloc function is not the same
     encapsulated in VPU_FRAME, manually copy to release the buffer */
  meta = GST_VPUDEC_META_GET (outbuf);
  g_assert (meta != NULL);
  meta->vpumem.offset = vpu_mem.offset;

  pool->buffers[vpu_mem.index] = NULL;
  pool->nb_queued--;

  GST_DEBUG_OBJECT (pool,
      "acquired buffer %p (%p) , index %d, queued %d", outbuf,
      (gpointer) vpu_mem.phy_addr, vpu_mem.index, pool->nb_queued);

  *buffer = outbuf;
  g_free (dec_out.data);

  return GST_FLOW_OK;

  /* ERRORS */
vpu_error:
  {
    switch (ret) {
      case VPU_API_EOS_STREAM_REACHED:
        GST_DEBUG_OBJECT (pool, "eos reached at libvpu");
        return GST_FLOW_EOS;
      default:
        return GST_FLOW_ERROR;
    }
  }
no_buffer:
  {
    GST_ERROR_OBJECT (pool, "No free buffer found in the pool at index %d",
        vpu_mem.index);
    return GST_FLOW_ERROR;
  }
}

static void
gst_vpudec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstVpuDecMeta *meta;

  meta = GST_VPUDEC_META_GET (buffer);
  g_assert (meta != NULL);
  if (meta == NULL) {
    GST_LOG_OBJECT (pool, "unref copied buffer %p", buffer);
    /* no meta, it was a copied buffer that we can unref */
    gst_buffer_unref (buffer);
    return;
  }

  if (pool->buffers[meta->index] != NULL)
    goto already_queued;

  /* put this in vpumem free list */
  VPUFreeLinear (&meta->vpumem);

  pool->buffers[meta->index] = buffer;
  pool->nb_queued++;

  GST_DEBUG_OBJECT (pool,
      "released buffer %p (%p), index %d, queued %d", buffer, meta->mem,
      meta->index, pool->nb_queued);

  return;

  /* ERRORS */
already_queued:
  {
    GST_WARNING_OBJECT (pool, "the buffer was already released");
    return;
  }
}

static void
gst_vpudec_buffer_pool_finalize (GObject * object)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (object);

  GST_DEBUG_OBJECT (pool, "finalize pool %p", pool);

  //g_free (pool->buffers);
  gst_object_unref (pool->dec);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_vpudec_buffer_pool_init (GstVpuDecBufferPool * pool)
{
}

static void
gst_vpudec_buffer_pool_class_init (GstVpuDecBufferPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_vpudec_buffer_pool_finalize;

  bufferpool_class->start = gst_vpudec_buffer_pool_start;
  bufferpool_class->stop = gst_vpudec_buffer_pool_stop;
  bufferpool_class->alloc_buffer = gst_vpudec_buffer_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_vpudec_buffer_pool_acquire_buffer;
  bufferpool_class->release_buffer = gst_vpudec_buffer_pool_release_buffer;
  bufferpool_class->free_buffer = gst_vpudec_buffer_pool_free_buffer;
}

/**
 * gst_vpudec_buffer_pool_new:
 * @dec: the vpu decoder owning the pool
 * @max: maximum buffers in the pool
 * @size: size of the buffer
 *
 * Construct a new buffer pool.
 *
 * Returns: the new pool, use gst_object_unref() to free resources
 */
GstBufferPool *
gst_vpudec_buffer_pool_new (GstVpuDec * dec, GstCaps * caps, guint max,
    guint size, GstVideoAlignment * align)
{
  GstVpuDecBufferPool *pool;
  GstStructure *s;

  GST_DEBUG_OBJECT (dec, "construct a new buffer pool (max buffers %u,"
      "buffer size %u)", max, size);

  pool =
      (GstVpuDecBufferPool *) g_object_new (GST_TYPE_VPUDEC_BUFFER_POOL, NULL);
  /* take a reference on vpudec to be sure that it will be released after the pool */

  pool->dec = gst_object_ref (dec);
  pool->dec = dec;
  pool->nb_buffers = max;
  pool->buf_alloc_size = size;

  s = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));
  gst_buffer_pool_config_set_params (s, caps, size, max, max);
  gst_buffer_pool_config_add_option (s, GST_BUFFER_POOL_OPTION_VIDEO_META);
  gst_buffer_pool_config_set_video_alignment (s, align);
  gst_buffer_pool_set_config (GST_BUFFER_POOL_CAST (pool), s);

  return GST_BUFFER_POOL (pool);
}
