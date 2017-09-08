/*
 * GStreamer simple video player
 *
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

#include <glib/gstdio.h>
#include <string.h>
#include <gst/gst.h>

typedef struct _PlayerData
{
  GstElement *pipeline;
  GMainLoop *main_loop;
  GList *playlist;
  GstElement *vsink;
  GstElement *asink;
} PlayerData;

static gboolean
exit_the_current_video (PlayerData * data)
{
  GList *list = NULL;
  list = g_list_previous (data->playlist);
  if (list) {
    gchar *uri = list->data;
    data->playlist = list;
    gst_element_set_state (data->pipeline, GST_STATE_NULL);
    g_object_set (G_OBJECT (data->pipeline), "uri", uri, NULL);
    g_print ("Now playing: %s\n", uri);
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
  } else {
    g_main_loop_quit (data->main_loop);
  }

  return FALSE;
}

/* Process messages from GStreamer */
static gboolean
handle_message (GstBus * bus, GstMessage * msg, PlayerData * data)
{
  GError *err;
  gchar *debug_info;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (msg, &err, &debug_info);
      g_printerr ("Error received from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), err->message);
      g_printerr ("Debugging information: %s\n",
          debug_info ? debug_info : "none");
      g_clear_error (&err);
      g_free (debug_info);
      g_main_loop_quit (data->main_loop);
      break;
    case GST_MESSAGE_EOS:{
      g_print ("End-Of-Stream reached.\n");
      exit_the_current_video (data);
    }
      break;
    case GST_MESSAGE_STATE_CHANGED:{
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed (msg, &old_state, &new_state,
          &pending_state);
    }
      break;
    case GST_MESSAGE_ASYNC_DONE:{
      g_print ("async done.\n");
    }
      break;
    default:
      break;
  }

  /* We want to keep receiving messages */
  return TRUE;
}

static gboolean
seek_to_offsets (GstElement * pipeline, gint64 seconds)
{
  gint64 pos;
  GstState state;

  if (GST_STATE_CHANGE_SUCCESS != gst_element_get_state (pipeline, &state, NULL,
          GST_SECOND))
    return FALSE;
  if (state != GST_STATE_PLAYING && state != GST_STATE_PAUSED)
    return FALSE;

  if (gst_element_query_position (pipeline, GST_FORMAT_TIME, &pos)) {
    return gst_element_seek_simple (pipeline, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
        pos + seconds * GST_SECOND);
  }

  return FALSE;
}

static gboolean
keypress (GIOChannel * source, GIOCondition condition, PlayerData * data)
{
  gchar *str = NULL;

  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) == G_IO_STATUS_NORMAL) {
    gchar c = str[0];
    switch (c) {
      case 'x':
        g_idle_add ((GSourceFunc) exit_the_current_video, (gpointer) data);
        break;
      case 'q':
        g_main_loop_quit (data->main_loop);
        break;
      case 'n':{
        gboolean result = FALSE;
        GstEvent *event = gst_event_new_eos ();
        result = gst_element_send_event (data->pipeline, event);
        if (!result)
          g_warning ("send eos failed");
      }
        break;
      case 'p':{
        GstState state;

        if (GST_STATE_CHANGE_SUCCESS != gst_element_get_state
            (data->pipeline, &state, NULL, GST_SECOND))
          break;

        if (state == GST_STATE_PLAYING)
          gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
        if (state == GST_STATE_PAUSED)
          gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
      }
        break;
      case 'f':
        seek_to_offsets (data->pipeline, 10L);
        break;
      case 'b':
        seek_to_offsets (data->pipeline, -5L);
        break;
      default:
        break;
    }
  }

  g_free (str);
  return TRUE;
}

