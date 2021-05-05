plugin_LTLIBRARIES = libgstrockchipmpp.la

libgstrockchipmpp_la_SOURCES =			\
	gstmppenc.c			\
	gstmpph264enc.c				\
	gstmppjpegenc.c				\
	gstmppvp8enc.c				\
	gstmppallocator.c			\
	gstmppvideodec.c			\
	gstmppjpegdec.c				\
	gstmpp.c				\
	gstmppdec.c				\
	$(NULL)

libgstrockchipmpp_la_CFLAGS =			\
	$(GST_CFLAGS)				\
	$(GST_BASE_CFLAGS)			\
	$(GST_PLUGINS_BASE_CFLAGS)		\
	$(GST_ALLOCATORS_CFLAGS)		\
	$(GST_VIDEO_CFLAGS)			\
	$(ROCKCHIP_MPP_CFLAGS)			\
	$(RGA_CFLAGS)				\
	$(NULL)

libgstrockchipmpp_la_LIBADD =			\
	$(GST_LIBS)				\
	$(GST_BASE_LIBS)			\
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_VIDEO_LIBS)			\
	$(GST_ALLOCATORS_LIBS)			\
	$(ROCKCHIP_MPP_LIBS)			\
	$(RGA_LIBS)				\
	$(NULL)

libgstrockchipmpp_la_LDFLAGS =			\
	$(GST_PLUGIN_LDFLAGS)			\
	$(NULL)

libgstrockchipmpp_la_LIBTOOLFLAGS =		\
	$(GST_PLUGIN_LIBTOOLFLAGS)		\
	$(NULL)

noinst_HEADERS =				\
	gstmppenc.h			\
	gstmpph264enc.h				\
	gstmppjpegenc.h				\
	gstmppvp8enc.h				\
	gstmppallocator.h			\
	gstmppvideodec.h			\
	gstmppdec.h				\
	$(NULL)
