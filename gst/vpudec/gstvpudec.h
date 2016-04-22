#ifndef  GSTVPUDEC_H
#define  GSTVPUDEC_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideodecoder.h>

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_VPUDEC	(gst_vpudec_get_type())
#define GST_VPUDEC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VPUDEC, GstVPUDec))
#define GST_VPUDEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_VPUDEC, GstVPUDecClass))
#define GST_IS_VPUDEC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_VPUDEC))
#define GST_IS_VPUDEC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_VPUDEC))

typedef struct _GstVPUDec GstVPUDec;
typedef struct _GstVPUDecClass GstVPUDecClass;

struct _GstVPUDec {
  GstVideoDecoder parent;
  /* add private members here */
};

struct _GstVPUDecClass {
  GstVideoDecoderClass parent_class;
};

GType gst_vpudec_get_type(void);


G_END_DECLS

#endif /* __GST_vpudec_H__ */
