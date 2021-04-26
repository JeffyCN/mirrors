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

$(info $(shell $(LOCAL_PATH)/genversion.sh Android.mk))

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \< 28)))

ifneq ($(strip $(BOARD_USE_DRM)), true)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_CFLAGS += -DROCKCHIP_GPU_LIB_ENABLE

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \> 25)))
LOCAL_CFLAGS += -DUSE_AHARDWAREBUFFER=1
endif

#LOCAL_CFLAGS += -Wall -Werror -Wunreachable-code

LOCAL_C_INCLUDES += external/tinyalsa/include
LOCAL_C_INCLUDES += hardware/rockchip/libgralloc
LOCAL_C_INCLUDES += hardware/rk29/libgralloc_ump
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_CFLAGS := \
        -DLOG_TAG=\"librga\"

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libbinder \
    libui \
    libEGL \
    libGLESv1_CM \
    libgui \
    libhardware

#has no "external/stlport" from Android 6.0 on
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
LOCAL_C_INCLUDES += \
    external/stlport/stlport

LOCAL_SHARED_LIBRARIES += \
    libstlport

LOCAL_C_INCLUDES += bionic
endif

LOCAL_SRC_FILES:= \
    core/RockchipRga.cpp \
    core/GrallocOps.cpp \
    core/NormalRga.cpp \
    core/NormalRgaApi.cpp \
    core/RgaApi.cpp \
    core/RgaUtils.cpp \
    im2d_api/im2d.cpp

ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.0)))
ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t720)
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T72X=1
LOCAL_CFLAGS += -DMALI_AFBC_GRALLOC=0
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t760)
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T76X=1
LOCAL_CFLAGS += -DMALI_AFBC_GRALLOC=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM_GPU)), mali-t860)
LOCAL_CFLAGS += -DMALI_PRODUCT_ID_T86X=1
LOCAL_CFLAGS += -DMALI_AFBC_GRALLOC=1
endif
endif #android 6.0 and later

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_CFLAGS += -DRK3368
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3188)
LOCAL_CFLAGS += -DRK3188
endif

ifeq ($(strip $(GRAPHIC_MEMORY_PROVIDER)),dma_buf)
LOCAL_CFLAGS += -DUSE_DMA_BUF
endif

LOCAL_MODULE:= librga
include $(BUILD_SHARED_LIBRARY)
endif #end of BOARD_USE_DRM=false
#############################################################################################

#############################################################################################
ifeq ($(strip $(BOARD_USE_DRM)), true)
include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
    core/RockchipRga.cpp \
    core/GrallocOps.cpp \
    core/NormalRga.cpp \
    core/NormalRgaApi.cpp \
    core/RgaApi.cpp \
    core/RgaUtils.cpp \
    im2d_api/im2d.cpp

LOCAL_MODULE := librga
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += external/libdrm/rockchip
LOCAL_C_INCLUDES += hardware/rockchip/libgralloc
LOCAL_C_INCLUDES += hardware/rk29/libgralloc_ump
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware
LOCAL_C_INCLUDES += hardware/libhardware/modules/gralloc
LOCAL_C_INCLUDES += frameworks/native/libs/nativewindow/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := libdrm
LOCAL_SHARED_LIBRARIES += \
        libdrm_rockchip \
        liblog \
        libui \
        libcutils \
        libhardware

LOCAL_CFLAGS := \
        -DLOG_TAG=\"librga\"

LOCAL_CFLAGS += -DANDROID

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \> 25)))
LOCAL_CFLAGS += -DUSE_AHARDWAREBUFFER=1
endif

ifneq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_SHARED_LIBRARIES += libgralloc_drm 
endif

ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 6.9)))
LOCAL_CFLAGS += -DANDROID_7_DRM
LOCAL_CFLAGS += -DRK_DRM_GRALLOC=1
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3368)
LOCAL_CFLAGS += -DRK3368
endif

ifeq ($(strip $(TARGET_BOARD_PLATFORM)),rk3126c)
LOCAL_CFLAGS += -DRK3126C
endif

ifneq (1,$(strip $(shell expr $(PLATFORM_VERSION) \< 8.0)))
LOCAL_CFLAGS += -DANDROID_8
endif

LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)

include $(BUILD_SHARED_LIBRARY)
endif #end of BOARD_USE_DRM = true
#############################################################################################

#############################################################################################
ifeq ($(strip $(BOARD_USE_DRM)), future)
ifeq ($(strip $(BOARD_USE_DRM)), true)
include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
    core/RockchipRga.cpp \
    core/GrallocOps.cpp \
    drm/DrmmodeRga.cpp \
    core/RgaApi.cpp \
    core/RgaUtils.cpp \
    im2d_api/im2d.cpp

LOCAL_MODULE := librga

LOCAL_C_INCLUDES += external/libdrm/rockchip
LOCAL_C_INCLUDES += hardware/rockchip/libgralloc
LOCAL_C_INCLUDES += hardware/rk29/libgralloc_ump
LOCAL_C_INCLUDES += $(LOCAL_PATH)/drm

LOCAL_SHARED_LIBRARIES := libdrm
LOCAL_SHARED_LIBRARIES += \
        libdrm_rockchip \
        liblog \
        libui \
        libcutils \
        libhardware

LOCAL_CFLAGS := \
        -DLOG_TAG=\"librga-drm\"

ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \> 25)))
LOCAL_CFLAGS += -DUSE_AHARDWAREBUFFER=1
endif

LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)

include $(BUILD_SHARED_LIBRARY)
endif #end of BOARD_USE_DRM=true
#############################################################################################
endif #end of BOARD_USE_DRM=future it will be used when upstrean in the future

endif #end of PLATFORM_SDK_VERSION < 28
include $(call first-makefiles-under,$(LOCAL_PATH))
