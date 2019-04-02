LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
				calibdb.cpp\
				xmltags.cpp\
				calibtags.cpp\

LOCAL_C_INCLUDES := \
				bionic\
				external/stlport/stlport\
				$(LOCAL_PATH)/../include\
				$(LOCAL_PATH)/include\
				$(LOCAL_PATH)/./calib_xml\
				$(LOCAL_PATH)/../../../xcore \

ifeq ($(IS_NEED_COMPILE_TINYXML2), true)
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../../../ext/tinyxml2 \

else
LOCAL_C_INCLUDES += \
    external/tinyxml2 \

endif


LOCAL_CPPFLAGS := -fuse-cxa-atexit -Wall -Wextra -Werror -Wno-unused -Wformat-nonliteral -g -O0 -Wno-error=unused-function 

LOCAL_CFLAGS += -DLINUX  -DMIPI_USE_CAMERIC -DHAL_MOCKUP -DCAM_ENGINE_DRAW_DOM_ONLY -D_FILE_OFFSET_BITS=64 -DHAS_STDINT_H
LOCAL_CFLAGS += -DENABLE_ASSERT
LOCAL_CFLAGS += -Wno-error=unused-function -Wno-error=unused-parameter
LOCAL_CPPFLAGS += -Wno-error=unused-function -Wno-error=unused-parameter

#LOCAL_STATIC_LIBRARIES := libtinyxml2 libstlport_static libisp_cam_calibdb libisp_ebase libisp_oslayer

LOCAL_STATIC_LIBRARIES += \
	libisp_log

LOCAL_MODULE:= libisp_calibdb
ifeq ($(IS_ANDROID_OS),true)
LOCAL_SHARED_LIBRARIES += libutils libcutils liblog
ifeq (1,$(strip $(shell expr $(PLATFORM_VERSION) \>= 8.0)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES += \
system/core/libutils/include \
system/core/include \
frameworks/native/libs/binder/include
endif
endif

LOCAL_MODULE_TAGS:= optional
include $(BUILD_STATIC_LIBRARY)

