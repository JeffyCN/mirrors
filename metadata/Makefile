# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CC = gcc
CFLAGS = -Ilibcamera_metadata/include/ -Iheader_files/include/system/core/include/
CFLAGS += -Ilibcamera_client/include/
CFLAGS += -std=c99

all: libcamera_metadata.o libcamera_client.o
	$(CC) -c $(CFLAGS) $< -o $@

libcamera_metadata.o: libcamera_metadata/src/camera_metadata.c
	$(CC) -c $(CFLAGS) $< -o $@

libcamera_client.o: libcamera_client/src/camera_metadata.cc
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o
