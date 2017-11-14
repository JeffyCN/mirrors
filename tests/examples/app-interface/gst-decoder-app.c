/*
 * Author: Randy Li <ayaka@soulik.info>
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

#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include <glib/gstdio.h>
#include <gst/gst.h>
#include <gst/gstmemory.h>
#include <gst/gstpad.h>
#include <gst/allocators/gstdmabuf.h>
#include <gst/app/gstappsink.h>
#include <gst/video/gstvideometa.h>

GST_DEBUG_CATEGORY_STATIC (rk_appsink_debug);
#define GST_CAT_DEFAULT rk_appsink_debug

#define MAX_NUM_PLANES 3

static void *video_frame_loop (void *arg);

inline static const char *
yesno (int yes)
{
  return yes ? "yes" : "no";
}

struct decoder
{
  GMainLoop *loop;
  GstElement *pipeline;
  GstElement *sink;
  pthread_t gst_thread;

  gint format;
  GstVideoInfo info;

  unsigned frame;
};

static GstPadProbeReturn
pad_probe (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  struct decoder *dec = user_data;
  GstEvent *event = GST_PAD_PROBE_INFO_EVENT (info);
  GstCaps *caps;

  (void) pad;

  if (GST_EVENT_TYPE (event) != GST_EVENT_CAPS)
    return GST_PAD_PROBE_OK;

  gst_event_parse_caps (event, &caps);

  if (!caps) {
    GST_ERROR ("caps event without caps");
    return GST_PAD_PROBE_OK;
  }

  if (!gst_video_info_from_caps (&dec->info, caps)) {
    GST_ERROR ("caps event with invalid video caps");
    return GST_PAD_PROBE_OK;
  }

  switch (GST_VIDEO_INFO_FORMAT (&(dec->info))) {
    case GST_VIDEO_FORMAT_I420:
      dec->format = 2;
      break;
    case GST_VIDEO_FORMAT_NV12:
      dec->format = 23;
      break;
    case GST_VIDEO_FORMAT_YUY2:
      dec->format = 4;
      break;
    default:
      GST_ERROR ("unknown format\n");
      return GST_PAD_PROBE_OK;
  }

  return GST_PAD_PROBE_OK;
}

static gboolean
bus_watch_cb (GstBus * bus, GstMessage * msg, gpointer user_data)
{
  struct decoder *dec = (struct decoder *) user_data;

  (void) bus;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_STATE_CHANGED:{
      gchar *dotfilename;
      GstState old_gst_state, cur_gst_state, pending_gst_state;

      /* Only consider state change messages coming from
       * the toplevel element. */
      if (GST_MESSAGE_SRC (msg) != GST_OBJECT (dec->pipeline))
        break;

      gst_message_parse_state_changed (msg, &old_gst_state, &cur_gst_state,
          &pending_gst_state);

      printf ("GStreamer state change:  old: %s  current: %s  pending: %s\n",
          gst_element_state_get_name (old_gst_state),
          gst_element_state_get_name (cur_gst_state),
          gst_element_state_get_name (pending_gst_state)
          );

      dotfilename = g_strdup_printf ("statechange__old-%s__cur-%s__pending-%s",
          gst_element_state_get_name (old_gst_state),
          gst_element_state_get_name (cur_gst_state),
          gst_element_state_get_name (pending_gst_state)
          );
      GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (dec->pipeline),
          GST_DEBUG_GRAPH_SHOW_ALL, dotfilename);
      g_free (dotfilename);

      break;
    }
    case GST_MESSAGE_REQUEST_STATE:{
      GstState requested_state;
      gst_message_parse_request_state (msg, &requested_state);
      printf ("state change to %s was requested by %s\n",
          gst_element_state_get_name (requested_state),
          GST_MESSAGE_SRC_NAME (msg)
          );
      gst_element_set_state (GST_ELEMENT (dec->pipeline), requested_state);
      break;
    }
    case GST_MESSAGE_LATENCY:{
      printf ("redistributing latency\n");
      gst_bin_recalculate_latency (GST_BIN (dec->pipeline));
      break;
    }
    case GST_MESSAGE_EOS:
      g_main_loop_quit (dec->loop);
      break;
    case GST_MESSAGE_INFO:
    case GST_MESSAGE_WARNING:
    case GST_MESSAGE_ERROR:{
      GError *error = NULL;
      gchar *debug_info = NULL;
      gchar const *prefix;

      switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_INFO:
          gst_message_parse_info (msg, &error, &debug_info);
          prefix = "INFO";
          break;
        case GST_MESSAGE_WARNING:
          gst_message_parse_warning (msg, &error, &debug_info);
          prefix = "WARNING";
          break;
        case GST_MESSAGE_ERROR:
          gst_message_parse_error (msg, &error, &debug_info);
          prefix = "ERROR";
          break;
        default:
          g_assert_not_reached ();
      }
      printf ("GStreamer %s: %s; debug info: %s", prefix, error->message,
          debug_info);

      g_clear_error (&error);
      g_free (debug_info);

      if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
        GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS (GST_BIN (dec->pipeline),
            GST_DEBUG_GRAPH_SHOW_ALL, "error");
      }
      // TODO: stop mainloop in case of an error

      break;
    }
    default:
      break;
  }

  return TRUE;
}

