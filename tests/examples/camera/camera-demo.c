/*
 * Author: Randy Li <ayaka@soulik.info> based by camerabin2-test.c
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

#include <glib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/*
 * debug logging
 */
GST_DEBUG_CATEGORY_STATIC (camerabin_test);
#define GST_CAT_DEFAULT camerabin_test

typedef struct _CaptureTiming
{
  GstClockTime start_capture;
  GstClockTime got_preview;
  GstClockTime capture_done;
  GstClockTime precapture;
  GstClockTime camera_capture;
} CaptureTiming;

typedef struct _CaptureTimingStats
{
  GstClockTime shot_to_shot;
  GstClockTime shot_to_save;
  GstClockTime shot_to_snapshot;
  GstClockTime preview_to_precapture;
  GstClockTime shot_to_buffer;
} CaptureTimingStats;

/*
 * Global vars
 */
static GstElement *camerabin = NULL;
static GstElement *viewfinder_sink = NULL;
static GMainLoop *loop = NULL;

/* commandline options */
static gchar *videosrc_name = NULL;
static gchar *videodevice_name = NULL;
static gchar *wrappersrc_name = NULL;
static gchar *vfsink_name = NULL;
static gint image_width = 0;
static gint image_height = 0;
static gint view_framerate_num = 0;
static gint view_framerate_den = 0;
static gchar *image_capture_caps_str = NULL;
static gchar *viewfinder_caps_str = NULL;
static gchar *video_capture_caps_str = NULL;
static gchar *camerabin_flags = NULL;


#define MODE_VIDEO 2
#define MODE_IMAGE 1
static gint mode = MODE_IMAGE;
static gint zoom = 100;

static gint capture_count = 0;

/* photography interface command line options */
#define EV_COMPENSATION_NONE -G_MAXFLOAT
#define APERTURE_NONE -G_MAXINT
#define FLASH_MODE_NONE -G_MAXINT
#define SCENE_MODE_NONE -G_MAXINT
#define EXPOSURE_NONE -G_MAXINT64
#define ISO_SPEED_NONE -G_MAXINT
#define WHITE_BALANCE_MODE_NONE -G_MAXINT
#define COLOR_TONE_MODE_NONE -G_MAXINT

static gchar *viewfinder_filter = NULL;

/* test configuration for common callbacks */
static GString *filename = NULL;

static gchar *preview_caps_name = NULL;

/*
 * Prototypes
 */
static gboolean run_preview_pipeline (gpointer user_data);
static void set_metadata (GstElement * camera);
static gboolean run_taking_pipeline (gpointer user_data);

static GstBusSyncReply
sync_bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  const GstStructure *st;
  const GValue *image;
  GstBuffer *buf = NULL;
  gchar *preview_filename = NULL;
  FILE *f = NULL;
  size_t written;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ELEMENT:{
      st = gst_message_get_structure (message);
      if (st) {
        if (gst_structure_has_name (st, "preview-image")) {
          GST_DEBUG ("preview-image");

          /* extract preview-image from msg */
          image = gst_structure_get_value (st, "buffer");
          if (image) {
            buf = gst_value_get_buffer (image);
            preview_filename = g_strdup_printf ("test_vga.rgb");
            f = g_fopen (preview_filename, "w");
            if (f) {
              GstMapInfo map;

              gst_buffer_map (buf, &map, GST_MAP_READ);
              written = fwrite (map.data, map.size, 1, f);
              gst_buffer_unmap (buf, &map);
              if (!written) {
                g_print ("error writing file\n");
              }
              fclose (f);
            } else {
              g_print ("error opening file for raw image writing\n");
            }
            g_free (preview_filename);
          }
        }
      }
      break;
    }
    default:
      /* unhandled message */
      break;
  }
  return GST_BUS_PASS;
}

static gboolean
bus_callback (GstBus * bus, GstMessage * message, gpointer data)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_clear_error (&err);
      g_free (debug);

      /* Write debug graph to file */
      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (camerabin),
          GST_DEBUG_GRAPH_SHOW_ALL, "camerabin.error");

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_STATE_CHANGED:
      if (GST_IS_BIN (GST_MESSAGE_SRC (message))) {
        GstState oldstate, newstate;

        gst_message_parse_state_changed (message, &oldstate, &newstate, NULL);
        GST_DEBUG_OBJECT (GST_MESSAGE_SRC (message), "state-changed: %s -> %s",
            gst_element_state_get_name (oldstate),
            gst_element_state_get_name (newstate));
      }
      break;
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      GST_INFO ("got eos() - should not happen");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ELEMENT:
      break;
    default:
      /* unhandled message */
      break;
  }
  return TRUE;
}

/*
 * Helpers
 */

