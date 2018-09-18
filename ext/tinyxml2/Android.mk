ifeq ($(IS_NEED_COMPILE_TINYXML2),true)
LOG_TO_ANDROID_LOGCAT := false

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= tinyxml2.cpp

LOCAL_MODULE:=libtinyxml2
LOCAL_MODULE_TAGS := optional

ifeq ($(LOG_TO_ANDROID_LOGCAT),true)
LOCAL_CFLAGS+= -DDEBUG -DANDROID_NDK
LOCAL_SHARED_LIBRARIES+= libcutils
endif

include $(BUILD_STATIC_LIBRARY)
endif
