# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include common.mk

libcamera_metadata_OBJS = libcamera_metadata/src/camera_metadata.o
libcamera_metadata_CPPFLAGS = -Ilibcamera_metadata/include

$(eval $(call add_object_rules,$(libcamera_metadata_OBJS),CC,c,CFLAGS))

CC_STATIC_LIBRARY(libcamera_metadata/libcamera_metadata.pic.a): \
	CPPFLAGS += $(libcamera_metadata_CPPFLAGS)
CC_STATIC_LIBRARY(libcamera_metadata/libcamera_metadata.pic.a): \
	$(libcamera_metadata_OBJS)

clean: CLEAN(libcamera_metadata/libcamera_metadata.pic.a)

libcamera_metadata/all: \
	CC_STATIC_LIBRARY(libcamera_metadata/libcamera_metadata.pic.a)

.PHONY: libcamera_metadata/all