static void
cleanup_pipeline (void)
{
  if (camerabin) {
    GST_INFO_OBJECT (camerabin, "stopping and destroying");
    gst_element_set_state (camerabin, GST_STATE_NULL);
    gst_object_unref (camerabin);
    camerabin = NULL;
  }
}

static gboolean
setup_pipeline_element (GstElement * element, const gchar * property_name,
    const gchar * element_name, GstElement ** res_elem)
{
  gboolean res = TRUE;
  GstElement *elem = NULL;

  if (element_name) {
    GError *error = NULL;

    elem = gst_parse_launch (element_name, &error);
    if (elem) {
      g_object_set (element, property_name, elem, NULL);
      g_object_unref (elem);
    } else {
      GST_WARNING ("can't create element '%s' for property '%s'", element_name,
          property_name);
      if (error) {
        GST_ERROR ("%s", error->message);
        g_clear_error (&error);
      }
      res = FALSE;
    }
  } else {
    GST_DEBUG ("no element for property '%s' given", property_name);
  }
  if (res_elem)
    *res_elem = elem;
  return res;
}

static void
set_camerabin_caps_from_string (void)
{
  GstCaps *caps = NULL;
  if (image_capture_caps_str != NULL) {
    caps = gst_caps_from_string (image_capture_caps_str);
    if (GST_CAPS_IS_SIMPLE (caps) && image_width > 0 && image_height > 0) {
      gst_caps_set_simple (caps, "width", G_TYPE_INT, image_width, "height",
          G_TYPE_INT, image_height, NULL);
    }
    GST_DEBUG ("setting image-capture-caps: %" GST_PTR_FORMAT, caps);
    g_object_set (camerabin, "image-capture-caps", caps, NULL);
    gst_caps_unref (caps);
  }

  if (viewfinder_caps_str != NULL) {
    caps = gst_caps_from_string (viewfinder_caps_str);
    if (GST_CAPS_IS_SIMPLE (caps) && view_framerate_num > 0
        && view_framerate_den > 0) {
      gst_caps_set_simple (caps, "framerate", GST_TYPE_FRACTION,
          view_framerate_num, view_framerate_den, NULL);
    }
    GST_DEBUG ("setting viewfinder-caps: %" GST_PTR_FORMAT, caps);
    g_object_set (camerabin, "viewfinder-caps", caps, NULL);
    gst_caps_unref (caps);
  }

  if (video_capture_caps_str != NULL) {
    caps = gst_caps_from_string (video_capture_caps_str);
    GST_DEBUG ("setting video-capture-caps: %" GST_PTR_FORMAT, caps);
    g_object_set (camerabin, "video-capture-caps", caps, NULL);
    gst_caps_unref (caps);
  }

}

static gboolean
keypress (GIOChannel * source, GIOCondition condition, gpointer data)
{
  gchar *str = NULL;

  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) == G_IO_STATUS_NORMAL) {
    gchar c = str[0];
    switch (c) {
      case 'x':
        g_main_loop_quit (loop);
        break;
      case 'c':
        g_idle_add ((GSourceFunc) run_taking_pipeline, NULL);
        break;
      default:
        break;
    }
  }

  g_free (str);
  return TRUE;
}

