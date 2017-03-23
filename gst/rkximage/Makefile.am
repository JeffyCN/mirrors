plugin_LTLIBRARIES = librkximage.la

librkximage_la_SOURCES =			\
	ximagesink.c				\
	rkx_kmsutils.c				\
	$(NULL)

librkximage_la_CFLAGS =				\
	$(X11_CFLAGS)				\
	$(DRMROCKCHIP_CFLAGS)			\
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_ALLOCATORS_CFLAGS)                \
	$(GST_CFLAGS)				\
	$(NULL)

librkximage_la_LIBADD =				\
	$(X11_LIBS)				\
	$(DRMROCKCHIP_LIBS)			\
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(NULL)

librkximage_la_LDFLAGS =			\
	$(GST_PLUGIN_LDFLAGS)			\
	$(NULL)

librkximage_la_LIBTOOLFLAGS =			\
	$(GST_PLUGIN_LIBTOOLFLAGS)		\
	$(NULL)
