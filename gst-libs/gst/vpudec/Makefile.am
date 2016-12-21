noinst_LTLIBRARIES =				\
	libgstvpu-base.la			\
	$(NULL)

nodist_libgstvpu_base_la_SOURCES = \
	$(NULL)

libgstvpu_base_la_CFLAGS =			\
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_BASE_CFLAGS)			\
	$(GST_VIDEO_CFLAGS)			\
	$(GST_CFLAGS)				\
	$(ROCKCHIP_MPP_CFLAGS)			\
	$(ROCKCHIP_VPU_CFLAGS)			\
	$(NULL)

libgstvpu_base_la_LIBADD =			\
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_BASE_LIBS)			\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(GST_LIBS)				\
	$(ROCKCHIP_MPP_LIBS)			\
	$(ROCKCHIP_VPU_LIBS)			\
	$(NULL)

libgstvpu_base_la_LDFLAGS =		\
	$(GST_PLUGIN_LDFLAGS)		\
libgstvpu_base_la_LIBTOOLFLAGS =		\
	$(GST_PLUGIN_LIBTOOLFLAGS)	\
	$(NUL)

-include $(top_srcdir)/git.mk
