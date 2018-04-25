plugin_LTLIBRARIES = libgstrockchipmpp.la

libgstrockchipmpp_la_SOURCES =			\
	gstmppvideoenc.c			\
	gstmpph264enc.c				\
	gstmppjpegenc.c				\
	gstmppbarebufferpool.c			\
	gstmppdecbufferpool.c			\
	gstmppallocator.c			\
	gstmppvideodec.c			\
	gstmppjpegdec.c				\
	gstmpp.c				\
        $(NULL)

libgstrockchipmpp_la_CFLAGS =			\
        $(GST_CFLAGS)                           \
        $(GST_BASE_CFLAGS)                      \
        $(GST_PLUGINS_BASE_CFLAGS)              \
        $(GST_ALLOCATORS_CFLAGS)                \
        $(GST_VIDEO_CFLAGS)                     \
	$(ROCKCHIP_MPP_CFLAGS)                  \
        $(NULL)

libgstrockchipmpp_la_LIBADD =			\
	$(GST_LIBS)				\
	$(GST_BASE_LIBS)			\
	$(GST_PLUGINS_BASE_LIBS)		\
	$(GST_VIDEO_LIBS)                       \
	$(GST_ALLOCATORS_LIBS)                  \
	$(ROCKCHIP_MPP_LIBS)			\
	$(NULL)

libgstrockchipmpp_la_LDFLAGS =			\
	$(GST_PLUGIN_LDFLAGS)			\
	$(NULL)

libgstrockchipmpp_la_LIBTOOLFLAGS =		\
	$(GST_PLUGIN_LIBTOOLFLAGS)		\
	$(NULL)

noinst_HEADERS =				\
	gstmppvideoenc.h			\
	gstmpph264enc.h				\
	gstmppjpegenc.h				\
	gstmppbarebufferpool.h			\
	gstmppdecbufferpool.h			\
	gstmppallocator.h			\
	gstmppvideodec.h			\
	gstmppjpegdec.h				\
	$(NULL)
