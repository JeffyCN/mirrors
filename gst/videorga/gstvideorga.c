/* GStreamer
 *  Copyright (C) 2016 Intel Corp
 *  Author: Sudip Jain <sudip.jain@intel.com>
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
 */

/**
 * SECTION:element-videorga
 *
 * Convert video frames between video formats.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v videotestsrc ! video/x-raw,format=\(string\)YUY2 ! videorga ! ximagesink
 * ]|
 *
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gstvideorga.h"

#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

GST_DEBUG_CATEGORY (videorga_debug);
#define GST_CAT_DEFAULT videorga_debug

GType gst_video_rga_get_type (void);

static GQuark _colorspace_quark;

#define RGA_DEVICE "/dev/rga"

#define gst_video_rga_parent_class parent_class
G_DEFINE_TYPE (GstVideoRga, gst_video_rga, GST_TYPE_VIDEO_FILTER);

enum
{
  PROP_0,
  PROP_ROTATION,
  PROP_MODE
};

#define GST_VIDEO_SRC_FORMATS "{ BGRx }"
#define GST_VIDEO_SINK_FORMATS "{ NV12, I420 } "

#define RGA_VIDEO_SRC_CAPS GST_VIDEO_CAPS_MAKE (GST_VIDEO_SRC_FORMATS) ";" \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES ("ANY", GST_VIDEO_SRC_FORMATS)

#define RGA_VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE (GST_VIDEO_SINK_FORMATS) ";" \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES ("ANY", GST_VIDEO_SINK_FORMATS)

static GstStaticPadTemplate gst_video_rga_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (RGA_VIDEO_SRC_CAPS)
    );

static GstStaticPadTemplate gst_video_rga_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (RGA_VIDEO_SINK_CAPS)
    );

static void gst_video_rga_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_video_rga_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);

#define GST_TYPE_VIDEO_RGA_ROTATE_METHOD (gst_video_rga_rotate_method_get_type())

static const GEnumValue video_rga_rotate_methods[] = {
  {GST_VIDEO_RGA_ROTATION_NONE, "(no rotation)", "none"},
  {GST_VIDEO_RGA_ROTATION_90R, "Rotate clockwise 90 degrees", "90R"},
  {GST_VIDEO_RGA_ROTATION_180, "Rotate 180 degrees", "180"},
  {GST_VIDEO_RGA_ROTATION_90L, "Rotate anti-clockwise 90 degrees", "90L"},
  {0, NULL, NULL},
};

static GType
gst_video_rga_rotate_method_get_type (void)
{
  static GType video_rga_rotate_method_type = 0;

  if (!video_rga_rotate_method_type) {
    video_rga_rotate_method_type =
        g_enum_register_static ("GstVideoRgaRotateMethod",
        video_rga_rotate_methods);
  }
  return video_rga_rotate_method_type;
}

#define GST_TYPE_VIDEO_RGA_MODE_METHOD (gst_video_rga_mode_method_get_type())

static const GEnumValue video_rga_mode_methods[] = {
  {GST_VIDEO_RGA_MODE_METHOD_MMAP, "uses hw memory", "mmap"},
  {GST_VIDEO_RGA_MODE_METHOD_DMABUF, "uses dmabuf handles", "dmabuf"},
  {GST_VIDEO_RGA_MODE_METHOD_USER, "for non-aligned component(s)", "user"},
  {GST_VIDEO_RGA_MODE_METHOD_CUSTOM, "custom memory such as ion allocator", "custom"},
  {0, NULL, NULL},
};

static GType
gst_video_rga_mode_method_get_type (void)
{
  static GType video_rga_mode_method_type = 0;

  if (!video_rga_mode_method_type) {
    video_rga_mode_method_type =
        g_enum_register_static ("GstVideoRgaModeMethod",
        video_rga_mode_methods);
  }
  return video_rga_mode_method_type;
}

static
    gboolean gst_video_rga_set_info (GstVideoFilter * filter,
    GstCaps * incaps, GstVideoInfo * in_info, GstCaps * outcaps,
    GstVideoInfo * out_info);
static GstFlowReturn gst_video_rga_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * in_frame, GstVideoFrame * out_frame);
static GstStateChangeReturn
gst_video_rga_change_state (GstElement * element, GstStateChange transition);

static GstCaps *
gst_video_rga_fixate_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * diffcaps)
{
  GstCaps *res;

  GST_DEBUG_OBJECT (trans, "trying to fixate other caps %" GST_PTR_FORMAT
      " based on caps %" GST_PTR_FORMAT, diffcaps, caps);

  res = gst_caps_intersect (diffcaps, caps);
  if (gst_caps_is_empty (res)) {
    gst_caps_unref (res);
    res = diffcaps;
  } else
      gst_caps_unref (diffcaps);

  res = gst_caps_make_writable (res);

  GST_DEBUG_OBJECT (trans, "fixating %" GST_PTR_FORMAT, res);

  res = gst_caps_fixate (res);

  return res;
}

static GstCaps *
gst_video_rga_transform_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstVideoRga *rga = GST_VIDEO_RGA (btrans);
  gint width, height;
  guint i;
  GstCaps *tmp, *result;
  GstStructure *str;
  GstCapsFeatures *ftr;

  GST_DEBUG_OBJECT (rga, "got caps %" GST_PTR_FORMAT " filter %"
      GST_PTR_FORMAT, caps, filter);

  tmp = gst_caps_copy (caps);

  for (i = 0; i < gst_caps_get_size (tmp); i++) {
    GstStructure *structure = gst_caps_get_structure (tmp, i);
    //gint par_n, par_d;

    if (gst_structure_get_int (structure, "width", &width) &&
        gst_structure_get_int (structure, "height", &height)) {

      switch (rga->flip) {
        case GST_VIDEO_RGA_ROTATION_90R:
        case GST_VIDEO_RGA_ROTATION_90L:
          gst_structure_set (structure, "width", G_TYPE_INT, height,
              "height", G_TYPE_INT, width, NULL);
          break;
        case GST_VIDEO_RGA_ROTATION_NONE:
        case GST_VIDEO_RGA_ROTATION_180:
          gst_structure_set (structure, "width", G_TYPE_INT, width,
              "height", G_TYPE_INT, height, NULL);
          break;
        default:
          g_assert_not_reached ();
          break;
      }
    }
  }

  GST_DEBUG_OBJECT (rga, "transformed %" GST_PTR_FORMAT " to %"
      GST_PTR_FORMAT, caps, tmp);

  /* rga caps remove format info */
  result = gst_caps_new_empty ();

  for (i = 0; i < gst_caps_get_size (tmp); i++) {
    str = gst_caps_get_structure (tmp, i);
    ftr = gst_caps_get_features (tmp, i);

    /* If this is already expressed by the existing caps
     * skip this structure */
    if (i > 0 && gst_caps_is_subset_structure_full (result, str, ftr))
      continue;

    str = gst_structure_copy (str);
    if (!gst_caps_features_is_any (ftr)
        && gst_caps_features_is_equal (ftr,
            GST_CAPS_FEATURES_MEMORY_SYSTEM_MEMORY))
      gst_structure_remove_fields (str, "format", "colorimetry", "chroma-site",
          NULL);

    gst_caps_append_structure_full (result, str, gst_caps_features_copy (ftr));
  }

  gst_caps_unref (tmp);

  GST_DEBUG_OBJECT (rga, "Intersect %" GST_PTR_FORMAT " to %"
      GST_PTR_FORMAT, result, filter);

  if (filter) {
    GstCaps *intersection;

    GST_DEBUG_OBJECT (rga, "Using filter caps %" GST_PTR_FORMAT, filter);
    intersection =
        gst_caps_intersect_full (filter, result, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (result);
    result = intersection;

    GST_DEBUG_OBJECT (rga, "Intersection %" GST_PTR_FORMAT, result);
  }

  GST_DEBUG_OBJECT (rga, "returning caps: %" GST_PTR_FORMAT, result);
  return result;
}

