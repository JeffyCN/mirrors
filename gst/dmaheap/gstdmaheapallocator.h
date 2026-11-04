#ifndef __GST_DMA_HEAP_ALLOCATOR_H__
#define __GST_DMA_HEAP_ALLOCATOR_H__

#include <gst/gst.h>
#include <gst/allocators/gstdmabuf.h>

G_BEGIN_DECLS;

#define GST_TYPE_DMA_HEAP_ALLOCATOR (gst_dma_heap_allocator_get_type ())
#define GST_IS_DMA_HEAP_ALLOCATOR(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_DMA_HEAP_ALLOCATOR))
#define GST_DMA_HEAP_ALLOCATOR(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_DMA_HEAP_ALLOCATOR, GstDmaHeapAllocator))
#define GST_IS_DMA_HEAP_ALLOCATOR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_DMA_HEAP_ALLOCATOR))
#define GST_DMA_HEAP_ALLOCATOR_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_DMA_HEAP_ALLOCATOR, GstDmaHeapAllocatorClass))
#define GST_DMA_HEAP_ALLOCATOR_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_DMA_HEAP_ALLOCATOR, GstDmaHeapAllocatorClass))

typedef struct _GstDmaHeapAllocator GstDmaHeapAllocator;
typedef struct _GstDmaHeapAllocatorClass GstDmaHeapAllocatorClass;

struct _GstDmaHeapAllocator
{
  GstDmaBufAllocator parent;
  /*< private > */
  gchar *heap_name;
  gint heap_fd;
};

struct _GstDmaHeapAllocatorClass
{
  GstDmaBufAllocatorClass parent_class;
};

GType gst_dma_heap_allocator_get_type (void);

GstAllocator *gst_dma_heap_allocator_new (const gchar * heap_name);

G_END_DECLS;

#endif /* __GST_DMA_HEAP_ALLOCATOR_H__ */
