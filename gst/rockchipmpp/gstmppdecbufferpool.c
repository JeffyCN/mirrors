/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *     Author: Randy Li <randy.li@rock-chips.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "gstmppdecbufferpool.h"

GST_DEBUG_CATEGORY_STATIC (mppdecbufferpool_debug);
#define GST_CAT_DEFAULT mppdecbufferpool_debug

/*
 * GstMppDecBufferPool:
 */
#define parent_class gst_mpp_dec_buffer_pool_parent_class
G_DEFINE_TYPE (GstMppDecBufferPool, gst_mpp_dec_buffer_pool,
    GST_TYPE_BUFFER_POOL);

static gboolean
gst_mpp_dec_is_buffer_valid (GstBuffer * buffer, GstMppMemory ** out_mem)
{
  GstMemory *mem = gst_buffer_peek_memory (buffer, 0);
  gboolean valid = FALSE;

  if (gst_is_dmabuf_memory (mem))
    mem = gst_mini_object_get_qdata (GST_MINI_OBJECT (mem),
        GST_MPP_MEMORY_QUARK);
  else
    goto done;

  if (mem && gst_is_mpp_memory (mem)) {
    *out_mem = (GstMppMemory *) mem;
    valid = TRUE;
  }

done:
  return valid;
}

static gboolean
gst_mpp_dec_buffer_pool_start (GstBufferPool * bpool)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (bpool);
  GstBufferPoolClass *pclass = GST_BUFFER_POOL_CLASS (parent_class);
  GstStructure *config;
  GstCaps *caps;
  guint size, min_buffers, max_buffers, count;

  GST_DEBUG_OBJECT (pool, "start pool %p", pool);

  config = gst_buffer_pool_get_config (bpool);
  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  count = gst_mpp_allocator_start (pool->vallocator, size, min_buffers);
  if (count < min_buffers)
    goto no_buffers;

  pool->dec->mpi->control (pool->dec->mpp_ctx, MPP_DEC_SET_EXT_BUF_GROUP,
      (gpointer) pool->vallocator->mpp_mem_pool);

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
gst_mpp_dec_buffer_pool_stop (GstBufferPool * bpool)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (bpool);
  GstBufferPoolClass *pclass = GST_BUFFER_POOL_CLASS (parent_class);
  gboolean ret;
  guint n;

  GST_DEBUG_OBJECT (pool, "stop pool %p", pool);

  /* free the remaining buffers */
  for (n = 0; n < VIDEO_MAX_FRAME; n++) {
    if (pool->buffers[n]) {
      GstBuffer *buffer = pool->buffers[n];

      pool->buffers[n] = NULL;
      pclass->release_buffer (bpool, buffer);

      g_atomic_int_add (&pool->num_queued, -1);
    }
  }
  /* free the buffers in the queue */
  ret = pclass->stop (bpool);

  if (ret && pool->vallocator) {
    gint vret;
    vret = gst_mpp_allocator_stop (pool->vallocator);

    ret = (vret == 0);
  }

  return ret;
}

