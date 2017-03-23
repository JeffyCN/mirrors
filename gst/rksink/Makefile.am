plugin_LTLIBRARIES = libgstrkvideo.la

libgstrkvideo_la_SOURCES =			\
	gstrkvideo.c					\
	$(NULL)

libgstrkvideo_la_CFLAGS =			\
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_ALLOCATORS_CFLAGS)                \
	$(GST_CFLAGS)				\
	$(KMS_DRM_CFLAGS)			\
	$(NULL)

libgstrkvideo_la_LIBADD =			\
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(KMS_DRM_LIBS)                         \
	$(NULL)

libgstrkvideo_la_LDFLAGS =			\
	$(GST_PLUGIN_LDFLAGS)			\
	$(NULL)

libgstrkvideo_la_LIBTOOLFLAGS =			\
	$(GST_PLUGIN_LIBTOOLFLAGS)		\
	$(NULL)

if USE_X11
libgstrkvideo_la_SOURCES +=			\
	rkximage/ximagesink.c			\
	rkximage/rkx_kmsutils.c			\
	$(NULL)	

if USE_EGL_X11
libgstrkvideo_la_SOURCES +=			\
	eglgles/gsteglglessink.c		\
	eglgles/gstegladaptation.c		\
	eglgles/gstegladaptation_egl.c		\
	eglgles/video_platform_wrapper.c	\
	eglgles/egl.c				\
	$(NULL)	
endif

libgstrkvideo_la_CFLAGS +=			\
	$(X11_CFLAGS)				\
	$(EGLGLES_CFLAGS)			\
	$(NULL)

libgstrkvideo_la_LIBADD +=			\
	$(X11_LIBS)				\
	$(EGLGLES_LIBS)				\
	$(NULL)
endif

if USE_RGA
libgstrkvideo_la_CFLAGS +=			\
	$(DRMROCKCHIP_CFLAGS)			\
	$(NULL)

libgstrkvideo_la_LIBADD +=			\
	$(DRMROCKCHIP_LIBS)			\
	$(NULL)
endif
