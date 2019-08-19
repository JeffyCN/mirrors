LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES = rkisp_3A_server.cpp

LOCAL_CPPFLAGS += -std=c++11 -Wno-error
LOCAL_CPPFLAGS += -DLINUX
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../interface \
	$(LOCAL_PATH)/../../xcore \
	$(LOCAL_PATH)/../../xcore/ia \
	$(LOCAL_PATH)/../../ext/rkisp \
	$(LOCAL_PATH)/../../plugins/3a/rkiq \
	$(LOCAL_PATH)/../../modules/isp \
	$(LOCAL_PATH)/../../rkisp/ia-engine \
	$(LOCAL_PATH)/../../rkisp/ia-engine/include \
	$(LOCAL_PATH)/../../rkisp/ia-engine/include/linux \
	$(LOCAL_PATH)/../../rkisp/ia-engine/include/linux/media \
	$(LOCAL_PATH)/../../rkisp/isp-engine \

ifeq ($(IS_ANDROID_OS),true)
LOCAL_32_BIT_ONLY := true
LOCAL_SHARED_LIBRARIES += libutils libcutils liblog
LOCAL_SHARED_LIBRARIES += \
	libcamera_metadata
LOCAL_C_INCLUDES += \
    system/media/camera/include \
    frameworks/av/include
LOCAL_C_INCLUDES += \
	external/libdrm/include/drm \
	external/libdrm
ifeq (1,$(strip $(shell expr $(PLATFORM_SDK_VERSION) \>= 26)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper
LOCAL_CFLAGS += -DANDROID_VERSION_ABOVE_8_X
else
LOCAL_SHARED_LIBRARIES += \
	libcamera_metadata \
	libcamera_client
endif
else
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../../metadata/libcamera_client/include \
	$(LOCAL_PATH)/../../metadata/libcamera_metadata/include \
	$(LOCAL_PATH)/../../metadata/header_files/include/system/core/include \
	$(LOCAL_PATH)/../../ext/rkisp/usr/include/drm
endif

LOCAL_SHARED_LIBRARIES += librkisp

LOCAL_MODULE:= rkisp_3A_server

include $(BUILD_EXECUTABLE)
