#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <linux/dma-heap.h>

#include "gstdmaheapallocator.h"

GST_DEBUG_CATEGORY (gst_dma_heap_allocator_debug);
#define GST_CAT_DEFAULT gst_dma_heap_allocator_debug

#define DEFAULT_HEAP_NAME "cma"

G_DEFINE_TYPE (GstDmaHeapAllocator, gst_dma_heap_allocator,
    GST_TYPE_DMABUF_ALLOCATOR);

static void
gst_dma_heap_allocator_finalize (GObject * object)
{
  GstDmaHeapAllocator *self = GST_DMA_HEAP_ALLOCATOR (object);

  g_free (self->heap_name);
  if (self->heap_fd >= 0)
    close (self->heap_fd);
  self->heap_fd = -1;

  G_OBJECT_CLASS (gst_dma_heap_allocator_parent_class)->finalize (object);
}

static GstMemory *
gst_dma_heap_allocator_alloc (GstAllocator * allocator, gsize size,
    GstAllocationParams * params)
{
  GstDmaHeapAllocator *self = GST_DMA_HEAP_ALLOCATOR (allocator);
  struct dma_heap_allocation_data data = { 0, };
  GstMemory *mem;
  gint fd;

  data.len = size;
  data.fd_flags = O_RDWR | O_CLOEXEC;

  if (ioctl (self->heap_fd, DMA_HEAP_IOCTL_ALLOC, &data)) {
    GST_ERROR_OBJECT (self, "dma_heap alloc of %" G_GSIZE_FORMAT
        " bytes failed: %s", size, g_strerror (errno));
    return NULL;
  }

  fd = (gint) data.fd;
  /* KEEP_MAPPED caches the userspace mapping for the lifetime of the
   * memory.  Without it every map/unmap pair re-issues mmap (and munmap
   * on release); for fault-mapped heaps such as CMA that also re-faults
   * every page of every frame, which costs milliseconds at 1080p.  The
   * per-map DMA_BUF_IOCTL_SYNC cache maintenance of the dmabuf allocator
   * is unaffected by this flag. */
  mem = gst_dmabuf_allocator_alloc_with_flags (GST_ALLOCATOR (self), fd,
      size, GST_FD_MEMORY_FLAG_KEEP_MAPPED);
  if (!mem) {
    close (fd);
    return NULL;
  }

  return mem;
}

static void
gst_dma_heap_allocator_class_init (GstDmaHeapAllocatorClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstAllocatorClass *allocator_class = GST_ALLOCATOR_CLASS (klass);

  gobject_class->finalize = gst_dma_heap_allocator_finalize;
  allocator_class->alloc = gst_dma_heap_allocator_alloc;
}

static void
gst_dma_heap_allocator_init (GstDmaHeapAllocator * self)
{
  GstAllocator *allocator = GST_ALLOCATOR (self);
  allocator->mem_type = GST_ALLOCATOR_DMABUF;
  self->heap_name = NULL;
  self->heap_fd = -1;
}

GstAllocator *
gst_dma_heap_allocator_new (const gchar * heap_name)
{
  GstDmaHeapAllocator *self;
  gchar *path;

  if (!heap_name || !*heap_name)
    heap_name = DEFAULT_HEAP_NAME;

  self = g_object_new (GST_TYPE_DMA_HEAP_ALLOCATOR, NULL);
  self->heap_name = g_strdup (heap_name);

  path = g_strdup_printf ("/dev/dma_heap/%s", heap_name);
  self->heap_fd = open (path, O_RDWR | O_CLOEXEC);
  if (self->heap_fd < 0) {
    GST_ERROR ("failed to open dma heap '%s': %s", path, g_strerror (errno));
    g_free (path);
    gst_object_unref (self);
    return NULL;
  }

  GST_DEBUG_CATEGORY_INIT (gst_dma_heap_allocator_debug,
      "dmaheapallocator", 0, "DMA-BUF heaps allocator");
  GST_INFO ("opened dma heap %s (fd %d)", path, self->heap_fd);
  g_free (path);

  return GST_ALLOCATOR (self);
}