static gboolean
gst_video_rga_set_info (GstVideoFilter * filter,
    GstCaps * incaps, GstVideoInfo * in_info, GstCaps * outcaps,
    GstVideoInfo * out_info)
{
  GstVideoRga *rga = GST_VIDEO_RGA_CAST (filter);
  const char *fmt;

  /* if present, these must match too */
  if (in_info->par_n != out_info->par_n || in_info->par_d != out_info->par_d)
    goto format_mismatch;

  /* if present, these must match too */
  if (in_info->interlace_mode != out_info->interlace_mode)
    goto format_mismatch;

  rga->src_img.width = in_info->width;
  rga->src_img.height = in_info->height;
  fmt = gst_video_format_to_string (GST_VIDEO_INFO_FORMAT (in_info));
  rga->src_img.format_name = fmt;
  rga->src_img.format = gst_to_rga_format (fmt);
  rga->src_img.crop_w = rga->src_img.width;
  rga->src_img.crop_h = rga->src_img.height;
  rga->src_img.off_x = 0;
  rga->src_img.off_y = 0;

  GST_INFO_OBJECT (rga, " in: width %d, height %d format %s", rga->src_img.width,
      rga->src_img.height, fmt);

  rga->dst_img.width = in_info->width;
  rga->dst_img.height = in_info->height;
  fmt = gst_video_format_to_string (GST_VIDEO_INFO_FORMAT (out_info));
  rga->dst_img.format_name = fmt;
  rga->dst_img.format = gst_to_rga_format (fmt); //GST_RGA_VIDEO_FORMAT_BGRA
  rga->dst_img.crop_w = rga->dst_img.width;
  rga->dst_img.crop_h = rga->dst_img.height;
  rga->dst_img.off_x = 0;
  rga->dst_img.off_y = 0;

  GST_INFO_OBJECT (rga, " out: width %d, height %d format %s", rga->dst_img.width,
      rga->dst_img.height, fmt);

  /* RGA Request do intial settings */
  rga->req.src.act_w = rga->src_img.width;
  rga->req.src.act_h = rga->src_img.height;
  rga->req.src.vir_w = rga->req.src.act_w;
  rga->req.src.vir_h = rga->req.src.act_h;
  rga->req.src.format = rga->src_img.format;
  rga->req.src.x_offset = rga->src_img.off_x;
  rga->req.src.y_offset = rga->src_img.off_y;

  rga->req.dst.act_w = rga->dst_img.width;
  rga->req.dst.act_h = rga->dst_img.height;
  rga->req.dst.vir_w = rga->req.dst.act_w;
  rga->req.dst.vir_h = rga->req.dst.act_h;
  rga->req.dst.format = rga->dst_img.format;
  rga->req.dst.x_offset = rga->dst_img.off_x;
  rga->req.dst.y_offset = rga->dst_img.off_y;

  rga->req.clip.xmin = 0;
  rga->req.clip.xmax = rga->dst_img.width - 1;
  rga->req.clip.ymin = 0;
  rga->req.clip.ymax = rga->dst_img.height - 1;

  rga->req.render_mode = 0x0;   /* bitblt_mode */

  rga->req.mmu_info.mmu_en = 1;

  if (rga->mode == GST_VIDEO_RGA_MODE_METHOD_CUSTOM) {
      //default settings
      rga->req.mmu_info.mmu_flag = ((2 & 0x3) << 4) | 1 | (1 << 8) | (1 << 10);
  } else if (rga->mode == GST_VIDEO_RGA_MODE_METHOD_MMAP) {
      /* decoder_settings */
      rga->req.mmu_info.mmu_flag  = ((2 & 0x3) << 4) | 1;
      rga->req.mmu_info.mmu_flag |=((1<<31) | (1<<10));
  }

  rga->req.alpha_rop_flag = (1 << 5);
  rga->req.scale_mode = 0;

  rga->req.sina = 0;
  rga->req.cosa = 0;
  rga->req.rotate_mode = 0;

  if (rga->flip) {
      gst_video_rga_flip (rga);
  }

  return TRUE;

  /* ERRORS */
format_mismatch:
  {
    GST_ERROR_OBJECT (rga, "input and output formats do not match");
    return FALSE;
  }
}