static GstFlowReturn
gst_mpp_dec_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (bpool);
  GstMppVideoDec *dec = pool->dec;
  GstMemory *mem;
  GstBuffer *newbuf = NULL;
  GstVideoInfo *info;

  info = &dec->info;

  mem = gst_mpp_allocator_alloc_dmabuf (pool->vallocator, pool->allocator);
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
gst_mpp_dec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (bpool);
  GstMppMemory *mem = NULL;
  gint index = -1;

  if (!gst_mpp_dec_is_buffer_valid (buffer, &mem))
    GST_ERROR_OBJECT (pool, "can't release an invalid buffer");

  index = mpp_buffer_get_index (mem->mpp_buf);

  if (pool->buffers[index] != NULL) {
    goto already_queued;
  } else {
    /* Release the internal refcount in mpp */
    mpp_buffer_put (mem->mpp_buf);
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
gst_mpp_dec_buffer_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (bpool);
  GstMppVideoDec *dec = pool->dec;
  GstBuffer *outbuf;
  MppFrame mframe = NULL;
  MppBuffer mpp_buf;
  gint buf_index, ret, mode;

  ret = dec->mpi->decode_get_frame (dec->mpp_ctx, &mframe);
  if (ret || NULL == mframe)
    goto mpp_error;

  if (mpp_frame_get_discard (mframe) || mpp_frame_get_errinfo (mframe))
    goto drop_frame;
  /* get from the pool the GstBuffer associated with the index */
  mpp_buf = mpp_frame_get_buffer (mframe);
  if (NULL == mpp_buf)
    goto mpp_eos;
  buf_index = mpp_buffer_get_index (mpp_buf);
  outbuf = pool->buffers[buf_index];
  if (outbuf == NULL)
    goto no_buffer;

  mode = mpp_frame_get_mode (mframe);
  switch (mode & MPP_FRAME_FLAG_FIELD_ORDER_MASK) {
    case MPP_FRAME_FLAG_BOT_FIRST:
      GST_BUFFER_FLAG_SET (outbuf, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_UNSET (outbuf, GST_VIDEO_BUFFER_FLAG_TFF);
      break;
    case MPP_FRAME_FLAG_TOP_FIRST:
      GST_BUFFER_FLAG_SET (outbuf, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_SET (outbuf, GST_VIDEO_BUFFER_FLAG_TFF);
      break;
    case MPP_FRAME_FLAG_DEINTERLACED:
    default:
      GST_BUFFER_FLAG_UNSET (outbuf, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_UNSET (outbuf, GST_VIDEO_BUFFER_FLAG_TFF);
      break;
  }
  /*
   * Increase the reference of the buffer or the destroy the mpp frame
   * would decrease the reference and put it back to unused status
   */
  mpp_buffer_inc_ref (mpp_buf);
  mpp_frame_deinit (&mframe);

  pool->buffers[buf_index] = NULL;
  g_atomic_int_add (&pool->num_queued, -1);

  GST_DEBUG_OBJECT (pool,
      "acquired buffer %p, index %d, queued %d", outbuf, buf_index,
      g_atomic_int_get (&pool->num_queued));

  *buffer = outbuf;

  return GST_FLOW_OK;

  /* ERRORS */
mpp_eos:
  {
    GST_INFO_OBJECT (pool, "got eos or %d", ret);
    return GST_FLOW_EOS;
  }
mpp_error:
  {
    *buffer = NULL;
    GST_ERROR_OBJECT (pool, "mpp error %d", ret);
    return GST_FLOW_ERROR;
  }
no_buffer:
  {
    *buffer = NULL;
    GST_ERROR_OBJECT (pool, "No free buffer found in the pool at index %d",
        buf_index);
    return GST_FLOW_ERROR;
  }
drop_frame:
  {
    mpp_frame_deinit (&mframe);
    return GST_FLOW_CUSTOM_ERROR_1;
  }
}

static gboolean
gst_mpp_dec_buffer_pool_set_config (GstBufferPool * bpool,
    GstStructure * config)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (bpool);
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
gst_mpp_dec_buffer_pool_dispose (GObject * object)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (object);

  if (pool->vallocator)
    gst_object_unref (pool->vallocator);
  pool->vallocator = NULL;

  if (pool->allocator)
    gst_object_unref (pool->allocator);
  pool->allocator = NULL;

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_mpp_dec_buffer_pool_finalize (GObject * object)
{
  GstMppDecBufferPool *pool = GST_MPP_DEC_BUFFER_POOL (object);

  pool->dec->mpi->control (pool->dec->mpp_ctx, MPP_DEC_SET_EXT_BUF_GROUP, NULL);
  gst_object_unref (pool->dec);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_mpp_dec_buffer_pool_init (GstMppDecBufferPool * pool)
{
  pool->dec = NULL;
  pool->num_queued = 0;
}

static void
gst_mpp_dec_buffer_pool_class_init (GstMppDecBufferPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->dispose = gst_mpp_dec_buffer_pool_dispose;
  object_class->finalize = gst_mpp_dec_buffer_pool_finalize;

  bufferpool_class->start = gst_mpp_dec_buffer_pool_start;
  bufferpool_class->stop = gst_mpp_dec_buffer_pool_stop;
  bufferpool_class->set_config = gst_mpp_dec_buffer_pool_set_config;
  bufferpool_class->alloc_buffer = gst_mpp_dec_buffer_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_mpp_dec_buffer_pool_acquire_buffer;
  bufferpool_class->release_buffer = gst_mpp_dec_buffer_pool_release_buffer;


  GST_DEBUG_CATEGORY_INIT (mppdecbufferpool_debug, "mppdecbufferpool", 0,
      "mpp decoder bufferpool");
}

/**
 * gst_mpp_dec_buffer_pool_new:
 * @dec: the mpp decoder owning the pool
 * @max: maximum buffers in the pool
 * @size: size of the buffer
 *
 * Construct a new buffer pool.
 *
 * Returns: the new pool, use gst_object_unref() to free resources
 */
GstBufferPool *
gst_mpp_dec_buffer_pool_new (GstMppVideoDec * dec, GstCaps * caps)
{
  GstMppDecBufferPool *pool;
  GstStructure *config;
  gchar *name, *parent_name;

  /* setting a significant unique name */
  parent_name = gst_object_get_name (GST_OBJECT (dec));
  name = g_strconcat (parent_name, ":", "pool:", "src", NULL);
  g_free (parent_name);

  pool =
      (GstMppDecBufferPool *) g_object_new (GST_TYPE_MPP_DEC_BUFFER_POOL,
      "name", name, NULL);
  g_object_ref_sink (pool);
  g_free (name);

  /* take a reference on decoder to be sure that it will be released
   * after the pool */
  pool->dec = gst_object_ref (dec);
  pool->vallocator = gst_mpp_allocator_new (GST_OBJECT (pool));
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
    GST_ERROR_OBJECT (pool, "Failed to create mpp allocator");
    gst_object_unref (pool);
    return NULL;
  }
}
