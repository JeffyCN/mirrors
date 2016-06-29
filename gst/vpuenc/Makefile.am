plugin_LTLIBRARIES = libgstvpuenc.la

libgstvpuenc_la_SOURCES = \
	gstvpuenc.c

noinst_HEADERS = \
	gstvpuenc.h

libgstvpuenc_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstvpuenc_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstvideo-$(GST_API_VERSION) \
	-lgstallocators-$(GST_API_VERSION)

libgstvpuenc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvpuenc_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS) \
	--tag=disable-static