static gboolean
setup_pipeline (void)
{
  gboolean res = TRUE;
  GstBus *bus;
  GstElement *sink = NULL;
  GIOChannel *key_control = NULL;

  camerabin = gst_element_factory_make ("camerabin", NULL);
  if (NULL == camerabin) {
    g_warning ("can't create camerabin element\n");
    goto error;
  }

  key_control = g_io_channel_unix_new (fileno (stdin));
  g_io_add_watch (key_control, G_IO_IN, (GIOFunc) keypress, NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (camerabin));
  /* Add sync handler for time critical messages that need to be handled fast */
  gst_bus_set_sync_handler (bus, sync_bus_callback, NULL, NULL);
  /* Handle normal messages asynchronously */
  gst_bus_add_watch (bus, bus_callback, NULL);
  gst_object_unref (bus);

  GST_INFO_OBJECT (camerabin, "camerabin created");

  if (camerabin_flags)
    gst_util_set_object_arg (G_OBJECT (camerabin), "flags", camerabin_flags);
  else
    gst_util_set_object_arg (G_OBJECT (camerabin), "flags", "");

  if (videosrc_name) {
    GstElement *wrapper;
    GstElement *videosrc;

    if (wrappersrc_name)
      wrapper = gst_element_factory_make (wrappersrc_name, NULL);
    else
      wrapper = gst_element_factory_make ("wrappercamerabinsrc", NULL);

    if (setup_pipeline_element (wrapper, "video-source", videosrc_name, NULL)) {
      g_object_set (camerabin, "camera-source", wrapper, NULL);
      g_object_unref (wrapper);
    } else {
      GST_WARNING ("Failed to set videosrc to %s", videosrc_name);
    }

    g_object_get (wrapper, "video-source", &videosrc, NULL);
    if (videodevice_name == NULL)
      videodevice_name = g_strdup ("/dev/video0");
    if (videosrc && videodevice_name &&
        g_object_class_find_property (G_OBJECT_GET_CLASS (videosrc),
            "device")) {
      g_object_set (videosrc, "device", videodevice_name, NULL);
    }
  }

  /* configure used elements */
  res &=
      setup_pipeline_element (camerabin, "viewfinder-sink", vfsink_name, &sink);
  res &=
      setup_pipeline_element (camerabin, "viewfinder-filter", viewfinder_filter,
      NULL);

  GST_INFO_OBJECT (camerabin, "elements created");

  if (sink) {
    g_object_set (sink, "sync", TRUE, NULL);
  } else {
    /* Get the inner viewfinder sink, this uses fixed names given
     * by default in camerabin */
    sink = gst_bin_get_by_name (GST_BIN (camerabin), "vf-bin");
    g_assert (sink);
    gst_object_unref (sink);

    sink = gst_bin_get_by_name (GST_BIN (sink), "vfbin-sink");
    g_assert (sink);
    gst_object_unref (sink);
  }
  viewfinder_sink = sink;

  GST_INFO_OBJECT (camerabin, "elements configured");

  /* configure a resolution and framerate */
  if (image_width > 0 && image_height > 0) {
    if (mode == MODE_VIDEO) {
      GstCaps *caps = NULL;
      if (view_framerate_num > 0)
        caps = gst_caps_new_full (gst_structure_new ("video/x-raw",
                "width", G_TYPE_INT, image_width,
                "height", G_TYPE_INT, image_height,
                "framerate", GST_TYPE_FRACTION, view_framerate_num,
                view_framerate_den, NULL), NULL);
      else
        caps = gst_caps_new_full (gst_structure_new ("video/x-raw",
                "width", G_TYPE_INT, image_width,
                "height", G_TYPE_INT, image_height, NULL), NULL);

      g_object_set (camerabin, "video-capture-caps", caps, NULL);
      gst_caps_unref (caps);
    } else {
      GstCaps *caps = gst_caps_new_full (gst_structure_new ("video/x-raw",
              "width", G_TYPE_INT, image_width,
              "height", G_TYPE_INT, image_height, NULL), NULL);

      g_object_set (camerabin, "image-capture-caps", caps, NULL);
      gst_caps_unref (caps);
    }
  }

  set_camerabin_caps_from_string ();

  if (GST_STATE_CHANGE_FAILURE ==
      gst_element_set_state (camerabin, GST_STATE_READY)) {
    g_warning ("can't set camerabin to ready\n");
    goto error;
  }
  GST_INFO_OBJECT (camerabin, "camera ready");

  if (GST_STATE_CHANGE_FAILURE ==
      gst_element_set_state (camerabin, GST_STATE_PLAYING)) {
    g_warning ("can't set camerabin to playing\n");
    goto error;
  }

  GST_INFO_OBJECT (camerabin, "camera started");

  return TRUE;
error:
  cleanup_pipeline ();
  return FALSE;
}

static void
set_metadata (GstElement * camera)
{
  GstTagSetter *setter = GST_TAG_SETTER (camera);
  GstDateTime *datetime;
  gchar *desc_str;

  datetime = gst_date_time_new_now_local_time ();

  desc_str = g_strdup_printf ("captured by %s", g_get_real_name ());

  gst_tag_setter_add_tags (setter, GST_TAG_MERGE_REPLACE,
      GST_TAG_DATE_TIME, datetime,
      GST_TAG_DESCRIPTION, desc_str,
      GST_TAG_TITLE, "rockchip camera capture",
      GST_TAG_GEO_LOCATION_LONGITUDE, 1.0,
      GST_TAG_GEO_LOCATION_LATITUDE, 2.0,
      GST_TAG_GEO_LOCATION_ELEVATION, 3.0,
      GST_TAG_DEVICE_MANUFACTURER, "gst-camerabin-test manufacturer",
      GST_TAG_DEVICE_MODEL, "gst-camerabin-test model", NULL);

  g_free (desc_str);
  gst_date_time_unref (datetime);
}

static gboolean
run_taking_pipeline (gpointer user_data)
{
  gchar *filename_str = NULL;
  const gchar *filename_suffix;

  filename_suffix = ".jpg";
  filename_str =
      g_strdup_printf ("%s/test_%04u%s", filename->str, capture_count,
      filename_suffix);
  GST_DEBUG ("Setting filename: %s", filename_str);
  g_object_set (camerabin, "location", filename_str, NULL);
  g_free (filename_str);

  g_object_set (camerabin, "zoom", zoom / 100.0f, NULL);

  capture_count++;

  g_signal_emit_by_name (camerabin, "start-capture", 0);

  return FALSE;
}

