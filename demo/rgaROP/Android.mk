#
# Copyright (C) 2018  Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
# Authors:
#     lihuang <putin.li@rock-chips.com>
#     libin <bin.li@rock-chips.com>
#
# This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# BY DOWNLOADING, INSTALLING, COPYING, SAVING OR OTHERWISE USING THIS SOFTWARE,
# YOU ACKNOWLEDGE THAT YOU AGREE THE SOFTWARE RECEIVED FORM ROCKCHIP IS PROVIDED
# TO YOU ON AN "AS IS" BASIS and ROCKCHP DISCLAIMS ANY AND ALL WARRANTIES AND
# REPRESENTATIONS WITH RESPECT TO SUCH FILE, WHETHER EXPRESS, IMPLIED, STATUTORY
# OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF TITLE,
# NON-INFRINGEMENT, MERCHANTABILITY, SATISFACTROY QUALITY, ACCURACY OR FITNESS FOR
# A PARTICULAR PURPOSE.
#

LOCAL_PATH:= $(call my-dir)
#======================================================================
#
#rgaBlit
#
#======================================================================
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include

LOCAL_C_INCLUDES += hardware/rockchip/librga
LOCAL_C_INCLUDES += hardware/rk29/librga

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware \
    librga

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_C_INCLUDES += bionic
endif

ifeq ($(strip $(BOARD_USE_DRM)), true)
ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.9)))
LOCAL_CFLAGS += -DANDROID_7_DRM
endif
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_CFLAGS += -DRK3368
endif

ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 8.0)))
LOCAL_CFLAGS += -DANDROID_8
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3188)
LOCAL_CFLAGS += -DRK3188
endif

LOCAL_SRC_FILES:= \
	RockchipFileOps.cpp \
        rgaRop.cpp

LOCAL_MODULE:= rgaRop

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

