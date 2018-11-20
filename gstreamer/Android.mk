ifeq ($(IS_BUILD_GSTREAMER_PLUGIN), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES +=\
	gstrkisp.cpp \
	gstxcambuffermeta.cpp \
	gstxcambufferpool.cpp \
	media-controller.c

LOCAL_SRC_FILES +=\
	interface/gstxcaminterface.c

#gstxcamfilter.cpp \
main_pipe_manager.cpp

LOCAL_CFLAGS += -Wno-error=unused-function -Wno-array-bounds -Wno-error
LOCAL_CFLAGS += -DLINUX  -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H -DENABLE_ASSERT
LOCAL_CFLAGS += $(PRJ_CPPFLAGS)
LOCAL_CPPFLAGS += -std=c++11 -Wno-error
LOCAL_CPPFLAGS +=  -DLINUX  -DENABLE_ASSERT
LOCAL_CPPFLAGS += $(PRJ_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
    $(LOCAL_PATH)/../metadata/header_files/include \
    $(LOCAL_PATH)/../metadata/libcamera_client/include \
    $(LOCAL_PATH)/../metadata/libcamera_metadata/include \
    $(LOCAL_PATH)/../metadata/header_files/include/system/core/include \
	$(LOCAL_PATH)/../modules/isp \
	$(LOCAL_PATH)/interface \
	$(LOCAL_PATH)/../interface \
	$(LOCAL_PATH)/../xcore/ia \
	$(LOCAL_PATH)/../plugins/3a/rkiq \
	$(LOCAL_PATH)/../rkisp/isp-engine \
    $(LOCAL_PATH)/../rkisp/ia-engine \
    $(LOCAL_PATH)/../rkisp/ia-engine/include \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux \
    $(LOCAL_PATH)/../rkisp/ia-engine/include/linux/media

LOCAL_SHARED_LIBRARIES := \
	libgstbase-1.0 \
	libgstreamer-1.0 \
	libgobject-2.0 \
	libglib-2.0 \
	libgstallocators-1.0 \
	libgstvideo-1.0 \
	libgstvideo4linux2

LOCAL_SHARED_LIBRARIES += \
    libdrm \
    librkisp

LOCAL_STATIC_LIBRARIES += \
	libisp_log\

LOCAL_MODULE:= libgstrkisp

include $(BUILD_SHARED_LIBRARY)
endif