static GstPadProbeReturn
appsink_query_cb (GstPad * pad G_GNUC_UNUSED, GstPadProbeInfo * info,
    gpointer user_data G_GNUC_UNUSED)
{
  GstQuery *query = info->data;

  if (GST_QUERY_TYPE (query) != GST_QUERY_ALLOCATION)
    return GST_PAD_PROBE_OK;

  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  return GST_PAD_PROBE_HANDLED;
}

static struct decoder *
video_init (const char *filename)
{
  struct decoder *dec;
  GstElement *src;
  GstPad *pad;
  GstBus *bus;
  static const char *pipeline = NULL;

  dec = g_new0 (struct decoder, 1);
  dec->loop = g_main_loop_new (NULL, FALSE);

  /* Setup pipeline: */
  pipeline =
      "filesrc name=\"src\" ! decodebin name=\"decode\" ! video/x-raw ! appsink sync=false name=\"sink\"";
  dec->pipeline = gst_parse_launch (pipeline, NULL);

  dec->sink = gst_bin_get_by_name (GST_BIN (dec->pipeline), "sink");

  /* Implement the allocation query using a pad probe. This probe will
   * adverstize support for GstVideoMeta, which avoid hardware accelerated
   * decoder that produce special strides and offsets from having to
   * copy the buffers.
   */
  pad = gst_element_get_static_pad (dec->sink, "sink");
  gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM,
      appsink_query_cb, NULL, NULL);
  gst_object_unref (pad);

  src = gst_bin_get_by_name (GST_BIN (dec->pipeline), "src");
  g_object_set (G_OBJECT (src), "location", filename, NULL);
  gst_object_unref (src);

  gst_base_sink_set_max_lateness (GST_BASE_SINK (dec->sink), 70 * GST_MSECOND);
  gst_base_sink_set_qos_enabled (GST_BASE_SINK (dec->sink), TRUE);

  g_object_set (G_OBJECT (dec->sink), "max-buffers", 2, NULL);

  gst_pad_add_probe (gst_element_get_static_pad (dec->sink, "sink"),
      GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, pad_probe, dec, NULL);

  /* add bus to be able to receive error message, handle latency
   * requests, produce pipeline dumps, etc. */
  bus = gst_pipeline_get_bus (GST_PIPELINE (dec->pipeline));
  gst_bus_add_watch (bus, bus_watch_cb, dec);
  gst_object_unref (GST_OBJECT (bus));

  /* let 'er rip! */
  gst_element_set_state (dec->pipeline, GST_STATE_PLAYING);

  pthread_create (&dec->gst_thread, NULL, video_frame_loop, dec);

  return dec;
}

