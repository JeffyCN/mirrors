plugin_LTLIBRARIES = libgstvpudec.la

libgstvpudec_la_SOURCES = \
	gstvpudec.c 

noinst_HEADERS = \
	gstvpudec.h 

libgstvpudec_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) \
			  $(GST_BASE_CFLAGS) \
			  $(GST_CFLAGS) 

libgstvpudec_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) \
			  $(GST_BASE_LIBS) \
			  $(GST_LIBS) \
			  -lgstvideo-$(GST_API_VERSION) 
		
libgstvpudec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvpudec_la_LIBTOOLFLAGS =$(GST_PLUGIN_LIBTOOLFLAGS) \
	--tag=disable-static
