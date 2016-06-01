plugin_LTLIBRARIES = libgstvpudec.la
VPU_LIBS = \
	-lion \
	-lrk_codec \
	-lvpu_avcdec \
	-lvpu_mpeg4dec \
	-lvpu \
	-lvpu_vp8dec

libgstvpudec_la_SOURCES = \
	gstvpudec.c \
	gstvpudecbufferpool.c

noinst_HEADERS = \
	gstvpudec.h \
	gstvpudecbufferpool.h

libgstvpudec_la_CFLAGS = \
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_BASE_CFLAGS) \
	$(GST_CFLAGS)

libgstvpudec_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstvideo-$(GST_API_VERSION) \
	-lgstallocators-$(GST_API_VERSION) \
	$(VPU_LIBS)

libgstvpudec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvpudec_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS) \
	--tag=disable-static
