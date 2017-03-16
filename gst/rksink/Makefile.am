plugin_LTLIBRARIES = libgstrkvideo.la

libgstrkvideo_la_SOURCES = \
	gstrkvideo.c					\
	$(NULL)

libgstrkvideo_la_CFLAGS =			\
	-I$(top_srcdir)/gst-libs                \
	-I$(top_builddir)/gst-libs              \
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_ALLOCATORS_CFLAGS)                \
	$(GST_CFLAGS)				\
	$(DRM_CFLAGS)                           \
	$(NULL)

libgstrkvideo_la_LIBADD =			\
	$(top_builddir)/gst-libs/gst/vpudec/libgstvpu-base.la \
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(DRM_LIBS)                             \
	$(NULL)

libgstrkvideo_la_LDFLAGS =		\
	$(GST_PLUGIN_LDFLAGS)		\
	$(NULL)

libgstrkvideo_la_LIBTOOLFLAGS =		\
	$(GST_PLUGIN_LIBTOOLFLAGS)	\
	$(NULL)

if USE_X11

libgstrkvideo_la_SOURCES +=			\
	rkximage/ximagesink.c                           \
	rkximage/rkx_kmsutils.c                         \
	eglgles/gsteglglessink.c               		\
	eglgles/gstegladaptation.c			\
	eglgles/gstegladaptation_egl.c			\
	eglgles/video_platform_wrapper.c		\
	eglgles/egl.c					\
	$(NULL)	

libgstrkvideo_la_CFLAGS +=			\
	$(X_CFLAGS)                             \
	$(EGLGLES_CFLAGS)			\
	$(NULL)

libgstrkvideo_la_LIBADD +=			\
	$(X_LIBS)				\
	$(EGLGLES_LIBS)				\
	$(NULL)

endif

if USE_RGA

libgstrkvideo_la_CFLAGS +=			\
	$(DRMROCKCHIP_CFLAGS)	\
	$(NULL)

libgstrkvideo_la_LIBADD +=			\
	$(DRMROCKCHIP_LIBS)				\
	$(NULL)

endif