static gboolean
run_preview_pipeline (gpointer user_data)
{
  GstCaps *preview_caps = NULL;

  g_object_set (camerabin, "mode", mode, NULL);

  if (preview_caps_name != NULL) {
    preview_caps = gst_caps_from_string (preview_caps_name);
    if (preview_caps) {
      g_object_set (camerabin, "preview-caps", preview_caps, NULL);
      GST_DEBUG ("Preview caps set");
    } else
      GST_DEBUG ("Preview caps set but could not create caps from string");
  }

  set_metadata (camerabin);

  return FALSE;
}

int
main (int argc, char *argv[])
{
  gchar *fn_option = NULL;

  GOptionEntry options[] = {
    {"directory", '\0', 0, G_OPTION_ARG_STRING, &fn_option,
        "Directory for capture file(s) (default is current directory)", NULL},
    {"zoom", '\0', 0, G_OPTION_ARG_INT, &zoom,
        "Zoom (100 = 1x (default), 200 = 2x etc.)", NULL},
    {"wrapper-source", '\0', 0, G_OPTION_ARG_STRING, &wrappersrc_name,
          "Camera source wrapper used for setting the video source (default is wrappercamerabinsrc)",
        NULL},
    {"video-source", '\0', 0, G_OPTION_ARG_STRING, &videosrc_name,
        "Video source used in still capture and video recording", NULL},
    {"video-device", '\0', 0, G_OPTION_ARG_STRING, &videodevice_name,
        "Video device to be set on the video source", NULL},
    {"viewfinder-sink", '\0', 0, G_OPTION_ARG_STRING, &vfsink_name,
        "Viewfinder sink (default = fakesink)", NULL},
    {"image-width", '\0', 0, G_OPTION_ARG_INT, &image_width,
        "Width for image capture", NULL},
    {"image-height", '\0', 0, G_OPTION_ARG_INT, &image_height,
        "Height for image capture", NULL},
    {"view-framerate-num", '\0', 0, G_OPTION_ARG_INT, &view_framerate_num,
        "Framerate numerator for viewfinder", NULL},
    {"view-framerate-den", '\0', 0, G_OPTION_ARG_INT, &view_framerate_den,
        "Framerate denominator for viewfinder", NULL},
    {"preview-caps", '\0', 0, G_OPTION_ARG_STRING, &preview_caps_name,
        "Preview caps (e.g. video/x-raw-rgb,width=320,height=240)", NULL},
    {"viewfinder-filter", '\0', 0, G_OPTION_ARG_STRING, &viewfinder_filter,
        "Filter to process all frames going to viewfinder sink", NULL},
    {"image-capture-caps", '\0', 0,
          G_OPTION_ARG_STRING, &image_capture_caps_str,
        "Image capture caps (e.g. video/x-raw-rgb,width=640,height=480)", NULL},
    {"viewfinder-caps", '\0', 0, G_OPTION_ARG_STRING,
          &viewfinder_caps_str,
        "Viewfinder caps (e.g. video/x-raw-rgb,width=640,height=480)", NULL},
    {"video-capture-caps", '\0', 0,
          G_OPTION_ARG_STRING, &video_capture_caps_str,
        "Video capture caps (e.g. video/x-raw-rgb,width=640,height=480)", NULL},
    {"flags", '\0', 0, G_OPTION_ARG_STRING, &camerabin_flags,
        "camerabin element flags (default = 0)", NULL},
    {NULL}
  };

  GOptionContext *ctx;
  GError *err = NULL;

  ctx = g_option_context_new ("\n\nrockchip camera test application.");
  g_option_context_add_main_entries (ctx, options, NULL);
  g_option_context_add_group (ctx, gst_init_get_option_group ());
  if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
    g_print ("Error initializing: %s\n", err->message);
    g_option_context_free (ctx);
    g_clear_error (&err);
    exit (1);
  }
  g_option_context_free (ctx);

  GST_DEBUG_CATEGORY_INIT (camerabin_test, "camerabin-test", 0,
      "camerabin test");

  if (vfsink_name == NULL)
    vfsink_name = g_strdup ("waylandsink");

  filename = g_string_new (fn_option);
  if (filename->len == 0)
    filename = g_string_append (filename, ".");

  /* init */
  if (setup_pipeline ()) {
    loop = g_main_loop_new (NULL, FALSE);
    g_idle_add ((GSourceFunc) run_preview_pipeline, NULL);
    g_main_loop_run (loop);
    cleanup_pipeline ();
    g_main_loop_unref (loop);
  }

  /* free */
  g_string_free (filename, TRUE);
  g_free (wrappersrc_name);
  g_free (videosrc_name);
  g_free (videodevice_name);
  g_free (vfsink_name);

  return 0;
}
