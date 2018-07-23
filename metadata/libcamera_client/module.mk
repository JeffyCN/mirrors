# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include common.mk

libcamera_client_OBJS = libcamera_client/src/camera_metadata.o
libcamera_client_CPPFLAGS = -Ilibcamera_metadata/include -Ilibcamera_client/include

$(eval $(call add_object_rules,$(libcamera_client_OBJS),CC,cc,CPPFLAGS))

CC_STATIC_LIBRARY(libcamera_client/libcamera_client.pic.a): \
	CPPFLAGS += $(libcamera_client_CPPFLAGS)
CC_STATIC_LIBRARY(libcamera_client/libcamera_client.pic.a): \
	$(libcamera_client_OBJS)

clean: CLEAN(libcamera_client/libcamera_client.pic.a)

libcamera_client/all: \
	CC_STATIC_LIBRARY(libcamera_client/libcamera_client.pic.a)

.PHONY: libcamera_client/all
