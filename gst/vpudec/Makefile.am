plugin_LTLIBRARIES = libgstvpudec.la

libgstvpudec_la_SOURCES = \
	gstvpudec.c \
	gstvpudecbufferpool.c

noinst_HEADERS = \
	gstvpudec.h \
	gstvpudecbufferpool.h

libgstvpudec_la_CFLAGS = \
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_BASE_CFLAGS) \
	$(GST_CFLAGS) \
	$(ROCKCHIP_MPP_CFLAGS) \
	$(ROCKCHIP_VPU_CFLAGS)

libgstvpudec_la_LIBADD = \
	$(GST_PLUGINS_BASE_LIBS) \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	-lgstvideo-$(GST_API_VERSION) \
	-lgstallocators-$(GST_API_VERSION) \
	$(ROCKCHIP_MPP_LIBS) \
	$(ROCKCHIP_VPU_LIBS)

libgstvpudec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvpudec_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS) \
	--tag=disable-static