static void
gst_video_rga_finalize (GObject * obj)
{
  GstVideoRga *rga = GST_VIDEO_RGA (obj);

  if (rga->fd > 0)
    close (rga->fd);

  if (rga->fd_ion > 0)
    close (rga->fd_ion);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_video_rga_class_init (GstVideoRgaClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;
  GstBaseTransformClass *gstbasetransform_class =
      (GstBaseTransformClass *) klass;
  GstVideoFilterClass *gstvideofilter_class = (GstVideoFilterClass *) klass;

  gobject_class->set_property = gst_video_rga_set_property;
  gobject_class->get_property = gst_video_rga_get_property;
  gobject_class->finalize = gst_video_rga_finalize;

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_video_rga_src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_video_rga_sink_template));

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_video_rga_change_state);

  gst_element_class_set_static_metadata (gstelement_class,
      "Intel Raster graphics Accelerator", "Video PostProcessor",
      "Convert/Scale/Rotate",
      "Sudip Jain <sudip.jain@intel.com>");

  gstbasetransform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_video_rga_transform_caps);
  gstbasetransform_class->fixate_caps =
      GST_DEBUG_FUNCPTR (gst_video_rga_fixate_caps);

  gstvideofilter_class->set_info = GST_DEBUG_FUNCPTR (gst_video_rga_set_info);
  gstvideofilter_class->transform_frame =
      GST_DEBUG_FUNCPTR (gst_video_rga_transform_frame);

  g_object_class_install_property (gobject_class, PROP_ROTATION,
      g_param_spec_enum ("flip", "Rotate", "Handle rotation",
          GST_TYPE_VIDEO_RGA_ROTATE_METHOD, GST_VIDEO_RGA_ROTATION_NONE,
          GST_PARAM_CONTROLLABLE | G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
          G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MODE,
      g_param_spec_enum ("mode", "io-mode", "data path selection",
          GST_TYPE_VIDEO_RGA_MODE_METHOD, GST_VIDEO_RGA_MODE_METHOD_MMAP,
          GST_PARAM_CONTROLLABLE | G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
          G_PARAM_STATIC_STRINGS));
}

