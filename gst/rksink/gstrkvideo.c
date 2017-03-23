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
#include "config.h"
#endif

#ifdef USE_X11
#include "rkximage/ximagesink.h"
#ifdef USE_EGL_X11
#include "eglgles/gstegladaptation.h"
#include "eglgles/gsteglglessink.h"
#endif
#endif

GST_DEBUG_CATEGORY (gst_debug_x_image_sink);
GST_DEBUG_CATEGORY (gst_eglglessink_debug);
GST_DEBUG_CATEGORY (CAT_PERFORMANCE);

static gboolean
plugin_init (GstPlugin * plugin)
{
#ifdef USE_X11
  if (!gst_element_register (plugin, "rkximagesink",
          GST_RANK_SECONDARY, GST_TYPE_X_IMAGE_SINK))
    return FALSE;

  if (!gst_element_register (plugin, "eglglessink",
          GST_RANK_SECONDARY, GST_TYPE_EGLGLESSINK))
    return FALSE;
#endif

#ifdef USE_X11
  GST_DEBUG_CATEGORY_INIT (gst_debug_x_image_sink, "rkximagesink", 0,
      "rkximagesink element");
#ifdef USE_EGL_X11
  GST_DEBUG_CATEGORY_INIT (gst_eglglessink_debug, "eglglessink",
      0, "Simple EGL/GLES Sink");
#endif

  gst_egl_adaption_init ();
#endif

  GST_DEBUG_CATEGORY_GET (CAT_PERFORMANCE, "GST_PERFORMANCE");

  return TRUE;
}


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rkvideo,
    "Rockchip Video Sink",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
