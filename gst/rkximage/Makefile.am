plugin_LTLIBRARIES = libgstrkximage.la

libgstrkximage_la_SOURCES =			\
	ximagesink.c				\
	rkx_kmsutils.c				\
	$(NULL)

libgstrkximage_la_CFLAGS =				\
	$(X11_CFLAGS)				\
	$(DRMROCKCHIP_CFLAGS)			\
	$(KMS_DRM_CFLAGS) 			\
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_ALLOCATORS_CFLAGS)                \
	$(GST_CFLAGS)				\
	$(NULL)

libgstrkximage_la_LIBADD =				\
	$(X11_LIBS)				\
	$(DRMROCKCHIP_LIBS)			\
	$(KMS_DRM_LIBS) 			\
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(NULL)

libgstrkximage_la_LDFLAGS =			\
	$(GST_PLUGIN_LDFLAGS)			\
	$(NULL)

libgstrkximage_la_LIBTOOLFLAGS =			\
	$(GST_PLUGIN_LIBTOOLFLAGS)		\
	$(NULL)
