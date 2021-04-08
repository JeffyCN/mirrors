if USE_ROCKCHIPMPP
ROCKCHIPMPP_DIR=rockchipmpp
else
ROCKCHIPMPP_DIR=
endif

if USE_RKXIMAGE
RKXIMAGE_DIR=rkximage
else
RKXIMAGE_DIR=
endif

SUBDIRS = $(ROCKCHIPMPP_DIR) $(RKXIMAGE_DIR)

DIST_SUBDIRS = rockchipmpp rkximage

include $(top_srcdir)/common/parallel-subdirs.mak
