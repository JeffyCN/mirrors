plugin_LTLIBRARIES = libgstvideorga.la

libgstvideorga_la_SOURCES = \
	gstvideorga.c

noinst_HEADERS = \
	gstvideorga.h

libgstvideorga_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) \
		$(GST_BASE_CFLAGS) \
		$(GST_CFLAGS)
		$(GST_VIDEORGA_CFLAGS)

libgstvideorga_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) \
		$(GST_BASE_LIBS) \
		$(GST_LIBS) \
		-lgstvideo-$(GST_API_VERSION)

libgstvideorga_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvideorga_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)
