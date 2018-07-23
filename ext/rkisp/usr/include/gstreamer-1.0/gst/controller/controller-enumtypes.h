


#ifndef __GST_CONTROLLER_ENUM_TYPES_H__
#define __GST_CONTROLLER_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* enumerations from "gstinterpolationcontrolsource.h" */
GType gst_interpolation_mode_get_type (void);
#define GST_TYPE_INTERPOLATION_MODE (gst_interpolation_mode_get_type())

/* enumerations from "gstlfocontrolsource.h" */
GType gst_lfo_waveform_get_type (void);
#define GST_TYPE_LFO_WAVEFORM (gst_lfo_waveform_get_type())
G_END_DECLS

#endif /* __GST_CONTROLLER_ENUM_TYPES_H__ */



