#ifndef __GST_X_IMAGE_SINK_H__
#define __GST_X_IMAGE_SINK_H__

#include <gst/video/gstvideosink.h>

#ifdef HAVE_XSHM
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif /* HAVE_XSHM */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string.h>
#include <math.h>

/* Helper functions */
#include <gst/video/video.h>

G_BEGIN_DECLS
#define GST_TYPE_X_IMAGE_SINK \
  (gst_x_image_sink_get_type())
#define GST_X_IMAGE_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_X_IMAGE_SINK, GstRkXImageSink))
#define GST_X_IMAGE_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_X_IMAGE_SINK, GstRkXImageSinkClass))
#define GST_IS_X_IMAGE_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_X_IMAGE_SINK))
#define GST_IS_X_IMAGE_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_X_IMAGE_SINK))

typedef struct _GstXContext GstXContext;
typedef struct _GstXWindow GstXWindow;

typedef struct _GstRkXImageSink GstRkXImageSink;
typedef struct _GstRkXImageSinkClass GstRkXImageSinkClass;

/*
 * GstXContext:
 * @disp: the X11 Display of this context
 * @screen: the default Screen of Display @disp
 * @screen_num: the Screen number of @screen
 * @visual: the default Visual of Screen @screen
 * @root: the root Window of Display @disp
 * @white: the value of a white pixel on Screen @screen
 * @black: the value of a black pixel on Screen @screen
 * @depth: the color depth of Display @disp
 * @bpp: the number of bits per pixel on Display @disp
 * @endianness: the endianness of image bytes on Display @disp
 * @width: the width in pixels of Display @disp
 * @height: the height in pixels of Display @disp
 * @widthmm: the width in millimeters of Display @disp
 * @heightmm: the height in millimeters of Display @disp
 *
 * Structure used to store various informations collected/calculated for a
 * Display.
 */
struct _GstXContext
{
  Display *disp;

  Screen *screen;
  gint screen_num;

  Visual *visual;

  Window root;

  gulong white, black;

  gint depth;
  gint bpp;

  gint width, height;
  gint widthmm, heightmm;
};

/*
 * GstXWindow:
 * @win: the Window ID of this X11 window
 * @width: the width in pixels of Window @win
 * @height: the height in pixels of Window @win
 * @internal: used to remember if Window @win was created internally or passed
 * through the #GstVideoOverlay interface
 * @gc: the Graphical Context of Window @win
 *
 * Structure used to store informations about a Window.
 */
struct _GstXWindow
{
  Window win;
  gint width, height;
  gboolean internal;
  GC gc;
};

/**
 * GstRkXImageSink:
 * @display_name: the name of the Display we want to render to
 * @xcontext: our instance's #GstXContext
 * @xwindow: the #GstXWindow we are rendering to
 * @ximage: internal #GstXImage used to store incoming buffers and render when
 * not using the buffer_alloc optimization mechanism
 * @cur_image: a reference to the last #GstXImage that was put to @xwindow. It
 * is used when Expose events are received to redraw the latest video frame
 * @event_thread: a thread listening for events on @xwindow and handling them
 * @running: used to inform @event_thread if it should run/shutdown
 * @fps_n: the framerate fraction numerator
 * @fps_d: the framerate fraction denominator
 * @x_lock: used to protect X calls as we are not using the XLib in threaded
 * mode
 * @flow_lock: used to protect data flow routines from external calls such as
 * events from @event_thread or methods from the #GstVideoOverlay interface
 * @par: used to override calculated pixel aspect ratio from @xcontext
 * @pool_lock: used to protect the buffer pool
 * @buffer_pool: a list of #GstXImageBuffer that could be reused at next buffer
 * allocation call
 * @synchronous: used to store if XSynchronous should be used or not (for
 * debugging purpose only)
 * @handle_events: used to know if we should handle select XEvents or not
 *
 * The #GstRkXImageSink data structure.
 */
struct _GstRkXImageSink
{
  /* Our element stuff */
  GstVideoSink videosink;

  gint fd;
  gint conn_id;
  gint crtc_id;
  gint plane_id;
  guint pipe;

  guint16 hdisplay, vdisplay;
  guint32 buffer_id;

  /* capabilities */
  gboolean has_prime_import;
  gboolean has_prime_export;
  gboolean has_async_page_flip;

  char *display_name;

  GstXContext *xcontext;
  GstXWindow *xwindow;
  GstBuffer *cur_image;
  GstVideoRectangle clip_rect;

  GThread *event_thread;
  gboolean running;

  /* Framerate numerator and denominator */
  gint fps_n;
  gint fps_d;
  gint par_n;
  gint par_d;

  GMutex x_lock;
  GMutex flow_lock;

  gboolean synchronous;
  gboolean handle_events;
  gboolean handle_expose;
  gboolean draw_border;

  /* stream metadata */
  gchar *media_title;

  GstVideoInfo vinfo;
  GstCaps *allowed_caps;
  GstBufferPool *pool;
  GstAllocator *allocator;
  GstBuffer *last_buffer;

  gchar *devname;

  guint32 mm_width, mm_height;

  GstPoll *poll;
  GstPollFD pollfd;

  guint32 last_fb_id;
  GstVideoRectangle save_rect;
  gboolean paused;
};

struct _GstRkXImageSinkClass
{
  GstVideoSinkClass parent_class;
};

GType gst_x_image_sink_get_type (void);

G_END_DECLS
#endif /* __GST_X_IMAGE_SINK_H__ */
