plugin_LTLIBRARIES = libgstvpudec.la

libgstvpudec_la_SOURCES = \
	gstvpu.c		\
	gstvpudec.c		\
	gstvpudecbufferpool.c	\
	gstvpuallocator.c	\
	gstkmssink.c		\
	gstkmsutils.c                           \
	ximagesink.c                           \
	$(NULL)

libgstvpudec_la_CFLAGS =			\
	-I$(top_srcdir)/gst-libs                \
	-I$(top_builddir)/gst-libs              \
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_ALLOCATORS_CFLAGS)                \
	$(GST_CFLAGS)				\
	$(ROCKCHIP_MPP_CFLAGS)			\
	$(ROCKCHIP_VPU_CFLAGS)			\
	$(DRM_CFLAGS)                           \
	$(X_CFLAGS)                             \
	$(NULL)

libgstvpudec_la_LIBADD =			\
	$(top_builddir)/gst-libs/gst/vpudec/libgstvpu-base.la \
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(ROCKCHIP_MPP_LIBS)			\
	$(ROCKCHIP_VPU_LIBS)			\
	$(DRM_LIBS)                             \
	$(X_LIBS)                             \
	$(XSHM_LIBS)                             \
	$(NULL)

libgstvpudec_la_LDFLAGS =		\
	$(GST_PLUGIN_LDFLAGS)		\
	$(NULL)

libgstvpudec_la_LIBTOOLFLAGS =		\
	$(GST_PLUGIN_LIBTOOLFLAGS)	\
	$(NULL)

noinst_HEADERS = \
	gstvpuallocator.h			\
	gstvpudec.h 				\
	gstvpudecbufferpool.h 			\
	gstkmssink.h                            \
	gstkmsutils.h                           \
	$(NULL)
