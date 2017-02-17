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

#include "gstvpuallocator.h"
#include "gstvpudecbufferpool.h"

GST_DEBUG_CATEGORY_STATIC (vpubufferpool_debug);
#define GST_CAT_DEFAULT vpubufferpool_debug

/*
 * GstVpuDecBufferPool:
 */
#define gst_vpudec_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (GstVpuDecBufferPool, gst_vpudec_buffer_pool,
    GST_TYPE_BUFFER_POOL);

static void gst_vpudec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer);

static gboolean
gst_vpudec_is_buffer_valid (GstBuffer * buffer, GstVpuMemory ** out_mem)
{
  GstMemory *mem = gst_buffer_peek_memory (buffer, 0);
  gboolean valid = FALSE;

  if (gst_is_dmabuf_memory (mem))
    mem = gst_mini_object_get_qdata (GST_MINI_OBJECT (mem),
        GST_VPU_MEMORY_QUARK);
  else
    goto done;

  if (mem && gst_is_vpu_memory (mem)) {
    *out_mem = (GstVpuMemory *) mem;
    valid = TRUE;
  }

done:
  return valid;
}

static gboolean
gst_vpudec_buffer_pool_start (GstBufferPool * bpool)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstBufferPoolClass *pclass = GST_BUFFER_POOL_CLASS (parent_class);
  GstStructure *config;
  GstCaps *caps;
  guint size, min_buffers, max_buffers, count;
  VpuCodecContext_t *vpu_codec_ctx;

  GST_DEBUG_OBJECT (pool, "start pool %p", pool);

  config = gst_buffer_pool_get_config (bpool);
  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  vpu_codec_ctx = pool->dec->vpu_codec_ctx;

  count = gst_vpu_allocator_start (pool->vallocator, vpu_codec_ctx,
      size, min_buffers);

  if (count < min_buffers)
    goto no_buffers;

  pool->size = size;

  if (max_buffers != 0 && max_buffers < min_buffers)
    max_buffers = min_buffers;

  gst_buffer_pool_config_set_params (config, caps, size, min_buffers,
      max_buffers);
  pclass->set_config (bpool, config);
  gst_structure_free (config);

  /* allocate the buffers */
  if (!pclass->start (bpool))
    goto start_failed;

  return TRUE;

  /* ERRORS */
wrong_config:
  {
    GST_ERROR_OBJECT (pool, "invalid config %" GST_PTR_FORMAT, config);
    return FALSE;
  }
no_buffers:
  {
    GST_ERROR_OBJECT (pool,
        "we received %d buffer, we want at least %d", count, min_buffers);
    gst_structure_free (config);
    return FALSE;
  }
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
  GstBufferPoolClass *pclass = GST_BUFFER_POOL_CLASS (parent_class);
  guint n;

  GST_DEBUG_OBJECT (pool, "stop pool %p", pool);

  /* free the remaining buffers */
  for (n = 0; n < VIDEO_MAX_FRAME; n++) {
    if (pool->buffers[n]) {
      GstBuffer *buffer = pool->buffers[n];

      pool->buffers[n] = NULL;
      gst_buffer_unref (buffer);

      g_atomic_int_add (&pool->num_queued, -1);
    }
  }
  /* free the buffers in the queue */
  ret = pclass->stop (bpool);

  if (ret && pool->vallocator) {
    gint vret;
    vret = gst_vpu_allocator_stop (pool->vallocator);

    ret = (vret == 0);
  }

  return ret;
}

static GstFlowReturn
gst_vpudec_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstVpuDec *dec = pool->dec;
  GstMemory *mem;
  GstBuffer *newbuf = NULL;
  GstVideoInfo *info;

  info = &dec->info;

  mem = gst_vpu_allocator_alloc_dmabuf (pool->vallocator, pool->allocator);
  if (mem != NULL) {
    newbuf = gst_buffer_new ();
    gst_buffer_append_memory (newbuf, mem);
  } else {
    goto allocation_failed;
  }

  gst_buffer_add_video_meta_full (newbuf, GST_VIDEO_FRAME_FLAG_NONE,
      GST_VIDEO_INFO_FORMAT (info), GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info), GST_VIDEO_INFO_N_PLANES (info),
      info->offset, info->stride);

  *buffer = newbuf;

  return GST_FLOW_OK;

  /* ERRORS */
allocation_failed:
  {
    GST_ERROR_OBJECT (pool, "failed to allocate buffer");
    return GST_FLOW_ERROR;
  }
}

static void
gst_vpudec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstVpuMemory *mem = NULL;
  gint index = -1;

  if (!gst_vpudec_is_buffer_valid (buffer, &mem)) {
    GST_ERROR_OBJECT (pool, "can't release an invalid buffer");
  }

  index = mem->vpu_mem->index;

  if (pool->buffers[index] != NULL) {
    goto already_queued;
  } else {
    /* Release the internal refcount in mpp */
    VPUFreeLinear (mem->vpu_mem);
    pool->buffers[index] = buffer;
    g_atomic_int_add (&pool->num_queued, 1);
  }

  GST_DEBUG_OBJECT (pool,
      "released buffer %p, index %d, queued %d", buffer, index,
      g_atomic_int_get (&pool->num_queued));

  return;
  /* ERRORS */
