LOCAL_PATH := $(call my-dir)
#############################################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES += \
	DrmRgaApi.cpp \
	DrmHandle.cpp

LOCAL_MODULE := librga.$(TARGET_BOARD_HARDWARE)

LOCAL_C_INCLUDES += external/libdrm/rockchip

LOCAL_SHARED_LIBRARIES := libdrm
LOCAL_SHARED_LIBRARIES += \
	libdrm_rockchip \
	liblog \
        libui \
        libcutils \
	libhardware

LOCAL_CFLAGS := \
        -DLOG_TAG=\"DrmRga\"

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := $(TARGET_SHLIB_SUFFIX)

include $(BUILD_SHARED_LIBRARY)
#############################################################################################
#include $(CLEAR_VARS)


#LOCAL_LDFLAGS := -Wl,--version-script,--export-dynamic
#LOCAL_CFLAGS := -DLOG_TAG=\"RGA_TEST\"
#LOCAL_CPPFLAGS := -std=c++11

#LOCAL_SRC_FILES := rockchip_rga_test.cpp

#LOCAL_SHARED_LIBRARIES := libdrm_rockchip
#LOCAL_SHARED_LIBRARIES += libdrm

#LOCAL_C_INCLUDES := \
        external/libdrm/rockchip

#LOCAL_MODULE:= rga_test
#LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

#include $(BUILD_EXECUTABLE)
############################################################################################
