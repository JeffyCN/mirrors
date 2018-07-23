LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	libcamera_metadata/src/camera_metadata.c \
	libcamera_client/src/camera_metadata.cpp

#LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds -Wno-error
#LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
#LOCAL_CPPFLAGS += -std=c++11 -Wno-error
LOCAL_CFLAGS += -std=c99
LOCAL_CPPFLAGS += -DLINUX 
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/libcamera_client/include \
	$(LOCAL_PATH)/header_files/include/system/core/include \
	$(LOCAL_PATH)/libcamera_metadata/include

#LOCAL_SHARED_LIBRARIES := \
	librkisp_ctrlloop \
	librkisp_analyzer

LOCAL_MODULE:= librkisp_metadata

ifeq ($(IS_ANDROID_OS),false)
include $(BUILD_STATIC_LIBRARY)
endif