already_queued:
  {
    GST_WARNING_OBJECT (pool, "the buffer was already released");
    return;
  }
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
  VPU_FRAME vpu_frame;
  VPUMemLinear_t vpu_mem;
  VPU_API_ERR ret;
  gint buf_index;

  memset (&dec_out, 0, sizeof (DecoderOut_t));
  memset (&vpu_frame, 0, sizeof (VPU_FRAME));
  dec_out.data = (guint8 *) & vpu_frame;
  ctx = (VpuCodecContext_t *) dec->vpu_codec_ctx;

  if ((ret = ctx->decode_getframe (ctx, &dec_out)) < 0)
    goto vpu_error;
  vpu_mem = vpu_frame.vpumem;
  /* Drop the invalid buffer */
  if (vpu_frame.ErrorInfo) {
    VPUFreeLinear (&vpu_mem);
    return GST_FLOW_CUSTOM_ERROR_1;
  }

  /* get from the pool the GstBuffer associated with the index */
  buf_index = vpu_mem.index;
  outbuf = pool->buffers[buf_index];
  if (outbuf == NULL)
    goto no_buffer;

  pool->buffers[buf_index] = NULL;
  g_atomic_int_add (&pool->num_queued, -1);

  GST_DEBUG_OBJECT (pool,
      "acquired buffer %p (%p) , index %d, queued %d data %p", outbuf,
      (gpointer) vpu_mem.vir_addr, buf_index,
      g_atomic_int_get (&pool->num_queued), vpu_mem.offset);

  *buffer = outbuf;

  return GST_FLOW_OK;

  /* ERRORS */
vpu_error:
  {
    switch (ret) {
      case VPU_API_EOS_STREAM_REACHED:
        GST_DEBUG_OBJECT (pool, "eos reached at libvpu size %d data %p",
            dec_out.size, dec_out.data);
        return GST_FLOW_EOS;
      default:
        return GST_FLOW_ERROR;
    }
  }
no_buffer:
  {
    GST_ERROR_OBJECT (pool, "No free buffer found in the pool at index %d",
        buf_index);
    return GST_FLOW_ERROR;
  }
}

static gboolean
gst_vpudec_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (bpool);
  GstCaps *caps;
  guint size, min_buffers, max_buffers;
  gboolean updated = FALSE;
  gboolean ret;

  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  GST_DEBUG_OBJECT (pool, "config %" GST_PTR_FORMAT, config);

  if (pool->allocator)
    gst_object_unref (pool->allocator);

  pool->allocator = gst_dmabuf_allocator_new ();

  if (max_buffers > VIDEO_MAX_FRAME || max_buffers == 0) {
    updated = TRUE;
    max_buffers = VIDEO_MAX_FRAME;
    GST_INFO_OBJECT (pool, "reducing maximum buffers to %u", max_buffers);
  }

  gst_buffer_pool_config_add_option (config, GST_BUFFER_POOL_OPTION_VIDEO_META);

  gst_buffer_pool_config_set_params (config, caps, size, min_buffers,
      max_buffers);

  ret = GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);

  return !updated && ret;

  /* ERROR */
wrong_config:
  {
    GST_ERROR_OBJECT (pool, "invalid config %" GST_PTR_FORMAT, config);
    return FALSE;
  }
}

static void
gst_vpudec_buffer_pool_dispose (GObject * object)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (object);

  if (pool->vallocator)
    gst_object_unref (pool->vallocator);
  pool->vallocator = NULL;

  if (pool->allocator)
    gst_object_unref (pool->allocator);
  pool->allocator = NULL;

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_vpudec_buffer_pool_finalize (GObject * object)
{
  GstVpuDecBufferPool *pool = GST_VPUDEC_BUFFER_POOL (object);

  gst_object_unref (pool->dec);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_vpudec_buffer_pool_init (GstVpuDecBufferPool * pool)
{
  pool->dec = NULL;
  pool->num_queued = 0;
}

static void
gst_vpudec_buffer_pool_class_init (GstVpuDecBufferPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->dispose = gst_vpudec_buffer_pool_dispose;
  object_class->finalize = gst_vpudec_buffer_pool_finalize;

  bufferpool_class->start = gst_vpudec_buffer_pool_start;
  bufferpool_class->stop = gst_vpudec_buffer_pool_stop;
  bufferpool_class->set_config = gst_vpudec_buffer_pool_set_config;
  bufferpool_class->alloc_buffer = gst_vpudec_buffer_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_vpudec_buffer_pool_acquire_buffer;
  bufferpool_class->release_buffer = gst_vpudec_buffer_pool_release_buffer;


  GST_DEBUG_CATEGORY_INIT (vpubufferpool_debug, "vpubufferpool", 0,
      "vpu bufferpool");
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
gst_vpudec_buffer_pool_new (GstVpuDec * dec, GstCaps * caps)
{
  GstVpuDecBufferPool *pool;
  GstStructure *config;
  gchar *name, *parent_name;

  /* setting a significant unique name */
  parent_name = gst_object_get_name (GST_OBJECT (dec));
  name = g_strconcat (parent_name, ":", "pool:", "src", NULL);
  g_free (parent_name);

  pool =
      (GstVpuDecBufferPool *) g_object_new (GST_TYPE_VPUDEC_BUFFER_POOL,
      "name", name, NULL);
  g_free (name);
  /* take a reference on vpudec to be sure that it will be released
   * after the pool */

  pool->dec = gst_object_ref (dec);
  pool->vallocator = gst_vpu_allocator_new (GST_OBJECT (pool));
  if (!pool->vallocator)
    goto allocator_failed;

  config = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));
  gst_buffer_pool_config_set_params (config, caps, dec->info.size, 0, 0);
  /* This will simply set a default config, but will not configure the pool
   * because min and max are not valid */
  gst_buffer_pool_set_config (GST_BUFFER_POOL_CAST (pool), config);

  return GST_BUFFER_POOL (pool);
  /* ERROR */
allocator_failed:
  {
    GST_ERROR_OBJECT (pool, "Failed to create vpu allocator");
    gst_object_unref (pool);
    return NULL;
  }
}