static gboolean
parse_video_path (PlayerData * data, gchar * path)
{
  if (g_file_test (path, G_FILE_TEST_IS_DIR)) {
    GError *err = NULL;
    const gchar *file = NULL;
    gchar *uri = NULL;
    GDir *dir = g_dir_open (path, 0, &err);
    GList *first = NULL, *last = NULL;
    if (err) {
      g_print ("Error open dir %s: %s\n", path, err->message);
      return FALSE;
    }
    do {
      file = g_dir_read_name (dir);
      uri = g_strdup_printf ("%s/%s", path, file);
      if (g_file_test (uri, G_FILE_TEST_EXISTS)) {
        g_free (uri);
        uri = g_strdup_printf ("file://%s/%s", path, file);
        data->playlist = g_list_prepend (data->playlist, uri);
      } else if (errno) {
        g_free (uri);
        g_print ("Error list file in directory %d\n", errno);
        break;
      }
    } while (file);
    last = data->playlist;
    data->playlist = g_list_reverse (data->playlist);
    /* NOTE: After it is connected, many functions won't work */
    first = data->playlist;
    first->prev = last;

    g_dir_close (dir);
    return TRUE;
  } else if (g_file_test (path, G_FILE_TEST_EXISTS)) {
    gchar *file = NULL;

    file = g_strdup_printf ("file://%s", path);
    data->playlist = g_list_prepend (data->playlist, file);
    data->playlist = g_list_prepend (data->playlist, NULL);
    data->playlist = g_list_reverse (data->playlist);
    data->playlist = g_list_last (data->playlist);

    return TRUE;
  }
  return FALSE;
}

gint
main (gint argc, gchar * argv[])
{
  PlayerData data;
  GIOChannel *io_stdin = NULL;
  GstBus *bus = NULL;
  GError *err = NULL;

  gchar *uri = NULL, *path = NULL, *vsink = NULL, *asink = NULL;
  GOptionEntry options[] = {
    {"uri", '\0', 0, G_OPTION_ARG_STRING, &uri, "video source URI", NULL}
    ,
    {"path", '\0', 0, G_OPTION_ARG_STRING, &path,
        "the path of a video file or directory", NULL}
    ,
    {"video-sink", '\0', 0, G_OPTION_ARG_STRING, &vsink,
        "the video sink", NULL}
    ,
    {"audio-sink", '\0', 0, G_OPTION_ARG_STRING, &asink,
        "the audio sink", NULL}
    ,
    {NULL}
  };
  GOptionContext *ctx;

  gst_init (&argc, &argv);

  ctx = g_option_context_new ("\n\na simple video player application.");
  g_option_context_add_main_entries (ctx, options, NULL);
  g_option_context_add_group (ctx, gst_init_get_option_group ());
  if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
    g_print ("Error initializing: %s\n", err->message);
    g_option_context_free (ctx);
    g_clear_error (&err);
    return -1;
  }
  g_option_context_free (ctx);

  /* Check input arguments */
  if (!uri && !path) {
    g_printerr ("Usage: %s <pipeline spec>\n", argv[0]);
    return -1;
  }
  /* Initialisation */
  memset (&data, 0, sizeof (data));

  if (uri == NULL) {
    if (!parse_video_path (&data, path))
      return -1;
  }

  if (vsink != NULL)
    data.vsink = gst_element_factory_make (vsink, "video-sink");
  if (asink != NULL)
    data.asink = gst_element_factory_make (asink, "audio-sink");

  data.main_loop = g_main_loop_new (NULL, FALSE);
  data.pipeline = gst_element_factory_make ("playbin", "play");
  g_object_set (G_OBJECT (data.pipeline), "video-sink", data.vsink, NULL);
  g_object_set (G_OBJECT (data.pipeline), "audio-sink", data.asink, NULL);

  io_stdin = g_io_channel_unix_new (fileno (stdin));
  g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) keypress, (gpointer) & data);

  bus = gst_element_get_bus (data.pipeline);
  gst_bus_add_watch (bus, (GstBusFunc) handle_message, &data);
  /* Set the pipeline to "playing" state */
  exit_the_current_video ((gpointer) & data);

  /* Looping */
  g_main_loop_run (data.main_loop);

  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (data.pipeline));

  g_io_channel_unref (io_stdin);
  gst_object_unref (bus);
  g_main_loop_unref (data.main_loop);
  g_list_free_full (data.playlist, g_free);

  return 0;
}
