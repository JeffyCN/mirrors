#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstkmssink.h"
#include "gstvpudec.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "vpudec", GST_RANK_PRIMARY + 1,
          GST_TYPE_VPUDEC))
    return FALSE;

  if (!gst_element_register (plugin, "kmssink", GST_RANK_SECONDARY,
          GST_TYPE_KMS_SINK))
    return FALSE;

  return TRUE;
}


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vpu,
    "VPU decoder",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