static void *
buffer_to_file (struct decoder *dec, GstBuffer * buf)
{
  struct
  {
    int fd, offset, stride;
  } planes[MAX_NUM_PLANES];
  GstVideoMeta *meta = gst_buffer_get_video_meta (buf);
  guint nmems = gst_buffer_n_memory (buf);
  guint nplanes = GST_VIDEO_INFO_N_PLANES (&(dec->info));
  guint i;
  guint width, height;
  gboolean is_dmabuf_mem;
  GstMemory *mem;
  GstMapInfo map_info;
  gchar filename[128];

  /* TODO: Query gst_is_dmabuf_memory() here, since the gstmemory
   * block might get merged below by gst_buffer_map(), meaning
   * that the mem pointer would become invalid */
  mem = gst_buffer_peek_memory (buf, 0);

  /* TODO: use the DMABUF directly */

  gst_buffer_map (buf, &map_info, GST_MAP_READ);

  /* Usually, a videometa should be present, since by using the internal kmscube
   * video_appsink element instead of the regular appsink, it is guaranteed that
   * video meta support is declared in the video_appsink's allocation query.
   * However, this assumes that upstream elements actually look at the allocation
   * query's contents properly, or that they even send a query at all. If this
   * is not the case, then upstream might decide to push frames without adding
   * a meta. It can happen, and in this case, look at the video info data as
   * a fallback (it is computed out of the input caps).
   */
  if (meta) {
    for (i = 0; i < nplanes; i++) {
      planes[i].offset = meta->offset[i];
      planes[i].stride = meta->stride[i];
    }
  } else {
    for (i = 0; i < nplanes; i++) {
      planes[i].offset = GST_VIDEO_INFO_PLANE_OFFSET (&(dec->info), i);
      planes[i].stride = GST_VIDEO_INFO_PLANE_STRIDE (&(dec->info), i);
    }
  }

  width = GST_VIDEO_INFO_WIDTH (&(dec->info));
  height = GST_VIDEO_INFO_HEIGHT (&(dec->info));

  /* output some information at the beginning (= when the first frame is handled) */
  if (dec->frame == 0) {
    GstVideoFormat pixfmt;
    const char *pixfmt_str;

    pixfmt = GST_VIDEO_INFO_FORMAT (&(dec->info));
    pixfmt_str = gst_video_format_to_string (pixfmt);

    printf ("===================================\n");
    printf ("GStreamer video stream information:\n");
    printf ("  size: %u x %u pixel\n", width, height);
    printf ("  pixel format: %s  number of planes: %u\n", pixfmt_str, nplanes);
    printf ("  can use zero-copy: %s\n", yesno (is_dmabuf_mem));
    printf ("  video meta found: %s\n", yesno (meta != NULL));
    printf ("===================================\n");
  }

  g_snprintf (filename, sizeof (filename), "img%05d.%s", dec->frame,
      pixfmt_str);
  g_file_set_contents (filename, map_info.data, map_info.size, NULL);

  gst_buffer_unmap (buf, &map_info);

  return 0;
}

static void *
video_frame_loop (void *arg)
{
  struct decoder *dec = arg;
  do {
    GstSample *samp;
    GstBuffer *buf;

    samp = gst_app_sink_pull_sample (GST_APP_SINK (dec->sink));
    if (!samp) {
      GST_DEBUG ("got no appsink sample");
      if (gst_app_sink_is_eos (GST_APP_SINK (dec->sink)))
        GST_DEBUG ("eos");
      return NULL;
    }

    buf = gst_sample_get_buffer (samp);
    buffer_to_file (dec, buf);

    gst_sample_unref (samp);
    dec->frame++;
  } while (1);

}

static void
video_clean (struct decoder *dec)
{
  gst_element_set_state (dec->pipeline, GST_STATE_NULL);
  pthread_join (dec->gst_thread, 0);
  gst_object_unref (dec->pipeline);
  g_main_loop_quit (dec->loop);
  g_main_loop_unref (dec->loop);
  g_free (dec);
}

gint
main (gint argc, gchar * argv[])
{
  struct decoder *dec = NULL;
  gchar *input;

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (rk_appsink_debug, "rk_appsink", 0, "App sink");

  if (argc < 2)
    return -EINVAL;

  input = g_strdup (argv[1]);
  dec = video_init (input);

  g_main_loop_run (dec->loop);

  g_free (input);
  video_clean (dec);
}
