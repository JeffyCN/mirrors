plugin_LTLIBRARIES = libgstrksink.la

libgstrksink_la_SOURCES = \
	gstrksink.c					\
	gstkmssink.c            			\
	gstkmsutils.c					\
	rkximage/ximagesink.c                           \
	eglgles/gsteglglessink.c               		\
	eglgles/gstegladaptation.c			\
	eglgles/gstegladaptation_egl.c			\
	eglgles/video_platform_wrapper.c		\
	eglgles/egl.c					\
	$(NULL)

libgstrksink_la_CFLAGS =			\
	-I$(top_srcdir)/gst-libs                \
	-I$(top_builddir)/gst-libs              \
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_ALLOCATORS_CFLAGS)                \
	$(GST_CFLAGS)				\
	$(DRM_CFLAGS)                           \
	$(X_CFLAGS)                             \
	$(EGLGLES_CFLAGS)			\
	$(NULL)

libgstrksink_la_LIBADD =			\
	$(top_builddir)/gst-libs/gst/vpudec/libgstvpu-base.la \
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(DRM_LIBS)                             \
	$(X_LIBS)				\
	$(EGLGLES_LIBS)				\
	$(NULL)

libgstrksink_la_LDFLAGS =		\
	$(GST_PLUGIN_LDFLAGS)		\
	$(NULL)

libgstrksink_la_LIBTOOLFLAGS =		\
	$(GST_PLUGIN_LIBTOOLFLAGS)	\
	$(NULL)

noinst_HEADERS = \
	gstkmssink.h                            \
	gstkmsutils.h                           \
	$(NULL)
