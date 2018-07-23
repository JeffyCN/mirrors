


#ifndef __GST_TAG_ENUM_TYPES_H__
#define __GST_TAG_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* enumerations from "tag.h" */
GType gst_tag_image_type_get_type (void);
#define GST_TYPE_TAG_IMAGE_TYPE (gst_tag_image_type_get_type())
GType gst_tag_license_flags_get_type (void);
#define GST_TYPE_TAG_LICENSE_FLAGS (gst_tag_license_flags_get_type())

/* enumerations from "gsttagdemux.h" */
GType gst_tag_demux_result_get_type (void);
#define GST_TYPE_TAG_DEMUX_RESULT (gst_tag_demux_result_get_type())
G_END_DECLS

#endif /* __GST_TAG_ENUM_TYPES_H__ */