static void
gst_video_rga_init (GstVideoRga * rga)
{
  GST_INFO_OBJECT (rga, "Opening device %s", RGA_DEVICE);
  if ((rga->fd = open (RGA_DEVICE, O_RDWR)) < 0)
    GST_ERROR_OBJECT (rga, "Failed to open the /dev/rga device");

  //GST_INFO_OBJECT (rga, "Opening device %s", ION_DEVICE);
  //if ((rga->fd_ion = open (ION_DEVICE, O_RDONLY)) < 0)
  //  GST_ERROR_OBJECT (rga, "Failed to open the /dev/ion device");

  memset (&rga->req, 0, sizeof (struct rga_req));
}

static void
gst_video_rga_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVideoRga *rga = GST_VIDEO_RGA (object);

  switch (property_id) {
    case PROP_MODE:
      rga->mode = g_value_get_enum (value);
      break;
    case PROP_ROTATION:
      rga->flip = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gst_video_rga_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstVideoRga *rga = GST_VIDEO_RGA (object);

  switch (property_id) {
    case PROP_MODE:
        g_value_set_enum (value, rga->mode);
      break;

    case PROP_ROTATION:
        g_value_set_enum (value, rga->flip);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static GstStateChangeReturn
gst_video_rga_change_state (GstElement * element, GstStateChange transition)
{
  GstVideoRga *rga = GST_VIDEO_RGA (element);
  GstStateChangeReturn ret;

  GST_INFO_OBJECT (rga, "transition %d", transition);
  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  return ret;
}

static void
gst_video_rga_flip (GstVideoRga * rga)
{
  switch (rga->flip) {
    case GST_VIDEO_RGA_ROTATION_90R:
      rga->req.dst.x_offset = rga->dst_img.height - 1;
      rga->req.dst.y_offset = 0;

      rga->req.clip.xmin = 0;
      rga->req.clip.xmax = rga->dst_img.height - 1;
      rga->req.clip.ymin = 0;
      rga->req.clip.ymax = rga->dst_img.width - 1;

      rga->req.dst.vir_w = rga->dst_img.height;
      rga->req.dst.vir_h = rga->dst_img.width;
      rga->req.dst.act_w = rga->dst_img.crop_w;
      rga->req.dst.act_h = rga->dst_img.crop_h;

      rga->req.sina = 65536;
      rga->req.cosa = 0;
      rga->req.rotate_mode = 1;

      break;
    case GST_VIDEO_RGA_ROTATION_180:
      rga->req.dst.x_offset = rga->dst_img.width - 1;
      rga->req.dst.y_offset = rga->dst_img.height - 1;

      rga->req.clip.xmin = 0;
      rga->req.clip.xmax = rga->dst_img.width - 1;
      rga->req.clip.ymin = 0;
      rga->req.clip.ymax = rga->dst_img.height - 1;

      rga->req.dst.vir_w = rga->dst_img.width;
      rga->req.dst.vir_h = rga->dst_img.height;
      rga->req.dst.act_w = rga->dst_img.crop_w;
      rga->req.dst.act_h = rga->dst_img.crop_h;

      rga->req.sina = 0;
      rga->req.cosa = -65536;
      rga->req.rotate_mode = 1;

      break;
    case GST_VIDEO_RGA_ROTATION_90L:
      rga->req.dst.x_offset = 0;
      rga->req.dst.y_offset = rga->dst_img.width - 1;

      rga->req.clip.xmin = 0;
      rga->req.clip.xmax = rga->dst_img.height - 1;
      rga->req.clip.ymin = 0;
      rga->req.clip.ymax = rga->dst_img.width - 1;

      rga->req.dst.vir_w = rga->dst_img.height;
      rga->req.dst.vir_h = rga->dst_img.width;
      rga->req.dst.act_w = rga->dst_img.crop_w;
      rga->req.dst.act_h = rga->dst_img.crop_h;

      rga->req.sina = -65536;
      rga->req.cosa = 0;
      rga->req.rotate_mode = 1;

      break;
    case GST_VIDEO_RGA_ROTATION_NONE:
      break;
    default:
      g_assert_not_reached ();
      break;
  }
}

static GstFlowReturn
gst_video_rga_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * in_frame, GstVideoFrame * out_frame)
{
  GstVideoRga *rga = GST_VIDEO_RGA_CAST (filter);

  GST_INFO_OBJECT (filter, "before conversion from %s -> to %s",
      GST_VIDEO_INFO_NAME (&filter->in_info),
      GST_VIDEO_INFO_NAME (&filter->out_info));

  GST_OBJECT_LOCK (rga);

  if (rga->mode == GST_VIDEO_RGA_MODE_METHOD_MMAP) {
      GST_LOG_OBJECT(filter,"--> fast path \n");
      rga->req.src.yrgb_addr = (unsigned int)GST_VIDEO_FRAME_COMP_DATA (in_frame, 0);
      rga->req.src.uv_addr = (unsigned int) GST_VIDEO_FRAME_COMP_DATA (in_frame, 1);
      rga->req.src.v_addr = 0x0;

      /* transformed frame addr */
      rga->req.dst.yrgb_addr = (guint) GST_VIDEO_FRAME_PLANE_DATA (out_frame, 0);
  }

  /* push sync process to rga, return until proccess is ending */
  if (ioctl (rga->fd, RGA_BLIT_SYNC, &rga->req) < 0)
    GST_WARNING_OBJECT (rga, "RGA operation error");

  GST_OBJECT_UNLOCK (rga);

  GST_INFO_OBJECT (filter, "after conversion from %s -> to %s",
      GST_VIDEO_INFO_NAME (&filter->in_info),
      GST_VIDEO_INFO_NAME (&filter->out_info));

  return GST_FLOW_OK;
}

static GstRgaVideoFormat
gst_to_rga_format (const char *str)
{
  GstRgaVideoFormat format;

  if (!strcasecmp (str, "NV12"))
    format = GST_RGA_VIDEO_FORMAT_NV12;
  else if (!strcasecmp (str, "I420"))
    format = GST_RGA_VIDEO_FORMAT_I420;
  else if (!strcasecmp (str, "RGB16"))
    format = GST_RGA_VIDEO_FORMAT_RGB16;
  else if (!strcasecmp (str, "BGR"))
    format = GST_RGA_VIDEO_FORMAT_BGR;
  else if (!strcasecmp (str, "BGRx"))
    format = GST_RGA_VIDEO_FORMAT_BGRA;
  else
    format = GST_RGA_VIDEO_FORMAT_UNKNOWN;
  return format;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (videorga_debug, "videorga", 0,
      "Colorspace Converter");

  _colorspace_quark = g_quark_from_static_string ("colorspace");

  return gst_element_register (plugin, "videorga",
      GST_RANK_PRIMARY + 1, GST_TYPE_VIDEO_RGA);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    videorga, "Intel Raster Graphics Accelerator", plugin_init, VERSION,
    GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
