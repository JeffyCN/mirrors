if USE_ROCKCHIPMPP
ROCKCHIPMPP_DIR=rockchipmpp
else
ROCKCHIPMPP_DIR=
endif

if USE_VPUDEC
VPUDEC_DIR=vpudec
else
VPUDEC_DIR=
endif

if USE_RKXIMAGE
RKXIMAGE_DIR=rkximage
else
RKXIMAGE_DIR=
endif

SUBDIRS = $(ROCKCHIPMPP_DIR) $(VPUDEC_DIR) $(RKXIMAGE_DIR)

DIST_SUBDIRS = rockchipmpp vpudec rkximage

include $(top_srcdir)/common/parallel-subdirs.mak
